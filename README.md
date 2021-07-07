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