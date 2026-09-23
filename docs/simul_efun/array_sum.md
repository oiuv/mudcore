> 源码：[sum.c](../../system/kernel/simul_efun/sum.c)

### 语法

```c
mixed array_sum(mixed *numbers);
```

### 描述

返回数组中所有数值之和，元素只能是整数或浮点数。空数组返回 `0`；含浮点数时返回浮点数。

```c
array_sum(({ 1, 2, 3 }));  // 6
array_sum(({}));          // 0
```
