/*------------------------------------------------------------------------
名称：ZhFont 12x12 字库绘制
说明：基于 HZK12 + ASC12（GB2312/ASCII），在 GBA Mode3 上绘制文本
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-08
备注：HZK12 每字 24 字节（12x12），ASC12 每字 12 字节（6x12）
------------------------------------------------------------------------*/

#pragma once

#include <gba.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 计算 GB2312 文本在 12x12 字库下的像素宽度
 * @param gb2312 GB2312 编码字符串（ASCII 单字节，中文双字节）
 * @return 像素宽度（ASCII 半角 6 像素，其它字符 12 像素）
 */
int ZhFont_GetGb2312TextWidth12(const char* gb2312);

/**
 * @brief 在 Mode 3 帧缓冲绘制 GB2312 文本（12x12）
 * @param gb2312 GB2312 编码字符串（ASCII 单字节，中文双字节）
 * @param x 起始 X（像素）
 * @param y 起始 Y（像素）
 * @param color 文本颜色（BGR555）
 */
void ZhFont_DrawGb2312TextMode3(const char* gb2312, int x, int y, u16 color);

/**
 * @brief 计算 UTF-8 文本在 12x12 字库下的像素宽度
 * @param utf8 UTF-8 编码字符串
 * @return 像素宽度（ASCII 半角 6 像素，其它字符 12 像素）
 */
int ZhFont_GetUtf8TextWidth12(const char* utf8);

/**
 * @brief 在 Mode 3 帧缓冲绘制 UTF-8 文本（12x12）
 * @param utf8 UTF-8 编码字符串
 * @param x 起始 X（像素）
 * @param y 起始 Y（像素）
 * @param color 文本颜色（BGR555）
 */
void ZhFont_DrawUtf8TextMode3(const char* utf8, int x, int y, u16 color);

#ifdef __cplusplus
}
#endif
