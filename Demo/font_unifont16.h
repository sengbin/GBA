/*------------------------------------------------------------------------
名称：Unifont16 字库接口
说明：提供 Unifont 16x16 点阵读取与 Mode 3 绘制函数
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-07
备注：通用复用版
------------------------------------------------------------------------*/
#pragma once

#include <gba.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取指定 Unicode 码点的 16x16 点阵（每行两字节，高位在前）。
 * @param codepoint Unicode 码点，超出 0xFFFF 时返回空指针。
 * @return 指向 32 字节点阵的指针。
 */
const uint8_t* Unifont16_GetGlyph(u32 codepoint);

/**
 * @brief 计算 UTF-8 字符串在 16x16 点阵下的总宽度（包含 1 像素字间距）。
 * @param utf8 UTF-8 编码字符串。
 * @return 像素宽度。
 */
int Unifont16_MeasureWidth(const char* utf8);

/**
 * @brief 在 Mode 3 帧缓冲绘制 UTF-8 字符串（16x16 点阵）。
 * @param utf8 UTF-8 编码字符串。
 * @param x 起始 X 坐标。
 * @param y 起始 Y 坐标。
 * @param color 前景色（BGR555）。
 * @param bg 背景色（当 transparentBg 为 0 时生效）。
 * @param transparentBg 为 1 时跳过背景像素，为 0 时填充背景色。
 */
void Unifont16_DrawString(const char* utf8, int x, int y, u16 color, u16 bg, int transparentBg);

/* 全量 16x16 点阵表，索引为 Unicode 码点（0x0000~0xFFFF），每项 32 字节。 */
extern const uint8_t g_Unifont16[65536][32];

#ifdef __cplusplus
}
#endif
