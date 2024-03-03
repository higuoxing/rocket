#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "vector.h"
#include "vm.h"

typedef struct SymbolTableElement {
  char *symbol_name;
  Object val;
} SymbolTableElement;

VECTOR_GENERATE_TYPE_NAME(SymbolTableElement, SymbolTable, symbol_table);

void symbol_table_add(SymbolTable *sym_tab, const char *symbol_name,
                      const Object val);
Object symbol_table_find(SymbolTable *sym_tab, const char *symbol_name,
                         bool *exists);

#endif
