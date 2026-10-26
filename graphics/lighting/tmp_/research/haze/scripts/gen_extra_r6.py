"""Round 6 extras for the haze track (research only, git-excluded test dir).

haze_tod_r6:    valley_morning at five times of day (05:15 .. 08:00), current
                Remastered (v0) and haze v33, to show the morning mist forming,
                peaking and burning off with the sun height.
haze_timing_r6: no screenshots; four interleaved rounds of classic / v0 / v31 /
                v33 on sunrise_ridge and valley_morning, timers read after 60
                frames each. The machine runs up to four games at once, so the
                per-round numbers are noisy; the minimum over rounds is the
                least contended estimate.

python gen_extra_r6.py <out_dir>
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
out = sys.argv[1]

HEADER = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "gen_tests.py"), encoding="utf-8").read()
HEADER = HEADER.split('HEADER = """', 1)[1].split('"""', 1)[0]
HEADER = HEADER.replace("{{", "{").replace("}}", "}")

TOML = """binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_abel.Abel"
tags = ["lighting_capture", "headful", "exclusive"]
timeout = 900
assert_timeout = 180
extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]
"""


def header():
    return HEADER.replace("{world}", "Abel").replace("{px}", "7300").replace("{pz}", "10900")


def shot(view, label, mode_stmt):
    return (f"{mode_stmt}\ntriSimFrames 90\n[] call capReadTimers\n[] call capPin\ntriSimFrames 3\n"
            f'triScreenshot "{view}__{label}"\n"{view}__{label}" call capRecord\n')


# ---- time of day
s = header()
for hh, mm in ((5, 15), (6, 0), (6, 30), (7, 15), (8, 0)):
    view = f"valley_t{hh:02d}{mm:02d}"
    s += f"\n// ---- {view}\n"
    s += f'["{view}", 6100, 5000, 1.75, 50, -6, [1985, 7, 5, {hh}, {mm}], 0, 0, 4000] call capBegin\n'
    s += "triSimFrames 150\n"
    s += shot(view, "haze_v0", "0 call capVar")
    s += shot(view, "haze_v33", "33 call capVar")
s += '\ncall { capSat = triSetLighting "saturation=1.12" }\ncall { "view=end;event=done" call capLog }\ntriEndTest\n'
with open(os.path.join(out, "haze_tod_r6.test.sqf"), "w", newline="\n") as f:
    f.write(s)
with open(os.path.join(out, "haze_tod_r6.test.toml"), "w", newline="\n") as f:
    f.write(TOML)

# ---- timing
s = header()
VIEWS = [
    ("sunrise_ridge", '["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin'),
    ("valley_morning", '["valley_morning", 6100, 5000, 1.75, 50, -6, [1985, 7, 5, 6, 30], 0, 0, 4000] call capBegin'),
]
for view, begin in VIEWS:
    s += f"\n// ---- {view} (timing)\n{begin}\ntriSimFrames 150\n"
    for r in range(4):
        for mode, stmt in (("classic", '"classic" call capMode'), ("v0", "0 call capVar"),
                           ("v31", "31 call capVar"), ("v33", "33 call capVar")):
            s += f"{stmt}\ntriSimFrames 60\n[] call capReadTimers\n\"{view}__t{r}_{mode}\" call capRecord\n"
s += '\ncall { capSat = triSetLighting "saturation=1.12" }\ncall { "view=end;event=done" call capLog }\ntriEndTest\n'
with open(os.path.join(out, "haze_timing_r6.test.sqf"), "w", newline="\n") as f:
    f.write(s)
with open(os.path.join(out, "haze_timing_r6.test.toml"), "w", newline="\n") as f:
    f.write(TOML)
print("wrote haze_tod_r6, haze_timing_r6")
