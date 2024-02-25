#ifndef _VM_H_
#define _VM_H_

#include <stdint.h>

#include "ast.h"
#include "vector.h"

#define CHUNK_CODE_DEFAULT_INIT_SIZE 32
#define VM_STACK_MAX_DEPTH 256

typedef enum OpCode {
  OP_CONSTANT,
  OP_PROC_CALL,
  OP_RETURN,
} OpCode;

typedef enum ValueType {
  VAL_BOOL,
  VAL_NUMBER,
} ValueType;

typedef struct Value {
  ValueType type;
  Datum value;
} Value;

VECTOR_GENERATE_TYPE_NAME(Value, ConstantPool, constant_pool);
VECTOR_GENERATE_TYPE_NAME(uint8_t, Chunk, chunk);

typedef struct VM {
  Chunk *code;
  ConstantPool *constant_pool;
  uint8_t *ip; /* instruction pointer */
  Value stack[VM_STACK_MAX_DEPTH];
  Value *sp; /* stack pointer */
} VM;

typedef enum Status {
  STATUS_OK,
  STATUS_ERR,
} ResultStatus;

typedef struct Result {
  ResultStatus status;
  Datum val;
} Result;

extern void write_byte(Chunk *chunk, uint8_t insn);
extern int add_constant(ConstantPool *constant_pool, Value val);
extern void vm_init(void);
extern void free_vm(VM *vm);

extern int compile(Ast *expr);
extern Result interpret(Chunk *chunk, ConstantPool *constant_pool);

#endif
