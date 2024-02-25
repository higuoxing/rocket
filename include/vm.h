#ifndef _VM_H_
#define _VM_H_

#include <stdint.h>

#include "vector.h"

#define CHUNK_CODE_DEFAULT_INIT_SIZE 32
#define VM_STACK_MAX_DEPTH 256

typedef enum OpCode {
  OP_CONSTANT,
  OP_RETURN,
} OpCode;

typedef Datum Value;
typedef Vector ValueArray;

typedef struct Chunk {
  int cap;
  int len;
  uint8_t *code;
  ValueArray *constants;
} CodeChunk;

typedef struct VM {
  CodeChunk *chunk;
  uint8_t *ip;
  Datum stack[VM_STACK_MAX_DEPTH];
  Datum *sp; /* stack pointer */
} VM;

typedef enum Status {
  StatusOK,
  StatusErr,
} ResultStatus;

typedef struct Result {
  ResultStatus status;
  Datum val;
} Result;

extern CodeChunk *make_chunk(void);
extern void write_insn(CodeChunk *chunk, uint8_t insn);
extern void free_chunk(CodeChunk *chunk);
extern int add_constant(CodeChunk *chunk, Value val);
extern void vm_init(void);
extern void free_vm(VM *vm);

extern Result interpret(CodeChunk *chunk);

#endif
