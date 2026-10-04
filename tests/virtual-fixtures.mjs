import { mkdirSync, writeFileSync } from 'node:fs';
import { dirname, join } from 'node:path';

// Only the disposable mudlib receives generated programs.
export function prepareVirtualFixtures(sandbox) {
    const put = (path, text) => {
        mkdirSync(dirname(join(sandbox, path)), { recursive: true });
        writeFileSync(join(sandbox, path), text, 'utf8');
    };
    const provider = `
int legacy_calls;
mixed create_virtual_object(string key) {
    if (key == "reject" || key == "9,9") return 0;
    if (key == "throw") error("expected virtual provider failure\\n");
    if (key == "bad") return 42;
    if (key == "self") return this_object();
    return new("/tests/virtual_result", key);
}
object query_maze_room(string key) {
    legacy_calls++;
    return new("/tests/virtual_result", "legacy");
}
int query_legacy_calls() { return legacy_calls; }
`;
    for (const path of ['fixtures/providers/modern.lpc', 'fixtures/virtual/modern.lpc', 'fixtures/mob/modern.lpc'])
        put(path, provider);
    for (const extension of ['c', 'lpc']) {
        put(`fixtures/providers/duplicate.${extension}`,
            `object create_virtual_object(string key) { return new("/tests/virtual_result", "${extension}"); }\n`);
        put(`fixtures/providers/only_${extension}.${extension}`, provider);
    }
    const grid = 'int *values; varargs void create(int x, int y, int z) { values = ({ x, y, z }); } int *query_values() { return values; }\n';
    put('fixtures/virtual/old_grid.lpc', grid);
    put('fixtures/providers/old_grid.lpc', grid);
    const mob = 'int value; void create(int n) { value = n; } int marker() { return value; }\n';
    put('fixtures/mob/old_mob.lpc', mob);
    put('fixtures/mob/7/monster.lpc', mob);
    const maze = 'object query_maze_room(string key) { return new("/tests/virtual_result", "old:" + key); }\n';
    put('fixtures/virtual/old_maze.lpc', maze);
    put('fixtures/mob/old_maze.lpc', maze);
    put('tests/virtual_unrouted.lpc', '#undef WORLD_DIR\n#undef MOB_DIR\n#include "/mudcore/system/daemons/virtual_d.c"\n');
    put('tests/virtual_overlap.lpc', '#undef WORLD_DIR\n#undef MOB_DIR\n#define WORLD_DIR "/fixtures/"\n#define MOB_DIR "/fixtures/mob/"\n#include "/mudcore/system/daemons/virtual_d.c"\n');
}
