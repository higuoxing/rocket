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

#define CURR_CHAR(tokenizer) (*(tokenizer->curr_pos))

/* <subsequent> -> <initial> ∣ <digit>
 *              ∣ <special_subsequent>
 */
#define is_subsequent(c)                                                       \
  (is_initial(c) || isdigit(c) || is_special_subsequent(c))

static Token *tokenizer_next(Tokenizer *tokenizer);

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

void initialize_tokenizer(Tokenizer *tokenizer, const char *filename,
                          char *program) {
  tokenizer->filename = filename;
  tokenizer->column = 0;
  tokenizer->line = 1;
  tokenizer->tokens = make_vector();
  tokenizer->program = program;
  tokenizer->curr_pos = tokenizer->program;
}

void destroy_tokenizer(Tokenizer *tokenizer) {
  if (tokenizer->tokens) {
    int num_tokens = vector_len(tokenizer->tokens);
    for (int i = 0; i < num_tokens; ++i) {
      Token *tok = (Token *)DatumGetPtr(vector_get(tokenizer->tokens, i));
      if (tok)
        free_token(tok);
    }
    free_vector(tokenizer->tokens);
  }
  /*
   * The filename and program are not controlled by tokenizer, so we cannot
   * destroy them.
   */
  tokenizer->program = NULL;
}

static Token *tokenizer_peek(Tokenizer *tokenizer) {
  int len = vector_len(tokenizer->tokens);
  if (len == 0)
    return NULL;
  return DatumGetPtr(vector_get(tokenizer->tokens, len - 1));
}

TokenIter tokenizer_iter(Tokenizer *tokenizer) {
  TokenIter iter = {.tokenizer = tokenizer, .index = -1};
  return iter;
}

Token *token_iter_peek(TokenIter *iter) {
  assert(iter->tokenizer);
  if (iter->index == -1) {
    ++iter->index;
    return tokenizer_next(iter->tokenizer);
  }
  return tokenizer_peek(iter->tokenizer);
}

Token *token_iter_next(TokenIter *iter) {
  assert(iter->tokenizer);
  token_iter_peek(iter);
  ++iter->index;
  return tokenizer_next(iter->tokenizer);
}

static void skip_whitespaces(Tokenizer *tokenizer) {
  while (CURR_CHAR(tokenizer) &&
         (whitespace(CURR_CHAR(tokenizer)) || CURR_CHAR(tokenizer) == '\n')) {
    if (CURR_CHAR(tokenizer) == '\n') {
      tokenizer->line += 1;
      tokenizer->column = 0;
    } else if (CURR_CHAR(tokenizer) == '\t') {
      tokenizer->column += 8;
    } else {
      tokenizer->column += 1;
    }
    ++tokenizer->curr_pos;
  }
}

/*
 * Move the cursor to the \newline character of the current line.
 */
static inline void skip_line(Tokenizer *tokenizer) {
  while (CURR_CHAR(tokenizer) && CURR_CHAR(tokenizer) != '\n')
    ++tokenizer->curr_pos;
}

static inline void consume_token(char **p) {
  /* Looking ahead. */
  while (**p && !whitespace(**p) && **p != '(' && **p != ')')
    ++(*p);
}

static Token *try_make_boolean_token(Tokenizer *tokenizer) {
  char *endp = tokenizer->curr_pos;
  int tok_len;
  TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
  consume_token(&endp);
  tok_len = (int)(endp - tokenizer->curr_pos);
  if ((tok_len == 5 && strncmp(tokenizer->curr_pos, "#true", 5) == 0) ||
      (tok_len == 2 && strncmp(tokenizer->curr_pos, "#t", 2) == 0)) {
    Token *tok = make_token(TOKEN_BOOL, loc, tokenizer->curr_pos, tok_len);
    vector_append(tokenizer->tokens, PointerGetDatum(tok));
    tokenizer->column += tok_len;
    tokenizer->curr_pos = endp;
    return tok;
  } else if ((tok_len == 6 && strncmp(tokenizer->curr_pos, "#false", 6) == 0) ||
             (tok_len == 2 && strncmp(tokenizer->curr_pos, "#f", 2) == 0)) {
    Token *tok = make_token(TOKEN_BOOL, loc, tokenizer->curr_pos, tok_len);
    vector_append(tokenizer->tokens, PointerGetDatum(tok));
    tokenizer->column += tok_len;
    tokenizer->curr_pos = endp;
    return tok;
  }
  return NULL;
}

static Token *try_make_char_token(Tokenizer *tokenizer) {
  char *endp = tokenizer->curr_pos;
  TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
  int tok_len;
  Token *tok;

  if (tokenizer->curr_pos[1] != '\\')
    return NULL;

  /* Looking ahead. */
  consume_token(&endp);
  tok_len = (int)(endp - tokenizer->curr_pos);
  /* Characters start with `#\`. */
  tok = make_token(TOKEN_CHAR, loc, tokenizer->curr_pos, tok_len);
  vector_append(tokenizer->tokens, PointerGetDatum(tok));
  tokenizer->column += tok_len;
  tokenizer->curr_pos = endp;
  return tok;
}

static Token *try_make_number_token(Tokenizer *tokenizer) {
  char *endp;
  Token *tok;
  TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};

  /* TODO: Better tokenizer for numbers. */
  if (!(is_explicit_sign(CURR_CHAR(tokenizer)) &&
        isdigit(*(tokenizer->curr_pos + 1))) &&
      !isdigit(CURR_CHAR(tokenizer)))
    return NULL;

  strtod(tokenizer->curr_pos, &endp);
  tok = make_token(TOKEN_NUMBER, loc, tokenizer->curr_pos,
                   (int)(endp - tokenizer->curr_pos));
  vector_append(tokenizer->tokens, PointerGetDatum(tok));
  tokenizer->column += (int)(endp - tokenizer->curr_pos);
  tokenizer->curr_pos = endp;
  return tok;
}

static Token *try_make_identifier1_token(Tokenizer *tokenizer) {
  /*
   * <identifier> -> <initial> <subsequent>*
   */
  Token *tok;
  TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};
  const char *tok_literal = tokenizer->curr_pos;

  if (!is_initial(CURR_CHAR(tokenizer)))
    return NULL;

  /* Consume <initial> */
  ++tokenizer->curr_pos;
  /* Consume <subsequent>* */
  while (is_subsequent(CURR_CHAR(tokenizer)))
    ++tokenizer->curr_pos;
  tok = make_token(TOKEN_IDENT, loc, tok_literal,
                   (int)(tokenizer->curr_pos - tok_literal));
  vector_append(tokenizer->tokens, PointerGetDatum(tok));
  tokenizer->column += (int)(tokenizer->curr_pos - tok_literal);
  return tok;
}

static Token *try_make_peculiar_identifier_token(Tokenizer *tokenizer) {
  /*
   * <peculiar_identifier> -> <explicit_sign>
   *                       |  <explicit_sign> <sign_subsequent> <subsequent>*
   *                       |  <explicit_sign> . <dot_subsequent> <subsequent>*
   *                       |  .                 <dot_subsequent> <subsequent>*
   */
  Token *tok;
  const char *tok_literal = tokenizer->curr_pos;
  char *orig_pos = tokenizer->curr_pos;
  TokenLoc loc = {.line = tokenizer->line, .column = tokenizer->column};

  if (!is_explicit_sign(CURR_CHAR(tokenizer)) && CURR_CHAR(tokenizer) != '.')
    return NULL;

  if (is_explicit_sign(CURR_CHAR(tokenizer))) {
    /* Consume <explicit_sign> */
    ++tokenizer->curr_pos;

    if (is_sign_subsequent(CURR_CHAR(tokenizer))) {
      /* Consume <sign_subsequent> */
      ++tokenizer->curr_pos;

      while (is_subsequent(CURR_CHAR(tokenizer))) {
        /* Consume <subsequent>* */
        ++tokenizer->curr_pos;
      }
    }

    tok = make_token(TOKEN_IDENT, loc, tok_literal,
                     (int)(tokenizer->curr_pos - tok_literal));
    vector_append(tokenizer->tokens, PointerGetDatum(tok));
    tokenizer->column += (int)(tokenizer->curr_pos - tok_literal);
    return tok;
  }

  if (CURR_CHAR(tokenizer) == '.') {
    /* Consume '.' */
    ++tokenizer->curr_pos;
  }

  /* Consume <dot_subsequent> */
  if (is_dot_subsequent(CURR_CHAR(tokenizer))) {
    ++tokenizer->curr_pos;
  } else {
    tokenizer->curr_pos = orig_pos;
    return NULL;
  }

  /* Consume <subsequent>* */
  while (is_subsequent(CURR_CHAR(tokenizer)))
    ++tokenizer->curr_pos;

  tok = make_token(TOKEN_IDENT, loc, tok_literal,
                   (int)(tokenizer->curr_pos - tok_literal));
  vector_append(tokenizer->tokens, PointerGetDatum(tok));
  tokenizer->column += (int)(tokenizer->curr_pos - tok_literal);
  return tok;
}

static Token *try_make_identifier2_token(Tokenizer *tokenizer) {
  /* TODO: <vertical_line> <symbol_element>* <vertical_line> */
  return NULL;
}

static inline Token *try_make_identifier3_token(Tokenizer *tokenizer) {
  return try_make_peculiar_identifier_token(tokenizer);
}

static Token *try_make_identifier_token(Tokenizer *tokenizer) {
  /*
   * <identifier> -> <initial> <subsequent>*
   *              | <vertical_line> <symbol_element>* <vertical_line>
   *              | <peculiar_identifier>
   */
  Token *tok;
  if ((tok = try_make_identifier1_token(tokenizer)))
    return tok;

  if ((tok = try_make_identifier2_token(tokenizer)))
    return tok;

  if ((tok = try_make_identifier3_token(tokenizer)))
    return tok;

  return NULL;
}

static Token *tokenizer_next(Tokenizer *tokenizer) {
  /* Program must be a null terminated string. */
  assert(tokenizer->program);

  if (!CURR_CHAR(tokenizer))
    goto out;

  while (CURR_CHAR(tokenizer)) {
    /* Consume whitespaces and newlines. */
    skip_whitespaces(tokenizer);

    /* No more tokens? Return directly. */
    if (!CURR_CHAR(tokenizer))
      goto out;

    switch (CURR_CHAR(tokenizer)) {
    case ';': {
      /* Skip comments. */
      skip_line(tokenizer);
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
      Token *tok = NULL;
      if ((tok = try_make_boolean_token(tokenizer)) != NULL) {
        return tok;
      } else if ((tok = try_make_char_token(tokenizer)) != NULL) {
        return tok;
      }
      /* Raise Error. */
      fprintf(stdout, "Error: %s:%d\n", __FILE__, __LINE__);
      goto fail;
    }
    default: {
      Token *tok = NULL;
      if ((tok = try_make_number_token(tokenizer))) {
        return tok;
      } else if ((tok = try_make_identifier_token(tokenizer))) {
        return tok;
      } else {
        /* Make sure we have consumed all tokens. */
        if (CURR_CHAR(tokenizer)) {
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
