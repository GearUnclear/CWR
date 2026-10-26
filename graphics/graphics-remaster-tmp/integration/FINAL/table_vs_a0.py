"""The final timing table against plan 5 A0: pooled clean shots (pool_*.json)
of the finished branch with everything on, beside A0's Remastered row."""
import json
import sys
from pathlib import Path

F = Path(__file__).resolve().parent
# plan 5 A0: Remastered frame median / min, world, post, CPU frame, draws, MHz
A0 = {
    "sunrise_ridge": (17.41, 17.34, 16.26, 1.01, 26.57, 12910, "2,790 (P0)"),
    "forest_edge_golden": (6.18, 5.58, 5.40, 0.58, 8.71, 2141, "1,680 (P0/P3)"),
    "town_night": (6.53, 5.42, 5.64, 0.63, 8.15, 1228, "1,860 (P0/P3)"),
    "coastline_sunset": (3.81, 1.88, 1.60, 1.95, 4.10, 178, "480 (P3/P5)"),
    "woods_pine2_evening": (6.47, 6.45, 5.21, 1.14, 9.25, 2840, "1,830 (P0)"),
    "lowsun_fp": (5.59, 4.60, 4.34, 1.08, 8.30, 1891, "1,500 (P0/P3)"),
    "road_flat_drive_moonless": (5.27, 4.52, 4.46, 0.59, 7.69, 1806, "1,950 (P0/P3)"),
    "ffh": (6.06, 5.66, 5.22, 0.69, 7.73, 796, "1,905 (P0/P3)"),
    "bay_offshore": (7.73, 7.72, 7.11, 0.49, 12.44, 4557, "2,775 (P0/P3)"),
    "town_night_rain": (5.23, 5.13, 4.32, 0.79, 7.10, 809, "1,575 (P0/P3)"),
    "pinewood_morn": (5.84, 5.46, 4.67, 0.95, 8.58, 2844, "1,852 (P0/P3)"),
    "valley_0515": (23.30, 22.47, 21.99, 1.16, 31.02, 11404, "2,790 (P0)"),
    "hill_1730": (1.99, 1.58, 1.49, 0.38, 3.51, 1663, "1,815 (P0/P5)"),
    "coast_oc_up": (3.87, 1.73, 1.64, 1.90, 3.61, 186, "990 (P0/P3/P5)"),
    "sea_noon": (4.56, 1.20, 3.01, 1.31, 5.41, 180, "765 (P0/P3/P5)"),
    "noon_fields": (9.79, 9.06, 9.16, 0.49, 14.79, 6399, "2,490 (P0)"),
    "overcast_broken": (11.85, 11.77, 11.10, 0.65, 18.51, 8066, "2,790 (P0)"),
    "broken_high": (24.31, 24.12, 23.42, 0.79, 27.35, 10784, "2,790 (P0)"),
    "sinai_lowsun_fp": (5.06, 4.24, 3.75, 1.21, 7.46, 764, "818 (P0/P3/P5)"),
    "sinai_desert_noon": (9.70, 9.31, 8.36, 1.08, 14.42, 2672, "1,245 (P3/P5)"),
}


def main():
    res = {}
    for p in sys.argv[1:]:
        res.update(json.loads(Path(p).read_text(encoding="utf-8")))
    out = ["| view | A0 frame (med / min) | final frame (med / min) | change (min) | final world | final post | "
           "A0 CPU | final CPU | A0 draws | final draws | final clean shots | final GPU MHz (P) |",
           "|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for v, a in A0.items():
        r = res.get(f"{v}|remastered")
        if not r or not r.get("frame"):
            out.append(f"| {v} | {a[0]:.2f} / {a[1]:.2f} | no clean shot | | | | {a[4]:.2f} | | {a[5]:,} | | | |")
            continue
        fm, fn = r["frame"]
        out.append("| %s | %.2f / %.2f | %.2f / %.2f | %+.2f | %.2f | %s | %.2f | %.2f | %s | %s | %d of %d | %s (%s) |" % (
            v, a[0], a[1], fm, fn, fn - a[1], r["world"][0], "%.2f" % r["post"][0] if r.get("post") else "",
            a[4], r["cpu"], f"{a[5]:,}", f"{r['draws']:,.0f}", r["clean"], r["all"],
            f"{r['mhz']:,.0f}" if r["mhz"] else "", "/".join(r["ps"])))
    print("\n".join(out))


if __name__ == "__main__":
    main()
