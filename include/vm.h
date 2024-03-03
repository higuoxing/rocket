#ifndef _VM_H_
#define _VM_H_

#include <stdint.h>

#include "ast.h"
#include "vector.h"

#define VM_STACK_MAX_DEPTH 256
#define VM_FRAME_MAX_DEPTH 256

typedef enum OpCode {
  OP_CONSTANT,
  OP_PROC_CALL,
  OP_RETURN,
  OP_LAST,
} OpCode;

typedef enum ValueType {
  VAL_NIL = 0,
  VAL_BOOL,
  VAL_NUMBER,
} ValueType;

typedef struct Value {
  ValueType type;
  Datum value;
} Value;

VECTOR_GENERATE_TYPE_NAME(Value, ValuesPool, values_pool);
VECTOR_GENERATE_TYPE_NAME(uint8_t, Instructions, instructions);

typedef struct CompiledFunction {
  Instructions *instructions;
  /* Local variables are stored on VM::stack. */
  int num_locals;
} CompiledFunction;

typedef struct Frame {
  uint8_t *ip; /* Instruction pointer */
  CompiledFunction *fn;
  uint32_t base_pointer;
} Frame;

typedef struct VM {
  uint32_t stack_pointer; /* Offset into the stack array. */
  uint32_t frame_pointer; /* Offset into the frames array. */
  Frame frames[VM_FRAME_MAX_DEPTH];
  Value stack[VM_STACK_MAX_DEPTH];
  ValuesPool *constants;
  ValuesPool *globals;
  ValuesPool *heap;
} VM;

typedef enum InterpretResult {
  INTERPRET_OK,
} InterpretResult;

static inline uint32_t values_pool_add_constant(ValuesPool *values_pool,
                                                Value val) {
  values_pool_append(values_pool, val);
  return values_pool_len(values_pool) - 1;
}

extern VM *make_vm(Instructions *instructions, ValuesPool *constants,
                   ValuesPool *globals);
extern InterpretResult vm_run(VM *vm);
extern void free_vm(VM *vm);

extern CompiledFunction *make_compiled_function(Instructions *instrs,
                                                int num_locals);
extern void free_compiled_function(CompiledFunction *compiled_fn);

#endif
