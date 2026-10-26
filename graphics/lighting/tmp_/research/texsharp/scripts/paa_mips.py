"""Offline look at how the stored PAA/PAC mip chains compare with rebuilt ones.

    python -I paa_mips.py <texture.paa|pac> <out_prefix>

Decodes every stored DXT1 mip of the file, rebuilds the chain from the top
level with a 2x2 box and with the Lanczos-2 kernel the engine prototype uses
(TextureGL33::UploadRemipped), with and without alpha-coverage preservation at
the cutout reference 0xc0, and writes:
    <out_prefix>_mips.png      rows: stored | box | Lanczos | Lanczos+coverage,
                               columns: mip 1..4 upscaled (nearest) to the
                               top-level size, so lost detail is visible
    <out_prefix>_alpha.txt     alpha coverage (fraction of texels >= 0xc0) and
                               mean colour per level for each chain
Only DXT1 (0xFF01) is handled (all the CWA terrain, tree and vehicle textures
looked at here are DXT1).
"""
import struct
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont


def rgb565(c):
    r = (c >> 11) & 31
    g = (c >> 5) & 63
    b = c & 31
    return np.stack([(r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2)], -1).astype(np.float32)


def decode_dxt1(data, w, h):
    bw, bh = max(1, (w + 3) // 4), max(1, (h + 3) // 4)
    blocks = np.frombuffer(data[: bw * bh * 8], dtype=np.uint8).reshape(bh, bw, 8)
    c0 = blocks[..., 0].astype(np.uint16) | (blocks[..., 1].astype(np.uint16) << 8)
    c1 = blocks[..., 2].astype(np.uint16) | (blocks[..., 3].astype(np.uint16) << 8)
    idx = (blocks[..., 4].astype(np.uint32) | (blocks[..., 5].astype(np.uint32) << 8) |
           (blocks[..., 6].astype(np.uint32) << 16) | (blocks[..., 7].astype(np.uint32) << 24))
    p0, p1 = rgb565(c0), rgb565(c1)
    four = (c0 > c1)[..., None]
    p2 = np.where(four, (2 * p0 + p1) / 3, (p0 + p1) / 2)
    p3 = np.where(four, (p0 + 2 * p1) / 3, 0)
    a3 = np.where(c0 > c1, 255, 0)
    pal = np.stack([p0, p1, p2, p3], 2)  # bh, bw, 4, 3
    out = np.zeros((bh * 4, bw * 4, 4), np.float32)
    for py in range(4):
        for px in range(4):
            sel = (idx >> (2 * (py * 4 + px))) & 3
            col = np.take_along_axis(pal, sel[..., None, None].repeat(3, -1), 2)[:, :, 0]
            alpha = np.where(sel == 3, a3, 255)
            out[py::4, px::4, :3] = col
            out[py::4, px::4, 3] = alpha
    return out[:h, :w]


def read_mips(path):
    b = open(path, "rb").read()
    typ = struct.unpack_from("<H", b, 0)[0]
    if typ != 0xFF01:
        raise SystemExit(f"type 0x{typ:04X} not handled (DXT1 only)")
    o = 2
    while b[o:o + 4] == b"GGAT":
        ln = struct.unpack_from("<I", b, o + 8)[0]
        o += 12 + ln
    pal = struct.unpack_from("<H", b, o)[0]
    o += 2 + pal * 3
    mips = []
    while o + 4 <= len(b):
        w, h = struct.unpack_from("<HH", b, o)
        if w == 0 and h == 0:
            break
        if w & 0x8000:
            raise SystemExit("LZO-compressed DXT mip not handled")
        size = b[o + 4] | (b[o + 5] << 8) | (b[o + 6] << 16)
        mips.append(decode_dxt1(b[o + 7:o + 7 + size], w, h))
        o += 7 + size
    return mips


KW = np.array([0.8774, 0.2353, -0.0847, -0.0179], np.float32)


def lanczos_axis(img, axis):
    n = img.shape[axis]
    if n <= 1:
        return img
    m = n // 2
    o = np.arange(m)
    acc = 0
    for k in range(4):
        a = (2 * o - k) % n
        bb = (2 * o + 1 + k) % n
        acc = acc + KW[k] * (np.take(img, a, axis) + np.take(img, bb, axis))
    return np.clip(acc / (2 * KW.sum()), 0, 255)


def box(img):
    h, w = img.shape[:2]
    return img.reshape(h // 2, 2, w // 2, 2, 4).mean((1, 3))


def coverage(img, ref=192.0, s=1.0):
    return float((img[..., 3] * s >= ref).mean())


def preserve(img, target, ref=192.0):
    lo, hi = 0.0, 4.0
    for _ in range(12):
        mid = 0.5 * (lo + hi)
        if coverage(img, ref, mid) < target:
            lo = mid
        else:
            hi = mid
    out = img.copy()
    out[..., 3] = np.minimum(out[..., 3] * hi, 255)
    return out


def main():
    path, prefix = sys.argv[1], sys.argv[2]
    stored = read_mips(path)
    top = stored[0]
    chains = {"stored (PAA)": stored}
    bx, lz, lc = [top], [top], [top]
    c0 = coverage(top)
    for i in range(1, len(stored)):
        bx.append(box(bx[-1]))
        lz.append(lanczos_axis(lanczos_axis(lz[-1], 1), 0))
        lc.append(preserve(lanczos_axis(lanczos_axis(lc[-1], 1), 0), c0))
    chains["box"] = bx
    chains["Lanczos-2"] = lz
    chains["Lanczos-2 + coverage"] = lc
    lines = [f"{path}  top {top.shape[1]}x{top.shape[0]}  coverage(a>=192) top={c0:.3f}"]
    for name, ch in chains.items():
        lines.append(name + ": " + "  ".join(f"L{i} cov={coverage(m):.3f} mean=({m[..., :3].mean((0, 1))[0]:.0f},"
                                                f"{m[..., :3].mean((0, 1))[1]:.0f},{m[..., :3].mean((0, 1))[2]:.0f})"
                                                for i, m in enumerate(ch[:5])))
    open(prefix + "_alpha.txt", "w").write("\n".join(lines) + "\n")
    print("\n".join(lines))

    S = top.shape[1] // 2
    levels = [1, 2, 3, 4]
    fnt = ImageFont.truetype("C:/Windows/Fonts/arial.ttf", 16)
    sheet = Image.new("RGB", (len(levels) * (S + 6) + 170, len(chains) * (S + 6) + 30), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    d.text((6, 6), f"{path.split(chr(92))[-1]}: mips 1-4, nearest-upscaled, composited on grey (alpha >= 0xc0 shown)",
           font=fnt, fill=(230, 230, 230))
    for r, (name, ch) in enumerate(chains.items()):
        d.text((6, 30 + r * (S + 6) + S // 2), name, font=fnt, fill=(230, 230, 230))
        for c, lv in enumerate(levels):
            if lv >= len(ch):
                continue
            m = ch[lv]
            keep = (m[..., 3] >= 192)[..., None]
            rgb = np.where(keep, m[..., :3], 128).astype(np.uint8)
            im = Image.fromarray(rgb).resize((S, S), Image.NEAREST)
            sheet.paste(im, (170 + c * (S + 6), 30 + r * (S + 6)))
    sheet.save(prefix + "_mips.png")
    print(prefix + "_mips.png")


if __name__ == "__main__":
    main()
