> 源码：[charset.c](../../system/kernel/simul_efun/charset.c)

### 语法

```c
string unicode(int c);
```

### 描述

将 Unicode 码点转换为包含该字符的字符串，内部使用 `sprintf("%c", c)`。

```c
unicode(65);      // "A"
unicode(0x4e2d);  // "中"
```

旧名称 `itoc()` 已于 2021 年改为 `unicode()`，当前框架未保留旧名别名。
