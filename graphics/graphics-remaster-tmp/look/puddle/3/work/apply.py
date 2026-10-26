"""Apply the puddle-only tuning to a copy of the shader sources.

python apply.py <src dir> [key=value ...]
Each replacement must match exactly once.
"""
import sys
from pathlib import Path

V = dict(film="0.1", filmRough="0.55", sheen="0.08", damp="0.06", dampDark="0.9", cone="0.0", pudDark="0.55", pudGround="0.67", pudRoad="0.61")
for a in sys.argv[2:]:
    k, v = a.split("=", 1)
    V[k] = v
src = Path(sys.argv[1])


def rep(name, old, new):
    p = src / name
    t = p.read_text(encoding="utf-8")
    n = t.count(old)
    if n != 1:
        raise SystemExit(f"{name}: {n} matches for {old[:60]!r}")
    p.write_text(t.replace(old, new), encoding="utf-8", newline="")


S = "EngineGL33_LightingShadersSurface.cpp"
rep(S, "const float kWetGrassRough = 0.4;   // ... this rough\n",
    "const float kWetGrassRough = 0.4;   // ... this rough\n"
    "// Only puddles mirror. On the ground outside them (land, roads, grass) the\n"
    "// rain shows as darker colour; its film is this share of the above ...\n"
    f"const float kWetGroundFilm = {V['film']};\n"
    "// ... and at least this rough: a dim, broad glint near a close light, no\n"
    "// streak and no image.\n"
    f"const float kWetGroundFilmRough = {V['filmRough']};\n")
rep(S, "    float film = w * (1.0 - 0.75 * porosity) * (1.0 - veg) + kWetGrassFilm * w * veg;\n",
    "    float film = (w * (1.0 - 0.75 * porosity) * (1.0 - veg) + kWetGrassFilm * w * veg) * (ground ? kWetGroundFilm : 1.0);\n")
rep(S, "    float rough = mix(kWetFilmRough, kWetGrassRough, veg);\n",
    "    float rough = max(mix(kWetFilmRough, kWetGrassRough, veg), ground ? kWetGroundFilmRough : 0.0);\n")
if V["damp"] != "0.06":
    rep(S, "const float kPuddleDamp = 0.06;       // the damp margin below the threshold\n",
        f"const float kPuddleDamp = {V['damp']};       // the damp margin below the threshold\n")
if V["dampDark"] != "0.9":
    rep(S, "const float kWetDampDark = 0.9;     // the damp margin around a puddle\n",
        f"const float kWetDampDark = {V['dampDark']};    // the damp margin around a puddle\n")

if V["pudDark"] != "0.55":
    rep(S, "const float kWetPuddleDark = 0.55;  // the ground under a puddle's water\n",
        f"const float kWetPuddleDark = {V['pudDark']};  // the ground under a puddle's water\n")
if V["pudGround"] != "0.67":
    rep(S, "const float kPuddleGround = 0.67;     // on bare ground\n", f"const float kPuddleGround = {V['pudGround']};     // on bare ground\n")
if V["pudRoad"] != "0.61":
    rep(S, "const float kPuddleRoad = 0.61;       // noise threshold on roads\n", f"const float kPuddleRoad = {V['pudRoad']};       // noise threshold on roads\n")

T = "EngineGL33_PostSsrShaders.cpp"
rep(T, "const float kSsrWetRoadDark = 0.8;\n",
    "// Under the wet weather only puddles mirror: the wet ground outside them\n"
    "// reflects this share of B6's sheen (a faint lift, no image).\n"
    f"const float kSsrWetSheen = {V['sheen']};\n"
    "const float kSsrWetRoadDark = 0.8;\n")
rep(T, "        float weight = mix(road ? 0.65 * wet : 0.5 * wet * (1.0 - 0.85 * grass), 1.0, puddle);\n",
    "        float sheen = (road ? 0.65 * wet : 0.5 * wet * (1.0 - 0.85 * grass)) * (wetWeather ? kSsrWetSheen : 1.0);\n"
    "        float weight = mix(sheen, 1.0, puddle);\n")
if V["cone"] != "0.0":
    rep(T, "        float cone = mix(road ? 0.10 : 0.22, 0.008, puddle);\n",
        f"        float cone = mix(max(road ? 0.10 : 0.22, wetWeather ? {V['cone']} : 0.0), 0.008, puddle);\n")
print("applied", V)
