"""collect.py RUN_DIR OUT_DIR : copy the shots of a director run into OUT_DIR as
<view>__classic.png / <view>__overhaul.png (run-captures views, 960x640), the
look_final lane views as <view>__1080__classic/overhaul.png (and as the plain
name when the view is not in the run-captures set), and the optics shots."""
import re
import shutil
import sys
from pathlib import Path

run, out = Path(sys.argv[1]), Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)
tri = run / "tri"
base = {}
for d in sorted(tri.iterdir()):
    if not d.is_dir():
        continue
    for f in sorted(d.rglob("*.png")):
        m = re.match(r"\d+_(.+?)__(.+)\.png$", f.name)
        if not m:
            continue
        view, label = m.group(1), m.group(2)
        if d.name in ("abel_lighting", "eden_lighting", "sinai_lobo_lighting"):
            lab = {"classic": "classic", "remastered": "overhaul"}.get(label)
            if lab:
                shutil.copy2(f, out / f"{view}__{lab}.png")
                base[view] = True
        elif d.name.startswith("look_final"):
            lab = label
            shutil.copy2(f, out / f"{view}__1080__{lab}.png")
        elif d.name.startswith("look_director_optics"):
            # optics__hud_0 (classic), optics__hud_1 (overhaul), optics__gun_<i>_f<n>
            m2 = re.match(r"(hud|gun)_(\d)(?:_f(\d+))?$", label)
            if not m2:
                # older runs: the index did not format; take it from the order
                k = int(f.name.split("_")[0])
                kind = "hud" if "hud" in label else "gun"
                fr = re.search(r"_f(\d+)$", label)
                m2 = re.match(r"(hud|gun)_(\d)(?:_f(\d+))?$", f"{kind}_{k // 4}" + (f"_f{fr.group(1)}" if fr else ""))
            if m2:
                kind, idx, fr = m2.group(1), int(m2.group(2)), m2.group(3)
                lab = "classic" if idx == 0 else "overhaul"
                if kind == "hud":
                    shutil.copy2(f, out / f"optics_mist_hud__{lab}.png")
                elif fr == "30":
                    shutil.copy2(f, out / f"optics_mist_sight__{lab}.png")
                elif fr == "1":
                    shutil.copy2(f, out / f"optics_mist_sight_f1__{lab}.png")
# lane views that are not in the run-captures set get their plain names too
for f in out.glob("*__1080__classic.png"):
    view = f.name.split("__")[0]
    if view not in base:
        o = out / f"{view}__1080__overhaul.png"
        if o.exists():
            shutil.copy2(f, out / f"{view}__classic.png")
            shutil.copy2(o, out / f"{view}__overhaul.png")
print("collected", len(list(out.glob("*.png"))))
