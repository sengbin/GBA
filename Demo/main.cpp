/*------------------------------------------------------------------------
名称：GBA Mode3 示例
说明：最小 Mode 3（位图）示例，十字键移动并显示鼠标图案与顶部文字
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-06
备注：教程示例，使用 libgba
------------------------------------------------------------------------*/

#include <gba.h>

#include "UnifontFont.h"

/**
 * @brief 在 Mode 3 帧缓冲写像素
 * @param x X 坐标（0-239）
 * @param y Y 坐标（0-159）
 * @param color BGR555 格式颜色
 */
static inline void PlotPixel(int x, int y, u16 color)
{
    volatile u16* vram = (volatile u16*)0x06000000;
    vram[y * 240 + x] = color;
}

/**
 * @brief 画一个简单的鼠标箭头图案（以箭头尖为定位点）
 * @param x 箭头尖 X 坐标
 * @param y 箭头尖 Y 坐标
 * @param color 箭头颜色
 * @param bg 背景色（保留，可用于未来描边）
 */
static void DrawCursor(int x, int y, u16 color, u16 bg)
{
    /* 11x7 箭头图案，1 为绘制像素 （顶部为箭头尖） */
    const uint8_t cursor[11] = {
        0b1000000,
        0b1100000,
        0b1010000,
        0b1001000,
        0b1000100,
        0b1000010,
        0b1000001,
        0b1000001,
        0b1000001,
        0b1000001,
        0b1111111
    };
    int h = 11, w = 7;
    for (int ry = 0; ry < h; ++ry)
    {
        for (int rx = 0; rx < w; ++rx)
        {
            if (cursor[ry] & (1 << (w - 1 - rx)))
            {
                int px = x + rx;
                int py = y + ry;
                if (px >= 0 && px < 240 && py >= 0 && py < 160)
                    PlotPixel(px, py, color);
            }
        }
    }
}

/**
 * @brief 使用内置 5x7 字模在 Mode3 上绘制一行文本（仅支持大写字母和空格）
 * @param str 要显示的字符串（建议大写 ASCII）
 * @param x 起始 X（像素）
 * @param y 起始 Y（像素）
 * @param color 文本颜色
 */
[[maybe_unused]] static void DrawText(const char* str, int x, int y, u16 color)
{
    /*
     * 完整 5x7 字库（索引 0 为空格，1..26 对应 A..Z）
     * 每字符 7 行，每行低 5 位表示像素（从高位到低位为列 0..4）
     */
    static const uint8_t fontAZ[27][7] = {
        /* ' ' */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
        /* 'A' */ {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},
        /* 'B' */ {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
        /* 'C' */ {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
        /* 'D' */ {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E},
        /* 'E' */ {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
        /* 'F' */ {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
        /* 'G' */ {0x0E,0x11,0x10,0x13,0x11,0x11,0x0E},
        /* 'H' */ {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
        /* 'I' */ {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
        /* 'J' */ {0x07,0x02,0x02,0x02,0x12,0x12,0x0C},
        /* 'K' */ {0x11,0x12,0x14,0x18,0x14,0x12,0x11},
        /* 'L' */ {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
        /* 'M' */ {0x11,0x1B,0x15,0x15,0x11,0x11,0x11},
        /* 'N' */ {0x11,0x19,0x15,0x13,0x11,0x11,0x11},
        /* 'O' */ {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
        /* 'P' */ {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
        /* 'Q' */ {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},
        /* 'R' */ {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
        /* 'S' */ {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E},
        /* 'T' */ {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
        /* 'U' */ {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
        /* 'V' */ {0x11,0x11,0x11,0x11,0x0A,0x0A,0x04},
        /* 'W' */ {0x11,0x11,0x11,0x15,0x15,0x1B,0x11},
        /* 'X' */ {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
        /* 'Y' */ {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
        /* 'Z' */ {0x1F,0x02,0x04,0x08,0x10,0x10,0x1F}
    };

    int cx = x;
    for (const char* p = str; *p; ++p)
    {
        char c = *p;
        if (c >= 'a' && c <= 'z') c -= 32; /* 转大写 */
        int idx = 0; /* 默认空格 */
        if (c >= 'A' && c <= 'Z') idx = (c - 'A') + 1;

        const uint8_t* g = fontAZ[idx];
        for (int row = 0; row < 7; ++row)
        {
            uint8_t bits = g[row];
            for (int col = 0; col < 5; ++col)
            {
                if (bits & (1 << (4 - col)))
                {
                    int px = cx + col;
                    int py = y + row;
                    if (px >= 0 && px < 240 && py >= 0 && py < 160)
                        PlotPixel(px, py, color);
                }
            }
        }
        cx += 7; /* 字间距 2 像素 */
    }
}

int main(void)
{
    irqInit();
    irqEnable(IRQ_VBLANK);

    // 设置为 Mode 3（位图）并启用 BG2
    REG_DISPCNT = MODE_3 | BG2_ENABLE;

    int x = 120, y = 80;
    const u16 black = RGB5(0, 0, 0);
    const u16 white = RGB5(31,31,31);
    const u16 cursorColor = RGB5(31,31,0); // 黄色

    const char* text = u8"中文测试"; /* 顶部显示的一行文字（UTF-8） */
    int textWidth = Unifont_GetUtf8TextWidth16(text);
    int textX = (240 - textWidth) / 2;
    if (textX < 0) textX = 0;
    int textY = 0; /* 最顶行 */
    int minCursorY = textY + 16; /* 保留顶部安全区，避免光标遮挡文字 */

    while (1)
    {
        VBlankIntrWait();
        scanKeys();
        u16 keys = keysHeld();

        /* 清屏：使用 DMA 快速填充，避免 CPU 循环超出 VBlank 导致局部显示异常 */
        DMA3COPY(&black, (void*)0x06000000, DMA16 | (240 * 160) | DMA_SRC_FIXED);

        /* 按键移动鼠标图案 */
        if (keys & KEY_LEFT)  x--;
        if (keys & KEY_RIGHT) x++;
        if (keys & KEY_UP)    y--;
        if (keys & KEY_DOWN)  y++;

        /* 限制边界（图案尺寸 7x11），并防止光标进入文字区域 */
        if (x < 0) x = 0;
        if (x > 240 - 7) x = 240 - 7;
        if (y < minCursorY) y = minCursorY;
        if (y > 160 - 11) y = 160 - 11;

        DrawCursor(x, y, cursorColor, black);

        /* 绘制顶部居中文字（最后绘制，保证可见） */
        Unifont_DrawUtf8TextMode3(text, textX, textY, white);
    }
    return 0;
}