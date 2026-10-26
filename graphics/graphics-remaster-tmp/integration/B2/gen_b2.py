"""B2 (shade) lane capture tests: the hero views of plan section 8 (lowsun_fp,
sinai_lowsun_fp), the shade track's side-lit calibration boxes at 12.7, 7.5 and
4.8 degrees (Abel) and 12.2 and 7.6 degrees (Sinai), the golden-hour crowns
(forest_edge_golden) and a noon reference (noon_fields).

    python gen_b2.py <captures dir of the scratch worktree>

Per view, in one game: classic, off (remastered,lowSunRig=0,crownCanopy=0), on
(remastered) and albedo (remastered,debug=6,post=0: the albedo debug view this
step adds; on a build without it the shot is a plain frame with post off and is
not used). Labels <view>__classic, __off, __on and <view>_alb__remastered.
"""
import sys
from pathlib import Path

cap = Path(sys.argv[1]).resolve()
sys.path.insert(0, str(cap))
import capgen  # noqa: E402
import views  # noqa: E402

OFF = "remastered,lowSunRig=0,crownCanopy=0"
ALB = "remastered,debug=6,post=0"
SIM_T0, SIM_DT = 2000, 600

BOX = ['[["M113"], 15, 3, -45, "veh"] call capSpawn;',
       '[["Jeep"], 10, -4.5, -45, "veh"] call capSpawn;',
       '[["SoldierWB"], 6, 0.5, -60, "man"] call capSpawn;']
SINAI_BOX = ['[["LoBo_Zelda", "M113"], 15, 3, -45, "veh"] call capSpawn;',
             '[["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -4.5, -45, "veh"] call capSpawn;',
             '[["LoBoGolaniWB", "SoldierWB"], 6, 0.5, -60, "man"] call capSpawn;']


def timing(name, table):
    for v in table:
        if v["name"] == name:
            v = dict(v)
            v.pop("f10", None)
            v.pop("note", None)
            return v
    raise KeyError(name)


ABEL = [
    dict(timing("lowsun_fp", views.TIMING_ABEL), name="lowsun_fp"),
    dict(name="lowsun_box", x=7300, z=10900, eye=1.75, h=207.5, p=-3, date=[1985, 3, 1, 8, 0], vd=1500, props=BOX),
    dict(name="dawn_box", x=7300, z=10900, eye=1.75, h=202.3, p=-3, date=[1985, 3, 1, 7, 30], vd=1500, props=BOX),
    dict(name="dawn_box5", x=7300, z=10900, eye=1.75, h=199.8, p=-3, date=[1985, 3, 1, 7, 15], vd=1500, props=BOX),
    timing("forest_edge_golden", views.TIMING_ABEL),
    timing("noon_fields", views.TIMING_ABEL),
]
SINAI = [
    timing("sinai_lowsun_fp", views.TIMING_SINAI),
    dict(name="sinai_box", x=8350, z=2950, eye=1.75, h=33.8, p=-3, date=[2005, 7, 21, 16, 10], vd=2000,
         props=SINAI_BOX),
    dict(name="sinai_dusk_box", x=8350, z=2950, eye=1.75, h=30.2, p=-3, date=[2005, 7, 21, 16, 35], vd=2000,
         props=SINAI_BOX),
]


def block(v, index):
    out = [f"\n// ---- {v['name']}"]
    out += capgen.begin(v, v["name"], False, sim_time=SIM_T0 + SIM_DT * index)
    out.append("triSimFrames 150")
    out += capgen.shot(v, v["name"], "classic", f"{v['name']}__classic")
    out += capgen.shot(v, v["name"], OFF, f"{v['name']}__off")
    out += capgen.shot(v, v["name"], "remastered", f"{v['name']}__on")
    out += capgen.shot(v, v["name"], ALB, f"{v['name']}_alb__remastered")
    out += capgen.end(v)
    return out


def write(name, world, vs):
    title = f"B2 shade lane on {world}: gen_b2.py; classic, off, on, albedo per view."
    lines = [capgen.header(world, vs[0], title)]
    for i, v in enumerate(vs):
        lines += block(v, i)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    capgen.write(cap / f"{name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(cap / f"{name}.test.toml", capgen.toml(world, title, views.WORLDS, timeout=3600))
    print(f"{name}: {len(vs)} views")


write("b2_abel", "Abel", ABEL)
write("b2_sinai", "Sinai", SINAI)
