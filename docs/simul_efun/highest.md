> 源码：[highest.c](../../system/kernel/simul_efun/highest.c)

### 语法

```c
mixed highest(mixed *numbers...);
```

### 描述

返回参数中的最大值，保留被选中数值的类型。至少传入一个参数，且每项只能是整数或浮点数，否则报错。

```c
highest(1, 4, 2.5);  // 4
```
