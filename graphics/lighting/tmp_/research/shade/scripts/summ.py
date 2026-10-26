"""Print measure_<view>.json tables: F, light g:r, light r:b per box, plus the
shade/lit ratios of the same paint.

    python summ.py <run_dir> <view> [<view> ...] [--alb <run_dir_for_albedo>]
"""
import json
import sys
from pathlib import Path

PAIRS = [
    ("m113", "m113_rear_shade135", "m113_side_lit45"),
    ("jeep", "jeep_rear_shade135", "jeep_side_lit45"),
    ("zelda", "zelda_rear_shade135", "zelda_side_lit45"),
    ("gshadow", "ground_shadow", "ground_lit"),
]


def main():
    run = sys.argv[1]
    for v in sys.argv[2:]:
        p = Path(run) / f"measure_{v}.json"
        if not p.exists():
            print("no", p)
            continue
        r = json.loads(p.read_text())
        names = list(next(iter(r.values())).keys())
        print(f"\n## {run} {v}  (F / g:r / r:b)")
        print("| label | " + " | ".join(names) + " | shade/lit |")
        print("|---|" + "---|" * (len(names) + 1))
        for lab, row in r.items():
            cells = [f"{row[n]['F']:.2f} / {row[n]['L'][1] / row[n]['L'][0]:.2f} / {row[n]['rb']:.2f}" for n in names]
            rat = []
            for tag, a, b in PAIRS:
                if a in row and b in row:
                    rat.append(f"{tag} {row[a]['F'] / max(row[b]['F'], 1e-6):.2f}")
            print(f"| {lab} | " + " | ".join(cells) + " | " + ", ".join(rat) + " |")


if __name__ == "__main__":
    main()
