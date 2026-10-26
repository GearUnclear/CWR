// Remastered lighting shaders: the sky piece of the fragment prelude
// (FragmentPiece::Sky, joined by RemasterFragmentCommon() in
// EngineGL33_LightingShaders.cpp): the sky light falling on a surface, the fog
// colour toward the sun, the sky table's lookups for the dome and the fog
// colour (aerial perspective) and the lit sky clouds. Sky lookups for
// reflections belong here too.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

const char kRemasterSkyGLSL[] = R"(
// Irradiance from the sky hemisphere (and the ground below it) onto a surface
// with normal n. The horizon band is brighter, and takes the sky's colour,
// toward the sun; opposite the sun it keeps the cool zenith colour, so a low
// sun gives warm light and cool shade.
vec3 skyIrradiance(vec3 n) {
    float h = length(n.xz);
    float sl = length(litSunDir.xz);
    float toward = (h > 1e-3 && sl > 1e-3) ? 0.5 + 0.5 * dot(n.xz / h, litSunDir.xz / sl) : 0.5;
    // The bounce takes the colour of the ground below: the rig's horizon holds
    // half the (neutral) ground bounce, so swap that half for the tinted one.
    vec3 ground = litGround.rgb * g_groundTint;
    vec3 horizon = mix(litHorizon.rgb, litHorizonSun.rgb, toward) + 0.5 * (ground - litGround.rgb);
    float t = clamp(n.y, -1.0, 1.0);
    return t >= 0.0 ? mix(horizon, litZenith.rgb, t) : mix(horizon, ground, -t);
}

// Fog toward the sun takes the sky dome's own sun glow (TLVertexTable::
// DoSkyLighting adds SunSkyColor * cos^6 to SkyColor, times the sky texture),
// so distant ground fades into the colour of the sky behind it instead of a
// flat band. Away from the sun this is exactly the classic fog colour.
vec3 fogColorToward(vec3 rel) {
    if (litFogSun.w < 0.5) return fogColor.rgb;
    float c = max(dot(rel * inversesqrt(max(dot(rel, rel), 1e-12)), litSunDir.xyz), 0.0);
    float c2 = c * c;
    return min(fogColor.rgb + litFogSun.rgb * (c2 * c2 * c2), max(litFogCap.rgb, fogColor.rgb));
}

// ---- The sky table (feature switch atmosphere) ------------------------------
// The sky-view table on unit 8 (Graphics/Remaster/Atmosphere.hpp; built on the
// GPU when the sun moves, EngineGL33_SkyTable.cpp), three regions side by side:
// Rayleigh plus multiple scattering with its phase, Mie single scattering
// without its phase (applied here per pixel, so the aureole around the sun
// stays sharp) and the azimuthal mean of each elevation row (rgb) with the
// row's mean of the original dome's sun-glow weight max(cos, 0)^6 (alpha).
// u = sqrt(azimuth from the sun / pi), v = 0.5 + 0.5 sign(l) sqrt(|l| / (pi / 2)).
// The radiance is per unit of solar irradiance: the dome and the fog scale it,
// row by row, to the original sky's brightness.
const float kSkyPi = 3.14159265;
// The cloud mask takes a sky texel brighter than its row's clearest texel as
// this white cloud over that clear sky.
const float kSkyCloudWhite = 0.92;

float skyMiePhase(float c) {
    float g = litSkyRight.w;
    float k = 3.0 / (8.0 * kSkyPi) * (1.0 - g * g) / (2.0 + g * g);
    return k * (1.0 + c * c) / pow(max(1.0 + g * g - 2.0 * g * c, 1e-6), 1.5);
}

// The table's v of an elevation (radians).
float skyRowV(float l) {
    return 0.5 + 0.5 * sign(l) * sqrt(abs(l) / (0.5 * kSkyPi));
}

// Unscaled sky radiance along the unit direction d, and its elevation row's
// mean: rgb radiance, a the original dome's sun-glow weight.
vec3 skyRadiance(vec3 d, out vec4 mean) {
    float l = asin(clamp(d.y, -1.0, 1.0));
    vec2 sh = litSunDir.xz;
    float sl = length(sh);
    float dl = length(d.xz);
    float cphi = (sl > 1e-4 && dl > 1e-4) ? dot(d.xz / dl, sh / sl) : 1.0;
    float phi = acos(clamp(cphi, -1.0, 1.0));
    const float n = 64.0; // texels per region (kSkyViewSize)
    float u = clamp(sqrt(phi / kSkyPi), 0.5 / n, 1.0 - 0.5 / n);
    float v = skyRowV(l);
    vec3 r = texture(skyLut, vec2(u / 3.0, v)).rgb;
    vec3 m = texture(skyLut, vec2((1.0 + u) / 3.0, v)).rgb;
    mean = texture(skyLut, vec2(2.5 / 3.0, v));
    return r + m * skyMiePhase(dot(d, litSunDir.xyz));
}

// The clear sky along d at the original sky's brightness. The original sky of
// that elevation is baseDisp plus its sun glow glowDisp times the glow weight,
// capped at capDisp (display, as the dome's vertex colour and the fog colour
// toward the sun add it), so on average over a row the result keeps the
// original row's brightness. The table supplies how a direction differs from
// the mean of its row (the glow and the aureole toward the sun, the darker side
// away from it). litAtmoDome.w picks the hue: 0 keeps the original hue, relit
// by that brightness ratio alone; 1 takes the table's own hue. Both have the
// same brightness, so the blend moves the hue only (a channel-by-channel relight
// turned the pink horizon opposite a setting sun olive).
vec3 skyClear(vec3 d, vec3 baseDisp, vec3 glowDisp, vec3 capDisp) {
    vec4 mean;
    vec3 a = skyRadiance(d, mean);
    vec3 classicLin = toLinear(min(baseDisp + glowDisp * mean.a, capDisp));
    float rowLum = max(dot(mean.rgb, kLuma), 1e-8);
    vec3 relit = classicLin * (dot(a, kLuma) / rowLum);
    vec3 phys = a * (dot(classicLin, kLuma) / rowLum);
    return mix(relit, phys, litAtmoDome.w);
}

// Linear sky radiance to the display, with the sky's own saturation
// (litAtmoGlow.w; the dome and the fog colour only).
vec3 skyDisplay(vec3 lin) {
    // Under the photoreal camera the sky is scene light too (the camera's
    // curve rolls off the glow around the sun).
    vec3 disp = litCamera.x > 0.5 ? toDisplay(max(lin, vec3(0.0))) : toDisplay(tonemapNeutral(lin));
    return max(mix(vec3(dot(disp, kLuma)), disp, litAtmoGlow.w), vec3(0.0));
}

// Aerial perspective: distant geometry fades into the clear sky just above the
// horizon in its own azimuth, at the brightness of the original fog colour with
// its glow toward the sun (fogColorToward's terms). Under an overcast deck
// (litAtmo.y) it keeps the classic fog colour.
vec3 atmoFogColor(vec3 rel, vec3 classicFog) {
    vec3 d = rel * inversesqrt(max(dot(rel, rel), 1e-12));
    vec3 h = vec3(d.x, clamp(d.y, 0.0, 0.25) * 0.5 + 0.015, d.z);
    h *= inversesqrt(max(dot(h, h), 1e-12));
    vec3 glow = litFogSun.w > 0.5 ? litFogSun.rgb : vec3(0.0);
    vec3 disp = skyDisplay(skyClear(h, fogColor.rgb, glow, max(litFogCap.rgb, fogColor.rgb)));
    disp = mix(disp, classicFog, litAtmo.y);
    return mix(classicFog, disp, litAtmo.z);
}

// ---- Lit sky clouds (feature switch litClouds; SkyDensity.hpp) --------------
// The sky's painted clouds read as a density field (the cloud-density map on
// unit 9, skyCloudDensity). A short march from a sky pixel toward the sun gives
// the cloud the sunlight crosses before it reaches the pixel; the difference to
// the pixel's own density is relief, mean-free over a field (the sky keeps its
// brightness): sun-facing edges lit and warm, the far side shaded. Thin rims
// facing a nearby sun get a forward-scattered silver lining, thick cloud in
// front of the sun goes dark, and under an overcast deck the dark lumps deepen
// into bellies. Relief and lining are on luminance, tinted only by the sun's
// colour. Every term is the low sun's: all of them fade out together between
// kLitCloudFadeStartDeg and kLitCloudFadeEndDeg, so from there up the terms are
// identity (SkyDensity.hpp LitCloudTerms). At midday the relief embossed the
// painted sky with colour fringes, and the darkening against the sun left on
// its own made the sky only darker.
const float kLitCloudRelief = 1.8;        // kLitCloudRelief
const float kLitCloudSilver = 1.0;        // kLitCloudSilver
const float kLitCloudStepDeg = 1.2;       // kLitCloudStepDeg
const float kLitCloudFadeStartDeg = 25.0; // kLitCloudFadeStartDeg
const float kLitCloudFadeEndDeg = 45.0;   // kLitCloudFadeEndDeg
const float kLitCloudSilverLight = 1.2;   // the lining's light, in the cloud's own luminance

// The sun's light on a sky cloud seen along the unit direction v: x the
// luminance gain, y how far its colour goes toward the sun's, z the silver
// lining (in the cloud's own luminance), w where it applies (the map has cloud
// there, times litSky.x). `d` is the density here, `tau` the mean density
// toward the sun, `feature`, `dark` and `deck` the pixel's broken cloud,
// darkness and deck weight.
vec4 litCloudTerms(vec3 v, float d, float tau, float feature, float dark, float deck) {
    vec3 l = litSunDir.xyz;
    float cosT = dot(v, l);
    // Overcast flattens the direct part: the light inside a deck is diffuse.
    float direct = smoothstep(-0.06, 0.04, l.y) * mix(0.45, 1.0, clamp(litSky.w, 0.0, 1.0));
    float relief = clamp((d - tau) * kLitCloudRelief, -0.35, 0.8);
    float gain = 1.0 + relief * direct;
    gain *= 1.0 + (0.55 - dark) * 0.7 * deck;
    gain *= 1.0 - 0.35 * pow(max(cosT, 0.0), 6.0) * smoothstep(0.35, 0.9, feature) * direct;
    float warm = clamp(relief, 0.0, 1.0) * direct;
    // The rim is where the density falls off toward the sun; Henyey-Greenstein
    // (g 0.8) relative to its peak toward the sun.
    float rim = clamp((d - tau) * 3.0, 0.0, 1.0) * (1.0 - 0.5 * d);
    float hg = 0.008 / pow(max(1.64 - 1.6 * cosT, 1e-4), 1.5); // 1 toward the sun
    float silver = hg * rim * kLitCloudSilver * direct;
    // All of it fades with the low sun, the darkening against the sun and the
    // bellies included: identity from kLitCloudFadeEndDeg up.
    float lowSun = 1.0 - smoothstep(kLitCloudFadeStartDeg, kLitCloudFadeEndDeg, litSun.w);
    return vec4(mix(1.0, gain, lowSun), warm * lowSun, silver * lowSun, litSky.x * smoothstep(0.02, 0.25, d));
}

// A cloud's light (linear) under those terms.
vec3 litCloudLight(vec3 lin, vec4 t) {
    if (t.w <= 0.0) return lin;
    float sl = dot(litSun.rgb, kLuma);
    vec3 sunC = sl > 1e-5 ? litSun.rgb / sl : vec3(1.0); // unit luminance
    vec3 lit = lin * (t.x * mix(vec3(1.0), sunC, 0.85 * t.y));
    lit += sunC * (max(dot(lin, kLuma), 0.02) * t.z * kLitCloudSilverLight);
    return mix(lin, lit, t.w);
}

// The same over a pre-lit colour (display), where the sky table does not shade
// it: a colour pushed past white goes toward white rather than clipping
// channel by channel (clipped channels made colour fringes).
vec3 litCloudsOverClassic(vec3 disp, vec4 t) {
    if (t.w <= 0.0) return disp;
    vec3 c = toDisplay(litCloudLight(toLinear(disp), t));
    float peak = max(c.r, max(c.g, c.b));
    if (peak > 1.0) c = mix(c / peak, vec3(1.0), clamp((peak - 1.0) * 0.5, 0.0, 1.0));
    return c;
}

// The terms for a dome pixel at its (turned) texture coordinate. The march
// steps the view ray toward the sun and maps each step through the dome's own
// mapping, relative to the pixel's mesh coordinate, so it is continuous across
// the dome's coarse triangles (screen-space derivatives jump at their edges).
vec4 litCloudTermsDome(vec2 uv) {
    // Off, or a sun so high the terms are identity: no march.
    if (litSky.x <= 0.0 || litSun.w >= kLitCloudFadeEndDeg) return vec4(1.0, 0.0, 0.0, 0.0);
    vec4 m = textureLod(skyDensity, uv, 0.0);
    float d = skyCloudDensity(m);
    if (d <= 0.02) return vec4(1.0, 0.0, 0.0, 0.0);
    vec3 v = vWorldRel * inversesqrt(max(dot(vWorldRel, vWorldRel), 1e-12));
    vec3 l = litSunDir.xyz;
    vec3 t = l - v * dot(v, l);
    float tl = length(t);
    t = tl > 1e-4 ? t / tl : vec3(0.0, 1.0, 0.0);
    vec2 uvA = skyDomeUv(v);
    float tau = 0.0;
    float wsum = 0.0;
    for (int k = 1; k <= 6; ++k) {
        float fk = float(k);
        float a = radians(kLitCloudStepDeg) * fk * (0.5 + 0.5 * fk);
        vec2 duv = skyDomeUv(v * cos(a) + t * sin(a)) - uvA;
        duv.x -= floor(duv.x + 0.5);
        float wk = 1.0 / fk;
        tau += wk * skyCloudDensity(textureLod(skyDensity, uv + duv, 0.0));
        wsum += wk;
    }
    float deck = m.g * (1.0 - smoothstep(0.35, 0.75, litSky.w));
    return litCloudTerms(v, d, tau / wsum, m.r * (0.75 + 0.25 * m.b), m.b, deck);
}

// The terms for a sky cloud sheet (DrawClouds' billboards), whose own texture
// alpha is its density: the march runs in its texture toward the sun, whose
// direction there comes from a small step of the view ray toward the sun on
// the screen through the texture's screen-space derivatives (the sheets are
// flat, so these are smooth).
vec4 litCloudTermsSheet(float alpha) {
    vec2 dUVdx = dFdx(vUV0);
    vec2 dUVdy = dFdy(vUV0);
    if (litSky.x <= 0.0 || litSun.w >= kLitCloudFadeEndDeg || alpha <= 0.02) return vec4(1.0, 0.0, 0.0, 0.0);
    vec3 v = vWorldRel * inversesqrt(max(dot(vWorldRel, vWorldRel), 1e-12));
    vec3 l = litSunDir.xyz;
    vec3 t = l - v * dot(v, l);
    float tl = length(t);
    vec2 dir = vec2(0.0);
    if (tl > 1e-4) {
        vec3 v2 = normalize(v + t * (0.01 / tl));
        vec3 R = litSkyRight.xyz;
        vec3 U = litSkyUp.xyz;
        vec3 F = litSkyFwd.xyz;
        vec2 n1 = vec2(dot(v, R) / max(dot(R, R), 1e-12), dot(v, U) / max(dot(U, U), 1e-12)) / max(dot(v, F), 1e-4);
        vec2 n2 = vec2(dot(v2, R) / max(dot(R, R), 1e-12), dot(v2, U) / max(dot(U, U), 1e-12)) / max(dot(v2, F), 1e-4);
        // Pixels per NDC unit go as tan(fov / 2) on each axis (square pixels).
        vec2 dPix = (n2 - n1) * vec2(length(R), length(U));
        vec2 duv = dUVdx * dPix.x + dUVdy * dPix.y;
        float len = length(duv);
        dir = len > 1e-9 ? duv / len : vec2(0.0);
    }
    float tau = 0.0;
    float wsum = 0.0;
    for (int k = 1; k <= 6; ++k) {
        float fk = float(k);
        float wk = 1.0 / fk;
        tau += wk * textureGrad(tex0, vUV0 + dir * (0.025 * fk * (0.5 + 0.5 * fk)), dUVdx, dUVdy).a;
        wsum += wk;
    }
    return litCloudTerms(v, alpha, tau / wsum, alpha, 0.0, 0.0);
}

// The sky dome under the original sky texture's clouds. The clear part of the
// texture (a painted gradient) becomes the table's sky at the brightness of the
// texture's clearest texel in the same row under the original dome light and
// its sun glow; its clouds stay as a mask (brighter than that texel by more
// than the dead zone litAtmo.w, or the overcast deck) and are lit by the sun
// seen from the cloud deck plus the sky above, with forward scattering toward
// the sun, and with lit clouds on (litSky.x) their relief and lining on top.
// `classic` is the original dome colour, which the night fade (litAtmo.x)
// returns to.
vec3 shadeSkyDome(vec4 t0, vec3 classic) {
    vec3 d = vWorldRel * inversesqrt(max(dot(vWorldRel, vWorldRel), 1e-12));
    float lumT = dot(t0.rgb, kLuma);
    float bg = lumT;
    vec3 bgC = t0.rgb;
    for (int k = 1; k < 12; ++k) {
        vec3 s = texture(tex0, vec2(vUV0.x + float(k) / 12.0, vUV0.y)).rgb;
        float ls = dot(s, kLuma);
        if (ls < bg) {
            bg = ls;
            bgC = s;
        }
    }
    float local = clamp((lumT - bg - litAtmo.w) / max(kSkyCloudWhite - bg, 0.08), 0.0, 1.0);
    float cloud = max(local, litAtmo.y);
    vec3 a = skyClear(d, bgC * litAtmoDome.rgb, bgC * litAtmoGlow.rgb, bgC);
    // The photoreal camera's clear sky gain (its clouds keep their light).
    if (litCamera.x > 0.5) a *= max(litCamera.z, 0.0);
    float c = dot(d, litSunDir.xyz);
    const float g = 0.6; // forward scattering of a thin cloud deck
    float hg = (1.0 - g * g) / (4.0 * kSkyPi * pow(max(1.0 + g * g - 2.0 * g * c, 1e-4), 1.5));
    float dirTerm = mix((0.3 + 2.0 * hg) / 0.46, 1.0, 0.8 * litAtmo.y);
    // The sky light on the clouds takes the colour of the sky above them (the
    // table's row mean 35 degrees up).
    vec3 above = texture(skyLut, vec2(2.5 / 3.0, skyRowV(0.61))).rgb;
    vec3 skyTint = above / max(dot(above, kLuma), 1e-8);
    // The original dome light of this row with its sun glow at the row's mean
    // weight: the clouds keep the original brightness on average, as the clear
    // sky does (litAtmoCloudSun is scaled to the dome light without the glow).
    float rowGlow = texture(skyLut, vec2(2.5 / 3.0, skyRowV(asin(clamp(d.y, -1.0, 1.0))))).a;
    vec3 rowLight = toLinear(min(litAtmoDome.rgb + litAtmoGlow.rgb * rowGlow, vec3(1.0)));
    float rowLum = max(dot(rowLight, kLuma), 1e-5);
    float gain = rowLum / max(dot(toLinear(litAtmoDome.rgb), kLuma), 1e-5);
    vec3 light = (litAtmoCloudSun.rgb * dirTerm + litAtmoCloudSun.w * skyTint) * gain;
    vec3 texLin = toLinear(t0.rgb);
    // A cloud's own colour: the texel taken out of the clear sky behind it by
    // the mask's alpha, so a faint cirrus is a white cloud at a low cover (the
    // texel is mostly the sky behind it) and a thick cloud keeps the texture's
    // shading; the overcast deck keeps the texture's grey.
    float alpha = (lumT - bg) / max(kSkyCloudWhite - bg, 0.08);
    vec3 cloudDisp = clamp(bgC + (t0.rgb - bgC) / max(alpha, 0.05), 0.0, 1.0);
    vec3 cloudTex = mix(toLinear(cloudDisp), texLin, litAtmo.y);
    float shade = dot(cloudTex, kLuma);
    float whiteLin = pow(kSkyCloudWhite, 2.2);
    vec3 cPhys = mix(vec3(shade * 0.8 / whiteLin), vec3(shade * 1.25), litAtmo.y) * light;
    vec3 cRelit = cloudTex * rowLight * (dot(light, kLuma) / rowLum);
    vec3 cloudLin = mix(cRelit, cPhys, litAtmoDome.w);
    // Lit clouds: the relief and lining of the density map's clouds, on the
    // cloud and on the sky behind it alike (the map also finds the faint bluish
    // streaks the mask leaves as sky).
    if (litSky.x > 0.0) {
        vec4 lc = litCloudTermsDome(vUV0);
        a = litCloudLight(a, lc);
        cloudLin = litCloudLight(cloudLin, lc);
    }
    // Cover over clear sky in display space, where the mask measured it: a
    // faint cloud keeps about the texture's own contrast (mixed in linear light
    // the texture's faint blotches read 40 % stronger than in the original).
    vec3 disp = mix(skyDisplay(a), skyDisplay(cloudLin), cloud);
    return mix(classic, disp, litAtmo.x);
}

// The clear sky a mirror-like surface reflects along the unit direction r
// (above the horizon), display. It is the sky the player sees: without the sky
// table the fog colour with its sun glow (fogColorToward, the original sky
// behind the horizon); with it, the table's sky in that direction at that
// brightness, as the dome draws it, and the classic colour again under an
// overcast deck (litAtmo.y) and as the table fades out at night (litAtmo.x).
vec3 skyReflection(vec3 r) {
    vec3 classic = fogColorToward(r);
    if (litAtmo.x <= 0.0) return classic;
    vec3 glow = litFogSun.w > 0.5 ? litFogSun.rgb : vec3(0.0);
    vec3 disp = skyDisplay(skyClear(r, fogColor.rgb, glow, max(litFogCap.rgb, fogColor.rgb)));
    disp = mix(disp, classic, litAtmo.y);
    return mix(classic, disp, litAtmo.x);
}
)";

} // namespace Poseidon::gl33lighting
