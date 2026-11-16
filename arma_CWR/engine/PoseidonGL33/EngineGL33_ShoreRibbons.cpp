// Shore ribbons (perf squad SURF-E, under the Shore Water option's surf).
//
// Breaking waves as thin ribbons along the coastline. The coastline is traced
// once per island from the terrain height map (Graphics/Remaster/
// ShoreRibbons.hpp: a few sea levels, points every 3 m with their sea normal
// and beach slope, sorted by 128 m tiles) and kept in one RGBA32F texture.
// Every frame the points of the tile rows around the camera at the level
// nearest the tide are drawn with one glMultiDrawArrays and no vertex buffer:
// the vertex shader takes everything from gl_VertexID. Per point it draws
// kRibbonSlots ribbons half a cycle apart, each a strip of kRibbonRows rows
// across the shore. The vertex shader slides the point along its sea normal
// onto the drawn sea level (the tide and the sea level's 12.5 s wave), then
// moves the rows through one wave: a line that rolls in from a few metres
// out, breaks into white water near the waterline, runs up the beach as a thin
// sheet, slides back and fades, leaving a wet band and a lace line at its
// reach that dry. How far out it starts and how far it runs up follow the
// beach slope (a vertical run-up of about 0.3 m). The timing follows the
// position along the coast (smooth noise that drifts slowly, plus a swell
// direction), so no two stretches of beach move in step and nothing loops;
// each wave's strength and reach are its own.
//
// The ribbons draw right after the sea's own draws (EndSea), depth tested and
// not written, pulled a little toward the camera along the view ray (the
// screen position stays; the depth wins over the sea and the sand they lie
// on), blended premultiplied (foam over, the wet band darkens) with the
// frame's alpha kept and the surface target masked. Objects drawn later
// (boats, people) cover them by depth; terrain in front hides them.
//
// They run only when the Graphics page's Surf row (or POSEIDON_XP_SURF) picks
// Ribbons; Painted, the default, leaves the waves to the sea and land
// programs (EngineGL33_LightingShaders.cpp) and nothing here is traced, built
// or uploaded. Picked in a running game, the coastline is traced on the next
// frame the sea draws (about 45 ms on Abel, logged), once per island.
//
// Off: Painted, the option off (Shore Water's parts mask without its surf), a
// missing height map, the classic set, night vision, a portrait or a 3D
// preview (EndSea runs only for the remastered sea): nothing is traced or
// drawn and litWater2 carries no ribbon flag.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Remaster/ShoreRibbons.hpp>
#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>
#include <vector>

using namespace Poseidon::gl33lighting;
namespace rm = Poseidon::remaster;

namespace
{

constexpr int kRibbonUnit = 17;       // the points texture (units 0 to 16 are the world's)
constexpr int kRibbonTexWidth = 2048; // points per texture row
constexpr int kRibbonSlots = 2;       // ribbons per point, half a cycle apart
constexpr int kRibbonRows = 6;        // rows across the shore (5 quads)
constexpr int kRibbonVertsPerPoint = kRibbonSlots * (kRibbonRows - 1) * 6;
constexpr float kRibbonFadeStart = 170.0f; // m: the ribbons fade from here ...
constexpr float kRibbonFadeEnd = 320.0f;   // ... to here (the sea's own fringe carries the surf beyond)
constexpr float kRibbonTimeWrap = 8160.0f; // s: the clock's wrap, a whole number of cycles (kRibbonPeriod)

} // namespace

namespace
{

struct RibbonState
{
    // The traced coastline and the map it came from.
    rm::ShoreRibbonSet set;
    const rm::TerrainHorizonMap* tracedFor = nullptr;
    // GL objects of the current context.
    GLuint pointsTex = 0;
    GLuint prog = 0, vao = 0;
    GLint locTime = -1, locGrid = -1;
    bool programTried = false;
    bool unavailable = false;
    bool uploaded = false;
    // The draw's ranges (kept to spare the allocations).
    std::vector<GLint> firsts;
    std::vector<GLsizei> counts;
};

RibbonState s_rib;

GLuint CompileRibbonShader(GLenum type, const std::string& builtIn, const char* name)
{
    std::string src = builtIn;
    std::string overrideSource;
    if (LoadShaderOverride(name, overrideSource))
        src = overrideSource;
    const char* p = src.c_str();
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &p, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 shore ribbons: shader compile error [{}]: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint LinkRibbonProgram()
{
    const std::string vsSrc = ShoreRibbonVertexSource();
    const std::string fsSrc = ShoreRibbonFragmentSource();
    GLuint vs = CompileRibbonShader(GL_VERTEX_SHADER, vsSrc, "vsShoreRibbon");
    GLuint fs = CompileRibbonShader(GL_FRAGMENT_SHADER, fsSrc, "psShoreRibbon");
    if (!vs || !fs)
    {
        if (vs)
            glDeleteShader(vs);
        if (fs)
            glDeleteShader(fs);
        return 0;
    }
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 shore ribbons: program link error: {}", log);
        glDeleteProgram(p);
        return 0;
    }
    // The world programs' blocks and samplers (InitPixelShaders'
    // bindProgramSamplersAndBlocks), and the points.
    const GLuint vsBlock = glGetUniformBlockIndex(p, "VSConstants");
    if (vsBlock != GL_INVALID_INDEX)
        glUniformBlockBinding(p, vsBlock, 0);
    const GLuint psBlock = glGetUniformBlockIndex(p, "PSConstants");
    if (psBlock != GL_INVALID_INDEX)
        glUniformBlockBinding(p, psBlock, 1);
    const GLuint litBlock = glGetUniformBlockIndex(p, "LitConstants");
    if (litBlock != GL_INVALID_INDEX)
        glUniformBlockBinding(p, litBlock, kLitBinding);
    const GLuint lightBlock = glGetUniformBlockIndex(p, "FrameLights");
    if (lightBlock != GL_INVALID_INDEX)
        glUniformBlockBinding(p, lightBlock, kLightListBinding);
    glUseProgram(p);
    for (const char* name : {"tex0", "tex1"})
    {
        const GLint loc = glGetUniformLocation(p, name);
        if (loc >= 0)
            glUniform1i(loc, name[3] == '0' ? 0 : 1);
    }
    for (const auto& ls : kLitSamplers)
    {
        const GLint loc = glGetUniformLocation(p, ls.name);
        if (loc >= 0)
            glUniform1i(loc, ls.unit);
    }
    const GLint pts = glGetUniformLocation(p, "ribbonPoints");
    if (pts >= 0)
        glUniform1i(pts, kRibbonUnit);
    glUseProgram(0);
    return p;
}

void DeleteRibbonTexture()
{
    if (s_rib.pointsTex)
    {
        GL33Bind::OnTexDeleted(s_rib.pointsTex);
        glDeleteTextures(1, &s_rib.pointsTex);
    }
    s_rib.pointsTex = 0;
    s_rib.uploaded = false;
}

} // namespace

bool EngineGL33::ShoreRibbonsActive()
{
    return (_shoreWaterParts & 2) != 0 && _surfRibbons && !s_rib.unavailable && _terrainMap && _terrainMap->Valid() &&
           _terrainHeightTex;
}

void EngineGL33::DeinitShoreRibbons()
{
    // GL objects only: the traced coastline is the island's.
    DeleteRibbonTexture();
    if (s_rib.prog)
        glDeleteProgram(s_rib.prog);
    s_rib.prog = 0;
    if (s_rib.vao)
    {
        GL33Bind::OnVaoDeleted(s_rib.vao);
        glDeleteVertexArrays(1, &s_rib.vao);
    }
    s_rib.vao = 0;
    s_rib.programTried = false;
    s_rib.unavailable = false;
}

void EngineGL33::DrawShoreRibbons()
{
    if ((_shoreWaterParts & 2) == 0 || !_surfRibbons || s_rib.unavailable || !_remasterPrograms || !GScene)
        return;
    if (!_terrainMap || !_terrainMap->Valid() || !_terrainHeightTex)
        return;
    Landscape* land = GScene->GetLandscape();
    const Camera* camera = GScene->GetCamera();
    if (!land || !camera)
        return;

    // The coastline: traced once per map (the island's load), on this thread.
    if (s_rib.tracedFor != _terrainMap.get())
    {
        const auto t0 = std::chrono::steady_clock::now();
        const rm::TerrainHorizonMap& m = *_terrainMap;
        s_rib.set = rm::BuildShoreRibbons(m.height, m.size, m.cellSize);
        s_rib.tracedFor = _terrainMap.get();
        DeleteRibbonTexture();
        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        LOG_INFO(Graphics,
                 "GL33: shore ribbons: {} contours, {:.1f} km over {} levels, {} points ({:.1f} MB), {}x{} tiles of "
                 "{:.0f} m, traced in {:.1f} ms",
                 s_rib.set.contours, s_rib.set.length / 1000.0, s_rib.set.levels, s_rib.set.Points(),
                 static_cast<double>(s_rib.set.Points()) * 16.0 / (1024.0 * 1024.0), s_rib.set.tilesPerSide,
                 s_rib.set.tilesPerSide, s_rib.set.tile, ms);
    }
    if (s_rib.set.Empty())
        return;
    if (!s_rib.programTried)
    {
        s_rib.programTried = true;
        const auto t0 = std::chrono::steady_clock::now();
        s_rib.prog = LinkRibbonProgram();
        if (!s_rib.prog)
        {
            LOG_ERROR(Graphics, "GL33 shore ribbons: no program; no ribbons");
            s_rib.unavailable = true;
            return;
        }
        s_rib.locTime = glGetUniformLocation(s_rib.prog, "ribbonTime");
        s_rib.locGrid = glGetUniformLocation(s_rib.prog, "ribbonGrid");
        glGenVertexArrays(1, &s_rib.vao);
        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        LOG_INFO(Graphics, "GL33: shore ribbons: program built in {:.1f} ms", ms);
    }
    if (!s_rib.uploaded)
    {
        const int points = s_rib.set.Points();
        const int rows = (points + kRibbonTexWidth - 1) / kRibbonTexWidth;
        std::vector<float> data(static_cast<size_t>(rows) * kRibbonTexWidth * 4, 0.0f);
        std::copy(s_rib.set.texels.begin(), s_rib.set.texels.end(), data.begin());
        GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glGenTextures(1, &s_rib.pointsTex);
        glBindTexture(GL_TEXTURE_2D, s_rib.pointsTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, kRibbonTexWidth, rows, 0, GL_RGBA, GL_FLOAT, data.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        GL33Bind::Invalidate();
        s_rib.uploaded = true;
    }

    // The tile rows around the camera at the level nearest the tide.
    const rm::ShoreRibbonSet& set = s_rib.set;
    const int level = set.LevelFor(land->GetSeaLevelNoWave());
    const Vector3 cam = camera->Position();
    const float reach = kRibbonFadeEnd + 40.0f;
    const int T = set.tilesPerSide;
    auto tileIndex = [&](float v) { return std::clamp(static_cast<int>(std::floor(v / set.tile)), 0, T - 1); };
    const int tx0 = tileIndex(cam.X() - reach), tx1 = tileIndex(cam.X() + reach);
    const int tz0 = tileIndex(cam.Z() - reach), tz1 = tileIndex(cam.Z() + reach);
    s_rib.firsts.clear();
    s_rib.counts.clear();
    for (int tz = tz0; tz <= tz1; ++tz)
    {
        const std::uint32_t first = set.TileFirst(level, tx0, tz);
        const std::uint32_t end = tx1 + 1 < T ? set.TileFirst(level, tx1 + 1, tz) : set.TileFirst(level, 0, tz + 1);
        if (end > first)
        {
            s_rib.firsts.push_back(static_cast<GLint>(first) * kRibbonVertsPerPoint);
            s_rib.counts.push_back(static_cast<GLsizei>(end - first) * kRibbonVertsPerPoint);
        }
    }
    if (s_rib.firsts.empty())
        return;

    // State: depth tested, not written; premultiplied blend that keeps the
    // frame's alpha; both faces; no stencil; the surface target masked.
    FlushVSConstants();
    const bool cull = glIsEnabled(GL_CULL_FACE) == GL_TRUE;
    const bool stencil = glIsEnabled(GL_STENCIL_TEST) == GL_TRUE;
    glDisable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
    Poseidon::render::depthstencil::ReadOnly(/*hasStencil*/ false);
    Poseidon::render::blend::PremultipliedKeepAlpha();
    glColorMaski(1, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    GL33Bind::Vao(s_rib.vao);
    glUseProgram(s_rib.prog);
    const float now = std::fmod(Glob.time.toFloat(), kRibbonTimeWrap);
    // The drawn sea level (XP sea flow draws a share of the sea level's wave),
    // as the sea and the shore's fringe are drawn.
    glUniform4f(s_rib.locTime, now, land->GetSeaLevelDrawn(), kRibbonFadeStart, kRibbonFadeEnd);
    glUniform4f(s_rib.locGrid, static_cast<float>(kRibbonTexWidth), 0.0f, 0.0f, 0.0f);
    GL33Bind::Tex2D(kRibbonUnit, s_rib.pointsTex);
    GL33Bind::Tex2D(kTerrainHeightUnit, _terrainHeightTex);
    glMultiDrawArrays(GL_TRIANGLES, s_rib.firsts.data(), s_rib.counts.data(),
                      static_cast<GLsizei>(s_rib.firsts.size()));

    glUseProgram(0);
    GL33Bind::Tex2D(kRibbonUnit, 0);
    GL33Bind::ActiveUnit(0);
    if (cull)
        glEnable(GL_CULL_FACE);
    if (stencil)
        glEnable(GL_STENCIL_TEST);
    _pixelShaderSel = PSNone;
    InvalidatePipelineCache();
    ApplyBlendMode(_currentBlendMode);
    ApplyDepthMode(_currentDepthMode);
    UpdateSurfaceMask(true);
}
