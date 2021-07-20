# IR规范

## IR格式

`<IR OpCode> <addr0> <addr1> <addr2>`

## IR地址类型定义

1. `VAR`：变量地址。显示前缀为`%`
2. `PARAM`：参数地址。显示前缀为`p`
3. `IMM`：立即数地址。显示前缀为`#`
4. `BRANCH_LABEL`：跳转标号。显示前缀为`.L`
5. `NAMED_LABEL`：具名标号。无显示前缀。

## IR操作码定义

### 单操作数指令

1. 标号指令 `LABEL`
  * 功能：设置一个跳转标号
  * `addr0`：`BRANCH_LABEL` 要设置的跳转标号编号

2. 跳转指令 `JMP`,`JLE`,`JLT`,`JGE`,`JGT`,`JE`,`JNE`
  * 功能：根据对应的判断结果跳转
  * `addr0`：`BRANCH_LABEL` 跳转到的跳转标号

3. 函数调用指令 `CALL`
  * 功能：执行函数调用
  * `addr0`：`NAMED_LABEL` 函数名称

4. 函数返回指令 `RET`
  * 功能：函数返回
  * `addr0`：`VAR`/`IMM`/`PARAM` 返回值

5. 标号值指令`LABVAL`
  * 功能：定义全局变量标号值
  * `addr0`：`NAMED_LABEL` 对应全局标号

### 双操作数指令

1. 数据移动指令 `MOV`,`MOVLE`,`MOVLT`,`MOVGE`,`MOVGT`,`MOVEQ`,`MOVNE`
  * 功能：根据对应的判断结果决定是否执行拷贝
  * `addr0`：`VAR`               目的寄存器
  * `addr1`：`VAR`/`IMM`/`PARAM` 源寄存器

2. 比较指令 `CMP`
  * 功能：执行比较操作
  * `addr0`：为空
  * `addr1`：`VAR`/`IMM`/`PARAM` 左操作数
  * `addr2`：`VAR`/`IMM`/`PARAM` 右操作数

3. 在栈中分配空间 `ALLOC_IN_STACK`
  * 功能：**为局部数组**在栈帧中分配内存
  * `addr0`：`VAR` 数组变量编号
  * `addr1`：`IMM` 大小
  * 注：大小以4字节为单位

4. 函数定义 `FUNCDEF`
  * 功能：函数定义
  * `addr0`：`NAMED_LABEL` 函数名
  * `addr1`：`IMM` 参数个数

5. 逻辑“非”指令
  * 功能：指定对应算术逻辑运算
  * `addr0`：`VAR`               目的寄存器
  * `addr1`：`VAR`/`IMM`/`PARAM`/`NAMED_LABEL` 左操作数

6. 传参指令 `PARAM`
  * 功能：执行函数传参，若干个连续的PARAM之后将紧跟一个CALL
  * 说明：相当于一个MOV指令，但是不可懒复制
  * `addr0`：`PARAM`             参数位置
  * `addr1`：`VAR`/`IMM`/`PARAM` 要传入的参数数值地址


### 三操作数指令

1. 算术逻辑指令 `ADD`,`SUB`,`MUL`,`DIV`,`MOD`,`AND`,`OR`,`XOR`
  * 功能：执行对应的算术逻辑运算
  * `addr0`：`VAR`               目的寄存器
  * `addr1`：`VAR`/`IMM`/`PARAM`/`NAMED_LABEL` 左操作数
  * `addr2`：`VAR`/`IMM`/`PARAM`/`NAMED_LABEL` 右操作数

2. 取数指令 `LOAD`
  * 功能：变址寻址，取数（针对数组）
  * `addr0`：`VAR`               目的寄存器
  * `addr1`：`VAR`/`NAMED_LABEL` 基址
  * `addr2`：`VAR`/`IMM`/`PARAM`/`NAMED_LABEL` 变址
  * 注：变址单位为4字节

3. 存数指令 `STORE`
  * 功能：变址寻址，存数（针对数组）
  * `addr0`：`VAR`/`NAMED_LABEL` 基址
  * `addr1`：`VAR`/`PARAM`/`IMM` 变址
  * `addr2`：`VAR`               源寄存器
  * 注：变址单位为4字节

# notes:
* LOAD/STORE 指令只针对数组存取数，不涉及单一变量的内存交换
* 所有算术逻辑指令、移动指令的目的寄存器保证是VAR型
* 

