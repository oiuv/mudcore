// error.c
#include <runtime_config.h>
/**
 * @brief 日志记录
 *
 * @param file
 * @param message
 */
void log_error(string file, string message)
{
    if (strsrch(message, "Warning") == -1)
    {
        if (this_player(1))
        {
            if (wizardp(this_player(1)))
                efun::write("编译时段错误：" + message + "\n");
            else
                efun::write(get_config(__DEFAULT_ERROR_MESSAGE__) + "\n");
        }
        // 记录错误日志
        log_file("log_error", message);
    }
    else
    {
        // 记录警告日志
        log_file("log", message);
    }
}

/**
 * @brief 编译错误追踪方法
 *
 * @param map
 * @param flag
 */
void error_handler(mapping map, int flag)
{
    string str = "[" + ctime() + "]";

    str += sprintf("\nError: %s\nProgram: %s\nObject: %O\nFile: %s - Line: %d\n[%s]\n",
                   map["error"], (map["program"] || "No program"),
                   (map["object"] || "No object"),
                   map["file"],
                   map["line"],
                   implode(map_array(map["trace"],
                                        (: sprintf("\n\tProgram: %s\n\tObject: %O \n\tFile: %s\n\tFunction : %s\n\tLine: %d\n", $1["program"], $1["object"], $1["file"], $1["function"], $1["line"]) :)
                                    ), "\n"));
    str += sprintf("%O\n\n", map);

    if (flag)
        printf("%s", map["error"]);
    else
        debug("出错啦！详情记录在 error_handler 日志。");

    log_file("error_handler", str);
}
