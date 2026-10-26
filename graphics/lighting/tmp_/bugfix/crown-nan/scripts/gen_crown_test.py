"""Writes the crown-nan research capture test (Abel): four tree views, each shot
Remastered and in the lighting debug view 3 (the lighting normal, or the NaN
classification when the diagnostic shader override is loaded)."""
import sys
from pathlib import Path

out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)

views = [
    ("forest_edge_golden", "4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500",
     '[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;'),
    ("woods_mixed_noon", "2864, 6656, 1.75, 300, -5, [1985, 7, 5, 14, 0], 0, 0, 1500",
     '[["SoldierWB"], 7, 1, 160, "man"] call capSpawn;'),
    ("woods_pine2_evening", "6940, 9964, 1.75, 240, -5, [1985, 7, 5, 17, 30], 0, 0, 1500",
     '[["SoldierWB"], 8, -1, 200, "man"] call capSpawn;'),
    ("town_day", "7152, 8915, 1.75, 345, 3, [1985, 7, 5, 16, 30], 0, 0, 1200",
     '[["Jeep"], 14, 4, 80, "veh"] call capSpawn; [["SoldierWB"], 9, -1, 170, "man"] call capSpawn;'),
]

head = '''// crown-nan research capture (generated; research only, git-excluded)
#include "capture_lib.sqf"

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
capDbg = "none"

triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present;run=crown-nan" call capLog }
'''

body = []
for name, args, spawn in views:
    body.append(f'''
// ---- {name}
["{name}", {args}] call capBegin
call {{ {spawn} }}
triSimFrames 150
"remastered" call capMode
call {{ capDbg = triSetLighting "debug=0" }}
triSimFrames 90
[] call capReadTimers
[] call capPin
triSimFrames 3
triScreenshot "{name}__remastered"
"{name}__remastered" call capRecord
call {{ capDbg = triSetLighting "debug=3" }}
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "{name}__dbg3"
"{name}__dbg3" call capRecord
call {{ capDbg = triSetLighting "debug=0" }}
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "{name}__late"
"{name}__late" call capRecord
[] call capClear
''')

tail = '''
triEndTest
'''
(out / "crown_nan_abel.test.sqf").write_text(head + "".join(body) + tail, newline="\n")
(out / "crown_nan_abel.test.toml").write_text('''# crown-nan research capture (generated; research only)
binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_abel.Abel"
tags = ["lighting_capture", "headful", "exclusive"]
timeout = 1800
assert_timeout = 180
extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]
''', newline="\n")
print("ok")
