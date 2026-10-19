// Remastered lighting shaders.
//
// A second program set, built next to the classic one in InitPixelShaders and
// selected at runtime by the Lighting setting (Remastered / Classic). The
// classic shader strings in EngineGL33_Shaders.cpp are untouched, so Classic is
// the legacy look bit for bit.
//
// What changes for geometry drawn through VSTransform (terrain, objects,
// vehicles, soldiers, trees, water):
//   - lighting is evaluated per fragment in linear HDR and tonemapped per
//     fragment (Khronos PBR Neutral without its toe), instead of a per-vertex
//     sum clamped to 1 before texturing;
//   - the sun is shadowed (shadow maps, terrain horizon, cloud field) while the
//     sky fill and local lights are not;
//   - the ambient is a sky hemisphere (zenith / ground bounce / a horizon
//     band that is warmer and brighter toward the sun) instead of a constant,
//     and loses light in valleys (terrain sky visibility);
//   - foliage (half-lit materials) is lit with the normal of an ellipsoid
//     fitted to its shape, so a tree crown has a lit side and a shaded side
//     instead of noise from scattered leaf cards;
//   - fog toward the sun takes the sky dome's sun glow, so distant ground
//     fades into the sky colour behind it;
//   - local lights (headlights, lamps, fires) are evaluated per fragment,
//     from the frame's tiled light list while it is on (lightTiles);
//   - glossy materials get normalised Blinn-Phong with a Fresnel boost and a
//     sky reflection.
// Fog, the night-eye desaturation, alpha test / alpha-to-coverage, the IsColored
// tint and the detail-texture modulation keep their classic order and space.
// Pre-lit geometry (VSScreen: sky, clouds, particles, 2D) passes through
// exactly as in the classic shaders, except under the sky table (feature
// switch `atmosphere`): the sky dome is then shaded from the physically based
// sky table (vLitMode 2, shadeSkyDome) and pre-lit world draws fade into the
// table's horizon colour with their fog (vLitMode -1); and under the clouds
// switches (`litClouds`, `skyCloudShadows`): the dome turns with the sky's
// drift, the dome's and the sky clouds' clouds are lit from their sun side
// (vLitMode 2 and -2) and the sun disk dims behind a dome cloud.
//
// A fragment shader is the common prelude followed by one main body per pixel
// shader. The prelude is split by concern (FragmentPiece in
// EngineGL33_Lighting.hpp) and RemasterFragmentCommon() joins the pieces in a
// fixed order: the header, the LitConstants block, the inputs and colour
// helpers, shadows, terrain and clouds, sky (EngineGL33_LightingShadersSky.cpp),
// local lights (EngineGL33_LightingShadersLights.cpp), surface
// (EngineGL33_LightingShadersSurface.cpp), shadeRemastered and finishFragment.
// A feature edits the piece of its concern. The LitConstants block is one
// string (LitConstantsBlock()) that every shader reading the rig pastes. The
// vertex shaders are standalone.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <iterator>
#include <string>

namespace
{

// Shared varyings of the remastered set: the classic ones plus the world-space
// normal and a flat flag telling the fragment shader whether the vertex shader
// supplied a normal (VSTransform) or a pre-lit colour (VSScreen / VSShadow).
//
// The screen vertex shader is three pieces (kScreenVertexPieces): its head, the
// LitConstants block (the sky pass reads the camera basis and the sky table's
// blends from it) and its main.

static const char s_vsScreenRemasterHead[] = R"(#version 330 core
layout(std140) uniform VSConstants {
    mat4 _pad_proj;
    mat4 _pad_view;
    mat4 _pad_world;
    vec4 _pad_sunDir;
    vec4 _pad_ambient;
    vec4 _pad_diffuse;
    vec4 _pad_emissive;
    vec4 _pad_fog;
    vec4 _pad_camPos;
    vec4 _pad_spec;
    vec4 _pad_specEn;
    vec4 _pad_sunEn;
    vec4 vpScale;       // 21: {2/width, 2/height, TAA jitter x, y (NDC)}
};
)";

static const char s_vsScreenRemasterMain[] = R"(
layout(location = 0) in vec3 aPos;
layout(location = 1) in float aRhw;
layout(location = 2) in vec4 aColor;
layout(location = 3) in vec4 aSpecular;
layout(location = 4) in vec2 aUV0;
layout(location = 5) in vec2 aUV1;

out vec4 vColor;
out vec4 vSpecColor;
out vec2 vUV0;
out vec2 vUV1;
out float vFogTC;
out vec3 vWorldRel;
out vec3 vNormalW;
out vec4 vCrown;
flat out float vLitMode;

void main() {
    float w = 1.0 / aRhw;
    gl_Position.x = (aPos.x * vpScale.x - 1.0) * w;
    gl_Position.y = (1.0 - aPos.y * vpScale.y) * w;
    gl_Position.z = aPos.z * w;
    gl_Position.w = w;
    // TAA sub-pixel jitter: zero outside a jittered world pass, and then the
    // position is left exactly as computed.
    if (vpScale.z != 0.0 || vpScale.w != 0.0) gl_Position.xy += vpScale.zw * gl_Position.w;
    vColor = aColor;
    vSpecColor = aSpecular;
    vUV0 = aUV0;
    vUV1 = aUV1;
    vFogTC = aSpecular.a;
    vWorldRel = vec3(0.0);
    vNormalW = vec3(0.0, 1.0, 0.0);
    vCrown = vec4(0.0);
    vLitMode = 0.0; // pre-lit colour: the fragment shader passes it through
    // The sky's own clouds turn with the sky's drift (litSky.z, 0 unless a
    // clouds switch is on): the dome's texture turns about the zenith.
    bool dome = litSkyFwd.w > 0.5 && litSkyFwd.w < 1.5;
    if (dome && litSky.z != 0.0) vUV0.x += litSky.z;
    // The sky table and the lit clouds: the camera-relative point on the view
    // ray, from the vertex's screen position and the camera basis,
    // perspective-interpolated so the fragment shader gets an exact view
    // direction. The sky dome (sky mode 1) is shaded from the table and its
    // clouds lit (vLitMode 2); the sky clouds (sky mode 2) are lit (-2) and,
    // like every other pre-lit draw of the world (the far horizon polygon,
    // particles), fade into the table's horizon colour with their fog (-1).
    // The sun disk (sky mode 3) keeps its colour and its classic fog.
    if (litAtmo.x > 0.0 || litAtmo.z > 0.0 || litSky.x > 0.0 || litSky.z != 0.0) {
        vec2 ndc = vec2(aPos.x * vpScale.x - 1.0, 1.0 - aPos.y * vpScale.y);
        vec3 rel = (litSkyFwd.xyz + ndc.x * litSkyRight.xyz + ndc.y * litSkyUp.xyz) * w;
        if (dome && (litAtmo.x > 0.0 || litSky.x > 0.0 || litSky.z != 0.0)) {
            vWorldRel = rel;
            vLitMode = 2.0;
        } else if (litSkyFwd.w > 1.5 && litSkyFwd.w < 2.5 && litSky.x > 0.0) {
            vWorldRel = rel;
            vLitMode = -2.0;
        } else if (litAtmo.z > 0.0 && litSkyFwd.w < 2.5) {
            vWorldRel = rel;
            vLitMode = -1.0;
        }
    }
}
)";

static const char s_vsTransformRemasterGLSL[] = R"(#version 330 core
layout(std140) uniform VSConstants {
    mat4 proj;          // c0-c3
    mat4 view;          // c4-c7
    mat4 world;         // c8-c11
    vec4 sunDir;        // c12
    vec4 ambient;       // c13
    vec4 diffuse;       // c14
    vec4 emissive;      // c15
    vec4 fogParam;      // c16: {start, invRange, enabled, 0}
    vec4 camPos;        // c17
    vec4 specular;      // c18: rgb + power(w)
    vec4 specEn;        // c19
    vec4 sunEn;         // c20
    vec4 vpScale;       // c21: zw TAA jitter (NDC)
    vec4 crownCenter;   // c22: xyz shape crown centre (model space), w = valid
    vec4 crownScale;    // c23: xyz 1 / crown half extents
    mat4 texMat0;       // c24-c27
    mat4 texMat1;       // c28-c31
    vec4 texCtrl;       // c32: {genTex0, genTex1, 0, 0}
};

layout(std140) uniform WorldInstances {
    mat4 worldArr[256];
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out vec4 vColor;
out vec4 vSpecColor;
out vec2 vUV0;
out vec2 vUV1;
out float vFogTC;
out vec3 vWorldRel;
out vec3 vNormalW;
out vec4 vCrown;
flat out float vLitMode;

void main() {
    vec4 worldPos    = worldArr[gl_InstanceID] * vec4(pos, 1.0);
    // Many tree crowns have leaf cards with zero mesh normals: keep them zero
    // (normalizing a zero vector gives NaN); the fragment shader gives them the
    // crown's normal.
    vec3 worldNormal = mat3(worldArr[gl_InstanceID]) * normal;
    worldNormal     *= inversesqrt(max(dot(worldNormal, worldNormal), 1e-12));
    vec4 viewPos     = view * worldPos;
    gl_Position      = proj * viewPos;
    // TAA sub-pixel jitter (see vsScreenRemaster).
    if (vpScale.z != 0.0 || vpScale.w != 0.0) gl_Position.xy += vpScale.zw * gl_Position.w;
    vWorldRel        = worldPos.xyz; // camera-relative world position
    vNormalW         = worldNormal;
    vLitMode         = 1.0;
    // Crown frame for foliage: the position inside an ellipsoid fitted to the
    // shape (w = 1 on its surface) and the ellipsoid's outward normal, which
    // lights a tree crown as one volume instead of as scattered leaf cards.
    if (crownCenter.w > 0.5) {
        vec3 e = (pos - crownCenter.xyz) * crownScale.xyz;
        vCrown = vec4(mat3(worldArr[gl_InstanceID]) * (e * crownScale.xyz), length(e));
    } else {
        vCrown = vec4(0.0);
    }

    // The fragment shader lights per pixel. The vertex colour keeps the classic
    // sun term for the unlit fallback (PSFlat) and, above all, the classic
    // material alpha, which depends on N.L and fades objects in and out.
    float NdotL = max(0.0, dot(worldNormal, -sunDir.xyz));
    vec4 litColor;
    litColor.rgb = emissive.rgb + ambient.rgb * sunEn.x + diffuse.rgb * NdotL * sunEn.x;
    litColor.a   = emissive.a   + ambient.a   * sunEn.x + diffuse.a   * NdotL * sunEn.x;
    vColor = clamp(litColor, 0.0, 1.0);
    vSpecColor = vec4(0.0);

    float dist = length(worldPos.xyz - camPos.xyz);
    float fogFactor = clamp(1.0 - (dist - fogParam.x) * fogParam.y, 0.0, 1.0);
    vFogTC = (fogParam.z > 0.5) ? fogFactor : 1.0;

    vUV0 = (texCtrl.x > 0.5) ? (texMat0 * vec4(uv, 0, 1)).xy : uv;
    vUV1 = (texCtrl.y > 0.5) ? (texMat1 * vec4(uv, 0, 1)).xy : uv;
}
)";

static const char s_vsShadowRemasterGLSL[] = R"(#version 330 core
layout(std140) uniform VSConstants {
    mat4 proj;          // c0-c3
    mat4 view;          // c4-c7
    mat4 world;         // c8-c11
    vec4 sunDir;        // c12
    vec4 ambient;       // c13
    vec4 diffuse;       // c14
    vec4 emissive;      // c15
    vec4 fogParam;      // c16
    vec4 camPos;        // c17
    vec4 specular;      // c18
    vec4 specEn;        // c19
    vec4 sunEn;         // c20
    vec4 vpScale;       // c21: zw TAA jitter (NDC)
    vec4 _pad22;
    vec4 _pad23;
    mat4 texMat0;       // c24-c27
    mat4 texMat1;       // c28-c31
    vec4 texCtrl;       // c32
};

layout(std140) uniform WorldInstances {
    mat4 worldArr[256];
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out vec4 vColor;
out vec4 vSpecColor;
out vec2 vUV0;
out vec2 vUV1;
out float vFogTC;
out vec3 vWorldRel;
out vec3 vNormalW;
out vec4 vCrown;
flat out float vLitMode;

void main() {
    vec4 worldPos = worldArr[gl_InstanceID] * vec4(pos, 1.0);
    gl_Position   = proj * view * worldPos;
    // TAA sub-pixel jitter (see vsScreenRemaster).
    if (vpScale.z != 0.0 || vpScale.w != 0.0) gl_Position.xy += vpScale.zw * gl_Position.w;
    vColor        = diffuse;
    vSpecColor    = vec4(0.0);
    vUV0          = (texCtrl.x > 0.5) ? (texMat0 * vec4(uv, 0, 1)).xy : uv;
    vUV1          = vUV0;
    vFogTC        = 1.0;
    vWorldRel     = vec3(0.0);
    vNormalW      = vec3(0.0, 1.0, 0.0);
    vCrown        = vec4(0.0);
    vLitMode      = 0.0;
}
)";

// The fragment prelude by concern (FragmentPiece), joined in kFragmentPieces
// order. Each piece after the header starts with the blank line that separates
// it from the one before. Header: the version and the classic pixel constants.
static const char s_psRemasterHeader[] = R"(#version 330 core
layout(std140) uniform PSConstants {
    vec4 fogColor;     // c0
    vec4 alphaRef;     // c1: {ref, enabled, alphaToCoverage, flatDebug}
    vec4 shadowCtl;    // c2: {enable, bias, darkness, texelSize}
    vec4 constColor;   // c3: per-object IsColored tint (white = no-op)
    vec4 psC4;         // c4: water lightDir
    vec4 psC5;         // c5: grass grassCoef1
    vec4 psC6;         // c6: grass grassCoef2
    vec4 rgbEyeCoef;   // c7
    mat4 cascadeVP[4]; // c8-c23
    vec4 cascadeSplits;// c24
    vec4 cascadeCtl;   // c25: {count, fadeRange, biasBase, omniCount}
    vec4 camFwd;       // c26
};
)";

} // namespace

namespace Poseidon::gl33lighting
{

// The LitConstants block (binding 3): the one GLSL declaration of the LitConst
// layout in EngineGL33_Lighting.hpp. Every shader that reads the rig pastes this
// string (LitConstantsBlock(); the post air pass and the reflections' trace
// name it in kPostShaders); the layout test checks it slot by slot.
const char kLitConstantsBlockGLSL[] = R"(
// Remastered light rig and per-draw material, binding 3. Layout mirrors
// LitConst in EngineGL33_Lighting.hpp.
layout(std140) uniform LitConstants {
    vec4 litSunDir;       // 0: xyz toward the key light (the sun; the moon under the moon key), w = sun enabled (sunEn)
    vec4 litSun;          // 1: rgb key light radiance, w = its elevation (degrees)
    vec4 litZenith;       // 2: rgb sky irradiance on an up-facing surface, w = exposure
    vec4 litHorizon;      // 3: rgb on a vertical surface facing away from the sun, w = specular scale
    vec4 litGround;       // 4: rgb on a down-facing surface, w = local light scale
    vec4 litMoonDir;      // 5: xyz toward the moon
    vec4 litMoon;         // 6: rgb moon radiance
    vec4 litCamPos;       // 7: xyz camera world position
    vec4 litMatDiffuse;   // 8: rgb material diffuse (carries the user brightness)
    vec4 litMatAmbient;   // 9: rgb material ambient
    vec4 litMatForced;    // 10: rgb forced diffuse (normal-independent sun)
    vec4 litMatEmissive;  // 11: rgb emissive
    vec4 litMatSpecular;  // 12: rgb specular, w = power (0 = none)
    vec4 litTerrain;      // 13: x enabled, y 1/cellSize, z 1/size, w sky occlusion strength
    vec4 litTerrainSel0;  // 14: one-hot component of the first horizon direction
    vec4 litTerrainSel1;  // 15: one-hot component of the second horizon direction
    vec4 litTerrainSun;   // 16: x layer of dir 0, y layer of dir 1, z blend, w penumbra (degrees)
    vec4 litTerrainLift;  // 17: x lift start (m), y lift end (m), z sun shadow strength
    vec4 litCloud;        // 18: x cover, y transmission under a cloud, z 1/scale (1/m), w enabled
    vec4 litCloudOffset;  // 19: xy drift (m), z deck altitude (m), w edge softness
    vec4 litShadowDelta;  // 20: xyz camera move since the shadow pass, w filter radius (texels)
    vec4 litFlags;        // 21: x AO share to alpha, y world effects on, z debug view, w saturation
    vec4 litLightCount;   // 22: x the draw's lights, y tile size of the tiled list (0 = off), z its night factor
    vec4 litHorizonSun;   // 23: rgb on a vertical surface facing the sun, w = crowns lit as a canopy
    vec4 litFogSun;       // 24: rgb sky-dome sun glow added to the fog (display), w enabled
    vec4 litFogCap;       // 25: rgb fog colour limit (the sky texture colour)
    vec4 litLightPos[8];  // 26-33: xyz camera-relative, w start attenuation distance
    vec4 litLightColor[8];// 34-41: rgb diffuse radiance, w = spotlight
    vec4 litLightAmb[8];  // 42-49: rgb ambient radiance
    vec4 litLightDir[8];  // 50-57: xyz beam direction
    vec4 litReserved;     // 58: planned per-draw material class (padding until it lands)
    vec4 litTaa;          // 59: x soft-shadow tap turn, y 1 = TAA frame, z taps per frame, w tan(sun radius)
    vec4 litPcss;         // 60: x search reach (m), y min filter (texels), z soft strength, w leaf lift (texels)
    vec4 litSkyRight;     // 61: xyz camera right * tan(fov x / 2), w Mie asymmetry g
    vec4 litSkyUp;        // 62: xyz camera up * tan(fov y / 2)
    vec4 litSkyFwd;       // 63: xyz camera forward, w sky mode (0 world, 1 dome, 2 sky clouds)
    vec4 litAtmo;         // 64: x dome blend, y overcast cloud floor, z aerial perspective blend, w cloud mask dead zone
    vec4 litAtmoCloudSun; // 65: rgb sun light on the sky clouds (linear), w sky light on them (linear)
    vec4 litAtmoGlow;     // 66: rgb classic dome sun glow (display), w sky saturation
    vec4 litAtmoDome;     // 67: rgb classic dome light (display), w hue (0 the original hue relit, 1 physical)
    vec4 litSky;          // 68: x lit-cloud strength, y sky-matched shadows on, z the dome's turn (u), w sky-through
    vec4 litSkyShadow;    // 69: xy shadow anchor - camera (m), z previous anchor's angle (rad), w its weight
    vec4 litSkySun;       // 70: xyz toward the sun, w its elevation (degrees): the sun the sky reads (skySunDir)
    vec4 litNight;        // 71: x physical lamps and low beams on, y lamp per start metre, z low-beam peak, w moon key share
    vec4 litPlanned[2];   // 72-73: allocated to features still to land (combat, materials)
    vec4 litWater0;       // 74: x wave relief scale, y share of the next sea frame, z mean sky weight, w sky tint
    vec4 litWater1;       // 75: rgb sheen light (linear, the original sun's hue), w sheen's flat-normal share
    vec4 litCanopy;       // 76: long tree shadows: x strength (0 = off), yz sun-visibility map origin (m), w 1 / its extent
    vec4 litDetail;       // 77: terrain detail: x self-detail strength, y detail kept, z anti-tiling, w land draw
    vec4 litHaze;         // 78: aerial haze (B8): x classic fog factor scale minus 1 (0 = the classic fog)
};
)";

} // namespace Poseidon::gl33lighting

namespace
{

// Inputs: samplers, varyings and the colour space and tonemap helpers the
// later pieces use.
static const char s_psRemasterInputs[] = R"(
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2DArrayShadow shadowMap;    // unit 2, comparison sampler bound in remastered mode
uniform sampler2DArray terrainHorizon;     // unit 3: horizon angles (16-bit unorm), 4 directions per layer
uniform sampler2D terrainHeight;           // unit 4: r = height (m), g = sky visibility, ba = ground tint (r, b)
uniform sampler2D cloudNoise;              // unit 5: equalised tileable noise
uniform sampler2DArray shadowDepth;        // unit 6: the cascade array again, raw depth (soft-shadow blocker search)
uniform sampler2D skyLut;                  // unit 8: sky-view table (the sky table; Atmosphere.hpp)
uniform sampler2D skyDensity;              // unit 9: cloud density of the weather sky (SkyDensity.hpp)
uniform sampler2D canopySun;               // unit 15: the canopy's sun-visibility map (long tree shadows; Canopy.hpp)

in vec4 vColor;
in vec4 vSpecColor;
in vec2 vUV0;
in vec2 vUV1;
in float vFogTC;
in vec3 vWorldRel;
in vec3 vNormalW;
in vec4 vCrown;
flat in float vLitMode;

out vec4 fragColor;

const vec3 kLuma = vec3(0.2126, 0.7152, 0.0722);

vec3 toLinear(vec3 c) { return pow(max(c, vec3(0.0)), vec3(2.2)); }
vec3 toDisplay(vec3 c) { return pow(max(c, vec3(0.0)), vec3(1.0 / 2.2)); }

// Khronos PBR Neutral without its toe (the toe would black out classic-level
// nights). Identity below 0.8, soft shoulder, hue kept, highlights whiten.
vec3 tonemapNeutral(vec3 c) {
    float peak = max(c.r, max(c.g, c.b));
    if (peak < 0.8) return c;
    const float d = 0.2;
    float newPeak = 1.0 - d * d / (peak + d - 0.8);
    c *= newPeak / peak;
    float g = 1.0 - 1.0 / (0.15 * (peak - newPeak) + 1.0);
    return mix(c, vec3(newPeak), g);
}
)";

// Shadows: the sun's visibility from the cascaded shadow maps.
static const char s_psRemasterShadows[] = R"(
// 12-tap Poisson disc; each tap is a hardware 2x2 comparison (bilinear PCF),
// so the penumbra is smooth without per-pixel noise.
const vec2 kPoisson[12] = vec2[](
    vec2(-0.326, -0.406), vec2(-0.840, -0.074), vec2(-0.696,  0.457), vec2(-0.203,  0.621),
    vec2( 0.962, -0.195), vec2( 0.473, -0.480), vec2( 0.519,  0.767), vec2( 0.185, -0.893),
    vec2( 0.507,  0.064), vec2( 0.896,  0.412), vec2(-0.322, -0.933), vec2(-0.792, -0.598));

// The stock filter: 12 hardware comparisons on the Poisson disc, `radius` in
// light-plane uv.
float pcfVisibility(vec2 suv, int c, float zRef, float radius) {
    float lit = 0.0;
    for (int k = 0; k < 12; ++k)
        lit += texture(shadowMap, vec4(suv + kPoisson[k] * radius, float(c), zRef));
    return lit / 12.0;
}

// Interleaved gradient noise: a value per pixel that differs from its
// neighbours, here the turn of the soft filter's tap pattern.
float shadowNoise(vec2 p) {
    return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

// Contact-hardening soft sun shadow (PCSS, softShadows; SunShadows.hpp). The
// sun is a disk of angular radius atan(litTaa.w). A blocker search in the raw
// depth map (shadowDepth) finds the mean depth of what shadows the receiver
// within litPcss.x metres along the sun ray; the penumbra in the light plane is
// that distance times the radius' tangent, so the shadow is sharp where a
// caster meets the ground and soft at the far end of a long evening shadow.
// Taps are golden-angle (Vogel) spirals turned per pixel; each filter tap is a
// hardware 2x2 comparison. Without TAA the search takes 16 taps on the near
// tiers (8 on the frustum tiers, where it spans a few texels) and the filter
// 12, 24 or 32 by its width; in a TAA frame (litTaa.y) 4 of each, turned anew
// every frame (litTaa.x), and the history averages them. `zRef` is the
// receiver's biased depth, `ts` one texel in uv, `texelWorld` one texel in
// metres, `tanSlope` the receiver's tilt against the light (its depth rises
// across the kernel; terrain, which never casts, passes 0).
float pcssVisibility(vec2 suv, int c, float zRef, float ts, float texelWorld, float tanSlope) {
    float tanA = litTaa.w;
    float depthPerM = max(length(vec3(cascadeVP[c][0][2], cascadeVP[c][1][2], cascadeVP[c][2][2])), 1e-9);
    bool temporal = litTaa.y > 0.5;
    float turn = 6.2831853 * (shadowNoise(gl_FragCoord.xy) + litTaa.x);
    float slopeDepth = tanSlope * texelWorld * depthPerM; // depth per texel of tap offset
    float searchT = clamp(litPcss.x * tanA / texelWorld, 2.0, 48.0);
    int nSearch = temporal ? int(litTaa.z) : (c < int(cascadeCtl.w) ? 16 : 8);
    float z0 = texture(shadowDepth, vec3(suv, float(c))).r;
    float bSum = 0.0;
    float bN = 0.0;
    if (z0 < zRef) { bSum += z0; bN += 1.0; }
    for (int k = 0; k < 16; ++k) {
        if (k >= nSearch) break;
        float r = sqrt((float(k) + 0.5) / float(nSearch)) * searchT;
        float a = float(k) * 2.39996323 + turn;
        float zb = texture(shadowDepth, vec3(suv + vec2(cos(a), sin(a)) * (r * ts), float(c))).r;
        if (zb < zRef - r * slopeDepth) { bSum += zb; bN += 1.0; }
    }
    if (bN < 0.5) return 1.0;
    float dB = max(zRef - bSum / bN, 0.0) / depthPerM; // metres along the sun ray
    float rT = clamp(dB * tanA / texelWorld, litPcss.y, searchT);
    int nTaps = temporal ? int(litTaa.z) : (rT < 3.0 ? 12 : (rT < 10.0 ? 24 : 32));
    float lit = 0.0;
    for (int k = 0; k < 32; ++k) {
        if (k >= nTaps) break;
        float r = sqrt((float(k) + 0.5) / float(nTaps)) * rT;
        float a = float(k) * 2.39996323 + turn + 1.3;
        lit += texture(shadowMap, vec4(suv + vec2(cos(a), sin(a)) * (r * ts), float(c), zRef - r * slopeDepth));
    }
    return lit / float(nTaps);
}

// Sun visibility from the cascaded shadow maps: 1 = lit. Same tier selection,
// cross-fade and far fade as the classic shaders, re-anchored to the camera
// position the depth pass used. Casters self-shadow, so their lookup point is
// pushed out along the normal by a fraction of a texel facing the sun and up to
// two texels at grazing sun (no stripes on walls and leaf cards). The terrain
// never casts into the map and passes offsetScale 0, and up-facing surfaces fade
// the offset out: an offset there would slide every shadow away from its caster
// by metres at a low sun. Leaves (treeShadows) also move their lookup
// `leafLift` texels toward the sun: their two-sided cards are in the map, and
// no normal keeps a card at a grazing sun off its own depth.
float shadowMapVisibility(vec3 n, float nl, float offsetScale, float leafLift) {
    if (shadowCtl.x < 0.5) return 1.0;
    vec3 rel = vWorldRel + litShadowDelta.xyz;
    int nC = int(cascadeCtl.x);
    int omniN = int(cascadeCtl.w);
    float eyeDepth = dot(rel, camFwd.xyz);
    float dist3D = length(rel);
    int ci = nC;
    for (int i = 0; i < 4; ++i) {
        if (i >= nC) break;
        float metric = (i < omniN) ? dist3D : eyeDepth;
        if (metric <= cascadeSplits[i]) { ci = i; break; }
    }
    if (ci >= nC) return 1.0;
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
        float w = (p == 0) ? (1.0 - bw) : ((wSum <= 0.0) ? 1.0 : ((p == 1) ? bw : 0.0));
        if (w <= 0.0) continue;
        float texelWorld = 2.0 * ts / max(length(vec3(cascadeVP[c][0][0], cascadeVP[c][1][0], cascadeVP[c][2][0])), 1e-6);
        float slope = sqrt(max(1.0 - nl * nl, 0.0));
        // Up-facing receivers (roads, decals, runways, roofs) do not need it
        // either: the depth pass stores back faces, so they never shadow
        // themselves, and an offset would shift their shadow edges against the
        // terrain beside them.
        float offset = texelWorld * offsetScale * (0.4 + 1.6 * slope) * (1.0 - smoothstep(0.75, 0.97, n.y));
        vec3 at = rel + n * offset;
        if (leafLift > 0.0) at += litSunDir.xyz * (leafLift * texelWorld);
        vec4 cp = cascadeVP[c] * vec4(at, 1.0);
        vec3 sc = cp.xyz / cp.w;
        vec2 suv = sc.xy * 0.5 + 0.5;
        if (suv.x > 0.0 && suv.x < 1.0 && suv.y > 0.0 && suv.y < 1.0 && sc.z > 0.0 && sc.z < 1.0) {
            float bias = cascadeCtl.z * float(c + 1) * float(c + 1);
            // Looser tiers cover more ground per texel: shrink their radius so
            // the penumbra width in metres stays comparable across tiers.
            float radius = ts * litShadowDelta.w / float(c + 1);
            // The soft filter (fading in over 2 degrees above the gate's edge)
            // or the stock one.
            float soft = litTaa.w > 0.0 ? litPcss.z : 0.0;
            float lit = 1.0;
            if (soft < 1.0) lit = pcfVisibility(suv, c, sc.z - bias, radius);
            if (soft > 0.0) {
                float tanSlope = offsetScale > 0.0 ? min(slope / max(nl, 0.25), 4.0) : 0.0;
                lit = mix(lit, pcssVisibility(suv, c, sc.z - bias, ts, texelWorld, tanSlope), soft);
            }
            litSum += w * lit;
            wSum += w;
        }
    }
    if (wSum <= 0.0) return 1.0;
    float lit = litSum / wSum;
    float lastSplit = cascadeSplits[nC - 1];
    float fade = clamp((lastSplit - eyeDepth) / max(cascadeCtl.y, 0.001), 0.0, 1.0);
    return 1.0 - (1.0 - lit) * fade;
}
)";

} // namespace

namespace Poseidon::gl33lighting
{

// Terrain and clouds: the sun's visibility past the hills and under the broken
// cloud field, the valley sky visibility and the ground-bounce tint. The post
// air pass pastes it too (cloudSunVisibility).
const char kRemasterTerrainCloudsGLSL[] = R"(
// Colour of the light the ground below bounces (luminance 1; white when there
// is no map). Set by terrainSunVisibility, read by skyIrradiance.
vec3 g_groundTint = vec3(1.0);
// Height of the ground below the fragment (m; far below anything without a
// map). Set by terrainSunVisibility, read by canopySunVisibility.
float g_groundHeight = -1e6;

// Hills shadowing the sun, from the precomputed horizon map. Also returns the
// terrain sky visibility (valley floors see less sky) and sets the ground
// tint. Geometry well above the ground (roofs, tree crowns, aircraft) is
// lifted out of the shadow and the occlusion.
float terrainSunVisibility(vec3 posAbs, out float skyVisibility) {
    skyVisibility = 1.0;
    if (litTerrain.x < 0.5 || litFlags.y < 0.5) return 1.0;
    vec2 g = posAbs.xz * litTerrain.y; // grid units
    float last = 1.0 / litTerrain.z - 1.0;
    if (g.x < 0.0 || g.y < 0.0 || g.x > last || g.y > last) return 1.0;
    vec2 uv = (g + 0.5) * litTerrain.z;
    vec4 hs = texture(terrainHeight, uv);
    g_groundTint = vec3(hs.b, (1.0 - 0.2126 * hs.b - 0.0722 * hs.a) / 0.7152, hs.a);
    g_groundHeight = hs.x;
    float lift = smoothstep(litTerrainLift.x, litTerrainLift.y, posAbs.y - hs.x);
    skyVisibility = mix(1.0, mix(hs.g, 1.0, lift), litTerrain.w);
    float h0 = dot(texture(terrainHorizon, vec3(uv, litTerrainSun.x)), litTerrainSel0);
    float h1 = dot(texture(terrainHorizon, vec3(uv, litTerrainSun.y)), litTerrainSel1);
    float horizonDeg = mix(-5.0, 45.0, mix(h0, h1, litTerrainSun.z)); // TerrainHorizonMap::kMin/kMaxAngleDeg
    float soft = litTerrainSun.w;
    float vis = smoothstep(horizonDeg - soft, horizonDeg + soft, litSun.w);
    return mix(1.0, mix(vis, 1.0, lift), litTerrainLift.z);
}

// ---- The sky's own clouds (feature switches litClouds and skyCloudShadows;
// Graphics/Remaster/SkyDensity.hpp holds the CPU mirrors and the constants) --
// The cloud-density map on unit 9, built from the weather sky texture: r broken
// cloud (whiter than the clearest sky of its row), g the row is grey all round
// (an overcast deck, counted as far as the weather is overcast), b darkness.
const float kSkyShadowDeck = 1800.0;         // kSkyShadowDeckAltitude (m above sea level)
const float kSkyShadowThreshold = 0.55;      // kSkyShadowThreshold
const float kSkyShadowEdge = 0.12;           // kSkyShadowEdge
const float kSkyShadowAnchorRadius = 1500.0; // kSkyShadowAnchorRadius (m)
const float kSkySunBehindCloud = 0.12;       // kSkySunBehindCloud: the sun disk's light behind a dome cloud

// SkyCloudDensity: broken cloud or the overcast deck, thicker where darker.
float skyCloudDensity(vec4 m) {
    float deck = m.g * (1.0 - smoothstep(0.35, 0.75, litSky.w));
    return max(m.r, deck) * mix(0.75 + 0.25 * m.b, 0.5 + 0.5 * m.b, deck);
}

// SkyDomeUv: the dome's texture coordinate of a world direction, from
// obloha.p3d's own mapping (u = azimuth / 180 deg, the texture wraps twice
// around the horizon; v piecewise over the rings at -15, 0.87, 29.5, 59.5 and
// 89 deg), turned with the sky (litSky.z).
vec2 skyDomeUv(vec3 w) {
    float az = atan(w.x, w.z) * 57.29578;
    float e = asin(clamp(w.y * inversesqrt(max(dot(w, w), 1e-12)), -1.0, 1.0)) * 57.29578;
    float v;
    if (e < 0.87) v = mix(1.0, 0.9869, clamp((e + 15.0) / 15.87, 0.0, 1.0));
    else if (e < 29.5) v = mix(0.9869, 0.5434, (e - 0.87) / 28.63);
    else if (e < 59.5) v = mix(0.5434, 0.1736, (e - 29.5) / 30.0);
    else v = mix(0.1736, 0.036, clamp((e - 59.5) / 29.5, 0.0, 1.0));
    return vec2(az / 180.0 + litSky.z, v);
}

// SkyShadowCover of the dome's cloud in a direction (seen from the camera).
float skyShadowCoverToward(vec3 w) {
    float d = skyCloudDensity(textureLod(skyDensity, skyDomeUv(w), 0.0));
    return smoothstep(kSkyShadowThreshold - kSkyShadowEdge, kSkyShadowThreshold + kSkyShadowEdge, d);
}

// The cloud shadow on a camera-relative point: projected along the sun to the
// deck, the deck point is looked up in the dome in the direction an anchor
// sees it (the camera's own view of the sky, from where the anchor stands,
// litSkyShadow.xy), so the cloud the player sees in the sky is the one whose
// shadow lies on the ground. While the anchor moves on, the old one's shadows
// fade out (litSkyShadow.zw).
float skyShadowCoverFrom(vec3 q, vec2 anchor) {
    vec3 w = q - vec3(anchor.x, 0.0, anchor.y);
    return w.y > 1.0 ? skyShadowCoverToward(w) : 0.0;
}
float domeCloudCover(vec3 rel) {
    vec3 l = litSunDir.xyz;
    float up = max(l.y, 0.06);
    vec3 q = rel + l * (max(kSkyShadowDeck - (rel.y + litCamPos.y), 0.0) / up);
    float c = skyShadowCoverFrom(q, litSkyShadow.xy);
    if (litSkyShadow.w > 0.0) {
        vec2 prev = litSkyShadow.xy + kSkyShadowAnchorRadius * vec2(cos(litSkyShadow.z), sin(litSkyShadow.z));
        c = mix(c, skyShadowCoverFrom(q, prev), litSkyShadow.w);
    }
    return c;
}

// Broken-cloud shadows: the ground point is projected along the sun ray up to
// the cloud deck and looked up in a drifting noise field whose covered fraction
// matches the overcast. The noise is histogram-equalised and read once, and
// the soft edge is symmetric about the threshold and narrows toward a clear or
// a fully covered sky, so the covered fraction is exactly the overcast (no sun
// patches under a full overcast, no stray shadows under a clear sky). With the
// sky-matched shadows (litSky.y) the dome's own clouds cast them instead, at
// the transmission that keeps the field's mean (LightRig skyShadowCoverage).
float cloudSunVisibility(vec3 posAbs) {
    if (litCloud.w < 0.5 || litFlags.y < 0.5) return 1.0;
    if (litSky.y > 0.5) return mix(1.0, litCloud.y, domeCloudCover(posAbs - litCamPos.xyz));
    float up = max(litSunDir.y, 0.12);
    vec2 p = posAbs.xz + litSunDir.xz / up * (litCloudOffset.z - posAbs.y);
    float n = texture(cloudNoise, (p + litCloudOffset.xy) * litCloud.z).r;
    float cover = clamp(litCloud.x, 0.0, 1.0);
    float edge = litCloudOffset.w * clamp(4.0 * cover * (1.0 - cover), 0.0, 1.0);
    float covered = edge > 1e-4 ? smoothstep(1.0 - cover - edge, 1.0 - cover + edge, n) : step(1.0 - cover, n);
    return mix(1.0, litCloud.y, covered);
}

// Long tree shadows beyond the shadow maps (canopyShadows; Canopy.hpp): the
// sun-visibility map traced through the canopy of every tree, bush, forest and
// building, read once. A texel holds r the ground's visibility, g the lowest
// raised receiver's (one that never shades itself), b the height above the
// ground below which a raised receiver (a crown, a wall, a roof) is in a
// neighbour's shadow (/ 48 m) and a that edge's softness (/ 16 m). It takes
// over where the shadow maps fade out (their far fade, or from 100 to 150 m
// without them) and fades out at the map's edge. 1 while off (litCanopy.x 0).
// Reads g_groundHeight: call it after terrainSunVisibility.
float canopySunVisibility(vec3 posAbs) {
    if (litCanopy.x <= 0.0 || litFlags.y < 0.5) return 1.0;
    int nC = int(cascadeCtl.x);
    bool maps = shadowCtl.x > 0.5 && nC > 0;
    vec3 rel = vWorldRel + litShadowDelta.xyz;
    float depth = maps ? dot(rel, camFwd.xyz) : length(vWorldRel);
    float reach = maps ? cascadeSplits[nC - 1] : 150.0;
    float fade = maps ? max(cascadeCtl.y, 1.0) : 50.0;
    float w = clamp((depth - (reach - fade)) / fade, 0.0, 1.0);
    vec2 uv = (posAbs.xz - litCanopy.yz) * litCanopy.w;
    vec2 e = abs(uv - 0.5);
    w *= litCanopy.x * (1.0 - smoothstep(0.42, 0.48, max(e.x, e.y)));
    if (w <= 0.0) return 1.0;
    vec4 m = texture(canopySun, uv);
    float hr = posAbs.y - g_groundHeight;
    float h = m.b * 48.0;
    float s = max(m.a * 16.0, 0.25);
    float raised = mix(m.g, 1.0, smoothstep(h - s, h + s, max(hr, 1.5)));
    return mix(1.0, mix(m.r, raised, smoothstep(0.5, 1.5, hr)), w);
}
)";

} // namespace Poseidon::gl33lighting

namespace
{

// Sky, local lights and surface are in EngineGL33_LightingShaders{Sky,Lights,Surface}.cpp.

// Shade: one fragment's light (sun, sky, moon, local lights, emissive, gloss),
// tonemapped.
static const char s_psRemasterShade[] = R"(
// Sun visibility of the last shaded fragment (water reuses it for its glint).
float g_sunVisibility = 1.0;

// Tree crowns lit as a canopy (crownCanopy, litHorizonSun.w; research track
// shade, crown V18). The sun reaching a leaf crosses the crown in front of it:
// the path, in crown radii, from the leaf to the crown's surface toward the
// sun, with the crown taken as a sphere around the ellipsoid's centre (the
// leaf at |e| = vCrown.w along the gradient direction `cnDir`).
float crownSunPath(vec3 cnDir, float r, vec3 l) {
    vec3 p = cnDir * min(r, 1.0);
    float b = dot(p, l);
    float c = dot(p, p) - 1.0;
    return max(-b + sqrt(max(b * b - c, 0.0)), 0.0);
}

// Lights a fragment of a VSTransform draw. `albedo` is the display-space surface
// colour (texture, tint and detail already applied); `terrain` marks surfaces
// that never cast into the shadow map. Returns the tonemapped display-space
// colour and the sky-light share of the radiance, which the post pass uses to
// apply ambient occlusion to the sky light only.
vec3 shadeRemastered(vec3 albedo, bool terrain, out float ambientShare) {
    vec3 n = vNormalW * inversesqrt(max(dot(vNormalW, vNormalW), 1e-12));
    vec3 posAbs = vWorldRel + litCamPos.xyz;
    vec3 v = -vWorldRel * inversesqrt(max(dot(vWorldRel, vWorldRel), 1e-12));
    vec3 l = litSunDir.xyz;
    float sunOn = litSunDir.w;

    // Foliage and other half-lit surfaces (a forced, normal-independent sun
    // term): leaf cards are two-sided and their normals scatter, so light them
    // with the crown's normal, wrap the terminator (light scatters through the
    // leaves), keep some sun inside the crown's own shadow and dim the sky fill
    // toward the crown's core.
    bool foliage = dot(litMatForced.rgb, kLuma) > 1e-4;
    bool crown = foliage && vCrown.w > 0.0;
    vec3 cn = crown ? vCrown.xyz * inversesqrt(max(dot(vCrown.xyz, vCrown.xyz), 1e-12)) : vec3(0.0);
    // A leaf card without a mesh normal (zero, see VSTransform) takes the
    // crown's normal for its sun, sky and shadow lookup.
    if (foliage && dot(n, n) < 0.25) n = cn;
    vec3 nCast = (foliage && dot(n, l) < 0.0) ? -n : n;
    vec3 nLight = n;
    // A crown lit as a canopy (crownCanopy; grass shades as terrain and keeps
    // its look): the sun reaching a leaf crosses the crown in front of it
    // (optical depth 1 per crown radius, a quarter of the sun passes whatever
    // the depth), the core sees less sky (0.45 of it, 0.6 without), and below
    // 35 degrees of sun, fully below 12, the crown takes 0.65 of the sun and
    // 0.75 of the sky: a low sun's wrapped terminator and the exposure meter
    // otherwise light the whole crown pale. `crownSun` and `crownSky` stay 1
    // and the stock values without it.
    bool canopy = crown && !terrain && litHorizonSun.w > 0.5;
    float crownSun = 1.0;
    float crownSky = 1.0;
    if (crown) {
        vec3 card = dot(n, cn) < 0.0 ? -n : n;
        vec3 blend = card * 0.3 + cn * 0.7;
        nLight = blend * inversesqrt(max(dot(blend, blend), 1e-12));
        crownSky = mix(canopy ? 0.45 : 0.6, 1.0, smoothstep(0.1, 0.95, vCrown.w));
    } else if (foliage) {
        nLight = nCast;
    }
    // The low sun's crown factors follow the sun, not the moon key (B3): the
    // night's crowns keep them under the moon.
    if (canopy) {
        float lowSun = 1.0 - smoothstep(12.0, 35.0, skySunElevation());
        crownSun = mix(1.0, exp(-crownSunPath(cn, vCrown.w, l)), 0.75) * mix(1.0, 0.65, lowSun);
        crownSky *= mix(1.0, 0.75, lowSun);
    }

    float nl = max(dot(n, l), 0.0);
    float nlCast = max(dot(nCast, l), 0.0);
    float skyVisibility;
    // Leaves (not grass, which shades as terrain) lift their lookup toward the
    // sun: treeShadows' cure for the stripes a two-sided card casts on itself.
    float leafLift = (foliage && !terrain) ? litPcss.w : 0.0;
    float sunVisibility = shadowMapVisibility(nCast, nlCast, terrain ? 0.0 : 1.0, leafLift) *
                          terrainSunVisibility(posAbs, skyVisibility) * cloudSunVisibility(posAbs);
    // Long tree shadows beyond the shadow maps (canopyShadows); 1 while off.
    sunVisibility *= canopySunVisibility(posAbs);
    g_sunVisibility = sunVisibility;

    vec3 kd = litMatDiffuse.rgb;
    vec3 ka = litMatAmbient.rgb;
    float sunTerm = nl;
    float sunShadow = sunVisibility;
    if (foliage) {
        sunTerm = clamp((dot(nLight, l) + 0.35) / 1.35, 0.0, 1.0);
        sunShadow = mix(0.45, 1.0, sunVisibility) * crownSun;
    }
    vec3 direct = litSun.rgb * (kd * sunTerm + litMatForced.rgb) * (sunShadow * sunOn);
    vec3 ambient = ka * skyIrradiance(nLight) * (skyVisibility * crownSky * sunOn);
    vec3 moon = kd * litMoon.rgb * (max(dot(nLight, litMoonDir.xyz), 0.0) * sunOn);
    // Classic adds local lights in display space before texturing and clamps
    // the sum at 1; taking that sum to linear keeps the authored size and
    // falloff of headlight and lamp pools (a linear sum would make them about
    // twice as wide). The same clamp keeps a lamp-lit surface from getting
    // brighter than the original's, where overlapping pools sum past 1.
    // The physical lamps and low beams (nightLights) add their own linear
    // light outside that clamp; leaves take them through the crown, as the sun.
    g_lightView = v;
    g_lightLeafNormal = nLight;
    g_lightLeaves = foliage;
    g_lightForced = litMatForced.rgb;
    vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.0))) + g_physicalLocal * litGround.w;
    vec3 emissive = toLinear(min(litMatEmissive.rgb, vec3(1.5)));

    vec3 a = toLinear(albedo);
    if (canopy) {
        // Light that bounces between leaves before it leaves the crown picks
        // up the leaf colour again: the canopy's albedo is the leaf's with its
        // saturation deepened (a power of 1.3) at the same luminance.
        vec3 p = pow(a, vec3(1.3));
        a = p * (dot(a, kLuma) / max(dot(p, kLuma), 1e-6));
    }
    float exposure = litZenith.w;
    vec3 radiance = (a * (direct + ambient + moon + local + emissive) + glossy(n, v, sunVisibility) * sunOn +
                     g_physicalSpec * litGround.w) * exposure;
    float total = dot(radiance, kLuma);
    ambientShare = clamp(dot(a * ambient * exposure, kLuma) / max(total, 1e-5), 0.0, 1.0);
    if (foliage) ambientShare *= 0.5;

    if (litFlags.z > 0.5 && litFlags.z < 4.5) {
        // Debug views: 1 = sun visibility, 2 = terrain sky visibility, 3 = normal,
        // 4 = the albedo in display space (debug=7; LightRig.hpp kShaderDebugAlbedo).
        if (litFlags.z < 1.5) return vec3(sunVisibility);
        if (litFlags.z < 2.5) return vec3(skyVisibility);
        if (litFlags.z < 3.5) return nLight * 0.5 + 0.5;
        return albedo;
    }
    vec3 display = toDisplay(tonemapNeutral(radiance));
    float grey = dot(display, kLuma);
    display = max(mix(vec3(grey), display, litFlags.w), vec3(0.0));
    // Debug view 6: the lights per tile of the tiled light list.
    return litFlags.z > 5.5 ? lightTileView(display) : display;
}
)";

// Finish: alpha, the night eye and fog, the tail every main ends with.
static const char s_psRemasterFinish[] = R"(
// Classic night-eye desaturation, fog and alpha handling, shared tail.
void finishFragment(vec4 r0, float ambientShare, bool nightEye) {
    if (alphaRef.z > 0.5) {
        // Alpha-to-coverage cutouts write coverage, and alpha-to-one makes the
        // covered samples 1, which the post pass reads as "no sky light to
        // occlude": leaves keep their light.
        float cov = clamp((r0.a - alphaRef.x) / max(fwidth(r0.a), 1e-4) + 0.5, 0.0, 1.0);
        if (cov <= 0.0) discard;
        r0.a = cov;
    } else {
        if (r0.a - alphaRef.x * alphaRef.y < 0.0) discard;
        // Opaque draws keep their alpha free (no blending reads it): store one
        // minus the share of sky light that survives the fog, for the post
        // pass's ambient occlusion. Pre-lit surfaces store 1 (nothing to
        // occlude), and so do alpha-tested cutouts, as they do under MSAA
        // (coverage, then alpha-to-one), so MSAA does not change their look.
        if (litFlags.x > 0.5) r0.a = alphaRef.y > 0.5 ? 1.0 : 1.0 - ambientShare * clamp(vFogTC, 0.0, 1.0);
    }
    if (nightEye) {
        float luminance = clamp(dot(r0.rgb, rgbEyeCoef.rgb), 0.0, 1.0);
        float nightBlend = clamp(luminance + rgbEyeCoef.a, 0.0, 1.0);
        // Lit surfaces fade toward a slightly cool grey of the same luminance:
        // the dark-adapted eye sees moonlit ground pale and cool, not neutral.
        // Only blue is raised, so a half-faded warm surface does not turn
        // green. Bright (lamp-lit) surfaces keep their own colour, as in classic.
        const vec3 kNightGrey = vec3(0.985, 0.995, 1.1);
        bool lit = vLitMode > 0.5 && vLitMode < 1.5;
        r0.rgb = mix(vec3(luminance) * (lit ? kNightGrey : vec3(1.0)), r0.rgb, nightBlend);
    }
    // Fog: lit geometry fades into the sky behind it (the sun glow, and with
    // the sky table the table's horizon colour in its own azimuth); pre-lit
    // world draws take the table's horizon colour too (vLitMode -1, set only
    // while the table's aerial perspective is on). The dome itself is unfogged.
    // The table is read only where fog shows (the near field has vFogTC 1, and
    // the mix below then ignores the fog colour).
    vec3 fogC = fogColor.rgb;
    if (vLitMode > 0.5 && vLitMode < 1.5) {
        fogC = fogColorToward(vWorldRel);
        if (litAtmo.z > 0.0 && vFogTC < 1.0) fogC = atmoFogColor(vWorldRel, fogC);
    } else if (vLitMode < -0.5 && litAtmo.z > 0.0 && vFogTC < 1.0) {
        fogC = atmoFogColor(vWorldRel, fogC);
    }
    // With the aerial haze on (litHaze.x above 0) the classic fog keeps only
    // the view-distance edge: its start moves out (AirMedium.hpp FogEdgeScale)
    // and the post pass's haze takes over the distance before it.
    r0.rgb = mix(fogC, r0.rgb, min(vFogTC * (1.0 + litHaze.x), 1.0));
    fragColor = alphaRef.w > 0.5 ? vec4(1.0, 0.0, 0.0, 1.0) : r0;
}
)";

static const char s_psNormalRemasterMain[] = R"(
void main() {
    vec4 t0 = texture(tex0, vUV0);
    vec4 r0;
    float ambientShare = 0.0;
    if (vLitMode > 1.5) {
        // The sky dome (the sky table and the lit clouds), over its pre-lit
        // classic colour.
        vec4 classic = vColor * t0;
        classic *= constColor;
        classic.rgb += vSpecColor.rgb;
        vec3 rgb = classic.rgb;
        if (litAtmo.x > 0.0) rgb = shadeSkyDome(t0, classic.rgb);
        else if (litSky.x > 0.0) rgb = litCloudsOverClassic(rgb, litCloudTermsDome(vUV0));
        r0 = vec4(rgb, classic.a);
    } else if (vLitMode > 0.5) {
        r0.rgb = shadeRemastered(t0.rgb * constColor.rgb, false, ambientShare);
        r0.a = vColor.a * t0.a * constColor.a;
    } else {
        r0 = vColor * t0;
        r0 *= constColor;
        r0.rgb += vSpecColor.rgb;
        // The sky clouds lit from their sun side (litClouds).
        if (vLitMode < -1.5) r0.rgb = litCloudsOverClassic(r0.rgb, litCloudTermsSheet(t0.a));
        // The sun disk behind a dome cloud (skyCloudShadows): its colour and
        // its coverage both, so it fades into the cloud whatever its blending
        // (a dimmed colour alone drew a dark disk over the sky).
        if (litSkyFwd.w > 2.5 && litSky.y > 0.5)
            r0 *= mix(1.0, kSkySunBehindCloud, skyShadowCoverToward(skySunDir()));
    }
    finishFragment(r0, ambientShare, true);
}
)";

// The terrain main (PSDetail: the land and, until the sea gets its own shader,
// the sea). Terrain detail (B11, research track texsharp; the C++ mirror of
// this math and its constants is Graphics/Remaster/TerrainDetail.hpp): every
// terrain square is one 128 px texture over 50 m, so from about 10 m to 150 m
// a texel covers 5 to 15 pixels and the slope is a bilinear smear. The land
// takes three luminance-only layers, each relative to the local mean, so hue
// and average brightness stay as authored:
//   - self-detail: the luminance high-pass of the square's own texture, zoomed
//     kTdZoom times and read near the fragment (grass from grass, rock from
//     rock), in cells of kTdCellTexels texels, each cell's patch rotated by
//     its own random angle; the four cells around the fragment are blended
//     keeping the grain's variance, then softly limited at +-kTdLimit. The
//     zoomed sample takes the zoomed derivatives, so it mip-maps away by itself
//     once the base texture is no longer magnified. Near the eye, where the
//     zoomed patch is itself magnified into soft blobs, it fades in with the
//     patch's mip level (kTdNearFromLod to kTdNearToLod) and leaves the
//     authored detail texture alone;
//   - detail kept: the detail texture's deviation from its mean, scaled up as
//     its mips average it out (at most kTdKeptCap);
//   - anti-tiling: a rotated, rescaled second detail sample mixed in where the
//     detail texture is minified.
// litDetail.xyz is zero while the switch or its gate is off, and litDetail.w
// is zero for the sea: then this is the stock terrain main. Nothing is
// evaluated beyond kTdMaxDistance (the frame gate's "terrain within 300 m").
static const char s_psDetailRemasterMain[] = R"(
// Terrain detail constants (Graphics/Remaster/TerrainDetail.hpp holds the same).
const float kTdZoom = 6.0;
const float kTdCellTexels = 2.0;
const float kTdJitterTexels = 6.0;
const float kTdLimit = 0.4;
const float kTdMinLocalMean = 0.04;
const float kTdFadeStart = 220.0;
const float kTdMaxDistance = 300.0;
const float kTdKeptCap = 2.0;
const float kTdKeptFadeStart = 150.0;
const float kTdNearFromLod = -3.5;
const float kTdNearToLod = -2.5;
const float kTdMaxAniso = 16.0;

float tdLum(vec3 c) { return dot(c, vec3(0.2126, 0.7152, 0.0722)); }

// The mip level the GPU picks (kTdMaxAniso anisotropy) for uv derivatives dx,
// dy on a texture of size ts.
float tdLod(vec2 dx, vec2 dy, vec2 ts) {
    float l1 = length(dx * ts);
    float l2 = length(dy * ts);
    float pmax = max(l1, l2);
    float pmin = max(min(l1, l2), 1e-6);
    return log2(max(pmax / min(kTdMaxAniso, pmax / pmin), 1e-6));
}

// Two values in [0, 1) from a cell index (Hoskins' hash22).
vec2 tdHash2(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xx + p3.yz) * p3.zy);
}

// Self-detail at base uv `uv` (texture size ts, uv derivatives dx, dy): the
// variance-preserving blend of the four cells' relative luminance high-pass,
// zero mean, about -1..1.
float tdSelfDetail(vec2 uv, vec2 ts, vec2 dx, vec2 dy) {
    vec2 p = uv * ts; // base texel space
    vec2 g = p / kTdCellTexels - 0.5;
    vec2 i0 = floor(g);
    vec2 f = g - i0;
    vec2 gx = dx * kTdZoom;
    vec2 gy = dy * kTdZoom;
    float lpLod = log2(kTdZoom);
    float acc = 0.0;
    float w2 = 0.0;
    for (int k = 0; k < 4; ++k) {
        vec2 o = vec2(float(k & 1), float(k >> 1));
        vec2 cell = i0 + o;
        vec2 c = (cell + 0.5) * kTdCellTexels;
        vec2 jit = (tdHash2(cell + kTdZoom) - 0.5) * kTdJitterTexels;
        float a = 6.2831853 * tdHash2(cell + 17.0 + kTdZoom).x;
        mat2 R = mat2(cos(a), sin(a), -sin(a), cos(a));
        vec2 q = (c + R * ((p - c) * kTdZoom) + jit) / ts;
        float hf = tdLum(textureGrad(tex0, q, R * gx, R * gy).rgb);
        float lp = tdLum(textureLod(tex0, q, lpLod).rgb);
        vec2 ww = mix(1.0 - f, f, o);
        float w = ww.x * ww.y;
        acc += w * (hf - lp) / max(lp, kTdMinLocalMean);
        w2 += w * w;
    }
    return acc * inversesqrt(max(w2, 1e-4));
}

void main() {
    vec4 t0 = texture(tex0, vUV0);
    vec4 t1 = texture(tex1, vUV1);
    vec4 r0;
    float ambientShare = 0.0;
    if (vLitMode > 0.5 && vLitMode < 1.5) {
        // Detail modulation stays in display space, as in the classic shader,
        // so the terrain keeps its authored micro-contrast.
        float detail = t1.a * 2.0;
        float selfMod = 1.0;
        if (litDetail.w > 0.5 && (litDetail.x > 0.0 || litDetail.y > 0.0 || litDetail.z > 0.0)) {
            // Derivatives before the distance branch (non-uniform control flow).
            vec2 d0x = dFdx(vUV0);
            vec2 d0y = dFdy(vUV0);
            vec2 d1x = dFdx(vUV1);
            vec2 d1y = dFdy(vUV1);
            float dist = length(vWorldRel);
            if (dist < kTdMaxDistance) {
                float fade = 1.0 - smoothstep(kTdFadeStart, kTdMaxDistance, dist);
                vec2 ts0 = vec2(textureSize(tex0, 0));
                float selfW = litDetail.x * fade *
                             smoothstep(kTdNearFromLod, kTdNearToLod, tdLod(d0x, d0y, ts0) + log2(kTdZoom));
                if (selfW > 0.0) {
                    float m = selfW * tdSelfDetail(vUV0, ts0, d0x, d0y);
                    selfMod = 1.0 + m * inversesqrt(1.0 + m * m / (kTdLimit * kTdLimit));
                }
                float lod1 = tdLod(d1x, d1y, vec2(textureSize(tex1, 0)));
                float dMean = textureLod(tex1, vUV1, 16.0).a * 2.0; // the 1x1 mip: the texture's mean
                float dev = detail - dMean;
                float antiTile = litDetail.z * clamp(lod1, 0.0, 1.0) * fade;
                if (antiTile > 0.0) {
                    mat2 M = mat2(0.6, -0.8, 0.8, 0.6) * 0.87;
                    float dB = textureGrad(tex1, M * vUV1 + vec2(0.31, 0.57), M * d1x, M * d1y).a * 2.0 - dMean;
                    dev = mix(dev, (dev + dB) * 0.70710678, antiTile);
                }
                if (litDetail.y > 0.0) {
                    float boost = clamp(exp2(lod1 * 0.5 * litDetail.y), 1.0, kTdKeptCap);
                    dev *= mix(boost, 1.0, smoothstep(kTdKeptFadeStart, kTdMaxDistance, dist));
                }
                detail = max(dMean + dev, 0.0);
            }
        }
        vec3 albedo = t0.rgb * constColor.rgb * (detail * selfMod);
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
)";

static const char s_psGrassRemasterMain[] = R"(
void main() {
    vec4 t0 = texture(tex0, vUV0);
    vec4 t1 = texture(tex1, vUV1);
    if (vFogTC < 0.0) discard;
    vec4 r0;
    float ambientShare = 0.0;
    r0.a = clamp((psC5.a * 2.0 - 1.0) + t1.a, 0.0, 1.0);
    if (vLitMode > 0.5 && vLitMode < 1.5) {
        r0.rgb = shadeRemastered(clamp(t0.rgb * t1.rgb * 2.0, 0.0, 1.0), true, ambientShare);
    } else {
        r0.rgb = clamp(vColor.rgb * t0.rgb * t1.rgb * 2.0, 0.0, 1.0);
    }
    r0.a = clamp(psC6.a * r0.a * 2.0, 0.0, 1.0);
    finishFragment(r0, ambientShare, false);
}
)";

static const char s_psWaterRemasterMain[] = R"(
// The sea (feature switch seaWaves, Graphics/Remaster/SeaWaves.hpp). The
// landscape's sea faces come here in Remastered with the switch on (the
// backend routes them between Engine::BeginSea and EndSea); off, they stay on
// the detail program. tex0 is the sea texture's animation frame (one repeat per
// 50 m), seaNext the frame it cross-fades into (litWater0.y of it) and tex1
// the specular detail texture (data/specular_dx.paa), whose alpha is a ripple
// pattern (32 repeats per 50 m through the detail texgen).
//
// The texture's brightness is taken as the wave height. Three layers (the
// texture, a swell copy at 0.27x frequency rotated 37 degrees, the ripples)
// give the wave normal. The texture-to-world mapping is solved per pixel from
// screen derivatives (exact on the flat sea, so no tangents are needed), and
// the finite-difference step follows the pixel footprint, so the gradient is
// filtered by the same mips as the colour. The sea is its lit texture under
// the sky reflected by Fresnel on that normal: a low mean share, so it never
// turns into a pale mirror, and the part of the Fresnel term that comes from
// the waves at full weight, so a facet tilted away from the viewer shows sky
// and one tilted toward shows water. The sun lays a broad lobe and a tight
// sparkle on the waves (the sparkle widens with the footprint, so distant
// glitter turns into sheen instead of aliasing), and the material's own broad
// lobe keeps the original's sheen in the original sun's hue (litWater1.rgb).
uniform sampler2D seaNext; // unit 11: the animation frame tex0 cross-fades into

const float kSeaWaveK = 0.8;          // metres of height per unit of texture luminance
const float kSeaSwellK = 1.2;         // the swell layer
const float kSeaRippleK = 0.004;      // the ripple layer
const float kSeaFrameRho = 0.75;      // gradient correlation of consecutive frames (SeaWaves.hpp)
const float kSeaSkyVar = 1.0;         // weight of the wave-driven part of the Fresnel term
const float kSeaBroadPow = 32.0;      // the sun path
const float kSeaBroadW = 2.5;
const float kSeaTightPow = 1500.0;    // the sparkle
const float kSeaTightW = 1.0;
const float kSeaFadeStart = 400.0;    // m: the waves fade from here ...
const float kSeaFadeEnd = 3000.0;     // ... to here,
const float kSeaFadeFloor = 0.35;     // ... down to this share
const float kSeaAnisoFloor = 0.3;     // wave share kept where the footprint exceeds the 16:1 filter
const float kSeaFarSkyStart = 1000.0; // m: the mean sky share fades from here ...
const float kSeaFarSkyEnd = 3000.0;   // ... to here,
const float kSeaFarSkyFloor = 0.5;    // ... down to this share: the far sea keeps its own colour

vec4 seaTex(vec2 uv) {
    vec4 a = texture(tex0, uv);
    return litWater0.y > 0.0 ? mix(a, texture(seaNext, uv), litWater0.y) : a;
}

float seaLum(vec2 uv) { return dot(seaTex(uv).rgb, kLuma); }

// World gradients of the two texture coordinates on the horizontal sea plane,
// gU = (du/dX, du/dZ), solved from the screen derivatives.
void seaUvGradients(vec2 uv, out vec2 gU, out vec2 gV) {
    vec2 dPx = dFdx(vWorldRel.xz), dPy = dFdy(vWorldRel.xz);
    vec2 dUx = dFdx(uv), dUy = dFdy(uv);
    float det = dPx.x * dPy.y - dPx.y * dPy.x;
    float id = abs(det) > 1e-12 ? 1.0 / det : 0.0;
    gU = vec2(dUx.x * dPy.y - dUy.x * dPx.y, dUy.x * dPx.x - dUx.x * dPy.x) * id;
    gV = vec2(dUx.y * dPy.y - dUy.y * dPx.y, dUy.y * dPx.x - dUx.y * dPy.x) * id;
}

// Finite-difference step in texture space, matched to what the anisotropic
// filter keeps.
float seaUvStep(vec2 uv, float texel) {
    float a = length(dFdx(uv)), b = length(dFdy(uv));
    return max(texel, max(min(a, b), max(a, b) / 16.0));
}

// World-space height gradient (metres per metre per unit of height) of the sea
// texture at uv, whose height there is h0.
vec2 seaGrad(vec2 uv, float h0, vec2 gU, vec2 gV, float e) {
    float hu = seaLum(uv + vec2(e, 0.0));
    float hv = seaLum(uv + vec2(0.0, e));
    return ((hu - h0) * gU + (hv - h0) * gV) / e;
}

// The same for the ripple pattern (tex1's alpha).
vec2 seaRippleGrad(vec2 uv, float h0, vec2 gU, vec2 gV, float e) {
    float hu = texture(tex1, uv + vec2(e, 0.0)).a;
    float hv = texture(tex1, uv + vec2(0.0, e)).a;
    return ((hu - h0) * gU + (hv - h0) * gV) / e;
}

// The sea's lit body under its flat normal: shadeRemastered without the gloss
// and the foliage branch, as linear radiance after exposure, with the sky
// light's share and the sun's visibility. The physical lamps and low beams
// (nightLights) leave localLights' classic sum, so the body adds them as
// shadeRemastered does: their linear light and their gleam on the material.
vec3 seaBody(vec3 albedo, out float ambientShare, out float sunVis) {
    vec3 n = vNormalW * inversesqrt(max(dot(vNormalW, vNormalW), 1e-12));
    vec3 posAbs = vWorldRel + litCamPos.xyz;
    float sunOn = litSunDir.w;
    float nl = max(dot(n, litSunDir.xyz), 0.0);
    float skyVisibility;
    // The sea casts nothing into the map and is no leaf: no offset, no lift.
    sunVis = shadowMapVisibility(n, nl, 0.0, 0.0) * terrainSunVisibility(posAbs, skyVisibility) *
             cloudSunVisibility(posAbs);
    g_sunVisibility = sunVis;
    vec3 kd = litMatDiffuse.rgb;
    vec3 ka = litMatAmbient.rgb;
    vec3 direct = litSun.rgb * (kd * nl + litMatForced.rgb) * (sunVis * sunOn);
    vec3 ambient = ka * skyIrradiance(n) * (skyVisibility * sunOn);
    vec3 moon = kd * litMoon.rgb * (max(dot(n, litMoonDir.xyz), 0.0) * sunOn);
    g_lightView = -vWorldRel * inversesqrt(max(dot(vWorldRel, vWorldRel), 1e-12));
    vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.0))) + g_physicalLocal * litGround.w;
    vec3 emissive = toLinear(min(litMatEmissive.rgb, vec3(1.5)));
    vec3 a = toLinear(albedo);
    float exposure = litZenith.w;
    vec3 radiance = (a * (direct + ambient + moon + local + emissive) + g_physicalSpec * litGround.w) * exposure;
    ambientShare = clamp(dot(a * ambient * exposure, kLuma) / max(dot(radiance, kLuma), 1e-5), 0.0, 1.0);
    return radiance;
}

void main() {
    vec4 t1 = texture(tex1, vUV1);
    vec4 r0;
    float ambientShare = 0.0;
    // Derivatives first, in uniform control flow.
    vec2 gU, gV;
    seaUvGradients(vUV0, gU, gV);
    float e0 = seaUvStep(vUV0, 1.0 / 512.0);
    float e1 = seaUvStep(vUV1, 1.0 / 256.0);
    float foot = length(fwidth(vWorldRel.xz));
    float fa = length(dFdx(vUV0)), fb = length(dFdy(vUV0));
    float anisoRatio = min(fa, fb) / max(max(fa, fb), 1e-12);
    bool lit = vLitMode > 0.5 && vLitMode < 1.5;
    if (lit && litFlags.z < 0.5) {
        vec4 t0 = seaTex(vUV0);
        vec3 albedo = t0.rgb * constColor.rgb * (t1.a * 2.0);
        float dist = length(vWorldRel);
        // ---- The wave normal.
        vec2 grad = kSeaWaveK * seaGrad(vUV0, dot(t0.rgb, kLuma), gU, gV, e0);
        // The swell: the same texture at about 1/4 frequency, rotated 37 degrees.
        const mat2 R = mat2(0.7986, 0.6018, -0.6018, 0.7986) * 0.27;
        vec2 uvS = R * vUV0 + vec2(0.31, 0.57);
        vec2 gUs = R[0][0] * gU + R[1][0] * gV;
        vec2 gVs = R[0][1] * gU + R[1][1] * gV;
        float eS = max(1.0 / 512.0, e0 * 0.27);
        grad += kSeaSwellK * seaGrad(uvS, seaLum(uvS), gUs, gVs, eS);
        // The ripples: specular_dx's alpha at the detail tiling (vUV1 = 32 x vUV0).
        grad += kSeaRippleK * seaRippleGrad(vUV1, t1.a, gU * 32.0, gV * 32.0, e1);
        grad *= mix(kSeaFadeFloor, 1.0, 1.0 - smoothstep(kSeaFadeStart, kSeaFadeEnd, dist));
        // Beyond the 16:1 the anisotropic filter covers, the texture is
        // under-filtered along the view and its gradient aliases into
        // pixel-high stripes at grazing range: fewer waves there.
        grad *= clamp(16.0 * anisoRatio, kSeaAnisoFloor, 1.0);
        // Two frames mixed keep the relief of one (SeaCrossFadeGain).
        float w = litWater0.y;
        float spread = (1.0 - w) * (1.0 - w) + w * w + 2.0 * w * (1.0 - w) * kSeaFrameRho;
        grad *= litWater0.x * inversesqrt(max(spread, 1e-6));
        vec3 n = normalize(vec3(-grad.x, 1.0, -grad.y));
        vec3 v = -vWorldRel * inversesqrt(max(dot(vWorldRel, vWorldRel), 1e-12));
        vec3 l = litSunDir.xyz;
        float sunOn = litSunDir.w;
        float exposure = litZenith.w;
        float sunVis;
        vec3 body = seaBody(albedo, ambientShare, sunVis);
        // ---- The sky in the reflected direction, by Fresnel on the wave
        // normal: the mean share low (litWater0.z, less at a high sun and
        // beyond 1 km), the waves' share at full weight. At a high sun the
        // reflected sky also takes litWater0.w of the original fog colour.
        float nv = max(dot(n, v), 0.0);
        float F = 0.02 + 0.98 * pow(1.0 - nv, 5.0);
        float Ff = 0.02 + 0.98 * pow(1.0 - clamp(v.y, 0.0, 1.0), 5.0);
        vec3 r = reflect(-v, n);
        r.y = max(r.y, 0.01);
        r = normalize(r);
        vec3 skyDisp = mix(skyReflection(r), fogColor.rgb, litWater0.w);
        vec3 sky = toLinear(skyDisp) * mix(1.0, 0.82, smoothstep(0.15, 0.9, r.y));
        float skyMean = litWater0.z * mix(kSeaFarSkyFloor, 1.0, 1.0 - smoothstep(kSeaFarSkyStart, kSeaFarSkyEnd, dist));
        float wSky = clamp(skyMean * Ff + kSeaSkyVar * (F - Ff), 0.0, 0.95);
        // ---- The sun: a broad path and a tight sparkle on the wave normal
        // (normalised Blinn-Phong, Schlick on v.h; the sparkle widens with the
        // footprint), and the original's sheen, the material's broad lobe in the
        // original sun's hue, on a normal that turns flat (litWater1.w) at a
        // high sun, a smooth wash as the original's per-vertex sheen.
        vec3 h = normalize(l + v);
        float nh = max(dot(n, h), 0.0);
        float vh = max(dot(v, h), 0.0);
        float Fh = 0.02 + 0.98 * pow(1.0 - vh, 5.0);
        float pT = kSeaTightPow / (1.0 + kSeaTightPow * 0.004 * foot);
        const float kInv8Pi = 0.0397887;
        float lobes = kSeaBroadW * (kSeaBroadPow + 8.0) * kInv8Pi * pow(nh, kSeaBroadPow) +
                      kSeaTightW * (pT + 8.0) * kInv8Pi * pow(nh, pT);
        vec3 nS = normalize(mix(n, vec3(0.0, 1.0, 0.0), litWater1.w));
        float nhS = max(dot(nS, h), 0.0);
        float pM = max(litMatSpecular.w, 1.0);
        float sheen = pow(nhS, pM) * (pM + 8.0) / 28.0 * (1.0 + 2.0 * pow(1.0 - vh, 5.0)) *
                      smoothstep(0.0, 0.15, dot(nS, l));
        vec3 sun = (litSun.rgb * (Fh * lobes * smoothstep(0.0, 0.04, l.y)) + litMatSpecular.rgb * litWater1.rgb * sheen) *
                   (sunVis * sunOn * litHorizon.w);
        vec3 radiance = body * (1.0 - wSky) + sky * wSky + sun * exposure;
        vec3 display = toDisplay(tonemapNeutral(radiance));
        float grey = dot(display, kLuma);
        r0.rgb = max(mix(vec3(grey), display, litFlags.w), vec3(0.0));
        r0.a = vColor.a * t0.a * constColor.a;
    } else if (lit) {
        // The debug views (sun visibility, sky visibility, normals) show the
        // sea as the detail program does.
        vec4 t0 = texture(tex0, vUV0);
        r0.rgb = shadeRemastered(t0.rgb * constColor.rgb * (t1.a * 2.0), true, ambientShare);
        r0.a = vColor.a * t0.a * constColor.a;
    } else {
        vec4 t0 = texture(tex0, vUV0);
        r0 = vColor * t0;
        r0 *= constColor;
        r0.rgb *= t1.a * 2.0;
        r0 += vSpecColor;
    }
    finishFragment(r0, ambientShare, true);
}
)";

} // namespace

namespace Poseidon::gl33lighting
{

namespace
{
// The fragment prelude: these pieces in FragmentPiece order. The lighting lab's
// glsl_extract.py reads this table to assemble the same source.
const char* const kFragmentPieces[] = {
    s_psRemasterHeader,         // Header
    kLitConstantsBlockGLSL,     // LitConstants
    s_psRemasterInputs,         // Inputs
    s_psRemasterShadows,        // Shadows
    kRemasterTerrainCloudsGLSL, // TerrainClouds
    kRemasterSkyGLSL,           // Sky
    kRemasterLightsGLSL,        // Lights
    kRemasterSurfaceGLSL,       // Surface
    s_psRemasterShade,          // Shade
    s_psRemasterFinish,         // Finish
};
static_assert(std::size(kFragmentPieces) == static_cast<size_t>(FragmentPiece::Count), "one entry per FragmentPiece");

// The screen vertex shader: these pieces in this order (the lighting lab's
// glsl_extract.py reads this table too).
const char* const kScreenVertexPieces[] = {
    s_vsScreenRemasterHead,
    kLitConstantsBlockGLSL,
    s_vsScreenRemasterMain,
};
} // namespace

const char* RemasterVertexSource(int vs)
{
    switch (vs)
    {
        case VSScreen:
        {
            // Joined once, on first use (thread-safe static).
            static const std::string screen = []
            {
                std::string joined;
                for (const char* piece : kScreenVertexPieces)
                    joined += piece;
                return joined;
            }();
            return screen.c_str();
        }
        case VSTransform:
            return s_vsTransformRemasterGLSL;
        case VSShadow:
            return s_vsShadowRemasterGLSL;
        default:
            return nullptr;
    }
}

const char* RemasterFragmentCommon()
{
    // Joined once, on first use (thread-safe static).
    static const std::string common = []
    {
        std::string joined;
        for (const char* piece : kFragmentPieces)
            joined += piece;
        return joined;
    }();
    return common.c_str();
}

const char* RemasterFragmentPiece(FragmentPiece piece)
{
    const int i = static_cast<int>(piece);
    return i >= 0 && i < static_cast<int>(FragmentPiece::Count) ? kFragmentPieces[i] : nullptr;
}

const char* LitConstantsBlock()
{
    return kLitConstantsBlockGLSL;
}

const char* RemasterFragmentMain(int ps)
{
    switch (ps)
    {
        case PSNormal:
            return s_psNormalRemasterMain;
        case PSDetail:
            return s_psDetailRemasterMain;
        case PSGrass:
            return s_psGrassRemasterMain;
        case PSWater:
            return s_psWaterRemasterMain;
        default:
            return nullptr; // PSFlat / PSShadow reuse the classic sources
    }
}

} // namespace Poseidon::gl33lighting
