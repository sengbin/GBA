# CityGame (GBA) — 开发要点速查

**项目概述**
- **目标**: 在 GBA 上实现基于 TMX 的城市场景（4 个图层独立渲染）、平滑滚动、角色两帧动画、左右翻转、树/屋顶碰撞与可编译输出 `.gba`。

**关键实现要点**
- **渲染模式**: 使用 MODE_0 + BG0..BG3（每层独立），避免将图层合并。
- **地图缓冲**: 采用 64×32（tile）环形缓冲区，按列/行做增量更新（`UpdateLayerColumn` / `UpdateLayerRow`），减少 VRAM 写入和闪烁。
- **对齐与竖条纹避免**: 绘制函数要求目标 X 为偶数、保证 tile/像素对齐以避免竖条纹；边缘多出一列/行需覆盖。
- **精灵与动画**: 玩家为 32×32 OBJ，两帧走路动画，水平翻转通过 OBJ attr1 bit 实现（`SetPlayerObjHFlip`）。
- **调色板与 tiles**: 在启动阶段用 `LoadPalette2()`、`LoadBgTiles()`、`LoadPlayerObjTiles()` 载入资源。
- **同步与帧**: 主循环使用 `VBlankIntrWait()`，在 VBlank 内做 BG HOFS/VOFS 更新，避免可见期间写 VRAM。

**资源与构建**
- **目录**: 资源均放在 `res/` 下（示例: `res/Map`, `res/Tiles`, `res/Ogg`, `res/Sounds`）。
- **自动生成**: `tools/build_assets.py` 用于生成 `src/generated_assets.cpp`（图块/地图/调色板）。

**已接入的音频（BGM）方案**
- **源文件**: `res/Ogg/morningmix.ogg`。
- **转码工具**: 新增 `tools/build_audio.py`，使用 `ffmpeg` 将 OGG → PCM（signed 8-bit, mono, 16384 Hz），输出 `obj/morningmix.pcm`，并补齐到 4 字节对齐以便 DMA32 读取。
- **Makefile 改动**: 增加规则先运行 `tools/build_audio.py` 生成 `obj/morningmix.pcm`，再用 `objcopy` 生成 `obj/morningmix.o`，并将该二进制段从默认 `.data` 重命名为 `.rodata`（避免占用 IWRAM/.data 区导致链接失败）。
- **运行时播放**: 在 `src/main.cpp` 中加入播放逻辑：
  - 使用 DirectSound A (`REG_FIFO_A`)；
  - 用 DMA1（start mode = FIFO empty / special、`DMA_REPEAT`、`DMA32`、`DMA_DST_FIXED`）持续喂 FIFO；
  - 用 `Timer0` 设置为 16384Hz 驱动采样；`Timer1` 级联用于统计/检测播放位置以实现循环重启；
  - 在代码中通过链接符号 `_binary_obj_morningmix_pcm_start/_end` 引用 ROM 中的 PCM 数据并计算长度。
- **循环策略**: 程序通过读取已播放采样计数，接近结尾时重启播放（`StartBgm()`），避免 DMA 读取越界。
- **注意事项**: 确保 PCM 为 signed 8-bit 单声道且采样率与 Timer 配置一致；objcopy 生成的段需放到只读数据区以节省可用 RAM。

**构建说明（快速）**
```powershell
cd CityGame
make clean
make
```
- 若系统上 `ffmpeg` 不在 PATH，可用环境变量 `FFMPEG` 指定其路径：
```powershell
set FFMPEG=C:\full\path\to\ffmpeg.exe
make
```

**已修改/新增的关键文件**
- `src/main.cpp`: 添加 DirectSound/DMA/Timer 播放与循环控制代码。
- `Makefile`: 增加 audio 转码与 obj 链接规则，并将二进制段改为 `.rodata`。
- `tools/build_audio.py`: 新增的 OGG→PCM 转码脚本（依赖 ffmpeg）。

**后续建议（待验证/可选）**
- 在模拟器（或真实硬件）上验证 BGM 循环、音量与 CPU 占用，必要时通过 DMA IRQ 或更精确的 FIFO 管理优化循环切换。
- 若需更高质量或节省空间，可考虑降采样或使用更短循环片段再做淡入淡出处理。

---
（文档由聊天记录整理生成，若需把 README 调整为更正式的项目文档或加入更多使用示例，请告知具体需求。）
# CityGame

GBA 地图渲染与角色移动示例。

## 编译

在 devkitPro/devkitARM 环境下执行：

- `make`

输出：

- `bin/citygame.gba`
