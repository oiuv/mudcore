### CORE_CHANNEL_D

聊天频道守护进程，默认包括系统(sys)、公告(msg)、闲聊(chat)、交易(bill)、广播(shout)五个频道。

默认实现中，普通玩家不能使用系统和公告频道；交易频道要求等级达到 20 级；广播频道对普通玩家每次扣除 100 钱币（`coin`），管理员不受这两项限制。宿主可按游戏规则替换该实现。

### 核心方法

```c
varargs int do_channel(object me, string verb, string arg, int emote);
```

> 参数说明

    me      发送消息的对象
    verb    频道动作，如：chat 或 chat*
    arg     消息内容或表情名称，如 hi
    emote   可选，非零表示表情；verb 以 * 结尾也会启用表情模式

如果不需要框架提供的守护进程，可以自行定义 `CHANNEL_D` 覆盖， `CHANNEL_D` 必须实现 `do_channel` 方法。
