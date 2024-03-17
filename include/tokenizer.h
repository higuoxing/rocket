#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "common.h"

#include "vector.h"

typedef enum TokenKind {
  /* '(' */
  TOKEN_LPAREN = 0,
  /* ')' */
  TOKEN_RPAREN = 1,
  TOKEN_DOT = 2,
  /* Identifier */
  TOKEN_IDENT = 3,
  /* '\'' */
  TOKEN_QUOTE = 4,
  /* '`' */
  TOKEN_BACKQUOTE = 5,
  TOKEN_BOOL = 6,
  TOKEN_CHAR = 7,
  TOKEN_NUMBER = 8,

  /* EOF */
  TOKEN_EOF,
} TokenKind;

typedef struct TokenLoc {
  int line;
  int column;
} TokenLoc;

typedef struct Token {
  TokenKind kind;

  /* Location of the token. */
  TokenLoc loc;

  /* Literal representation. (null-terminated string) */
  const char *literal;
} Token;

typedef struct Tokenizer {
  const char *filename;
  char *program;
  char *curr_pos;
  int line;
  int column;
  Vector *tokens;
} Tokenizer;

typedef struct TokenIter {
  int index;
  Tokenizer *tokenizer;
} TokenIter;

extern TokenIter tokenizer_iter(Tokenizer *tokenizer);
extern Token *token_iter_peek(TokenIter *iter);
extern Token *token_iter_next(TokenIter *iter);

extern void initialize_tokenizer(Tokenizer *tokenizer, const char *filename,
                                 char *program);
extern void destroy_tokenizer(Tokenizer *tokenizer);

extern Token *make_token(TokenKind kind, TokenLoc loc, const char *literal,
                         int tok_len);
extern Vector *tokenize(const char *program, const char *filename);
extern TokenKind token_kind(const Token *tok);
extern const char *token_kind_str(const Token *tok);
extern void free_token(Token *);

#endif /* _TOKENIZER_H_ */
