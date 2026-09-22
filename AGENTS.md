# Repository Guidelines

## 框架定位与设计边界

mudcore 是独立的通用 LPMUD 开发框架，基于 UTF-8 FluffOS，提供与具体玩法解耦的核心功能。既支持从零开发新 MUD，也支持老 MUD 按需集成；模块应可选择、组合和替换，不要求整体采用。minimud 是新项目模板，MyMud 是基于框架开发的新 MUD；老 MUD 仅使用部分能力也是正常用法。

新增功能应保持通用性，具体剧情、门派、数值规则和运营配置放在使用方项目。可选功能须明确依赖，不得因未启用无关模块而阻断基础功能。

Socket、HTTP、TLS、数据库、外部命令和 Intermud 协议属于合理的通用能力，不能以“去依赖”为由删除或禁用。框架不得内置具体服务地址、凭据或默认主动调用；MUDLIB 提供配置并按需启用，第三方业务适配放在使用方项目。示例中的地址不等于运行依赖。

## 目录与扩展约定

`include/` 定义公共宏和接口；`inherit/` 提供可组合组件；`system/kernel/` 提供 master 和 simul efun；`system/daemons/` 提供服务；`system/object/` 提供基础对象。传统指令放在 `cmds/`，parser 谓词放在 `verbs/`，示例世界放在 `world/`，接口说明放在 `docs/`。

维护 `include/mudcore.h` 的扩展约定：`CORE_*` 指向框架实现，已有 `_MODULE`、daemon 和对象别名允许使用方预先覆盖；`<mudcore.h>` 应在使用方 `globals.h` 最后包含。游戏定制优先通过继承、覆盖或配置完成；通用修复在本仓库实现，并保持公共签名、存档结构和默认行为兼容。

## 开发与验证

本仓库不自带 FluffOS 驱动。独立检出运行 `node tests/run.mjs <driver路径>`；作为子模块，从宿主根目录运行 `node mudcore/tests/run.mjs fluffos/build/bin/driver.exe`。需要 Node.js 18+ 和支持 `.lpc` 的 FluffOS；测试在临时 mudlib 中检查默认实现、宿主覆盖和本机 Socket 收发，不读取实际玩家数据或访问外部服务，详见 `tests/README.md`。

游戏集成使用 minimud 等测试宿主或当前 MUDLIB。参考 `config.mini.ini`、`config.example.ini` 配置实际 mudlib、include、master 和 simul efun 路径，不能假定模板可直接启动任意检出。

在已配置的测试宿主根目录执行 `driver config.ini -d` 启动调试。编译或加载受影响对象，查看配置指定的调试与错误日志，复测相关登录、命令、移动或存取流程。公共接口变更需覆盖默认实现和使用方覆盖两种情况；保存可复现步骤，说明未验证的场景。目前没有统一覆盖率门槛。

## LPC 风格与命名

使用 UTF-8、LF、四空格缩进、K&R 大括号和末尾换行，不用制表符；目标行宽 100。变量在函数开头声明；常量使用 `UPPER_SNAKE_CASE`，新局部变量和业务辅助函数使用 camelCase，驱动 apply、efun 和既有公共接口保留原名。

新 LPC 文件使用小写 `.lpc`，头文件使用 `.h`；保留已有 `.c` 文件名，不新增同路径同名的双扩展文件。对象引用优先省略扩展名，`#include` 使用实际文件名。新增 `.lpc` 时同步检查命令索引、谓词、预加载和虚拟对象扫描对双扩展名的支持。

## 格式化与语法依据

修改 `.c`、`.lpc`、`.h` 后，交付前必须格式化并运行检查，仅处理本次变更。使用 FluffOS LPC 格式化器，固定 `indentSize: 4`、`printWidth: 100`，验证 token、字面量和注释内容不变及格式化幂等性；保留字符串、heredoc 和预处理指令的语义。

宿主提供 `tools/format_lpc.mjs` 时，从宿主根目录执行，替换为实际变更文件：

```sh
node tools/format_lpc.mjs mudcore/inherit/dbase.c
node tools/format_lpc.mjs --check mudcore/inherit/dbase.c
```

该入口需要 Node.js 18+ 和宿主 `fluffos/tools/lpc-syntax/`。独立检出不自带该工具，须先准备等效入口；工具缺失或失败应解决并如实报告，不得声称检查通过。

宿主有 `docs/LPC_Language_FluffOS.md` 时，以其最新内容为项目语法标准；独立开发以所用 FluffOS 版本的源码、efun 定义和测试为依据，记录版本及可选包要求。驱动升级后同步核对相关文档。

## 提交与协作

沿用 `fix:`、`feat:`、`refactor:`、`style:`、`docs:` 等简短提交主题。PR 写明行为变化、兼容影响、验证环境和步骤，关联相关问题；接口变更同步更新 `README.md`、`docs/`，破坏性变更补充 `CHANGELOG.md` 和迁移说明。

作为子模块提交时，先提交框架变更，再在宿主提交新的 gitlink；提交和推送按用户授权执行。不要提交凭据、玩家存档、日志或本地索引。仓库存在 `.codegraph/` 时，定位代码先用 `codegraph explore`；不存在时不自动建立索引。
