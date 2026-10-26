"""Timing-only test for track "tiling": on fixed Abel views, alternate the
stock shader (v0) and prototype variants several times, reading the GPU timers
after each settle period (no screenshots). Interleaving makes the delta robust
against other agents' games loading the GPU at the same time.

    python gen_timing.py --variants 0,7,9 --reps 4
"""
import argparse
from pathlib import Path

WT = Path(r"D:\Arma_CWA_worktrees\lighting\arma_CWR")
DST = WT / "tests/rendering/research/tiling"

VIEWS = [
    ("noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500),
    ("high_fields_noon", 6250, 10800, 60, 165, -14, [1985, 7, 5, 12, 30], 0, 0, 2500),
    ("forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500),
]

HEAD = '''#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Abel"
capFov = 0.7
capSimT = 0
capPinTime = false
capUseView = false
capProps = []
capView = "setup"
capModeName = "none"
capLit = "none"
capGpu = "none"
capPerf = "none"

triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]
triSimFrames 240
'''

TOML = '''binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_abel.Abel"
tags = ["lighting_capture", "headful", "exclusive"]
timeout = 900
assert_timeout = 240
extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]
'''


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--variants", default="0,7,9")
    ap.add_argument("--reps", type=int, default=4)
    a = ap.parse_args()
    ks = [int(k) for k in a.variants.split(",")]
    body = HEAD
    for v in VIEWS:
        vid, x, z, eye, h, p, date, oc, fog, vd = v
        body += f'\n["{vid}", {x}, {z}, {eye}, {h}, {p}, {date}, {oc}, {fog}, {vd}] call capBegin\ntriSimFrames 150\n'
        body += '"classic" call capMode\ntriSimFrames 120\n[] call capReadTimers\n'
        body += f'"{vid}__classic" call capRecord\n'
        body += '"remastered" call capMode\n'
        for r in range(a.reps):
            for k in ks:
                body += f'{k} call capVar\ntriSimFrames 120\n[] call capReadTimers\n"{vid}__r{r}__v{k}" call capRecord\n'
        body += "0 call capVar\n"
    body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
    (DST / "tiling_timing.test.sqf").write_text(body, encoding="utf-8", newline="\n")
    (DST / "tiling_timing.test.toml").write_text(TOML, encoding="utf-8", newline="\n")
    print("wrote", DST / "tiling_timing.test.sqf")


if __name__ == "__main__":
    main()
