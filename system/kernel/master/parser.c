/*
 * @Author: 雪风@mud.ren
 * @Date: 2022-03-29 11:30:45
 * @LastEditTime: 2022-03-29 19:37:12
 * @LastEditors: 雪风
 * @Description: Natural Language Parser
 *  https://bbs.mud.ren
 */
#include <parser_error.h>

string parse_command_all_word() { return "all"; }

string *parse_command_prepos_list()
{
    // debug_message("parse_command_prepos_list");
    return ({"at", "in", "inside", "on"});
}

string parser_error_message(int type, object ob, mixed arg, int flag)
{
    // debug_message(sprintf("parser_error_message : type = %d, ob = %O, arg = %O, flag = %d", type, ob, arg, flag));
    return arg;
}
