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

TARGET := gba-demo
SOURCES_CPP := main.cpp
OBJECTS := $(SOURCES_CPP:.cpp=.o)

FONTLIB := ../ZhFontLib/ZhFontLib.a

all: $(TARGET).gba

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

$(FONTLIB):
	$(MAKE) -C ../ZhFontLib

$(TARGET).elf: $(OBJECTS) $(FONTLIB)
	$(CXX) $(OBJECTS) $(FONTLIB) $(LDFLAGS) $(LIBS) -o $(TARGET).elf

$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).gba

clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).gba
