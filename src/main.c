#include "common.h"

#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include "vector.h"

#define PROMPT_STYLE "> "

static void dump_ast(Ast *ast) {
  if (ast) {
    switch (ast->kind) {
    case AST_BOOL: {
      fprintf(stdout, "<bool>: %s\n", ast->val.boolean ? "#t" : "#f");
      break;
    }
    case AST_CHAR: {
      fprintf(stdout, "<char>: %c\n", ast->val.char_);
      break;
    }
    case AST_NUMBER: {
      fprintf(stdout, "<number>: %.2f\n", ast->val.number);
      break;
    }
    case AST_CONS: {
      Cons *cons = ast->val.cons;

      while (cons) {
        dump_ast(cons->car);
        cons = cons->cdr;
      }
      break;
    }
    case AST_IDENT: {
      fprintf(stdout, "<ident>: %s\n", ast->val.ident);
      break;
    }
    case AST_PROC_CALL: {
      fprintf(stdout, "<proc_call>: ");
      dump_ast(ast->val.proc_call.callable);
      for (int i = 0; i < vector_len(ast->val.proc_call.args); ++i) {
        fprintf(stdout, "             ");
        dump_ast((Ast *)DatumGetPtr(vector_get(ast->val.proc_call.args, i)));
      }
      fprintf(stdout, "\n");
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

static const char *token_kind_to_string(TokenKind kind) {
  switch (kind) {
  case TOKEN_LPAREN:
    return "LParen";
  case TOKEN_RPAREN:
    return "RParen";
  case TOKEN_DOT:
    return "Dot";
  case TOKEN_IDENT:
    return "Ident";
  case TOKEN_QUOTE:
    return "Singlequote";
  case TOKEN_BACKQUOTE:
    return "Backquote";
  case TOKEN_BOOL:
    return "Boolean";
  case TOKEN_NUMBER:
    return "Number";
  case TOKEN_EOF:
    return "EOF";
  default:
    return "Unknown Token";
  }
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

    Vector *tokens = tokenize(line, "<stdin>");
    int num_tokens = vector_len(tokens);
    Vector *program;

    program = parse_program(tokens);

    for (int i = 0; i < vector_len(program); ++i) {
      dump_ast(DatumGetPtr(vector_get(program, i)));
    }

    /* Clean up. */
    for (int i = 0; i < num_tokens; ++i) {
      free_token((Token *)vector_get(tokens, i));
    }
    free_vector(tokens);

    free(line);
    line = NULL;
  }

  return 0;
}

int main(int argc, char **argv) {
  return rocket_main(argc, argv);
}
