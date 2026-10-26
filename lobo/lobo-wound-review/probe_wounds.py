"""Read-only review probe for repair.wounds (2026-10-11.4). Writes only under this folder."""
import hashlib
import json
import struct
import sys
import time
from pathlib import Path

TOOLS_DIR = Path(r"D:\Arma_CWA\.claude\worktrees\lobo-repair-4\arma_CWR\tools\lobo")
sys.path.insert(0, str(TOOLS_DIR))
from repair import paa, wounds  # noqa: E402
from repair.pbo import Archive  # noqa: E402
from repair import textures  # noqa: E402

PBO = Path(r"D:\Arma_CWA\@LoBo\addons\LoBo_Pilot.pbo")
TOOL = Path(r"D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe")
OUT = Path(r"D:\Arma_CWA\tmp\lobo-wound-review")


def sha(b):
    return hashlib.sha256(b).hexdigest()


raw = PBO.read_bytes()
arc = Archive(raw)
print("pbo sha", sha(raw), "entries", len(arc.entries), "checksummed", arc.checksummed, "stuffpbo", arc.stuffpbo,
      "trailer", arc.trailer[:24], "prefix", arc.prefix)
comp = [e.key for e in arc.entries if e.fields[0] != 0]
print("compressed members:", comp)
print("existing _dr:", [k for k in arc.by_key if k.endswith("_dr.paa")])

# 1. declarations consistency
print("SOURCE_MEMBERS == SOURCE_SHA256 keys:", set(wounds.SOURCE_MEMBERS) == set(wounds.SOURCE_SHA256),
      len(wounds.SOURCE_MEMBERS), len(wounds.SOURCE_SHA256))
print("OUTPUTS == outputs():", {k: v[0] for k, v in wounds.OUTPUTS.items()} == wounds.outputs(),
      "order same:", list(wounds.OUTPUTS) == list(wounds.ADDED_MEMBERS))

# stored-name case vs existing members in same folder
for key, (stored, _) in wounds.OUTPUTS.items():
    folder = key.rsplit("/", 1)[0].split("/", 1)[1]
    sib = [e.name.decode("latin1") for e in arc.entries if e.key.rsplit("/", 1)[0] == folder][:2]
    print(" stored", stored, "siblings", sib)

sources = {}
for k in wounds.SOURCE_MEMBERS:
    sources[k] = arc.read(k.split("/", 1)[1], PBO, TOOL)
bad = [k for k in sources if sha(sources[k]) != wounds.SOURCE_SHA256[k]]
print("source pin mismatches:", bad)

# shipped _d structure vs generated
for part in wounds.PARTS:
    d = sources[wounds._key(wounds.OLIVE_WOUNDED, part + "_d.paa")]
    info = paa.read(d)
    tags = {n: v for n, v in info["tags"]}
    offs = struct.unpack("<%di" % (len(tags.get(b"SFFO", b"")) // 4), tags.get(b"SFFO", b""))
    print("shipped", part, "_d len", len(d), "tags", [(n, len(v)) for n, v in info["tags"]],
          "levels", [(w, h, len(p)) for w, h, p in info["levels"]], "end", info["end"], "tail", d[info["end"]:].hex(),
          "AVGC %08x" % struct.unpack("<I", tags[b"CGVA"])[0] if b"CGVA" in tags else None, "OFFS", offs)
    # healthy sources formats
for k, v in sources.items():
    i = paa.read(v)
    print(" src", k, "fmt %04x" % i["format"], [(w, h) for w, h, _ in i["levels"]][:2], "n", len(i["levels"]),
          "tags", [n for n, _ in i["tags"]])

t = time.time()
made = wounds.generate(sources)
print("generate secs", round(time.time() - t, 1))
mism = [k for k in made if sha(made[k]) != wounds.OUTPUTS[k][1]]
print("output pin mismatches:", mism, "count", len(made))
(OUT / "gen").mkdir(exist_ok=True)
for k, v in made.items():
    (OUT / "gen" / k.replace("/", "__")).write_bytes(v)

# structure of generated + transparency check over every level
for k, v in made.items():
    info = paa.read(v)
    tags = {n: val for n, val in info["tags"]}
    offs = struct.unpack("<16i", tags[b"SFFO"])
    three = trans = 0
    for (w, h, blocks) in info["levels"]:
        for i in range(0, len(blocks), 8):
            c0, c1, idx = struct.unpack_from("<HHI", blocks, i)
            if c0 <= c1:
                three += 1
                for j in range(16):
                    if (idx >> (2 * j)) & 3 == 3:
                        trans += 1
    print("gen", k, "levels", [(w, h) for w, h, _ in info["levels"]], "offs", offs[:8], "3col", three,
          "transparent texels", trans, "AVGC %08x" % struct.unpack("<I", tags[b"CGVA"])[0], "tail", v[info["end"]:].hex())
