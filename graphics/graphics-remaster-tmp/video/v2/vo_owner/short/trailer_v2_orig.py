"""Uslu dur! trailer, part 1 (graphics): compose the montage from the two sides.

    python trailer_v2.py [--layout split|sbs] [--out <dir>] [--plan-only] [--stills] [--seconds a-b]
                         [--only clip,clip] [--style-stills] [--timing <json>]

The owner (2026-10-19): "video should wipe and transition between the scenes
and show a split screen be creative but dont cram two 16:9 videos in one 16:9
video". Both sides are full 16:9 frames of the same camera, so the master
(--layout split, 1920x1080 at 60 fps) composites them in ONE full-size frame:
left of an animated divider the vanilla game, right of it Uslu dur!, every
pixel at full size. Each scene opens fully vanilla, the divider sweeps in and
reveals the overhaul, settles into a split (vertical or diagonal), sweeps back
once, and some scenes end fully on the overhaul. The divider is a thin glowing
line; small upper-case labels ride with it. Scenes join with wipes, dips
through black and a flash cut into the firefight, never hard cuts.

Type and styling (the owner: "pick a less shitty looking font and styling"):
the game's own OFL UI fonts from arma_CWR/thirdparty/guerrilla-fonts, Oswald
(cwr_title.ttf) for the wordmark, titles and labels in upper case with wide
letter spacing, Roboto (cwr_body.ttf) for the small lines; off-white text with
a soft, wide, low-opacity shadow, no boxes; one accent, the game's HUD green, in thin rules, the label tick and the typing cursor (no gold
or orange anywhere); a
5% safe margin; eased fades and slides. The intro and the closing card set the
wordmark over a darkened, blurred frame of the footage.

Guards: a scene is refused unless its vanilla side was shot on the vanilla
upstream build ("mode=vanilla") and its Uslu dur! side on the look-director
build with "mode=remastered;lit=OK" (collect_frames.py's proof in
collect.json), the two videos are different files, and their first, middle
and last frames differ clearly (mean absolute difference, logged).

--layout sbs writes the secondary file: the two full frames side by side
(3840x1080, labels in the top corners), the same timeline and transitions.

Inputs: tmp/video/v2/{vanilla,ud}/<clip>/<clip>__<side>.mkv (collect_frames.py).
Writes into <out> (default tmp/video/v2/out-<layout>/):
    uslu_dur_v2.mp4   the silent montage (H.264 High, CRF 17, yuv420p, closed GOP
                      of half a second, 60 fps, faststart)
    timeline.json     segments, transitions and the narration windows (vo_v2.py,
                      check_vo.py and mix_vo.py read it)
    pairs.json        the per-scene provenance and difference check
    stills/           with --stills: frames from every transition and scene
"""
import argparse
import json
import math
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent))

import gen_video_v2 as G  # noqa: E402
import make_video  # noqa: E402
import make_video_v2 as M  # noqa: E402

FPS = G.FPS
W, H = 1920, 1080
V2 = M.V2
FONT_DIR = Path("D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/thirdparty/guerrilla-fonts")
TITLE_TTF, BODY_TTF = FONT_DIR / "cwr_title.ttf", FONT_DIR / "cwr_body.ttf"   # Oswald, Roboto (OFL)
MONO_TTF = FONT_DIR / "cwr_mono.ttf"                                            # Courier Prime (OFL)
TYPE_KIND = "mono"         # the opening's typed field report: "mono" or "title" (compared by eye)
TYPE_CPS = 15              # typing speed, characters a second
TYPE_START = 8.9           # seconds into the opening (the intro card has faded by then)
SAFE_X, SAFE_Y = 96, 54                     # 5% safe margins
INK = (240, 241, 243)                       # off-white, neutral
SOFT = (196, 200, 206)                      # small lines, light grey
# The one accent: the game's own HUD green, sampled from the vanilla 1080p HUD weapon text
# (tmp/look/ref/vanilla/optics_mist_hud.png, optics_mist_sight*.png: the median of the brightest
# 40% of the green text pixels, edges left out). The owner: "use green like the ingame hud text if anything".
HUD_GREEN = (63, 148, 61)
LINE = np.array([236, 238, 241], np.float32)   # the divider: a plain 2 px line, neutral
LINE_ALPHA = 0.55
INTRO_UNTIL = 9.0          # seconds of the opening scene under the intro overlay
CLOSING_S = 10.5
INTRO = ("USLU DUR!", "TRAILER  \u00b7  PART 1: GRAPHICS")
OUTRO = ("USLU DUR!", "PART 2: GAMEPLAY  \u00b7  COMING SOON")
MIN_PAIR_DIFF = 4.0        # mean absolute difference (0..255) a vanilla/ud pair must exceed

# Divider keys per scene: (seconds into the scene, position, angle). Position is
# where the line crosses the middle of the frame as a fraction of the width
# (left of it vanilla, right of it Uslu dur!); "V" = fully vanilla (line off the
# right edge), "U" = fully Uslu dur! (off the left edge). Angle in degrees, the
# top leaning right when positive. Between keys the line eases (smoothstep).
# The owner, 2026-10-19: "let the wipes go across the whole screen". Every scene
# is one full sweep, edge to edge: it opens fully vanilla, the line eases in from
# the right edge to a split, pauses there, then eases on out past the left edge and
# the scene ends fully Uslu dur!, so each scene is seen whole in both versions.
# Each leg takes about 2.5 s; the sweep starts once the incoming transition is done
# and ends half a second or more before the outgoing one begins.


def sweep(start, pause_at, pause, ang=0, leg=2.5):
    """Keys of one full sweep: V until start, a leg to pause_at, a pause, a leg to U."""
    t1 = start + leg
    t2 = t1 + pause
    return [(0, "V", ang), (start, "V", ang), (t1, pause_at, ang), (t2, pause_at, ang), (t2 + leg, "U", ang)]


CHOREO = {
    "opening": sweep(9.0, 0.5, 1.0),                  # after the intro card; U at 15.0, the dip at 15.1
    "valley_mist": sweep(1.5, 0.52, 1.5, 12),
    "pine_shafts": sweep(1.2, 0.55, 1.8),
    "bay_reflections": sweep(1.2, 0.5, 1.8, -8),
    "outpost": sweep(1.2, 0.5, 2.3),
    "cloud_shadows": sweep(1.2, 0.5, 1.0, 20, leg=2.4),
    "sinai_camp": sweep(1.2, 0.55, 1.3),
    "ambush": sweep(1.2, 0.5, 1.8, -10),
    "convoy": sweep(1.2, 0.46, 1.2, leg=2.4),
    "checkpoint": sweep(1.5, 0.5, 1.1, 6, leg=2.3),
    # the pause holds the line between the fire (vanilla) and the lit sentry (Uslu dur!);
    # the second leg carries it across the fire
    "town_patrol": sweep(1.5, 0.40, 1.5),
    "rain_town": sweep(1.2, 0.5, 1.3, -12),
    "supply_run": sweep(1.5, 0.5, 1.8),                # the pause splits the car down the middle
    "firefight": sweep(1.3, 0.5, 1.9),
}
# How each scene hands over to the next: (kind, seconds). The owner: "the vibe of
# the video should be cool calm and composed": long dissolves, soft slow wipes
# and slow dips through black (for a change of time of day or into the night);
# even the firefight enters on a dissolve and lets the explosions be the only
# sudden thing.
TRANSITIONS = {
    "opening": ("dip", 1.4), "valley_mist": ("fade", 1.0), "pine_shafts": ("wipe_left", 1.0),
    "bay_reflections": ("fade", 1.0), "outpost": ("fade", 1.0), "cloud_shadows": ("wipe_right", 1.0),
    "sinai_camp": ("fade", 1.0), "ambush": ("fade", 1.0), "convoy": ("dip", 1.4),
    "checkpoint": ("dip", 1.4), "town_patrol": ("fade", 1.0), "rain_town": ("dip", 1.4),
    "supply_run": ("fade", 1.2), "firefight": ("dip", 1.8),
}
# --timing <json> (optional; without it nothing below changes the plan): a per-scene timing
# override so a re-render follows the owner's narration ("the picture follows the voice",
# owner_vo.py fit writes the file) without editing CLIPS or the tables above:
#   "frames":      {clip: n}      the clip's length in 60 fps frames; a longer clip must be reshot
#                                 with that many frames on both sides (gen_video_v2.py --frames n,
#                                 the same camera path, slower); half the extra time lengthens the
#                                 divider's pause, half the fully Uslu dur! end
#   "transitions": {clip: [kind, seconds]}   the transition that takes that scene away
#   "intro_extra": s              the intro card holds s seconds longer over the opening (the
#                                 typed field report and the opening's sweep move by s too; give the
#                                 opening s * 60 more frames)
#   "closing_s":   s              the closing card's length
FRAMES_OVERRIDE = {}


def apply_timing(spec):
    """Apply a --timing override to this module's tables (in place)."""
    global INTRO_UNTIL, TYPE_START, CLOSING_S
    known = {c["clip"]: c for c in G.CLIPS}
    unknown = set(spec) - {"frames", "transitions", "intro_extra", "closing_s", "note"}
    if unknown:
        raise SystemExit(f"--timing: unknown keys {sorted(unknown)}")
    e = float(spec.get("intro_extra", 0.0))
    if e:
        INTRO_UNTIL += e
        TYPE_START += e
        CHOREO["opening"] = [(t + e if t > 0 else t, p, ang) for t, p, ang in CHOREO["opening"]]
    for clip, n in spec.get("frames", {}).items():
        if clip not in known:
            raise SystemExit(f"--timing: no clip {clip!r}")
        n = int(n)
        base = G.clip_frames(known[clip])
        if n < base:
            raise SystemExit(f"--timing: {clip} may only get longer ({n} < {base} frames)")
        FRAMES_OVERRIDE[clip] = n
        extra = (n - base) / FPS - (e if clip == "opening" else 0.0)
        if extra > 0:
            keys = CHOREO[clip]
            CHOREO[clip] = keys[:-2] + [(t + extra / 2, p, ang) for t, p, ang in keys[-2:]]
    for clip, (kind, secs) in spec.get("transitions", {}).items():
        if clip not in TRANSITIONS:
            raise SystemExit(f"--timing: no transition after {clip!r}")
        TRANSITIONS[clip] = (kind, float(secs))
    if "closing_s" in spec:
        CLOSING_S = float(spec["closing_s"])


def plan_with(spec, only=None, closing=True):
    """plan() under a timing override, leaving this module's tables as they were."""
    global INTRO_UNTIL, TYPE_START, CLOSING_S
    saved = (dict(FRAMES_OVERRIDE), dict(TRANSITIONS), {k: list(v) for k, v in CHOREO.items()},
             INTRO_UNTIL, TYPE_START, CLOSING_S)
    try:
        apply_timing(spec or {})
        return plan(only, closing)
    finally:
        FRAMES_OVERRIDE.clear()
        FRAMES_OVERRIDE.update(saved[0])
        TRANSITIONS.clear()
        TRANSITIONS.update(saved[1])
        CHOREO.clear()
        CHOREO.update(saved[2])
        INTRO_UNTIL, TYPE_START, CLOSING_S = saved[3], saved[4], saved[5]


def smooth(x):
    x = min(1.0, max(0.0, x))
    return x * x * (3 - 2 * x)


def when_of(c):
    world = {"Abel": "Malden", "Eden": "Everon", "Sinai": "Sinai"}.get(c["world"], c["world"])
    d = c["date"]
    s = f"{world}, {d[3]:02d}:{d[4]:02d}"
    if c.get("rain"):
        s += ", rain"
    return s


# ---------------------------------------------------------------- the plan
def plan(only=None, closing=True):
    """Segments (frames), transitions and the narration windows (seconds). only: a test
    montage of just these clips."""
    segs = []
    for c in G.CLIPS:
        if only and c["clip"] not in only:
            continue
        segs.append(dict(kind="clip", clip=c["clip"], title=c["title"],
                         frames=FRAMES_OVERRIDE.get(c["clip"], G.clip_frames(c))))
    if closing:
        segs.append(dict(kind="closing", clip="", title="", frames=int(round(CLOSING_S * FPS))))
    start = 0
    for i, s in enumerate(segs):
        if i:
            kind, secs = TRANSITIONS[segs[i - 1]["clip"]]
            tf = int(round(secs * FPS))
            segs[i - 1]["out"] = (kind, tf)
            s["in"] = (kind, tf)
            start = segs[i - 1]["start"] + segs[i - 1]["frames"] - tf
        s["start"] = start
    total = segs[-1]["start"] + segs[-1]["frames"]
    windows = {}
    for s in segs:
        a = s["start"] / FPS + (s["in"][1] / FPS / 2 if "in" in s else 0)
        b = (s["start"] + s["frames"]) / FPS - (s["out"][1] / FPS / 2 if "out" in s else 0)
        if s["kind"] == "closing":
            windows["outro"] = [round(a, 3), round(b, 3)]
        elif s["clip"] == "opening":
            windows["intro"] = [round(a, 3), round(a + INTRO_UNTIL, 3)]
            windows["opening"] = [round(a + INTRO_UNTIL + 0.6, 3), round(b, 3)]   # after the typing starts
        else:
            windows[s["clip"]] = [round(a, 3), round(b, 3)]
    return segs, total, windows


# ---------------------------------------------------------------- type
def font(kind, size):
    return ImageFont.truetype(str({"title": TITLE_TTF, "mono": MONO_TTF}.get(kind, BODY_TTF)), size)


def text_layer(items, align="left", shadow=(0.55, 9, 3), glow=None):
    """Render stacked text items into one RGBA float layer with a soft shadow.
    items: ("text", kind, size, tracking px, colour, space after) or ("rule", width, thickness, colour,
    space after). Returns (layer, anchor offset) where the anchor is the block's top-left."""
    pad = 48
    measured, width, height = [], 0, 0
    for it in items:
        if it[0] == "rule":
            _, rw, th, col, gap = it
            measured.append(("rule", rw, th, col, gap, rw, th))
            width, height = max(width, rw), height + th + gap
            continue
        _, text, kind, size, track, col, gap = it
        f = font(kind, size)
        asc, desc = f.getmetrics()
        tw = sum(f.getlength(ch) for ch in text) + track * max(0, len(text) - 1)
        measured.append(("text", text, f, track, col, gap, tw, asc + desc))
        width, height = max(width, int(math.ceil(tw))), height + asc + desc + gap
    img = Image.new("RGBA", (width + 2 * pad, height + 2 * pad), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    y = pad
    for m in measured:
        if m[0] == "rule":
            _, rw, th, col, gap, _, _ = m
            x = pad if align == "left" else pad + (width - rw) // 2
            d.rectangle([x, y, x + rw - 1, y + th - 1], fill=col + (255,))
            y += th + gap
            continue
        _, text, f, track, col, gap, tw, lh = m
        x = pad if align == "left" else pad + (width - tw) / 2
        for ch in text:
            d.text((x, y), ch, font=f, fill=col + (255,))
            x += f.getlength(ch) + track
        y += lh + gap
    a = img.split()[3]
    strength, radius, dy = shadow
    sh = Image.new("RGBA", img.size, (0, 0, 0, 0))
    sh.putalpha(a.filter(ImageFilter.GaussianBlur(radius)).point(lambda v: int(v * strength)))
    sh = sh.transform(sh.size, Image.AFFINE, (1, 0, 0, 0, 1, -dy))
    base = Image.new("RGBA", img.size, (0, 0, 0, 0))
    if glow:
        gcol, gs, gr = glow
        g = Image.new("RGBA", img.size, gcol + (0,))
        g.putalpha(a.filter(ImageFilter.GaussianBlur(gr)).point(lambda v: int(v * gs)))
        base = Image.alpha_composite(base, g)
    base = Image.alpha_composite(base, sh)
    base = Image.alpha_composite(base, img)
    return np.asarray(base).astype(np.float32) / 255.0, (pad, pad), (width, height)


def paste(frame, rgba, x, y, alpha=1.0):
    """Alpha-composite an RGBA float layer onto a uint8 or float RGB frame at (x, y), clipped."""
    if alpha <= 0.001:
        return frame
    h, w = rgba.shape[:2]
    fh, fw = frame.shape[:2]
    x, y = int(round(x)), int(round(y))
    x0, y0 = max(0, x), max(0, y)
    x1, y1 = min(fw, x + w), min(fh, y + h)
    if x1 <= x0 or y1 <= y0:
        return frame
    src = rgba[y0 - y:y1 - y, x0 - x:x1 - x]
    a = src[..., 3:4] * alpha
    region = frame[y0:y1, x0:x1].astype(np.float32) * (1 - a) + src[..., :3] * 255.0 * a
    frame[y0:y1, x0:x1] = np.clip(region + 0.5, 0, 255).astype(np.uint8) if frame.dtype == np.uint8 else region
    return frame


class Typo:
    """Every text element, rendered once."""

    def __init__(self, width):
        self.width = width
        lab = dict(shadow=(0.35, 6, 1))
        self.lab_v = text_layer([("text", "VANILLA", "title", 22, 4, INK, 0)], **lab)
        self.lab_u = text_layer([("text", "USLU DUR!", "title", 22, 4, INK, 0)], **lab)
        lu, (px, py), (tw, th) = self.lab_u                        # a small HUD-green tick before it
        lu = lu.copy()
        lu[py + 4:py + th - 4, px - 9:px - 6, :3] = np.array(HUD_GREEN, np.float32) / 255.0
        lu[py + 4:py + th - 4, px - 9:px - 6, 3] = 1.0
        self.lab_u = (lu, (px, py), (tw, th))
        self.typed = {}
        self.cards = {}
        self.titles = {}
        self.bottom = np.linspace(0, 1, 360, dtype=np.float32)[:, None, None] ** 1.6   # bottom gradient

    def card(self, lines):
        if lines not in self.cards:
            word, sub = lines
            self.cards[lines] = text_layer([("text", word, "title", 150, 16, INK, 22),
                                            ("rule", 150, 3, HUD_GREEN, 26),
                                            ("text", sub, "body", 30, 5, SOFT, 0)],
                                           align="center", shadow=(0.6, 16, 4))
        return self.cards[lines]

    def title(self, c):
        if c["clip"] not in self.titles:
            self.titles[c["clip"]] = text_layer([("text", c["title"].upper(), "title", 46, 4, INK, 12),
                                                 ("rule", 64, 2, HUD_GREEN, 12),
                                                 ("text", when_of(c), "body", 24, 2, SOFT, 0)],
                                                shadow=(0.75, 14, 3))
        return self.titles[c["clip"]]

    def put_card(self, frame, lines, alpha):
        layer, (px, py), (tw, th) = self.card(lines)
        return paste(frame, layer, (self.width - tw) / 2 - px, (H - th) / 2 - py - 10, alpha)

    def put_title(self, frame, c, t, t0):
        """The lower-third: fades and slides in at t0, out 3.6 s later; a gentle dark gradient under it."""
        a_in, a_out = smooth((t - t0) / 1.0), smooth((t - t0 - 3.8) / 1.0)
        alpha = a_in * (1 - a_out)
        if alpha <= 0.001:
            return frame
        frame = frame.astype(np.float32) if frame.dtype == np.uint8 else frame
        frame[H - 360:] *= (1 - 0.42 * alpha * self.bottom)
        layer, (px, py), (tw, th) = self.title(c)
        dx = -14 * (1 - a_in) - 6 * a_out
        return paste(frame, layer, SAFE_X - px + dx, H - 190 - th - py, alpha)

    def typed_layer(self, lines, n, cursor):
        """The first n characters of the field report lines (+ a HUD-green block cursor)."""
        key = (tuple(lines), n, cursor)
        if key in self.typed:
            return self.typed[key]
        items, left = [], n
        for i, (text, kind, size, track, col) in enumerate(lines):
            part = text[:max(0, left)]
            left -= len(text)
            items.append(("text", part if part else " ", kind, size, track, col, 14 if i == 0 else 0))
        layer, (px, py), (tw, th) = text_layer(items, shadow=(0.6, 12, 3))
        if cursor:
            # the line being typed: the last one with characters, or the first
            k, left = 0, n
            for i, (text, *_rest) in enumerate(lines):
                if left > 0:
                    k = i
                left -= len(text)
            text, kind, size, track, col = lines[k]
            f = font(kind, size)
            done = text[:max(0, n - sum(len(x[0]) for x in lines[:k]))]
            x = px + sum(f.getlength(ch) for ch in done) + track * len(done)
            y = py
            for text2, kind2, size2, _t, _c in lines[:k]:
                asc, desc = font(kind2, size2).getmetrics()
                y += asc + desc + 14
            asc, desc = f.getmetrics()
            cw = max(3, int(size * 0.12))
            layer = layer.copy()
            y0, y1 = int(y + asc * 0.18), int(y + asc + desc * 0.2)
            layer[y0:y1, int(x):int(x) + cw, :3] = np.array(HUD_GREEN, np.float32) / 255.0
            layer[y0:y1, int(x):int(x) + cw, 3] = 1.0
        self.typed[key] = (layer, (px, py), (tw, th))
        return self.typed[key]

    def put_typed(self, frame, c, t):
        """The opening's lower-third typed on letter by letter like a field report."""
        world, hhmm = when_of(c).split(", ")[:2]
        lines = [(f"{world.upper()}  {hhmm}", "mono" if TYPE_KIND == "mono" else "body", 26, 3, SOFT),
                 (c["title"].split(", ")[-1].upper(), TYPE_KIND, 44 if TYPE_KIND == "mono" else 46,
                  2 if TYPE_KIND == "mono" else 4, INK)]
        total = sum(len(x[0]) for x in lines)
        t_done = TYPE_START + total / TYPE_CPS
        if t < TYPE_START:
            return frame
        n = min(total, int((t - TYPE_START) * TYPE_CPS) + 1)
        blink = t - t_done
        cursor = n < total or (0 <= blink < 0.25) or (0.5 <= blink < 0.75)
        alpha = smooth((t - TYPE_START) / 0.15) * (1 - smooth((t - t_done - 2.6) / 1.0))
        if alpha <= 0.001:
            return frame
        frame = frame.astype(np.float32) if frame.dtype == np.uint8 else frame
        frame[H - 360:] *= (1 - 0.28 * alpha * self.bottom)
        layer, (px, py), (tw, th) = self.typed_layer(lines, n, cursor)
        _, _, (tw_full, th_full) = self.typed_layer(lines, total, False)
        return paste(frame, layer, SAFE_X - px, H - 190 - th_full - py, alpha)

    def put_labels(self, frame, x_line, av, au):
        lv, (pvx, pvy), (wv, hv) = self.lab_v
        lu, (pux, puy), (wu, hu) = self.lab_u
        y = SAFE_Y
        # the labels ride the line and stop at the 5% safe margin when it nears or leaves an edge
        xv = min(self.width - SAFE_X - wv, max(SAFE_X, x_line - 18 - wv))
        xu = min(self.width - SAFE_X - wu, max(SAFE_X, x_line + 30))
        frame = paste(frame, lv, xv - pvx, y - pvy, av * 0.9)
        return paste(frame, lu, xu - pux, y - puy, au * 0.9)


def soft_blur(frame_u8, darken):
    """A darkened, blurred copy of a frame (downscale, blur, upscale: fast)."""
    im = Image.fromarray(frame_u8 if frame_u8.dtype == np.uint8 else np.clip(frame_u8, 0, 255).astype(np.uint8))
    small = im.resize((im.width // 6, im.height // 6), Image.BILINEAR).filter(ImageFilter.GaussianBlur(3))
    return np.asarray(small.resize(im.size, Image.BILINEAR), np.float32) * darken


class Choreo:
    def __init__(self, keys, width):
        self.keys = []
        for t, p, ang in keys:
            reach = 1 + (H / 2 * abs(math.tan(math.radians(ang)))) / width + 0.03
            if p == "V":
                p = reach
            elif p == "U":
                p = 1 - reach
            self.keys.append((t, p, ang))

    def at(self, t):
        ks = self.keys
        if t <= ks[0][0]:
            return ks[0][1], ks[0][2]
        for (t0, p0, a0), (t1, p1, a1) in zip(ks, ks[1:]):
            if t <= t1:
                s = smooth((t - t0) / max(1e-6, t1 - t0))
                return p0 + (p1 - p0) * s, a0 + (a1 - a0) * s
        return ks[-1][1], ks[-1][2]


class Composer:
    def __init__(self, layout):
        self.layout = layout
        self.w = W if layout == "split" else 2 * W
        self.X = np.arange(self.w, dtype=np.float32)[None, :]
        self.Y = np.arange(H, dtype=np.float32)[:, None]
        self.typo = Typo(self.w)

    def split(self, a, b, p, ang, label_alpha=1.0):
        """a, b: uint8 frames (H, W, 3); one frame, the divider at p (fraction), angle ang."""
        tan = math.tan(math.radians(ang))
        cos = math.cos(math.radians(ang))
        x_mid = p * W
        xl = x_mid + (H / 2 - np.arange(H, dtype=np.float32)) * tan     # the line's x on every row
        out = b.copy()
        if abs(tan) < 1e-6:
            xi = int(min(W, max(0, round(x_mid))))
            out[:, :xi] = a[:, :xi]
        else:
            xs = np.clip(np.round(xl), 0, W).astype(np.int32)
            for y in range(H):
                if xs[y]:
                    out[y, :xs[y]] = a[y, :xs[y]]
        x0 = int(max(0, math.floor(xl.min()) - 4))
        x1 = int(min(W, math.ceil(xl.max()) + 4))
        if x1 > x0:
            Xb = np.arange(x0, x1, dtype=np.float32)[None, :]
            d = (Xb - xl[:, None]) * cos                       # signed distance to the line, px
            wgt = np.clip(d + 0.5, 0.0, 1.0)[..., None]
            band = a[:, x0:x1].astype(np.float32) * (1 - wgt) + b[:, x0:x1].astype(np.float32) * wgt
            cover = np.clip(1.5 - np.abs(d), 0.0, 1.0)[..., None] * LINE_ALPHA   # 2 px, anti-aliased
            band += (LINE - band) * cover
            out[:, x0:x1] = np.clip(band + 0.5, 0, 255).astype(np.uint8)
        vis_v = min(1.0, max(0.0, p))
        av = smooth((vis_v - 0.04) / 0.10) * label_alpha
        au = smooth(((1 - vis_v) - 0.04) / 0.10) * label_alpha
        x_lab = x_mid + (H / 2 - (SAFE_Y + 16)) * tan
        return self.typo.put_labels(out, x_lab, av, au)

    def sbs(self, a, b, label_alpha=1.0):
        out = np.concatenate([a, b], axis=1)
        out[:, W - 1:W + 1] = (out[:, W - 1:W + 1] * (1 - LINE_ALPHA) + LINE * LINE_ALPHA).astype(np.uint8)
        lv, (pvx, pvy), _ = self.typo.lab_v
        lu, (pux, puy), (wu, _) = self.typo.lab_u
        out = paste(out, lv, SAFE_X - pvx, SAFE_Y - pvy, label_alpha)
        return paste(out, lu, self.w - SAFE_X - wu - pux, SAFE_Y - puy, label_alpha)

    def transition(self, kind, q, prev, nxt):
        """Blend the outgoing and the incoming frame, q in 0..1."""
        prev = prev.astype(np.float32)
        nxt = nxt.astype(np.float32)
        if kind == "fade":
            s = smooth(q)
            return prev * (1 - s) + nxt * s
        if kind == "dip":
            if q < 0.5:
                return prev * (1 - smooth(q * 2))
            return nxt * smooth(q * 2 - 1)
        if kind == "flash":
            white = np.array([245, 246, 248], np.float32)
            if q < 0.5:
                s = smooth(q * 2) ** 1.5
                return prev + (white - prev) * s
            s = smooth(q * 2 - 1)
            return white + (nxt - white) * s
        s = smooth(q)
        soft = 320.0
        if kind == "wipe_left":      # the edge travels right -> left, the new scene enters from the right
            edge = self.w + soft - s * (self.w + 2 * soft)
            d = self.X - edge + 0 * self.Y
        elif kind == "wipe_right":   # left -> right, the new scene enters from the left
            edge = -soft + s * (self.w + 2 * soft)
            d = edge - self.X + 0 * self.Y
        else:                        # wipe_up: bottom -> top, the new scene rises from below
            edge = H + soft - s * (H + 2 * soft)
            d = self.Y - edge + 0 * self.X
        wgt = np.clip(d / soft + 0.5, 0.0, 1.0)
        wgt = (wgt * wgt * (3 - 2 * wgt))[..., None]
        return prev * (1 - wgt) + nxt * wgt


# ---------------------------------------------------------------- segments
def check_pair(clip):
    """Provenance and difference of a scene's two sides; raises on a bad pair."""
    infos = {}
    for side in ("vanilla", "ud"):
        j = V2 / side / clip / "collect.json"
        if not j.exists():
            raise SystemExit(f"{clip}: no {side} side (collect_frames.py)")
        infos[side] = json.loads(j.read_text(encoding="utf-8"))
    v, u = infos["vanilla"], infos["ud"]
    problems = []
    if "mode=vanilla" not in v.get("mode_line", "") or "vanilla-upstream-rwdi" not in v.get("game_dir", ""):
        problems.append(f"vanilla side not proven vanilla: {v.get('game_dir')!r} {v.get('mode_line', '')[:60]!r}")
    if ("mode=remastered;lit=OK" not in u.get("mode_line", "")
            or "look-director-rwdi" not in u.get("game_dir", "")):
        problems.append(f"ud side not proven look-director remastered: {u.get('game_dir')!r} "
                        f"{u.get('mode_line', '')[:60]!r}")
    if Path(v["video"]).resolve() == Path(u["video"]).resolve():
        problems.append("both sides are the same video")
    diffs = {}
    for still in ("first.png", "mid.png", "last.png"):
        a = np.asarray(Image.open(V2 / "vanilla" / clip / still).convert("RGB"), np.float32)
        b = np.asarray(Image.open(V2 / "ud" / clip / still).convert("RGB"), np.float32)
        diffs[still] = round(float(np.abs(a - b).mean()), 2)
    mean = sum(diffs.values()) / len(diffs)
    if mean < MIN_PAIR_DIFF:
        problems.append(f"the sides barely differ (mean abs difference {mean:.2f} < {MIN_PAIR_DIFF})")
    rec = dict(clip=clip, vanilla_game=v.get("game_dir"), vanilla_mode=v.get("mode_line", "")[:80],
               ud_game=u.get("game_dir"), ud_mode=u.get("mode_line", "")[:80], diff=diffs, mean_diff=round(mean, 2),
               ok=not problems, problems=problems)
    print(f"  pair {clip}: mean abs difference {mean:.1f} {diffs} {'OK' if not problems else 'REFUSED'}")
    if problems:
        raise SystemExit(f"{clip}: refused: " + "; ".join(problems))
    return rec


class ClipSeg:
    def __init__(self, seg, comp, ff):
        self.seg, self.comp = seg, comp
        self.c = G.clip_by_name(seg["clip"])
        self.n = seg["frames"]
        va, ua = M.source("vanilla", seg["clip"], V2), M.source("ud", seg["clip"], V2)
        if not va or va[1] != self.n or not ua or ua[1] != self.n:
            raise SystemExit(f"{seg['clip']}: both sides must hold {self.n} frames (collect_frames.py)")
        self.ra, self.rb = M.Reader(ff, va[0], va[2]), M.Reader(ff, ua[0], ua[2])
        self.choreo = Choreo(CHOREO[seg["clip"]], W)
        self.k = 0

    def frame(self):
        a, b = self.ra.next(), self.rb.next()
        if a is None or b is None:
            raise SystemExit(f"{self.seg['clip']}: a side ended at frame {self.k}")
        a, b = np.asarray(a), np.asarray(b)
        t = self.k / FPS
        opening = self.seg["clip"] == "opening"
        label_alpha = smooth((t - INTRO_UNTIL - 0.2) / 1.0) if opening else 1.0
        t_in = self.seg["in"][1] / FPS if "in" in self.seg else 0.0
        t_out = self.seg["out"][1] / FPS if "out" in self.seg else 0.0
        t_end = self.n / FPS
        if t_in:
            label_alpha *= smooth((t - t_in * 0.8) / 0.5)
        if t_out:
            label_alpha *= 1 - smooth((t - (t_end - t_out * 1.2)) / 0.5)
        if self.comp.layout == "split":
            p, ang = self.choreo.at(t)
            out = self.comp.split(a, b, p, ang, label_alpha)
        else:
            out = self.comp.sbs(a, b, label_alpha)
        if opening:
            out = self.comp.typo.put_typed(out, self.c, t)
        else:
            out = self.comp.typo.put_title(out, self.c, t, 0.7)
        if opening:  # the intro: the wordmark over a darkened, blurred frame of the scene
            ia = smooth((t - 0.3) / 1.4) * (1 - smooth((t - INTRO_UNTIL + 1.4) / 1.3))
            if ia > 0.001:
                f = out.astype(np.float32)
                out = f * (1 - ia) + soft_blur(f, 0.45) * ia
                out = self.comp.typo.put_card(out, INTRO, ia)
        self.k += 1
        return out

    def close(self):
        self.ra.close()
        self.rb.close()


class ClosingSeg:
    def __init__(self, seg, comp):
        self.seg, self.comp, self.k = seg, comp, 0
        still = V2 / "ud" / "opening" / "mid.png"
        if not still.exists():
            raise SystemExit("closing card: no ud opening still yet")
        im = Image.open(still).convert("RGB")
        if comp.layout == "sbs":
            im = im.resize((comp.w, int(comp.w * im.size[1] / im.size[0])))
        small = im.resize((im.width // 6, im.height // 6), Image.BILINEAR).filter(ImageFilter.GaussianBlur(3))
        self.bg = small.resize(im.size, Image.BICUBIC)
        self.n = seg["frames"]

    def frame(self):
        t = self.k / FPS
        z = 1.0 + 0.05 * smooth(self.k / max(1, self.n - 1))
        bw, bh = self.bg.size
        cw, ch = int(self.comp.w / z), int(H / z)
        x0, y0 = (bw - cw) // 2, (bh - ch) // 2
        im = self.bg.crop((x0, y0, x0 + cw, y0 + ch)).resize((self.comp.w, H), Image.BILINEAR)
        out = np.asarray(im, np.float32) * 0.42
        out = self.comp.typo.put_card(out, OUTRO, smooth((t - 0.8) / 1.8))
        self.k += 1
        return out

    def close(self):
        pass


def encoder(ff, dst, size, crf):
    return subprocess.Popen([ff, "-y", "-loglevel", "error", "-f", "rawvideo", "-pix_fmt", "rgb24", "-s",
                             f"{size[0]}x{size[1]}", "-framerate", str(FPS), "-i", "-", "-c:v", "libx264",
                             "-profile:v", "high", "-crf", str(crf), "-preset", "medium", "-g", str(FPS // 2),
                             "-bf", "2", "-flags", "+cgop", "-pix_fmt", "yuv420p", "-r", str(FPS),
                             "-movflags", "+faststart", str(dst)], stdin=subprocess.PIPE)


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--layout", choices=("split", "sbs"), default="split")
    ap.add_argument("--out", default="")
    ap.add_argument("--plan-only", action="store_true")
    ap.add_argument("--stills", action="store_true")
    ap.add_argument("--seconds", default="", help="render only a-b seconds (a test of the look)")
    ap.add_argument("--only", default="", help="a test montage of these clips (comma list), no closing card")
    ap.add_argument("--timing", default="", help="an optional per-scene timing override (JSON; see apply_timing); "
                                                 "without it the plan is the tables above")
    a = ap.parse_args(argv)
    spec = None
    if a.timing:
        spec = json.loads(Path(a.timing).read_text(encoding="utf-8"))
        apply_timing(spec)
    out = Path(a.out) if a.out else V2 / f"out-{a.layout}"
    out.mkdir(parents=True, exist_ok=True)
    only = [x for x in a.only.split(",") if x]
    segs, total, windows = plan(only or None, closing=not only)
    if only:
        out = out / "test"
        out.mkdir(parents=True, exist_ok=True)
    timeline = dict(fps=FPS, layout=a.layout, total_frames=total, duration=round(total / FPS, 3),
                    video=str(out / "uslu_dur_v2.mp4"), windows=windows,
                    segments=[dict(kind=s["kind"], clip=s["clip"], title=s["title"], start=round(s["start"] / FPS, 3),
                                   end=round((s["start"] + s["frames"]) / FPS, 3), frames=s["frames"],
                                   transition_in=s.get("in"), transition_out=s.get("out")) for s in segs])
    if spec is not None:
        timeline["timing"] = dict(file=str(Path(a.timing).resolve()), spec=spec)
    (out / "timeline.json").write_text(json.dumps(timeline, indent=1), encoding="utf-8")
    print(f"plan: {len(segs)} segments, {total} frames, {total / FPS:.2f} s -> {out / 'timeline.json'}")
    if a.plan_only:
        return 0
    pairs = [check_pair(s["clip"]) for s in segs if s["kind"] == "clip"]
    (out / "pairs.json").write_text(json.dumps(pairs, indent=1), encoding="utf-8")
    g0, g1 = 0, total
    if a.seconds:
        lo, hi = (float(x) for x in a.seconds.split("-"))
        g0, g1 = int(lo * FPS), min(total, int(hi * FPS))
    ff = make_video.ffmpeg_exe()
    comp = Composer(a.layout)
    dst = out / ("uslu_dur_v2.mp4" if not a.seconds else f"test_{a.seconds}.mp4")
    enc = encoder(ff, dst, (comp.w, H), 17 if a.layout == "split" else 18)
    stills_at = {}
    if a.stills:
        for s in segs:
            stills_at[s["start"] + s["frames"] // 2] = f"{s['clip'] or 'closing'}__mid"
            if "in" in s:
                stills_at[s["start"] + s["in"][1] // 2] = f"{s['clip'] or 'closing'}__in_{s['in'][0]}"
            if s["kind"] == "clip":
                for frac, tag in ((0.12, "a"), (0.3, "b"), (0.72, "c")):
                    stills_at[s["start"] + int(s["frames"] * frac)] = f"{s['clip']}__{tag}"
                if s["clip"] == "opening":
                    stills_at[s["start"] + int(4.5 * FPS)] = "opening__intro"
                    stills_at[s["start"] + int(11.5 * FPS)] = "opening__lowerthird"
        (out / "stills").mkdir(exist_ok=True)
    live = {}
    for g in range(0, g1):
        for i, s in enumerate(segs):
            if s["start"] == g and i not in live:
                live[i] = ClipSeg(s, comp, ff) if s["kind"] == "clip" else ClosingSeg(s, comp)
        act = sorted(i for i in live if segs[i]["start"] <= g < segs[i]["start"] + segs[i]["frames"])
        frames = {i: live[i].frame() for i in act}
        if len(act) == 1:
            img = frames[act[0]]
        else:
            i0, i1 = act[0], act[-1]
            kind, tf = segs[i1]["in"]
            q = (g - segs[i1]["start"] + 0.5) / tf
            img = comp.transition(kind, q, frames[i0], frames[i1])
        for i in list(live):
            if g >= segs[i]["start"] + segs[i]["frames"] - 1:
                live[i].close()
                del live[i]
        if g < g0:
            continue
        b = img if img.dtype == np.uint8 else np.clip(img + 0.5, 0, 255).astype(np.uint8)
        enc.stdin.write(b.tobytes())
        if g in stills_at:
            Image.fromarray(b).save(out / "stills" / f"{g / FPS:06.2f}_{stills_at[g]}.png")
        if g % (FPS * 10) == 0:
            print(f"  {g / FPS:6.1f} s", flush=True)
    enc.stdin.close()
    if enc.wait() != 0:
        raise SystemExit("ffmpeg failed")
    print(f"montage {(g1 - g0) / FPS:.2f} s -> {dst}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
