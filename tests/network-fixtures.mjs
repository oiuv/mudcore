import net from 'node:net';
import tls from 'node:tls';
import dgram from 'node:dgram';
import { readFileSync } from 'node:fs';
import { join } from 'node:path';
import assert from 'node:assert/strict';

// All endpoints are ephemeral loopback listeners; certificates are public test fixtures.
export async function startNetworkFixtures(root) {
    const servers = [], sockets = new Set(), timers = new Set();
    const requests = [], packets = [], sniNames = [];
    function later(fn, ms) {
        const timer = setTimeout(() => { timers.delete(timer); fn(); }, ms);
        timers.add(timer);
    }
    function track(socket) {
        sockets.add(socket);
        socket.on('error', () => {});
        socket.on('close', () => sockets.delete(socket));
    }
    function fragments(socket, bytes, step = 7) {
        bytes = Buffer.from(bytes);
        let offset = 0;
        function send() {
            if (socket.destroyed) return;
            socket.write(bytes.subarray(offset, offset + step));
            offset += step;
            if (offset < bytes.length) later(send, 10);
        }
        send();
    }
    function respond(socket, packet) {
        const [line] = packet.split('\r\n');
        const [method, path] = line.split(' ');
        requests.push({ method, path });
        if (path === '/stall') return;
        if (path === '/truncated') {
            socket.end('HTTP/1.1 200 OK\r\nContent-Length: 20\r\n\r\nshort');
        } else if (path === '/eof') {
            socket.write('HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nfirst');
            later(() => socket.end('-last'), 40);
        } else if (path === '/chunked') {
            fragments(socket, 'HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n3;x=y\r\nabc\r\n6\r\n中文\r\n0\r\nX-Test: yes\r\n\r\n', 5);
        } else if (method === 'HEAD') {
            fragments(socket, 'HTTP/1.1 200 OK\r\nContent-Length: 123\r\n\r\n');
        } else {
            const body = path === '/tls' ? 'tls-ok' : 'hello 中文 world';
            fragments(socket, `HTTP/1.1 200 OK\r\nContent-Length: ${Buffer.byteLength(body)}\r\n\r\n${body}`, 4);
        }
    }
    function http(socket) {
        track(socket);
        let packet = '';
        socket.on('data', chunk => {
            packet += chunk.toString('utf8');
            if (!packet.includes('\r\n\r\n')) return;
            socket.removeAllListeners('data');
            respond(socket, packet);
        });
    }
    async function listen(server) {
        servers.push(server);
        await new Promise((resolve, reject) => {
            server.once('error', reject);
            server.listen(0, '127.0.0.1', resolve);
        });
        return server.address().port;
    }
    const httpPort = await listen(net.createServer(http));
    const otherHttpPort = await listen(net.createServer(http));
    async function secure(prefix, handler) {
        const server = tls.createServer({
            key: readFileSync(join(root, 'tests/fixtures', `${prefix}-key.pem`)),
            cert: readFileSync(join(root, 'tests/fixtures', `${prefix}-cert.pem`)),
        }, socket => { sniNames.push(socket.servername); handler(socket); });
        server.on('connection', track);
        server.on('tlsClientError', () => {});
        return listen(server);
    }
    const tlsPort = await secure('localhost', http);
    const untrustedPort = await secure('untrusted', http);
    const mismatchPort = await secure('mismatch', http);
    const socketTlsPort = await secure('localhost', socket => {
        track(socket);
        socket.on('data', () => socket.end('tls-ok'));
    });
    const udp = dgram.createSocket('udp4');
    await new Promise(resolve => udp.bind(0, '127.0.0.1', resolve));
    udp.on('message', packet => packets.push(packet.toString('utf8')));
    async function reservePort() {
        const server = net.createServer();
        await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
        const port = server.address().port;
        await new Promise(resolve => server.close(resolve));
        return port;
    }
    async function reserveUdpPort() {
        const reservation = dgram.createSocket('udp4');
        await new Promise((resolve, reject) => {
            reservation.once('error', reject);
            reservation.bind(0, '127.0.0.1', resolve);
        });
        const port = reservation.address().port;
        await new Promise(resolve => reservation.close(resolve));
        return port;
    }
    return {
        config: { httpPort, otherHttpPort, tlsPort, untrustedPort, mismatchPort, socketTlsPort,
            udpPort: udp.address().port, loginPort: await reservePort(), tuiPort: await reservePort(), intermudPort: await reserveUdpPort() },
        verify() {
            assert(requests.some(x => x.method === 'HEAD' && x.path.includes('q=%E4%B8%AD%E6%96%87')), 'HEAD query reached server');
            assert(packets.filter(x => x.startsWith('@@@ping_q')).length >= 2, 'Intermud DNS peer received ping');
            assert(packets.filter(x => x.startsWith('@@@mudlist_q')).length >= 2, 'Intermud DNS peer received mudlist request');
            assert(sniNames.length >= 2 && sniNames.every(x => x === 'localhost'), 'HTTP and Socket TLS supplied SNI');
        },
        async close() {
            for (const timer of timers) clearTimeout(timer);
            for (const socket of sockets) socket.destroy();
            udp.close();
            await Promise.all(servers.map(server => new Promise(resolve => server.close(resolve))));
        },
    };
}

export async function exerciseLogin(port) {
    let socket, text = '';
    async function connect() {
        text = '';
        socket = net.createConnection({ host: '127.0.0.1', port });
        socket.on('data', data => { text += data.toString('utf8').replace(/\x1b\[[0-9;]*m/g, ''); });
        await new Promise((resolve, reject) => { socket.once('connect', resolve); socket.once('error', reject); });
    }
    async function expect(pattern) {
        const limit = Date.now() + 3000;
        while (!pattern.test(text)) {
            if (Date.now() >= limit) throw new Error(`Login did not reach ${pattern}: ${text}`);
            await new Promise(resolve => setTimeout(resolve, 20));
        }
        text = '';
    }
    function send(line) { socket.write(line + '\r\n'); }
    try {
        await connect();
        await expect(/登录ID/); send('root');
        await expect(/至少/); send('regression');
        await expect(/确定吗/); send('y');
        await expect(/登录密码/); send('local-test-only');
        await expect(/请再输入/); send('local-test-only');
        await expect(/名字/); send('PlainName');
        await expect(/纯中文/); send('回归玩家');
        await expect(/男性/); send('x');
        await expect(/只能扮演/); send('m');
        // The driver test checks the resulting UID and persisted body as well.
        await new Promise(resolve => setTimeout(resolve, 500));
        socket.destroy();
        await new Promise(resolve => setTimeout(resolve, 150));
        await connect();
        await expect(/登录ID/); send('regression');
        await expect(/密码/); send('local-test-only');
        await expect(/重新连线完毕/);
    } finally {
        socket?.destroy();
    }
}
