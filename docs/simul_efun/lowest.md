> 源码：[lowest.c](../../system/kernel/simul_efun/lowest.c)

### 语法

```c
mixed lowest(mixed *numbers...);
```

### 描述

返回参数中的最小值，保留被选中数值的类型。至少传入一个参数，且每项只能是整数或浮点数，否则报错。

```c
lowest(1, 4, 0.5);  // 0.5
```
