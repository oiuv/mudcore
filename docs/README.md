# mudcore 开发文档

初次使用从 [框架接入与扩展](integration.md) 开始；已有项目升级时先阅读 [版本管理与升级验证](maintenance.md) 和 [兼容迁移说明](dependency-boundary.md)。

## 入门与维护

| 文档 | 内容 |
| --- | --- |
| [框架概览](../README.md) | 定位、安装方式、目录及主要能力 |
| [框架接入与扩展](integration.md) | 新项目、旧 MUD 集成、宏覆盖、配置与驱动要求 |
| [架构与扩展规范](architecture.md) | 分层责任、机制与策略、默认/最小组合及兼容约束 |
| [公共模块契约](module-contracts.md) | 组件依赖矩阵、输入/返回/生命周期、可选协作与诊断边界 |
| [函数命名与迁移](function-naming.md) | snake_case 规范、旧名兼容、继承覆盖与动态回调迁移 |
| [版本管理与升级验证](maintenance.md) | 版本宏、兼容规则、发布与多宿主同步 |
| [框架能力与兼容迁移](dependency-boundary.md) | 通用能力边界、默认行为及权限变化 |
| [更新日志](../CHANGELOG.md) | 按版本整理的更新与历史变更 |
| [隔离回归测试](../tests/README.md) | 执行方法、检查范围及验证边界 |
| [精简运行配置](../config.mini.ini) / [完整配置示例](../config.example.ini) | FluffOS 配置参考 |

## 核心对象与模块

- [系统对象与守护进程](system.md)
- [可组合继承模块](inherit.md)
- [登录与角色身份](daemons/login_d.md)、[角色初始化](daemons/char_d.md)
- [命令查找与别名](daemons/command_d.md)、[虚拟对象](daemons/virtual_d.md)
- [房间](inherit/room.md)、[区域地图与虚拟房间](inherit/area.md)、[玩家消息与分页](inherit/message.md)
- [TUI 终端界面](TUI.md)：可选表格、菜单、输入编辑与全屏控件
- [任务](quest.md)、[战斗接口](daemons/combat_d.md)

## 网络、存储与服务

- [HTTP 客户端](Http.md)、[Socket 与 TLS](Socket.md)
- [数据库接口及授权](inherit/DB.md)、[对象数据存档](daemons/dbase_d.md)
- [频道](daemons/channel_d.md)、[表情](daemons/emote_d.md)
- [时间与调度](daemons/time_d.md)、[中文名称字典](daemons/chinese_d.md)
- [Intermud 显式启停与状态](dependency-boundary.md#迁移调整)

## 模拟外部函数

单项说明位于 [simul_efun](simul_efun/)；包括 [求和](simul_efun/sum.md)、[范围限制](simul_efun/range.md)、[位检查](simul_efun/bit_check.md)、[Unicode 字符](simul_efun/unicode.md)、[点阵字](simul_efun/bitmap_font.md) 和 [数组打印](simul_efun/print_r.md)。常用数据接口包括 [JSON](simul_efun/json.md)、[Base64](simul_efun/base64.md)、[精确小数](simul_efun/decimal.md)、[百分比](simul_efun/percent.md) 和 [按来源查找物品](simul_efun/present_clone.md)。完整入口及源码分别见 [框架函数目录](../README.md#系统函数simul-efuns) 与 [simul efun 实现](../system/kernel/simul_efun/)。

文档尚未逐一覆盖所有公共方法，接口签名以对应源码为准。修改接口时同步维护相关说明，不以一次编译成功替代实际行为验证。
