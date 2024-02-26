#include "common.h"

#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include "vector.h"
#include <assert.h>
#include <string.h>

static AstNode *parse_boolean(Token *tok);
static AstNode *parse_char(Token *tok);
static AstNode *parse_number(Token *tok);
static AstNode *parse_ident(Token *tok);
static AstNode *parse_expression(Token **tokens, int *cursor);

Vector *parse_program(Vector *tokens) {
  int cursor = 0;
  Vector *program = make_vector();

  while (((Token *)vector_get(tokens, cursor))->kind != TOKEN_EOF) {
    AstNode *expr = parse_expression((Token **)vector_data(tokens), &cursor);
    vector_append(program, PointerGetDatum(expr));
  }

  return program;
}

static AstNode *parse_expression(Token **tokens, int *cursor) {
  assert(tokens[*cursor]);
  switch (tokens[*cursor]->kind) {
  case TOKEN_BOOL: {
    AstNode *bool_ast = parse_boolean(tokens[*cursor]);
    *cursor += 1;
    return bool_ast;
  }
  case TOKEN_CHAR: {
    AstNode *char_ast = parse_char(tokens[*cursor]);
    *cursor += 1;
    return char_ast;
  }
  case TOKEN_NUMBER: {
    AstNode *number_ast = parse_number(tokens[*cursor]);
    *cursor += 1;
    return number_ast;
  }
  case TOKEN_IDENT: {
    AstNode *ident_ast = parse_ident(tokens[*cursor]);
    *cursor += 1;
    return ident_ast;
  }
  case TOKEN_LPAREN: {
    AstNode *callable = NULL;
    int arg_index = 0;
    Vector *args = NULL;

    /* Consume '(' */
    *cursor += 1;

    if (tokens[*cursor]->kind == TOKEN_RPAREN) {
      /* Consume ')' */
      *cursor += 1;
      /* This is `()` (or nil), we return it directly. */
      return NULL;
    }

    args = make_vector();

    /* Parse until ')' */
    while (tokens[*cursor]->kind != TOKEN_RPAREN) {
      AstNode *inner_ast = NULL;
      if (tokens[*cursor]->kind == TOKEN_EOF) {
        /* Need more tokens. */
        fprintf(stderr, "%s: Expected more tokens.", __FUNCTION__);
        exit(1);
      }

      inner_ast = parse_expression(tokens, cursor);

      if (arg_index == 0) {
        callable = inner_ast;
      } else {
        vector_append(args, PointerGetDatum(inner_ast));
      }

      ++arg_index;
    }

    /* Consume ')' */
    assert(tokens[*cursor]->kind == TOKEN_RPAREN);
    *cursor += 1;

    return make_ast_proc_call(callable, args);
  }
  case TOKEN_EOF: {
    /* Need more tokens. */
    fprintf(stderr, "%s: Expected more tokens.", __FUNCTION__);
    exit(1);
  }
  default: {
    fprintf(stderr, "%s: Unexpected token kind (%d)", __FUNCTION__,
            tokens[*cursor]->kind);
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
