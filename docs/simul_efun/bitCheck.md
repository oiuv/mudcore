> 源码：[bit.c](../../system/kernel/simul_efun/bit.c)

### 语法

```c
int bitCheck(int state, int pos);
```

### 描述

检查 `state` 的第 `pos` 位。位置从 `1` 开始，对应最低位；未设置时返回 `0`，已设置时返回该位的掩码，**不保证为 `1`**。在条件中判断非零即可。

```c
bitCheck(4, 3);  // 4
bitCheck(4, 2);  // 0
```

调用方应保证 `pos` 在驱动整数的有效位宽内；函数不检查越界位置。
