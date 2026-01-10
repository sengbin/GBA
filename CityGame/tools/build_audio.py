import os
import shutil
import subprocess
import sys
from pathlib import Path


def _find_ffmpeg() -> str:
    env = os.environ.get("FFMPEG")
    if env:
        return env

    p = shutil.which("ffmpeg")
    if p:
        return p

    p = shutil.which("ffmpeg.exe")
    if p:
        return p

    return ""


def main() -> int:
    tool_dir = Path(__file__).resolve().parent
    project_dir = tool_dir.parent

    ogg_path = project_dir / "res" / "Ogg" / "morningmix.ogg"
    if not ogg_path.exists():
        print(f"找不到音频文件：{ogg_path}", file=sys.stderr)
        return 2

    obj_dir = project_dir / "obj"
    obj_dir.mkdir(parents=True, exist_ok=True)

    pcm_path = obj_dir / "morningmix.pcm"

    sample_rate = 16384

    ffmpeg = _find_ffmpeg()
    if not ffmpeg:
        print("未找到 ffmpeg（可通过环境变量 FFMPEG 指定 ffmpeg 路径）", file=sys.stderr)
        return 3

    cmd = [
        ffmpeg,
        "-y",
        "-i",
        str(ogg_path),
        "-vn",
        "-ac",
        "1",
        "-ar",
        str(sample_rate),
        "-f",
        "s8",
        str(pcm_path),
    ]

    subprocess.run(cmd, check=True)

    # DMA32 读取时按 4 字节对齐更安全，这里将 PCM 补齐到 4 的倍数。
    size = pcm_path.stat().st_size
    pad = (-size) % 4
    if pad:
        with open(pcm_path, "ab") as f:
            f.write(b"\x00" * pad)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
