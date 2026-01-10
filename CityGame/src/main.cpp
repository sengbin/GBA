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
extern const unsigned short g_GidToBaseTile8[];
extern const unsigned int g_BgTileCount;
extern const unsigned char g_BgTiles[];

extern const int g_PlayerWidth;
extern const int g_PlayerHeight;
extern const unsigned short g_PlayerObjFrame0TileId;
extern const unsigned short g_PlayerObjFrame1TileId;
extern const unsigned char g_PlayerObjTiles[];

#if 0

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

#endif

typedef struct
{
    u16 attr0;
    u16 attr1;
    u16 attr2;
    u16 pad;
} ObjAttr;

static volatile ObjAttr* const g_Oam = (volatile ObjAttr*)0x07000000;
static volatile u16* const g_ObjPal = (volatile u16*)0x05000200;
static volatile u16* const g_ObjVram16 = (volatile u16*)0x06010000;
static volatile u16* const g_BgVram16 = (volatile u16*)0x06000000;

static const u16 g_DispcntObj1DMap = 0x0040;

static inline u16 BgCnt(u16 prio, u16 charBase, u16 screenBase, bool is8bpp, u16 sizeCode)
{
    return (u16)((prio & 3) | ((charBase & 3) << 2) | (is8bpp ? (1 << 7) : 0) | ((screenBase & 31) << 8) | ((sizeCode & 3) << 14));
}

static inline void WriteBgMapEntry(volatile u16* base, int vramX, int vramY, u16 tileId)
{
    // 64x32：由两个 32x32 screenblock 组成（左右各一个）
    const int block = (vramX >= 32) ? 1 : 0;
    const int x = vramX & 31;
    base[block * 1024 + vramY * 32 + x] = tileId;
}

static const unsigned short* GetLayerData2(int layerIndex)
{
    switch(layerIndex) {
        case 0: return g_Layer0;
        case 1: return g_Layer1;
        case 2: return g_Layer2;
        case 3: return g_Layer3;
        default: return g_Layer0;
    }
}

static inline u16 GetBgTileIdForWorld(int layerIndex, int worldTileX8, int worldTileY8)
{
    const int worldTilesW = g_MapWidth * 2;
    const int worldTilesH = g_MapHeight * 2;

    if(worldTileX8 < 0 || worldTileY8 < 0 || worldTileX8 >= worldTilesW || worldTileY8 >= worldTilesH) {
        return 0;
    }

    const int cellX16 = worldTileX8 >> 1;
    const int cellY16 = worldTileY8 >> 1;
    const unsigned short* layer = GetLayerData2(layerIndex);
    const unsigned short gid = layer[cellY16 * g_MapWidth + cellX16];
    if(gid == 0) {
        return 0;
    }

    const unsigned short base = g_GidToBaseTile8[gid];
    if(base == 0) {
        return 0;
    }

    const int q = (worldTileX8 & 1) | ((worldTileY8 & 1) << 1);
    return (u16)(base + q);
}

static inline int Wrap64(int v)
{
    return v & 63;
}

static inline int Wrap32(int v)
{
    return v & 31;
}

static inline int WorldXFromVramX(int bufX, int vramX)
{
    return bufX + Wrap64(vramX - Wrap64(bufX));
}

static inline int WorldYFromVramY(int bufY, int vramY)
{
    return bufY + Wrap32(vramY - Wrap32(bufY));
}

static void FillAllLayerMaps(volatile u16* bgMap, int layerIndex, int bufX, int bufY)
{
    for(int vy = 0; vy < 32; vy++) {
        const int worldY = WorldYFromVramY(bufY, vy);
        for(int vx = 0; vx < 64; vx++) {
            const int worldX = WorldXFromVramX(bufX, vx);
            const u16 tileId = GetBgTileIdForWorld(layerIndex, worldX, worldY);
            WriteBgMapEntry(bgMap, vx, vy, tileId);
        }
    }
}

static void UpdateLayerColumn(volatile u16* bgMap, int layerIndex, int bufX, int bufY, int vramX, int worldX)
{
    for(int vy = 0; vy < 32; vy++) {
        const int worldY = WorldYFromVramY(bufY, vy);
        const u16 tileId = GetBgTileIdForWorld(layerIndex, worldX, worldY);
        WriteBgMapEntry(bgMap, vramX, vy, tileId);
    }
}

static void UpdateLayerRow(volatile u16* bgMap, int layerIndex, int bufX, int bufY, int vramY, int worldY)
{
    for(int vx = 0; vx < 64; vx++) {
        const int worldX = WorldXFromVramX(bufX, vx);
        const u16 tileId = GetBgTileIdForWorld(layerIndex, worldX, worldY);
        WriteBgMapEntry(bgMap, vx, vramY, tileId);
    }
}

static inline void LoadPalette2()
{
    for(int i = 0; i < 256; i++) {
        BG_PALETTE[i] = g_Palette[i];
        g_ObjPal[i] = g_Palette[i];
    }
}

static void LoadBgTiles()
{
    const int halfwords = (int)(g_BgTileCount * 64) / 2;
    const u16* src = (const u16*)g_BgTiles;
    for(int i = 0; i < halfwords; i++) {
        g_BgVram16[i] = src[i];
    }
}

static void LoadPlayerObjTiles()
{
    const int halfwords = (32 * 32 * 2) / 2;
    const u16* src = (const u16*)g_PlayerObjTiles;
    for(int i = 0; i < halfwords; i++) {
        g_ObjVram16[i] = src[i];
    }
}

static void HideAllObjects()
{
    for(int i = 0; i < 128; i++) {
        g_Oam[i].attr0 = 160;
        g_Oam[i].attr1 = 0;
        g_Oam[i].attr2 = 0;
        g_Oam[i].pad = 0;
    }
}

static void InitPlayerObj(int screenX, int screenY, u16 tileId)
{
    // attr0: Y(0-255) + 256色 + 正方形
    // attr1: X(0-511) + 32x32
    // attr2: tileId(0-1023) + priority
    g_Oam[0].attr0 = (u16)((screenY & 0xFF) | 0x2000 | 0x0000);
    g_Oam[0].attr1 = (u16)((screenX & 0x1FF) | 0x8000);
    g_Oam[0].attr2 = (u16)((tileId & 0x03FF) | 0x0000);
    g_Oam[0].pad = 0;
}

static void SetPlayerObjTile(u16 tileId)
{
    g_Oam[0].attr2 = (u16)((g_Oam[0].attr2 & 0xFC00) | (tileId & 0x03FF));
}

int main()
{
    irqInit();
    irqEnable(IRQ_VBLANK);

    SetMode(MODE_0 | BG0_ON | BG1_ON | BG2_ON | BG3_ON | OBJ_ON);
    REG_DISPCNT |= g_DispcntObj1DMap;

    // 4 个 BG 图层，不合并
    // BG 大小：64x32 tiles（512x256），方便横向滚动并减少 map 占用
    // screenbase 采用 24..31，避免覆盖 BG tiles
    REG_BG0CNT = BgCnt(3, 0, 24, true, 1);
    REG_BG1CNT = BgCnt(2, 0, 26, true, 1);
    REG_BG2CNT = BgCnt(1, 0, 28, true, 1);
    REG_BG3CNT = BgCnt(0, 0, 30, true, 1);

    volatile u16* bg0Map = (volatile u16*)(0x06000000 + 24 * 0x800);
    volatile u16* bg1Map = (volatile u16*)(0x06000000 + 26 * 0x800);
    volatile u16* bg2Map = (volatile u16*)(0x06000000 + 28 * 0x800);
    volatile u16* bg3Map = (volatile u16*)(0x06000000 + 30 * 0x800);

    LoadPalette2();
    LoadBgTiles();
    LoadPlayerObjTiles();

    HideAllObjects();

    const int mapPixelW = g_MapWidth * g_TileWidth;
    const int mapPixelH = g_MapHeight * g_TileHeight;

    int playerX = mapPixelW / 2;
    int playerY = mapPixelH / 2;

    int camX = playerX - 120;
    int camY = playerY - 80;

    if(camX < 0) camX = 0;
    if(camY < 0) camY = 0;
    if(camX > mapPixelW - 240) camX = mapPixelW - 240;
    if(camY > mapPixelH - 160) camY = mapPixelH - 160;

    // world 8x8 tile 维度
    const int worldTilesW = g_MapWidth * 2;
    const int worldTilesH = g_MapHeight * 2;

    int bufX = 0;
    int bufY = 0;

    // 初次填充地图
    FillAllLayerMaps(bg0Map, 0, bufX, bufY);
    FillAllLayerMaps(bg1Map, 1, bufX, bufY);
    FillAllLayerMaps(bg2Map, 2, bufX, bufY);
    FillAllLayerMaps(bg3Map, 3, bufX, bufY);

    InitPlayerObj(120 - 16, 80 - 16, g_PlayerObjFrame0TileId);

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

        camX = playerX - 120;
        camY = playerY - 80;

        if(camX < 0) camX = 0;
        if(camY < 0) camY = 0;
        if(camX > mapPixelW - 240) camX = mapPixelW - 240;
        if(camY > mapPixelH - 160) camY = mapPixelH - 160;

        // 计算需要的 buffer world tile 起点（64x32）
        const int camTileX8 = camX >> 3;
        const int camTileY8 = camY >> 3;

        int wantBufX = bufX;
        int wantBufY = bufY;

        // 保证可视范围落在 [buf, buf+size) 内
        if(camTileX8 < bufX) {
            wantBufX = camTileX8;
        } else if(camTileX8 + 29 > bufX + 63) {
            wantBufX = camTileX8 - 34;
        }

        if(camTileY8 < bufY) {
            wantBufY = camTileY8;
        } else if(camTileY8 + 19 > bufY + 31) {
            wantBufY = camTileY8 - 12;
        }

        if(wantBufX < 0) wantBufX = 0;
        if(wantBufY < 0) wantBufY = 0;
        if(wantBufX > worldTilesW - 64) wantBufX = worldTilesW - 64;
        if(wantBufY > worldTilesH - 32) wantBufY = worldTilesH - 32;

        // X 方向增量更新
        while(bufX < wantBufX) {
            bufX++;
            const int vramX = Wrap64(bufX + 63);
            const int worldX = bufX + 63;
            UpdateLayerColumn(bg0Map, 0, bufX, bufY, vramX, worldX);
            UpdateLayerColumn(bg1Map, 1, bufX, bufY, vramX, worldX);
            UpdateLayerColumn(bg2Map, 2, bufX, bufY, vramX, worldX);
            UpdateLayerColumn(bg3Map, 3, bufX, bufY, vramX, worldX);
        }
        while(bufX > wantBufX) {
            bufX--;
            const int vramX = Wrap64(bufX);
            const int worldX = bufX;
            UpdateLayerColumn(bg0Map, 0, bufX, bufY, vramX, worldX);
            UpdateLayerColumn(bg1Map, 1, bufX, bufY, vramX, worldX);
            UpdateLayerColumn(bg2Map, 2, bufX, bufY, vramX, worldX);
            UpdateLayerColumn(bg3Map, 3, bufX, bufY, vramX, worldX);
        }

        // Y 方向增量更新
        while(bufY < wantBufY) {
            bufY++;
            const int vramY = Wrap32(bufY + 31);
            const int worldY = bufY + 31;
            UpdateLayerRow(bg0Map, 0, bufX, bufY, vramY, worldY);
            UpdateLayerRow(bg1Map, 1, bufX, bufY, vramY, worldY);
            UpdateLayerRow(bg2Map, 2, bufX, bufY, vramY, worldY);
            UpdateLayerRow(bg3Map, 3, bufX, bufY, vramY, worldY);
        }
        while(bufY > wantBufY) {
            bufY--;
            const int vramY = Wrap32(bufY);
            const int worldY = bufY;
            UpdateLayerRow(bg0Map, 0, bufX, bufY, vramY, worldY);
            UpdateLayerRow(bg1Map, 1, bufX, bufY, vramY, worldY);
            UpdateLayerRow(bg2Map, 2, bufX, bufY, vramY, worldY);
            UpdateLayerRow(bg3Map, 3, bufX, bufY, vramY, worldY);
        }

        // 设置 BG 滚动（不闪烁）
        const int hofs = camX - bufX * 8 + (Wrap64(bufX) * 8);
        const int vofs = camY - bufY * 8 + (Wrap32(bufY) * 8);

        REG_BG0HOFS = (u16)(hofs & 511);
        REG_BG1HOFS = (u16)(hofs & 511);
        REG_BG2HOFS = (u16)(hofs & 511);
        REG_BG3HOFS = (u16)(hofs & 511);

        REG_BG0VOFS = (u16)(vofs & 255);
        REG_BG1VOFS = (u16)(vofs & 255);
        REG_BG2VOFS = (u16)(vofs & 255);
        REG_BG3VOFS = (u16)(vofs & 255);

        // 角色屏幕坐标（相机边缘时不强制居中）
        int sprX = playerX - camX - 16;
        int sprY = playerY - camY - 16;
        if(sprX < -32) sprX = -32;
        if(sprY < -32) sprY = -32;
        if(sprX > 240) sprX = 240;
        if(sprY > 160) sprY = 160;
        g_Oam[0].attr0 = (u16)((g_Oam[0].attr0 & 0xFF00) | (sprY & 0x00FF));
        g_Oam[0].attr1 = (u16)((g_Oam[0].attr1 & 0xFE00) | (sprX & 0x01FF));

        // 走路帧切换
        const bool walking = (dx != 0 || dy != 0);
        if(walking) {
            const u16 tileId = (((animTick / 6) & 1) != 0) ? g_PlayerObjFrame1TileId : g_PlayerObjFrame0TileId;
            SetPlayerObjTile(tileId);
        } else {
            SetPlayerObjTile(g_PlayerObjFrame0TileId);
        }
    }
}
