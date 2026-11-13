r"""Makes the models and textures of the kjbeam addon from nothing (plain Python, no Blender).

  python gen_kjbeam.py <out_dir>

Writes into <out_dir>:
  kjbeam_seg.p3d    one 50 m piece of the beam, along model Z, centred on its origin: a thin
                    opaque white-hot core (a six sided prism) inside three crossed glow planes
                    whose texture fades to nothing at the edges. Every face is fullbright
                    (FACE_NOLIGHT), so it shows at full strength at night.
  kjbeam_glow.p3d   a single fullbright face: the sprite for `drop` billboards (chest flare,
                    impact flash, sparks). The engine wants exactly one face in a billboard.
  kjbeam_null.p3d   a model with nothing to see (one hair-thin face) and the memory point
                    "kjfire": the body of the light, fire and probe classes.
  tex\*.png         the textures; build-kjbeam.ps1 turns each into a PAA.
  kjbeam_gen.json   what was written (sizes, texture formats).

The two textures with soft alpha are ARGB8888, the one alpha format that both the tool
writes correctly and the game loads:
* ARGB4444, ARGB1555, AI88: the PAA that PoseidonTools `image convert` writes is rejected
  by the engine's own loader (Pactext.cpp:478 "Checksum", :910 "LZW Decode error"), in
  the tool and in the game alike.
* DXT5 (and DXT3): the game's loader has no case for them (Pactext.cpp:1354-1359,
  "Texture format"); the face then draws opaque.
"""
import json
import math
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ENGINE_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", "..", ".."))
sys.path.insert(0, os.path.join(ENGINE_ROOT, "apps", "tools", "BlenderAddon", "io_import_p3d"))
import mlod_writer as mw  # noqa: E402  (pure Python, does not import bpy)

ADDON = "kjbeam"
FACE_NOLIGHT = 0x1
FACE_NOSHADOW = 0x10
GLOW_FLAGS = FACE_NOLIGHT | FACE_NOSHADOW

# --- the beam piece ---------------------------------------------------------------------------
SEG_LEN = 50.0          # metres along Z
SEG_OVERLAP = 0.15      # each end reaches this far into its neighbour, so no gap shows
CORE_RADIUS = 0.42      # the opaque white core
HALO_WIDTH = 7.0        # full width of each glow plane
HALO_PLANES = 3


def tex(name):
    return "%s\\%s.paa" % (ADDON, name)


class Lod:
    def __init__(self, resolution):
        self.lod = mw.MlodLod(resolution)
        self.lod.normals.append((0.0, 1.0, 0.0))

    def point(self, p):
        self.lod.points.append(tuple(float(v) for v in p))
        return len(self.lod.points) - 1

    def quad(self, pts, uvs, texture, flags, both=True):
        idx = [self.point(p) for p in pts]
        corners = [(i, 0, u, v) for i, (u, v) in zip(idx, uvs)]
        self.lod.faces.append(mw.MlodFace(mw.file_order(corners), texture, flags))
        if both:
            self.lod.faces.append(mw.MlodFace(list(corners), texture, flags))

    def tri(self, pts, texture, flags):
        idx = [self.point(p) for p in pts]
        corners = [(i, 0, 0.5, 0.5) for i in idx]
        self.lod.faces.append(mw.MlodFace(mw.file_order(corners), texture, flags))


def build_seg(path, core_radius, halo_width):
    z0, z1 = -SEG_LEN / 2 - SEG_OVERLAP, SEG_LEN / 2 + SEG_OVERLAP
    v = Lod(1.0)
    # Core: six side quads, both windings (no need to know which one the engine calls front).
    ring = [(core_radius * math.cos(math.radians(60 * i)), core_radius * math.sin(math.radians(60 * i))) for i in range(6)]
    for i in range(6):
        (xa, ya), (xb, yb) = ring[i], ring[(i + 1) % 6]
        v.quad([(xa, ya, z0), (xb, yb, z0), (xb, yb, z1), (xa, ya, z1)],
               [(0.25, 0.25), (0.75, 0.25), (0.75, 0.75), (0.25, 0.75)], tex("kjbeam_core"), GLOW_FLAGS)
    # Halo: crossed planes through the axis, U across the width, V along the length.
    h = halo_width / 2
    for i in range(HALO_PLANES):
        a = math.radians(180.0 * i / HALO_PLANES)
        cx, cy = math.cos(a) * h, math.sin(a) * h
        v.quad([(-cx, -cy, z0), (cx, cy, z0), (cx, cy, z1), (-cx, -cy, z1)],
               [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)], tex("kjbeam_halo"), GLOW_FLAGS)
    v.lod.properties.append(("lodnoshadow", "1"))
    return mw.write_mlod_file(path, [v.lod])


def build_glow(path):
    v = Lod(1.0)
    v.quad([(-0.5, -0.5, 0), (0.5, -0.5, 0), (0.5, 0.5, 0), (-0.5, 0.5, 0)],
           [(0, 1), (1, 1), (1, 0), (0, 0)], tex("kjbeam_glow"), GLOW_FLAGS, both=False)
    v.lod.properties.append(("lodnoshadow", "1"))
    return mw.write_mlod_file(path, [v.lod])


def build_null(path):
    v = Lod(1.0)
    v.tri([(-0.001, 0, 0), (0.001, 0, 0), (0, 0.001, 0)], tex("kjbeam_core"), GLOW_FLAGS)
    v.lod.properties.append(("lodnoshadow", "1"))
    mem = mw.MlodLod(mw.MEMORY_SPEC)
    mem.points.append((0.0, 0.0, 0.0))
    sel = mem.get_or_add_selection("kjfire")
    sel.points[0] = 1.0
    return mw.write_mlod_file(path, [v.lod, mem])


def png_core(path):
    Image.new("RGB", (8, 8), (255, 250, 236)).save(path)


def png_halo(path):
    """Across the width: a hot narrow centre, then a long soft orange skirt."""
    w, hgt = 128, 16
    img = Image.new("RGBA", (w, hgt))
    px = img.load()
    for x in range(w):
        d = abs((x + 0.5) / w - 0.5) * 2.0          # 0 at the axis, 1 at the edge
        a = 0.95 * math.exp(-((d / 0.10) ** 2)) + 0.55 * math.exp(-d / 0.22) * (1.0 - d) ** 1.2
        a = max(0.0, min(1.0, a))
        heat = math.exp(-d / 0.18)                   # white at the axis, orange outside
        r, g, b = 255, int(140 + 110 * heat), int(40 + 190 * heat)
        for y in range(hgt):
            px[x, y] = (r, g, b, int(round(a * 255)))
    img.save(path)


def png_glow(path):
    """Radial falloff, white: the particle colour tints it."""
    n = 64
    img = Image.new("RGBA", (n, n))
    px = img.load()
    for y in range(n):
        for x in range(n):
            d = math.hypot((x + 0.5) / n - 0.5, (y + 0.5) / n - 0.5) * 2.0
            a = 0.0 if d >= 1.0 else (0.9 * math.exp(-((d / 0.22) ** 2)) + 0.5 * (1.0 - d) ** 2.2)
            px[x, y] = (255, 255, 255, int(round(max(0.0, min(1.0, a)) * 255)))
    img.save(path)


def main():
    if len(sys.argv) < 2:
        raise SystemExit("usage: python gen_kjbeam.py <out_dir>")
    out = os.path.abspath(sys.argv[1])
    os.makedirs(os.path.join(out, "tex"), exist_ok=True)
    summary = {"models": {}, "textures": []}
    summary["models"]["kjbeam_seg.p3d"] = build_seg(os.path.join(out, "kjbeam_seg.p3d"), CORE_RADIUS, HALO_WIDTH)
    summary["models"]["kjbeam_glow.p3d"] = build_glow(os.path.join(out, "kjbeam_glow.p3d"))
    summary["models"]["kjbeam_null.p3d"] = build_null(os.path.join(out, "kjbeam_null.p3d"))
    for name, fn, fmt in (("kjbeam_core", png_core, "DXT1"), ("kjbeam_halo", png_halo, "ARGB8888"),
                          ("kjbeam_glow", png_glow, "ARGB8888")):
        p = os.path.join(out, "tex", name + ".png")
        fn(p)
        summary["textures"].append({"png": p, "paa": name + ".paa", "format": fmt})
    summary["seg_len_m"] = SEG_LEN
    summary["core_radius_m"] = CORE_RADIUS
    summary["halo_width_m"] = HALO_WIDTH
    with open(os.path.join(out, "kjbeam_gen.json"), "w") as f:
        json.dump(summary, f, indent=1)
    print("[kjbeam] wrote", ", ".join("%s (%d bytes)" % kv for kv in summary["models"].items()))


if __name__ == "__main__":
    main()
