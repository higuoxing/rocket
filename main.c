#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <readline/chardefs.h>
#include <readline/history.h>
#include <readline/readline.h>

#define PROMPT_STYLE "> "

typedef enum TokenKind {
  TK_Unknown,
  /* '(' */
  TK_LParen,
  /* ')' */
  TK_RParen,
  /* Identifier */
  TK_Ident,
  /* '\'' */
  TK_Quote,
  /* '`' */
  TK_Backquote,
  TK_Number,
} TokenKind;

typedef struct Token {
  TokenKind kind;

  /* Location of the token. */
  char *file;
  int line;
  int column;

  /* Literal representation. */
  const char *literal;
  /* Length of the literal string. */
  int tok_len;
} Token;

static Token tokens[1024];

static bool is_special_initial(char c) {
  /*
   * <special_initial> -> ! ∣ $ ∣ % ∣ & ∣ * ∣ / ∣ : ∣ < ∣ = ∣ > ∣ ? ∣ @ ∣
   *                      ^ ∣ _ ∣ ~
   */
  return c == '!' || c == '$' || c == '%' || c == '&' || c == '*' || c == '/' ||
         c == ':' || c == '<' || c == '=' || c == '>' || c == '?' || c == '@' ||
         c == '^' || c == '_' || c == '~';
}

static bool is_explicit_sign(char c) {
  /*
   * <explicit_sign> -> + ∣ -
   */
  return c == '+' || c == '-';
}

static bool is_special_subsequent(char c) {
  /*
   * <special_subsequent> -> <explicit_sign> ∣ . ∣ @
   * <explicit_sign> -> + ∣ -
   */
  return is_explicit_sign(c) || c == '.' || c == '@';
}

static bool is_initial(char c) {
  /* <initial> -> <letter> | <special_initial> */
  return isalpha(c) || is_special_initial(c);
}

static bool is_sign_subsequent(char c) {
  /* <sign_subsequent> -> <initial> ∣ <explicit_sign> ∣ @ */
  return is_initial(c) || is_explicit_sign(c) || c == '@';
}

static bool is_dot_subsequent(char c) {
  /* <dot_subsequent> -> <sign_subsequent> ∣ . */
  return is_sign_subsequent(c) || c == '.';
}

static bool is_subsequent(char c) {
  /* <subsequent> -> <initial> ∣ <digit>
   *              ∣ <special_subsequent>
   */
  return is_initial(c) || isdigit(c) || is_special_subsequent(c);
}

/*
 * Tokenize the given program and returns the number of tokens.
 */
static int tokenize(const char *program) {
  /* Program must be a null terminated string. */
  const char *p = program;
  int tok_index = 0;

  while (*p) {
    /* Consume whitespaces. */
    while (p && *p && whitespace(*p)) {
      ++p;
    }

    /* No more tokens? Return directly. */
    if (!p)
      return tok_index;

    switch (*p) {
    case '(': {
      tokens[tok_index].kind = TK_LParen;
      tokens[tok_index].literal = p;
      tokens[tok_index].tok_len = 1;
      ++tok_index;
      ++p;
      break;
    }
    case ')': {
      tokens[tok_index].kind = TK_RParen;
      tokens[tok_index].literal = p;
      tokens[tok_index].tok_len = 1;
      ++tok_index;
      ++p;
      break;
    }
    case '\'': {
      tokens[tok_index].kind = TK_Quote;
      tokens[tok_index].literal = p;
      tokens[tok_index].tok_len = 1;
      ++tok_index;
      ++p;
      break;
    }
    case '`': {
      tokens[tok_index].kind = TK_Backquote;
      tokens[tok_index].literal = p;
      tokens[tok_index].tok_len = 1;
      ++tok_index;
      ++p;
      break;
    }
    default:
      if (isdigit(*p)) {
        char *endp;
        strtod(p, &endp);

        tokens[tok_index].kind = TK_Number;
        tokens[tok_index].literal = p;
        tokens[tok_index].tok_len = (int)(endp - p);
        p = endp;
        ++tok_index;
        break;
      } else if (/* <initial> <subsequent>* */ (is_initial(*p)) ||
                 /* TODO: <vertical_line> <symbol_element>* <vertical_line> */
                 /* <peculiar_identifier> */ (is_explicit_sign(*p) ||
                                              *p == '.')) {
        /*
         * <identifier> -> <initial> <subsequent>*
         *              | <vertical_line> <symbol_element>* <vertical_line>
         *              | <peculiar_identifier>
         * <initial> -> <letter> | <special_initial>
         * <letter> -> a | b | c ... | z | A | B | C ... | Z
         * <special_initial> -> ! ∣ $ ∣ % ∣ & ∣ * ∣ / ∣ : ∣ < ∣ = ∣ > ∣ ? ∣ @ ∣
         *                      ^ ∣ _ ∣ ~
         * <subsequent> -> <initial> ∣ <digit> | <special_subsequent>
         * <special_subsequent> -> <explicit_sign> ∣ . ∣ @
         * <explicit_sign> -> + ∣ -
         * <peculiar_identifier> -> <explicit_sign>
         *                    ∣ <explicit_sign> <sign_subsequent> <subsequent>*
         *                    ∣ <explicit_sign> . <dot_subsequent> <subsequent>*
         *                    ∣ . <dot_subsequent> <subsequent>*
         * <sign_subsequent> -> <initial> ∣ <explicit_sign> ∣ @
         */
        if (is_initial(*p)) {
          /* Case: <initial> <subsequent>* */
          tokens[tok_index].kind = TK_Ident;
          tokens[tok_index].literal = p;
          ++p;
          while (*p && is_subsequent(*p)) {
            ++p;
          }

          tokens[tok_index].tok_len = (int)(p - tokens[tok_index].literal);
        } else if (is_explicit_sign(*p) || *p == '.') {
          /* Case: <peculiar_identifier> */
          tokens[tok_index].kind = TK_Ident;
          tokens[tok_index].literal = p;
          if (is_explicit_sign(*p)) {
            ++p;
            if (*p && *p != '.') {
              /* <sign_subsequent> <subsequent>* */
              if (*p && (is_sign_subsequent(*p))) {
                ++p;
              } else {
                /* Raise error. */
                fprintf(stdout, "Error\n");
              }
              /* Consume subsequent. */
              while (*p && is_subsequent(*p)) {
                ++p;
              }
            } else if (*p && *p == '.') {
              /* . <dot_subsequent> <subsequent>* */
              /* Consume '.' */
              ++p;
              /* Consume dot subsequent. */
              if (*p && is_dot_subsequent(*p)) {
                ++p;
              } else {
                /* Raise error. */
                fprintf(stdout, "Error\n");
              }
              /* Consume subsequent. */
              while (*p && is_subsequent(*p)) {
                ++p;
              }
            }
            tokens[tok_index].tok_len = (int)(p - tokens[tok_index].literal);
          } else {
            /* . <dot_subsequent> <subsequent>* */
            if (*p && *p == '.') {
              /* Consume '.' */
              ++p;
              /* Consume dot subsequent. */
              if (*p && is_dot_subsequent(*p)) {
                ++p;
              } else {
                /* Raise error. */
                fprintf(stdout, "Error\n");
              }
              /* Consume subsequent. */
              while (*p && is_subsequent(*p)) {
                ++p;
              }
            } else {
              /* Raise error. */
              fprintf(stdout, "Error\n");
            }
            tokens[tok_index].tok_len = (int)(p - tokens[tok_index].literal);
          }
        }
        ++tok_index;
        break;
      } else {
        /* Returns how many tokens we have consumed. */
        return tok_index;
      }
    }
  }

  /* Returns how many tokens we have consumed. */
  return tok_index;
}

/*
 * Strip whitespace from the start and end of str.  Return a pointer
 * into str.
 */
char *stripwhite(char *str) {
  char *s, *t;

  for (s = str; whitespace(*s); s++)
    ;

  if (*s == 0)
    return (s);

  t = s + strlen(s) - 1;
  while (t > s && whitespace(*t))
    t--;
  *++t = '\0';

  return s;
}

static int rocket_main(int argc, char **argv) {
  char *line = NULL;

  rl_initialize();
  for (;;) {
    char *stripped;
    line = readline(PROMPT_STYLE);
    if (!line) {
      /* Handle ctrl-d */
      break;
    }
    stripped = stripwhite(line);
    if (stripped[0]) {
      int consumed;
      fprintf(stdout, "%s\n", stripped);
      consumed = tokenize(stripped);
      for (int I = 0; I < consumed; ++I) {
        for (int l = 0; l < tokens[I].tok_len; ++l)
          fprintf(stdout, "%c", tokens[I].literal[l]);
        fprintf(stdout, "(%d)\n", tokens[I].kind);
      }
    }

    free(line);
    line = NULL;
  }

  exit(0);
}

int main(int argc, char **argv) {
  return rocket_main(argc, argv);
}
