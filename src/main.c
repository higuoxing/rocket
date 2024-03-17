#include "common.h"

#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "tokenizer.h"
#include "vector.h"
#include "vm.h"

#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int flag_debug_only_tokenize = 0;
static int flag_debug_dump_tokens = 0;
static char *tokens_output_file = NULL;
static int flag_debug_dump_ast = 0;

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
      {"debug-dump-tokens", optional_argument, &flag_debug_dump_tokens, 1},
      {"debug-dump-ast", no_argument, &flag_debug_dump_ast, 1},
      {"debug-only-tokenize", no_argument, &flag_debug_only_tokenize, 1},
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
}

static void debug_dump_tokens(const char *output_file_name,
                              Tokenizer *tokenizer) {
  int i = 0;
  FILE *output_file = NULL;
  Token *tok = NULL;
  TokenIter iter = tokenizer_iter(tokenizer);

  if (output_file_name) {
    output_file = fopen(output_file_name, "w+");
    if (!output_file) {
      fprintf(stderr, "cannot open file \"%s\"for dumping tokens\n",
              output_file_name);
      exit(1);
    }
  }

  /* Force all tokens to be consumed. */
  for (tok = token_iter_peek(&iter); tok->kind != TOKEN_EOF;
       tok = token_iter_next(&iter)) {
    fprintf(output_file ? output_file : stdout, "(%d:%d) %s: %s\n",
            tok->loc.line, tok->loc.column, token_kind_str(tok),
            tok->literal ? tok->literal : "");
  }

  if (output_file)
    fclose(output_file);
}

static int eval_script(const char *script_name) {
  char *script;
  Tokenizer tokenizer;

  script = read_file(script_name);
  initialize_tokenizer(&tokenizer, script_name, script);

  if (flag_debug_dump_tokens)
    debug_dump_tokens(tokens_output_file, &tokenizer);

  if (flag_debug_dump_ast)
    (void)(0);

  destroy_tokenizer(&tokenizer);
  free(script);

  return 0;
}

static int rocket_main(int argc, char **argv) {
  Vector *tokens = NULL;
  char *script_name;
  Vector *parsed_prog = NULL;

  rocket_parse_command_args(argc, argv);

  if (optind >= argc) {
    fprintf(stderr, "script file is missing\n");
    exit(1);
  }

  return eval_script(argv[optind]);
}

int main(int argc, char **argv) {
  return rocket_main(argc, argv);
}
