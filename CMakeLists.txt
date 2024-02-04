cmake_minimum_required(VERSION 3.28)

project(rocket C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(ROCKET_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(ROCKET_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(ROCKET_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

include_directories(${ROCKET_INSTALL_INCLUDE_DIR})

add_subdirectory(src)
# add_subdirectory(test)
