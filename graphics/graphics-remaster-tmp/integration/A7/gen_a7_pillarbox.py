"""A7 pillarbox check: the sky table's view ray under a cropped world rect.

    python gen_a7_pillarbox.py <captures dir> [--name a7_pillarbox]

coastline_sunset (heading 250) with the world cropped to the middle 4:3 of the
1920x1080 window (triSetManualViewport [0.125, 0, 0.875, 1]), shot off then on,
then the crop cleared. With the view ray right, the aureole sits on the sun disc
and the dome meets the horizon polygon without a step, as in the full frame.
"""
import argparse
import sys
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("captures")
ap.add_argument("--name", default="a7_pillarbox")
a = ap.parse_args()
sys.path.insert(0, a.captures)
import capgen  # noqa: E402
import views  # noqa: E402

v = None
for t in views.TIMING_ABEL:
    if t["name"] == "coastline_sunset":
        v = dict(t)
v.update(name="atmosphere_pillarbox", h=250)
v.pop("note", None)

lines = [capgen.header("Abel", v, "A7 pillarbox: the sky table under a cropped world rect.")]
lines += capgen.begin(v, v["name"], False, sim_time=2000)
lines.append('call { capVp = triSetManualViewport [0.125, 0, 0.875, 1]; '
             'format ["view=%1;event=viewport;res=%2;world=%3", capView, capVp, triGetWorldViewport] call capLog }')
lines.append("triSimFrames 150")
lines += capgen.shot(v, v["name"], "remastered,atmosphere=0", f"{v['name']}__off")
lines += capgen.shot(v, v["name"], "remastered", f"{v['name']}__on")
lines.append('call { capVp = triSetManualViewport [] }')
lines += capgen.end(v, False)
lines.append('\ncall { "view=done;event=done" call capLog }')
lines.append("triEndTest")
out = Path(a.captures)
capgen.write(out / f"{a.name}.test.sqf", "\n".join(lines) + "\n")
capgen.write(out / f"{a.name}.test.toml", capgen.toml("Abel", "A7 pillarbox", views.WORLDS, timeout=1800))
print(f"{a.name}: 1 view")
