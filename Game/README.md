GBA 瓦片地图最小示例

说明
- 本目录提供一个最小可运行示例（`src/main.c`），使用 Mode 3 帧缓冲模式绘制模拟瓦片格子，便于在没有 tile 转换步骤时快速验证显示与布局。

目标
- 演示地图格子布局与显示流程。
- 提供常用的 tile/map 资源与转换流程说明，便于下一步把真实 tileset 与 map 加入工程。

先决条件
- 推荐安装 devkitPro（含 libgba）用于构建 GBA 程序。
- 可选工具：Aseprite/GIMP/Krita（绘制 tiles）、Tiled（编辑地图）、grit（或其他工具，将 PNG/Tiled 导出为 C 数据）。

典型 tile/map 工作流（高层）
1. 使用 Aseprite 绘制 tileset（通常 8x8 或 16x16 元块），保存为索引颜色 PNG（4bpp，16 色）以节省 VRAM。
2. 在 Tiled 中用 tileset 制作地图（推荐使用 metatile 以简化地图编辑）。
3. 导出 tileset PNG 与地图数据（TMX 或 CSV）。
4. 使用 `grit` 或自定义脚本将 PNG/TMX 转为 C 头文件（包含 tiles、palette、map）。
5. 在程序中用 `memcpy` 将 palette/tiles/map 拷贝到 GBA 的 `BG_PALETTE`、`CHAR_BASE_BLOCK`、`SCREEN_BASE_BLOCK` 并设置 BG 寄存器渲染。

示例代码片段（加载步骤示意）

- 在转换后生成的头文件中，通常包含类似变量：`tiles[]`、`map[]`、`palette[]`。
- 在程序中使用：

  memcpy(BG_PALETTE, palette, sizeof(palette));
  memcpy(&CHAR_BASE_BLOCK(0)[0], tiles, tiles_size);
  memcpy(&SCREEN_BASE_BLOCK(31)[0], map, map_size);
  REG_BG0CNT = BG_COLOR_16 | BG_SIZE_0 | BG_CHARBASE(0) | BG_SCREENBASE(31);

说明：不同工具或库的宏名略有差异，请根据所用 SDK（如 libgba / TONC）调整宏。

后续
- 如果需要，我可以把一个真实的 tileset 与 Tiled 地图一起导出并用 grit 转换成可直接包含的 `.h`，再把加载代码加入到样例中。请回复是否需要我继续生成完整 tile 示例。
