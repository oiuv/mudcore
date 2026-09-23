> 源码：[charset.c](../../system/kernel/simul_efun/charset.c)

### 语法

```c
int atoi(mixed str);
```

### 描述

调用驱动的 `to_int()` 转为整数，常用于玩家指令中的数字参数；支持的输入类型与转换规则由 `to_int()` 决定。

```c
atoi("123");  // 123
atoi(12.8);   // 12
```
