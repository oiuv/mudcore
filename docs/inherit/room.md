### CORE_ROOM

房间环境组件，已组合 `_DBASE`、`_NAME` 和 `_CLEAN_UP`，无需重复继承。宿主房间通常继承 `_ROOM`；定制 `_ROOM` 对应的基础组件时继承 `CORE_ROOM`，避免循环继承。详见 [接入指南](../integration.md#用别名扩展框架)。

实现的核心方法如下：

```c
// 初始化环境，加载环境中的对象
void setup();

// 获取环境坐标
string coordinate();

// 设置环境区域和坐标
varargs void setArea(mixed area, int x, int y, int z);

// 移除指定出口
void removeExit(string dir);

// 出口数大于 1 时移除随机出口
void removeRandomExit();

// 增加出口
void addExit(string dir, mixed dest);

// 检查是否合法移动
int valid_leave(object me, string dir);

// 设置门
varargs void set_door(string dir, mixed data, string other_side_dir, int status);

// 打开门
varargs int open_door(string dir, int from_other_side);

// 关闭门
varargs int close_door(string dir, int from_other_side);

// 获取指定的门
mixed query_door(string dir, string prop);

// 获取所有的门
mapping query_doors();

// 查看门
string look_door(string dir);
```

另有兼容命名 `set_area()`、`remove_exit()`、`remove_random_exit()`、`add_exit()`，分别对应上述驼峰方法；`create_door()` 对应 `set_door()`。光照接口为 `int query_light()` 和 `void add_light(int light)`。完整实现见 [room.c](../../inherit/room.c)。

房间在 `create()` 中设置描述、出口和 `objects` 后调用 `setup()`，由它触发首次 `reset()`：

```c
inherit _ROOM;

void create() {
    set("short", "庭院");
    set("long", "庭院中有一株老树。\n");
    set("exits", ([]));
    setup();
}
```
