/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: file.c
Description: 文件读取与处理相关模拟函数
Author: xuefeng
Version: v1.0
Date: 2019-03-12
History:
*****************************************************************************/
string *read_lines(string file)
{
    return filter_array(explode(read_file(file), "\n"), (: $1[0]!='#' :));
}

void assure_file(string file)
{
    string path, dir, *dirs;

    if (file_size(file) != -1)
        return;

    dirs = explode(file, "/");

    if (file[strlen(file) - 1] != '/')
        dirs = dirs[0..sizeof(dirs)-2];

    path = "";

    foreach (dir in dirs)
    {
        if (dir == "")
            continue;
        path += "/" + dir;
        switch (file_size(path))
        {
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

void log_file(string file, string text)
{
    assure_file(LOG_DIR + file);
    write_file(LOG_DIR + file, text);
}

void cat(string file)
{
    write(read_file(file));
}

int tail(string path)
{
    if (stringp(path) && file_size(path) >= 0)
    {
        write(read_file(path, file_length(path) - 9));
        return 1;
    }

    return 0;
}
