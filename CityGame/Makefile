CC := /c/devkitPro/devkitARM/bin/arm-none-eabi-gcc
CXX := /c/devkitPro/devkitARM/bin/arm-none-eabi-g++
OBJCOPY := /c/devkitPro/devkitARM/bin/arm-none-eabi-objcopy

CFLAGS := -mthumb -mthumb-interwork -O2 -ffunction-sections -fdata-sections -Wall
INCLUDES := -I/c/devkitPro/libgba/include
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-exceptions -fno-rtti $(INCLUDES)
LIBS := -L/c/devkitPro/libgba/lib -lgba
LDFLAGS := -specs=gba.specs

OUTDIR := bin
OBJDIR := obj
TARGET := citygame

SOURCES_CPP := src/main.cpp src/generated_assets.cpp
OBJECTS := $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(SOURCES_CPP))

ELF := $(OUTDIR)/$(TARGET).elf
GBA := $(OUTDIR)/$(TARGET).gba

all: $(GBA)

src/generated_assets.cpp: tools/build_assets.py res/Map/map.tmx res/Map/roguelikeSheet_transparent.png res/Tiles/tile_0008.png res/Tiles/tile_0009.png
	python tools/build_assets.py

$(OBJDIR)/%.o: src/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(ELF): $(OBJECTS) | $(OUTDIR)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $(ELF)

$(GBA): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(GBA)

clean:
	rm -f $(OBJECTS) $(ELF) $(GBA)
