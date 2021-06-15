# 待处理列表
* if语句与while语句的表达式子如果是可以在编译时计算的会出错，例如if(4+5>9)，见代码ir_translator.cc中的IfStmt::ir_translate()
* 目前只能识别并转换十进制数，见代码ast.cc中的Number::Number()
* 数组声明未能转换为中间代码
* 全局变量声明未能转换为中间代码
