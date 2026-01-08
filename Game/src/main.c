#include <gba.h>
#include "../assets/tiles.h"
#include "../assets/map.h"

/**
 * 初始化显示为 Mode 3（帧缓冲模式）并启用 BG2。
 * 本函数用于示例展示，便于快速运行与调试。
 */
void init_display(void)
{
    REG_DISPCNT = MODE_3 | BG2_ON;
}

/**
 * 绘制 8x8 像素的瓦片到屏幕（演示用）
 * @param px 顶点 X（像素）
 * @param py 顶点 Y（像素）
 * @param color 颜色（RGB15 0-31）
 */
void draw_tile8(int px, int py, u16 color)
{
    u16 *fb = (u16*)0x06000000;
    for (int y = 0; y < 8; y++)
    {
        int vy = py + y;
        for (int x = 0; x < 8; x++)
        {
            int vx = px + x;
            fb[vy * 240 + vx] = color;
        }
    }
}

/**
 * 从 tileset 绘制 8x8 瓦片（支持调色板索引）
 * @param px 顶点 X（像素）
 * @param py 顶点 Y（像素）
 * @param tileIndex 瓦片索引（0 开始）
 */
void draw_tile8_from_tileset(int px, int py, int tileIndex)
{
    u16 *fb = (u16*)0x06000000;
    const unsigned char *tile = &tiles_data[tileIndex * 64];
    for (int y = 0; y < 8; y++)
    {
        int vy = py + y;
        for (int x = 0; x < 8; x++)
        {
            int vx = px + x;
            unsigned char idx = tile[y * 8 + x];
            u16 color = tiles_palette[idx];
            fb[vy * 240 + vx] = color;
        }
    }
}

/**
 * 程序入口：使用 tileset 与 map 渲染 30x20 的瓦片地图
 */
int main(void)
{
    init_display();

    for (int ty = 0; ty < MAP_HEIGHT; ty++)
    {
        for (int tx = 0; tx < MAP_WIDTH; tx++)
        {
            int t = map_data[ty * MAP_WIDTH + tx];
            draw_tile8_from_tileset(tx * 8, ty * 8, t);
        }
    }

    while (1)
    {
        VBlankIntrWait();
    }

    return 0;
}
