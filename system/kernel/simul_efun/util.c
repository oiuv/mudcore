/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: util.c
Description: 其它模拟函数
Author: xuefeng
Version: v1.0
Date: 2019-03-12
History:
*****************************************************************************/
#include <ansi.h>

// generate time
varargs string log_time(int timestamp)
{
    if (!timestamp) timestamp = time();

    return CORE_TIME_D->replace_ctime(timestamp);
}

// 获取复制对象的唯一ID
varargs int getcid(object ob)
{
    int id;

    sscanf(file_name(ob || previous_object()), "%*s#%d", id);
    return id;
}

void debug(mixed arg)
{
    string *color = ({HIB, HIC, HIG, HIM, HIR, HIW, HIY});

    if (objectp(arg))
    {
        arg = file_name(arg);
    }

    write(element_of(color) + arg + NOR "\n");
}
