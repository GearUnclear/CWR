"""Read-only visual sanity of the generated _dr textures; writes PNGs under this folder."""
import struct
import sys
import zlib
from pathlib import Path

sys.path.insert(0, r"D:\Arma_CWA\.claude\worktrees\lobo-repair-4\arma_CWR\tools\lobo")
from repair import paa, wounds  # noqa: E402
from repair.pbo import Archive  # noqa: E402

PBO = Path(r"D:\Arma_CWA\@LoBo\addons\LoBo_Pilot.pbo")
OUT = Path(r"D:\Arma_CWA\tmp\lobo-wound-review")
arc = Archive(PBO.read_bytes())
src = {k: arc.read(k.split("/", 1)[1]) for k in wounds.SOURCE_MEMBERS}
gen = {k: (OUT / "gen" / k.replace("/", "__")).read_bytes() for k in wounds.OUTPUTS}


def png(path, rows_rgb, w, h):
    raw = b"".join(b"\0" + bytes(r) for r in rows_rgb)

    def chunk(t, d):
        return struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d) & 0xFFFFFFFF)
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
                     + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def strip(images, path, scale=1):
    w, h = images[0][0], images[0][1]
    rows = []
    for y in range(h):
        row = []
        for im in images:
            for x in range(w):
                i = y * w + x
                row += [im[2][i], im[3][i], im[4][i]] * scale
            row += [255, 255, 255] * 4 * scale
        for _ in range(scale):
            rows.append(row)
    png(path, rows, (w * scale + 4 * scale) * len(images), h * scale)


for part in wounds.PARTS:
    shipped = src[wounds._key(wounds.OLIVE_WOUNDED, part + "_d.paa")]
    wounded = paa.decode(shipped)
    w, h = wounded[0], wounded[1]
    base = wounds._fit(paa.decode(src[wounds._key(wounds.OLIVE_BASE[0], wounds.OLIVE_BASE[1][part])]), w, h)
    fill = wounds._fit(paa.decode(src[wounds._key(wounds.OLIVE_FILL[0], wounds.OLIVE_FILL[1][part])]), w, h)
    alpha, layer = wounds.blood_layer(wounded, base, fill)
    nblood = sum(1 for a in alpha if a > 0)
    for folder, names in wounds.SUITS.items():
        if part not in names:
            continue
        key = wounds._key(folder, part + "_dr.paa")
        healthy = wounds._fit(paa.decode(src[wounds._key(folder, names[part])]), w, h)
        comp = wounds.composite(healthy, alpha, layer)
        dec = paa.decode(gen[key])
        n = w * h
        exact_out = sum(1 for i in range(n) if alpha[i] == 0 and any(comp[c][i] != healthy[c][i] for c in (2, 3, 4)))
        outside = [i for i in range(n) if alpha[i] == 0]
        dxt_err = sum(abs(dec[c][i] - healthy[c][i]) for i in outside for c in (2, 3, 4)) / max(1, len(outside)) / 3
        # healthy itself through the same encoder, for a noise baseline
        base_enc = paa.decode(paa.write(healthy, 1))
        base_err = sum(abs(base_enc[c][i] - healthy[c][i]) for i in outside for c in (2, 3, 4)) / max(1, len(outside)) / 3
        core = [i for i in range(n) if alpha[i] > 0.6]
        olive = sum(1 for i in core if comp[3][i] >= comp[2][i] * 0.9 or comp[4][i] >= comp[2][i] * 0.9)
        greenish = sum(1 for i in core if comp[3][i] > comp[2][i])
        mean = [sum(comp[c][i] for i in core) / max(1, len(core)) for c in (2, 3, 4)]
        hmean = [sum(healthy[c][i] for i in outside) / max(1, len(outside)) for c in (2, 3, 4)]
        # Olive-tinted ring: partial alpha pixels where G exceeds R noticeably while healthy G does not
        ring = [i for i in range(n) if 0.05 < alpha[i] <= 0.6]
        ring_green = sum(1 for i in ring if comp[3][i] > comp[2][i] + 4 and healthy[3][i] <= healthy[2][i] + 4)
        print(f"{key}: {w}x{h} blood px {nblood} core {len(core)} | outside-blood mismatches pre-DXT {exact_out} "
              f"| DXT mean abs err outside {dxt_err:.2f} (healthy alone {base_err:.2f}) | core olive-ish {olive} "
              f"green>red {greenish} mean core RGB {[round(v) for v in mean]} healthy mean {[round(v) for v in hmean]} "
              f"| ring {len(ring)} greener-than-suit {ring_green}")
        if key.endswith(("eaf/tex/back_dr.paa", "iaf/tex2/front_dr.paa", "iaf/tex4/armle_dr.paa",
                         "iaf/tex2/legslft_dr.paa", "eaf/tex/legsfr_dr.paa")):
            strip([healthy, wounded, dec], OUT / ("view_" + key.replace("/", "_") + ".png"),
                  scale=2 if w < 128 else 1)
