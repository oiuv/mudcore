import net from 'node:net';
import assert from 'node:assert/strict';
import { StringDecoder } from 'node:string_decoder';

const pause = ms => new Promise(resolve => setTimeout(resolve, ms));

// Minimal Telnet peer for actual driver input, independent of any browser client.
class TerminalPeer {
    constructor(port) {
        this.socket = net.createConnection({ host: '127.0.0.1', port });
        this.text = '';
        this.state = 0;
        this.sub = [];
        this.decoder = new StringDecoder('utf8');
        this.charMode = false;
        this.failure = null;
        this.socket.on('error', error => { this.failure = error; });
        this.socket.on('data', data => this.feed(data));
    }
    bytes(data) { this.socket.write(Buffer.from(data)); }
    send(text) { this.socket.write(text); }
    line(text) { this.send(text + '\r\n'); }
    resize(width, height) {
        const payload = [width >> 8, width & 255, height >> 8, height & 255];
        this.bytes([255, 250, 31, ...payload.flatMap(byte => byte === 255 ? [255, 255] : [byte]), 255, 240]);
    }
    feed(data) {
        const plain = [];
        for (const byte of data) {
            if (this.state === 0) {
                if (byte === 255) this.state = 1;
                else plain.push(byte);
            } else if (this.state === 1) {
                if (byte === 255) { plain.push(byte); this.state = 0; }
                else if (byte === 250) { this.sub = []; this.state = 3; }
                else if ([251, 252, 253, 254].includes(byte)) { this.command = byte; this.state = 2; }
                else this.state = 0;
            } else if (this.state === 2) {
                if (this.command === 251) {
                    this.bytes([255, [1, 3].includes(byte) ? 253 : 254, byte]);
                    if (byte === 3) this.charMode = true;
                } else if (this.command === 252) {
                    this.bytes([255, 254, byte]);
                    if (byte === 3) this.charMode = false;
                } else if (this.command === 253) {
                    this.bytes([255, [3, 24, 31].includes(byte) ? 251 : 252, byte]);
                    if (byte === 31) this.resize(80, 24);
                } else this.bytes([255, 252, byte]);
                this.state = 0;
            } else if (this.state === 3) {
                if (byte === 255) this.state = 4;
                else this.sub.push(byte);
            } else {
                if (byte === 240) {
                    if (this.sub[0] === 24 && this.sub[1] === 1)
                        this.bytes([255, 250, 24, 0, ...Buffer.from('xterm-256color'), 255, 240]);
                    this.state = 0;
                } else { if (byte === 255) this.sub.push(255); this.state = 3; }
            }
        }
        this.text += this.decoder.write(Buffer.from(plain));
    }
    async expect(pattern) {
        const deadline = Date.now() + 3000;
        while (!pattern.test(this.text)) {
            if (this.failure) throw this.failure;
            if (Date.now() >= deadline) throw new Error(`TUI expected ${pattern}, received ${JSON.stringify(this.text)}`);
            await pause(10);
        }
        const output = this.text;
        this.text = '';
        return output;
    }
    close() { this.socket.destroy(); }
}

export async function exerciseTui(port) {
    let peer = new TerminalPeer(port);
    try {
        assert.match(await peer.expect(/READY/), /SUPPORTED:1/, 'driver supports interactive TUI');
        peer.line('read');
        await peer.expect(/INPUT>/);
        // Split a Chinese UTF-8 character across TCP writes, then edit with DEL.
        peer.bytes([0xe4]);
        await pause(20);
        peer.bytes([0xb8, 0xad]);
        peer.send('文x\x7f\r');
        assert.match(await peer.expect(/READY/), /LINE:\["中文",1\]/);

        peer.line('select');
        await peer.expect(/SELECT>/);
        peer.send('\x1b[B\r');
        assert.match(await peer.expect(/READY/), /SELECT:\[1,"干粮",1\]/);
        peer.line('multi');
        await peer.expect(/MULTI>/);
        peer.send(' \x1b[B\x1b[B \r');
        assert.match(await peer.expect(/READY/), /MULTI:\[\[0,2\],\["a","c"\],1\]/);

        peer.line('chain');
        await peer.expect(/CHAIN SELECT>/);
        peer.send('\r');
        await peer.expect(/CHAIN>/);
        peer.send('chained\r');
        assert.match(await peer.expect(/READY/), /LINE:\["chained",1\]/);

        peer.line('fail');
        await peer.expect(/FAIL>/);
        peer.send('test\r');
        await peer.expect(/READY/);
        peer.line('ping');
        assert.match(await peer.expect(/READY/), /COMMAND:ping/);
        for (const command of ['idle-close', 'busy']) {
            peer.line(command);
            await peer.expect(/CAPTURE>/);
            peer.line('retained');
            assert.match(await peer.expect(/READY/), /CAPTURE:retained/);
        }

        peer.line('async-app');
        await peer.expect(/ASYNC APP REFUSED/);
        peer.resize(100, 30);
        await pause(50);
        peer.send('\x1b[B\r');
        await peer.expect(/已选择/);
        peer.send('q');
        const closed = await peer.expect(/READY/);
        assert(closed.includes('\x1b[?1049l'), 'full-screen close restores main screen');
        assert.equal(peer.charMode, false, 'full-screen exit restores Telnet line mode');
        peer.line('status');
        assert.match(await peer.expect(/READY/), /SIZE:100,30 ACTIVE:0/);

        peer.line('app');
        await peer.expect(/终端界面演示/);
        peer.close(); // app, screen, widgets and character handler must be cleaned up
        await pause(100);
        peer = new TerminalPeer(port);
        await peer.expect(/READY/);
        peer.line('read');
        await peer.expect(/INPUT>/);
        peer.send('\x03');
        assert.match(await peer.expect(/READY/), /LINE:\[0,2\]/);
        peer.line('ping');
        assert.match(await peer.expect(/READY/), /COMMAND:ping/);
        peer.line('confirm');
        await peer.expect(/CONFIRM>/);
        peer.send('\x1b'); // an unfinished escape must not prevent Ctrl+C cancellation
        await pause(100);
        peer.send('\x03');
        assert.match(await peer.expect(/READY/), /CONFIRM:\[0,2\]/);
        assert.equal(peer.charMode, false, 'Ctrl+C restores Telnet line mode');
        peer.line('after-cancel');
        assert.match(await peer.expect(/READY/), /COMMAND:after-cancel/);
        peer.line('async-close');
        await peer.expect(/ASYNC REFUSED/);
        peer.send('y');
        assert.match(await peer.expect(/READY/), /CONFIRM:\[1,1\]/);
        console.log('TUI Telnet: UTF-8, keys, menus, callbacks, resize and cleanup passed.');
    } finally {
        peer.close();
    }
}
