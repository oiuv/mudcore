/*
 * @Author: 雪风@mud.ren
 * @Date: 2022-03-29 10:38:10
 * @LastEditTime: 2022-03-29 11:16:56
 * @LastEditors: 雪风
 * @Description: 文件读取与处理相关模拟函数
 *  https://bbs.mud.ren
 */

// 判断文件是否存在
int file_exists(string file) {
    return (file_size(file) >= 0);
}

string *read_lines(string file) {
    string content;

    if (file_exists(file)) {
        content = read_file(file);
        if (!stringp(content))
            return ({});
        return filter_array(
            map(explode(content, "\n"), (: trim($1) :)),
            (: $1 != "" && $1[0] != '#' && $1[0] != ';' :)
        );
    } else
        error("文件 " + file + " 不存在！");
}

void assure_file(string file) {
    string path, dir, *dirs;

    if (file_size(file) != -1)
        return;

    dirs = explode(file, "/");

    if (file[strlen(file) - 1] != '/')
        dirs = dirs[0..sizeof(dirs) - 2];

    path = "";

    foreach (dir in dirs) {
        if (dir == "")
            continue;
        path += "/" + dir;
        switch (file_size(path)) {
            case -1:
                mkdir(path);
                break;
            case -2:
                continue;
            default:
                return;
        }
    }
}

void log_file(string file, string text, int flag) {
    assure_file(LOG_DIR + file);
    write_file(LOG_DIR + file, text, flag);
}

void cat(string file) {
    write(read_file(file));
}

int tail(string path) {
    if (stringp(path) && file_size(path) >= 0) {
        write(read_file(path, file_length(path) - 9));
        return 1;
    }

    return 0;
}

// 对象名只去掉末尾的源码扩展名，目录中出现的 .c 不受影响。
string lpc_object_path(string path) {
    if (!stringp(path))
        return 0;
    if (strlen(path) > 4 && path[<4..] == ".lpc")
        return path[0..<5];
    if (strlen(path) > 2 && path[<2..] == ".c")
        return path[0..<3];
    return path;
}

mixed lpc_file(string path) {
    if (!stringp(path) || path == "")
        return 0;
    if (lpc_object_path(path) != path)
        return file_exists(path) ? path : 0;
    if (file_exists(path + ".lpc"))
        return path + ".lpc";
    if (file_exists(path + ".c"))
        return path + ".c";
    return file_exists(path) ? path : 0;
}

// 单层目录扫描；与驱动无扩展名加载一致，优先 .lpc 并去重。
string *lpc_source_files(string dir) {
    string *entries, *names;
    string entry, name;

    if (!stringp(dir) || dir == "")
        return ({});
    if (dir[<1] != '/')
        dir += "/";
    entries = get_dir(dir);
    names = ({});
    if (!arrayp(entries))
        return names;
    foreach (entry in entries) {
        name = lpc_object_path(entry);
        if (name != entry && file_size(dir + entry) >= 0 && member_array(name, names) == -1)
            names += ({ name });
    }
    return map(sort_array(names, 1), (: lpc_file($(dir) + $1) :));
}
