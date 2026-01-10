---
applyTo: '**'
---

# 指令

1. 必须使用c++开发GBA项目。
2. GBA项目程序编译输出目录规范
- 二进制文件输出到 bin 目录
- o 文件输出到 obj 目录

3. GBA项目资源文件目录规范
- 资源文件放在 res 目录下，按类型分子目录存放，如 res/images, res/sounds 等

4. GBA项目源代码目录规范
- 源代码文件放在 src 目录下，按模块分子目录存放，如 src/graphics, src/audio 等

5. 中文库在 D:\GitCode\GBA-Github\ZhFont，如需要中文输出，使用该库。
6. GBA项目都要创建 Makefile 文件，方便编译和管理项目。
7. GBA项目都要创建 README.md 文件，说明项目功能和使用方法。
8. GBA项目都要使用 libgba 库，方便调用 GBA 硬件功能。
9. GBA项目都要使用 devkitPro 工具链进行编译和链接。
10. 每次修改完代码都要执行 make clean; make，确保没有编译错误和链接错误。