"""Build final/: per view, classic + albedo + final (+ midway 'remastered') from
run13 (scratch build, ovr_final) and shipped + shippedClassic from run13s
(shared shipped exe, no override); merge both udcap logs with relabelled shot
lines so make_cmp.py finds the GPU times of each panel.

    python merge_final.py
"""
import re
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
A, S, OUT = ROOT / "run13", ROOT / "run13s", ROOT / "final"
ANSI = re.compile(r"\x1b\[[0-9;]*m")


def main():
    OUT.mkdir(exist_ok=True)
    n = 0
    for p in A.glob("*__*.png"):
        v, lab = p.stem.split("__", 1)
        if lab in ("classic", "albedo", "final", "remastered"):
            shutil.copy2(p, OUT / p.name)
            n += 1
    for p in S.glob("*__*.png"):
        v, lab = p.stem.split("__", 1)
        if lab == "remastered":
            shutil.copy2(p, OUT / f"{v}__shipped.png")
            n += 1
        elif lab == "classic":
            shutil.copy2(p, OUT / f"{v}__shippedClassic.png")
            n += 1
    lines = []
    for line in (A / "udcap.log").read_text(encoding="utf-8", errors="replace").splitlines():
        lines.append(ANSI.sub("", line))
    for line in (S / "udcap.log").read_text(encoding="utf-8", errors="replace").splitlines():
        line = ANSI.sub("", line)
        line = re.sub(r"(label=[^;]+__)remastered;", r"\1shipped;", line)
        line = re.sub(r"(label=[^;]+__)classic;", r"\1shippedClassic;", line)
        lines.append(line)
    (OUT / "udcap.log").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(n, "images ->", OUT)


if __name__ == "__main__":
    main()
