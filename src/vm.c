#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "vector.h"
#include "vm.h"

VECTOR_GENERATE_TYPE_NAME_IMPL(Object, ObjectsPool, objects_pool);
VECTOR_GENERATE_TYPE_NAME_IMPL(uint8_t, Instructions, instructions);

static inline Frame *vm_current_frame(VM *vm) {
  return &vm->frames[vm->frame_pointer];
}

void initialize_vm(VM* vm, Instructions *instructions, ObjectsPool *constants,
            ObjectsPool *globals) {
  assert(vm != NULL);
  assert(instructions);

  vm->stack_pointer = 0;
  vm->frame_pointer = 0;

  vm->globals = globals ? globals : make_objects_pool();
  vm->constants = constants ? constants : make_objects_pool();
  vm->heap = make_objects_pool();

  vm->frames[0].base_pointer = 0;
  vm->frames[0].fn = make_compiled_function(instructions, 0);
  vm->frames[0].ip = instructions_data(vm->frames[0].fn->instructions);
}

void destroy_vm(VM *vm) {
  free_objects_pool(vm->globals);
  free_objects_pool(vm->constants);
  free_objects_pool(vm->heap);
}

InterpretResult vm_run(VM *vm) {
  Frame *frame = vm_current_frame(vm);

  while (*frame->ip != OP_LAST) {
    switch (*frame->ip) {
    case OP_CONSTANT: {
      ++frame->ip;
      int constant_idx = *frame->ip;
      vm->stack[vm->stack_pointer++] =
          objects_pool_get(vm->constants, constant_idx);
      ++frame->ip;
      continue;
    }
    default: {
      fprintf(stderr, "%s: unrecoginzed operator %d", __FUNCTION__,
              (*frame->ip));
      exit(1);
    }
    }
  }

  return INTERPRET_OK;
}

CompiledFunction *make_compiled_function(Instructions *instructions,
                                         int num_locals) {
  CompiledFunction *compiled_fn = malloc(sizeof(CompiledFunction));
  compiled_fn->instructions = instructions;
  compiled_fn->num_locals = num_locals;
  return compiled_fn;
}

void free_compiled_function(CompiledFunction *fn) {
  // free_instructions(fn->instructions);
  free(fn);
}
