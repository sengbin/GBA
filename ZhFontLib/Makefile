CC := /c/devkitPro/devkitARM/bin/arm-none-eabi-gcc
CXX := /c/devkitPro/devkitARM/bin/arm-none-eabi-g++
AR := /c/devkitPro/devkitARM/bin/arm-none-eabi-ar

CFLAGS := -mthumb -mthumb-interwork -O2 -ffunction-sections -fdata-sections -Wall
# 包含 libgba 头文件目录
INCLUDES := -I/c/devkitPro/libgba/include
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-exceptions -fno-rtti $(INCLUDES)

TARGET := ZhFontLib.a
SOURCES_CPP := UnifontFont.cpp
SOURCES_S := unifont16x16.S
OBJECTS := $(SOURCES_CPP:.cpp=.o) $(SOURCES_S:.S=.o)

all: $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(AR) rcs $(TARGET) $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(TARGET)
