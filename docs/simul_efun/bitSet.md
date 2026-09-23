> 源码：[bit.c](../../system/kernel/simul_efun/bit.c)

### 语法

```c
int bitSet(int state, int pos);
```

### 描述

返回将 `state` 第 `pos` 位设为 `1` 后的新值。位置从 `1` 开始，对应最低位；不会修改传入的变量，需要自行接收返回值。

```c
bitSet(0, 3);  // 4
```

调用方应保证 `pos` 在驱动整数的有效位宽内；函数不检查越界位置。
