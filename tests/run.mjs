import { mkdtempSync, cpSync, mkdirSync, readFileSync, readdirSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join, resolve, relative, sep } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawn } from 'node:child_process';
import { startNetworkFixtures, exerciseLogin } from './network-fixtures.mjs';
import assert from 'node:assert/strict';
import { exerciseTui } from './tui-network.mjs';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const driver = process.argv[2];
if (!driver) throw new Error('Usage: node tests/run.mjs <path-to-fluffos-driver>');
const sourceDirs = ['include', 'inherit', 'system', 'cmds', 'verbs', 'world'];
function walk(dir) {
    return readdirSync(dir, { withFileTypes: true }).flatMap(entry => {
        const path = join(dir, entry.name);
        return entry.isDirectory() ? walk(path) : [path];
    });
}
const sourceFiles = sourceDirs.flatMap(dir => walk(join(root, dir)))
    .filter(file => /\.(?:c|lpc|h)$/.test(file));
const included = new Set();
for (const file of sourceFiles) {
    const text = readFileSync(file, 'utf8');
    for (const match of text.matchAll(/#include\s+"([^"\n]+\.(?:c|lpc))"/g)) {
        included.add(resolve(dirname(file), match[1]));
    }
    // Generic networking is allowed. Concrete services and credentials are not.
    assert.doesNotMatch(text, /qrenco\.de|5\.9\.243\.188|118\.190\.104\.241|\/world\/npc\/ivy/, file);
}
const programs = sourceFiles.filter(file => /\.(?:c|lpc)$/.test(file) && !included.has(file))
    .map(file => '/mudcore/' + relative(root, file).split(sep).join('/'));
for (const mode of ['default', 'overrides']) {
    const sandbox = mkdtempSync(join(tmpdir(), 'mudcore-test-'));
    console.log(`Running ${mode} in ${sandbox}`);
    mkdirSync(join(sandbox, 'mudcore'));
    for (const dir of sourceDirs) cpSync(join(root, dir), join(sandbox, 'mudcore', dir), { recursive: true });
    cpSync(join(root, 'tests', 'lpc'), join(sandbox, 'tests'), { recursive: true });
    for (const dir of ['log', 'data', 'fixtures/commands', 'fixtures/verbs/group', 'fixtures/preload', 'fixtures/virtual']) {
        mkdirSync(join(sandbox, dir), { recursive: true });
    }
    const put = (path, text) => writeFileSync(join(sandbox, path), text, 'utf8');
    put('tests/overrides.h', mode === 'overrides'
        ? '#define _DBASE "/tests/override_dbase"\n#define ENV_D "/tests/override_env"\n#define HOST_UID_POLICY 1\n' : '');
    put('manifest.txt', programs.join('\n') + '\n');
    put('fixtures/commands/modern.lpc', 'int main(object me, string arg) { return 1; }\n');
    put('fixtures/commands/legacy.c', 'int main(object me, string arg) { return 1; }\n');
    put('fixtures/commands/duplicate.c', 'int marker() { return 1; }\n');
    put('fixtures/commands/duplicate.lpc', 'int marker() { return 2; }\n');
    put('fixtures/commands/modern.alias', 'modern.lpc\n');
    put('fixtures/commands/shortcut.alias', 'modern.lpc\r\n');
    put('fixtures/commands/empty.alias', '');
    put('fixtures/commands/fake.c.bak', 'this is not LPC');
    put('fixtures/commands/empty.txt', '');
    put('fixtures/verbs/group/fallback.lpc', 'int marker() { return 1; }\n');
    put('fixtures/verbs/legacy.c', 'string *getVerbs() { return ({ "legacyverb" }); }\n');
    put('fixtures/preload/a.lpc', 'int marker() { return 1; }\n');
    put('fixtures/preload/b.c', 'int marker() { return 1; }\n');
    put('fixtures/preload.txt', '  # comment\r\n  ; comment\r\n\r\n /fixtures/preload \r\n');
    put('fixtures/virtual/grid.lpc', 'int x; void create(int value, int y) { x = value; } int marker() { return x; }\n');
    // Test loadall against the temporary mudlib root, never the real host tree.
    for (const file of [
        'mudcore/tests/loadall_fixture.lpc', 'mudcore/tests/nested/loadall_fixture.lpc',
        'mudcore/docs/loadall_fixture.lpc', 'mudcore/system/kernel/master/loadall_fixture.lpc',
        'mudcore/system/kernel/simul_efun/loadall_fixture.lpc',
        'fixtures/.hidden/loadall_fixture.lpc', '.loadall_hidden/nested/loadall_fixture.lpc',
        'cmds/test/loadall_fixture.c', 'docs/loadall_fixture.lpc', 'fluffos/loadall_fixture.lpc',
        'data/loadall_fixture.lpc', 'tests/loadall_fixture.lpc', 'master/loadall_fixture.lpc',
        'simul_efun/loadall_fixture.lpc', 'mudcore/tests_extra/loadall_fixture.lpc',
    ]) {
        mkdirSync(dirname(join(sandbox, file)), { recursive: true });
        put(file, 'int marker() { return 1; }\n');
    }
    const network = await startNetworkFixtures(root);
    put('network.json', JSON.stringify(network.config));
    put('test-ca.pem', ['localhost', 'mismatch'].map(name => readFileSync(join(root, `tests/fixtures/${name}-cert.pem`), 'utf8')).join('\n'));
    put('driver.cfg', [
        'name : Mudcore Regression', 'mud ip : 127.0.0.1', 'port number : ' + network.config.loginPort,
        'external_port_2 : telnet ' + network.config.tuiPort,
        'mudlib directory : ' + sandbox.replaceAll('\\', '/'),
        'log directory : /log', 'debug log file : debug.log',
        'include directories : /tests:/mudcore/include', 'global include file : <globals.h>',
        'master file : /tests/master', 'simulated efun file : /mudcore/system/kernel/simul_efun',
        'maximum evaluation cost : 100000000', 'gametick msec : 100',
    ].join('\n') + '\n');
    let result, loginError, loginTask;
    try {
        result = await new Promise((resolveResult, reject) => {
            const child = spawn(resolve(driver), ['driver.cfg'], {
                cwd: sandbox, windowsHide: true,
                env: { ...process.env, SSL_CERT_FILE: join(sandbox, 'test-ca.pem') },
            });
            let stdout = '', stderr = '';
            let timeoutError;
            const timer = setTimeout(() => { timeoutError = new Error('Driver test timed out'); child.kill(); }, 60000);
            child.stdout.on('data', data => {
                stdout += data.toString('utf8');
                if (!loginTask && stdout.includes('LOGIN TEST READY'))
                    loginTask = Promise.all([exerciseLogin(network.config.loginPort), exerciseTui(network.config.tuiPort)])
                        .catch(error => { loginError = error; });
            });
            child.stderr.on('data', data => { stderr += data.toString('utf8'); });
            child.on('error', error => { clearTimeout(timer); reject(error); });
            child.on('close', status => { clearTimeout(timer); resolveResult({ status, stdout, stderr, error: timeoutError }); });
        });
        await loginTask;
    } finally {
        await network.close();
    }
    put('driver-output.txt', (result.stdout || '') + (result.stderr || ''));
    console.log((result.stdout || '').split('\n').filter(line => /PASS|FAIL|CHECKS|COMPILED|TLS HOSTNAME/.test(line)).join('\n'));
    if (loginError) console.error(loginError.message);
    if (loginError || result.error || result.status !== 0 || !result.stdout?.includes('MUDCORE TESTS PASS')) {
        console.error(result.stderr || '');
        throw new Error(`Regression failed (${result.status}): ${sandbox}/driver-output.txt`);
    }
    network.verify();
}
console.log('Dependency audit and both isolated driver suites passed. Test artifacts remain in the printed temporary directories.');
// Imported only after both original suites pass; any contract failure rejects this entrypoint too.
await import('./contracts.mjs');
console.log('All four mudcore suites passed (default, overrides, minimal, custom).');
