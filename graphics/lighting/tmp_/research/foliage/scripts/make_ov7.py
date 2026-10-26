"""Build ov7 from ov6: geometric trunk mask instead of the alpha-test flag, and
the canopy-map functions (needs the gfx-foliage build with litResearch2/3 and
the canopyMap sampler on unit 6)."""
import shutil
from pathlib import Path

R = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\foliage")
src, dst = R / "ov6", R / "ov7"
dst.mkdir(exist_ok=True)
for f in src.glob("*.glsl"):
    shutil.copy2(f, dst / f.name)
P = dst / "psRemasterCommon.glsl"
s = P.read_text(encoding="utf-8")
crlf = "\r\n" in s


def R_(a, b):
    global s
    if crlf:
        a = a.replace("\n", "\r\n")
        b = b.replace("\n", "\r\n")
    assert s.count(a) == 1, (a[:70], s.count(a))
    s = s.replace(a, b)


R_("""    vec4 litResearch1;    // 59: foliage research knobs f4..f7
};""", """    vec4 litResearch1;    // 59: foliage research knobs f4..f7
    vec4 litResearch2;    // 60: canopy research knobs c0..c3
    vec4 litResearch3;    // 61: canopy research knobs c4..c7
};""")
R_("""uniform sampler2D cloudNoise;              // unit 5: equalised tileable noise
""", """uniform sampler2D cloudNoise;              // unit 5: equalised tileable noise
uniform sampler2D canopyMap;               // unit 6 (research): r cover, g top * cover, b base * cover (m above ground)
""")
# terrain height of the last terrainSunVisibility call
R_("""vec3 g_groundTint = vec3(1.0);
""", """vec3 g_groundTint = vec3(1.0);
// Terrain height under the last shaded point (research: canopy map), -1e6 = no map.
float g_groundY = -1e6;
""")
R_("""    vec4 hs = texture(terrainHeight, uv);
    g_groundTint = vec3(hs.b, (1.0 - 0.2126 * hs.b - 0.0722 * hs.a) / 0.7152, hs.a);""",
   """    vec4 hs = texture(terrainHeight, uv);
    g_groundY = hs.x;
    g_groundTint = vec3(hs.b, (1.0 - 0.2126 * hs.b - 0.0722 * hs.a) / 0.7152, hs.a);""")
# canopy functions after cloudSunVisibility
R_("""    return mix(1.0, litCloud.y, covered);
}
""", """    return mix(1.0, litCloud.y, covered);
}

// ---- canopy map research (knobs c0..c7) -----------------------------------
// c0  canopy sky occlusion strength (0 off)
// c1  canopy sun shadow beyond the shadow-map range (0 off, 1 full)
// c2  debug: 1 cover, 2 canopy sun visibility, 3 canopy sky occlusion
// c3  extinction per metre of canopy along the sun ray (default 0.12)
vec2 canopyUV(vec2 xz) { return xz * (litTerrain.y * litTerrain.z); }

// Share of the sky hidden by the canopy around and above a point: the crown
// right above (6 m texels, skipped for leaves, whose own crown is the crown
// model's business) and the stand around it (25 m and 100 m mips), each
// counted while the point is below that canopy's top.
float canopySkyOcclusion(vec3 posAbs, bool foliage) {
    if (g_groundY < -9e5) return 0.0;
    vec2 uv = canopyUV(posAbs.xz);
    float h = posAbs.y - g_groundY;
    vec4 c0 = textureLod(canopyMap, uv, 0.0);
    vec4 c2 = textureLod(canopyMap, uv, 2.0);
    vec4 c4 = textureLod(canopyMap, uv, 4.0);
    float top0 = c0.g / max(c0.r, 1e-3);
    float base0 = c0.b / max(c0.r, 1e-3);
    float top2 = c2.g / max(c2.r, 1e-3);
    float top4 = c4.g / max(c4.r, 1e-3);
    float below0 = clamp((top0 - h) / max(top0 - base0, 2.0), 0.0, 1.0);
    float below2 = clamp((top2 - h) / max(0.6 * top2, 2.0), 0.0, 1.0);
    float below4 = clamp((top4 - h) / max(0.6 * top4, 2.0), 0.0, 1.0);
    float occ = 0.3 * c2.r * below2 + 0.25 * c4.r * below4;
    if (!foliage) occ += 0.45 * c0.r * below0;
    return clamp(occ, 0.0, 1.0);
}

// Sun through the canopy layer: a march toward the sun over the canopy map
// (and the terrain height), steps growing from 3 m to ~45 m, out to ~170 m.
float canopySunVisibility(vec3 posAbs, float startT) {
    vec3 l = litSunDir.xyz;
    if (l.y <= 0.02 || g_groundY < -9e5) return 1.0;
    float sigma = litResearch2.w > 0.0 ? litResearch2.w : 0.12;
    float occ = 0.0;
    float t = max(startT, 1.5);
    float stepLen = 3.0;
    for (int i = 0; i < 10; ++i) {
        vec3 p = posAbs + l * (t + 0.5 * stepLen);
        vec4 c = textureLod(canopyMap, canopyUV(p.xz), 0.0);
        float g = texture(terrainHeight, (p.xz * litTerrain.y + 0.5) * litTerrain.z).r;
        float h = p.y - g;
        float top = c.g / max(c.r, 1e-3);
        float base = c.b / max(c.r, 1e-3);
        float inside = c.r * smoothstep(top + 0.5, top - 1.5, h) * smoothstep(base - 1.5, base + 0.5, h);
        occ += inside * stepLen * sigma;
        if (h > 45.0) break;
        t += stepLen;
        stepLen *= 1.35;
    }
    return exp(-occ);
}

// 0 inside the shadow-map range, 1 beyond it (and everywhere without maps).
float canopyFarWeight() {
    if (shadowCtl.x < 0.5) return 1.0;
    int nC = int(cascadeCtl.x);
    if (nC < 1) return 1.0;
    float last = cascadeSplits[nC - 1];
    float eyeDepth = dot(vWorldRel, camFwd.xyz);
    return smoothstep(last - max(cascadeCtl.y, 1.0), last, eyeDepth);
}
""")
# trunk mask: replace the alpha-flag gate
R_("""    float leafDraw = (alphaRef.y > 0.5 || alphaRef.z > 0.5) ? 1.0 : 0.0;""",
   """    // (Alpha test turned out to be on for whole trees, trunks included.) A
    // trunk is the thin column on the crown's vertical axis below the crown:
    // keep bark out of the leaf terms by that geometry instead.
    float trunk = (1.0 - smoothstep(0.07, 0.16, length(vCrownE.xz))) * (1.0 - smoothstep(-0.55, -0.35, vCrownE.y));
    float leafDraw = 1.0 - trunk;""")
R_("""        return vec3(alphaRef.y, alphaRef.z, 0.0);""", """        return vec3(leafDraw, 1.0 - leafDraw, 0.0);""")
# foliage: canopy terms
R_("""    float visExit = tVis * cVis;
    if (sunOn > 0.0 && (transK > 0.0 || msK > 0.0) && visExit > 0.0)
        visExit *= shadowMapVisibilityAt(vWorldRel + l * (dSunM + 0.75));""",
   """    float visExit = tVis * cVis;
    if (sunOn > 0.0 && (transK > 0.0 || msK > 0.0) && visExit > 0.0)
        visExit *= shadowMapVisibilityAt(vWorldRel + l * (dSunM + 0.75));
    float canSun = 1.0;
    float canSky = 0.0;
    if (litResearch2.x > 0.0 || litResearch2.y > 0.0) {
        canSky = canopySkyOcclusion(posAbs, true);
        skyVisibility *= 1.0 - litResearch2.x * canSky;
        float far = canopyFarWeight() * litResearch2.y;
        if (far > 0.0 && sunOn > 0.0) {
            canSun = mix(1.0, canopySunVisibility(posAbs, dSunM + 1.0), far);
            visFront *= canSun;
            visExit *= canSun;
            g_sunVisibility = visFront;
        }
    }""")
R_("""    if (dbg > 0.5) {
        if (dbg < 1.5) return vec3(dSun);""", """    if (litResearch2.z > 0.5) {
        if (litResearch2.z < 1.5) return vec3(textureLod(canopyMap, canopyUV(posAbs.xz), 0.0).r, 0.0, 0.0);
        if (litResearch2.z < 2.5) return vec3(canSun);
        return vec3(1.0 - canSky);
    }
    if (dbg > 0.5) {
        if (dbg < 1.5) return vec3(dSun);""")
# non-foliage path
R_("""    float sunVisibility = shadowMapVisibility(nCast, nlCast, terrain ? 0.0 : 1.0) *
                          terrainSunVisibility(posAbs, skyVisibility) * cloudSunVisibility(posAbs);
    g_sunVisibility = sunVisibility;""",
   """    float sunVisibility = shadowMapVisibility(nCast, nlCast, terrain ? 0.0 : 1.0) *
                          terrainSunVisibility(posAbs, skyVisibility) * cloudSunVisibility(posAbs);
    float canSun = 1.0;
    float canSky = 0.0;
    if (litResearch2.x > 0.0 || litResearch2.y > 0.0) {
        canSky = canopySkyOcclusion(posAbs, foliage);
        skyVisibility *= 1.0 - litResearch2.x * canSky;
        float far = canopyFarWeight() * litResearch2.y;
        if (far > 0.0 && sunOn > 0.0) {
            canSun = mix(1.0, canopySunVisibility(posAbs, 1.0), far);
            sunVisibility *= canSun;
        }
    }
    g_sunVisibility = sunVisibility;""")
R_("""    if (litFlags.z > 0.5) {
        // Debug views: 1 = sun visibility, 2 = terrain sky visibility, 3 = normal.""",
   """    if (litResearch2.z > 0.5) {
        if (litResearch2.z < 1.5) return vec3(textureLod(canopyMap, canopyUV(posAbs.xz), 0.0).r, 0.0, 0.0);
        if (litResearch2.z < 2.5) return vec3(canSun);
        return vec3(1.0 - canSky);
    }
    if (litFlags.z > 0.5) {
        // Debug views: 1 = sun visibility, 2 = terrain sky visibility, 3 = normal.""")
P.write_bytes(s.encode("utf-8"))
print("ov7 written", P.stat().st_size)
