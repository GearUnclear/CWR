"""Build the track-aa comparison figures from a v2 run.

    python make_figures.py <run_dir> <out_dir>
"""
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).parent
run = Path(sys.argv[1])
out = Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)
T = str(run / "timings.csv")

CROPS = {
    "forest_edge_golden": ["560,20,960,320", "0,180,400,480"],
    "overcast_broken": ["560,240,960,400", "100,380,420,560"],
    "town_night": ["300,40,700,300", "0,160,400,420"],
    "sinai_lowsun_fp": ["100,180,600,420"],
}
BEST = {"H": "H-best3", "D": "D-best"}


def cmp(name, title, items, crop=None, zoom=1, cols=0, note=""):
    args = [sys.executable, str(HERE / "compare.py"), str(out / name), "--title", title, "--timings", T]
    if crop:
        args += ["--crop", crop]
    if zoom > 1:
        args += ["--zoom", str(zoom)]
    if cols:
        args += ["--cols", str(cols)]
    if note:
        args += ["--note", note]
    for path, label in items:
        if not (run / path).exists():
            print("missing", path)
            return
        args.append(f"{run / path}={label}")
    subprocess.run(args, check=True)


for view, crops in CROPS.items():
    f = lambda lab: f"{view}__{lab}.png"
    # overcast_broken: the remastered cloud shadow drifted over the camera between
    # H-best2 and H-best3 (foreground luma 87.7 -> 63.5), so use H-best2 (j2) there.
    BEST["H"] = "H-best2" if view == "overcast_broken" else "H-best3"
    # three-way, full frame, H setting (the capture default)
    cmp(f"BEST_{view}.png", f"{view}: Classic | Remastered (current) | Remastered + aa prototype (4x MSAA, 1.5x)",
        [(f("H-classic"), "Classic"), (f("H-rem"), "Remastered (current)"),
         (f(BEST["H"]), "Remastered + cutout coverage + Lanczos/RCAS present")],
        note=("prototype = world override j=%s + scratch present pass p=2 (Lanczos2 downsample, RCAS 0.5)" % ("2 (A2C coverage = filtered alpha once minified)" if BEST["H"] == "H-best2" else "3 (cutout threshold 0.5 once minified)")))
    for i, c in enumerate(crops):
        zoom = 2 if view != "sinai_lowsun_fp" else 2
        cmp(f"{view}__H_threeway_crop{i}.png", f"{view} crop x{zoom}: Classic | Remastered | + prototype (H)",
            [(f("H-classic"), "Classic"), (f("H-rem"), "Remastered"), (f(BEST["H"]), "+ prototype")], crop=c, zoom=zoom)
        cmp(f"{view}__H_coverage_crop{i}.png", f"{view} crop x{zoom}: cutout coverage variants (H: 4x MSAA alpha-to-coverage)",
            [(f("H-rem"), "current (thr 0.753)"), (f("H-cov2"), "j2 A2C = filtered alpha far"),
             (f("H-cov3"), "j3 thr 0.5 once minified")], crop=c, zoom=zoom)
        cmp(f"{view}__H_sharpen_crop{i}.png", f"{view} crop x{zoom}: downsample / sharpening variants (H)",
            [(f("H-rem"), "current (GL_LINEAR blit)"), (f("H-lz"), "Lanczos2 downsample"),
             (f("H-lzrcas05"), "Lanczos2 + RCAS 0.5"), (f("H-lzrcas10"), "Lanczos2 + RCAS 1.0"),
             (f("H-blrcas05"), "bilinear + RCAS 0.5"), (f("H-compcas10"), "CAS 1.0 at render res")],
            crop=c, zoom=zoom, cols=3)
        cmp(f"{view}__D_crop{i}.png", f"{view} crop x{zoom}: shipped default (no MSAA, 1.0x)",
            [(f("D-classic"), "Classic"), (f("D-rem"), "Remastered"), (f("D-cov3"), "j3 thr 0.5 minified"),
             (f("D-cov1"), "j1 Golus mip scale"), (f("D-fxaa"), "FXAA"), (f("D-best"), "j3 + FXAA + RCAS 0.5")],
            crop=c, zoom=zoom, cols=3)
    cmp(f"{view}__D_threeway.png", f"{view}: shipped default AA (none): Classic | Remastered | + prototype",
        [(f("D-classic"), "Classic"), (f("D-rem"), "Remastered (current)"), (f("D-best"), "+ j3 coverage, FXAA, RCAS 0.5")])
print("figures in", out)
