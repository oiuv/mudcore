// unicode字符
inherit CORE_CLEAN_UP;

int main(object me, string arg)
{
    int x = atoi(arg);

    if (x < 1 || x > 1114109)
    {
        for (int i = 32; i < 127; i++)
        {
            printf("%d(%X) = %c\n", i, i, i);
        }
    }
    else
    {
        catch(printf("%d(%X) = %c\n", x, x, x));
    }

    return 1;
}

int help(object me)
{
    if (!wizardp(me))
        return 0;

    write(@TEXT
指令格式: unicode [code]
指令说明:
    显示指定编码的 unicode 字符，code 为字符的十进制编码(范围：1~1114109)，如：unicode 128050
TEXT
    );
    return 1;
}
