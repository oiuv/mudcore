> 源码：[array.c](../../system/kernel/simul_efun/array.c)

### 语法

```c
varargs void print_r(mixed *arr, int step);
```

### 描述

向当前玩家格式化输出数组及嵌套数组，用于开发调试。`step` 是可选的初始缩进层数，通常省略；此函数不返回字符串。

```c
print_r(({ 1, ({ 2, 3 }) }));
```
