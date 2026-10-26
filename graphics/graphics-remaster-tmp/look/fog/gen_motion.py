"""look-fog lane motion clips (critic round 1: fast pans and moving headlights against the
volume's 12 % history blend). Writes tests/video/<clip>.test.sqf with gen_video_tests.py's
S mode: the whole path with volFog=0 (the landed B8 frame), then with the fog.

    python gen_motion.py [--frames N]
"""
import argparse
import shutil
import sys
from pathlib import Path

CAP = Path(r"D:\Arma_CWA_worktrees\look-fog\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAP))
sys.path.insert(0, str(CAP / "video"))
import gen_video_tests as gv  # noqa: E402

HERE = Path(__file__).resolve().parent
OUT = HERE / "tests" / "video"

PINE = [6830, 9905, 1.75, 40, 6]
TOWN = [7152, 8915, 1.75, 345, 3]

CLIPS = [
    # A fast pan across the sun through the pine wood: 70 degrees in 24
    # frames (about 3 degrees a frame).
    dict(clip="fog_pan_pine", feature="fog", keys=["volFog"], world="Abel", mode="S", frames=24,
         date=[1985, 7, 5, 6, 0], vd=1500, fog=0.1, stage=gv.stage_of(PINE),
         path=gv.line(PINE, [6830, 9905, 1.75, 110, 6])),
    # A Jeep with its lights on driving at the camera up the foggy town street
    # at about a metre a frame.
    dict(clip="fog_jeep_town", feature="fog", keys=["volFog"], world="Abel", mode="S", frames=30,
         date=[1985, 2, 27, 22, 0], vd=800, fog=0.35, stage=gv.stage_of(TOWN), path=gv.line(TOWN, TOWN),
         props=gv.jeep_with_driver("capMover1", 40, 1.5, 180),
         movers=[("capMover1", [[0, 40, 1.5, 180], [29, 10, 1.5, 180]])]),
]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--frames", type=int, default=0)
    a = ap.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    shutil.copy(CAP / "video" / "capture_motion.sqf", OUT / "capture_motion.sqf")
    for c in CLIPS:
        n = gv.write_clip(c, a.frames, OUT)
        print(c["clip"], n, "frames")


if __name__ == "__main__":
    main()
