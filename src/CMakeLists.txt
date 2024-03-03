add_executable(rsi main.c vector.c tokenizer.c parser.c ast.c vm.c compiler.c)
target_link_libraries(rsi readline)

add_executable(tokenizer_test tokenizer_test.c vector.c tokenizer.c)
add_executable(vector_test vector_test.c vector.c)
add_executable(vm_test vm_test.c vm.c vector.c)

add_test(NAME TokenizerTest COMMAND tokenizer_test)
add_test(NAME VectorTest COMMAND vector_test)
add_test(NAME VMTest COMMAND vm_test)
