/*****************************************************************************
Copyright: 2022, Mud.Ren
File name: login.c
Description: 登录示例对象 LOGIN_OB，可继承使用
Author: xuefeng
Version: v1.1
Date: 2022-11-11
*****************************************************************************/
inherit _DBASE;
inherit _SAVE;

void time_out();
string query_save_file();

void login()
{
    if (interactive(this_object()))
        set_temp("ip_number", query_ip_number(this_object()));

    LOGIN_D->login(this_object());
}

void logon()
{
    call_out("time_out", 60);
    // 延迟登陆，解决mudlet初次连接乱码问题
    call_out_walltime("login", 0.25);
}

void net_dead()
{
    debug_message(this_object() + " net_dead!");
    remove_call_out("time_out");
    call_out("time_out", 1);
}

void terminal_type(string term)
{
    set_temp("terminal_type", term);
}

/**
 * 以下为自定义方法
 */

void time_out()
{
    if (objectp(query_temp("user_ob")))
    {
        return;
    }
    if(interactive(this_object()))
        write("\n您花在登录的时间太久了，下次想好再来吧。(-_-)!\n");
    destruct(this_object());
}

string query_save_file()
{
    string id;

    id = query("id", 1);
    if (! stringp(id)) return 0;
    return sprintf(DATA_DIR "login/%s", id);
}
