### 虚拟对象守护进程

采用框架 master 时，找不到实体源码的对象会交由 `VIRTUAL_D` 处理。宿主按需定义 `WORLD_DIR`（虚拟地区）或 `MOB_DIR`（虚拟怪物），两者独立可选；目录宏使用以 `/` 结尾的路径。默认还支持框架 `CORE_DIR "world/area/"` 下的虚拟地区。已有 MUD 保留自己的 master 时，需要自行接入相应的 `compile_object()` 调用。

### 核心方法

```c
mixed compile_area(string file);
mixed compile_mob(string file);
mixed compile_object(string file);
```

### 虚拟对象功能说明

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

2. 非逗号坐标路径转交模板的 `query_maze_room()`。`CORE_VRM` 提供迷宫实现，路径包括 `实际对象文件/x-y`、`实际对象文件/entry` 和 `实际对象文件/exit`。

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

需要开发者自己根据游戏开发需要实现。
