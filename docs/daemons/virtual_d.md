# 虚拟对象守护进程

采用框架 master 时，找不到实体源码的对象会交由 `VIRTUAL_D` 处理。新开发统一实现 `create_virtual_object(string key)`，房间、物品和 NPC 使用同一接口，不要求定义世界/怪物目录。已有 MUD 保留自己的 master 时，自行接入 `VIRTUAL_D->compile_object(file)`；也可仅采用同一处理程序约定而保留宿主 daemon。

框架依赖对象/文件 efun 与 `lpc_file()`、`log_file()` 等框架 sefun；无外部服务或游戏数据依赖。宿主的 master 权限、UID 策略和业务检查仍有效，通用分派不负责授予权限。

### 核心方法

```c
mixed compile_area(string file);
mixed compile_mob(string file);
mixed compile_object(string file);
```

## 新开发约定

路径 `/provider/key` 的最后一段是标识，前面对应实体 `provider.lpc` 或 `provider.c`（同名优先 `.lpc`）。标识非空，原样传入，不根据数字、逗号或物品类型猜测参数。对象引用不带源扩展名。

```c
// /obj/catalog.lpc：具体对象 /obj/cloth 及其数据由宿主提供。
object create_virtual_object(string key) {
    if (key != "cloth")
        return 0;
    return new("/obj/cloth", key);
}
```

使用 `load_object("/obj/catalog/cloth")` 读取虚拟蓝图，使用 `new("/obj/catalog/cloth")` 创建实例。处理程序也可以解析 `1,2,3` 后调用带多个参数的构造；框架 `world/area/tower.c` 是坐标示例。`CORE_VRM` 已提供新接口，继承并完成迷宫配置即可使用 `entry`、`exit`、`x-y` 路径，无须自己转发。

- 返回新克隆对象，或以 `0` 拒绝未知标识；不能返回处理程序自身、已登记虚拟对象、共享蓝图或错误字符串。新接口存在时，返回 `0` 或抛错都不会重试旧式构造；真实异常传播供诊断。
- 返回后驱动才赋予虚拟路径、最终克隆标志及 UID，并调用 `virtual_start()`。需要最终 `base_name()` 或品种默认对象的初始化放在那里，不提前假定构造时身份已稳定，不重复 `setup()`。
- 无参加载处理程序是定位回调的一部分，不应产生可交付业务实例或重复初始化副作用。每次请求创建独立对象，可变状态不共享；不要手动改名或提升权限。
- `base_name(ob)` 用于品种/房间规范身份；可用 `new(base_name(ob))` 重建同品种实例。业务自身仍负责未知键、实例归属及存取资格，虚拟化不等于自动可存档。

## 宿主覆盖与旧接口兼容

`WORLD_DIR`、`MOB_DIR` 是可选旧式路由，均须以 `/` 结尾；两者重叠时世界优先，其后为默认 `CORE_DIR "world/area/"`。匹配宿主目录时仍通过 `VIRTUAL_D` 调用 `compile_area()` / `compile_mob()`，宿主可覆盖并调用 `::` 父实现。完全替换 VIRTUAL_D 的宿主自行决定是否采用新能力。

框架默认的这两个辅助入口先检查新回调；不存在才执行下面的旧规则。未匹配目录时只支持新回调，不对任意普通程序开放隐式坐标/编号构造。`compile_object()` 只向驱动交付对象或 `0`；旧 `compile_mob()` 被直接调用时的字符串失败结果保留，但不会当成虚拟对象交给驱动。

以下仅供既有宿主迁移，新代码使用上面的统一约定。原合法虚拟路径和存档可保持不变。

#### 地区对象

需在 `WORLD_DIR` 中，有二种虚拟地形方式：

1. 虚拟对象文件为`实际对象文件/x,y,z`或`实际对象文件/x,y`，常用于无限随机地形。

以 `WORLD_DIR` 下的 `wild.c` 为例，虚拟路径引用不带扩展名，如 `wild/1,2,0`。模板可继承房间组件，并接收坐标参数：

```c
inherit _ROOM;

varargs void create(int x, int y, int z) {
    set("exits", ([
        "north": __DIR__ "wild/" + x + "," + (y + 1) + "," + z,
        "south": __DIR__ "wild/" + x + "," + (y - 1) + "," + z,
        "west": __DIR__ "wild/" + (x - 1) + "," + y + "," + z,
        "east": __DIR__ "wild/" + (x + 1) + "," + y + "," + z,
    ]));
    set("short", "荒野");
    set("long", "四周是一片荒野。\n");
    setup();
}
```

2. 未提供新接口时，非逗号坐标路径转交旧 `query_maze_room()`。`CORE_VRM` 保留该公开方法作为兼容同义入口，实际迷宫实现与新接口共用，路径仍包括 `实际对象文件/x-y`、`实际对象文件/entry` 和 `实际对象文件/exit`。

以下示例放在 `WORLD_DIR` 下，并需要宿主提供相邻的 `start_room` 和 `tower` 房间。怪物列表是可选项；启用前须配置 `MOB_DIR` 并提供实际模板。

```c
// 迷宫
#include <ansi.h>
inherit CORE_VRM;

void create() {
    //迷宫房间所继承的对象的档案名称。
    set_inherit_room(_ROOM);

    //迷宫房间里的怪物。
    // set_maze_npcs(({ MOB_DIR "9/11", MOB_DIR "9/28" }));

    //迷宫的单边长
    set_maze_long(10);

    //入口方向(出口在对面)
    set_entry_dir("south");

    //入口与区域的连接方向
    set_link_entry_dir("south");

    //入口与区域的连接档案名
    set_link_entry_room(__DIR__ "start_room");

    //出口与区域的连接方向
    set_link_exit_dir("north");

    //出口与区域的连接档案名
    set_link_exit_room(__DIR__ "tower");

    //入口房间短描述
    set_entry_short(HIB "黑森林" NOR);

    //入口房间描述
    set_entry_desc(HIB @LONG
这里据说就是黑森林，里面全是阴雾，阴气逼人，不小心就可能迷失方向了。
LONG NOR);

    //出口房间短描述
    set_exit_short(HIB "黑森林" NOR);

    //出口房间描述
    set_exit_desc(HIB @LONG
你眼前一亮，深深的吸了口气，心想总算是出来了。不过景色忽的一变，眼前出现一座高耸入云的魔法塔。
LONG NOR);

    //迷宫房间的短描述
    set_maze_room_short(HIB "黑森林" NOR);

    //迷宫房间的描述，如果有多条描述，制造每个房间的时候会从中随机选择一个。
    set_maze_room_desc(HIB @LONG
四周越来越暗了，你胆颤心惊的向前摸索着，到处是一些迷路人的尸体和骷髅。不时传来一阵阵鬼哭儿狼嚎,好象有什么东西在暗中窥视着，你不由的加快了脚步。
LONG NOR);

    // 迷宫房间是否为户外房间？
    set_outdoors(1);
}
```

#### 魔物对象

需要在 `MOB_DIR` 中，虚拟对象文件为 `实际对象/n`，`n` 会转换为整数传入模板的 `create(int n)`；该编号代表什么由宿主定义，并不自动保证怪物唯一。

#### 其他对象

在对应处理程序实现新回调即可，不必再扩充 daemon 类型分支。

## 验证与升级

`node tests/run.mjs <driver路径>` 使用隔离宿主检查新旧接口、`.c/.lpc`、数字形状键、宿主覆盖、目录优先级、真实虚拟加载/克隆与驱动初始化；不依赖本游戏或正式存档。详情见 [测试说明](../../tests/README.md)。

先升级框架，再迁移宿主处理程序及文档；框架兼容入口允许分批迁移。继承链已有驻留对象时安排维护重启，不能保证任意顺序热更。若虚拟路径未变，本接口升级本身不需转换存档；宿主同时实施路径迁移时仍须遵循自己的备份/转换方案。
