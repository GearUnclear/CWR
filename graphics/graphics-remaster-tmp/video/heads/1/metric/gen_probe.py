"""Agent 1 head-look probe generator (look_probe_head1_* / van_probe_head1_*).

8 men in 4 groups of 2 in one static shot, 7.5 m from a fixed camera:
  G1 SAFE standing, plain capSpawn "man"              (as the clips)
  G2 SAFE standing + doWatch a far point straight ahead
  G3 COMBAT kneeling (MIDDLE), plain                  (as ambush/rain_town)
  G4 COMBAT kneeling + doWatch a far point straight ahead
In each group man 1 faces the camera, man 2 faces 45 deg to its right; the
group formation direction stays north (AISubgroup default), and nobody faces north.
Per frame: clock step, camera pin, triSimFrames 2, screenshot (as the clips).
"""
import sys

N = int(sys.argv[2]) if len(sys.argv) > 2 else 120
side = sys.argv[1]  # ud | vanilla
variant = sys.argv[3] if len(sys.argv) > 3 else "a"
out = sys.argv[4]

CAM_X, CAM_Z, CAM_H = 7300, 10900, 30.0
FWD = 7.5
# (group, right offset, dir relative to the view heading, pose, behaviour, watch)
MEN = [
    ("capGrpH1", -3.9, 180, "UP", "SAFE", False),
    ("capGrpH1", -2.8, 135, "UP", "SAFE", False),
    ("capGrpH2", -1.5, 180, "UP", "SAFE", True),
    ("capGrpH2", -0.4, 135, "UP", "SAFE", True),
    ("capGrpH3", 0.9, 180, "MIDDLE", "COMBAT", False),
    ("capGrpH3", 2.0, 135, "MIDDLE", "COMBAT", False),
    ("capGrpH4", 3.3, 180, "MIDDLE", "COMBAT", True),
    ("capGrpH4", 4.4, 135, "MIDDLE", "COMBAT", True),
]

L = []
a = L.append
a(f"// agent 1 head-look probe ({side}, variant {variant}): 8 posed men, static camera, {N} frames at 60 fps")
a('#include "../capture_lib.sqf"')
a('#include "capture_motion.sqf"')
a("")
a("triSimUntil { alive player }")
a("")
for l in ['capWorld = "Abel"', "capFov = 0.7", "capSimT = 0", "capPinTime = false", "capUseView = false",
          "capProps = []", 'capView = "setup"', 'capModeName = "none"', 'capLit = "none"', 'capGpu = "none"',
          'capPerf = "none"']:
    a(l)
a("")
if side == "ud":
    a("triCaptureAtPresent true")
a("triSetMsaa 4")
a("triSetRenderScale 1.0")
a("showCinemaBorder false")
a("enableRadio false")
a("player setCaptive true")
a("player allowDamage false")
a("createCenter west")
a("createCenter east")
a("createCenter resistance")
a("capGrpW = createGroup west")
for g in ["capGrpH1", "capGrpH2", "capGrpH3", "capGrpH4"]:
    a(f"{g} = createGroup resistance")
a(f"capProbe = \"HeliHEmpty\" createVehicle [{CAM_X}, {CAM_Z}, 0]")
a(f"capCam = \"camera\" camCreate [{CAM_X}, {CAM_Z}, 2]")
a('capCam cameraEffect ["internal", "back"]')
a("triSimFrames 120")
a("triSimFrames 120")
a("capFps = 60")
a("capH1Men = []")
a("call { capRain = 0; capEyeAbs = false }")
a(f'["head1", {CAM_X}, {CAM_Z}, 1.5, {CAM_H}, -4, [1985, 7, 5, 12, 0], 0, 0, 1000] call capBegin')
a("call { capSimT = 2000; capSimRes = triSetSimTime capSimT }")
blk = ["call {"]
for i, (g, r, rd, pose, beh, watch) in enumerate(MEN):
    blk.append(f'    capU = [["SoldierGB"], {FWD}, {r}, {rd}, "man", {g}] call capSpawn;')
    if pose != "UP" or beh != "SAFE":
        blk.append(f'    capU setUnitPos "{pose}"; capU setBehaviour "{beh}";')
    if watch:
        # a far point straight ahead of the man's own facing, at eye height
        blk.append(f"    capWD = getDir capU; capWP = getPos capU; capU doWatch [(capWP select 0) + 300 * (sin capWD), (capWP select 1) + 300 * (cos capWD), 1.6];")
    blk.append("    capH1Men = capH1Men + [capU];")
blk.append("}")
L += blk
a("call { capPathKind = \"line\"; capPathA = [" + f"{CAM_X}, {CAM_Z}, 1.5, {CAM_H}, -4" + "]; capPathB = capPathA; capN = " + str(N) + "; capLapse = 0 }")
if side == "ud":
    a("call { capSettleArgs = [150, 30, 2] }")
    a("triSimFrames 90")
    a("triSimFrames 90")
    a('"remastered" call capMode')
else:
    a("call { capSettleFrames = 90 }")
    a("triSimFrames 90")
    a("triSimFrames 90")
    a("triSimFrames 90")
    a('"vanilla" call capMode')
a("[] call capPin")
for k in range(N):
    lab = f"head1{variant}__{side}__f{k+1:04d}"
    a(f"[{k}] call capClockAt")
    a(f"[{k}, {N}] call capPathAt")
    a("triSimFrames 2")
    a(f'triScreenshot "{lab}"')
    if k % 10 == 0:
        a(f'call {{ format ["view=head1;event=dirs;k={k};t=%1;dirs=%2", time, capH1Men apply {{ getDir _x }}] call capLog }}' if False else
          f'call {{ capDirs = []; {{ capDirs = capDirs + [getDir _x] }} forEach capH1Men; format ["view=head1;event=dirs;k={k};t=%1;dirs=%2", time, capDirs] call capLog }}')
a('call { "view=head1;event=done" call capLog }')
a("triEndTest")
open(out, "w", newline="\n").write("\n".join(L) + "\n")
print(out, len(L))
