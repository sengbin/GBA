/*------------------------------------------------------------------------
名称：ZhFont 字库数据链接
说明：将 HZK12 与 ASC12 以 incbin 方式链接进 ROM，提供起止符号给 C/C++ 使用
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-08
备注：符号 g_zhfont_hzk12_start/end 与 g_zhfont_asc12_start/end
------------------------------------------------------------------------*/

    .section .rodata
    .align 2

    .global g_zhfont_hzk12_start
    .global g_zhfont_hzk12_end

    .global g_zhfont_asc12_start
    .global g_zhfont_asc12_end

g_zhfont_hzk12_start:
    .incbin "HZK12"

g_zhfont_hzk12_end:

g_zhfont_asc12_start:
    .incbin "ASC12"

g_zhfont_asc12_end:
