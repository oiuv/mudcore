/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: name.c
Description: 游戏对象名称设置接口
Author: xuefeng
Version: v1.0
Date: 2019-03-15
History:
*****************************************************************************/
#include <dbase.h>

nosave string *my_id;

// void create(){}

// function called by present() in order to identify an object
int id(string arg)
{
    if (!arg)
        return 0;

    if (my_id && member_array(arg, my_id) != -1)
        return 1;
    else
        return query("id") == arg;
}

varargs void set_name(string name, string *id)
{
    if (stringp(name))
    {
        set("name", name);
    }
    else
    {
        set("name", "无名氏");
    }

    if (pointerp(id))
    {
        set("id", lower_case(id[0]));
        my_id = id;
        // 非玩家对象增加首字母ID，不可以使用 userp() 判断
        if (!this_object()->is_user())
        {
            my_id += ({lower_case(id[0][0..0])});
        }
    }
}

string name()
{
    string str;
    if (stringp(str = query("name")))
        return str;
    else
        return file_name(this_object());
}

string short()
{
    string str;
    if (!stringp(str = query("short")))
        str = name() + (query("id") ? "(" + capitalize(query("id")) + ")" : "");

    return str;
}

string long()
{
    string str, extra;

    if (!stringp(str = query("long")))
        str = short() + "\n";
    if (stringp(extra = this_object()->extra_long()))
        str += extra;
    return str;
}

string *id_list()
{
    return my_id;
}
