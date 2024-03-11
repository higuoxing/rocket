#include "common.h"

#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "tokenizer.h"
#include "vector.h"
#include "vm.h"

#include <assert.h>
#include <bits/getopt_core.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int flag_dump_tokens = 0;
static char *tokens_output_file = NULL;
static int flag_dump_ast = 0;

static void dump_ast(AstNode *ast) {
  //   if (ast) {
  //     switch (ast->kind) {
  //     case AST_BOOL: {
  //       fprintf(stdout, "<bool>: %s\n", ((AstBool *)ast)->boolean ? "#t" :
  //       "#f"); break;
  //     }
  //     case AST_CHAR: {
  //       fprintf(stdout, "<char>: %c\n", ((AstChar *)ast)->char_);
  //       break;
  //     }
  //     case AST_NUMBER: {
  //       fprintf(stdout, "<number>: %.2f\n", ((AstNumber *)ast)->number);
  //       break;
  //     }
  //     case AST_IDENT: {
  //       fprintf(stdout, "<ident>: %s\n", ((AstIdent *)ast)->ident);
  //       break;
  //     }
  //     case AST_PROC_CALL: {
  //       fprintf(stdout, "<proc_call>: ");
  //       dump_ast(((AstProcCall *)ast)->callable);
  //       for (int i = 0; i < vector_len(((AstProcCall *)ast)->args); ++i) {
  //         fprintf(stdout, "             ");
  //         dump_ast(
  //             (AstNode *)DatumGetPtr(vector_get(((AstProcCall *)ast)->args,
  //             i)));
  //       }
  //       fprintf(stdout, "\n");
  //       break;
  //     }
  //     default:
  //       fprintf(stderr, "%s: AstKind (%d) not implemented!\n", __FUNCTION__,
  //               ast->kind);
  //       exit(1);
  //     }
  //   } else {
  //     fprintf(stdout, "<nil>\n");
  //   }
}

// static int rocket_main(int argc, char **argv) {
//   char *line = NULL;
//
//   rl_initialize();
//
//   for (;;) {
//     line = readline(PROMPT_STYLE);
//     if (!line) {
//       /* Handle ctrl-d */
//       break;
//     }
//
//     Vector *tokens = tokenize(line, "<stdin>");
//     int num_tokens = vector_len(tokens);
//     Vector *program = parse_program(tokens);
//     Compiler compiler;
//     VM vm;
//
//     initialize_compiler(&compiler);
//
// #if 0
//     for (int i = 0; i < vector_len(program); ++i) {
//       dump_ast(DatumGetPtr(vector_get(program, i)));
//     }
// #endif
//
//     /*
//      * TODO: Add support for executing multiple expressions.
//      */
//     assert(vector_len(program) == 1);
//     for (int i = 0; i < vector_len(program); ++i) {
//       assert(
//           compile_expression(&compiler, DatumGetPtr(vector_get(program, i)))
//           == COMPILE_SUCCESS);
//     }
//     initialize_vm(&vm, compiler_give_out_instructions(&compiler),
//                   compiler_give_out_constants(&compiler), NULL);
//     vm_run(&vm);
//
//     int stack_top = vm.stack_pointer;
//     switch (vm.stack[stack_top - 1].type) {
//     case OBJ_BOOL: {
//       fprintf(stdout, "%s\n",
//               DatumGetBool(vm.stack[stack_top - 1].value) ? "#t" : "#f");
//       break;
//     }
//     case OBJ_NUMBER: {
//       fprintf(stdout, "%.4f\n", DatumGetFloat(vm.stack[stack_top -
//       1].value)); break;
//     }
//     default: {
//       fprintf(stderr, "%s: unrecognized value type: %d\n", __FUNCTION__,
//               vm.stack[stack_top - 1].type);
//       exit(1);
//     }
//     }
//
//     /* Clean up. */
//     for (int i = 0; i < num_tokens; ++i) {
//       free_token((Token *)vector_get(tokens, i));
//     }
//     free_vector(tokens);
//
//     for (int i = 0; i < vector_len(program); ++i) {
//       AstNode *ast = DatumGetPtr(vector_get(program, i));
//       if (ast)
//         free_ast_node(ast);
//     }
//     free_vector(program);
//     destroy_compiler(&compiler);
//     destroy_vm(&vm);
//
//     free(line);
//     line = NULL;
//     break;
//   }
//
//   return 0;
// }

static char *read_file(const char *filename) {
  char *script = malloc(BLKSZ);
  FILE *script_file = NULL;
  uint32_t total_read_size = 0;
  uint32_t curr_read_size = 0;
  uint32_t curr_buffer_size = BLKSZ;

  if (!script) {
    fprintf(stderr, "OOM! %m");
    exit(1);
  }

  script_file = fopen(filename, "r");
  if (!script_file) {
    fprintf(stderr, "cannot open script file: \"%s\" %m\n", filename);
    exit(1);
  }

  while ((curr_read_size = fread(&script[total_read_size], sizeof(char), BLKSZ,
                                 script_file)) > 0) {
    total_read_size += curr_read_size;

    if (total_read_size == curr_buffer_size) {
      curr_buffer_size *= 2;
      script = realloc(script, curr_buffer_size);
    }
  }

  script[total_read_size] = '\0';

  fclose(script_file);
  return script;
}

static void rocket_parse_command_args(int argc, char **argv) {
  int c;
  int option_index = 0;
  static struct option long_options[] = {
      {"dump-tokens", optional_argument, &flag_dump_tokens, 1},
      {"dump-ast", no_argument, &flag_dump_ast, 1},
      {0, 0, 0, 0},
  };

  while (1) {
    c = getopt_long(argc, argv, "", long_options, &option_index);

    /* Detect the end of options. */
    if (c == -1) {
      break;
    }

    switch (c) {
    case 0: {
      if (optarg)
        tokens_output_file = strdup(optarg);
      break;
    }
    default:
      abort();
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "script file is missing\n");
    exit(1);
  }
}

static void dump_tokens(const char *output_file_name, Tokenizer *tokenizer) {
  int i = 0;
  FILE *output_file = NULL;
  Token *tok = NULL;

  if (output_file_name) {
    output_file = fopen(output_file_name, "w+");
    if (!output_file) {
      fprintf(stderr, "cannot open file \"%s\"for dumping tokens\n",
              output_file_name);
      exit(1);
    }
  }

  while ((tok = tokenizer_next(tokenizer)) != NULL) {
    fprintf(output_file ? output_file : stdout, "(%d:%d) %s: %s\n",
            tok->loc.line, tok->loc.column, token_kind_str(tok),
            tok->literal ? tok->literal : "");
  }

  if (output_file)
    fclose(output_file);
}

static int rocket_main(int argc, char **argv) {
  char *script = NULL;
  Vector *tokens = NULL;
  char *script_name;
  Tokenizer tokenizer;

  rocket_parse_command_args(argc, argv);

  script_name = argv[optind];
  reset_tokenizer(&tokenizer, script_name);

  if (flag_dump_tokens) {
    dump_tokens(tokens_output_file, &tokenizer);
  }

  if (flag_dump_ast) {
    dump_ast(NULL);
  }

  return 0;
}

int main(int argc, char **argv) {
  return rocket_main(argc, argv);
}
