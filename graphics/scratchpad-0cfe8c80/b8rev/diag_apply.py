"""Temporary diagnostic edits (never committed): volGain 5.x selects a variant."""
from pathlib import Path
E = Path(r"D:\Arma_CWA_worktrees\gr-B8\arma_CWR\engine\PoseidonGL33")
def rep(path, old, new, count=1):
    s = path.read_text(encoding="utf-8")
    assert s.count(old) == count, (path.name, old[:60], s.count(old))
    path.write_text(s.replace(old, new), encoding="utf-8", newline="\n")

sh = E / "EngineGL33_PostAirShaders.cpp"
rep(sh, "uniform vec4 airSpots;      // x headlight count, y their march steps\n",
    "uniform vec4 airSpots;      // x headlight count, y their march steps\nuniform vec4 airDiag;\n")
rep(sh, """        float optical = 0.0;
        float sumLit = 0.0;
""", """        float optical = 0.0;
        float sumLit = 0.0;
        float sumOpen = 0.0;
        float capF = 1.0;
        if (airDiag.x == 6.0) capF = min(1.0, 60.0 / max(min(tEnd, 2.0 * airMedium.w / max(abs(wdir.y), 1e-3)), 1.0));
""")
rep(sh, """                          airMedium.z * exp(-max(hA, 0.0) / airMedium.w);
            float w = sigma * exp(-optical) * dt;
            optical += sigma * dt;
            float vis = airShadowAt(rel) * cloudSunVisibility(pa);""",
"""                          capF * airMedium.z * exp(-max(hA, 0.0) / airMedium.w);
            float w = sigma * exp(-optical) * dt;
            optical += sigma * dt;
            sumOpen += w;
            float vis = (airDiag.x == 2.0 ? 1.0 : airShadowAt(rel)) * cloudSunVisibility(pa);""")
rep(sh, """        sunBeams = phase * 0.25 * sumLit * airSun.x;
    }
""", """        sunBeams = phase * 0.25 * sumLit * airSun.x;
        sunOpenDiag = phase * 0.25 * sumOpen * airSun.x;
    }
""")
rep(sh, """    float sunBeams = 0.0;
    if (airSun.x > 0.0) {""", """    float sunBeams = 0.0;
    float sunOpenDiag = 0.0;
    if (airSun.x > 0.0) {""")
rep(sh, """    fragColor = vec4(sunBeams, spots, vis, min(z, 6e4));""",
"""    if (airDiag.x == 5.0 || airDiag.x == 7.0 || airDiag.x == 8.0 || airDiag.x == 9.0) spots = sunOpenDiag;
    fragColor = vec4(sunBeams, spots, vis, min(z, 6e4));""")
rep(sh, "uniform vec4 airFog;          // x the classic fog's start as the world drew it (m), y 1 / (end - start), z 1 = on\n",
    "uniform vec4 airFog;          // x the classic fog's start as the world drew it (m), y 1 / (end - start), z 1 = on\nuniform vec4 airDiag;\n")
rep(sh, """        vec2 shaft = air.rg;
        // No half-resolution tap matched this pixel's depth: the upsample
        // borrowed a neighbour's (the sky's) scattering. Clamp it to the mean
        // and give it no boost, so crowns against the glow get no bright rim.
        shaft = mix(min(shaft, mean), shaft, conf);
        float boost = (airBeams.z - 1.0) * conf;
        vec2 floorAt = (1.0 - airBeams.y) * shaft;
        shaft = max(shaft - airBeams.y * mean, floorAt) + boost * max(shaft - mean, vec2(0.0));
        glow = airSunColor.rgb * shaft.x + airSpotTint.rgb * shaft.y;""",
"""        vec2 shaft = air.rg;
        float code = airDiag.x;
        float sl = dot(airSunColor.rgb, kLuma);
        if (code == 3.0 || code == 4.0 || code == 7.0) {
            float X = code == 3.0 ? air.r : (code == 4.0 ? mean.x : air.g);
            fragColor = vec4(toDisplay(vec3(clamp(4.0 * sl * X, 0.0, 1.0))), 1.0);
            return;
        }
        if (code == 5.0 || code == 8.0 || code == 9.0) {
            float open = max(air.g, 1e-7);
            float mo = max(mean.y, 1e-7);
            float v = clamp(air.r / open, 0.0, 1.0);
            float mv = clamp(mean.x / mo, 0.0, 1.0);
            v = mix(min(v, mv), v, conf);
            float hpv = code == 5.0 ? airBeams.y : 1.0;
            float boostv = (airBeams.z - 1.0) * conf;
            float s = max(v - hpv * mv, (1.0 - hpv) * v) + boostv * max(v - mv, 0.0);
            if (code == 9.0) s = (1.0 - airBeams.y) * v + boostv * max(v - mv, 0.0);
            glow = airSunColor.rgb * (open * s);
        } else {
        float hp = code == 1.0 ? 1.0 : airBeams.y;
        // No half-resolution tap matched this pixel's depth: the upsample
        // borrowed a neighbour's (the sky's) scattering. Clamp it to the mean
        // and give it no boost, so crowns against the glow get no bright rim.
        shaft = mix(min(shaft, mean), shaft, conf);
        float boost = (airBeams.z - 1.0) * conf;
        vec2 floorAt = (1.0 - hp) * shaft;
        shaft = max(shaft - hp * mean, floorAt) + boost * max(shaft - mean, vec2(0.0));
        glow = airSunColor.rgb * shaft.x + airSpotTint.rgb * shaft.y;
        }""")

pa = E / "EngineGL33_PostAir.cpp"
rep(pa, "        _airMedium = rm::ComputeAirMedium(in, parts, _lighting.volGain);",
    "        _airMedium = rm::ComputeAirMedium(in, parts, _lighting.volGain >= 4.95f ? -1.0f : _lighting.volGain);")
rep(pa, """    float viewToWorld[9];
    ViewToWorld(_frameState.view, viewToWorld);
""", """    float viewToWorld[9];
    ViewToWorld(_frameState.view, viewToWorld);
    const float diag = _lighting.volGain >= 4.95f ? std::round((_lighting.volGain - 5.0f) * 10.0f) : 0.0f;
""")
rep(pa, """        Set4(prog, "airSpots", static_cast<float>(spots), static_cast<float>(rm::kAirSpotSteps), 0.0f, 0.0f);""",
"""        Set4(prog, "airSpots", static_cast<float>(spots), static_cast<float>(rm::kAirSpotSteps), 0.0f, 0.0f);
        Set4(prog, "airDiag", diag, 0.0f, 0.0f, 0.0f);""")
rep(pa, """    Set4(prog, "airBeams", beams ? 1.0f : 0.0f, m.beamHighpass, m.beamGain, 1.0f);""",
"""    Set4(prog, "airBeams", beams ? 1.0f : 0.0f, m.beamHighpass, m.beamGain, 1.0f);
    Set4(prog, "airDiag", diag, 0.0f, 0.0f, 0.0f);""")
print("ok")
