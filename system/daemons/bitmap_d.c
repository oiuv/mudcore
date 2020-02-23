/*****************************************************************************
Copyright: 2020, Mud.Ren
File name: bitmap_d.c
Description: 点阵文字(bitmap font)显示系统守护进程
Author: xuefeng
Version: v1.1
Date: 2020-02-20
*****************************************************************************/
#define HZK CORE_DIR "system/etc/fonts/hzk"
#define ASC CORE_DIR "system/etc/fonts/asc"
#define DEFAULT_FILL "88"
#define DEFAULT_BG "  "
#define DEFAULT_FCOLOR ""
#define DEFAULT_BGCOLOR ""
#define AUTO_SIZE 12

varargs string show(string str, int size, string fill, string bg, string fcolor, string bgcolor)
{
    int offset;
    int *mask = ({0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1});
    int scale;
    buffer char, bstr = string_encode(str, "GBK");
    string *out;

    if (member_array(size, ({12, 14, 16})) < 0)
        size = AUTO_SIZE;

    out = allocate(size);
    out = map_array(out, (: $1 = "" :));
    size *= 2;

    if (!sizeof(fill)) fill = DEFAULT_FILL;
    if (!sizeof(bg)) bg = DEFAULT_BG;
    if (sizeof(fill) != sizeof(bg))
    {
        fill = DEFAULT_FILL;
        bg = DEFAULT_BG;
    }

    if (!fcolor) fcolor = DEFAULT_FCOLOR;
    if (!bgcolor) bgcolor = DEFAULT_BGCOLOR;

    for (int k = 0; k < sizeof(bstr); k++)
    {
        if (mask[0] & bstr[k])
        {
            offset = size * ((atoi(sprintf("%d", bstr[k])) - 0xA1) * 94 + atoi(sprintf("%d", bstr[k+1])) - 0xA1);
            char = read_buffer(HZK + (size / 2), offset, size);
            k++;
            scale = 2;
        } else {
            offset = bstr[k] * (size / 2);
            char = read_buffer(ASC + (size / 2), offset, (size / 2));
            scale = 1;
        }

        if (!sizeof(char)) error("Can't read bytes from character lib\n");

        for (int i = 0; i < sizeof(char); i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (mask[j] & char[i])
                    out[i / scale] += fcolor + fill;
                else
                    out[i / scale] += bgcolor + bg;
            }
        }

        for (int i = 0; i < sizeof(out); i++)
        {
            out[i] = replace_string(out[i], fill + fcolor + fill, fill + fill);
            out[i] = replace_string(out[i], bg + bgcolor + bg, bg + bg);
        }
    }
    return implode(out, "\n");
}
