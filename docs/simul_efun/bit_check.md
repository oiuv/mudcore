# bit_check

> 源码：[bit.c](../../system/kernel/simul_efun/bit.c)

```c
int bit_check(int state, int pos);
```

检查 `state` 的第 `pos` 位。位序从 `1` 开始，对应最低位；未设置返回 `0`，已设置返回该位的掩码，不保证为 `1`。

```c
bit_check(4, 3);  // 4
bit_check(4, 2);  // 0
```

调用方应保证 `pos` 在驱动整数有效位宽内；函数不检查越界位置。旧名 `bitCheck()` 保留兼容，新代码使用 `bit_check()`。
