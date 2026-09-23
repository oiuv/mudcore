> 源码：[range.c](../../system/kernel/simul_efun/range.c)

### 语法

```c
mixed range(mixed lower, mixed upper, mixed value);
```

### 描述

将数值限制在指定范围内。按 `lower <= upper` 传入下限和上限：低于下限返回下限，高于上限返回上限，否则返回原值。三个参数都必须是整数或浮点数。

```c
range(0, 100, 120);  // 100
range(0, 100, -5);   // 0
range(0, 100, 50);   // 50
```
