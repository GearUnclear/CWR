r"""Measures the chart frames of a texres_charts run.

  python analyze_texres.py <tri output dir> <report dir> [--gen <build\gen>] [--left 1.3333] [--top 0.75]

For every chart frame it projects the known chart faces into the picture (the camera
position, target and lens come from the TEXSHOT lines of game_stdout.log; the faces from
the proxy model and gen_texres.py's own layout code) and writes:

  overlay_<shot>.png    the frame with every chart face outlined (proves the projection)
  patches_<shot>_<part>.png   the six density patches of one body part, cut out and
                        enlarged without smoothing, low to high, each labelled with its
                        texels per metre and the screen pixels per metre at that spot
  measurements.json / measurements.txt
        per patch: screen pixels per metre across and along (the smaller one is the
        grazing direction). Which density is enough is judged by eye on the patch
        pictures: two automatic measures (Laplacian energy, line pair contrast) were
        tried and dropped, the breathing body moves the patches by more than a group
        of the ruler;
        per mip tell face: the level the GPU sampled (decoded from the colour), the
        texels per metre that level stands for, and the level that the geometry predicts
        without and with 16x anisotropic filtering.

The camera model is the engine's own (World/Scene/Camera/Camera.cpp:27-31): the half
width of the view at distance 1 is lens * leftFOV and the half height lens * topFOV.
"""
import colorsys
import json
import math
import os
import re
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import gen_texres as G  # noqa: E402

ORIGIN = (5055.0, 16.335, 3870.0)      # the creature's feet: world x, metres above sea level, world z
PART_NAME = {"pprsty": "toes", "pholen": "shin", "pstehno": "thigh", "pruka": "hand", "pzadek": "pelvis_underside",
             "bricho": "belly", "zebra": "ribs", "hrudnik": "chest", "hlava": "head",
             "lprsty": "toes", "lholen": "shin", "lstehno": "thigh", "lruka": "hand", "lzadek": "pelvis_underside"}


def norm(v):
    n = math.sqrt(sum(c * c for c in v))
    return tuple(c / n for c in v)


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


class Cam:
    def __init__(self, pos, target, lens, w, h, left, top):
        self.pos = pos
        self.f = norm(tuple(t - p for t, p in zip(target, pos)))
        self.r = norm(cross((0.0, 1.0, 0.0), self.f))
        self.u = cross(self.f, self.r)
        self.cl = lens * left
        self.ct = lens * top
        self.w, self.h = w, h

    def project(self, p):
        d = tuple(a - b for a, b in zip(p, self.pos))
        z = dot(d, self.f)
        if z <= 0.05:
            return None
        x = dot(d, self.r) / z / self.cl
        y = dot(d, self.u) / z / self.ct
        return ((0.5 + 0.5 * x) * self.w, (0.5 - 0.5 * y) * self.h, z)


def world(p):
    return tuple(o + c for o, c in zip(ORIGIN, p))


def parse_log(path):
    shots = {}
    extra = []
    rx = re.compile(r'TEXSHOT (\S+) cam \[x,asl,z\] to \[x,asl,z\] \[([^\]]+)\] lens ([0-9.]+) window (.*)"')
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        for line in fh:
            m = rx.search(line)
            if m:
                v = [float(x) for x in m.group(2).split(",")]
                shots[m.group(1)] = {"pos": tuple(v[0:3]), "target": tuple(v[3:6]), "lens": float(m.group(3)), "window": m.group(4)}
            elif "TEXRES" in line:
                extra.append(line.strip())
    return shots, extra


def chart_faces(base_p3d):
    """Every chart face of the CHART model: patches and tell faces, in model (game) space."""
    lods = G.load_base(base_p3d)
    boxes, _extra = G.boxes_of(lods[0])
    faces = []
    pair_patch = {p[0]: p for p in G.CHART_PAIRS}
    pair_tell = {p[1]: p for p in G.CHART_PAIRS}
    centre = {c[0]: c for c in G.CHART_CENTRE}
    seen_head = False

    def rect(box, side, s0, s1, t0, t1, kind, density=None):
        origin, ua, va, w, h = G.face_frame(side, box)

        def pt(s, t):
            return tuple(origin[k] + ua[k] * s * w + va[k] * t * h for k in range(3))
        faces.append({"bone": box.bone, "part": PART_NAME.get(box.bone, box.bone), "side": side, "kind": kind, "density": density,
                      "corners": [pt(s0, t0), pt(s1, t0), pt(s1, t1), pt(s0, t1)], "ua": ua, "va": va,
                      "size": ((s1 - s0) * w, (t1 - t0) * h), "normal": G.SIDE_NORMAL[side]})

    def patches(box, side, sa, sb, cols, rows):
        k = 0
        for r in range(rows):
            for c in range(cols):
                rect(box, side, sa + (sb - sa) * c / cols, sa + (sb - sa) * (c + 1) / cols, r / rows, (r + 1) / rows,
                     "patch", G.DENSITIES[k])
                k += 1

    for box in boxes:
        if box.bone == "hlava":
            if seen_head:
                continue
            seen_head = True
        for side in G.SIDES:
            if box.bone in pair_patch and pair_patch[box.bone][2] == side:
                _p, _t, _s, cols, rows, (pa, pb), _tr = pair_patch[box.bone]
                patches(box, side, pa, pb, cols, rows)
            elif box.bone in pair_tell and pair_tell[box.bone][2] == side:
                ta, tb = pair_tell[box.bone][6]
                rect(box, side, ta, tb, 0.0, 1.0, "tell")
            elif box.bone in centre and centre[box.bone][1] == side:
                _b, _s, cols, rows = centre[box.bone]
                s_mid = 0.5
                if box.bone == "hrudnik":
                    s_mid = (-3.5 - box.lo[0]) / (box.hi[0] - box.lo[0])
                patches(box, side, 0.0, s_mid, cols, rows)
                rect(box, side, 1.0 - s_mid, 1.0, 0.0, 1.0, "tell")
    return faces


def screen_density(cam, face):
    """Screen pixels per metre along the major and minor directions at the face centre."""
    c = tuple(sum(p[k] for p in face["corners"]) / 4.0 for k in range(3))
    p0 = cam.project(world(c))
    if p0 is None:
        return None
    e = 0.05
    pu = cam.project(world(tuple(c[k] + face["ua"][k] * e for k in range(3))))
    pv = cam.project(world(tuple(c[k] + face["va"][k] * e for k in range(3))))
    if pu is None or pv is None:
        return None
    j = np.array([[(pu[0] - p0[0]) / e, (pv[0] - p0[0]) / e], [(pu[1] - p0[1]) / e, (pv[1] - p0[1]) / e]])
    s = np.linalg.svd(j, compute_uv=False)
    return {"major": float(s[0]), "minor": float(s[1]), "distance": float(p0[2]),
            "u": float(np.hypot(j[0, 0], j[1, 0])), "v": float(np.hypot(j[0, 1], j[1, 1])), "centre": (p0[0], p0[1])}


def facing(cam, face):
    c = tuple(sum(p[k] for p in face["corners"]) / 4.0 for k in range(3))
    view = tuple(a - b for a, b in zip(world(c), cam.pos))
    return dot(view, face["normal"]) < 0


def poly_mask(size, pts, shrink=0.0):
    cx = sum(p[0] for p in pts) / len(pts)
    cy = sum(p[1] for p in pts) / len(pts)
    q = [(cx + (p[0] - cx) * (1 - shrink), cy + (p[1] - cy) * (1 - shrink)) for p in pts]
    im = Image.new("L", size, 0)
    ImageDraw.Draw(im).polygon(q, fill=255)
    return np.asarray(im) > 0


def decode_level(rgb):
    """Mip tell colour -> level (float). Levels 0..5 run round the hue circle, 6 is white."""
    r, g, b = (float(c) / 255.0 for c in rgb)
    h, s, v = colorsys.rgb_to_hsv(r, g, b)
    if v < 0.15:
        return float("nan")            # black: a level past the seventh
    # A blend of two neighbouring levels is a straight line between two pure colours, and
    # along such a line the hue runs linearly: hue / 60 degrees IS the fractional level.
    level = h * 6.0
    if level > 5.5:                    # red seen as hue 0.95: wrap
        level -= 6.0
    if s < 0.2:
        return 6.0                     # white: the seventh level
    if level >= 4.9:
        return 5.0 + (1.0 - s)         # magenta fading to white: saturation carries the fraction
    return max(0.0, level)


def font(px):
    try:
        return ImageFont.truetype("arial.ttf", px)
    except OSError:
        return ImageFont.load_default()


def detail_energy(crop):
    """Variance of a Laplacian of the brightness, after removing the mean level."""
    g = np.asarray(crop.convert("L"), dtype=np.float32)
    if g.shape[0] < 5 or g.shape[1] < 5:
        return 0.0
    g = g / max(1.0, float(g.mean()))
    lap = 4 * g[1:-1, 1:-1] - g[:-2, 1:-1] - g[2:, 1:-1] - g[1:-1, :-2] - g[1:-1, 2:]
    return float(lap.var())


def main():
    argv = sys.argv[1:]
    if len(argv) < 2:
        raise SystemExit(__doc__)
    run_dir, out_dir = os.path.abspath(argv[0]), os.path.abspath(argv[1])
    gen = r"D:\Arma_CWA_worktrees\kaiju-notes\features\texres\build\gen"
    left, top = 4.0 / 3.0, 0.75
    i = 2
    while i < len(argv):
        if argv[i] == "--gen":
            gen = argv[i + 1]
        elif argv[i] == "--left":
            left = float(argv[i + 1])
        elif argv[i] == "--top":
            top = float(argv[i + 1])
        i += 2
    os.makedirs(out_dir, exist_ok=True)
    log = None
    frames = {}
    for root, _d, files in os.walk(run_dir):
        for f in files:
            if f == "game_stdout.log":
                log = os.path.join(root, f)
            m = re.match(r"\d+_(.+)\.png$", f)
            if m:
                frames[m.group(1)] = os.path.join(root, f)
    if not log:
        raise SystemExit("no game_stdout.log under %s" % run_dir)
    shots, extra = parse_log(log)
    faces = chart_faces(os.path.join(r"D:\Arma_CWA_worktrees\kaiju-notes\features\texres\base\uddedushka", "dedushka.p3d"))
    report = {"shots": {}, "log": extra}
    lines = []
    for name in sorted(shots):
        if not (name.startswith("c") or name.startswith("z")) or name not in frames:
            continue
        sh = shots[name]
        img = Image.open(frames[name]).convert("RGB")
        w, h = img.size
        cam = Cam(sh["pos"], sh["target"], sh["lens"], w, h, left, top)
        arr = np.asarray(img)
        over = img.copy()
        dr = ImageDraw.Draw(over)
        rows = {}
        tells = []
        for face in faces:
            if not facing(cam, face):
                continue
            pts = [cam.project(world(p)) for p in face["corners"]]
            if any(p is None for p in pts):
                continue
            xy = [(p[0], p[1]) for p in pts]
            sd = screen_density(cam, face)
            if sd is None:
                continue
            dr.polygon(xy, outline=(255, 255, 255) if face["kind"] == "patch" else (0, 0, 0))
            if face["kind"] == "tell":
                # sample the inner part of the face on a grid; keep what falls inside the frame
                c0, c1, c2, c3 = face["corners"]
                px = []
                for gi in range(17):
                    for gj in range(17):
                        s_, t_ = 0.15 + 0.7 * gi / 16.0, 0.15 + 0.7 * gj / 16.0
                        pm = tuple(c0[k] + (c1[k] - c0[k]) * s_ + (c3[k] - c0[k]) * t_ for k in range(3))
                        q = cam.project(world(pm))
                        if q is not None and 1 <= q[0] < w - 1 and 1 <= q[1] < h - 1:
                            px.append(arr[int(q[1]), int(q[0])])
                if len(px) < 30:
                    continue
                px = np.array(px)
                med = np.median(px, axis=0)
                lv = decode_level(med)
                d_tex = G.TELL_TPM
                iso = math.log2(d_tex / sd["minor"]) if sd["minor"] > 0 else float("nan")
                aniso = math.log2(d_tex / min(sd["major"], 16.0 * sd["minor"]))
                tells.append({"part": face["part"], "bone": face["bone"], "side": face["side"], "rgb": [int(c) for c in med],
                              "level": lv, "texels_per_metre_shown": G.TELL_TPM / (2.0 ** lv) if lv == lv else None,
                              "px_per_m_major": sd["major"], "px_per_m_minor": sd["minor"], "distance_m": sd["distance"],
                              "level_predicted_isotropic": max(0.0, iso), "level_predicted_aniso16": max(0.0, aniso),
                              "pixels": int(len(px))})
            else:
                x0, y0 = min(p[0] for p in xy), min(p[1] for p in xy)
                x1, y1 = max(p[0] for p in xy), max(p[1] for p in xy)
                full = max(1.0, (x1 - x0) * (y1 - y0))
                x0, y0, x1, y1 = max(0.0, x0), max(0.0, y0), min(float(w), x1), min(float(h), y1)
                if x1 <= x0 or y1 <= y0 or (x1 - x0) * (y1 - y0) < 0.6 * full:
                    continue
                rows.setdefault(face["part"], []).append({"density": face["density"], "box": (x0, y0, x1, y1), "xy": xy,
                                                          "sd": sd, "size": face["size"]})
        over.save(os.path.join(out_dir, "overlay_%s.png" % name))
        entry = {"lens": sh["lens"], "window": [w, h], "tells": tells, "patches": {}}
        lines.append("== %s  (lens %.3f, %dx%d)" % (name, sh["lens"], w, h))
        for t in tells:
            lines.append("  tell %-17s dist %6.1f m  screen %6.1f / %6.1f px per m (major/minor)  level shown %s -> %s texels/m"
                         "   predicted %.1f isotropic, %.1f with 16x aniso   rgb %s"
                         % (t["part"], t["distance_m"], t["px_per_m_major"], t["px_per_m_minor"],
                            "%.2f" % t["level"] if t["level"] == t["level"] else "black",
                            "%.0f" % t["texels_per_metre_shown"] if t["texels_per_metre_shown"] else "?",
                            t["level_predicted_isotropic"], t["level_predicted_aniso16"], t["rgb"]))
        for part, items in rows.items():
            if len(items) < 3:
                continue
            items.sort(key=lambda it: it["density"])
            crops = []
            energies = []
            for it in items:
                x0, y0, x1, y1 = [int(round(v)) for v in it["box"]]
                crop = img.crop((x0, y0, max(x1, x0 + 1), max(y1, y0 + 1)))
                # the hide only: leave out the ruler band (the top 0.5 m) and a margin
                frac = min(0.8, 0.5 / it["size"][1])
                cw, ch = crop.size
                hide = crop.crop((int(cw * 0.12), int(ch * (frac + 0.06)), int(cw * 0.88), int(ch * 0.92)))
                energies.append(detail_energy(hide))
                crops.append(crop)
            best = max(energies) if max(energies) > 0 else 1.0
            scale = max(1, min(8, int(280 / max(8, max(c.size[0] for c in crops)))))
            cw = max(c.size[0] for c in crops) * scale
            ch = max(c.size[1] for c in crops) * scale
            sheet = Image.new("RGB", (len(items) * (cw + 8) + 8, ch + 64), (24, 24, 24))
            d2 = ImageDraw.Draw(sheet)
            f = font(15)
            pe = []
            for k, (it, crop, en) in enumerate(zip(items, crops, energies)):
                big = crop.resize((crop.size[0] * scale, crop.size[1] * scale), Image.NEAREST)
                x = 8 + k * (cw + 8)
                sheet.paste(big, (x, 60))
                sd = it["sd"]
                d2.text((x, 4), "%d texels/m" % it["density"], fill=(255, 255, 255), font=f)
                d2.text((x, 22), "screen %.0f x %.0f px/m" % (sd["u"], sd["v"]), fill=(200, 200, 200), font=f)
                d2.text((x, 40), "%.1f x %.1f texels per pixel" % (it["density"] / max(0.01, sd["u"]), it["density"] / max(0.01, sd["v"])),
                        fill=(255, 220, 90), font=f)
                pe.append({"density": it["density"], "px_per_m_across": sd["u"], "px_per_m_along": sd["v"],
                           "px_per_m_major": sd["major"], "px_per_m_minor": sd["minor"], "distance_m": sd["distance"],
                           "detail_rel": en / best, "crop_px": list(crop.size)})
            sheet.save(os.path.join(out_dir, "patches_%s_%s.png" % (name, part)))
            entry["patches"][part] = pe
            lines.append("  patches %-17s dist %6.1f m  screen %6.1f / %6.1f px per m (major/minor)  densities in frame: %s  (picture: patches_%s_%s.png)"
                         % (part, pe[0]["distance_m"], pe[0]["px_per_m_major"], pe[0]["px_per_m_minor"],
                            " ".join(str(p["density"]) for p in pe), name, part))
        report["shots"][name] = entry
    with open(os.path.join(out_dir, "measurements.json"), "w", encoding="utf-8") as fh:
        json.dump(report, fh, indent=1)
    with open(os.path.join(out_dir, "measurements.txt"), "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines) + "\n\n" + "\n".join(extra) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
