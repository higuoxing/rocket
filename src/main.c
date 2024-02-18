#include "common.h"

#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include "vector.h"

#define PROMPT_STYLE "> "

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
    Vector *program = NULL;

    program = parse_program(tokens);

    for (int i = 0; i < vector_len(program); ++i) {
      dump_ast(vector_get(program, i));
    }

    /* Clean up. */
    for (int i = 0; i < num_tokens; ++i) {
      free(vector_get(tokens, i));
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
