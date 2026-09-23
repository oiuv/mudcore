import assert from 'node:assert/strict';
import net from 'node:net';
import { spawn } from 'node:child_process';
import { cpSync, existsSync, mkdirSync, mkdtempSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join, relative, resolve, sep } from 'node:path';
import { fileURLToPath } from 'node:url';
import { StringDecoder } from 'node:string_decoder';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const driver = process.argv[2];
if (!driver) throw new Error('Usage: node tests/contracts.mjs <path-to-fluffos-driver>');

async function reservePort() {
    const server = net.createServer();
    await new Promise((resolveReady, reject) => {
        server.once('error', reject);
        server.listen(0, '127.0.0.1', resolveReady);
    });
    const port = server.address().port;
    await new Promise(resolveClosed => server.close(resolveClosed));
    return port;
}

async function session(port) {
    const socket = net.createConnection({ host: '127.0.0.1', port });
    const decoder = new StringDecoder('utf8');
    let text = '', ended = false, socketError;
    // The fixture does not negotiate binary Telnet options; preserve UTF-8 chunk boundaries.
    socket.on('data', data => { text += decoder.write(data).replace(/\x1b\[[0-9;]*m/g, ''); });
    socket.on('close', () => { ended = true; });
    socket.on('error', error => { socketError = error; });
    await new Promise((resolveConnected, reject) => {
        socket.once('connect', resolveConnected);
        socket.once('error', reject);
    });
    return {
        send(line) { socket.write(line + '\r\n'); },
        close() { socket.destroy(); },
        async expectCreationFailure() {
            const deadline = Date.now() + 5000;
            while (!ended && Date.now() < deadline)
                await new Promise(resolveTick => setTimeout(resolveTick, 20));
            assert(ended, 'failed creation closes its connection');
            assert.doesNotMatch(text, /CONTRACT READY|ROOM:/, 'failed creation never enters the world');
        },
        async expect(pattern) {
            const deadline = Date.now() + 5000;
            while (true) {
                const match = pattern.exec(text);
                if (match) {
                    text = text.slice(match.index + match[0].length);
                    return match;
                }
                if (socketError || ended || Date.now() >= deadline)
                    throw new Error(`Contract Telnet expected ${pattern}: ${socketError?.message || text}`);
                await new Promise(resolveTick => setTimeout(resolveTick, 20));
            }
        },
    };
}

async function exercise(port, custom) {
    let client;
    async function register(id, name) {
        client = await session(port);
        await client.expect(/登录ID/); client.send(id);
        await client.expect(/确定吗/); client.send('y');
        await client.expect(/登录密码/); client.send('contract-local-only');
        await client.expect(/请再输入/); client.send('contract-local-only');
        await client.expect(/Character name:/); client.send('bad');
        await client.expect(/REJECT_NAME/);
        await client.expect(/Character name:/); client.send(name);
        if (custom) {
            await client.expect(/驾驶员/); client.send('wrong');
            await client.expect(/只能扮演/); client.send('PILOT');
        }
    }
    async function login() {
        client = await session(port);
        await client.expect(/登录ID/); client.send('contractone');
        await client.expect(/密码/); client.send('contract-local-only');
    }
    async function state(expectedSetups) {
        client.send('contract state');
        const match = await client.expect(/STATE (\{[^\r\n]*\})\r?\n/);
        const value = JSON.parse(match[1]);
        assert.equal(value.id, 'contractone');
        assert.equal(value.name, 'Traveler');
        assert.equal(value.init_count, 1, 'creation hook runs once');
        assert.equal(value.setup_count, expectedSetups, 'restore activates, reconnect preserves lifecycle');
        assert.equal(value.host_attribute, 'kept');
        assert.equal(value.has_rpg, 0);
        assert.equal(value.has_gender, custom ? 1 : 0);
        if (custom) assert.equal(value.gender, 'pilot-role');
        assert.equal(value.dbase_marker, 42);
        assert.equal(value.base_marker, 73);
        return value;
    }
    try {
        await register('blockedbyhost', 'Denied');
        await client.expect(/无法登录/);
        client.close();
        await register('failedbyhook', 'Interrupted');
        await client.expectCreationFailure();
        await register('contractone', 'Traveler');
        await client.expect(/ROOM: \/world\/contract_start/);
        await state(1);
        client.send('contract two words');
        await client.expect(/ARG=two words;ACTOR=contractone/);
        client.send('go north');
        await client.expect(/ROOM: \/world\/contract_other/);
        client.send('north');
        await client.expect(/ROOM: \/world\/contract_start/);
        const moved = await state(1);
        if (custom) assert(moved.gmcp_calls >= 3, 'selected GMCP receives each movement');
        else assert.equal(moved.gmcp_calls, 0);
        client.close();
        await new Promise(resolveTick => setTimeout(resolveTick, 200));
        await login();
        await client.expect(/重新连线完毕/);
        await state(1);
        client.send('contract savequit');
        await client.expect(/SAVED/);
        client.close();
        await new Promise(resolveTick => setTimeout(resolveTick, 200));
        await login();
        await client.expect(/ROOM: \/world\/contract_start/);
        await state(2);
        if (custom) {
            client.send('contract gmcpfail');
            await client.expect(/GMCP FAILURE CHECKED/);
        }
        client.send('contract done');
        // Keep the connection alive until the driver has processed the finish command.
        await new Promise(resolveTick => setTimeout(resolveTick, 200));
    } finally {
        client?.close();
    }
}

for (const profile of ['minimal', 'custom']) {
    const custom = profile === 'custom';
    const sandbox = mkdtempSync(join(tmpdir(), 'mudcore-contract-'));
    const excluded = new Set([
        'inherit/attack.c', 'inherit/condition.c', 'inherit/condition_mod.c', 'inherit/team.c',
        'inherit/user_quest.c', 'inherit/user_combat_record.c',
        'system/daemons/combat_d.c', 'system/daemons/quest_d.c', 'system/daemons/ever_quest_d.c',
        'system/daemons/emote_d.c', 'system/daemons/channel_d.c', 'system/daemons/verb_d.c',
        ...(!custom ? ['inherit/user_gmcp.c'] : []),
    ]);
    console.log(`Running ${profile} contracts in ${sandbox}`);
    for (const dir of ['log', 'data', 'cmds', 'world', 'system']) mkdirSync(join(sandbox, dir));
    for (const dir of ['include', 'inherit', 'system']) {
        cpSync(join(root, dir), join(sandbox, 'mudcore', dir), {
            recursive: true,
            filter: source => !excluded.has(relative(root, source).split(sep).join('/')),
        });
    }
    for (const file of excluded) assert(!existsSync(join(sandbox, 'mudcore', file)), `absent: ${file}`);
    cpSync(join(root, 'tests/contracts/lpc'), join(sandbox, 'tests'), { recursive: true });
    cpSync(join(root, 'tests/lpc/command_baseline.lpc'), join(sandbox, 'tests/command_baseline.lpc'));
    for (const name of ['login', 'user', 'char'])
        cpSync(join(root, `tests/contracts/lpc/${name}.lpc`), join(sandbox, `system/contract_${name}.lpc`));
    for (const name of ['start', 'other'])
        cpSync(join(root, 'tests/contracts/lpc/room.lpc'), join(sandbox, `world/contract_${name}.lpc`));
    cpSync(join(root, 'tests/contracts/lpc/contract.lpc'), join(sandbox, 'cmds/contract.lpc'));
    const put = (name, text) => writeFileSync(join(sandbox, name), text, 'utf8');
    put('tests/profile.h', `#define CONTRACT_CUSTOM ${custom ? 1 : 0}\n`);
    put('cmds/look.alias', 'contract\n');
    put('cmds/go.alias', 'contract\n');
    const port = await reservePort();
    put('driver.cfg', [
        'name : Mudcore Contracts', 'mud ip : 127.0.0.1', `port number : ${port}`,
        'mudlib directory : ' + sandbox.replaceAll('\\', '/'),
        'log directory : /log', 'debug log file : debug.log',
        'include directories : /tests:/mudcore/include', 'global include file : <globals.h>',
        'master file : /tests/master', 'simulated efun file : /mudcore/system/kernel/simul_efun',
        'maximum evaluation cost : 100000000', 'gametick msec : 100',
    ].join('\n') + '\n');
    let stdout = '', stderr = '', exerciseTask, exerciseError;
    const result = await new Promise((resolveExit, reject) => {
        const child = spawn(resolve(driver), ['driver.cfg'], { cwd: sandbox, windowsHide: true });
        const timer = setTimeout(() => { exerciseError = new Error('Contract driver timed out'); child.kill(); }, 45000);
        child.stdout.on('data', data => {
            stdout += data.toString('utf8');
            if (!exerciseTask && stdout.includes('CONTRACT LOGIN READY')) {
                exerciseTask = exercise(port, custom).catch(error => {
                    exerciseError = error;
                    child.kill();
                });
            }
        });
        child.stderr.on('data', data => { stderr += data.toString('utf8'); });
        child.once('error', error => { clearTimeout(timer); reject(error); });
        child.once('close', status => { clearTimeout(timer); resolveExit(status); });
    });
    await exerciseTask;
    put('driver-output.txt', stdout + stderr);
    console.log(stdout.split('\n').filter(line => /PASS|FAIL|CHECKS/.test(line)).join('\n'));
    if (exerciseError) console.error(exerciseError.stack || exerciseError.message);
    assert(!exerciseError && exerciseTask && result === 0 && stdout.includes('MUDCORE CONTRACTS PASS'),
        `Contract regression failed (${result}): ${sandbox}/driver-output.txt`);
}
console.log('Minimal and custom host contracts passed; temporary fixtures retained for diagnosis.');
