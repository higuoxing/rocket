#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include "common.h"

#include "vector.h"

typedef enum TokenKind {
  /* '(' */
  TK_LParen,
  /* ')' */
  TK_RParen,
  TK_Dot,
  /* Identifier */
  TK_Ident,
  /* '\'' */
  TK_Quote,
  /* '`' */
  TK_Backquote,
  TK_Boolean,
  TK_Number,

  /* EOF */
  TK_EOF,
} TokenKind;

typedef struct TokenLoc {
  const char *file;
  int line;
  int column;
} TokenLoc;

typedef struct Token {
  TokenKind kind;

  /* Location of the token. */
  TokenLoc loc;

  /* Literal representation. */
  const char *literal;
  /* Length of the literal string. */
  int literal_len;
} Token;

extern Token *make_token(TokenKind kind, TokenLoc loc, const char *literal,
                         int tok_len);
extern Vector *tokenize(const char *program, const char *filename);
extern TokenKind token_kind(Token *tok);
extern int token_len(Token *tok);

#endif /* _TOKENIZER_H_ */
