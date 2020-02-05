#include <ansi.h>
#include "master/error.c"
#include "master/valid.c"
#include "master/preload.c"

object connect(int port)
{
    object login_ob;
    mixed err;

#ifdef LOGIN_OB
    err = catch (login_ob = new(LOGIN_OB));
#else
    err = catch (login_ob = new(CORE_LOGIN_OB));
#endif

    if (err)
    {
        write("服务器维护中，请稍候……\n");
        destruct(this_object());
    }
    return login_ob;
}

private void create()
{
    debug_message("[" + ctime() + "]CORE_MASTER_OB->create()!");
}

string creator_file(string str)
{
    if (DEBUG)
        debug_message("[CORE_MASTER_OB]->creator_file():" + str);
    return (string)call_other(__DIR__ "simul_efun", "creator_file", str);
}

string domain_file(string str)
{
    if (DEBUG)
        debug_message("[CORE_MASTER_OB]->domain_file():" + str);
    return (string)call_other(__DIR__ "simul_efun", "domain_file", str);
}

string author_file(string str)
{
    if (DEBUG)
        debug_message("[CORE_MASTER_OB]->author_file():" + str);
    return (string)call_other(__DIR__ "simul_efun", "author_file", str);
}

string get_bb_uid()
{
    debug_message("[" + ctime() + "]CORE_MASTER_OB->get_bb_uid()!");
    return BACKBONE_UID;
}

string get_root_uid()
{
    debug_message("[" + ctime() + "]CORE_MASTER_OB->get_root_uid()!");
    return ROOT_UID;
}

// 可以在这里自定义不同文件的include目录
mixed get_include_path(string object_path)
{
    if (DEBUG)
        debug_message("[CORE_MASTER_OB]->get_include_path():" + object_path);

    return ({":DEFAULT:"});
}

// apply: driver挂掉时呼叫此方法
void crash(string crash_message, object command_giver, object current_object)
{
    foreach (object ob in users())
        tell_object(ob, "[CORE_MASTER_OB]->crash():Damn!\nThe game is crashing.\n");
}

// driver 启动测试
void flag(string str)
{
    switch (str)
    {
    case "debug":
        DEBUG = 1;
        break;
    default:
        write(HIR "[CORE_MASTER_OB]->flag():The only supproted flag is 'debug', got '" + str + "'.\n" NOR);
    }
    // otherwise wait for auto shutdown
}

// compile_object: This is used for loading MudOS "virtual" objects.
// It should return the object the mudlib wishes to associate with the
// filename named by 'file'.  It should return 0 if no object is to be
// associated.
mixed compile_object(string str)
{
    if (DEBUG)
        debug_message("[CORE_MASTER_OB]->compile_object():" + str);

#ifdef WORLD_DIR
    if (sscanf(str, WORLD_DIR + "%*s", str))
    {
        return call_other(CORE_VIRTUAL_D, "compile_area", str);
    }
#endif

#ifdef MOB_DIR
    if (sscanf(str, MOB_DIR + "%*s", str))
    {
        return call_other(CORE_VIRTUAL_D, "compile_mob", str);
    }
#endif

    return 0;
}

string object_name(object ob)
{
    if (DEBUG)
        debug_message("[CORE_MASTER_OB]->object_name():" + file_name(ob));
    if (ob)
        return ob->short();
    return "未知对象";
}

// mapping get_mud_stats( void ) {};

// string get_save_file_name( string ) {};
// string make_path_absolute( string rel_path ) {};
// int retrieve_ed_setup( object user );
// int save_ed_setup( object user, int config );
