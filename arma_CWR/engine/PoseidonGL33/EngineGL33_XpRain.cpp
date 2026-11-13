// Rain you can see (perf-squad lane EX4-RAIN, plan item R-9 (b); the switch
// POSEIDON_XP_RAIN=world, Graphics/Remaster/RainXp.hpp). Off by default: with
// the switch unset no program is built and nothing here runs.
//
// The rain overlay (Scene::DrawRain) is a screen texture: it does not stand in
// the world, nothing hides it, no lamp lights it, and under TAA its moving
// streaks average into a haze. Here the rain is drops in the world instead:
//   - a box of drops round the camera (kRainBoxSize, its centre kBoxAhead
//     metres ahead along the view's heading), each drop a fixed point of a
//     lattice of such boxes that has travelled with its layer (RainDriftStep:
//     four layers, each with its own fall speed and wind share, their travel
//     summed frame by frame), folded into the box (RainWrapShift, double
//     precision on the CPU), so the drops stay where they are in the world
//     while the camera moves and the box follows it; drops fade out over the
//     last metres at the box's faces;
//   - one instanced draw of camera-facing quads, each the streak a drop draws
//     over the camera's shutter (kShutter) in its motion relative to the camera
//     (so a moving camera slants the rain), brightest at its head (the drop,
//     the lower end) and fading up its trail, as wide as the drop but never
//     under kMinWidthPx (its true width in the opacity), its edges and ends
//     the exact share of each pixel the line covers (so a streak keeps its
//     light whatever its place between pixels), fainter the longer its trail;
//   - lit as a water drop: the sky's light all round it, the key (the sun or
//     the moon) and the frame's street lamps and low beams (the physical
//     model's intensities, FrameLights.hpp) scattered forward (a
//     Henyey-Greenstein lobe), so rain shows bright where it falls through a
//     lamp's light or in front of the low sun, and hardly at all against a
//     bright sky; at the camera's exposure, white balance and tone curve (the
//     composite's, PhotoCamera.hpp), so it sits in the picture at its light;
//   - faded by the fog at its depth (the scene's fog range, as the shafts read
//     it), hidden where the world's depth is nearer than the drop, and faded
//     behind smoke and fire puffs (RainCoverGrid: a coarse screen grid of the
//     frame's puffs, their cover and depth, filled as they are drawn);
//   - drawn into the frame target after the TAA resolve and sharpen (the
//     history never sees it, so its streaks stay crisp), before the HUD.
// It draws nothing the fog or AI sight reads: the drawn fog is unchanged.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/EngineGL33_PostPass.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Remaster/NightLights.hpp>
#include <Poseidon/World/Scene/Scene.hpp>

#include <algorithm>
#include <cmath>
#include <string>

using namespace Poseidon::gl33post;
namespace rm = Poseidon::remaster;
namespace fl = Poseidon::lighting;

namespace
{

// The drop volume (rm::kRainBoxSize: m, east, up, north): its centre ahead of
// the camera along the view's heading, and how far below the camera it reaches.
constexpr const float* kBoxSize = rm::kRainBoxSize;
constexpr float kBoxAhead = 12.0f;
constexpr float kBoxBelow = 9.0f;
// Drops in the box at rain density 1 (about 1.25 per cubic metre: the streaks
// a camera resolves, not every drop in the air).
constexpr int kMaxDrops = 48000;
// The camera's shutter (s): a drop draws the streak it falls over this time.
constexpr float kShutter = 1.0f / 36.0f;
// The narrowest streak drawn (px): a thinner drop is drawn this wide and that
// much fainter, so every streak covers whole pixels' worth and does not
// sparkle as it crosses them.
constexpr float kMinWidthPx = 1.3f;
// A drop's diameter scale (m; each drop 0.6 to 1.4 of it) and the opacity of a
// drop kMinWidthPx wide.
constexpr float kDropDiameter = 0.003f;
constexpr float kOpacity = 0.9f;
// A drop's light from the sky round it, against the sky's mean radiance (the
// mean takes in the dark ground; a drop shows mostly the bright sky above and
// round it, so it is a little brighter: at 1 the streaks read as dark lines on
// an overcast sky).
constexpr float kDropSky = 1.25f;
// The forward lobes (Henyey-Greenstein g) of the key and of the local lights.
constexpr float kKeyLobe = 0.6f;
constexpr float kLampLobe = 0.55f;
// Local lights the drops take (the frame's strongest lamps and low beams).
constexpr int kRainLights = 8;

const char kXpRainVS[] = R"(#version 330 core
uniform vec4 rnBox;      // xyz the box's size (m)
uniform vec4 rnMin;      // xyz the box's minimum corner from the camera (m), w the shutter (s)
uniform vec4 rnShift[4]; // per layer: xyz the wrap shift (m)
uniform vec4 rnVel[4];   // per layer: xyz the drops' velocity relative to the camera (m/s), w the layer's size
uniform vec4 rnMisc;     // x the opacity of a drop of the least width, y the fog's extinction (1/m), z the least width (px)
uniform sampler2D coverTex; // unit 3: the smoke grid (r cover, g cover times depth in m), rows top down
uniform vec4 rnCover;    // x 1 = the grid is in use
uniform vec4 rnRect;     // the 3D viewport in the frame (x, y, w, h; 0..1, y up)
uniform vec4 rnViewX;    // camera-relative world to view: view = (dot(p, X), dot(p, Y), dot(p, Z))
uniform vec4 rnViewY;
uniform vec4 rnViewZ;
uniform vec4 rnProj;     // x _11, y _22, zw the 3D viewport (px)
uniform vec4 rnSky;      // rgb the sky's light on a drop (scene light at the exposure), w the drop diameter (m)
uniform vec4 rnKey;      // rgb the key's light at the exposure, w its lobe g
uniform vec4 rnToward;   // xyz toward the key (world), w the lights' lobe g
uniform vec4 rnLightPos[8]; // xyz camera-relative (m), w the intensity (the physical model's peak)
uniform vec4 rnLightCol[8]; // rgb linear colour at the exposure, w 1 = a low beam
uniform vec4 rnLightDir[8]; // xyz the low beam's axis, w the reach squared (m^2)
uniform vec4 rnCount;    // x lights
uniform vec4 camParams;  // z the curve (as the composite's)
uniform vec4 camCurve;
uniform vec4 camNight;   // z the curve's white, w stops past it to white
uniform vec4 camColor0;
uniform vec4 camColor1;
uniform vec4 camColor2;
out vec4 vColor;   // rgb display, a opacity
out vec4 vStreak;  // px: x across from the axis, y along from the head, z the streak's length, w its half width
out float vViewZ;

uint hashU(uint x) {
    x ^= x >> 16; x *= 0x7feb352du;
    x ^= x >> 15; x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}
float h01(uint x) { return float(hashU(x) & 0xffffffu) / 16777216.0; }

// Henyey-Greenstein, times 4 pi (1 on average over the sphere).
float hg(float c, float g) {
    float d = 1.0 + g * g - 2.0 * g * c;
    return (1.0 - g * g) / (d * sqrt(d));
}

// The low beam (FrameLights.hpp LowBeamPattern, the lit shaders' lowBeamPattern).
float lowBeamPattern(vec3 L, vec3 beam) {
    float c = dot(L, beam);
    if (c <= 0.0) return 0.0;
    vec3 right = vec3(beam.z, 0.0, -beam.x) * inversesqrt(max(beam.x * beam.x + beam.z * beam.z, 1e-12));
    vec3 u = cross(beam, right);
    float ah = atan(dot(L, right), c);
    float av = atan(dot(L, u), c);
    float cut = 1.0 - smoothstep(-0.005, 0.025, av);
    float hv = (av + 0.03) / 0.05;
    float vert = exp(-hv * hv) + 0.02 * smoothstep(-0.6, -0.1, av);
    float h1 = ah / 0.25;
    float h2 = ah / 0.6;
    float horiz = exp(-h1 * h1) + 0.15 * exp(-h2 * h2);
    return vert * horiz * cut;
}

// The composite's tone curve (EngineGL33_PostShaders.cpp camCurveY, camTone).
float camCurveY(float x) {
    x = max(x, 0.0);
    return clamp(pow(x, camCurve.x) / (pow(x, camCurve.x * camCurve.y) * camCurve.z + camCurve.w), 0.0, 1.0);
}
vec3 camTone(vec3 x) {
    int kind = int(camParams.z + 0.5);
    if (kind == 1) return vec3(camCurveY(x.r), camCurveY(x.g), camCurveY(x.b));
    if (kind == 2) {
        x *= 0.6;
        return clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
    }
    if (kind == 3) {
        float lo = min(x.r, min(x.g, x.b));
        x -= lo < 0.08 ? lo - 6.25 * lo * lo : 0.04;
        float pk = max(x.r, max(x.g, x.b));
        if (pk < 0.76) return x;
        float newPeak = 1.0 - 0.0576 / (pk - 0.52);
        x *= newPeak / pk;
        return mix(x, vec3(newPeak), 1.0 - 1.0 / (0.15 * (pk - newPeak) + 1.0));
    }
    float peak = max(x.r, max(x.g, x.b));
    if (peak <= 0.0) return vec3(0.0);
    float y = camCurveY(peak);
    vec3 ratio = x / peak;
    float excess = clamp(log2(max(peak / camNight.z, 1.0)) / camNight.w, 0.0, 1.0);
    float chroma = 1.0 - min(ratio.r, min(ratio.g, ratio.b));
    float whiten = max(pow(y, 8.0), excess) * (1.0 - 0.8 * chroma * chroma);
    vec3 w = pow(vec3(max(whiten, 1e-6)), 1.0 + 3.0 * (1.0 - ratio));
    return mix(ratio, vec3(1.0), w) * y;
}
vec3 cameraDisplay(vec3 x) {
    x = max(vec3(dot(camColor0.xyz, x), dot(camColor1.xyz, x), dot(camColor2.xyz, x)), vec3(0.0));
    return pow(clamp(camTone(x), 0.0, 1.0), vec3(1.0 / 2.2));
}

vec3 toView(vec3 p) { return vec3(dot(p, rnViewX.xyz), dot(p, rnViewY.xyz), dot(p, rnViewZ.xyz)); }

void main() {
    const float kNear = 0.3;
    const float kScatter = 0.1; // the share a drop scatters (0.3 / pi)
    uint id = uint(gl_InstanceID);
    vec3 base = vec3(h01(id * 5u + 1u), h01(id * 5u + 2u), h01(id * 5u + 3u)) * rnBox.xyz;
    float rSize = h01(id * 5u + 4u);
    float rLight = h01(id * 5u + 5u);
    int layer = int(id & 3u);
    vec3 vel = rnVel[layer].xyz;
    vec3 rel = rnMin.xyz + mod(base + rnShift[layer].xyz, rnBox.xyz);
    vec3 e = min(rel - rnMin.xyz, rnMin.xyz + rnBox.xyz - rel);
    float edge = smoothstep(0.0, 3.0, min(e.x, min(e.y, e.z)));
    // The head is the drop now (the lower end of a falling streak), the tail
    // where it was a shutter ago.
    vec3 tail = rel - vel * rnMin.w;
    vec3 vh = toView(rel);
    vec3 vt = toView(tail);
    vColor = vec4(0.0);
    vStreak = vec4(0.0);
    vViewZ = 0.0;
    if ((vh.z < kNear && vt.z < kNear) || edge <= 0.0) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        return;
    }
    if (vh.z < kNear) vh = vt + (vh - vt) * ((kNear - vt.z) / (vh.z - vt.z));
    if (vt.z < kNear) vt = vh + (vt - vh) * ((kNear - vh.z) / (vt.z - vh.z));
    vec2 sh = (vh.xy * rnProj.xy / vh.z * 0.5 + 0.5) * rnProj.zw;
    vec2 st = (vt.xy * rnProj.xy / vt.z * 0.5 + 0.5) * rnProj.zw;
    vec2 dv = st - sh;
    float len = length(dv);
    vec2 d = len > 1e-3 ? dv / len : vec2(0.0, 1.0);
    vec2 n = vec2(-d.y, d.x);
    float zMid = 0.5 * (vh.z + vt.z);
    float px = 2.0 * zMid / (rnProj.y * rnProj.w); // metres per pixel at the streak
    float wPx = rnSky.w * (0.6 + 0.8 * rSize) * rnVel[layer].w / px;
    float cover = min(wPx / rnMisc.z, 1.0);
    float halfW = 0.5 * max(wPx, rnMisc.z);
    float trail = length(rel - tail);
    float trailKeep = clamp(0.2 / max(trail, 1e-3), 0.25, 1.0);
    int corner = gl_VertexID;
    float along = (corner & 1) == 0 ? 0.0 : 1.0;
    float across = (corner & 2) == 0 ? -1.0 : 1.0;
    // The quad reaches a pixel past the line all round, for its soft edge.
    float ext = halfW + 1.0;
    vec2 p = mix(sh - d, st + d, along) + n * (across * ext);
    gl_Position = vec4(p / rnProj.zw * 2.0 - 1.0, 0.0, 1.0);
    vStreak = vec4(across * ext, along > 0.5 ? len + 1.0 : -1.0, len, halfW);
    vViewZ = mix(vh.z, vt.z, along);

    // Smoke and fire puffs in front of the drop hide it by what they cover.
    float smoke = 1.0;
    if (rnCover.x > 0.5) {
        vec2 mid = rnRect.xy + (0.5 * (sh + st) / rnProj.zw) * rnRect.zw;
        vec2 c = textureLod(coverTex, vec2(mid.x, 1.0 - mid.y), 0.0).rg;
        float cz = c.g / max(c.r, 1e-4);
        smoke = 1.0 - clamp(c.r, 0.0, 1.0) * smoothstep(cz - 0.5, cz + 1.5, zMid);
    }

    float dist = length(rel);
    vec3 v = rel / max(dist, 1e-3);
    vec3 L = rnSky.rgb + rnKey.rgb * (hg(dot(v, rnToward.xyz), rnKey.w) * kScatter);
    int count = int(rnCount.x + 0.5);
    for (int i = 0; i < 8; ++i) {
        if (i >= count) break;
        vec3 toL = rnLightPos[i].xyz - rel;
        float d2 = dot(toL, toL);
        float reach2 = rnLightDir[i].w;
        if (d2 >= reach2) continue;
        float dl = sqrt(max(d2, 1e-8));
        vec3 Ld = toL / dl;
        float I = rnLightPos[i].w;
        if (rnLightCol[i].w > 0.5) I *= lowBeamPattern(-Ld, rnLightDir[i].xyz);
        I *= (1.0 - smoothstep(0.5 * reach2, reach2, d2)) * smoothstep(0.25, 1.0, dl);
        L += rnLightCol[i].rgb * (I / (d2 + 0.25) * hg(dot(v, Ld), rnToward.w) * kScatter);
    }
    float T = exp(-dist * rnMisc.y);
    vec3 col = cameraDisplay(mix(rnSky.rgb, L, T));
    float nearFade = smoothstep(kNear, 1.2, zMid);
    // sqrt: the drops a pixel does not resolve still add up along its streak
    float a = rnMisc.x * sqrt(cover) * trailKeep * (0.55 + 0.45 * rLight) * edge * nearFade * T * smoke;
    vColor = vec4(col, a);
}
)";

const char kXpRainFS[] = R"(#version 330 core
in vec4 vColor;
in vec4 vStreak;
in float vViewZ;
uniform sampler2D depthTex; // unit 2: the world's depth (resolved)
uniform vec4 projInfo;      // x = _33, y = _43 (viewZ = y / (ndcZ - x))
uniform vec4 depthMap;      // ndcZ = depth * x + y
out vec4 fragColor;
void main() {
    float d = texelFetch(depthTex, ivec2(gl_FragCoord.xy), 0).r;
    float zs = d >= 0.999999 ? 1e6 : projInfo.y / ((d * depthMap.x + depthMap.y) - projInfo.x);
    // Hidden behind whatever stands nearer (a soft edge a few centimetres deep).
    float occ = clamp((zs - vViewZ) / (0.05 + 0.01 * vViewZ), 0.0, 1.0);
    // The share of this pixel the line covers: across its width (a line
    // vStreak.w either side of the axis) and at its two ends.
    float across = clamp(vStreak.w + 0.5 - abs(vStreak.x), 0.0, 1.0);
    float ends = clamp(vStreak.y + 0.5, 0.0, 1.0) * clamp(vStreak.z - vStreak.y + 0.5, 0.0, 1.0);
    // Brightest at the head (the drop), fading up the trail.
    float s = clamp(vStreak.y / max(vStreak.z, 1e-3), 0.0, 1.0);
    float body = mix(1.0, 0.4, smoothstep(0.0, 1.0, s));
    float a = vColor.a * across * ends * body * occ;
    if (a < 1.0 / 1024.0) discard;
    fragColor = vec4(vColor.rgb, a);
}
)";

GLuint CompileXpRain()
{
    auto compile = [](GLenum kind, const char* src)
    {
        GLuint s = glCreateShader(kind);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            char log[2048] = {};
            glGetShaderInfoLog(s, sizeof(log), nullptr, log);
            LOG_ERROR(Graphics, "GL33 xp rain: {} shader failed: {}", kind == GL_VERTEX_SHADER ? "vertex" : "fragment",
                      log);
            glDeleteShader(s);
            return 0u;
        }
        return s;
    };
    const GLuint vs = compile(GL_VERTEX_SHADER, kXpRainVS);
    const GLuint fs = compile(GL_FRAGMENT_SHADER, kXpRainFS);
    GLuint prog = 0;
    if (vs && fs)
    {
        prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glLinkProgram(prog);
        GLint ok = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            char log[2048] = {};
            glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
            LOG_ERROR(Graphics, "GL33 xp rain: link failed: {}", log);
            glDeleteProgram(prog);
            prog = 0;
        }
    }
    if (vs)
        glDeleteShader(vs);
    if (fs)
        glDeleteShader(fs);
    return prog;
}

} // namespace

bool EngineGL33::RainXpWorldReady() const
{
    if (rm::RainXpModeFromEnv() != rm::RainXpMode::World)
        return false;
    const std::uint32_t frame = GetFrameCounter();
    return _glContext && _worldEffectsOn && !_portraitTargetSize && _lightingActive && _lighting.postEffects &&
           SSAAActive() && !_post.unavailable && _camera.frameOn && _camera.frame == frame &&
           _xpRain.lightFrame == frame && !(_xpRain.tried && !_xpRain.prog) && !_worldDepthCleared;
}

void EngineGL33::SetRainXpFrame(const rm::RainXpFrame& rain)
{
    _xpRain.scene = rain;
}

void EngineGL33::DestroyXpRain()
{
    if (_xpRain.prog)
        glDeleteProgram(_xpRain.prog);
    _xpRain.prog = 0;
    _xpRain.tried = false;
    if (_xpRain.coverTex)
        glDeleteTextures(1, &_xpRain.coverTex);
    _xpRain.coverTex = 0;
}

void EngineGL33::NoteRainCover(float x, float y, float sx, float sy, float opacity, float depthM)
{
    const std::uint32_t frame = GetFrameCounter();
    // Only while the world-space rain draws: the scene's rain of this frame or
    // the last (the puffs come before the scene's rain each frame).
    if (frame - _xpRain.scene.frame > 1u || _xpRain.scene.density < 0.01f)
        return;
    if (_xpRain.coverFrame != frame)
    {
        _xpRain.coverFrame = frame;
        _xpRain.cover.Clear();
    }
    _xpRain.cover.Add(x, y, sx, sy, static_cast<float>(_w), static_cast<float>(_h), opacity, depthM);
}

void EngineGL33::PostXpRain(const PostFrame& f, bool nightLights)
{
    const std::uint32_t frame = GetFrameCounter();
    const rm::RainXpFrame& rain = _xpRain.scene;
    if (!f.camera || _xpRain.lightFrame != frame || _worldDepthCleared || rain.density < 0.01f || !GScene)
        return;
    if (!_xpRain.tried)
    {
        _xpRain.tried = true;
        _xpRain.prog = CompileXpRain();
        LOG_INFO(Graphics, "GL33 xp rain: world-space rain program {}", _xpRain.prog ? "built" : "FAILED");
    }
    const GLuint prog = _xpRain.prog;
    if (!prog)
        return;

    // The box: ahead of the camera along the view's heading (world east, north).
    const GfxMatrix& v = _frameState.view;
    // The view's forward axis in world terms is the third column of R (view.z = dot(p, (_13, _23, _33))).
    float fx = v._13, fz = v._33;
    const float fl2 = fx * fx + fz * fz;
    if (fl2 > 1e-6f)
    {
        const float k = 1.0f / std::sqrt(fl2);
        fx *= k;
        fz *= k;
    }
    else
    {
        fx = 0.0f;
        fz = 0.0f;
    }
    const float boxMin[3] = {fx * kBoxAhead - 0.5f * kBoxSize[0], -kBoxBelow, fz * kBoxAhead - 0.5f * kBoxSize[2]};
    // Each layer's wrap shift from its travel (RainDriftStep), and its
    // velocity relative to the camera for the streak.
    float shift[rm::kRainLayers * 4] = {};
    float relVel[rm::kRainLayers * 4] = {};
    for (int l = 0; l < rm::kRainLayers; ++l)
    {
        rm::RainWrapShift(rain.camPos, rain.travel[l], boxMin, kBoxSize, shift + l * 4);
        for (int i = 0; i < 3; ++i)
            relVel[l * 4 + i] = rain.relVel[l][i];
        // Big drops fall faster: the layer's drop size goes with its fall speed.
        relVel[l * 4 + 3] = rm::kRainLayerFall[l] * rm::kRainLayerFall[l];
    }

    // The light at the exposure: scene light times the rig's exposure and the
    // camera's (pre-exposure and the rest).
    const float exposure = _xpRain.rigExposure * std::exp2(_camera.exposureLog2);
    const float* sky = _xpRain.sky;
    const float* key = _xpRain.key;
    // The frame's strongest lamps and low beams (the physical model, as the
    // lit shaders and ParticleNightLight take them).
    float lightPos[kRainLights * 4] = {};
    float lightCol[kRainLights * 4] = {};
    float lightDir[kRainLights * 4] = {};
    int nLights = 0;
    if (nightLights && _lightTiles.frame == frame)
    {
        const rm::NightConstants night = rm::ComputeNightConstants(_lighting, _featureGates, true);
        const float nightFactor = GScene->MainLight() ? GScene->MainLight()->NightEffect() : 1.0f;
        const float scale = exposure * nightFactor * _xpRain.localScale;
        for (const fl::FrameLight& l : _lightTiles.lights)
        {
            if (nLights >= kRainLights)
                break;
            if (l.hdr || !fl::UsesPhysicalModel(l.kind))
                continue;
            const bool spot = l.IsSpot();
            float peak = spot ? night.night[2] : night.night[1] * l.start;
            if (!spot)
                peak *= rm::kCameraLampGain;
            const float reach2 =
                std::min(fl::kRangeFactor * fl::kRangeFactor * l.start * l.start, peak / fl::kPhysicalMinIrradiance);
            float col[3];
            fl::PhysicalColour(l, col);
            float* p = lightPos + nLights * 4;
            float* c = lightCol + nLights * 4;
            float* d = lightDir + nLights * 4;
            p[0] = l.pos[0];
            p[1] = l.pos[1];
            p[2] = l.pos[2];
            p[3] = peak;
            c[0] = col[0] * scale;
            c[1] = col[1] * scale;
            c[2] = col[2] * scale;
            c[3] = spot ? 1.0f : 0.0f;
            d[0] = l.dir[0];
            d[1] = l.dir[1];
            d[2] = l.dir[2];
            d[3] = reach2;
            ++nLights;
        }
    }
    const float fogRange = std::max(GScene->GetFogMaxRange(), 50.0f);
    const int drops = std::max(0, std::min(kMaxDrops, static_cast<int>(rain.density * kMaxDrops + 0.5f)));
    if (drops <= 0)
        return;

    // The 3D viewport in pixels.
    const int vx = static_cast<int>(f.rect[0] * f.w + 0.5f);
    const int vy = static_cast<int>(f.rect[1] * f.h + 0.5f);
    const int vw = std::max(1, static_cast<int>(f.rect[2] * f.w + 0.5f));
    const int vh = std::max(1, static_cast<int>(f.rect[3] * f.h + 0.5f));

    glUseProgram(prog);
    Set4(prog, "rnBox", kBoxSize[0], kBoxSize[1], kBoxSize[2], 0.0f);
    Set4(prog, "rnMin", boxMin[0], boxMin[1], boxMin[2], kShutter);
    if (const GLint loc = glGetUniformLocation(prog, "rnShift"); loc >= 0)
        glUniform4fv(loc, rm::kRainLayers, shift);
    if (const GLint loc = glGetUniformLocation(prog, "rnVel"); loc >= 0)
        glUniform4fv(loc, rm::kRainLayers, relVel);
    Set4(prog, "rnMisc", kOpacity, 3.0f / fogRange, kMinWidthPx, 0.0f);
    // The smoke grid, when this frame's puffs filled it.
    bool coverOn = false;
    if (_xpRain.coverFrame == frame && _xpRain.cover.puffs > 0)
    {
        static float texels[rm::kRainCoverW * rm::kRainCoverH * 2];
        for (int i = 0; i < rm::kRainCoverW * rm::kRainCoverH; ++i)
        {
            float c, z;
            _xpRain.cover.Cell(i, c, z);
            texels[i * 2] = c;
            texels[i * 2 + 1] = c * z;
        }
        const bool fresh = !_xpRain.coverTex;
        if (fresh)
            glGenTextures(1, &_xpRain.coverTex);
        BindTex(3, _xpRain.coverTex);
        if (fresh)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, rm::kRainCoverW, rm::kRainCoverH, 0, GL_RG, GL_FLOAT, texels);
        }
        else
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rm::kRainCoverW, rm::kRainCoverH, GL_RG, GL_FLOAT, texels);
        }
        coverOn = true;
    }
    SetInt(prog, "coverTex", 3);
    Set4(prog, "rnCover", coverOn ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
    Set4(prog, "rnRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    Set4(prog, "rnViewX", v._11, v._21, v._31, 0.0f);
    Set4(prog, "rnViewY", v._12, v._22, v._32, 0.0f);
    Set4(prog, "rnViewZ", v._13, v._23, v._33, 0.0f);
    Set4(prog, "rnProj", _frameState.projection._11, _frameState.projection._22, static_cast<float>(vw),
         static_cast<float>(vh));
    const float skyK = exposure * kDropSky;
    Set4(prog, "rnSky", sky[0] * skyK, sky[1] * skyK, sky[2] * skyK, kDropDiameter);
    Set4(prog, "rnKey", key[0] * exposure, key[1] * exposure, key[2] * exposure, kKeyLobe);
    Set4(prog, "rnToward", _xpRain.toward[0], _xpRain.toward[1], _xpRain.toward[2], kLampLobe);
    Set4(prog, "rnCount", static_cast<float>(nLights), 0.0f, 0.0f, 0.0f);
    if (const GLint loc = glGetUniformLocation(prog, "rnLightPos"); loc >= 0)
        glUniform4fv(loc, kRainLights, lightPos);
    if (const GLint loc = glGetUniformLocation(prog, "rnLightCol"); loc >= 0)
        glUniform4fv(loc, kRainLights, lightCol);
    if (const GLint loc = glGetUniformLocation(prog, "rnLightDir"); loc >= 0)
        glUniform4fv(loc, kRainLights, lightDir);
    // The camera, as the composite set it this frame.
    const rm::ToneCurve curve = rm::MakeToneCurve(_lighting.camContrast, _lighting.camShoulder, _lighting.camWhite);
    Set4(prog, "camParams", 1.0f, 1.0f, _lighting.camCurve, 0.0f);
    Set4(prog, "camCurve", curve.a, curve.d, curve.b, curve.c);
    Set4(prog, "camNight", 0.0f, 0.0f, curve.white, rm::kWhiteExcessStops);
    const float* m = _camera.color.m;
    Set4(prog, "camColor0", m[0], m[1], m[2], 0.0f);
    Set4(prog, "camColor1", m[3], m[4], m[5], 0.0f);
    Set4(prog, "camColor2", m[6], m[7], m[8], 0.0f);
    SetInt(prog, "depthTex", 2);
    Set4(prog, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    Set4(prog, "depthMap", f.depthScale, f.depthBias, 0.0f, 0.0f);
    BindTex(2, _post.depthTex);
    glActiveTexture(GL_TEXTURE0);

    glBindFramebuffer(GL_FRAMEBUFFER, _ssaaFbo);
    glViewport(vx, vy, vw, vh);
    Poseidon::render::blend::AlphaBlendKeepAlpha();
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, drops);
    Poseidon::render::blend::Opaque();
    glViewport(0, 0, f.w, f.h);

    if (_xpRain.logged < 3)
    {
        ++_xpRain.logged;
        LOG_INFO(Graphics,
                 "GL33 xp rain: frame {} drops {} lights {} density {:.2f} exposure {:.3g} sky {:.3g} key {:.3g} "
                 "streak {:.1f},{:.1f},{:.1f} fog {:.0f} puffs {}{}",
                 frame, drops, nLights, rain.density, exposure, sky[1], key[1], rain.relVel[1][0], rain.relVel[1][1],
                 rain.relVel[1][2], fogRange, coverOn ? _xpRain.cover.puffs : 0,
                 rm::RainXpRound1FromEnv() ? " ROUND 1 PLACEMENT" : "");
    }
}
