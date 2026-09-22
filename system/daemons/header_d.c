// 自动生成头文件列表
#ifndef INCLUDE_DIR
#define INCLUDE_DIR "/include/"
#endif

string header = @LEAD
/** This program is a part of mudcore
 *-----------------------------------------
 * File   : %^FILENAME%^
 * Author : MUDCORE
 * Note   : Include File，由 creator 自动生成
 * Update : %^CTIME%^
 *-----------------------------------------
 */
LEAD;

int valid_file_name(string file) {
    foreach (int a in file) {
        if (!(a >= '0' && a <= '9') && !(a >= 'A' && a <= 'Z') && !(a >= 'a' && a <= 'z') && a != '_')
            return 0;
    }

    return 1;
}

varargs string files(string *files, string prefix) {
    string sf, file = "", pre = "";
    mapping seen = ([]);

    if (stringp(prefix)) {
        pre = prefix + "_";
    }

    foreach (string f in files) {
        if (lpc_object_path(f) == f || seen[lpc_object_path(f)])
            continue;
        seen[lpc_object_path(f)] = 1;

        sf = explode(lpc_object_path(f), "/")[<1];

        if (valid_file_name(sf))
            file += sprintf(
                "%s%-40s%s",
                "#define ",
                pre + upper_case(sf),
                " \"" + lpc_object_path(f) + "\"\n"
            );
    }

    return file;
}

varargs void create_include(string base_name, string dir, string prefix) {
    string filename = "_" + base_name + ".h";
    string file = terminal_colour(header, ([ "FILENAME": filename, "CTIME": ctime() ]));
    string *files = deep_path_list(dir);

    file += files(files, prefix);

    write_file(INCLUDE_DIR + filename, file, 1);
    write(INCLUDE_DIR + filename + " 已生成 💚\n");
}

void create_inherit_include() {
#ifdef INHERIT_DIR
    create_include("inherit", INHERIT_DIR, "");
#endif
}

void create_daemon_include() {
#ifdef DAEMON_DIR
    create_include("daemon", DAEMON_DIR);
#endif
}

void create_std_include() {
#ifdef STD_DIR
    create_include("std", STD_DIR, "STD");
#endif
}

void create_all_include() {
    // _inherit.h
    create_inherit_include();

    // _daemon.h
    create_daemon_include();

    // _std.h
    create_std_include();
}

void create() {
    // create_all_include();
}

string short() {
    return "头文件自动化系统(HEADER_D)";
}
