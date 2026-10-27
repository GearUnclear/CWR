"""Collect a v2 clip from its run-captures.ps1 output into one near-lossless
video per side, tmp/video/v2/<side>/<clip>/<clip>__<side>.mkv, plus stills.

    python collect_frames.py <run dir> [--side vanilla|ud] [--clip <clip>] [--gamma 1.2] [--keep-raw] [--expect N]

The clips are shot at 60 frames a second (gen_video_v2.FPS); 60 fps PNGs of
both sides would need about 50 GB, so the frames go into an RGB H.264 file
(libx264rgb, CRF 4: visually lossless, a fraction of the PNGs) that
make_video_v2.py decodes. The vanilla side gets the vanilla engine's gamma
pass first: vanilla triScreenshot reads the last rendered frame before
ApplyGammaPass (pow(c, 1/gamma), default 1.2; tmp/look/ref/NOTES.md), the
fork's triCaptureAtPresent reads the presented frame after it.

Also writes <clip>/first.png, mid.png, last.png (as they go into the video)
and <clip>/collect.json (frame count, size, video, and the proof of where the
frames come from: the game dir of the run's tri command and the clip's
event=mode line from the game log). A side whose proof does not match is
refused: the ud side must come from REQUIRE["ud"] with "mode=remastered;lit=OK",
the vanilla side from REQUIRE["vanilla"] with "mode=vanilla". The raw tri PNGs
are deleted after a complete collection unless --keep-raw. Exit 1 when frames
are missing or the proof fails.
"""
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import udpaths  # noqa: E402  (where things live; see udpaths.py)
V2 = udpaths.WORK
# Which game and lighting mode each side must come from (substring of the run's --game-dir, mode line).
REQUIRE = {"ud": ("look-director\\arma_CWR\\dist\\look-director-rwdi", "mode=remastered;lit=OK"),
           "vanilla": ("vanilla-upstream\\dist\\vanilla-upstream-rwdi", "mode=vanilla;lit=vanilla")}


def proof(run, clip):
    """(game dir, mode line) of a run: from run.txt's tri command and the game log."""
    game = ""
    rt = run / "run.txt"
    if rt.exists():
        words = rt.read_text(encoding="utf-8-sig", errors="replace").split()
        if "--game-dir" in words:
            game = words[words.index("--game-dir") + 1]
    mode = ""
    for log in run.rglob("game_stdout.log"):
        for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
            if f"view={clip};event=mode;" in line:
                mode = line[line.index("view="):].rstrip('"').strip()
    return game, mode
FFMPEG = udpaths.FFMPEG


def lut(g):
    one = [min(255, max(0, round(255.0 * ((i / 255.0) ** (1.0 / g))))) for i in range(256)]
    return one * 3


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("run")
    ap.add_argument("--side", choices=("vanilla", "ud"), default="vanilla")
    ap.add_argument("--clip", default="")
    ap.add_argument("--gamma", type=float, default=1.2)
    ap.add_argument("--keep-raw", action="store_true")
    ap.add_argument("--expect", type=int, default=0, help="frames the clip should have (0: from gen_video_v2)")
    a = ap.parse_args(argv)
    import gen_video_v2  # noqa: E402
    run, side = Path(a.run), a.side
    shot_re = re.compile(r"^(\d{3,})_(.+)__" + side + r"__f(\d{4})\.png$")
    found = {}
    for p in run.rglob("*.png"):
        m = shot_re.match(p.name)
        if not m:
            continue
        clip, f = m.group(2), int(m.group(3))
        if a.clip and clip != a.clip:
            continue
        slot = found.setdefault(clip, {})
        if f not in slot or p.stat().st_mtime > slot[f].stat().st_mtime:
            slot[f] = p
    if not found:
        print(f"no {side} frames under {run}")
        return 1
    table = lut(a.gamma)
    bad = 0
    for clip, frames in found.items():
        n = a.expect or gen_video_v2.clip_frames(gen_video_v2.clip_by_name(clip))
        missing = [f for f in range(1, n + 1) if f not in frames]
        dst = V2 / side / clip
        dst.mkdir(parents=True, exist_ok=True)
        if missing:
            print(f"{clip}: {len(frames)}/{n} frames, missing {len(missing)} (first {missing[:5]}); not encoded")
            bad += 1
            continue
        game, mode = proof(run, clip)
        need_game, need_mode = REQUIRE[side]
        if need_game.lower() not in game.lower() or need_mode not in mode:
            print(f"{clip}: REFUSED, the {side} side must come from {need_game} with {need_mode}; "
                  f"this run: game {game!r}, mode {mode!r}")
            bad += 1
            continue
        w, h = Image.open(frames[1]).size
        video = dst / f"{clip}__{side}.mkv"
        enc = subprocess.Popen([FFMPEG, "-y", "-loglevel", "error", "-f", "rawvideo", "-pix_fmt", "rgb24", "-s",
                                f"{w}x{h}", "-framerate", str(gen_video_v2.FPS), "-i", "-", "-c:v", "libx264rgb",
                                "-crf", "4", "-preset", "fast", str(video)], stdin=subprocess.PIPE)
        stills = {1: "first.png", n // 2 + 1: "mid.png", n: "last.png"}
        for f in range(1, n + 1):
            im = Image.open(frames[f]).convert("RGB")
            if im.size != (w, h):
                im = im.resize((w, h))
            if side == "vanilla":
                im = im.point(table)
            enc.stdin.write(im.tobytes())
            if f in stills:
                im.save(dst / stills[f], compress_level=3)
        enc.stdin.close()
        if enc.wait() != 0:
            print(f"{clip}: ffmpeg failed")
            bad += 1
            continue
        info = dict(clip=clip, side=side, frames=n, fps=gen_video_v2.FPS, size=[w, h], video=str(video),
                    game_dir=game, mode_line=mode,
                    gamma=a.gamma if side == "vanilla" else None, run=str(run),
                    mb=round(video.stat().st_size / 1e6, 1))
        (dst / "collect.json").write_text(json.dumps(info, indent=1), encoding="utf-8")
        print(f"{clip}: {n}/{n} frames {w}x{h} -> {video} ({info['mb']} MB)")
        if not a.keep_raw:
            for p in frames.values():
                p.unlink(missing_ok=True)
                bmp = p.with_suffix(".bmp")
                if bmp.exists():
                    bmp.unlink(missing_ok=True)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
