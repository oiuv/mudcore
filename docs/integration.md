# 框架接入与扩展

mudcore 可以作为新 MUD 的基础，也可以为已有 MUD 提供部分模块。下文的“宿主”指使用框架的游戏项目；除非另有说明，LPC 路径都相对宿主 mudlib 根目录。

## 选择接入方式

| 场景 | 建议方式 | 需要验证的内容 |
| --- | --- | --- |
| 新项目 | 从 minimud 模板开始，使用默认 master、simul efun、登录和玩家对象 | 启动、注册登录、基础命令、存档与重连 |
| 已有 MUD 按需集成 | 保留原有 master、登录和安全策略，只接入所需模块及其依赖 | 宏冲突、simul efun 重名、UID 与已有存档兼容 |
| 替换框架默认实现 | 在宿主定义别名，继承 `CORE_*` 并覆盖扩展点 | 默认行为、覆盖后的行为及组合模块中的调用 |

基本模板是 [mudcore/mud](https://github.com/mudcore/mud)，完整游戏示例是 [mudcore/mymud](https://github.com/mudcore/mymud)。模板负责提供最小宿主，驱动安装和运维由使用方完成。

## 新项目的最小接入

已有模板可直接沿用相应文件。手工创建宿主时，至少准备以下结构：

```text
项目根目录/
├── config.cfg
├── include/globals.h
├── system/kernel/master.c
├── system/kernel/simul_efun.c
├── data/
├── log/
└── mudcore/
```

驱动配置中的关键路径如下；监听端口等运行参数由宿主选择：

```ini
mudlib directory : .
log directory : /log
include directories : /include:/mudcore/include
global include file : <globals.h>
master file : /system/kernel/master
simulated efun file : /system/kernel/simul_efun
```

在宿主根目录启动驱动，使 `mudlib directory : .` 指向正确位置。可参考 [精简配置](../config.mini.ini) 与 [完整配置示例](../config.example.ini)；其中的端口、网页目录等应与自己的项目一致。

宿主 `include/globals.h` 最后包含框架头文件：

```c
// 宿主自己的宏与别名放在这里。
#include <mudcore.h>
```

两个入口文件分别继承框架实现：

```c
// system/kernel/master.c
inherit CORE_MASTER_OB;
```

```c
// system/kernel/simul_efun.c
inherit CORE_SIMUL_EFUN_OB;
```

框架默认使用 `/data/`、`/log/`，不要求为空项目另建 `.env` 或 `config.json`。默认登录先完成账号认证，再创建玩家对象；角色出生点取 `START_ROOM`，未定义时使用 `VOID_OB`。

对外开放注册前，由运维先注册并保管管理员账号。管理员 ID 取 `WIZARD`，默认是 `mudren`；模板不会替运维预先创建账号或设置密码。默认 master 的文件和 Socket 权限较宽松，正式游戏应按自身权限模型覆盖相应检查。

## 旧 MUD 按需集成

在运行配置中加入 `/mudcore/include`，然后检查宿主全局头文件与框架的公共宏，再将 `<mudcore.h>` 放在宿主宏定义之后。不必为了使用一个模块替换已有 master 或登录系统。

单个模块可能调用框架 simul efun、依赖其他组件或 daemon；“可按需使用”不等于“每个文件都能脱离其他文件独立运行”。接入前核对模块的 `inherit`、公共宏和调用接口，为需要保留的宿主实现设置别名，并检查函数重名。尤其不要把默认框架的 UID、数据库授权或存档路径直接套用到已有游戏。

升级已有项目时，另读 [兼容迁移说明](dependency-boundary.md) 与 [升级验证流程](maintenance.md)。

## 用别名扩展框架

`CORE_*` 指向框架原始实现；`_DBASE`、`_ROOM` 等组件别名，以及 `ENV_D`、`CHAR_D`、`LOGIN_OB` 等服务或对象别名可由宿主提前定义。以属性组件为例：

```c
// 宿主 include/globals.h
#define _DBASE "/inherit/project_dbase"
#include <mudcore.h>
```

```c
// 宿主 inherit/project_dbase.c
inherit CORE_DBASE;

string query_project_name() {
    return "示例游戏";
}
```

定制文件继承 `CORE_DBASE`，不要继承已经指向自己的 `_DBASE`。组合组件使用对应别名时会采用宿主实现，例如 `CORE_ROOM` 已组合 `_DBASE`、`_NAME` 和 `_CLEAN_UP`，无需再次重复继承。

| 定制目标 | 入口 | 约定 |
| --- | --- | --- |
| 初次创建角色 | `CHAR_D->init_player(user, loginObject)` | 默认登录在新角色创建时调用 |
| 每次激活玩家 | `CHAR_D->setup(user)` | 默认 `USER_OB->setup()` 调用；覆盖玩家方法时保留需要的父类流程 |
| 登录提示 | `MOTD` | 指向宿主欢迎文本 |
| 登录与玩家对象 | `LOGIN_OB`、`USER_OB`、`LOGIN_D` | 默认登录会检查实际对象路径、交互状态与身份，见 [登录说明](daemons/login_d.md) |
| 普通与管理员命令 | `CMD_PATH_STD`、`CMD_PATH_WIZ` | 指令实现 `main(object me, string arg)`；查找和别名见 [命令说明](daemons/command_d.md) |
| 预加载 | `PRELOAD` | 指向文本清单，一行一个对象或目录；目录只扫描一层 |
| 出生点 | `START_ROOM` | 使用无扩展名对象路径 |

框架的玩法示例可供参考，具体战斗、成长、剧情与业务适配由宿主实现。

## 显式选择最小玩家组合

默认 `_USER` 仍使用完整玩家组合，已有宿主不必更改。非战斗或非 RPG 游戏可选择 `_USER_BASE`，保留默认 `USER_OB` 的登录、文件存档与重连生命周期。它不继承战斗、状态、组队、任务、战斗记录或 GMCP，但仍需要基础组件、登录/命令服务与宿主配置；依赖见 [模块契约](module-contracts.md)。

```c
// 宿主 include/globals.h；在 <mudcore.h> 之前定义。
#define _USER _USER_BASE
#define _COMMAND "/inherit/project_command"
#define MUDCORE_ENABLE_PARSER 0
#define CHAR_D "/system/project_char"
#define START_ROOM "/world/start"
#define CMD_PATH_STD ({ "/cmds/basic/" })
#define CMD_PATH_WIZ ({ "/cmds/admin/" })
#include <mudcore.h>
```

`/inherit/project_command.c` 只选择方向和 action：

```c
inherit CORE_COMMAND;

protected string *query_command_handlers() {
    return ({ "exit", "command" });
}
```

`/system/project_char.c` 继承 `CORE_CHAR_D`，覆盖 `init_player(object user, mixed *data...)`，仅设置自己的属性；不调用父初始化即可不写入经验、等级、HP，见 [属性示例](daemons/char_d.md#非-rpg-属性与调用时序)。名称/性别可保持默认，或让 `LOGIN_D` 指向继承 `CORE_LOGIN_D` 的宿主策略，见 [登录示例](daemons/login_d.md#角色创建策略钩子)。宿主 daemon 路径还须满足自己的 master UID 规则。

准备真实 `/world/start` 房间，提供 `/cmds/basic/` 中的 `look`、`go` 等命令，接口为 `main(object me, string arg)`。不要不加检查地复制依赖战斗/队伍的整套游戏命令。默认移动对交互 living 玩家会执行 `look`，方向阶段会调用 `go`；虽然关闭了表情/频道/parser，`COMMAND_D` 别名与命令索引仍保留。

最小宿主不预加载 `VERB_D`、谓词或未选组件；`MUDCORE_ENABLE_PARSER 0` 同时阻止默认登录 daemon 重载谓词。仍使用 parser 的游戏保持默认 `1` 并提供驱动及谓词能力。编译选项变动后重新编译受影响对象或重启；不要在活跃玩家上临时改继承组合。

完整可运行示例由 [contracts.mjs](../tests/contracts.mjs) 在临时目录组装；[globals](../tests/contracts/lpc/globals.h)、[command](../tests/contracts/lpc/command.lpc)、[char](../tests/contracts/lpc/char.lpc)、[room](../tests/contracts/lpc/room.lpc) 与 [命令实现](../tests/contracts/lpc/contract.lpc) 展示配套接口。测试版 master、宽松权限、状态输出和关闭驱动命令只用于隔离测试，不得直接部署。运行：

```sh
node mudcore/tests/contracts.mjs /absolute/path/to/driver
```

独立检出去掉命令中的 `mudcore/` 前缀。测试实际省略未选组件，再进行本机注册、登录、命令、移动、保存恢复和重连，不需要当前游戏的数据。

## 检查宿主权限来源

管理对象可继承 `_HOST_POLICY`，显式检查已加载 master：

```c
inherit _HOST_POLICY;

mapping inspect_current_policy() {
    return inspect_host_policy(find_object(MASTER_OB));
}
```

返回逐项 `source/status`、`warnings` 与 `notice`，详见 [诊断契约](module-contracts.md#只读宿主策略诊断)。`host-defined` 只表示来自宿主实现，哪怕放行全部也会得到此状态；`unknown` 不是通过。组件不主动调用权限 apply、写文件、联网或切换身份，不更改原有授权。宿主仍须设计文件/网络/数据库 ACL 并测试实际拒绝路径。

## 区分三类配置

| 配置 | 使用者 | 作用 |
| --- | --- | --- |
| `config.cfg` / `config.ini` | FluffOS 驱动 | mudlib 路径、入口对象、端口、运行限制 |
| `include/globals.h` | LPC 预处理器 | 模块别名、目录、编译时功能选择 |
| `DATA_DIR + ".env"` 与 `/config.json` | 框架 simul efun / daemon | 宿主的可选业务参数 |

框架 `.env` 使用 `键: 值`，由 `ENV_D` 读取，格式与操作系统环境变量、常见 Python dotenv 的 `KEY=value` 不同。值会去除前后空白，`true`、`false` 和数字按解析规则转换；未配置时可保持文件不存在。

`env(key)` 读取参数，`env(key, value)` 修改当前 `ENV_D` 内存中的值，不自动写回 `.env`。`config(key)` 读取并缓存宿主根目录的 JSON 配置，也不提供文件写回功能。修改配置文件后，需要按宿主流程重载相应对象或重启；凭据与玩家数据不要提交到代码仓库。

## 源文件与驱动能力

框架生产源码默认保留 `.c`，测试文件可使用 `.lpc`。这避免单纯因扩展名使尚未升级驱动的旧 MUD 无法加载框架，不代表所有历史驱动都支持框架引用的 efun。宿主是否采用 `.lpc` 由宿主规范和实际驱动决定。

对象引用优先不写扩展名。框架的 `lpc_file()`、`lpc_source_files()` 支持 `.c` / `.lpc`，同名时优先 `.lpc` 并去重；实际编译 `.lpc` 仍需要驱动支持，不应同时维护同名的两种源文件。

按所选模块检查驱动能力：默认对象流程使用 UID 等基础能力；parser 谓词需要相应 parser 支持；数据库需要 DB 包及指定后端；Socket、TLS、外部命令分别需要对应 efun 与宿主授权。HTTPS 的证书验证边界见 [Socket 文档](Socket.md#框架-tls-客户端)。

[隔离回归测试](../tests/README.md) 编译全部框架程序，因此它的驱动要求高于仅接入少量模块的宿主；测试所需 Node.js 不是游戏运行依赖。
