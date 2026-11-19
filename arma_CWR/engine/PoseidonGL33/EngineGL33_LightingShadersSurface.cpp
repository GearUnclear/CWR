// Remastered lighting shaders: the surface pieces of the fragment prelude
// (FragmentPiece::Puddles and FragmentPiece::Surface, joined by
// RemasterFragmentCommon() in EngineGL33_LightingShaders.cpp): where puddles
// stand, the sun highlight and sky reflection of glossy materials, and the wet
// weather's response of every surface. Material and wet-surface responses
// belong here.
//
// Wet weather (feature switches wetWeather and lampStreaks;
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
// Surfaces and ambient light (Graphics/Remaster/SurfaceLight.hpp): the sky light
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
// (a straight seam in the sheen and the puddles).
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
// Only the puddles mirror (the wet ground around them does not), so they carry
// the rain's look: roads and bare ground hold this much more water than the reflections'
// thresholds above (lowered by it), grass no more.
const float kPuddleMore = 0.04;
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
    // The far fade first: where it is 0 (from kPuddleFadeEnd on) the result is
    // 0 whatever the noise and the height map say, so they are not read.
    float fade = 1.0 - smoothstep(kPuddleFadeStart, kPuddleFadeEnd, length(posW - litCamPos.xyz));
    if (fade <= 0.0) return vec2(0.0);
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
    float th = mix(mix(kPuddleGround, kPuddleRoad, road) - kPuddleMore, kPuddleGrass, veg * veg) + kPuddleLevelShift * (1.0 - level);
    float keep = flatness * min(level * 4.0, 1.0) * fade;
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
const float kWetGroundDarken = 0.35; // ... the ground this (the reflections' wet ground kept 0.8 of its colour)
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

// ---- Scorched ground (the fire spread, POSEIDON_XP_FIRESPREAD) ----
// The fire's burn map (World/Simulation/FireSpreadWorld.hpp GetBurnMap,
// litBurnRect): a world-space map, 0 untouched, 1 burnt black, a cell still
// flaming part way (about 0.4 to 0.9), in the first rows of an atlas whose
// rows after them hold the land's height on a coarser grid over the same
// window (litBurnGround). Only the land, grass and object mains declare its
// sampler (burnMap, unit 17) and pass it in, and only while litBurnRect.w is
// 1; every other program never calls these.
const vec3 kBurnAsh = vec3(0.026, 0.022, 0.02);    // charred ground (linear albedo, the mean)
const float kBurnTone = 0.3;                       // the ash's tone wanders by this share over 2 to 10 m ...
const float kBurnMottle = 0.12;                    // ... and a soft mottling of under a metre by this share ...
const float kBurnMottleFar = 30.0;                 // ... fading out by here (m: far mottling would shimmer)
const float kBurnScorch = 0.55;                    // a still flaming cell's char keeps this of the ash colour
const float kBurnWarp = 2.0;                       // the edge's raggedness: the map read up to this far off (m)
const float kBurnSpecKeep = 0.2;                   // charred ground keeps this share of its specular
const float kBurnGrassGone = 0.5;                  // grass clutter is gone above this burn ...
const float kBurnGrassSinge = 0.85;                // ... and singes toward the ash below it, by up to this share
// The specular the shaded fragment keeps (shadeRemastered): 1 off the burn.
float g_burnSpec = 1.0;

// The burn map's texel at map coordinates uv (0..1 over the window): the
// atlas's burn rows, held off the height rows after them (no bilinear tap
// blends a height in). 0 outside the window.
float burnTexel(sampler2D map, vec2 uv) {
    if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) return 0.0;
    uv.y = min(uv.y * litBurnGround.x, litBurnGround.x - 0.5 * litBurnGround.w);
    return textureLod(map, uv, 0.0).r;
}

// The burn (0..1) at world xz: the map read through a two-octave warp of
// about 3 m and 1 m cells, so its 4 m texels show as a ragged edge instead of
// blocks; 0 outside the map. No derivatives: the map has one level.
float burnAt(sampler2D map, vec2 xz) {
    vec2 w = vec2(puddleNoise(xz * 0.35), puddleNoise(xz * 0.35 + vec2(41.3, 17.9))) - 0.5;
    w += (vec2(puddleNoise(xz + vec2(7.1, 63.7)), puddleNoise(xz + vec2(91.5, 29.3))) - 0.5) * 0.5;
    return burnTexel(map, (xz + w * (kBurnWarp / 0.75) - litBurnRect.xy) * litBurnRect.z);
}

// ---- Charred objects (the object main, while litBurnRect.w is 1) ----
// Each object draw carries its char class in surfClass.w (EngineGL33::
// ApplySurfaceClass): 2 a tree, a bush or a forest block, 1 any other static
// object (a house, a wall, a rock, a fence), 0 never charred (men, vehicles,
// weapons, decals on the ground). Leaves are the draws with a forced diffuse.
const float kBurnObjectReach = 4.5;      // an object reads the burn this far round its foot (m) ...
const float kBurnObjectRing = 0.85;      // ... at this share: a wall beside burnt ground catches it
const vec3 kBurnSoot = vec3(0.022, 0.019, 0.017);     // soot on walls and rocks (linear albedo, the mean)
const vec3 kBurnCharcoal = vec3(0.018, 0.015, 0.013); // a burnt trunk and its branches (linear albedo)
const float kBurnObjectTone = 0.35;      // the soot's and the char's tone wander by this share
const float kBurnObjectGrain = 0.35;     // and keep this share of the texture's own light and dark
const float kBurnSootTop = 3.0;          // soot rises this high above the ground (m) on burnt-out ground ...
const float kBurnSootTopFlaming = 0.55;  // ... this share of it beside a cell still flaming
const float kBurnSootRagged = 0.45;      // its top wanders by this share up and down along the wall ...
const float kBurnSootStreak = 1.6;       // ... and vertical streaks reach up to this much higher (m)
const float kBurnSootFade = 0.55;        // the soot fades out over the top this share of its height
const float kBurnSootMax = 0.92;         // the soot covers at most this much of the wall's colour
const float kBurnTrunkScorch = 2.5;      // a tree's trunk is scorched this high while its cell flames (m) ...
const float kBurnTrunkAll = 60.0;        // ... and charred to this height, the whole tree, once burnt out
const float kBurnTrunkAllFrom = 0.85;    // burnt out: from this burn to 1
const float kBurnCharMax = 0.97;         // the char covers at most this much of the bark's colour
const float kBurnObjectSpec = 0.08;      // charred and sooted surfaces keep this share of their specular
const float kBurnDetailFar = 120.0;      // streaks and fine raggedness fade out by here (m)
const vec3 kBurnLeafBrown = vec3(0.10, 0.05, 0.02); // scorched leaves (linear albedo) ...
const float kBurnLeafScorch = 0.12;      // ... from this burn ...
const float kBurnLeafGone = 0.86;        // ... and gone above this one (a burnt-out cell is 1)
const float kBurnLeafGoneJitter = 0.04;  // the threshold wanders by this much from clump to clump

// The burn an object reads at world xz: the map at the fragment and on a
// ring kBurnObjectReach round it (kBurnObjectRing of the strongest), so a
// wall or a trunk standing on the edge of burnt ground catches it. No warp:
// the char's own noise breaks its edges.
float burnObjectAt(sampler2D map, vec2 xz) {
    vec2 uv = (xz - litBurnRect.xy) * litBurnRect.z;
    float r = kBurnObjectReach * litBurnRect.z;
    float ring = max(max(burnTexel(map, uv + vec2(r, 0.0)), burnTexel(map, uv - vec2(r, 0.0))),
                     max(burnTexel(map, uv + vec2(0.0, r)), burnTexel(map, uv - vec2(0.0, r))));
    return max(burnTexel(map, uv), kBurnObjectRing * ring);
}

// The land's height (m) at world xz from the atlas's height grid, bilinear
// between its samples (each at its square's centre); `fallback` without one.
float burnGroundY(sampler2D map, vec2 xz, float fallback) {
    float n = litBurnGround.y;
    if (n < 0.5) return fallback;
    vec2 g = clamp((xz - litBurnRect.xy) * litBurnRect.z * n, vec2(0.5), vec2(n - 0.5));
    return textureLod(map, vec2(g.x * litBurnGround.z, litBurnGround.x + g.y * litBurnGround.w), 0.0).r;
}

// A leaf's fate in burn `b` at world position p: true when it has burnt away
// (whole clumps of about a metre go together, at a threshold that wanders
// round kBurnLeafGone, so a crown thins out instead of vanishing at once).
bool burnLeafGone(float b, vec3 p) {
    float n = puddleNoise(vec2(p.x + 0.6 * p.y, p.z - 0.4 * p.y) * 0.9 + vec2(11.3, 47.9));
    return b > kBurnLeafGone + (n - 0.5) * 2.0 * kBurnLeafGoneJitter;
}

// Scorched leaves (display-space albedo): toward a dry brown, then toward
// black as the cell burns out, keeping the texture's light and dark.
vec3 burnLeaves(vec3 albedo, float b, vec3 p) {
    float scorch = smoothstep(kBurnLeafScorch, 0.6, b);
    if (scorch <= 0.0) return albedo;
    float n = puddleNoise(vec2(p.x - 0.5 * p.y, p.z + 0.3 * p.y) * 0.7 + vec2(3.7, 29.1));
    float grain = clamp(dot(toLinear(albedo), kLuma) / 0.06, 0.5, 1.6);
    vec3 dry = mix(kBurnLeafBrown, kBurnCharcoal, smoothstep(0.6, kBurnLeafGone, b)) * (0.7 + 0.6 * n) * grain;
    g_burnSpec = min(g_burnSpec, mix(1.0, kBurnObjectSpec, scorch));
    return mix(albedo, toDisplay(dry), scorch);
}

// A charred object's display-space albedo in burn `b` at world position p
// with normal n: soot rising from the ground on walls and rocks (strongest at
// the foot, gone by about kBurnSootTop up, its top ragged along the wall and
// broken by vertical streaks, never a straight line); on a tree's trunk and
// branches (vegetation) a scorch a few metres high while it flames, the
// whole tree once it has burnt out. Sets g_burnSpec (soot is matt).
vec3 burnObject(sampler2D map, vec3 albedo, float b, vec3 p, vec3 n, bool vegetation) {
    float above = p.y - burnGroundY(map, p.xz, p.y - 1.0);
    // Along the face, horizontally (a roof or a floor takes x, a little z).
    vec2 t = vec2(-n.z, n.x) + vec2(1e-3, 3.7e-4);
    float s = dot(p.xz, t * inversesqrt(dot(t, t)));
    float near = 1.0 - smoothstep(0.5 * kBurnDetailFar, kBurnDetailFar, length(vWorldRel));
    float ragged = 0.65 * puddleNoise(vec2(s * 0.6, 3.1)) + 0.35 * mix(0.5, puddleNoise(vec2(s * 1.9, 17.3)), near);
    float top;
    if (vegetation) {
        top = mix(kBurnTrunkScorch, kBurnTrunkAll, smoothstep(kBurnTrunkAllFrom, 1.0, b)) *
              (1.0 + kBurnSootRagged * 2.0 * (ragged - 0.5));
    } else {
        float streak = puddleNoise(vec2(s * 3.5, p.y * 0.22 + 7.7)) * near;
        top = kBurnSootTop * mix(kBurnSootTopFlaming, 1.0, smoothstep(0.4, 0.95, b)) *
                  (1.0 + kBurnSootRagged * 2.0 * (ragged - 0.5)) +
              kBurnSootStreak * smoothstep(0.55, 0.85, streak);
    }
    float cover = smoothstep(0.03, 0.4, b) * (1.0 - smoothstep((1.0 - kBurnSootFade) * top, top, above));
    if (cover <= 0.0) return albedo;
    float tone = 0.6 * puddleNoise(vec2(s, p.y) * 1.1 + vec2(5.1, 0.0)) + 0.4 * mix(0.5, puddleNoise(vec2(s, p.y) * 3.7), near);
    float grain = mix(1.0, clamp(dot(toLinear(albedo), kLuma) / 0.2, 0.4, 1.8), kBurnObjectGrain);
    vec3 soot = (vegetation ? kBurnCharcoal : kBurnSoot) * (1.0 + kBurnObjectTone * 2.0 * (tone - 0.5)) * grain;
    g_burnSpec = min(g_burnSpec, mix(1.0, kBurnObjectSpec, cover));
    return mix(albedo, toDisplay(soot), cover * (vegetation ? kBurnCharMax : kBurnSootMax));
}

// The land's display-space albedo over the burn `b` at world xz: black-brown
// ash whose tone wanders softly over a few metres, mottled softly below a
// metre where it has burnt out, darker still where the cell is flaming. Only
// smooth noise (puddleNoise), so no hard steps anywhere. Sets g_burnSpec (the
// ash is matt).
vec3 burnGround(vec3 albedo, float b, vec2 xz) {
    float cover = smoothstep(0.03, 0.45, b);
    float flaming = smoothstep(0.3, 0.45, b) * (1.0 - smoothstep(0.85, 0.95, b));
    // Tone: cells of about 8 m and 2.5 m, 0..1.
    float tone = 0.65 * puddleNoise(xz * 0.12 + vec2(13.7, 5.1)) + 0.35 * puddleNoise(xz * 0.4 + vec2(3.3, 57.1));
    // Mottling: cells of about 0.6 m and 0.25 m, 0..1, on burnt-out ground near the eye.
    float mottle = 0.6 * puddleNoise(xz * 1.7 + vec2(29.9, 81.3)) + 0.4 * puddleNoise(xz * 4.1 + vec2(67.1, 7.7));
    float mottleW = smoothstep(0.85, 0.95, b) * (1.0 - smoothstep(0.5 * kBurnMottleFar, kBurnMottleFar, length(vWorldRel)));
    vec3 ash = kBurnAsh * (1.0 + kBurnTone * 2.0 * (tone - 0.5)) * (1.0 + kBurnMottle * 2.0 * (mottle - 0.5) * mottleW);
    ash *= mix(1.0, kBurnScorch, flaming);
    g_burnSpec = mix(1.0, kBurnSpecKeep, cover);
    return mix(albedo, toDisplay(ash), cover);
}

// Grass clutter's display-space albedo at the fire's edge (b at most
// kBurnGrassGone; above it the main discards the blade): singed toward the ash.
vec3 burnGrass(vec3 albedo, float b) {
    float singe = smoothstep(0.1, kBurnGrassGone, b) * kBurnGrassSinge;
    return mix(albedo, toDisplay(kBurnAsh * 1.5), singe);
}

// ---- Firelight (the land, grass and object mains, while litBurnRect.w is 1) ----
// From the atlas's two heat grids (EngineGL33_Lighting.cpp BuildFireHeat, at
// the simulation's cell, rebuilt every frame from the flames as drawn): the
// ember grid, each cell's ash glow, and the halo grid, the light each cell
// takes from the flames round it, already flickered and cut by the land
// between (no light through a hill), both on the rig's scale of a lux (the
// fire's light by day and by night: CombatLights.hpp kWildfireRigPerLux and
// WildfireLightGain), so both are scene light, lit and exposed with the frame
// (shadeRemastered adds the halo to the combat light and the glow to the
// radiance before the exposure).
const vec3 kFireLight = vec3(3.049, 0.488, 0.0366); // the flames' light: CombatLights.hpp kWildfireRgb at luminance 1
const float kFireLightSide = 0.35;                // a vertical face takes this share of it, a face turned down none
const float kFireLightHeight = 6.0;               // objects: it fades out by this height above the ground (m)
const vec3 kEmberColour = vec3(1.0, 0.22, 0.035); // embers in a flaming cell's ash (linear) ...
const float kEmberNits = 12.0;                    // ... their luminance on their hot veins (cd/m2) at full glow
const float kEmberVeins = 0.08;                   // the share of the ash that glows (a noise threshold) ...
const float kEmberBase = 0.0;                     // ... and the dull glow between the veins, as a share of theirs
const float kEmberCrawl = 3.0;                    // the veins wander this far (noise cells) and back, slowly
const float kEmberFar = 80.0;                     // their fine mottling fades out by here (m): far, an even glow
// What the fire adds to this fragment (shadeRemastered): the light it takes
// (irradiance on the rig's scale), and the ash's own glow (scene light).
vec3 g_fireLight = vec3(0.0);
vec3 g_fireEmit = vec3(0.0);

// One heat grid at world xz (0 the embers, 1 the halo); 0 outside the window
// or without the grids.
float burnHeatAt(sampler2D map, vec2 xz, float grid) {
    float n = litBurnHeat.x;
    if (n < 0.5) return 0.0;
    vec2 uv = (xz - litBurnRect.xy) * litBurnRect.z;
    if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) return 0.0;
    vec2 g = clamp(uv * n, vec2(0.5), vec2(n - 0.5));
    return textureLod(map, vec2((g.x + grid * n) * litBurnGround.z, litBurnHeat.y + g.y * litBurnGround.w), 0.0).r;
}

// The firelight on a fragment at world xz whose normal's y is `up`, times
// `lift` (1 on the ground): sets g_fireLight. Up-facing surfaces take it all.
void burnFireLight(sampler2D map, vec2 xz, float up, float lift) {
    float halo = burnHeatAt(map, xz, 1.0);
    if (halo <= 0.0) return;
    float facing = up >= 0.0 ? mix(kFireLightSide, 1.0, up) : kFireLightSide * (1.0 + up);
    g_fireLight = kFireLight * (halo * facing * lift);
}

// Embers smouldering in a flaming cell's ash (the land): veins of glow that
// crawl and pulse slowly, brightest just behind the flames, gone as the ash
// cools. Sets g_fireEmit: a luminance of kEmberNits on the rig's scale (as a
// white surface, whose materials carry 1.6, lit by pi L lux), so by day it
// is lost in the sun.
void burnEmbers(sampler2D map, vec2 xz) {
    float heat = burnHeatAt(map, xz, 0.0);
    if (heat <= 0.0) return;
    float t = litBurnHeat.z;
    float near = 1.0 - smoothstep(0.5 * kEmberFar, kEmberFar, length(vWorldRel));
    vec2 crawl = vec2(sin(t * 0.05), cos(t * 0.04)) * kEmberCrawl;
    float n = 0.6 * puddleNoise(xz * 0.8 + crawl) +
              0.4 * mix(0.5, puddleNoise(xz * 2.6 - crawl * 1.7 + vec2(19.1, 7.3)), near);
    float veins = mix(kEmberVeins, smoothstep(1.0 - kEmberVeins, 1.2 - kEmberVeins, n), near);
    float pulse = 0.8 + 0.2 * sin(t * 1.7 + n * 6.2832);
    g_fireEmit = kEmberColour * (kEmberNits * 3.14159265 * 1.6 * heat * mix(kEmberBase, 1.0, veins) * pulse);
}

// ---- A burning wreck (followups; the object main, while litBurnRect.w is 1) ----
// A vehicle the fire burns, or a proxy of one (FireSpreadWorld.hpp
// WreckBurnClass: surfClass.w 3 and up, its char in 128 steps, its shell's
// glow in 64 and that glow's light on the scene's scale, log-packed, after
// the 3). The consult's tenth answer: a car's paint goes "to grey and
// rust-brown, glass gone, tyres to wire"; its thin bodywork glows dull red by
// night while it burns, its shell hot for hours after. Its paint first
// blisters black where its flames stand (low first: its fuel burns under it),
// then burns clean to a grey ash mottled with rust-brown, a share of it left
// black; its glass breaks out early, pane by pane; its canvas and its
// camouflage branches burn away in ragged holes, black at their edges; its
// rubber chars black, then greys with ash. By night its hot shell glows dull
// red in patches (g_fireEmit, scene light on the ground's embers' scale),
// over most of its lower half while it flames, shrinking to embers low in its
// floor and wheels as it cools.
const vec3 kWreckSoot = vec3(0.018, 0.016, 0.014);      // blistered, sooted paint (linear albedo)
const vec3 kWreckAsh = vec3(0.13, 0.125, 0.115);        // paint burnt clean to its grey ash ...
const vec3 kWreckRust = vec3(0.15, 0.055, 0.022);       // ... and the heat-scaled steel, rusting
const vec3 kWreckRubberAsh = vec3(0.045, 0.042, 0.04);  // burnt rubber, greyed with ash
const float kWreckBlackKeep = 0.3;   // the share of its burnt shell left black (streaks, under its edges)
const float kWreckRustShare = 0.45;  // the share of its clean-burnt metal gone rust-brown
const float kWreckLowM = 2.5;        // its lower parts char first and glow longest, up to this high (m)
const float kWreckThrMax = 0.6;      // the char its last spots need to blacken (its top, here and there)
const vec3 kWreckGlowHot = vec3(1.0, 0.18, 0.03);       // its glow, hot ...
const vec3 kWreckGlowCool = vec3(1.0, 0.06, 0.008);     // ... and cooling (linear)
const float kWreckGlowLogMin = -7.0, kWreckGlowLogSpan = 8.0; // FireSpreadWorld.hpp, its glow's light's packing

// A wreck's look at world position p (normal n; originY its origin's
// height; leaves: a draw with a forced diffuse, camouflage branches): its
// display-space albedo burnt by its char, g_burnSpec (char is matt) and
// g_fireEmit (its glow). False when the fragment has burnt away (glass,
// canvas, branches).
bool burnWreck(sampler2D map, float code, float cls, inout vec3 albedo, vec3 p, vec3 n, float originY, bool leaves) {
    float f = floor((code - 3.0) * 2097152.0 + 0.5);
    float cq = floor(f / 16384.0);
    float rest = f - cq * 16384.0;
    float gq = floor(rest / 256.0);
    float eq = rest - gq * 256.0;
    float charK = cq / 127.0;
    float glow = gq / 63.0;
    float light = eq > 0.5 ? pow(10.0, kWreckGlowLogMin + (eq - 1.0) / 254.0 * kWreckGlowLogSpan) : 0.0;
    int k = int(cls + 0.5);
    float near = 1.0 - smoothstep(0.5 * kBurnDetailFar, kBurnDetailFar, length(vWorldRel));
    vec2 uv = (p.xz - litBurnRect.xy) * litBurnRect.z;
    bool inMap = all(greaterThanEqual(uv, vec2(0.0))) && all(lessThanEqual(uv, vec2(1.0)));
    float above = p.y - (inMap ? burnGroundY(map, p.xz, originY - 1.0) : originY - 1.0);
    float low = 1.0 - smoothstep(0.2, kWreckLowM, above);
    // patches of about a metre and a quarter metre over its body, different
    // on each face (the normal shifts them)
    vec3 q = p + n * 0.37;
    float n1 = puddleNoise(vec2(q.x + 0.61 * q.y, q.z - 0.47 * q.y) * 1.1 + vec2(13.1, 7.7));
    float n2 = mix(0.5, puddleNoise(vec2(q.z - 0.53 * q.y, q.x + 0.39 * q.y) * 3.7 + vec2(3.9, 41.7)), near);
    float n3 = puddleNoise(vec2(q.x - 0.29 * q.y, q.z + 0.71 * q.y) * 0.8 + vec2(57.3, 21.9));
    float pat = 0.65 * n1 + 0.35 * n2;
    // the char this spot needs to blacken: low and patchy first
    float thr = clamp((0.05 + 0.5 * (1.0 - low)) * (0.6 + 0.8 * pat), 0.0, kWreckThrMax);
    float soot = smoothstep(thr, thr + 0.15, charK);
    float clean = smoothstep(thr + 0.25, thr + 0.55, charK) *
                  smoothstep(kWreckBlackKeep - 0.08, kWreckBlackKeep + 0.08, n3);
    if (k == 3 || k == 12) {
        // glass and lenses crack and fall out early, pane by pane
        if (charK > 0.06 + 0.2 * n1 + 0.12 * (1.0 - low)) return false;
    } else if (k == 5 || leaves) {
        // canvas and camouflage branches burn away in ragged holes, black at
        // their edges (branches first: dry leaves go in the first minutes)
        if (charK > (leaves ? 0.5 * thr + 0.04 + 0.12 * n2 : thr + 0.1 + 0.2 * n2)) return false;
        clean = 0.0;
    }
    vec3 base = toLinear(albedo);
    float grain = mix(1.0, clamp(dot(base, kLuma) / 0.15, 0.5, 1.6), 0.35);
    vec3 c = mix(base, kWreckSoot * (0.8 + 0.4 * n2), soot);
    if (k == 4) {
        c = mix(c, kWreckRubberAsh * (0.8 + 0.4 * n2), clean);
    } else if (k != 5 && k != 9 && k != 10 && !leaves) {
        float rust = 1.0 - smoothstep(kWreckRustShare - 0.12, kWreckRustShare + 0.12, n2 * 0.5 + n1 * 0.5);
        vec3 burnt = mix(kWreckAsh, kWreckRust, rust) * (0.75 + 0.5 * n2) * grain;
        c = mix(c, burnt, clean);
    }
    g_burnSpec = min(g_burnSpec, mix(1.0, kBurnObjectSpec, max(soot, clean)));
    albedo = toDisplay(c);
    // its hot shell's glow: over much of its lower half while it flames, in
    // fewer patches, low in its floor and wheels, as it cools; only where it
    // has burnt
    if (light > 0.0 && k != 3 && k != 12 && k != 5 && !leaves) {
        float t = litBurnHeat.z;
        vec2 crawl = vec2(sin(t * 0.05), cos(t * 0.04)) * 0.6;
        // patches of about half a metre and a sixth, so a panel glows in
        // places, never whole
        float ng = 0.6 * puddleNoise(vec2(q.x - 0.3 * q.y, q.z + 0.5 * q.y) * 2.2 + crawl + vec2(71.3, 9.1)) +
                   0.4 * mix(0.5, puddleNoise(vec2(q.z + 0.2 * q.y, q.x - 0.6 * q.y) * 6.0 - crawl + vec2(5.3, 88.1)), near);
        // (a noise threshold: about 0.35 over its low parts in its flames,
        // most of them glowing; about 0.5 just after them, half; 0.7 at the
        // end, a few specks; its tyres' wire and its rubber's ash at half)
        float thrG = 0.78 - 0.45 * sqrt(glow) * (0.45 + 0.55 * low);
        float m = smoothstep(thrG, thrG + 0.18, ng) * smoothstep(0.3, 0.7, soot) * (k == 4 ? 0.5 : 1.0);
        float pulse = 0.85 + 0.15 * sin(t * 1.3 + ng * 6.2832);
        g_fireEmit = mix(kWreckGlowCool, kWreckGlowHot, glow) * (light * m * pulse);
    }
    return true;
}

// ---- Plants' own fire (the object main, while litBurnRect.w is 1) ----
// The fire spread's plant table (FireSpreadWorld.hpp PlantTable) in the burn
// atlas's last rows, from row litBurnHeat.w on (0: none): every tree, bush,
// reed clump and olive the fire came near, found by its foot (the drawn
// object's origin), with what happened to it, its times counted from when the
// drawn flames reached it (its contact, on the fire clock litBurnHeat.z
// wrapped at kPlantClockWrap). Its leaves scorch, burn away and char, and its
// bark chars, from that alone, never from the cells' burn map: every edge is
// a smooth noise contour in the world, and everything moves on with the clock.
const int kPlantSlotFloats = 32;           // FireSpreadWorld.hpp kPlantSlotFloats
const int kPlantProbes = 16;               // ... kPlantProbes
const float kPlantKeyNear = 0.06;          // ... kPlantKeyNearM
const float kPlantClockWrap = 628.31853;   // ... kPlantClockWrap
const vec3 kPlantEmber = vec3(1.0, 0.3, 0.05); // a burning leaf's glow (display-linear) ...
const float kPlantEmberGlow = 0.35;           // ... at this light on the burning sweep's edge
const float kPlantStrandPx = 0.8;             // a kept stem is at least this many pixels' noise wide

// The plant whose foot is at world xz: its slot's first texel in the atlas,
// x -1 without one. The bucket and the probing are PlantTable::Find's.
ivec2 burnPlantAt(sampler2D map, vec2 xz) {
    if (litBurnHeat.w < 0.5) return ivec2(-1);
    int row0 = int(litBurnHeat.w + 0.5);
    int perRow = int(1.0 / litBurnGround.z + 0.5) / kPlantSlotFloats;
    int slots = (int(1.0 / litBurnGround.w + 0.5) - row0) * perRow;
    if (perRow <= 0 || slots <= 0) return ivec2(-1);
    ivec2 b = ivec2(floor(xz));
    uint mask = uint(slots - 1);
    uint s = uint(b.x) * 73856093u ^ uint(b.y) * 19349663u;
    s ^= s >> 16;
    s *= 0x85ebca6bu;
    s ^= s >> 13;
    s *= 0xc2b2ae35u;
    s ^= s >> 16;
    s &= mask;
    for (int i = 0; i < kPlantProbes; ++i) {
        ivec2 at = ivec2((int(s) % perRow) * kPlantSlotFloats, row0 + int(s) / perRow);
        float kx = texelFetch(map, at, 0).r;
        if (kx < -1.0e8) break;
        if (abs(kx - xz.x) < kPlantKeyNear && abs(texelFetch(map, at + ivec2(1, 0), 0).r - xz.y) < kPlantKeyNear)
            return at;
        s = (s + 1u) & mask;
    }
    return ivec2(-1);
}

// One of a plant's fields (FireSpreadWorld.hpp PlantSlotField).
float plantField(sampler2D map, ivec2 at, int k) {
    return texelFetch(map, at + ivec2(k, 0), 0).r;
}

// Seconds since a plant's contact: -1 before it, very long once its look is final.
float plantSince(float contact) {
    if (contact > 1.0e8) return -1.0;
    if (contact < -0.5) return 1.0e6;
    return mod(litBurnHeat.z - contact, kPlantClockWrap);
}

// A plant's leaf fragment at world p (origin: its foot) in its own fire: the
// display-space albedo scorched brown from the ground up to a soft ragged top
// (rising from the flames' height to its own, higher on the lee side), and,
// while it flares or torches, burnt away clump by clump in a ragged sweep
// from its windward base to its lee top (its scorched leaves charring
// toward its scorchChar over the sweep's first third; each going clump brown,
// then black and glowing on the sweep's edge, then gone: false, discard), the
// clumps it keeps charred toward its keptChar (thin strands, its stems, for a
// thicket or a reed clump; pixM, the fragment's footprint in metres, keeps
// them about a pixel wide at least). Sets g_burnSpec and g_fireEmit.
bool plantLeaves(sampler2D map, ivec2 at, vec3 p, vec3 origin, float pixM, inout vec3 albedo) {
    float since = plantSince(plantField(map, at, 4));
    if (since < 0.0) return true;
    float h = max(plantField(map, at, 3), 0.3);
    float above = p.y - plantField(map, at, 2);
    vec2 dir = vec2(plantField(map, at, 5), plantField(map, at, 6));
    // -1 on the side the fire came from, 1 on its lee
    float side = clamp(dot(p.xz - origin.xz, dir) / max(0.6 * h, 0.5), -1.0, 1.0);
    float top = plantField(map, at, 7);
    float rise = max(plantField(map, at, 8), 0.1);
    float ragged = puddleNoise(vec2(p.x + 0.5 * p.y, p.z - 0.3 * p.y) * 0.6 + vec2(17.3, 5.9));
    float topNow = top * mix(0.35, 1.0, smoothstep(0.0, rise, since)) * (1.0 + 0.2 * side) * (0.8 + 0.4 * ragged);
    float brown = (1.0 - smoothstep(topNow - 0.5, topNow + 0.3, above)) * smoothstep(0.0, 1.5, since);
    float black = 0.0;
    float b0 = plantField(map, at, 11), b1 = plantField(map, at, 12);
    if (b1 > b0) {
        // In its flames its scorched leaves char (a dark lattice) over the
        // sweep's first third.
        float charring = plantField(map, at, 20) * smoothstep(b0, mix(b0, b1, 0.3), since) * brown;
        float sweep = mix(-0.35, plantField(map, at, 13) + 0.05, clamp((since - b0) / (b1 - b0), 0.0, 1.0));
        float u = clamp(above / h, 0.0, 1.0);
        float order = 0.6 * (0.7 * u + 0.3 * (0.5 + 0.5 * side)) +
                      0.4 * puddleNoise(vec2(p.x - 0.4 * p.y, p.z + 0.7 * p.y) * 0.8 + vec2(3.1, 61.7));
        float f = sweep - order; // over 0: the sweep has passed
        float clump = max(plantField(map, at, 18), 0.2);
        float nk = puddleNoise(vec2(p.x + 0.6 * p.y, p.z - 0.4 * p.y) / clump + vec2(11.3, 47.9));
        // What it keeps: its highest clumps, or (a skeleton of stems) a band
        // round the noise's median, never under kPlantStrandPx pixels wide.
        bool goes = plantField(map, at, 21) > 0.5
                        ? abs(nk - 0.5) >= max(plantField(map, at, 14), kPlantStrandPx * pixM / clump)
                        : nk < plantField(map, at, 14);
        goes = goes && above > plantField(map, at, 17);
        if (goes) {
            if (f > 0.0) return false;
            brown = max(brown, smoothstep(-0.3, -0.12, f));
            black = max(charring, smoothstep(-0.12, -0.02, f));
            float edge = smoothstep(-0.1, -0.04, f) * (1.0 - smoothstep(-0.02, 0.0, f));
            g_fireEmit = max(g_fireEmit, kPlantEmber * (kPlantEmberGlow * plantField(map, at, 19) * edge));
        } else {
            brown = max(brown, smoothstep(-0.3, -0.05, f));
            black = max(charring, plantField(map, at, 15) * smoothstep(-0.05, 0.15, f));
        }
    }
    float scorch = max(brown, black);
    if (scorch <= 0.0) return true;
    float n = puddleNoise(vec2(p.x - 0.5 * p.y, p.z + 0.3 * p.y) * 0.7 + vec2(3.7, 29.1));
    float grain = clamp(dot(toLinear(albedo), kLuma) / 0.06, 0.5, 1.6);
    vec3 dry = mix(kBurnLeafBrown, kBurnCharcoal, black) * (0.7 + 0.6 * n) * grain;
    g_burnSpec = min(g_burnSpec, mix(1.0, kBurnObjectSpec, scorch));
    albedo = mix(albedo, toDisplay(dry), scorch);
    return true;
}

// A plant's bark (its trunk and branches) at world p in its own fire: the
// display-space albedo charred from the ground up, higher on the lee side
// with a ragged slanted top, and whole up to its frame top as its flare or
// torch climbs it. Sets g_burnSpec (char is matt).
vec3 plantBark(sampler2D map, ivec2 at, vec3 albedo, vec3 p, vec3 origin) {
    float since = plantSince(plantField(map, at, 4));
    if (since < 0.0) return albedo;
    float above = p.y - plantField(map, at, 2);
    vec2 dir = vec2(plantField(map, at, 5), plantField(map, at, 6));
    vec2 rel = p.xz - origin.xz;
    vec2 ring = rel * inversesqrt(max(dot(rel, rel), 1e-6));
    float side = dot(ring, dir);
    float near = 1.0 - smoothstep(0.5 * kBurnDetailFar, kBurnDetailFar, length(vWorldRel));
    float ragged = 0.65 * puddleNoise(ring * 1.7 + origin.xz * 0.37) +
                   0.35 * mix(0.5, puddleNoise(ring * 4.3 + vec2(p.y * 0.8, 9.7)), near);
    float top = mix(plantField(map, at, 9), plantField(map, at, 10), smoothstep(-0.9, 0.9, side));
    top *= mix(0.4, 1.0, smoothstep(0.0, 4.0, since)) * (1.0 + kBurnSootRagged * 2.0 * (ragged - 0.5));
    float b0 = plantField(map, at, 11), b1 = plantField(map, at, 12);
    if (b1 > b0) top = max(top, plantField(map, at, 16) * smoothstep(b0, b1, since));
    float cover = smoothstep(0.0, 1.5, since) * (1.0 - smoothstep(0.7 * top, top, above));
    if (cover <= 0.0) return albedo;
    float tone = 0.6 * puddleNoise(vec2(dot(ring, vec2(-dir.y, dir.x)) * 2.0, p.y * 1.1) + vec2(5.1, 0.0)) +
                 0.4 * mix(0.5, puddleNoise(vec2(p.x + p.z, p.y) * 3.7), near);
    float grain = mix(1.0, clamp(dot(toLinear(albedo), kLuma) / 0.2, 0.4, 1.8), kBurnObjectGrain);
    vec3 soot = kBurnCharcoal * (1.0 + kBurnObjectTone * 2.0 * (tone - 0.5)) * grain;
    g_burnSpec = min(g_burnSpec, mix(1.0, kBurnObjectSpec, cover));
    return mix(albedo, toDisplay(soot), cover * kBurnCharMax);
}

// ---- A building's own fire (buildings builder; the object main, while litBurnRect.w is 1) ----
// A building the fire set alight has a slot in the plant table (found by its
// origin as a plant is; FireSpreadWorld.cpp WriteBuildingLook), marked
// kBuildingMark in its last field: its ground (2) and height (3), its soot's
// reach down its walls from the eaves as a share of their height (5), its char all over once
// its roof is in (6), its rubble's glow (7), the share of its windows alight
// (8: their fans of soot), its wall tops (9), how far a shed or a barn has
// burnt away (10: its lowest metre is left), and how far a gutted stone
// house's or church's roof has fallen in (15: its faces above the wall tops go,
// a ragged hole first; the walls, gables and chimney stand), and 1 once it is
// gutted and wears its shell (16, end-state agent: FireShell.hpp).
const float kBuildingMark = 7777.0;   // FireSpreadWorld.cpp kBuildingMark
const float kBuildingFanM = 3.5;      // a window every this along a wall (FireSpreadWorld.cpp kWindowSpacingM)
const float kBuildingFanHead = 2.1;   // the fans rise from the window heads (kWindowTopM)
const float kBuildingGlowShare = 0.5; // the rubble's glow on its low walls, a share of the ash's embers
// A gutted shell's glow by day, a share of its glow by night (the day:
// litBurnRect.w less 1, by the sun's elevation, EngineGL33_Lighting.cpp
// UploadLitBurn): by day only a faint shimmer of it shows, at dusk and by
// night all of it (the owner: no strong orange by day).
const float kBuildingDayGlow = 0.004;
float buildingGlowByDay() {
    return mix(1.0, kBuildingDayGlow, clamp(litBurnRect.w - 1.0, 0.0, 1.0));
}
const float kBuildingRubbleGlow = 0.1; // (end-state agent) the gutted shell's rubble: its veins and points, dull

ivec2 burnBuildingAt(sampler2D map, vec2 xz) {
    ivec2 at = burnPlantAt(map, xz);
    if (at.x < 0 || abs(plantField(map, at, kPlantSlotFloats - 1) - kBuildingMark) > 0.5) return ivec2(-1);
    return at;
}

// A building's fragment at world p (normal n) in its own fire: the
// display-space albedo sooted from its eaves down as far as its fire has
// gone (a ragged edge), with fans of soot above its windows, charred all
// over once burnt out (its roof's faces first); its rubble's red glow low on
// its walls by night (g_fireEmit). False (discard) above what is left of a
// shed or a barn that has burnt away, and for a gutted shell's roof. Sets
// g_burnSpec.
bool burnBuilding(sampler2D map, ivec2 at, inout vec3 albedo, vec3 p, vec3 n) {
    float h = max(plantField(map, at, 3), 1.0);
    float above = p.y - plantField(map, at, 2);
    float gone = plantField(map, at, 10);
    if (gone > 0.0 && above > mix(h + 1.0, 0.8, gone)) return false;
    float soot = plantField(map, at, 5), charred = plantField(map, at, 6), glow = plantField(map, at, 7);
    float windows = plantField(map, at, 8), wallM = plantField(map, at, 9);
    // The roof fallen in: its faces (turned up or down, above the wall tops)
    // go in ragged holes as it falls; the upright faces stay (walls, gables,
    // the chimney, a church's tower).
    float roof = plantField(map, at, 15);
    if (roof > 0.0 && above > wallM - 0.3 && abs(n.y) > 0.3 &&
        puddleNoise(p.xz * 0.45 + vec2(2.3, 8.1)) < roof * 1.2 - 0.05)
        return false;
    // (end-state agent) The rubble inside a gutted shell (FireShell.cpp: the
    // roof's own tiles fallen in, 0.3 to 1 m deep): broken red tile mottled
    // with grey-white ash and burnt lime, black only in scattered charred
    // pieces (the eleventh answer), its embers glowing in patches by night.
    // Found as the faces turned up, low, inside the walls of a gutted house
    // (16: FireSpreadWorld.cpp WriteBuildingLook, 1 once its shell is on).
    if (plantField(map, at, 16) > 0.5 && n.y > 0.6 && above < min(1.4, wallM - 0.4)) {
        vec2 lp = p.xz - vec2(plantField(map, at, 0), plantField(map, at, 1));
        vec2 ax = vec2(plantField(map, at, 11), plantField(map, at, 12));
        float lx = abs(dot(lp, ax)), lz = abs(dot(lp, vec2(-ax.y, ax.x)));
        if (lx < plantField(map, at, 13) - 0.1 && lz < plantField(map, at, 14) - 0.1) {
            float ash = smoothstep(0.42, 0.68, puddleNoise(p.xz * 0.9 + vec2(3.7, 1.3)));
            float burnt = smoothstep(0.6, 0.78, puddleNoise(p.xz * 2.3 + vec2(9.1, 4.2)));
            vec3 tile = toLinear(albedo) * vec3(0.85, 0.74, 0.68);
            vec3 ashC = vec3(0.30, 0.285, 0.265) * (0.8 + 0.45 * puddleNoise(p.xz * 5.3 + vec2(1.1, 7.7)));
            vec3 c = mix(tile, ashC, 0.7 * ash);
            c = mix(c, kBurnSoot * 1.6, 0.85 * burnt);
            c = mix(c, kBurnSoot * 3.0, 0.2 * charred);
            albedo = toDisplay(c);
            g_burnSpec = min(g_burnSpec, kBurnObjectSpec);
            if (glow > 0.0) {
                // thin veins through the heap and points where the charred
                // pieces lie, dull red (kBuildingRubbleGlow of a cell's bed)
                float vn = puddleNoise(p.xz * 2.2 + vec2(litBurnHeat.z * 0.02, 5.0));
                float vein = smoothstep(0.9, 0.98, 1.0 - abs(2.0 * vn - 1.0));
                float spot = smoothstep(0.7, 0.85, puddleNoise(p.xz * 4.5 + vec2(2.0, 9.0))) * burnt;
                g_fireEmit = max(g_fireEmit, kEmberColour * (kEmberNits * 3.14159265 * 1.6 * kBuildingRubbleGlow * glow *
                                                             buildingGlowByDay() * max(vein, spot)));
            }
            return true;
        }
    }
    vec2 t = vec2(-n.z, n.x) + vec2(1e-3, 3.7e-4);
    float s = dot(p.xz, t * inversesqrt(dot(t, t)));
    float near = 1.0 - smoothstep(0.5 * kBurnDetailFar, kBurnDetailFar, length(vWorldRel));
    float ragged = 0.65 * puddleNoise(vec2(s * 0.6, 3.1)) + 0.35 * mix(0.5, puddleNoise(vec2(s * 1.9, 17.3)), near);
    // Smoke pushing out under the roof stains the walls from the eaves down
    // (ragged), as far as its fire has gone (soot 0..1 of the wall's height);
    // the roof's edge with it.
    float down = soot * wallM * (0.75 + 0.5 * ragged);
    float cover = soot > 0.0 ? smoothstep(wallM - down - 0.4, wallM - down + 0.6, above) *
                                   (1.0 - smoothstep(wallM + 0.6, wallM + 1.6, above))
                             : 0.0;
    // Fans above the windows: from each window head up to the wall tops,
    // widening as they rise.
    float u = s / kBuildingFanM + 0.3 * puddleNoise(vec2(s * 0.15, 5.0));
    float across = abs(fract(u) - 0.5) / (0.18 + 0.12 * max(above - kBuildingFanHead, 0.0));
    float fan = (1.0 - smoothstep(0.6, 1.0, across)) * smoothstep(kBuildingFanHead - 0.3, kBuildingFanHead + 0.2, above) *
                (1.0 - smoothstep(wallM, wallM + 1.0, above)) * clamp(windows * 1.5, 0.0, 1.0);
    cover = max(cover, 0.9 * fan);
    // Charred all over as it burns out, the faces turned up (the roof's
    // rubble, the wall tops) first.
    cover = max(cover, charred * mix(0.75, 1.0, smoothstep(0.2, 0.6, n.y)));
    cover *= 0.85 + 0.3 * puddleNoise(vec2(s, p.y) * 1.3 + vec2(7.0, 3.0));
    cover = clamp(cover, 0.0, 1.0);
    if (cover > 0.0) {
        float tone = 0.6 * puddleNoise(vec2(s, p.y) * 1.1 + vec2(5.1, 0.0)) + 0.4 * mix(0.5, puddleNoise(vec2(s, p.y) * 3.7), near);
        float grain = mix(1.0, clamp(dot(toLinear(albedo), kLuma) / 0.2, 0.4, 1.8), kBurnObjectGrain);
        vec3 c = kBurnSoot * (1.0 + kBurnObjectTone * 2.0 * (tone - 0.5)) * grain;
        g_burnSpec = min(g_burnSpec, mix(1.0, kBurnObjectSpec, cover));
        albedo = mix(albedo, toDisplay(c), cover * kBurnSootMax);
    }
    // The rubble's glow (end-state agent: the eleventh answer, "the glow is
    // inside and low"): what lies flat low down glows, its veins slow; the
    // walls' inner faces are lit from below by the bed, fading up them and
    // flickering slowly; stone never glows itself, and the outer faces stay
    // dark (the bed shows through the openings only).
    if (glow > 0.0) {
        vec2 toC = vec2(plantField(map, at, 0), plantField(map, at, 1)) - p.xz;
        // inside the walls (a doorstep or a plinth outside them never glows)
        vec2 ax = vec2(plantField(map, at, 11), plantField(map, at, 12));
        float inFoot = step(abs(dot(toC, ax)), plantField(map, at, 13) - 0.1) *
                       step(abs(dot(toC, vec2(-ax.y, ax.x))), plantField(map, at, 14) - 0.1);
        float inner = smoothstep(0.05, 0.4, dot(n.xz, toC) * inversesqrt(max(dot(toC, toC), 1e-4)));
        float lit = inFoot * inner * (1.0 - smoothstep(0.2, 2.6, above)) *
                    (0.75 + 0.25 * puddleNoise(vec2(litBurnHeat.z * 0.15, s * 0.3)));
        float low = inFoot * (1.0 - smoothstep(0.0, 1.2, above)) * smoothstep(0.5, 0.8, n.y);
        float veins = smoothstep(0.5, 0.8, puddleNoise(p.xz * 1.7 + vec2(litBurnHeat.z * 0.03, 0.0)));
        g_fireEmit = max(g_fireEmit, kEmberColour * (kEmberNits * 3.14159265 * 1.6 * kBuildingGlowShare * glow *
                                                     buildingGlowByDay() * max(0.04 * lit, low * veins)));
    }
    return true;
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
    float F = 0.02 + 0.98 * pow(clamp(1.0 - vh, 0.0, 1.0), 5.0);
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
// is the sky's own sun (litSkySun): under the moon key litSunDir is the
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

// The cavity term (the surfaces switch, litSurface3.x): the texture's height
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
// value; the combat lights at the head of the list at their own linear
// light, which no night factor scales), or the draw's own list. Two lobes in
// one walk of the lights: `streakA` at roughness `roughA` (the film's) when
// `wantA`, `streakB` at `roughB` (the puddle's) when `wantB`; each light's
// term is worked out once and each lobe sums into its own total in the order
// one walk per lobe summed them. A light behind the surface (n.L <= 0, where
// wetLobe is 0 for every roughness) adds nothing and is passed over before
// its term. The tiled list is the one localLightsTiled read for this fragment
// (g_tileCount; read again if it did not run).
void wetLampStreaks(vec3 n, vec3 v, float roughA, bool wantA, float roughB, bool wantB, out vec3 streakA,
                    out vec3 streakB) {
    vec3 sumA = vec3(0.0);
    vec3 sumB = vec3(0.0);
    vec3 combatA = vec3(0.0);
    vec3 combatB = vec3(0.0);
    float nv = clamp(dot(n, v), 1e-3, 1.0);
    float aA = max(roughA * roughA, 0.002);
    float aB = max(roughB * roughB, 0.002);
    if (litLightCount.y > 0.5) {
        int count = g_tileCount >= 0 ? g_tileCount : loadTileList();
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
            if (dot(n, L) <= 0.0) continue;
            if (color.w > 1.5) {
                // A combat light (combatLights' model, without its shadow).
                float r2 = frameLights[at + 2].x * frameLights[at + 2].x;
                if (d2 >= r2) continue;
                float x = d2 / r2;
                float win = (1.0 - x * x) * (1.0 - x * x);
                vec3 combat = color.rgb * (win / (1.0 + d2 / max(pos.w * pos.w, 1e-4)));
                if (wantA) combatA += combat * wetLobe(L, d, n, v, nv, aA, kWetSpecCap);
                if (wantB) combatB += combat * wetLobe(L, d, n, v, nv, aB, kWetSpecCap);
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
            if (wantA) sumA += light * wetLobe(L, d, n, v, nv, aA, kWetSpecCap);
            if (wantB) sumB += light * wetLobe(L, d, n, v, nv, aB, kWetSpecCap);
        }
        streakA = sumA * litLightCount.z + combatA;
        streakB = sumB * litLightCount.z + combatB;
        return;
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
        float d = sqrt(max(d2, 1e-12));
        vec3 L = toLight / d;
        if (dot(n, L) <= 0.0) continue;
        float atten = (d2 > start2 ? start2 / d2 : 1.0) * (1.0 - smoothstep(0.5 * end2, end2, d2)) * cone;
        vec3 light = toLinear(min(litLightColor[i].rgb * (atten * litGround.w), vec3(1.5)));
        if (wantA) sumA += light * wetLobe(L, d, n, v, nv, aA, kWetSpecCap);
        if (wantB) sumB += light * wetLobe(L, d, n, v, nv, aB, kWetSpecCap);
    }
    streakA = sumA;
    streakB = sumB;
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
    // shine (only puddles reflect): asphalt and gravel, which soak
    // up little, darken by their film instead.
    if (ground) a *= 1.0 - kWetGroundFilmDark * film * (1.0 - pud.x);
    mirror = pud.x;
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
    // roughness drew a bright ring at every puddle's edge). Only a puddle draws
    // a streak: the ground around it takes the broad, dim glint of its grit
    // (kWetSunGroundRough, kWetSunGroundFilm), as for the sun.
    if (litMatClass.z > 0.5) {
        vec3 streakFilm, streakPuddle;
        wetLampStreaks(nf, v, ground ? max(rough, kWetSunGroundRough) : rough, pud.x < 1.0, kWetPuddleRough, pud.x > 0.0,
                       streakFilm, streakPuddle);
        if (pud.x < 1.0) lit += streakFilm * (film * (1.0 - pud.x) * (ground ? kWetSunGroundFilm : 1.0));
        if (pud.x > 0.0) lit += streakPuddle * pud.x;
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
        vec3 F = f0 + (1.0 - f0) * pow(clamp(1.0 - vh, 0.0, 1.0), 5.0);
        // Rough natural surfaces (soil, grass, cloth, plaster, brick) shadow
        // their own microfacets: GGX's broad lobe on them reads as an even
        // grey sheen that takes the warmth out of a sunlit facade and lifts
        // its dark windows (a grazing glare), so it keeps 0.15 of itself from
        // roughness 0.9 down from 0.55.
        float roughCut = mix(1.0, 0.15, smoothstep(0.55, 0.9, rough) * (1.0 - metal));
        o.sun = litSun.rgb * (D * vis * nl * kSurfPi * sunVisibility * roughCut) * F;
    }
    // The sky: a smooth surface mirrors the sky as the player sees it, a rough
    // one the sky light around its reflected direction (the irradiance there);
    // both lose what the receiver's sky visibility hides. Where none of it
    // shows (roughSky exactly 0: the land, rubber, a blending texture that is
    // not glass, a matt surface seen face on) the reflection and its
    // environment term are skipped: o.sky stays 0 and the diffuse keeps
    // 1 - metal, what the full sum gives there. Nothing in the skipped block
    // takes a derivative (the table lookup samples unit 8 without mip levels),
    // and its sky irradiance call's g_surfGroundShare is read by
    // shadeRemastered before this function runs.
    if (roughSky > 0.0) {
        vec2 ab = surfaceEnvBrdf(rough, nv);
        vec3 fEnv = f0 * ab.x + ab.y;
        vec3 r = reflect(-v, n);
        vec3 rd = normalize(mix(r, n, rough * rough));
        vec3 rough_l = surfaceSkyIrradiance(rd, false) * 1.6;
        vec3 pref = rough_l;
        // The sky as seen only where the lobe is narrow (glass, lenses, metal,
        // wet film): the table lookup is the costly part, and most surfaces
        // skip it.
        float smoothW = 1.0 - smoothstep(0.08, 0.45, rough);
        if (smoothW > 0.0) {
            vec3 smooth_l = toLinear(skyReflection(normalize(vec3(r.x, max(r.y, 0.02), r.z)))) / max(litZenith.w, 1e-3);
            smooth_l = mix(smooth_l, rough_l, 1.0 - smoothstep(-0.15, 0.0, r.y));
            pref = mix(rough_l, smooth_l, smoothW);
        }
        o.sky = pref * fEnv * (skyVis * roughSky);
        // The diffuse gives up only the energy the reflection actually shows.
        o.diffuse = (1.0 - dot(fEnv, kLuma) * roughSky) * (1.0 - metal);
    } else {
        o.diffuse = 1.0 - metal;
    }
    float s = litSurface.y * litHorizon.w;
    o.sun *= s;
    o.sky *= s;
    return o;
}
)";

} // namespace Poseidon::gl33lighting
