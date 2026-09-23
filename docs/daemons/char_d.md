### CORE_CHAR_D

`CHAR_D` 提供角色初始化和扩展钩子，源码见 [char_d.c](../../system/daemons/char_d.c)。

### 核心方法

```c
void init_player(object user, mixed *data...);
void update(object ob, mixed *data...);
void setup(object ob, mixed *data...);
```

| 方法 | 默认行为与调用时机 |
| --- | --- |
| `init_player()` | 默认 `LOGIN_D` 创建新角色时调用，并传入登录对象作为额外参数；设置生日、经验 `0`、等级 `1`、生命 `99`。 |
| `setup()` | 默认为空；框架 `USER_OB->setup()` 已调用，用于每次激活角色时的宿主扩展。 |
| `update()` | 默认为空；由宿主在需要更新角色数据时主动调用。 |

宿主可以继承 `CORE_CHAR_D` 并覆盖所需方法，再用 `CHAR_D` 别名指向它。已有默认实现无需重新写一遍；覆盖后是否调用父方法由游戏的初始化规则决定。若替换了玩家对象或登录流程，则由宿主保留相应调用。参见 [登录说明](login_d.md) 和 [接入指南](../integration.md)。

## 非 RPG 属性与调用时序

`init_player()` 仅用于新角色，不在存档恢复或在线对象重连时重复执行。新建/从存档加载后的 `USER_OB->setup()` 调用 `CHAR_D->setup()`；在线断线对象的 `reconnect()` 只恢复连接相关状态，不再次调用这两个钩子。

宿主不需要等级、经验或 HP 时覆盖 `init_player()`，不调用默认父方法，只设置自己的字段：

```c
inherit CORE_CHAR_D;

void init_player(object user, mixed *data...) {
    user->set("host_attribute", "kept");
}
```

配合 [最小玩家组合](../integration.md#显式选择最小玩家组合) 选择满足依赖的命令。框架不会自动补写被覆盖的 RPG 初始属性，也不会删除旧存档中的字段；旧角色迁移由宿主另外设计。[组合夹具](../../tests/contracts/lpc/char.lpc) 用计数器验证新建一次、重连不重复、重新加载调用 setup。
