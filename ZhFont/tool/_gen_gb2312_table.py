import os

def gen(out_path: str):
    items = []

    # 仅扫描 HZK12 覆盖的区位码范围：高字节 0xA1-0xF8，低字节 0xA1-0xFE
    for high in range(0xA1, 0xF8 + 1):
        for low in range(0xA1, 0xFE + 1):
            b = bytes([high, low])
            try:
                s = b.decode("gb2312")
            except Exception:
                continue
            if len(s) != 1:
                continue
            cp = ord(s)
            gb = (high << 8) | low
            items.append((cp, gb))

    items.sort(key=lambda x: x[0])

    lines = []
    lines.append("/*------------------------------------------------------------------------")
    lines.append("名称：GB2312 转换表")
    lines.append("说明：Unicode 码点到 GB2312 双字节编码的映射表，用于 UTF-8 转 GB2312 绘制")
    lines.append("作者：Lion")
    lines.append("邮箱：chengbin@3578.cn")
    lines.append("日期：2026-01-08")
    lines.append("备注：由工具脚本 tool/_gen_gb2312_table.py 生成")
    lines.append("------------------------------------------------------------------------*/")
    lines.append("")
    lines.append("#pragma once")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("")
    lines.append("typedef struct ZhFontUnicodeToGb2312Map")
    lines.append("{")
    lines.append("    uint16_t unicode;")
    lines.append("    uint16_t gb2312; /* 高字节在高 8 位 */")
    lines.append("} ZhFontUnicodeToGb2312Map;")
    lines.append("")
    lines.append("static const ZhFontUnicodeToGb2312Map g_zhfont_unicode_to_gb2312_map[] = {")

    for cp, gb in items:
        lines.append(f"    {{0x{cp:04X}, 0x{gb:04X}}},")

    lines.append("};")
    lines.append("")
    lines.append("static const unsigned int g_zhfont_unicode_to_gb2312_map_count =")
    lines.append("    (unsigned int)(sizeof(g_zhfont_unicode_to_gb2312_map) / sizeof(g_zhfont_unicode_to_gb2312_map[0]));")
    lines.append("")

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))


if __name__ == "__main__":
    out = os.path.join(os.path.dirname(os.path.dirname(__file__)), "ZhFontGb2312Table.h")
    gen(out)
    print("written", out)
