# ZhFontLib

GBA 中文字符库（静态库），基于 GNU Unifont 16x16 点阵字库，提供 UTF-8 文本在 Mode3（位图）上的绘制接口。

## 功能

- 支持 UTF-8 解码（含中文）
- 采用 Unifont 16x16 点阵渲染
- 支持 ASCII 半角显示：英文/数字/常用符号按 8 像素宽绘制与步进
- 其它字符按 16 像素宽绘制与步进

## 目录结构

- 头文件：UnifontFont.h
- 实现：UnifontFont.cpp
- 字库数据：unifont16x16.bin
- 字库数据链接：unifont16x16.S（通过 `.incbin` 把 bin 链接进 ROM）
- 构建脚本：Makefile

## 构建输出

- 静态库：bin/ZhFontLib.a
- 中间文件：obj/*.o

## 构建方法

在本目录执行：

```bash
make
```

清理：

```bash
make clean
```

## 在项目中使用

### 1）包含头文件

在你的工程编译参数中加入头文件搜索路径：

- 例如：`-I../ZhFontLib`

代码中包含：

```cpp
#include "UnifontFont.h"
```

### 2）链接静态库

链接时把库文件作为输入加入：

- `../ZhFontLib/bin/ZhFontLib.a`

Makefile 示例（仅示例核心部分）：

```makefile
INCLUDES += -I../ZhFontLib
FONTLIB := ../ZhFontLib/bin/ZhFontLib.a

$(TARGET).elf: $(OBJECTS) $(FONTLIB)
	$(CXX) $(OBJECTS) $(FONTLIB) $(LDFLAGS) $(LIBS) -o $(TARGET).elf
```

如果希望在构建 Demo 时自动先构建库，可以添加：

```makefile
$(FONTLIB):
	$(MAKE) -C ../ZhFontLib
```

## API

### Unifont_GetUtf8TextWidth16

```c
int Unifont_GetUtf8TextWidth16(const char* utf8);
```

- 作用：计算一行 UTF-8 文本的像素宽度
- 规则：
  - ASCII 可见字符（U+0020 ~ U+007E）按 8 像素累计
  - 其它字符按 16 像素累计
  - 遇到 `\n` 停止

### Unifont_DrawUtf8TextMode3

```c
void Unifont_DrawUtf8TextMode3(const char* utf8, int x, int y, u16 color);
```

- 作用：在 GBA Mode3 帧缓冲直接绘制 UTF-8 文本（Unifont 点阵）
- 参数：
  - utf8：UTF-8 字符串
  - x/y：起始像素坐标
  - color：BGR555 颜色

## 备注

- 本库使用静态链接，不依赖文件系统；字库数据随 ROM 一并打包。
- 当前字形索引范围为 U+0000 ~ U+FFFF（BMP）。
