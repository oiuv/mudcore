// 自动生成头文件列表
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
#ifdef INHERIT_DIR
    string file = terminal_colour(header, (["FILENAME":"_inherit.h"]));
    string *files = deep_path_list(INHERIT_DIR);

    file += files(files, "");

    write_file(INCLUDE_DIR "_inherit.h", file, 1);
#endif
}

private void create_daemon_include()
{
#ifdef DAEMON_DIR
    string file = terminal_colour(header, (["FILENAME":"_daemon.h"]));
    string *files = deep_path_list(DAEMON_DIR);

    file += files(files);

    write_file(INCLUDE_DIR "_daemon.h", file, 1);
#endif
}

private void create_std_include()
{
#ifdef STD_DIR
    string file = terminal_colour(header, (["FILENAME":"_std.h"]));
    string *files = deep_path_list(STD_DIR);

    file += files(files, "STD");

    write_file(INCLUDE_DIR "_std.h", file, 1);
#endif
}

void create_all_include()
{
    // _inherit.h
    create_inherit_include();

    // _daemon.h
    create_daemon_include();

    // _std.h
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
