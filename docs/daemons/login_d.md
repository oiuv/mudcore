# 登录与角色身份

`LOGIN_D` 默认指向 `CORE_LOGIN_D`（`/mudcore/system/daemons/login_d`），配合 `LOGIN_OB`、`USER_OB`、`NAME_D` 和 `CHAR_D` 完成注册、登录与重连。已有 MUD 可以保留自己的登录系统。

## 默认流程

1. 驱动通过 master 创建 `LOGIN_OB`，连接对象调用自己的 `login()`，再进入 `LOGIN_D->login(ob)`。
2. 登录 daemon 读取 ID；已有账号校验密码，新账号进入注册及角色创建流程。
3. 认证通过后创建 `USER_OB`，导出该玩家的 UID，恢复或初始化角色。
4. `enter_world()` 转移连接、激活玩家、保存信息并进入出生点；在线角色断线后的再次登录进入重连流程。

账号 ID 使用小写英文字母，默认至少三位，拒绝 Root、Backbone 对应的保留身份。默认账号存档在 `DATA_DIR + "login/"`，角色存档在 `DATA_DIR + "user/"`，由相应对象的 `query_save_file()` 决定。

## 调用约束

| 方法 | 使用约定 |
| --- | --- |
| `login(object ob)` | 由交互状态的 `LOGIN_OB` 克隆自身调用；不能让其他对象代替任意登录对象发起认证 |
| `check_password(string str, string password)` | `nomask` 密码验证辅助方法，返回是否匹配，不设置已认证状态 |
| `make_body(object ob)` | 默认认证流程内部创建玩家；仅为可信 `NAME_D` 保留受限离线查重入口，不是通用对象工厂 |
| `enter_world(object ob, object user)` | 仅允许内部流程转移已认证连接；检查玩家路径、ID 和 UID |
| `reconnect(object ob, object user)` | 仅允许内部流程把已认证连接交还对应玩家 |

认证状态保存在 daemon 的私有 mapping 中。仅修改登录对象的 `id` 或临时属性不能获得认证，也不能通过外部调用辅助接口导出任意身份。宿主 master 拒绝 UID 切换时，默认创建流程清理失败对象并恢复 daemon 的身份。

`NAME_D` 的离线查重例外要求受信任 daemon 的 UID/EUID，并限制正常玩家 ID；该例外不赋予进入世界或接管在线连接的能力。权限变化见 [迁移说明](../dependency-boundary.md#默认登录与数据库权限)。

## 宿主扩展

在宿主 `globals.h` 中、包含 `<mudcore.h>` 之前定义需要替换的 `LOGIN_OB`、`USER_OB`、`LOGIN_D` 或 `CHAR_D`。默认实现按这些别名检查对象路径，定制对象与宏必须对应。

优先使用较小的扩展点：

- `MOTD`：欢迎文本路径。
- `START_ROOM`：出生点，未定义时使用 `VOID_OB`。
- `CHAR_D->init_player(user, loginObject)`：新角色创建时初始化属性。
- `CHAR_D->setup(user)`：默认玩家对象激活时调用。
- `WIZARD`：管理员 ID，默认 `mudren`；由运维在开放注册前完成账号初始化。

需要继承登录 daemon 时，以下是部分 `protected` 扩展方法的当前签名：

```c
protected void welcome(object ob);
protected void signin(object ob);
protected void get_id(string arg, object ob);
protected void get_passwd(string pass, object ob);
protected void init_new_player(object user, object ob);
```

这些方法属于认证流程的一部分。改变认证机制时，应由宿主实现完整的授权与连接交接，不能通过外部调用默认 `make_body()`、`enter_world()` 或 `reconnect()` 绕过检查。单纯扩展初始属性时优先使用 `CHAR_D`，无需改写认证流程。

## 集成验证

使用测试账号检查注册、错误密码、正常登录、退出再登录、断线重连及姓名查重，同时核对 UID/EUID 和存档路径。隔离测试已包含默认与宿主 UID 策略覆盖场景；实际游戏的自定义登录仍需单独验证，见 [测试说明](../../tests/README.md)。
