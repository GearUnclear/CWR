"""Build the normals-track shader override from the engine's dumped GLSL.

Research only. Reads glsl_dump/ (python glsl_extract.py --source
EngineGL33_LightingShaders.cpp --dump glsl_dump) and writes an override dir
with psRemasterCommon.glsl, psNormalRemasterMain.glsl and
psDetailRemasterMain.glsl.

The bump variant is chosen at runtime through the saturation knob, so one game
session can shoot every variant at the same view and sim time:
    saturation = 100 * O + 10 * V + s   (s = the real saturation, default 1.12)
    O = 1  (C++ prototype) late-loaded shapes back on the CPU T&L path, i.e. the
           look the shipped Remastered build gives spawned vehicles
    V = 0  current Remastered (bit-identical maths, no bump)
    V = 1  screen-space derivative of the albedo luminance (dFdx/dFdy only)
    V = 2  texture-space taps: two scales, Retinex chroma gate, terrain detail map
    V = 3  V2 at twice the strength
    V = 4  V2 without the chroma gate
    V = 5  load-time normal map (unit 6, C++ prototype only), same frame as V2
    V = 6  (round 6) V9 strength, mean-preserving: linear bump in the sun term and
           micro shadows divided by their Smith (GGX G1) expectation
    V = 7  V5 plus micro self-shadowing (round 4: sun march over a height field:
           the companion's b on objects, the detail grain tex1.a on terrain)
    V = 8  V7 plus cavity occlusion of the sky light (horizon-based, same field)
    V = 9  V8 with the bump and the height both doubled

Round 3 (2026-10-17 evening): the companion map holds the RAW signed slope
(RG16F); round 2 decoded it as unorm (*2-1), which would have tilted every
normal by -k. Fixed here; V5-V8 were never captured before round 3.

usage: python make_override.py <out_dir> [--kobj 3] [--kterr 2.5] [--kdet 1.5] [--kss 3]
"""
import argparse
import pathlib
import sys

HERE = pathlib.Path(__file__).resolve().parent
DUMP = HERE / "glsl_dump"

BUMP_LIB = r"""
// ---------------------------------------------------------------------------
// RESEARCH (normals track): bump from the existing albedo, no new art.
// Variant from the saturation knob: saturation = 100 * O + 10 * V + s, where
// O = 1 puts late-loaded shapes back on the CPU path (C++ prototype reads it).
// ---------------------------------------------------------------------------
float bumpKnob() { return litFlags.w > 99.5 ? litFlags.w - 100.0 : litFlags.w; }
int bumpVariant() { float w = bumpKnob(); return w > 9.5 ? int(floor(w / 10.0 + 1e-3)) : 0; }
float bumpSaturation() { return bumpKnob() - 10.0 * float(bumpVariant()); }

// Sun visibility from the micro self-shadow march (V7-V9), 1 = lit, and the
// sky-light share the height-field cavities leave (V8/V9), 1 = open.
float g_bumpSunVis = 1.0;
float g_bumpAO = 1.0;
// V6: linear (unnormalised) bump in the sun term, so a symmetric relief keeps
// the mean brightness, and micro shadows divided by their Smith expectation.
bool g_bumpLinear = false;

const float kBumpObj = @KOBJ@;   // slope per unit luminance step per texel, objects
const float kBumpTerr = @KTERR@; // terrain base texture
const float kBumpDet = @KDET@;   // terrain detail texture (tex1.a)
const float kBumpSS = @KSS@;     // screen-space variant
const float kBumpNmap = @KNMAP@; // load-time normal map (V5-V8), objects
const float kBumpTerrNmap = @KTNMAP@; // load-time normal map, terrain base texture
const float kHeightObj = @KHOBJ@; // texel widths per unit of the companion height (b), objects
const float kHeightDet = @KHDET@; // texel widths per unit of the detail grain (tex1.a - 0.5), terrain

// Surface gradient of the height, subtracted from the shading normal
// (Mikkelsen's surface-gradient framework). Zero = no bump.
vec3 g_bumpGrad = vec3(0.0);

#ifdef BUMP_NORMALMAP
uniform sampler2D tex0Normal; // unit 6: load-time normal map of tex0 (rg = slope per texel * 0.5 + 0.5)
#endif

bool bumpAllowed() {
    // Foliage keeps its crown normal. (Soldiers and many vehicle parts draw
    // with alpha-to-coverage under MSAA, so that flag cannot exclude cutouts.)
    return dot(litMatForced.rgb, kLuma) <= 1e-4;
}

// Unit surface directions of increasing u and v (texels), from screen
// derivatives of the position and the uv: no mesh tangents needed.
void bumpFrame(vec3 n, vec2 uv, vec2 texSize, out vec3 tu, out vec3 tv, out float texPerM) {
    vec3 dpdx = dFdx(vWorldRel);
    vec3 dpdy = dFdy(vWorldRel);
    vec2 dx = dFdx(uv) * texSize;
    vec2 dy = dFdy(uv) * texSize;
    vec3 r1 = cross(dpdy, n);
    vec3 r2 = cross(n, dpdx);
    float det = dot(dpdx, r1);
    float inv = abs(det) > 1e-20 ? 1.0 / det : 0.0;
    vec3 gu = (dx.x * r1 + dy.x * r2) * inv; // texels per metre, along the surface
    vec3 gv = (dx.y * r1 + dy.y * r2) * inv;
    float lu = dot(gu, gu), lv = dot(gv, gv);
    tu = lu > 1e-12 ? gu * inversesqrt(lu) : vec3(0.0);
    tv = lv > 1e-12 ? gv * inversesqrt(lv) : vec3(0.0);
    texPerM = sqrt(0.5 * (lu + lv));
}

// Texture LOD the taps use: between the minor and major axis of the pixel
// footprint (about 4x anisotropy), never below the base level.
float bumpLod(vec2 uv, vec2 texSize) {
    vec2 dx = dFdx(uv) * texSize;
    vec2 dy = dFdy(uv) * texSize;
    float a = dot(dx, dx), b = dot(dy, dy);
    float major = sqrt(max(a, b)), minor = sqrt(min(a, b));
    return max(log2(max(max(minor, major * 0.25), 1e-6)), 0.0);
}

vec3 bumpTap(sampler2D s, vec2 uv, float lod) {
    vec3 c = textureLod(s, uv, lod).rgb;
    float sum = c.r + c.g + c.b + 1e-3;
    return vec3(dot(c, kLuma), c.r / sum, c.g / sum);
}

// Luminance gradient per BASE texel at one scale. Retinex gate: a luminance
// step that comes with a chromaticity step is paint (reflectance), not relief,
// so it is dropped; a pure light/dark step (crevice, rivet, pebble) is kept.
vec2 bumpGradAt(sampler2D s, vec2 uv, vec2 texSize, float lod, bool gate) {
    vec2 o = exp2(lod) / texSize;
    vec3 xm = bumpTap(s, uv - vec2(o.x, 0.0), lod);
    vec3 xp = bumpTap(s, uv + vec2(o.x, 0.0), lod);
    vec3 ym = bumpTap(s, uv - vec2(0.0, o.y), lod);
    vec3 yp = bumpTap(s, uv + vec2(0.0, o.y), lod);
    vec2 gl = 0.5 * vec2(xp.x - xm.x, yp.x - ym.x);
    float w = 1.0;
    if (gate) {
        float lMean = 0.25 * (xm.x + xp.x + ym.x + yp.x) + 0.04;
        vec2 gcx = 0.5 * (xp.yz - xm.yz);
        vec2 gcy = 0.5 * (yp.yz - ym.yz);
        float chroma = 3.0 * sqrt(dot(gcx, gcx) + dot(gcy, gcy));
        float shade = length(gl) / lMean;
        w = shade * shade / (shade * shade + 4.0 * chroma * chroma + 1e-7);
    }
    return gl * w / exp2(lod);
}

// Same on one channel (the terrain detail texture keeps its grain in alpha).
vec2 bumpGradAlpha(sampler2D s, vec2 uv, vec2 texSize, float lod) {
    vec2 o = exp2(lod) / texSize;
    float xm = textureLod(s, uv - vec2(o.x, 0.0), lod).a;
    float xp = textureLod(s, uv + vec2(o.x, 0.0), lod).a;
    float ym = textureLod(s, uv - vec2(0.0, o.y), lod).a;
    float yp = textureLod(s, uv + vec2(0.0, o.y), lod).a;
    return 0.5 * vec2(xp - xm, yp - ym) / exp2(lod);
}

// Two scales: fine (the pixel's own LOD) and mid (two levels coarser, its
// per-texel slope doubled so wider features keep some relief).
vec2 bumpGradTex(sampler2D s, vec2 uv, vec2 texSize, bool gate) {
    float lod = bumpLod(uv, texSize);
    return bumpGradAt(s, uv, texSize, lod, gate) + 2.0 * bumpGradAt(s, uv, texSize, lod + 2.0, gate);
}

// Soft limit so a hard paint edge that slips through cannot fold the normal.
vec3 bumpLimit(vec3 g) {
    float m = length(g);
    return m > 1e-6 ? g * (0.9 / (m + 0.9)) : vec3(0.0);
}

// Screen-space derivative of the luminance (V1): the gradient of whatever the
// pixel shows, per texel of the finest texture.
vec3 bumpScreen(vec3 n, float h, float texPerM, float k) {
    vec3 dpdx = dFdx(vWorldRel);
    vec3 dpdy = dFdy(vWorldRel);
    vec3 r1 = cross(dpdy, n);
    vec3 r2 = cross(n, dpdx);
    float det = dot(dpdx, r1);
    float inv = abs(det) > 1e-20 ? 1.0 / det : 0.0;
    vec3 gradH = (dFdx(h) * r1 + dFdy(h) * r2) * inv; // luminance per metre
    return k * gradH / max(texPerM, 1e-3);
}

#ifdef BUMP_NORMALMAP
// Round 4 height field: the companion's b (gated high-pass luminance of the
// albedo, load time) on objects, the detail map's alpha grain on terrain.
// Heights are in texel widths after the k factor, the units the bump uses.
float bumpH(bool det, vec2 uv, float lod) {
    return det ? textureLod(tex1, uv, lod).a - 0.5 : textureLod(tex0Normal, uv, lod).b;
}

// Micro self-shadow (V7-V9): march 8 steps toward the sun at the pixel's mip
// and test the height field against the sun ray rising at the sun's elevation
// above the surface; the penumbra widens with distance (contact hardening).
float bumpHeightShadow(bool det, vec2 uv, vec2 texSize, vec3 tu, vec3 tv, vec3 nGeo, float kh) {
    vec3 l = litSunDir.xyz;
    float nl = dot(nGeo, l);
    if (nl <= 0.02) return 1.0;
    vec2 d = vec2(dot(l, tu), dot(l, tv));
    float dl = length(d);
    if (dl < 1e-3) return 1.0;
    d /= dl;
    float tanE = nl / dl;
    float lod = bumpLod(uv, texSize);
    float st = exp2(lod);
    vec2 duv = d * (st / texSize);
    float h0 = bumpH(det, uv, lod) * kh;
    float worst = 0.0;
    for (int i = 1; i <= 8; ++i) {
        float t = st * float(i);
        float above = bumpH(det, uv + duv * float(i), lod) * kh - h0 - tanE * t;
        worst = max(worst, above / (0.2 * t + 0.4 * st));
    }
    return 1.0 - smoothstep(0.0, 1.0, worst);
}

// Cavity (V8/V9): horizon-based sky occlusion over the height field, 6
// directions x 3 radii (1, 2, 4 texels at the pixel's mip), cosine weighted
// (mean of sin^2 of the horizon angle). Only concave spots lose sky light.
float bumpCavity(bool det, vec2 uv, vec2 texSize, float kh) {
    float lod = bumpLod(uv, texSize);
    float st = exp2(lod);
    float h0 = bumpH(det, uv, lod) * kh;
    float occ = 0.0;
    for (int a = 0; a < 6; ++a) {
        float ang = float(a) * 1.0471976 + 0.5;
        vec2 dir = vec2(cos(ang), sin(ang));
        float best = 0.0;
        float r = 1.0;
        for (int i = 0; i < 3; ++i) {
            float t = st * r;
            float dh = bumpH(det, uv + dir * (t / texSize), lod) * kh - h0;
            best = max(best, dh * inversesqrt(dh * dh + t * t));
            r *= 2.0;
        }
        occ += best * best;
    }
    return 1.0 - occ / 6.0;
}
// V6 (bold, mean-preserving): the micro shadow divided by its expected value
// over a rough surface, Smith's GGX masking G1 = 2 / (1 + sqrt(1 + a^2 / tan^2 E))
// with a^2 = 2 x the local directional slope variance (from the cavity taps at
// one texel). Lit tops brighten as much as the shadowed gaps darken on
// average, so the relief does not make the ground darker overall.
float bumpSmithNorm(bool det, vec2 uv, vec2 texSize, vec3 tu, vec3 tv, vec3 nGeo, float kh) {
    vec3 l = litSunDir.xyz;
    float nl = dot(nGeo, l);
    if (nl <= 0.02) return 1.0;
    float dl = length(vec2(dot(l, tu), dot(l, tv)));
    float tanE = nl / max(dl, 1e-3);
    float lod = bumpLod(uv, texSize);
    float st = exp2(lod);
    float h0 = bumpH(det, uv, lod) * kh;
    float s2 = 0.0;
    for (int a = 0; a < 6; ++a) {
        float ang = float(a) * 1.0471976 + 0.5;
        float dh = bumpH(det, uv + vec2(cos(ang), sin(ang)) * (st / texSize), lod) * kh - h0;
        s2 += (dh / st) * (dh / st);
    }
    float a2 = 2.0 * s2 / 6.0;
    float g1 = 2.0 / (1.0 + sqrt(1.0 + a2 / max(tanE * tanE, 1e-4)));
    return 1.0 / max(g1, 0.6);
}
#endif

void bumpObject(vec3 albedo) {
    int V = bumpVariant();
    if (V <= 0 || !bumpAllowed()) return;
    vec3 n = vNormalW * inversesqrt(max(dot(vNormalW, vNormalW), 1e-12));
    vec2 ts = vec2(textureSize(tex0, 0));
    vec3 tu, tv;
    float tpm;
    bumpFrame(n, vUV0, ts, tu, tv, tpm);
    vec3 g = vec3(0.0);
    if (V == 1) {
        g = bumpScreen(n, dot(albedo, kLuma), tpm, kBumpSS);
    } else if (V <= 4) {
        float k = kBumpObj * (V == 3 ? 2.0 : 1.0);
        vec2 gt = bumpGradTex(tex0, vUV0, ts, V != 4) * k;
        g = gt.x * tu + gt.y * tv;
    } else {
#ifdef BUMP_NORMALMAP
        // The companion holds the signed slope per base texel (rg) and the height (b).
        bool x2 = V == 6 || V == 9;
        float k = kBumpNmap * (x2 ? 2.0 : 1.0);
        vec2 gt = texture(tex0Normal, vUV0).rg * k;
        g = gt.x * tu + gt.y * tv;
        float kh = kHeightObj * (x2 ? 2.0 : 1.0);
        if (V >= 7 || V == 6) g_bumpSunVis = bumpHeightShadow(false, vUV0, ts, tu, tv, n, kh);
        if (V >= 8 || V == 6) g_bumpAO = bumpCavity(false, vUV0, ts, kh);
        if (V == 6) { g_bumpLinear = true; g_bumpSunVis = min(g_bumpSunVis * bumpSmithNorm(false, vUV0, ts, tu, tv, n, kh), 1.6); }
#endif
    }
    g_bumpGrad = bumpLimit(g);
}

void bumpTerrain(vec3 albedo) {
    int V = bumpVariant();
    if (V <= 0) return;
    vec3 n = vNormalW * inversesqrt(max(dot(vNormalW, vNormalW), 1e-12));
    vec2 ts0 = vec2(textureSize(tex0, 0));
    vec2 ts1 = vec2(textureSize(tex1, 0));
    vec3 tu0, tv0, tu1, tv1;
    float tpm0, tpm1;
    bumpFrame(n, vUV0, ts0, tu0, tv0, tpm0);
    bumpFrame(n, vUV1, ts1, tu1, tv1, tpm1);
    vec3 g = vec3(0.0);
    if (V == 1) {
        g = bumpScreen(n, dot(albedo, kLuma), max(tpm0, tpm1), kBumpSS);
    } else if (V <= 4) {
        float s = (V == 3 ? 2.0 : 1.0);
        vec2 g0 = bumpGradTex(tex0, vUV0, ts0, V != 4) * (kBumpTerr * s);
        float lod1 = bumpLod(vUV1, ts1);
        vec2 g1 = (bumpGradAlpha(tex1, vUV1, ts1, lod1) + 2.0 * bumpGradAlpha(tex1, vUV1, ts1, lod1 + 2.0)) * (kBumpDet * s);
        g = g0.x * tu0 + g0.y * tv0 + g1.x * tu1 + g1.y * tv1;
    } else {
#ifdef BUMP_NORMALMAP
        bool x2 = V == 6 || V == 9;
        float s = x2 ? 2.0 : 1.0;
        vec2 g0 = texture(tex0Normal, vUV0).rg * (kBumpTerrNmap * s);
        float lod1 = bumpLod(vUV1, ts1);
        vec2 g1 = (bumpGradAlpha(tex1, vUV1, ts1, lod1) + 2.0 * bumpGradAlpha(tex1, vUV1, ts1, lod1 + 2.0)) * (kBumpDet * s);
        g = g0.x * tu0 + g0.y * tv0 + g1.x * tu1 + g1.y * tv1;
        // Micro shadows and cavities ride on the fine detail grain (tex1.a).
        float kh = kHeightDet * (x2 ? 2.0 : 1.0);
        if (V >= 7 || V == 6) g_bumpSunVis = bumpHeightShadow(true, vUV1, ts1, tu1, tv1, n, kh);
        if (V >= 8 || V == 6) g_bumpAO = bumpCavity(true, vUV1, ts1, kh);
        if (V == 6) { g_bumpLinear = true; g_bumpSunVis = min(g_bumpSunVis * bumpSmithNorm(true, vUV1, ts1, tu1, tv1, n, kh), 1.6); }
#endif
    }
    g_bumpGrad = bumpLimit(g);
}
"""


def patch_common(src: str, consts: dict, normalmap: bool) -> str:
    lib = BUMP_LIB
    for k, v in consts.items():
        lib = lib.replace("@" + k + "@", repr(float(v)))
    if normalmap:
        lib = "#define BUMP_NORMALMAP 1\n" + lib
    anchor = "// Sun visibility of the last shaded fragment"
    assert src.count(anchor) == 1
    src = src.replace(anchor, lib + "\n" + anchor)

    old_n = "    vec3 n = vNormalW * inversesqrt(max(dot(vNormalW, vNormalW), 1e-12));\n    vec3 posAbs = vWorldRel + litCamPos.xyz;"
    assert src.count(old_n) == 1
    new_n = (
        "    vec3 nGeo = vNormalW * inversesqrt(max(dot(vNormalW, vNormalW), 1e-12));\n"
        "    // RESEARCH: the bumped normal shades; the geometric one keeps the shadow lookup.\n"
        "    vec3 n = dot(g_bumpGrad, g_bumpGrad) > 0.0 ? normalize(nGeo - g_bumpGrad) : nGeo;\n"
        "    vec3 posAbs = vWorldRel + litCamPos.xyz;"
    )
    src = src.replace(old_n, new_n)

    old_cast = "    vec3 nCast = (foliage && dot(n, l) < 0.0) ? -n : n;"
    assert src.count(old_cast) == 1
    src = src.replace(old_cast, "    vec3 nCast = (foliage && dot(nGeo, l) < 0.0) ? -nGeo : nGeo;")

    old_nl = "    float nl = max(dot(n, l), 0.0);"
    assert src.count(old_nl) == 1
    # A bump never lights a face that the geometry turns away from the sun.
    src = src.replace(old_nl, "    float nl = (g_bumpLinear ? max(dot(nGeo, l) - dot(g_bumpGrad, l), 0.0) : max(dot(n, l), 0.0)) * smoothstep(-0.05, 0.05, dot(nGeo, l));")

    old_vis = "                          terrainSunVisibility(posAbs, skyVisibility) * cloudSunVisibility(posAbs);"
    assert src.count(old_vis) == 1
    src = src.replace(old_vis, old_vis[:-1] + " * g_bumpSunVis;")

    old_amb = "    vec3 ambient = ka * skyIrradiance(nLight) * (skyVisibility * crownSky * sunOn);"
    assert src.count(old_amb) == 1
    src = src.replace(old_amb, old_amb[:-2] + " * g_bumpAO);")

    old_sat = "    return max(mix(vec3(grey), display, litFlags.w), vec3(0.0));"
    assert src.count(old_sat) == 1
    src = src.replace(old_sat, "    return max(mix(vec3(grey), display, bumpSaturation()), vec3(0.0));")
    return src


def patch_normal_main(src: str) -> str:
    old = "        r0.rgb = shadeRemastered(t0.rgb * constColor.rgb, false, ambientShare);"
    assert src.count(old) == 1
    return src.replace(
        old,
        "        bumpObject(t0.rgb * constColor.rgb);\n"
        "        r0.rgb = shadeRemastered(t0.rgb * constColor.rgb, false, ambientShare);",
    )


def patch_detail_main(src: str) -> str:
    old = "        r0.rgb = shadeRemastered(albedo, true, ambientShare);"
    assert src.count(old) == 1
    return src.replace(old, "        bumpTerrain(albedo);\n        r0.rgb = shadeRemastered(albedo, true, ambientShare);")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--kobj", type=float, default=3.0)
    ap.add_argument("--kterr", type=float, default=2.5)
    ap.add_argument("--kdet", type=float, default=1.5)
    ap.add_argument("--kss", type=float, default=3.0)
    ap.add_argument("--knmap", type=float, default=2.0)
    ap.add_argument("--ktnmap", type=float, default=1.7)
    ap.add_argument("--khobj", type=float, default=8.0)
    ap.add_argument("--khdet", type=float, default=6.0)
    ap.add_argument("--normalmap", action="store_true", help="declare tex0Normal (unit 6), needs the C++ prototype")
    a = ap.parse_args()
    out = pathlib.Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    consts = {"KOBJ": a.kobj, "KTERR": a.kterr, "KDET": a.kdet, "KSS": a.kss, "KNMAP": a.knmap, "KTNMAP": a.ktnmap, "KHOBJ": a.khobj, "KHDET": a.khdet}
    common = patch_common((DUMP / "psRemasterCommon.glsl").read_text(), consts, a.normalmap)
    (out / "psRemasterCommon.glsl").write_text(common, newline="\n")
    (out / "psNormalRemasterMain.glsl").write_text(patch_normal_main((DUMP / "psNormalRemasterMain.glsl").read_text()), newline="\n")
    (out / "psDetailRemasterMain.glsl").write_text(patch_detail_main((DUMP / "psDetailRemasterMain.glsl").read_text()), newline="\n")
    print("wrote", out, consts, "normalmap" if a.normalmap else "")
    return 0


if __name__ == "__main__":
    sys.exit(main())
