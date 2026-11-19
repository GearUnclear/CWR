// The fire spread's sparks (POSEIDON_XP_FIRESPREAD;
// World/Simulation/FireSparks.hpp, FireSpreadWorld.hpp GetSparks).
//
// All of them go in one instanced draw from a streamed instance buffer (each
// spark camera-relative: its place and size, its velocity and temperature,
// its light's weight and spread), a quad a spark round its streak
// (EngineGL33_FireSparkShaders.cpp), exposed as the flames are
// (FireFlameExposureLog2: the flames' own exposure over the camera's, fix 1).
// It runs inside the flames' draw
// (EngineGL33::DrawFireFlames, right after the flame tongues), into the same
// target with the same blend and depth state: into the linear flame target
// (half the frame's size) when the post pass merges it, so the sparks are
// exposed, clipped and glared with the flames, or straight into the frame
// without it. The flames' depth copy is already bound on kFlameDepthUnit.
// It saves and puts back what it binds (the program, the vertex array and
// the array buffer); nothing is made or drawn while GetSparks has none.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Core/MatrixConversion.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Simulation/FireSpreadWorld.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

using namespace Poseidon::gl33lighting;

namespace
{

// Floats per spark in the instance buffer: aPos, aVel, aLight.
constexpr int kSparkInstanceAttribs = 3;
constexpr int kSparkInstanceFloats = 4 * kSparkInstanceAttribs;
// At most this many (FireSparks.hpp SparkPool::kMaxSparks is less).
constexpr int kMaxSparkPoints = 16384;
// The shutter a spark's streak is drawn over (s: half a frame at 30 fps, a
// film camera's 180 degrees), and the blur's least sigma (target pixels).
constexpr float kSparkShutterSec = 1.0f / 60.0f;
constexpr float kSparkMinSigmaPx = 0.6f;

struct SparkGL
{
    GLuint prog = 0;
    bool tried = false; // built (or failed: then never again until DeinitFireSparks)
    GLint locView = -1, locProj = -1, locCam = -1, locTarget = -1, locGlare = -1, locDepthMap = -1, locOut = -1;
    GLuint vao = 0, vbo = 0;
    std::vector<float> staging;
};
SparkGL s_sparks;

GLuint CompileSparkShader(GLenum type, const char* src, const char* name)
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
        LOG_ERROR(Graphics, "GL33 fire sparks: shader compile error [{}]: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

bool EnsureSparkProgram()
{
    SparkGL& g = s_sparks;
    if (g.tried)
        return g.prog != 0;
    g.tried = true;
    const GLuint vs = CompileSparkShader(GL_VERTEX_SHADER, FireSparkVertexSource(), "vsFireSparks");
    const GLuint fs = CompileSparkShader(GL_FRAGMENT_SHADER, FireSparkFragmentSource(), "psFireSparks");
    if (!vs || !fs)
    {
        if (vs)
            glDeleteShader(vs);
        if (fs)
            glDeleteShader(fs);
        return false;
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
        LOG_ERROR(Graphics, "GL33 fire sparks: program link error: {}", log);
        glDeleteProgram(p);
        return false;
    }
    const GLuint lit = glGetUniformBlockIndex(p, "LitConstants");
    if (lit != GL_INVALID_INDEX)
        glUniformBlockBinding(p, lit, kLitBinding);
    g.locView = glGetUniformLocation(p, "sparkView");
    g.locProj = glGetUniformLocation(p, "sparkProj");
    g.locCam = glGetUniformLocation(p, "sparkCam");
    g.locTarget = glGetUniformLocation(p, "sparkTarget");
    g.locGlare = glGetUniformLocation(p, "sparkGlare");
    g.locDepthMap = glGetUniformLocation(p, "sparkDepthMap");
    g.locOut = glGetUniformLocation(p, "sparkOut");
    GLint prevProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProg);
    glUseProgram(p);
    const GLint depth = glGetUniformLocation(p, "sparkDepth");
    if (depth >= 0)
        glUniform1i(depth, kFlameDepthUnit);
    glUseProgram(static_cast<GLuint>(prevProg));
    g.prog = p;
    LOG_INFO(Graphics, "GL33 fire sparks: program built");
    return true;
}

// The instance buffer behind three per-instance attributes; no per-vertex
// attribute (the quad's corners come from gl_VertexID).
void EnsureSparkVertexArray()
{
    SparkGL& g = s_sparks;
    if (g.vao)
        return;
    GLint prevVao = 0, prevBuf = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevBuf);
    glGenVertexArrays(1, &g.vao);
    glGenBuffers(1, &g.vbo);
    Poseidon::GL33Bind::Vao(g.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g.vbo);
    const GLsizei stride = kSparkInstanceFloats * static_cast<GLsizei>(sizeof(float));
    for (int i = 0; i < kSparkInstanceAttribs; ++i)
    {
        glEnableVertexAttribArray(static_cast<GLuint>(i));
        glVertexAttribPointer(static_cast<GLuint>(i), 4, GL_FLOAT, GL_FALSE, stride,
                              reinterpret_cast<const void*>(static_cast<std::size_t>(i) * 4 * sizeof(float)));
        glVertexAttribDivisor(static_cast<GLuint>(i), 1);
    }
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(prevBuf));
    Poseidon::GL33Bind::Vao(static_cast<GLuint>(prevVao));
}
} // namespace

void EngineGL33::DeinitFireSparks()
{
    SparkGL& g = s_sparks;
    if (g.prog)
        glDeleteProgram(g.prog);
    if (g.vao)
    {
        Poseidon::GL33Bind::OnVaoDeleted(g.vao);
        glDeleteVertexArrays(1, &g.vao);
    }
    if (g.vbo)
        glDeleteBuffers(1, &g.vbo);
    g = SparkGL{};
}

void EngineGL33::DrawFireSparks(bool hdr, bool soft, const GfxMatrix& proj, int targetScale)
{
    Poseidon::firespread::SparkView view;
    if (!Poseidon::firespread::GetSparks(view) || !view.points || view.count <= 0)
        return;
    if (!EnsureSparkProgram())
        return;
    EnsureSparkVertexArray();
    SparkGL& g = s_sparks;

    // The sparks, camera-relative (the world's draws are).
    const int count = std::min(view.count, kMaxSparkPoints);
    g.staging.resize(static_cast<std::size_t>(count) * kSparkInstanceFloats);
    const float cx = _frameState.cameraPos[0], cy = _frameState.cameraPos[1], cz = _frameState.cameraPos[2];
    float* o = g.staging.data();
    for (int i = 0; i < count; ++i, o += kSparkInstanceFloats)
    {
        const Poseidon::firespread::SparkPoint& s = view.points[i];
        o[0] = s.x - cx;
        o[1] = s.y - cy;
        o[2] = s.z - cz;
        o[3] = s.size;
        o[4] = s.vx;
        o[5] = s.vy;
        o[6] = s.vz;
        o[7] = s.kelvin;
        o[8] = s.light;
        o[9] = s.area;
        o[10] = 0.0f;
        o[11] = 0.0f;
    }

    GLint prevProg = 0, prevVao = 0, prevBuf = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProg);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevBuf);
    GLint vp[4] = {0, 0, 1, 1};
    glGetIntegerv(GL_VIEWPORT, vp);

    glBindBuffer(GL_ARRAY_BUFFER, g.vbo);
    const GLsizeiptr bytes = static_cast<GLsizeiptr>(g.staging.size() * sizeof(float));
    glBufferData(GL_ARRAY_BUFFER, bytes, nullptr, GL_STREAM_DRAW); // orphan last frame's
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, g.staging.data());

    glUseProgram(g.prog);
    glUniformMatrix4fv(g.locView, 1, GL_FALSE, &_frameState.view._11);
    glUniformMatrix4fv(g.locProj, 1, GL_FALSE, &proj._11);
    glUniform4f(g.locCam, _taa.ndcApplied.x, _taa.ndcApplied.y, _frameState.fogParams[0], _frameState.fogParams[1]);
    glUniform4f(g.locTarget, static_cast<float>(std::max(vp[2], 1)), static_cast<float>(std::max(vp[3], 1)),
                kSparkShutterSec, kSparkMinSigmaPx);
    glUniform4f(g.locGlare, view.glareM, view.glareFromM, std::max(view.glareToM, view.glareFromM + 1.0f), 0.0f);
    const float depthScale = GLAD_GL_ARB_clip_control ? 1.0f : 2.0f;
    const float depthBias = GLAD_GL_ARB_clip_control ? 0.0f : -1.0f;
    glUniform4f(g.locDepthMap, proj._33, soft ? proj._43 : 0.0f, depthScale, depthBias);
    glUniform4f(g.locOut, hdr ? 0.0f : 1.0f, static_cast<float>(std::max(targetScale, 1)), FireFlameExposureLog2(),
                0.0f);
    Poseidon::GL33Bind::Vao(g.vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);

    Poseidon::GL33Bind::Vao(static_cast<GLuint>(prevVao));
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(prevBuf));
    glUseProgram(static_cast<GLuint>(prevProg));
}
