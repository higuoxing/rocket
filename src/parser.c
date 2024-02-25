#include "common.h"

#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include "vector.h"
#include <assert.h>
#include <string.h>

static Ast *parse_boolean(Token *tok);
static Ast *parse_char(Token *tok);
static Ast *parse_number(Token *tok);
static Ast *parse_ident(Token *tok);
static Ast *parse_expression(Token **tokens, int *cursor);

Vector *parse_program(Vector *tokens) {
  int cursor = 0;
  Vector *program = make_vector();

  while (((Token *)vector_get(tokens, cursor))->kind != TOKEN_EOF) {
    Ast *expr = parse_expression((Token **)vector_data(tokens), &cursor);
    vector_append(program, PointerGetDatum(expr));
  }

  return program;
}

static Ast *parse_expression(Token **tokens, int *cursor) {
  assert(tokens[*cursor]);
  switch (tokens[*cursor]->kind) {
  case TOKEN_BOOL: {
    Ast *bool_ast = parse_boolean(tokens[*cursor]);
    *cursor += 1;
    return bool_ast;
  }
  case TOKEN_CHAR: {
    Ast *char_ast = parse_char(tokens[*cursor]);
    *cursor += 1;
    return char_ast;
  }
  case TOKEN_NUMBER: {
    Ast *number_ast = parse_number(tokens[*cursor]);
    *cursor += 1;
    return number_ast;
  }
  case TOKEN_IDENT: {
    Ast *ident_ast = parse_ident(tokens[*cursor]);
    *cursor += 1;
    return ident_ast;
  }
  case TOKEN_LPAREN: {
    Cons *list = NULL;
    AstVal val;
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
      Ast *inner_object = NULL;
      if (tokens[*cursor]->kind == TOKEN_EOF) {
        /* Need more tokens. */
        fprintf(stderr, "%s: Expected more tokens.", __FUNCTION__);
        exit(1);
      }

      inner_object = parse_expression(tokens, cursor);

      if (arg_index == 0) {
        val.proc_call.callable = inner_object;
      } else {
        vector_append(args, PointerGetDatum(inner_object));
      }

      ++arg_index;
    }

    /* Consume ')' */
    assert(tokens[*cursor]->kind == TOKEN_RPAREN);
    *cursor += 1;

    val.proc_call.args = args;
    return make_ast_node(AST_PROC_CALL, val);
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

static Ast *parse_boolean(Token *tok) {
  AstVal val;
  assert(tok->kind == TOKEN_BOOL);
  val.boolean = (strcmp(tok->literal, "#t") == 0 ? true : false);
  return make_ast_node(AST_BOOL, val);
}

static Ast *parse_char(Token *tok) {
  AstVal val;
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
    val.char_ = tok->literal[2];
  } else {
    if (strlen(tok->literal) >= 4 && tok->literal[2] == 'x') {
      /* Handle #\x<hex_scalar> */
      val.char_ = (char)strtol(tok->literal, NULL, 16);
    } else {
      /* Handle #\<character name> */
      /* TODO: Add more characters. */
      if (strcmp(tok->literal, "#\\alarm") == 0) {
        val.char_ = 7;
      } else if (strcmp(tok->literal, "#\\backspace") == 0) {
        val.char_ = 8;
      } else if (strcmp(tok->literal, "#\\delete") == 0) {
        val.char_ = 127;
      } else if (strcmp(tok->literal, "#\\newline") == 0) {
        val.char_ = 10;
      } else if (strcmp(tok->literal, "#\\return") == 0) {
        val.char_ = 13;
      } else if (strcmp(tok->literal, "#\\space") == 0) {
        val.char_ = 32;
      } else if (strcmp(tok->literal, "#\\tab") == 0) {
        val.char_ = 9;
      } else {
        /* Otherwise, Raise error. */
        fprintf(stderr, "%s: Error\n", __FUNCTION__);
        exit(1);
      }
    }
  }

  return make_ast_node(AST_CHAR, val);
}

static Ast *parse_number(Token *tok) {
  AstVal val;
  assert(tok->kind == TOKEN_NUMBER);
  val.number = strtod(tok->literal, NULL);
  return make_ast_node(AST_NUMBER, val);
}

static Ast *parse_ident(Token *tok) {
  AstVal val;
  assert(tok->kind == TOKEN_IDENT);
  val.ident = strdup(tok->literal);
  return make_ast_node(AST_IDENT, val);
}
