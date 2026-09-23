# present_clone

按来源文件查找容器的直属物品，不依赖物品的 `id()`。

```c
object present_clone(mixed source);
object present_clone(mixed source, int n);
object present_clone(mixed source, object env);
object present_clone(mixed source, object env, int n);
```

`source` 可以是来源对象，也可以是对象路径。路径可省略开头 `/`，允许 `.c`、`.lpc` 和克隆编号 `#123`；比较时使用无扩展名、无克隆编号的对象名。

`env` 默认为调用者对象。`n` 从 **1** 开始，省略时取第一个匹配物品；多个匹配按 `all_inventory(env)` 的顺序返回。找不到或匹配数量不足返回 `0`，不递归搜索容器内部，也不会为查找而加载来源文件。

```c
object first, second;

first = present_clone("/obj/coin", this_player());
second = present_clone("/obj/coin", this_player(), 2);
```

参数数量、类型或小于 1 的序号会抛错。传入克隆对象只指定来源，并不要求返回该克隆本身。此函数也可以匹配容器中的蓝图对象。
