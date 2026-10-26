"""v9 = v8 + 'passed behind a near surface' miss kind (3) + vehicle fallback restricted to upward rays."""
from pathlib import Path
src = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\ssr\glsl_v8\psPostSsr.glsl").read_text()
dst = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\ssr\glsl_v9\psPostSsr.glsl")

def rep(s, old, new, count=1):
    assert s.count(old) == count, (old, s.count(old))
    return s.replace(old, new)

s = src
s = rep(s, "    float startT = 2.0 / max(max(abs(dS.x), abs(dS.y)), 1e-3); // skip the first two pixels\n",
        "    float startT = 2.0 / max(max(abs(dS.x), abs(dS.y)), 1e-3); // skip the first two pixels\n"
        "    bool behindNear = false; // v9: went behind a surface within 3 m of the start\n")
s = rep(s, "        if (cand && sz < 1e8 && dot(w2v[1], viewPos(uv, sz)) + camWorld.y < ignoreY) cand = false;\n",
        "        if (cand && sz < 1e8 && dot(w2v[1], viewPos(uv, sz)) + camWorld.y < ignoreY) cand = false;\n"
        "        if (!cand && sz < 1e8 && min(zPrev, rz) > sz + thick) {\n"
        "            float tr = abs(r.z) > 1e-3 ? abs((rz - p0.z) / r.z) : len * t;\n"
        "            if (tr < 3.0) behindNear = true;\n"
        "        }\n")
s = rep(s, "    if (sceneZ(exitUV) > 1e8) { kind = 2; travel = 1e9; }\n    return exitUV;\n",
        "    // v9: a ray that slipped behind a surface right at its start (under a\n"
        "    // vehicle, behind a kerb) saw something the frame does not hold: dark.\n"
        "    if (behindNear) { kind = 3; return exitUV; }\n"
        "    if (sceneZ(exitUV) > 1e8) { kind = 2; travel = 1e9; }\n    return exitUV;\n")
# water: kind 3 -> dark underside
s = rep(s, "        vec3 sky = skyEstimate(rP, clamp(log2(max(sigma / pixAngle, 1.0)) + 2.0, 0.0, maxLod));\n        R = mix(sky, R, conf);\n",
        "        vec3 sky = skyEstimate(rP, clamp(log2(max(sigma / pixAngle, 1.0)) + 2.0, 0.0, maxLod));\n"
        "        if (kind == 3 && !okP) { sky = lin * 0.5; }\n"
        "        R = mix(sky, R, conf);\n")
# non-water: guard kind 3 in the hit branch, then fallbacks
s = rep(s, "        if (kind != 0) {\n            vec2 e = min(huv - worldRect.xy, worldRect.xy + worldRect.zw - huv) / worldRect.zw;",
        "        if (kind == 1 || kind == 2) {\n            vec2 e = min(huv - worldRect.xy, worldRect.xy + worldRect.zw - huv) / worldRect.zw;")
s = rep(s, "        vec3 sky = skyEstimate(rW, clamp(log2(max(cone / pixAngle, 1.0)) + 1.0, 0.0, maxLod));\n        R = mix(sky, R, conf);\n",
        "        vec3 sky = skyEstimate(rW, clamp(log2(max(cone / pixAngle, 1.0)) + 1.0, 0.0, maxLod));\n"
        "        if (kind == 3) sky = lin * 0.4;\n"
        "        float fallbackW = 1.0;\n"
        "        // v9: a paint reflection that leaves the frame sideways or back toward\n"
        "        // the camera sees ground the frame does not hold: no reflection, only\n"
        "        // upward rays fall back to the sky.\n"
        "        if (vehicle || glossy) fallbackW = smoothstep(0.1, 0.4, rW.y);\n"
        "        R = mix(sky, R, conf);\n")
s = rep(s, "            float w = F * weight * strength;\n            lin = mix(lin, R, w);\n",
        "            float w = F * weight * strength * mix(fallbackW, 1.0, conf);\n            lin = mix(lin, R, w);\n")
# dry paint weaker
s = rep(s, "            weight = ssrB.x * mix(0.55, 0.9, wet * up);\n", "            weight = ssrB.x * mix(0.4, 0.9, wet * up);\n")
dst.parent.mkdir(exist_ok=True)
dst.write_text(s)
print("wrote", dst, len(s))

# (then: water hit branch condition changed by hand to 'kind == 1 || kind == 2 || okP')
