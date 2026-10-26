"""Summarise tree_texture_mip_coverage.txt: for every DXT1 cutout (L0 texel
coverage < 0.98), the texel coverage of L0 and L3 and the bilinear pass rate at
the engine's 0.753 threshold and at 0.5, averaged over L1-L4."""
import re
import sys
from pathlib import Path

txt = Path(sys.argv[1]).read_text(encoding="utf-8", errors="replace").splitlines()
cur = None
data = {}
for line in txt:
    m = re.match(r"^(\S.*?)\s+format=0x([0-9a-f]+)\s+mips=(\d+)", line)
    if m:
        cur = Path(m.group(1)).name
        data[cur] = []
        continue
    m = re.match(r"^\s+L(\d)\s+(\d+)x(\d+)\s+texel coverage ([\d.]+)\s+bilinear\(corner\) >= 0.753: ([\d.]+)\s+>= 0.5: ([\d.]+)", line)
    if m and cur:
        data[cur].append(tuple(float(x) for x in m.groups()))
rows = []
for name, levels in data.items():
    if not levels or levels[0][3] >= 0.98:
        continue
    mid = [l for l in levels if 1 <= l[0] <= 4]
    if not mid:
        continue
    texel = sum(l[3] for l in mid) / len(mid)
    p753 = sum(l[4] for l in mid) / len(mid)
    p50 = sum(l[5] for l in mid) / len(mid)
    rows.append((name, levels[0][3], texel, p753, p50))
print(f"{'texture':28s} L0cov  L1-4cov  pass@0.753  pass@0.5   loss@0.753")
for r in sorted(rows):
    print(f"{r[0]:28s} {r[1]:.3f}  {r[2]:.3f}    {r[3]:.3f}      {r[4]:.3f}    {100 * (1 - r[3] / r[2]):5.1f}%")
if rows:
    n = len(rows)
    t = sum(r[2] for r in rows) / n
    a = sum(r[3] for r in rows) / n
    b = sum(r[4] for r in rows) / n
    print(f"MEAN over {n} cutouts: texel cov {t:.3f}, bilinear pass @0.753 {a:.3f} ({100 * (1 - a / t):.1f}% thinner), @0.5 {b:.3f}")
