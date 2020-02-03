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

void log_file(string file, string text)
{
    write_file(LOG_DIR + file, text);
}


void assure_file(string file)
{
    string path, *dir;
    int i;

    if (file_size(file) != -1) return;

    dir = explode(file, "/");

    if (file[strlen(file) - 1] != '/')
        dir = dir[0..sizeof(dir)-2];

    path = "/";
    for (i = 0; i < sizeof(dir); i++)
    {
        path += dir[i];
        mkdir(path);
        path += "/";
    }
    dir = 0;
}

void cat(string file)
{
    write(read_file(file));
}
