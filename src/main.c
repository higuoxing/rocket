#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/chardefs.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "tokenizer.h"
#include "vector.h"

#define PROMPT_STYLE "> "

typedef enum {
  AK_Boolean,
  AK_Number,
  AK_Cons,
} AstKind;

typedef struct Cons {
  void *car;
  void *cdr;
} Cons;

Cons *make_cons(void *car, void *cdr) {
  Cons *cons = (Cons *)malloc(sizeof(Cons));
  cons->car = car;
  cons->cdr = cdr;
  return cons;
}

void cons_append(Cons *head, void *tail) {
  assert(head);
  head->cdr = tail;
}

Cons *list_reverse(Cons *list) {
  Cons *tail = NULL;
  if (!list)
    return NULL;

  while (list) {
    Cons *curr = list;
    tail = make_cons(curr->car, tail);
    list = curr->cdr;
    free(curr);
  }

  return tail;
}

typedef union AstVal {
  /* AK_Boolean */
  bool boolean;
  /* AK_Number */
  double number;
  /* AK_Cons */
  Cons *cons;
} AstVal;

typedef struct AstNode {
  AstKind kind;
  AstVal val;
} Ast;

static Ast *make_ast_node(AstKind kind, AstVal val) {
  Ast *node = (Ast *)malloc(sizeof(Ast));
  node->kind = kind;
  node->val = val;
  return node;
}

static Ast *try_parse_boolean(Token *tok) {
  AstVal val;
  assert(tok->kind == TK_Boolean);
  val.boolean = (strncmp(tok->literal, "#t", 2) == 0 ? true : false);
  return make_ast_node(AK_Boolean, val);
}

static Ast *try_parse_number(Token *tok) {
  AstVal val;
  assert(tok->kind == TK_Number);
  val.number = strtod(tok->literal, NULL);
  return make_ast_node(AK_Number, val);
}

static void dump_ast(Ast *ast) {
  if (ast) {
    switch (ast->kind) {
    case AK_Boolean: {
      fprintf(stdout, "<bool>: %s\n", ast->val.boolean ? "#t" : "#f");
      break;
    }
    case AK_Number: {
      fprintf(stdout, "<number>: %.2f\n", ast->val.number);
      break;
    }
    case AK_Cons: {
      Cons *cons = ast->val.cons;

      while (cons) {
        dump_ast(cons->car);
        cons = cons->cdr;
      }
      break;
    }
    default:
      fprintf(stderr, "%s: AstKind (%d) not implemented!\n", __FUNCTION__,
              ast->kind);
      exit(1);
    }
  } else {
    fprintf(stdout, "<nil>\n");
  }
}

static Ast *parse_object(Token **tokens, int *cursor) {
  switch (tokens[*cursor]->kind) {
  case TK_Boolean: {
    Ast *bool_ast = try_parse_boolean(tokens[*cursor]);
    *cursor += 1;
    return bool_ast;
  }
  case TK_Number: {
    Ast *number_ast = try_parse_number(tokens[*cursor]);
    *cursor += 1;
    return number_ast;
  }
  case TK_LParen: {
    Cons *list = NULL;
    AstVal val;
    /* Consume '(' */
    *cursor += 1;

    /* Parse until ')' */
    while (tokens[*cursor]->kind != TK_RParen) {
      if (tokens[*cursor]->kind == TK_EOF) {
        /* Need more tokens. */
        fprintf(stderr, "%s: Expected more tokens.", __FUNCTION__);
        exit(1);
      }

      list = make_cons(parse_object(tokens, cursor), list);
    }

    /* Consume ')' */
    assert(tokens[*cursor]->kind == TK_RParen);
    *cursor += 1;

    val.cons = list_reverse(list);
    return make_ast_node(AK_Cons, val);
  }
  default: {
    fprintf(stderr, "%s: Unexpected token kind (%d)", __FUNCTION__,
            tokens[*cursor]->kind);
    exit(1);
  }
  }
  return NULL;
}

Ast *parse_program(Token **tokens) {
  int cursor = 0;
  dump_ast(parse_object(tokens, &cursor));
  return NULL;
}

static const char *token_kind_to_string(TokenKind kind) {
  switch (kind) {
  case TK_LParen:
    return "LParen";
  case TK_RParen:
    return "RParen";
  case TK_Dot:
    return "Dot";
  case TK_Ident:
    return "Ident";
  case TK_Quote:
    return "Singlequote";
  case TK_Backquote:
    return "Backquote";
  case TK_Boolean:
    return "Boolean";
  case TK_Number:
    return "Number";
  case TK_EOF:
    return "EOF";
  default:
    return "Unknown Token";
  }
}

/*
 * Strip whitespace from the start and end of str.  Return a pointer
 * into str.
 */
static char *stripwhite(char *str) {
  char *s, *t;

  for (s = str; whitespace(*s); s++)
    ;

  if (*s == 0)
    return (s);

  t = s + strlen(s) - 1;
  while (t > s && whitespace(*t))
    t--;
  *++t = '\0';

  return s;
}

static int rocket_main(int argc, char **argv) {
  char *line = NULL;

  rl_initialize();

  for (;;) {
    line = readline(PROMPT_STYLE);
    if (!line) {
      /* Handle ctrl-d */
      break;
    }
    if (line[0]) {
      Vector *tokens = tokenize(line, "<stdin>");
      int num_tokens = vector_len(tokens);
      Ast *ast = NULL;

      ast = parse_program((Token **)vector_data(tokens));

      dump_ast(ast);

      /* Clean up. */
      for (int i = 0; i < num_tokens; ++i) {
        free(vector_get(tokens, i));
      }
      free_vector(tokens);
    }

    free(line);
    line = NULL;
  }

  return 0;
}

int main(int argc, char **argv) {
  return rocket_main(argc, argv);
}
