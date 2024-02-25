#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "vector.h"
#include "vm.h"

static VM vm;

CodeChunk *make_chunk(void) {
  CodeChunk *chunk = (CodeChunk *)malloc(sizeof(CodeChunk));
  chunk->len = 0;
  chunk->cap = CHUNK_CODE_DEFAULT_INIT_SIZE;
  chunk->code = (uint8_t *)malloc(CHUNK_CODE_DEFAULT_INIT_SIZE);
  chunk->constants = make_vector();
  return chunk;
}

static void chunk_code_resize(CodeChunk *chunk, int new_size) {
  uint8_t *new_code = (uint8_t *)realloc(chunk->code, new_size);
  if (new_code) {
    chunk->code = new_code;
    chunk->cap = new_size;
  }
}

void write_insn(CodeChunk *chunk, uint8_t insn) {
  if (chunk->cap <= chunk->len + sizeof(insn)) {
    chunk_code_resize(chunk, chunk->cap * 2);
  }
  chunk->code[chunk->len] = insn;
  chunk->len += sizeof(insn);
}

void free_chunk(CodeChunk *chunk) {
  free(chunk->code);
  free_vector(chunk->constants);
  free(chunk);
}

int add_constant(CodeChunk *chunk, Value val) {
  vector_append(chunk->constants, val);
  return vector_len(chunk->constants) - 1;
}

static void vm_reset_stack() {
  vm.sp = vm.stack;
}

void vm_init() {
  vm_reset_stack();
}

void free_vm(VM *vm) {
}

static void vm_stack_push(Datum d) {
  *vm.sp = d;
  ++vm.sp;
}

static Datum vm_stack_pop(void) {
  --vm.sp;
  return *vm.sp;
}

static inline uint8_t read_insn(void) {
  uint8_t insn = *vm.ip;
  ++vm.ip;
  return insn;
}

static inline Datum read_constant(void) {
  uint8_t byte = read_insn();
  return vector_get(vm.chunk->constants, byte);
}

static Result run() {
  while (1) {
    uint8_t insn = read_insn();
    switch (insn) {
    case OP_CONSTANT: {
      vm_stack_push(read_constant());
      break;
    }
    case OP_RETURN: {
      Result res;
      res.status = StatusOK;
      return res;
    }
    default:
      printf("%s: unrecognized opcode: %d\n", __FUNCTION__, insn);
      exit(1);
    }
  }
}

Result interpret(CodeChunk *chunk) {
  vm.chunk = chunk;
  vm.ip = chunk->code;
  return run();
}
