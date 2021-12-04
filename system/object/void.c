/**
 * @brief VOID_OB 房间模式示例，无边无际
 *
 */
inherit CORE_CLEAN_UP;
inherit CORE_DBASE;
inherit CORE_NAME;
inherit CORE_ROOM;

private varargs void create(int x, int y, int z)
{
    set("short", "时空中转站");
    set("long", @LONG
这里是多维宇宙的时空中转站，正常情况下，没有人能来到这里。
LONG);
    set("void", 1); // 特殊环境标记
#ifdef START_ROOM
    set("exits", ([
        "down" : START_ROOM,
    ]));
#else
    set("exits", ([
        "north" : __DIR__ "void/" + x + "," + (y + 1) + "," + z,
        "south" : __DIR__ "void/" + x + "," + (y - 1) + "," + z,
        "west" : __DIR__ "void/" + (x - 1) + "," + y + "," + z,
        "east" : __DIR__ "void/" + (x + 1) + "," + y + "," + z,
        "down" : ([
            "filename" : CORE_DIR "world/area/void",
            "x_axis" : 55,
            "y_axis" : 5,
        ]),
    ]));
    setArea("void", x, y, z);
#endif
}
