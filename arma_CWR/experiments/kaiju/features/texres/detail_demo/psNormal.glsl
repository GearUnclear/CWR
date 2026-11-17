#version 330 core
// DEMO ONLY: a stand-in for a tiling detail layer, for the texture-resolution study.
// This is the engine's own psNormal source (PoseidonGL33/EngineGL33_Shaders.cpp, s_psNormalGLSL)
// with one addition, marked DETAIL DEMO below. The game loads it in place of the built-in
// source when the environment variable CWR_SHADER_OVERRIDE_DIR names this folder
// (EngineGL33_State.cpp:523-524, EngineGL33_Shaders.cpp:716-735). It changes EVERY draw that
// uses psNormal (the tank, the houses, the soldier too), so it is a picture of what a detail
// layer would add, not a way to ship one. The real thing is the engine's existing psDetail
// path (base * detail.a * 2 at 32 times the base UV), which no model can reach today.
layout(std140) uniform PSConstants {
    vec4 fogColor;    // c0
    vec4 alphaRef;    // c1: {ref, enabled, alphaToCoverage, flatDebug}
    vec4 shadowCtl;   // c2: {enable, bias, darkness, texelSize}
    vec4 constColor; // c3: per-object IsColored tint (white = no-op)
    vec4 _pad4;
    vec4 _pad5;
    vec4 _pad6;
    vec4 rgbEyeCoef;  // c7
    mat4 cascadeVP[4]; // c8-c23: per-cascade light view-projection
    vec4 cascadeSplits;// c24: per-tier select distance (omni: radius; frustum: far eye-depth)
    vec4 cascadeCtl;   // c25: {count, fadeRange, biasBase, omniCount}
    vec4 camFwd;       // c26: camera forward (eye-depth = dot(vWorldRel, camFwd))
};

uniform sampler2D tex0;

in vec4 vColor;
in vec4 vSpecColor;
in vec2 vUV0;
in vec2 vUV1;
in float vFogTC;

uniform sampler2DArray shadowMap; // unit 2: cascade depth-map array (unused unless shadowCtl.x>0.5)
in vec3 vWorldRel;

out vec4 fragColor;

// ---- DETAIL DEMO: a procedural grain at 32 times the base UV, like the engine's TGDetail ----
vec2 ddHash2(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}
float ddCells(vec2 p) {           // distance to the nearest cell border, 0 on a crack
    vec2 ip = floor(p), fp = fract(p);
    float d1 = 8.0, d2 = 8.0;
    for (int j = -1; j <= 1; ++j)
        for (int i = -1; i <= 1; ++i) {
            vec2 g = vec2(float(i), float(j));
            float d = length(g + ddHash2(ip + g) - fp);
            if (d < d1) { d2 = d1; d1 = d; } else if (d < d2) { d2 = d; }
        }
    return d2 - d1;
}
float ddNoise(vec2 p) {
    vec2 ip = floor(p), fp = fract(p);
    fp = fp * fp * (3.0 - 2.0 * fp);
    float a = ddHash2(ip).x, b = ddHash2(ip + vec2(1.0, 0.0)).x;
    float c = ddHash2(ip + vec2(0.0, 1.0)).x, d = ddHash2(ip + vec2(1.0, 1.0)).x;
    return mix(mix(a, b, fp.x), mix(c, d, fp.x), fp.y);
}
float ddDetail(vec2 uv) {         // returns a multiplier around 1.0
    vec2 p = uv * 32.0 * 48.0;    // 48 cells per detail tile
    float px = 1.0 / max(1e-6, max(fwidth(p.x), fwidth(p.y)));   // screen pixels per cell
    float fade = clamp((px - 2.5) / 5.0, 0.0, 1.0);              // gone before it can shimmer
    if (fade <= 0.0) return 1.0;
    float crack = smoothstep(0.0, 0.10, ddCells(p));
    float grain = ddNoise(p * 3.7);
    float d = (0.62 + 0.38 * crack) * (0.86 + 0.28 * grain);
    return mix(1.0, d * 1.22, fade);
}
// ---- end DETAIL DEMO ----

void main() {
    // No gl_FragDepth: opaque draws use DepthMode::Normal (stencil
    // ALWAYS+REPLACE 0).  Even if early-Z fires the stencil write
    // before discard, REPLACE 0 to a stencil already 0 (cleared at
    // frame start, never written non-zero outside the shadow
    // accumulator pass) is idempotent.  Removing the late-test forcing
    // lets early-Z and hierarchical-Z work: meaningful perf win on
    // opaque-heavy scenes.
    vec4 r0 = vColor * texture(tex0, vUV0);
    r0 *= constColor; // per-object IsColored tint (opacity + fade); white = no-op
    if (dot(vWorldRel, vWorldRel) > 1e-6) r0.rgb *= ddDetail(vUV0);   // DETAIL DEMO (3D draws only)
    r0.rgb += vSpecColor.rgb;

    if (shadowCtl.x > 0.5) {
        // Tiered shadow maps: the first omniCount tiers are camera-centred spheres
        // (selected by 3D distance, so a caster in ANY direction around the player -
        // including behind the camera: casts into view); the rest are frustum
        // slices reaching the far view distance (selected by eye-depth). Pick the
        // tightest matching tier, then advance to the first tier whose projection is
        // in bounds (coverage fallthrough, so a too-tight near tier never drops the
        // shadow). 3x3-PCF the layer, cross-fade to the next tier over a band, fade
        // at the far edge, and dim by the fog factor so distant shadows aren't harsh.
        // cascadeCtl = {count, fadeRange, biasBase, omniCount}; cascadeSplits =
        // per-tier select distance (omni: sphere radius; frustum: far eye-depth).
        int nC = int(cascadeCtl.x);
        int omniN = int(cascadeCtl.w);
        float eyeDepth = dot(vWorldRel, camFwd.xyz);
        float dist3D = length(vWorldRel);
        int ci = nC;
        for (int i = 0; i < 4; ++i) {
            if (i >= nC) break;
            float metric = (i < omniN) ? dist3D : eyeDepth;
            if (metric <= cascadeSplits[i]) { ci = i; break; }
        }
        if (ci < nC) {
            float ts = shadowCtl.w;
            float prevEdge = (ci > 0) ? cascadeSplits[ci - 1] : 0.0;
            float ciMetric = (ci < omniN) ? dist3D : eyeDepth;
            float band = (cascadeSplits[ci] - prevEdge) * 0.15;
            float bw = (ci + 1 < nC) ? clamp((ciMetric - (cascadeSplits[ci] - band)) / max(band, 0.001), 0.0, 1.0) : 0.0;
            float litSum = 0.0;
            float wSum = 0.0;
            for (int p = 0; p < 4; ++p) {
                int c = ci + p;
                if (c >= nC) break;
                // p0 = primary, p1 = blend partner; while nothing has covered yet a
                // later p force-samples the next looser tier (coverage fallthrough).
                float w = (p == 0) ? (1.0 - bw) : ((wSum <= 0.0) ? 1.0 : ((p == 1) ? bw : 0.0));
                if (w <= 0.0) continue;
                vec4 cp = cascadeVP[c] * vec4(vWorldRel, 1.0);
                vec3 sc = cp.xyz / cp.w;
                vec2 suv = sc.xy * 0.5 + 0.5;
                if (suv.x > 0.0 && suv.x < 1.0 && suv.y > 0.0 && suv.y < 1.0 && sc.z > 0.0 && sc.z < 1.0) {
                    float bias = cascadeCtl.z * float(c + 1) * float(c + 1);
                    float lit = 0.0;
                    for (int dy = -1; dy <= 1; ++dy)
                        for (int dx = -1; dx <= 1; ++dx)
                            lit += (sc.z - bias > texture(shadowMap, vec3(suv + vec2(float(dx), float(dy)) * ts, float(c))).r) ? 0.0 : 1.0;
                    litSum += w * (lit / 9.0);
                    wSum += w;
                }
            }
            if (wSum > 0.0) {
                float lit = litSum / wSum;
                float lastSplit = cascadeSplits[nC - 1];
                float fade = clamp((lastSplit - eyeDepth) / max(cascadeCtl.y, 0.001), 0.0, 1.0);
                float strength = (1.0 - lit) * fade * clamp(vFogTC, 0.0, 1.0); // dimmer in fog / far
                r0.rgb *= mix(1.0, shadowCtl.z, strength);
            }
        }
    }

    if (alphaRef.z > 0.5) {
        // Alpha-to-coverage: sharpen alpha around the cutout threshold so the
        // MSAA resolve grades sub-pixel cutout features (fence wire, foliage)
        // instead of the hard test keeping or killing the whole pixel.
        float cov = clamp((r0.a - alphaRef.x) / max(fwidth(r0.a), 1e-4) + 0.5, 0.0, 1.0);
        if (cov <= 0.0) discard;
        r0.a = cov;
    } else if (r0.a - alphaRef.x * alphaRef.y < 0.0) discard;

    float luminance= clamp(dot(r0.rgb, rgbEyeCoef.rgb), 0.0, 1.0);
    float nightBlend = clamp(luminance + rgbEyeCoef.a, 0.0, 1.0);
    r0.rgb = mix(vec3(luminance), r0.rgb, nightBlend);

    r0.rgb = mix(fogColor.rgb, r0.rgb, vFogTC);
    fragColor = alphaRef.w > 0.5 ? vec4(1.0, 0.0, 0.0, 1.0) : r0;
}
