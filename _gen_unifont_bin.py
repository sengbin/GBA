import sys
src = r"d:\\GitCode\\GBA-Github\\中文字库\\unifont-17.0.03.hex"
dst = r"d:\\GitCode\\GBA-Github\\Demo\\unifont16x16.bin"
out = bytearray(0x10000 * 32)
with open(src, "r", encoding="utf-8", errors="ignore") as f:
    for line in f:
        line = line.strip()
        if not line or ":" not in line:
            continue
        cp_s, data_s = line.split(":", 1)
        try:
            cp = int(cp_s, 16)
        except Exception:
            continue
        if cp < 0 or cp > 0xFFFF:
            continue
        data_s = data_s.strip()
        if len(data_s) == 32:
            b = bytes.fromhex(data_s)
            buf = bytearray(32)
            for i in range(16):
                buf[2 * i] = b[i]
                buf[2 * i + 1] = 0
        elif len(data_s) == 64:
            buf = bytes.fromhex(data_s)
        else:
            continue
        off = cp * 32
        out[off:off + 32] = buf
with open(dst, "wb") as wf:
    wf.write(out)
print("written", len(out), "bytes")