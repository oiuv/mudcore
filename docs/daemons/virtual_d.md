### 虚拟对象守护进程

需要项目定义 `WORLD_DIR` 和 `MOB_DIR`，并按以下规范实现虚拟对象服务。

#### 地区对象

需在 `WORLD_DIR` 中，虚拟对象文件为`实际对象文件/x,y,z`或`实际对象文件/x,y`，以实际对象文件 `wild.c` 为列，实际对象文件需要实现以下方法:

```c
varargs void create(int x, int y, int z)
{
    set("exits", ([
        "north" : __DIR__ "wild/" + x + "," + (y + 1) + "," + z,
        "south" : __DIR__ "wild/" + x + "," + (y - 1) + "," + z,
        "west" : __DIR__ "wild/" + (x - 1) + "," + y + "," + z,
        "east" : __DIR__ "wild/" + (x + 1) + "," + y + "," + z,
    ]));
    //
}
```

#### 魔物对象

需要在 `MOB_DIR` 中，虚拟对象文件为 `实际对象/n`，`n` 为怪物唯一ID，需要自己实现相应功能。
