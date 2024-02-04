#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <readline/chardefs.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "tokenizer.h"
#include "vector.h"

#define PROMPT_STYLE "> "

typedef enum {
  AK_Boolean,
  AK_Number,
} AstKind;

typedef union {
  bool boolean;
  double number;
} AstVal;

typedef struct AstNode {
  AstKind kind;
  AstVal val;
} AstNode;

static AstNode *make_ast_node(AstKind kind, AstVal val) {
  AstNode *node = (AstNode *)malloc(sizeof(AstNode));
  node->kind = kind;
  node->val = val;
  return node;
}

static AstNode *try_parse_number(Token *tok) {
  AstVal val;
  assert(tok->kind == TK_Number);
  val.number = strtod(tok->literal, NULL);
  return make_ast_node(AK_Number, val);
}

static void free_ast_node(AstNode *ast) {
}

AstNode *parse_object_recursive(Vector *tokens) {
  int num_tokens = vector_len(tokens);
  for (int i = 0; i < num_tokens; ++i) {
    Token *tok = vector_get(tokens, i);
    switch (tok->kind) {
    case TK_Number: {
      return try_parse_number(tok);
    }
    default:
      fprintf(stderr, "parse_object_recursive: not implemented!\n");
      exit(1);
    }
  }
  assert(0);
}

void dump_ast(AstNode *ast) {
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
    default:
      fprintf(stderr, "dump_ast: not implemented!\n");
      exit(1);
    }
  } else {
    fprintf(stdout, "<nil>\n");
  }
}

static const char *token_kind_to_string(TokenKind kind) {
  switch (kind) {
  case TK_Unknown:
    return "Unknown";
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
  default:
    return "Unknown Token";
  }
}

/*
 * Strip whitespace from the start and end of str.  Return a pointer
 * into str.
 */
char *stripwhite(char *str) {
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
    char *stripped;
    line = readline(PROMPT_STYLE);
    if (!line) {
      /* Handle ctrl-d */
      break;
    }
    stripped = stripwhite(line);
    if (stripped[0]) {
      Vector *tokens = tokenize(stripped);
      int num_tokens = vector_len(tokens);
      AstNode *ast = NULL;
      for (int i = 0; i < num_tokens; ++i) {
        Token *tok = vector_get(tokens, i);
        for (int l = 0; l < tok->tok_len; ++l)
          fprintf(stdout, "%c", tok->literal[l]);
        fprintf(stdout, " -- %s\n", token_kind_to_string(tok->kind));
      }

      ast = parse_object_recursive(tokens);

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

  exit(0);
}

int main(int argc, char **argv) {
  return rocket_main(argc, argv);
}
