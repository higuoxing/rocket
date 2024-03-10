#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "ast.h"
#include "common.h"
#include "vm.h"

typedef struct Compiler {
  ObjectsPool *constants;
  Instructions *instructions;
} Compiler;

typedef enum CompilerErr { COMPILE_SUCCESS } CompilerErr;

extern void initialize_compiler(Compiler *c);
extern void destroy_compiler(Compiler *c);
extern Compiler *make_compiler(void);
extern CompilerErr compile_expression(Compiler *c, AstNode *ast);
extern uint32_t compiler_add_constant(Compiler *c, Object val);
extern void compiler_emit_instruction(Compiler *c, uint8_t instruction);
extern ObjectsPool *compiler_give_out_constants(Compiler *c);
extern Instructions *compiler_give_out_instructions(Compiler *c);
extern void free_compiler(Compiler *c);

#endif
