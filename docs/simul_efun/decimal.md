# decimal：精确定点小数

沿用 FluffOS `testsuite/std/decimal.lpc`，在框架中保存为 `system/kernel/simul_efun/decimal.c`。以下函数随 simul efun 加载，可直接调用，无需额外继承或配置；以 64 位 LPC 整数运算，不依赖外部服务。

## 基本用法

```c
int *price, *total;

price = to_decimal("12.34");
total = decimal_mul(price, to_decimal(3));
decimal_to_string(total);  // "37.02"，price 不变

decimal_to_string(decimal_add(to_decimal("0.1"), to_decimal("0.2")));  // "0.3"
decimal_to_string(decimal_div(to_decimal(1), to_decimal(3)));  // "0.333333333333"
```

适用于需要精确小数的价格、手续费和比例计算。使用字符串或整数构造精确值；浮点输入已经可能存在误差，转为 decimal 无法恢复其原始精度。

## 接口

| 接口 | 返回与行为 |
| --- | --- |
| `int *to_decimal(mixed value)` | 接受十进制字符串、整数、浮点数或已有 decimal；始终返回新数组 |
| `int decimalp(mixed value)` | 判断是否为两个整数构成的数组；仅检查结构，不验证小数位范围 |
| `int *decimal_add(int *a, int *b)` | 加法 |
| `int *decimal_sub(int *a, int *b)` | 减法 |
| `int *decimal_mul(int *a, int *b)` | 乘法 |
| `int *decimal_div(int *a, int *b)` | 除法，截断而非四舍五入 |
| `int *decimal_mod(int *a, int *b)` | 取余，符号遵循 LPC 整数取余 |
| `int *decimal_neg(int *a)` | 取相反数 |
| `int decimal_cmp(int *a, int *b)` | 小于、等于、大于分别返回 `-1`、`0`、`1` |
| `int decimal_eq(int *a, int *b)` | 数值是否相等，不要求小数位数相同 |
| `int decimal_lt(int *a, int *b)` / `int decimal_gt(int *a, int *b)` | 小于 / 大于 |
| `string decimal_to_string(int *value)` | 转为十进制字符串，保留当前小数位数 |
| `int decimal_to_int(int *value)` | 转为整数，向零截断 |
| `float decimal_to_float(int *value)` | 转为浮点数，可能损失精度 |

## 精度与输入约定

- 内部表示为 `({ mant, scale })`，值为 `mant / 10^scale`；例如 `"12.34"` 为 `({ 1234, 2 })`。通过 `to_decimal()` 构造，不手工拼接或修改数组。计算结果均为新数组，不修改操作数。
- 这是定点库，不是任意精度库。有效数字受 64 位有符号整数范围限制，通常约 18 位；小数位数最多 18。对齐小数位、乘法或除法中的中间放大也受此范围限制，最终结果较小不代表一定能计算。
- 加减保留两者较大的小数位数；乘法相加小数位数，超出 18 位时仅能通过移除末尾零缩减，否则报错。
- 除法按 `max(a 的小数位数, b 的小数位数, 12)` 计算并向零截断，再移除结果末尾的小数零。例如 `5 / 2` 输出 `"2.5"`，`1 / 3` 输出 `"0.333333333333"`。没有自动四舍五入到金额位数的功能。
- 字符串支持正负号、小数点、前导空格/制表符与分组下划线（如 `"1_000.00"`）；不支持科学计数法、逗号或尾部空白。浮点输入先按 12 位小数格式化。
- 非法输入、超出支持的小数位数、除零和检测到的整数溢出会抛错；调用方按业务需要用 `catch()` 处理，不将异常当作零值。

## 来源与验证

移植自 FluffOS `testsuite/std/decimal.lpc`，保留公开接口和算法，仅适配框架注册、排版及变量声明位置。对应官方测试移植到 `tests/lpc/decimal.lpc`，由框架回归入口执行：

```sh
node tests/run.mjs <driver路径>
```

移植源码及测试保留 [FluffOS 原始版权声明](../../system/kernel/simul_efun/UPSTREAM-COPYRIGHT)，不因纳入 mudcore 而改授框架的 MIT 许可。
