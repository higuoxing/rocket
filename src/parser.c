#include "common.h"

#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include "vector.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static AstNode *parse_boolean(Token *tok);
static AstNode *parse_char(Token *tok);
static AstNode *parse_number(Token *tok);
static AstNode *parse_ident(Token *tok);
static AstNode *parse_quote(TokenIter *iter);
static AstNode *parse_expression(TokenIter *iter);

#define CURR_TOKEN(iter) (token_iter_peek(iter))
#define NEXT_TOKEN(iter) (token_iter_next(iter))

Vector *parse_program(Tokenizer *tokenizer) {
  int cursor = 0;
  Token *tok;
  TokenIter iter = tokenizer_iter(tokenizer);
  Vector *program = make_vector();

  while (CURR_TOKEN(&iter)->kind != TOKEN_EOF) {
    AstNode *expr = parse_expression(&iter);
    vector_append(program, PointerGetDatum(expr));
  }

  return program;
}

static AstNode *parse_expression(TokenIter *iter) {
  switch (CURR_TOKEN(iter)->kind) {
  case TOKEN_BOOL: {
    AstNode *bool_ast = parse_boolean(CURR_TOKEN(iter));
    NEXT_TOKEN(iter);
    return bool_ast;
  }
  case TOKEN_CHAR: {
    AstNode *char_ast = parse_char(CURR_TOKEN(iter));
    NEXT_TOKEN(iter);
    return char_ast;
  }
  case TOKEN_NUMBER: {
    AstNode *number_ast = parse_number(CURR_TOKEN(iter));
    NEXT_TOKEN(iter);
    return number_ast;
  }
  case TOKEN_IDENT: {
    AstNode *ident_ast = parse_ident(CURR_TOKEN(iter));
    NEXT_TOKEN(iter);
    return ident_ast;
  }
  case TOKEN_QUOTE: {
    NEXT_TOKEN(iter);
    AstNode *quote_ast = parse_quote(iter);
    return quote_ast;
  }
  case TOKEN_LPAREN: {
    AstNode *callable = NULL;
    int arg_index = 0;
    Vector *args = NULL;

    /* Consume '(' */
    NEXT_TOKEN(iter);

    if (CURR_TOKEN(iter)->kind == TOKEN_RPAREN) {
      /* Consume ')' */
      NEXT_TOKEN(iter);
      /* This is `()` (or nil), we return it directly. */
      return NULL;
    }

    args = make_vector();

    /* Parse until ')' */
    while (CURR_TOKEN(iter)->kind != TOKEN_RPAREN) {
      AstNode *inner_ast = NULL;
      if (CURR_TOKEN(iter)->kind == TOKEN_EOF) {
        /* Need more tokens. */
        fprintf(stderr, "%s: Expected more tokens.", __FUNCTION__);
        exit(1);
      }

      inner_ast = parse_expression(iter);

      if (arg_index == 0) {
        callable = inner_ast;
      } else {
        vector_append(args, PointerGetDatum(inner_ast));
      }

      ++arg_index;
    }

    /* Consume ')' */
    assert(CURR_TOKEN(iter)->kind == TOKEN_RPAREN);
    NEXT_TOKEN(iter);

    return make_ast_proc_call(callable, args);
  }
  case TOKEN_EOF: {
    /* Need more tokens. */
    fprintf(stderr, "%s: Expected more tokens.\n", __FUNCTION__);
    exit(1);
  }
  default: {
    fprintf(stderr, "%s: Unexpected token kind (%d)\n", __FUNCTION__,
            CURR_TOKEN(iter)->kind);
    exit(1);
  }
  }
  return NULL;
}

static AstNode *parse_boolean(Token *tok) {
  return make_ast_bool((strcmp(tok->literal, "#t") == 0 ? true : false));
}

static AstNode *parse_char(Token *tok) {
  char c;
  assert(tok->kind == TOKEN_CHAR);
  /*
   * <character> -> #\ <any character>
   *              | #\ <character name>
   *              | #\x<hex scalar value>
   * <character name> -> alarm | backspace | delete
   *                  | escape | newline | null | return | space | tab
   */
  if (strlen(tok->literal) == 3) {
    /* #\<any_char> */
    c = tok->literal[2];
  } else {
    if (strlen(tok->literal) >= 4 && tok->literal[2] == 'x') {
      /* Handle #\x<hex_scalar> */
      c = (char)strtol(tok->literal, NULL, 16);
    } else {
      /* Handle #\<character name> */
      /* TODO: Add more characters. */
      if (strcmp(tok->literal, "#\\alarm") == 0) {
        c = 7;
      } else if (strcmp(tok->literal, "#\\backspace") == 0) {
        c = 8;
      } else if (strcmp(tok->literal, "#\\delete") == 0) {
        c = 127;
      } else if (strcmp(tok->literal, "#\\newline") == 0) {
        c = 10;
      } else if (strcmp(tok->literal, "#\\return") == 0) {
        c = 13;
      } else if (strcmp(tok->literal, "#\\space") == 0) {
        c = 32;
      } else if (strcmp(tok->literal, "#\\tab") == 0) {
        c = 9;
      } else {
        /* Otherwise, Raise error. */
        fprintf(stderr, "%s: Error\n", __FUNCTION__);
        exit(1);
      }
    }
  }

  return make_ast_char(c);
}

static AstNode *parse_number(Token *tok) {
  return make_ast_number(strtod(tok->literal, NULL));
}

static AstNode *parse_ident(Token *tok) {
  return make_ast_ident(tok->literal);
}

static AstNode *parse_quote(TokenIter *iter) {
  AstQuote *quote_ast = malloc(sizeof(AstQuote));
  quote_ast->base.kind = AST_QUOTE;
  quote_ast->inner = parse_expression(iter);
  return (AstNode *)quote_ast;
}
