"""sheet.py DIR OUT.png : the director's contact sheet from a collected folder."""
import subprocess
import sys
from pathlib import Path

d, out = Path(sys.argv[1]), sys.argv[2]
order = ["lowsun_fp", "sunrise_ridge", "noon_fields", "forest_edge_golden", "coastline_sunset", "town_night",
         "firefight_1", "firefight_2", "firefight_3", "tank_gunner_interior", "tank_gunner_optics",
         "overcast_broken", "overcast_heavy", "sinai_lowsun_fp", "sinai_desert_noon",
         "woods_pine2_evening", "blue_hour_coast", "dawn_valley", "pine_rays", "town_fog_night", "town_night_rain",
         "jeep_close_noon", "town_noon", "forest_morning", "rain_day", "noon_vista",
         "optics_mist_hud", "optics_mist_sight"]
pairs = []
for v in order:
    c, o = d / f"{v}__classic.png", d / f"{v}__overhaul.png"
    if c.exists() and o.exists():
        pairs.append(f"{v}={c},{o}")
    else:
        print("missing", v)
tool = r"D:\Arma_CWA_worktrees\look-director\arma_CWR\tests\rendering\captures\make_look_sheet.py"
subprocess.run([sys.executable, tool, out, *pairs, "--panel", "480", "--per-row", "2"], check=True)
print(out, len(pairs))
