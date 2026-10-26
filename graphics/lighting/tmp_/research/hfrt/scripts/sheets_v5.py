"""v5 sheets: fine canopy cone trace.

  python sheets_v5.py <run_dir> [--best fine10]

Writes into the run dir:
  <view>__v5_3way.png   Classic | Remastered (current) | Remastered + best, with
                        GPU frame ms (min over the run's timing rounds) and a
                        zoomed crop row
  <view>__v5_modes.png  remastered | sun8 (coarse canopy) | fine10 | fine11 | fine12
  <view>__v5_debug.png  sun visibility: shipped | coarse (8) | fine (10) | fine everywhere (12),
                        sky visibility with canopy AO (11)
"""
import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from sheets import panel, sheet  # noqa: E402

LABEL = {
    "classic": "Classic",
    "remastered": "Remastered (current)",
    "sun8": "HFRT mode 8: terrain + 12.5 m canopy",
    "fine10": "HFRT mode 10: terrain + 3.1 m canopy cone trace",
    "fine11": "HFRT mode 11: 10 + canopy sky occlusion",
    "fine12": "HFRT mode 12: 3.1 m canopy everywhere",
    "dbgsun_rem": "sun vis: shipped horizon map",
    "dbgsun_8": "sun vis: mode 8 (12.5 m canopy)",
    "dbgsun_10": "sun vis: mode 10 (3.1 m canopy)",
    "dbgsun_12": "sun vis: mode 12 (canopy also near)",
    "dbgsky_11": "sky vis: mode 11 (canopy AO)",
    "fine13": "HFRT mode 13: 3.1 m canopy everywhere + canopy sky occlusion",
    "fine15": "HFRT mode 15: baked terrain map x 3.1 m canopy trace + canopy AO",
    "fine16": "HFRT mode 16: 15 at half the canopy steps",
    "dbgsun_13": "sun vis: mode 13",
    "dbgsun_15": "sun vis: mode 15",
    "dbgsky_rem": "sky vis: shipped",
    "dbgsky_13": "sky vis: mode 13 (canopy AO)",
    "dbgsky_15": "sky vis: mode 15 (canopy AO)",
}

CROPS = {
    "hill_1730": (380, 250, 960, 560),
    "hill_0700": (300, 240, 960, 600),
    "overcast_broken": (480, 230, 960, 470),
    "forest_edge_golden": (380, 40, 960, 420),
    "sunrise_ridge": (0, 180, 960, 420),
}


def minima(run):
    out = {}
    p = run / "timing_rounds.md"
    if not p.exists():
        return out
    for line in p.read_text().splitlines()[2:]:
        c = [x.strip() for x in line.strip("|").split("|")]
        if len(c) >= 4:
            out[(c[0], c[1])] = (float(c[3]), int(c[2]))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--best", default="fine10")
    ap.add_argument("--views", default="")
    ap.add_argument("--modes", default="remastered,sun8,fine10,fine11,fine12")
    ap.add_argument("--dbg", default="dbgsun_rem,dbgsun_8,dbgsun_10,dbgsun_12,dbgsky_11")
    ap.add_argument("--tag", default="v5")
    a = ap.parse_args()
    run = Path(a.run)
    mins = minima(run)
    views = sorted({p.name.split("__")[0] for p in run.glob("*__classic.png")})
    if a.views:
        views = [v for v in views if v in a.views.split(",")]
    for view in views:
        def note(m):
            t = mins.get((view, m))
            return f"GPU frame {t[0]:.2f} ms (min of {t[1]} rounds)" if t else None

        def have(m):
            return (run / f"{view}__{m}.png").exists()

        three = [m for m in ("classic", "remastered", a.best) if have(m)]
        if len(three) == 3:
            full = [panel(run / f"{view}__{m}.png", LABEL.get(m, m), note(m)) for m in three]
            rows = [full]
            c = CROPS.get(view)
            if c:
                s = 960 / (c[2] - c[0])
                rows.append([panel(run / f"{view}__{m}.png", LABEL.get(m, m) + f" (crop x{s:.1f})", crop=c, scale=s)
                             for m in three])
            sheet(rows, f"{view}: Classic | Remastered (current) | Remastered + {LABEL.get(a.best, a.best)}",
                  run / f"{view}__{a.tag}_3way.png")
        modes = [m for m in a.modes.split(",") if have(m)]
        if modes:
            ims = [panel(run / f"{view}__{m}.png", LABEL.get(m, m), note(m), scale=0.75) for m in modes]
            sheet([ims[:3], ims[3:]] if len(ims) > 3 else [ims], f"{view}: HFRT {a.tag} modes", run / f"{view}__{a.tag}_modes.png")
        dbg = [m for m in a.dbg.split(",") if have(m)]
        if dbg:
            ims = [panel(run / f"{view}__{m}.png", LABEL.get(m, m), scale=0.75) for m in dbg]
            sheet([ims[:3], ims[3:]] if len(ims) > 3 else [ims], f"{view}: what the sun sees (white = lit)",
                  run / f"{view}__{a.tag}_debug.png")


if __name__ == "__main__":
    main()
