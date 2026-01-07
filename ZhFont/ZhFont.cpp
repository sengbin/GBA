/*------------------------------------------------------------------------
名称：ZhFont 12x12 字库绘制实现
说明：基于 HZK12 + ASC12（GB2312/ASCII），在 GBA Mode3 上绘制文本
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-08
备注：HZK12 区码范围 0xA1-0xF8，位码范围 0xA1-0xFE
------------------------------------------------------------------------*/

#include "ZhFont.h"

#include "ZhFontGb2312Table.h"

extern const unsigned char g_zhfont_hzk12_start[];
extern const unsigned char g_zhfont_hzk12_end[];
extern const unsigned char g_zhfont_asc12_start[];
extern const unsigned char g_zhfont_asc12_end[];

static inline void PlotPixelMode3(int x, int y, u16 color)
{
    volatile u16* vram = (volatile u16*)0x06000000;
    vram[y * 240 + x] = color;
}

static inline const unsigned char* GetHzk12Glyph(unsigned char high, unsigned char low)
{
    if (high < 0xA1 || high > 0xF8)
        return nullptr;
    if (low < 0xA1 || low > 0xFE)
        return nullptr;

    const unsigned int qu = (unsigned int)(high - 0xA1);
    const unsigned int wei = (unsigned int)(low - 0xA1);
    const unsigned int index = qu * 94u + wei;
    const unsigned int offset = index * 24u;

    const unsigned int size = (unsigned int)(g_zhfont_hzk12_end - g_zhfont_hzk12_start);
    if (offset + 24u > size)
        return nullptr;

    return g_zhfont_hzk12_start + offset;
}

static inline const unsigned char* GetAsc12Glyph(unsigned char ch)
{
    if (ch < 0x20 || ch > 0x7E)
        return nullptr;

    const unsigned int index = (unsigned int)(ch - 0x20);
    const unsigned int offset = index * 12u;

    const unsigned int size = (unsigned int)(g_zhfont_asc12_end - g_zhfont_asc12_start);
    if (offset + 12u > size)
        return nullptr;

    return g_zhfont_asc12_start + offset;
}

static inline bool IsAsciiHalfWidth(unsigned char c)
{
    return (c >= 0x20 && c <= 0x7E);
}

static inline int GetAdvanceWidth(unsigned char c)
{
    return IsAsciiHalfWidth(c) ? 6 : 12;
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

static bool UnicodeToGb2312(unsigned int codepoint, unsigned char* outHigh, unsigned char* outLow)
{
    unsigned int left = 0;
    unsigned int right = g_zhfont_unicode_to_gb2312_map_count;

    while (left < right)
    {
        const unsigned int mid = left + ((right - left) >> 1);
        const unsigned int key = (unsigned int)g_zhfont_unicode_to_gb2312_map[mid].unicode;
        if (codepoint == key)
        {
            const unsigned int gb = (unsigned int)g_zhfont_unicode_to_gb2312_map[mid].gb2312;
            *outHigh = (unsigned char)((gb >> 8) & 0xFF);
            *outLow = (unsigned char)(gb & 0xFF);
            return true;
        }
        if (codepoint < key)
        {
            right = mid;
        }
        else
        {
            left = mid + 1;
        }
    }

    return false;
}

static void DrawHzk12GlyphMode3(unsigned char high, unsigned char low, int x, int y, u16 color)
{
    const unsigned char* g = GetHzk12Glyph(high, low);
    if (!g)
        return;

    for (int row = 0; row < 12; ++row)
    {
        const unsigned int hi = (unsigned int)g[row * 2 + 0];
        const unsigned int lo = (unsigned int)g[row * 2 + 1];
        const unsigned int bits = (hi << 8) | lo;

        for (int col = 0; col < 12; ++col)
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

static void DrawAsc12GlyphMode3(unsigned char ch, int x, int y, u16 color)
{
    const unsigned char* g = GetAsc12Glyph(ch);
    if (!g)
        return;

    for (int row = 0; row < 12; ++row)
    {
        const unsigned int bits = (unsigned int)g[row];
        for (int col = 0; col < 6; ++col)
        {
            if (bits & (1u << (7 - col)))
            {
                const int px = x + col;
                const int py = y + row;
                if (px >= 0 && px < 240 && py >= 0 && py < 160)
                    PlotPixelMode3(px, py, color);
            }
        }
    }
}

int ZhFont_GetGb2312TextWidth12(const char* gb2312)
{
    if (!gb2312)
        return 0;

    int width = 0;
    const unsigned char* p = (const unsigned char*)gb2312;
    while (*p)
    {
        const unsigned char c0 = p[0];
        if (c0 == '\n')
            break;

        if (c0 < 0x80)
        {
            width += GetAdvanceWidth(c0);
            ++p;
            continue;
        }

        const unsigned char c1 = p[1];
        if (c1 == 0)
            break;

        width += 12;
        p += 2;
    }

    return width;
}

void ZhFont_DrawGb2312TextMode3(const char* gb2312, int x, int y, u16 color)
{
    if (!gb2312)
        return;

    int cx = x;
    const unsigned char* p = (const unsigned char*)gb2312;
    while (*p)
    {
        const unsigned char c0 = p[0];
        if (c0 == '\n')
            break;

        if (c0 < 0x80)
        {
            if (cx >= 240)
                break;

            DrawAsc12GlyphMode3(c0, cx, y, color);
            cx += GetAdvanceWidth(c0);
            ++p;
            continue;
        }

        const unsigned char c1 = p[1];
        if (c1 == 0)
            break;

        if (cx >= 240)
            break;

        DrawHzk12GlyphMode3(c0, c1, cx, y, color);
        cx += 12;
        p += 2;

        if (cx >= 240)
            break;
    }
}

int ZhFont_GetUtf8TextWidth12(const char* utf8)
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

        if (cp < 0x80)
        {
            width += GetAdvanceWidth((unsigned char)cp);
        }
        else
        {
            width += 12;
        }
    }

    return width;
}

void ZhFont_DrawUtf8TextMode3(const char* utf8, int x, int y, u16 color)
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

        if (cp < 0x80)
        {
            if (cx >= 240)
                break;

            DrawAsc12GlyphMode3((unsigned char)cp, cx, y, color);
            cx += GetAdvanceWidth((unsigned char)cp);
            continue;
        }

        if (cx >= 240)
            break;

        unsigned char high = 0;
        unsigned char low = 0;
        if (UnicodeToGb2312(cp, &high, &low))
        {
            DrawHzk12GlyphMode3(high, low, cx, y, color);
        }

        cx += 12;
        if (cx >= 240)
            break;
    }
}
