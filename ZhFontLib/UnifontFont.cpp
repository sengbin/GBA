/*------------------------------------------------------------------------
名称：Unifont 16x16 字库绘制实现
说明：基于 GNU Unifont 16x16 点阵字库，在 GBA Mode3 上绘制 UTF-8 文本
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-07
备注：字库数据为固定表（0x0000-0xFFFF，每码位 32 字节）
------------------------------------------------------------------------*/

#include "UnifontFont.h"

extern const unsigned char g_unifont16x16_bin_start[];
extern const unsigned char g_unifont16x16_bin_end[];

static inline void PlotPixelMode3(int x, int y, u16 color)
{
    volatile u16* vram = (volatile u16*)0x06000000;
    vram[y * 240 + x] = color;
}

static inline const unsigned char* GetGlyph16x16(unsigned int codepoint)
{
    if (codepoint > 0xFFFF)
        return nullptr;

    const unsigned int offset = codepoint * 32;
    const unsigned int size = (unsigned int)(g_unifont16x16_bin_end - g_unifont16x16_bin_start);

    if (offset + 32 > size)
        return nullptr;

    return g_unifont16x16_bin_start + offset;
}

static unsigned int Utf8NextCodepoint(const char*& p)
{
    const unsigned char c0 = (unsigned char)p[0];
    if (c0 == 0)
        return 0;

    if (c0 < 0x80)
    {
        ++p;
        return c0;
    }

    if ((c0 & 0xE0) == 0xC0)
    {
        const unsigned char c1 = (unsigned char)p[1];
        if ((c1 & 0xC0) != 0x80)
        {
            ++p;
            return 0xFFFD;
        }
        const unsigned int cp = ((unsigned int)(c0 & 0x1F) << 6) | (unsigned int)(c1 & 0x3F);
        p += 2;
        return cp;
    }

    if ((c0 & 0xF0) == 0xE0)
    {
        const unsigned char c1 = (unsigned char)p[1];
        const unsigned char c2 = (unsigned char)p[2];
        if (((c1 & 0xC0) != 0x80) || ((c2 & 0xC0) != 0x80))
        {
            ++p;
            return 0xFFFD;
        }
        const unsigned int cp = ((unsigned int)(c0 & 0x0F) << 12)
            | ((unsigned int)(c1 & 0x3F) << 6)
            | (unsigned int)(c2 & 0x3F);
        p += 3;
        return cp;
    }

    if ((c0 & 0xF8) == 0xF0)
    {
        const unsigned char c1 = (unsigned char)p[1];
        const unsigned char c2 = (unsigned char)p[2];
        const unsigned char c3 = (unsigned char)p[3];
        if (((c1 & 0xC0) != 0x80) || ((c2 & 0xC0) != 0x80) || ((c3 & 0xC0) != 0x80))
        {
            ++p;
            return 0xFFFD;
        }
        const unsigned int cp = ((unsigned int)(c0 & 0x07) << 18)
            | ((unsigned int)(c1 & 0x3F) << 12)
            | ((unsigned int)(c2 & 0x3F) << 6)
            | (unsigned int)(c3 & 0x3F);
        p += 4;
        return cp;
    }

    ++p;
    return 0xFFFD;
}

/**
 * @brief 判断当前码点是否按半角宽度绘制
 * @param codepoint Unicode 码点
 * @return true=半角（8 像素宽），false=全角（16 像素宽）
 */
static inline bool IsHalfWidthCodepoint(unsigned int codepoint)
{
    /* 基础 ASCII 可见字符按半角处理（英文/数字/常用符号） */
    return (codepoint >= 0x20 && codepoint <= 0x7E);
}

/**
 * @brief 获取当前码点的像素步进宽度
 * @param codepoint Unicode 码点
 * @return 像素宽度（半角 8 / 全角 16）
 */
static inline int GetAdvanceWidth(unsigned int codepoint)
{
    return IsHalfWidthCodepoint(codepoint) ? 8 : 16;
}

static void DrawGlyphMode3(unsigned int codepoint, int x, int y, u16 color, int drawWidth)
{
    const unsigned char* g = GetGlyph16x16(codepoint);
    if (!g)
        return;

    for (int row = 0; row < 16; ++row)
    {
        const unsigned int hi = (unsigned int)g[row * 2 + 0];
        const unsigned int lo = (unsigned int)g[row * 2 + 1];
        const unsigned int bits = (hi << 8) | lo;

        for (int col = 0; col < drawWidth; ++col)
        {
            if (bits & (1u << (15 - col)))
            {
                const int px = x + col;
                const int py = y + row;
                if (px >= 0 && px < 240 && py >= 0 && py < 160)
                    PlotPixelMode3(px, py, color);
            }
        }
    }
}

int Unifont_GetUtf8TextWidth16(const char* utf8)
{
    if (!utf8)
        return 0;

    int width = 0;
    const char* p = utf8;
    while (*p)
    {
        const unsigned int cp = Utf8NextCodepoint(p);
        if (cp == 0)
            break;
        if (cp == '\n')
            break;

        width += GetAdvanceWidth(cp);
    }
    return width;
}

void Unifont_DrawUtf8TextMode3(const char* utf8, int x, int y, u16 color)
{
    if (!utf8)
        return;

    int cx = x;
    const char* p = utf8;
    while (*p)
    {
        const unsigned int cp = Utf8NextCodepoint(p);
        if (cp == 0)
            break;
        if (cp == '\n')
            break;

        const int advance = GetAdvanceWidth(cp);
        const int drawWidth = advance;

        if (cx >= 240)
            break;

        DrawGlyphMode3(cp, cx, y, color, drawWidth);
        cx += advance;

        if (cx >= 240)
            break;
    }
}
