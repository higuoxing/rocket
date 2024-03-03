#include <assert.h>

#include "common.h"
#include "symbol.h"
#include "vm.h"

int main() {
  SymbolTable *symbol_table = make_symbol_table();
  Object val;
  bool exists;
  val.type = OBJ_NUMBER;
  val.value = FloatGetDatum(1.24);
  symbol_table_add(symbol_table, "foo", val);
  assert(symbol_table_find(symbol_table, "foo", &exists).value == FloatGetDatum(1.24));
  assert(exists);
  assert(symbol_table_find(symbol_table, "bar", &exists).type == OBJ_NIL);
  assert(!exists);
  for (int i = 0; i < symbol_table_len(symbol_table); ++i)
    free(symbol_table_get(symbol_table, i).symbol_name);
  free_symbol_table(symbol_table);
}
