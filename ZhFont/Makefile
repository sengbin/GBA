CC := /c/devkitPro/devkitARM/bin/arm-none-eabi-gcc
CXX := /c/devkitPro/devkitARM/bin/arm-none-eabi-g++
AR := /c/devkitPro/devkitARM/bin/arm-none-eabi-ar

CFLAGS := -mthumb -mthumb-interwork -O2 -ffunction-sections -fdata-sections -Wall
# 包含 libgba 头文件目录
INCLUDES := -I/c/devkitPro/libgba/include
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-exceptions -fno-rtti $(INCLUDES)

OUTDIR := bin
OBJDIR := obj
TARGET := $(OUTDIR)/zhfont.a
SOURCES_CPP := ZhFont.cpp
SOURCES_S := zhfont_data.S
OBJECTS := $(addprefix $(OBJDIR)/,$(SOURCES_CPP:.cpp=.o) $(SOURCES_S:.S=.o))

all: $(TARGET)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.S | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TARGET): $(OBJECTS) | $(OUTDIR)
	$(AR) rcs $(TARGET) $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(TARGET)
