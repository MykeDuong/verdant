cmake_minimum_required(VERSION 3.10)

project(Verdant VERSION 0.1)

set(CMAKE_CXX_STANDARD, 14)
set(CMAKE_CXX_STANDARD_REQUIRED, True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CXXFLAGS  "-Wall -Werror -g")
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")

include_directories(include)
include_directories(template)

configure_file(./include/version.h.in version.h)

file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "src/main.cpp")
set(MAIN "src/main.cpp")
add_executable(verdant ${SOURCES} ${MAIN})

target_compile_definitions(verdant PUBLIC VERDANT_FLAG_DEBUG)
target_include_directories(verdant PUBLIC "${PROJECT_BINARY_DIR}")

set(BTREE_TEST "test/btree_test.cpp")
add_executable(btree_test ${SOURCES} ${BTREE_TEST})
add_test(NAME BtreeTest COMMAND btree_test)

target_compile_definitions(verdant PUBLIC VERDANT_FLAG_DEBUG)
target_include_directories(verdant PUBLIC "${PROJECT_BINARY_DIR}")
