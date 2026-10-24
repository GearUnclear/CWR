"""Iterations on thumbnail 3 (split3_bay): vanilla left of a thin divider, Uslu dur! right, same frame.

Built on thumbs/1/src/thumb.py (same split, same type treatment). The footage is only
cropped and scaled (never graded); the only darkening is the soft glyph shadow, kept
on sky. Usage: python thumb3.py specs.json [name ...]"""
import math, json, sys, os
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter

V2 = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2"
FR = V2 + "/thumbs/iter3/work/frames"
FONTS = "D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts/"
TITLE = FONTS + "cwr_title.ttf"
BODY = FONTS + "cwr_body.ttf"
W, H = 1280, 720
INK = (240, 241, 243)
HUD_GREEN = (63, 148, 61)
LINE = np.array([236, 238, 241], np.float32)
# YouTube overlay zones at 1280x720 (hover icons top right, duration badge bottom right)
HOVER = (1150, 0, 1280, 235)
BADGE = (1090, 615, 1280, 720)


def load(clip, side, n, crop, size=(W, H)):
    im = Image.open(f"{FR}/{clip}_{side}_f{n:04d}.png").convert("RGB")
    if crop:
        x0, y0, cw = crop
        ch = cw * size[1] / size[0]
        assert x0 >= 0 and y0 >= 0 and x0 + cw <= 1920 + 0.5 and y0 + ch <= 1080 + 0.5, crop
        assert size[0] / cw <= 1.26, "upscale past 1.25x"
        im = im.resize(size, Image.LANCZOS, box=(x0, y0, x0 + cw, y0 + ch))
    else:
        im = im.resize(size, Image.LANCZOS)
    return np.asarray(im).astype(np.float32)


def split(a, b, xm, ang, lw, lalpha, line_top=0):
    """Left a, right b; the line passes x=xm at mid-height, tilted ang degrees (top leans right).
    The drawn line starts at y=line_top (the seam itself always runs full height)."""
    tan, cos = math.tan(math.radians(ang)), math.cos(math.radians(ang))
    Y = np.arange(H, dtype=np.float32)[:, None]
    X = np.arange(W, dtype=np.float32)[None, :]
    xl = xm + (H / 2 - Y) * tan
    d = (X - xl) * cos
    wgt = np.clip(d + 0.5, 0, 1)[..., None]
    out = a * (1 - wgt) + b * wgt
    cover = np.clip(lw / 2 + 0.5 - np.abs(d), 0, 1) * lalpha
    if line_top:
        cover *= np.clip((Y - line_top) / 24.0, 0, 1)      # short fade-in at the top end
    out += (LINE - out) * cover[..., None]
    return out


def text_rgba(text, path, size, track, col, shadow=(0.6, 12, 3)):
    f = ImageFont.truetype(path, size)
    asc, desc = f.getmetrics()
    tw = sum(f.getlength(ch) for ch in text) + track * (len(text) - 1)
    pad = int(size * 0.6) + 20
    img = Image.new("RGBA", (int(tw) + 2 * pad, asc + desc + 2 * pad), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    x = pad
    for ch in text:
        d.text((x, pad), ch, font=f, fill=col + (255,))
        x += f.getlength(ch) + track
    bb = f.getbbox("U")
    cap_top, cap_bot = pad + bb[1], pad + bb[3]
    a = img.split()[3]
    s, r, dy = shadow
    sh = Image.new("RGBA", img.size, (0, 0, 0, 0))
    sh.putalpha(a.filter(ImageFilter.GaussianBlur(r)).point(lambda v: int(min(255, v * s))))
    sh = sh.transform(sh.size, Image.AFFINE, (1, 0, 0, 0, 1, -dy))
    base = Image.alpha_composite(sh, img)
    arr = np.asarray(base).astype(np.float32) / 255.0
    return arr, dict(x=pad, cap_top=cap_top, cap_bot=cap_bot, w=tw)


def paste(frame, rgba, x, y):
    h, w = rgba.shape[:2]
    x, y = int(round(x)), int(round(y))
    x0, y0, x1, y1 = max(0, x), max(0, y), min(W, x + w), min(H, y + h)
    if x1 <= x0 or y1 <= y0:
        return
    src = rgba[y0 - y:y1 - y, x0 - x:x1 - x]
    al = src[..., 3:4]
    frame[y0:y1, x0:x1] = frame[y0:y1, x0:x1] * (1 - al) + src[..., :3] * 255 * al


def place(frame, text, path, size, track, *, left=None, right=None, center=None, cap_top=None, cap_bot=None,
          shadow=(0.6, 12, 3), col=INK):
    arr, m = text_rgba(text, path, size, track, col, shadow)
    if left is not None:
        x = left - m["x"]
    elif right is not None:
        x = right - m["w"] - m["x"]
    else:
        x = center - m["w"] / 2 - m["x"]
    y = cap_top - m["cap_top"] if cap_top is not None else cap_bot - m["cap_bot"]
    paste(frame, arr, x, y)
    return dict(x0=x + m["x"], x1=x + m["x"] + m["w"], top=y + m["cap_top"], bot=y + m["cap_bot"])


def rule(frame, x, y, w, t, col=HUD_GREEN):
    frame[int(y):int(y + t), int(x):int(x + w)] = np.array(col, np.float32)


def line_x(spec, y):
    return spec["xm"] + (H / 2 - y) * math.tan(math.radians(spec["ang"]))


def overlaps(box, zone):
    return not (box[2] <= zone[0] or box[0] >= zone[2] or box[3] <= zone[1] or box[1] >= zone[3])


def build(spec):
    if "pair" in spec:                            # before/after: the same window of each side, side by side
        hw = int(round(spec["xm"]))
        a = np.zeros((H, W, 3), np.float32)
        b = np.zeros((H, W, 3), np.float32)
        a[:, :hw] = load(spec["clip"], "vanilla", spec["n"], spec["pair"], (hw, H))
        b[:, hw:] = load(spec["clip"], "ud", spec["n"], spec["pair"], (W - hw, H))
    else:
        a = load(spec["clip"], "vanilla", spec["n"], spec.get("crop"))
        b = load(spec["clip"], "ud", spec["n"], spec.get("crop"))
    out = split(a, b, spec["xm"], spec["ang"], spec.get("lw", 3.0), spec.get("lalpha", 0.85), spec.get("line_top", 0))
    boxes = []
    for t in spec["texts"]:
        kind = t["kind"]
        size = t["size"]
        font = BODY if t.get("body") else TITLE
        shadow = tuple(t.get("shadow", (0.65, 16, 4) if kind == "brand" else (0.55, 8, 2)))
        track = t.get("track", 8 if kind == "brand" else 5)
        text = t.get("text", "USLU DUR!" if kind == "brand" else "VANILLA")
        kw = dict(shadow=shadow)
        yk = "cap_top" if "cap_top" in t else "cap_bot"
        kw[yk] = t[yk]
        ycap = t[yk]
        # vertical middle of the caps, for anchoring against a tilted line
        cap_h = size * 0.83
        ymid = ycap + cap_h / 2 if yk == "cap_top" else ycap - cap_h / 2
        if "before_line" in t:                    # right edge a gap left of the divider
            kw["right"] = line_x(spec, ymid) - t["before_line"]
        elif "after_line" in t:                   # left edge a gap right of the divider
            kw["left"] = line_x(spec, ymid) + t["after_line"]
        elif "center_line" in t:                  # centred on the divider
            kw["center"] = line_x(spec, ymid) + t["center_line"]
        elif "left" in t:
            kw["left"] = t["left"]
        elif "right" in t:
            kw["right"] = t["right"]
        else:
            kw["center"] = t["center"]
        r = place(out, text, font, size, track, col=tuple(t.get("col", INK)), **kw)
        boxes.append((text, r))
        if t.get("rule"):
            rw, rt, rg = t["rule"]
            rx = r["x0"] if t.get("rule_align", "left") == "left" else (r["x0"] + r["x1"]) / 2 - rw / 2
            if t.get("rule_align") == "right":
                rx = r["x1"] - rw
            rule(out, rx, r["bot"] + rg, rw, rt)
    for text, r in boxes:
        bx = (r["x0"], r["top"], r["x1"], r["bot"])
        warn = [nm for nm, z in (("HOVER", HOVER), ("BADGE", BADGE)) if overlaps(bx, z)]
        print(f"   {text!r:28} x {r['x0']:.0f}..{r['x1']:.0f}  caps y {r['top']:.0f}..{r['bot']:.0f}  {' '.join(warn)}")
    return Image.fromarray(np.clip(out + 0.5, 0, 255).astype(np.uint8))


if __name__ == "__main__":
    specs = json.load(open(sys.argv[1]))
    only = set(sys.argv[2:])
    for name, spec in specs.items():
        if only and name not in only:
            continue
        print(name)
        im = build(spec)
        p = spec["out"]
        os.makedirs(os.path.dirname(p), exist_ok=True)
        im.save(p + ".png")
        if spec.get("jpg", True):
            im.save(p + ".jpg", quality=93, subsampling=0)
