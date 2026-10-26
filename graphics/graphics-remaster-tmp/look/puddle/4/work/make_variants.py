"""Fix variants on top of the fix in the scratch copy (puddle agent 4).

Selector as in make_probe.py: wetness = 1 - p / 1000.
  0   the fix as built
  8   the puddle mask (red water, green the damp margin)
  11  more puddles: threshold - 0.03
  12  more puddles: threshold - 0.06
  13  no ground glint at all (lamps and sun)
  14  no film darkening (kWetGroundFilmDark 0)
  15  threshold - 0.04 and glint share x 0.5
python make_variants.py <src dir (copies)> <out dir>
"""
import shutil
import subprocess
import sys
from pathlib import Path

src = Path(sys.argv[1])
out = Path(sys.argv[2])
NL = chr(10)


def sub(path, old, new, count=1):
    old = old.replace("@", NL)
    new = new.replace("@", NL)
    t = path.read_text(encoding="utf-8")
    n = t.count(old)
    if n != count:
        raise SystemExit(f"{path.name}: expected {count} of {old!r}, found {n}")
    path.write_text(t.replace(old, new), encoding="utf-8")


surf = src / "EngineGL33_LightingShadersSurface.cpp"
lit = src / "EngineGL33_LightingShaders.cpp"
ssr = src / "EngineGL33_PostSsrShaders.cpp"

sub(surf, 'const char kRemasterPuddlesGLSL[] = R"(@',
    'const char kRemasterPuddlesGLSL[] = R"(@'
    'int wetProbe() { return litWeather.x > 0.5 ? int(floor((1.0 - litWeather.x) * 1000.0 + 0.5)) : -1; }@'
    'bool wetProbeIs(int a) { return wetProbe() == a; }@')
sub(surf, "kPuddleGrass, veg * veg) + kPuddleLevelShift * (1.0 - level);",
    "kPuddleGrass, veg * veg) + kPuddleLevelShift * (1.0 - level)@"
    "        - (wetProbeIs(11) ? 0.03 : 0.0) - (wetProbeIs(12) ? 0.06 : 0.0) - (wetProbeIs(15) ? 0.04 : 0.0);")
sub(surf, "    if (ground) a *= 1.0 - kWetGroundFilmDark * film * (1.0 - pud.x);",
    "    if (ground) a *= 1.0 - (wetProbeIs(14) ? 0.0 : kWetGroundFilmDark) * film * (1.0 - pud.x);")
sub(surf, "(film * (1.0 - pud.x) * (ground ? kWetSunGroundFilm : 1.0));",
    "(film * (1.0 - pud.x) * (ground ? kWetSunGroundFilm * (wetProbeIs(13) ? 0.0 : 1.0) * (wetProbeIs(15) ? 0.5 : 1.0) : 1.0));",
    count=2)
sub(surf, "    mirror = pud.x;@", "    mirror = pud.x;@    g_probePud = pud;@")
sub(surf, "int g_wetDraw = 0;@", "int g_wetDraw = 0;@vec2 g_probePud = vec2(0.0);@")
sub(lit, "    vec3 display = litDisplay(radiance);@",
    "    if (wetProbeIs(8) && (terrain || litMatClass.y > 7.5)) {@"
    "        ambientShare = 0.0;@"
    "        return vec3(g_probePud.x, g_probePud.y * (1.0 - g_probePud.x), 0.0);@"
    "    }@"
    "    vec3 display = litDisplay(radiance);@")
sub(ssr, "        float weight = wetWeather ? puddle : ",
    "        float weight = wetProbeIs(8) ? 0.0 : wetWeather ? puddle : ")

dump = out / "all"
if dump.exists():
    shutil.rmtree(dump)
tool = Path(r"D:/Arma_CWA_worktrees/look-puddle-4/arma_CWR/tests/rendering/lighting_lab/glsl_extract.py")
for f in ("EngineGL33_LightingShaders.cpp", "EngineGL33_PostSsrShaders.cpp"):
    subprocess.run([sys.executable, "-I", str(tool), "--source", str(src / f), "--dump", str(dump)], check=True,
                   stdout=subprocess.DEVNULL)
keep = ["psRemasterCommon", "psPostSsrTrace"]
out.mkdir(parents=True, exist_ok=True)
for k in keep:
    shutil.copy(dump / f"{k}.glsl", out / f"{k}.glsl")
shutil.rmtree(dump)
print("ok", [p.name for p in out.iterdir()])
