# 泥芯(MudCore) - LPMUD 游戏开发框架

![mudcore](https://api.mud.ren/storage/uploads/2020/02/13/c5a880553865b046d14c13b042318438.png)

LPMUD游戏开发框架核心代码，仅仅包括核心代码，可以在此基础上开发任何MUD，本框架需配合 FluffOS utf-8版使用。

当前开发版本：`2.0.0-dev`，由 [MUDCORE_VERSION](include/mudcore.h) 定义；正式发布前保留 `-dev`。变更见 [更新日志](CHANGELOG.md)，版本规则见 [版本管理](docs/maintenance.md)。

这个项目的诞生源于我的[LPC零基础开发教程](https://bbs.mud.ren)，在写教程的过程中，发现很多同学并不关心底层细节，只想能直接开发游戏项目，国内绝大多数MUD游戏都是《东方故事2》底层，在已有MUD基础上修改的优点是可以快速上手，但这个算不上独立的游戏框架。能不能把底层独立出来，开发任何类型的MUD都可以使用？在思考后，我开始了这个项目，特色是只提供底层代码和接口，几乎不提供任何游戏性的内容，独立于游戏项目之外，只需简单配置即可实现MUD游戏开发。

> 面向 UTF-8 FluffOS。框架生产源码默认使用 `.c`，仅测试代码可使用 `.lpc`；具体模块仍需对应的驱动 efun 和可选包。接入要求见 [兼容说明](docs/integration.md#源文件与驱动能力)。

 - 框架下载地址：https://github.com/mudcore/mudcore
 - 国内镜像地址：https://gitee.com/mudcore/mudcore
 - 框架示例项目：https://github.com/mudcore/mymud
 - 框架基础模板：https://github.com/mudcore/mud (可用模板直接开发新MUD)
 - 框架使用教程：https://bbs.mud.ren/threads/66
 - 框架集成示例：https://bbs.mud.ren/threads/99

致谢：本框架开发代码、结构、功能和思路上大量的参考借鉴了ES2系列游戏、重生的世界MUD、火影忍者MUD、DeadSouls等，感谢这些优秀MUD。

## 框架特色

框架提供大量常用模拟外部函数和最基本的游戏底层（包括指令系统、聊天频道、表情系统和档案存取功能），可以用来快速开发MUD游戏，框架部分独立维护，更新升级不影响个人项目。个人项目目录结构和功能灵活自主，框架提供的功能可灵活选择使用，只需要继承对应模块即可。

## 框架依赖边界

框架保留 HTTP/HTTPS、Socket、数据库和外部命令等通用能力。服务地址、凭据、请求时机和业务规则由 MUDLIB 提供；加载框架不能自动访问某个具体外部服务。Intermud 需由使用方显式调用 `INTERMUD_D->start(host, port)`，不会自动连接预置中心。固定二维码网站等业务功能放在使用方项目。

开发验证和兼容调整见 [测试说明](tests/README.md) 与 [依赖边界迁移说明](docs/dependency-boundary.md)。

默认完整玩家组合保持不变，也可显式选择 `CORE_USER_BASE` 最小组合，配套命令阶段、parser 开关和角色策略；不能只改一个宏就假定所有游戏命令都兼容。选用模块前查阅 [架构规范](docs/architecture.md)、[组件依赖与接口契约](docs/module-contracts.md) 和 [最小组合接入](docs/integration.md#显式选择最小玩家组合)。

## 框架使用说明

LPC 函数统一使用 `snake_case`，例如 `bit_check()`、`set_connection()`。历史公开名称保留兼容入口，新代码不再使用驼峰函数名；继承覆盖和回调迁移见 [函数命名规范](docs/function-naming.md)。

新项目推荐从 [minimud 基础模板](https://github.com/mudcore/mud) 开始：

```sh
git clone --recurse-submodules https://github.com/mudcore/mud.git
```

已有 Git 项目可在宿主根目录添加框架子模块：

```sh
git submodule add https://github.com/mudcore/mudcore.git mudcore
```

未使用 Git 子模块的项目也可单独克隆框架到 `mudcore/`。保留宿主自己的脚本、配置、玩法与玩家数据，定制优先通过继承和别名覆盖完成。

- [框架接入与扩展](docs/integration.md)：新项目最小接入、已有 MUD 按需集成、宏覆盖及三类配置。
- [版本管理与升级验证](docs/maintenance.md)：版本宏、兼容升级、验证和子模块同步顺序。
- [开发文档索引](docs/README.md)：登录、命令、数据库、网络及其他模块。
- [隔离回归测试](tests/README.md)：默认实现与宿主覆盖测试，不读取实际玩家数据。

模板已提供基本入口文件，仍需准备可用驱动，并由运维在开放注册前完成管理员账号初始化。已有 MUD 无需整体替换原来的 master、登录和安全策略。

## 框架目录结构

下表的目录与文件路径相对 `mudcore/`；实际对象路径包含框架目录前缀，默认是 `/mudcore/`。

目录|说明
-|-
cmds|框架提供的基本action指令
docs|开发者文档，包括框架模拟外部函数说明文档等
include|框架头文件，包括fluffos驱动内置头文件
inherit|框架特性继承文件
system|系统文件目录
verbs|框架提供的基本parser指令
world|框架示例世界环境

### system 系统目录

目录|说明
-|-
system/kernel|系统核心文件，包括master对象和simul_efun对象
system/daemons|系统守护进程
system/object|系统核心对象，包括登录对象、基本玩家对象和 VOID 环境

### 核心对象

名称|文件路径|核心功能
-|-|-
CORE_MASTER_OB|/system/kernel/master|主控对象，负责接收玩家连接请求、安全验证、日志记录等核心功能
CORE_SIMUL_EFUN_OB|/system/kernel/simul_efun|模拟外部函数对象，负责实现模拟外部函数功能
CORE_LOGIN_OB|/system/object/login|示例登陆对象，仅供参考，正式开发可继承使用
CORE_USER_OB|/system/object/user|示例玩家对象，仅供参考，正式开发可继承使用
CORE_VOID_OB|/system/object/void|临时环境，示例登录房间

所有核心对象有去掉`CORE_`前缀的同名宏定义，方便覆盖和继承，如`LOGIN_OB`。

### 系统头文件(Include Headers)

头文件在 `/include/` 目录中，主要是常量宏定义和函数声明，框架基本提供如下头文件：

文件|说明
-|-
ansi.h|定义ANSI 色彩/游标控制码
dbase.h|数据增删改查功能（CORE_DBASE）函数声明
function.h|驱动提供，配合 functionp() 使用
localtime.h|驱动提供，配合 localtime() 使用
mudcore.h|框架全局头文件，需要在项目的 <globals.h> 中引用
mysql.h|定义数据库配置
origin.h|驱动提供，配合 origin() 使用
parser_error.h|驱动提供，定义 parser error
runtime_config.h|驱动提供，配合 get_config() 使用
socket_err.h|驱动提供，配合 socket_error() 使用
socket.h|定义 socket 类型，配合 socket_create() 使用
type.h|驱动提供，配合 typeof() 使用
[tui.h](docs/TUI.md)|可选终端界面的路径、按键和控件常量

### 继承特征模块(Inherit Objects)

继承文件在 `/inherit/` 目录，实现了核心的功能片段，方便直接使用，具体参考 `/docs/inherit/` 目录。基本提供如下继承文件：

名称|文件|说明
-|-|-
CORE_ACTION|/inherit/action.c|角色行动接口，实现限制行为功能
CORE_AREA|/inherit/area/area.c|游戏区域环境标准接口，实现区域模式环境功能
CORE_ATTACK|/inherit/attack.c|角色攻击接口，实现战斗行为功能，需自己实现具体战斗方式
CORE_CAMP|/inherit/camp.c|游戏阵营接口，实现阵营声望相关功能
CORE_CLEAN_UP|/inherit/clean_up.c|自动清理接口，实现 clean_up() 方法的垃圾回收功能
CORE_CMD|/inherit/CMD.c|通用外部命令接口，由 MUDLIB 配置可执行文件
CORE_COMMAND|/inherit/command.c|角色指令系统接口，实现生物对象特征功能
CORE_CONDITION_MOD|/inherit/condition_mod.c|角色增益功能接口，提供增益相关方法
CORE_CONDITION|/inherit/condition.c|角色增益状态控制接口，实现游戏BUFF功能
CORE_DB|/inherit/DB.c|数据库接口，按驱动可选包支持 MySQL、SQLite 等后端
CORE_DBASE|/inherit/dbase.c|数据存取功能接口，实现对象参数的增删改查功能
CORE_DBSAVE|/inherit/dbsave.c|系统数据存取接口，配合 DBASE_D 使用
CORE_HTTP|/inherit/Http.c|HTTP客户端，方便发起http请求
CORE_LIVING|/inherit/living.c|生物对象功能接口，所有生物对接可直接继承使用
CORE_MESSAGE|/inherit/message.c|玩家信息处理功能接口，实现分页显示
CORE_MOVE|/inherit/move.c|对象移动接口，由角色、物品对象继承，方便移动
CORE_NAME|/inherit/name.c|ID和名称接口，让对象可以被看见(查找)和命名
CORE_NOCLONE|/inherit/noclone.c|限制对象禁止被复制功能接口，需调用`check_clone()`检查
CORE_ROOM|/inherit/room.c|游戏环境标准接口，已组合可覆盖的 _DBASE、_NAME 和 _CLEAN_UP
CORE_SAVE|/inherit/save.c|对象数据存取接口，主要是玩家角色使用存档和读档
CORE_SOCKET|/inherit/Socket.c|通用 Socket 封装，提供 TCP、UDP 及 TLS 基础支持
CORE_TEAM|/inherit/team.c|角色组队功能接口，实现组队相关功能
[TUI_PRINT / TUI_TERMINAL / TUI_APP](docs/TUI.md)|/inherit/tui/|可选终端输出、菜单、输入编辑及全屏控件，需包含 `<tui.h>`；上游授权见文档
CORE_UNIQUE|/inherit/unique.c|对象唯一性功能接口，注意和noclone不同，唯一性允许复制，但仅限一次
CORE_USER_COMBAT_RECORD|/inherit/user_combat_record.c|玩家战斗记录功能，实现玩家战斗数据记录
CORE_USER_GMCP|/inherit/user_gmcp.c|玩家GMCP功能接口，可配合mudlet客户端使用
CORE_USER_QUEST|/inherit/user_quest.c|玩家任务功能接口，实现玩家任务记录与管理
CORE_USER|/inherit/user.c|玩家对象功能接口
CORE_VERB|/inherit/verb.c|自然语法分析指令功能接口
CORE_VRM|/inherit/vrm.c|随机迷宫功能接口，实现随机迷宫功能

所有特征模块有去掉`CORE`前缀的同名宏定义，方便覆盖和继承，如`_CLEAN_UP`。

### 守护进程(Daemons)

守护进程是独立出来的系统服务，每个进程有自己的API可以调用，具体参考 [docs/daemons/](docs/daemons/) 目录。

名称|文件路径|核心功能
-|-|-
CORE_AREA_PATTERN_D|/system/daemons/area_pattern_d.c|区域模式环境样式模型管理
CORE_CAMP_D|/system/daemons/camp_d.c|游戏阵营守护进程
CORE_CHANNEL_D|/system/daemons/channel_d.c|负责处理玩家聊天功能和频道管理
CORE_CHAR_D|/system/daemons/char_d.c|负责处理角色数据，需开发者实现具体功能
CORE_CHINESE_D|/system/daemons/chinese_d.c|负责处理中、英文对照，以及一些与中文相关的功能，字典存档`/data/e2c_dict.o`
CORE_COMBAT_D|/system/daemons/combat_d.c|负责战斗功能，需开发者实现具体功能
CORE_COMMAND_D|/system/daemons/command_d.c|负责指令及别名处理功能
CORE_DBASE_D|/system/daemons/dbase_d.c|负责系统数据存档处理，默认存储位置`/data/dbase_d.o`
CORE_EMOTE_D|/system/daemons/emote_d.c|负责管理游戏表情动作和相应功能，默认存储位置`/data/emote_d.o`
CORE_ENV_D|/system/daemons/env_d.c|游戏环境变量配置守护进程，默认配置文件为`/data/.env`
CORE_EVER_QUEST_D|/system/daemons/ever_quest_d.c|随机任务守护进程
CORE_HEADER_D|/system/daemons/header_d.c|头文件自动生成系统守护进程
CORE_INTERMUD_D|/system/daemons/intermud/i2d.c|MUD网际互联守护进程
CORE_LOGIN_D|/system/daemons/login_d.c|框架示例登录管理功能，可做开发参考
CORE_NAME_D|/system/daemons/name_d.c|负责角色姓名记录与检测，默认存储位置`/data/name_d.o`
CORE_NATURE_D|/system/daemons/nature_d.c|负责控制游戏季节和天气变化
CORE_QUEST_D|/system/daemons/quest_d.c|游戏任务守护进程
CORE_TIME_D|/system/daemons/time_d.c|负责控制游戏系统时间和计划任务
CORE_VERB_D|/system/daemons/verb_d.c|自然语法谓词指令守护进程
CORE_VIRTUAL_D|/system/daemons/virtual_d.c|负责虚拟对象处理

所有守护进程有去掉`CORE_`前缀的同名宏定义，方便覆盖和继承，如`LOGIN_D`。


### 系统函数(Simul-Efuns)

除驱动自带的 efun 外，框架提供了部分模拟外部函数，具体使用参考 `/docs/simul_efun/` 目录，或查看函数源码（`/system/kernel/simul_efun/`）。

simul-efun|简介
-|-
all_environment|返回对象的所有环境
ansi|转换 $HIR$ 等为 ansi 颜色代码
area_environment|比較二個对象是否處在相同的區域座標中
area_move|將對象移動到指定區域的指定坐標位置
area_move_side|將who移到與me同一格的區域坐標位置
[array_sum](docs/simul_efun/array_sum.md)|返回数组元素的和
assure_file|建立文件目录
atof|string 类型数字转 float 类型数字
[atoi](docs/simul_efun/atoi.md)|将数字字符串或数值转为整数
[base64encode / base64decode](docs/simul_efun/base64.md)|UTF-8 字符串的标准 Base64 编解码
[bit_check](docs/simul_efun/bit_check.md)|检查指定二进制位，返回位掩码或 0；位序从 1 开始
[bit_clear](docs/simul_efun/bit_clear.md)|返回数值指定位为0的值
[bitmap_font](docs/simul_efun/bitmap_font.md)|处理中英文字符并点阵输出
[bit_set](docs/simul_efun/bit_set.md)|返回数值指定位为1的值
break_string|根据指定宽度换行显示内容
cat|输出文件内容到屏幕
cecho|随机颜色显示内容到屏幕
check_control|判断指定字符串是否包含控制字符
check_return|判断指定字符串是否包含换行符
check_space|判断指定字符串是否包含空格
chinese_number|返回中文数字
chinese|返回指定字符串的中文名称
color_cat|彩色输出内容到屏幕
color_len|统计字符串中颜色占用的长度
config|读取并缓存游戏自定义 JSON 配置(`/config.json`)
debug|随机颜色输出内容至驱动控制台
deep_path_list|获取指定目录及子目录下的文件列表
element_of_weighted|根据权重随机返回元素，权重高机率大
env|读取游戏配置或修改 ENV_D 内存中的值；不自动写回 `/data/.env`
expand_keys|把键为数组的映射展示
file_exists|判断文件是否存在
getcid|返回复制对象的ID
graph_draw|自定义进度条绘图
[highest](docs/simul_efun/highest.md)|返回一组数值的最大值
is_chinese|判断指定字符串是否为中文
is_english|判断指定字符串是否为英文
is_numeric|判断指定字符串是否为数字
[json_encode / json_decode](docs/simul_efun/json.md)|JSON 编解码，支持 UTF-8 字符串和 buffer 输入
log_file|记录日志到指定文件
log_time|方便日志记录的 ctime 时间格式
[lowest](docs/simul_efun/lowest.md)|返回一组数值的最小值
msg|智能区域消息
number_string|转化数字为科学计数法的字符串格式
[percent](docs/simul_efun/percent.md)|返回数字占基数的百分比，支持整数和浮点数
[percent_of](docs/simul_efun/percent.md)|返回基数的指定百分比，支持整数和浮点数
[present_clone](docs/simul_efun/present_clone.md)|按来源文件查找容器中的第 n 个直属物品
[print_r](docs/simul_efun/print_r.md)|数组打印功能，格式化输出，开发调试用
process_bar|显示指定百分比的进度条
pronoun|返回中文人称代词
[range](docs/simul_efun/range.md)|将数值限制在指定上下限之间
read_lines|返回有效行并保留前后空白，去掉 CRLF 行末 CR；跳过空白行及允许缩进的 `#`/`;` 注释行
remove_ansi|清理 ansi 颜色代码
sort_string|根据指定宽度换行显示内容，中文支持更友好
[sum](docs/simul_efun/sum.md)|返回一组数值的和
tail|输出文件尾部最多10行内容到屏幕
[unicode](docs/simul_efun/unicode.md)|将 Unicode 码点转为字符，如 unicode(65) 返回 "A"

### 默认指令

泥芯框架提供了部分基础指令辅助使用，具体指令参考`/mudcore/cmds/`目录和`/mudcore/verbs/`目录。
