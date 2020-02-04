/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: npc.c
Description: 非玩家角色公共接口：NPC/魔物等
Author: xuefeng
Version: v1.0
Date: 2019-03-14
*****************************************************************************/
inherit CORE_CHAR;
inherit CORE_CLEAN_UP;

/**
 * reset时回到起始环境
 */
int return_home(object home)
{
    object ob = this_object();
    if (!environment() || environment() == home)
        return 1;

    if (!living(ob) || ob->is_fighting() || !mapp(environment()->query("exits")))
        return 0;

    message("vision", ob->name() + "急急忙忙地离开了。\n", environment(), ob);

    return move(home);
}

/**
 * 非战斗状态的行为
 */

// 随机移动
void random_move()
{
    mapping exits;
    string *dirs;

    if (!mapp(exits = environment()->query("exits")) || !sizeof(exits))
        return 0;
    dirs = keys(exits);
    command("go " + dirs[element_of(dirs)]);
}

/**
 * NPC行为控制
 */
void action()
{

}
