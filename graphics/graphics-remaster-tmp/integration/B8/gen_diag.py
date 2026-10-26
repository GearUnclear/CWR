"""B8 review diagnostics (not a gate): lowsun_fp and pine_eve with the beams
alone under temporary variant codes (volGain 5.x in a diagnostic build only).

    python gen_diag.py
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B8\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

OFF = "remastered,airBeams=0,airHaze=0,valleyMist=0"
BEAMS = "remastered,airHaze=0,valleyMist=0"


def block(v, name, sim_time, shots, settle=150):
    out = [f"\n// ---- {name}"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append(f"triSimFrames {settle}")
    for label, setting in shots:
        out += capgen.shot(v, name, setting, f"{name}__{label}")
    out += capgen.end(v)
    return out


def main():
    low = views._timing("lowsun_fp")
    low.pop("f10", None)
    low.pop("note", None)
    pine_eve = dict(name="pine_eve", x=4355, z=3275, eye=1.75, h=288, p=5, date=[1985, 7, 5, 18, 15], vd=1500)
    codes = [("off", OFF), ("beams", BEAMS), ("g1", BEAMS + ",volGain=1")]
    codes += [(f"c{k}", BEAMS + f",volGain=5.{k}") for k in (1, 2, 3, 4, 5, 6, 7, 8, 9)]
    lines = [capgen.header("Abel", low, "B8 review diagnostics: beam variants.")]
    lines += block(low, "b8_lowsun_fp", 10400, codes)
    lines += block(pine_eve, "b8_pine_eve", 9800, codes)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "b8diag.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b8diag.test.toml", capgen.toml("Abel", "B8 review diagnostics", views.WORLDS, timeout=1800))
    print(out / "b8diag.test.sqf")


if __name__ == "__main__":
    main()
