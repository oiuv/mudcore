# 公共模块契约

本文随 mudcore 独立维护，接入者无需读取上层仓库的规划文件。架构边界见 [架构规范](architecture.md)，组件索引见 [inherit](inherit.md)。表中 `X` 对应原始宏 `CORE_X` 和可覆盖别名 `_X`；源路径相对框架根目录，实际对象路径省略 `.c`。宿主先定义别名，最后包含 `<mudcore.h>`。

## 公共组件依赖矩阵

“基础”表示 UTF-8 FluffOS 对象/属性/文件等 efun，不等于不需要宿主接口。使用框架 simul efun，或为所选接口提供等价实现；表中列出关键的非 efun 协作。可选项指对应路径被调用时才需要，不是任意缺失都自动容错。

| 模块 / 源路径 | 必需组件、服务或宿主接口 | 可选协作 | 驱动能力 | 生命周期与宿主责任 |
| --- | --- | --- | --- | --- |
| ACTION / `inherit/action.c` | 行动计数及回调 | 宿主 busy/interrupt 闭包 | 基础、函数指针 | 宿主心跳调用 `continue_action()`，定义行动规则 |
| AREA / `inherit/area/area.c` | `_CLEAN_UP/_DBASE/_NAME/_SAVE`、`AREA_MAP`、区域地图/样式 | 地图 NPC 任务提示调用 `QUEST_D` | 基础 | 设置尺寸、地形及区域坐标；使用区域接口进出，详见 [区域地图](inherit/area.md) |
| ATTACK / `inherit/attack.c` | 属性、队伍/敌人接口、`COMBAT_D`、`msg()` 等 | 宿主战斗规则 | 基础 | 维护敌人状态并调度战斗；不适用于未提供这些接口的最小玩家 |
| CAMP / `inherit/camp.c` | 属性、`CAMP_D`、阵营对象与 `msg()` | `CAMP_DIR` 宿主内容 | 基础 | 宿主定义阵营及关系，组件不生成游戏内容 |
| CLEAN_UP / `inherit/clean_up.c` | 对象及物品的 `query()` | `no_clean_up`、日志目录 | 文件 | 驱动调用 `clean_up()`；宿主决定保留条件 |
| CMD / `inherit/CMD.c` | `external_cmd_N` 驱动配置、回调 | 宿主指定可执行文件 | external、Socket | 显式 `external_cmd()`；启动失败抛错，输出缓冲在 `response()` 前清理；不预置业务命令 |
| COMMAND / `inherit/command.c` | 属性/临时属性、`COMMAND_D`（别名及 action 查找） | 选中的 EMOTE/CHANNEL/parser 阶段 | commands；parser 按选项 | 先设置 ID/名称，再 `enable_living()`；见下文 |
| CONDITION_MOD / `inherit/condition_mod.c` | 状态定义、宿主作用对象、`msg()` 等 | 自定义开始/结束/心跳回调 | 基础 | 定义状态属性及效果；不独立调度 |
| CONDITION / `inherit/condition.c` | 属性、状态文件的持续时间及回调接口 | 具体增益/减益效果 | 基础 | 宿主心跳更新；完整默认玩家提供调度 |
| DB / `inherit/DB.c` | DB 后端、连接参数、master 授权；默认配置经 `env()` | MySQL/SQLite 等宿主选择 | DB 包及后端 | 显式连接、查询与关闭，详见 [DB](inherit/DB.md) |
| DBASE / `inherit/dbase.c` | 无 daemon | 默认属性对象 | 基础 | 管理持久/临时 mapping；不自行保存 |
| DBSAVE / `inherit/dbsave.c` | `DBASE_D`、`save_dbase_data/receive_dbase_data` | 无 | 文件存档，不是 SQL | 宿主显式恢复/保存；按对象原型路径分组 |
| HTTP / `inherit/Http.c` | Socket efun、权限、编码/JSON 辅助 | TLS、宿主响应钩子 | Socket；HTTPS 需 TLS | 显式请求、取消、超时/完成回调，见 [HTTP](Http.md)；不继承 `_SOCKET` |
| HOST_POLICY / `inherit/host_policy.c` | 已加载的待检查对象、`function_exists` | 无 | 对象元信息 | 按需检查；不启动服务、不修改 ACL |
| LIVING / `inherit/living.c` | `_ACTION/_ATTACK/_COMMAND/_CONDITION/_DBASE/_MESSAGE/_MOVE/_NAME/_SAVE/_TEAM` | 继承组件的协作者 | 所选组件能力 | 既有完整生物组合；宿主提供激活/心跳/存档策略 |
| MESSAGE / `inherit/message.c` | ANSI/字符串辅助、输出配置 | 分页与宿主输入结束钩子 | 输出；分页需交互/input_to | 接收消息；分页时需有效当前玩家与连接 |
| MOVE / `inherit/move.c` | 属性接口、目标环境/区域接口 | `gmcp()`、`is_db_saved()` | 对象移动、commands | 移动后按条件 look；清理见下文 |
| NAME / `inherit/name.c` | 属性、`is_user()` 等角色标志及字符串辅助 | 额外描述 | 基础 | 设置显示名/ID；提供 parser 名称列表本身不调用 parser efun |
| NOCLONE / `inherit/noclone.c` | 对象状态；出生点路径需 `query/move` 等 | 角色提示 | 基础 | 显式调用 `check_clone()`，仅继承不会阻止克隆 |
| ROOM / `inherit/room.c` | `_CLEAN_UP/_DBASE/_NAME`；刷新物品需其 `move()` | `query_light()` 路径调用 `NATURE_D` | 基础 | 设置出口/描述/物品；驱动 reset/clean_up，见 [房间](inherit/room.md) |
| SAVE / `inherit/save.c` | `query_save_file()`、`assure_file()` | 无 | save_object/restore_object、文件授权 | 显式调用，路径及数据兼容由宿主负责 |
| SOCKET / `inherit/Socket.c` | Socket efun、master 授权 | TLS、宿主回调 | Socket；TLS 按选用 | 显式建连/监听/发送/关闭，见 [Socket](Socket.md) |
| TEAM / `inherit/team.c` | 队友组队接口、跟随时的命令/移动能力 | 宿主组队规则 | 基础、commands | 维护临时队伍与跟随，离队/销毁由宿主协调 |
| UNIQUE / `inherit/unique.c` | 属性、替代物原型 | 替代品配置 | children/new/destruct | 显式检查唯一性/创建替代品；不是禁止克隆 |
| USER_COMBAT_RECORD / `inherit/user_combat_record.c` | 无 daemon | 宿主战斗记录调用 | 基础 | 数据随持久字段保存，不自行记录战斗或存档 |
| USER_GMCP / `inherit/user_gmcp.c` | GMCP/JSON；GUI/地图配置经 `env()` | MSP、客户端支持 | GMCP efun | 显式初始化/通知；组件不自行请求运营服务 |
| USER_QUEST / `inherit/user_quest.c` | 任务对象 `is_quest/get_required_kills/get_required_items`、`QUEST_SIZE` | 宿主奖励/任务服务 | 基础 | 管理接受/完成记录，不自动调度或发奖 |
| USER / `inherit/user.c` | `_LIVING/_USER_COMBAT_RECORD/_USER_GMCP/_USER_QUEST` | 上述组件的协作者 | 所选组件能力 | 保留既有完整玩家组合；连接由 `USER_OB` 管理 |
| USER_BASE / `inherit/user_base.c` | `_ACTION/_COMMAND/_DBASE/_MESSAGE/_MOVE/_NAME/_SAVE` | 移动的可选协作 | 所选命令阶段能力 | 显式选择最小组合；不是单独连接对象 |
| VERB / `inherit/verb.c` | parser efun、谓词规则/回调 | 同义词 | parser | 创建时注册规则；关闭 parser 后不要预加载该组件/谓词 |
| VRM / `inherit/vrm.c` | 房间模板/入口出口、`lpc_file()`；虚拟加载需 `VIRTUAL_D` 路径路由 | 怪物、`.map` 输出 | 基础、虚拟对象/文件 | 配置迷宫再按坐标生成；移除清理房间，见 [虚拟对象](daemons/virtual_d.md) |

TUI 是额外组件组，入口与依赖在 `<tui.h>`，不由 `_USER_BASE` 自动携带。`TUI_PRINT`、`TUI_TERMINAL`、`TUI_APP` 的驱动能力、玩家组合、输入接管、退出和断线清理以 [TUI 手册](TUI.md) 为准。Intermud 是 daemon 而非继承组件，按 [显式启停契约](dependency-boundary.md#迁移调整) 接入。

## 属性与两种存档

- DBASE 的 `set/query/add/delete` 与临时属性版本接受斜杠分隔的属性路径。`query(prop, raw)` 默认会求值函数属性；`raw` 取原值。`query_entire_dbase()` 等暴露当前 mapping，不保证防御性复制；需要区分不存在与零值时使用 `undefinedp()`。`tmp_dbase` 是 `nosave`；继承 DBASE 不会自动恢复或保存。
- SAVE 的 `int save()` / `int restore()` 调用宿主 `query_save_file()`；非字符串路径返回 `0`，有效路径返回驱动 save/restore 的结果，驱动权限或 I/O 异常可传播。保存前 `assure_file()` 准备目录；不提供事务、迁移或自动重试。默认玩家按合法 ID 存储。
- DBSAVE 的 `int save()` 将 `save_dbase_data()` 交给 `DBASE_D`；`int restore()` 把 `query_data()` 交给 `receive_dbase_data(mixed)`，宿主需接受首次无数据的 `0`。两方法固定返回 `1`，不能据此证明磁盘持久化成功。键是 `base_name(previous_object())`，同原型克隆共享数据，不是玩家级隔离存档；删除/清理行为见 [DBASE_D](daemons/dbase_d.md)。

不要在同一对象中无规划地混用 SAVE 与 DBSAVE 的同名方法。持久字段结构与恢复时机属于宿主契约，本轮不转换既有存档。

## 模拟函数与原生 efun

[decimal](simul_efun/decimal.md) 随框架 simul efun 注册，提供基于 64 位 LPC 整数的定点小数运算；无外部服务、宿主方法或初始化/清理要求。输入应由 `to_decimal()` 构造；运算返回新数组，精度、范围限制及异常约定见该文档。

覆盖 efun 仅用于增强功能，原生参数、返回值、查找顺序、权限和错误行为必须保留。当前框架覆盖 `present` 和 `notify_fail`：

- `present()` 在普通对象中委托原生 efun；隐式查找先背包，再环境本身和环境内对象。对象参数省略环境时返回其父对象，显式指定环境时返回对象本身。序号、非法参数和隐藏对象权限以驱动为准；AREA 的同格筛选见 [区域地图](inherit/area.md)。
- 原生查找通过绑定给调用者的 efun 闭包执行，避免使用 simul efun 自身的隐藏权限。宿主自定义 `valid_bind()` 时需允许框架 simul efun 完成该绑定；框架默认 Root 策略已允许。不应通过放宽所有对象的绑定权限解决接入问题。
- `notify_fail()` 接受字符串或函数，先交给驱动处理；函数保持延迟执行，不预先求值。当前玩家提供 `set_temp()` 时，额外记录原参数到 `notify_fail` 临时属性；该属性可能是函数，不能一律当作字符串拼接。

核对顺序为本地 `fluffos/docs/efun/` → 有歧义时查 `fluffos/src/` 及测试；没有本地驱动源码时由 [官方文档索引](https://www.fluffos.info/llms.txt) 进入对应文档，再按需查官方源码。维护规则见 [AGENTS.md](../AGENTS.md#efun-兼容原则)。

## 命令

`process_input(string)` 仍通过 `COMMAND_D->default_alias()` 处理输入；禁用 action 阶段并不会自动移除该别名依赖。`nomask int command_hook(string arg)` 接收当前命令参数，verb 来自驱动 `query_verb()`；不是任意对象可以调用的通用命令执行 API。

`protected string *query_command_handlers()` 返回已知且不重复的阶段名；默认 `exit → command → emote → channel → parser`。五个钩子均为 `protected mixed handle_*(object actor, string verb, string arg)`，具体名称、返回规则及示例见 [命令手册](daemons/command_d.md#可选择的处理阶段)。

`nomask void enable_living()` 在首次激活前验证配置，随后注册命令；无效阶段不会留下 living 状态。只有选择 parser 才调用 `parse_init()`。`nomask void disable_living(string type)` 清除命令状态，参数保留兼容。阶段列表不是逐条命令动态重读；重新配置需按宿主停用/重编译流程处理。

`MUDCORE_ENABLE_PARSER` 默认 `1`，`MUDCORE_HAS_PARSER` 是内部能力检测结果，不应由宿主伪造。关闭时命令/登录不引用 parser efun，不加载 `VERB_D`；`master->refresh_parser()` 被显式调用时报告不可用。默认启用但驱动缺少所需 efun 时明确报错。显式选择 parser 不可用的组合也报错；这不保证直接加载 `_VERB` 在裁剪驱动上可用。

## 移动与房间

`varargs int move(mixed dest, int raw)` 接受对象或对象路径，调用驱动移动及 init，正常返回 `1`，对象被 init 销毁时返回 `-1`。目标必须提供对应环境契约；区域出入需要区域坐标与 `move_out()`。交互玩家若有 `gmcp()` 则发送 `Room.Info.Get`，没有时跳过；`raw == 0` 且玩家 living 时执行 `look`。选择的命令需满足自身依赖。

`move_or_destruct(object dest)` 默认把玩家移到 `VOID_OB`，非玩家若提供且启用 `is_db_saved()` 则保存，返回 `1`；参数不改变默认玩家去向。`varargs void remove(string euid)` 调用可选 DB 存档及区域离开处理，本身不执行 `destruct()`。缺少可选协作可跳过；协作存在而抛错、必需属性/区域调用失败都传播，不保证移动事务回滚。

ROOM 已组合属性、命名与清理。设置 `exits` 后，实际离开仍由宿主选择的移动命令及 `valid_leave(object, string)` 规则处理；直接 `move()` 不是出口授权检查器。`reset()` 按 `objects` 配置补充对象并移动进房间；`query_light()` 引入天气服务依赖。完整字段和方法见 [房间接口](inherit/room.md)。

## 玩家、登录与属性策略

`_USER` 默认仍指向完整 `CORE_USER`。`CORE_USER_BASE` 提供基础属性/消息/移动/命令/行动/文件存档和 `is_living()`，不包含战斗、状态、队伍、任务、战斗记录或 GMCP。默认 `USER_OB` 继承 `_USER`，为两种组合提供同一连接生命周期；接入样例见 [最小玩家组合](integration.md#显式选择最小玩家组合)。

默认时序：认证成功 → `make_body()` 按宿主 UID 权限建体 → 新角色 `CHAR_D->init_player()` 或旧角色恢复 → `enter_world()` 转移连接、`USER_OB->setup()`、保存和入场。`setup()` 激活命令/心跳并调用 `CHAR_D->setup()`；已有在线对象重连走 `reconnect()`，不再次创建或调用这两个初始化钩子。断线对象保留有期限，保存时机见登录/玩家源码；不要假定继承 MOVE 的 `remove()` 会保存普通文件存档。

名称提示/校验和性别选项可局部覆盖，详情见 [登录策略](daemons/login_d.md#角色创建策略钩子)。名称合法返回 `0`，非法返回错误字符串。性别选项含非空字符串 `key/label/value`，空数组表示不询问、不写入默认性别。内部收尾重新验证连接与认证；失败销毁未完成对象并恢复身份，再由 `NAME_D->assure_map_name(name)` 校验离线存档并 `save()` 姓名索引：未保存则释放姓名，已保存则保留占用，原始异常继续传播。不开放绕过认证的角色工厂，也不提供跨账号文件/姓名字典的事务回滚。

初始属性由 [CHAR_D](daemons/char_d.md) 控制；覆盖时不调用默认 `init_player()` 即可不写入等级/经验/HP。最小组合不会替宿主删掉存档中已有字段。选择与重连均有真实 Telnet 测试，不只检查继承列表。

## 只读宿主策略诊断

`mapping inspect_host_policy(object masterObject)` 由 `_HOST_POLICY` 提供，参数必须是已加载对象，否则抛错；不会为了检查自动加载 master。结果包含 `checks`（以方法名为键的 mapping）、`warnings`（字符串数组）、`notice`（局限性说明）。

检查 `valid_read`、`valid_write`、`valid_socket`、`valid_seteuid`、`valid_bind`、`valid_database`。每项 `source` 为归一化程序路径或 `0`；`status` 为 `framework-default`、`host-defined` 或 `unknown`。框架默认的读写/Socket 给出宽松策略提醒；无法确认来源给出 unknown 提醒。

只调用驱动 `function_exists` 读取实现来源，不调用权限 apply，不试写文件/连接网络/访问数据库/切换 UID，不自动运行。没有“安全通过”字段；宿主自己写的放行全部实现同样只标成 host-defined。它不分析函数内容或 shadow，不能代替宿主 ACL 设计、调用授权测试和安全审计。
