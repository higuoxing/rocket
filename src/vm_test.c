#include <stdio.h>

#include "common.h"
#include "vector.h"
#include "vm.h"

int disassemble_instruction(CodeChunk *chunk, int offset) {
  printf("%04d ", offset);
  uint8_t instr = chunk->code[offset];

  switch (instr) {
  case OP_CONSTANT: {
    printf(
        "OP_CONSTANT %.2f\n",
        DatumGetFloat(vector_get(chunk->constants, chunk->code[offset + 1])));
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

void disassemble_chunk(CodeChunk *chunk, const char *name) {
  int offset = 0;
  printf("== %s ==\n", name);

  while (offset < chunk->len) {
    offset = disassemble_instruction(chunk, offset);
  }
}

int main() {
  CodeChunk *chunk = make_chunk();
  int constant = add_constant(chunk, FloatGetDatum(1.2));
  write_byte(chunk, OP_CONSTANT);
  write_byte(chunk, constant);
  write_byte(chunk, OP_RETURN);
  disassemble_chunk(chunk, "test_chunk");
  vm_init();
  interpret(chunk);
  free_chunk(chunk);
  return 0;
}
