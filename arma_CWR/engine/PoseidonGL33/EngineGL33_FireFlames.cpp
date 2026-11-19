// The fire spread's flames (POSEIDON_XP_FIRESPREAD;
// World/Simulation/FireSpreadWorld.hpp GetFlames).
//
// Every FlameSheet is one flame tongue, drawn as an axial billboard: a
// ribbon along the tongue's bent axis (its foot sunk into the ground, no gap
// on a slope; the axis standing up from the fuel and bending over downwind),
// turned about that axis to face the eye, so no tongue ever shows a plane;
// seen along its axis it closes into a round body. A burning front is many
// of them side by side (FireSpreadWorld.cpp Draw). All of them go in one
// instanced draw from a streamed instance buffer (the sheet list copied as it
// is) at the start of the world's translucent objects
// (BeginTranslucentObjects), after the opaque world and the TAA's copy of it.
//
// The flame is a flipbook (tools/firespread/gen_flame_flipbook.py: one
// tongue a frame, 128 frames looping, in four styles: front, tongues, creep
// and bushy; the sim picks each tongue's) of physical fields: the soot's
// temperature and its optical depth (the emissivity is rebuilt from the
// temperature in the shader). They load the first time there are flames from
// fx/flame_<style>.png beside the executable into one RG texture array,
// RGTC2 (each channel compressed on its own, encoded here from the mips built
// here per layer), a layer a frame, style-major, so no mip ever bleeds across
// frames; nothing wraps (a frame's tongue never reaches its sides). The frame
// size is the atlas's (a 16 x 8 grid of frames, 1:2 tall). A missing style
// takes the first that loaded. Each tongue plays its style at its own phase
// (from its seed), the next frame blended in, with its own flip, at the rate
// that makes the flipbook's gas climb as fast as the tongue's does
// (FlameSheet::rise and lengthRef, both fixed for its life; kFlipRise is how
// far the gas climbs in a frame of each style, as the generator measured
// it): the frame is the fire's clock times that rate, so a pause freezes it
// and nothing has to be kept per tongue. So is the scroll of the fine detail
// the shader adds close up (psFireFlamesBook), in its own noise cells, so it
// climbs with the gas too. Without any style it logs once and draws the
// procedural flame (psFireFlames: domain-warped noise,
// EngineGL33_FireFlameShaders.cpp), through the same output.
//
// Linear light: the shader makes the flame's light (a black body at the
// temperature, times the emissivity) and the soot's (a medium that lets
// exp(-tau) through and scatters the sun and the sky) in scene light, and
// the draw accumulates them into a linear RGBA16F target of the frame's size
// (rgb the light added at the frame's pre-exposure, a the product of the
// transmittances), not into the coded frame. The post pass merges that target
// onto the frame decoded (camDecode, or the classic display's gamma), before
// the bloom, the camera's meter, its exposure, glare and tone curve
// (MergeFireFlames), so the camera's own glare makes the glow. The target and
// its merge exist only in frames with flames; without the post pass's targets
// at the frame's size (or with its effects off) the flames go straight into
// the frame, coded, as before.
//
// Soft particles: the world's depth is copied into the post pass's depth
// target just before the flames (the post resolve writes it again later),
// and a flame fragment fades out as it nears what is behind it
// (kFlameSoftDepth); with the linear target, which has no depth of its own,
// that fade is also its depth test.
//
// Off (the switch, no flames, classic lighting, a portrait or a 3D preview,
// night vision) nothing is made, loaded, bound or drawn.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/EngineGL33_PostPass.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Core/MatrixConversion.hpp>
#include <Poseidon/Graphics/Remaster/PhotoCamera.hpp>
#include <Poseidon/Graphics/Textures/Image.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Simulation/FireSpreadWorld.hpp>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

namespace rm = Poseidon::remaster;
using namespace Poseidon::gl33lighting;

namespace
{

// kFlameRows rows of two vertices (the vertex shader's strip).
constexpr int kFlameStripVertices = 16;
// Floats per sheet in the instance buffer: aFoot, aShape, aSeed, aOptic,
// aVolume (fix 6: a big tongue drawn as a volume) at locations 0 to 4, then
// aFar (fix 08: the level of detail's gain) at location kFlameFarLocation,
// kFlameFarFloat floats into a sheet's record.
constexpr int kFlameInstanceAttribs = 5;
constexpr int kFlameFarLocation = 5;
constexpr int kFlameFarFloat = 4 * kFlameInstanceAttribs;
constexpr int kFlameInstanceFloats = kFlameFarFloat + 4;
// More than the fire ever hands over (FireSpreadWorld.cpp kMaxSheets tongues plus kMaxBedSheets of the burning
// bed); the rest are dropped.
constexpr int kMaxFlameSheets = 24576;

// ---- The flipbook (tunables) ----
// Its files beside the executable (apps/cwr/Game/CMakeLists.txt copies them
// there), one a style in FireSpreadWorld.hpp FlameStyle order
// (gen_flame_flipbook.py STYLE_ORDER).
constexpr const char* kFlameStyleFiles[Poseidon::firespread::kFlameStyles] = {
    "fx/flame_front.png", "fx/flame_tongues.png", "fx/flame_creep.png", "fx/flame_bushy.png"};
constexpr int kFlipbookColumns = 16; // frames across the atlas ...
constexpr int kFlipbookRows = 8;     // ... and down (row 0 at the image's top); frame i at (i % 16, i / 16)
constexpr int kFlipbookChannels = 2; // uploaded: R the temperature, G the soot's optical depth
// How far the luminous gas climbs in one frame of each style, in frame heights
// (FlameStyle order; gen_flame_flipbook.py prints them: "rise"). A frame is
// the flame's length over its style's tip share (FireSpreadWorld.hpp
// kFlameTipShare, the shaders' kFlipFlameShare) tall.
constexpr float kFlipRise[Poseidon::firespread::kFlameStyles] = {0.02189f, 0.02305f, 0.01936f, 0.02288f};
constexpr float kFlipFpsMin = 2.0f;  // frames a second at least ...
constexpr float kFlipFpsMax = 60.0f; // ... and at most: past two flipbook frames a shown frame at 30 fps a tongue jumps
                                     // between unrelated shapes and strobes, which reads faster still (the consult)
constexpr float kFlipFpsNoRise = 24.0f; // a sheet without a rise: this for a 1 m flame, over sqrt(height)
// The shader's fine detail: its noise cell is kDetailCell of the sheet's
// height, within kDetailCellMin..Max metres (psFireFlamesBook's same names),
// and its lattice repeats every kDetailPeriod cells (the scroll wraps there).
constexpr float kDetailCell = 0.08f, kDetailCellMin = 0.03f, kDetailCellMax = 0.6f;
constexpr float kDetailPeriod = 289.0f;
// The flames' cover depth (psFireFlamesCoverDepth) is dithered by this share
// of itself, at least this much (m), either way, a new phase each frame.
constexpr float kCoverDitherShare = 0.2f, kCoverDitherMinM = 2.0f;

struct SheetPhase
{
    float frame = 0.0f;  // the flipbook frame it shows (0..frames)
    float scroll = 0.0f; // the detail's climb, in its noise cells (0..kDetailPeriod)
};

struct FlameProgram
{
    GLuint prog = 0;
    GLint locView = -1, locProj = -1, locCam = -1, locFog = -1, locDepthMap = -1, locOut = -1;
    GLint locExpose = -1; // flameExpose: the flames' own exposure (FlameExposureLog2)
    GLint locScreen = -1; // fix 08: flameScreen
};

struct FlameGL
{
    FlameProgram book;                               // the flipbook flame
    FlameProgram procedural;                         // the fallback
    bool bookTried = false, proceduralTried = false; // built (or failed: then never again until DeinitFireFlames)
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint bookTex = 0; // the flipbook's texture array, 0 without (the procedural flame then)
    GLuint farTex = 0;  // the far book's (fix 08), made with it
    int bookFrames = 0;
    bool bookLoadTried = false;
    std::uint32_t frame = 0xffffffffu;
    std::vector<float> staging;
    // The linear flame target (RGBA16F) and its depth target (RG16F: the
    // flames' weighted view depth and weight), half the frame's size, drawn
    // together (hdrFbo); the depth buffer the smoke after the flames is
    // tested against, with the smoke's own target (coverFbo); the merged frame
    // (RGBA16F, the frame's size; mergeTaaFbo with the TAA's opaque copy as
    // its second target, rebuilt when that texture changes); the merge's and
    // the cover depth's programs, and the frames drawn into and merged.
    GLuint hdrTex = 0, zTex = 0, coverDepthTex = 0, hdrFbo = 0, coverFbo = 0;
    // ---- Smoke over the flames (fix 02) ----
    // The smoke drawn after the flames in front of them (half size, RGBA16F,
    // coverFbo's colour): rgb its colour as the frame blended it (coded,
    // laid over), a the share of the flames' light it leaves (the product of
    // its layers' 1 - alpha). The merge dims the flames by it and keeps the
    // smoke's own colour over them.
    GLuint frontTex = 0;
    // ---- end smoke over the flames ----
    GLuint mergeTex = 0, mergeFbo = 0, mergeTaaFbo = 0, mergeTaaOpaque = 0;
    int targetW = 0, targetH = 0; // the frame's size (the flame targets are half of it, rounded up)
    bool targetsFailed = false;
    GLuint mergeProg = 0, coverProg = 0;
    bool mergeTried = false;
    std::uint32_t hdrFrame = 0xffffffffu;
    std::uint32_t mergedFrame = 0xffffffffu;
    // The half-resolution viewport the flames drew with (the smoke after them
    // draws into the linear target with it), and what a cover draw puts back.
    GLint coverViewport[4] = {};
    GLint savedViewport[4] = {};
    GLboolean savedMask[4] = {}, savedDepthMask = GL_FALSE;
    bool savedScissor = false;
    // The flames' own exposure over the camera's this frame, log2
    // (FireFlameExposureLog2; 0 by day and without the photoreal camera).
    float exposeLog2 = 0.0f;
};
FlameGL s_flames;

GLuint CompileFlameShader(GLenum type, const char* src, const char* name)
{
    std::string overrideSource;
    if (LoadShaderOverride(name, overrideSource))
        src = overrideSource.c_str();
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 fire flames: shader compile error [{}]: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint LinkFlameProgram(GLuint vs, GLuint fs)
{
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
        char log[1024];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 fire flames: program link error: {}", log);
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

// One variant's program, built the first time it is wanted.
bool EnsureFlameProgram(bool flipbook)
{
    FlameGL& f = s_flames;
    FlameProgram& fp = flipbook ? f.book : f.procedural;
    bool& tried = flipbook ? f.bookTried : f.proceduralTried;
    if (tried)
        return fp.prog != 0;
    tried = true;
    const GLuint p = LinkFlameProgram(CompileFlameShader(GL_VERTEX_SHADER, FireFlameVertexSource(flipbook),
                                                         flipbook ? "vsFireFlamesBook" : "vsFireFlames"),
                                      CompileFlameShader(GL_FRAGMENT_SHADER, FireFlameFragmentSource(flipbook),
                                                         flipbook ? "psFireFlamesBook" : "psFireFlames"));
    if (!p)
        return false;
    const GLuint lit = glGetUniformBlockIndex(p, "LitConstants");
    if (lit != GL_INVALID_INDEX)
        glUniformBlockBinding(p, lit, kLitBinding);
    fp.locView = glGetUniformLocation(p, "flameView");
    fp.locProj = glGetUniformLocation(p, "flameProj");
    fp.locCam = glGetUniformLocation(p, "flameCam");
    fp.locFog = glGetUniformLocation(p, "flameFog");
    fp.locDepthMap = glGetUniformLocation(p, "flameDepthMap");
    fp.locOut = glGetUniformLocation(p, "flameOut");
    fp.locExpose = glGetUniformLocation(p, "flameExpose");
    fp.locScreen = glGetUniformLocation(p, "flameScreen");
    GLint prevProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProg);
    glUseProgram(p);
    const GLint book = glGetUniformLocation(p, "flameBook");
    if (book >= 0)
        glUniform1i(book, kFlameBookUnit);
    const GLint farBook = glGetUniformLocation(p, "flameFar");
    if (farBook >= 0)
        glUniform1i(farBook, kFlameFarUnit);
    const GLint depth = glGetUniformLocation(p, "flameDepth");
    if (depth >= 0)
        glUniform1i(depth, kFlameDepthUnit);
    glUseProgram(static_cast<GLuint>(prevProg));
    fp.prog = p;
    LOG_INFO(Graphics, "GL33 fire flames: {} program built", flipbook ? "flipbook" : "procedural");
    return true;
}

// The instance buffer behind four per-instance attributes; no per-vertex
// attribute (the strip's corners come from gl_VertexID).
void EnsureFlameVertexArray()
{
    FlameGL& f = s_flames;
    if (f.vao)
        return;
    GLint prevVao = 0, prevBuf = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevBuf);
    glGenVertexArrays(1, &f.vao);
    glGenBuffers(1, &f.vbo);
    Poseidon::GL33Bind::Vao(f.vao);
    glBindBuffer(GL_ARRAY_BUFFER, f.vbo);
    const GLsizei stride = kFlameInstanceFloats * static_cast<GLsizei>(sizeof(float));
    for (int i = 0; i < kFlameInstanceAttribs; ++i)
    {
        glEnableVertexAttribArray(static_cast<GLuint>(i));
        glVertexAttribPointer(static_cast<GLuint>(i), 4, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<const void*>(static_cast<std::size_t>(i) * 4 * sizeof(float)));
        glVertexAttribDivisor(static_cast<GLuint>(i), 1);
    }
    // (fix 08: the far flames' aFar)
    glEnableVertexAttribArray(static_cast<GLuint>(kFlameFarLocation));
    glVertexAttribPointer(static_cast<GLuint>(kFlameFarLocation), 4, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(static_cast<std::size_t>(kFlameFarFloat) * sizeof(float)));
    glVertexAttribDivisor(static_cast<GLuint>(kFlameFarLocation), 1);
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(prevBuf));
    Poseidon::GL33Bind::Vao(static_cast<GLuint>(prevVao));
}

// One style's atlas as RG layers (R the temperature, G the soot's optical
// depth), a layer a frame, rows bottom up (texture v 0 at the fuel bed).
// False when the file is missing or not a 16 x 8 atlas of frames (of `wantW`
// x `wantH` texels when wantW is not 0).
bool LoadFlameStyle(const std::string& path, int wantW, int wantH, int& fw, int& fh, std::vector<std::uint8_t>& out)
{
    const Poseidon::Image img = Poseidon::Image::FromFile(path);
    fw = img.valid() ? img.width() / kFlipbookColumns : 0;
    fh = img.valid() ? img.height() / kFlipbookRows : 0;
    if (!img.valid() || img.format() != Poseidon::PixelFormat::RGBA8888 || fw <= 0 || fh <= 0 ||
        fw * kFlipbookColumns != img.width() || fh * kFlipbookRows != img.height() ||
        (wantW != 0 && (fw != wantW || fh != wantH)))
        return false;
    const int frames = kFlipbookColumns * kFlipbookRows;
    out.resize(static_cast<std::size_t>(fw) * fh * kFlipbookChannels * frames);
    const std::uint8_t* src = img.data().data();
    const std::size_t srcStride = static_cast<std::size_t>(img.width()) * 4;
    std::uint8_t* to = out.data();
    for (int i = 0; i < frames; ++i)
    {
        const int col = i % kFlipbookColumns, row = i / kFlipbookColumns;
        for (int y = 0; y < fh; ++y)
        {
            const std::uint8_t* from = src + static_cast<std::size_t>(row * fh + (fh - 1 - y)) * srcStride +
                                       static_cast<std::size_t>(col) * fw * 4;
            for (int x = 0; x < fw; ++x, from += 4, to += kFlipbookChannels)
            {
                to[0] = from[0];
                to[1] = from[1];
            }
        }
    }
    return true;
}

// The next mip of `layers` RG layers of `w` x `h` texels: a box over each
// 2 x 2 (the texels there are, at an odd size), per layer.
std::vector<std::uint8_t> HalveFlameLayers(const std::vector<std::uint8_t>& src, int w, int h, int layers)
{
    const int nw = std::max(w / 2, 1), nh = std::max(h / 2, 1);
    constexpr int ch = kFlipbookChannels;
    std::vector<std::uint8_t> dst(static_cast<std::size_t>(nw) * nh * ch * static_cast<std::size_t>(layers));
    for (int l = 0; l < layers; ++l)
    {
        const std::uint8_t* s = src.data() + static_cast<std::size_t>(l) * w * h * ch;
        std::uint8_t* d = dst.data() + static_cast<std::size_t>(l) * nw * nh * ch;
        for (int y = 0; y < nh; ++y)
            for (int x = 0; x < nw; ++x)
                for (int c = 0; c < ch; ++c)
                {
                    int sum = 0;
                    for (int dy = 0; dy < 2; ++dy)
                        for (int dx = 0; dx < 2; ++dx)
                        {
                            const int sx = std::min(2 * x + dx, w - 1), sy = std::min(2 * y + dy, h - 1);
                            sum += s[(static_cast<std::size_t>(sy) * w + sx) * ch + c];
                        }
                    d[(static_cast<std::size_t>(y) * nw + x) * ch + c] = static_cast<std::uint8_t>((sum + 2) / 4);
                }
    }
    return dst;
}

// One 4 x 4 block of one channel as BC4 (RGTC1, unsigned): the block's
// maximum and minimum as its two endpoints (the eight-value mode), each texel
// the nearest of the eight levels between them, 3 bits each.
void EncodeBC4Block(const std::uint8_t v[16], std::uint8_t out[8])
{
    int lo = 255, hi = 0;
    for (int i = 0; i < 16; ++i)
    {
        lo = std::min(lo, static_cast<int>(v[i]));
        hi = std::max(hi, static_cast<int>(v[i]));
    }
    out[0] = static_cast<std::uint8_t>(hi);
    out[1] = static_cast<std::uint8_t>(lo);
    std::uint64_t bits = 0;
    if (hi > lo)
        for (int i = 0; i < 16; ++i)
        {
            // Steps from the maximum toward the minimum, 0..7; index 0 is the
            // maximum, 1 the minimum, 2..7 the six levels between.
            const int step = ((hi - static_cast<int>(v[i])) * 7 + (hi - lo) / 2) / (hi - lo);
            const std::uint64_t idx = step == 0 ? 0u : (step == 7 ? 1u : static_cast<std::uint64_t>(step + 1));
            bits |= idx << (3 * i);
        }
    for (int b = 0; b < 6; ++b)
        out[2 + b] = static_cast<std::uint8_t>(bits >> (8 * b));
}

// `layers` RG layers of `w` x `h` texels as BC5 (RGTC2: a BC4 block of R,
// then one of G, 16 bytes a 4 x 4 block); a level smaller than a block
// repeats its edge texels.
std::vector<std::uint8_t> EncodeRgtc2Layers(const std::vector<std::uint8_t>& rg, int w, int h, int layers)
{
    const int bw = (w + 3) / 4, bh = (h + 3) / 4;
    std::vector<std::uint8_t> out(static_cast<std::size_t>(layers) * bw * bh * 16);
    std::uint8_t* to = out.data();
    std::uint8_t r[16], g[16];
    for (int l = 0; l < layers; ++l)
    {
        const std::uint8_t* src = rg.data() + static_cast<std::size_t>(l) * w * h * kFlipbookChannels;
        for (int by = 0; by < bh; ++by)
            for (int bx = 0; bx < bw; ++bx, to += 16)
            {
                for (int y = 0; y < 4; ++y)
                    for (int x = 0; x < 4; ++x)
                    {
                        const int sx = std::min(bx * 4 + x, w - 1), sy = std::min(by * 4 + y, h - 1);
                        const std::uint8_t* t = src + (static_cast<std::size_t>(sy) * w + sx) * kFlipbookChannels;
                        r[y * 4 + x] = t[0];
                        g[y * 4 + x] = t[1];
                    }
                EncodeBC4Block(r, to);
                EncodeBC4Block(g, to + 8);
            }
    }
    return out;
}

// POSEIDON_XP_FIRESPREAD_FLIPDEBUG=1: the flipbook uncompressed (RG8) and
// read at its full resolution only, to tell a texture fault from a filtering
// one in a still. Read once.
bool FlipbookDebug()
{
    static const bool on = []
    {
        const char* v = std::getenv("POSEIDON_XP_FIRESPREAD_FLIPDEBUG");
        return v && v[0] == '1';
    }();
    return on;
}

// POSEIDON_XP_FIRESPREAD_FLAMEDEBUG bits, read once (diagnosis only): 1 the
// merge skips the fog, 2 no smoke cover, 4 the upsample keeps every texel,
// 8 the full resolution, 16 the merge shows the smoke's transmittance over the
// flames as grey, 32 8 px tiles in turn show the merge, the flames laid over
// the smoke as before, and that transmittance (smoke over the flames, fix 02).
int FlameDebugBits()
{
    static const int bits = []
    {
        const char* v = std::getenv("POSEIDON_XP_FIRESPREAD_FLAMEDEBUG");
        return v ? std::atoi(v) : 0;
    }();
    return bits;
}

// The flipbook's preparation off the render thread: each style's file read,
// its mips built and each level encoded (RGTC2, or kept RG8 with
// FlipbookDebug) on a worker thread started as soon as the fire is on and the
// world draws (StartFlipbookPrep), long before any flame, so the first flames
// never wait for it (v4's first flames hitched about 0.36 s on it). A style
// whose file is missing takes the first style that loaded.
struct FlipPrep
{
    std::thread worker;
    std::atomic<bool> done{false};
    bool started = false;
    int fw = 0, fh = 0, levels = 0;
    // [style][level]: what goes to the GL as it is (RGTC2 blocks, or RG8 texels).
    std::vector<std::vector<std::uint8_t>> data[Poseidon::firespread::kFlameStyles];
    // [style][level]: the far book (fix 08), RGBA half floats, its frames
    // farW x farH, farLevels of them down to 1 x 1.
    std::vector<std::vector<std::uint16_t>> farData[Poseidon::firespread::kFlameStyles];
    int farW = 0, farH = 0, farLevels = 0;
    bool loaded[Poseidon::firespread::kFlameStyles] = {};
    int uploaded = -1; // styles uploaded so far (-1: the array not made yet)
    ~FlipPrep() { Join(); }
    void Join()
    {
        if (worker.joinable())
            worker.join();
    }
};
FlipPrep s_flipPrep;

// ---- The far book (fix 08; the realism audit's C13) ----
// The flipbook's mips average its kelvin with the air round a tongue before
// the shader's luminous-zone threshold, so a far tongue lost its light. The
// far book holds, per frame and kFarBookDiv times smaller each way, the
// fields that are linear in what a tongue makes on screen
// (gl33lighting::FlameFarTexel), so its box mips (down to 1 x 1, a frame's
// mean) keep a far tongue's light exactly; psFireFlamesBook reads it past
// kFarFromLod of the flipbook's mip, and its 1 x 1 level for a far splat.
constexpr int kFarBookDiv = 4;
constexpr int kFarBookChannels = 4;

// A float (0 and up, the far book's range) as an IEEE half, rounded; under
// the half's smallest normal it goes to 0, past its largest to the largest.
std::uint16_t FloatToHalf(float v)
{
    if (!(v > 6.1035156e-5f))
        return 0;
    if (v >= 65504.0f)
        return 0x7bff;
    int e = 0;
    const float m = std::frexp(v, &e); // v = m 2^e, m in [0.5, 1)
    const int exp = e - 1 + 15;        // the half's biased exponent
    const int mant = static_cast<int>(std::lround((m * 2.0f - 1.0f) * 1024.0f));
    return mant >= 1024 ? static_cast<std::uint16_t>((exp + 1) << 10) : static_cast<std::uint16_t>((exp << 10) | mant);
}

// Every level of one style's far book from its flipbook level 0 (`rg`:
// `frames` RG layers of w x h): each texel the mean of FlameFarTexel over the
// kFarBookDiv x kFarBookDiv flipbook texels under it, then box mips down to
// 1 x 1, as half floats; fw x fh its frame's size, levels their number.
std::vector<std::vector<std::uint16_t>> BuildFarBook(const std::vector<std::uint8_t>& rg, int w, int h, int frames,
                                                     int& fw, int& fh, int& levels)
{
    // R is 8 bits: its zone, relative luminance and cover by table.
    float lut[256][kFarBookChannels];
    for (int r = 0; r < 256; ++r)
        Poseidon::gl33lighting::FlameFarTexel(static_cast<unsigned char>(r), 0, lut[r]);
    fw = std::max(w / kFarBookDiv, 1);
    fh = std::max(h / kFarBookDiv, 1);
    std::vector<float> level(static_cast<std::size_t>(fw) * fh * kFarBookChannels * frames, 0.0f);
    for (int l = 0; l < frames; ++l)
    {
        const std::uint8_t* s = rg.data() + static_cast<std::size_t>(l) * w * h * kFlipbookChannels;
        float* d = level.data() + static_cast<std::size_t>(l) * fw * fh * kFarBookChannels;
        for (int y = 0; y < fh; ++y)
            for (int x = 0; x < fw; ++x)
            {
                float acc[kFarBookChannels] = {};
                int n = 0;
                for (int dy = 0; dy < kFarBookDiv; ++dy)
                    for (int dx = 0; dx < kFarBookDiv; ++dx)
                    {
                        const int sx = std::min(x * kFarBookDiv + dx, w - 1),
                                  sy = std::min(y * kFarBookDiv + dy, h - 1);
                        const std::uint8_t* t = s + (static_cast<std::size_t>(sy) * w + sx) * kFlipbookChannels;
                        const float* f = lut[t[0]];
                        acc[0] += f[0];
                        acc[1] += f[1];
                        acc[2] += f[2];
                        acc[3] += (static_cast<float>(t[1]) / 255.0f) * (1.0f - f[0]);
                        ++n;
                    }
                for (int c = 0; c < kFarBookChannels; ++c)
                    d[(static_cast<std::size_t>(y) * fw + x) * kFarBookChannels + c] = acc[c] / static_cast<float>(n);
            }
    }
    std::vector<std::vector<std::uint16_t>> out;
    int lw = fw, lh = fh;
    for (;;)
    {
        std::vector<std::uint16_t> half(level.size());
        for (std::size_t i = 0; i < level.size(); ++i)
            half[i] = FloatToHalf(level[i]);
        out.push_back(std::move(half));
        if (lw == 1 && lh == 1)
            break;
        const int nw = std::max(lw / 2, 1), nh = std::max(lh / 2, 1);
        std::vector<float> next(static_cast<std::size_t>(nw) * nh * kFarBookChannels * frames);
        for (int l = 0; l < frames; ++l)
        {
            const float* s = level.data() + static_cast<std::size_t>(l) * lw * lh * kFarBookChannels;
            float* d = next.data() + static_cast<std::size_t>(l) * nw * nh * kFarBookChannels;
            for (int y = 0; y < nh; ++y)
                for (int x = 0; x < nw; ++x)
                    for (int c = 0; c < kFarBookChannels; ++c)
                    {
                        float sum = 0.0f;
                        for (int dy = 0; dy < 2; ++dy)
                            for (int dx = 0; dx < 2; ++dx)
                            {
                                const int sx = std::min(2 * x + dx, lw - 1), sy = std::min(2 * y + dy, lh - 1);
                                sum += s[(static_cast<std::size_t>(sy) * lw + sx) * kFarBookChannels + c];
                            }
                        d[(static_cast<std::size_t>(y) * nw + x) * kFarBookChannels + c] = 0.25f * sum;
                    }
        }
        level.swap(next);
        lw = nw;
        lh = nh;
    }
    levels = static_cast<int>(out.size());
    return out;
}

void PrepareFlipbook(FlipPrep* prep, std::string dir, bool debug)
{
    const int frames = kFlipbookColumns * kFlipbookRows;
    int first = -1;
    for (int st = 0; st < Poseidon::firespread::kFlameStyles; ++st)
    {
        const std::string path = dir + kFlameStyleFiles[st];
        std::vector<std::uint8_t> level;
        int w = 0, h = 0;
        if (!LoadFlameStyle(path, prep->fw, prep->fh, w, h, level))
            continue;
        if (first < 0)
        {
            first = st;
            prep->fw = w;
            prep->fh = h;
            prep->levels = 0;
            while ((std::max(w, h) >> prep->levels) > 0)
                ++prep->levels;
        }
        prep->loaded[st] = true;
        // (fix 08: the far book, from the flipbook's full resolution)
        prep->farData[st] = BuildFarBook(level, w, h, frames, prep->farW, prep->farH, prep->farLevels);
        for (int lv = 0; lv < prep->levels; ++lv)
        {
            prep->data[st].push_back(debug ? level : EncodeRgtc2Layers(level, w, h, frames));
            if (lv + 1 < prep->levels)
            {
                level = HalveFlameLayers(level, w, h, frames);
                w = std::max(w / 2, 1);
                h = std::max(h / 2, 1);
            }
        }
    }
    if (first >= 0)
        for (int st = 0; st < Poseidon::firespread::kFlameStyles; ++st)
            if (!prep->loaded[st])
            {
                prep->data[st] = prep->data[first];
                prep->farData[st] = prep->farData[first];
            }
    prep->done.store(true, std::memory_order_release);
}

// Starts the preparation once (the fire on, a world drawing).
void StartFlipbookPrep()
{
    FlipPrep& p = s_flipPrep;
    if (p.started)
        return;
    p.started = true;
    const char* base = SDL_GetBasePath();
    p.worker = std::thread(PrepareFlipbook, &p, std::string(base ? base : ""), FlipbookDebug());
}

// Uploads the prepared flipbook into the GL: the array first, then `styles`
// styles at most this call (one a frame ahead of the flames, so no frame
// carries the whole upload), or all of them. True once every style is in.
bool UploadFlipbook(int styles)
{
    FlameGL& f = s_flames;
    FlipPrep& p = s_flipPrep;
    const int frames = kFlipbookColumns * kFlipbookRows;
    const int layers = frames * Poseidon::firespread::kFlameStyles;
    const bool debug = FlipbookDebug();
    if (p.uploaded >= Poseidon::firespread::kFlameStyles)
        return true;
    GLint alignment = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    Poseidon::GL33Bind::ActiveUnit(kFlameBookUnit);
    if (p.uploaded < 0)
    {
        while (glGetError() != GL_NO_ERROR)
        {
        }
        // The array at the first style's frame size, every level allocated.
        glGenTextures(1, &f.bookTex);
        glBindTexture(GL_TEXTURE_2D_ARRAY, f.bookTex);
        for (int lv = 0; lv < p.levels; ++lv)
        {
            const int lw = std::max(p.fw >> lv, 1), lh = std::max(p.fh >> lv, 1);
            const int bw = (lw + 3) / 4, bh = (lh + 3) / 4;
            if (debug)
                glTexImage3D(GL_TEXTURE_2D_ARRAY, lv, GL_RG8, lw, lh, layers, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr);
            else
                glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, lv, GL_COMPRESSED_RG_RGTC2, lw, lh, layers, 0,
                                       layers * bw * bh * 16, nullptr);
        }
        // (fix 08: the far book on its own unit, every level allocated, its
        // filtering set now: nothing samples it before the last style is in)
        Poseidon::GL33Bind::ActiveUnit(kFlameFarUnit);
        glGenTextures(1, &f.farTex);
        glBindTexture(GL_TEXTURE_2D_ARRAY, f.farTex);
        for (int lv = 0; lv < p.farLevels; ++lv)
            glTexImage3D(GL_TEXTURE_2D_ARRAY, lv, GL_RGBA16F, std::max(p.farW >> lv, 1), std::max(p.farH >> lv, 1),
                         layers, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, std::max(p.farLevels - 1, 0));
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindSampler(static_cast<GLuint>(kFlameFarUnit), 0);
        Poseidon::GL33Bind::ActiveUnit(kFlameBookUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, f.bookTex);
        p.uploaded = 0;
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, f.bookTex);
    }
    for (int n = 0; n < styles && p.uploaded < Poseidon::firespread::kFlameStyles; ++n, ++p.uploaded)
    {
        const int st = p.uploaded;
        for (int lv = 0; lv < p.levels && lv < static_cast<int>(p.data[st].size()); ++lv)
        {
            const int w = std::max(p.fw >> lv, 1), h = std::max(p.fh >> lv, 1);
            const std::vector<std::uint8_t>& level = p.data[st][lv];
            if (debug)
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, lv, 0, 0, st * frames, w, h, frames, GL_RG, GL_UNSIGNED_BYTE,
                                level.data());
            else
                glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, lv, 0, 0, st * frames, w, h, frames,
                                          GL_COMPRESSED_RG_RGTC2, static_cast<GLsizei>(level.size()), level.data());
        }
        std::vector<std::vector<std::uint8_t>>().swap(p.data[st]); // the GL holds it now
        // (fix 08: its far book)
        Poseidon::GL33Bind::ActiveUnit(kFlameFarUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, f.farTex);
        for (int lv = 0; lv < p.farLevels && lv < static_cast<int>(p.farData[st].size()); ++lv)
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, lv, 0, 0, st * frames, std::max(p.farW >> lv, 1),
                            std::max(p.farH >> lv, 1), frames, GL_RGBA, GL_HALF_FLOAT, p.farData[st][lv].data());
        std::vector<std::vector<std::uint16_t>>().swap(p.farData[st]);
        Poseidon::GL33Bind::ActiveUnit(kFlameBookUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, f.bookTex);
    }
    const bool all = p.uploaded >= Poseidon::firespread::kFlameStyles;
    if (all)
    {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, p.levels - 1);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,
                        GL_CLAMP_TO_EDGE); // one tongue a frame, nothing at its sides
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindSampler(static_cast<GLuint>(kFlameBookUnit), 0);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    Poseidon::GL33Bind::ActiveUnit(0);
    if (all)
    {
        const GLenum error = glGetError();
        f.bookFrames = frames;
        int missing = 0;
        for (bool l : p.loaded)
            missing += l ? 0 : 1;
        if (error != GL_NO_ERROR)
            LOG_WARN(Graphics, "GL33 fire flames: the flipbook's upload raised GL error 0x{:04x}",
                     static_cast<unsigned>(error));
        LOG_INFO(Graphics,
                 "GL33 fire flames: flipbook of {} styles ({} layers of {}x{}, {} mips, {}; {} missing), far book "
                 "{}x{} RGBA16F, {} mips",
                 Poseidon::firespread::kFlameStyles, layers, p.fw, p.fh, p.levels,
                 debug ? "RG8, full resolution only (FLIPDEBUG)" : "RGTC2, encoded on a worker", missing, p.farW,
                 p.farH, p.farLevels);
    }
    return all;
}

// Ahead of the flames, every frame the fire is on: starts the preparation,
// and once it is done uploads one style a frame. Never waits.
void FlipbookAhead()
{
    FlameGL& f = s_flames;
    StartFlipbookPrep();
    if (f.bookLoadTried || !s_flipPrep.done.load(std::memory_order_acquire))
        return;
    if (s_flipPrep.levels == 0)
        return; // nothing loaded: EnsureFlipbook logs it with the first flames
    if (UploadFlipbook(1))
    {
        f.bookLoadTried = true;
        // Its program too, a frame before any flame needs it (a shader
        // compile is the other part of the first flames' hitch).
        EnsureFlameProgram(true);
    }
}

// The flipbook's texture array (every style, style-major: layer = style * 128 +
// frame), ready for the first frame with flames: RGTC2, encoded level by
// level (each channel compressed on its own) and uploaded as compressed data,
// never left to the driver: an uncompressed upload into a compressed format
// is a driver's to refuse, and then the levels keep their undefined contents
// (the flat blocks of the first in-game run). RG8 with FlipbookDebug. If the
// flames come before FlipbookAhead has it all in, this waits for the worker
// and uploads the rest at once. False, logged once, when no style loaded: the
// procedural flame draws then.
bool EnsureFlipbook()
{
    FlameGL& f = s_flames;
    if (f.bookLoadTried)
        return f.bookTex != 0 && f.bookFrames > 0;
    f.bookLoadTried = true;
    StartFlipbookPrep();
    s_flipPrep.Join();
    if (s_flipPrep.levels == 0)
    {
        LOG_WARN(Graphics,
                 "GL33 fire flames: no flipbook style loaded from fx/flame_*.png; the procedural flames draw");
        return false;
    }
    UploadFlipbook(Poseidon::firespread::kFlameStyles);
    return f.bookTex != 0;
}

// The frame's pixels per flame target pixel across: 2 (half resolution), 1
// with FLAMEDEBUG 8 (the full resolution, to compare).
int FlameScale()
{
    return (FlameDebugBits() & 8) ? 1 : 2;
}

// The flame targets' size: the frame's over FlameScale, rounded up (at 2 the
// post pass's half-resolution targets are the same).
int FlameHalf(int n)
{
    return (n + FlameScale() - 1) / FlameScale();
}

void DeleteFlameTargets()
{
    FlameGL& f = s_flames;
    for (GLuint* fb : {&f.hdrFbo, &f.coverFbo, &f.mergeFbo, &f.mergeTaaFbo})
        if (*fb)
        {
            glDeleteFramebuffers(1, fb);
            *fb = 0;
        }
    for (GLuint* t : {&f.hdrTex, &f.zTex, &f.coverDepthTex, &f.frontTex, &f.mergeTex})
        if (*t)
        {
            Poseidon::GL33Bind::OnTexDeleted(*t);
            glDeleteTextures(1, t);
            *t = 0;
        }
    f.mergeTaaOpaque = 0;
}

// One flame target's texture, bound on the upload unit.
GLuint MakeFlameTexture(GLint format, GLenum layout, GLenum type, int w, int h, GLint filter)
{
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, layout, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    return tex;
}

// The flame targets for a frame of w x h (the linear target, its depth target
// and the cover's depth at half of it, the merged frame at it), and the
// merge's and the cover depth's programs; made in the first frame with
// flames, remade at a new size.
bool EnsureFlameTargets(int w, int h)
{
    FlameGL& f = s_flames;
    if (!f.mergeTried)
    {
        f.mergeTried = true;
        auto post = [](const char* src, const char* name)
        {
            return LinkFlameProgram(CompileFlameShader(GL_VERTEX_SHADER, PostShaderSource(PostShader::Vertex),
                                                       PostShaderName(PostShader::Vertex)),
                                    CompileFlameShader(GL_FRAGMENT_SHADER, src, name));
        };
        f.mergeProg = post(FireFlameMergeSource(), "psFireFlamesMerge");
        f.coverProg = post(FireFlameCoverDepthSource(), "psFireFlamesCoverDepth");
    }
    if (!f.mergeProg || !f.coverProg)
        return false;
    if (f.targetW == w && f.targetH == h && f.hdrFbo && f.coverFbo && f.mergeFbo)
        return true;
    if (f.targetsFailed && f.targetW == w && f.targetH == h)
        return false;
    DeleteFlameTargets();
    f.targetW = w;
    f.targetH = h;
    const int hw = FlameHalf(w), hh = FlameHalf(h);
    GLint prevDraw = 0, prevRead = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
    Poseidon::GL33Bind::ActiveUnit(static_cast<int>(EngineGL33::kUploadUnit - GL_TEXTURE0));
    f.hdrTex = MakeFlameTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, GL_NEAREST);
    f.zTex = MakeFlameTexture(GL_RG16F, GL_RG, GL_HALF_FLOAT, hw, hh, GL_NEAREST);
    f.coverDepthTex = MakeFlameTexture(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, hw, hh, GL_NEAREST);
    // Smoke over the flames (fix 02): the smoke in front of the flames.
    f.frontTex = MakeFlameTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, GL_NEAREST);
    f.mergeTex = MakeFlameTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, w, h, GL_LINEAR);
    bool complete = true;
    auto fbo = [&](GLuint& id, GLuint color0, GLuint color1, GLuint depth)
    {
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color0, 0);
        if (color1)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, color1, 0);
        if (depth)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
        const GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(color1 ? 2 : 1, bufs);
        complete = complete && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    };
    fbo(f.hdrFbo, f.hdrTex, f.zTex, 0);
    fbo(f.coverFbo, f.frontTex, 0, f.coverDepthTex); // smoke over the flames (fix 02): the smoke's own target
    fbo(f.mergeFbo, f.mergeTex, 0, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    Poseidon::GL33Bind::Invalidate();
    Poseidon::GL33Bind::ActiveUnit(0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(prevRead));
    if (!complete)
    {
        LOG_WARN(Graphics, "GL33 fire flames: linear targets {}x{} incomplete; the flames draw into the frame", hw, hh);
        f.targetsFailed = true;
        return false;
    }
    f.targetsFailed = false;
    LOG_INFO(Graphics, "GL33 fire flames: linear targets {}x{} (RGBA16F, RG16F depth), merge {}x{}", hw, hh, w, h);
    return true;
}

// The merge's framebuffer with the TAA's opaque copy as its second target
// (blended there by the mark's alpha), remade when that texture changes; 0
// when it cannot be made.
GLuint FlameMergeTaaFbo(GLuint opaqueTex)
{
    FlameGL& f = s_flames;
    if (!opaqueTex || !f.mergeTex)
        return 0;
    if (f.mergeTaaFbo && f.mergeTaaOpaque == opaqueTex)
        return f.mergeTaaFbo;
    if (f.mergeTaaFbo)
        glDeleteFramebuffers(1, &f.mergeTaaFbo);
    f.mergeTaaFbo = 0;
    f.mergeTaaOpaque = opaqueTex;
    GLuint id = 0;
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, f.mergeTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, opaqueTex, 0);
    const GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, bufs);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1, &id);
        LOG_WARN(Graphics, "GL33 fire flames: the merge with the TAA's opaque copy is incomplete; no TAA mark");
        return 0;
    }
    f.mergeTaaFbo = id;
    return id;
}

// How the frame's tongues draw (fix 08), by vsFireFlamesBook's own sizes
// (flameFarState): splats (their ribbon under kCountSplatPx of the flame
// target's pixels), far-book ribbons (from kCountFarLod of the near book's
// mip), near ones; for the stats line (firespread::NoteFlameDraw). pxAtOneM
// is a metre's size in the target's pixels a metre off.
constexpr float kCountSplatPx = 4.5f, kCountFarLod = 2.0f;
void CountFarFlames(const Poseidon::firespread::FlameView& view, int count, float pxAtOneM, float cx, float cy,
                    float cz)
{
    const float fh = static_cast<float>(std::max(s_flipPrep.fh, 1)),
                fw = static_cast<float>(std::max(s_flipPrep.fw, 1));
    int nearN = 0, farN = 0, splatN = 0;
    for (int i = 0; i < count; ++i)
    {
        const Poseidon::firespread::FlameSheet& s = view.sheets[i];
        if (static_cast<int>(s.style) >= Poseidon::firespread::kFlameStyles)
            continue; // (fix 7's beds: not flipbook tongues)
        const float len = std::max(std::sqrt(s.height * s.height + s.leanX * s.leanX + s.leanZ * s.leanZ), 0.05f);
        const float frameH = len / Poseidon::firespread::FlameTipShare(static_cast<int>(s.style));
        const float mx = s.x - cx + 0.5f * s.leanX, my = s.y - cy + 0.5f * s.height, mz = s.z - cz + 0.5f * s.leanZ;
        const float px = pxAtOneM / std::max(std::sqrt(mx * mx + my * my + mz * mz), 0.1f);
        const float tall = frameH * px, wide = std::max(s.width, 0.02f) * px;
        if (std::sqrt(tall * wide) < kCountSplatPx)
            ++splatN;
        else if (std::log2(std::max(std::max(fh / std::max(tall, 1e-3f), fw / std::max(wide, 1e-3f)), 1.0f)) >=
                 kCountFarLod)
            ++farN;
        else
            ++nearN;
    }
    Poseidon::firespread::NoteFlameDraw(nearN, farN, splatN);
}

// The flipbook frame of each tongue and its detail's scroll on the fire's
// clock: from a phase of its seed, at its own rate (its gas's rise over the
// distance the style's gas climbs in a frame, kFlipRise of a frame
// lengthRef / kFlipFlameShare tall). Both the rise and lengthRef are fixed
// for the tongue's life, so the frame never jumps and nothing is kept per
// tongue; the clock stops with the game.
SheetPhase SheetFrame(const Poseidon::firespread::FlameSheet& s, double time)
{
    // The burning bed (FlameStyle::Bed) is drawn procedurally: no flipbook to play.
    if (static_cast<int>(s.style) >= Poseidon::firespread::kFlameStyles)
        return SheetPhase{};
    const FlameGL& f = s_flames;
    const float length = std::max(s.lengthRef, 0.05f);
    const int style = std::clamp(static_cast<int>(s.style), 0, Poseidon::firespread::kFlameStyles - 1);
    const float perFrame = kFlipRise[style] * length / Poseidon::firespread::FlameTipShare(style); // m a frame
    const float fps =
        std::clamp(s.rise > 0.0f ? s.rise / perFrame : kFlipFpsNoRise / std::sqrt(length), kFlipFpsMin, kFlipFpsMax);
    const double frames = static_cast<double>(f.bookFrames);
    SheetPhase p;
    p.frame = static_cast<float>(std::fmod(static_cast<double>(s.seed) * frames + time * fps, frames));
    const float cell = std::clamp(kDetailCell * length, kDetailCellMin, kDetailCellMax);
    const float rise = s.rise > 0.0f ? s.rise : perFrame * fps;
    p.scroll = static_cast<float>(std::fmod(static_cast<double>(s.seed) * kDetailPeriod + time * (rise / cell),
                                            static_cast<double>(kDetailPeriod)));
    return p;
}
} // namespace

void EngineGL33::DeinitFireFlames()
{
    FlameGL& f = s_flames;
    for (GLuint p : {f.book.prog, f.procedural.prog, f.mergeProg, f.coverProg})
        if (p)
            glDeleteProgram(p);
    if (f.vao)
    {
        Poseidon::GL33Bind::OnVaoDeleted(f.vao);
        glDeleteVertexArrays(1, &f.vao);
    }
    if (f.vbo)
        glDeleteBuffers(1, &f.vbo);
    DeinitFireSparks();
    DeleteFlameTargets();
    if (f.bookTex)
    {
        Poseidon::GL33Bind::OnTexDeleted(f.bookTex);
        glDeleteTextures(1, &f.bookTex);
    }
    if (f.farTex)
    {
        Poseidon::GL33Bind::OnTexDeleted(f.farTex);
        glDeleteTextures(1, &f.farTex);
    }
    f = FlameGL{};
    // The prepared flipbook goes with the context (a new one prepares it again).
    s_flipPrep.Join();
    for (auto& d : s_flipPrep.data)
        d.clear();
    for (auto& d : s_flipPrep.farData)
        d.clear();
    s_flipPrep.farW = s_flipPrep.farH = s_flipPrep.farLevels = 0;
    s_flipPrep.started = false;
    s_flipPrep.done.store(false);
    s_flipPrep.uploaded = -1;
    s_flipPrep.fw = s_flipPrep.fh = s_flipPrep.levels = 0;
    for (bool& l : s_flipPrep.loaded)
        l = false;
    _flameCoverFrame = 0xffffffffu;
}

void EngineGL33::DrawFireFlames()
{
    // Only in this frame's own world pass under the remastered lighting (the
    // flames read the lit constants), not in a portrait, a 3D preview or the
    // night-vision intensifier (_worldEffectsOn), and once a frame.
    if (!_glContext || !_worldPassOpen || !_worldEffectsOn || _worldSceneEnded || _portraitTargetSize ||
        !_remasterPrograms || !_lightingActive)
        return;
    // The fire on: the flipbook gets ready ahead of the first flames.
    if (Poseidon::firespread::Enabled())
        FlipbookAhead();
    Poseidon::firespread::FlameView view;
    if (!Poseidon::firespread::GetFlames(view) || !view.sheets || view.count <= 0)
    {
        if (Poseidon::firespread::Enabled())
            Poseidon::firespread::NoteFlameDraw(0, 0, 0); // (fix 08: no flames this frame)
        return;
    }
    Camera* camera = GScene ? GScene->GetCamera() : nullptr;
    if (!camera)
        return;
    FlameGL& f = s_flames;
    const std::uint32_t frame = GetFrameCounter();
    if (f.frame == frame)
        return;
    f.frame = frame;
    const bool flipbook = EnsureFlipbook() && EnsureFlameProgram(true);
    if (!flipbook && !EnsureFlameProgram(false))
        return;
    EnsureFlameVertexArray();
    const FlameProgram& fp = flipbook ? f.book : f.procedural;
    FlushAndFreeAllQueues(_queueNo);

    // The sheets, camera-relative (the world's draws are), as they are, each
    // with its flipbook frame (the fire's clock: a pause holds every phase).
    const int count = std::min(view.count, kMaxFlameSheets);
    f.staging.resize(static_cast<std::size_t>(count) * kFlameInstanceFloats);
    const float cx = _frameState.cameraPos[0], cy = _frameState.cameraPos[1], cz = _frameState.cameraPos[2];
    float* o = f.staging.data();
    for (int i = 0; i < count; ++i, o += kFlameInstanceFloats)
    {
        const Poseidon::firespread::FlameSheet& s = view.sheets[i];
        o[0] = s.x - cx;
        o[1] = s.y - cy;
        o[2] = s.z - cz;
        o[3] = s.width;
        o[4] = s.height;
        o[5] = s.leanX;
        o[6] = s.leanZ;
        o[7] = s.heat;
        o[8] = s.seed;
        const SheetPhase ph = flipbook ? SheetFrame(s, static_cast<double>(view.time)) : SheetPhase{};
        o[9] = ph.frame;
        o[10] = static_cast<float>(std::min(static_cast<int>(s.style), Poseidon::firespread::kFlameKinds - 1));
        o[11] = ph.scroll;
        o[12] = s.emissivity;
        o[13] = s.bend;
        o[14] = s.baseFade;
        o[15] = s.tempOffsetK;
        // (fix 6) a volume tongue: its visible absorption (0: a ribbon), its radius' scale, its tips' soot
        o[16] = s.volumeK;
        o[17] = s.volumeR;
        o[18] = s.volumeSoot;
        // (its puff's bulge: the wave's length, its size and its phase at the foot, in one number)
        o[19] =
            s.volumeK > 0.0f ? Poseidon::firespread::VolumePuffCode(s.volumePuff, s.volumeWaveM, s.volumeBulge) : 0.0f;
        // (fix 08: aFar)
        o[kFlameFarFloat] = s.gain;
        o[kFlameFarFloat + 1] = 0.0f;
        o[kFlameFarFloat + 2] = 0.0f;
        o[kFlameFarFloat + 3] = 0.0f;
    }

    GpuMark(Poseidon::gpu::Mark::FlamesBegin);
    // The post pass's targets at the frame's size, and the world drawing into
    // the frame target: then the world's depth goes into the post pass's
    // depth target (the soft fade, and the linear target's depth test; the
    // post resolve writes it again at the end of the world), and, with the
    // post effects on, the flames into the linear target the post pass merges.
    int tw = 0, th = 0;
    RenderTargetSize(tw, th);
    GLint drawFbo = 0, readFbo = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFbo);
    const bool soft = SSAAActive() && static_cast<GLuint>(drawFbo) == _ssaaFbo && _post.depthFbo && _post.depthTex &&
                      _post.w == tw && _post.h == th;
    const bool hdr = soft && _lighting.postEffects && EnsureFlameTargets(tw, th);
    const bool scissor = glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE;
    if (soft)
    {
        if (scissor)
            glDisable(GL_SCISSOR_TEST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, _ssaaFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _post.depthFbo);
        glBlitFramebuffer(0, 0, tw, th, 0, 0, tw, th, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(drawFbo));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(readFbo));
        if (scissor)
            glEnable(GL_SCISSOR_TEST);
    }
    // Into the linear target at half resolution: the world's viewport halved
    // (the scissor, in the frame's pixels, off), every channel of both targets
    // written (the surface target's mask is the frame's: UpdateSurfaceMask
    // puts it back at the end).
    GLboolean mask0[4] = {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
    if (hdr)
    {
        glGetIntegerv(GL_VIEWPORT, f.savedViewport);
        glGetBooleani_v(GL_COLOR_WRITEMASK, 0, mask0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.hdrFbo);
        if (scissor)
            glDisable(GL_SCISSOR_TEST);
        glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glColorMaski(1, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        // Nothing added, everything behind kept, no flame depth.
        const float clear[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        const float none[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        glClearBufferfv(GL_COLOR, 0, clear);
        glClearBufferfv(GL_COLOR, 1, none);
        f.coverViewport[0] = f.savedViewport[0] / FlameScale();
        f.coverViewport[1] = f.savedViewport[1] / FlameScale();
        f.coverViewport[2] = FlameHalf(f.savedViewport[2]);
        f.coverViewport[3] = FlameHalf(f.savedViewport[3]);
        glViewport(f.coverViewport[0], f.coverViewport[1], f.coverViewport[2], f.coverViewport[3]);
    }

    // What the draw changes that the next draws do not set again: the
    // program, the vertex array and array buffer, the draw target, cull,
    // stencil and alpha-to-coverage are put back as found, the depth unit
    // emptied; depth, blend and the surface target's write mask through the
    // engine's own modes at the end.
    GLint prevProg = 0, prevVao = 0, prevBuf = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProg);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevBuf);
    const bool cull = glIsEnabled(GL_CULL_FACE) == GL_TRUE;
    const bool stencil = glIsEnabled(GL_STENCIL_TEST) == GL_TRUE;
    const bool a2c = glIsEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE) == GL_TRUE;

    glBindBuffer(GL_ARRAY_BUFFER, f.vbo);
    const GLsizeiptr bytes = static_cast<GLsizeiptr>(f.staging.size() * sizeof(float));
    glBufferData(GL_ARRAY_BUFFER, bytes, nullptr, GL_STREAM_DRAW); // orphan last frame's
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, f.staging.data());

    // The flipbook stays on its own unit (made there; nothing else binds it),
    // and the depth unit always holds a complete texture (the white stand-in
    // without the copy).
    Poseidon::GL33Bind::ActiveUnit(kFlameDepthUnit);
    glBindTexture(GL_TEXTURE_2D, soft ? _post.depthTex : _post.whiteTex);
    Poseidon::GL33Bind::ActiveUnit(0);

    glUseProgram(fp.prog);
    GfxMatrix proj;
    ConvertProjectionMatrix(proj, camera->ProjectionNormal(), 0.0f);
    glUniformMatrix4fv(fp.locView, 1, GL_FALSE, &_frameState.view._11);
    glUniformMatrix4fv(fp.locProj, 1, GL_FALSE, &proj._11);
    glUniform4f(fp.locCam, _taa.ndcApplied.x, _taa.ndcApplied.y, _frameState.fogParams[0], _frameState.fogParams[1]);
    glUniform4f(fp.locFog, _frameState.fogColor[0], _frameState.fogColor[1], _frameState.fogColor[2], view.time);
    // The depth's mapping to view z (y 0: no soft fade), as the post passes read it.
    const float depthScale = GLAD_GL_ARB_clip_control ? 1.0f : 2.0f;
    const float depthBias = GLAD_GL_ARB_clip_control ? 0.0f : -1.0f;
    glUniform4f(fp.locDepthMap, proj._33, soft ? proj._43 : 0.0f, depthScale, depthBias);
    glUniform4f(fp.locOut, hdr ? 0.0f : 1.0f, FlipbookDebug() ? 1.0f : 0.0f,
                hdr ? static_cast<float>(FlameScale()) : 1.0f, 0.0f);
    // The flames' own exposure over the camera's (more at dusk and at night).
    f.exposeLog2 = FireFlameExposureLog2();
    glUniform4f(fp.locExpose, f.exposeLog2, 0.0f, 0.0f, 0.0f);
    // (fix 08: the far flames' scale: half the target's height in its own
    // pixels, and the near flipbook frame's size in texels)
    {
        GLint vp[4] = {};
        glGetIntegerv(GL_VIEWPORT, vp);
        glUniform4f(fp.locScreen, 0.5f * static_cast<float>(vp[3]), static_cast<float>(std::max(s_flipPrep.fh, 1)),
                    static_cast<float>(std::max(s_flipPrep.fw, 1)), 0.0f);
        if (flipbook)
            CountFarFlames(view, count, 0.5f * static_cast<float>(vp[3]) * proj._22, cx, cy, cz);
    }

    Poseidon::render::depthstencil::ReadOnlyNoStencil();
    if (hdr)
        Poseidon::render::blend::TransmittanceAccumulate();
    else
        Poseidon::render::blend::PremultipliedKeepAlpha();
    if (cull)
        glDisable(GL_CULL_FACE);
    if (a2c)
        glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    // Straight into the frame, the surface target takes the flame's cover
    // like smoke's (fragSurf); into the linear target, the depth target takes
    // the flames' depth (fragSurf there).
    if (_surfTargetOn && !hdr)
    {
        glColorMaski(1, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        _surfMaskOn = true;
    }
    Poseidon::GL33Bind::Vao(f.vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, kFlameStripVertices, count);
    // The sparks (EngineGL33_FireSparks.cpp), into the same target.
    DrawFireSparks(hdr, soft, proj, hdr ? FlameScale() : 1);

    if (hdr)
    {
        // The flames' depth for the smoke drawn after them (CoverFireFlames):
        // the cover's depth buffer from the depth target, every texel, no
        // colour.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.coverFbo);
        glViewport(0, 0, FlameHalf(tw), FlameHalf(th));
        glColorMaski(0, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        Poseidon::render::blend::Opaque();
        Poseidon::render::depthstencil::AlwaysWrite();
        glUseProgram(f.coverProg);
        Poseidon::GL33Bind::ActiveUnit(kFlameDepthUnit);
        glBindTexture(GL_TEXTURE_2D, f.zTex);
        Poseidon::GL33Bind::ActiveUnit(0);
        Poseidon::gl33post::SetInt(f.coverProg, "flameZTex", kFlameDepthUnit);
        Poseidon::gl33post::Set4(f.coverProg, "flameProj", proj._33, proj._43, depthScale, depthBias);
        Poseidon::gl33post::Set4(f.coverProg, "flameCoverDither", kCoverDitherShare, kCoverDitherMinM,
                                 static_cast<float>(frame % 64U) / 64.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // Smoke over the flames (fix 02): nothing in front of them yet (no
        // colour, all of their light left).
        glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        const float open[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, open);
        glColorMaski(0, mask0[0], mask0[1], mask0[2], mask0[3]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(drawFbo));
        glViewport(f.savedViewport[0], f.savedViewport[1], f.savedViewport[2], f.savedViewport[3]);
        if (scissor)
            glEnable(GL_SCISSOR_TEST);
        f.hdrFrame = frame;
        _flameCoverFrame = frame;
    }
    Poseidon::GL33Bind::Vao(static_cast<GLuint>(prevVao));
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(prevBuf));
    glUseProgram(static_cast<GLuint>(prevProg));
    Poseidon::GL33Bind::ActiveUnit(kFlameDepthUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
    Poseidon::GL33Bind::ActiveUnit(0);
    if (cull)
        glEnable(GL_CULL_FACE);
    if (stencil)
        glEnable(GL_STENCIL_TEST);
    if (a2c)
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    ApplyDepthMode(_currentDepthMode);
    ApplyBlendMode(_currentBlendMode);
    UpdateSurfaceMask(true);
    InvalidatePipelineCache();
    GpuMark(Poseidon::gpu::Mark::FlamesEnd);
}

bool EngineGL33::FireFlamesPending() const
{
    const FlameGL& f = s_flames;
    return f.hdrFrame == GetFrameCounter() && f.mergedFrame != GetFrameCounter() && f.mergeFbo;
}

float EngineGL33::FireFlameExposureLog2() const
{
    // Under the photoreal camera only (this frame's world writes scene light):
    // the camera's dusk and night from the sun (UpdateCameraFrame), against
    // the exposure the frame was drawn at.
    const CameraState& c = _camera;
    if (!c.frameOn || c.frame != GetFrameCounter())
        return 0.0f;
    const float dark = rm::FlameDark(c.duskEffect, c.nightEffect);
    const float e =
        rm::FlameExposureLog2(c.preExposureLog2, dark, _lighting.camEv, _lighting.camFlameNight) - c.preExposureLog2;
    return std::isfinite(e) ? std::min(std::max(e, 0.0f), 12.0f) : 0.0f;
}

// The camera's clip on the flames' light in the merge (PhotoCamera.hpp
// FlameFilm; flameFilmK): on under the photoreal camera with a knee, the
// exposure the composite applies, the knee and the spill.
static void SetFlameFilm(GLuint prog, bool camera, float exposure, float knee)
{
    const bool on = camera && knee > 0.0f && exposure > 0.0f;
    Poseidon::gl33post::Set4(prog, "flameFilmK", on ? 1.0f : 0.0f, exposure, on ? knee : 1.0f, rm::kFlameSpill);
}

void EngineGL33::MergeFireFlames(PostFrame& f)
{
    // Only in a frame whose flames went into the linear target, once, at the
    // target's size; inside the post pass's fullscreen state.
    FlameGL& fl = s_flames;
    if (!FireFlamesPending() || fl.targetW != f.w || fl.targetH != f.h)
        return;
    GpuMark(Poseidon::gpu::Mark::FlameMergeBegin);
    using namespace Poseidon::gl33post;
    const std::uint32_t frame = GetFrameCounter();
    // The volumetric fog applied this frame (the scene the merge reads is its
    // output): the flames take its medium up to their own depth.
    const bool fog = f.plan.Runs(rm::PostStage::Air) && f.plan.fog && _fog.appliedFrame == frame && _fog.integTex &&
                     _fog.grid.slices > 0 && (FlameDebugBits() & 1) == 0;
    // The TAA reads its opaque copy of this frame: the flames mark it.
    const GLuint taaFbo = f.plan.Runs(rm::PostStage::Temporal) && _taa.opaqueCopied && _taa.frame == frame
                              ? FlameMergeTaaFbo(_post.opaqueTex)
                              : 0;
    glUseProgram(fl.mergeProg);
    SetInt(fl.mergeProg, "sceneTex", 0);
    SetInt(fl.mergeProg, "flameTex", 2);
    SetInt(fl.mergeProg, "flameZTex", 3);
    SetInt(fl.mergeProg, "depthTex", 4);
    SetInt(fl.mergeProg, "worldTex", 5);
    SetInt(fl.mergeProg, "fogVolume", 6);
    Set4(fl.mergeProg, "flameMerge", f.camera ? 1.0f : 0.0f, 0.0f, fog ? 1.0f : 0.0f, taaFbo ? 1.0f : 0.0f);
    // The flames' light as the camera's clip shows it, at the exposure the
    // composite applies to the merged frame.
    SetFlameFilm(fl.mergeProg, f.camera, f.exposure, _lighting.camFlameKnee);
    Set4(fl.mergeProg, "flameProj", f.projInfo[0], f.projInfo[1], f.depthScale, f.depthBias);
    Set4(fl.mergeProg, "flameFogDepth", _fog.prevFar, _fog.prevK, static_cast<float>(_fog.grid.slices), 0.0f);
    Set4(fl.mergeProg, "flameRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    Set4(fl.mergeProg, "flameMergeDebug", (FlameDebugBits() & 4) ? 1.0f : 0.0f, 1.0f / static_cast<float>(FlameScale()),
         (FlameDebugBits() & 16) ? 1.0f : 0.0f, (FlameDebugBits() & 32) ? 1.0f : 0.0f);
    BindTex(0, PostSceneTex(f));
    BindTex(2, fl.hdrTex);
    BindTex(3, fl.zTex);
    BindTex(4, _post.depthTex);
    BindTex(5, _post.colorTex);
    // Smoke over the flames (fix 02): the smoke in front of them, on the
    // flames' own depth unit (empty after their draw), emptied again below.
    SetInt(fl.mergeProg, "frontTex", kFlameDepthUnit);
    BindTex(kFlameDepthUnit, fl.frontTex);
    // Unit 6 is the world's raw shadow unit (kShadowRawUnit): its sampler
    // object is set aside while the fog volume is read there.
    GLint sampler6 = 0;
    if (fog)
    {
        glActiveTexture(GL_TEXTURE0 + 6);
        glGetIntegerv(GL_SAMPLER_BINDING, &sampler6);
        glBindSampler(6, 0);
        glBindTexture(GL_TEXTURE_3D, _fog.integTex);
    }
    if (taaFbo)
    {
        // The merged frame as it is, the opaque copy only where the mark's
        // alpha says (its own alpha kept).
        Poseidon::render::blend::AlphaBlendKeepAlpha();
        glDisablei(GL_BLEND, 0);
        DrawTo(taaFbo, f.w, f.h);
        Poseidon::render::blend::Opaque();
    }
    else
    {
        DrawTo(fl.mergeFbo, f.w, f.h);
    }
    if (fog)
    {
        glActiveTexture(GL_TEXTURE0 + 6);
        glBindTexture(GL_TEXTURE_3D, 0);
        glBindSampler(6, static_cast<GLuint>(sampler6));
    }
    BindTex(kFlameDepthUnit, 0); // smoke over the flames (fix 02)
    glActiveTexture(GL_TEXTURE0);
    Poseidon::GL33Bind::Invalidate();
    f.flameScene = fl.mergeTex;
    fl.mergedFrame = frame;
    GpuMark(Poseidon::gpu::Mark::FlameMergeEnd);
}

void EngineGL33::BlendFireFlamesIntoFrame(bool camera)
{
    // No post stage to merge them before: the linear target, upsampled, laid
    // over the frame target, coded (inside the post pass's fullscreen state;
    // the frame's surface target is left alone).
    FlameGL& fl = s_flames;
    if (!FireFlamesPending())
        return;
    GpuMark(Poseidon::gpu::Mark::FlameMergeBegin);
    using namespace Poseidon::gl33post;
    glUseProgram(fl.mergeProg);
    SetInt(fl.mergeProg, "sceneTex", 0);
    SetInt(fl.mergeProg, "flameTex", 2);
    SetInt(fl.mergeProg, "flameZTex", 3);
    SetInt(fl.mergeProg, "depthTex", 4);
    SetInt(fl.mergeProg, "worldTex", 5);
    SetInt(fl.mergeProg, "fogVolume", 6);
    Set4(fl.mergeProg, "flameMerge", camera ? 1.0f : 0.0f, 1.0f, 0.0f, 0.0f);
    // (no composite after it: the camera's clip at the frame's own exposure)
    SetFlameFilm(fl.mergeProg, camera, 1.0f, _lighting.camFlameKnee);
    Set4(fl.mergeProg, "flameMergeDebug", 0.0f, 1.0f / static_cast<float>(FlameScale()), 0.0f, 0.0f);
    const float depthScale = GLAD_GL_ARB_clip_control ? 1.0f : 2.0f;
    const float depthBias = GLAD_GL_ARB_clip_control ? 0.0f : -1.0f;
    Camera* cam = GScene ? GScene->GetCamera() : nullptr;
    GfxMatrix proj;
    if (cam)
        ConvertProjectionMatrix(proj, cam->ProjectionNormal(), 0.0f);
    Set4(fl.mergeProg, "flameProj", cam ? proj._33 : 0.0f, cam ? proj._43 : 0.0f, depthScale, depthBias);
    BindTex(0, _post.blackTex);
    BindTex(2, fl.hdrTex);
    BindTex(3, fl.zTex);
    BindTex(4, cam ? _post.depthTex : _post.whiteTex);
    BindTex(5, _post.blackTex);
    SetInt(fl.mergeProg, "frontTex", kFlameDepthUnit); // smoke over the flames (fix 02)
    BindTex(kFlameDepthUnit, fl.frontTex);
    Poseidon::render::blend::TransmittanceKeepAlpha();
    BindFrameRenderTarget();
    GLboolean mask1[4] = {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
    glGetBooleani_v(GL_COLOR_WRITEMASK, 1, mask1);
    glColorMaski(1, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glColorMaski(1, mask1[0], mask1[1], mask1[2], mask1[3]);
    BindTex(kFlameDepthUnit, 0); // smoke over the flames (fix 02)
    glActiveTexture(GL_TEXTURE0);
    Poseidon::render::blend::Opaque();
    fl.mergedFrame = GetFrameCounter();
    GpuMark(Poseidon::gpu::Mark::FlameMergeEnd);
}

bool EngineGL33::BeginFireFlameCover()
{
    // A blended draw after the flames, in this frame's world, into the frame
    // target: drawn again at the flames' half resolution, tested against
    // their depth (passing only in front of them, and nowhere without
    // flames), into the smoke's own target (smoke over the flames, fix 02):
    // laid over there as in the frame, its alpha multiplying what of the
    // flames' light the smoke leaves. The merge dims the flames by that and
    // keeps the smoke's colour over them. The draw's own program, vertices
    // and depth.
    FlameGL& f = s_flames;
    if (_worldSceneEnded || !f.coverFbo || f.hdrFrame != GetFrameCounter() || (FlameDebugBits() & 2) != 0)
        return false;
    GLint draw = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw);
    if (static_cast<GLuint>(draw) != _ssaaFbo)
        return false;
    glGetIntegerv(GL_VIEWPORT, f.savedViewport);
    glGetBooleani_v(GL_COLOR_WRITEMASK, 0, f.savedMask);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &f.savedDepthMask);
    f.savedScissor = glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, f.coverFbo);
    glViewport(f.coverViewport[0], f.coverViewport[1], f.coverViewport[2], f.coverViewport[3]);
    if (f.savedScissor)
        glDisable(GL_SCISSOR_TEST);
    glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    Poseidon::render::depthstencil::DepthWrite(false);
    Poseidon::render::blend::SmokeOverKeepTransmittance(); // smoke over the flames (fix 02)
    return true;
}

void EngineGL33::EndFireFlameCover()
{
    FlameGL& f = s_flames;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _ssaaFbo);
    glViewport(f.savedViewport[0], f.savedViewport[1], f.savedViewport[2], f.savedViewport[3]);
    if (f.savedScissor)
        glEnable(GL_SCISSOR_TEST);
    glColorMaski(0, f.savedMask[0], f.savedMask[1], f.savedMask[2], f.savedMask[3]);
    Poseidon::render::depthstencil::DepthWrite(f.savedDepthMask == GL_TRUE);
    if (_lightingActive)
        Poseidon::render::blend::AlphaBlendKeepAlpha();
    else
        Poseidon::render::blend::AlphaBlend();
}
