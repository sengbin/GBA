import os

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


def gen(out_path: str):
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


if __name__ == '__main__':
    out = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'ASC12')
    gen(out)
    print('written', out)
