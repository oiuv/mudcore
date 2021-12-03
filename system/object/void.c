inherit CORE_CLEAN_UP;
inherit CORE_DBASE;
inherit CORE_NAME;
inherit CORE_ROOM;

private void create()
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
        "down" : ([
            "filename" : CORE_DIR "world/area/void",
            "x_axis" : 55,
            "y_axis" : 5,
        ]),
    ]));
#endif
}
