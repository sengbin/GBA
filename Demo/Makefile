CC := /c/devkitPro/devkitARM/bin/arm-none-eabi-gcc
CXX := /c/devkitPro/devkitARM/bin/arm-none-eabi-g++
OBJCOPY := /c/devkitPro/devkitARM/bin/arm-none-eabi-objcopy

CFLAGS := -mthumb -mthumb-interwork -O2 -ffunction-sections -fdata-sections -Wall
# 包含 libgba 头文件目录
INCLUDES := -I/c/devkitPro/libgba/include
CFLAGS += $(INCLUDES)
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-exceptions -fno-rtti
# 链接时加入 libgba 库目录与库
LIBS := -L/c/devkitPro/libgba/lib -lgba
LDFLAGS := -specs=gba.specs

TARGET := gba-demo
SOURCES := main.cpp
CSOURCES := font_unifont16.c
OBJECTS := $(SOURCES:.cpp=.o) $(CSOURCES:.c=.o)

all: $(TARGET).gba

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $(TARGET).elf

$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).gba

clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).gba
