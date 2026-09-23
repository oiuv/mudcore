> 源码：[sum.c](../../system/kernel/simul_efun/sum.c)

### 语法

```c
mixed sum(mixed *numbers...);
```

### 描述

返回所有参数之和。参数只能是整数或浮点数；全部为整数时返回整数，含浮点数时返回浮点数，不传参数返回 `0`。

```c
sum(1, 2, 3);  // 6
sum(1, 2.5);   // 3.5
```
