/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: atoi.c
Description: ASCII和数字转换模拟函数
Author: xuefeng
Version: v1.0
Date: 2019-03-12
*****************************************************************************/

int atoi(string str)
{
    int v;

    if (! stringp(str) || ! sscanf(str, "%d", v))
        return 0;

    return v;
}

string itoc(int c)
{
    return sprintf("%c", c);
}
