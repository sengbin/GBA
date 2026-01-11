# ------------------------------------------------------------------------
# Makefile：GBA Demo 项目构建脚本
# 说明：使用 devkitPro 工具链与 libgba 库，编译并链接生成 GBA 可运行文件
# 作者：Lion
# 日期：2026-01-11
# 备注：将二进制输出到 bin，目标中间文件输出到 obj
# ------------------------------------------------------------------------

# 工具链命令（根据 devkitPro 安装路径配置）
CC := /c/devkitPro/devkitARM/bin/arm-none-eabi-gcc
CXX := /c/devkitPro/devkitARM/bin/arm-none-eabi-g++
OBJCOPY := /c/devkitPro/devkitARM/bin/arm-none-eabi-objcopy

# 基本编译选项：生成 Thumb 指令、优化等级、消除未使用节以便链接时剔除
CFLAGS := -mthumb -mthumb-interwork -O2 -ffunction-sections -fdata-sections -Wall
# 供头文件搜索的目录（包含 libgba 及中文字体库头文件）
INCLUDES := -I/c/devkitPro/libgba/include -I/d/GitCode/GBA-Github/ZhFont
# C++ 编译选项：继承 CFLAGS，设置 C++ 标准并禁用异常/RTTI（适合嵌入式）
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-exceptions -fno-rtti $(INCLUDES)
# 链接时需要的库路径与库名（使用 libgba）
LIBS := -L/c/devkitPro/libgba/lib -lgba
# 链接器标志：使用 gba.specs 以生成符合 GBA 的可执行格式
LDFLAGS := -specs=gba.specs

# 输出目录与目标设置
OUTDIR := bin
OBJDIR := obj
TARGET := gba-demo
# 要编译的 C++ 源文件列表
SOURCES_CPP := main.cpp
# 根据源文件生成对应的目标文件路径（放在 obj 目录）
OBJECTS := $(addprefix $(OBJDIR)/,$(SOURCES_CPP:.cpp=.o))

# 中文字体静态库（本工程使用的预编译字体库）
FONTLIB := /d/GitCode/GBA-Github/ZhFont/zhfont.a

# 中间 ELF 文件与最终 GBA 二进制文件路径
ELF := $(OUTDIR)/$(TARGET).elf
GBA := $(OUTDIR)/$(TARGET).gba

# 默认目标：生成最终 GBA 可执行文件
all: $(GBA)

# 规则：编译 C++ 源文件到 obj 目录下对应的 .o 文件
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 规则：编译汇编源文件 (.S) 到目标文件
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

# 如果字体静态库不存在，则切换到 ZhFont 目录构建
$(FONTLIB):
	$(MAKE) -C ../ZhFont

# 确保输出目录存在
$(OUTDIR):
	mkdir -p $(OUTDIR)

# 确保目标文件目录存在
$(OBJDIR):
	mkdir -p $(OBJDIR)

# 链接阶段：将所有目标文件与字体库链接生成 ELF 可执行文件
$(ELF): $(OBJECTS) $(FONTLIB) | $(OUTDIR)
	$(CXX) $(OBJECTS) $(FONTLIB) $(LDFLAGS) $(LIBS) -o $(ELF)

# 把 ELF 转换成 GBA 可装载的二进制格式
$(GBA): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(GBA)

# 清理中间文件与生成物
clean:
	rm -f $(OBJECTS) $(ELF) $(GBA)
