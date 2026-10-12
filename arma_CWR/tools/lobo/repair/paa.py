"""Pure-Python reader and writer for OFP DXT1 PAA textures (stdlib only).

Layout read and written (the one every LoBo_Pilot body texture uses): the
format word 0xFF01, TAGG records (AVGC, OFFS, others kept opaque on read), a
zero palette count, uncompressed mip levels (u16 width, u16 height, u24 size,
DXT1 blocks) and a six-byte zero terminator. A level whose width carries the
0x8000 LZO flag, a non-zero palette or a truncated level is rejected. The
reader also decodes the top level of an AI88 (0x8080) PAA, whose levels are
LZSS streams: three grey IAF tex2 leg textures use it.

Decoding matches the engine (PAADecoder.cpp decodeDXT1Block): 565 endpoints
expand by bit replication, four-colour blocks interpolate as
(2a + b + 1) / 3 and a block whose c0 <= c1 is a three-colour block whose
index 3 is transparent black. Images are (width, height, r, g, b) with flat
row-major lists of 0..255 ints.

The encoder is deterministic: integer arithmetic plus IEEE double +, -, *
and / in a fixed order (no libm call, no builtin float sum),
so the same pixels give the same bytes on every platform and Python version.
Per 4x4 block it tries the principal-axis extremes and the inset bounding
box, quantises to 565, assigns each pixel its nearest palette colour, refines
the endpoints twice by least squares and keeps the lowest squared RGB error.
Every block is written in four-colour mode (c0 > c1), so nothing decodes as
transparent; a block whose endpoints quantise to the same colour stores
c0 = c1 with all indices 0.
"""
import struct

DXT1 = 0xFF01
AI88 = 0x8080
TAGG = b"GGAT"
AVGC = b"CGVA"
OFFS = b"SFFO"
TERMINATOR = bytes(6)


def _expand(c: int) -> tuple[int, int, int]:
    r, g, b = (c >> 11) & 31, (c >> 5) & 63, c & 31
    return (r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2)


def _palette(c0: int, c1: int) -> list[tuple[int, int, int]]:
    a, b = _expand(c0), _expand(c1)
    if c0 > c1:
        return [a, b, tuple((2 * x + y + 1) // 3 for x, y in zip(a, b)),
                tuple((x + 2 * y + 1) // 3 for x, y in zip(a, b))]
    return [a, b, tuple((x + y) // 2 for x, y in zip(a, b)), (0, 0, 0)]


def read(data: bytes) -> dict:
    """{'format', 'tags': [(name, payload)], 'levels': [(w, h, payload)], 'end': offset} of a DXT1 or AI88 PAA."""
    if len(data) < 4 or struct.unpack_from("<H", data)[0] not in (DXT1, AI88):
        raise ValueError("Not a DXT1 or AI88 PAA")
    fmt = struct.unpack_from("<H", data)[0]
    pos, tags = 2, []
    while data[pos:pos + 4] == TAGG:
        if pos + 12 > len(data):
            raise ValueError("Truncated PAA tag")
        name, size = data[pos + 4:pos + 8], struct.unpack_from("<i", data, pos + 8)[0]
        if size < 0 or pos + 12 + size > len(data):
            raise ValueError("Truncated PAA tag")
        tags.append((name, data[pos + 12:pos + 12 + size]))
        pos += 12 + size
    if data[pos:pos + 2] != b"\0\0":
        raise ValueError("Unexpected PAA palette")
    pos += 2
    levels = []
    while True:
        if pos + 4 > len(data):
            raise ValueError("Truncated PAA level")
        w, h = struct.unpack_from("<HH", data, pos)
        if w == 0 and h == 0:
            break
        if w & 0x8000 or not w or not h or w % 4 or h % 4:
            raise ValueError(f"Unsupported PAA level {w}x{h}")
        if pos + 7 > len(data):
            raise ValueError("Truncated PAA level")
        size = int.from_bytes(data[pos + 4:pos + 7], "little")
        if fmt == DXT1 and size != w * h // 2:
            raise ValueError(f"Invalid PAA level size {size} for {w}x{h}")
        if pos + 7 + size > len(data):
            raise ValueError("Truncated PAA level")
        levels.append((w, h, data[pos + 7:pos + 7 + size]))
        pos += 7 + size
    if not levels:
        raise ValueError("PAA has no levels")
    return {"format": fmt, "tags": tags, "levels": levels, "end": pos}


def decode(data: bytes) -> tuple:
    """Top level as (w, h, r, g, b). DXT1 transparent texels decode as black; AI88
    gives its 8-bit intensity in all three channels (the GL33 backend uploads AI88
    as RG8, so the engine draws the full 8 bits) and must be opaque."""
    info = read(data)
    w, h, payload = info["levels"][0]
    if info["format"] == DXT1:
        return decode_blocks(w, h, payload)
    raw = lzss(payload, w * h * 2)
    if any(a != 255 for a in raw[1::2]):
        raise ValueError("AI88 level is not opaque")
    grey = list(raw[0::2])
    return w, h, grey, grey[:], grey[:]


def lzss(payload: bytes, size: int) -> bytes:
    """OFP PAA LZSS (Pactext.cpp DecodeLZW): 4096-byte window filled with spaces,
    18-byte max match, backward offsets, then a signed byte-sum checksum (int32)."""
    window = bytearray(b" " * 4096)
    r, out, pos, flags = 4096 - 18, bytearray(), 0, 0
    try:
        while len(out) < size:
            flags >>= 1
            if not flags & 256:
                flags = payload[pos] | 0xFF00
                pos += 1
            if flags & 1:
                c = payload[pos]
                pos += 1
                out.append(c)
                window[r] = c
                r = (r + 1) & 4095
            else:
                lo, hi = payload[pos], payload[pos + 1]
                pos += 2
                start = r - (lo | ((hi & 0xF0) << 4))
                for k in range(start, start + (hi & 0x0F) + 3):
                    if len(out) >= size:
                        break
                    c = window[k & 4095]
                    out.append(c)
                    window[r] = c
                    r = (r + 1) & 4095
        stored = struct.unpack_from("<i", payload, pos)[0]
    except (IndexError, struct.error):
        raise ValueError("Truncated LZSS level") from None
    if pos + 4 != len(payload):
        raise ValueError("Unexpected bytes after an LZSS level")
    checksum = 0
    for c in out:
        checksum += c - 256 if c > 127 else c
    if (checksum - stored) % (1 << 32):
        raise ValueError("LZSS checksum mismatch")
    return bytes(out)


def decode_blocks(w: int, h: int, blocks: bytes) -> tuple:
    n = w * h
    r, g, b = [0] * n, [0] * n, [0] * n
    i = 0
    for by in range(0, h, 4):
        for bx in range(0, w, 4):
            c0, c1, idx = struct.unpack_from("<HHI", blocks, i)
            i += 8
            pal = _palette(c0, c1)
            for y in range(4):
                row = (by + y) * w + bx
                for x in range(4):
                    pr, pg, pb = pal[idx & 3]
                    idx >>= 2
                    r[row + x], g[row + x], b[row + x] = pr, pg, pb
    return w, h, r, g, b


def downscale(image: tuple) -> tuple:
    """Halve both sides like Pillow's BOX resize: horizontal then vertical pair means, each rounded half up."""
    w, h, *channels = image
    nw, nh = w // 2, h // 2
    out = []
    for c in channels:
        half = [(c[i] + c[i + 1] + 1) >> 1 for i in range(0, w * h, 2)]
        out.append([(half[y * 2 * nw + x] + half[(y * 2 + 1) * nw + x] + 1) >> 1
                    for y in range(nh) for x in range(nw)])
    return (nw, nh, *out)


def _mip(image: tuple) -> tuple:
    """2x2 box mean, rounded half up, for the mip chain."""
    w, h, *channels = image
    nw, nh = w // 2, h // 2
    out = []
    for c in channels:
        out.append([(c[(2 * y) * w + 2 * x] + c[(2 * y) * w + 2 * x + 1] + c[(2 * y + 1) * w + 2 * x]
                     + c[(2 * y + 1) * w + 2 * x + 1] + 2) >> 2 for y in range(nh) for x in range(nw)])
    return (nw, nh, *out)


def _q5(v: float) -> int:
    v = int(v * 31.0 / 255.0 + 0.5)
    return 0 if v < 0 else 31 if v > 31 else v


def _q6(v: float) -> int:
    v = int(v * 63.0 / 255.0 + 0.5)
    return 0 if v < 0 else 63 if v > 63 else v


def _pack(c) -> int:
    return (_q5(c[0]) << 11) | (_q6(c[1]) << 5) | _q5(c[2])


def _fit(px: list, c0: int, c1: int) -> tuple[int, list[int]]:
    """Squared error and indices of the block for endpoints c0, c1 in four-colour order."""
    if c0 < c1:
        c0, c1 = c1, c0
    if c0 == c1:
        er, eg, eb = _expand(c0)
        err = 0
        for r, g, b in px:
            err += (r - er) * (r - er) + (g - eg) * (g - eg) + (b - eb) * (b - eb)
        return err, [0] * 16
    pal = _palette(c0, c1)
    err, idx = 0, []
    for r, g, b in px:
        best, bi = None, 0
        for k in range(4):
            pr, pg, pb = pal[k]
            d = (r - pr) * (r - pr) + (g - pg) * (g - pg) + (b - pb) * (b - pb)
            if best is None or d < best:
                best, bi = d, k
        err += best
        idx.append(bi)
    return err, idx


_WEIGHT = (1.0, 0.0, 2.0 / 3.0, 1.0 / 3.0)  # share of c0 per four-colour index


def _refine(px: list, idx: list[int]):
    """Least-squares endpoints for fixed indices, or None when the system is singular."""
    aa = bb = ab = 0.0
    ax = [0.0, 0.0, 0.0]
    bx = [0.0, 0.0, 0.0]
    for p, k in zip(px, idx):
        a = _WEIGHT[k]
        b = 1.0 - a
        aa += a * a
        bb += b * b
        ab += a * b
        for c in range(3):
            ax[c] += a * p[c]
            bx[c] += b * p[c]
    det = aa * bb - ab * ab
    if det == 0.0:
        return None
    e0 = tuple((ax[c] * bb - bx[c] * ab) / det for c in range(3))
    e1 = tuple((bx[c] * aa - ax[c] * ab) / det for c in range(3))
    return _pack(e0), _pack(e1)


def _block(px: list) -> bytes:
    first = px[0]
    if all(p == first for p in px):
        c = _pack(first)
        return struct.pack("<HHI", c, c, 0)
    n = 16.0
    mean = [0.0, 0.0, 0.0]
    for p in px:
        for c in range(3):
            mean[c] += p[c]
    mean = [m / n for m in mean]
    cov = [0.0] * 6  # rr rg rb gg gb bb
    for p in px:
        dr, dg, db = p[0] - mean[0], p[1] - mean[1], p[2] - mean[2]
        cov[0] += dr * dr
        cov[1] += dr * dg
        cov[2] += dr * db
        cov[3] += dg * dg
        cov[4] += dg * db
        cov[5] += db * db
    lo = [min(p[c] for p in px) for c in range(3)]
    hi = [max(p[c] for p in px) for c in range(3)]
    # Power iteration from the covariance column of the most varying channel
    # (a start of max - min can be orthogonal to the principal axis when two
    # channels are anti-correlated).
    columns = ((cov[0], cov[1], cov[2]), (cov[1], cov[3], cov[4]), (cov[2], cov[4], cov[5]))
    k = 0
    for c in (1, 2):
        if columns[c][c] > columns[k][k]:
            k = c
    axis = list(columns[k])
    for _ in range(8):
        m = max(abs(axis[0]), abs(axis[1]), abs(axis[2]))
        if m == 0.0:
            break
        x, y, z = (v / m for v in axis)
        axis = [cov[0] * x + cov[1] * y + cov[2] * z,
                cov[1] * x + cov[3] * y + cov[4] * z,
                cov[2] * x + cov[4] * y + cov[5] * z]
    m = max(abs(axis[0]), abs(axis[1]), abs(axis[2]))
    axis = [v / m for v in axis] if m else [1.0, 1.0, 1.0]
    proj = [p[0] * axis[0] + p[1] * axis[1] + p[2] * axis[2] for p in px]
    pmin = pmax = 0
    for i in range(1, 16):
        if proj[i] < proj[pmin]:
            pmin = i
        if proj[i] > proj[pmax]:
            pmax = i
    # Inset bounding box, its corners oriented along the axis (a channel that
    # falls while the axis rises takes its low end at the high endpoint).
    inset = [(hi[c] - lo[c]) / 16.0 for c in range(3)]
    top = [hi[c] - inset[c] if axis[c] >= 0.0 else lo[c] + inset[c] for c in range(3)]
    bottom = [lo[c] + inset[c] if axis[c] >= 0.0 else hi[c] - inset[c] for c in range(3)]
    candidates = [(_pack(px[pmax]), _pack(px[pmin])), (_pack(top), _pack(bottom))]
    best = None
    for c0, c1 in candidates:
        for _ in range(3):
            err, idx = _fit(px, c0, c1)
            if best is None or err < best[0]:
                best = (err, max(c0, c1), min(c0, c1), idx)
            if c0 == c1:
                break
            hi_c, lo_c = max(c0, c1), min(c0, c1)
            refined = _refine(px, idx)
            if refined is None or (max(refined), min(refined)) == (hi_c, lo_c):
                break
            c0, c1 = refined
    _, c0, c1, idx = best
    bits = 0
    for i in range(15, -1, -1):
        bits = (bits << 2) | idx[i]
    return struct.pack("<HHI", c0, c1, bits)


def encode_blocks(image: tuple) -> bytes:
    w, h, r, g, b = image
    if w % 4 or h % 4:
        raise ValueError("DXT1 levels must be multiples of 4")
    out = []
    for by in range(0, h, 4):
        for bx in range(0, w, 4):
            px = []
            for y in range(4):
                row = (by + y) * w + bx
                for x in range(4):
                    px.append((r[row + x], g[row + x], b[row + x]))
            out.append(_block(px))
    return b"".join(out)


def average(image: tuple) -> int:
    """AVGC value (0xAARRGGBB, opaque): per-channel mean of the level, rounded half up."""
    w, h, *channels = image
    n = w * h
    r, g, b = ((sum(c) * 2 + n) // (2 * n) for c in channels)
    return 0xFF000000 | (r << 16) | (g << 8) | b


def write(image: tuple, levels: int) -> bytes:
    """A DXT1 PAA with AVGC and OFFS tags and `levels` box-filtered mip levels."""
    chain = [image]
    while len(chain) < levels:
        if min(chain[-1][0], chain[-1][1]) < 8:
            raise ValueError(f"Cannot build {levels} DXT1 levels from {image[0]}x{image[1]}")
        chain.append(_mip(chain[-1]))
    encoded = [encode_blocks(level) for level in chain]
    top = decode_blocks(image[0], image[1], encoded[0])
    head_size = 2 + 16 + 12 + 64 + 2
    offsets, pos = [], head_size
    for level, blocks in zip(chain, encoded):
        offsets.append(pos)
        pos += 7 + len(blocks)
    if len(offsets) > 16:
        raise ValueError("Too many PAA levels")
    parts = [struct.pack("<H", DXT1), TAGG + AVGC + struct.pack("<iI", 4, average(top)),
             TAGG + OFFS + struct.pack("<i", 64) + struct.pack("<16I", *(offsets + [0] * (16 - len(offsets)))),
             b"\0\0"]
    for level, blocks in zip(chain, encoded):
        parts.append(struct.pack("<HH", level[0], level[1]) + len(blocks).to_bytes(3, "little") + blocks)
    parts.append(TERMINATOR)
    result = b"".join(parts)
    if len(parts[1]) + len(parts[2]) + 4 != head_size:
        raise ValueError("PAA header layout changed")
    return result
