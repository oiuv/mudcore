/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: charset.c
Description: 字符集相关模拟函数
Author: xuefeng
Version: v1.0
Date: 2019-03-31
*****************************************************************************/

int is_chinese(string str)
{
    if (!str)
        return 0;

    str = remove_ansi(str);

    return pcre_match(str, "^\\p{Han}+$");
}

int is_english(string str)
{
    if( !str || !str[0] )
        return 0;

    foreach( int a in str )
        if((a < 'a' || a > 'z') && (a < 'A' || a > 'Z'))
            return 0;

    return 1;
}

varargs string break_string(string str, int len, mixed indent)
{
    string indent_str;
    int br = 0;

    if (!stringp(str)) return 0;

    if (!len) len = 40;

    if(indent && intp(indent)) indent_str = sprintf("%*s", indent, " ");
    else if(stringp(indent)) indent_str = indent;
    else indent_str = "";

    str = indent_str + replace_string(str, "\n", "");

    while(strlen(str[br..]) > len) {
        int en = 0;
        for(int i = 0; i < len; i++) {
            if( str[br + i] < 127 )
            {
                en++;
            }
        }
        br += len + en / 2;

        str = str[0..br-1] + "\n" + str[br..<1];
    }

    return str;
}

int check_control(string name)
{
    int i;
    if (!name)
        return 0;

    i = strlen(name);
    while (i--)
    {
        if (name[i] == ' ' || name[i] == '\n')
            continue;

        if (name[i] < ' ')
            return 1;
    }
    return 0;
}

int check_space(string name)
{
    int i;
    if (!name)
        return 0;

    i = strlen(name);
    while (i--)
    {
        if (name[i] == ' ')
            return 1;
    }
    return 0;
}

int check_return(string name)
{
    int i;
    if (!name)
        return 0;

    i = strlen(name);
    while (i--)
    {
        if (name[i] == '\n')
            return 1;
    }
    return 0;
}

// 中文数字
string chinese_number(int i)
{
    return CORE_CHINESE_D->chinese_number(i);
}
// 英文转中文
string chinese(string str)
{
    return CORE_CHINESE_D->chinese(str);
}
