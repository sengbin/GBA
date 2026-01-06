/*------------------------------------------------------------------------
名称：GBA Mode3 + 精灵光标示例
说明：Mode 3 背景绘制中文文字，光标使用 OBJ 精灵移动，可到达屏幕最顶行
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-07
备注：教程示例，使用 libgba
------------------------------------------------------------------------*/

#include <gba.h>

#include "UnifontFont.h"

/**
 * @brief OAM 精灵属性（本地定义，避免依赖特定宏）
 */
typedef struct ObjAttrLocal
{
    u16 attr0;
    u16 attr1;
    u16 attr2;
    u16 pad;
} ObjAttrLocal;

/**
 * @brief 获取 OAM 内存指针
 * @return OAM 首地址
 */
static volatile ObjAttrLocal* GetOam()
{
    return (volatile ObjAttrLocal*)0x07000000;
}

/**
 * @brief 获取 OBJ 调色板内存指针
 * @return OBJ 调色板首地址
 */
static volatile u16* GetObjPalette()
{
    return (volatile u16*)0x05000200;
}

/**
 * @brief 获取 OBJ 图块内存指针（4bpp）
 * @return OBJ 图块首地址
 */
static volatile u16* GetObjTiles4bpp()
{
    return (volatile u16*)0x06010000;
}

/**
 * @brief 在 4bpp 8x8 图块缓冲里设置一个像素
 * @param tile 图块缓冲（32 字节，对齐为 16 个 u16）
 * @param x 图块内 X（0-7）
 * @param y 图块内 Y（0-7）
 * @param palIndex 调色板索引（0-15）
 */
static void ObjTileSetPixel4bpp(u16* tile, int x, int y, u8 palIndex)
{
    u8* bytes = (u8*)tile;
    int byteIndex = y * 4 + (x >> 1);

    u8 b = bytes[byteIndex];
    if ((x & 1) == 0)
    {
        b = (u8)((b & 0xF0) | (palIndex & 0x0F));
    }
    else
    {
        b = (u8)((b & 0x0F) | ((palIndex & 0x0F) << 4));
    }
    bytes[byteIndex] = b;
}

/**
 * @brief 初始化一个 16x16 的箭头光标精灵（4bpp，使用调色板索引 1）
 */
static void InitCursorSpriteTiles()
{
    /*
     * Mode 3 位图占用 VRAM：0x06000000 ~ 0x06012BFF（240*160*2 字节）
     * OBJ 图块基址为 0x06010000，因此需要把 tileIndex 设到不与位图重叠的位置。
     * 为避免位图区域的边界/保留区差异，直接放到 0x06014000 之后更稳妥。
     * 计算：0x06010000 + tileIndex*32 >= 0x06014000 => tileIndex >= 512
     */
    const int baseTileIndex = 512;

    u16 tileBuf[4][16];

    for (int t = 0; t < 4; ++t)
    {
        for (int i = 0; i < 16; ++i)
        {
            tileBuf[t][i] = 0;
        }
    }

    /* 11x7 箭头图案，1 为绘制像素（左上角为箭头尖） */
    const u8 cursor[11] = {
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

    const int h = 11;
    const int w = 7;

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            if (cursor[y] & (1 << (w - 1 - x)))
            {
                int px = x;
                int py = y;

                if (px < 0 || px >= 16 || py < 0 || py >= 16)
                    continue;

                int tileX = (px >> 3);
                int tileY = (py >> 3);
                int tileIndex = tileY * 2 + tileX;

                int inTileX = px & 7;
                int inTileY = py & 7;

                ObjTileSetPixel4bpp(tileBuf[tileIndex], inTileX, inTileY, 1);
            }
        }
    }

    volatile u16* objTiles = GetObjTiles4bpp();
    for (int t = 0; t < 4; ++t)
    {
        for (int i = 0; i < 16; ++i)
        {
            objTiles[(baseTileIndex + t) * 16 + i] = tileBuf[t][i];
        }
    }
}

/**
 * @brief 初始化光标精灵调色板（索引 0 为透明，索引 1 为黄色）
 */
static void InitCursorSpritePalette()
{
    volatile u16* objPal = GetObjPalette();
    objPal[0] = RGB5(0, 0, 0);
    objPal[1] = RGB5(31, 31, 0);
}

/**
 * @brief 初始化 OAM（隐藏所有精灵）
 */
static void InitOam()
{
    volatile ObjAttrLocal* oam = GetOam();
    for (int i = 0; i < 128; ++i)
    {
        oam[i].attr0 = 160;
        oam[i].attr1 = 0;
        oam[i].attr2 = 0;
        oam[i].pad = 0;
    }
}

/**
 * @brief 设置 0 号精灵为 16x16 方形光标并更新坐标
 * @param x 屏幕 X（0-239）
 * @param y 屏幕 Y（0-159）
 */
static void SetCursorSpritePos(int x, int y)
{
    volatile ObjAttrLocal* oam = GetOam();

    const int baseTileIndex = 512;

    /* attr0: Y(0-7) + 规则模式 + 4bpp + 方形 */
    oam[0].attr0 = (u16)(y & 0x00FF);

    /* attr1: X(0-8) + 方形 16x16（size=1 => bit14=1） */
    oam[0].attr1 = (u16)((x & 0x01FF) | 0x4000);

    /* attr2: tileIndex=baseTileIndex + priority=0 + paletteBank=0 */
    oam[0].attr2 = (u16)baseTileIndex;
}

int main(void)
{
    irqInit();
    irqEnable(IRQ_VBLANK);

    /* Mode 3（位图） + BG2 + OBJ（精灵） + 1D 图块映射 */
    REG_DISPCNT = MODE_3 | BG2_ENABLE | OBJ_ENABLE | OBJ_1D_MAP;

    const u16 black = RGB5(0, 0, 0);
    const u16 white = RGB5(31, 31, 31);

    /* 清屏：只在初始化时清一次，背景文字绘制一次 */
    DMA3COPY(&black, (void*)0x06000000, DMA16 | (240 * 160) | DMA_SRC_FIXED);

    const char* text = u8"中文测试程序abcABCａｂｃ"; /* 顶部显示的一行文字（UTF-8） */
    int textWidth = Unifont_GetUtf8TextWidth16(text);
    int textX = (240 - textWidth) / 2;
    if (textX < 0) textX = 0;
    int textY = 0; /* 最顶行 */

    Unifont_DrawUtf8TextMode3(text, textX, textY, white);

    InitOam();
    InitCursorSpritePalette();
    InitCursorSpriteTiles();

    int x = 120;
    int y = 80;

    while (1)
    {
        scanKeys();
        u16 keys = keysHeld();

        if (keys & KEY_LEFT)  x--;
        if (keys & KEY_RIGHT) x++;
        if (keys & KEY_UP)    y--;
        if (keys & KEY_DOWN)  y++;

        /* 限制边界（光标精灵 16x16），允许到最顶行 */
        if (x < 0) x = 0;
        if (x > 240 - 16) x = 240 - 16;
        if (y < 0) y = 0;
        if (y > 160 - 16) y = 160 - 16;

        VBlankIntrWait();

        SetCursorSpritePos(x, y);
    }

    return 0;
}
