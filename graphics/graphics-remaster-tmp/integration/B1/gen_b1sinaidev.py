"""B1 Sinai diagnosis: which part of softShadows draws the foreground slab.

    python gen_b1sinaidev.py  ->  tests/b1sinaidev.test.sqf
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B1\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

v = dict([x for x in views.TIMING_SINAI if x["name"] == "sinai_lowsun_fp"][0])
name = "sdev"
lines = [capgen.header("Sinai", v, "B1 Sinai slab diagnosis.")]
lines += capgen.begin(v, name, sim_time=2600)
lines.append("triSimFrames 150")
for label, setting in [
    ("stocktiers_stock", "remastered,softShadows=0,debug=1"),
    ("tight_stock", "remastered,sunSize=0,debug=1"),
    ("tight_pcss", "remastered,debug=1"),
    ("tight_pcss_notaa", "remastered,taa=0,debug=1"),
    ("tight_pcss_reach10", "remastered,shadowReach=10,debug=1"),
    ("tight_pcss_notree", "remastered,treeShadows=0,debug=1"),
]:
    lines += capgen.shot(v, name, setting, f"{name}__{label}")
lines += capgen.end(v)
lines.append('\ncall { "view=done;event=done" call capLog }')
lines.append("triEndTest")
out = HERE / "tests"
capgen.write(out / "b1sinaidev.test.sqf", "\n".join(lines) + "\n")
capgen.write(out / "b1sinaidev.test.toml", capgen.toml("Sinai", "B1 Sinai diagnosis", views.WORLDS, timeout=3600))
print(out / "b1sinaidev.test.sqf")
