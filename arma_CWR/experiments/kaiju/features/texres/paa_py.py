"""A small PAA writer in plain Python and numpy, for texture tests.

PoseidonTools `image convert` is the tool the artists use and is what the
texture standard names. This module exists for the cases that tool cannot
make: a PAA whose mip levels are authored one by one (the "mip tell", every
level a different flat colour, which shows in a screenshot which level the
GPU sampled), a PAA with more levels than the tool writes, or a format
written raw to see what the engine's reader does with it.

File layout, as the engine reads it (Graphics/Rendering/Font/Pactext.cpp,
TextureSourcePac::Init and PacLevelMem::Init / LoadPaa):

    u16 magic (DXT1 0xFF01, DXT5 0xFF05, ARGB8888 0x8888, ARGB4444 0x4444)
    [TAGG blocks, optional; none written here, like PoseidonTools]
    u16 palette colours (0)
    per mip: u16 width, u16 height, u24 byte size, data
    u16 0, u16 0

The byte size is 24 bits, so one mip cannot be 16 MiB or more: DXT1 tops out
at 4096 x 4096 (8 MiB), DXT5 and ARGB4444 at 2048 x 2048, ARGB8888 at
1024 x 1024. The reader also refuses any side over 4096 (Pactext.cpp:1410).
"""
import struct

import numpy as np

MAGIC = {"DXT1": 0xFF01, "DXT5": 0xFF05, "ARGB8888": 0x8888, "ARGB4444": 0x4444}


def rgb565(rgb):
    """uint8 (..., 3) -> uint16 565."""
    r = rgb[..., 0].astype(np.uint16) >> 3
    g = rgb[..., 1].astype(np.uint16) >> 2
    b = rgb[..., 2].astype(np.uint16) >> 3
    return (r << 11) | (g << 5) | b


def dxt1_flat(w, h, colour):
    """One flat colour, four colour mode (c0 > c1), every index 0."""
    c0 = int(rgb565(np.array(colour, dtype=np.uint8)))
    c1 = c0 - 1 if c0 > 0 else 0
    if c0 == 0:
        c0, c1 = 1, 0       # near black, still c0 > c1
    block = struct.pack("<HHI", c0, c1, 0)
    return block * ((w // 4) * (h // 4))


def dxt1_encode(img):
    """Range-fit DXT1 for a uint8 (H, W, 3) array, H and W multiples of 4.

    Bounding-box endpoints, four colour mode. Not a quality encoder; good
    enough for charts with two colours per block.
    """
    h, w = img.shape[:2]
    blocks = img.reshape(h // 4, 4, w // 4, 4, 3).transpose(0, 2, 1, 3, 4).reshape(-1, 16, 3).astype(np.int32)
    hi = blocks.max(axis=1)
    lo = blocks.min(axis=1)
    c0 = rgb565(hi.astype(np.uint8)).astype(np.uint32)
    c1 = rgb565(lo.astype(np.uint8)).astype(np.uint32)
    swap = c0 < c1
    c0, c1 = np.where(swap, c1, c0), np.where(swap, c0, c1)
    hi2 = np.where(swap[:, None], lo, hi)
    lo2 = np.where(swap[:, None], hi, lo)
    same = c0 == c1
    c0 = np.where(same & (c0 == 0), 1, c0)
    c1 = np.where(same & (c1 > 0), c1 - 1, c1)
    d = (hi2 - lo2).astype(np.float64)
    den = (d * d).sum(axis=1)
    den[den == 0] = 1.0
    t = ((blocks - lo2[:, None, :]) * d[:, None, :]).sum(axis=2) / den[:, None]     # 1 at c0, 0 at c1
    idx = np.where(t >= 5 / 6, 0, np.where(t >= 0.5, 2, np.where(t >= 1 / 6, 3, 1))).astype(np.uint32)
    idx = np.where(same[:, None], 0, idx)
    bits = np.zeros(len(blocks), dtype=np.uint32)
    for i in range(16):
        bits |= idx[:, i] << np.uint32(2 * i)
    out = np.zeros(len(blocks), dtype=[("c0", "<u2"), ("c1", "<u2"), ("bits", "<u4")])
    out["c0"], out["c1"], out["bits"] = c0, c1, bits
    return out.tobytes()


def write_paa(path, fmt, mips):
    """mips: list of (width, height, bytes), largest first."""
    with open(path, "wb") as fh:
        fh.write(struct.pack("<H", MAGIC[fmt]))
        fh.write(struct.pack("<H", 0))
        for w, h, data in mips:
            if len(data) >= 1 << 24:
                raise ValueError("%dx%d %s mip is %d bytes: over the 24 bit size field" % (w, h, fmt, len(data)))
            fh.write(struct.pack("<HH", w, h))
            fh.write(struct.pack("<I", len(data))[:3])
            fh.write(data)
        fh.write(struct.pack("<HH", 0, 0))


def read_paa_header(path):
    """Returns (magic, [(w, h, size)]) without decoding. Reads TAGG-less and TAGG files."""
    with open(path, "rb") as fh:
        data = fh.read()
    magic = struct.unpack_from("<H", data, 0)[0]
    pos = 2
    while data[pos:pos + 4] == b"GGAT":
        size = struct.unpack_from("<I", data, pos + 8)[0]
        pos += 12 + size
    pos += 2  # palette count (0)
    mips = []
    while pos + 4 <= len(data):
        w, h = struct.unpack_from("<HH", data, pos)
        if w == 0 and h == 0:
            break
        size = data[pos + 4] | (data[pos + 5] << 8) | (data[pos + 6] << 16)
        mips.append((w, h, size))
        pos += 7 + size
    return magic, mips
