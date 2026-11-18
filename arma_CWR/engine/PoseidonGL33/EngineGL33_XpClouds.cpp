// The experiment POSEIDON_XP_CLOUDS (perf-squad lane EX8-CLOUDS, PERF-PLAN
// R-13, owner question O-15): real 3D clouds built from the weather sky's own
// cloud-density map, in place of the painted dome's thick broken cloud. Off by
// default: with the variable unset nothing here runs, the litXpClouds slot
// stays zero and every draw is the one before.
//
// The cloud field is the sky-matched shadows' (SkyDensity.hpp,
// domeCloudCover): a point is looked up in the density map in the direction
// the shadow anchor sees it on the deck at kSkyShadowDeckAltitude, so the
// cloud drawn over a spot of ground is the one whose shadow lies there, and it
// turns with the sky's drift (litSky.z) as the painted dome did. The map's
// broken cloud (the overcast deck left out) gives each column its cover, its
// top (taller where the painted cloud is thicker and darker) and its density,
// in a slab from kBase to kBase + kThick metres above sea level. A tileable 3D
// noise (built once on the CPU, 64^3: Perlin-Worley billows and Worley
// detail) under the height profile, cut by the cover (Schneider 2015), shapes
// the clouds: where the cover is thin only small puffs stay, where it is whole
// the cloud is whole with a rolling base and top; the detail erodes the edges.
// The noise's frame turns with the field, so the billows ride on the clouds.
// The dome keeps the painted thin cloud, the cirrus and an overcast deck (as a
// flat sheet seen from below the deck read better painted, stills job 5), and
// gives up its painted cloud only where the march's cover is (litXpClouds.yz).
//
// Light (the dome's linear units): the sun as the deck sees it
// (litAtmoCloudSun.rgb, times a gain that falls from kSunGainLow at a low sun
// to kSunGainHigh at a high one) through a 5-step march toward the sun (Beer
// plus a slab's diffuse transmission, so thick cloud goes grey, not black), a
// two-lobe phase (forward 0.6, back -0.2) averaging 1, and the sky above
// (litAtmoCloudSun.w, half in the colour of the sky table 35 degrees up)
// through the cloud above. Far clouds fade into the clear sky behind them
// (aerial perspective, kHaze).
//
// Two passes before the dome draws (BeginRemasterSky(Dome)): the march at half
// the 3D viewport's width and height (a quarter of its pixels; 48 steps,
// the start jittered per pixel and per frame), then a resolve that blends the
// march with the last frame's result reprojected by direction (the clouds are
// kilometres away: a view direction is enough), clamped to the march's 3x3
// neighbourhood so nothing smears when the field changes. The dome
// (shadeSkyDome) and the sun disk read the resolve on unit 11; the painted
// sky-cloud sheets (Landscape::DrawClouds) are not drawn while it runs.
//
// The cloud shadows (a third pass, after the resolve): a world-space map
// around the camera (kShadowRes^2 texels over kShadowSize metres, wrapping, so
// its grid is fixed to the world), each texel the sun's share through the
// march's own density along the sun from its point on the cloud base (the
// same columns, billows and detail the march draws), times what an overcast
// deck lets through (the dome still paints the deck, so its shadow stays
// today's). domeCloudCover reads it
// on unit kXpCloudsUnit while litXpClouds.y is set (the world pass and
// the air and fog passes after the sky pass), so the shadows on the ground
// are those of the clouds drawn above it, darkest (the rig's transmission
// under a cloud) where the march's cloud is thick; past the map's reach they
// fade back to today's. POSEIDON_XP_CLOUDS_SHADOWS=0 keeps today's shadows.
// POSEIDON_XP_CLOUDS_SHADOW_DUMP=<dir> writes the map every 200th pass as two
// 8-bit PGMs, the march's shadow cover and today's cover over the same ground,
// and logs their covered shares and overlap (the log's time tells the view).
//
// What it does not do (the prototype's limits): the clouds do not take part in
// the AI's sight or the classic fog (neither did the painted ones); the camera
// above kBase - 50 m switches it off; night keeps the classic sky (the march
// runs only while the sky table shades the dome).

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Core/GLPipelineState.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace Poseidon::gl33lighting;

namespace
{

// The march's numbers (env overrides for look iterations; see XpEnvFloat).
constexpr float kBase = 1450.0f;      // cloud base (m ASL), under the shadow deck at 1800
constexpr float kThick = 900.0f;      // slab thickness (m)
constexpr float kSigma = 0.035f;      // extinction at density 1 (1/m)
constexpr float kCoverFrom = 0.32f;   // the map density where a layer starts ...
constexpr float kCoverFull = 0.6f;    // ... and is whole (the shadows' edge is 0.43 to 0.67)
constexpr float kErode = 0.75f;       // billow erosion
constexpr float kDetailErode = 0.45f; // detail erosion
// The sun's gain on the clouds: kSunGainLow up to kSunGainFrom degrees of sun,
// kSunGainHigh from kSunGainTo (stills bracket, job 8: at 0.6 a midday cumulus
// overhead lifted the camera's sky guard by over 1 EV and darkened the ground
// by a quarter; 0.3 keeps the ground within about 6 %, while a low sun's
// clouds need 0.6 or more to keep the painted sky's glow).
constexpr float kSunGainLow = 0.7f;
constexpr float kSunGainHigh = 0.3f;
constexpr float kSunGainFrom = 10.0f;
constexpr float kSunGainTo = 40.0f;
constexpr float kMultiScatter = 0.4f;
constexpr float kSkyGain = 0.6f;
constexpr float kHaze = 30000.0f; // m
constexpr float kHistory = 0.85f; // the history's share in the resolve
constexpr int kSteps = 48;
constexpr int kNoiseSize = 64;
constexpr float kShadowSize = 14000.0f; // m the shadow map covers (it wraps)
constexpr int kShadowRes = 512;         // its texels per side (27 m each)
constexpr int kShadowSteps = 12;        // its steps along the sun through the slab

float XpEnvFloat(const char* name, float fallback)
{
    const char* v = std::getenv(name);
    if (!v || !*v)
        return fallback;
    char* end = nullptr;
    const float f = std::strtof(v, &end);
    return end != v && std::isfinite(f) ? f : fallback;
}

bool XpShadowsEnvOn()
{
    static const bool on = []
    {
        const char* v = std::getenv("POSEIDON_XP_CLOUDS_SHADOWS");
        return !(v && *v && std::string(v) == "0");
    }();
    return on;
}

// On unless POSEIDON_XP_CLOUDS=0 (today's painted sky, for the identity
// gates). Owner verdict 2026-11-11: "3d clouds good".
bool XpCloudsEnvOn()
{
    static const bool on = []
    {
        const char* v = std::getenv("POSEIDON_XP_CLOUDS");
        return !(v && *v && std::string(v) == "0");
    }();
    return on;
}

// ---- The 3D noise (tileable, built once) ---------------------------------------
std::uint32_t Hash3(int x, int y, int z, std::uint32_t seed)
{
    std::uint32_t h = seed ^ (static_cast<std::uint32_t>(x) * 0x8da6b343u) ^
                      (static_cast<std::uint32_t>(y) * 0xd8163841u) ^ (static_cast<std::uint32_t>(z) * 0xcb1ab31fu);
    h = (h ^ (h >> 16)) * 0x7feb352du;
    h = (h ^ (h >> 15)) * 0x846ca68bu;
    return h ^ (h >> 16);
}

float Rand01(std::uint32_t h)
{
    return static_cast<float>(h & 0xffffffu) / 16777216.0f;
}

int Wrap(int i, int period)
{
    return ((i % period) + period) % period;
}

// Distance to the nearest feature point (one per cell), cells wrapping every
// `period` (x, y, z in cell units).
float Worley(float x, float y, float z, int period, std::uint32_t seed)
{
    const int ix = static_cast<int>(std::floor(x)), iy = static_cast<int>(std::floor(y)),
              iz = static_cast<int>(std::floor(z));
    float best = 1e9f;
    for (int dz = -1; dz <= 1; ++dz)
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
            {
                const int cx = ix + dx, cy = iy + dy, cz = iz + dz;
                const std::uint32_t h = Hash3(Wrap(cx, period), Wrap(cy, period), Wrap(cz, period), seed);
                const float fx = static_cast<float>(cx) + Rand01(h) - x;
                const float fy = static_cast<float>(cy) + Rand01(h * 747796405u + 2891336453u) - y;
                const float fz = static_cast<float>(cz) + Rand01(h * 2654435761u + 1013904223u) - z;
                best = std::min(best, fx * fx + fy * fy + fz * fz);
            }
    return std::sqrt(best);
}

float Fade(float t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float Grad(std::uint32_t h, float x, float y, float z)
{
    static const float g[12][3] = {{1, 1, 0},  {-1, 1, 0},  {1, -1, 0}, {-1, -1, 0}, {1, 0, 1},  {-1, 0, 1},
                                   {1, 0, -1}, {-1, 0, -1}, {0, 1, 1},  {0, -1, 1},  {0, 1, -1}, {0, -1, -1}};
    const float* v = g[h % 12u];
    return v[0] * x + v[1] * y + v[2] * z;
}

// Gradient noise about 0 (roughly -1..1), lattice wrapping every `period`.
float Perlin(float x, float y, float z, int period, std::uint32_t seed)
{
    const int ix = static_cast<int>(std::floor(x)), iy = static_cast<int>(std::floor(y)),
              iz = static_cast<int>(std::floor(z));
    const float fx = x - static_cast<float>(ix), fy = y - static_cast<float>(iy), fz = z - static_cast<float>(iz);
    float c[2][2][2];
    for (int k = 0; k < 2; ++k)
        for (int j = 0; j < 2; ++j)
            for (int i = 0; i < 2; ++i)
                c[k][j][i] = Grad(Hash3(Wrap(ix + i, period), Wrap(iy + j, period), Wrap(iz + k, period), seed),
                                  fx - static_cast<float>(i), fy - static_cast<float>(j), fz - static_cast<float>(k));
    const float u = Fade(fx), v = Fade(fy), w = Fade(fz);
    auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };
    const float x00 = lerp(c[0][0][0], c[0][0][1], u), x10 = lerp(c[0][1][0], c[0][1][1], u);
    const float x01 = lerp(c[1][0][0], c[1][0][1], u), x11 = lerp(c[1][1][0], c[1][1][1], u);
    return lerp(lerp(x00, x10, v), lerp(x01, x11, v), w);
}

// RG8, n^3: r Perlin-Worley billows (Perlin fbm eroded by inverted Worley
// fbm, frequencies 4, 8, 16), g inverted Worley fbm (8, 16, 32).
std::vector<std::uint8_t> BuildXpNoise(int n)
{
    std::vector<std::uint8_t> out(static_cast<size_t>(n) * n * n * 2);
    auto clamp01 = [](float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); };
    for (int k = 0; k < n; ++k)
        for (int j = 0; j < n; ++j)
            for (int i = 0; i < n; ++i)
            {
                const float x = (static_cast<float>(i) + 0.5f) / static_cast<float>(n);
                const float y = (static_cast<float>(j) + 0.5f) / static_cast<float>(n);
                const float z = (static_cast<float>(k) + 0.5f) / static_cast<float>(n);
                float per = 0.0f, amp = 1.0f, norm = 0.0f;
                for (int o = 0; o < 3; ++o)
                {
                    const int f = 4 << o;
                    per += amp * Perlin(x * f, y * f, z * f, f, 11u + o);
                    norm += amp;
                    amp *= 0.5f;
                }
                const float p01 = clamp01(per / norm * 0.9f + 0.5f);
                auto worleyFbm = [&](int f0, std::uint32_t seed)
                {
                    float s = 0.0f;
                    const float wts[3] = {0.625f, 0.25f, 0.125f};
                    for (int o = 0; o < 3; ++o)
                    {
                        const int f = f0 << o;
                        s += wts[o] * (1.0f - clamp01(Worley(x * f, y * f, z * f, f, seed + o)));
                    }
                    return s;
                };
                const float w1 = worleyFbm(4, 101u);
                const float r = clamp01((p01 - (w1 - 1.0f)) / (1.0f - (w1 - 1.0f)));
                const float g = worleyFbm(8, 211u);
                const size_t at = ((static_cast<size_t>(k) * n + j) * n + i) * 2;
                out[at + 0] = static_cast<std::uint8_t>(clamp01(r) * 255.0f + 0.5f);
                out[at + 1] = static_cast<std::uint8_t>(clamp01(g) * 255.0f + 0.5f);
            }
    // Stretch r over the full range (its spread is narrow after the erosion).
    int lo = 255, hi = 0;
    for (size_t i = 0; i < out.size(); i += 2)
    {
        lo = std::min<int>(lo, out[i]);
        hi = std::max<int>(hi, out[i]);
    }
    if (hi > lo)
        for (size_t i = 0; i < out.size(); i += 2)
            out[i] = static_cast<std::uint8_t>((out[i] - lo) * 255 / (hi - lo));
    return out;
}

struct XpCloudsState
{
    bool tried = false;
    bool failed = false;
    GLuint progMarch = 0, progResolve = 0, progShadow = 0, vao = 0;
    GLuint shadowTex = 0, shadowFbo = 0;
    bool shadowDrawn = false;
    int shadowPasses = 0;
    GLuint noiseTex = 0;
    GLuint marchTex = 0, marchFbo = 0;
    GLuint histTex[2] = {0, 0};
    GLuint histFbo[2] = {0, 0};
    int w = 0, h = 0;
    int cur = 0;
    bool history = false;
    float prev[3][3] = {};
    std::uint32_t drawnFrame = 0;
    bool drawn = false;
    std::uint32_t seed = 0;
};

XpCloudsState s_xp;

GLuint CompileXp(GLenum type, const std::string& src, const char* name)
{
    std::string overrideSource;
    const char* text = src.c_str();
    if (LoadShaderOverride(name, overrideSource))
        text = overrideSource.c_str();
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &text, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 xp clouds: shader compile error [{}]: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint LinkXp(GLuint vs, GLuint fs, const char* name)
{
    if (!vs || !fs)
        return 0;
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 xp clouds: program link error [{}]: {}", name, log);
        glDeleteProgram(p);
        return 0;
    }
    const GLuint lit = glGetUniformBlockIndex(p, "LitConstants");
    if (lit != GL_INVALID_INDEX)
        glUniformBlockBinding(p, lit, kLitBinding);
    return p;
}

void Uniform4(GLuint prog, const char* name, float a, float b, float c, float d)
{
    const GLint loc = glGetUniformLocation(prog, name);
    if (loc >= 0)
        glUniform4f(loc, a, b, c, d);
}

void UniformInt(GLuint prog, const char* name, int v)
{
    const GLint loc = glGetUniformLocation(prog, name);
    if (loc >= 0)
        glUniform1i(loc, v);
}

void DeleteTex(GLuint& t)
{
    if (t)
    {
        Poseidon::GL33Bind::OnTexDeleted(t);
        glDeleteTextures(1, &t);
    }
    t = 0;
}

void DeleteFbo(GLuint& f)
{
    if (f)
        glDeleteFramebuffers(1, &f);
    f = 0;
}

void DestroyShadow()
{
    DeleteTex(s_xp.shadowTex);
    DeleteFbo(s_xp.shadowFbo);
    s_xp.shadowDrawn = false;
}

void DestroyTargets()
{
    DeleteTex(s_xp.marchTex);
    DeleteFbo(s_xp.marchFbo);
    for (int i = 0; i < 2; ++i)
    {
        DeleteTex(s_xp.histTex[i]);
        DeleteFbo(s_xp.histFbo[i]);
    }
    s_xp.w = s_xp.h = 0;
    s_xp.history = false;
}

// A half-float target, cleared at creation to "no cloud" (transmittance 1).
bool MakeTarget(GLuint& tex, GLuint& fbo, int w, int h)
{
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    const bool ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    if (ok)
    {
        const float clear[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, clear);
    }
    return ok;
}

// The shadow map: RGBA16F (r the sun's share, g today's cover for the dump),
// wrapping, so a lookup in world xz needs no centre.
bool MakeShadowTarget()
{
    if (!MakeTarget(s_xp.shadowTex, s_xp.shadowFbo, kShadowRes, kShadowRes))
        return false;
    glBindTexture(GL_TEXTURE_2D, s_xp.shadowTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return true;
}

// POSEIDON_XP_CLOUDS_SHADOW_DUMP: the map read back, two PGMs (the march's
// shadow cover 1 - r and today's cover g, 255 = fully covered) and one log line.
void DumpShadow(const char* dir, int pass, float camX, float camZ)
{
    const size_t n = static_cast<size_t>(kShadowRes) * kShadowRes;
    std::vector<float> px(n * 4);
    glBindFramebuffer(GL_FRAMEBUFFER, s_xp.shadowFbo);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, kShadowRes, kShadowRes, GL_RGBA, GL_FLOAT, px.data());
    std::vector<std::uint8_t> nw(n), od(n);
    double sumNew = 0.0, sumOld = 0.0, both = 0.0, either = 0.0, coveredNew = 0.0, coveredOld = 0.0;
    double sxy = 0.0, sxx = 0.0, syy = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
        const float a = std::min(std::max(1.0f - px[i * 4 + 0], 0.0f), 1.0f);
        const float b = std::min(std::max(px[i * 4 + 1], 0.0f), 1.0f);
        // PGM rows run top down; the map's row 0 is its bottom (south).
        const size_t row = static_cast<size_t>(kShadowRes - 1) - i / static_cast<size_t>(kShadowRes);
        const size_t at = row * static_cast<size_t>(kShadowRes) + i % static_cast<size_t>(kShadowRes);
        nw[at] = static_cast<std::uint8_t>(a * 255.0f + 0.5f);
        od[at] = static_cast<std::uint8_t>(b * 255.0f + 0.5f);
        sumNew += a;
        sumOld += b;
        sxy += static_cast<double>(a) * b;
        sxx += static_cast<double>(a) * a;
        syy += static_cast<double>(b) * b;
        const bool cn = a > 0.5f, co = b > 0.5f;
        coveredNew += cn ? 1.0 : 0.0;
        coveredOld += co ? 1.0 : 0.0;
        both += (cn && co) ? 1.0 : 0.0;
        either += (cn || co) ? 1.0 : 0.0;
    }
    const double count = static_cast<double>(n);
    const double mn = sumNew / count, mo = sumOld / count;
    const double cov = sxy / count - mn * mo;
    const double vn = sxx / count - mn * mn, vo = syy / count - mo * mo;
    const double corr = (vn > 1e-9 && vo > 1e-9) ? cov / std::sqrt(vn * vo) : 0.0;
    const double iou = either > 0.0 ? both / either : 1.0;
    // Of today's covered ground, the share the march's clouds also cover.
    const double kept = coveredOld > 0.0 ? both / coveredOld : 1.0;
    for (int k = 0; k < 2; ++k)
    {
        char path[1024];
        std::snprintf(path, sizeof(path), "%s/xpshadow_pass%d_x%d_z%d_%s.pgm", dir, pass, static_cast<int>(camX),
                      static_cast<int>(camZ), k == 0 ? "march" : "today");
        if (std::FILE* f = std::fopen(path, "wb"))
        {
            std::fprintf(f, "P5\n%d %d\n255\n", kShadowRes, kShadowRes);
            std::fwrite(k == 0 ? nw.data() : od.data(), 1, n, f);
            std::fclose(f);
        }
    }
    LOG_INFO(Graphics,
             "GL33 xp clouds: shadow dump pass {} cam {:.0f},{:.0f}: mean cover march {:.3f} today {:.3f}; "
             "covered (>0.5) march {:.3f} today {:.3f}; overlap (IoU) {:.3f}; today's covered also covered by the "
             "march {:.3f}; correlation {:.3f}",
             pass, camX, camZ, mn, mo, coveredNew / count, coveredOld / count, iou, kept, corr);
}

} // namespace

float EngineGL33::XpCloudsTimelapse() const
{
    static const float rate = XpEnvFloat("POSEIDON_XP_CLOUDS_TIMELAPSE", 0.0f);
    return rate != 0.0f ? rate * Glob.time.toFloat() : 0.0f;
}

bool EngineGL33::VolumetricCloudsDrawn() const
{
    return s_xp.drawn && s_xp.drawnFrame == GetFrameCounter();
}

void EngineGL33::DeinitXpClouds()
{
    DestroyTargets();
    DestroyShadow();
    DeleteTex(s_xp.noiseTex);
    for (GLuint* p : {&s_xp.progMarch, &s_xp.progResolve, &s_xp.progShadow})
    {
        if (*p)
            glDeleteProgram(*p);
        *p = 0;
    }
    if (s_xp.vao)
    {
        GL33Bind::OnVaoDeleted(s_xp.vao);
        glDeleteVertexArrays(1, &s_xp.vao);
    }
    s_xp.vao = 0;
    s_xp.tried = false;
    s_xp.failed = false;
    s_xp.drawn = false;
}

void EngineGL33::BindXpClouds(bool on)
{
    if (!XpCloudsEnvOn())
        return;
    // Unit 17 (xpClouds): during the sky pass's dome and sun draws the clouds'
    // resolve, after them this frame's shadow map (white while there is none).
    // Past the bind cache's units: bound raw, the cache told.
    const bool use = on && s_xp.drawn && s_xp.drawnFrame == GetFrameCounter() && s_xp.histTex[s_xp.cur];
    const bool shadow = s_xp.shadowDrawn && s_xp.drawn && s_xp.drawnFrame == GetFrameCounter();
    glActiveTexture(GL_TEXTURE0 + kXpCloudsUnit);
    glBindTexture(GL_TEXTURE_2D, use ? s_xp.histTex[s_xp.cur] : (shadow ? s_xp.shadowTex : _post.whiteTex));
    glBindSampler(kXpCloudsUnit, 0);
    glActiveTexture(GL_TEXTURE0);
    GL33Bind::Invalidate();
    GL33Bind::ActiveUnit(0);
    // y, z: the density map's range over which the painted thick cloud gives
    // way to the march's layer (its cover); w: the layer's middle over the
    // camera per haze distance (the march's aerial fade at the zenith).
    float hazeW = 0.0f;
    if (use && GScene && GScene->GetCamera())
    {
        const float mid =
            XpEnvFloat("POSEIDON_XP_CLOUDS_BASE", kBase) + 0.5f * XpEnvFloat("POSEIDON_XP_CLOUDS_THICK", kThick);
        hazeW = std::max(mid - GScene->GetCamera()->Position().Y(), 0.0f) /
                std::max(XpEnvFloat("POSEIDON_XP_CLOUDS_HAZE", kHaze), 1.0f);
    }
    if (use)
    {
        SetLitSlot(LitConst::XpClouds, 1.0f, XpEnvFloat("POSEIDON_XP_CLOUDS_COVER_FROM", kCoverFrom),
                   XpEnvFloat("POSEIDON_XP_CLOUDS_COVER_FULL", kCoverFull), hazeW);
        return;
    }
    // Outside the sky pass's dome and sun draws: x 0, y 1 / the shadow map's
    // size while this frame's map is drawn (0 keeps today's shadows), z the
    // cloud base the map lies on (m above sea level).
    SetLitSlot(LitConst::XpClouds, 0.0f,
               shadow ? 1.0f / std::max(XpEnvFloat("POSEIDON_XP_CLOUDS_SHADOW_SIZE", kShadowSize), 1000.0f) : 0.0f,
               shadow ? XpEnvFloat("POSEIDON_XP_CLOUDS_BASE", kBase) : 0.0f, 0.0f);
}

void EngineGL33::RunXpClouds(bool domeTable)
{
    if (!XpCloudsEnvOn())
        return;
    s_xp.drawn = false;
    s_xp.shadowDrawn = false;
    Camera* camera = GScene ? GScene->GetCamera() : nullptr;
    if (!domeTable || !camera || s_xp.failed)
    {
        s_xp.history = false;
        return;
    }
    const float base = XpEnvFloat("POSEIDON_XP_CLOUDS_BASE", kBase);
    if (camera->Position().Y() > base - 50.0f)
    {
        s_xp.history = false;
        return;
    }
    if (!s_xp.tried)
    {
        s_xp.tried = true;
        GLuint vs = CompileXp(GL_VERTEX_SHADER, PostShaderSource(PostShader::Vertex), "vsPost");
        GLuint fm = CompileXp(GL_FRAGMENT_SHADER, XpCloudsShaderSource(0), "psXpCloudsMarch");
        GLuint fr = CompileXp(GL_FRAGMENT_SHADER, XpCloudsShaderSource(1), "psXpCloudsResolve");
        GLuint fs = CompileXp(GL_FRAGMENT_SHADER, XpCloudsShaderSource(2), "psXpCloudsShadow");
        s_xp.progMarch = LinkXp(vs, fm, "psXpCloudsMarch");
        s_xp.progResolve = LinkXp(vs, fr, "psXpCloudsResolve");
        s_xp.progShadow = LinkXp(vs, fs, "psXpCloudsShadow");
        for (GLuint s : {vs, fm, fr, fs})
            if (s)
                glDeleteShader(s);
        glGenVertexArrays(1, &s_xp.vao);
        // The noise, once (about 0.1 s on the main thread at the first frame).
        const std::vector<std::uint8_t> noise = BuildXpNoise(kNoiseSize);
        GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
        glGenTextures(1, &s_xp.noiseTex);
        glBindTexture(GL_TEXTURE_3D, s_xp.noiseTex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RG8, kNoiseSize, kNoiseSize, kNoiseSize, 0, GL_RG, GL_UNSIGNED_BYTE,
                     noise.data());
        glGenerateMipmap(GL_TEXTURE_3D);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
        glBindTexture(GL_TEXTURE_3D, 0);
        GL33Bind::Invalidate();
        GL33Bind::ActiveUnit(0);
        {
            double sum[2] = {0.0, 0.0};
            int lo[2] = {255, 255}, hi[2] = {0, 0};
            for (size_t i = 0; i < noise.size(); ++i)
            {
                const int c = static_cast<int>(i & 1u);
                sum[c] += noise[i];
                lo[c] = std::min<int>(lo[c], noise[i]);
                hi[c] = std::max<int>(hi[c], noise[i]);
            }
            const double n = static_cast<double>(noise.size() / 2);
            LOG_INFO(Graphics, "GL33 xp clouds: noise r mean {:.1f} range {}..{}, g mean {:.1f} range {}..{}",
                     sum[0] / n, lo[0], hi[0], sum[1] / n, lo[1], hi[1]);
        }
        if (!s_xp.progShadow)
            LOG_ERROR(Graphics, "GL33 xp clouds: shadow program failed; today's cloud shadows stay");
        if (!s_xp.progMarch || !s_xp.progResolve)
        {
            LOG_ERROR(Graphics, "GL33 xp clouds: programs failed; the painted clouds stay");
            s_xp.failed = true;
            return;
        }
        LOG_INFO(Graphics, "GL33 xp clouds: on (steps {}, noise {}^3)", kSteps, kNoiseSize);
    }

    // The 3D viewport the dome draws into, at half its width and height.
    GLint viewport[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    const int w = std::max(1, (viewport[2] + 1) / 2);
    const int h = std::max(1, (viewport[3] + 1) / 2);
    if (w != s_xp.w || h != s_xp.h)
    {
        DestroyTargets();
        GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
        bool ok = MakeTarget(s_xp.marchTex, s_xp.marchFbo, w, h);
        ok = MakeTarget(s_xp.histTex[0], s_xp.histFbo[0], w, h) && ok;
        ok = MakeTarget(s_xp.histTex[1], s_xp.histFbo[1], w, h) && ok;
        glBindTexture(GL_TEXTURE_2D, 0);
        GL33Bind::Invalidate();
        GL33Bind::ActiveUnit(0);
        BindFrameRenderTarget();
        if (!ok)
        {
            LOG_ERROR(Graphics, "GL33 xp clouds: targets incomplete; the painted clouds stay");
            DestroyTargets();
            s_xp.failed = true;
            return;
        }
        s_xp.w = w;
        s_xp.h = h;
        LOG_INFO(Graphics, "GL33 xp clouds: targets {}x{}", w, h);
    }

    // The camera basis, unjittered (as UploadLitSky's for the dome).
    Vector3 r = camera->DirectionAside();
    Vector3 u = camera->DirectionUp();
    Vector3 f = camera->Direction();
    r.Normalize();
    u.Normalize();
    f.Normalize();
    const float l = camera->Left(), t = camera->Top();
    const float cur[3][3] = {
        {r.X() * l, r.Y() * l, r.Z() * l}, {u.X() * t, u.Y() * t, u.Z() * t}, {f.X(), f.Y(), f.Z()}};
    Landscape* land = GScene->GetLandscape();
    const float skyThrough = land ? land->SkyThrough() : 1.0f;
    // SkyDeckWeight (the shaders' 1 - smoothstep(0.35, 0.75, skyThrough)).
    float dk = std::min(std::max((skyThrough - 0.35f) / 0.4f, 0.0f), 1.0f);
    dk = 1.0f - dk * dk * (3.0f - 2.0f * dk);
    const float cloudsPos = (land ? land->CloudsPosition() : 0.0f) + XpCloudsTimelapse();
    // A frame seed for the march's jitter (deterministic: the frame counter).
    const float seed = static_cast<float>(GetFrameCounter() % 64u);
    float sunGain = XpEnvFloat("POSEIDON_XP_CLOUDS_SUN", -1.0f);
    if (sunGain < 0.0f)
    {
        LightSun* sun = GScene->MainLight();
        const float elev = sun ? std::asin(std::max(-1.0f, std::min(1.0f, -sun->Direction().Y()))) * 57.29578f : 0.0f;
        float t = std::min(std::max((elev - kSunGainFrom) / (kSunGainTo - kSunGainFrom), 0.0f), 1.0f);
        t = t * t * (3.0f - 2.0f * t);
        sunGain = kSunGainLow + (kSunGainHigh - kSunGainLow) * t;
    }

    // State the passes change that the next draw does not set again (as the
    // sky table's GPU build saves it).
    const bool cull = glIsEnabled(GL_CULL_FACE) == GL_TRUE;
    const bool scissor = glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE;
    const bool stencil = glIsEnabled(GL_STENCIL_TEST) == GL_TRUE;
    const bool depthClamp = glIsEnabled(GL_DEPTH_CLAMP) == GL_TRUE;
    Poseidon::render::depthstencil::Disabled(/*hasStencil*/ false);
    glDisable(GL_STENCIL_TEST);
    Poseidon::render::blend::Opaque();
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    Poseidon::render::pipeline::DisableDepthClamp();
    GL33Bind::Vao(s_xp.vao);
    // Unit 0's sampler object belongs to the pipeline (restored by the next
    // draw); the noise reads its own mipmaps. Unit 11 has none outside the sea.
    glBindSampler(0, 0);
    _samplerSlot0 = -1;

    // The march.
    GLuint pm = s_xp.progMarch;
    glUseProgram(pm);
    UniformInt(pm, "xpNoise", 0);
    UniformInt(pm, "skyLut", kSkyLutUnit);
    UniformInt(pm, "skyDensity", kSkyDensityUnit);
    Uniform4(pm, "xpRight", cur[0][0], cur[0][1], cur[0][2], seed);
    Uniform4(pm, "xpUp", cur[1][0], cur[1][1], cur[1][2], cloudsPos * 1.5f);
    Uniform4(pm, "xpFwd", cur[2][0], cur[2][1], cur[2][2],
             static_cast<float>(std::max(8, static_cast<int>(XpEnvFloat("POSEIDON_XP_CLOUDS_STEPS", kSteps)))));
    Uniform4(pm, "xpLayer", base, XpEnvFloat("POSEIDON_XP_CLOUDS_THICK", kThick),
             XpEnvFloat("POSEIDON_XP_CLOUDS_SIGMA", kSigma), dk);
    Uniform4(pm, "xpShape", XpEnvFloat("POSEIDON_XP_CLOUDS_COVER_FROM", kCoverFrom),
             XpEnvFloat("POSEIDON_XP_CLOUDS_COVER_FULL", kCoverFull), XpEnvFloat("POSEIDON_XP_CLOUDS_ERODE", kErode),
             XpEnvFloat("POSEIDON_XP_CLOUDS_DETAIL", kDetailErode));
    Uniform4(pm, "xpLight", sunGain, XpEnvFloat("POSEIDON_XP_CLOUDS_MS", kMultiScatter),
             XpEnvFloat("POSEIDON_XP_CLOUDS_SKY", kSkyGain), XpEnvFloat("POSEIDON_XP_CLOUDS_HAZE", kHaze));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, s_xp.noiseTex);
    glBindFramebuffer(GL_FRAMEBUFFER, s_xp.marchFbo);
    glViewport(0, 0, w, h);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindTexture(GL_TEXTURE_3D, 0);
    GL33Bind::Invalidate();

    // The resolve into the other history target.
    const int next = s_xp.cur ^ 1;
    GLuint pr = s_xp.progResolve;
    glUseProgram(pr);
    UniformInt(pr, "xpCur", 0);
    UniformInt(pr, "xpHist", kSeaNextFrameUnit);
    Uniform4(pr, "xpRight", cur[0][0], cur[0][1], cur[0][2], 0.0f);
    Uniform4(pr, "xpUp", cur[1][0], cur[1][1], cur[1][2], 0.0f);
    Uniform4(pr, "xpFwd", cur[2][0], cur[2][1], cur[2][2], 0.0f);
    Uniform4(pr, "xpPrevRight", s_xp.prev[0][0], s_xp.prev[0][1], s_xp.prev[0][2], 0.0f);
    Uniform4(pr, "xpPrevUp", s_xp.prev[1][0], s_xp.prev[1][1], s_xp.prev[1][2], 0.0f);
    Uniform4(pr, "xpPrevFwd", s_xp.prev[2][0], s_xp.prev[2][1], s_xp.prev[2][2], s_xp.history ? 1.0f : 0.0f);
    Uniform4(pr, "xpResolve", XpEnvFloat("POSEIDON_XP_CLOUDS_HISTORY", kHistory), 0.0f, 0.0f, 0.0f);
    GL33Bind::Tex2D(0, s_xp.marchTex);
    GL33Bind::Tex2D(kSeaNextFrameUnit, s_xp.histTex[s_xp.cur]);
    glBindFramebuffer(GL_FRAMEBUFFER, s_xp.histFbo[next]);
    glViewport(0, 0, w, h);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    GL33Bind::Tex2D(0, 0);
    GL33Bind::Tex2D(kSeaNextFrameUnit, _post.whiteTex);

    // The cloud shadows: the march's density along the sun, on a world-space
    // map the world, air and fog passes read on kXpCloudsUnit (BindXpClouds).
    if (XpShadowsEnvOn() && s_xp.progShadow)
    {
        if (!s_xp.shadowTex)
        {
            GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
            if (!MakeShadowTarget())
            {
                LOG_ERROR(Graphics, "GL33 xp clouds: shadow target incomplete; today's cloud shadows stay");
                DestroyShadow();
                glDeleteProgram(s_xp.progShadow);
                s_xp.progShadow = 0;
            }
            glBindTexture(GL_TEXTURE_2D, 0);
            GL33Bind::Invalidate();
        }
        if (s_xp.shadowTex)
        {
            const float size = std::max(XpEnvFloat("POSEIDON_XP_CLOUDS_SHADOW_SIZE", kShadowSize), 1000.0f);
            const int steps =
                std::max(4, static_cast<int>(XpEnvFloat("POSEIDON_XP_CLOUDS_SHADOW_STEPS", kShadowSteps)));
            GLuint ps = s_xp.progShadow;
            glUseProgram(ps);
            UniformInt(ps, "xpNoise", 0);
            UniformInt(ps, "skyDensity", kSkyDensityUnit);
            Uniform4(ps, "xpUp", cur[1][0], cur[1][1], cur[1][2], cloudsPos * 1.5f);
            Uniform4(ps, "xpLayer", base, XpEnvFloat("POSEIDON_XP_CLOUDS_THICK", kThick),
                     XpEnvFloat("POSEIDON_XP_CLOUDS_SIGMA", kSigma), dk);
            Uniform4(ps, "xpShape", XpEnvFloat("POSEIDON_XP_CLOUDS_COVER_FROM", kCoverFrom),
                     XpEnvFloat("POSEIDON_XP_CLOUDS_COVER_FULL", kCoverFull),
                     XpEnvFloat("POSEIDON_XP_CLOUDS_ERODE", kErode),
                     XpEnvFloat("POSEIDON_XP_CLOUDS_DETAIL", kDetailErode));
            Uniform4(ps, "xpShadow", size, static_cast<float>(steps), size / static_cast<float>(kShadowRes), 0.0f);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, s_xp.noiseTex);
            glBindFramebuffer(GL_FRAMEBUFFER, s_xp.shadowFbo);
            glViewport(0, 0, kShadowRes, kShadowRes);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindTexture(GL_TEXTURE_3D, 0);
            GL33Bind::Invalidate();
            ++s_xp.shadowPasses;
            static const char* dump = std::getenv("POSEIDON_XP_CLOUDS_SHADOW_DUMP");
            if (dump && *dump && s_xp.shadowPasses % 200 == 0)
                DumpShadow(dump, s_xp.shadowPasses, camera->Position().X(), camera->Position().Z());
            s_xp.shadowDrawn = true;
        }
    }

    glUseProgram(0);
    BindFrameRenderTarget();
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    if (cull)
        glEnable(GL_CULL_FACE);
    if (scissor)
        glEnable(GL_SCISSOR_TEST);
    if (stencil)
        glEnable(GL_STENCIL_TEST);
    if (depthClamp)
        Poseidon::render::pipeline::EnableDepthClamp();
    _pixelShaderSel = PSNone;
    GL33Bind::Invalidate();
    InvalidatePipelineCache();
    GL33Bind::ActiveUnit(0);

    s_xp.cur = next;
    s_xp.history = true;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            s_xp.prev[i][j] = cur[i][j];
    s_xp.drawn = true;
    s_xp.drawnFrame = GetFrameCounter();
}
