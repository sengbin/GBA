/*------------------------------------------------------------------------
名称：Unifont 16x16 字库绘制
说明：提供基于 GNU Unifont 16x16 点阵的 UTF-8 文本绘制接口（Mode3）
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-07
备注：字库数据通过 unifont16x16.bin 以 incbin 方式链接到 ROM
------------------------------------------------------------------------*/

#pragma once

#include <gba.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 计算 UTF-8 文本在 Unifont 16x16 下的像素宽度
 * @param utf8 UTF-8 编码字符串
 * @return 像素宽度（ASCII 半角 8 像素，其它字符 16 像素）
 */
int Unifont_GetUtf8TextWidth16(const char* utf8);

/**
 * @brief 在 Mode 3 帧缓冲绘制 UTF-8 文本（Unifont 16x16）
 * @param utf8 UTF-8 编码字符串
 * @param x 起始 X（像素）
 * @param y 起始 Y（像素）
 * @param color 文本颜色（BGR555）
 */
void Unifont_DrawUtf8TextMode3(const char* utf8, int x, int y, u16 color);

#ifdef __cplusplus
}
#endif
