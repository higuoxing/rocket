add_executable(rsi main.c vector.c tokenizer.c parser.c ast.c)
target_link_libraries(rsi readline)

add_executable(tokenizer_test tokenizer_test.c vector.c tokenizer.c)
