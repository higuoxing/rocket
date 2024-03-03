#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "common.h"
#include "compiler.h"
#include "vm.h"

Compiler *make_compiler(void) {
  Compiler *c = malloc(sizeof(Compiler));
  c->constants = make_values_pool();
  c->instructions = make_instructions();
  return c;
}

CompilerErr compile_expression(Compiler *c, AstNode *ast) {
  switch (ast->kind) {
  case AST_BOOL: {
    Value boolean;
    AstBool *boolean_ast = (AstBool *)ast;
    boolean.type = VAL_BOOL;
    boolean.value = BoolGetDatum(boolean_ast->boolean);
    compiler_emit_instruction(c, OP_CONSTANT);
    compiler_emit_instruction(c, compiler_add_constant(c, boolean));
    break;
  case AST_NUMBER: {
    Value number;
    AstNumber *number_ast = (AstNumber *)ast;
    number.type = VAL_NUMBER;
    number.value = FloatGetDatum(number_ast->number);
    compiler_emit_instruction(c, OP_CONSTANT);
    compiler_emit_instruction(c, compiler_add_constant(c, number));
    break;
  }
  default: {
    fprintf(stderr, "%s: unrecognized ast node (%d)", __FUNCTION__, ast->kind);
    exit(1);
  }
  }
  }
  compiler_emit_instruction(c, OP_LAST);
  return COMPILE_SUCCESS;
}

uint32_t compiler_add_constant(Compiler *compiler, Value val) {
  values_pool_add_constant(compiler->constants, val);
  return values_pool_len(compiler->constants) - 1;
}

void compiler_emit_instruction(Compiler *compiler, uint8_t instruction) {
  instructions_append(compiler->instructions, instruction);
}

ValuesPool *compiler_give_out_constants(Compiler *compiler) {
  ValuesPool *constants = compiler->constants;
  compiler->constants = NULL;
  return constants;
}

Instructions *compiler_give_out_instructions(Compiler *compiler) {
  Instructions *instructions = compiler->instructions;
  compiler->instructions = NULL;
  return instructions;
}

void free_compiler(Compiler *c) {
  if (c->constants)
    free_values_pool(c->constants);
  if (c->instructions)
    free_instructions(c->instructions);
  free(c);
}
