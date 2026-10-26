"""B9 lane motion check (S mode: the whole off pass, then the on pass):

  b9walk   the hfrt hero vantage (hill_1730, eye 15 m) moving 0.5 m a frame
           along its heading for 30 frames: does anything crawl or swim in the
           far canopy shadows as the camera moves?
  b9lapse  the telephoto vantage held still while the clock runs 60 s a frame
           (the sun moves about 0.2 deg a frame, so the sun-visibility map is
           rebuilt every frame): do the shadows step or pop?

    python gen_b9motion.py
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B9\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
sys.path.insert(0, str(CAPTURES / "video"))
import capgen  # noqa: E402
import gen_video_tests as gv  # noqa: E402
import views  # noqa: E402

KEYS = ["canopyShadows"]
CLIPS = [
    dict(clip="b9walk", feature="hfrt", keys=KEYS, world="Abel", mode="S", frames=30, date=[1985, 9, 20, 17, 30],
         vd=2000, stage=gv.stage_of(gv.A_HILL), path=gv.line(gv.A_HILL, gv.along(gv.A_HILL, 0.5 * 29))),
    dict(clip="b9lapse", feature="hfrt", keys=KEYS, world="Abel", mode="S", frames=30, date=[1985, 9, 20, 17, 20],
         vd=2000, stage=gv.stage_of(gv.A_SLOPE), path=gv.line(gv.A_SLOPE, gv.A_SLOPE), lapse=60),
]


def main():
    title = "B9 lane motion: a walk at the hfrt hero vantage and a time-lapse on the slope, off pass then on pass"
    lines = [capgen.header("Abel", CLIPS[0]["stage"], title, include="../capture_lib.sqf",
                           extra_includes=("capture_motion.sqf",))]
    lines.append("capEase = { _this }")
    for c in CLIPS:
        lines += [f"\n// ---- {c['clip']}"]
        lines += gv.stage_block(c, c["frames"])
        lines += gv.s_mode(c, c["frames"])
        lines += capgen.end(c)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests" / "video"
    out.mkdir(parents=True, exist_ok=True)
    capgen.write(out / "b9motion.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b9motion.test.toml", capgen.toml("Abel", title, views.WORLDS, timeout=3600, assert_timeout=180))
    (out / "capture_motion.sqf").write_bytes((CAPTURES / "video" / "capture_motion.sqf").read_bytes())
    print(out / "b9motion.test.sqf")


main()
