/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: command.c
Description: 角色指令控制接口
Author: xuefeng
Version: v1.0
Date: 2019-03-12
*****************************************************************************/
#include <dbase.h>

/**
 * apply函数：如果用户对象中有process_input()，ＭＵＤＯＳ会将玩家所有输入传入这里
 * 如果本方法傳回一個字串，則此字串就取代使用者原先輸入的資料，用於往
 * 後的處理。如果本方法傳回一個非零、非字串的值，就不會再作處理。如果
 * 傳回零，則以原來輸入的資料作處理。在此之後才會看使用者輸入是否符合
 * add_actions 當時添加的動詞。
 */
mixed process_input(string arg) {
    return COMMAND_D->default_alias(arg);
}

private nosave string *commandHandlers;

protected string *query_command_handlers() {
#if MUDCORE_ENABLE_PARSER
    return ({ "exit", "command", "emote", "channel", "parser" });
#else
    return ({ "exit", "command", "emote", "channel" });
#endif
}

private string *validatedHandlers() {
    mixed handlers, handler;
    string *known, *seen;

    handlers = query_command_handlers();
    known = ({ "exit", "command", "emote", "channel", "parser" });
    seen = ({});
    if (!arrayp(handlers)) error("COMMAND: handlers must be an array.\n");
    foreach (handler in handlers) {
        if (!stringp(handler) || member_array(handler, known) == -1)
            error("COMMAND: unknown handler.\n");
        if (member_array(handler, seen) != -1)
            error("COMMAND: duplicate handler: " + handler + ".\n");
#if !MUDCORE_HAS_PARSER
        if (handler == "parser") error("COMMAND: parser is disabled or unavailable.\n");
#endif
        seen += ({ handler });
    }
    return seen;
}

protected mixed handle_exit(object actor, string verb, string arg) {
    return !arg && objectp(environment()) && environment()->query("exits/" + verb) && command("go " + verb);
}

protected mixed handle_action_command(object actor, string verb, string arg) {
    object file;

    file = COMMAND_D->find_command(query_verb());
    return objectp(file) && call_other(file, "main", actor, arg) ? 1 : 0;
}

protected mixed handle_emote(object actor, string verb, string arg) {
    return EMOTE_D->do_emote(actor, verb, arg) ? 1 : 0;
}

protected mixed handle_channel(object actor, string verb, string arg) {
    return CHANNEL_D->do_channel(actor, verb, arg) ? 1 : 0;
}

protected mixed handle_parser(object actor, string verb, string arg) {
#if MUDCORE_HAS_PARSER
    mixed result;

    result = parse_sentence(arg ? verb + " " + arg : verb, 0);
    return intp(result) ? result == 1 : result;
#else
    error("COMMAND: parser is disabled or unavailable.\n");
#endif
}

nomask int command_hook(string arg) {
    string verb, handler;
    object actor;
    mixed result;

    actor = this_object();
    verb = trim(query_verb());
    if (verb == "") return 0;
    if (!arrayp(commandHandlers)) commandHandlers = validatedHandlers();
    foreach (handler in commandHandlers) {
        switch (handler) {
            case "exit": result = handle_exit(actor, verb, arg); break;
            case "command": result = handle_action_command(actor, verb, arg); break;
            case "emote": result = handle_emote(actor, verb, arg); break;
            case "channel": result = handle_channel(actor, verb, arg); break;
            case "parser": result = handle_parser(actor, verb, arg); break;
        }
        if (stringp(result)) return notify_fail(result);
        if (result == 1) return 1;
        if (result != 0) error("COMMAND: handler must return 0, 1 or an error string.\n");
    }
    return 0;
}

nomask void enable_living() {
    object me = this_object();
    string *handlers;
    mixed err;

    if (!query_temp("living")) handlers = validatedHandlers();

    if (stringp(query("id")))
        set_living_name(query("id"));
    else
        set_living_name(query("name"));

    if (!query_temp("living")) {
        enable_commands();
        err = catch {
#if MUDCORE_HAS_PARSER
            if (member_array("parser", handlers) != -1) parse_init();
#endif
            commandHandlers = handlers;
            set_temp("living", 1);
            add_action("command_hook", "", 1);
        };
        if (err) {
            disable_commands();
            delete_temp("living");
            commandHandlers = 0;
            error(err);
        }
    }

    if (userp(me) && getuid(me) == WIZARD) {
        enable_wizard();
    }
}

nomask void disable_living(string type) {
    if (query_temp("living")) {
        disable_commands();
        delete_temp("living");
        commandHandlers = 0;
    }
}
