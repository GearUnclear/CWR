"""Screen every v2 clip pair for the same band flicker: bandmetric on the ud and the vanilla mkv,
default band (rows 8-13, cols 5-31) and the wide lower band (rows 6-17, cols 0-31).
usage: python screen_clips.py > screen.txt   (plain python)"""
import json, subprocess, sys
from pathlib import Path
V2 = Path("D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2")
BM = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/mist/tools/bandmetric.py"
clips = sorted(p.name for p in (V2 / "ud").iterdir() if p.is_dir())
print("clip | band ud / van (ratio) spikes ud / van | wide ud / van (ratio) spikes ud / van")
for c in clips:
    row = [c]
    for band in ("8,13,5,31", "6,17,0,31"):
        r = {}
        for side in ("ud", "vanilla"):
            mkv = V2 / side / c / f"{c}__{side}.mkv"
            if not mkv.exists():
                r[side] = None
                continue
            p = subprocess.run([sys.executable, BM, str(mkv), "--band", band], capture_output=True, text=True)
            js = [l for l in p.stdout.splitlines() if l.startswith("{")]
            r[side] = json.loads(js[-1]) if js else None
        u, v = r["ud"], r["vanilla"]
        if u and v:
            row.append(f"{u['band']:.3f} / {v['band']:.3f} ({u['band'] / max(v['band'], 1e-6):.1f}x) {u['spikes']} / {v['spikes']}")
        else:
            row.append("missing")
    print(" | ".join(row), flush=True)
