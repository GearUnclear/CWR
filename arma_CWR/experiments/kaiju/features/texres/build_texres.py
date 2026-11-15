r"""Builds the texture-resolution test addons from nothing and mounts them. No Blender, no game.

  python build_texres.py [--out <dir>] [--after-layout <json>] [--no-install] [--no-gen]

Steps: gen_texres.py (models, PNGs, the hand-made PAAs), PNG to PAA with PoseidonTools
`image convert` (the artists' tool), stage, pack two pbos, config probe, mount copy.

  <out>\@kj_texres\addons\uddedushka.pbo   the creature addon, same addon name and class as the
                                           pipeline's (UDDedushka = the CHART model here), plus
                                           UDDedushkaB (before), UDDedushkaA (after) and
                                           UDDedushkaS (after, faces shuffled). config.cpp is the
                                           pipeline's own, taken from the copy in <notes>\base,
                                           with those three classes added.
  <out>\@kj_texres\addons\kjtexres.pbo     the limits panels (KJTxp_<id>).

The mount is <git root>\tmp\@kj_texres. A test mounts it IN PLACE OF @uddedushka:
  --mods-dir ".." --mod "@kj_texres"
and names both addons in its mission.sqm addOns[]. The shared @uddedushka is never touched.
Every binary goes under kaiju-notes\features\texres; nothing binary is written to the repo.
"""
import json
import os
import re
import shutil
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
GIT_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", "..", "..", ".."))
NOTES = r"D:\Arma_CWA_worktrees\kaiju-notes\features\texres"
TOOLS = r"D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe"
BASE = os.path.join(NOTES, "base", "uddedushka")
VARIANTS = [("UDDedushkaB", "dedushka_b", "Dedushka (texture test: before)"),
            ("UDDedushkaA", "dedushka_a", "Dedushka (texture test: after)"),
            ("UDDedushkaS", "dedushka_s", "Dedushka (texture test: shuffled sections)"),
            ("UDDedushkaM", "dedushka_m", "Dedushka (texture test: after, with mip anchors)")]


def run(argv, log):
    t0 = time.time()
    res = subprocess.run(argv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors="replace")
    with open(log, "a", encoding="utf-8") as fh:
        fh.write("$ %s\n%s\n" % (" ".join(argv), res.stdout))
    if res.returncode != 0:
        print(res.stdout[-2000:])
        raise SystemExit("%s failed with exit %d (log: %s)" % (argv[0], res.returncode, log))
    return time.time() - t0, res.stdout


def fresh(path):
    if os.path.isdir(path):
        shutil.rmtree(path)
    os.makedirs(path)


def patch_config(text):
    """The pipeline's creature config plus the three test classes."""
    names = "".join(', "%s"' % v[0] for v in VARIANTS)
    text, n = re.subn(r'units\[\] = \{"UDDedushka"\};', 'units[] = {"UDDedushka"%s};' % names, text, count=1)
    if n != 1:
        raise SystemExit("config.cpp: units[] line not found")
    models = "".join("\n    class %s : Man {};" % v[1] for v in VARIANTS)
    text, n = re.subn(r"(class dedushka : Man \{\};)", lambda m: m.group(1) + models, text, count=1)
    if n != 1:
        raise SystemExit("config.cpp: CfgModels entry not found")
    classes = ""
    for cls, model, title in VARIANTS:
        classes += ("    class %s : UDDedushka\n    {\n        displayName = \"%s\";\n        model = \"\\uddedushka\\%s\";\n    };\n"
                    % (cls, title, model))
    end = text.rstrip().rfind("};")
    if end < 0:
        raise SystemExit("config.cpp: end of CfgVehicles not found")
    return text[:end] + classes + "};\n"


def main():
    out = os.path.join(NOTES, "build")
    layout = None
    install = True
    gen = True
    argv = sys.argv[1:]
    i = 0
    while i < len(argv):
        if argv[i] == "--out":
            out = os.path.abspath(argv[i + 1])
            i += 1
        elif argv[i] == "--after-layout":
            layout = os.path.abspath(argv[i + 1])
            i += 1
        elif argv[i] == "--no-install":
            install = False
        elif argv[i] == "--no-gen":
            gen = False
        else:
            raise SystemExit("unknown option %s" % argv[i])
        i += 1
    os.makedirs(out, exist_ok=True)
    log = os.path.join(out, "build.log")
    if os.path.isfile(log):
        os.remove(log)
    gen_dir = os.path.join(out, "gen")
    if gen:
        cmd = [sys.executable, os.path.join(HERE, "gen_texres.py"), gen_dir, "--base", BASE]
        if layout:
            cmd += ["--after-layout", layout]
        secs, text = run(cmd, log)
        print(text.rstrip())
        print("[texres] generated in %.0f s" % secs)
    with open(os.path.join(gen_dir, "texres_gen.json"), "r", encoding="utf-8") as fh:
        summary = json.load(fh)

    stage_u = os.path.join(out, "stage", "uddedushka")
    stage_k = os.path.join(out, "stage", "kjtexres")
    fresh(stage_u)
    fresh(stage_k)
    shutil.copytree(os.path.join(BASE, "anim"), os.path.join(stage_u, "anim"))
    shutil.copy2(os.path.join(BASE, "ded_glow.paa"), stage_u)
    with open(os.path.join(BASE, "config.cpp"), "r", encoding="utf-8") as fh:
        cfg = patch_config(fh.read())
    with open(os.path.join(stage_u, "config.cpp"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write(cfg)
    shutil.copy2(os.path.join(gen_dir, "kjtexres_config.cpp"), os.path.join(stage_k, "config.cpp"))
    for name in summary["models"]:
        dst = stage_k if name.startswith("txp_") else stage_u
        shutil.copy2(os.path.join(gen_dir, "models", name), dst)

    timings = []
    for t in summary["creature_png"]:
        secs, _ = run([TOOLS, "image", "convert", t["png"], os.path.join(stage_u, t["paa"]), "-f", t["format"]], log)
        timings.append((t["paa"], t["size"], t["format"], secs, os.path.getsize(os.path.join(stage_u, t["paa"]))))
    for t in summary["creature_paa"]:
        shutil.copy2(t["paa"], stage_u)
    for t in summary["panel_png"]:
        secs, _ = run([TOOLS, "image", "convert", t["png"], os.path.join(stage_k, t["paa"]), "-f", t["format"]], log)
        timings.append((t["paa"], t["size"], t["format"], secs, os.path.getsize(os.path.join(stage_k, t["paa"]))))
    for t in summary["panel_paa"]:
        if t.get("paa"):
            shutil.copy2(t["paa"], stage_k)
    for t in summary["panel_loose"]:
        shutil.copy2(t["file"], stage_k)
    print("[texres] PNG to PAA with PoseidonTools:")
    for name, size, fmt, secs, nbytes in timings:
        if size >= 1024:
            print("    %-14s %5d %-8s %5.1f s  %9d bytes" % (name, size, fmt, secs, nbytes))

    # every texture a model names must be in its pbo
    for name, m in summary["models"].items():
        texs = m.get("textures") or [m["texture"]]
        for tex in texs:
            addon, rest = tex.split("\\", 1)
            root = stage_u if addon == "uddedushka" else stage_k
            if not os.path.isfile(os.path.join(root, rest)):
                raise SystemExit("%s names %s but it was not staged" % (name, tex))

    addons = os.path.join(out, "@kj_texres", "addons")
    fresh(addons)
    for stage, pbo in ((stage_u, "uddedushka.pbo"), (stage_k, "kjtexres.pbo")):
        run([TOOLS, "pbo", "pack", stage, os.path.join(addons, pbo)], log)
        print("[texres] packed %s (%.1f MB)" % (pbo, os.path.getsize(os.path.join(addons, pbo)) / 1048576.0))

    shadow = os.path.join(GIT_ROOT, "tmp", "remaster-shadow-kaiju")
    if os.path.isdir(os.path.join(shadow, "BIN")):
        res = subprocess.run([TOOLS, "guerrilla", "probe", "--data-dir", shadow, "--mod", os.path.join(out, "@kj_texres")],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors="replace")
        with open(os.path.join(out, "probe.log"), "w", encoding="utf-8") as fh:
            fh.write(res.stdout)
        hits = [l for l in res.stdout.splitlines() if re.search(r"UDDedushka|KJTxp|SUMMARY", l)]
        for l in hits[:24]:
            print("    " + l.strip())
        if not any("PROBE UDDedushka ok" in l for l in hits):
            print("[texres] WARNING: the config probe did not accept UDDedushka (see probe.log)")
    if install:
        mount = os.path.join(GIT_ROOT, "tmp", "@kj_texres", "addons")
        os.makedirs(mount, exist_ok=True)
        for pbo in ("uddedushka.pbo", "kjtexres.pbo"):
            # copy beside, then swap in one step: a game that starts meanwhile never sees half a file
            tmp = os.path.join(mount, pbo + ".new")
            shutil.copy2(os.path.join(addons, pbo), tmp)
            os.replace(tmp, os.path.join(mount, pbo))
        print("[texres] mounted copy: %s" % mount)
    # the stage is a second copy of every texture; the pbo and the gen folder are enough
    shutil.rmtree(os.path.join(out, "stage"))
    print("[texres] done")


if __name__ == "__main__":
    main()
