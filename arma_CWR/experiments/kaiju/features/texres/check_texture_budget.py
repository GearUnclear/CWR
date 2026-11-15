r"""Checks an exported model against TEXTURE-BUDGET.md. Plain Python, no Blender, no game.

  python check_texture_budget.py <model.p3d> <folder with its .paa files> [--lod 0] [--json out.json]

It reads the MLOD P3D the exporter wrote and the header of every PAA the first visual LOD
names (the folder is searched by file name, so point it at the build's stage or tex folder),
then prints, by height band above the ground:

  texels per metre   area-weighted median, and the 10th percentile (the blurriest tenth);
                     per face it is sqrt(texture pixels covered / square metres)
  against the target of TEXTURE-BUDGET.md for that band

and flags what the standard forbids: a sheet that is not DXT1, not a power of two or over
4096; more than 8 sheets on the LOD; faces of one texture scattered through the file (every
run of faces is a draw call, see NOTES.md); stretched faces (texels more than 1.6 times
denser one way than the other); a pair of UV corners that share a point and a normal and
lie closer than 0.005 without being equal (the engine welds them, Vertex.cpp:151).

Exit code 0 when nothing is flagged, 1 otherwise.
"""
import json
import math
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ENGINE_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", "..", ".."))
sys.path.insert(0, os.path.join(ENGINE_ROOT, "apps", "tools", "BlenderAddon", "io_import_p3d"))
import mlod_writer as mw  # noqa: E402

# Height of the face centre above the ground (m) up to -> target texels per metre.
# Keep in step with TEXTURE-BUDGET.md.
BANDS = [(3.0, 192), (12.0, 128), (24.0, 96), (1000.0, 64)]
UP_FACING = 32          # faces whose normal points up more than 60 degrees: never seen from the ground
TOLERANCE = 0.75        # a band passes when its median reaches this share of the target
MAX_SHEETS = 8
FACE_NOLIGHT = 0x1
MAGIC = {0xFF01: "DXT1", 0xFF02: "DXT2", 0xFF03: "DXT3", 0xFF04: "DXT4", 0xFF05: "DXT5", 0x4444: "ARGB4444",
         0x1555: "ARGB1555", 0x8888: "ARGB8888", 0x8080: "AI88"}


def paa_info(path):
    with open(path, "rb") as fh:
        data = fh.read(4096)
    magic = struct.unpack_from("<H", data, 0)[0]
    pos = 2
    while data[pos:pos + 4] == b"GGAT":
        size = struct.unpack_from("<I", data, pos + 8)[0]
        pos += 12 + size
        if pos + 16 > len(data):
            with open(path, "rb") as fh:
                data = fh.read()
    pos += 2
    w, h = struct.unpack_from("<HH", data, pos)
    return MAGIC.get(magic, "0x%04x" % magic), w, h


def find_textures(folder):
    found = {}
    for root, _d, files in os.walk(folder):
        for f in files:
            if f.lower().endswith((".paa", ".pac", ".png", ".tga", ".jpg")):
                found.setdefault(f.lower(), os.path.join(root, f))
    return found


def sub(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def length(a):
    return math.sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2])


def weighted_quantile(pairs, q):
    """pairs: [(value, weight)]."""
    pairs = sorted(pairs)
    total = sum(w for _v, w in pairs)
    if total <= 0:
        return 0.0
    acc = 0.0
    for v, w in pairs:
        acc += w
        if acc >= q * total:
            return v
    return pairs[-1][0]


def main():
    argv = sys.argv[1:]
    if len(argv) < 2:
        raise SystemExit(__doc__)
    p3d, folder = argv[0], argv[1]
    lod_index = 0
    out_json = None
    i = 2
    while i < len(argv):
        if argv[i] == "--lod":
            lod_index = int(argv[i + 1])
        elif argv[i] == "--json":
            out_json = argv[i + 1]
        i += 2
    with open(p3d, "rb") as fh:
        _ver, lods, _warn = mw.read_mlod(fh.read(), strict=False)
    visual = [l for l in lods if l.resolution < 900]
    lod = visual[lod_index]
    files = find_textures(folder)
    flags = []
    sheets = {}
    for f in lod.faces:
        if f.texture and f.texture not in sheets:
            key = f.texture.replace("/", "\\").split("\\")[-1].lower()
            path = files.get(key)
            if not path:
                # not converted yet: take the size from the source picture of the same name
                for ext in (".png", ".tga"):
                    src = files.get(os.path.splitext(key)[0] + ext)
                    if src:
                        from PIL import Image
                        with Image.open(src) as im:
                            sheets[f.texture] = ("DXT1", im.size[0], im.size[1])
                        if im.size[0] > 4096 or im.size[1] > 4096 or (im.size[0] & (im.size[0] - 1)) or (im.size[1] & (im.size[1] - 1)):
                            flags.append("texture %s is %d x %d: sides must be powers of two, at most 4096" % (f.texture, im.size[0], im.size[1]))
                        break
                if f.texture in sheets:
                    continue
            if not path:
                flags.append("texture %s: no file found under %s" % (f.texture, folder))
                sheets[f.texture] = None
                continue
            if not key.endswith(".paa"):
                flags.append("texture %s: not a PAA (a loose picture is uploaded as 16 bit and held decoded in memory)" % f.texture)
                sheets[f.texture] = None
                continue
            fmt, w, h = paa_info(path)
            sheets[f.texture] = (fmt, w, h)
            if fmt != "DXT1":
                flags.append("texture %s is %s: use DXT1 (anything with soft alpha goes to the blended pass)" % (f.texture, fmt))
            if w > 4096 or h > 4096 or (w & (w - 1)) or (h & (h - 1)):
                flags.append("texture %s is %d x %d: sides must be powers of two, at most 4096" % (f.texture, w, h))
    ground = min(p[1] for p in lod.points)
    bands = [[] for _ in BANDS]
    up_faces = []
    stretched = 0.0
    total_area = 0.0
    per_sheet = {}
    runs = 0
    prev = None
    welds = 0
    seen = {}
    for f in lod.faces:
        if f.texture != prev:
            runs += 1
            prev = f.texture
        info = sheets.get(f.texture)
        if not info:
            continue
        _fmt, tw, th = info
        pts = [lod.points[c[0]] for c in f.corners]
        uvs = [(c[2] * tw, c[3] * th) for c in f.corners]
        tris = [(0, 1, 2)] if len(pts) == 3 else [(0, 1, 2), (0, 2, 3)]
        area = 0.0
        tex_area = 0.0
        nsum = (0.0, 0.0, 0.0)
        for a, b, c in tris:
            n = cross(sub(pts[b], pts[a]), sub(pts[c], pts[a]))
            area += 0.5 * length(n)
            nsum = (nsum[0] + n[0], nsum[1] + n[1], nsum[2] + n[2])
            tex_area += 0.5 * abs((uvs[b][0] - uvs[a][0]) * (uvs[c][1] - uvs[a][1]) - (uvs[c][0] - uvs[a][0]) * (uvs[b][1] - uvs[a][1]))
        if area < 1e-6:
            continue
        tpm = math.sqrt(tex_area / area)
        total_area += area
        per_sheet.setdefault(f.texture, [0.0, []])
        per_sheet[f.texture][0] += area
        per_sheet[f.texture][1].append((tpm, area))
        # stretch: texels per metre along the first two edges
        e1, e2 = sub(pts[1], pts[0]), sub(pts[-1], pts[0])
        if length(e1) > 1e-4 and length(e2) > 1e-4:
            d1 = math.hypot(uvs[1][0] - uvs[0][0], uvs[1][1] - uvs[0][1]) / length(e1)
            d2 = math.hypot(uvs[-1][0] - uvs[0][0], uvs[-1][1] - uvs[0][1]) / length(e2)
            if min(d1, d2) > 0 and max(d1, d2) / min(d1, d2) > 1.6:
                stretched += area
        # welded seams: the same point and normal with nearly the same UV
        for c in f.corners:
            key = (c[0], c[1], f.texture)
            for (u, v) in seen.get(key, ()):
                du, dv = abs(u - c[2]), abs(v - c[3])
                if (du > 1e-6 or dv > 1e-6) and du <= 0.005 and dv <= 0.005:
                    welds += 1
            seen.setdefault(key, set()).add((c[2], c[3]))
        if f.flags & FACE_NOLIGHT:
            continue                    # glow faces are judged by eye, not by band
        centre_h = sum(p[1] for p in pts) / len(pts) - ground
        # the stored corner normals point outward (game space, Y up)
        up = sum(lod.normals[c[1]][1] for c in f.corners) / len(f.corners)
        if up > 0.87:
            up_faces.append((tpm, area))
            continue
        for bi, (limit, _target) in enumerate(BANDS):
            if centre_h <= limit:
                bands[bi].append((tpm, area))
                break
    print("%s: LOD %s, %d faces, %.0f square metres, %d textures in %d runs of faces"
          % (os.path.basename(p3d), mw.lod_name(lod.resolution), len(lod.faces), total_area, len(sheets), runs))
    for tex in sorted(sheets):
        info = sheets[tex]
        if info and tex in per_sheet:
            area, pairs = per_sheet[tex]
            print("  %-28s %-5s %4d x %-4d %7.0f m2  median %5.0f texels/m" % (tex, info[0], info[1], info[2], area, weighted_quantile(pairs, 0.5)))
    result = {"bands": [], "flags": flags}
    low = 0.0
    for bi, (limit, target) in enumerate(BANDS):
        pairs = bands[bi]
        if not pairs:
            continue
        med, p10 = weighted_quantile(pairs, 0.5), weighted_quantile(pairs, 0.1)
        ok = med >= TOLERANCE * target
        print("  height %5.0f to %-5s m: median %5.0f, blurriest tenth %5.0f texels/m, target %3d  %s"
              % (low, "%.0f" % limit if limit < 999 else "top", med, p10, target, "ok" if ok else "TOO LOW"))
        result["bands"].append({"from": low, "to": limit, "median": med, "p10": p10, "target": target, "ok": ok})
        if not ok:
            flags.append("height band up to %s m: median %.0f texels/m, target %d" % (limit, med, target))
        if med > 2.0 * target:
            flags.append("height band up to %s m: median %.0f texels/m is over twice the target %d (wasted, and it shimmers sooner at range)"
                         % (limit, med, target))
        low = limit
    if up_faces:
        print("  faces that look up:     median %5.0f texels/m (guide %d)" % (weighted_quantile(up_faces, 0.5), UP_FACING))
    n_sheets = len([s for s in sheets.values() if s])
    if n_sheets > MAX_SHEETS:
        flags.append("%d sheets on one LOD: the standard allows %d" % (n_sheets, MAX_SHEETS))
    if runs > 2 * max(1, len(sheets)):
        flags.append("%d runs of faces for %d textures: sort the faces by texture on export (each run is a draw call)" % (runs, len(sheets)))
    if total_area > 0 and stretched / total_area > 0.05:
        flags.append("%.0f%% of the surface is stretched more than 1.6 to 1" % (100.0 * stretched / total_area))
    if welds:
        flags.append("%d corner pairs share a point and normal and lie within 0.005 UV of each other: the engine welds them" % welds)
    for fl in flags:
        print("  FLAG: " + fl)
    if not flags:
        print("  nothing flagged")
    if out_json:
        with open(out_json, "w", encoding="utf-8") as fh:
            json.dump(result, fh, indent=1)
    return 1 if flags else 0


if __name__ == "__main__":
    sys.exit(main())
