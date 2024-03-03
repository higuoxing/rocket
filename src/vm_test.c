#include <stdio.h>

#include "common.h"
#include "vector.h"
#include "vm.h"

int main() {
  VM *vm = make_vm(make_instructions(), /*constants=*/NULL, /*globals=*/NULL);
  free_vm(vm);
}
