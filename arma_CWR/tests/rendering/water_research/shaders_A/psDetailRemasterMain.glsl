
#define VARIANT 1

// ============================================================================
// Research prototype (override piece psDetailRemasterMain.glsl): wave-normal
// water for the Remastered lighting. The sea is drawn through PSDetail with the
// SpecularTexture flag (Landscape WaterFlags), so tex0 is the animated sea
// colour texture (one repeat per 50 m) and tex1 is data/specular_dx.paa (alpha
// = ripple pattern, 32 repeats per 50 m). Everything that is not sea is the
// stock detail shader.
//   VARIANT 1 (A): the stock remastered water shading (shadeRemastered +
//                  glossy) evaluated with a wave normal taken from the sea
//                  texture's luminance.
//   VARIANT 2 (B): A's normal; sky reflection = visible sky (fog colour with
//                  its sun glow) by Schlick Fresnel of the wave normal, capped,
//                  over the lit texture; sun = broad + tight normalised
//                  Blinn-Phong on the wave normal.
//   VARIANT 3 (C): B plus a swell layer (the same texture at about 1/4
//                  frequency, rotated) and a ripple layer (specular_dx), faded
//                  with distance; the tight lobe widens where the normals are
//                  filtered (Toksvig-style).
// ============================================================================
#ifndef VARIANT
#define VARIANT 2
#endif
const float WAVE_K      = 0.6;   // metres of height per unit of texture luminance
const float SWELL_K     = 0.0;  // same, swell layer (VARIANT 3)
const float RIPPLE_K    = 0.0; // same, ripple layer (VARIANT 3)
const float REFL_MAX    = 0.3; // weight of the flat-water Fresnel reflection
const float REFL_VAR    = 1.0; // weight of the wave-driven Fresnel variation
const float BROAD_POW   = 32.0;
const float BROAD_W     = 2.5;
const float TIGHT_POW   = 1500.0;
const float TIGHT_W     = 1.0;
const float SHEEN_W     = 0.35;  // scale of the material's broad lobe (stock = 1)
const float FADE_START  = 400.0; // m, VARIANT 3 normal-strength fade
const float FADE_END    = 3000.0;
const float FADE_FLOOR  = 0.35;
const float ANISO_FLOOR = 0.3; // wave strength kept where the footprint exceeds 16:1

bool isSea() { return vLitMode > 0.5 && litMatSpecular.w > 0.0 && dot(litMatSpecular.rgb, kLuma) > 0.3; }

// World gradients of the two UV coordinates on the (horizontal) sea plane,
// solved per pixel from screen derivatives: gU = (du/dX, du/dZ).
void uvGradients(vec2 uv, out vec2 gU, out vec2 gV) {
    vec2 dPx = dFdx(vWorldRel.xz), dPy = dFdy(vWorldRel.xz);
    vec2 dUx = dFdx(uv), dUy = dFdy(uv);
    float det = dPx.x * dPy.y - dPx.y * dPy.x;
    float id = abs(det) > 1e-12 ? 1.0 / det : 0.0;
    gU = vec2(dUx.x * dPy.y - dUy.x * dPx.y, dUy.x * dPx.x - dUx.x * dPy.x) * id;
    gV = vec2(dUx.y * dPy.y - dUy.y * dPx.y, dUy.y * dPx.x - dUx.y * dPy.x) * id;
}

// Finite-difference step in UV matched to what the (anisotropic) filter keeps.
float uvStep(vec2 uv, float texel) {
    float a = length(dFdx(uv)), b = length(dFdy(uv));
    float major = max(a, b), minor = min(a, b);
    return max(texel, max(minor, major / 16.0));
}

float lumAt(sampler2D s, vec2 uv) { return dot(texture(s, uv).rgb, kLuma); }

// World-space height gradient (m/m per unit height) of a luminance layer.
vec2 layerGrad(sampler2D s, vec2 uv, float h0, vec2 gU, vec2 gV, float e, bool alpha) {
    float hu = alpha ? texture(s, uv + vec2(e, 0.0)).a : lumAt(s, uv + vec2(e, 0.0));
    float hv = alpha ? texture(s, uv + vec2(0.0, e)).a : lumAt(s, uv + vec2(0.0, e));
    return ((hu - h0) * gU + (hv - h0) * gV) / e;
}

// Lit water body under a flat normal: shadeRemastered without glossy() and
// without the foliage branch. Linear radiance after exposure.
vec3 waterBody(vec3 albedo, out float ambientShare, out float sunVis) {
    vec3 n = vNormalW * inversesqrt(max(dot(vNormalW, vNormalW), 1e-12));
    vec3 posAbs = vWorldRel + litCamPos.xyz;
    vec3 l = litSunDir.xyz;
    float sunOn = litSunDir.w;
    float nl = max(dot(n, l), 0.0);
    float skyVisibility;
    sunVis = shadowMapVisibility(n, nl, 0.0) * terrainSunVisibility(posAbs, skyVisibility) * cloudSunVisibility(posAbs);
    g_sunVisibility = sunVis;
    vec3 kd = litMatDiffuse.rgb;
    vec3 ka = litMatAmbient.rgb;
    vec3 direct = litSun.rgb * (kd * nl + litMatForced.rgb) * (sunVis * sunOn);
    vec3 ambient = ka * skyIrradiance(n) * (skyVisibility * sunOn);
    vec3 moon = kd * litMoon.rgb * (max(dot(n, litMoonDir.xyz), 0.0) * sunOn);
    vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.5)));
    vec3 emissive = toLinear(min(litMatEmissive.rgb, vec3(1.5)));
    vec3 a = toLinear(albedo);
    float exposure = litZenith.w;
    vec3 rad = a * (direct + ambient + moon + local + emissive) * exposure;
    ambientShare = clamp(dot(a * ambient * exposure, kLuma) / max(dot(rad, kLuma), 1e-5), 0.0, 1.0);
    return rad;
}

void main() {
    vec4 t0 = texture(tex0, vUV0);
    vec4 t1 = texture(tex1, vUV1);
    vec4 r0;
    float ambientShare = 0.0;
    vec3 albedo = t0.rgb * constColor.rgb * (t1.a * 2.0);
    // Derivatives first, in uniform control flow.
    vec2 gU, gV;
    uvGradients(vUV0, gU, gV);
    float e0 = uvStep(vUV0, 1.0 / 512.0);
    float e1 = uvStep(vUV1, 1.0 / 256.0);
    float foot = length(fwidth(vWorldRel.xz));
    float fa = length(dFdx(vUV0)), fb = length(dFdy(vUV0));
    float anisoRatio = min(fa, fb) / max(max(fa, fb), 1e-12);
    if (isSea() && litFlags.z < 0.5) {
        // ---- wave normal
        float dist = length(vWorldRel);
        vec2 grad = WAVE_K * layerGrad(tex0, vUV0, dot(t0.rgb, kLuma), gU, gV, e0, false);
#if VARIANT == 3
        // swell: the same texture at about 1/4 frequency, rotated 37 degrees
        const mat2 R = mat2(0.7986, 0.6018, -0.6018, 0.7986) * 0.27;
        vec2 uvS = R * vUV0 + vec2(0.31, 0.57);
        vec2 gUs = R[0][0] * gU + R[1][0] * gV;
        vec2 gVs = R[0][1] * gU + R[1][1] * gV;
        float eS = max(1.0 / 512.0, e0 * 0.27);
        grad += SWELL_K * layerGrad(tex0, uvS, lumAt(tex0, uvS), gUs, gVs, eS, false);
        // ripples: specular_dx alpha at the detail tiling (vUV1 = 32 x vUV0)
        grad += RIPPLE_K * layerGrad(tex1, vUV1, t1.a, gU * 32.0, gV * 32.0, e1, true);
        grad *= mix(FADE_FLOOR, 1.0, 1.0 - smoothstep(FADE_START, FADE_END, dist));
#endif
#if VARIANT >= 2
        // Beyond the 16:1 the anisotropic filter covers, the texture is
        // under-filtered along the view direction and its gradient aliases into
        // pixel-high stripes at grazing range: scale the waves down there.
        grad *= clamp(16.0 * anisoRatio, ANISO_FLOOR, 1.0);
#endif
        vec3 n = normalize(vec3(-grad.x, 1.0, -grad.y));
        vec3 v = -vWorldRel * inversesqrt(max(dot(vWorldRel, vWorldRel), 1e-12));
        vec3 l = litSunDir.xyz;
        float sunOn = litSunDir.w;
        float exposure = litZenith.w;
        float sunVis;
        vec3 body = waterBody(albedo, ambientShare, sunVis);
        vec3 radiance;
#if VARIANT == 1
        // stock glossy() with the wave normal
        float power = litMatSpecular.w;
        float nl = dot(n, l);
        vec3 h = normalize(l + v);
        float nh = max(dot(n, h), 0.0);
        float vh = max(dot(v, h), 0.0);
        float p = max(power, 1.0);
        float norm = (p + 8.0) / 28.0;
        float fr = 1.0 + 2.0 * pow(clamp(1.0 - vh, 0.0, 1.0), 5.0);
        vec3 sunSpec = litMatSpecular.rgb * litSun.rgb * (pow(nh, p) * norm * fr * smoothstep(0.0, 0.15, nl) * sunVis);
        vec3 r = reflect(-v, n);
        float fv = pow(clamp(1.0 - dot(n, v), 0.0, 1.0), 5.0);
        float gloss = clamp(dot(litMatSpecular.rgb, kLuma), 0.0, 1.0);
        vec3 skySpec = skyIrradiance(r) * (gloss * (0.04 + 0.96 * fv) * 0.6);
        radiance = body + (sunSpec + skySpec) * litHorizon.w * sunOn * exposure;
#else
        // sky reflection: the visible sky in the reflected direction
        float nv = max(dot(n, v), 0.0);
        float F = 0.02 + 0.98 * pow(1.0 - nv, 5.0);
        vec3 r = reflect(-v, n);
        r.y = max(r.y, 0.01);
        vec3 sky = toLinear(fogColorToward(r)) * mix(1.0, 0.82, smoothstep(0.15, 0.9, r.y));
        // Mean reflection stays low (no pale sky wash at grazing range, where
        // the filtered normals are flat anyway); the part of the Fresnel term
        // that comes from the waves goes in at full weight, so a facet tilted
        // away from the viewer shows sky and one tilted toward shows water.
        float Ff = 0.02 + 0.98 * pow(1.0 - clamp(v.y, 0.0, 1.0), 5.0);
        float w = clamp(REFL_MAX * Ff + REFL_VAR * (F - Ff), 0.0, 0.95);
        // sun: broad + tight normalised Blinn-Phong on the wave normal
        vec3 h = normalize(l + v);
        float nh = max(dot(n, h), 0.0);
        float vh = max(dot(v, h), 0.0);
        float Fh = 0.02 + 0.98 * pow(1.0 - vh, 5.0);
        float pT = TIGHT_POW;
        // sheen: the material's own broad lobe (the stock glossy() sun term),
        // now on the wave normal, so the classic-style sheen toward the sun
        // stays but carries the waves
        float pM = max(litMatSpecular.w, 1.0);
        float sheen = SHEEN_W * pow(nh, pM) * (pM + 8.0) / 28.0 * (1.0 + 2.0 * pow(1.0 - vh, 5.0)) *
                      smoothstep(0.0, 0.15, dot(n, l));
#if VARIANT == 3
        // filtered normals lose slope variance: widen the tight lobe with the
        // footprint so distant sparkle becomes sheen instead of aliasing
        pT = TIGHT_POW / (1.0 + TIGHT_POW * 0.004 * foot);
#endif
        const float INV8PI = 0.0397887;
        float lobes = BROAD_W * (BROAD_POW + 8.0) * INV8PI * pow(nh, BROAD_POW) +
                      TIGHT_W * (pT + 8.0) * INV8PI * pow(nh, pT);
        vec3 sun = litSun.rgb * (Fh * lobes * smoothstep(0.0, 0.04, l.y) + litMatSpecular.rgb * sheen) * (sunVis * sunOn) * litHorizon.w;
        radiance = body * (1.0 - w) + sky * w + sun * exposure;
#endif
        vec3 display = toDisplay(tonemapNeutral(radiance));
        float grey = dot(display, kLuma);
        r0.rgb = max(mix(vec3(grey), display, litFlags.w), vec3(0.0));
        r0.a = vColor.a * t0.a * constColor.a;
    } else if (vLitMode > 0.5) {
        r0.rgb = shadeRemastered(albedo, true, ambientShare);
        r0.a = vColor.a * t0.a * constColor.a;
    } else {
        r0 = vColor * t0;
        r0 *= constColor;
        r0.rgb *= t1.a * 2.0;
        r0 += vSpecColor;
    }
    finishFragment(r0, ambientShare, true);
}
