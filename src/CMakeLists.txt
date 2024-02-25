add_executable(rsi main.c vector.c tokenizer.c parser.c ast.c vm.c)
target_link_libraries(rsi readline)

add_executable(tokenizer_test tokenizer_test.c vector.c tokenizer.c)
add_executable(vector_test vector_test.c vector.c)
add_executable(vm_test vm_test.c vm.c vector.c)
