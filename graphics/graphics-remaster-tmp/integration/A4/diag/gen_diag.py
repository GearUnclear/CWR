"""A4 scratch: which switch changes which pixels at sunrise_ridge. Each pair is
shot in one game: <name>__off then <name>__on, with the settings below."""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = HERE.parents[3] / "arma_CWR" / "tests" / "rendering" / "captures"
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

BOTH0 = "remastered,drawGrouping=0,hillCull=0"
PAIRS = [
    ("d_same_off", BOTH0, BOTH0),
    ("d_same_on", "remastered", "remastered"),
    ("d_hill", BOTH0, "remastered,drawGrouping=0"),
    ("d_group", BOTH0, "remastered,hillCull=0"),
    ("d_both", BOTH0, "remastered"),
]
EXTRA = sys.argv[1] if len(sys.argv) > 1 else ""
NAME = sys.argv[2] if len(sys.argv) > 2 else "diag_sunrise"
BASE = sys.argv[3] if len(sys.argv) > 3 else "sunrise_ridge"


def main():
    base = views._timing(BASE)
    lines = [capgen.header("Abel", base, f"A4 diag {NAME}: which switch changes pixels.")]
    t = 1000
    for name, a, b in PAIRS:
        v = dict(base)
        v["name"] = name
        a = a + ("," + EXTRA if EXTRA else "")
        b = b + ("," + EXTRA if EXTRA else "")
        lines.append(f"\n// ---- {name}")
        lines += capgen.begin(v, name, sim_time=t)
        t += 100
        lines.append("triSimFrames 150")
        lines += capgen.shot(v, name, a, f"{name}__off")
        lines += capgen.shot(v, name, b, f"{name}__on")
        lines += capgen.end(v)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    capgen.write(HERE / f"{NAME}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(HERE / f"{NAME}.test.toml", capgen.toml("Abel", "A4 diag", views.WORLDS, timeout=3600))
    print(NAME)


main()
