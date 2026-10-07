#!/usr/bin/env python3
"""Exercise the real mbusd process through TCP and a pseudo-terminal (stdlib only)."""
import argparse
import os
import pty
import select
import socket
import struct
import subprocess
import tempfile
import time


def crc(data):
    value = 0xffff
    for byte in data:
        value ^= byte
        for _ in range(8):
            value = (value >> 1) ^ (0xa001 if value & 1 else 0)
    return data + struct.pack('<H', value)


def exact(sock, count):
    data = b''
    while len(data) < count:
        chunk = sock.recv(count - len(data))
        if not chunk:
            raise AssertionError('Gateway closed TCP connection')
        data += chunk
    return data


def case(binary, name, echo, chunks, expected, request=b'\x01\x03\x08\x00\x00\x02', omit_option=False, repeat=1):
    master, slave = pty.openpty()
    with socket.socket() as port_socket:
        port_socket.bind(('127.0.0.1', 0))
        port = port_socket.getsockname()[1]
    try:
        with tempfile.TemporaryDirectory(prefix='mbusd-regression-') as directory:
            config = directory + '/mbusd.conf'
            with open(config, 'w') as file:
                file.write(f'device = {os.ttyname(slave)}\nspeed = 9600\nmode = 8N1\n'
                           f'address = 127.0.0.1\nport = {port}\nloglevel = 2\n'
                           'retries = 0\npause = 10\nwait = 500\ntimeout = 5\n')
                if not omit_option:
                    file.write(f'local_echo = {"yes" if echo else "no"}\n')
            with open(directory + '/log', 'w+') as log:
                process = subprocess.Popen([binary, '-d', '-L', '-', '-c', config], stdout=log, stderr=log)
                try:
                    deadline = time.monotonic() + 3
                    while True:
                        try:
                            client = socket.create_connection(('127.0.0.1', port), timeout=1)
                            break
                        except OSError:
                            if process.poll() is not None or time.monotonic() >= deadline:
                                raise AssertionError('Gateway did not start')
                            time.sleep(.02)
                    with client:
                        client.settimeout(3)
                        for transaction in range(1, repeat + 1):
                            client.sendall(struct.pack('>HHH', transaction, 0, len(request)) + request)
                            actual = b''
                            deadline = time.monotonic() + 2
                            while len(actual) < len(request) + 2:
                                ready, _, _ = select.select([master], [], [], max(0, deadline-time.monotonic()))
                                assert ready, 'No RTU request'
                                actual += os.read(master, 512)
                            assert actual == crc(request), actual.hex()
                            for chunk in chunks(crc(request)):
                                if chunk:
                                    os.write(master, chunk)
                                time.sleep(.005)
                            header = exact(client, 6)
                            tid, protocol, length = struct.unpack('>HHH', header)
                            assert (tid, protocol) == (transaction, 0)
                            assert 2 <= length <= 254
                            response = exact(client, length)
                            assert response == expected, f'{name}: expected {expected.hex()}, got {response.hex()}'
                except Exception:
                    log.flush()
                    log.seek(0)
                    print(log.read())
                    raise
                finally:
                    process.terminate()
                    try:
                        process.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()
    finally:
        os.close(master)
        os.close(slave)
    print(f'PASS {name}', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary')
    parser.add_argument('--upstream-repro', action='store_true')
    args = parser.parse_args()
    response = b'\x01\x03\x04' + struct.pack('>f', 250.5)
    frame = crc(response)
    if args.upstream_repro:
        case(args.binary, 'echo plus voltage', True, lambda req: [req + frame], response, omit_option=True)
        return
    case(args.binary, 'echo and response in one write; resets per request', True, lambda req: [req + frame], response, repeat=3)
    case(args.binary, 'fragmented echo and response', True, lambda req: [bytes([b]) for b in req + frame], response)
    case(args.binary, 'no echo with option enabled', True, lambda req: [frame], response)
    case(args.binary, 'default off', False, lambda req: [frame], response, omit_option=True)
    case(args.binary, 'mismatch after shared request prefix', True, lambda req: [frame[:3], frame[3:]], response,
         request=b'\x01\x03\x04\x00\x00\x02')
    for label, chunks in [('echo only', lambda req: [req]), ('partial echo', lambda req: [req[:4]]),
                          ('no response', lambda req: []),
                          ('bad response CRC', lambda req: [req + frame[:-1] + bytes([frame[-1] ^ 1])]),
                          ('bad echo CRC', lambda req: [req[:-1] + bytes([req[-1] ^ 1])])]:
        case(args.binary, label, True, chunks, bytes([1, 0x83, 4 if label.startswith('bad') else 0x0b]))
    exception = b'\x01\x83\x02'
    case(args.binary, 'meter exception after echo', True, lambda req: [req + crc(exception)], exception)
    # FC16 requests can be much longer than their 8-byte replies. Replaying
    # a damaged echo must not underflow the next read length, even with more
    # than a buffer's worth of serial data following it (PR #135).
    for registers in (2, 123):
        request = struct.pack('>BBHHB', 1, 16, 0, registers, registers * 2) + b'\x00\x01' * registers
        reply = request[:6]
        case(args.binary, f'FC16 {registers} registers with echo', True,
             lambda req: [req + crc(reply)], reply, request=request)
        case(args.binary, f'FC16 {registers} registers without echo', True,
             lambda req: [crc(reply)], reply, request=request)
        case(args.binary, f'damaged FC16 {registers} registers followed by excess serial data', True,
             lambda req: [req[:-1] + bytes([req[-1] ^ 1]), b'\x55' * 1024],
             bytes([1, 0x90, 4]), request=request, repeat=2)
    request = bytes.fromhex('01 10 0000 0002 04 0001 0002')
    case(args.binary, 'fragmented damaged FC16 echo followed by excess serial data', True,
         lambda req: [req[:7], req[7:-1] + bytes([req[-1] ^ 1]), b'\x55' * 1024],
         bytes([1, 0x90, 4]), request=request)
    for function in (5, 6):
        request = bytes([1, function, 0, 1, 0, 0])
        case(args.binary, f'FC{function:02} identical echo and write reply', True, lambda req: [req + req], request, request=request)
        case(args.binary, f'FC{function:02} ordinary write reply with echo disabled', False, lambda req: [req], request, request=request)
        case(args.binary, f'FC{function:02} echo alone cannot count as acknowledgement', True, lambda req: [req], bytes([1, function | 0x80, 0x0b]), request=request)


if __name__ == '__main__':
    main()
