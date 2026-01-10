import base64
import struct
import zlib
from dataclasses import dataclass
from pathlib import Path
import xml.etree.ElementTree as ET

from PIL import Image


@dataclass
class TmxLayer:
    name: str
    gids: list[int]


def _decode_layer_data_base64_zlib(data_text: str, cell_count: int) -> list[int]:
    raw = base64.b64decode(data_text.strip())
    raw = zlib.decompress(raw)
    gids = struct.unpack(f"<{cell_count}I", raw)
    return list(gids)


def _rgb_to_bgr555(r: int, g: int, b: int) -> int:
    r5 = (r & 0xFF) >> 3
    g5 = (g & 0xFF) >> 3
    b5 = (b & 0xFF) >> 3
    return (b5) | (g5 << 5) | (r5 << 10)


def _fill_transparent_with_magenta(im_rgba: Image.Image) -> Image.Image:
    magenta = (255, 0, 255, 255)
    out = Image.new("RGBA", im_rgba.size, magenta)
    out.paste(im_rgba, (0, 0), im_rgba)
    return out.convert("RGB")


def _find_palette_index(palette: list[int], rgb: tuple[int, int, int]) -> int:
    r, g, b = rgb
    for i in range(256):
        if palette[i * 3 + 0] == r and palette[i * 3 + 1] == g and palette[i * 3 + 2] == b:
            return i
    return -1


def _swap_palette_entries(palette: list[int], index_a: int, index_b: int) -> list[int]:
    if index_a == index_b:
        return palette
    out = palette[:]
    for c in range(3):
        out[index_a * 3 + c], out[index_b * 3 + c] = out[index_b * 3 + c], out[index_a * 3 + c]
    return out


def _quantize_palette_source(palette_source_rgba: Image.Image) -> list[int]:
    pal_source = palette_source_rgba.copy()

    # 强制加入一个纯品红像素，确保调色板一定包含 (255,0,255)
    pal_source.putpixel((0, 0), (255, 0, 255, 255))

    rgb = _fill_transparent_with_magenta(pal_source)
    pal_im = rgb.quantize(colors=256, dither=Image.Dither.NONE)
    palette = pal_im.getpalette() or []
    if len(palette) < 256 * 3:
        palette = palette + [0] * (256 * 3 - len(palette))
    palette = palette[: 256 * 3]

    magenta_index = _find_palette_index(palette, (255, 0, 255))
    if magenta_index < 0:
        raise RuntimeError("调色板未包含品红色(255,0,255)，无法保留透明色索引")

    # 透明色固定使用索引 0
    palette = _swap_palette_entries(palette, 0, magenta_index)
    return palette


def _make_palette_image(palette: list[int]) -> Image.Image:
    pal = Image.new("P", (1, 1))
    pal.putpalette(palette)
    return pal


def _quantize_with_palette(im_rgba: Image.Image, pal_im: Image.Image) -> Image.Image:
    rgb = _fill_transparent_with_magenta(im_rgba)
    return rgb.quantize(palette=pal_im, dither=Image.Dither.NONE)


def _crop_tileset_tile(tileset_rgba: Image.Image, tile_index: int, columns: int, tile_w: int, tile_h: int, spacing: int) -> Image.Image:
    x = (tile_index % columns) * (tile_w + spacing)
    y = (tile_index // columns) * (tile_h + spacing)
    return tileset_rgba.crop((x, y, x + tile_w, y + tile_h))


def main() -> None:
    project_dir = Path(__file__).resolve().parents[1]

    tmx_path = project_dir / "Map" / "map.tmx"
    if not tmx_path.exists():
        raise FileNotFoundError(str(tmx_path))

    tree = ET.parse(tmx_path)
    root = tree.getroot()

    map_w = int(root.get("width"))
    map_h = int(root.get("height"))
    tile_w = int(root.get("tilewidth"))
    tile_h = int(root.get("tileheight"))

    tileset = root.find("tileset")
    if tileset is None:
        raise RuntimeError("TMX 未包含 tileset")

    columns = int(tileset.get("columns"))
    spacing = int(tileset.get("spacing") or "0")

    image_tag = tileset.find("image")
    if image_tag is None:
        raise RuntimeError("TMX tileset 未包含 image")

    tileset_image_source = image_tag.get("source")
    if not tileset_image_source:
        raise RuntimeError("TMX tileset image source 为空")

    tileset_png_path = (tmx_path.parent / tileset_image_source).resolve()
    if not tileset_png_path.exists():
        raise FileNotFoundError(str(tileset_png_path))

    layer_nodes = root.findall("layer")
    if len(layer_nodes) < 4:
        raise RuntimeError("TMX 图层数量不足 4")

    layers: list[TmxLayer] = []
    used_gids: set[int] = set()
    cell_count = map_w * map_h

    for layer in layer_nodes[:4]:
        data = layer.find("data")
        if data is None:
            raise RuntimeError("TMX layer 缺少 data")
        if data.get("encoding") != "base64" or data.get("compression") != "zlib":
            raise RuntimeError("仅支持 base64+zlib 编码")

        gids = _decode_layer_data_base64_zlib(data.text or "", cell_count)
        layers.append(TmxLayer(name=layer.get("name") or "", gids=gids))

        for gid in gids:
            if gid != 0:
                used_gids.add(gid)

    used_gid_list = sorted(used_gids)
    max_gid = max(used_gid_list) if used_gid_list else 0

    tileset_rgba = Image.open(tileset_png_path).convert("RGBA")

    # 角色两帧
    player0_path = project_dir / "Images" / "Tiles" / "tile_0008.png"
    player1_path = project_dir / "Images" / "Tiles" / "tile_0009.png"
    if not player0_path.exists():
        raise FileNotFoundError(str(player0_path))
    if not player1_path.exists():
        raise FileNotFoundError(str(player1_path))

    player0_rgba = Image.open(player0_path).convert("RGBA")
    player1_rgba = Image.open(player1_path).convert("RGBA")

    # 生成调色板源图：把所有会用到的 tile 与角色帧拼起来，再量化为 256 色
    composite_w = len(used_gid_list) * tile_w + player0_rgba.size[0] + player1_rgba.size[0]
    composite_h = max(tile_h, player0_rgba.size[1], player1_rgba.size[1], 1)
    if composite_w <= 0:
        composite_w = 1

    composite = Image.new("RGBA", (composite_w, composite_h), (0, 0, 0, 0))

    x_cursor = 0
    for gid in used_gid_list:
        tile_index = gid - 1
        tile_rgba = _crop_tileset_tile(tileset_rgba, tile_index, columns, tile_w, tile_h, spacing)
        composite.paste(tile_rgba, (x_cursor, 0), tile_rgba)
        x_cursor += tile_w

    composite.paste(player0_rgba, (x_cursor, 0), player0_rgba)
    x_cursor += player0_rgba.size[0]
    composite.paste(player1_rgba, (x_cursor, 0), player1_rgba)

    palette = _quantize_palette_source(composite)
    pal_im = _make_palette_image(palette)

    # GBA 调色板（BGR555）
    gba_palette = [_rgb_to_bgr555(palette[i * 3 + 0], palette[i * 3 + 1], palette[i * 3 + 2]) for i in range(256)]

    # gid -> 8x8 tile baseIndex 映射（16x16 由 2x2 个 8x8 组成）
    # 0 号 tile 预留为空白 tile，便于 gid==0 时使用。
    gid_to_base_tile8 = [0] * (max_gid + 1)

    bg_tiles: list[int] = [0] * (8 * 8)
    bg_tile_count = 1

    for gid in used_gid_list:
        base = bg_tile_count
        gid_to_base_tile8[gid] = base

        tile_index = gid - 1
        tile_rgba = _crop_tileset_tile(tileset_rgba, tile_index, columns, tile_w, tile_h, spacing)
        tile_p = _quantize_with_palette(tile_rgba, pal_im)
        tile_bytes = tile_p.tobytes()

        # 按象限拆分为 4 个 8x8（顺序：左上、右上、左下、右下）
        for qy in range(2):
            for qx in range(2):
                for y in range(8):
                    row = (qy * 8 + y) * 16
                    col = qx * 8
                    bg_tiles.extend(tile_bytes[row + col : row + col + 8])

        bg_tile_count += 4

    # 角色精灵：打包为 32x32 的 8x8 tiles（256 色 OBJ）
    player0_p = _quantize_with_palette(player0_rgba, pal_im)
    player1_p = _quantize_with_palette(player1_rgba, pal_im)

    player0_top = (player0_rgba.getchannel("A").getbbox() or (0, 0, 0, 0))[1]
    player1_top = (player1_rgba.getchannel("A").getbbox() or (0, 0, 0, 0))[1]
    player_ref_top = max(player0_top, player1_top)

    def pack_obj_32x32_tiles(p_im: Image.Image, top: int) -> list[int]:
        w, h = p_im.size
        src = list(p_im.tobytes())

        # 32x32 画布，透明为 0
        canvas = [[0 for _ in range(32)] for _ in range(32)]
        ox = (32 - w) // 2
        oy = (32 - h) // 2 + (player_ref_top - top)
        for y in range(h):
            for x in range(w):
                yy = oy + y
                xx = ox + x
                if 0 <= xx < 32 and 0 <= yy < 32:
                    canvas[yy][xx] = src[y * w + x]

        out: list[int] = []
        for ty in range(4):
            for tx in range(4):
                for y in range(8):
                    for x in range(8):
                        out.append(canvas[ty * 8 + y][tx * 8 + x])
        return out

    player_obj_tiles: list[int] = []
    player_obj_frame0_tile_id = 0
    player_obj_tiles.extend(pack_obj_32x32_tiles(player0_p, player0_top))
    player_obj_frame1_tile_id = (32 * 32) // 32
    player_obj_tiles.extend(pack_obj_32x32_tiles(player1_p, player1_top))

    out_cpp = project_dir / "src" / "generated_assets.cpp"

    header_comment = """/*------------------------------------------------------------------------
名称：资源生成文件
说明：由 tools/build_assets.py 自动生成的资源数据（调色板、地图图层、瓦片与角色帧）
作者：Lion
邮箱：chengbin@3578.cn
日期：2026-01-10
备注：请勿手工修改
------------------------------------------------------------------------*/
"""

    def fmt_u16_array(name: str, values: list[int], per_line: int = 12) -> str:
        lines = [f"extern const unsigned short {name}[] __attribute__((aligned(4))) = {{"]
        for i in range(0, len(values), per_line):
            chunk = values[i : i + per_line]
            lines.append("    " + ", ".join(f"0x{v:04X}" for v in chunk) + ",")
        lines.append("};")
        return "\n".join(lines)

    def fmt_u8_array(name: str, values: list[int], per_line: int = 24) -> str:
        lines = [f"extern const unsigned char {name}[] __attribute__((aligned(4))) = {{"]
        for i in range(0, len(values), per_line):
            chunk = values[i : i + per_line]
            lines.append("    " + ", ".join(str(v) for v in chunk) + ",")
        lines.append("};")
        return "\n".join(lines)

    cpp_parts: list[str] = [
        header_comment,
        f"extern const int g_MapWidth = {map_w};\nextern const int g_MapHeight = {map_h};\nextern const int g_TileWidth = {tile_w};\nextern const int g_TileHeight = {tile_h};\n",
        f"extern const unsigned int g_UsedTileCount = {len(used_gid_list)};\n",
        f"extern const unsigned int g_BgTileCount = {bg_tile_count};\n",
        fmt_u16_array("g_Palette", gba_palette, per_line=12),
        "",
    ]

    # 图层数据（u16 存 gid，0 表示空）
    for i, layer in enumerate(layers):
        layer_u16 = [int(gid) & 0xFFFF for gid in layer.gids]
        cpp_parts.append(fmt_u16_array(f"g_Layer{i}", layer_u16, per_line=16))
        cpp_parts.append("")

    cpp_parts.append(fmt_u16_array("g_GidToBaseTile8", gid_to_base_tile8, per_line=16))
    cpp_parts.append("")

    cpp_parts.append(fmt_u8_array("g_BgTiles", bg_tiles, per_line=32))
    cpp_parts.append("")

    cpp_parts.append(f"extern const int g_PlayerWidth = {player0_rgba.size[0]};\nextern const int g_PlayerHeight = {player0_rgba.size[1]};\n")
    cpp_parts.append(f"extern const unsigned short g_PlayerObjFrame0TileId = {player_obj_frame0_tile_id};\n")
    cpp_parts.append(f"extern const unsigned short g_PlayerObjFrame1TileId = {player_obj_frame1_tile_id};\n")
    cpp_parts.append(fmt_u8_array("g_PlayerObjTiles", player_obj_tiles, per_line=32))
    cpp_parts.append("")

    out_cpp.write_text("\n".join(cpp_parts), encoding="utf-8")


if __name__ == "__main__":
    main()
