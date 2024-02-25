#include "common.h"

#include "tokenizer.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

/*
 * <explicit_sign> -> + ∣ -
 */
#define is_explicit_sign(c) (c == '+' || c == '-')

/*
 * <special_initial> -> ! ∣ $ ∣ % ∣ & ∣ * ∣ / ∣ : ∣ < ∣ = ∣ > ∣ ? ∣ @ ∣
 *                      ^ ∣ _ ∣ ~
 */
#define is_special_initial(c)                                                  \
  (c == '!' || c == '$' || c == '%' || c == '&' || c == '*' || c == '/' ||     \
   c == ':' || c == '<' || c == '=' || c == '>' || c == '?' || c == '@' ||     \
   c == '^' || c == '_' || c == '~')

/*
 * <special_subsequent> -> <explicit_sign> ∣ . ∣ @
 * <explicit_sign> -> + ∣ -
 */
#define is_special_subsequent(c) (is_explicit_sign(c) || c == '.' || c == '@')

/* <initial> -> <letter> | <special_initial> */
#define is_initial(c) (isalpha(c) || is_special_initial(c))

/* <sign_subsequent> -> <initial> ∣ <explicit_sign> ∣ @ */
#define is_sign_subsequent(c) (is_initial(c) || is_explicit_sign(c) || c == '@')

/* <dot_subsequent> -> <sign_subsequent> ∣ . */
#define is_dot_subsequent(c) (is_sign_subsequent(c) || c == '.')

/* <subsequent> -> <initial> ∣ <digit>
 *              ∣ <special_subsequent>
 */
#define is_subsequent(c)                                                       \
  (is_initial(c) || isdigit(c) || is_special_subsequent(c))

Token *make_token(TokenKind kind, TokenLoc loc, const char *literal,
                  int tok_len) {
  Token *token = (Token *)malloc(sizeof(Token));
  token->kind = kind;
  token->loc = loc;
  token->literal = strndup(literal, tok_len);
  return token;
}

void free_token(Token *tok) {
  free((void *)tok->literal);
  free(tok);
}

TokenKind token_kind(Token *tok) {
  return tok->kind;
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
      column += (*p == ' ' ? 1 : 8);
    }

    /* No more tokens? Return directly. */
    if (!p) {
      goto out;
    }

    switch (*p) {
    case '(': {
      TokenLoc loc = {.file = filename, .line = line, .column = column};
      vector_append(tokens, PointerGetDatum(make_token(TK_LParen, loc, p, 1)));
      ++p;
      break;
    }
    case ')': {
      TokenLoc loc = {.file = filename, .line = line, .column = column};
      vector_append(tokens, PointerGetDatum(make_token(TK_RParen, loc, p, 1)));
      ++p;
      break;
    }
    case '.': {
      TokenLoc loc = {.file = filename, .line = line, .column = column};
      vector_append(tokens, PointerGetDatum(make_token(TK_Dot, loc, p, 1)));
      ++p;
      break;
    }
    case '\'': {
      TokenLoc loc = {.file = filename, .line = line, .column = column};
      vector_append(tokens, PointerGetDatum(make_token(TK_Quote, loc, p, 1)));
      ++p;
      break;
    }
    case '`': {
      TokenLoc loc = {.file = filename, .line = line, .column = column};
      vector_append(tokens,
                    PointerGetDatum(make_token(TK_Backquote, loc, p, 1)));
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
        TokenLoc loc = {.file = filename, .line = line, .column = column};
        vector_append(tokens,
                      PointerGetDatum(make_token(TK_Boolean, loc, p, tok_len)));
        p = endp;
      } else if ((tok_len == 6 && strncmp(p, "#false", 6) == 0) ||
                 (tok_len == 2 && strncmp(p, "#f", 2) == 0)) {
        TokenLoc loc = {.file = filename, .line = line, .column = column};
        vector_append(tokens,
                      PointerGetDatum(make_token(TK_Boolean, loc, p, tok_len)));
        p = endp;
      } else if (p[1] == '\\') {
        /* Characters start with `#\`. */
        TokenLoc loc = {.file = filename, .line = line, .column = column};
        vector_append(tokens,
                      PointerGetDatum(make_token(TK_Char, loc, p, tok_len)));
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
        TokenLoc loc = {.file = filename, .line = line, .column = column};
        vector_append(tokens, PointerGetDatum(make_token(TK_Number, loc, p,
                                                         (int)(endp - p))));

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
          TokenLoc loc = {.file = filename, .line = line, .column = column};
          vector_append(tokens,
                        PointerGetDatum(make_token(TK_Ident, loc, tok_literal,
                                                   (int)(p - tok_literal))));
        } else if (is_explicit_sign(*p) || *p == '.') {
          /* Case: <peculiar_identifier> */
          if (is_explicit_sign(*p)) {
            const char *tok_literal = p;
            ++p;
            if (*p && *p != '.') {
              /* <sign_subsequent> <subsequent>* */
              if (*p && (is_sign_subsequent(*p))) {
                ++p;
                /* Consume subsequent. */
                while (*p && is_subsequent(*p)) {
                  ++p;
                }
              } else if (*p && whitespace(*p)) {
                /* <peculiar_identifier> -> <explicit_sign> */
                /* Don't need to consume whitespaces. */
              } else if (*p == ')') {
                /* `(+)` */
              } else {
                /* Raise error. */
                goto fail;
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
            TokenLoc loc = {.file = filename, .line = line, .column = column};
            vector_append(tokens,
                          PointerGetDatum(make_token(TK_Ident, loc, tok_literal,
                                                     (int)(p - tok_literal))));
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
            TokenLoc loc = {.file = filename, .line = line, .column = column};
            vector_append(tokens,
                          PointerGetDatum(make_token(TK_Ident, loc, tok_literal,
                                                     (int)(p - tok_literal))));
          }
        }
        break;
      } else {
        /* Make sure we have consumed all tokens. */
        if (*p) {
          goto fail;
        } else {
          goto out;
        }
      }
    } /* default: */
    }
  }

out : {
  TokenLoc loc = {.file = filename, .line = line, .column = column};
  vector_append(tokens, PointerGetDatum(make_token(TK_EOF, loc, "", 0)));
  return tokens;
}

fail : {
  fprintf(stdout, "Error: %s:%d\n", __FILE__, __LINE__);
  exit(1);
}
}
