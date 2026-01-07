# GBA-ZhFont

GBA 中文点阵字库（12x12），提供在 **GBA Mode 3** 帧缓冲上直接绘制文本的接口。

该库包含两部分字模数据：

- **HZK12**：GB2312 双字节字符（12x12）
- **ASC12**：ASCII 半角字符（6x12）

输出效果：ASCII 以 6 像素宽前进，非 ASCII 以 12 像素宽前进。

## 功能介绍

- 在 GBA Mode 3（240x160，16bpp）上绘制文本
- 支持两种输入：
	- **GB2312 编码字符串**：ASCII 单字节，中文双字节
	- **UTF-8 编码字符串**：内部将 Unicode 码点映射为 GB2312 后绘制
- 提供文本宽度计算接口：用于布局/对齐
- 字库数据已打包进静态库 `bin/zhfont.a`

## 支持范围与说明

### GB2312 输入

- ASCII：`0x20` ~ `0x7E`（半角 6x12）
- 中文/符号：双字节 `high`、`low`（12x12）
	- 区码范围：`0xA1` ~ `0xF8`
	- 位码范围：`0xA1` ~ `0xFE`

### UTF-8 输入

- 支持解析 1~4 字节 UTF-8 序列
- 仅当 Unicode 码点能在库内的 **Unicode→GB2312 映射表**中找到时才会绘制
	- 若未找到映射：不绘制该字符，但仍按 12 像素宽前进（用于保持排版一致）
	- 映射表使用 16 位 Unicode（`uint16_t`），超出 `0xFFFF` 的码点无法映射

### 其他限制

- 仅提供 **Mode 3** 绘制接口（直接写 VRAM）
- 遇到 `\n` 会停止绘制/计算宽度（不做自动换行）
- 超出屏幕右侧时会停止继续绘制

## 构建

仓库根目录执行：

```bash
make
```

产物：

- `bin/zhfont.a`：可直接被你的 GBA 工程链接使用

说明：

- 构建库本身时需要仓库中的 `HZK12`、`ASC12` 文件参与汇编（`incbin`）
- 你的工程只需要链接 `zhfont.a`，不需要额外拷贝 `HZK12`/`ASC12`

## 集成到你的 GBA 工程

### 1) 头文件包含路径

确保编译参数中包含本仓库目录（能找到 `ZhFont.h`）：

- 例如：`-I/path/to/GBA-ZhFont-Github`

### 2) 链接静态库

把 `bin/zhfont.a` 加入链接输入（与目标文件一起传给链接器）。

如果你使用 Makefile，示例（仅示意关键项）：

```makefile
INCLUDES := -I... -I/path/to/GBA-ZhFont-Github
FONTLIB  := /path/to/GBA-ZhFont-Github/bin/zhfont.a

$(CXX) $(OBJECTS) $(FONTLIB) $(LDFLAGS) $(LIBS) -o $(ELF)
```

## API 调用方法

对外接口定义在 `ZhFont.h`：

### 1) 计算宽度

- `int ZhFont_GetGb2312TextWidth12(const char* gb2312);`
	- 返回 GB2312 字符串在 12x12 字库下的像素宽度
	- ASCII 按 6 像素宽，其它字符按 12 像素宽

- `int ZhFont_GetUtf8TextWidth12(const char* utf8);`
	- 返回 UTF-8 字符串在 12x12 字库下的像素宽度
	- ASCII 按 6 像素宽，其它字符按 12 像素宽

### 2) Mode 3 绘制

- `void ZhFont_DrawGb2312TextMode3(const char* gb2312, int x, int y, u16 color);`
	- 在 Mode 3 帧缓冲绘制 GB2312 字符串
	- `x/y` 为像素坐标；`color` 为 16 位颜色（可用 `RGB5(r,g,b)` 生成）

- `void ZhFont_DrawUtf8TextMode3(const char* utf8, int x, int y, u16 color);`
	- 在 Mode 3 帧缓冲绘制 UTF-8 字符串
	- UTF-8 会先转码/映射为 GB2312 再绘制

## 最小示例

下面示例演示：设置 Mode 3，清屏，绘制 UTF-8 文本，并用宽度接口做居中。

```cpp
#include <gba.h>
#include "ZhFont.h"

int main()
{
		irqInit();
		irqEnable(IRQ_VBLANK);

		SetMode(MODE_3 | BG2_ON);

		// 清屏（黑色）
		for (int y = 0; y < 160; ++y)
		{
				for (int x = 0; x < 240; ++x)
				{
						((volatile u16*)0x06000000)[y * 240 + x] = RGB5(0, 0, 0);
				}
		}

		const char* text = "你好，GBA"; // UTF-8
		const int w = ZhFont_GetUtf8TextWidth12(text);
		const int x = (240 - w) / 2;
		const int y = 70;

		ZhFont_DrawUtf8TextMode3(text, x, y, RGB5(31, 31, 31));

		while (1)
		{
				VBlankIntrWait();
		}
}
```

如果你已经有 GB2312 编码文本，也可以直接调用：

```cpp
ZhFont_DrawGb2312TextMode3(gb2312Text, 10, 10, RGB5(31, 31, 0));
```

## 工具脚本（可选）

- `tool/_gen_asc12.py`：生成 ASCII 6x12 字模数据
- `tool/_gen_gb2312_table.py`：生成 Unicode→GB2312 映射表头文件

