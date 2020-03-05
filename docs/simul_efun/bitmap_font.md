
点阵字体显示系统，可以在游戏中显示点阵文字。

### 核心方法

```c
varargs string bitmap_font(string str, int size, string fill, string bg, string fcolor, string bgcolor);
```

> 参数说明

    str     要显示的文字
    size    字体大小，限制为12、14、16，如果省略，默认为12
    fill    填充字符，如果省略默认为`88`
    bg      背景字符，如果省略默认为`  `
    fcolor  字符前景颜色，默认为空
    bgcolor 字符背景颜色，默认为空
