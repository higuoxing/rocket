add_executable(rocket_regress rocket_regress.c)

add_custom_target(
  check
  COMMAND
    ${CMAKE_COMMAND} -E env "PATH=${CMAKE_CURRENT_BINARY_DIR}/../src:$ENV{PATH}"
      ${CMAKE_CURRENT_BINARY_DIR}/rocket_regress --base-dir=${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS rsi rocket_regress
)
