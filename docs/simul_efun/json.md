# json_encode / json_decode

```c
string json_encode(mixed value);
mixed json_decode(mixed text);
```

`json_encode` 将 LPC 数值、字符串、数组及 mapping 转为 JSON；中文和其他 Unicode 字符保留为可读的 UTF-8，引号、反斜杠及控制字符按 JSON 规则转义。

`json_decode` 接受 JSON 字符串或 UTF-8 buffer，返回对应的 LPC 值，不修改传入的 buffer。支持嵌套数组/对象、小数、指数和 `\uXXXX` 转义，包括合法的 UTF-16 代理对。

```c
mapping data;
string text;

data = json_decode("{\"name\":\"侠客\",\"level\":10}");
text = json_encode(([ "name": "侠客", "items": ({ "剑", "药" }) ]));
data = json_decode(string_encode(text, "UTF-8"));
```

沿用现有 LPC 映射规则：

- JSON `true` 解码为 `1`，`false`、`null` 解码为 `0`，三者不会保留独立类型；普通 LPC `0`、`1` 编码为数值。
- 为兼容既有调用，`json_decode(0)` 返回 `0`；空字符串不是合法 JSON，会抛错。
- mapping 的非字符串键会被跳过；重复的 JSON 对象键以最后一个值为准。
- 循环引用的内层引用编码为 `null`；重复使用同一个非循环数组或 mapping 不会被误判为循环。
- 对象、函数等无法表示的值编码为 `null`。数值范围受 LPC 的整数和浮点类型限制。

损坏的 UTF-8、非法转义、孤立代理项、未转义控制字符、不完整数字、尾随逗号或额外内容均会抛错，可用 `catch(...)` 处理。相比旧实现，对这些非法输入的校验更严格。
