"""recorder.py: the triRecord dumps (P0-10) on the gate side: the bit-identity check against
triScreenshot, the lossless FFV1 encode and an exact per-frame compare of two recordings.
Image work and ffmpeg: run it through squad-heavy.ps1 (-Kind analysis for check and diff,
-Kind encode for encode).

    python -I recorder.py check <dir> [--json out.json]
    python -I recorder.py encode <dump.raw> --out <clip.mkv> [--fps 60] [--keep-raw] [--no-verify]
    python -I recorder.py diff <a.raw|a.mkv> <b.raw|b.mkv> [--json out.json]

A dump is raw RGB24, rows bottom-up (as glReadPixels returns them), frame after frame, with a
sidecar <dump>.txt: "w=.. h=.. frames=.. mode=.. half=.. rows=bottom-up format=rgb24", then one
line per frame "<i> <wall ms since frame 0> <camera path time s> <1 if a triScreenshot captured
this frame>". Mode 0 every presented frame, 1 only triScreenshot frames, 2 every frame at half
size (2x2 box mean (a + b + c + d + 2) / 4).

check: under <dir>, every dump <stem>.raw is paired with the shots its test took as
triScreenshot "<stem>_<k>" (files NNN_<stem>_<k>.png, k = 1, 2, ...): the k-th frame flagged in
the sidecar against shot k, full size or box-filtered for a half dump. PASS = as many flagged
frames as shots (at least one) and 0 differing pixels on every pair. Exit 0 PASS, 1 FAIL, 2 no
dump found.

encode: FFV1 (level 3, pix_fmt bgr0: lossless for 8-bit RGB, intra only), rows flipped to
top-down, 60 fps; writes <clip>.pacing.json (wall-clock pacing, camera path steps); verifies
by decoding every frame back and comparing it with the dump (skip with --no-verify), then
deletes the dump unless --keep-raw. Never judge motion from a crf 18 encode.

diff: frame by frame, the differing pixels (any channel) and the largest level difference;
"identical" when every frame has 0. Clips of different length compare their common frames
and say so.
"""
import argparse
import json
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gatelib as gl  # noqa: E402
import numpy as np  # noqa: E402
from PIL import Image  # noqa: E402


# ----------------------------------------------------------------------------- dumps

def read_sidecar(raw):
    with open(raw + ".txt", encoding="utf-8") as f:
        lines = [l.strip() for l in f.read().splitlines() if l.strip()]
    hdr = dict(kv.split("=", 1) for kv in lines[0].split())
    rows = []
    for l in lines[1:]:
        p = l.split()
        rows.append(dict(i=int(p[0]), wall_ms=float(p[1]), path_t=float(p[2]), shot=int(p[3])))
    info = dict(w=int(hdr["w"]), h=int(hdr["h"]), frames=int(hdr["frames"]), mode=int(hdr.get("mode", 0)),
                half=int(hdr.get("half", 0)), rows_order=hdr.get("rows", "bottom-up"), frame_rows=rows)
    if info["rows_order"] != "bottom-up":
        raise SystemExit(f"{raw}: unknown row order {info['rows_order']}")
    size = os.path.getsize(raw)
    want = info["w"] * info["h"] * 3 * info["frames"]
    if size != want:
        raise SystemExit(f"{raw}: {size} bytes, the sidecar says {want}")
    return info


def frame(mm, info, i):
    """Frame i of a dump, top-down rows (like a PNG)."""
    n = info["w"] * info["h"] * 3
    a = np.asarray(mm[i * n:(i + 1) * n]).reshape(info["h"], info["w"], 3)
    return a[::-1]


def box_half(img):
    """The recorder's half-size filter on a top-down image of even height: rows are paired the same
    way bottom-up and top-down, so the result equals the recorder's (flipped)."""
    h, w = img.shape[0] // 2 * 2, img.shape[1] // 2 * 2
    bu = img[::-1][:h, :w].astype(np.uint16)  # bottom-up, as the recorder sees it
    s = bu[0::2, 0::2] + bu[0::2, 1::2] + bu[1::2, 0::2] + bu[1::2, 1::2]
    return ((s + 2) // 4).astype(np.uint8)[::-1]


def compare(a, b):
    if a.shape != b.shape:
        return dict(size_a=list(a.shape), size_b=list(b.shape), px=-1, max=-1)
    d = np.abs(a.astype(np.int16) - b.astype(np.int16))
    return dict(px=int((d.max(axis=2) > 0).sum()), max=int(d.max()))


# ----------------------------------------------------------------------------- check

SHOT_RE = re.compile(r"^(\d+)_(.+)_(\d+)\.png$")


def find_shots(root):
    shots = {}
    for d, _, fs in os.walk(root):
        for f in fs:
            m = SHOT_RE.match(f)
            if m:
                shots.setdefault((m.group(2), int(m.group(3))), []).append(os.path.join(d, f))
    return shots


def pick(cands, near):
    """The shot copy inside the dump's own tri output tree when there are several (run-captures may
    copy shots out of the tri tree)."""
    if len(cands) == 1:
        return cands[0]
    near = os.path.normcase(os.path.abspath(near))
    inside = [c for c in cands if os.path.normcase(os.path.abspath(c)).startswith(near)]
    return sorted(inside or cands)[0]


def cmd_check(o):
    dumps = []
    for d, _, fs in os.walk(o.dir):
        dumps += [os.path.join(d, f) for f in fs if f.endswith(".raw") and os.path.exists(os.path.join(d, f + ".txt"))]
    if not dumps:
        print(f"recorder-check: NO DUMP under {o.dir}")
        return 2
    shots = find_shots(o.dir)
    report = dict(dir=os.path.abspath(o.dir), dumps=[], verdict="PASS")
    for raw in sorted(dumps):
        info = read_sidecar(raw)
        stem = os.path.splitext(os.path.basename(raw))[0]
        flagged = [r["i"] for r in info["frame_rows"] if r["shot"]]
        ks = sorted(k for (s, k) in shots if s == stem)
        rec = dict(dump=raw, mode=info["mode"], half=info["half"], frames=info["frames"], size=[info["w"], info["h"]],
                   flagged=len(flagged), shots=len(ks), pairs=[])
        mm = np.memmap(raw, dtype=np.uint8, mode="r")
        for n, k in enumerate(ks):
            png = pick(shots[(stem, k)], os.path.dirname(os.path.dirname(raw)))
            if n >= len(flagged):
                rec["pairs"].append(dict(shot=png, frame=None, px=-1, max=-1, note="no flagged frame left"))
                continue
            with Image.open(png) as im:
                ref = np.asarray(im.convert("RGB"))
            if info["half"]:
                ref = box_half(ref)
            r = compare(frame(mm, info, flagged[n]), ref)
            r.update(shot=png, frame=flagged[n])
            rec["pairs"].append(r)
        del mm
        ok = rec["shots"] >= 1 and rec["flagged"] == rec["shots"] and all(p["px"] == 0 for p in rec["pairs"])
        rec["verdict"] = "PASS" if ok else "FAIL"
        if not ok:
            report["verdict"] = "FAIL"
        report["dumps"].append(rec)
        worst = max((p["px"] for p in rec["pairs"]), default=-1)
        print(f"  {stem}: mode {info['mode']}, {info['frames']} frames {info['w']}x{info['h']}, flagged {len(flagged)}, "
              f"shots {len(ks)}, worst pair {worst} px -> {rec['verdict']}")
    npairs = sum(len(r["pairs"]) for r in report["dumps"])
    nbad = sum(1 for r in report["dumps"] for p in r["pairs"] if p["px"] != 0)
    print(f"recorder-check: {report['verdict']} {len(report['dumps'])} dumps, {npairs} pairs, {nbad} pairs with differing pixels")
    if o.json:
        gl.write_json(o.json, report)
    return 0 if report["verdict"] == "PASS" else 1


# ----------------------------------------------------------------------------- encode

def decode_frames(src, w, h):
    """Yields top-down RGB frames of an ffmpeg-readable clip."""
    p = subprocess.Popen([gl.ffmpeg(), "-hide_banner", "-loglevel", "error", "-i", src, "-f", "rawvideo", "-pix_fmt",
                          "rgb24", "-"], stdout=subprocess.PIPE)
    n = w * h * 3
    try:
        while True:
            b = p.stdout.read(n)
            if len(b) < n:
                break
            yield np.frombuffer(b, dtype=np.uint8).reshape(h, w, 3)
    finally:
        p.stdout.close()
        p.wait()


def clip_size(src):
    probe = os.path.join(os.path.dirname(gl.ffmpeg()), "ffprobe.exe" if gl.ffmpeg().lower().endswith(".exe") else "ffprobe")
    out = subprocess.run([probe, "-v", "error", "-select_streams", "v:0", "-count_packets", "-show_entries",
                          "stream=width,height,nb_read_packets", "-of", "json", src], capture_output=True, text=True,
                         check=True).stdout
    s = json.loads(out)["streams"][0]
    return int(s["width"]), int(s["height"]), int(s["nb_read_packets"])


def pacing(info):
    rows = info["frame_rows"]
    wall = np.array([r["wall_ms"] for r in rows])
    pt = np.array([r["path_t"] for r in rows])
    dw = np.diff(wall) if len(wall) > 1 else np.zeros(1)
    dp = np.diff(pt) if len(pt) > 1 else np.zeros(1)
    return dict(frames=len(rows), wall_ms=round(float(wall[-1] - wall[0]), 1) if len(wall) else 0,
                fps=round(float((len(wall) - 1) / max(1e-6, (wall[-1] - wall[0]) / 1000.0)), 2) if len(wall) > 1 else 0,
                dwall_ms=dict(min=round(float(dw.min()), 2), med=round(float(np.median(dw)), 2), max=round(float(dw.max()), 2)),
                dpath_ms=dict(min=round(float(dp.min() * 1000), 3), med=round(float(np.median(dp) * 1000), 3),
                              max=round(float(dp.max() * 1000), 3)),
                zero_path_steps=int((dp == 0).sum()), shots=int(sum(r["shot"] for r in rows)),
                path_first=round(float(pt[0]), 4) if len(pt) else 0, path_last=round(float(pt[-1]), 4) if len(pt) else 0)


def cmd_encode(o):
    raw = o.raw
    info = read_sidecar(raw)
    w, h, n = info["w"], info["h"], info["frames"]
    out = o.out
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
    subprocess.run([gl.ffmpeg(), "-hide_banner", "-loglevel", "error", "-y", "-f", "rawvideo", "-pix_fmt", "rgb24",
                    "-s", f"{w}x{h}", "-framerate", str(o.fps), "-i", raw, "-vf", "vflip", "-c:v", "ffv1", "-level", "3",
                    "-g", "1", "-slicecrc", "1", "-pix_fmt", "bgr0", out], check=True)
    rec = dict(src=os.path.abspath(raw), mkv=os.path.abspath(out), w=w, h=h, frames=n, mode=info["mode"], half=info["half"],
               codec="ffv1 level 3 bgr0 (lossless)", fps=o.fps, pacing=pacing(info), verified=None)
    if not o.no_verify:
        mm = np.memmap(raw, dtype=np.uint8, mode="r")
        k = 0
        bad = 0
        for img in decode_frames(out, w, h):
            if k >= n or not np.array_equal(img, frame(mm, info, k)):
                bad += 1
            k += 1
        del mm
        rec["verified"] = dict(decoded=k, mismatched=bad, ok=bool(k == n and bad == 0))
        if not rec["verified"]["ok"]:
            gl.write_json(os.path.splitext(out)[0] + ".pacing.json", rec)
            print(f"recorder-encode: VERIFY FAILED {out}: decoded {k} of {n}, {bad} frames differ (dump kept)")
            return 1
    gl.write_json(os.path.splitext(out)[0] + ".pacing.json", rec)
    if not o.keep_raw and (o.no_verify is False) and os.path.getsize(out) > 0:
        os.remove(raw)
        rec["raw_deleted"] = True
    print(f"recorder-encode: {out} {n} frames {w}x{h}, verified={rec['verified']}, "
          f"{rec['pacing']['fps']} fps recorded, raw {'deleted' if rec.get('raw_deleted') else 'kept'}")
    return 0


# ----------------------------------------------------------------------------- diff

def frames_of(src):
    if src.endswith(".raw"):
        info = read_sidecar(src)
        mm = np.memmap(src, dtype=np.uint8, mode="r")
        return info["frames"], (frame(mm, info, i) for i in range(info["frames"]))
    w, h, n = clip_size(src)
    return n, decode_frames(src, w, h)


def cmd_diff(o):
    na, fa = frames_of(o.a)
    nb, fb = frames_of(o.b)
    per = []
    for i, (x, y) in enumerate(zip(fa, fb)):
        r = compare(x, y)
        r["frame"] = i
        per.append(r)
    diff = [r for r in per if r["px"] != 0]
    rep = dict(a=o.a, b=o.b, frames_a=na, frames_b=nb, compared=len(per), differing_frames=len(diff),
               max_px=max((r["px"] for r in per), default=0), max_level=max((r["max"] for r in per), default=0),
               first_differing=diff[0]["frame"] if diff else None, verdict="identical" if not diff and na == nb else "differ",
               per_frame=per)
    if o.json:
        gl.write_json(o.json, rep)
    print(f"recorder-diff: {rep['verdict']}: {len(per)} frames compared ({na} vs {nb}), {len(diff)} differ, "
          f"max {rep['max_px']} px, max level {rep['max_level']}")
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    c = sub.add_parser("check")
    c.add_argument("dir")
    c.add_argument("--json", default="")
    e = sub.add_parser("encode")
    e.add_argument("raw")
    e.add_argument("--out", required=True)
    e.add_argument("--fps", type=int, default=60)
    e.add_argument("--keep-raw", action="store_true")
    e.add_argument("--no-verify", action="store_true")
    d = sub.add_parser("diff")
    d.add_argument("a")
    d.add_argument("b")
    d.add_argument("--json", default="")
    o = ap.parse_args()
    return {"check": cmd_check, "encode": cmd_encode, "diff": cmd_diff}[o.cmd](o)


if __name__ == "__main__":
    sys.exit(main())
