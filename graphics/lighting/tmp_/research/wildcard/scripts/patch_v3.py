"""Scratch-worktree edit: NaN-proof sky normal, leaf-tinted transmission, sky gain."""
from pathlib import Path

E = Path(r"D:\Arma_CWA_worktrees\gfx-wildcard\arma_CWR\engine")


def sub(path, old, new):
    p = E / path
    s = p.read_text(encoding="utf-8")
    assert old in s, (path, old[:60])
    p.write_text(s.replace(old, new, 1), encoding="utf-8")


SH = "PoseidonGL33/EngineGL33_LightingShaders.cpp"
# 1. NaN-proof normal for the maps
sub(SH, """    vec3 nSky = (any(isnan(nLight)) || any(isinf(nLight))) ? nCast : nLight;""",
    """    // Leaf cards with zero mesh normals give a NaN n (normalize(0)) as well.
    vec3 nSky = nLight;
    if (any(isnan(nSky)) || any(isinf(nSky))) {
        float gc = crownOk ? length(vCrown.xyz) : 0.0;
        bool nOk = !(any(isnan(n)) || any(isinf(n)));
        nSky = gc > 1e-6 ? vCrown.xyz / gc : (nOk ? n : vec3(0.0, 1.0, 0.0));
    }""")
# 2. per-channel visibility with leaf-tinted transmission
sub(SH, """    float leaf = clamp(m.w / c, 0.0, 1.0);
    return clamp(c, 0.0, 1.0) * p * (1.0 - skyCtl4.x * leaf);
}""", """    float leaf = clamp(m.w / c, 0.0, 1.0);
    g_occLeaf = clamp(c, 0.0, 1.0) * p * leaf;
    return clamp(c, 0.0, 1.0) * p * (1.0 - skyCtl4.x * leaf);
}""")
sub(SH, """// Occluded fraction of a filtered footprint for a receiver at depth z: the""",
    """// Share of the last skyOccluded footprint that is foliage between the
// receiver and the sky (before transmission).
float g_occLeaf = 0.0;

// Occluded fraction of a filtered footprint for a receiver at depth z: the""")
sub(SH, """        float vis = 1.0;
        vec3 pl = p + skyOff[i].xyz;""", """        float vis = 1.0;
        vec3 visTint = vec3(0.0);
        vec3 pl = p + skyOff[i].xyz;""")
sub(SH, """                float occ = skyOccluded(m1, z);
                occ = clamp((occ - skyCtl2.x) / (1.0 - skyCtl2.x), 0.0, 1.0);
                vis = 1.0 - occ;""", """                float occ = skyOccluded(m1, z);
                float leafOcc = g_occLeaf;
                occ = clamp((occ - skyCtl2.x) / (1.0 - skyCtl2.x), 0.0, 1.0);
                vis = 1.0 - occ;
                // The light foliage lets through takes the leaves' colour
                // (skyCtl3.z: from the caster textures): green shade in woods.
                if (skyCtl4.w > 0.0 && skyCtl3.z > 0.5 && leafOcc > 0.0) {
                    vec4 la = textureLod(skyAlbedo, vec3(uv, float(i)), lod);
                    if (la.a > 1e-3) {
                        vec3 lc = toLinear(la.rgb / la.a);
                        vec3 tn = lc / max(dot(lc, kLuma), 1e-4);
                        float tr = min(skyCtl4.x * leafOcc, vis);
                        visTint = tr * (mix(vec3(1.0), clamp(tn, vec3(0.0), vec3(3.0)), skyCtl4.w) - vec3(1.0));
                    }
                }""")
sub(SH, """        sumW += w;
        sumV += w * vis;
        sumC += cw;""", """        sumW += w;
        sumV += w * max(vec3(vis) + visTint, vec3(0.0));
        sumC += cw;""")
# 3. sky gain while the maps are on
sub(SH, """    vec3 ambient = ka * (skyIrradiance(nLight) * mix(vec3(1.0), skyV, upShare) + g_skyBounce * upShare) *""",
    """    float skyGain = skyCtl.x > 0.5 ? skyGainU() : 1.0;
    vec3 ambient = ka * (skyIrradiance(nLight) * mix(vec3(1.0), skyV, upShare) * skyGain + g_skyBounce * upShare) *""")
sub(SH, """vec3 g_skyBounce = vec3(0.0);""", """vec3 g_skyBounce = vec3(0.0);
float skyGainU() { return skyCtl5.x > 0.0 ? skyCtl5.x : 1.0; }""")
sub(SH, """    vec4 skyCtl4;      // x foliage transmission, y crown shell offset, z visibility floor
};""", """    vec4 skyCtl4;      // x foliage transmission, y crown shell offset, z visibility floor, w leaf tint
    vec4 skyCtl5;      // x sky gain while the maps are on
};""")

GL = "PoseidonGL33/EngineGL33_ShadowDepth.cpp"
sub(GL, """    float ctl4[4]; // x leaf transmission, y crown shell offset scale, z visibility floor
};""", """    float ctl4[4]; // x leaf transmission, y crown shell offset scale, z visibility floor, w leaf tint
    float ctl5[4]; // x sky gain while the maps are on
};""")
sub(GL, """    s_sky.ctl4[3] = 0.0f;""", """    s_sky.ctl4[3] = t.skyTint;
    s_sky.ctl5[0] = t.skyGain;""")

LR = "Poseidon/Graphics/Remaster/LightRig.hpp"
sub(LR, """    float skyFloor = 0.0f;   // lower bound of the sky visibility""",
    """    float skyFloor = 0.0f;   // lower bound of the sky visibility
    float skyTint = 0.0f;    // 0..1: light through foliage takes the leaf colour (needs skyAlbedo)
    float skyGain = 1.0f;    // sky fill gain while the maps are on (open ground brighter, occluded darker)""")
TR = "Poseidon/Game/Commands/GameStateExtTestRender.cpp"
sub(TR, """        {"skyFloor", &remaster::LightingTuning::skyFloor},""",
    """        {"skyFloor", &remaster::LightingTuning::skyFloor},
        {"skyTint", &remaster::LightingTuning::skyTint},
        {"skyGain", &remaster::LightingTuning::skyGain},""")
print("ok")
