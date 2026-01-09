#include <gba.h>
#include "ZhFont.h"
#include <string.h>

int main()
{
    irqInit();
    irqEnable(IRQ_VBLANK);

    SetMode(MODE_3 | BG2_ON);

    // 清屏（画背景色）
    for(int y = 0; y < 160; ++y) {
        for(int x = 0; x < 240; ++x) {
            ((volatile u16*)0x06000000)[y * 240 + x] = RGB5(10, 10, 10);
        }
    }

    // 使用打字机效果显示文本
    const char* text = "你好,GBA哈哈哈哈哈哈";
    ZhFont_DrawUtf8Text_Typing(text, 0, 0, RGB5(31, 31, 31), 6);
    
    while(1) {
        VBlankIntrWait();
    }
}