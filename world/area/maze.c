// 迷宫
#include <ansi.h>
/**
 * @brief 迷宫类地图示例环境
 *
 */
inherit CORE_VRM;

void create()
{
    //迷宫房间所继承的对象的档案名称
    set_inherit_room( CORE_STD_ROOM );

    //迷宫房间里的怪物
    // set_maze_npcs(({}));

    //迷宫的单边长
    set_maze_long(10);

    //入口方向(出口在对面)
    set_entry_dir("south");

    //入口与区域的连接方向
    set_link_entry_dir("down");

    //入口与区域的连接档案名
    set_link_entry_room(VOID_OB);

    //出口与区域的连接方向
    set_link_exit_dir("enter");

    //出口与区域的连接档案名
    set_link_exit_room(__DIR__"tower");

    //入口房间短描述
    set_entry_short(HIB "魔宫入口" NOR);

    //入口房间描述
    set_entry_desc(HIB @LONG
这里是人类绝望和憎恶情绪映射到现实世界而异化出的魔宫，进入其中很容易变得绝望并充满憎恶的情绪。
LONG NOR);

    //出口房间短描述
    set_exit_short(HIB "魔宫出口" NOR);

    //出口房间描述
    set_exit_desc(HIB @LONG
你眼前一亮，深深的吸了口气，心想总算是出来了。不过景色忽的一变，眼前出现一座高耸入云的魔法塔。
LONG NOR);

    //迷宫房间的短描述
    set_maze_room_short(HIB "绝望与憎恶魔宫" NOR);

    //迷宫房间的描述，如果有多条描述，制造每个房间的时候会从中随机选择一个。
    set_maze_room_desc(HIB @LONG
这里是人类绝望和憎恶情绪映射到现实世界而异化出的魔宫，在这里呆久了很容易迷失自我，变得绝望并充满憎恶的情绪。
LONG NOR);

    // 迷宫房间是否为户外房间？
    // set_outdoors(1);

    // 是否绘制迷宫地图?
    set_maze_map(1);

    // 迷宫的额外参数
    set_extra_info(([
        "area" : "maze",
        "zone/z" : 1,
    ]));
}
