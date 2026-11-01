"""Self-tests of the gate tools on existing pictures (no game). Image work: run it through
squad-heavy.ps1 -Kind analysis.

    python -I selftest.py tripwires --look <tmp\\look\\director\\final>
        Synthetic edits of real 1080p shots; each edit must fire its tripwire, and the
        identity compare must fire none. Tests the code paths, NOT the thresholds (those are
        calibrated blind in P0-11).
    python -I selftest.py floor <p5-stills1-timing> <p5-stills2-timing> <p5-stills3-timing>
        vgate's pair counts on M2's three boots must equal M2's table (measurements-2.md 5).
    python -I selftest.py text
        gen_perfgate / gen_golden plans and bench_report on synthetic logs (no images).
"""
import json
import os
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gatelib as gl  # noqa: E402
import numpy as np  # noqa: E402
from scipy import ndimage  # noqa: E402

M2_TABLE = {  # view: [(max, >2, >8) for pairs 1-2, 1-3, 2-3], reset shots
    "noon_fields_reset": [(13, 999, 5), (8, 285, 0), (13, 1234, 5)],
    "valley_0515_reset": [(25, 1519, 16), (10, 184, 2), (24, 1590, 15)],
    "sunrise_ridge_reset": [(16, 173, 10), (19, 432, 13), (20, 312, 10)],
    "broken_high_reset": [(11, 174, 2), (12, 182, 2), (7, 21, 0)],
}


def fired(view, a, b):
    import vgate
    return sorted(t["name"] for t in vgate.tripwires(view, a, b) if t.get("fired"))


def lin(x):
    return gl.srgb_to_linear(x)


def to8(lin_):
    x = np.clip(lin_, 0, 1)
    s = np.where(x <= 0.0031308, 12.92 * x, 1.055 * np.power(x, 1 / 2.4) - 0.055)
    return np.clip(np.round(s * 255), 0, 255).astype(np.uint8)


def in_box(img, box, fn):
    import golden_views as gv  # noqa: F401
    out = img.copy()
    sy, sx = gl.box_px(box, img.shape)
    out[sy, sx] = fn(img[sy, sx])
    return out


def t_tripwires(look):
    import golden_views as gv
    ok = True

    def check(name, view, a, b, want):
        nonlocal ok
        got = fired(view, a, b)
        if want and want.startswith("-"):
            good = want[1:] not in got  # must NOT fire (others may)
        else:
            good = (want in got) if want else (got == [])
        ok &= good
        print(f"{'PASS' if good else 'FAIL'} {name}: fired {got or 'none'} (want {want or 'none'})")

    img = lambda v: gl.load_rgb(os.path.join(look, f"{v}__1080__overhaul.png"))  # noqa: E731
    noon = img("noon_fields")
    check("identity noon_fields", "noon_fields", noon, noon, None)
    check("darker x0.8 linear", "noon_fields", noon, to8(lin(noon) * 0.8), "only_darker")
    blue = lin(noon) * np.array([0.92, 0.97, 1.08], np.float32)
    check("blue tint", "noon_fields", noon, to8(blue), "blue_wash")
    check("blur 1.5 px", "noon_fields", noon, np.stack([ndimage.gaussian_filter(noon[..., c].astype(np.float32), 1.5)
                                                       for c in range(3)], -1).astype(np.uint8), "sharpness_loss")
    gb = gv.boxes("noon_fields")["ground"][0]
    check("mustard ground", "noon_fields", noon,
          in_box(noon, gb, lambda x: to8(lin(x) * np.array([1.06, 1.03, 0.80], np.float32))), "mustard_ground")
    hb = gv.boxes("noon_fields")["hull"][0]

    def crush(x):
        y = gl.luma(x)
        t = np.median(y)
        f = np.where(y < t, 0.75, 1.0)[..., None]
        return np.clip(x * f, 0, 255).astype(np.uint8)
    check("crushed hull shade", "noon_fields", noon, in_box(noon, hb, crush), "crushed_shade")
    fe = img("forest_edge_golden")
    check("identity forest_edge_golden", "forest_edge_golden", fe, fe, None)
    flat = fe
    for cb in gv.boxes("forest_edge_golden")["crown"]:
        flat = in_box(flat, cb, lambda x: np.stack([ndimage.gaussian_filter(x[..., c].astype(np.float32), 6)
                                                    for c in range(3)], -1).astype(np.uint8))
    check("flattened crowns (blur 6 px)", "forest_edge_golden", fe, flat, "flattened_crowns")
    cs = img("coastline_sunset")
    warm = to8(lin(cs) * np.array([1.10, 1.0, 0.85], np.float32))
    cool = to8(lin(cs) * np.array([0.90, 1.0, 1.12], np.float32))
    check("warmer evening: evening_warmth must stay quiet", "coastline_sunset", cs, warm, "-evening_warmth")
    check("cooler evening", "coastline_sunset", cs, cool, "evening_warmth")
    rn = img("town_night_rain")
    wb = gv.boxes("town_night_rain")["wet"][0]

    def streaks(x):
        y = x.astype(np.float32).copy()
        for c in range(40, x.shape[1], 37):
            y[:, c:c + 2] += 60
        return np.clip(y, 0, 255).astype(np.uint8)
    check("rain streaks on wet ground", "town_night_rain", rn, in_box(rn, wb, streaks), "rain_rule")
    return ok


def t_floor(dirs):
    import vgate
    shots = [vgate.find_shots(d, "_reset") for d in dirs]
    ok = True
    for k, want in M2_TABLE.items():
        imgs = [gl.load_rgb(s[k]) for s in shots]
        got = []
        for i, j in ((0, 1), (0, 2), (1, 2)):
            _, m = vgate.diff_core(imgs[i], imgs[j])
            got.append((m["max"], m["over2"], m["over8"]))
        good = got == want
        ok &= good
        print(f"{'PASS' if good else 'FAIL'} {k}: {got} (M2 {want})")
    return ok


def t_text():
    import subprocess
    here = gl.HERE
    ok = True
    for s in ("quick", "full", "abba", "abba-scoped", "obs", "hitch", "legacy"):
        args = [sys.executable, "-I", os.path.join(here, "gen_perfgate.py"), "--set", s, "--list"]
        if s == "abba-scoped":
            args += ["--views", "valley_0515"]
        p = subprocess.run(args, capture_output=True, text=True)
        plan = json.loads(p.stdout) if p.returncode == 0 else {}
        good = p.returncode == 0 and plan.get("worlds")
        ok &= bool(good)
        print(f"{'PASS' if good else 'FAIL'} gen_perfgate --set {s}: {plan.get('worlds')}")
    # bench_report on a synthetic two-boot log: B is 1 ms slower in CPU work -> one regression
    import bench_report as br
    with tempfile.TemporaryDirectory() as td:
        for side, extra in (("A", 0.0), ("B", 1.0)):
            d = os.path.join(td, side, "tri", "t")
            os.makedirs(d)
            with open(os.path.join(d, "game_stdout.log"), "w", encoding="utf-8") as f:
                f.write('[2026-10-31 10:00:00.000] "UDCAP;view=noon_fields_r1;event=mode;mode=remastered;lit=OK;detail=OK:1;settle=OK:30;draws=1"\n')
                for w in (1, 2, 3):
                    f.write(f'[2026-10-31 10:00:{10 * w:02d}.000] "UDCAP;view=noon_fields_r1;event=meas;label=noon_fields_r1__remastered_w{w};'
                            f'mode=remastered;lit=OK;fps=60;draws=1565;objects=4508;perf=fps=60 frame={9.0 + extra:.2f}/12.00/13.00 '
                            f'swap=0.30/0.40 calls=1 frames=256;gpu=frame 11.00 ms | world 9.0 (shadow 0.1) [ground 2.00 pass1 6.80 rest 0.40] | post 1.50: x | min frame=10.20"\n')
        plan = dict(set="quick", read="meas", config="fixed", sides={}, expected_windows={"A": 3, "B": 3},
                    boots=[dict(idx=0, side="A", run_dir=os.path.join(td, "A", "tri"), foreign=None, exit=0),
                           dict(idx=1, side="B", run_dir=os.path.join(td, "B", "tri"), foreign=None, exit=0)])
        rep = br.analyse(plan)
        row = rep["rows"][0]
        regs = row.get("compare", {}).get("regression", [])
        good = len(regs) == 1 and "cpu_work +1.00" in regs[0] and not rep["contended"]
        ok &= good
        print(f"{'PASS' if good else 'FAIL'} bench_report synthetic +1 ms CPU: {regs}")
    return ok


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    cmd = sys.argv[1]
    if cmd == "tripwires":
        look = sys.argv[sys.argv.index("--look") + 1]
        ok = t_tripwires(look)
    elif cmd == "floor":
        ok = t_floor(sys.argv[2:5])
    elif cmd == "text":
        ok = t_text()
    else:
        print(__doc__)
        return 2
    print("ALL PASS" if ok else "SOME FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
