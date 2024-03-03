#include "common.h"

#include "tokenizer.h"
#include "vector.h"

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
  if (tok->literal) {
    free((void *)tok->literal);
  }
  free(tok);
}

TokenKind token_kind(const Token *tok) {
  return tok->kind;
}

const char *token_kind_str(const Token *tok) {
  switch (tok->kind) {
  case TOKEN_LPAREN: {
    return "LPAREN";
  }
  case TOKEN_RPAREN: {
    return "RPAREN";
  }
  case TOKEN_DOT: {
    return "DOT";
  }
  case TOKEN_IDENT: {
    return "IDENTIFIER";
  }
  case TOKEN_QUOTE: {
    return "QUOTE";
  }
  case TOKEN_BACKQUOTE: {
    return "BACKQUOTE";
  }
  case TOKEN_BOOL: {
    return "BOOL";
  }
  case TOKEN_CHAR: {
    return "CHAR";
  }
  case TOKEN_NUMBER: {
    return "NUMBER";
  }
  case TOKEN_EOF: {
    return "EOF";
  }
  default:
    abort();
  }
}

static char *read_file(const char *filename) {
  char *script = malloc(BLKSZ);
  FILE *script_file = NULL;
  uint32_t total_read_size = 0;
  uint32_t curr_read_size = 0;
  uint32_t curr_buffer_size = BLKSZ;

  if (!script) {
    fprintf(stderr, "OOM! %m");
    exit(1);
  }

  script_file = fopen(filename, "r");
  if (!script_file) {
    fprintf(stderr, "cannot open script file: \"%s\" %m\n", filename);
    exit(1);
  }

  while ((curr_read_size = fread(&script[total_read_size], sizeof(char), BLKSZ,
                                 script_file)) > 0) {
    total_read_size += curr_read_size;

    if (total_read_size == curr_buffer_size) {
      curr_buffer_size *= 2;
      script = realloc(script, curr_buffer_size);
    }
  }

  script[total_read_size] = '\0';

  fclose(script_file);
  return script;
}

void reset_tokenizer(Tokenizer *tokenizer, const char *filename) {
  destroy_tokenizer(tokenizer);
  tokenizer->filename = filename;
  tokenizer->column = 0;
  tokenizer->line = 1;
  tokenizer->tokens = make_vector();
  tokenizer->program = read_file(filename);
  tokenizer->curr_pos = tokenizer->program;
}

void destroy_tokenizer(Tokenizer *tokenizer) {
  if (tokenizer->tokens) {
    int num_tokens = vector_len(tokenizer->tokens);
    for (int i = 0; i < num_tokens; ++i) {
      Token *tok = (Token *)DatumGetPtr(vector_get(tokenizer->tokens, i));
      free_token(tok);
    }
    free_vector(tokenizer->tokens);
  }
  if (tokenizer->program)
    free(tokenizer->program);
}

Token *tokenizer_peek(Tokenizer *tokenizer) {
  int len = vector_len(tokenizer->tokens);
  if (len == 0)
    return NULL;
  return DatumGetPtr(vector_get(tokenizer->tokens, len - 1));
}

Token *tokenizer_next(Tokenizer *tokenizer) {
  /* Program must be a null terminated string. */
  assert(tokenizer->program);

  if (*tokenizer->curr_pos == '\0')
    return NULL;

  while (*tokenizer->curr_pos) {
    /* Consume whitespaces and newlines. */
    while (*tokenizer->curr_pos &&
           (whitespace(*tokenizer->curr_pos) || *tokenizer->curr_pos == '\n')) {
      if (*tokenizer->curr_pos == '\n') {
        tokenizer->line += 1;
        tokenizer->column = 0;
      } else if (*tokenizer->curr_pos == '\t') {
        tokenizer->column += 8;
      } else {
        tokenizer->column += 1;
      }
      ++tokenizer->curr_pos;
    }

    /* No more tokens? Return directly. */
    if (*tokenizer->curr_pos == '\0') {
      goto out;
    }

    switch (*tokenizer->curr_pos) {
    case ';': {
      /* Skip comments. */
      while (*tokenizer->curr_pos && *tokenizer->curr_pos != '\n') {
        ++tokenizer->curr_pos;
      }
      break;
    }
    case '(': {
      TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
      Token *tok = make_token(TOKEN_LPAREN, loc, tokenizer->curr_pos, 1);
      vector_append(tokenizer->tokens, PointerGetDatum(tok));
      tokenizer->column += 1;
      ++tokenizer->curr_pos;
      return tok;
    }
    case ')': {
      TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
      Token *tok = make_token(TOKEN_RPAREN, loc, tokenizer->curr_pos, 1);
      vector_append(tokenizer->tokens, PointerGetDatum(tok));
      tokenizer->column += 1;
      ++tokenizer->curr_pos;
      return tok;
    }
    case '.': {
      TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
      Token *tok = make_token(TOKEN_DOT, loc, tokenizer->curr_pos, 1);
      vector_append(tokenizer->tokens, PointerGetDatum(tok));
      tokenizer->column += 1;
      ++tokenizer->curr_pos;
      return tok;
    }
    case '\'': {
      TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
      Token *tok = make_token(TOKEN_QUOTE, loc, tokenizer->curr_pos, 1);
      vector_append(tokenizer->tokens, PointerGetDatum(tok));
      tokenizer->column += 1;
      ++tokenizer->curr_pos;
      return tok;
    }
    case '`': {
      TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
      Token *tok = make_token(TOKEN_BACKQUOTE, loc, tokenizer->curr_pos, 1);
      vector_append(tokenizer->tokens, PointerGetDatum(tok));
      tokenizer->column += 1;
      ++tokenizer->curr_pos;
      return tok;
    }
    case '#': {
      char *endp = tokenizer->curr_pos;
      int tok_len;
      /* Looking ahead. */
      while (*endp && !whitespace(*endp) && *endp != '(' && *endp != ')') {
        ++endp;
      }
      tok_len = (int)(endp - tokenizer->curr_pos);
      /* FIXME: Can we simplify the logic? */
      if ((tok_len == 5 && strncmp(tokenizer->curr_pos, "#true", 5) == 0) ||
          (tok_len == 2 && strncmp(tokenizer->curr_pos, "#t", 2) == 0)) {
        TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
        Token *tok = make_token(TOKEN_BOOL, loc, tokenizer->curr_pos, tok_len);
        vector_append(tokenizer->tokens, PointerGetDatum(tok));
        tokenizer->column += tok_len;
        tokenizer->curr_pos = endp;
        return tok;
      } else if ((tok_len == 6 &&
                  strncmp(tokenizer->curr_pos, "#false", 6) == 0) ||
                 (tok_len == 2 && strncmp(tokenizer->curr_pos, "#f", 2) == 0)) {
        TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
        Token *tok = make_token(TOKEN_BOOL, loc, tokenizer->curr_pos, tok_len);
        vector_append(tokenizer->tokens, PointerGetDatum(tok));
        tokenizer->column += tok_len;
        tokenizer->curr_pos = endp;
        return tok;
      } else if (tokenizer->curr_pos[1] == '\\') {
        /* Characters start with `#\`. */
        TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
        Token *tok = make_token(TOKEN_CHAR, loc, tokenizer->curr_pos, tok_len);
        vector_append(tokenizer->tokens, PointerGetDatum(tok));
        tokenizer->column += tok_len;
        tokenizer->curr_pos = endp;
        return tok;
      } else {
        /* Raise Error. */
        fprintf(stdout, "Error: %s:%d\n", __FILE__, __LINE__);
        goto fail;
      }
      break;
    }
    default: {
      if (isdigit(*tokenizer->curr_pos)) {
        char *endp;
        strtod(tokenizer->curr_pos, &endp);
        TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
        Token *tok = make_token(TOKEN_NUMBER, loc, tokenizer->curr_pos,
                                (int)(endp - tokenizer->curr_pos));
        vector_append(tokenizer->tokens, PointerGetDatum(tok));
        tokenizer->column += (int)(endp - tokenizer->curr_pos);
        tokenizer->curr_pos = endp;
        return tok;
      } else if (/* <initial> <subsequent>* */ (
                     is_initial(*tokenizer->curr_pos)) ||
                 /* TODO: <vertical_line> <symbol_element>* <vertical_line> */
                 /* <peculiar_identifier> */ (
                     is_explicit_sign(*tokenizer->curr_pos) ||
                     *tokenizer->curr_pos == '.')) {
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
        if (is_initial(*tokenizer->curr_pos)) {
          /* Case: <initial> <subsequent>* */
          Token *tok;
          const char *tok_literal = tokenizer->curr_pos;
          /* Consume initial. */
          ++tokenizer->curr_pos;
          /* Consume subsequent. */
          while (*tokenizer->curr_pos && is_subsequent(*tokenizer->curr_pos)) {
            ++tokenizer->curr_pos;
          }
          TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
          tok = make_token(TOKEN_IDENT, loc, tok_literal,
                           (int)(tokenizer->curr_pos - tok_literal));
          vector_append(tokenizer->tokens, PointerGetDatum(tok));
          tokenizer->column += (int)(tokenizer->curr_pos - tok_literal);
          return tok;
        } else if (is_explicit_sign(*tokenizer->curr_pos) ||
                   *tokenizer->curr_pos == '.') {
          /* Case: <peculiar_identifier> */
          if (is_explicit_sign(*tokenizer->curr_pos)) {
            const char *tok_literal = tokenizer->curr_pos;
            ++tokenizer->curr_pos;
            if (*tokenizer->curr_pos && *tokenizer->curr_pos != '.') {
              /* <sign_subsequent> <subsequent>* */
              if (*tokenizer->curr_pos &&
                  (is_sign_subsequent(*tokenizer->curr_pos))) {
                ++tokenizer->curr_pos;
                /* Consume subsequent. */
                while (*tokenizer->curr_pos &&
                       is_subsequent(*tokenizer->curr_pos)) {
                  ++tokenizer->curr_pos;
                }
              } else if (*tokenizer->curr_pos &&
                         whitespace(*tokenizer->curr_pos)) {
                /* <peculiar_identifier> -> <explicit_sign> */
                /* Don't need to consume whitespaces. */
              } else if (*tokenizer->curr_pos == ')') {
                /* `(+)` */
              } else if (isdigit(*tokenizer->curr_pos)) {
                /* parse digits */
                char *endp;
                strtod(tokenizer->curr_pos, &endp);
                TokenLoc loc = {.line = tokenizer->line,
                                .column = tokenizer->column};
                Token *tok = make_token(TOKEN_NUMBER, loc, tok_literal,
                                        (int)(endp - tok_literal));
                vector_append(tokenizer->tokens, PointerGetDatum(tok));
                tokenizer->column += (int)(endp - tok_literal);
                tokenizer->curr_pos = endp;
                return tok;
              } else {
                /* Raise error. */
                fprintf(stdout, "Error: %s:%d\n", __FILE__, __LINE__);
                abort();
                goto fail;
              }

            } else if (*tokenizer->curr_pos && *tokenizer->curr_pos == '.') {
              /* . <dot_subsequent> <subsequent>* */
              /* Consume '.' */
              ++tokenizer->curr_pos;
              /* Consume dot subsequent. */
              if (*tokenizer->curr_pos &&
                  is_dot_subsequent(*tokenizer->curr_pos)) {
                ++tokenizer->curr_pos;
              } else {
                /* Raise error. */
                fprintf(stdout, "Error: %s:%d\n", __FILE__, __LINE__);
                goto fail;
              }
              /* Consume subsequent. */
              while (*tokenizer->curr_pos &&
                     is_subsequent(*tokenizer->curr_pos)) {
                ++tokenizer->curr_pos;
              }
            }
            TokenLoc loc = {.line = tokenizer->line,
                            .column = tokenizer->column};
            Token *tok = make_token(TOKEN_IDENT, loc, tok_literal,
                                    (int)(tokenizer->curr_pos - tok_literal));
            vector_append(tokenizer->tokens, PointerGetDatum(tok));
            tokenizer->column += (int)(tokenizer->curr_pos - tok_literal);
            return tok;
          } else {
            /* . <dot_subsequent> <subsequent>* */
            const char *tok_literal = tokenizer->curr_pos;
            if (*tokenizer->curr_pos == '.') {

              /* Consume '.' */
              ++tokenizer->curr_pos;
              /* Consume dot subsequent. */
              if (*tokenizer->curr_pos &&
                  is_dot_subsequent(*tokenizer->curr_pos)) {
                ++tokenizer->curr_pos;
              } else {
                /* Raise error. */
                fprintf(stdout, "Error: %s:%d\n", __FILE__, __LINE__);
                goto fail;
              }
              /* Consume subsequent. */
              while (is_subsequent(*tokenizer->curr_pos)) {
                ++tokenizer->curr_pos;
              }
            } else {
              /* Raise error. */
              fprintf(stdout, "Error: %s:%d\n", __FILE__, __LINE__);
              goto fail;
            }
            TokenLoc loc = {.line = tokenizer->line,
                            .column = tokenizer->column};
            Token *tok = make_token(TOKEN_IDENT, loc, tok_literal,
                                    (int)(tokenizer->curr_pos - tok_literal));
            vector_append(tokenizer->tokens, PointerGetDatum(tok));
            tokenizer->column += (int)(tokenizer->curr_pos - tok_literal);
            return tok;
          }
        }
        break;
      } else {
        /* Make sure we have consumed all tokens. */
        if (*tokenizer->curr_pos) {
          fprintf(stdout, "Error: %s:%d\n", __FILE__, __LINE__);
          goto fail;
        } else {
          goto out;
        }
      }
    } /* default: */
    }
  }

out : {
  TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
  Token *tok = make_token(TOKEN_EOF, loc, "", 0);
  vector_append(tokenizer->tokens, PointerGetDatum(tok));
  return tok;
}

fail : { exit(1); }
}
