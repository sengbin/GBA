# ------------------------------------------------------------------------
# Makefile：CityGame 项目构建脚本
# 说明：使用 devkitPro 工具链与 libgba 库，编译 CityGame 项目并生成 GBA 可运行文件
# 作者：Lion
# 日期：2026-01-11
# 备注：二进制输出到 bin，目标中间文件输出到 obj
# ------------------------------------------------------------------------

# 工具链命令（根据 devkitPro 安装路径配置）
CC := /c/devkitPro/devkitARM/bin/arm-none-eabi-gcc
CXX := /c/devkitPro/devkitARM/bin/arm-none-eabi-g++
OBJCOPY := /c/devkitPro/devkitARM/bin/arm-none-eabi-objcopy

# 基本编译选项：生成 Thumb 指令、优化等级、并将未使用节放入单独节以便链接时剔除
CFLAGS := -mthumb -mthumb-interwork -O2 -ffunction-sections -fdata-sections -Wall
# 头文件搜索路径（包含 libgba）
INCLUDES := -I/c/devkitPro/libgba/include
# C++ 编译选项：继承 CFLAGS，设置 C++ 标准并禁用异常/RTTI（适合嵌入式）
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-exceptions -fno-rtti $(INCLUDES)
# 链接时需要的库路径与库名（使用 libgba）
LIBS := -L/c/devkitPro/libgba/lib -lgba
# 链接器标志：使用 gba.specs 以生成符合 GBA 的可执行格式
LDFLAGS := -specs=gba.specs

# 输出目录与目标设置
OUTDIR := bin
OBJDIR := obj
TARGET := citygame

# 源文件列表（以 src 子目录组织）
SOURCES_CPP := src/main.cpp src/generated_assets.cpp
# 根据源文件生成对应的目标文件路径（放在 obj 目录），并包含音频对象
OBJECTS := $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(SOURCES_CPP)) $(OBJDIR)/morningmix.o

# 中间 ELF 文件与最终 GBA 二进制文件路径
ELF := $(OUTDIR)/$(TARGET).elf
GBA := $(OUTDIR)/$(TARGET).gba

# 默认目标：生成最终 GBA 可执行文件
all: $(GBA)

# 资源生成规则：当资源或脚本变更时，运行构建脚本生成 C++ 资源源文件
src/generated_assets.cpp: tools/build_assets.py res/Map/map.tmx res/Map/roguelikeSheet_transparent.png res/Tiles/tile_0008.png res/Tiles/tile_0009.png
	python tools/build_assets.py

# 音频生成规则：将 Ogg 转换为原始 PCM 数据，供后续转换为对象文件
$(OBJDIR)/morningmix.pcm: tools/build_audio.py res/Ogg/morningmix.ogg | $(OBJDIR)
	python tools/build_audio.py

# 将 PCM 二进制转换为 ELF 可链接对象，并把数据节重命名为只读节
$(OBJDIR)/morningmix.o: $(OBJDIR)/morningmix.pcm | $(OBJDIR)
	$(OBJCOPY) -I binary -O elf32-littlearm -B arm --rename-section .data=.rodata,alloc,load,readonly,data,contents $< $@

# C++ 源文件编译规则：把 src/*.cpp 编译为 obj/*.o
$(OBJDIR)/%.o: src/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 确保输出目录存在
$(OUTDIR):
	mkdir -p $(OUTDIR)

# 确保目标文件目录存在
$(OBJDIR):
	mkdir -p $(OBJDIR)

# 链接阶段：将所有目标文件链接为 ELF 可执行文件
$(ELF): $(OBJECTS) | $(OUTDIR)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $(ELF)

# 把 ELF 转换成 GBA 可装载的二进制格式
$(GBA): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(GBA)

# 清理中间文件与生成物
clean:
	rm -f $(OBJECTS) $(ELF) $(GBA) $(OBJDIR)/morningmix.pcm
