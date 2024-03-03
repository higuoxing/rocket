#ifndef _VM_H_
#define _VM_H_

#include <stdint.h>

#include "ast.h"
#include "object.h"
#include "vector.h"

#define VM_STACK_MAX_DEPTH 256
#define VM_FRAME_MAX_DEPTH 256

typedef enum OpCode {
  OP_CONSTANT,
  OP_PROC_CALL,
  OP_RETURN,
  OP_LAST,
} OpCode;

VECTOR_GENERATE_TYPE_NAME(Object, ObjectsPool, objects_pool);
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
  Object stack[VM_STACK_MAX_DEPTH];
  ObjectsPool *constants;
  ObjectsPool *globals;
  ObjectsPool *heap;
} VM;

typedef enum InterpretResult {
  INTERPRET_OK,
} InterpretResult;

static inline uint32_t objects_pool_add_constant(ObjectsPool *objects_pool,
                                                 Object val) {
  objects_pool_append(objects_pool, val);
  return objects_pool_len(objects_pool) - 1;
}

extern void initialize_vm(VM *vm, Instructions *instructions,
                          ObjectsPool *constants, ObjectsPool *globals);
extern InterpretResult vm_run(VM *vm);
extern void destroy_vm(VM *vm);

extern CompiledFunction *make_compiled_function(Instructions *instrs,
                                                int num_locals);
extern void free_compiled_function(CompiledFunction *compiled_fn);

#endif
