"""Per-mip alpha coverage of OFP/CWA DXT1 PAC/PAA textures.

    python paa_coverage.py <file.pac> [...]

Decodes every DXT1 mip level the file carries (1-bit punch-through alpha) and
prints its opaque fraction, and what the GPU's bilinear filter does to it at
the engine's cutout threshold 0xc0 (0.753): the fraction of bilinear samples
(at texel corners, i.e. half-texel offsets) whose filtered alpha passes.
"""
import struct
import sys

import numpy as np


def rgb565(c):
    r = (c >> 11) & 31
    g = (c >> 5) & 63
    b = c & 31
    return (r * 255 // 31, g * 255 // 63, b * 255 // 31)


def decode_dxt1_alpha(data, w, h):
    bw, bh = max(1, (w + 3) // 4), max(1, (h + 3) // 4)
    out = np.zeros((bh * 4, bw * 4), dtype=np.float32)
    off = 0
    for by in range(bh):
        for bx in range(bw):
            c0, c1, bits = struct.unpack_from("<HHI", data, off)
            off += 8
            for i in range(16):
                idx = (bits >> (2 * i)) & 3
                a = 1.0
                if c0 <= c1 and idx == 3:
                    a = 0.0
                out[by * 4 + i // 4, bx * 4 + i % 4] = a
    return out[:h, :w]


def read_mips(path):
    b = open(path, "rb").read()
    fmt = struct.unpack_from("<H", b, 0)[0]
    off = 2
    while b[off:off + 4] == b"GGAT":
        ln = struct.unpack_from("<I", b, off + 8)[0]
        off += 12 + ln
    npal = struct.unpack_from("<H", b, off)[0]
    off += 2 + 3 * npal
    mips = []
    while off + 4 <= len(b):
        w, h = struct.unpack_from("<HH", b, off)
        off += 4
        if w == 0 or h == 0:
            break
        size = b[off] | (b[off + 1] << 8) | (b[off + 2] << 16)
        off += 3
        mips.append((w & 0x7FFF, h, b[off:off + size], bool(w & 0x8000)))
        off += size
    return fmt, mips


def bilinear_pass_fraction(a, ref):
    # filtered alpha at texel corners (offset half a texel): mean of 2x2
    p = np.pad(a, ((0, 1), (0, 1)), mode="wrap")
    f = 0.25 * (p[:-1, :-1] + p[1:, :-1] + p[:-1, 1:] + p[1:, 1:])
    return float((f >= ref).mean()), float(f.mean())


def main():
    ref = 0xC0 / 255.0
    for path in sys.argv[1:]:
        fmt, mips = read_mips(path)
        print(f"{path}  format=0x{fmt:04x}  mips={len(mips)}")
        if fmt != 0xFF01:
            print("  not DXT1, skipped")
            continue
        for lvl, (w, h, data, lzo) in enumerate(mips):
            if lzo:
                print(f"  L{lvl} {w}x{h} LZO-compressed, skipped")
                continue
            a = decode_dxt1_alpha(data, w, h)
            cov = float(a.mean())
            passf, meanf = bilinear_pass_fraction(a, ref)
            print(f"  L{lvl} {w:4d}x{h:<4d} texel coverage {cov:5.3f}   bilinear(corner) >= 0.753: {passf:5.3f}   >= 0.5: {bilinear_pass_fraction(a, 0.5)[0]:5.3f}")


if __name__ == "__main__":
    main()
