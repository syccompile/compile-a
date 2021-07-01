# IR规范

## IR格式

`<IR OpCode> <addr0> <addr1> <addr2>`

## IR地址类型定义
    enum Kind: int { VAR, PARAM, IMM, BRANCH_LABEL, NAMED_LABEL } kind;

1. 变量地址   VAR
2. 参数地址   PARAM
3. 立即数地址 IMM
4. 跳转标号   BRANCH_LABEL
5. 具名标号   NAMED_LABEL

## IR操作码定义

### 单操作数指令

1. 标号指令 LABEL
功能：设置一个跳转标号
addr0：BRANCH_LABEL 要设置的跳转标号编号

2. 跳转指令 JMP,JLE,JLT,JGE,JGT,JE,JNE
功能：根据对应的判断结果跳转
addr0：BRANCH_LABEL 跳转到的跳转标号

3. 传参指令 PARAM
功能：执行函数传参，若干个连续的PARAM之后将紧跟一个CALL
addr0：VAR/IMM 要传入的参数地址

4. 函数调用指令 CALL
功能：执行函数调用
addr0：NAMED_LABEL 函数名称

5. 函数返回指令 RET
功能：函数返回
addr0：VAR/IMM 返回值

### 双操作数指令

1. 数据移动指令 MOV,MOVLE,MOVLT,MOVGE,MOVGT,MOVEQ,MOVNE
功能：根据对应的判断结果决定是否执行拷贝
addr0：VAR     目的寄存器
addr1：VAR/IMM 源寄存器

2. 比较指令 CMP
功能：执行比较操作
addr0：VAR/IMM 左操作数
addr1：VAR/IMM 右操作数

3. 在栈中分配空间 ALLOC_IN_STACK
功能：**为数组**在栈帧中分配内存
addr0：VAR 数组变量编号
addr1：IMM 大小（以字节计算）

### 三操作数指令

1. 算术指令 ADD,SUB,MUL,DIV,MOD
功能：执行对应的算术运算
addr0：VAR     目的寄存器
addr1：VAR/IMM 左操作数
addr2：VAR/IMM 右操作数

2. 取数指令 LOAD
功能：变址寻址，取数
addr0：VAR             目的寄存器
addr1：VAR/NAMED_LABEL 基址
addr2：VAR/IMM         变址

3. 存数指令 STORE
功能：变址寻址，存数
addr0：VAR             目的寄存器
addr1：VAR/NAMED_LABEL 基址
addr2：VAR/IMM         变址

# 待处理列表
* 全局变量赋值问题
* 局部数组
  - 赋值
  - IR地址添加
  - IR分配栈空间指令

