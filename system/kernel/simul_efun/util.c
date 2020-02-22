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

// 进度条
string process_bar(int n)
{
    string sp;
    int start;

    if (n < 0)
        n = 0;
    else if (n > 100)
        n = 100;

    sp = "                                                  " NOR;

    if (n == 100)
    {
        sp[22] = '1';
        sp[23] = '0';
        sp[24] = '0';
        sp[25] = '%';
        start = 22;
    }
    else if (n >= 10)
    {
        sp[23] = (n / 10) + '0';
        sp[24] = (n % 10) + '0';
        sp[25] = '%';
        start = 23;
    }
    else
    {
        sp[24] = n + '0';
        sp[25] = '%';
        start = 24;
    }

    n /= 2;
    if (start > n)
    {
        sp = sp[0..start-1] + HIY + sp[start..<0];
        sp = sp[0..n - 1] + BBLU + sp[n..<0];
    }
    else
    {
        sp = sp[0..n - 1] + BBLU + sp[n..<0];
        sp = sp[0..start-1] + HIY + sp[start..<0];
    }

    sp = NOR + BCYN + sp;
    return sp;
}
