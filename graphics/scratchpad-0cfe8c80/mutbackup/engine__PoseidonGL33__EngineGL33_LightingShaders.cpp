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
//   - local lights (headlights, lamps, fires) are evaluated per fragment;
//   - glossy materials get normalised Blinn-Phong with a Fresnel boost and a
//     sky reflection.
// Fog, the night-eye desaturation, alpha test / alpha-to-coverage, the IsColored
// tint and the detail-texture modulation keep their classic order and space.
// Pre-lit geometry (VSScreen: sky, clouds, particles, 2D) passes through
// exactly as in the classic shaders, except under the sky table (feature
// switch `atmosphere`): the sky dome is then shaded from the physically based
// sky table (vLitMode 2, shadeSkyDome) and pre-lit world draws fade into the
// table's horizon colour with their fog (vLitMode -1).
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
    vec4 vpScale;       // 21 — {2/width, 2/height, 0, 0}
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
    vColor = aColor;
    vSpecColor = aSpecular;
    vUV0 = aUV0;
    vUV1 = aUV1;
    vFogTC = aSpecular.a;
    vWorldRel = vec3(0.0);
    vNormalW = vec3(0.0, 1.0, 0.0);
    vCrown = vec4(0.0);
    vLitMode = 0.0; // pre-lit colour: the fragment shader passes it through
    // The sky table: the camera-relative point on the view ray, from the
    // vertex's screen position and the camera basis, perspective-interpolated
    // so the fragment shader gets an exact view direction. The sky dome (sky
    // mode 1) is shaded from the table (vLitMode 2); every other pre-lit draw of
    // the world (the far horizon polygon, the sky clouds, particles) keeps its
    // colour and fades into the table's horizon colour with its fog (-1).
    if (litAtmo.x > 0.0 || litAtmo.z > 0.0) {
        vec2 ndc = vec2(aPos.x * vpScale.x - 1.0, 1.0 - aPos.y * vpScale.y);
        vec3 rel = (litSkyFwd.xyz + ndc.x * litSkyRight.xyz + ndc.y * litSkyUp.xyz) * w;
        if (litSkyFwd.w > 0.5 && litSkyFwd.w < 1.5 && litAtmo.x > 0.0) {
            vWorldRel = rel;
            vLitMode = 2.0;
        } else if (litAtmo.z > 0.0) {
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
    vec4 vpScale;       // c21
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
    vec4 vpScale;       // c21
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

// The LitConstants block (binding 3): the one GLSL declaration of the LitConst
// layout in EngineGL33_Lighting.hpp. Every shader that reads the rig pastes this
// string (LitConstantsBlock()); the layout test checks it slot by slot.
static const char s_glslLitConstantsBlock[] = R"(
// Remastered light rig and per-draw material, binding 3. Layout mirrors
// LitConst in EngineGL33_Lighting.hpp.
layout(std140) uniform LitConstants {
    vec4 litSunDir;       // 0: xyz toward the sun, w = sun enabled (sunEn)
    vec4 litSun;          // 1: rgb sun radiance, w = sun elevation (degrees)
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
    vec4 litLightCount;   // 22: x active local lights
    vec4 litHorizonSun;   // 23: rgb on a vertical surface facing the sun
    vec4 litFogSun;       // 24: rgb sky-dome sun glow added to the fog (display), w enabled
    vec4 litFogCap;       // 25: rgb fog colour limit (the sky texture colour)
    vec4 litLightPos[8];  // 26-33: xyz camera-relative, w start attenuation distance
    vec4 litLightColor[8];// 34-41: rgb diffuse radiance, w = spotlight
    vec4 litLightAmb[8];  // 42-49: rgb ambient radiance
    vec4 litLightDir[8];  // 50-57: xyz beam direction
    vec4 litReserved[3];  // 58-60: allocated to features still to land (materials, temporal, shadows)
    vec4 litSkyRight;     // 61: xyz camera right * tan(fov x / 2), w Mie asymmetry g
    vec4 litSkyUp;        // 62: xyz camera up * tan(fov y / 2)
    vec4 litSkyFwd;       // 63: xyz camera forward, w sky mode (0 world, 1 dome, 2 sky clouds)
    vec4 litAtmo;         // 64: x dome blend, y overcast cloud floor, z aerial perspective blend, w cloud mask dead zone
    vec4 litAtmoCloudSun; // 65: rgb sun light on the sky clouds (linear), w sky light on them (linear)
    vec4 litAtmoGlow;     // 66: rgb classic dome sun glow (display), w sky saturation
    vec4 litAtmoDome;     // 67: rgb classic dome light (display), w hue (0 the original hue relit, 1 physical)
};
)";

// Inputs: samplers, varyings and the colour space and tonemap helpers the
// later pieces use.
static const char s_psRemasterInputs[] = R"(
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2DArrayShadow shadowMap;    // unit 2, comparison sampler bound in remastered mode
uniform sampler2DArray terrainHorizon;     // unit 3: horizon angles (16-bit unorm), 4 directions per layer
uniform sampler2D terrainHeight;           // unit 4: r = height (m), g = sky visibility, ba = ground tint (r, b)
uniform sampler2D cloudNoise;              // unit 5: equalised tileable noise
uniform sampler2D skyLut;                  // unit 8: sky-view table (the sky table; Atmosphere.hpp)

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

// Sun visibility from the cascaded shadow maps: 1 = lit. Same tier selection,
// cross-fade and far fade as the classic shaders, re-anchored to the camera
// position the depth pass used. Casters self-shadow, so their lookup point is
// pushed out along the normal by a fraction of a texel facing the sun and up to
// two texels at grazing sun (no stripes on walls and leaf cards). The terrain
// never casts into the map and passes offsetScale 0, and up-facing surfaces fade
// the offset out: an offset there would slide every shadow away from its caster
// by metres at a low sun.
float shadowMapVisibility(vec3 n, float nl, float offsetScale) {
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
        vec4 cp = cascadeVP[c] * vec4(rel + n * offset, 1.0);
        vec3 sc = cp.xyz / cp.w;
        vec2 suv = sc.xy * 0.5 + 0.5;
        if (suv.x > 0.0 && suv.x < 1.0 && suv.y > 0.0 && suv.y < 1.0 && sc.z > 0.0 && sc.z < 1.0) {
            float bias = cascadeCtl.z * float(c + 1) * float(c + 1);
            // Looser tiers cover more ground per texel: shrink their radius so
            // the penumbra width in metres stays comparable across tiers.
            float radius = ts * litShadowDelta.w / float(c + 1);
            float lit = 0.0;
            for (int k = 0; k < 12; ++k)
                lit += texture(shadowMap, vec4(suv + kPoisson[k] * radius, float(c), sc.z - bias));
            litSum += w * (lit / 12.0);
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

// Terrain and clouds: the sun's visibility past the hills and under the broken
// cloud field, the valley sky visibility and the ground-bounce tint.
static const char s_psRemasterTerrainClouds[] = R"(
// Colour of the light the ground below bounces (luminance 1; white when there
// is no map). Set by terrainSunVisibility, read by skyIrradiance.
vec3 g_groundTint = vec3(1.0);

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
    float lift = smoothstep(litTerrainLift.x, litTerrainLift.y, posAbs.y - hs.x);
    skyVisibility = mix(1.0, mix(hs.g, 1.0, lift), litTerrain.w);
    float h0 = dot(texture(terrainHorizon, vec3(uv, litTerrainSun.x)), litTerrainSel0);
    float h1 = dot(texture(terrainHorizon, vec3(uv, litTerrainSun.y)), litTerrainSel1);
    float horizonDeg = mix(-5.0, 45.0, mix(h0, h1, litTerrainSun.z)); // TerrainHorizonMap::kMin/kMaxAngleDeg
    float soft = litTerrainSun.w;
    float vis = smoothstep(horizonDeg - soft, horizonDeg + soft, litSun.w);
    return mix(1.0, mix(vis, 1.0, lift), litTerrainLift.z);
}

// Broken-cloud shadows: the ground point is projected along the sun ray up to
// the cloud deck and looked up in a drifting noise field whose covered fraction
// matches the overcast. The noise is histogram-equalised and read once, and
// the soft edge is symmetric about the threshold and narrows toward a clear or
// a fully covered sky, so the covered fraction is exactly the overcast (no sun
// patches under a full overcast, no stray shadows under a clear sky).
float cloudSunVisibility(vec3 posAbs) {
    if (litCloud.w < 0.5 || litFlags.y < 0.5) return 1.0;
    float up = max(litSunDir.y, 0.12);
    vec2 p = posAbs.xz + litSunDir.xz / up * (litCloudOffset.z - posAbs.y);
    float n = texture(cloudNoise, (p + litCloudOffset.xy) * litCloud.z).r;
    float cover = clamp(litCloud.x, 0.0, 1.0);
    float edge = litCloudOffset.w * clamp(4.0 * cover * (1.0 - cover), 0.0, 1.0);
    float covered = edge > 1e-4 ? smoothstep(1.0 - cover - edge, 1.0 - cover + edge, n) : step(1.0 - cover, n);
    return mix(1.0, litCloud.y, covered);
}
)";

// Sky, local lights and surface are in EngineGL33_LightingShaders{Sky,Lights,Surface}.cpp.

// Shade: one fragment's light (sun, sky, moon, local lights, emissive, gloss),
// tonemapped.
static const char s_psRemasterShade[] = R"(
// Sun visibility of the last shaded fragment (water reuses it for its glint).
float g_sunVisibility = 1.0;

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
    float crownSky = 1.0;
    if (crown) {
        vec3 card = dot(n, cn) < 0.0 ? -n : n;
        vec3 blend = card * 0.3 + cn * 0.7;
        nLight = blend * inversesqrt(max(dot(blend, blend), 1e-12));
        crownSky = mix(0.6, 1.0, smoothstep(0.1, 0.95, vCrown.w));
    } else if (foliage) {
        nLight = nCast;
    }

    float nl = max(dot(n, l), 0.0);
    float nlCast = max(dot(nCast, l), 0.0);
    float skyVisibility;
    float sunVisibility = shadowMapVisibility(nCast, nlCast, terrain ? 0.0 : 1.0) *
                          terrainSunVisibility(posAbs, skyVisibility) * cloudSunVisibility(posAbs);
    g_sunVisibility = sunVisibility;

    vec3 kd = litMatDiffuse.rgb;
    vec3 ka = litMatAmbient.rgb;
    float sunTerm = nl;
    float sunShadow = sunVisibility;
    if (foliage) {
        sunTerm = clamp((dot(nLight, l) + 0.35) / 1.35, 0.0, 1.0);
        sunShadow = mix(0.45, 1.0, sunVisibility);
    }
    vec3 direct = litSun.rgb * (kd * sunTerm + litMatForced.rgb) * (sunShadow * sunOn);
    vec3 ambient = ka * skyIrradiance(nLight) * (skyVisibility * crownSky * sunOn);
    vec3 moon = kd * litMoon.rgb * (max(dot(nLight, litMoonDir.xyz), 0.0) * sunOn);
    // Classic adds local lights in display space before texturing and clamps
    // the sum at 1; taking that sum to linear keeps the authored size and
    // falloff of headlight and lamp pools (a linear sum would make them about
    // twice as wide). The same clamp keeps a lamp-lit surface from getting
    // brighter than the original's, where overlapping pools sum past 1.
    vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.0)));
    vec3 emissive = toLinear(min(litMatEmissive.rgb, vec3(1.5)));

    vec3 a = toLinear(albedo);
    float exposure = litZenith.w;
    vec3 radiance = (a * (direct + ambient + moon + local + emissive) + glossy(n, v, sunVisibility) * sunOn) * exposure;
    float total = dot(radiance, kLuma);
    ambientShare = clamp(dot(a * ambient * exposure, kLuma) / max(total, 1e-5), 0.0, 1.0);
    if (foliage) ambientShare *= 0.5;

    if (litFlags.z > 0.5) {
        // Debug views: 1 = sun visibility, 2 = terrain sky visibility, 3 = normal.
        if (litFlags.z < 1.5) return vec3(sunVisibility);
        if (litFlags.z < 2.5) return vec3(skyVisibility);
        return nLight * 0.5 + 0.5;
    }
    vec3 display = toDisplay(tonemapNeutral(radiance));
    float grey = dot(display, kLuma);
    return max(mix(vec3(grey), display, litFlags.w), vec3(0.0));
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
    vec3 fogC = fogColor.rgb;
    if (vLitMode > 0.5 && vLitMode < 1.5) {
        fogC = fogColorToward(vWorldRel);
        if (litAtmo.z > 0.0) fogC = atmoFogColor(vWorldRel, fogC);
    } else if (vLitMode < -0.5 && litAtmo.z > 0.0 && vFogTC < 0.999) {
        fogC = atmoFogColor(vWorldRel, fogC);
    }
    r0.rgb = mix(fogC, r0.rgb, vFogTC);
    fragColor = alphaRef.w > 0.5 ? vec4(1.0, 0.0, 0.0, 1.0) : r0;
}
)";

static const char s_psNormalRemasterMain[] = R"(
void main() {
    vec4 t0 = texture(tex0, vUV0);
    vec4 r0;
    float ambientShare = 0.0;
    if (vLitMode > 1.5) {
        // The sky dome (the sky table), over its pre-lit classic colour.
        vec4 classic = vColor * t0;
        classic *= constColor;
        classic.rgb += vSpecColor.rgb;
        r0 = vec4(shadeSkyDome(t0, classic.rgb), classic.a);
    } else if (vLitMode > 0.5) {
        r0.rgb = shadeRemastered(t0.rgb * constColor.rgb, false, ambientShare);
        r0.a = vColor.a * t0.a * constColor.a;
    } else {
        r0 = vColor * t0;
        r0 *= constColor;
        r0.rgb += vSpecColor.rgb;
    }
    finishFragment(r0, ambientShare, true);
}
)";

static const char s_psDetailRemasterMain[] = R"(
void main() {
    vec4 t0 = texture(tex0, vUV0);
    vec4 t1 = texture(tex1, vUV1);
    vec4 r0;
    float ambientShare = 0.0;
    if (vLitMode > 0.5 && vLitMode < 1.5) {
        // Detail modulation stays in display space, as in the classic shader,
        // so the terrain keeps its authored micro-contrast.
        vec3 albedo = t0.rgb * constColor.rgb * (t1.a * 2.0);
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
void main() {
    vec4 t0 = texture(tex0, vUV0);
    vec4 t1 = texture(tex1, vUV1);
    vec3 bumpNormal = -(t1.xyz * 2.0 - 1.0);
    float glint = clamp(dot(psC4.xyz, bumpNormal), 0.0, 1.0);
    vec4 r0;
    bool lit = vLitMode > 0.5 && vLitMode < 1.5;
    if (lit) {
        float ambientShare;
        vec3 base = shadeRemastered(t0.rgb, true, ambientShare);
        // The water reflects the sky the player sees behind it (the fog colour
        // with its sun glow, so the low sun lays a bright path across the sea),
        // rising toward grazing angles (Fresnel). The classic bump glint becomes
        // a tight HDR sparkle that clouds and shadows put out.
        vec3 v = -vWorldRel * inversesqrt(max(dot(vWorldRel, vWorldRel), 1e-12));
        float fresnel = 0.02 + 0.98 * pow(1.0 - clamp(v.y, 0.0, 1.0), 5.0);
        vec3 r = reflect(-v, vec3(0.0, 1.0, 0.0));
        vec3 sky = toLinear(fogColorToward(vec3(r.x, 0.0, r.z)));
        vec3 sparkle = litSun.rgb * pow(glint, 12.0) * 6.0 * (litSunDir.w * g_sunVisibility);
        vec3 radiance = mix(toLinear(base), sky, fresnel * 0.6) + sparkle * litZenith.w;
        r0 = vec4(toDisplay(tonemapNeutral(radiance)), vColor.a * t0.a);
    } else {
        r0 = vColor * t0;
        r0.rgb += glint;
    }
    // The sea's own fog: as finishFragment's (the reflection term above is the
    // water step's to change).
    vec3 fogC = fogColor.rgb;
    if (lit) {
        fogC = fogColorToward(vWorldRel);
        if (litAtmo.z > 0.0) fogC = atmoFogColor(vWorldRel, fogC);
    } else if (vLitMode < -0.5 && litAtmo.z > 0.0 && vFogTC < 0.999) {
        fogC = atmoFogColor(vWorldRel, fogC);
    }
    r0.rgb = mix(fogC, r0.rgb, vFogTC);
    fragColor = alphaRef.w > 0.5 ? vec4(1.0, 0.0, 0.0, 1.0) : r0;
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
    s_psRemasterHeader,        // Header
    s_glslLitConstantsBlock,   // LitConstants
    s_psRemasterInputs,        // Inputs
    s_psRemasterShadows,       // Shadows
    s_psRemasterTerrainClouds, // TerrainClouds
    kRemasterSkyGLSL,          // Sky
    kRemasterLightsGLSL,       // Lights
    kRemasterSurfaceGLSL,      // Surface
    s_psRemasterShade,         // Shade
    s_psRemasterFinish,        // Finish
};
static_assert(std::size(kFragmentPieces) == static_cast<size_t>(FragmentPiece::Count), "one entry per FragmentPiece");

// The screen vertex shader: these pieces in this order (the lighting lab's
// glsl_extract.py reads this table too).
const char* const kScreenVertexPieces[] = {
    s_vsScreenRemasterHead,
    s_glslLitConstantsBlock,
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
    return s_glslLitConstantsBlock;
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
