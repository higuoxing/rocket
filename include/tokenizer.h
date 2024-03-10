#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "common.h"

#include "vector.h"

typedef enum TokenKind {
  /* '(' */
  TOKEN_LPAREN,
  /* ')' */
  TOKEN_RPAREN,
  TOKEN_DOT,
  /* Identifier */
  TOKEN_IDENT,
  /* '\'' */
  TOKEN_QUOTE,
  /* '`' */
  TOKEN_BACKQUOTE,
  TOKEN_BOOL,
  TOKEN_CHAR,
  TOKEN_NUMBER,

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

extern Token *make_token(TokenKind kind, TokenLoc loc, const char *literal,
                         int tok_len);
extern Vector *tokenize(const char *program, const char *filename);
extern TokenKind token_kind(const Token *tok);
extern const char *token_kind_str(const Token *tok);
extern void free_token(Token *);

#endif /* _TOKENIZER_H_ */
