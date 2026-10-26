"""Read-only: dump LoBo_Pilot config classes/wounds/models and p3d face textures."""
import re
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, r"D:\Arma_CWA\.claude\worktrees\lobo-repair-4\arma_CWR\tools\lobo")
from repair import textures, wounds  # noqa: E402
from repair.pbo import Archive  # noqa: E402

PBO = Path(r"D:\Arma_CWA\@LoBo\addons\LoBo_Pilot.pbo")
OUT = Path(r"D:\Arma_CWA\tmp\lobo-wound-review")
arc = Archive(PBO.read_bytes())
cfg = arc.read("config.cpp").decode("latin1")
(OUT / "lobo_pilot_config.cpp").write_text(cfg, encoding="latin1", newline="")
after = wounds.repair({"lobo_pilot/config.cpp": cfg})["lobo_pilot/config.cpp"]
(OUT / "lobo_pilot_config.after.cpp").write_text(after, encoding="latin1", newline="")

lines = cfg.replace("\r\n", "\n").split("\n")
cls = None
for i, line in enumerate(lines):
    m = re.match(r"^\t?class\s+(\w+)\s*(:\s*\w+)?", line)
    if m and line.startswith("\tclass "):
        cls = line.strip()
    if "model" in line and "=" in line and line.startswith("\t\tmodel"):
        print(f"{i+1:5d} {cls} :: {line.strip()}")
    if "wounds" in line.lower():
        print(f"{i+1:5d} {cls} :: {line.strip()[:60]}")
        j = i + 1
        while j < len(lines) and "};" not in lines[j - 1] or j == i + 1:
            print("        ", lines[j].strip()[:400])
            if "};" in lines[j]:
                break
            j += 1

print("p3d members:", [k for k in arc.by_key if k.endswith(".p3d")])
for key in sorted(k for k in arc.by_key if k.endswith(".p3d")):
    data = arc.read(key)
    try:
        lods = textures.face_textures(data)
    except Exception as exc:  # noqa: BLE001
        try:
            lods = textures.face_textures(data, data[-32:])
        except Exception as exc2:  # noqa: BLE001
            print(key, "parse failed", exc, exc2)
            continue
    names = Counter()
    for res, fields in lods:
        for f in fields:
            names[f.split(b"\0", 1)[0].decode("latin1").lower()] += 1
    body = sorted(n for n in names if "\\tex" in n and any(p in n for p in wounds.PARTS))
    print(key, "body textures:", body)
