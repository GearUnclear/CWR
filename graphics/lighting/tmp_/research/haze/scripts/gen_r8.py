"""Round 8 for the haze track: the sky-edge seam fix (variants 39..49 = 29..39 + fix).

haze_abel_r8: valley at 05:15 (sunrise mist) and 06:30, sunrise_ridge; classic,
current Remastered (v0), v43 (= v33 + seam fix) and v47 (= v37 + seam fix).

python gen_r8.py <out_dir>
"""
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
out = sys.argv[1]
src = open(os.path.join(here, "gen_tests.py"), encoding="utf-8").read()
HEADER = src.split('HEADER = """', 1)[1].split('"""', 1)[0].replace("{{", "{").replace("}}", "}")
HEADER = HEADER.replace("{world}", "Abel").replace("{px}", "7300").replace("{pz}", "10900")
TOML = """binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_abel.Abel"
tags = ["lighting_capture", "headful", "exclusive"]
timeout = 900
assert_timeout = 180
extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]
"""


def shot(view, label, stmt):
    return (f"{stmt}\ntriSimFrames 90\n[] call capReadTimers\n[] call capPin\ntriSimFrames 3\n"
            f'triScreenshot "{view}__{label}"\n"{view}__{label}" call capRecord\n')


VIEWS = [
    ("valley_t0515", '["valley_t0515", 6100, 5000, 1.75, 50, -6, [1985, 7, 5, 5, 15], 0, 0, 4000] call capBegin'),
    ("valley_morning", '["valley_morning", 6100, 5000, 1.75, 50, -6, [1985, 7, 5, 6, 30], 0, 0, 4000] call capBegin'),
    ("sunrise_ridge", '["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin'),
]
s = HEADER
for view, begin in VIEWS:
    s += f"\n// ---- {view}\n{begin}\ntriSimFrames 150\n"
    s += shot(view, "classic", '"classic" call capMode')
    s += shot(view, "haze_v0", "0 call capVar")
    s += shot(view, "haze_v43", "43 call capVar")
    s += shot(view, "haze_v47", "47 call capVar")
s += '\ncall { capSat = triSetLighting "saturation=1.12" }\ncall { "view=end;event=done" call capLog }\ntriEndTest\n'
with open(os.path.join(out, "haze_abel_r8.test.sqf"), "w", newline="\n") as f:
    f.write(s)
with open(os.path.join(out, "haze_abel_r8.test.toml"), "w", newline="\n") as f:
    f.write(TOML)
print("wrote haze_abel_r8")
