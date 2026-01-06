CC := /c/devkitPro/devkitARM/bin/arm-none-eabi-gcc
CXX := /c/devkitPro/devkitARM/bin/arm-none-eabi-g++
OBJCOPY := /c/devkitPro/devkitARM/bin/arm-none-eabi-objcopy

CFLAGS := -mthumb -mthumb-interwork -O2 -ffunction-sections -fdata-sections -Wall
# 包含 libgba 头文件目录
INCLUDES := -I/c/devkitPro/libgba/include -I../ZhFontLib
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-exceptions -fno-rtti $(INCLUDES)
# 链接时加入 libgba 库目录与库
LIBS := -L/c/devkitPro/libgba/lib -lgba
LDFLAGS := -specs=gba.specs

OUTDIR := bin
OBJDIR := obj
TARGET := gba-demo
SOURCES_CPP := main.cpp
OBJECTS := $(addprefix $(OBJDIR)/,$(SOURCES_CPP:.cpp=.o))

FONTLIB := ../ZhFontLib/bin/ZhFontLib.a

ELF := $(OUTDIR)/$(TARGET).elf
GBA := $(OUTDIR)/$(TARGET).gba

all: $(GBA)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

$(FONTLIB):
	$(MAKE) -C ../ZhFontLib

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(ELF): $(OBJECTS) $(FONTLIB) | $(OUTDIR)
	$(CXX) $(OBJECTS) $(FONTLIB) $(LDFLAGS) $(LIBS) -o $(ELF)

$(GBA): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(GBA)

clean:
	rm -f $(OBJECTS) $(ELF) $(GBA)
