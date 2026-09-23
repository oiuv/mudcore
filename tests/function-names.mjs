// Static API inventory, independent of the host's formatter/driver checkout.
import assert from 'node:assert/strict';
import { readFileSync, readdirSync } from 'node:fs';
import { dirname, join, relative, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
export const aliases = JSON.parse(readFileSync(join(root, 'tests/function-names.json'), 'utf8'));
function walk(dir) {
    return readdirSync(dir, { withFileTypes: true }).flatMap(entry => {
        const file = join(dir, entry.name);
        return entry.isDirectory() ? walk(file) : [file];
    });
}
const files = ['include', 'inherit', 'system', 'cmds', 'verbs', 'world', 'tests']
    .flatMap(dir => walk(join(root, dir))).filter(file => /\.(c|lpc|h)$/.test(file));
const found = new Set();
let total = 0;
for (const file of files) {
    const path = relative(root, file).replaceAll('\\', '/');
    const source = readFileSync(file, 'utf8');
    // Mask comments and quoted data before inspecting declarations, keeping line boundaries.
    const code = source.replace(/\/\*[\s\S]*?\*\/|\/\/[^\n]*|"(?:\\[\s\S]|[^"\\])*"|'(?:\\.|[^'\\])*'/g,
        text => text.replace(/[^\n]/g, ' '));
    const definitions = [...code.matchAll(/^[ \t]*(?:(?:private|protected|public|nomask|varargs|static|nosave|async)\s+)*(?:void|int|float|string|mixed|mapping|object|function|buffer)\s+\**\s*([A-Za-z_]\w*)\s*\([^()]*\)\s*\{/gm)]
        .map(match => match[1]);
    for (const name of definitions) {
        total++;
        if (!/[A-Z]/.test(name)) continue;
        assert(aliases[path]?.[name], `Unregistered non-snake-case function: ${path}:${name}`);
        const key = `${path}:${name}`;
        assert(!found.has(key), `Duplicate compatibility entry: ${key}`);
        found.add(key);
    }
    for (const [old, current] of Object.entries(aliases[path] || {})) {
        assert(definitions.includes(old), `Missing legacy entry: ${path}:${old}`);
        assert(definitions.includes(current), `Missing canonical entry: ${path}:${current}`);
        assert(/^[a-z_][a-z0-9_]*$/.test(current), `Invalid canonical name: ${current}`);
    }
}
for (const [file, names] of Object.entries(aliases)) {
    for (const old of Object.keys(names)) assert(found.has(`${file}:${old}`), `Stale alias: ${file}:${old}`);
}
console.log(`Function naming audit: ${total} definitions; ${found.size} registered legacy aliases; no unregistered mixed-case functions.`);
