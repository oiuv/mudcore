/*
 * @Author: 雪风@mud.ren
 * @Date: 2022-03-29 15:08:51
 * @LastEditTime: 2022-03-29 15:59:31
 * @LastEditors: 雪风
 * @Description: 自然语法分析指令守护进程
 *  https://bbs.mud.ren
 */
#ifndef VERB_DIR
#define VERB_DIR CORE_DIR "verbs/"
#endif

private mapping Verbs = ([]);
private mapping VerbNames = ([]);
private int reloadGeneration;
private int reloadPending;

#include <function_compat.h>

private mapping _mudcore_impl_get_verbs();
mapping getVerbs();
mapping get_verbs() {
    if (_mudcore_forward_name("get_verbs", "getVerbs", __FILE__)) { return getVerbs(); }
    return _mudcore_impl_get_verbs();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
mapping getVerbs() {
    if (_mudcore_forward_name("getVerbs", "get_verbs", __FILE__)) { return get_verbs(); }
    return _mudcore_impl_get_verbs();
}
private mapping _mudcore_impl_get_verbs() { return copy(Verbs); }
private mixed _mudcore_impl_get_verb(string str);
mixed getVerb(string str);
mixed get_verb(string str) {
    if (_mudcore_forward_name("get_verb", "getVerb", __FILE__)) { return getVerb(str); }
    return _mudcore_impl_get_verb(str);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
mixed getVerb(string str) {
    if (_mudcore_forward_name("getVerb", "get_verb", __FILE__)) { return get_verb(str); }
    return _mudcore_impl_get_verb(str);
}
private mixed _mudcore_impl_get_verb(string str) { return Verbs[str]; }

private string _mudcore_impl_get_error_message(string verb);
string getErrorMessage(string verb);
string get_error_message(string verb) {
    if (_mudcore_forward_name("get_error_message", "getErrorMessage", __FILE__)) {
        return getErrorMessage(verb);
    }
    return _mudcore_impl_get_error_message(verb);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getErrorMessage(string verb) {
    if (_mudcore_forward_name("getErrorMessage", "get_error_message", __FILE__)) {
        return get_error_message(verb);
    }
    return _mudcore_impl_get_error_message(verb);
}
private string _mudcore_impl_get_error_message(string verb) {
    return Verbs[verb] ? _mudcore_call_named(
        Verbs[verb],
        "get_error_message",
        "getErrorMessage"
    ) : 0;
}

private int _mudcore_impl_is_valid_verb_path(string verb);
int getValidVerb(string verb);
int is_valid_verb_path(string verb) {
    if (_mudcore_forward_name("is_valid_verb_path", "getValidVerb", __FILE__)) {
        return getValidVerb(verb);
    }
    return _mudcore_impl_is_valid_verb_path(verb);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int getValidVerb(string verb) {
    if (_mudcore_forward_name("getValidVerb", "is_valid_verb_path", __FILE__)) {
        return is_valid_verb_path(verb);
    }
    return _mudcore_impl_is_valid_verb_path(verb);
}
private int _mudcore_impl_is_valid_verb_path(string verb) {
    return stringp(verb) && !strsrch(verb, VERB_DIR) && strsrch(verb, "..") == -1;
}

private void _mudcore_impl_load_verb_batch(int generation, string *queue, mapping nextNames);
protected void scheduledVerbLoad(int generation, string *queue, mapping nextNames);
protected void load_verb_batch(int generation, string *queue, mapping nextNames) {
    if (_mudcore_forward_name("load_verb_batch", "scheduledVerbLoad", __FILE__)) {
        scheduledVerbLoad(generation, queue, nextNames); return;
    }
    _mudcore_impl_load_verb_batch(generation, queue, nextNames);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected void scheduledVerbLoad(int generation, string *queue, mapping nextNames) {
    if (_mudcore_forward_name("scheduledVerbLoad", "load_verb_batch", __FILE__)) {
        load_verb_batch(generation, queue, nextNames); return;
    }
    _mudcore_impl_load_verb_batch(generation, queue, nextNames);
}
private void _mudcore_impl_load_verb_batch(int generation, string *queue, mapping nextNames) {
    string path, source, name;
    string *names, *synonyms;
    mapping nextVerbs;
    object ob;
    mixed err;
    int count;

    if (generation != reloadGeneration)
        return;
    count = sizeof(queue) > 10 ? 10 : sizeof(queue);
    foreach (path in queue[0..count - 1]) {
        path = lpc_object_path(path);
        map_delete(nextNames, path);
        source = lpc_file(path);
        if (!source)
            continue;
        err = catch {
            if (ob = find_object(path))
                destruct(ob);
            ob = load_object(source);
            names = _mudcore_call_named(ob, "get_verbs", "getVerbs");
            if (!arrayp(names))
                names = ({ explode(path, "/")[<1] });
            synonyms = _mudcore_call_named(ob, "get_synonyms", "getSynonyms");
            if (arrayp(synonyms))
                names += synonyms;
            names = filter(names, (: stringp($1) && $1 != "" :));
            nextNames[path] = names;
        };
        if (err)
            log_file("verbs", path + ": " + err);
    }
    if (generation != reloadGeneration)
        return;
    if (sizeof(queue) > count) {
        call_out("load_verb_batch", 1, generation, queue[count..], nextNames);
        return;
    }
    nextVerbs = ([]);
    foreach (path in sort_array(keys(nextNames), 1)) {
        foreach (name in nextNames[path])
            nextVerbs[name] = path;
    }
    VerbNames = nextNames;
    Verbs = nextVerbs;
    reloadPending = 0;
}

private varargs void _mudcore_impl_reload_verbs(mixed val);
varargs void eventReloadVerbs(mixed val);
varargs void reload_verbs(mixed val) {
    if (_mudcore_forward_name("reload_verbs", "eventReloadVerbs", __FILE__)) {
        eventReloadVerbs(val); return;
    }
    _mudcore_impl_reload_verbs(val);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
varargs void eventReloadVerbs(mixed val) {
    if (_mudcore_forward_name("eventReloadVerbs", "reload_verbs", __FILE__)) {
        reload_verbs(val); return;
    }
    _mudcore_impl_reload_verbs(val);
}
private varargs void _mudcore_impl_reload_verbs(mixed val) {
    string *verbs;
    string dir;
    mapping nextNames;

    if (stringp(val)) {
        if (!is_valid_verb_path(val))
            return;
        verbs = ({ val });
    } else if (arrayp(val)) {
        verbs = filter(val, (: is_valid_verb_path($1) :));
        if (!sizeof(verbs))
            return;
    }
    // 新请求覆盖尚未完成的重载时重新扫描，避免遗失前一批变更。
    if (!arrayp(verbs) || reloadPending) {
        verbs = lpc_source_files(VERB_DIR);
        foreach (dir in get_dir(VERB_DIR) || ({})) {
            dir = VERB_DIR + dir;
            if (file_size(dir) == -2)
                verbs += lpc_source_files(dir);
        }
        nextNames = ([]);
    } else {
        nextNames = copy(VerbNames);
    }
    reloadGeneration++;
    reloadPending = 1;
    remove_call_out("load_verb_batch");
    if (!sizeof(verbs)) {
        VerbNames = ([]);
        Verbs = ([]);
        reloadPending = 0;
        return;
    }
    call_out("load_verb_batch", 1, reloadGeneration, verbs, nextNames);
}

string short() {
    return "谓词指令精灵(VERB_D)";
}

void rehash() {
    reload_verbs();
}

protected void create() {
    Verbs = ([]);
    reload_verbs();
}
