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
