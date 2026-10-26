"""Amplification variant of the perf-mask override (cost by slope, robust to GPU sharing).

On top of make_mask_override.py's mask (saturation knob), the localLights knob
carries an amplification count: triSetLighting "localLights=<100 + K>" makes
shadeRemastered evaluate its target K extra times with the position nudged by
a sub-millimetre amount (defeats common-subexpression elimination, keeps the
same texels) and add the results at 1e-6 weight (keeps them alive). The real
local-light scale is then 1. Target, from the mask bits:
    2048  the whole lit shading (shadow map, horizon, cloud, sky, glossy, ...)
    4096  shadowMapVisibility only
    8192  terrainSunVisibility only
    (none of them: the whole lit shading)
The frame-time slope against K is the cost of one evaluation in this very
scene (same overdraw, quad overshading and MSAA), as a share of the frame.
"""
import sys
from pathlib import Path

src = Path(sys.argv[1]).read_text()
out = Path(sys.argv[2])


def rep(old, new, count=1):
    global src
    n = src.count(old)
    if n != count:
        raise SystemExit(f"expected {count} match(es), found {n}: {old[:70]!r}")
    src = src.replace(old, new)


# A position global the amplified evaluations can nudge.
rep("float perfSaturation() {",
    "int perfAmpK() { return litGround.w > 99.5 ? int(litGround.w + 0.5) - 100 : 0; }\n"
    "float perfLocalScale() { return litGround.w > 99.5 ? 1.0 : litGround.w; }\n"
    "vec3 g_rel = vec3(0.0); // vWorldRel, nudged by the amplified evaluations\n"
    "float perfSaturation() {")
rep("    vec3 rel = vWorldRel + litShadowDelta.xyz;\n", "    vec3 rel = g_rel + litShadowDelta.xyz;\n")
rep("        vec3 toLight = litLightPos[i].xyz - vWorldRel;\n", "        vec3 toLight = litLightPos[i].xyz - g_rel;\n")
rep("    vec3 posAbs = vWorldRel + litCamPos.xyz;\n    vec3 v = -vWorldRel * inversesqrt(max(dot(vWorldRel, vWorldRel), 1e-12));\n",
    "    vec3 posAbs = g_rel + litCamPos.xyz;\n    vec3 v = -g_rel * inversesqrt(max(dot(g_rel, g_rel), 1e-12));\n")
rep("    vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.5)));\n",
    "    vec3 local = toLinear(min(localLights(n, kd, ka) * perfLocalScale(), vec3(1.5)));\n")

# Rename the shading entry point to a core and wrap it.
rep("vec3 shadeRemastered(vec3 albedo, bool terrain, out float ambientShare) {\n",
    "vec3 shadeRemasteredCore(vec3 albedo, bool terrain, out float ambientShare) {\n")
rep("// Classic night-eye desaturation, fog and alpha handling, shared tail.\n",
    """vec3 shadeRemastered(vec3 albedo, bool terrain, out float ambientShare) {
    g_rel = vWorldRel;
    vec3 c = shadeRemasteredCore(albedo, terrain, ambientShare);
    int k = perfAmpK();
    if (k > 0) {
        vec3 acc = vec3(0.0);
        bool shadowOnly = perfBit(4096);
        bool terrainOnly = perfBit(8192);
        vec3 nn = normalize(vNormalW);
        for (int i = 0; i < 64; ++i) {
            if (i >= k) break;
            g_rel = vWorldRel + vec3(float(i + 1) * 2e-4);
            if (shadowOnly) {
                acc.x += shadowMapVisibility(nn, max(dot(nn, litSunDir.xyz), 0.0), terrain ? 0.0 : 1.0);
            } else if (terrainOnly) {
                float sv;
                acc.y += terrainSunVisibility(g_rel + litCamPos.xyz, sv) + sv;
            } else {
                float s;
                acc += shadeRemasteredCore(albedo, terrain, s);
            }
        }
        g_rel = vWorldRel;
        c += acc * 1e-6;
    }
    return c;
}

// Classic night-eye desaturation, fog and alpha handling, shared tail.
""")
out.write_text(src)
print("wrote", out)
