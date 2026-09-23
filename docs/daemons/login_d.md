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

## 角色创建策略钩子

继承 `CORE_LOGIN_D` 并通过 `LOGIN_D` 别名选择，可覆盖以下 protected 方法。它们只控制角色策略，不改变账号 ID、密码、保留身份和连接交接检查：

```c
protected string query_name_prompt();
protected string validate_character_name(string name);
protected mapping *query_gender_options();
protected string query_gender_prompt();
```

名称合法返回 `0`，不合法返回可显示的错误文本（包含需要的换行）。默认仍要求中文、既有长度和禁用词规则，并由 `NAME_D` 查重；覆盖验证后不会再额外执行默认规则，需要查重时自行保留 `NAME_D->invalid_new_name(name)`。

性别返回有序 mapping 数组，每项为 `([ "key": "m", "label": "男性", "value": "男性" ])`。默认保持 m/f 和男性/女性。key/label/value 必须是非空字符串；拒绝重复（不区分大小写）、key 前后空白和单字符前缀冲突。单字符 key 保留旧的首字符匹配方式，多字符 key 精确匹配，均忽略大小写。默认提示由选项生成；宿主可独立改提示。

返回 `({})` 表示省略性别步骤，不设置默认 `gender` 字段。空输入继续等待，无效输入重试；畸形配置抛出 `LOGIN:` 错误，未经过内部收尾不会建体。策略应在一次登录中保持稳定。

最小覆盖示例（非中文名称和自定义选项）：

```c
inherit CORE_LOGIN_D;

protected string query_name_prompt() { return "Character name: "; }

protected string validate_character_name(string name) {
    if (!stringp(name) || !sizeof(regexp(({ name }), "^[A-Z][A-Za-z]+$")))
        return "REJECT_NAME\n";
    return NAME_D->invalid_new_name(name);
}

protected mapping *query_gender_options() {
    return ({ ([ "key": "pilot", "label": "驾驶员", "value": "pilot-role" ]) });
}
```

这是机制示例，不是推荐游戏的性别分类；不用该字段时返回空数组即可。对应可执行覆盖见 [组合登录夹具](../../tests/contracts/lpc/login.lpc)。

内部创建收尾再次验证登录对象及私有认证状态，再调用 `make_body()`、`init_new_player()`、`enter_world()`；不能从外部调用策略或伪造临时属性获得身份。失败清理连接/玩家对象、恢复 daemon 身份；这不是跨多个存档和姓名索引的原子事务。初始属性仍交给 `CHAR_D`；保存/重连时不重复调用新角色初始化。

`MUDCORE_ENABLE_PARSER` 默认为 `1`，登录 daemon 创建时会重载 `VERB_D`；宿主在全局头文件设为 `0` 可关闭该依赖。编译选项或 daemon 别名改变后按宿主流程重编译/重启，不支持登录中途热切换认证策略。
