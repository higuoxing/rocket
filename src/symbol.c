#include "symbol.h"
#include "vector.h"
#include "vm.h"
#include <string.h>

VECTOR_GENERATE_TYPE_NAME_IMPL(SymbolTableElement, SymbolTable, symbol_table);

void symbol_table_add(SymbolTable *sym_tab, const char *symbol_name,
                      const Object val) {
  int len = symbol_table_len(sym_tab);
  for (int i = 0; i < len; ++i) {
    char *name = symbol_table_get(sym_tab, i).symbol_name;
    if (strcmp(symbol_name, name) == 0) {
      SymbolTableElement ele = {
          .symbol_name = name,
          .val = val,
      };
      symbol_table_set(sym_tab, i, ele);
      return;
    }
  }
  SymbolTableElement ele = {
      .symbol_name = strdup(symbol_name),
      .val = val,
  };
  symbol_table_append(sym_tab, ele);
}

Object symbol_table_find(SymbolTable *sym_tab, const char *symbol_name,
                         bool *exists) {
  int len = symbol_table_len(sym_tab);
  Object val;
  *exists = false;
  for (int i = 0; i < len; ++i) {
    const char *name = symbol_table_get(sym_tab, i).symbol_name;
    if (strcmp(name, symbol_name) == 0) {
      *exists = true;
      return symbol_table_get(sym_tab, i).val;
    }
  }
  val.type = OBJ_NIL;
  return val;
}
