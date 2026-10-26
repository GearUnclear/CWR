"""Probe shaders for the wet-ground audit (puddle agent 4).

The probe number is carried by the forced wetness knob: wetness = 1 - p / 1000
(litWeather.x in the forward pass, the same block in the SSR trace). p = 0 is
the reference; every probe takes one term out:
  1  film lamp streaks (wetLampStreaks on the film, non-puddle)
  2  film sun / moon glint (non-puddle)
  3  SSR trace on non-puddle ground (its weight kept only for puddles)
  4  the surface lane's specular on the ground (sp.sun, sp.sky, g_physicalSpec)
  5  every puddle term (puddle streaks, puddle glint, puddle SSR mirror)
  6  every non-puddle reflective term at once (1 + 2 + 3 + 4 + 7)
  7  the forward sky reflection in wetSurface (env) on the ground
  8  the puddle mask (red standing water, green the damp margin; no SSR)
python make_probe.py <src dir (copies of the shader cpp files)> <out dir>
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
# film sun glint
sub(surf, "               (film * (1.0 - pud.x) * (ground ? kWetSunGroundFilm : 1.0));",
    "               (film * (1.0 - pud.x) * (ground ? kWetSunGroundFilm : 1.0)) *@"
    "               ((ground && (wetProbeIs(2) || wetProbeIs(6))) ? 0.0 : 1.0);")
# puddle sun glint
sub(surf, "        sun += wetLobe(litSunDir.xyz, 1e4, nf, v, nv, ap, kWetSunCap) * pud.x;",
    "        sun += wetLobe(litSunDir.xyz, 1e4, nf, v, nv, ap, kWetSunCap) * pud.x * (wetProbeIs(5) ? 0.0 : 1.0);")
# film lamp streaks and puddle lamp streaks
sub(surf, "        if (pud.x < 1.0) lit += wetLampStreaks(nf, v, rough) * (film * (1.0 - pud.x));",
    "        if (pud.x < 1.0) lit += wetLampStreaks(nf, v, rough) * (film * (1.0 - pud.x)) *@"
    "                                ((ground && (wetProbeIs(1) || wetProbeIs(6))) ? 0.0 : 1.0);")
sub(surf, "        if (pud.x > 0.0) lit += wetLampStreaks(nf, v, kWetPuddleRough) * pud.x;",
    "        if (pud.x > 0.0) lit += wetLampStreaks(nf, v, kWetPuddleRough) * pud.x * (wetProbeIs(5) ? 0.0 : 1.0);")
# forward sky env on the ground
sub(surf, "        env = sky * (F * mix(0.3, 1.0, skyVisibility) * litSunDir.w);",
    "        if (ground && (wetProbeIs(7) || wetProbeIs(6))) F = 0.0;@"
    "        env = sky * (F * mix(0.3, 1.0, skyVisibility) * litSunDir.w);")
# p8 mask
sub(surf, "    mirror = pud.x;@", "    mirror = pud.x;@    g_probePud = pud;@")
sub(surf, "int g_wetDraw = 0;@", "int g_wetDraw = 0;@vec2 g_probePud = vec2(0.0);@")

sub(lit, "        radiance = (diffuseLight + specSun + specSky + g_physicalSpec * litGround.w + combatGloss + wetLit) * exposure +",
    "        if ((wetProbeIs(4) || wetProbeIs(6)) && (terrain || litMatClass.y > 7.5)) {@"
    "            specSun = vec3(0.0);@"
    "            specSky = vec3(0.0);@"
    "            g_physicalSpec = vec3(0.0);@"
    "        }@"
    "        radiance = (diffuseLight + specSun + specSky + g_physicalSpec * litGround.w + combatGloss + wetLit) * exposure +")
sub(lit, "    vec3 display = litDisplay(radiance);@",
    "    if (wetProbeIs(8) && (terrain || litMatClass.y > 7.5)) {@"
    "        ambientShare = 0.0;@"
    "        return vec3(g_probePud.x, g_probePud.y * (1.0 - g_probePud.x), 0.0);@"
    "    }@"
    "    vec3 display = litDisplay(radiance);@")

sub(ssr, "        float weight = mix(road ? 0.65 * wet : 0.5 * wet * (1.0 - 0.85 * grass), 1.0, puddle);",
    "        float weight = mix(road ? 0.65 * wet : 0.5 * wet * (1.0 - 0.85 * grass), 1.0, puddle);@"
    "        if (wetProbeIs(3) || wetProbeIs(6)) weight = puddle;@"
    "        if (wetProbeIs(8)) weight = 0.0;@"
    "        if (wetProbeIs(5)) weight = (road ? 0.65 * wet : 0.5 * wet * (1.0 - 0.85 * grass)) * (1.0 - puddle);")

dump = out / "all"
if dump.exists():
    shutil.rmtree(dump)
tool = Path(r"D:/Arma_CWA_worktrees/look-puddle-4/arma_CWR/tests/rendering/lighting_lab/glsl_extract.py")
for f in ("EngineGL33_LightingShaders.cpp", "EngineGL33_PostSsrShaders.cpp"):
    subprocess.run([sys.executable, "-I", str(tool), "--source", str(src / f), "--dump", str(dump)], check=True)
keep = ["psRemasterCommon", "psPostSsrTrace"]
out.mkdir(parents=True, exist_ok=True)
for k in keep:
    shutil.copy(dump / f"{k}.glsl", out / f"{k}.glsl")
shutil.rmtree(dump)
print("ok", [p.name for p in out.iterdir()])
