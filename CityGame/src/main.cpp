/*------------------------------------------------------------------------
名称：CityGame 主程序
说明：显示 TMX 地图（4 个图层独立渲染），并在地图上控制角色移动与走路帧切换
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-10
备注：使用 MODE_4 双缓冲避免闪烁
------------------------------------------------------------------------*/

#include <gba.h>

extern const int g_MapWidth;
extern const int g_MapHeight;
extern const int g_TileWidth;
extern const int g_TileHeight;
extern const unsigned int g_UsedTileCount;

extern const unsigned short g_Palette[256];
extern const unsigned short g_Layer0[];
extern const unsigned short g_Layer1[];
extern const unsigned short g_Layer2[];
extern const unsigned short g_Layer3[];
extern const unsigned short g_GidToTileIndex[];
extern const unsigned char g_TilePixels[];

extern const int g_PlayerWidth;
extern const int g_PlayerHeight;
extern const unsigned char g_PlayerFrame0[];
extern const unsigned char g_PlayerFrame1[];

static volatile u16* GetPageBuffer(bool pageBit)
{
    return (volatile u16*)(0x06000000 + (pageBit ? 0x0000A000 : 0x00000000));
}

static volatile u16* GetDrawBuffer()
{
    const bool pageBit = (REG_DISPCNT & BACKBUFFER) != 0;
    return GetPageBuffer(!pageBit);
}

static void FlipPage()
{
    REG_DISPCNT ^= BACKBUFFER;
}

static inline u16 Pack2(u8 a, u8 b)
{
    return (u16)(a | ((u16)b << 8));
}

static inline void Blend2(volatile u16* dst, u8 a, u8 b)
{
    const u16 cur = *dst;
    u8 lo = (u8)(cur & 0xFF);
    u8 hi = (u8)(cur >> 8);

    if(a != 0) lo = a;
    if(b != 0) hi = b;

    *dst = Pack2(lo, hi);
}

static void ClearScreen(u8 color)
{
    volatile u16* dst = GetDrawBuffer();
    const u16 packed = Pack2(color, color);

    for(int i = 0; i < (240 * 160) / 2; i++) {
        dst[i] = packed;
    }
}

/// <summary>
/// 将调色板写入 BG 调色板内存。
/// </summary>
static void LoadPalette()
{
    for(int i = 0; i < 256; i++) {
        BG_PALETTE[i] = g_Palette[i];
    }
}

static const unsigned short* GetLayerData(int layerIndex)
{
    switch(layerIndex) {
        case 0: return g_Layer0;
        case 1: return g_Layer1;
        case 2: return g_Layer2;
        case 3: return g_Layer3;
        default: return g_Layer0;
    }
}

static void DrawTile16(const unsigned char* tilePixels, int dstX, int dstY)
{
    if(dstX >= 240 || dstY >= 160 || dstX <= -16 || dstY <= -16) {
        return;
    }

    int srcX0 = 0;
    int srcY0 = 0;
    int w = 16;
    int h = 16;

    if(dstX < 0) {
        srcX0 = -dstX;
        w -= srcX0;
        dstX = 0;
    }
    if(dstY < 0) {
        srcY0 = -dstY;
        h -= srcY0;
        dstY = 0;
    }
    if(dstX + w > 240) {
        w = 240 - dstX;
    }
    if(dstY + h > 160) {
        h = 160 - dstY;
    }

    if(w <= 0 || h <= 0) {
        return;
    }

    // 为避免竖条纹与对齐问题，要求 dstX 必须是偶数
    if((dstX & 1) != 0) {
        return;
    }

    volatile u16* vram = GetDrawBuffer();

    for(int y = 0; y < h; y++) {
        const int srcY = srcY0 + y;
        const unsigned char* srcRow = tilePixels + srcY * 16;

        const int screenY = dstY + y;
        volatile u16* dstRow = vram + (screenY * 240 + dstX) / 2;

        for(int x = 0; x < w; x += 2) {
            const int srcX = srcX0 + x;
            const u8 a = srcRow[srcX + 0];
            const u8 b = (srcX + 1 < 16) ? srcRow[srcX + 1] : 0;
            Blend2(dstRow, a, b);
            dstRow++;
        }
    }
}

static void DrawSprite(const unsigned char* spritePixels, int spriteW, int spriteH, int dstX, int dstY)
{
    if(dstX >= 240 || dstY >= 160 || dstX <= -spriteW || dstY <= -spriteH) {
        return;
    }

    int srcX0 = 0;
    int srcY0 = 0;
    int w = spriteW;
    int h = spriteH;

    if(dstX < 0) {
        srcX0 = -dstX;
        w -= srcX0;
        dstX = 0;
    }
    if(dstY < 0) {
        srcY0 = -dstY;
        h -= srcY0;
        dstY = 0;
    }
    if(dstX + w > 240) {
        w = 240 - dstX;
    }
    if(dstY + h > 160) {
        h = 160 - dstY;
    }

    if(w <= 0 || h <= 0) {
        return;
    }

    if((dstX & 1) != 0) {
        return;
    }

    volatile u16* vram = GetDrawBuffer();

    for(int y = 0; y < h; y++) {
        const int srcY = srcY0 + y;
        const unsigned char* srcRow = spritePixels + srcY * spriteW;

        const int screenY = dstY + y;
        volatile u16* dstRow = vram + (screenY * 240 + dstX) / 2;

        for(int x = 0; x < w; x += 2) {
            const int srcX = srcX0 + x;
            const u8 a = srcRow[srcX + 0];
            const u8 b = (srcX + 1 < spriteW) ? srcRow[srcX + 1] : 0;
            Blend2(dstRow, a, b);
            dstRow++;
        }
    }
}

static void DrawMapLayer(int layerIndex, int camX, int camY)
{
    const unsigned short* layer = GetLayerData(layerIndex);

    const int tileW = g_TileWidth;
    const int tileH = g_TileHeight;

    const int mapPixelW = g_MapWidth * tileW;
    const int mapPixelH = g_MapHeight * tileH;

    if(camX < 0) camX = 0;
    if(camY < 0) camY = 0;
    if(camX > mapPixelW - 240) camX = mapPixelW - 240;
    if(camY > mapPixelH - 160) camY = mapPixelH - 160;

    camX &= ~1;

    const int startTileX = camX / tileW;
    const int startTileY = camY / tileH;
    const int offsetX = camX % tileW;
    const int offsetY = camY % tileH;

    const int tilesX = (240 + tileW - 1) / tileW + 1;
    const int tilesY = (160 + tileH - 1) / tileH + 1;

    for(int ty = 0; ty < tilesY; ty++) {
        const int mapTy = startTileY + ty;
        if(mapTy < 0 || mapTy >= g_MapHeight) {
            continue;
        }

        const int dstY = ty * tileH - offsetY;

        for(int tx = 0; tx < tilesX; tx++) {
            const int mapTx = startTileX + tx;
            if(mapTx < 0 || mapTx >= g_MapWidth) {
                continue;
            }

            const int dstX = tx * tileW - offsetX;
            const unsigned short gid = layer[mapTy * g_MapWidth + mapTx];
            if(gid == 0) {
                continue;
            }

            const unsigned short tileIndex = g_GidToTileIndex[gid];
            if(tileIndex == 0xFFFF || tileIndex >= g_UsedTileCount) {
                continue;
            }

            const unsigned char* tilePixels = g_TilePixels + (tileIndex * 16 * 16);
            DrawTile16(tilePixels, dstX, dstY);
        }
    }
}

int main()
{
    irqInit();
    irqEnable(IRQ_VBLANK);

    SetMode(MODE_4 | BG2_ON);

    LoadPalette();

    const int mapPixelW = g_MapWidth * g_TileWidth;
    const int mapPixelH = g_MapHeight * g_TileHeight;

    int playerX = mapPixelW / 2;
    int playerY = mapPixelH / 2;

    int animTick = 0;

    while(1) {
        VBlankIntrWait();

        scanKeys();
        const u16 keys = keysHeld();

        int dx = 0;
        int dy = 0;

        if(keys & KEY_LEFT) dx -= 4;
        if(keys & KEY_RIGHT) dx += 4;
        if(keys & KEY_UP) dy -= 4;
        if(keys & KEY_DOWN) dy += 4;

        if(dx != 0 || dy != 0) {
            playerX += dx;
            playerY += dy;
            animTick++;
        }

        if(playerX < 0) playerX = 0;
        if(playerY < 0) playerY = 0;
        if(playerX > mapPixelW) playerX = mapPixelW;
        if(playerY > mapPixelH) playerY = mapPixelH;

        playerX &= ~1;

        int camX = playerX - 120;
        int camY = playerY - 80;

        if(camX < 0) camX = 0;
        if(camY < 0) camY = 0;
        if(camX > mapPixelW - 240) camX = mapPixelW - 240;
        if(camY > mapPixelH - 160) camY = mapPixelH - 160;

        camX &= ~1;

        ClearScreen(0);

        DrawMapLayer(0, camX, camY);
        DrawMapLayer(1, camX, camY);
        DrawMapLayer(2, camX, camY);
        DrawMapLayer(3, camX, camY);

        const bool walking = (dx != 0 || dy != 0);
        const unsigned char* frame = g_PlayerFrame0;
        if(walking) {
            frame = ((animTick / 8) & 1) ? g_PlayerFrame1 : g_PlayerFrame0;
        }

        const int spriteX = 120 - (g_PlayerWidth / 2);
        const int spriteY = 80 - (g_PlayerHeight / 2);

        DrawSprite(frame, g_PlayerWidth, g_PlayerHeight, spriteX & ~1, spriteY);

        FlipPage();
    }
}
