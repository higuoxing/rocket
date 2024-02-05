#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#include "vector.h"

typedef enum TokenKind {
  TK_Unknown,
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

extern Token *make_token(TokenKind kind, const char *literal, int tok_len);
extern Vector *tokenize(const char *program);
extern TokenKind token_kind(Token *tok);
extern int token_len(Token *tok);

#endif /* _TOKENIZER_H_ */
