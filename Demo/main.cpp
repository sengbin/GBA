/*------------------------------------------------------------------------
名称：GBA Mode3 示例
说明：最小 Mode 3（位图）示例，十字键移动并显示鼠标图案与顶部文字
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-06
备注：教程示例，使用 libgba
------------------------------------------------------------------------*/

#include <gba.h>
#include "font_unifont16.h"

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

    const char* textUtf8 = "你好GBA"; /* 顶部显示的一行 UTF-8 文字，支持中文 */
    int textWidth = Unifont16_MeasureWidth(textUtf8);
    int textX = (240 - textWidth) / 2;
    int textY = 8; /* 留出顶部边距，避免贴边 */
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
        Unifont16_DrawString(textUtf8, textX, textY, white, black, 1);
    }
    return 0;
}