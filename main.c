#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <readline/chardefs.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "vector.h"

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
  TK_Boolean,
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

Token *make_token(TokenKind kind, const char *literal, int tok_len) {
  Token *token = (Token *)malloc(sizeof(Token));
  token->kind = kind;
  token->literal = literal;
  token->tok_len = tok_len;
  return token;
}

static const char *token_kind_to_string(TokenKind kind) {
  switch (kind) {
  case TK_Unknown:
    return "Unknown";
  case TK_LParen:
    return "LParen";
  case TK_RParen:
    return "RParen";
  case TK_Ident:
    return "Ident";
  case TK_Quote:
    return "Singlequote";
  case TK_Backquote:
    return "Backquote";
  case TK_Boolean:
    return "Boolean";
  case TK_Number:
    return "Number";
  default:
    return "Unknown Token";
  }
}

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
static Vector *tokenize(const char *program) {
  /* Program must be a null terminated string. */
  const char *p = program;
  Vector *tokens = make_vector();
  assert(program);

  while (*p) {
    /* Consume whitespaces. */
    while (*p && whitespace(*p)) {
      ++p;
    }

    /* No more tokens? Return directly. */
    if (!p)
      return tokens;

    switch (*p) {
    case '(': {
      vector_append(tokens, make_token(TK_LParen, p, 1));
      ++p;
      break;
    }
    case ')': {
      vector_append(tokens, make_token(TK_RParen, p, 1));
      ++p;
      break;
    }
    case '\'': {
      vector_append(tokens, make_token(TK_Quote, p, 1));
      ++p;
      break;
    }
    case '`': {
      vector_append(tokens, make_token(TK_Backquote, p, 1));
      ++p;
      break;
    }
    case '#': {
      const char *endp = p;
      int tok_len;
      /* Looking ahead. */
      while (*endp && !whitespace(*endp)) {
        ++endp;
      }
      tok_len = (int)(endp - p);
      /* FIXME: Can we simplify??? */
      if ((tok_len == 5 && strncmp(p, "#true", 5) == 0) ||
          (tok_len == 2 && strncmp(p, "#t", 2) == 0)) {
        vector_append(tokens, make_token(TK_Boolean, p, tok_len));
        p = endp;
      } else if ((tok_len == 6 && strncmp(p, "#false", 6) == 0) ||
                 (tok_len == 2 && strncmp(p, "#f", 2) == 0)) {
        vector_append(tokens, make_token(TK_Boolean, p, tok_len));
        p = endp;
      } else {
        /* Raise Error. */
        fprintf(stdout, "Error\n");
        exit(1);
      }
      break;
    }
    default: {
      if (isdigit(*p)) {
        char *endp;
        strtod(p, &endp);

        vector_append(tokens, make_token(TK_Number, p, (int)(endp - p)));

        p = endp;
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
          const char *tok_literal = p;
          /* Consume initial. */
          ++p;
          /* Consume subsequent. */
          while (*p && is_subsequent(*p)) {
            ++p;
          }

          vector_append(tokens, make_token(TK_Ident, tok_literal,
                                           (int)(p - tok_literal)));
        } else if (is_explicit_sign(*p) || *p == '.') {
          /* Case: <peculiar_identifier> */
          if (is_explicit_sign(*p)) {
            const char *tok_literal = p;
            ++p;
            if (*p && *p != '.') {
              /* <sign_subsequent> <subsequent>* */
              if (*p && (is_sign_subsequent(*p))) {
                ++p;
              } else {
                /* Raise error. */
                fprintf(stdout, "Error\n");
                exit(1);
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
            vector_append(tokens, make_token(TK_Ident, tok_literal,
                                             (int)(p - tok_literal)));
          } else {
            /* . <dot_subsequent> <subsequent>* */
            const char *tok_literal = p;
            if (*p && *p == '.') {

              /* Consume '.' */
              ++p;
              /* Consume dot subsequent. */
              if (*p && is_dot_subsequent(*p)) {
                ++p;
              } else {
                /* Raise error. */
                fprintf(stdout, "Error\n");
                exit(1);
              }
              /* Consume subsequent. */
              while (*p && is_subsequent(*p)) {
                ++p;
              }
            } else {
              /* Raise error. */
              fprintf(stdout, "Error\n");
              exit(1);
            }
            vector_append(tokens, make_token(TK_Ident, tok_literal,
                                             (int)(p - tok_literal)));
          }
        }
        break;
      } else {
        /* Returns how many tokens we have consumed. */
        return tokens;
      }
    } /* default: */
    }
  }

  /* Returns how many tokens we have consumed. */
  return tokens;
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
      Vector *tokens = tokenize(stripped);
      int num_tokens = vector_len(tokens);
      for (int I = 0; I < num_tokens; ++I) {
        Token *tok = vector_get(tokens, I);
        for (int l = 0; l < tok->tok_len; ++l)
          fprintf(stdout, "%c", tok->literal[l]);
        fprintf(stdout, " -- %s\n", token_kind_to_string(tok->kind));
      }

      /* Clean up. */
      for (int i = 0; i < num_tokens; ++i) {
        free(vector_get(tokens, i));
      }
      free_vector(tokens);
    }

    free(line);
    line = NULL;
  }

  exit(0);
}

int main(int argc, char **argv) {
  return rocket_main(argc, argv);
}
