/**
 * @file user_quest.c
 * @author Acme (雪风@mud.ren)
 * @brief 玩家任务管理接口，由玩家对接继承
 * @version 1.0
 * @date 2021-12-04
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <ansi.h>
#include <dbase.h>

mapping toDoList = ([]);
string *solved = ({});

#include <function_compat.h>

mapping get_todo_list();
string *get_solved_quests();
void set_todo(string quest_file);
void mark_solved(string quest_file);
void add_kill_count(string quest_file, string killed_file, int amount);
int get_kill_count(string quest_file, string killed_file);
void add_item_count(string quest_file, string item_file, int amount);
int get_item_count(string quest_file, string item_file);
mapping get_todo(string quest_file);
int is_solved(string quest_file);
void del_todo(string quest_file);
void remove_solved(string quest_file);

private mapping _mudcore_impl_get_todo_list();
mapping getToDoList();
mapping get_todo_list() {
    if (_mudcore_forward_name("get_todo_list", "getToDoList", __FILE__)) { return getToDoList(); }
    return _mudcore_impl_get_todo_list();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
mapping getToDoList() {
    if (_mudcore_forward_name("getToDoList", "get_todo_list", __FILE__)) {
        return get_todo_list();
    }
    return _mudcore_impl_get_todo_list();
}
private mapping _mudcore_impl_get_todo_list() {
    return toDoList;
}

private string *_mudcore_impl_get_solved_quests();
string *getSolved();
string *get_solved_quests() {
    if (_mudcore_forward_name("get_solved_quests", "getSolved", __FILE__)) { return getSolved(); }
    return _mudcore_impl_get_solved_quests();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string *getSolved() {
    if (_mudcore_forward_name("getSolved", "get_solved_quests", __FILE__)) {
        return get_solved_quests();
    }
    return _mudcore_impl_get_solved_quests();
}
private string *_mudcore_impl_get_solved_quests() {
    return solved;
}

private int _mudcore_impl_get_todo_list_size();
int getToDoListSize();
int get_todo_list_size() {
    if (_mudcore_forward_name("get_todo_list_size", "getToDoListSize", __FILE__)) {
        return getToDoListSize();
    }
    return _mudcore_impl_get_todo_list_size();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int getToDoListSize() {
    if (_mudcore_forward_name("getToDoListSize", "get_todo_list_size", __FILE__)) {
        return get_todo_list_size();
    }
    return _mudcore_impl_get_todo_list_size();
}
private int _mudcore_impl_get_todo_list_size() {
    return sizeof(toDoList);
}

private void _mudcore_impl_set_todo(string quest_file);
void setToDo(string quest_file);
void set_todo(string quest_file) {
    if (_mudcore_forward_name("set_todo", "setToDo", __FILE__)) { setToDo(quest_file); return; }
    _mudcore_impl_set_todo(quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
void setToDo(string quest_file) {
    if (_mudcore_forward_name("setToDo", "set_todo", __FILE__)) { set_todo(quest_file); return; }
    _mudcore_impl_set_todo(quest_file);
}
private void _mudcore_impl_set_todo(string quest_file) {
    int i, size;
    mapping kill;
    string *key;

    if (!quest_file || !stringp(quest_file))
        return;

    if (!_mudcore_call_named(quest_file, "is_quest", "isQuest"))
        return;

    if (sizeof(toDoList) >= QUEST_SIZE)
        return;

    if (!undefinedp(toDoList[quest_file]))
        return;

    toDoList[quest_file] = ([]);
    toDoList[quest_file]["killed"] = ([]);
    toDoList[quest_file]["item"] = ([]);

    kill = _mudcore_call_named(quest_file, "get_required_kills", "getKill");
    key = keys(kill);
    size = sizeof(kill);

    for (i = 0; i < size; i++)
        add_kill_count(quest_file, key[i], 0);
}

private void _mudcore_impl_mark_solved(string quest_file);
void setSolved(string quest_file);
void mark_solved(string quest_file) {
    if (_mudcore_forward_name("mark_solved", "setSolved", __FILE__)) {
        setSolved(quest_file); return;
    }
    _mudcore_impl_mark_solved(quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
void setSolved(string quest_file) {
    if (_mudcore_forward_name("setSolved", "mark_solved", __FILE__)) {
        mark_solved(quest_file); return;
    }
    _mudcore_impl_mark_solved(quest_file);
}
private void _mudcore_impl_mark_solved(string quest_file) {
    if (!quest_file || !stringp(quest_file))
        return;

    if (!_mudcore_call_named(quest_file, "is_quest", "isQuest"))
        return;

    // 已經解決就不再加一次
    if (member_array(quest_file, solved) != -1)
        return;

    solved += ({ quest_file });
}

private void _mudcore_impl_add_kill_count(string quest_file, string killed_file, int amount);
void addKilled(string quest_file, string killed_file, int amount);
void add_kill_count(string quest_file, string killed_file, int amount) {
    if (_mudcore_forward_name("add_kill_count", "addKilled", __FILE__)) {
        addKilled(quest_file, killed_file, amount); return;
    }
    _mudcore_impl_add_kill_count(quest_file, killed_file, amount);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
void addKilled(string quest_file, string killed_file, int amount) {
    if (_mudcore_forward_name("addKilled", "add_kill_count", __FILE__)) {
        add_kill_count(quest_file, killed_file, amount); return;
    }
    _mudcore_impl_add_kill_count(quest_file, killed_file, amount);
}
private void _mudcore_impl_add_kill_count(string quest_file, string killed_file, int amount) {
    mapping kill;

    if (!_mudcore_call_named(quest_file, "is_quest", "isQuest"))
        return;

    if (undefinedp(toDoList[quest_file]))
        return;

    kill = _mudcore_call_named(quest_file, "get_required_kills", "getKill");

    if (undefinedp(kill[killed_file]))
        return;

    if (undefinedp(toDoList[quest_file]["killed"][killed_file]))
        toDoList[quest_file]["killed"][killed_file] = 0;

    toDoList[quest_file]["killed"][killed_file] += amount;
}

private int _mudcore_impl_get_kill_count(string quest_file, string killed_file);
int getKilled(string quest_file, string killed_file);
int get_kill_count(string quest_file, string killed_file) {
    if (_mudcore_forward_name("get_kill_count", "getKilled", __FILE__)) {
        return getKilled(quest_file, killed_file);
    }
    return _mudcore_impl_get_kill_count(quest_file, killed_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int getKilled(string quest_file, string killed_file) {
    if (_mudcore_forward_name("getKilled", "get_kill_count", __FILE__)) {
        return get_kill_count(quest_file, killed_file);
    }
    return _mudcore_impl_get_kill_count(quest_file, killed_file);
}
private int _mudcore_impl_get_kill_count(string quest_file, string killed_file) {
    if (!_mudcore_call_named(quest_file, "is_quest", "isQuest"))
        return 0;

    if (undefinedp(toDoList[quest_file]))
        return 0;

    if (undefinedp(toDoList[quest_file]["killed"][killed_file]))
        return 0;

    return toDoList[quest_file]["killed"][killed_file];
}

private void _mudcore_impl_add_item_count(string quest_file, string item_file, int amount);
void addItem(string quest_file, string item_file, int amount);
void add_item_count(string quest_file, string item_file, int amount) {
    if (_mudcore_forward_name("add_item_count", "addItem", __FILE__)) {
        addItem(quest_file, item_file, amount); return;
    }
    _mudcore_impl_add_item_count(quest_file, item_file, amount);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
void addItem(string quest_file, string item_file, int amount) {
    if (_mudcore_forward_name("addItem", "add_item_count", __FILE__)) {
        add_item_count(quest_file, item_file, amount); return;
    }
    _mudcore_impl_add_item_count(quest_file, item_file, amount);
}
private void _mudcore_impl_add_item_count(string quest_file, string item_file, int amount) {
    mapping item;

    if (!_mudcore_call_named(quest_file, "is_quest", "isQuest"))
        return;

    if (undefinedp(toDoList[quest_file]))
        return;

    item = _mudcore_call_named(quest_file, "get_required_items", "getItem");

    if (undefinedp(item[item_file]))
        return;

    if (undefinedp(toDoList[quest_file]["item"][item_file]))
        toDoList[quest_file]["item"][item_file] = 0;

    toDoList[quest_file]["item"][item_file] += amount;
}

private int _mudcore_impl_get_item_count(string quest_file, string item_file);
int getItem(string quest_file, string item_file);
int get_item_count(string quest_file, string item_file) {
    if (_mudcore_forward_name("get_item_count", "getItem", __FILE__)) {
        return getItem(quest_file, item_file);
    }
    return _mudcore_impl_get_item_count(quest_file, item_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int getItem(string quest_file, string item_file) {
    if (_mudcore_forward_name("getItem", "get_item_count", __FILE__)) {
        return get_item_count(quest_file, item_file);
    }
    return _mudcore_impl_get_item_count(quest_file, item_file);
}
private int _mudcore_impl_get_item_count(string quest_file, string item_file) {
    if (!_mudcore_call_named(quest_file, "is_quest", "isQuest"))
        return 0;

    if (undefinedp(toDoList[quest_file]))
        return 0;

    if (undefinedp(toDoList[quest_file]["item"][item_file]))
        return 0;

    return toDoList[quest_file]["item"][item_file];
}

private mapping _mudcore_impl_get_todo(string quest_file);
mapping getToDo(string quest_file);
mapping get_todo(string quest_file) {
    if (_mudcore_forward_name("get_todo", "getToDo", __FILE__)) { return getToDo(quest_file); }
    return _mudcore_impl_get_todo(quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
mapping getToDo(string quest_file) {
    if (_mudcore_forward_name("getToDo", "get_todo", __FILE__)) { return get_todo(quest_file); }
    return _mudcore_impl_get_todo(quest_file);
}
private mapping _mudcore_impl_get_todo(string quest_file) {
    if (!mapp(toDoList) || undefinedp(toDoList[quest_file]))
        return 0;
    return toDoList[quest_file];
}

/* 檢查是否已經解過
       0 : 未解過
       1 : 解過
 */
private int _mudcore_impl_is_solved(string quest_file);
int isSolved(string quest_file);
int is_solved(string quest_file) {
    if (_mudcore_forward_name("is_solved", "isSolved", __FILE__)) { return isSolved(quest_file); }
    return _mudcore_impl_is_solved(quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int isSolved(string quest_file) {
    if (_mudcore_forward_name("isSolved", "is_solved", __FILE__)) { return is_solved(quest_file); }
    return _mudcore_impl_is_solved(quest_file);
}
private int _mudcore_impl_is_solved(string quest_file) {
    return member_array(quest_file, solved || ({})) > -1;
}

private void _mudcore_impl_del_todo(string quest_file);
void delToDo(string quest_file);
void del_todo(string quest_file) {
    if (_mudcore_forward_name("del_todo", "delToDo", __FILE__)) { delToDo(quest_file); return; }
    _mudcore_impl_del_todo(quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
void delToDo(string quest_file) {
    if (_mudcore_forward_name("delToDo", "del_todo", __FILE__)) { del_todo(quest_file); return; }
    _mudcore_impl_del_todo(quest_file);
}
private void _mudcore_impl_del_todo(string quest_file) {
    if (undefinedp(toDoList[quest_file]))
        return;
    map_delete(toDoList, quest_file);
}

private void _mudcore_impl_remove_solved(string quest_file);
void delSolved(string quest_file);
void remove_solved(string quest_file) {
    if (_mudcore_forward_name("remove_solved", "delSolved", __FILE__)) {
        delSolved(quest_file); return;
    }
    _mudcore_impl_remove_solved(quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
void delSolved(string quest_file) {
    if (_mudcore_forward_name("delSolved", "remove_solved", __FILE__)) {
        remove_solved(quest_file); return;
    }
    _mudcore_impl_remove_solved(quest_file);
}
private void _mudcore_impl_remove_solved(string quest_file) {
    if (member_array(quest_file, solved) == -1)
        return;
    solved -= ({ quest_file });
}
