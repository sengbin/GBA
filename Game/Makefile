# 简易 GBA Makefile（基于 repo Demo/Makefile 风格）
# 使用说明：在 devkitPro 环境中运行 `make` 即可生成 `bin/gba-game.gba`。

CC := /c/devkitPro/devkitARM/bin/arm-none-eabi-gcc
OBJCOPY := /c/devkitPro/devkitARM/bin/arm-none-eabi-objcopy

CFLAGS := -mthumb -mthumb-interwork -O2 -ffunction-sections -fdata-sections -Wall
INCLUDES := -I/c/devkitPro/libgba/include
LIBS := -L/c/devkitPro/libgba/lib -lgba
LDFLAGS := -specs=gba.specs

OUTDIR := bin
OBJDIR := obj
TARGET := gba-game

SOURCES := $(wildcard src/*.c)
OBJECTS := $(patsubst src/%.c,$(OBJDIR)/%.o,$(SOURCES))

ELF := $(OUTDIR)/$(TARGET).elf
GBA := $(OUTDIR)/$(TARGET).gba

all: $(GBA)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(ELF): $(OBJECTS) | $(OUTDIR)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $(ELF)

$(GBA): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(GBA)

clean:
	rm -f $(OBJECTS) $(ELF) $(GBA)

.PHONY: all clean
