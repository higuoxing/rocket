#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "vector.h"
#include "vm.h"

VECTOR_GENERATE_TYPE_NAME_IMPL(Value, ConstantPool, constant_pool);
VECTOR_GENERATE_TYPE_NAME_IMPL(uint8_t, Chunk, chunk);

static VM vm;

void write_byte(Chunk *chunk, uint8_t insn) {
  chunk_append(chunk, insn);
}

int add_constant(ConstantPool *constant_pool, Value val) {
  constant_pool_append(constant_pool, val);
  return constant_pool_len(constant_pool) - 1;
}

static void vm_reset_stack() {
  vm.sp = vm.stack;
}

void vm_init() {
  vm_reset_stack();
}

void free_vm(VM *vm) {
}

static void vm_stack_push(Value v) {
  *vm.sp = v;
  ++vm.sp;
}

static Value vm_stack_pop(void) {
  --vm.sp;
  return *vm.sp;
}

static inline uint8_t read_byte(void) {
  uint8_t insn = *vm.ip;
  ++vm.ip;
  return insn;
}

static inline Value read_constant(void) {
  uint8_t byte = read_byte();
  return constant_pool_get(vm.constant_pool, byte);
}

static Result run() {
  while (1) {
    uint8_t insn = read_byte();
    switch (insn) {
    case OP_CONSTANT: {
      vm_stack_push(read_constant());
      break;
    }
    case OP_PROC_CALL: {
      uint8_t arg_count = read_byte();
      break;
    }
    case OP_RETURN: {
      Result res;
      res.status = STATUS_OK;
      return res;
    }
    default:
      printf("%s: unrecognized opcode: %d\n", __FUNCTION__, insn);
      exit(1);
    }
  }
}

Result interpret(Chunk *chunk, ConstantPool *constant_pool) {
  vm.code = chunk;
  vm.constant_pool = constant_pool;
  vm.ip = chunk_data(vm.code);
  return run();
}

int compile(AstNode *expr) {
  switch (expr->kind) {
  case AST_BOOL:
  case AST_NUMBER:
  case AST_CHAR:
  default:
    fprintf(stderr, "%s: unrecognized expr kind: %d", __FUNCTION__, expr->kind);
    exit(1);
  }
}
