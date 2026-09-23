# bit_set

> 源码：[bit.c](../../system/kernel/simul_efun/bit.c)

```c
int bit_set(int state, int pos);
```

返回将 `state` 第 `pos` 位设为 `1` 后的新值。位序从 `1` 开始，不修改传入的变量，需自行接收返回值。

```c
bit_set(0, 3);  // 4
```

调用方应保证 `pos` 在驱动整数有效位宽内；函数不检查越界位置。旧名 `bitSet()` 保留兼容，新代码使用 `bit_set()`。
