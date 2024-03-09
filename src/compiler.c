#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "common.h"
#include "compiler.h"
#include "vm.h"

Compiler *make_compiler(void) {
  Compiler *c = malloc(sizeof(Compiler));
  c->constants = make_objects_pool();
  c->instructions = make_instructions();
  return c;
}

CompilerErr compile_expression(Compiler *c, AstNode *ast) {
  switch (ast->kind) {
  case AST_BOOL: {
    Object boolean;
    AstBool *node = (AstBool *)ast;
    boolean.type = OBJ_BOOL;
    boolean.value = BoolGetDatum(node->boolean);
    compiler_emit_instruction(c, OP_CONSTANT);
    compiler_emit_instruction(c, compiler_add_constant(c, boolean));
    break;
  case AST_NUMBER: {
    Object number;
    AstNumber *node = (AstNumber *)ast;
    number.type = OBJ_NUMBER;
    number.value = FloatGetDatum(node->number);
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

uint32_t compiler_add_constant(Compiler *c, Object val) {
  objects_pool_add_constant(c->constants, val);
  return objects_pool_len(c->constants) - 1;
}

void compiler_emit_instruction(Compiler *c, uint8_t instr) {
  instructions_append(c->instructions, instr);
}

ObjectsPool *compiler_give_out_constants(Compiler *c) {
  ObjectsPool *constants = c->constants;
  c->constants = NULL;
  return constants;
}

Instructions *compiler_give_out_instructions(Compiler *c) {
  Instructions *instructions = c->instructions;
  c->instructions = NULL;
  return instructions;
}

void free_compiler(Compiler *c) {
  if (c->constants)
    free_objects_pool(c->constants);
  if (c->instructions)
    free_instructions(c->instructions);
  free(c);
}
