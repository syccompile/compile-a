cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-g")
project(compiler VERSION 0.1.0)

find_package(BISON)
find_package(FLEX)
BISON_TARGET(Parser src/analyzer.y ${CMAKE_CURRENT_BINARY_DIR}/analyzer.cc)
FLEX_TARGET(Scanner src/flex.l ${CMAKE_CURRENT_BINARY_DIR}/flex.cc)
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(src)

set(COMPILER ${PROJECT_NAME})
file(GLOB srcs "src/*.cc" "src/context/*.cc" "src/ir_opt/*.cc")

add_executable(${COMPILER}
        ${srcs}
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Scanner_OUTPUTS}
        )

add_executable(test test/test.cpp)
target_link_libraries(test pthread)
add_dependencies(test ${COMPILER})

#set(TEST_TXT test/input/example.txt)
#enable_testing()
#add_test(NAME test
#        COMMAND zsh -c "${CMAKE_CURRENT_BINARY_DIR}/${COMPILER} < ${TEST_TXT}"
#        )
