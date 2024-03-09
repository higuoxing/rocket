#include "common.h"

#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "tokenizer.h"
#include "vector.h"
#include "vm.h"
#include <assert.h>
#include <stdio.h>

#define PROMPT_STYLE "> "

static void dump_ast(AstNode *ast) {
  if (ast) {
    switch (ast->kind) {
    case AST_BOOL: {
      fprintf(stdout, "<bool>: %s\n", ((AstBool *)ast)->boolean ? "#t" : "#f");
      break;
    }
    case AST_CHAR: {
      fprintf(stdout, "<char>: %c\n", ((AstChar *)ast)->char_);
      break;
    }
    case AST_NUMBER: {
      fprintf(stdout, "<number>: %.2f\n", ((AstNumber *)ast)->number);
      break;
    }
    case AST_IDENT: {
      fprintf(stdout, "<ident>: %s\n", ((AstIdent *)ast)->ident);
      break;
    }
    case AST_PROC_CALL: {
      fprintf(stdout, "<proc_call>: ");
      dump_ast(((AstProcCall *)ast)->callable);
      for (int i = 0; i < vector_len(((AstProcCall *)ast)->args); ++i) {
        fprintf(stdout, "             ");
        dump_ast(
            (AstNode *)DatumGetPtr(vector_get(((AstProcCall *)ast)->args, i)));
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
    Vector *program = parse_program(tokens);
    Compiler *compiler = make_compiler();
    VM vm;

#if 0
    for (int i = 0; i < vector_len(program); ++i) {
      dump_ast(DatumGetPtr(vector_get(program, i)));
    }
#endif

    /*
     * TODO: Add support for executing multiple expressions.
     */
    assert(vector_len(program) == 1);
    for (int i = 0; i < vector_len(program); ++i) {
      assert(compile_expression(compiler, DatumGetPtr(vector_get(
                                              program, i))) == COMPILE_SUCCESS);
    }
    initialize_vm(&vm, compiler_give_out_instructions(compiler),
                  compiler_give_out_constants(compiler), NULL);
    vm_run(&vm);

    int stack_top = vm.stack_pointer;
    switch (vm.stack[stack_top - 1].type) {
    case OBJ_BOOL: {
      fprintf(stdout, "%s\n",
              DatumGetBool(vm.stack[stack_top - 1].value) ? "#t" : "#f");
      break;
    }
    case OBJ_NUMBER: {
      fprintf(stdout, "%.4f\n", DatumGetFloat(vm.stack[stack_top - 1].value));
      break;
    }
    default: {
      fprintf(stderr, "%s: unrecognized value type: %d\n", __FUNCTION__,
              vm.stack[stack_top - 1].type);
      exit(1);
    }
    }

    /* Clean up. */
    for (int i = 0; i < num_tokens; ++i) {
      free_token((Token *)vector_get(tokens, i));
    }
    free_vector(tokens);

    for (int i = 0; i < vector_len(program); ++i) {
      AstNode *ast = DatumGetPtr(vector_get(program, i));
      if (ast)
        free_ast_node(ast);
    }
    free_vector(program);
    free_compiler(compiler);
    destroy_vm(&vm);

    free(line);
    line = NULL;
    break;
  }

  return 0;
}

int main(int argc, char **argv) {
  return rocket_main(argc, argv);
}
