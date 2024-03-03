#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "ast.h"
#include "common.h"
#include "vm.h"

typedef struct Compiler {
  ValuesPool *constants;
  Instructions *instructions;
} Compiler;

typedef enum CompilerErr { COMPILE_SUCCESS } CompilerErr;

extern Compiler *make_compiler(void);
extern CompilerErr compile_expression(Compiler *c, AstNode *ast);
extern uint32_t compiler_add_constant(Compiler *c, Value val);
extern void compiler_emit_instruction(Compiler *c, uint8_t instruction);
extern ValuesPool *compiler_give_out_constants(Compiler *c);
extern Instructions *compiler_give_out_instructions(Compiler *c);
extern void free_compiler(Compiler *c);

#endif
