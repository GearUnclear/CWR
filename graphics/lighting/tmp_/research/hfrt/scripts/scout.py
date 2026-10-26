"""Scouting run: candidate high vantages, few shots each (v2 override, scratch build)."""
import gen_tests as g
SCOUT = [
    ("remastered", [], 60),
    ("hf4", ["terrainShadows=4", "terrainOcclusion=1"], 60),
    ("dbgsun_hf4", ["terrainShadows=4", "terrainOcclusion=1", "debug=1"], 20),
]
views = []
for t, date in (("0700", "[1985, 10, 10, 7, 0]"), ("1730", "[1985, 9, 20, 17, 30]")):
    views += [
        dict(id=f"sumN_{t}", ox=5500, oz=6250, eye=15, heading=5, pitch=-10, date=date, overcast=0, vd=4000, note="summit north"),
        dict(id=f"sumS_{t}", ox=5500, oz=6250, eye=15, heading=185, pitch=-14, date=date, overcast=0, vd=4000, note="summit south"),
        dict(id=f"shoulder_{t}", ox=5200, oz=6400, eye=40, heading=200, pitch=-14, date=date, overcast=0, vd=4000, note="shoulder SSW"),
    ]
g.write("hfrt_scout", "Abel", "abel.Abel", views, None, SCOUT)
