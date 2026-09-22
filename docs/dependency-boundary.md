# 框架能力与业务依赖边界

本文记录 `2.0.0-dev` 开发线相对旧实现的兼容调整。初次接入见 [接入指南](integration.md)，升级和正式发布步骤见 [版本管理](maintenance.md)。

mudcore 提供通用机制，由 MUDLIB 决定使用哪些模块、连接什么服务、何时调用。支持网络或数据库不等于依赖某个运营服务；不能通过删掉通用能力实现解耦。

## 保留的接口

| 能力 | 框架入口 | 使用方责任 |
| --- | --- | --- |
| TCP、UDP、TLS | `CORE_SOCKET` / `_SOCKET` | 地址、端口、回调和访问权限 |
| HTTP、HTTPS | `CORE_HTTP` / `_HTTP` | URL、请求数据、凭据和响应处理 |
| 数据库 | `CORE_DB` / `_DB` | 数据库类型、连接参数和权限；SQLite 可纯本地使用 |
| 外部命令 | `CORE_CMD` / `_CMD` | 驱动配置中的可执行文件白名单及参数 |
| MUD 互联 | `CORE_INTERMUD_D` / `INTERMUD_D` | 对端地址、端口及显式启停 |

HTTP 的 `get`、`post`、`head`、`ws` 入口保留。`ws` 仍仅构造升级请求，不代表实现完整 WebSocket 帧协议。HTTP 回调沿用原始数据块接口，新增按请求编号区分的完成、失败及取消接口；分帧与超时由客户端管理，业务仍需解析响应内容，详见 [HTTP 接口](Http.md)。

框架没有预设远程数据库或凭据。数据库可选驱动包、Socket/TLS 驱动支持及 MUDLIB 安全策略仍是对应功能的正常运行条件，不应称为“完全没有运行依赖”。

## 迁移调整

- `cmds/player/qrcode.c` 调用了固定二维码网站，不属于通用协议层，已移出框架。需要此业务的 MUDLIB 应在自己的命令目录维护实现；当前老 MUD 已有 `cmds/usr/qrcode.c`，保留不动。
- 不再定义固定 `MUDLIST_SERVER`。Intermud 加载仅恢复本地列表；有 `ROOT_UID` 权限的宿主启动对象读取自己的配置后调用 `INTERMUD_D->start(host, port)`，接受启动请求时返回 `1`，不表示对端已连通。第三参数可指定本地 UDP 端口，省略或 `0` 使用 `MUD_PORT + 4`。已启动时返回 `1`；换对端需先 `stop()`。`is_started()` 表示本地 socket 已建立（包括等待解析）。`query_startup_status()` 提供 `state/host/address/port/error`；状态为 `stopped/resolving/running/failed`，解析失败或默认 30 秒解析超时会释放 socket。绑定与发送错误记录到 `log/intermud/error.log`。停用、重启后到达的旧解析结果不会复活连接。
- 频道不再默认寻找 `/world/npc/ivy`。需要机器人时，在全局头文件中定义宿主自己的 `ROBOT_NPC` 对象路径。
- 配置模板保留 Linux/Windows 的 curl 路径作为注释示例，默认不启用。使用 `CORE_CMD` 的 MUDLIB 按部署环境取消对应注释、调整实际路径，并确保 `external_cmd_N` 的编号与调用一致。旧 MUD 自己已有的运行配置不变。
- 默认 master 限制非 Root 对象提升 euid 及跨 UID 的函数绑定。若宿主需要特定授权，应覆盖相应检查；框架其他默认读写权限较宽松，生产部署仍需宿主安全策略。

## 默认登录与数据库权限

默认登录辅助接口只为已认证的交互登录导出普通玩家 UID，拒绝保留身份及外部伪造的 `make_body/enter_world/reconnect`。`NAME_D` 的离线姓名查重保留受限入口，只能读取正常玩家 ID；宿主 master 的 `valid_seteuid()` 拒绝时会清理对象并恢复 daemon 的身份。使用自定义登录流程的宿主应在自己的登录模块实现授权，不能再把默认 `make_body()` 当作通用身份工厂。

默认 `valid_database()` 仅允许 UID/EUID 均为 Root 的服务对象。SQLite 还需显式配置 `DB_SQLITE_DATABASE`，路径必须完全匹配，连接参数使用空 host/user，返回 `1` 代表允许无密码访问。其他后端要求 database/host/user 匹配 `DB_DATABASE/DB_HOST/DB_USERNAME`，且 `DB_PASSWORD` 为字符串（空字符串代表明确配置的空密码）。不配置即不允许连接。

宿主可覆盖 `valid_database(caller, operation, info)` 对自己的 DAO 对象做白名单授权。FluffOS 的连接 `info` 为 `({ database, host, user })`，不含后端类型；需要限制类型的宿主应让指定 DAO 固定使用相应驱动常量（SQLite 为 `__USE_SQLITE3__`），并在 master 限制该 DAO 和数据库路径，不能假称此 apply 可以检查驱动未传入的类型。数据库组件仅对 MySQL 执行字符集初始化，不再向 SQLite 发送 MySQL 语句。

数据库构造器现在统一编码值，原生 SQL 可用 `sql(sqlText, params)` 的匿名 `?` 参数；这不是驱动原生预处理接口。`groupBy()` / `having()` 开始生成实际 SQL，数组条件不再覆盖前面的筛选。字段及运算符改为受限结构输入，复杂表达式迁移到受控 SQL 模板；整数、文本和 `undefined` 分别保留数值、文本、NULL 语义。`setConnection()` 切换目标会关闭旧连接并清空查询。完整示例和兼容边界见 [数据库接口](inherit/DB.md)。

`read_lines()` 保留有效行的前后空白，只去掉 CRLF 的行末 CR；过滤空行与缩进注释时才使用裁剪后的文本。预加载在解析路径时单独 `trim()`，公告等普通文本调用方无需为了本次修复自行补回缩进。

## 兼容与验证

公共 `CORE_*` 宏及原有继承路径继续可用；不要求旧 MUD 改成整套框架。为兼容旧驱动，框架源码默认使用 `.c`，仅测试代码可使用 `.lpc`；扫描同名对象时优先 `.lpc`，但项目不应同时维护同名双扩展文件。`env()`、中文转换和时间辅助函数尊重宿主 daemon 别名覆盖。

运行 [隔离测试](../tests/README.md) 后，还应在实际宿主验证受影响的登录、存取和命令流程。自动测试不会请求短信、机器人、二维码等真实外部服务。
