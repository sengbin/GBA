import os
import urllib.request

# 5x7 基础字形表（自定义简单风格），用于生成更统一的 ASCII 6x12 点阵。
# 说明：每个字形为 7 行，每行 5 列，'1' 表示点亮。
_FONT_5X7 = {
    '0': [
        '01110',
        '10001',
        '10011',
        '10101',
        '11001',
        '10001',
        '01110',
    ],
    '1': [
        '00100',
        '01100',
        '00100',
        '00100',
        '00100',
        '00100',
        '01110',
    ],
    '2': [
        '01110',
        '10001',
        '00001',
        '00010',
        '00100',
        '01000',
        '11111',
    ],
    '3': [
        '11110',
        '00001',
        '00001',
        '01110',
        '00001',
        '00001',
        '11110',
    ],
    '4': [
        '00010',
        '00110',
        '01010',
        '10010',
        '11111',
        '00010',
        '00010',
    ],
    '5': [
        '11111',
        '10000',
        '10000',
        '11110',
        '00001',
        '00001',
        '11110',
    ],
    '6': [
        '01110',
        '10000',
        '10000',
        '11110',
        '10001',
        '10001',
        '01110',
    ],
    '7': [
        '11111',
        '00001',
        '00010',
        '00100',
        '01000',
        '01000',
        '01000',
    ],
    '8': [
        '01110',
        '10001',
        '10001',
        '01110',
        '10001',
        '10001',
        '01110',
    ],
    '9': [
        '01110',
        '10001',
        '10001',
        '01111',
        '00001',
        '00001',
        '01110',
    ],

    'A': [
        '01110',
        '10001',
        '10001',
        '11111',
        '10001',
        '10001',
        '10001',
    ],
    'B': [
        '11110',
        '10001',
        '10001',
        '11110',
        '10001',
        '10001',
        '11110',
    ],
    'C': [
        '01110',
        '10001',
        '10000',
        '10000',
        '10000',
        '10001',
        '01110',
    ],
    'D': [
        '11110',
        '10001',
        '10001',
        '10001',
        '10001',
        '10001',
        '11110',
    ],
    'E': [
        '11111',
        '10000',
        '10000',
        '11110',
        '10000',
        '10000',
        '11111',
    ],
    'F': [
        '11111',
        '10000',
        '10000',
        '11110',
        '10000',
        '10000',
        '10000',
    ],
    'G': [
        '01110',
        '10001',
        '10000',
        '10000',
        '10011',
        '10001',
        '01110',
    ],
    'H': [
        '10001',
        '10001',
        '10001',
        '11111',
        '10001',
        '10001',
        '10001',
    ],
    'I': [
        '01110',
        '00100',
        '00100',
        '00100',
        '00100',
        '00100',
        '01110',
    ],
    'J': [
        '00001',
        '00001',
        '00001',
        '00001',
        '10001',
        '10001',
        '01110',
    ],
    'K': [
        '10001',
        '10010',
        '10100',
        '11000',
        '10100',
        '10010',
        '10001',
    ],
    'L': [
        '10000',
        '10000',
        '10000',
        '10000',
        '10000',
        '10000',
        '11111',
    ],
    'M': [
        '10001',
        '11011',
        '10101',
        '10101',
        '10001',
        '10001',
        '10001',
    ],
    'N': [
        '10001',
        '11001',
        '10101',
        '10011',
        '10001',
        '10001',
        '10001',
    ],
    'O': [
        '01110',
        '10001',
        '10001',
        '10001',
        '10001',
        '10001',
        '01110',
    ],
    'P': [
        '11110',
        '10001',
        '10001',
        '11110',
        '10000',
        '10000',
        '10000',
    ],
    'Q': [
        '01110',
        '10001',
        '10001',
        '10001',
        '10101',
        '10010',
        '01101',
    ],
    'R': [
        '11110',
        '10001',
        '10001',
        '11110',
        '10100',
        '10010',
        '10001',
    ],
    'S': [
        '01111',
        '10000',
        '10000',
        '01110',
        '00001',
        '00001',
        '11110',
    ],
    'T': [
        '11111',
        '00100',
        '00100',
        '00100',
        '00100',
        '00100',
        '00100',
    ],
    'U': [
        '10001',
        '10001',
        '10001',
        '10001',
        '10001',
        '10001',
        '01110',
    ],
    'V': [
        '10001',
        '10001',
        '10001',
        '10001',
        '01010',
        '01010',
        '00100',
    ],
    'W': [
        '10001',
        '10001',
        '10001',
        '10101',
        '10101',
        '10101',
        '01010',
    ],
    'X': [
        '10001',
        '01010',
        '00100',
        '00100',
        '00100',
        '01010',
        '10001',
    ],
    'Y': [
        '10001',
        '01010',
        '00100',
        '00100',
        '00100',
        '00100',
        '00100',
    ],
    'Z': [
        '11111',
        '00001',
        '00010',
        '00100',
        '01000',
        '10000',
        '11111',
    ],

    'a': [
        '00000',
        '00000',
        '01110',
        '00001',
        '01111',
        '10001',
        '01111',
    ],
    'b': [
        '10000',
        '10000',
        '10110',
        '11001',
        '10001',
        '10001',
        '11110',
    ],
    'c': [
        '00000',
        '00000',
        '01110',
        '10001',
        '10000',
        '10001',
        '01110',
    ],
    'd': [
        '00001',
        '00001',
        '01101',
        '10011',
        '10001',
        '10001',
        '01111',
    ],
    'e': [
        '00000',
        '00000',
        '01110',
        '10001',
        '11111',
        '10000',
        '01110',
    ],
    'f': [
        '00110',
        '01001',
        '01000',
        '11100',
        '01000',
        '01000',
        '01000',
    ],
    'g': [
        '00000',
        '00000',
        '01111',
        '10001',
        '01111',
        '00001',
        '01110',
    ],
    'h': [
        '10000',
        '10000',
        '10110',
        '11001',
        '10001',
        '10001',
        '10001',
    ],
    'i': [
        '00100',
        '00000',
        '01100',
        '00100',
        '00100',
        '00100',
        '01110',
    ],
    'j': [
        '00010',
        '00000',
        '00110',
        '00010',
        '00010',
        '10010',
        '01100',
    ],
    'k': [
        '10000',
        '10000',
        '10010',
        '10100',
        '11000',
        '10100',
        '10010',
    ],
    'l': [
        '01100',
        '00100',
        '00100',
        '00100',
        '00100',
        '00100',
        '01110',
    ],
    'm': [
        '00000',
        '00000',
        '11010',
        '10101',
        '10101',
        '10101',
        '10101',
    ],
    'n': [
        '00000',
        '00000',
        '10110',
        '11001',
        '10001',
        '10001',
        '10001',
    ],
    'o': [
        '00000',
        '00000',
        '01110',
        '10001',
        '10001',
        '10001',
        '01110',
    ],
    'p': [
        '00000',
        '00000',
        '11110',
        '10001',
        '11110',
        '10000',
        '10000',
    ],
    'q': [
        '00000',
        '00000',
        '01111',
        '10001',
        '01111',
        '00001',
        '00001',
    ],
    'r': [
        '00000',
        '00000',
        '10110',
        '11001',
        '10000',
        '10000',
        '10000',
    ],
    's': [
        '00000',
        '00000',
        '01111',
        '10000',
        '01110',
        '00001',
        '11110',
    ],
    't': [
        '01000',
        '01000',
        '11110',
        '01000',
        '01000',
        '01001',
        '00110',
    ],
    'u': [
        '00000',
        '00000',
        '10001',
        '10001',
        '10001',
        '10011',
        '01101',
    ],
    'v': [
        '00000',
        '00000',
        '10001',
        '10001',
        '10001',
        '01010',
        '00100',
    ],
    'w': [
        '00000',
        '00000',
        '10001',
        '10001',
        '10101',
        '10101',
        '01010',
    ],
    'x': [
        '00000',
        '00000',
        '10001',
        '01010',
        '00100',
        '01010',
        '10001',
    ],
    'y': [
        '00000',
        '00000',
        '10001',
        '10001',
        '01111',
        '00001',
        '01110',
    ],
    'z': [
        '00000',
        '00000',
        '11111',
        '00010',
        '00100',
        '01000',
        '11111',
    ],
}

def _set_pixel(g, x, y):
    if 0 <= x < 6 and 0 <= y < 12:
        g[y][x] = 1


def _hline(g, y, x0, x1):
    if x0 > x1:
        x0, x1 = x1, x0
    for x in range(x0, x1 + 1):
        _set_pixel(g, x, y)


def _vline(g, x, y0, y1):
    if y0 > y1:
        y0, y1 = y1, y0
    for y in range(y0, y1 + 1):
        _set_pixel(g, x, y)


def _diag_down(g, x0, y0, x1, y1):
    # 简单对角线：从 (x0,y0) 到 (x1,y1)，按步进取样
    dx = x1 - x0
    dy = y1 - y0
    steps = max(abs(dx), abs(dy))
    if steps == 0:
        _set_pixel(g, x0, y0)
        return
    for i in range(steps + 1):
        x = x0 + (dx * i) // steps
        y = y0 + (dy * i) // steps
        _set_pixel(g, x, y)


def _blank():
    return [[0 for _ in range(6)] for _ in range(12)]


def _pack(g):
    # 每行 1 字节，使用高位对齐：bit7 对应 x=0
    out = bytearray(12)
    for y in range(12):
        b = 0
        for x in range(6):
            if g[y][x]:
                b |= 1 << (7 - x)
        out[y] = b
    return bytes(out)


def _blit_5x7_to_6x12(g, pattern5x7):
    # 将 5x7 点阵按最近邻缩放到 6x12
    for y in range(12):
        sy = (y * 7) // 12
        row = pattern5x7[sy]
        for x in range(6):
            sx = (x * 5) // 6
            if row[sx] == '1':
                _set_pixel(g, x, y)


def _draw_7seg(g, segs):
    # 6x12 的 7 段数码风格
    # a:top, b:upper-right, c:lower-right, d:bottom, e:lower-left, f:upper-left, g:middle
    if 'a' in segs:
        _hline(g, 1, 1, 4)
    if 'd' in segs:
        _hline(g, 10, 1, 4)
    if 'g' in segs:
        _hline(g, 6, 1, 4)
    if 'f' in segs:
        _vline(g, 1, 2, 5)
    if 'b' in segs:
        _vline(g, 4, 2, 5)
    if 'e' in segs:
        _vline(g, 1, 7, 9)
    if 'c' in segs:
        _vline(g, 4, 7, 9)


def _draw_char(ch):
    g = _blank()

    # 优先使用 5x7 字形表生成（更统一）
    if ch in _FONT_5X7:
        _blit_5x7_to_6x12(g, _FONT_5X7[ch])
        return g

    # 控制：空格
    if ch == ' ':
        return g

    # 数字 0-9
    if '0' <= ch <= '9':
        mapping = {
            '0': 'abcfed',
            '1': 'bc',
            '2': 'abged',
            '3': 'abgcd',
            '4': 'fgbc',
            '5': 'afgcd',
            '6': 'afgcde',
            '7': 'abc',
            '8': 'abcdefg',
            '9': 'abfgcd',
        }
        _draw_7seg(g, mapping[ch])
        return g

    # 小写字母（用简化笔画，需与大写区分）
    if 'a' <= ch <= 'z':
        if ch == 'a':
            _hline(g, 6, 2, 4)
            _hline(g, 10, 2, 4)
            _vline(g, 1, 7, 9)
            _vline(g, 4, 6, 10)
            _hline(g, 8, 2, 4)
        elif ch == 'b':
            _vline(g, 1, 2, 10)
            _hline(g, 6, 2, 4)
            _hline(g, 8, 2, 4)
            _vline(g, 4, 7, 7)
            _vline(g, 4, 9, 9)
        elif ch == 'c':
            _hline(g, 6, 2, 4)
            _hline(g, 10, 2, 4)
            _vline(g, 1, 7, 9)
        elif ch == 'd':
            _vline(g, 4, 2, 10)
            _hline(g, 6, 1, 3)
            _hline(g, 10, 1, 3)
            _vline(g, 1, 7, 9)
        elif ch == 'e':
            _hline(g, 6, 2, 4)
            _hline(g, 10, 2, 4)
            _hline(g, 8, 2, 4)
            _vline(g, 1, 7, 9)
        elif ch == 'f':
            _vline(g, 3, 2, 10)
            _hline(g, 3, 2, 4)
            _hline(g, 6, 1, 4)
        elif ch == 'g':
            _hline(g, 6, 2, 4)
            _hline(g, 10, 2, 4)
            _vline(g, 1, 7, 9)
            _vline(g, 4, 6, 11)
            _hline(g, 8, 2, 4)
            _set_pixel(g, 2, 11)
        elif ch == 'h':
            _vline(g, 1, 2, 10)
            _hline(g, 6, 2, 4)
            _vline(g, 4, 7, 10)
        elif ch == 'i':
            _set_pixel(g, 3, 3)
            _vline(g, 3, 6, 10)
            _hline(g, 10, 2, 4)
        elif ch == 'j':
            _set_pixel(g, 3, 3)
            _vline(g, 3, 6, 11)
            _hline(g, 11, 1, 3)
        elif ch == 'k':
            _vline(g, 1, 2, 10)
            _diag_down(g, 4, 6, 2, 8)
            _diag_down(g, 2, 8, 4, 10)
        elif ch == 'l':
            _vline(g, 3, 2, 10)
            _hline(g, 10, 2, 4)
        elif ch == 'm':
            _vline(g, 1, 6, 10)
            _vline(g, 3, 7, 10)
            _vline(g, 4, 7, 10)
            _hline(g, 6, 1, 4)
        elif ch == 'n':
            _vline(g, 1, 6, 10)
            _hline(g, 6, 1, 4)
            _vline(g, 4, 7, 10)
        elif ch == 'o':
            _hline(g, 6, 2, 4)
            _hline(g, 10, 2, 4)
            _vline(g, 1, 7, 9)
            _vline(g, 4, 7, 9)
        elif ch == 'p':
            _vline(g, 1, 6, 11)
            _hline(g, 6, 2, 4)
            _hline(g, 8, 2, 4)
            _vline(g, 4, 7, 7)
        elif ch == 'q':
            _vline(g, 4, 6, 11)
            _hline(g, 6, 1, 3)
            _hline(g, 10, 1, 3)
            _vline(g, 1, 7, 9)
        elif ch == 'r':
            _vline(g, 2, 6, 10)
            _hline(g, 6, 2, 4)
            _set_pixel(g, 4, 7)
        elif ch == 's':
            _hline(g, 6, 2, 4)
            _hline(g, 8, 2, 4)
            _hline(g, 10, 2, 4)
            _set_pixel(g, 1, 7)
            _set_pixel(g, 4, 9)
        elif ch == 't':
            _vline(g, 3, 3, 10)
            _hline(g, 3, 2, 4)
            _hline(g, 6, 1, 4)
        elif ch == 'u':
            _vline(g, 1, 6, 9)
            _vline(g, 4, 6, 9)
            _hline(g, 10, 2, 4)
        elif ch == 'v':
            _diag_down(g, 1, 6, 3, 10)
            _diag_down(g, 4, 6, 3, 10)
        elif ch == 'w':
            _diag_down(g, 1, 6, 2, 10)
            _diag_down(g, 2, 10, 3, 8)
            _diag_down(g, 3, 8, 4, 10)
            _diag_down(g, 4, 10, 5, 6)
        elif ch == 'x':
            _diag_down(g, 1, 6, 4, 10)
            _diag_down(g, 4, 6, 1, 10)
        elif ch == 'y':
            _diag_down(g, 1, 6, 3, 10)
            _diag_down(g, 4, 6, 3, 10)
            _vline(g, 3, 10, 11)
        elif ch == 'z':
            _hline(g, 6, 1, 4)
            _hline(g, 10, 1, 4)
            _diag_down(g, 4, 7, 1, 9)
        return g

    # 大写字母（用简化笔画）
    if 'A' <= ch <= 'Z':
        if ch == 'A':
            _hline(g, 1, 1, 4)
            _vline(g, 1, 2, 10)
            _vline(g, 4, 2, 10)
            _hline(g, 6, 1, 4)
        elif ch == 'B':
            _vline(g, 1, 1, 10)
            _hline(g, 1, 1, 4)
            _hline(g, 6, 1, 4)
            _hline(g, 10, 1, 4)
            _vline(g, 4, 2, 5)
            _vline(g, 4, 7, 9)
        elif ch == 'C':
            _hline(g, 1, 1, 4)
            _hline(g, 10, 1, 4)
            _vline(g, 1, 2, 9)
        elif ch == 'D':
            _vline(g, 1, 1, 10)
            _hline(g, 1, 1, 3)
            _hline(g, 10, 1, 3)
            _vline(g, 4, 2, 9)
        elif ch == 'E':
            _vline(g, 1, 1, 10)
            _hline(g, 1, 1, 4)
            _hline(g, 6, 1, 4)
            _hline(g, 10, 1, 4)
        elif ch == 'F':
            _vline(g, 1, 1, 10)
            _hline(g, 1, 1, 4)
            _hline(g, 6, 1, 4)
        elif ch == 'G':
            _hline(g, 1, 1, 4)
            _hline(g, 10, 1, 4)
            _vline(g, 1, 2, 9)
            _hline(g, 6, 2, 4)
            _vline(g, 4, 6, 9)
        elif ch == 'H':
            _vline(g, 1, 1, 10)
            _vline(g, 4, 1, 10)
            _hline(g, 6, 1, 4)
        elif ch == 'I':
            _hline(g, 1, 1, 4)
            _hline(g, 10, 1, 4)
            _vline(g, 3, 2, 9)
        elif ch == 'J':
            _hline(g, 1, 1, 4)
            _vline(g, 3, 2, 9)
            _hline(g, 10, 1, 3)
            _vline(g, 1, 8, 9)
        elif ch == 'K':
            _vline(g, 1, 1, 10)
            _diag_down(g, 4, 1, 2, 6)
            _diag_down(g, 2, 6, 4, 10)
        elif ch == 'L':
            _vline(g, 1, 1, 10)
            _hline(g, 10, 1, 4)
        elif ch == 'M':
            _vline(g, 1, 1, 10)
            _vline(g, 4, 1, 10)
            _diag_down(g, 1, 1, 2, 4)
            _diag_down(g, 4, 1, 3, 4)
        elif ch == 'N':
            _vline(g, 1, 1, 10)
            _vline(g, 4, 1, 10)
            _diag_down(g, 1, 1, 4, 10)
        elif ch == 'O':
            _hline(g, 1, 1, 4)
            _hline(g, 10, 1, 4)
            _vline(g, 1, 2, 9)
            _vline(g, 4, 2, 9)
        elif ch == 'P':
            _vline(g, 1, 1, 10)
            _hline(g, 1, 1, 4)
            _hline(g, 6, 1, 4)
            _vline(g, 4, 2, 5)
        elif ch == 'Q':
            _hline(g, 1, 1, 4)
            _hline(g, 10, 1, 4)
            _vline(g, 1, 2, 9)
            _vline(g, 4, 2, 9)
            _set_pixel(g, 3, 9)
            _set_pixel(g, 4, 11)
        elif ch == 'R':
            _vline(g, 1, 1, 10)
            _hline(g, 1, 1, 4)
            _hline(g, 6, 1, 4)
            _vline(g, 4, 2, 5)
            _diag_down(g, 2, 6, 4, 10)
        elif ch == 'S':
            _hline(g, 1, 1, 4)
            _hline(g, 6, 1, 4)
            _hline(g, 10, 1, 4)
            _vline(g, 1, 2, 5)
            _vline(g, 4, 7, 9)
        elif ch == 'T':
            _hline(g, 1, 1, 4)
            _vline(g, 3, 2, 10)
        elif ch == 'U':
            _vline(g, 1, 1, 9)
            _vline(g, 4, 1, 9)
            _hline(g, 10, 1, 4)
        elif ch == 'V':
            _diag_down(g, 1, 1, 3, 10)
            _diag_down(g, 4, 1, 3, 10)
        elif ch == 'W':
            _vline(g, 1, 1, 10)
            _vline(g, 4, 1, 10)
            _diag_down(g, 1, 10, 2, 7)
            _diag_down(g, 4, 10, 3, 7)
        elif ch == 'X':
            _diag_down(g, 1, 1, 4, 10)
            _diag_down(g, 4, 1, 1, 10)
        elif ch == 'Y':
            _diag_down(g, 1, 1, 3, 6)
            _diag_down(g, 4, 1, 3, 6)
            _vline(g, 3, 6, 10)
        elif ch == 'Z':
            _hline(g, 1, 1, 4)
            _hline(g, 10, 1, 4)
            _diag_down(g, 4, 2, 1, 9)
        return g

    # 常用标点
    if ch == '.':
        _set_pixel(g, 3, 10)
        return g
    if ch == ',':
        _set_pixel(g, 3, 10)
        _set_pixel(g, 2, 11)
        return g
    if ch == ':':
        _set_pixel(g, 3, 5)
        _set_pixel(g, 3, 10)
        return g
    if ch == ';':
        _set_pixel(g, 3, 5)
        _set_pixel(g, 3, 10)
        _set_pixel(g, 2, 11)
        return g
    if ch == '!':
        _vline(g, 3, 2, 8)
        _set_pixel(g, 3, 10)
        return g
    if ch == '?':
        _hline(g, 1, 1, 4)
        _vline(g, 4, 2, 4)
        _set_pixel(g, 3, 6)
        _set_pixel(g, 3, 8)
        _set_pixel(g, 3, 10)
        return g
    if ch == '-':
        _hline(g, 6, 1, 4)
        return g
    if ch == '_':
        _hline(g, 10, 1, 4)
        return g
    if ch == '+':
        _hline(g, 6, 1, 4)
        _vline(g, 3, 4, 8)
        return g
    if ch == '=':
        _hline(g, 5, 1, 4)
        _hline(g, 7, 1, 4)
        return g
    if ch == '/':
        _diag_down(g, 4, 1, 1, 10)
        return g
    if ch == '\\':
        _diag_down(g, 1, 1, 4, 10)
        return g
    if ch == '(':
        _vline(g, 2, 2, 9)
        _set_pixel(g, 3, 1)
        _set_pixel(g, 3, 10)
        return g
    if ch == ')':
        _vline(g, 3, 2, 9)
        _set_pixel(g, 2, 1)
        _set_pixel(g, 2, 10)
        return g
    if ch == '[':
        _vline(g, 2, 2, 9)
        _hline(g, 1, 2, 4)
        _hline(g, 10, 2, 4)
        return g
    if ch == ']':
        _vline(g, 3, 2, 9)
        _hline(g, 1, 1, 3)
        _hline(g, 10, 1, 3)
        return g
    if ch == '*':
        _set_pixel(g, 3, 5)
        _set_pixel(g, 2, 6)
        _set_pixel(g, 4, 6)
        _set_pixel(g, 3, 7)
        _set_pixel(g, 2, 8)
        _set_pixel(g, 4, 8)
        return g
    if ch == "'":
        _set_pixel(g, 3, 2)
        _set_pixel(g, 3, 3)
        return g
    if ch == '"':
        _set_pixel(g, 2, 2)
        _set_pixel(g, 2, 3)
        _set_pixel(g, 4, 2)
        _set_pixel(g, 4, 3)
        return g

    # 其它未覆盖字符：显示为 ?
    return _draw_char('?')


_U8G2_6X12_BDF_URL = "https://raw.githubusercontent.com/olikraus/u8g2/refs/heads/master/tools/font/bdf/6x12.bdf"


def _gen_custom(out_path: str):
    # 0x20-0x7F 共 96 个字符，最后一个 DEL 留空
    data = bytearray()
    for code in range(0x20, 0x80):
        ch = chr(code)
        if code == 0x7F:
            g = _blank()
        else:
            g = _draw_char(ch)
        data.extend(_pack(g))

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, 'wb') as f:
        f.write(data)


def _fetch_text_from_url(url: str) -> str:
    with urllib.request.urlopen(url) as resp:
        data = resp.read()
    return data.decode('utf-8', errors='replace')


def _normalize_bdf_row_byte(row_byte: int) -> int:
    # ZhFont.cpp 读取 6 列：bit7..bit2，对应 col 0..5。
    # BDF 单字节行通常已左对齐（bit7 为最左），这里额外做一次归一化。
    b = row_byte & 0xFF
    if (b & 0x03) != 0 and (b & 0xC0) == 0:
        b = (b << 2) & 0xFC
    return b & 0xFC


def _parse_bdf_extract_ascii_6x12(bdf_text: str) -> dict:
    glyphs: dict[int, list[int]] = {}

    encoding = None
    bbx_w = None
    bbx_h = None
    in_bitmap = False
    bitmap_rows: list[str] = []

    def _finish_glyph():
        nonlocal encoding, bbx_w, bbx_h, in_bitmap, bitmap_rows

        if encoding is None:
            return
        if encoding < 0x20 or encoding > 0x7E:
            return
        if bbx_w is None or bbx_h is None:
            return

        rows: list[int] = []
        for row_hex in bitmap_rows:
            row_hex = row_hex.strip()
            if not row_hex:
                rows.append(0)
                continue
            row_bytes = bytes.fromhex(row_hex)
            row_byte = int(row_bytes[0]) if len(row_bytes) > 0 else 0
            rows.append(_normalize_bdf_row_byte(row_byte))

        # 保持固定 12 行（不足补 0，多余截断）
        if len(rows) < 12:
            rows.extend([0] * (12 - len(rows)))
        elif len(rows) > 12:
            rows = rows[:12]

        glyphs[int(encoding)] = rows

    for raw in bdf_text.splitlines():
        line = raw.strip()
        if not line:
            continue

        if line.startswith('STARTCHAR'):
            encoding = None
            bbx_w = None
            bbx_h = None
            in_bitmap = False
            bitmap_rows = []
            continue

        if line.startswith('ENCODING'):
            parts = line.split()
            if len(parts) >= 2:
                try:
                    encoding = int(parts[1])
                except ValueError:
                    encoding = None
            continue

        if line.startswith('BBX'):
            parts = line.split()
            if len(parts) >= 3:
                try:
                    bbx_w = int(parts[1])
                    bbx_h = int(parts[2])
                except ValueError:
                    bbx_w = None
                    bbx_h = None
            continue

        if line == 'BITMAP':
            in_bitmap = True
            bitmap_rows = []
            continue

        if line == 'ENDCHAR':
            _finish_glyph()
            encoding = None
            bbx_w = None
            bbx_h = None
            in_bitmap = False
            bitmap_rows = []
            continue

        if in_bitmap:
            bitmap_rows.append(line)

    return glyphs


def _gen_from_u8g2_bdf(out_path: str):
    bdf_text = _fetch_text_from_url(_U8G2_6X12_BDF_URL)
    glyphs = _parse_bdf_extract_ascii_6x12(bdf_text)

    # 0x20-0x7F 共 96 个字符，最后一个 DEL 留空
    data = bytearray()
    fallback = glyphs.get(0x3F, [0] * 12)  # '?' 兜底

    for code in range(0x20, 0x80):
        if code == 0x7F:
            rows = [0] * 12
        else:
            rows = glyphs.get(code, fallback)

        if len(rows) != 12:
            raise RuntimeError('BDF glyph height mismatch')
        for b in rows:
            data.append(int(b) & 0xFF)

    if len(data) != 1152:
        raise RuntimeError('ASC12 size mismatch')

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, 'wb') as f:
        f.write(data)


def gen(out_path: str):
    _gen_from_u8g2_bdf(out_path)


if __name__ == '__main__':
    out = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'ASC12')
    gen(out)
    print('written', out)
