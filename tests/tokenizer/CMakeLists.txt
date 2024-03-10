file(GLOB TESTING_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/*.scm)

foreach(SCRIPT ${TESTING_SCRIPTS})
  get_filename_component(SCRIPT_NAME ${SCRIPT} NAME)
  add_custom_target(
    check_tokenizer_${SCRIPT_NAME}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../../src/rsi --dump-tokens=${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NAME}.out ${SCRIPT}
    COMMAND diff --color -u ${SCRIPT}.expected ${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NAME}.out
  )
  list(APPEND CHECK_TOKENIZER_TARGETS check_tokenizer_${SCRIPT_NAME})
endforeach()
add_custom_target(check_tokenizer DEPENDS rsi ${CHECK_TOKENIZER_TARGETS})
