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
//     clutter apart (g_wetDraw). The land tells grass from bare ground, and
//     how porous it is, from the ground field (groundFieldAt: the land's own
//     colour, continuous across the landscape's squares), not per pixel from
//     its square's texture, whose class jumped at every texture edge.
//   - Water fills the pores: a surface darkens by its porosity (cloth a lot,
//     plaster less, paint not at all). What it does not soak up forms a smooth
//     film (paint, glass, metal, asphalt), which reflects the sky (objects; the
//     ground's reflection is the reflections' pass while it runs, litMatClass.w),
//     the sun and, at night, every lamp, fire and headlight as a streak toward
//     the eye (lampStreaks, litMatClass.z). Grass blades break the film into a
//     dull sheen. On the ground only the puddles mirror: around them the film
//     deepens the colour and glints dimly and broadly near a light.
//   - Rain reaches up-facing surfaces fully, vertical ones with run-down
//     streaks near the eye, undersides little; the hills' sky visibility shelters
//     the ground. Puddles (wetPuddle, shared with the reflections' trace) are
//     dark smooth water with rain rings while it rains.
//
// The look lane "surface" (Graphics/Remaster/SurfaceLight.hpp): the sky light
// from the sky table's spherical harmonics, the sky hidden by the canopy, the
// texture's relief and cavities, and a Fresnel specular on every surface.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

// The puddle field: a pure function of the world position, so the forward
// pass (the puddle's darkening and its water film) and the reflections' trace
// (its mirror, EngineGL33_PostSsrShaders.cpp) agree pixel for pixel without a
// buffer between them. It reads the LitConstants block, the terrain height
// map (terrainHeight, unit 4) and the ground field (groundField, unit 12),
// which both declare. The land's classes (grass, bare ground, its porosity)
// are here too, from the ground field, so both passes class the land alike.
const char kRemasterPuddlesGLSL[] = R"(
int wetProbe() { return litWeather.x > 0.5 ? int(floor((1.0 - litWeather.x) * 1000.0 + 0.5)) : -1; }
bool wetProbeIs(int a) { return wetProbe() == a; }
// ---- The land's wet classes (wetWeather; TerrainHorizon.hpp groundColor) ----
// The grass share of a ground colour (display): green above red and blue,
// or olive and yellow-green grass (green about equal to red, well above blue).
float wetVegetation(vec3 c) {
    float green = clamp((c.g - max(c.r, c.b)) * 10.0 + (c.g - c.b) * 1.5, 0.0, 1.0);
    float gr = c.g / max(c.r, 1e-3);
    float gb = c.g / max(c.b, 1e-3);
    return max(green, smoothstep(1.1, 1.3, gb) * smoothstep(0.9, 1.0, gr));
}

// The porosity of bare ground from its colour: soil and sand soak up a lot,
// grey gravel and rock little.
float wetGroundPorosity(vec3 c) {
    float mx = max(c.r, max(c.g, c.b));
    float sat = (mx - min(c.r, min(c.g, c.b))) / max(mx, 1e-3);
    float warm = clamp((c.r - c.b) * 5.0, 0.0, 1.0);
    float rock = clamp(1.0 - sat * 5.0, 0.0, 1.0);
    return mix(mix(0.7, 0.9, warm), 0.3, rock);
}

// The ground's own colour (display) at world xz: the landscape textures'
// average colours blended over the squares that share each vertex
// (SampleGroundColor mirrors this), read with smoothed interpolation between
// the texels, so it has no edge and no crease at any square or grid line.
// a = 1 where the field is built, 0 without a map or ground colours.
// The land is classed from this, not from the pixel's own texture: each
// square of the landscape has its own texture, and a class read per pixel
// jumped at every square whose texture differed a little from its neighbour's
// (a straight seam in the sheen and the puddles; the owner's wet seam).
vec4 groundFieldAt(vec2 xz) {
    if (litTerrain.x < 0.5) return vec4(0.0);
    vec2 t = xz * litTerrain.y;
    vec2 i = floor(t);
    vec2 f = t - i;
    f = f * f * (3.0 - 2.0 * f);
    return textureLod(groundField, (i + f + 0.5) * litTerrain.z, 0.0);
}

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
    // The land's grass share is an average over mixed ground (the ground
    // field), whose water stands in its bare patches: the threshold leans
    // toward bare ground's (veg squared), so a mixed square holds about the
    // puddles its bare and grassy parts would hold apart.
    float th = mix(mix(kPuddleGround, kPuddleRoad, road), kPuddleGrass, veg * veg) + kPuddleLevelShift * (1.0 - level)
        - (wetProbeIs(11) ? 0.03 : 0.0) - (wetProbeIs(12) ? 0.06 : 0.0) - (wetProbeIs(15) ? 0.04 : 0.0);
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
// The ground outside its puddles: grit, stones and grass stand through the thin
// film, so it mirrors nothing. Its glints (the sun's, the lamps') are this
// rough, a dim broad sheen near a light, never a streak or an image ...
const float kWetSunGroundRough = 0.6;
const float kWetSunGroundFilm = 0.3;  // ... at this share of the film
const float kWetGroundFilmDark = 0.25; // and the film darkens the ground by this share of it (deeper, not shinier)
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
vec2 g_probePud = vec2(0.0);

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

// ---- Surfaces and ambient light (litSurface.x; SurfaceLight.hpp) -------------
const float kSurfPi = 3.14159265;
// The share of a leaf's occludable light the post pass's horizon occlusion may
// darken (shadeRemastered): at most kSurfLeafShare of it goes, whatever the
// search finds among a crown's cards.
const float kSurfLeafShare = 0.4;

// The clear sky's irradiance onto n from the sky table's spherical harmonics:
// n turned into the sun frame (x toward the sun's azimuth, y up, z across;
// the sky is mirror-symmetric in z, so its sign does not matter). The frame
// is the sky's own sun (litSkySun): under the moon key (B3) litSunDir is the
// moon, but the table was projected for the sun.
vec3 skyLightSH(vec3 n) {
    vec2 s = litSkySun.xz;
    float sl = length(s);
    s = sl > 1e-4 ? s / sl : vec2(1.0, 0.0);
    float x = dot(n.xz, s);
    float y = n.y;
    float z2 = max(1.0 - x * x - y * y, 0.0);
    vec3 e = litSkySh[0].rgb * 0.282095 + litSkySh[1].rgb * (0.488603 * y) + litSkySh[2].rgb * (0.488603 * x) +
             litSkySh[3].rgb * (0.315392 * (3.0 * y * y - 1.0)) + litSkySh[4].rgb * (1.092548 * x * y) +
             litSkySh[5].rgb * (0.546274 * (x * x - z2));
    return max(e, vec3(0.0));
}

// The sky light onto n: the rig's hemisphere, or (by litSurface.w) the sky
// table's sky above and the ground's own bounce below (the rig's ground
// irradiance in the ground's colour, over the share of the lower hemisphere n
// faces). The table gives the sky's brightness toward n; its colour is the
// rig's own sky toward n (its hemisphere less its ground bounce) with
// litSurface2.z of the table's mixed in, so a hull's shaded side keeps the
// rig's horizon colour instead of the zenith's blue. Leaves keep the rig's
// hemisphere: their crowns are lit as a canopy (crownCanopy) against the rig's
// sky.
// g_surfGroundShare: the share of that light the ground below bounces (the
// rig's hemisphere: its ground and half its horizon's bounce), which the
// horizon occlusion darkens less (the ground the search finds around a
// receiver is that light's source, not an occluder of it).
float g_surfGroundShare = 0.0;
vec3 surfaceSkyIrradiance(vec3 n, bool leaves) {
    vec3 rig = skyIrradiance(n);
    float t = clamp(n.y, -1.0, 1.0);
    vec3 rigGround = litGround.rgb * g_groundTint * (t >= 0.0 ? 0.5 * (1.0 - t) : 0.5 + 0.5 * (-t));
    vec3 ground = litGround.rgb * g_groundTint * clamp(0.5 - 0.5 * n.y, 0.0, 1.0);
    vec3 e = rig;
    float w = leaves ? 0.0 : litSurface.w;
    if (w > 0.0) {
        vec3 rigSky = max(rig - rigGround, vec3(0.0));
        vec3 sh = skyLightSH(n);
        vec3 sky = mix(rigSky * (dot(sh, kLuma) / max(dot(rigSky, kLuma), 1e-6)), sh, litSurface2.z);
        e = mix(rig, sky + ground, w);
        rigGround = mix(rigGround, ground, w);
    }
    g_surfGroundShare = clamp(dot(rigGround, kLuma) / max(dot(e, kLuma), 1e-6), 0.0, 1.0);
    return e;
}

// Sky visibility under the canopy heightfield (Canopy.hpp: tree crowns,
// forest blocks, buildings; r the top's height, g the cover's density), at
// two scales: the texel above the receiver (6 m) and the neighbourhood
// (25 m). A receiver well below the top loses the sky its cover hides; one
// near the top of its own crown keeps it. The lookup moves out along the
// receiver's horizontal facing, so a wall or the outer leaves of a crown read
// the cover in front of them, not the footprint of their own building or
// tree. Leaves lose at most kSurfCanopyLeafMax (the core of their own crown
// is the crown canopy's, crownSky). 1 off.
const float kSurfCanopyLeafMax = 0.35;
float canopySkyVisibility(vec3 posAbs, vec3 n, bool foliage) {
    if (litSurface.z <= 0.0 || litSurfCanopy.w < 0.5) return 1.0;
    vec2 uv = (posAbs.xz + n.xz * 4.0) * litSurfCanopy.z;
    if (uv.x < 0.0 || uv.y < 0.0 || uv.x > 1.0 || uv.y > 1.0) return 1.0;
    float occ = 0.0;
    const vec2 kLods = vec2(0.0, 2.0);
    // The coarser level keeps the tallest cover over a wider area (a town's
    // roofs over its open square): it counts less.
    const vec2 kWeight = vec2(0.85, 0.6);
    for (int i = 0; i < 2; ++i) {
        vec2 c = textureLod(canopyHeight, uv, kLods[i]).rg;
        float top = litSurfCanopy.x + c.r * litSurfCanopy.y;
        float below = top - posAbs.y;
        // Under its own crown a leaf sees less of the cover above it.
        float depth = smoothstep(foliage ? 2.0 : 0.3, foliage ? 9.0 : 3.5, below);
        occ = max(occ, c.g * depth * kWeight[i]);
    }
    return 1.0 - clamp(occ * litSurface.z, 0.0, foliage ? kSurfCanopyLeafMax : 0.92);
}

// Relief from the texture (Mikkelsen 2010 surface gradients, no tangents): a
// main sets g_surfBumpN before shadeRemastered, which then lights the relief's
// normal with the geometric one's energy: the sun term moves around its
// geometric value by the unnormalised gradient's share (g_surfBumpDelta, zero
// on average, so the relief adds contrast and takes no light from the land
// under a high sun) and by at most kSurfReliefSunLimit of it (a grazing sun
// never digs black pockets). Zero = no relief. `sx`, `sy` are the screen
// derivatives of vWorldRel, `dB` the height change toward the next pixel in x
// and y (metres).
vec3 g_surfBumpN = vec3(0.0);
vec3 g_surfBumpDelta = vec3(0.0);
const float kSurfReliefHeight = 0.04;   // the land's metres of relief per unit of albedo luminance
const float kSurfReliefSunLimit = 0.35; // the relief moves the sun term by at most this share of N.L
vec3 surfaceBumpNormal(vec3 n, vec3 sx, vec3 sy, vec2 dB) {
    vec3 r1 = cross(sy, n);
    vec3 r2 = cross(n, sx);
    float det = dot(sx, r1);
    vec3 grad = sign(det) * (dB.x * r1 + dB.y * r2);
    vec3 nb = abs(det) * n - grad;
    float len2 = dot(nb, nb);
    g_surfBumpDelta = abs(det) > 1e-20 ? -grad / abs(det) : vec3(0.0);
    return len2 > 1e-20 ? nb * inversesqrt(len2) : n;
}

// The height change toward the next pixel in x and y, read from the texture
// itself: the fragment and the points one pixel's footprint away (`dx`, `dy`,
// the uv derivatives), all three through the same widened footprint (a mip
// coarser than the pixel's: finer, the relief's grain shimmered under the TAA
// jitter), so the result is a smooth function of the position. The screen derivatives of the shaded albedo it replaces
// stepped in 2x2 quads and moved with the TAA jitter. `alphaH` reads the
// height from alpha (the terrain's detail texture), else from luminance.
vec2 surfaceReliefDelta(sampler2D tex, vec2 uv, vec2 dx, vec2 dy, bool alphaH) {
    vec2 gx = dx * 2.0;
    vec2 gy = dy * 2.0;
    vec4 c0 = textureGrad(tex, uv, gx, gy);
    vec4 cx = textureGrad(tex, uv + dx, gx, gy);
    vec4 cy = textureGrad(tex, uv + dy, gx, gy);
    if (alphaH) return vec2(cx.a - c0.a, cy.a - c0.a);
    return vec2(dot(cx.rgb - c0.rgb, kLuma), dot(cy.rgb - c0.rgb, kLuma));
}

// Roughness, F0 and metalness of the per-draw surface class (surfClass.x,
// SurfaceLight.hpp SurfaceClass).
vec3 surfaceClassParams(float cls) {
    int i = int(cls + 0.5);
    if (i == 1) return vec3(0.5, 0.02, 0.0);    // paint (matt military)
    if (i == 2) return vec3(0.32, 0.04, 0.85);  // metal
    if (i == 3) return vec3(0.05, 0.04, 0.0);   // glass
    if (i == 4) return vec3(0.62, 0.035, 0.0);  // rubber
    if (i == 5) return vec3(0.88, 0.03, 0.0);   // fabric
    if (i == 6) return vec3(0.5, 0.028, 0.0);   // skin
    if (i == 7) return vec3(0.72, 0.04, 0.0);   // road
    if (i == 8) return vec3(0.82, 0.035, 0.0);  // masonry
    if (i == 9) return vec3(0.75, 0.035, 0.0);  // wood
    if (i == 10) return vec3(0.5, 0.035, 0.0);  // leather
    if (i == 11) return vec3(0.38, 0.04, 0.5);  // weapon
    if (i == 12) return vec3(0.06, 0.04, 0.0);  // lens
    if (i == 13) return vec3(0.68, 0.04, 0.0);  // roof
    return vec3(0.8, 0.035, 0.0);               // default
}

// Metres of relief per unit of albedo luminance for the per-draw surface class
// (masonry, roads and wood carry the most; glass and lenses none).
float surfaceClassRelief(float cls) {
    int i = int(cls + 0.5);
    if (i == 1 || i == 2 || i == 10) return 0.006; // paint, metal, leather
    if (i == 3 || i == 12) return 0.0;             // glass, lens
    if (i == 4) return 0.0;                        // rubber (a tread keeps its own grooves)
    if (i == 5) return 0.012;                      // fabric
    if (i == 6 || i == 11) return 0.003;           // skin, weapon
    if (i == 7 || i == 13) return 0.03;            // road, roof
    if (i == 8) return 0.035;                      // masonry
    if (i == 9) return 0.025;                      // wood
    return 0.02;
}

// How much of a texture's pits the cavity term darkens per surface class:
// stone, roads, roofs and wood carry the most; paint, metal, glass, rubber,
// skin and cloth none (their dark texels are colour, not pits).
float surfaceClassCavity(float cls) {
    int i = int(cls + 0.5);
    if (i == 7 || i == 8 || i == 13) return 1.0; // road, masonry, roof
    if (i == 9) return 0.8;                      // wood
    if (i == 0) return 0.5;                      // default
    return 0.0;
}

// The cavity term (the surfaces lane, litSurface3.x): the texture's height
// (2.5 pixels' footprint) below the mean of its neighbourhood (10 pixels'),
// from alpha (the terrain's detail texture) or luminance.
float g_surfCavity = 0.0;
const float kSurfCavityGain = 3.0;
const float kSurfBaseCavity = 0.8; // the land's square texture at a metre's scale, against the detail grain's 1
float surfaceCavity(sampler2D tex, vec2 uv, vec2 dx, vec2 dy, bool alphaH) {
    // A band of the texture's height: at 2.5 pixels' footprint (finer grain
    // shimmers under the TAA jitter once the cavity deepens it) against 10.
    vec4 c = textureGrad(tex, uv, dx * 2.5, dy * 2.5);
    vec4 m = textureGrad(tex, uv, dx * 10.0, dy * 10.0);
    return alphaH ? c.a - m.a : dot(c.rgb - m.rgb, kLuma);
}

// Karis' analytic split-sum environment BRDF: F0 * x + y.
vec2 surfaceEnvBrdf(float rough, float nv) {
    const vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    const vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
    vec4 r = rough * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * nv)) * r.x + r.y;
    return vec2(-1.04, 1.04) * a004 + r.zw;
}

// The local lights' streaks in the water film at this fragment (linear light,
// before the exposure): the lights of the tiled list (the physical lamps and
// low beams at their physical intensity, the other kinds at their classic
// value; the combat lights at the head of the list, B4, at their own linear
// light, which no night factor scales), or the draw's own list.
vec3 wetLampStreaks(vec3 n, vec3 v, float rough) {
    vec3 sum = vec3(0.0);
    vec3 combat = vec3(0.0);
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
            if (color.w > 1.5) {
                // A combat light (combatLights' model, without its shadow).
                float r2 = frameLights[at + 2].x * frameLights[at + 2].x;
                if (d2 >= r2) continue;
                float x = d2 / r2;
                float win = (1.0 - x * x) * (1.0 - x * x);
                combat += color.rgb * (win / (1.0 + d2 / max(pos.w * pos.w, 1e-4))) *
                          wetLobe(L, d, n, v, nv, a, kWetSpecCap);
                continue;
            }
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
        return sum * litLightCount.z + combat;
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
    // The land's classes come from the ground field (groundFieldAt), the same
    // continuous colour the reflections' trace reads, so the sheen, the
    // darkening and the puddles blend across a texture edge instead of jumping
    // at it; the albedo only while the field is not built yet.
    vec3 groundC = albedo;
    if (g_wetDraw == 1) {
        vec4 field = groundFieldAt(posAbs.xz);
        if (field.a > 0.5) groundC = field.rgb;
    }
    float veg = g_wetDraw == 2 ? 1.0 : (g_wetDraw == 1 ? wetVegetation(groundC) : 0.0);
    float porosity = g_wetDraw == 0 ? kWetPorosity[cls] : mix(wetGroundPorosity(groundC), kWetGrassPorosity, veg);
    float w = (ground ? mix(0.45, 1.0, skyVisibility) : wetCoverage(n, posAbs, skyVisibility, dist)) * wet;
    // Standing water on the land and on roads. The land pools as bare ground
    // or grass by its field, never by its square's texture name (the trace
    // never treats the land as a road either).
    vec2 pud = vec2(0.0);
    if (ground && g_wetDraw != 2) {
        float footT = max(length(fwidth(posAbs.xz)), 1e-3);
        pud = wetPuddle(posAbs, footT, g_wetDraw == 0 ? 1.0 : 0.0, veg, litWeather.z);
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
    // The film: what the surface does not soak up; grass breaks it up.
    float film = w * (1.0 - 0.75 * porosity) * (1.0 - veg) + kWetGrassFilm * w * veg;
    a *= (1.0 - (ground ? kWetGroundDarken : kWetDarken) * soak * w) * mix(1.0, kWetDampDark, pud.y * (1.0 - pud.x)) *
         mix(1.0, kWetPuddleDark, pud.x);
    // Outside its puddles the ground's film shows as a deeper colour, not as a
    // shine (the owner: only puddles reflect): asphalt and gravel, which soak
    // up little, darken by their film instead.
    if (ground) a *= 1.0 - (wetProbeIs(14) ? 0.0 : kWetGroundFilmDark) * film * (1.0 - pud.x);
    mirror = pud.x;
    g_probePud = pud;
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
               (film * (1.0 - pud.x) * (ground ? kWetSunGroundFilm * (wetProbeIs(13) ? 0.0 : 1.0) * (wetProbeIs(15) ? 0.5 : 1.0) : 1.0));
    }
    if (pud.x > 0.0) {
        float ap = min(kWetSunRough * kWetSunRough + sunFoot, 1.0);
        sun += wetLobe(litSunDir.xyz, 1e4, nf, v, nv, ap, kWetSunCap) * pud.x;
    }
    lit = litSun.rgb * (sun * sunLight * litHorizon.w);
    // The lamps' streaks: the film's lobe and the puddle's, each at its own
    // roughness and blended by the water's share (a lobe of the blended
    // roughness drew a bright ring at every puddle's edge). Only a puddle draws
    // a streak: the ground around it takes the broad, dim glint of its grit
    // (kWetSunGroundRough, kWetSunGroundFilm), as for the sun.
    if (litMatClass.z > 0.5) {
        if (pud.x < 1.0)
            lit += wetLampStreaks(nf, v, ground ? max(rough, kWetSunGroundRough) : rough) *
                   (film * (1.0 - pud.x) * (ground ? kWetSunGroundFilm * (wetProbeIs(13) ? 0.0 : 1.0) * (wetProbeIs(15) ? 0.5 : 1.0) : 1.0));
        if (pud.x > 0.0) lit += wetLampStreaks(nf, v, kWetPuddleRough) * pud.x;
    }
    // The sky in the film: objects always (weaker: rain beads on them); the
    // ground only while the reflections' pass does not reflect it (it then
    // reflects the ground with what is on screen, sky included), and there
    // only in its puddles. A face that looks down reflects the ground, which
    // its own light already shows.
    if (!ground || litMatClass.w < 0.5) {
        vec3 r = reflect(-v, nf);
        float up = smoothstep(-0.1, 0.1, r.y);
        r.y = max(r.y, 0.02);
        r *= inversesqrt(dot(r, r));
        vec3 sky = mix(toLinear(skyReflection(r)), toLinear(fogColorToward(r)), kWetSkyFog);
        vec2 ab = wetEnvBRDF(ground ? kWetPuddleRough : max(rough, kWetObjectSkyRough), nv);
        float F = (0.02 * ab.x + ab.y) * (ground ? pud.x : filmW) * up;
        F = ground ? F : min(F * kWetObjectSky, kWetSkyCap);
        env = sky * (F * mix(0.3, 1.0, skyVisibility) * litSunDir.w);
        keep = 1.0 - F;
    }
}

// What surfaceSpecular hands shadeRemastered.
struct SurfaceSpec {
    vec3 sun;      // the sun's highlight (rig units, before exposure)
    vec3 sky;      // the sky's reflection (rig units, before exposure)
    float diffuse; // what the diffuse keeps of the light (the energy the reflection took)
};

// GGX sun highlight and split-sum sky reflection for every surface but
// leaves. `a` is the linear albedo, `skyVis` the sky the receiver sees
// (valleys, canopy).
SurfaceSpec surfaceSpecular(vec3 n, vec3 v, vec3 a, float sunVisibility, bool terrain, bool foliage, float skyVis) {
    SurfaceSpec o;
    o.sun = vec3(0.0);
    o.sky = vec3(0.0);
    o.diffuse = 1.0;
    // Leaves reflect nothing here: a leaf card's plane mirrored the sky as a
    // pale sheet across the crown and took the green out of it, and the
    // crown's own wrap and canopy terms already carry how a crown looks.
    if (litSurface.y <= 0.0 || foliage) return o;
    vec3 p = terrain ? vec3(0.9, 0.03, 0.0) : surfaceClassParams(surfClass.x);
    // An authored glossy material keeps its own lobe width.
    if (litMatSpecular.w > 0.0 && !terrain) p.x = min(p.x, sqrt(2.0 / (litMatSpecular.w + 2.0)));
    float rough = clamp(p.x, 0.03, 1.0);
    float metal = p.z;
    // Darker texels (grime, seams, cavities) read rougher; not paint, whose
    // olive is dark all over and whose lobe would then spread into an even
    // grey sheen across the hull instead of a highlight where the sun's half
    // vector meets it.
    float lum = dot(a, kLuma);
    bool paint = !terrain && int(surfClass.x + 0.5) == 1;
    if (!paint) rough = clamp(rough + 0.15 * (1.0 - smoothstep(0.02, 0.2, lum)) * (1.0 - metal), 0.03, 1.0);
    vec3 f0 = mix(vec3(p.y), clamp(a * 2.2, 0.0, 1.0), metal);
    float nv = clamp(dot(n, v), 1e-4, 1.0);
    vec2 ab = surfaceEnvBrdf(rough, nv);
    vec3 fEnv = f0 * ab.x + ab.y;
    // The sun: GGX, Schlick-GGX visibility, Schlick Fresnel; times pi into
    // the rig's units (the rig's diffuse is albedo * irradiance).
    vec3 l = litSunDir.xyz;
    float nl = dot(n, l);
    if (nl > 0.0) {
        vec3 h = normalize(l + v);
        float nh = max(dot(n, h), 0.0);
        float vh = max(dot(v, h), 0.0);
        float al = rough * rough;
        float a2 = al * al;
        float dd = nh * nh * (a2 - 1.0) + 1.0;
        float D = a2 / (kSurfPi * dd * dd);
        float k = al * 0.5;
        float vis = 0.25 / ((nl * (1.0 - k) + k) * (nv * (1.0 - k) + k));
        vec3 F = f0 + (1.0 - f0) * pow(1.0 - vh, 5.0);
        // Rough natural surfaces (soil, grass, cloth, plaster, brick) shadow
        // their own microfacets: GGX's broad lobe on them reads as an even
        // grey sheen that takes the warmth out of a sunlit facade and lifts
        // its dark windows (the materials track found the grazing glare), so
        // it keeps 0.15 of itself from roughness 0.9 down from 0.55.
        float roughCut = mix(1.0, 0.15, smoothstep(0.55, 0.9, rough) * (1.0 - metal));
        o.sun = litSun.rgb * (D * vis * nl * kSurfPi * sunVisibility * roughCut) * F;
    }
    // The sky: a smooth surface mirrors the sky as the player sees it, a rough
    // one the sky light around its reflected direction (the irradiance there);
    // both lose what the receiver's sky visibility hides.
    vec3 r = reflect(-v, n);
    vec3 rd = normalize(mix(r, n, rough * rough));
    vec3 rough_l = surfaceSkyIrradiance(rd, false) * 1.6;
    vec3 pref = rough_l;
    // The sky as seen only where the lobe is narrow (glass, lenses, metal,
    // wet film): the table lookup is the costly part, and most surfaces skip it.
    float smoothW = 1.0 - smoothstep(0.08, 0.45, rough);
    if (smoothW > 0.0) {
        vec3 smooth_l = toLinear(skyReflection(normalize(vec3(r.x, max(r.y, 0.02), r.z)))) / max(litZenith.w, 1e-3);
        smooth_l = mix(smooth_l, rough_l, 1.0 - smoothstep(-0.15, 0.0, r.y));
        pref = mix(rough_l, smooth_l, smoothW);
    }
    // A rough dielectric's lobe takes in the dim ground and its own surface's
    // relief as much as the sky, and the sky fill already carries its
    // diffuse-looking part: none of the sky reflection from roughness 0.5 on
    // (matt paint, rubber, cloth, soil, plaster) seen face on, so they keep
    // their colour instead of a grey-blue veil (a dark window or a shaded
    // facade lifted toward the sky's colour). Grazing views catch some at an
    // edge (a hull's silhouette, a wall seen along its face), up to 0.3 (paint
    // 0.2); the land none (the haze carries the far ground's sheen). The
    // reflection loses what the receiver's occlusion hides: skyVis here, the
    // post pass's horizon occlusion through the share it is part of.
    float matt = smoothstep(0.25, 0.5, rough) * (1.0 - metal);
    float roughSky = mix(1.0, 0.0, matt);
    int cls = terrain ? 0 : int(surfClass.x + 0.5);
    float edgeSky = terrain ? roughSky : mix(1.0, paint ? 0.2 : 0.3, matt);
    roughSky = mix(roughSky, edgeSky, pow(1.0 - nv, 5.0));
    // Rubber reflects no sky (a tyre's tread keeps its grooves); a blending
    // texture that is not glass (a canvas window, a net, a sign) none either:
    // its sheen made a see-through panel a flat pale card.
    bool blendTex = !terrain && surfClass.z > 0.5;
    if (cls == 4 || (blendTex && cls != 3 && cls != 12)) roughSky = 0.0;
    o.sky = pref * fEnv * (skyVis * roughSky);
    // The diffuse gives up only the energy the reflection actually shows.
    o.diffuse = (1.0 - dot(fEnv, kLuma) * roughSky) * (1.0 - metal);
    float s = litSurface.y * litHorizon.w;
    o.sun *= s;
    o.sky *= s;
    return o;
}
)";

} // namespace Poseidon::gl33lighting
