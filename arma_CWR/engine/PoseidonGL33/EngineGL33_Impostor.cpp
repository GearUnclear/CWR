// XP-impostor (experimental; POSEIDON_XP_IMPOSTOR=1, default off): far trees
// and bushes as baked billboards. The backend-free half and the overview are
// in Poseidon/Graphics/Rendering/Impostor.hpp.
//
// Bake (once per shape, again kRebakeFrames later so textures the first bake
// found at a small mip are taken at their streamed level): the shape's level
// is drawn kFrames times with an orthographic view from the horizon, at
// azimuth 2 pi k / kFrames around +y, into a kBakeSize target with two
// colour attachments (albedo + alpha cut at kAlphaRef; the model-space normal
// + the depth toward the view), read back, halved to kTile, given a
// coverage-keeping mip chain on the CPU and stored in layer slot * kFrames + k
// of two texture arrays.
//
// Draw: one instanced quad per far object, facing the camera about the
// model's up axis, through a program made of the remastered object lighting
// prelude (RemasterFragmentCommon) with three varyings turned into globals
// (vWorldRel, vNormalW, vCrown) and the alpha reference made local; its main
// rebuilds the model point and normal under the pixel from the two nearest
// frames and calls shadeRemastered and finishFragment as psNormalRemaster
// does. The render state (depth, stencil tag, material, fog, the lit
// constants) is the one the scene prepared with the baked level's leaf
// section; this draw only forces an opaque cutout on top of it (no blending,
// depth writes, alpha to coverage on a multisampled target, no culling) and
// puts back what it changed.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <PoseidonGL33/TextureGL33.hpp>
#include <Poseidon/Graphics/Rendering/Impostor.hpp>
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>
#include <Poseidon/Graphics/Rendering/RenderFlags.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <Poseidon/Graphics/Textures/TextureBank.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLClear.hpp>
#include <Poseidon/Graphics/Core/GLCullState.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Core/GLPipelineState.hpp>

#include <glad/gl.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace imp = Poseidon::render::impostor;

namespace
{
constexpr int kAlbedoUnit = 17;
constexpr int kNormalUnit = 18;
constexpr int kBakeSourceUnit = 19;
constexpr int kLevels = 8; // kTile 128 .. 1
constexpr unsigned int kRebakeFrames = 90;
static_assert((imp::kTile >> (kLevels - 1)) == 1, "the chain ends at 1x1");

struct ImpostorSlotData
{
    Ref<Poseidon::LODShape> shape;
    int level = -1;
    int stateSection = -1;
    float ctrX = 0.0f, ctrZ = 0.0f, halfW = 1.0f, minY = 0.0f, maxY = 1.0f;
    float crownC[3] = {0, 0, 0};
    float crownS[3] = {1, 1, 1};
    unsigned int rebakeAt = 0; // frame of the second bake (0: done)
};

struct ImpostorGL
{
    bool init = false;
    bool failed = false;
    GLuint prog = 0, bakeProg = 0;
    GLuint vao = 0, quadVbo = 0, instVbo = 0;
    GLuint bakeVao = 0, bakeVbo = 0, bakeFbo = 0, bakeColor[2] = {0, 0}, bakeDepth = 0;
    GLuint albedoArr = 0, normalArr = 0, sampAtlas = 0, sampSource = 0;
    GLint locSlots = -1, locA2C = -1;
    GLint locBakeR = -1, locBakeG = -1, locBakeY = -1;
    size_t instCapacity = 0;
    std::vector<ImpostorSlotData> slots;
    std::unordered_map<const Poseidon::LODShape*, int> index; // -1: the shape cannot be baked
    std::vector<float> slotUniforms;                          // kMaxSlots * 4 vec4
    bool slotsDirty = true;
    int bakes = 0;
};

ImpostorGL& G()
{
    static ImpostorGL s;
    return s;
}

// The remastered object vertex shader's constants (s_vsTransformRemasterGLSL),
// member for member.
const char kImpostorVS[] = R"(#version 330 core
layout(std140) uniform VSConstants {
    mat4 proj;
    mat4 view;
    mat4 world;
    vec4 sunDir;
    vec4 ambient;
    vec4 diffuse;
    vec4 emissive;
    vec4 fogParam;
    vec4 camPos;
    vec4 specular;
    vec4 specEn;
    vec4 sunEn;
    vec4 vpScale;
    vec4 crownCenter;
    vec4 crownScale;
    mat4 texMat0;
    mat4 texMat1;
    vec4 texCtrl;
};
// Per slot: [0] centre x, z, half width, first layer; [1] min y, max y,
// frames; [2] crown centre, valid; [3] crown 1 / half extents.
uniform vec4 impSlots[128];

layout(location = 0) in vec2 aCorner; // x -1..1 across, y 0..1 up
layout(location = 1) in vec4 iPos;    // camera-relative model origin, slot
layout(location = 2) in vec4 iX;      // model axes in the world, with scale
layout(location = 3) in vec4 iY;
layout(location = 4) in vec4 iZ;

out vec4 vColor;
out vec4 vSpecColor;
out vec2 vUV0;
out vec2 vUV1;
out float vFogTC;
out vec3 vWorldRelIn;
out vec3 vNormalWIn;
out vec4 vCrownIn;
flat out float vLitMode;
flat out vec4 vImpA; // layer of frame 0, of frame 1, weight of frame 1, half width
flat out vec4 vImpB; // centre x, z; view direction in the model (x, z)
flat out vec3 vImpPos;
flat out vec3 vImpX;
flat out vec3 vImpY;
flat out vec3 vImpZ;
flat out vec4 vImpCC;
flat out vec3 vImpCS;
out vec2 vImpQ; // across (-1..1), model y

void main() {
    int si = int(iPos.w + 0.5) * 4;
    vec4 s0 = impSlots[si];
    vec4 s1 = impSlots[si + 1];
    vec3 X = iX.xyz;
    vec3 Y = iY.xyz;
    vec3 Z = iZ.xyz;
    vec3 toCam = -iPos.xyz;
    vec2 ch = vec2(dot(toCam, X) / max(dot(X, X), 1e-12), dot(toCam, Z) / max(dot(Z, Z), 1e-12));
    float chl = length(ch);
    ch = chl > 1e-6 ? ch / chl : vec2(0.0, 1.0);
    float frames = s1.z;
    float f = mod(atan(ch.x, ch.y) * (frames / 6.2831853), frames);
    float f0 = floor(f);
    vec3 R = vec3(ch.y, 0.0, -ch.x);
    vec3 pm = vec3(s0.x, mix(s1.x, s1.y, aCorner.y), s0.y) + R * (aCorner.x * s0.z);
    vec3 wrel = iPos.xyz + X * pm.x + Y * pm.y + Z * pm.z;
    gl_Position = proj * (view * vec4(wrel, 1.0));
    if (vpScale.z != 0.0 || vpScale.w != 0.0) gl_Position.xy += vpScale.zw * gl_Position.w;

    // The classic vertex colour (its alpha fades objects), with the normal
    // toward the camera.
    vec3 nW = X * ch.x + Z * ch.y;
    nW *= inversesqrt(max(dot(nW, nW), 1e-12));
    float NdotL = max(0.0, dot(nW, -sunDir.xyz));
    vec4 litColor;
    litColor.rgb = emissive.rgb + ambient.rgb * sunEn.x + diffuse.rgb * NdotL * sunEn.x;
    litColor.a   = emissive.a   + ambient.a   * sunEn.x + diffuse.a   * NdotL * sunEn.x;
    vColor = clamp(litColor, 0.0, 1.0);
    vSpecColor = vec4(0.0);
    float dist = length(wrel - camPos.xyz);
    float fogFactor = clamp(1.0 - (dist - fogParam.x) * fogParam.y, 0.0, 1.0);
    vFogTC = (fogParam.z > 0.5) ? fogFactor : 1.0;
    vUV0 = vec2(aCorner.x * 0.5 + 0.5, aCorner.y);
    vUV1 = vUV0;
    vWorldRelIn = wrel;
    vNormalWIn = nW;
    vCrownIn = vec4(0.0);
    vLitMode = 1.0;
    vImpA = vec4(s0.w + mod(f0, frames), s0.w + mod(f0 + 1.0, frames), f - f0, s0.z);
    vImpB = vec4(s0.x, s0.y, ch.x, ch.y);
    vImpPos = iPos.xyz;
    vImpX = X;
    vImpY = Y;
    vImpZ = Z;
    vImpCC = impSlots[si + 2];
    vImpCS = impSlots[si + 3].xyz;
    vImpQ = vec2(aCorner.x, pm.y);
}
)";

const char kImpostorMain[] = R"(
uniform sampler2DArray impAlbedo;
uniform sampler2DArray impNormal;
uniform float impA2C;
flat in vec4 vImpA;
flat in vec4 vImpB;
flat in vec3 vImpPos;
flat in vec3 vImpX;
flat in vec3 vImpY;
flat in vec3 vImpZ;
flat in vec4 vImpCC;
flat in vec3 vImpCS;
in vec2 vImpQ;

void main() {
    // An opaque cutout whatever the prepared state's alpha mode.
    alphaRef = vec4(0.5, 1.0, impA2C, alphaRefUbo.w);
    vec4 A = mix(texture(impAlbedo, vec3(vUV0, vImpA.x)), texture(impAlbedo, vec3(vUV0, vImpA.y)), vImpA.z);
    vec4 N = mix(texture(impNormal, vec3(vUV0, vImpA.x)), texture(impNormal, vec3(vUV0, vImpA.y)), vImpA.z);
    vec3 nm = N.xyz * 2.0 - 1.0;
    float d = N.w * 2.0 - 1.0;
    vec3 R = vec3(vImpB.w, 0.0, -vImpB.z);
    vec3 C = vec3(vImpB.z, 0.0, vImpB.w);
    vec3 pm = vec3(vImpB.x, vImpQ.y, vImpB.y) + R * (vImpQ.x * vImpA.w) + C * (d * vImpA.w);
    vWorldRel = vImpPos + vImpX * pm.x + vImpY * pm.y + vImpZ * pm.z;
    vec3 nw = vImpX * nm.x + vImpY * nm.y + vImpZ * nm.z;
    // A leaf card without a mesh normal (zero after the bake) takes the
    // crown's, as in the mesh draw.
    vNormalW = dot(nm, nm) > 0.25 ? nw : vec3(0.0);
    if (vImpCC.w > 0.5) {
        vec3 e = (pm - vImpCC.xyz) * vImpCS;
        vCrown = vec4(mat3(vImpX, vImpY, vImpZ) * (e * vImpCS), length(e));
    } else {
        vCrown = vec4(0.0);
    }
    float ambientShare = 0.0;
    vec4 r0;
    r0.rgb = shadeRemastered(A.rgb * constColor.rgb, false, ambientShare);
    r0.a = vColor.a * A.a * constColor.a;
    finishFragment(r0, ambientShare, true);
}
)";

const char kBakeVS[] = R"(#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNrm;
layout(location = 2) in vec2 aUV;
uniform vec4 bR; // across (x, z), toward the view (x, z)
uniform vec4 bG; // centre x, z, 1 / half width
uniform vec4 bY; // min y, 1 / height
out vec2 uv;
out vec3 nrm;
out float dep;
void main() {
    vec2 p = aPos.xz - bG.xy;
    float x = dot(p, bR.xy) * bG.z;
    float d = dot(p, bR.zw) * bG.z;
    float y = (aPos.y - bY.x) * bY.y * 2.0 - 1.0;
    gl_Position = vec4(x, y, 0.5 - 0.5 * d, 1.0);
    uv = aUV;
    nrm = aNrm;
    dep = d;
}
)";

const char kBakeFS[] = R"(#version 330 core
uniform sampler2D src;
in vec2 uv;
in vec3 nrm;
in float dep;
layout(location = 0) out vec4 oA;
layout(location = 1) out vec4 oN;
void main() {
    vec4 t = texture(src, uv);
    if (t.a < 0.5) discard;
    float l = dot(nrm, nrm);
    vec3 n = l > 1e-8 ? nrm * inversesqrt(l) : vec3(0.0);
    oA = vec4(t.rgb, 1.0);
    oN = vec4(n * 0.5 + 0.5, clamp(dep * 0.5 + 0.5, 0.0, 1.0));
}
)";

GLuint ImpCompile(GLenum type, const char* src, const char* name)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[4096] = {};
        glGetShaderInfoLog(s, sizeof(log) - 1, nullptr, log);
        LOG_ERROR(Graphics, "XP impostor: shader {} failed: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint ImpLink(GLuint vs, GLuint fs, const char* name)
{
    if (!vs || !fs)
        return 0;
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    glDetachShader(p, vs);
    glDetachShader(p, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[4096] = {};
        glGetProgramInfoLog(p, sizeof(log) - 1, nullptr, log);
        LOG_ERROR(Graphics, "XP impostor: program {} failed: {}", name, log);
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

// Replace the one occurrence of `from` in `s`; false when it is not there
// exactly once (the prelude changed: the impostor stays off).
bool ReplaceOnce(std::string& s, const char* from, const char* to)
{
    const size_t at = s.find(from);
    if (at == std::string::npos || s.find(from, at + 1) != std::string::npos)
        return false;
    s.replace(at, std::strlen(from), to);
    return true;
}

// GL state the bake and the draw change that the engine's own appliers do not
// set back (EngineGL33::RestoreAfterImpostor puts back depth, stencil,
// blending, culling and alpha to coverage through them).
struct SavedCaps
{
    GLboolean scissor, srgb;
    GLint program;
    GLint arrayBuffer;
    void Save()
    {
        scissor = glIsEnabled(GL_SCISSOR_TEST);
        srgb = glIsEnabled(GL_FRAMEBUFFER_SRGB);
        glGetIntegerv(GL_CURRENT_PROGRAM, &program);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBuffer);
    }
    void Restore() const
    {
        if (scissor)
            glEnable(GL_SCISSOR_TEST);
        else
            glDisable(GL_SCISSOR_TEST);
        if (srgb)
            glEnable(GL_FRAMEBUFFER_SRGB);
        else
            glDisable(GL_FRAMEBUFFER_SRGB);
        glUseProgram(static_cast<GLuint>(program));
        glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(arrayBuffer));
    }
};

struct BakeVertex
{
    float p[3];
    float n[3];
    float uv[2];
};

int AlphaRank(Texture* tex)
{
    if (!tex)
        return -1;
    switch (tex->GetAlphaClass())
    {
        case Poseidon::AlphaStats::Cutout:
            return 2;
        case Poseidon::AlphaStats::Blend:
            return 1;
        default:
            return 0;
    }
}
} // namespace

bool EngineGL33::ImpostorsAvailable() const
{
    const ImpostorGL& g = G();
    return imp::Get().on && _glContext && _remasterPrograms && _lightingActive && !g.failed;
}

static bool InitImpostorGL(ImpostorGL& g)
{
    g.init = true;
    GLint units = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &units);
    if (units <= kBakeSourceUnit)
    {
        LOG_WARN(Graphics, "XP impostor: off ({} fragment texture units, need {})", units, kBakeSourceUnit + 1);
        return false;
    }
    // The draw program: the remastered prelude with the three varyings the
    // main rebuilds per pixel made globals, and the alpha reference made local.
    std::string common = Poseidon::gl33lighting::RemasterFragmentCommon();
    bool ok = ReplaceOnce(common, "in vec3 vWorldRel;", "in vec3 vWorldRelIn;\nvec3 vWorldRel;") &&
              ReplaceOnce(common, "in vec3 vNormalW;", "in vec3 vNormalWIn;\nvec3 vNormalW;") &&
              ReplaceOnce(common, "in vec4 vCrown;", "in vec4 vCrownIn;\nvec4 vCrown;") &&
              ReplaceOnce(common, "vec4 alphaRef;", "vec4 alphaRefUbo;") &&
              ReplaceOnce(common, "uniform sampler2D tex0;", "vec4 alphaRef;\nuniform sampler2D tex0;");
    if (!ok)
    {
        LOG_WARN(Graphics, "XP impostor: off (the remastered prelude no longer has the expected declarations)");
        return false;
    }
    const std::string fs = common + kImpostorMain;
    g.prog = ImpLink(ImpCompile(GL_VERTEX_SHADER, kImpostorVS, "impostorVS"),
                     ImpCompile(GL_FRAGMENT_SHADER, fs.c_str(), "impostorFS"), "impostor");
    g.bakeProg = ImpLink(ImpCompile(GL_VERTEX_SHADER, kBakeVS, "impostorBakeVS"),
                         ImpCompile(GL_FRAGMENT_SHADER, kBakeFS, "impostorBakeFS"), "impostorBake");
    if (!g.prog || !g.bakeProg)
        return false;

    // Blocks and samplers as bindProgramSamplersAndBlocks gives the world
    // programs (EngineGL33_Shaders.cpp), plus the two atlases.
    GLint prevProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
    const GLuint p = g.prog;
    auto block = [p](const char* name, GLuint binding)
    {
        const GLuint b = glGetUniformBlockIndex(p, name);
        if (b != GL_INVALID_INDEX)
            glUniformBlockBinding(p, b, binding);
    };
    block("WorldInstances", 2);
    block("VSConstants", 0);
    block("PSConstants", 1);
    block("LitConstants", Poseidon::gl33lighting::kLitBinding);
    block("FrameLights", Poseidon::gl33lighting::kLightListBinding);
    glUseProgram(p);
    auto sampler = [p](const char* name, int unit)
    {
        const GLint loc = glGetUniformLocation(p, name);
        if (loc >= 0)
            glUniform1i(loc, unit);
    };
    sampler("tex0", 0);
    sampler("tex1", 1);
    for (const auto& ls : Poseidon::gl33lighting::kLitSamplers)
        sampler(ls.name, ls.unit);
    sampler("impAlbedo", kAlbedoUnit);
    sampler("impNormal", kNormalUnit);
    g.locSlots = glGetUniformLocation(p, "impSlots");
    g.locA2C = glGetUniformLocation(p, "impA2C");
    glUseProgram(g.bakeProg);
    {
        const GLint loc = glGetUniformLocation(g.bakeProg, "src");
        if (loc >= 0)
            glUniform1i(loc, kBakeSourceUnit);
    }
    g.locBakeR = glGetUniformLocation(g.bakeProg, "bR");
    g.locBakeG = glGetUniformLocation(g.bakeProg, "bG");
    g.locBakeY = glGetUniformLocation(g.bakeProg, "bY");
    glUseProgram(static_cast<GLuint>(prevProgram));

    GLint prevArray = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevArray);
    // Quad + instances.
    glGenVertexArrays(1, &g.vao);
    glGenBuffers(1, &g.quadVbo);
    glGenBuffers(1, &g.instVbo);
    GL33Bind::Vao(g.vao);
    const float quad[8] = {-1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f, 1.0f};
    glBindBuffer(GL_ARRAY_BUFFER, g.quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, g.instVbo);
    g.instCapacity = 4096;
    glBufferData(GL_ARRAY_BUFFER, g.instCapacity * sizeof(imp::Instance), nullptr, GL_STREAM_DRAW);
    for (int a = 0; a < 4; ++a)
    {
        glEnableVertexAttribArray(1 + a);
        glVertexAttribPointer(1 + a, 4, GL_FLOAT, GL_FALSE, sizeof(imp::Instance),
                              reinterpret_cast<void*>(static_cast<intptr_t>(a * 16)));
        glVertexAttribDivisor(1 + a, 1);
    }
    // Bake geometry.
    glGenVertexArrays(1, &g.bakeVao);
    glGenBuffers(1, &g.bakeVbo);
    GL33Bind::Vao(g.bakeVao);
    glBindBuffer(GL_ARRAY_BUFFER, g.bakeVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BakeVertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BakeVertex), reinterpret_cast<void*>(12));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BakeVertex), reinterpret_cast<void*>(24));
    GL33Bind::Vao(0);
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(prevArray));

    // Samplers: the atlases trilinear and clamped, the bake's sources
    // trilinear and repeating.
    glGenSamplers(1, &g.sampAtlas);
    glSamplerParameteri(g.sampAtlas, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(g.sampAtlas, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(g.sampAtlas, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(g.sampAtlas, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenSamplers(1, &g.sampSource);
    glSamplerParameteri(g.sampSource, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(g.sampSource, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(g.sampSource, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(g.sampSource, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindSampler(kAlbedoUnit, g.sampAtlas);
    glBindSampler(kNormalUnit, g.sampAtlas);
    glBindSampler(kBakeSourceUnit, g.sampSource);

    // Atlases: kMaxSlots * kFrames layers, kLevels levels.
    GLint prevUnpack = 0;
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &prevUnpack);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    const int layers = imp::kMaxSlots * imp::kFrames;
    auto makeArray = [layers](GLuint& tex, int unit)
    {
        glGenTextures(1, &tex);
        GL33Bind::ActiveUnit(unit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
        for (int l = 0; l < kLevels; ++l)
        {
            const int s = imp::kTile >> l;
            glTexImage3D(GL_TEXTURE_2D_ARRAY, l, GL_RGBA8, s, s, layers, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, kLevels - 1);
    };
    makeArray(g.albedoArr, kAlbedoUnit);
    makeArray(g.normalArr, kNormalUnit);
    GL33Bind::ActiveUnit(0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, static_cast<GLuint>(prevUnpack));

    // Bake target.
    GLint prevDraw = 0, prevRead = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
    glGenFramebuffers(1, &g.bakeFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, g.bakeFbo);
    glGenRenderbuffers(2, g.bakeColor);
    for (int i = 0; i < 2; ++i)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, g.bakeColor[i]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, imp::kBakeSize, imp::kBakeSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, g.bakeColor[i]);
    }
    glGenRenderbuffers(1, &g.bakeDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, g.bakeDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, imp::kBakeSize, imp::kBakeSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g.bakeDepth);
    const GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, bufs);
    const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(prevRead));
    if (!complete)
    {
        LOG_WARN(Graphics, "XP impostor: off (bake target incomplete)");
        return false;
    }
    g.slotUniforms.assign(imp::kMaxSlots * 4 * 4, 0.0f);
    const GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        LOG_WARN(Graphics, "XP impostor: GL error 0x{:04X} during setup", static_cast<unsigned int>(err));
    }
    LOG_INFO(Graphics, "XP impostor: on (from {:.0f} m, {} frames, {} px tiles, level {})", imp::Get().distance,
             imp::kFrames, imp::kTile, imp::Get().lod0 ? "finest" : "coarsest");
    return true;
}

// Bakes slot `s` (all frames). The world pass's state is put back afterwards.
static bool BakeImpostorSlot(ImpostorGL& g, int s)
{
    ImpostorSlotData& slot = g.slots[s];
    Poseidon::LODShape* lshape = slot.shape.GetRef();
    Shape* level = lshape ? lshape->Level(slot.level) : nullptr;
    if (!level)
        return false;

    // Geometry: every visible textured section as triangles, model space,
    // with the shader's normal convention (VertexBufferGL33::FillVertices
    // negates the stored normal).
    std::vector<BakeVertex> verts;
    struct Run
    {
        Texture* tex;
        int first;
        int count;
    };
    std::vector<Run> runs;
    for (int i = 0; i < level->NSections(); ++i)
    {
        const Poseidon::ShapeSection& sec = level->GetSection(i);
        if (sec.properties.Special() & (IsHidden | IsHiddenProxy))
            continue;
        Texture* tex = sec.properties.GetTexture();
        if (!tex)
            continue;
        const int first = static_cast<int>(verts.size());
        for (Offset o = sec.beg; o < sec.end; level->NextFace(o))
        {
            const Poly& face = level->Face(o);
            for (int k = 2; k < face.N(); ++k)
            {
                const int idx[3] = {face.GetVertex(0), face.GetVertex(k - 1), face.GetVertex(k)};
                for (int v : idx)
                {
                    BakeVertex bv;
                    const Vector3 p = level->Pos(v);
                    const Vector3 n = level->Norm(v);
                    bv.p[0] = p.X(), bv.p[1] = p.Y(), bv.p[2] = p.Z();
                    bv.n[0] = -n.X(), bv.n[1] = -n.Y(), bv.n[2] = -n.Z();
                    bv.uv[0] = level->U(v), bv.uv[1] = level->V(v);
                    verts.push_back(bv);
                }
            }
        }
        const int count = static_cast<int>(verts.size()) - first;
        if (count > 0)
            runs.push_back({tex, first, count});
    }
    if (verts.empty())
        return false;

    // Frame: the centre of the horizontal box, the largest distance from it
    // (so every azimuth fits), the height.
    float mnX = 1e30f, mxX = -1e30f, mnY = 1e30f, mxY = -1e30f, mnZ = 1e30f, mxZ = -1e30f;
    for (const BakeVertex& v : verts)
    {
        mnX = std::min(mnX, v.p[0]), mxX = std::max(mxX, v.p[0]);
        mnY = std::min(mnY, v.p[1]), mxY = std::max(mxY, v.p[1]);
        mnZ = std::min(mnZ, v.p[2]), mxZ = std::max(mxZ, v.p[2]);
    }
    slot.ctrX = 0.5f * (mnX + mxX);
    slot.ctrZ = 0.5f * (mnZ + mxZ);
    float r2 = 0.0f;
    for (const BakeVertex& v : verts)
        r2 = std::max(r2, (v.p[0] - slot.ctrX) * (v.p[0] - slot.ctrX) + (v.p[2] - slot.ctrZ) * (v.p[2] - slot.ctrZ));
    slot.halfW = std::sqrt(r2) * 1.02f + 0.01f;
    const float pad = (mxY - mnY) * 0.01f + 0.01f;
    slot.minY = mnY - pad;
    slot.maxY = mxY + pad;
    {
        const Vector3 bmin = level->Min();
        const Vector3 bmax = level->Max();
        const float boxMin[3] = {bmin.X(), bmin.Y(), bmin.Z()};
        const float boxMax[3] = {bmax.X(), bmax.Y(), bmax.Z()};
        float invHalf[3];
        Poseidon::remaster::CrownEllipsoid(boxMin, boxMax, slot.crownC, invHalf);
        slot.crownS[0] = invHalf[0], slot.crownS[1] = invHalf[1], slot.crownS[2] = invHalf[2];
    }

    // Source textures at a useful level (synchronously, once).
    AbstractTextBank* bank = Poseidon::GEngine->TextBank();
    std::vector<GLuint> handles(runs.size(), 0);
    for (size_t r = 0; r < runs.size(); ++r)
    {
        bank->UseMipmap(runs[r].tex, 1, 1);
        handles[r] = static_cast<TextureGL33*>(runs[r].tex)->GetHandle();
    }

    SavedCaps caps;
    caps.Save();
    GLint prevDraw = 0, prevRead = 0, viewport[4], prevPack = 0, prevUnpack = 0;
    GLint packAlign, packRow, packSkipR, packSkipP, unAlign, unRow, unSkipR, unSkipP, unImgH, unSkipI;
    GLfloat clearColor[4], clearDepth;
    GLboolean colorMask0[4], colorMask1[4];
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &prevPack);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &prevUnpack);
    glGetIntegerv(GL_PACK_ALIGNMENT, &packAlign);
    glGetIntegerv(GL_PACK_ROW_LENGTH, &packRow);
    glGetIntegerv(GL_PACK_SKIP_ROWS, &packSkipR);
    glGetIntegerv(GL_PACK_SKIP_PIXELS, &packSkipP);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &unAlign);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &unRow);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS, &unSkipR);
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &unSkipP);
    glGetIntegerv(GL_UNPACK_IMAGE_HEIGHT, &unImgH);
    glGetIntegerv(GL_UNPACK_SKIP_IMAGES, &unSkipI);
    glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
    glGetFloatv(GL_DEPTH_CLEAR_VALUE, &clearDepth);
    glGetBooleani_v(GL_COLOR_WRITEMASK, 0, colorMask0);
    glGetBooleani_v(GL_COLOR_WRITEMASK, 1, colorMask1);

    const Poseidon::render::pipeline::PolygonOffsetState offset = Poseidon::render::pipeline::CapturePolygonOffset();
    glBindFramebuffer(GL_FRAMEBUFFER, g.bakeFbo);
    glViewport(0, 0, imp::kBakeSize, imp::kBakeSize);
    Poseidon::render::blend::Opaque();
    Poseidon::render::cull::None();
    // The bake target has no stencil: the stencil leg is left alone.
    Poseidon::render::depthstencil::Normal(false);
    Poseidon::render::pipeline::SetPolygonOffsetForDecals(false);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_ALPHA_TO_ONE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glColorMaski(1, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearDepth(1.0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);

    glUseProgram(g.bakeProg);
    GL33Bind::Vao(g.bakeVao);
    glBindBuffer(GL_ARRAY_BUFFER, g.bakeVbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(BakeVertex)), verts.data(),
                 GL_STREAM_DRAW);
    glUniform4f(g.locBakeG, slot.ctrX, slot.ctrZ, 1.0f / slot.halfW, 0.0f);
    glUniform4f(g.locBakeY, slot.minY, 1.0f / std::max(slot.maxY - slot.minY, 1e-3f), 0.0f, 0.0f);
    GL33Bind::ActiveUnit(kBakeSourceUnit);

    const int n = imp::kBakeSize;
    imp::Image bakeA, bakeN, tileA, tileN;
    bakeA.w = bakeA.h = bakeN.w = bakeN.h = n;
    bakeA.rgba.resize(static_cast<size_t>(n) * n * 4);
    bakeN.rgba.resize(static_cast<size_t>(n) * n * 4);
    std::vector<imp::Image> chainA, chainN;
    constexpr float kTwoPi = 6.28318530717958647692f;
    for (int k = 0; k < imp::kFrames; ++k)
    {
        const float th = kTwoPi * static_cast<float>(k) / static_cast<float>(imp::kFrames);
        const float c = std::cos(th), sn = std::sin(th);
        // across R = (cos, -sin) in (x, z); toward the view C = (sin, cos).
        glUniform4f(g.locBakeR, c, -sn, sn, c);
        Poseidon::render::pipeline::SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        Poseidon::render::clear::WithMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (size_t r = 0; r < runs.size(); ++r)
        {
            if (!handles[r])
                continue;
            glBindTexture(GL_TEXTURE_2D, handles[r]);
            glDrawArrays(GL_TRIANGLES, runs[r].first, runs[r].count);
        }
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, n, n, GL_RGBA, GL_UNSIGNED_BYTE, bakeA.rgba.data());
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        glReadPixels(0, 0, n, n, GL_RGBA, GL_UNSIGNED_BYTE, bakeN.rgba.data());

        // The fill for empty texels: the frame's mean leaf colour (so the mips
        // do not bleed black), a zero normal at the centre depth.
        double sum[3] = {0, 0, 0};
        double cnt = 0;
        for (size_t i = 0; i < bakeA.rgba.size(); i += 4)
            if (bakeA.rgba[i + 3] > 0)
            {
                sum[0] += bakeA.rgba[i], sum[1] += bakeA.rgba[i + 1], sum[2] += bakeA.rgba[i + 2];
                cnt += 1;
            }
        std::uint8_t fill[4] = {96, 104, 72, 0};
        if (cnt > 0)
            for (int ch = 0; ch < 3; ++ch)
                fill[ch] = static_cast<std::uint8_t>(sum[ch] / cnt + 0.5);
        const std::uint8_t fillN[4] = {128, 128, 128, 128};
        for (size_t i = 0; i < bakeA.rgba.size(); i += 4)
            if (bakeA.rgba[i + 3] == 0)
            {
                std::memcpy(&bakeA.rgba[i], fill, 4);
                std::memcpy(&bakeN.rgba[i], fillN, 4);
            }
        imp::Downsample(bakeA, bakeN, tileA, tileN, fill, fillN);
        imp::BuildChain(tileA, tileN, fill, fillN, chainA, chainN);

        const int layer = s * imp::kFrames + k;
        GL33Bind::ActiveUnit(kAlbedoUnit);
        for (int l = 0; l < kLevels && l < static_cast<int>(chainA.size()); ++l)
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, l, 0, 0, layer, chainA[l].w, chainA[l].h, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            chainA[l].rgba.data());
        GL33Bind::ActiveUnit(kNormalUnit);
        for (int l = 0; l < kLevels && l < static_cast<int>(chainN.size()); ++l)
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, l, 0, 0, layer, chainN[l].w, chainN[l].h, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            chainN[l].rgba.data());
        GL33Bind::ActiveUnit(kBakeSourceUnit);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::ActiveUnit(0);

    // Put back the world pass.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(prevRead));
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glColorMaski(0, colorMask0[0], colorMask0[1], colorMask0[2], colorMask0[3]);
    glColorMaski(1, colorMask1[0], colorMask1[1], colorMask1[2], colorMask1[3]);
    Poseidon::render::pipeline::SetClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    Poseidon::render::pipeline::RestorePolygonOffset(offset);
    glClearDepth(clearDepth);
    glPixelStorei(GL_PACK_ALIGNMENT, packAlign);
    glPixelStorei(GL_PACK_ROW_LENGTH, packRow);
    glPixelStorei(GL_PACK_SKIP_ROWS, packSkipR);
    glPixelStorei(GL_PACK_SKIP_PIXELS, packSkipP);
    glPixelStorei(GL_UNPACK_ALIGNMENT, unAlign);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, unRow);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, unSkipR);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, unSkipP);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, unImgH);
    glPixelStorei(GL_UNPACK_SKIP_IMAGES, unSkipI);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, static_cast<GLuint>(prevPack));
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, static_cast<GLuint>(prevUnpack));
    caps.Restore();

    // Slot constants for the draw program.
    float* u = &g.slotUniforms[static_cast<size_t>(s) * 16];
    u[0] = slot.ctrX, u[1] = slot.ctrZ, u[2] = slot.halfW, u[3] = static_cast<float>(s * imp::kFrames);
    u[4] = slot.minY, u[5] = slot.maxY, u[6] = static_cast<float>(imp::kFrames), u[7] = 0.0f;
    u[8] = slot.crownC[0], u[9] = slot.crownC[1], u[10] = slot.crownC[2], u[11] = 1.0f;
    u[12] = slot.crownS[0], u[13] = slot.crownS[1], u[14] = slot.crownS[2], u[15] = 0.0f;
    g.slotsDirty = true;
    ++g.bakes;
    LOG_INFO(Graphics, "XP impostor: baked slot {} '{}' level {} ({} triangles, {} textures, {:.1f} x {:.1f} m)", s,
             static_cast<const char*>(lshape->Name()), slot.level, verts.size() / 3, runs.size(), 2.0f * slot.halfW,
             slot.maxY - slot.minY);
    return true;
}

// The engine's own depth, stencil and blend state, and its cached alpha to
// coverage, back after the bake or the draw; the next draw re-applies its whole
// pipeline (culling among it).
void EngineGL33::RestoreAfterImpostor()
{
    ApplyDepthMode(_currentDepthMode);
    ApplyBlendMode(_currentBlendMode);
    if (_a2cBound)
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    else
        glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    if (_a2oneBound)
        glEnable(GL_SAMPLE_ALPHA_TO_ONE);
    else
        glDisable(GL_SAMPLE_ALPHA_TO_ONE);
    InvalidatePipelineCache();
}

int EngineGL33::ImpostorSlot(Poseidon::LODShape* shape, int& stateSection, int& level)
{
    stateSection = -1;
    level = -1;
    ImpostorGL& g = G();
    if (!shape || g.failed)
        return -1;
    if (!g.init)
    {
        FlushAndFreeAllQueues(_queueNo, true);
        if (!InitImpostorGL(g))
        {
            g.failed = true;
            return -1;
        }
        InvalidatePipelineCache();
    }
    auto it = g.index.find(shape);
    if (it != g.index.end())
    {
        const int s = it->second;
        if (s < 0)
            return -1;
        ImpostorSlotData& slot = g.slots[s];
        if (slot.rebakeAt && GetFrameCounter() >= slot.rebakeAt)
        {
            slot.rebakeAt = 0;
            FlushAndFreeAllQueues(_queueNo, true);
            BakeImpostorSlot(g, s);
            RestoreAfterImpostor();
        }
        stateSection = slot.stateSection;
        level = slot.level;
        return s;
    }
    // A new shape: pick the level and the state section, then bake.
    g.index[shape] = -1;
    if (static_cast<int>(g.slots.size()) >= imp::kMaxSlots || shape->NLevels() <= 0)
        return -1;
    int lv = 0;
    if (!imp::Get().lod0)
    {
        for (int i = shape->NLevels() - 1; i > 0; --i)
            if (shape->Resolution(i) < 900.0f && shape->Level(i) && shape->Level(i)->NFaces() > 0)
            {
                lv = i;
                break;
            }
    }
    Shape* lvl = shape->Level(lv);
    if (!lvl || lvl->NFaces() <= 0 || lvl->NProxies() > 0)
        return -1;
    // The state: the section with the most faces among the best alpha class
    // (cutout leaves before blended ones before opaque bark).
    int best = -1, bestRank = -2, bestFaces = -1;
    for (int i = 0; i < lvl->NSections(); ++i)
    {
        const Poseidon::ShapeSection& sec = lvl->GetSection(i);
        if (sec.properties.Special() & (IsHidden | IsHiddenProxy))
            continue;
        Texture* tex = sec.properties.GetTexture();
        if (!tex)
            continue;
        int faces = 0;
        for (Offset o = sec.beg; o < sec.end; lvl->NextFace(o))
            ++faces;
        const int rank = AlphaRank(tex);
        if (rank > bestRank || (rank == bestRank && faces > bestFaces))
        {
            best = i, bestRank = rank, bestFaces = faces;
        }
    }
    if (best < 0)
        return -1;
    const int s = static_cast<int>(g.slots.size());
    g.slots.emplace_back();
    g.slots[s].shape = shape;
    g.slots[s].level = lv;
    g.slots[s].stateSection = best;
    FlushAndFreeAllQueues(_queueNo, true);
    const bool ok = BakeImpostorSlot(g, s);
    RestoreAfterImpostor();
    if (!ok)
    {
        g.slots.pop_back();
        return -1;
    }
    g.slots[s].rebakeAt = GetFrameCounter() + kRebakeFrames;
    g.index[shape] = s;
    stateSection = best;
    level = lv;
    return s;
}

void EngineGL33::DrawImpostors(const void* instances, int n)
{
    ImpostorGL& g = G();
    if (!g.init || g.failed || n <= 0 || !instances)
        return;
    // The prepared state's queued draws first.
    FlushAndFreeAllQueues(_queueNo, true);

    // Camera-relative positions, as the instanced runs (InstancedRunAdd).
    static std::vector<imp::Instance> rel;
    rel.assign(static_cast<const imp::Instance*>(instances), static_cast<const imp::Instance*>(instances) + n);
    for (imp::Instance& i : rel)
    {
        i.pos[0] -= _frameState.cameraPos[0];
        i.pos[1] -= _frameState.cameraPos[1];
        i.pos[2] -= _frameState.cameraPos[2];
    }

    SavedCaps caps;
    caps.Save();
    GLint sampleBuffers = 0;
    glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);
    const bool a2c = sampleBuffers > 0;

    glUseProgram(g.prog);
    if (g.slotsDirty && g.locSlots >= 0)
    {
        glUniform4fv(g.locSlots, imp::kMaxSlots * 4, g.slotUniforms.data());
        g.slotsDirty = false;
    }
    if (g.locA2C >= 0)
        glUniform1f(g.locA2C, a2c ? 1.0f : 0.0f);
    GL33Bind::ActiveUnit(kAlbedoUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, g.albedoArr);
    GL33Bind::ActiveUnit(kNormalUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, g.normalArr);
    GL33Bind::ActiveUnit(0);

    GL33Bind::Vao(g.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g.instVbo);
    if (static_cast<size_t>(n) > g.instCapacity)
    {
        g.instCapacity = static_cast<size_t>(n) * 2;
    }
    // Orphan and refill: one upload per group.
    glBufferData(GL_ARRAY_BUFFER, g.instCapacity * sizeof(imp::Instance), nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(n * sizeof(imp::Instance)), rel.data());

    Poseidon::render::blend::Opaque();
    Poseidon::render::cull::None();
    // Depth written, the stencil tag the prepared state chose (the leaf
    // section's reflections tag).
    Poseidon::render::depthstencil::Normal(
        _hasStencilBuffer, Poseidon::render::depthstencil::StencilTag{_ssrStencil.ref, _ssrStencil.writeMask,
                                                                      _ssrStencil.shadowCompareMask});
    if (a2c)
    {
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        glEnable(GL_SAMPLE_ALPHA_TO_ONE);
    }
    else
    {
        glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        glDisable(GL_SAMPLE_ALPHA_TO_ONE);
    }
    FlushPSConstants();
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, n);
    ++Poseidon::gPerfDrawCalls;
    caps.Restore();
    RestoreAfterImpostor();
}

void EngineGL33::DestroyImpostors()
{
    ImpostorGL& g = G();
    if (!g.init)
        return;
    if (g.prog)
        glDeleteProgram(g.prog);
    if (g.bakeProg)
        glDeleteProgram(g.bakeProg);
    if (g.vao)
    {
        GL33Bind::OnVaoDeleted(g.vao);
        glDeleteVertexArrays(1, &g.vao);
    }
    if (g.bakeVao)
    {
        GL33Bind::OnVaoDeleted(g.bakeVao);
        glDeleteVertexArrays(1, &g.bakeVao);
    }
    const GLuint buffers[3] = {g.quadVbo, g.instVbo, g.bakeVbo};
    glDeleteBuffers(3, buffers);
    if (g.bakeFbo)
        glDeleteFramebuffers(1, &g.bakeFbo);
    glDeleteRenderbuffers(2, g.bakeColor);
    if (g.bakeDepth)
        glDeleteRenderbuffers(1, &g.bakeDepth);
    const GLuint arrays[2] = {g.albedoArr, g.normalArr};
    glDeleteTextures(2, arrays);
    const GLuint samplers[2] = {g.sampAtlas, g.sampSource};
    glDeleteSamplers(2, samplers);
    g = ImpostorGL{};
}
