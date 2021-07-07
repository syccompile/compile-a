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

add_executable(personal_test test/personal_test.cpp)
target_link_libraries(personal_test pthread)
add_dependencies(personal_test ${COMPILER})

add_executable(functional_test test/functional_test.cpp)
target_link_libraries(functional_test pthread)
add_dependencies(functional_test ${COMPILER})

add_executable(performance_test test/performance_test.cpp)
target_link_libraries(performance_test pthread)
add_dependencies(performance_test ${COMPILER})

add_executable(file_test test/file_test.cpp)
add_executable(sim test/sim.cpp)
