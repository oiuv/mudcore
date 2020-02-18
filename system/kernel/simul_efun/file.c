/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: file.c
Description: 文件读取与处理相关模拟函数
Author: xuefeng
Version: v1.0
Date: 2019-03-12
History:
*****************************************************************************/
string *update_file(string file)
{
    string *arr;
    string str;
    int i;

    str = read_file(file);
    if (!str) {
        return ({});
    }
    arr = explode(str, "\n");
    for (i = 0; i < sizeof(arr); i++) {
        if (arr[i][0] == '#') {
            arr[i] = 0;
        }
    }
    arr -= ({ 0 });
    return arr;
}

void assure_file(string file)
{
    string path, dir, *dirs;

    if (file_size(file) != -1) return;

    dirs = explode(file, "/");

    if (file[strlen(file) - 1] != '/')
        dirs = dirs[0..sizeof(dirs)-2];

    path = "";

    foreach(dir in dirs)
    {
        if (dir == "") continue;
        path += "/" + dir;
        switch(file_size(path))
        {
            case -1 : mkdir(path); break;
            case -2 : continue;
            default : return;
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
