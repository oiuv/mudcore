# LPC 函数命名与兼容迁移

## 命名规范

新函数统一使用小写 `snake_case`：sefun、lfun、公开方法、protected 钩子、private 辅助函数和回调遵循同一规则，例如 `bit_check()`、`set_connection()`、`on_connect()`。不要通过驼峰区分 efun 与自定义函数；通过定义位置、接口文档及必要的 `efun::`、`::` 限定区分来源。驱动要求的 apply 名称不变。

本次只规范函数。变量、存档字段（如 `toDoList`）、协议键、宏、文件名和对象路径不因此改名。不要为风格改变数据格式或要求宿主全量迁移。

## 常用名称与冲突处理

| 历史名称 | 新代码使用 | 说明 |
| --- | --- | --- |
| `bitCheck` / `bitSet` / `bitClear` | `bit_check` / `bit_set` / `bit_clear` | 位序从 1 开始，返回规则不变 |
| `setConnection` / `setAutoClose` | `set_connection` / `set_auto_close` | 数据库配置 |
| `whereIn` / `orderBy` / `groupBy` | `where_in` / `order_by` / `group_by` | 链式查询仍返回原对象 |
| `setArea` / `removeExit` | `set_area` / `remove_exit` | 房间接口 |
| `getToDoList` / `isQuest` | `get_todo_list` / `is_quest` | 任务接口 |
| `getVerbs` / `setRules` | `get_verbs` / `set_rules` | parser 谓词接口 |
| `sendGMCP` | `send_gmcp_data` | mapping 编码封装；已有 `send_gmcp(string)` 原始发送接口不变 |
| `moveObject` | `move_area_object` | 区域坐标移动，避免与 `move_object` efun 混淆 |
| `parseRefresh` | `refresh_parser` | master 包装器，避免遮蔽内部调用的 `parse_refresh` efun |
| `isNewly` / `noGiveUp` | `is_repeatable` / `is_abandonment_forbidden` | 判断能否重复、是否禁止放弃，不反转原布尔含义 |
| `getValidVerb` | `is_valid_verb_path` | 判断路径，不是获取一个谓词 |
| `getSelect` / `isReward` | `handle_quest_selection` / `try_complete_quest` | 前者处理输入；后者检查并改变任务状态，不是纯查询 |
| `getItem`（任务定义） / `getItem`（玩家组件） | `get_required_items` / `get_item_count` | 分别返回需求映射、已收集数量，不能因旧名相同而共用模糊新名 |
| `getCamp` / `getCampScore` / `getCampRank` | `get_camp_reputation` / `get_camp_rank` / `get_camp_rank_name` | 区分原始声望、数值等级和等级名称 |
| `setCamp` / `updateCamp` | `adjust_camp_reputation` / `adjust_camp_reputations` | 都是增量；后者还联动友好和敌对阵营，不改成赋值 |
| `implodeX` / `addArrayOfWheres` | `join_sql_values` / `append_where_group` | SQL 值编码连接、追加条件组 |
| `patternInfo` / `setPattern` | `show_pattern_info` / `apply_pattern` | 显示图样信息、应用图样到区域 |
| `query_LOLO` / `query_LOO` | `query_load_locations` / `query_object_locations` | 返回配置对象加载的坐标集、存在对象的坐标集 |

先核对函数实现、调用上下文、返回类型和副作用，再选择名称：查询使用 `get_*`/`query_*`；判断使用 `is_*`/`has_*`；输出、输入处理和状态变更使用明确动作。集合、数量、描述通过复数或后缀区分。含义清楚的原词保留，不因改名扩大功能重构。完整旧公开/受保护接口清单见 [兼容清单](../tests/function-names.json)。

按语义确定单词边界，不机械拆字：`todo` 表示待办事项，是一个完整概念。因此使用 `get_todo_list()`、`get_todo_list_size()`、`get_todo()`、`set_todo()`、`del_todo()`，不使用 `get_to_do_list()`。

## 兼容边界

旧公开名称和受保护钩子暂时保留，供已有 MUD 分步升级；它们不是新代码的推荐接口。private 函数和测试辅助函数直接改名。删除旧入口必须另行按破坏性变更评估，不能在普通样式整理中顺便移除。

兼容入口使用同一个下划线实现，并识别宿主只覆盖旧名或只覆盖新名的情况；两种名称的调用都可到达该覆盖。覆盖方法用对应名称的 `::` 调用父实现。若宿主同时覆盖新旧名称，两者各自负责自己的行为，不应互相递归转调。不要依赖 `_mudcore_*` 私有迁移辅助函数。

上述双覆盖规则也适用于不同继承层分别覆盖新旧名：对应名称的 `::` 不会自动串联另一名称的覆盖。需要保留同一扩展链时，统一迁移该链上的名称，不要仅改最末级。

兼容机制共用 `include/function_compat.h`；按文件移植组件时需同时提供此头文件和正确的 include 搜索路径，它不连接服务、不保存玩家状态。

框架访问宿主提供的任务、阵营、区域和谓词方法时优先使用新名，缺少新名才选择旧名；已存在的新方法抛错不会被吞掉或改调旧方法。仍可只实现旧接口，不要求继承框架示例对象。

普通任务与自由任务都可使用 `is_quest()`，但这不是完整能力协议；自由任务的信息扫描另要求 `register_information()`，不因名称统一而自动混入普通任务。见 [任务接口边界](quest.md#与自由任务的边界)。

## 升级与验证

1. 新功能只写下划线名；存量宿主可继续调用兼容入口。
2. 迁移覆盖时同步原型、调用、`::`、闭包、`call_out` / `input_to` 的方法字符串，以及 `function_exists` 等反射查询。
3. 更新 simul efun 和继承基类后，再重新加载依赖对象，或完整重启测试 MUD。源码替换不会自动刷新内存中的程序；已有子对象也不会自动改用重载后的父程序。
4. 运行 `node tests/function-names.mjs`，再运行 `node tests/run.mjs <driver路径>`；使用方另行执行全量编译和相关游戏流程回归。

作为子模块从宿主根目录执行时，命令路径加 `mudcore/` 前缀。新增 LPC 文件和改动文件仍须遵守 `AGENTS.md` 的格式化及编译要求。

### 已运行 MUD 的更新顺序

`updateall` 按目录遍历不等于按继承依赖排序。例如先编译新版 `verbs/common/look.c`、后更新 `inherit/verb.c` 时，内存中的旧父程序只有 `setVerb()` 等名称，新子程序会报 `Undefined function set_verb`、`set_synonyms`、`set_rules`、`set_error_message`；旧名兼容入口只有在新版父程序加载后才存在。

当前老 MUD 使用默认 `_VERB` 时，在游戏内先执行：

```text
update /mudcore/inherit/verb
updateall /mudcore
```

全库更新可将最后一行改为 `updateall /`。这组顺序仅解决该谓词基类的升级，不能保证其他已加载继承链也已更新；此次跨模块改名的完整验收建议保存游戏状态、按宿主正常流程重启，再全量编译。不要为绕过旧缓存把源码改回驼峰，或在命令中直接强制销毁在线玩家的继承链。

若重载后仍报同样错误，应核对实际运行的 mudlib 路径和 `_VERB` 宏是否被宿主覆盖；独立宿主基类也必须实现或适配新接口。隔离回归已加入“旧父程序驻留 → 替换源码 → 新子程序编译失败 → 父类先重载 → look/go 编译并初始化成功”，不代表任意在线对象的热迁移都安全。
