"""Split thumbnails: vanilla left of a thin divider, Uslu dur! right, same frame.

The footage is only cropped and scaled (never graded); text carries a soft shadow
the way the trailer's titles do; no boxes, one accent (HUD green)."""
import math, json, sys
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter

V2 = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2"
FR = V2 + "/thumbs/1/frames"
FONTS = "D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts/"
TITLE = FONTS + "cwr_title.ttf"
BODY = FONTS + "cwr_body.ttf"
W, H = 1280, 720
INK = (240, 241, 243)
HUD_GREEN = (63, 148, 61)
LINE = np.array([236, 238, 241], np.float32)


def load(clip, side, n, crop):
    im = Image.open(f"{FR}/{clip}_{side}_f{n:04d}.png").convert("RGB")
    if crop:
        x0, y0, cw = crop
        ch = cw * 9 / 16
        im = im.crop((int(round(x0)), int(round(y0)), int(round(x0 + cw)), int(round(y0 + ch))))
    return np.asarray(im.resize((W, H), Image.LANCZOS)).astype(np.float32)


def split(a, b, xm, ang, lw, lalpha):
    """Left a, right b; the line passes x=xm at mid-height, tilted ang degrees (top leans right)."""
    tan, cos = math.tan(math.radians(ang)), math.cos(math.radians(ang))
    Y = np.arange(H, dtype=np.float32)[:, None]
    X = np.arange(W, dtype=np.float32)[None, :]
    xl = xm + (H / 2 - Y) * tan
    d = (X - xl) * cos                                   # signed distance, px
    wgt = np.clip(d + 0.5, 0, 1)[..., None]
    out = a * (1 - wgt) + b * wgt
    cover = np.clip(lw / 2 + 0.5 - np.abs(d), 0, 1)[..., None] * lalpha
    out += (LINE - out) * cover
    return out


def text_rgba(text, path, size, track, col, shadow=(0.6, 12, 3), tick=None):
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
    bb = f.getbbox("U")                                   # cap top/bottom relative to the draw origin
    cap_top, cap_bot = pad + bb[1], pad + bb[3]
    if tick:                                              # a HUD-green tick before the word
        tw_, gap = tick
        d.rectangle([pad - gap - tw_, cap_top, pad - gap - 1, cap_bot], fill=HUD_GREEN + (255,))
    a = img.split()[3]
    s, r, dy = shadow
    sh = Image.new("RGBA", img.size, (0, 0, 0, 0))
    sh.putalpha(a.filter(ImageFilter.GaussianBlur(r)).point(lambda v: int(min(255, v * s))))
    sh = sh.transform(sh.size, Image.AFFINE, (1, 0, 0, 0, 1, -dy))
    base = Image.alpha_composite(sh, img)
    arr = np.asarray(base).astype(np.float32) / 255.0
    # anchor info: left x of the ink, cap top, cap bottom, ink width
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


def place(frame, text, path, size, track, *, left=None, right=None, cap_top=None, cap_bot=None,
          shadow=(0.6, 12, 3), tick=None, col=INK):
    arr, m = text_rgba(text, path, size, track, col, shadow, tick)
    x = left - m["x"] if left is not None else right - m["w"] - m["x"]
    y = cap_top - m["cap_top"] if cap_top is not None else cap_bot - m["cap_bot"]
    paste(frame, arr, x, y)
    return dict(x0=x + m["x"], x1=x + m["x"] + m["w"], top=y + m["cap_top"], bot=y + m["cap_bot"])


def rule(frame, x, y, w, t, col=HUD_GREEN):
    frame[int(y):int(y + t), int(x):int(x + w)] = np.array(col, np.float32)


def line_x(spec, y):
    return spec["xm"] + (H / 2 - y) * math.tan(math.radians(spec["ang"]))


def build(spec):
    a = load(spec["clip"], "vanilla", spec["n"], spec.get("crop"))
    b = load(spec["clip"], "ud", spec["n"], spec.get("crop"))
    out = split(a, b, spec["xm"], spec["ang"], spec.get("lw", 3.0), spec.get("lalpha", 0.85))
    for t in spec["texts"]:
        kind = t["kind"]
        if kind == "labels":                              # small labels riding the line
            y, size = t["cap_top"], t["size"]
            xl = line_x(spec, y + size * 0.35)
            place(out, "VANILLA", TITLE, size, t.get("track", 5), right=xl - t.get("gap", 22), cap_top=y,
                  shadow=t.get("shadow", (0.55, 8, 2)))
            if t.get("ud", True):
                place(out, "USLU DUR!", TITLE, size, t.get("track", 5), left=xl + t.get("gap", 22) + 12, cap_top=y,
                      shadow=t.get("shadow", (0.55, 8, 2)), tick=(4, 11))
        elif kind == "vanilla":
            y, size = t["cap_top"], t["size"]
            xl = line_x(spec, y + size * 0.35)
            xl = line_x(spec, y - size * 0.35) if t.get("bottom") else xl
            place(out, "VANILLA", TITLE, size, t.get("track", 5), right=xl - t.get("gap", 22),
                  cap_top=y if not t.get("bottom") else None, cap_bot=y if t.get("bottom") else None,
                  shadow=t.get("shadow", (0.55, 8, 2)))
        elif kind == "brand":
            size = t["size"]
            kw = dict(shadow=t.get("shadow", (0.65, 16, 4)), tick=t.get("tick"))
            if "after_line" in t:                         # left edge a fixed gap right of the divider
                yy = t.get("cap_top", t.get("cap_bot"))
                kw["left"] = line_x(spec, yy) + t["after_line"]
            elif "left" in t:
                kw["left"] = t["left"]
            else:
                kw["right"] = t["right"]
            if "cap_top" in t:
                kw["cap_top"] = t["cap_top"]
            else:
                kw["cap_bot"] = t["cap_bot"]
            r = place(out, t.get("text", "USLU DUR!"), TITLE, size, t.get("track", 8), **kw)
            if t.get("rule"):
                rw, rt, rg = t["rule"]
                rule(out, r["x0"], r["bot"] + rg, rw, rt)
        elif kind == "text":
            place(out, t["text"], BODY if t.get("body") else TITLE, t["size"], t.get("track", 4),
                  left=t.get("left"), right=t.get("right"), cap_top=t.get("cap_top"), cap_bot=t.get("cap_bot"),
                  shadow=tuple(t.get("shadow", (0.55, 8, 2))), col=tuple(t.get("col", INK)))
    return Image.fromarray(np.clip(out + 0.5, 0, 255).astype(np.uint8))


def preview(im, path):
    """Full 640 wide, then 360x202 and 168x94 at 1:1, side by side, plus the 168 one zoomed x2."""
    m = im.resize((360, 202), Image.LANCZOS)
    s = im.resize((168, 94), Image.LANCZOS)
    big = im.resize((640, 360), Image.LANCZOS)
    sheet = Image.new("RGB", (640 + 360 + 168 + 40, 360), (255, 255, 255))
    sheet.paste(big, (0, 0))
    sheet.paste(m, (660, 0))
    sheet.paste(s, (1040, 0))
    sheet.paste(s.resize((336, 188), Image.NEAREST), (660, 222))
    sheet.save(path)


if __name__ == "__main__":
    specs = json.load(open(sys.argv[1]))
    for name, spec in specs.items():
        im = build(spec)
        p = spec.get("out", f"{FR}/../wip_{name}")
        im.save(p + ".png")
        preview(im, p + "_pv.png")
        print(name)
