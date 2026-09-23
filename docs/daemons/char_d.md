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
