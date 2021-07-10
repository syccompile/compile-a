# Compiler of SysY

## 简单使用

首先修改test.sh的权限：

```shell
chmod 755 test.sh
```

测试文件放在test目录下，运行以下命令进行测试：

```shell
./test.sh # default: personal_test
./test.sh personal_test
./test.sh functional_test
./test.sh performance_test
```

输出放在对应的output目录下。

## CMakeLists.txt文件说明

```cmake
cmake_minimum_required(VERSION 3.4.1)   # CMake版本
set(CMAKE_CXX_STANDARD 17)      # C++标准
set(CMAKE_CXX_FLAGS "-g")       # C++编译标志
project(compiler VERSION 0.1.0) # 项目名和版本

find_package(BISON) # 查找Bison
find_package(FLEX)  # 查找Flex
BISON_TARGET(Parser src/analyzer.y ${CMAKE_CURRENT_BINARY_DIR}/analyzer.cc)
# 上面一行使用bison处理src/analyzer.y，将生成文件放到${CMAKE_CURRENT_BINARY_DIR}下
# CMAKE_CURRENT_BINARY_DIR指的是存放生成二进制文件的目录，也即cmake-build-debug目录，当然你也可以修改这个目录
FLEX_TARGET(Scanner src/flex.l ${CMAKE_CURRENT_BINARY_DIR}/flex.cc)
# 使用flex处理src/flex.l，生成文件放到${CMAKE_CURRENT_BINARY_DIR}下
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(src)
# include_directories用于将目录下的文件添加到头文件搜索目录中
# 也就是说即使analyzer.h生成在cmake-build-debug目录下，
# 但是你引用里面的头文件时可以直接使用analyzer.h而不需要使用cmake-build-debug/analyzer.h

set(COMPILER ${PROJECT_NAME})   # 将变量COMPILER设置成项目的名字
file(GLOB srcs "src/*.cc" "src/context/*.cc" "src/ir_opt/*.cc")
# 将src目录下的*.cc文件,src/context/*.cc文件,src/ir_opt/*.cc文件放到变量srcs中

add_executable(${COMPILER}
        ${srcs}
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Scanner_OUTPUTS}
        )
# 利用srcs中存储的源文件和flex，bison的输出文件构建可执行文件compiler

add_executable(personal_test test/personal_test.cpp)
target_link_libraries(personal_test pthread)    # 这条语句可以删了，我并没有使用
add_dependencies(personal_test ${COMPILER}) # 添加依赖，personal_test运行前必须先构造好COMPILER
```