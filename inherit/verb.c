private nosave string Verb, ErrorMessage;
private nosave string *Synonyms, *Rules;

#include <function_compat.h>

protected void create() {
    parse_init();
    Verb = 0;
    ErrorMessage = 0;
    Rules = ({});
    Synonyms = ({});
}

private string _mudcore_impl_set_error_message(string str);
protected string setErrorMessage(string str);
protected string set_error_message(string str) {
    if (_mudcore_forward_name("set_error_message", "setErrorMessage", __FILE__)) {
        return setErrorMessage(str);
    }
    return _mudcore_impl_set_error_message(str);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected string setErrorMessage(string str) {
    if (_mudcore_forward_name("setErrorMessage", "set_error_message", __FILE__)) {
        return set_error_message(str);
    }
    return _mudcore_impl_set_error_message(str);
}
private string _mudcore_impl_set_error_message(string str) { return (ErrorMessage = str); }

private string _mudcore_impl_get_error_message();
string getErrorMessage();
string get_error_message() {
    if (_mudcore_forward_name("get_error_message", "getErrorMessage", __FILE__)) {
        return getErrorMessage();
    }
    return _mudcore_impl_get_error_message();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getErrorMessage() {
    if (_mudcore_forward_name("getErrorMessage", "get_error_message", __FILE__)) {
        return get_error_message();
    }
    return _mudcore_impl_get_error_message();
}
private string _mudcore_impl_get_error_message() { return ErrorMessage; }

private varargs string *_mudcore_impl_set_rules(mixed *args...);
varargs protected string *setRules(mixed *args...);
varargs protected string *set_rules(mixed *args...) {
    if (_mudcore_forward_name("set_rules", "setRules", __FILE__)) { return setRules(args...); }
    return _mudcore_impl_set_rules(args...);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
varargs protected string *setRules(mixed *args...) {
    if (_mudcore_forward_name("setRules", "set_rules", __FILE__)) { return set_rules(args...); }
    return _mudcore_impl_set_rules(args...);
}
private varargs string *_mudcore_impl_set_rules(mixed *args...) {
    if (sizeof(Rules))
        error("Cannot reset rules list.");
    foreach (mixed arg in args) {
        if (stringp(arg))
            Rules += ({ arg });
        else
            Rules += arg;
    }
    if (Verb) {
        foreach (string rule in Rules)
            parse_add_rule(Verb, rule);
        if (sizeof(Synonyms))
            foreach (string cmd in Synonyms)
                parse_add_synonym(cmd, Verb);
    }
    return Rules;
}

private string *_mudcore_impl_get_rules();
string *getRules();
string *get_rules() {
    if (_mudcore_forward_name("get_rules", "getRules", __FILE__)) { return getRules(); }
    return _mudcore_impl_get_rules();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string *getRules() {
    if (_mudcore_forward_name("getRules", "get_rules", __FILE__)) { return get_rules(); }
    return _mudcore_impl_get_rules();
}
private string *_mudcore_impl_get_rules() { return copy(Rules); }

private varargs string *_mudcore_impl_set_synonyms(mixed *args...);
varargs protected string *setSynonyms(mixed *args...);
varargs protected string *set_synonyms(mixed *args...) {
    if (_mudcore_forward_name("set_synonyms", "setSynonyms", __FILE__)) {
        return setSynonyms(args...);
    }
    return _mudcore_impl_set_synonyms(args...);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
varargs protected string *setSynonyms(mixed *args...) {
    if (_mudcore_forward_name("setSynonyms", "set_synonyms", __FILE__)) {
        return set_synonyms(args...);
    }
    return _mudcore_impl_set_synonyms(args...);
}
private varargs string *_mudcore_impl_set_synonyms(mixed *args...) {
    if (sizeof(Synonyms))
        error("Cannot reset synonym list.\n");
    foreach (mixed arg in args) {
        if (stringp(arg))
            Synonyms += ({ arg });
        else
            Synonyms += arg;
    }
    if (Verb && sizeof(Rules))
        foreach (string cmd in Synonyms)
            parse_add_synonym(cmd, Verb);
    return Synonyms;
}

private string *_mudcore_impl_get_synonyms();
string *getSynonyms();
string *get_synonyms() {
    if (_mudcore_forward_name("get_synonyms", "getSynonyms", __FILE__)) { return getSynonyms(); }
    return _mudcore_impl_get_synonyms();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string *getSynonyms() {
    if (_mudcore_forward_name("getSynonyms", "get_synonyms", __FILE__)) { return get_synonyms(); }
    return _mudcore_impl_get_synonyms();
}
private string *_mudcore_impl_get_synonyms() { return copy(Synonyms); }

private string _mudcore_impl_set_verb(string str);
protected string setVerb(string str);
protected string set_verb(string str) {
    if (_mudcore_forward_name("set_verb", "setVerb", __FILE__)) { return setVerb(str); }
    return _mudcore_impl_set_verb(str);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected string setVerb(string str) {
    if (_mudcore_forward_name("setVerb", "set_verb", __FILE__)) { return set_verb(str); }
    return _mudcore_impl_set_verb(str);
}
private string _mudcore_impl_set_verb(string str) {
    if (!stringp(str))
        error("Bad argument 1 to set_verb().\n");
    Verb = str;
    if (sizeof(Rules))
        foreach (string rule in Rules)
            parse_add_rule(Verb, rule);
    if (sizeof(Synonyms))
        foreach (string cmd in Synonyms)
            parse_add_synonym(cmd, Verb);
    return Verb;
}

private string _mudcore_impl_get_verb();
string getVerb();
string get_verb() {
    if (_mudcore_forward_name("get_verb", "getVerb", __FILE__)) { return getVerb(); }
    return _mudcore_impl_get_verb();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getVerb() {
    if (_mudcore_forward_name("getVerb", "get_verb", __FILE__)) { return get_verb(); }
    return _mudcore_impl_get_verb();
}
private string _mudcore_impl_get_verb() { return Verb; }

private string *_mudcore_impl_get_verbs();
string *getVerbs();
string *get_verbs() {
    if (_mudcore_forward_name("get_verbs", "getVerbs", __FILE__)) { return getVerbs(); }
    return _mudcore_impl_get_verbs();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string *getVerbs() {
    if (_mudcore_forward_name("getVerbs", "get_verbs", __FILE__)) { return get_verbs(); }
    return _mudcore_impl_get_verbs();
}
private string *_mudcore_impl_get_verbs() { return ({ Verb }); }
