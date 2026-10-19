// Remastered lighting shaders: the surface pieces of the fragment prelude
// (FragmentPiece::Puddles and FragmentPiece::Surface, joined by
// RemasterFragmentCommon() in EngineGL33_LightingShaders.cpp): where puddles
// stand, the sun highlight and sky reflection of glossy materials, and the wet
// weather's response of every surface. Material and wet-surface responses
// belong here.
//
// Wet weather (feature switches wetWeather and lampStreaks, plan section 5 B7;
// Graphics/Remaster/Wetness.hpp holds the world's wetness, SurfaceMaterials.hpp
// the surface classes, and both mirror the constants below that a test pins):
//
//   - litWeather (x surface wetness, y rain, z puddle level, w time) is zero
//     while the gate is off, and then no fragment does any of this.
//   - Each lit draw carries its surface class (litMatClass.x, from its texture
//     name and object kind) and the main tells objects, the land and grass
//     clutter apart (g_wetDraw). The land tells grass from bare ground per
//     pixel from its albedo (wetVegetation).
//   - Water fills the pores: a surface darkens by its porosity (cloth a lot,
//     plaster less, paint not at all). What it does not soak up forms a smooth
//     film (paint, glass, metal, asphalt), which reflects the sky (objects; the
//     ground's reflection is the reflections' pass while it runs, litMatClass.w),
//     the sun and, at night, every lamp, fire and headlight as a streak toward
//     the eye (lampStreaks, litMatClass.z). Grass blades break the film into a
//     dull sheen.
//   - Rain reaches up-facing surfaces fully, vertical ones with run-down
//     streaks near the eye, undersides little; the hills' sky visibility shelters
//     the ground. Puddles (wetPuddle, shared with the reflections' trace) are
//     dark smooth water with rain rings while it rains.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

// The puddle field: a pure function of the world position, so the forward
// pass (the puddle's darkening and its water film) and the reflections' trace
// (its mirror, EngineGL33_PostSsrShaders.cpp) agree pixel for pixel without a
// buffer between them. It reads the LitConstants block and the terrain height
// map (terrainHeight, unit 4), which both declare.
const char kRemasterPuddlesGLSL[] = R"(
// ---- Puddles (wetWeather; Graphics/Remaster/Wetness.hpp) ----
// Level ground pools water: three octaves of value noise (the finer ones fade
// out below the pixel's footprint, so far puddles do not shimmer), raised in
// the hollows of the height map and held to its level stretches. The height
// map is read with smoothed interpolation between its texels, so neither the
// hollows nor the slope crease along the terrain's cells and triangles (the
// depth buffer's normal is flat per triangle). Roads pool water readily, bare
// ground less, grass only in its deepest hollows.
const float kPuddleRoad = 0.61;       // noise threshold on roads
const float kPuddleGround = 0.67;     // on bare ground
const float kPuddleGrass = 0.74;      // on grass
const float kPuddleEdge = 0.035;      // the threshold's soft edge
const float kPuddleDamp = 0.06;       // the damp margin below the threshold
const float kPuddleHollow = 0.1;      // the field's rise per metre of hollow (capped at a metre) ...
const float kPuddleCrest = 0.3;       // ... and its fall per metre of crest (capped here: an open slope keeps some)
const float kPuddleLevelShift = 0.08; // a low puddle level raises the threshold this much
const float kPuddleFadeStart = 150.0; // puddles shrink below the pixel from here ...
const float kPuddleFadeEnd = 400.0;   // ... and the far ground is one wet sheen

float puddleHash(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}
float puddleNoise(vec2 p) {
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(puddleHash(i), puddleHash(i + vec2(1, 0)), u.x),
               mix(puddleHash(i + vec2(0, 1)), puddleHash(i + vec2(1, 1)), u.x), u.y);
}
// The terrain's height at world xz, smoothly interpolated between the map's
// texels (a smoothstep on the bilinear weights: no crease at the cell edges).
float puddleHeight(vec2 xz) {
    vec2 t = xz * litTerrain.y;
    vec2 i = floor(t);
    vec2 f = t - i;
    f = f * f * (3.0 - 2.0 * f);
    return textureLod(terrainHeight, (i + f + 0.5) * litTerrain.z, 0.0).r;
}

// x: standing water 0..1, y: the damp margin around it. posW the world
// position, footT the pixel's footprint along the ground (m), road 1 on a road,
// veg the grass share (0..1), level the puddle level (litWeather.z).
vec2 wetPuddle(vec3 posW, float footT, float road, float veg, float level) {
    if (level <= 0.0) return vec2(0.0);
    float o2 = mix(0.5, puddleNoise(posW.xz / 1.3 + 17.0), clamp(1.3 / (3.0 * footT), 0.0, 1.0));
    float o3 = mix(0.5, puddleNoise(posW.xz / 0.45 + 41.0), clamp(0.45 / (3.0 * footT), 0.0, 1.0));
    float field = puddleNoise(posW.xz / 3.5) * 0.55 + o2 * 0.3 + o3 * 0.15;
    float flatness = 1.0;
    if (litTerrain.x > 0.5) {
        float cell = 1.0 / litTerrain.y;
        float h0 = puddleHeight(posW.xz);
        float hx0 = puddleHeight(posW.xz - vec2(cell, 0.0));
        float hx1 = puddleHeight(posW.xz + vec2(cell, 0.0));
        float hz0 = puddleHeight(posW.xz - vec2(0.0, cell));
        float hz1 = puddleHeight(posW.xz + vec2(0.0, cell));
        field += kPuddleHollow * clamp((hx0 + hx1 + hz0 + hz1) * 0.25 - h0, -kPuddleCrest, 1.0);
        vec2 g = vec2(hx1 - hx0, hz1 - hz0) * (0.5 / cell);
        flatness = smoothstep(0.85, 0.98, inversesqrt(1.0 + dot(g, g)));
    }
    float th = mix(mix(kPuddleGround, kPuddleRoad, road), kPuddleGrass, veg) + kPuddleLevelShift * (1.0 - level);
    float keep = flatness * min(level * 4.0, 1.0) * (1.0 - smoothstep(kPuddleFadeStart, kPuddleFadeEnd, length(posW - litCamPos.xyz)));
    return vec2(smoothstep(th, th + kPuddleEdge, field), smoothstep(th - kPuddleDamp, th, field)) * keep;
}
)";

const char kRemasterSurfaceGLSL[] = R"(
// Sun highlight and sky reflection for glossy materials (specular power > 0).
vec3 glossy(vec3 n, vec3 v, float sunVisibility) {
    float power = litMatSpecular.w;
    if (power <= 0.0) return vec3(0.0);
    vec3 l = litSunDir.xyz;
    float nl = dot(n, l);
    vec3 h = normalize(l + v);
    float nh = max(dot(n, h), 0.0);
    float vh = max(dot(v, h), 0.0);
    float p = max(power, 1.0);
    // Normalised Blinn-Phong (tighter lobes are brighter) with a Schlick-style
    // grazing boost.
    float norm = (p + 8.0) / 28.0;
    float fresnel = 1.0 + 2.0 * pow(clamp(1.0 - vh, 0.0, 1.0), 5.0);
    vec3 sunSpec = litMatSpecular.rgb * litSun.rgb * (pow(nh, p) * norm * fresnel * smoothstep(0.0, 0.15, nl) * sunVisibility);
    vec3 r = reflect(-v, n);
    float fv = pow(clamp(1.0 - dot(n, v), 0.0, 1.0), 5.0);
    float gloss = clamp(dot(litMatSpecular.rgb, kLuma), 0.0, 1.0);
    vec3 skySpec = skyIrradiance(r) * (gloss * (0.04 + 0.96 * fv) * 0.6);
    return (sunSpec + skySpec) * litHorizon.w;
}

// ---- Wet weather (wetWeather, lampStreaks) ----
// The porosity of each surface class (SurfaceMaterials.hpp SurfacePorosity):
// unknown, paint, metal, glass, rubber, fabric, skin, road, masonry, wood,
// leather, weapon, lens, roof.
const float kWetPorosity[14] = float[](0.5, 0.05, 0.0, 0.0, 0.1, 0.85, 0.3, 0.35, 0.6, 0.7, 0.4, 0.0, 0.0, 0.4);
const float kWetDarken = 0.55;      // a soaked object keeps 1 - this times its porosity of its albedo ...
const float kWetGroundDarken = 0.35; // ... the ground this (B6's wet ground kept 0.8 of its colour)
const float kWetSaturate = 0.3;     // objects: saturation gain at porosity 1 (never the ground: no mustard)
// How much a matte sealed surface darkens when wet, as a porosity (paint, rubber,
// skin, leather, unknown); glass, metal, weapons and lenses keep their colour.
const float kWetMatte[14] = float[](0.35, 0.35, 0.0, 0.0, 0.35, 0.0, 0.35, 0.0, 0.0, 0.0, 0.35, 0.0, 0.0, 0.0);
const float kWetGrassPorosity = 0.23; // grass darkens a little (0.92 of its albedo soaked)
const float kWetFoliageDark = 0.88; // leaves shed the water: soaked crowns keep this of their albedo
const float kWetPuddleDark = 0.55;  // the ground under a puddle's water
const float kWetDampDark = 0.9;     // the damp margin around a puddle
const float kWetFilmRough = 0.12;   // the water film's roughness
const float kWetPuddleRough = 0.05; // a puddle's
const float kWetGrassFilm = 0.1;    // grass blades break the film up: this share of it ...
const float kWetGrassRough = 0.4;   // ... this rough
const float kWetSunRough = 0.3;     // the film's sun glint is at least this rough (a sharp one burns white)
const float kWetSunGroundRough = 0.5; // bare ground's: grit and stones stand through the thin film ...
const float kWetSunGroundFilm = 0.5;  // ... and break up this share of it
const float kWetSunFoot = 0.25;     // a pixel's footprint (m) widens the glint's alpha by this per metre
const float kWetSunCap = 3.0;       // the sun glint's peak (the low sun's lobe would clip a white column)
const float kWetObjectSky = 0.5;    // objects: rain beads, so their film reflects this share of the sky ...
const float kWetObjectSkyRough = 0.3; // ... at least this rough ...
const float kWetSkyCap = 0.12;      // ... and at most this share of it ...
const float kWetSkyFog = 0.5;       // ... of the sky tinted this far toward the fog colour (wet paint not blue)
const float kWetSpecCap = 80.0;     // a lamp's lobe's peak
const float kWetBulb = 0.15;        // a lamp's radius (m): widens the lobe near it
const float kWetStreakNear = 12.0;  // run-down streaks on walls and hulls fade out from here ...
const float kWetStreakFar = 20.0;   // ... to here (m)
const float kWetRingsFar = 40.0;    // rain rings on puddles within this distance

// What the main is shading: 0 an object (its class from litMatClass.x), 1 the
// land, 2 grass clutter, 3 the sea on the detail program (never wet).
int g_wetDraw = 0;

// The grass share of a ground albedo (display): green above red and blue,
// or olive and yellow-green grass (green about equal to red, well above blue).
float wetVegetation(vec3 c) {
    float green = clamp((c.g - max(c.r, c.b)) * 10.0 + (c.g - c.b) * 1.5, 0.0, 1.0);
    float gr = c.g / max(c.r, 1e-3);
    float gb = c.g / max(c.b, 1e-3);
    return max(green, smoothstep(1.1, 1.3, gb) * smoothstep(0.9, 1.0, gr));
}

// The porosity of bare ground from its albedo: soil and sand soak up a lot,
// grey gravel and rock little.
float wetGroundPorosity(vec3 c) {
    float mx = max(c.r, max(c.g, c.b));
    float sat = (mx - min(c.r, min(c.g, c.b))) / max(mx, 1e-3);
    float warm = clamp((c.r - c.b) * 5.0, 0.0, 1.0);
    float rock = clamp(1.0 - sat * 5.0, 0.0, 1.0);
    return mix(mix(0.7, 0.9, warm), 0.3, rock);
}

// How much rain a surface takes: up-facing fully, vertical with run-down
// streaks near the eye (low and soft: hulls must not stripe), undersides
// little; sheltered by the hills' sky visibility.
float wetCoverage(vec3 n, vec3 posAbs, float skyVis, float dist) {
    float facing = mix(0.3, 1.0, smoothstep(-0.6, 0.0, n.y));
    float vertical = (1.0 - smoothstep(0.55, 0.9, n.y)) * (1.0 - smoothstep(kWetStreakNear, kWetStreakFar, dist));
    if (vertical > 0.0) {
        vec2 t = vec2(-n.z, n.x);
        float u = dot(posAbs.xz, t) * inversesqrt(max(dot(t, t), 1e-6));
        float streak = textureLod(cloudNoise, vec2(u * 0.2, posAbs.y * 0.03), 0.0).r;
        facing *= mix(1.0, mix(0.75, 1.0, smoothstep(0.3, 0.7, streak)), vertical);
    }
    return facing * mix(0.45, 1.0, skyVis);
}

// Rain rings on standing water: the slope of expanding rings, one drop per
// half-metre cell at a random phase.
vec2 wetRainRings(vec2 p, float t) {
    vec2 cell = floor(p);
    vec2 f = fract(p);
    vec2 g = vec2(0.0);
    for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
            vec2 o = vec2(float(i), float(j));
            vec2 h = vec2(puddleHash(cell + o), puddleHash(cell + o + 7.3));
            float tt = fract(t * 0.9 + puddleHash(cell + o + 17.0));
            vec2 d = f - (o + h);
            float r = length(d);
            float x = (r - tt * 0.6) * 22.0;
            g += d / max(r, 1e-3) * (sin(x * 2.4) * exp(-x * x * 0.35) * (1.0 - tt) * (1.0 - tt));
        }
    }
    return g;
}

// The GGX reflection of a light toward unit L at distance d on a surface of
// unit normal n, seen from v: pi D V F (n.l), the light a small sphere
// (Karis's representative-point widening), Fresnel of water, the peak at most
// `cap`.
float wetLobe(vec3 L, float d, vec3 n, vec3 v, float nv, float a, float cap) {
    float nl = dot(n, L);
    if (nl <= 0.0) return 0.0;
    float aP = clamp(a + kWetBulb / (2.0 * max(d, 0.1)), 0.0, 1.0);
    float norm = (a / aP) * (a / aP);
    vec3 h = L + v;
    h *= inversesqrt(max(dot(h, h), 1e-12));
    float nh = max(dot(n, h), 0.0);
    float vh = max(dot(v, h), 0.0);
    float a2 = aP * aP;
    float dd = nh * nh * (a2 - 1.0) + 1.0;
    float D = a2 / (3.14159265 * dd * dd);
    float vis = 0.5 / max(nl * sqrt(nv * nv * (1.0 - a2) + a2) + nv * sqrt(nl * nl * (1.0 - a2) + a2), 1e-5);
    float F = 0.02 + 0.98 * pow(1.0 - vh, 5.0);
    return min(3.14159265 * D * vis * nl * norm, cap) * F;
}

// The split-sum environment BRDF, analytic fit (Karis 2014): the film's
// reflectance toward the sky is f0 * x + y.
vec2 wetEnvBRDF(float rough, float nv) {
    const vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    const vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
    vec4 r = rough * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * nv)) * r.x + r.y;
    return vec2(-1.04, 1.04) * a004 + r.zw;
}

// The local lights' streaks in the water film at this fragment (linear light,
// before the exposure): the lights of the tiled list (the physical lamps and
// low beams at their physical intensity, the other kinds at their classic
// value), or the draw's own list.
vec3 wetLampStreaks(vec3 n, vec3 v, float rough) {
    vec3 sum = vec3(0.0);
    float nv = clamp(dot(n, v), 1e-3, 1.0);
    float a = max(rough * rough, 0.002);
    if (litLightCount.y > 0.5) {
        int count = loadTileList();
        for (int k = 0; k < kMaxTileLights; ++k) {
            if (k >= count) break;
            int id = tileEntry(k);
            int at = (id - 1) * kTexelsPerLight;
            vec4 pos = frameLights[at];
            vec4 color = frameLights[at + 1];
            vec3 toLight = pos.xyz - vWorldRel;
            float d2 = dot(toLight, toLight);
            float d = sqrt(max(d2, 1e-12));
            vec3 L = toLight / d;
            bool spot = color.w > 0.5;
            vec3 light;
            if (physicalKind(id)) {
                float peak = spot ? litNight.z : litNight.y * pos.w;
                float end2 = min(kReachFactor * kReachFactor * pos.w * pos.w, peak / kNightMinIrradiance);
                if (d2 >= end2) continue;
                float I = spot ? peak * lowBeamPattern(-L, frameLights[at + 3].xyz) : peak;
                I *= (1.0 - smoothstep(0.5 * end2, end2, d2)) * smoothstep(kNightNearStart, kNightNearEnd, d);
                vec3 col = toLinear(color.rgb);
                if (spot) col = vec3(dot(col, kLuma));
                light = col * (I / (d2 + kNightSoftening));
            } else {
                float start2 = pos.w * pos.w;
                float end2 = start2 * 100.0;
                if (d2 >= end2) continue;
                float cone = 1.0;
                if (spot) {
                    float inside = -dot(toLight, frameLights[at + 3].xyz);
                    if (inside <= 0.0) continue;
                    cone = smoothstep(0.9415, 0.9806, inside * inside / max(d2, 1e-6));
                }
                float atten = (d2 > start2 ? start2 / d2 : 1.0) * (1.0 - smoothstep(0.5 * end2, end2, d2)) * cone;
                light = toLinear(min(color.rgb * (atten * litGround.w), vec3(1.5)));
            }
            sum += light * wetLobe(L, d, n, v, nv, a, kWetSpecCap);
        }
        return sum * litLightCount.z;
    }
    int count = int(litLightCount.x);
    for (int i = 0; i < 8; ++i) {
        if (i >= count) break;
        vec4 pos = litLightPos[i];
        vec3 toLight = pos.xyz - vWorldRel;
        float d2 = dot(toLight, toLight);
        float start2 = pos.w * pos.w;
        float end2 = start2 * 100.0;
        if (d2 >= end2) continue;
        float cone = 1.0;
        if (litLightColor[i].w > 0.5) {
            float inside = -dot(toLight, litLightDir[i].xyz);
            if (inside <= 0.0) continue;
            cone = smoothstep(0.9415, 0.9806, inside * inside / max(d2, 1e-6));
        }
        float atten = (d2 > start2 ? start2 / d2 : 1.0) * (1.0 - smoothstep(0.5 * end2, end2, d2)) * cone;
        float d = sqrt(max(d2, 1e-12));
        sum += toLinear(min(litLightColor[i].rgb * (atten * litGround.w), vec3(1.5))) * wetLobe(toLight / d, d, n, v, nv, a, kWetSpecCap);
    }
    return sum;
}

// The wet response of one lit fragment, called by shadeRemastered while
// litWeather is set. `a` (linear albedo) darkens and saturates; `keep` scales
// the diffuse light (the film's reflection takes its share); `lit` is the film's
// sun glint and lamp streaks (linear, before the exposure); `env` its sky
// reflection (linear, after it, as the sea's); `mirror` the standing water's
// share (no sky occlusion on a mirror).
void wetSurface(vec3 albedo, vec3 n, vec3 v, vec3 posAbs, float skyVisibility, float sunLight, bool foliage,
                inout vec3 a, out float keep, out vec3 lit, out vec3 env, out float mirror) {
    keep = 1.0;
    lit = vec3(0.0);
    env = vec3(0.0);
    mirror = 0.0;
    // The pixel's footprint (m) for the sun glint, taken before the first
    // return that varies per pixel (a derivative needs the whole quad).
    float sunFoot = kWetSunFoot * length(fwidth(posAbs));
    if (g_wetDraw == 3) return;
    float dist = length(vWorldRel);
    float wet = clamp(litWeather.x, 0.0, 1.0);
    if (foliage) {
        a *= mix(1.0, kWetFoliageDark, wet * mix(0.45, 1.0, skyVisibility));
        return;
    }
    bool ground = g_wetDraw == 1 || g_wetDraw == 2 || litMatClass.y > 7.5;
    int cls = int(clamp(litMatClass.x + 0.5, 0.0, 13.0));
    float veg = g_wetDraw == 2 ? 1.0 : (g_wetDraw == 1 ? wetVegetation(albedo) : 0.0);
    float porosity = g_wetDraw == 0 ? kWetPorosity[cls] : mix(wetGroundPorosity(albedo), kWetGrassPorosity, veg);
    float w = (ground ? mix(0.45, 1.0, skyVisibility) : wetCoverage(n, posAbs, skyVisibility, dist)) * wet;
    // Standing water on the land and on roads.
    vec2 pud = vec2(0.0);
    if (ground && g_wetDraw != 2) {
        float footT = max(length(fwidth(posAbs.xz)), 1e-3);
        pud = wetPuddle(posAbs, footT, (g_wetDraw == 0 || cls == 7) ? 1.0 : 0.0, veg, litWeather.z);
    }
    w = max(w, pud.y);
    if (w <= 0.0 && pud.x <= 0.0) return;
    // Water fills the pores; on a matte sealed surface (paint, rubber) the film
    // takes away the surface's own pale scatter: darker and deeper either way.
    float soak = max(porosity, kWetMatte[cls] * float(g_wetDraw == 0));
    if (!ground) {
        float l = dot(a, kLuma);
        a = max(mix(vec3(l), a, 1.0 + kWetSaturate * w * soak), vec3(0.0));
    }
    a *= (1.0 - (ground ? kWetGroundDarken : kWetDarken) * soak * w) * mix(1.0, kWetDampDark, pud.y * (1.0 - pud.x)) *
         mix(1.0, kWetPuddleDark, pud.x);
    mirror = pud.x;
    // The film: what the surface does not soak up; grass breaks it up.
    float film = w * (1.0 - 0.75 * porosity) * (1.0 - veg) + kWetGrassFilm * w * veg;
    float filmW = max(film, pud.x);
    if (filmW <= 0.0) return;
    float rough = mix(kWetFilmRough, kWetGrassRough, veg);
    vec3 nf = n;
    if (pud.x > 0.0) {
        // A puddle is level; rain rings break it while it rains near the eye.
        nf = vec3(0.0, 1.0, 0.0);
        float rain = clamp(litWeather.y, 0.0, 1.0);
        if (rain > 0.02 && dist < kWetRingsFar) {
            vec2 g = wetRainRings(posAbs.xz * 2.2, litWeather.w) * (rain * (1.0 - smoothstep(15.0, kWetRingsFar, dist)));
            nf = vec3(-g.x * 0.06, 1.0, -g.y * 0.06);
        }
        nf = mix(n, nf, pud.x);
        nf *= inversesqrt(max(dot(nf, nf), 1e-12));
    }
    float nv = clamp(dot(nf, v), 1e-3, 1.0);
    // The sun's glint: the film's lobe and the puddle's, blended by the water's
    // share as the lamps' streaks below, no sharper than kWetSunRough and its
    // peak far below a lamp's. On bare ground grit and stones stand through the
    // film, so its glint is rougher and smaller; a pixel that covers much
    // ground averages many facets, so the footprint widens the lobe further
    // (no clipped column and no white dashes on far wet ground).
    float sun = 0.0;
    if (pud.x < 1.0) {
        float rs = max(rough, ground ? kWetSunGroundRough : kWetSunRough);
        sun += wetLobe(litSunDir.xyz, 1e4, nf, v, nv, min(rs * rs + sunFoot, 1.0), kWetSunCap) *
               (film * (1.0 - pud.x) * (ground ? kWetSunGroundFilm : 1.0));
    }
    if (pud.x > 0.0) {
        float ap = min(kWetSunRough * kWetSunRough + sunFoot, 1.0);
        sun += wetLobe(litSunDir.xyz, 1e4, nf, v, nv, ap, kWetSunCap) * pud.x;
    }
    lit = litSun.rgb * (sun * sunLight * litHorizon.w);
    // The lamps' streaks: the film's lobe and the puddle's, each at its own
    // roughness and blended by the water's share (a lobe of the blended
    // roughness drew a bright ring at every puddle's edge).
    if (litMatClass.z > 0.5) {
        if (pud.x < 1.0) lit += wetLampStreaks(nf, v, rough) * (film * (1.0 - pud.x));
        if (pud.x > 0.0) lit += wetLampStreaks(nf, v, kWetPuddleRough) * pud.x;
    }
    // The sky in the film: objects always (weaker: rain beads on them); the
    // ground only while the reflections' pass does not reflect it (it then
    // reflects the ground with what is on screen, sky included). A face that
    // looks down reflects the ground, which its own light already shows.
    if (!ground || litMatClass.w < 0.5) {
        vec3 r = reflect(-v, nf);
        float up = smoothstep(-0.1, 0.1, r.y);
        r.y = max(r.y, 0.02);
        r *= inversesqrt(dot(r, r));
        vec3 sky = mix(toLinear(skyReflection(r)), toLinear(fogColorToward(r)), kWetSkyFog);
        vec2 ab = wetEnvBRDF(ground ? mix(rough, kWetPuddleRough, pud.x) : max(rough, kWetObjectSkyRough), nv);
        float F = (0.02 * ab.x + ab.y) * filmW * up;
        F = ground ? F : min(F * kWetObjectSky, kWetSkyCap);
        env = sky * (F * mix(0.3, 1.0, skyVisibility) * litSunDir.w);
        keep = 1.0 - F;
    }
}
)";

} // namespace Poseidon::gl33lighting
