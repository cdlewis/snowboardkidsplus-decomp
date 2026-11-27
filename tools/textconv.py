#!/usr/bin/env python3
"""Convert preprocessed UTF-8 text markers to Shift-JIS C byte strings."""
import argparse
from pathlib import Path
import re

STRING = r'"(?:\\.|[^"\\])*"'
TOKEN = re.compile(r'//[^\n]*|/\*.*?\*/|' + STRING + r"|'(?:\\.|[^'\\])*'|\bSKP_ENCODE_SJIS\b", re.S)
ESCAPE = re.compile(r'\\(?:[0-7]{1,3}|x[0-9a-fA-F]+|.)', re.S)


def encode(body):
    result = bytearray()
    cursor = 0
    for match in ESCAPE.finditer(body):
        result.extend(body[cursor:match.start()].encode('shift_jis'))
        value = match[0][1:]
        if value[0] in '01234567x':
            number = int(value[1:], 16) if value[0] == 'x' else int(value, 8)
            result.append(number)
        else:
            escapes = {'n': '\n', 'r': '\r', 't': '\t', 'a': '\a', 'b': '\b', 'f': '\f', 'v': '\v', '\\': '\\', '"': '"', "'": "'", '?': '?', '\n': ''}
            if value not in escapes:
                raise ValueError('unsupported escape ' + match[0])
            result.extend(escapes[value].encode('shift_jis'))
        cursor = match.end()
    result.extend(body[cursor:].encode('shift_jis'))
    return result


def convert(source):
    output = []
    cursor = 0
    for token in TOKEN.finditer(source):
        if token.start() < cursor or token[0] != 'SKP_ENCODE_SJIS':
            continue
        call = re.match(r'\s*\(\s*((?:' + STRING + r'\s*)+)\)', source[token.end():], re.S)
        try:
            if not call:
                raise ValueError('expected adjacent string literals')
            data = b''.join(encode(m[0][1:-1]) for m in re.finditer(STRING, call[1], re.S))
        except ValueError as error:
            location = f'preprocessed line {source.count(chr(10), 0, token.start()) + 1}'
            markers = list(re.finditer(r'^#\s+(\d+)\s+"([^"\n]+)"[^\n]*\n', source[:token.start()], re.M))
            if markers:
                marker = markers[-1]
                line = int(marker[1]) + source.count('\n', marker.end(), token.start())
                location = f'{marker[2]}:{line}'
            raise ValueError(f'{location}: {error}') from error
        output.extend((source[cursor:token.start()], '"' + ''.join('\\%03o' % byte for byte in data) + '"', '\n' * call[0].count('\n')))
        cursor = token.end() + call.end()
    output.append(source[cursor:])
    return ''.join(output)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('path', type=Path)
    args = parser.parse_args()
    try:
        result = convert(args.path.read_text(encoding='utf-8'))
        args.path.write_text(result, encoding='utf-8')
    except ValueError as error:
        parser.exit(1, f'{args.path}: {error}\n')
