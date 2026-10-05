# 命令查找与别名

`COMMAND_D` 默认指向 `CORE_COMMAND_D`，管理传统 action 指令。parser 谓词由 `VERB_D` 管理，两者不是同一份索引。

## 目录与查找

`CMD_PATH_STD` 为普通命令目录，`CMD_PATH_WIZ` 为管理员命令目录。默认分别使用框架的 `cmds/player/` 和 `cmds/wizard/`；宿主可在包含 `<mudcore.h>` 前覆盖。

普通玩家按 `CMD_PATH_STD` 顺序查找，管理员在其后追加 `CMD_PATH_WIZ`。需要宿主同名命令优先时，将宿主目录放在对应数组的前面。目录扫描只处理本层 `.c` / `.lpc`，忽略备份等文件，同名优先 `.lpc`。

命令对象实现 `int main(object me, string arg)`，帮助可实现 `int help(object me)`。新增、删除、改名或修改文件别名后调用 `COMMAND_D->rehash()` 重建索引；这不会自动重编译所有已加载命令，源码修改仍需宿主自己的更新流程。

## 公共方法

| 方法 | 作用 |
| --- | --- |
| `default_alias(string verb)` | 展开命令行首词别名；空输入返回空字符串 |
| `query_default_alias()` | 返回当前命令行别名 mapping |
| `set_alias(mapping aliases)` | 替换命令行别名表 |
| `add_alias(mapping aliases)` | 增加或覆盖命令行别名 |
| `rehash()` | 清空并重新扫描命令文件与文件别名 |
| `query_commands()` | 返回按目录组织的命令索引 |
| `find_command(string verb)` | 按当前玩家权限寻找并加载命令对象，未找到返回 `0` |

## 两种别名

命令行别名可以包含参数，例如默认 `n` 展开为 `go north`，并保留原命令行的后续参数。宿主可在自己的初始化流程中调用 `add_alias()`；这些修改只作用于当前 daemon，重载后需重新设置。

文件别名使用同目录的 `名称.alias` 文件，第一行写目标命令名，例如 `l.alias` 中写 `look`，也接受 `look.c` 或 `look.lpc`。它引用同目录扫描出的命令，不能把任意外部路径或带参数的命令行放进去。空文件或不存在的目标会被忽略。

## 与谓词重载的区别

`VERB_D->rehash()` 按批次构建新索引，完成后替换；构建期间旧索引仍可用，过期重载任务失效。加载失败的谓词记录错误并从新索引排除，其他有效谓词继续保留。宿主应分别验证 action 命令和实际启用的 parser 谓词。

## 可选择的处理阶段

`COMMAND_D` 负责查找；继承组件 `_COMMAND` 负责调度。保留 `nomask command_hook(string arg)`、`enable_living()`、`disable_living(string type)`，宿主通过 protected 钩子定制，无需复制整个入口：

| 阶段 | 钩子（返回 `mixed`，参数均为 `object actor, string verb, string arg`） | 默认依赖 |
| --- | --- | --- |
| `exit` | `handle_exit` | 环境的出口属性及 `go` 命令 |
| `command` | `handle_action_command` | `COMMAND_D->find_command()` 和命令 `main()` |
| `emote` | `handle_emote` | `EMOTE_D` |
| `channel` | `handle_channel` | `CHANNEL_D` |
| `parser` | `handle_parser` | `parse_sentence()` 与谓词规则 |

`protected string *query_command_handlers()` 默认按表格顺序返回五阶段；返回子集可关闭阶段，调整顺序可改变优先级。激活前拒绝未知、重复阶段及不可用 parser。钩子返回 `0` 继续，`1` 成功短路，错误字符串通过 `notify_fail()` 停止；其他返回值报错。异常传播，不当成普通未处理。默认钩子归一化旧 service/parser 返回值，保持默认分派行为。

仅保留方向与 action 的宿主组件：

```c
inherit CORE_COMMAND;

protected string *query_command_handlers() {
    return ({ "exit", "command" });
}
```

用 `_COMMAND` 指向该文件。未选择表情/频道不访问对应 daemon；选择后缺失或出错会暴露错误。`process_input()` 仍依赖 `COMMAND_D` 的别名处理。阶段列表在激活时保存，不承诺运行中热切换。

在宿主 `<mudcore.h>` 之前定义 `MUDCORE_ENABLE_PARSER 0`，命令与登录才会整体跳过 parser 初始化和 `VERB_D` 重载；仅删掉阶段不等于关闭登录的谓词加载。选项默认 `1`，关闭后不要在预加载/命令中主动使用谓词服务；显式选择 parser 将报错。完整配置见 [最小组合](../integration.md#显式选择最小玩家组合)。

## 内置命令的对象与文件边界

以下说明针对框架内置命令，不替代宿主同名命令。管理员命令仍使用既有 `wizardp()` 检查，文件访问仍受宿主 UID/EUID 和 master 策略控制。

### 单对象更新：update

`update /world/example` 支持 `.c`、`.lpc`、无扩展名和虚拟蓝图路径；`update here` 使用当前环境的完整身份。显式扩展名沿驱动语义选择源码，不把 `.c` 自动换成 `.lpc`。

- 克隆路径（含 `#编号`）直接拒绝，不静默改成更新其蓝图；没有环境的 `here` 也拒绝。
- `VOID_OB` 的等价路径均受保护，含扩展名、重复斜杠和点段；不能更新临时安置内容的安全环境。
- 销毁旧环境前，将直接内容通过各自 `move(VOID_OB, 1)` 安置，核验实际位置；对象须提供 `move()`。安全环境不可用或移出失败时保留旧环境，不靠销毁回调清空内容。
- 重载成功后仅迁回仍存活、仍留在本次安全地点的对象；玩家字段、携带物品及物品实例不重建。钩子已转移走的对象不强行拉回。
- 重载/回迁失败记录 `LOG_DIR "update"`，存活且仍在安全地点的内容留待处理。宿主钩子已造成的销毁或转移不能回滚；旧程序一经销毁，编译失败也不能恢复它。

虚拟路径更新只重新请求该虚拟蓝图，不自动重载提供程序、父类或其他实例。这不是继承链重载器，也不保证业务状态的原子热更新。宿主应在维护窗口使用并核对自己的移动钩子。

### 查询与批量加载：which / loadall

`which` 先查 `COMMAND_D`，仅在 `MUDCORE_HAS_PARSER` 为真且普通命令未命中时查 `VERB_D`。关闭 parser 时可物理移除谓词服务；启用时兼容宿主 `get_verb` / `getVerb`，真实服务异常仍传播，不伪装成未找到。

`loadall /world/` 逐目录加载 `.c` / `.lpc`，同名优先 `.lpc`；`updateall` 只是它的别名。已驻留对象不会被销毁或强制重新编译。每个目录分别报告成功/失败数，异常或未返回对象都计失败并记录 `LOG_DIR "loadall"`；本层结果不代表异步子目录已经完成。`main()` 返回命令已处理状态，不代表全树成功。

继续跳过点开头的目录，以及 `CORE_DIR` 内的 `docs/`、`tests/`、`system/kernel/master/`、`system/kernel/simul_efun/`；宿主同名目录不因此排除。保留 `skip_load_dir` / `skipLoadDir` 的宿主覆盖。

### 检查与文本工具

- `sa` 分别显示运行期对象身份和实体源码；克隆保留 `#编号`，源码查找去除该编号。虚拟区域标为没有同名独立源码，不伪造 `.c`，不为显示信息加载其提供程序。
- `all_inventory`、`call_other`、`variables` 支持原生实体/虚拟对象加载。失败就结束当前操作，不对空对象调用业务方法；详细加载错误记录 `LOG_DIR "command"`。`variables here` 没有环境时明确拒绝。已有后续业务调用规则不变。
- `CRLF example.txt` 是实体文本转换，不是对象加载。先相对玩家 `cwd` 解析，再对同一路径检查、读取、写入；文件缺失、目录和读写失败都不报转换成功。不会转换根目录的同名文件或虚拟对象；写入仍遵守宿主文件权限。

### 玩家移动与观察

`go` / `look` 遇到出口加载拒绝、异常或空对象时，保留玩家位置并给出游戏内提示；内部路径和异常写入 `LOG_DIR "command"`，不拼进玩家反馈。有效实体/虚拟出口沿原流程处理，不改变 `valid_leave`、宿主移动钩子及虚拟提供程序拒绝后不回退的规则。
