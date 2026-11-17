"""Makes a finished model build folder ready for build-kj-dedushka.ps1: its textures as PAA under
stage/uddedushka (DXT1, mips by the repo's own tool).

  python stage_build.py <build dir> [--cache <dir of PAAs already converted>]...

The build's dedushka_export.json names every sheet (PNG and PAA name). A sheet whose PAA is in a
cache folder and is newer than the PNG is copied; anything else is converted. 4096 sheets take 5 to 9
seconds each, so the unchanged body sheets come from the finals' cache.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys

TOOLS = r"D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("build")
    ap.add_argument("--cache", action="append", default=[])
    a = ap.parse_args()
    summary = json.load(open(os.path.join(a.build, "dedushka_export.json"), encoding="utf-8"))
    stage = os.path.join(a.build, "stage", "uddedushka")
    os.makedirs(stage, exist_ok=True)
    for old in os.listdir(stage):
        if old.lower().endswith(".paa"):
            os.remove(os.path.join(stage, old))
    for t in summary["textures"]:
        dst = os.path.join(stage, t["paa"])
        done = False
        for c in a.cache:
            src = os.path.join(c, t["paa"])
            if os.path.isfile(src) and os.path.getmtime(src) >= os.path.getmtime(t["png"]):
                shutil.copyfile(src, dst)
                done = True
                print("cached    %s" % t["paa"])
                break
        if not done:
            r = subprocess.run([TOOLS, "image", "convert", t["png"], dst, "-f", "DXT1"], capture_output=True, text=True)
            if r.returncode != 0 or not os.path.isfile(dst):
                print(r.stdout, r.stderr)
                raise SystemExit("could not convert %s" % t["png"])
            print("converted %s (%d bytes)" % (t["paa"], os.path.getsize(dst)))
    print("staged %d textures in %s" % (len(summary["textures"]), stage))


if __name__ == "__main__":
    sys.exit(main())
