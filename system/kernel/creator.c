// 自动生成头文件列表
#ifndef INHERIT_DIR
#define INHERIT_DIR "/inherit/"
#endif

#ifndef DAEMON_DIR
#define DAEMON_DIR "/system/daemons/"
#endif

#ifndef STD_DIR
#define STD_DIR "/std/"
#endif

#ifndef INCLUDE_DIR
#define INCLUDE_DIR "/include/"
#endif

string header = @LEAD
/** This program is a part of mudcore
 *-----------------------------------------
 * File   : %^FILENAME%^
 * Author : 由 creator 自动生成
 * Note   : Include File
 *-----------------------------------------
 */
LEAD;

private int valid_file_name(string file)
{
    foreach (int a in file)
    {
        if (!(a >= '0' && a <= '9') && !(a >= 'A' && a <= 'Z') && !(a >= 'a' && a <= 'z') && a != '_')
            return 0;
    }

    return 1;
}

varargs string files(string *files, string prefix)
{
    string sf, file = "", pre = "";

    if (stringp(prefix))
    {
        pre = prefix + "_";
    }

    foreach (string f in files)
    {
        if (f[ < 2.. < 1] != ".c")
            continue;

        sf = f[strsrch(f, "/", -1) + 1.. < 3];

        if (valid_file_name(sf))
            file += sprintf("%s%-40s%s", "#define ", pre + upper_case(sf), " \"" + f + "\"\n");
    }

    return file;
}

private void create_inherit_include()
{
    string file = terminal_colour(header, (["FILENAME":"inherit.h"]));
    string *files = deep_path_list(INHERIT_DIR);

    file += files(files, "");

    write_file(INCLUDE_DIR"inherit.h", file, 1);
}

private void create_daemon_include()
{
    string file = terminal_colour(header, (["FILENAME":"daemon.h"]));
    string *files = deep_path_list(DAEMON_DIR);

    file += files(files);

    write_file(INCLUDE_DIR"daemon.h", file, 1);
}

private void create_std_include()
{
    string file = terminal_colour(header, (["FILENAME":"std.h"]));
    string *files = deep_path_list(STD_DIR);

    file += files(files, "STD");

    write_file(INCLUDE_DIR"std.h", file, 1);
}

void create_all_include()
{
    // inherit.h
    create_inherit_include();

    // daemon.h
    create_daemon_include();

    // std.h
    create_std_include();
}

void create()
{
    create_all_include();
}

string short()
{
    return "头文件自动化系统(CREATOR)";
}
