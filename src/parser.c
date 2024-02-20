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
static Ast *parse_expression(Token **tokens, int *cursor);

Vector *parse_program(Vector *tokens) {
  int cursor = 0;
  Vector *program = make_vector();

  while (((Token *)vector_get(tokens, cursor))->kind != TK_EOF) {
    Ast *expr = parse_expression((Token **)vector_data(tokens), &cursor);
    vector_append(program, expr);
  }

  return program;
}

static Ast *parse_expression(Token **tokens, int *cursor) {
  assert(tokens[*cursor]);
  switch (tokens[*cursor]->kind) {
  case TK_Boolean: {
    Ast *bool_ast = parse_boolean(tokens[*cursor]);
    *cursor += 1;
    return bool_ast;
  }
  case TK_Char: {
    Ast *char_ast = parse_char(tokens[*cursor]);
    *cursor += 1;
    return char_ast;
  }
  case TK_Number: {
    Ast *number_ast = parse_number(tokens[*cursor]);
    *cursor += 1;
    return number_ast;
  }
  case TK_Ident: {
    // Ast *ident_ast =
    *cursor += 1;
    return NULL;
  }
  case TK_LParen: {
    Cons *list = NULL;
    AstVal val;
    /* Consume '(' */
    *cursor += 1;

    /* Parse until ')' */
    while (tokens[*cursor]->kind != TK_RParen) {
      Ast *inner_object = NULL;
      if (tokens[*cursor]->kind == TK_EOF) {
        /* Need more tokens. */
        fprintf(stderr, "%s: Expected more tokens.", __FUNCTION__);
        exit(1);
      }

      inner_object = parse_expression(tokens, cursor);

      list = make_cons(inner_object, list);
    }

    /* Consume ')' */
    assert(tokens[*cursor]->kind == TK_RParen);
    *cursor += 1;

    val.cons = list_reverse(list);
    return make_ast_node(AK_Cons, val);
  }
  case TK_EOF: {
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
  assert(tok->kind == TK_Boolean);
  val.boolean = (strncmp(tok->literal, "#t", 2) == 0 ? true : false);
  return make_ast_node(AK_Boolean, val);
}

static Ast *parse_char(Token *tok) {
  AstVal val;
  assert(tok->kind == TK_Char);
  /*
   * <character> -> #\ <any character>
   *              | #\ <character name>
   *              | #\x<hex scalar value>
   * <character name> -> alarm | backspace | delete
   *                  | escape | newline | null | return | space | tab
   */
  if (tok->literal_len == 3) {
    /* #\<any_char> */
    val.char_ = tok->literal[2];
  } else {
    if (tok->literal_len >= 4 && tok->literal[2] == 'x') {
      /* Handle #\x<hex_scalar> */
      char hex_str[100];

      if (tok->literal_len > sizeof(hex_str) - 1) {
        /* Raise error. */
        fprintf(stderr, "%s: Error\n", __FUNCTION__);
        exit(1);
      }
      memcpy(hex_str, tok->literal + 3, tok->literal_len - 3);
      hex_str[tok->literal_len - 2] = '\0';
      val.char_ = (char)strtol(hex_str, NULL, 16);
    } else {
      /* Handle #\<character name> */
      /* TODO: Add more characters. */
      if (strncmp(tok->literal, "#\\alarm", tok->literal_len) == 0) {
        val.char_ = 7;
      } else if (strncmp(tok->literal, "#\\backspace", tok->literal_len) == 0) {
        val.char_ = 8;
      } else if (strncmp(tok->literal, "#\\delete", tok->literal_len) == 0) {
        val.char_ = 127;
      } else if (strncmp(tok->literal, "#\\newline", tok->literal_len) == 0) {
        val.char_ = 10;
      } else if (strncmp(tok->literal, "#\\return", tok->literal_len) == 0) {
        val.char_ = 13;
      } else if (strncmp(tok->literal, "#\\space", tok->literal_len) == 0) {
        val.char_ = 32;
      } else if (strncmp(tok->literal, "#\\tab", tok->literal_len) == 0) {
        val.char_ = 9;
      } else {
        /* Otherwise, Raise error. */
        fprintf(stderr, "%s: Error\n", __FUNCTION__);
        exit(1);
      }
    }
  }

  return make_ast_node(AK_Char, val);
}

static Ast *parse_number(Token *tok) {
  AstVal val;
  assert(tok->kind == TK_Number);
  val.number = strtod(tok->literal, NULL);
  return make_ast_node(AK_Number, val);
}
