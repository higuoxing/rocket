#include <assert.h>
#include <stdio.h>

#include <readline/chardefs.h>

#include "tokenizer.h"
#include "vector.h"

static bool is_special_initial(char c);
static bool is_explicit_sign(char c);
static bool is_special_subsequent(char c);
static bool is_initial(char c);
static bool is_sign_subsequent(char c);
static bool is_dot_subsequent(char c);
static bool is_subsequent(char c);

Token *make_token(TokenKind kind, TokenLoc loc, const char *literal,
                  int tok_len) {
  Token *token = (Token *)malloc(sizeof(Token));
  token->kind = kind;
  token->loc = loc;
  token->literal = literal;
  token->literal_len = tok_len;
  return token;
}

TokenKind token_kind(Token *tok) {
  return tok->kind;
}

int token_len(Token *tok) {
  return tok->literal_len;
}

/*
 * Tokenize the given program and returns the number of tokens.
 */
Vector *tokenize(const char *program, const char *filename) {
  /* Program must be a null terminated string. */
  const char *p = program;
  Vector *tokens = make_vector();
  int line = 1;
  int column = 1;
  assert(program);

  while (*p) {
    /* Consume whitespaces and newlines. */
    while (*p && whitespace(*p)) {
      ++p;
    }

    /* No more tokens? Return directly. */
    if (!p) {
      goto out;
    }

    switch (*p) {
    case '(': {
      TokenLoc loc = {.file = filename, .line = 1, .column = 2};
      vector_append(tokens, make_token(TK_LParen, loc, p, 1));
      ++p;
      break;
    }
    case ')': {
      TokenLoc loc = {.file = filename, .line = 1, .column = 2};
      vector_append(tokens, make_token(TK_RParen, loc, p, 1));
      ++p;
      break;
    }
    case '.': {
      TokenLoc loc = {.file = filename, .line = 1, .column = 2};
      vector_append(tokens, make_token(TK_Dot, loc, p, 1));
      ++p;
      break;
    }
    case '\'': {
      TokenLoc loc = {.file = filename, .line = 1, .column = 2};
      vector_append(tokens, make_token(TK_Quote, loc, p, 1));
      ++p;
      break;
    }
    case '`': {
      TokenLoc loc = {.file = filename, .line = 1, .column = 2};
      vector_append(tokens, make_token(TK_Backquote, loc, p, 1));
      ++p;
      break;
    }
    case '#': {
      const char *endp = p;
      int tok_len;
      /* Looking ahead. */
      while (*endp && !whitespace(*endp) && *endp != '(' && *endp != ')') {
        ++endp;
      }
      tok_len = (int)(endp - p);
      /* FIXME: Can we simplify the logic? */
      if ((tok_len == 5 && strncmp(p, "#true", 5) == 0) ||
          (tok_len == 2 && strncmp(p, "#t", 2) == 0)) {
        TokenLoc loc = {.file = filename, .line = 1, .column = 2};
        vector_append(tokens, make_token(TK_Boolean, loc, p, tok_len));
        p = endp;
      } else if ((tok_len == 6 && strncmp(p, "#false", 6) == 0) ||
                 (tok_len == 2 && strncmp(p, "#f", 2) == 0)) {
        TokenLoc loc = {.file = filename, .line = 1, .column = 2};
        vector_append(tokens, make_token(TK_Boolean, loc, p, tok_len));
        p = endp;
      } else {
        /* Raise Error. */
        goto fail;
      }
      break;
    }
    default: {
      if (isdigit(*p)) {
        char *endp;
        strtod(p, &endp);
        TokenLoc loc = {.file = filename, .line = 1, .column = 2};
        vector_append(tokens, make_token(TK_Number, loc, p, (int)(endp - p)));

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
          TokenLoc loc = {.file = filename, .line = 1, .column = 2};
          vector_append(tokens, make_token(TK_Ident, loc, tok_literal,
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
                goto fail;
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
                goto fail;
              }
              /* Consume subsequent. */
              while (*p && is_subsequent(*p)) {
                ++p;
              }
            }
            TokenLoc loc = {.file = filename, .line = 1, .column = 2};
            vector_append(tokens, make_token(TK_Ident, loc, tok_literal,
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
                goto fail;
              }
              /* Consume subsequent. */
              while (*p && is_subsequent(*p)) {
                ++p;
              }
            } else {
              /* Raise error. */
              goto fail;
            }
            TokenLoc loc = {.file = filename, .line = 1, .column = 2};
            vector_append(tokens, make_token(TK_Ident, loc, tok_literal,
                                             (int)(p - tok_literal)));
          }
        }
        break;
      } else {
        goto out;
      }
    } /* default: */
    }
  }

out : {
  TokenLoc loc = {.file = filename, .line = 1, .column = 2};
  vector_append(tokens, make_token(TK_EOF, loc, "", 0));
  return tokens;
}

fail : {
  fprintf(stdout, "Error: %s:%d\n", __FILE__, __LINE__);
  exit(1);
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
