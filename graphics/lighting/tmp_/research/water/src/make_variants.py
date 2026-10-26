"""Write shaders_<id>/psDetailRemasterMain.glsl for each variant from water_template.glsl.

    python make_variants.py [A B C ...]
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
BASE = dict(WAVE_K=0.8, SWELL_K=0.0, RIPPLE_K=0.0, REFL_MAX=0.3, REFL_VAR=1.0, BROAD_POW=32.0, BROAD_W=2.5,
            TIGHT_POW=1500.0, TIGHT_W=1.0, SHEEN_W=0.35, FADE_START=400.0, FADE_END=3000.0, FADE_FLOOR=0.35,
            ANISO_FLOOR=0.3)
VARIANTS = {
    "A": dict(VARIANT=1, WAVE_K=0.6),
    "B": dict(VARIANT=2),
    "C": dict(VARIANT=3, SWELL_K=1.2, RIPPLE_K=0.004),
}


def main(argv):
    tpl = (HERE / "water_template.glsl").read_text(encoding="utf-8")
    for vid in (argv or list(VARIANTS)):
        p = dict(BASE)
        p.update(VARIANTS[vid])
        src = tpl
        for k, v in p.items():
            if k != "VARIANT":
                src = src.replace(f"{k}_VALUE", repr(float(v)))
        assert "_VALUE" not in src, "unfilled constant"
        src = f"\n#define VARIANT {p['VARIANT']}\n" + src
        d = ROOT / f"shaders_{vid}"
        d.mkdir(exist_ok=True)
        (d / "psDetailRemasterMain.glsl").write_text(src, encoding="utf-8", newline="\n")
        (d / "params.txt").write_text("\n".join(f"{k}={v}" for k, v in p.items()) + "\n", encoding="utf-8")
        print("wrote", d)


if __name__ == "__main__":
    main(sys.argv[1:])
