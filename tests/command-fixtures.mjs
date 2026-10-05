import { mkdirSync, writeFileSync } from 'node:fs';
import { dirname, join } from 'node:path';

// All generated programs and editable files belong to the disposable test mudlib.
export function prepareCommandFixtures(sandbox) {
    const put = (path, text) => {
        mkdirSync(dirname(join(sandbox, path)), { recursive: true });
        writeFileSync(join(sandbox, path), text, 'utf8');
    };
    const commands = ['update', 'which', 'loadall', 'sa', 'CRLF', 'all_inventory', 'call_other', 'variables'];
    for (const name of commands) {
        let prefix = '';
        let suffix = '';
        if (name === 'which') prefix = '#undef COMMAND_D\n#define COMMAND_D "/tests/command_lookup"\n#undef VERB_D\n#define VERB_D "/tests/verb_lookup"\n';
        if (name === 'loadall') {
            prefix = 'object fixture_load(string path);\n#define load_object fixture_load\n';
            suffix = '#undef load_object\nobject fixture_load(string path) { return strsrch(path, "return_zero") != -1 ? 0 : efun::load_object(path); }\n';
        }
        put(`tests/command_${name}.lpc`, `${prefix}#include "command_capture.h"\n#include "/mudcore/cmds/wizard/${name}.c"\n#include "command_capture_impl.h"\n${suffix}`);
    }
    put('tests/command_update_no_void.lpc', '#undef VOID_OB\n#define VOID_OB "/fixtures/cmd/missing"\n#include "command_capture.h"\n#include "/mudcore/cmds/wizard/update.c"\n#include "command_capture_impl.h"\n');
    for (const name of ['go', 'look'])
        put(`tests/command_${name}.lpc`, `object fixture_player();\nvoid fixture_rules(mixed *args...);\n#define this_player fixture_player\n#define set_rules fixture_rules\n#include "command_capture.h"\n#include "/mudcore/verbs/common/${name}.c"\n#include "command_capture_impl.h"\n// Avoid registering test-only verbs in the real Telnet parser.\nvoid fixture_rules(mixed *args...) {}\nobject fixture_player() { return load_object("/tests/command_probe")->actor(); }\n`);
    put('tests/command_lookup.lpc', 'object find_command(string name) { return name == "known" ? load_object("/tests/command_room") : 0; }\n');
    put('tests/verb_lookup.lpc', 'int calls; mixed getVerb(string name) { calls++; if (name == "throw") error("expected verb failure\\n"); return name == "verb" ? "/fixtures/verbs/legacy" : 0; } int query_calls() { return calls; }\n');
    put('tests/verb_lookup_modern.lpc', 'string get_verb(string name) { return "/fixtures/verbs/modern"; } string getVerb(string name) { error("legacy must not override modern\\n"); }\n');
    put('tests/command_which_modern.lpc', '#undef COMMAND_D\n#define COMMAND_D "/tests/command_lookup"\n#undef VERB_D\n#define VERB_D "/tests/verb_lookup_modern"\n#include "command_capture.h"\n#include "/mudcore/cmds/wizard/which.c"\n#include "command_capture_impl.h"\n');
    for (const name of ['first.c', 'second.lpc', 'other.lpc'])
        put(`fixtures/cmd/${name}`, 'inherit "/tests/command_room";\n');
    put('fixtures/cmd/provider.lpc', `
int mode, calls, legacy;
void set_mode(int value) { mode = value; }
int query_calls() { return calls; }
int query_legacy() { return legacy; }
object create_virtual_object(string key) {
    calls++;
    if (key == "reject" || mode == 1) return 0;
    if (key == "throw" || mode == 2) error("expected command provider failure\\n");
    return new("/tests/command_room");
}
object query_maze_room(string key) { legacy++; return new("/tests/command_room"); }
`);
    put('fixtures/cmd/batch/ok.lpc', 'int value; void set_value(int v) { value = v; } int query_value() { return value; }\n');
    put('fixtures/cmd/batch/broken.lpc', 'void create( {\n');
    put('fixtures/cmd/batch/return_zero.lpc', 'int marker() { return 1; }\n');
    put('fixtures/cmd/batch/nested/ok.lpc', 'int marker() { return 1; }\n');
    put('fixtures/cmd/skipme/ok.lpc', 'int marker() { return 1; }\n');
    for (const [style, method] of [['modern', 'skip_load_dir'], ['legacy', 'skipLoadDir']])
        put(`tests/command_skip_${style}.lpc`, `inherit CORE_DIR "cmds/wizard/loadall";\nint calls;\nprotected int ${method}(string dir) { calls++; return strsrch(dir, "/fixtures/cmd/skipme") == 0 || ::${method}(dir); }\nint query_calls() { return calls; }\n`);
    put('fixtures/cmd/text/example.txt', 'first\r\nsecond\r\n');
    put('fixtures/cmd/text/absolute.txt', 'absolute\r\n');
    put('fixtures/cmd/text/write_denied.txt', 'keep\r\n');
    put('fixtures/cmd/text/read_denied.txt', 'keep\r\n');
    put('example.txt', 'root stays\r\n');
}
