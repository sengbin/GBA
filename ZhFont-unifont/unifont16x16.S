/*------------------------------------------------------------------------
名称：Unifont 16x16 字库数据链接
说明：将 unifont16x16.bin 以 incbin 方式链接进 ROM，提供起止符号给 C/C++ 使用
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-07
备注：符号 g_unifont16x16_bin_start / g_unifont16x16_bin_end
------------------------------------------------------------------------*/

    .section .rodata
    .align 2

    .global g_unifont16x16_bin_start
    .global g_unifont16x16_bin_end

g_unifont16x16_bin_start:
    .incbin "unifont16x16.bin"

g_unifont16x16_bin_end:
