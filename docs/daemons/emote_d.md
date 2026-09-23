### CORE_EMOTE_D

表情功能守护进程，实现游戏表情功能，存档为 `DATA_DIR + "emote_d.o"`（默认 `/data/emote_d.o`）。

### 核心方法

```c
varargs int do_emote(object me, string verb, string arg, object *obs, string channel);
```

> 参数说明

    me          发送动作的对象
    verb        表情动词，如 `hi`
    arg         可选的目标 ID，在当前环境或在线玩家中查找
    obs         可选的接收对象数组，默认当前环境中的所有对象
    channel     可选的 sprintf 格式模板，包含一个 %s；不是频道名称

```c
// 设置表情：def 为两个字符串，依次是无目标和有目标时的动作模板
int set_emote(string pattern, string *def);

// 删除表情
int delete_emote(string pattern);

// 查询表情
string *query_emote(string pattern);

// 获取所有表情动词
string *query_all_emote();
```

如果不需要框架提供的守护进程，可以自行定义 `EMOTE_D` 覆盖， `EMOTE_D` 必须实现 `do_emote` 方法。
