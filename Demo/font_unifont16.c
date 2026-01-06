/*------------------------------------------------------------------------
名称：Unifont16 字库实现
说明：提供 Unifont 16x16 点阵获取与 Mode 3 绘制实现
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-07
备注：通用复用版
------------------------------------------------------------------------*/

#include "font_unifont16.h"

/* 全量点阵数据表，由 unifont-17.0.03.hex 转换生成 */
const uint8_t g_Unifont16[65536][32] = {
#include "font_unifont16_data.inc"
};

static inline void PutPixel(int x, int y, u16 color)
{
    volatile u16* vram = (volatile u16*)0x06000000;
    vram[y * 240 + x] = color;
}

/**
 * @brief 获取指定 Unicode 码点的 16x16 点阵（每行两字节，高位在前）。
 */
const uint8_t* Unifont16_GetGlyph(u32 codepoint)
{
    if (codepoint <= 0xFFFF)
    {
        return g_Unifont16[codepoint];
    }
    return 0;
}

/* 简单 UTF-8 解码，返回消耗的字节数，遇到无效字节回退为单字节跳过 */
static int Utf8NextCodepoint(const char* p, u32* codepoint)
{
    const uint8_t c0 = (uint8_t)p[0];
    if (c0 == 0)
    {
        return 0;
    }
    if (c0 < 0x80)
    {
        *codepoint = c0;
        return 1;
    }

    if (p[1] == 0)
    {
        *codepoint = c0;
        return 1;
    }

    const uint8_t c1 = (uint8_t)p[1];
    if ((c0 & 0xE0) == 0xC0 && c1 >= 0x80 && c1 <= 0xBF)
    {
        *codepoint = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
        return 2;
    }

    if (p[2] == 0)
    {
        *codepoint = c0;
        return 1;
    }

    const uint8_t c2 = (uint8_t)p[2];
    if ((c0 & 0xF0) == 0xE0 && c1 >= 0x80 && c1 <= 0xBF && c2 >= 0x80 && c2 <= 0xBF)
    {
        *codepoint = ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
        return 3;
    }

    if (p[3] == 0)
    {
        *codepoint = c0;
        return 1;
    }

    const uint8_t c3 = (uint8_t)p[3];
    if ((c0 & 0xF8) == 0xF0 && c1 >= 0x80 && c1 <= 0xBF && c2 >= 0x80 && c2 <= 0xBF && c3 >= 0x80 && c3 <= 0xBF)
    {
        *codepoint = ((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        return 4;
    }

    *codepoint = c0;
    return 1;
}

/* 绘制单个 16x16 点阵，支持透明背景 */
static void DrawGlyph16(int x, int y, const uint8_t* glyph, u16 color, u16 bg, int transparentBg)
{
    for (int row = 0; row < 16; ++row)
    {
        u16 bits = ((u16)glyph[row * 2] << 8) | glyph[row * 2 + 1];
        int py = y + row;
        if (py < 0 || py >= 160)
        {
            continue;
        }
        for (int col = 0; col < 16; ++col)
        {
            int px = x + col;
            if (px < 0 || px >= 240)
            {
                continue;
            }
            if (bits & (0x8000 >> col))
            {
                PutPixel(px, py, color);
            }
            else if (!transparentBg)
            {
                PutPixel(px, py, bg);
            }
        }
    }
}

int Unifont16_MeasureWidth(const char* utf8)
{
    const int glyphWidth = 16;
    const int gap = 1;
    int count = 0;
    const char* p = utf8;
    while (p && *p)
    {
        u32 codepoint = 0;
        int used = Utf8NextCodepoint(p, &codepoint);
        if (used <= 0)
        {
            break;
        }
        p += used;
        ++count;
    }
    if (count == 0)
    {
        return 0;
    }
    return count * (glyphWidth + gap) - gap;
}

void Unifont16_DrawString(const char* utf8, int x, int y, u16 color, u16 bg, int transparentBg)
{
    const int glyphWidth = 16;
    const int gap = 1;
    int cx = x;
    const char* p = utf8;
    while (p && *p)
    {
        u32 codepoint = 0;
        int used = Utf8NextCodepoint(p, &codepoint);
        if (used <= 0)
        {
            break;
        }
        p += used;

        const uint8_t* glyph = Unifont16_GetGlyph(codepoint);
        if (glyph)
        {
            DrawGlyph16(cx, y, glyph, color, bg, transparentBg);
        }
        cx += glyphWidth + gap;
    }
}
