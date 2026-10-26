"""Split a multi-test run: copy NNN_<view>__<label>.png of each tri test folder
(<test>_abel / <test>_sinai) into <run>/<test>/ and the matching UDCAP shot lines
into <run>/<test>/udcap.log, so labels shared by several tests do not mix.

    python split_collect.py <run_dir>
"""
import re
import shutil
import sys
from pathlib import Path

ANSI = re.compile(r"\x1b\[[0-9;]*m")
RX = re.compile(r"^\d{3}_(.+?)__(.+)\.png$")


def main():
    run = Path(sys.argv[1])
    for tdir in sorted(p for p in (run / "tri").iterdir() if p.is_dir()):
        test = tdir.name.rsplit("_", 1)[0]
        out = run / test
        out.mkdir(exist_ok=True)
        best = {}
        for p in tdir.rglob("*.png"):
            m = RX.match(p.name)
            if not m:
                continue
            key = (m.group(1), m.group(2))
            if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
                best[key] = p
        for (v, lab), src in best.items():
            shutil.copy2(src, out / f"{v}__{lab}.png")
        lines = []
        for log in tdir.rglob("game_stdout.log"):
            for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
                if "UDCAP;" in line:
                    lines.append(ANSI.sub("", line))
        with open(out / "udcap.log", "a", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
        print(tdir.name, "->", out, len(best), "shots")


if __name__ == "__main__":
    main()
