cmake_minimum_required(VERSION 3.28)

project(rocket C)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(ROCKET_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(ROCKET_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(ROCKET_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

include_directories(${ROCKET_INSTALL_INCLUDE_DIR})

enable_testing()
add_subdirectory(src)
# add_subdirectory(test)
