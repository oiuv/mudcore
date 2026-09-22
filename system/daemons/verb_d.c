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

mapping getVerbs() { return copy(Verbs); }
mixed getVerb(string str) { return Verbs[str]; }

string getErrorMessage(string verb) {
    return Verbs[verb] ? Verbs[verb]->getErrorMessage() : 0;
}

int getValidVerb(string verb) {
    return stringp(verb) && !strsrch(verb, VERB_DIR) && strsrch(verb, "..") == -1;
}

protected void scheduledVerbLoad(int generation, string *queue, mapping nextNames) {
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
            names = ob->getVerbs();
            if (!arrayp(names))
                names = ({ explode(path, "/")[<1] });
            synonyms = ob->getSynonyms();
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
        call_out("scheduledVerbLoad", 1, generation, queue[count..], nextNames);
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

varargs void eventReloadVerbs(mixed val) {
    string *verbs;
    string dir;
    mapping nextNames;

    if (stringp(val)) {
        if (!getValidVerb(val))
            return;
        verbs = ({ val });
    } else if (arrayp(val)) {
        verbs = filter(val, (: getValidVerb($1) :));
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
    remove_call_out("scheduledVerbLoad");
    if (!sizeof(verbs)) {
        VerbNames = ([]);
        Verbs = ([]);
        reloadPending = 0;
        return;
    }
    call_out("scheduledVerbLoad", 1, reloadGeneration, verbs, nextNames);
}

string short() {
    return "谓词指令精灵(VERB_D)";
}

void rehash() {
    eventReloadVerbs();
}

protected void create() {
    Verbs = ([]);
    eventReloadVerbs();
}
