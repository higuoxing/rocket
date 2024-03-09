#include <stdio.h>

#include "common.h"
#include "vector.h"
#include "vm.h"

int main() {
  VM vm;
  initialize_vm(&vm,make_instructions(), /*constants=*/NULL, /*globals=*/NULL);
  destroy_vm(&vm);
}
