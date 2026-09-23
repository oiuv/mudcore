# percent / percent_of

```c
mixed percent(mixed num, mixed den);
mixed percent_of(mixed percentage, mixed base);
```

参数应为 `int` 或 `float`：`percent` 计算 `num * 100 / den`，`percent_of` 计算 `percentage * base / 100`。

两个参数均为整数时返回整数，小数部分按 LPC 整数除法截断；任一参数为浮点数时返回浮点数。`percent` 的分母不能为零。

```c
percent(1, 8);        // 12
percent(1, 8.0);      // 12.5
percent_of(25, 50);   // 12
percent_of(25.0, 50); // 12.5
```
