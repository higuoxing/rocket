add_executable(rsi main.c vector.c tokenizer.c parser.c ast.c vm.c compiler.c)
target_link_libraries(rsi readline)

add_executable(vector_test vector_test.c vector.c)
add_executable(vm_test vm_test.c vm.c vector.c)
add_executable(symbol_test symbol_test.c symbol.c vector.c)

add_test(NAME VectorTest COMMAND vector_test)
add_test(NAME SymbolTest COMMAND symbol_test)
add_test(NAME VMTest COMMAND vm_test)
