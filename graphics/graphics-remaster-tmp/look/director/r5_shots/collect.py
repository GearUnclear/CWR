"""Collect round 5 shots into director/final (overhaul only) and final/rainviews."""
import re, shutil, sys
from pathlib import Path
S = Path(r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/director/r5_shots")
F = Path(r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/director/final")
R = F / "rainviews"
F.mkdir(exist_ok=True); R.mkdir(exist_ok=True)
pat = re.compile(r"^\d{3}_(.+?)\.png$")
got = {}
def put(src, name):
    dst = F / name
    shutil.copy2(src, dst); got[name] = src
# 960 cuts
for lane, test in [("a", "look_r5_960_abel"), ("d", "look_r5_960_eden"), ("d", "look_r5_960_sinai")]:
    for p in sorted((S / lane / "tri" / test).glob("*.png")):
        m = pat.match(p.name)
        if m and m.group(1).endswith("__remastered"):
            put(p, m.group(1)[: -len("__remastered")] + "__overhaul.png")
small = {n[: -len("__overhaul.png")] for n in got}
# 1080 views
for lane, test in [("b", "look_r5_final_abel_a"), ("c", "look_r5_final_abel_b"), ("d", "look_final_eden"), ("d", "look_final_sinai")]:
    for p in sorted((S / lane / "tri" / test).glob("*.png")):
        m = pat.match(p.name)
        if m and m.group(1).endswith("__overhaul"):
            v = m.group(1)[: -len("__overhaul")]
            put(p, f"{v}__1080__overhaul.png")
            if v not in small:
                put(p, f"{v}__overhaul.png")
# optics
opt = {"optics__hud_0": "optics_mist_hud", "optics__gun_0_f1": "optics_mist_sight_f1", "optics__gun_0_f30": "optics_mist_sight"}
for p in sorted((S / "d" / "tri" / "look_r5_optics").glob("*.png")):
    m = pat.match(p.name)
    if m and m.group(1) in opt:
        put(p, opt[m.group(1)] + "__overhaul.png")
# rain views
rain = {}
for lane, test in [("rain_abel", "look_shine_abel"), ("rain_eden", "look_shine_eden")]:
    for p in sorted((S / lane / "tri" / test).glob("*.png")):
        m = pat.match(p.name)
        if m and m.group(1).endswith("__ovh"):
            v = m.group(1)[: -len("__ovh")]
            shutil.copy2(p, R / f"{v}.png"); rain[v] = p
print(len(got), "final shots;", len(rain), "rain views:", sorted(rain))
want = [n.name for n in (F.parent / "round4" / "final").glob("*.png")]
missing = sorted(set(want) - set(got))
print("missing vs round 4:", missing)
