"""Preview of the 2026-10-11.4 wounds rule from its actual output bytes (stdlib only).

Reads the shipped LoBo_Pilot.pbo (healthy and olive _d textures, read-only) and
a repaired copy built from repair_lobo.plan (the generated _dr members), checks
every _dr member against its pin, and writes preview.html with base64 PNGs.
"""
import base64
import hashlib
from pathlib import Path
import struct
import sys
import zlib

LOBO = Path(r"D:\Arma_CWA\.claude\worktrees\lobo-repair-4\arma_CWR\tools\lobo")
sys.path.insert(0, str(LOBO))
from repair import paa, wounds  # noqa: E402
from repair.pbo import Archive  # noqa: E402

SHIPPED = Path(r"D:\Arma_CWA\@LoBo\addons\LoBo_Pilot.pbo")
REPAIRED = Path(r"D:\Arma_CWA\tmp\lobo-wound-recolor\rule-work\repaired\LoBo_Pilot.pbo")
OUT = Path(__file__).resolve().parent / "preview.html"


def png(img):
    w, h, r, g, b = img
    rows = b"".join(b"\0" + bytes(v for x in range(w) for v in (r[y * w + x], g[y * w + x], b[y * w + x]))
                    for y in range(h))

    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(rows, 9)) + chunk(b"IEND", b""))


def uri(img):
    return "data:image/png;base64," + base64.b64encode(png(img)).decode()


shipped = Archive(SHIPPED.read_bytes())
repaired = Archive(REPAIRED.read_bytes())


def member(archive, key):
    return bytes(archive.stored(archive.by_key[key.split("/", 1)[1]]))


CLASSES = {"EAF" + chr(92) + "tex": "LoBo_Egypt_Pilot (+_cap), EAF reddish brown",
           "IAF" + chr(92) + "tex2": ("LoBo_IAF_Pilot_grey (69th Sqn); body (all but the left arm) of "
                                      "LoBo_IAF_Pilot_grey2 (106th) and _grey3 (133th); each with its _cap"),
           "IAF" + chr(92) + "tex3": "left arm of LoBo_IAF_Pilot_grey2 (+Cap_grey2), 106th Sqn",
           "IAF" + chr(92) + "tex4": "left arm of LoBo_IAF_Pilot_grey3 (+Cap_grey3), 133th Sqn"}
sections = []
for folder, names in wounds.SUITS.items():
    rows = []
    for part, healthy_name in names.items():
        key = wounds._key(folder, part + "_dr.paa")
        made = member(repaired, key)
        digest = hashlib.sha256(made).hexdigest()
        if digest != wounds.OUTPUTS[key][1]:
            raise SystemExit(f"{key}: not the pinned output")
        healthy = paa.decode(member(shipped, wounds._key(folder, healthy_name)))
        olive_d = paa.decode(member(shipped, wounds._key(folder, part + "_d.paa")) if folder in ("EAF\\tex", "IAF\\tex2", "IAF\\tex3")
                             else member(shipped, wounds._key(wounds.OLIVE_WOUNDED, part + "_d.paa")))
        red = paa.decode(made)
        levels = len(paa.read(made)["levels"])
        tall = red[1] > red[0]
        cls = "tall" if tall else "sq"
        rows.append(
            f"<tr class='{cls}'><th>{part}<br><small>{red[0]}x{red[1]}, {levels} levels<br>{len(made):,} bytes"
            f"<br>sha256 {digest[:12]}</small></th>"
            f"<td><img src='{uri(healthy)}'><br><small>{healthy_name} ({healthy[0]}x{healthy[1]})</small></td>"
            f"<td><img src='{uri(olive_d)}'><br><small>{part}_d.paa (shipped)</small></td>"
            f"<td><img src='{uri(red)}'><br><small>{part}_dr.paa (generated)</small></td></tr>")
    sections.append(f"<h2>{folder.replace(chr(92), ' ')}: {CLASSES[folder]}</h2>"
                    "<table><tr><th></th><th>healthy (shipped)</th><th>shipped olive _d</th>"
                    "<th>generated red _dr</th></tr>" + "".join(rows) + "</table>")

html = f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>LoBo pilot wounds</title>
<style>
:root {{ --bg: #1d1f21; --fg: #d8d8d8; --muted: #9a9a9a; --line: #3a3d40; }}
body {{ background: var(--bg); color: var(--fg); font: 14px/1.45 system-ui, sans-serif; margin: 16px; }}
h1 {{ font-size: 20px; }} h2 {{ font-size: 16px; margin-top: 28px; }}
p, li {{ max-width: 70em; }} small {{ color: var(--muted); }}
table {{ border-collapse: collapse; }} th, td {{ border-top: 1px solid var(--line); padding: 6px 10px; text-align: center; vertical-align: top; }}
img {{ image-rendering: pixelated; background: #444; }}
tr.sq img {{ height: 256px; }} tr.tall img {{ height: 384px; }}
</style></head><body>
<h1>LoBo pilot wounded textures, repair 2026-10-11.4 (unreleased)</h1>
<p>Every image is decoded from PAA bytes: healthy and olive <code>_d</code> from the installed LoBo_Pilot.pbo,
generated <code>_dr</code> from a LoBo_Pilot.pbo built in tmp from <code>repair_lobo.plan</code>'s output (each checked
against its pin in <code>repair/wounds.py</code>). The olive suits (RJAF, IAF tex 101st and 253rd Sqn, SyAF) keep the
shipped olive <code>_d</code> set and get no new member. Every pilot class now pairs every body face of its model:
the 253rd Sqn class gains its <code>tex5</code> left arm (shipped <code>tex5\armle_d</code>), the SyAF class gets its
own SyAF <code>tex</code> pairs (shipped SyAF <code>_d</code>) instead of the EAF pairs that matched none of its faces,
and the 106th and 133th Sqn classes get the full face and grey body set shown here plus their own left arm.</p>
{''.join(sections)}
</body></html>
"""
OUT.write_text(html, encoding="utf-8")
print(OUT, OUT.stat().st_size)
