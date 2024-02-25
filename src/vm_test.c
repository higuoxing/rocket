#include <stdio.h>

#include "common.h"
#include "vector.h"
#include "vm.h"

int disassemble_instruction(Chunk *chunk, ConstantPool *constant_pool,
                            int offset) {
  printf("%04d ", offset);
  uint8_t instr = chunk_get(chunk, offset);

  switch (instr) {
  case OP_CONSTANT: {
    printf("OP_CONSTANT %.2f\n",
           DatumGetFloat(
               constant_pool_get(constant_pool, chunk_get(chunk, offset + 1))
                   .value));
    return offset + 2;
  }
  case OP_RETURN: {
    printf("OP_RETURN\n");
    return offset + 1;
  }
  default: {
    printf("unrecognized opcode: %d", instr);
    exit(1);
  }
  }
  return 0;
}

void disassemble_chunk(Chunk *chunk, ConstantPool *constant_pool,
                       const char *name) {
  int offset = 0;
  printf("== %s ==\n", name);

  while (offset < chunk->len) {
    offset = disassemble_instruction(chunk, constant_pool, offset);
  }
}

int main() {
  Chunk *chunk = make_chunk();
  ConstantPool *constant_pool = make_constant_pool();
  Value constant = {
      .type = VAL_NUMBER,
      .value = FloatGetDatum(1.20),
  };

  int off = add_constant(constant_pool, constant);
  write_byte(chunk, OP_CONSTANT);
  write_byte(chunk, off);
  write_byte(chunk, OP_RETURN);
  disassemble_chunk(chunk, constant_pool, "test_chunk");
  vm_init();
  interpret(chunk, constant_pool);
  free_chunk(chunk);
  return 0;
}
