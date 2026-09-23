// Deliberately generate legacy host APIs; production/tests LPC use snake_case.
import { mkdirSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';
import { aliases } from './function-names.mjs';

export function prepareNamingFixtures(sandbox) {
    const put = (file, source) => writeFileSync(join(sandbox, 'tests', `naming_${file}.lpc`), source);
    // A cached pre-migration parent, isolated from the real framework and host.
    const reloadDir = join(sandbox, 'fixtures', '.naming_reload');
    mkdirSync(reloadDir, { recursive: true });
    writeFileSync(join(reloadDir, 'verb.c'), `
        protected void create() {}
        protected string setVerb(string value) { return value; }
        protected string setErrorMessage(string value) { return value; }
        protected varargs string *setSynonyms(mixed *args...) { return ({}); }
        protected varargs string *setRules(mixed *args...) { return ({}); }
    `);
    writeFileSync(join(reloadDir, 'legacy.c'), `
        inherit "/fixtures/.naming_reload/verb";
        string configure() { return setVerb("legacy"); }
    `);
    for (const verb of ['look', 'go']) {
        writeFileSync(join(reloadDir, `${verb}.c`),
            '#undef _VERB\n#define _VERB "/fixtures/.naming_reload/verb"\n' +
            `#include "/mudcore/verbs/common/${verb}.c"\n`);
    }
    const inventory = [];
    for (const [file, names] of Object.entries(aliases)) {
        let program = '/mudcore/' + file.replace(/\.(c|lpc)$/, '');
        if (file.startsWith('system/kernel/master/')) program = '/mudcore/system/kernel/master';
        if (file.startsWith('system/kernel/simul_efun/')) program = '/mudcore/system/kernel/simul_efun';
        for (const [old, current] of Object.entries(names)) inventory.push([program, old, current]);
    }
    writeFileSync(join(sandbox, 'naming-inventory.json'), JSON.stringify(inventory));
    for (const mode of ['old', 'new', 'both']) {
        let room = 'inherit CORE_ROOM;\n';
        for (const [variant, name] of [['old', 'setArea'], ['new', 'set_area']]) {
            if (mode !== variant && mode !== 'both') continue;
            room += `varargs void ${name}(mixed area, int x, int y, int z) {
                add_temp("${variant}", 1);
                ::${name}(area, x, y, z);
            }\n`;
        }
        put(`room_${mode}`, room);
    }
    put('room_leaf', 'inherit "/tests/naming_room_old";\n');
    for (const [parent, leaf, method] of [['old', 'new', 'set_area'], ['new', 'old', 'setArea']]) {
        put(`room_${parent}_${leaf}`, `inherit "/tests/naming_room_${parent}";
            varargs void ${method}(mixed area, int x, int y, int z) {
                add_temp("${leaf}", 1);
                ::${method}(area, x, y, z);
            }
        `);
    }
    put('quest_unmarked', 'void register_information() { error("non-quest registration\\n"); }\n');
    put('quest_optout', `int is_quest() { return 0; }
        void register_information() { error("opted-out quest registration\\n"); }
    `);
    for (const mode of ['old', 'new']) {
        const rules = mode === 'old' ? 'setRules' : 'set_rules';
        put(`verb_${mode}`, `inherit CORE_VERB;
            int calls;
            protected varargs string *${rules}(mixed *args...) {
                calls++;
                return ::${rules}(args...);
            }
            string *apply_rules() { return ${mode === 'old' ? 'set_rules' : 'setRules'}("LIV", ({ "OBJ" })); }
            int query_calls() { return calls; }
        `);
        const gmcp = mode === 'old' ? 'sendGMCP' : 'send_gmcp_data';
        put(`gmcp_${mode}`, `inherit "/mudcore/inherit/user_gmcp";
            mixed *seen;
            varargs void ${gmcp}(mapping data, mixed *modules...) {
                seen = ({ data, modules });
                ::${gmcp}(data, modules...);
            }
            mixed *query_seen() { return seen; }
        `);
    }
    for (const mode of ['old', 'new', 'both', 'error']) {
        let provider = '';
        for (const [variant, names] of Object.entries({
            old: ['isQuest', 'getKill', 'getItem', 'getName', 'preCondition'],
            new: ['is_quest', 'get_required_kills', 'get_required_items', 'get_name', 'check_prerequisites'],
        })) {
            if (mode !== variant && mode !== 'both' && mode !== 'error') continue;
            const [quest, kill, item, name, condition] = names;
            provider += `int ${quest}() { return 1; }
                mapping ${kill}() { return ([ "/fixtures/mob.c": 2 ]); }
                mapping ${item}() { return ([ "/fixtures/token.c": 3 ]); }
                string ${name}() { ${mode === 'error' && variant === 'new' ? 'error("naming provider failure\\n");' : `return "${variant}";`} }
                int ${condition}(int a, int b) { return a + b; }\n`;
        }
        put(`provider_${mode}`, provider);
    }
    put('error_override', `inherit CORE_ROOM;
        varargs void setArea(mixed area, int x, int y, int z) { error("naming override failure\\n"); }
    `);
}
