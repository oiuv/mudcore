# base64encode / base64decode

对 UTF-8 字符串进行标准 Base64 编解码，不需要外部服务。

```c
string base64encode(string source);
string base64decode(string source);

base64encode("中文");       // "5Lit5paH"
base64decode("5Lit5paH");   // "中文"
```

- 空字符串编码、解码均返回空字符串。
- 解码允许空格、制表符、CR 和 LF；其余内容必须符合标准 Base64 字母表及末尾 `=` 填充规则。
- 长度错误、非法字符、错误填充、非零填充位或解码后不是合法 UTF-8 时会抛错，可用 `catch(...)` 处理。
- 这是文本接口，解码结果按 UTF-8 解释；不提供任意二进制 buffer 或 URL-safe Base64 接口。
