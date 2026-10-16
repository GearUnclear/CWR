#!/usr/bin/env python3
"""S directional aerial-color CPU/source regression contracts.

Compiles the selected engine tree's exact descriptor, builder and eligibility
headers, plus unchanged complete definitions of ApplyPipeline,
DrawExperimentalSky, NextFrame, both activation functions and compileSky.
Engine fields and external scene/GL/upload dependencies are recording stubs,
NOT the real engine or a GL implementation. Explicit allowed tuples, named
material cases and independent numerical oracles define the intended scope.

No engine launch, GPU execution, assets, screenshots or visual acceptance.
Requires Python 3 and a C++17 GNU-style compiler; no Python dependencies.
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile


HEADER_PATHS = [
    "Poseidon/Graphics/Rendering/RenderFlags.hpp",
    "Poseidon/Graphics/Rendering/RenderPassDescriptor.hpp",
    "Poseidon/Graphics/Rendering/BuildRenderPassDescriptor.hpp",
    "Poseidon/Graphics/Rendering/ValidateRenderPassDescriptor.hpp",
    "Poseidon/Graphics/Rendering/AerialColorEligibility.hpp",
]
STATE = "PoseidonGL33/EngineGL33_State.cpp"
SKY = "PoseidonGL33/EngineGL33_ExperimentalSky.cpp"
LIFECYCLE = "PoseidonGL33/EngineGL33_Lifecycle.cpp"
ENGINE = "PoseidonGL33/EngineGL33.hpp"
SHADERS = "PoseidonGL33/EngineGL33_Shaders.cpp"
SOURCE_PATHS = [STATE, SKY, LIFECYCLE, ENGINE, SHADERS]
# Raw GLSL literals are masked too, so their braces never affect C++ extraction.
CPP_NON_CODE = re.compile(
    r'R"(?P<delimiter>[^ ()\\\t\r\n]{0,16})\(.*?\)(?P=delimiter)"|'
    r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S
)


def code_mask(text):
    return CPP_NON_CODE.sub(lambda match: " " * len(match.group()), text)


def definition(text, signature):
    """Return the exact source definition; never rewrite its implementation."""
    masked = code_mask(text)
    matches = list(re.finditer(signature, masked))
    if len(matches) != 1:
        raise AssertionError(f"Expected one definition for {signature!r}, found {len(matches)}")
    start = matches[0].start()
    opening = masked.index("{", matches[0].end() - 1)
    depth = 0
    for pos in range(opening, len(masked)):
        depth += (masked[pos] == "{") - (masked[pos] == "}")
        if depth == 0:
            return text[start:pos + 1]
    raise AssertionError(f"Unclosed source definition: {signature}")


def compact(text):
    text = CPP_NON_CODE.sub(
        lambda match: " " * len(match.group()) if match.group().startswith(("//", "/*")) else match.group(), text
    )
    return re.sub(r"\s+", "", text)


PREAMBLE = r'''
#include <Poseidon/Graphics/Rendering/AerialColorEligibility.hpp>
#include <Poseidon/Graphics/Rendering/BuildRenderPassDescriptor.hpp>
#include <Poseidon/Graphics/Rendering/ValidateRenderPassDescriptor.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
using namespace Poseidon::render;
unsigned checks = 0;
void require(bool okay, const std::string& label) {
    ++checks;
    if (!okay) throw std::runtime_error(label);
}
void near(float actual, float expected, const std::string& label, float tolerance = 0.000004f) {
    require(std::isfinite(actual) && std::abs(actual - expected) <= tolerance, label);
}
'''

DESCRIPTOR_TEST = r'''
void descriptorMatrix() {
    const PassKind passes[] = {
        PassKind::WorldOpaque, PassKind::WorldCutout, PassKind::WorldTransparent,
        PassKind::WorldShadow, PassKind::WorldLight, PassKind::WorldWater,
        PassKind::SurfaceOverlay, PassKind::CockpitOpaque, PassKind::CockpitCutout,
        PassKind::CockpitTransparent, PassKind::ScreenSpace3D};
    const DepthMode depths[] = {DepthMode::Normal, DepthMode::ReadOnly, DepthMode::Disabled, DepthMode::Shadow};
    const BlendMode blends[] = {BlendMode::Opaque, BlendMode::AlphaBlend, BlendMode::Additive, BlendMode::Shadow};
    const FogMode fogs[] = {FogMode::Enabled, FogMode::Disabled, FogMode::AlphaFog};
    const ShaderFamily shaders[] = {ShaderFamily::Normal, ShaderFamily::Shadow, ShaderFamily::Water,
                                    ShaderFamily::Detail, ShaderFamily::Grass, ShaderFamily::Flat};
    const LightingMode lights[] = {LightingMode::Lit, LightingMode::SunDisabled, LightingMode::Unlit,
                                   LightingMode::ShadowDarkPolygon};
    const SurfaceMode surfaces[] = {SurfaceMode::Default, SurfaceMode::OnSurface};
    using Key = std::tuple<PassKind, DepthMode, BlendMode, FogMode, ShaderFamily, bool>;
    std::vector<Key> allowed;
    // Independent explicit scope table. Each row lists its complete allowed
    // depth/blend combinations; shader support is Normal, Detail and Grass.
    // Lighting and surface metadata are intentionally absent from the key.
    const std::tuple<PassKind, DepthMode, BlendMode> drawClasses[] = {
        {PassKind::WorldOpaque, DepthMode::Normal, BlendMode::Opaque},
        {PassKind::WorldOpaque, DepthMode::Normal, BlendMode::AlphaBlend},
        {PassKind::WorldOpaque, DepthMode::ReadOnly, BlendMode::Opaque},
        {PassKind::WorldOpaque, DepthMode::ReadOnly, BlendMode::AlphaBlend},
        {PassKind::WorldCutout, DepthMode::Normal, BlendMode::Opaque},
        {PassKind::WorldCutout, DepthMode::Normal, BlendMode::AlphaBlend},
        {PassKind::WorldCutout, DepthMode::ReadOnly, BlendMode::Opaque},
        {PassKind::WorldCutout, DepthMode::ReadOnly, BlendMode::AlphaBlend},
        {PassKind::WorldTransparent, DepthMode::Normal, BlendMode::Opaque},
        {PassKind::WorldTransparent, DepthMode::Normal, BlendMode::AlphaBlend},
        {PassKind::WorldTransparent, DepthMode::ReadOnly, BlendMode::Opaque},
        {PassKind::WorldTransparent, DepthMode::ReadOnly, BlendMode::AlphaBlend},
        {PassKind::SurfaceOverlay, DepthMode::Normal, BlendMode::Opaque},
        {PassKind::SurfaceOverlay, DepthMode::Normal, BlendMode::AlphaBlend},
        {PassKind::SurfaceOverlay, DepthMode::ReadOnly, BlendMode::Opaque},
        {PassKind::SurfaceOverlay, DepthMode::ReadOnly, BlendMode::AlphaBlend},
        {PassKind::WorldWater, DepthMode::Normal, BlendMode::Opaque},
        {PassKind::WorldWater, DepthMode::Normal, BlendMode::AlphaBlend},
        {PassKind::WorldWater, DepthMode::ReadOnly, BlendMode::Opaque},
        {PassKind::WorldWater, DepthMode::ReadOnly, BlendMode::AlphaBlend},
    };
    for (const auto& row : drawClasses)
    for (auto shader : {ShaderFamily::Normal, ShaderFamily::Detail, ShaderFamily::Grass})
        allowed.emplace_back(std::get<0>(row), std::get<1>(row), std::get<2>(row), FogMode::Enabled, shader, true);
    // The special Water shader is supported only in WorldWater, never on
    // arbitrary world/cockpit/UI passes, independently of normal-family water.
    for (auto depth : {DepthMode::Normal, DepthMode::ReadOnly})
    for (auto blend : {BlendMode::Opaque, BlendMode::AlphaBlend})
        allowed.emplace_back(PassKind::WorldWater, depth, blend, FogMode::Enabled, ShaderFamily::Water, true);
    unsigned rows = 0, positive = 0;
    for (auto pass : passes) for (auto depth : depths) for (auto blend : blends)
    for (auto fog : fogs) for (auto shader : shaders) for (bool mesh : {false, true})
    for (auto light : lights) for (auto surface : surfaces) {
        RenderPassDescriptor d;
        d.pass = pass; d.depth = depth; d.blend = blend; d.fog = fog;
        d.shader = shader; d.lighting = light; d.surface = surface;
        const Key key{pass, depth, blend, fog, shader, mesh};
        const bool expected = std::find(allowed.begin(), allowed.end(), key) != allowed.end();
        require(IsAerialColorReceiver(d, mesh) == expected, "aerial Cartesian row " + std::to_string(rows));
        positive += expected; ++rows;
    }
    require(rows == 50688 && positive == 512 && allowed.size() == 64, "matrix cardinality and positive controls");
}

void orthogonalFields() {
    RenderPassDescriptor d;
    for (auto alpha : {AlphaMode::Disabled, AlphaMode::Test, AlphaMode::Blend, AlphaMode::TestAndBlend})
    for (auto cull : {CullMode::Back, CullMode::Front, CullMode::None})
    for (auto front : {FrontFaceMode::CW, FrontFaceMode::CCW})
    for (auto filter : {SamplerFilter::Linear, SamplerFilter::Point})
    for (auto tg : {TexGenMode::None, TexGenMode::Fixed, TexGenMode::Water, TexGenMode::Detail, TexGenMode::Grass})
    for (bool flag : {false, true}) {
        d.alpha = alpha; d.cull = cull; d.frontFace = front; d.sampler = {filter, flag, !flag};
        d.texGen = tg; d.terrainDetail = flag; d.stencilExclusion = flag; d.alphaRef = flag ? 255 : 0;
        require(IsAerialColorReceiver(d, true), "orthogonal material metadata does not narrow atmosphere scope");
    }
    const RenderPassDescriptor base;
    auto different = [&](RenderPassDescriptor d, const char* name) {
        require(d != base && !(d == base), std::string("descriptor cache key: ") + name);
    };
    d = base; d.fog = FogMode::Disabled; different(d, "fog");
    d = base; d.pass = PassKind::CockpitOpaque; different(d, "pass");
    d = base; d.depth = DepthMode::Disabled; different(d, "depth");
    d = base; d.blend = BlendMode::Additive; different(d, "blend");
    d = base; d.shader = ShaderFamily::Flat; different(d, "shader");
}

struct MaterialCase {
    const char* name; bool eligible;
    Backend backend = Backend::None;
    Routing routing = Routing::None;
    Material material = Material::None;
    bool multi = false;
    PassKindHint hint = PassKindHint::None;
};
void buildCases() {
    const MaterialCase cases[] = {
        {"world opaque", true},
        {"world cutout", true, Backend::IsTransparent},
        {"world alpha", true, Backend::IsAlpha},
        {"alpha overrides cutout", true, Backend::IsAlpha | Backend::IsTransparent},
        {"depth read only", true, Backend::NoZWrite},
        {"alpha read only", true, Backend::IsAlpha | Backend::NoZWrite},
        {"Detail", true, Backend::DetailTexture, Routing::None, Material::None, true},
        {"Grass", true, Backend::GrassTexture, Routing::None, Material::None, true},
        {"Grass alpha read only", true, Backend::GrassTexture | Backend::IsAlpha | Backend::NoZWrite,
         Routing::None, Material::None, true},
        {"specular Detail", true, Backend::SpecularTexture, Routing::None, Material::None, true},
        {"inactive multitexture remains Normal", true, Backend::GrassTexture},
        {"sun disabled is still atmosphere", true, Backend::None, Routing::None, Material::DisableSun},
        {"surface decal", true, Backend::None, Routing::OnSurface},
        {"surface road", true, Backend::NoZWrite, Routing::IsOnSurface},
        {"blended surface Detail", true, Backend::DetailTexture | Backend::IsAlpha | Backend::NoZWrite,
         Routing::OnSurface, Material::None, true},
        {"alpha fog is not ordinary fog", false, Backend::IsAlphaFog},
        {"fog disabled", false, Backend::None, Routing::FogDisabled},
        {"NoDropdown disables fog", false, Backend::None, Routing::NoDropdown},
        {"depth disabled", false, Backend::NoZBuf},
        {"water", true, Backend::IsWater},
        {"water read only", true, Backend::IsWater | Backend::NoZWrite},
        {"water fog disabled", false, Backend::IsWater, Routing::FogDisabled},
        {"water depth disabled", false, Backend::IsWater | Backend::NoZBuf},
        {"projected shadow", false, Backend::IsShadow},
        {"additive light", false, Backend::IsLight},
        {"cockpit opaque", false, Backend::None, Routing::None, Material::None, false, PassKindHint::Cockpit},
        {"cockpit cutout", false, Backend::IsTransparent, Routing::None, Material::None, false, PassKindHint::Cockpit},
        {"cockpit alpha", false, Backend::IsAlpha, Routing::None, Material::None, false, PassKindHint::Cockpit},
        {"UI opaque", false, Backend::None, Routing::None, Material::None, false, PassKindHint::ScreenSpace3D},
        {"UI cutout", false, Backend::IsTransparent, Routing::None, Material::None, false, PassKindHint::ScreenSpace3D},
        {"UI alpha", false, Backend::IsAlpha, Routing::None, Material::None, false, PassKindHint::ScreenSpace3D},
    };
    for (const auto& c : cases) {
        BuildContext context;
        context.isMultitexturing = c.multi; context.passKindHint = c.hint;
        const auto d = BuildRenderPassDescriptor({c.routing, c.material, c.backend}, context);
        require(IsAerialColorReceiver(d, true) == c.eligible, std::string(c.name) + " builder classification");
        require(!IsAerialColorReceiver(d, false), std::string(c.name) + " TL exclusion");
    }
}
'''

RECORDING_SHELL = r'''
// RECORDING STUBS, NOT A REAL ENGINE OR GL CONTEXT. These dependencies carry
// supplied test inputs and record effects; no aerial predicate/parser/activation
// or production ApplyPipeline/DrawExperimentalSky body is recreated here.
using GLuint = unsigned;
using GLenum = unsigned;
using GLint = int;
using GLboolean = unsigned char;
enum RecordingGLToken : GLenum {
    GL_VERTEX_SHADER = 1, GL_FRAGMENT_SHADER, GL_COMPILE_STATUS, GL_LINK_STATUS,
    GL_DRAW_FRAMEBUFFER_BINDING, GL_CURRENT_PROGRAM, GL_VERTEX_ARRAY_BINDING,
    GL_VIEWPORT, GL_POLYGON_MODE, GL_DEPTH_WRITEMASK, GL_COLOR_WRITEMASK,
    GL_CULL_FACE, GL_SCISSOR_TEST, GL_SAMPLE_ALPHA_TO_COVERAGE, GL_COLOR_LOGIC_OP,
    GL_RASTERIZER_DISCARD, GL_DEPTH_TEST, GL_STENCIL_TEST, GL_MAX_CLIP_DISTANCES,
    GL_FRONT_AND_BACK, GL_FILL, GL_TRIANGLES, GL_CLIP_DISTANCE0 = 100,
};
std::vector<std::string> calls;
std::map<int, std::array<float, 4>> uploads;
std::map<std::string, float> uniforms;
std::map<std::string, int> locations;
bool shaderOkay = true, programOkay = true, vaoOkay = true;
int boundFbo = 23, boundProgram = 3, boundVao = 5;
std::array<int, 4> viewport{4, 6, 800, 600};
unsigned skyDraws = 0;
template<class... Args> void recordingLog(const Args&...) {}
#define LOG_INFO(category, ...) recordingLog(__VA_ARGS__)
#define LOG_DEBUG(category, ...) recordingLog(__VA_ARGS__)
#define LOG_ERROR(category, ...) recordingLog(__VA_ARGS__)
GLuint glCreateShader(GLenum) { calls.push_back("create-shader"); return 31; }
void glShaderSource(GLuint, int, const char* const*, const int*) {}
void glCompileShader(GLuint) { calls.push_back("compile-shader"); }
void glGetShaderiv(GLuint, GLenum, GLint* value) { *value = shaderOkay; }
void glGetShaderInfoLog(GLuint, int, int*, char* text) { text[0] = '\0'; }
void glDeleteShader(GLuint) { calls.push_back("delete-shader"); }
GLuint glCreateProgram() { return 37; }
void glAttachShader(GLuint, GLuint) {}
void glLinkProgram(GLuint) { calls.push_back("link-program"); }
void glGetProgramiv(GLuint, GLenum, GLint* value) { *value = programOkay; }
void glDeleteProgram(GLuint) { calls.push_back("delete-program"); }
void glGenVertexArrays(int, GLuint* value) { *value = vaoOkay ? 41 : 0; }
void glGetIntegerv(GLenum token, GLint* value) {
    switch (token) {
    case GL_DRAW_FRAMEBUFFER_BINDING: *value = boundFbo; break;
    case GL_CURRENT_PROGRAM: *value = boundProgram; break;
    case GL_VERTEX_ARRAY_BINDING: *value = boundVao; break;
    case GL_VIEWPORT: std::copy(viewport.begin(), viewport.end(), value); break;
    case GL_POLYGON_MODE: value[0] = value[1] = GL_FILL; break;
    case GL_MAX_CLIP_DISTANCES: *value = 2; break;
    default: throw std::runtime_error("unsupported recording glGetIntegerv token");
    }
}
void glGetBooleanv(GLenum token, GLboolean* value) {
    if (token == GL_DEPTH_WRITEMASK) *value = 1;
    else if (token == GL_COLOR_WRITEMASK) std::fill(value, value + 4, GLboolean(1));
    else throw std::runtime_error("unsupported recording glGetBooleanv token");
}
GLboolean glIsEnabled(GLenum) { return 0; }
void glEnable(GLenum) {}
void glDisable(GLenum) {}
void glViewport(int x, int y, int w, int h) { viewport = {x, y, w, h}; }
void glPolygonMode(GLenum, GLenum) {}
void glUseProgram(GLuint handle) { boundProgram = int(handle); }
void glBindVertexArray(GLuint handle) { boundVao = int(handle); }
int glGetUniformLocation(GLuint, const char* name) {
    auto item = locations.emplace(name, int(locations.size()) + 1);
    return item.first->second;
}
void glUniform1f(int location, float value) {
    for (const auto& entry : locations) if (entry.second == location) uniforms[entry.first] = value;
}
void glUniform3f(int, float, float, float) {}
void glDrawArrays(GLenum, int, int) { ++skyDraws; calls.push_back("draw-sky"); }
namespace GL33Bind { void Invalidate() { calls.push_back("invalidate-bindings"); } }
namespace Poseidon { namespace render {
namespace depthstencil { struct ScopedBackground { void Restore() {} }; }
namespace blend { struct ScopedAlphaOverlay { void Restore() {} bool MatchesSavedState() { return true; } }; }
namespace pipeline {
struct ScopedRgbOnly { void Restore() {} };
void SetPolygonOffsetForShadows(bool) {}
void SetPolygonOffsetForDecals(bool) {}
}
namespace cull { void Back() {} void Front() {} void None() {} void FrontFaceCW() {} void FrontFaceCCW() {} }
}}
struct Vector3 {
    float x = 0, y = 0, z = 1;
    float X() const { return x; } float Y() const { return y; } float Z() const { return z; }
    Vector3 operator*(float s) const { return {x*s, y*s, z*s}; }
};
using Vector3Par = const Vector3&;
struct Color {
    float r = .4f, g = .5f, b = .6f;
    float R() const { return r; } float G() const { return g; } float B() const { return b; }
    Color operator*(Color rhs) const { return {r*rhs.r, g*rhs.g, b*rhs.b}; }
};
struct RecordingSun {
    Vector3 direction{-.6f, -.4f, -.6928203f};
    float night = 0;
    Color sky{}, aureole{};
    Vector3 SunDirection() const { return direction; }
    float NightEffect() const { return night; }
    Color SkyColor() const { return sky; } Color SunSkyColor() const { return aureole; }
};
struct RecordingCamera {
    Vector3 forward{0, 0, 1}, right{1, 0, 0}, up{0, 1, 0};
    float left = 1, top = 1;
    Vector3 Direction() const { return forward; } Vector3 DirectionAside() const { return right; }
    Vector3 DirectionUp() const { return up; } float Left() const { return left; } float Top() const { return top; }
};
struct RecordingScene {
    RecordingSun* sun = nullptr; RecordingCamera* camera = nullptr;
    const RecordingSun* MainLight() const { return sun; }
    const RecordingCamera* GetCamera() const { return camera; }
};
RecordingScene* GScene = nullptr;
struct RecordingBase { void NextFrame() { calls.push_back("base-next-frame"); } };
class EngineGL33 : public RecordingBase {
public:
    using base = RecordingBase;
    // Backend IDs below are irrelevant dependency tokens, not replacements for
    // the exact descriptor enums or PipelineVertexInput under test.
    enum VertexShaderID { VSNone, VSShadow, VSScreen, VSTransform };
    enum PixelShaderID { PSNone, PSShadow, PSWater, PSDetail, PSGrass, PSFlat, PSNormal };
    enum VFormatSet { SingleTex, DetailTex, GrassTex };
    enum TexGenMode { TGNone, TGFixed, TGWater, TGDetail, TGGrass };
    // Exact production PipelineVertexInput declaration is inserted here.
'''

RECORDING_ENGINE_REST = r'''
    struct {
        RenderPassDescriptor d;
        bool in3d = false, a2c = false;
        PipelineVertexInput vertexInput = PipelineVertexInput::ActivePass;
        bool valid = false;
    } _lastApplied;
    PipelineVertexInput _pipelineVertexInput = PipelineVertexInput::Mesh;
    bool _glContext = true, _experimentalSkyFailed = false, _cloudValid = false;
    bool in3d = true, a2c = false, nvg = false;
    bool _pointSampling = false, _lastClampU = false, _lastClampV = false;
    GLuint _experimentalSkyProgram = 37, _experimentalSkyVao = 41, _ssaaFbo = 23;
    GLuint _lastTexture1Handle = 11;
    PixelShaderID _pixelShaderSel = PSNone;
    Color _fogColor{}, accommodation{1, 1, 1};
    float _aerialControl[4] = {};
    struct { float worldLeft = 0, worldRight = 1, worldTop = 0, worldBottom = 1; } _aspectSettings;
    int width = 800, height = 600;
    bool IsIn3DPass() const { return in3d; } bool GetAlphaToCoverage() const { return a2c; }
    bool GetNightVision() const { return nvg; } Color GetAccomodateEye() const { return accommodation; }
    void RenderTargetSize(int& w, int& h) const { w = width; h = height; }
    void InvalidatePipelineCache() { _lastApplied.valid = false; calls.push_back("invalidate-pipeline"); }
    void FlushQueues() { calls.push_back("flush-queues"); }
    void BackToFront() { calls.push_back("back-to-front"); }
    void ApplySamplerState() {} void ApplyDepthMode(DepthMode) {} void ApplyBlendMode(BlendMode) {}
    void SetShaderFogEnabled(bool) {} void SetAlphaTest(bool, unsigned, bool) {}
    void DoStencilExclusion(bool, bool) {} void EnableDetailTexGen(TexGenMode, bool) {}
    void SelectVertexShader(VertexShaderID) {} void SelectPixelShader(PixelShaderID) {}
    void SetMultiTexturing(VFormatSet) {}
    void ApplyCloudRegistration(bool) { calls.push_back("cloud-registration-stub"); }
    void UploadPSConstant(int slot, const float* value) {
        calls.push_back("upload:" + std::to_string(slot));
        std::copy(value, value + 4, uploads[slot].begin());
    }
    void ApplyPipeline(const RenderPassDescriptor& d);
    bool DrawExperimentalSky(float skyThrough);
    void NextFrame();
};
'''

RUNTIME_TEST = r'''
void resetRecording() {
    calls.clear(); uploads.clear(); uniforms.clear(); locations.clear();
    shaderOkay = programOkay = vaoOkay = true;
    boundFbo = 23; boundProgram = 3; boundVao = 5;
    viewport = {4, 6, 800, 600}; skyDraws = 0;
}
float smoothOracle(double sunY, double startDegrees, double endDegrees) {
    if (!std::isfinite(sunY)) return 0;
    const double radians = std::acos(-1.0) / 180.0;
    const double low = std::sin(startDegrees*radians), high = std::sin(endDegrees*radians);
    const double x = std::min(1.0, std::max(0.0, (sunY-low)/(high-low)));
    // Bernstein form is independent of production's expanded polynomial.
    return float(3.0*x*x*(1.0-x) + x*x*x);
}
void activationCases() {
    float previous = 0;
    for (int i = 0; i <= 2000; ++i) {
        const float y = -1.0f + float(i)/1000.0f;
        const float actual = ExperimentalAerialActivation(y);
        near(actual, smoothOracle(y, 15, 20), "15..20 degree analytical activation oracle");
        require(actual >= previous && actual >= 0 && actual <= 1, "activation is bounded monotone");
        previous = actual;
        near(ExperimentalSkyActivation(y), smoothOracle(y, 10, 15), "N activation contract remains 10..15 degrees");
    }
    for (float y : {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity(),
                    -std::numeric_limits<float>::infinity()}) {
        require(ExperimentalAerialActivation(y) == 0, "nonfinite S activation is zero");
        require(ExperimentalSkyActivation(y) == 0, "nonfinite N activation is zero");
    }
    require(ExperimentalAerialActivation(.25881905f) == 0, "S lower endpoint exact zero");
    require(ExperimentalAerialActivation(.34202014f) == 1, "S upper endpoint exact one");
    // Central finite differences around the endpoints approach zero on both
    // sides; this guards against a discontinuous step or linear fade.
    for (float edge : {.25881905f, .34202014f}) {
        const float h = .00001f;
        require(std::abs(ExperimentalAerialActivation(edge+h)-ExperimentalAerialActivation(edge-h))/(2*h) < .01f,
                "smooth zero-slope activation endpoints");
    }
}

void uploadCases() {
    EngineGL33 engine;
    engine._aerialControl[0] = .6f; engine._aerialControl[1] = .4f;
    engine._aerialControl[2] = .6928203f; engine._aerialControl[3] = .73f;
    RenderPassDescriptor base;
    struct Row { const char* name; RenderPassDescriptor descriptor; EngineGL33::PipelineVertexInput input; bool allowed; };
    std::vector<Row> rows;
    auto add = [&](const char* name, RenderPassDescriptor d, bool allowed,
                   EngineGL33::PipelineVertexInput input = EngineGL33::PipelineVertexInput::Mesh) {
        rows.push_back({name, d, input, allowed});
    };
    add("world", base, true);
    add("TL reset", base, false, EngineGL33::PipelineVertexInput::Screen);
    add("world after TL", base, true);
    add("implicit ActivePass is not explicit Mesh", base, false, EngineGL33::PipelineVertexInput::ActivePass);
    add("world after ActivePass", base, true);
    for (auto pass : {PassKind::CockpitOpaque, PassKind::CockpitCutout, PassKind::CockpitTransparent,
                     PassKind::ScreenSpace3D, PassKind::WorldShadow, PassKind::WorldLight}) {
        auto d = base; d.pass = pass; add("excluded pass reset", d, false); add("world resumes", base, true);
    }
    for (auto fog : {FogMode::Disabled, FogMode::AlphaFog}) {
        auto d = base; d.fog = fog; add("excluded fog reset", d, false); add("fog resumes", base, true);
    }
    for (auto shader : {ShaderFamily::Shadow, ShaderFamily::Water, ShaderFamily::Flat}) {
        auto d = base; d.shader = shader; add("excluded shader reset", d, false); add("shader resumes", base, true);
    }
    for (auto depth : {DepthMode::Disabled, DepthMode::Shadow}) {
        auto d = base; d.depth = depth; add("excluded depth reset", d, false); add("depth resumes", base, true);
    }
    for (auto blend : {BlendMode::Additive, BlendMode::Shadow}) {
        auto d = base; d.blend = blend; add("excluded blend reset", d, false); add("blend resumes", base, true);
    }
    for (auto pass : {PassKind::WorldOpaque, PassKind::WorldCutout, PassKind::WorldTransparent, PassKind::SurfaceOverlay, PassKind::WorldWater})
    for (auto shader : {ShaderFamily::Normal, ShaderFamily::Detail, ShaderFamily::Grass})
    for (auto depth : {DepthMode::Normal, DepthMode::ReadOnly})
    for (auto blend : {BlendMode::Opaque, BlendMode::AlphaBlend}) {
        auto d = base; d.pass = pass; d.shader = shader; d.depth = depth; d.blend = blend;
        add("all supported draw classes", d, true);
    }
    for (auto depth : {DepthMode::Normal, DepthMode::ReadOnly})
    for (auto blend : {BlendMode::Opaque, BlendMode::AlphaBlend}) {
        auto d = base; d.pass = PassKind::WorldWater; d.shader = ShaderFamily::Water;
        d.depth = depth; d.blend = blend; add("special water fog color", d, true);
    }
    unsigned rowIndex = 0;
    for (const auto& row : rows) {
        engine._pipelineVertexInput = row.input;
        engine.InvalidatePipelineCache(); // exercise actual body, not a cached no-op
        calls.clear(); uploads.clear();
        engine.ApplyPipeline(row.descriptor);
        const std::string label = std::string(row.name) + " row " + std::to_string(rowIndex++);
        require(uploads.count(30) == 1 && uploads.count(27) == 1 && uploads.size() == 2, label + " dedicated c30 upload");
        require(std::count(calls.begin(), calls.end(), "upload:30") == 1, label + " one c30 write");
        const std::array<float, 4> expected{.6f, .4f, .6928203f, row.allowed ? .73f : 0};
        require(uploads[30] == expected, label + " exact direction and explicit weight reset");
        require(engine._lastApplied.valid && engine._lastApplied.d == row.descriptor, label + " cache commit");
        calls.clear();
        engine.ApplyPipeline(row.descriptor);
        require(calls.empty(), label + " identical descriptor/context deduplicates");
    }
    // Explicitly exercise the cache key transitions, without invalidating it.
    engine._pipelineVertexInput = EngineGL33::PipelineVertexInput::Mesh;
    engine.InvalidatePipelineCache(); engine.ApplyPipeline(base);
    for (auto input : {EngineGL33::PipelineVertexInput::Screen, EngineGL33::PipelineVertexInput::Mesh,
                       EngineGL33::PipelineVertexInput::ActivePass, EngineGL33::PipelineVertexInput::Mesh}) {
        engine._pipelineVertexInput = input; calls.clear(); engine.ApplyPipeline(base);
        require(std::count(calls.begin(), calls.end(), "upload:30") == 1, "vertex input change cannot cache-hit");
        const float expected = input == EngineGL33::PipelineVertexInput::Mesh ? .73f : 0;
        require(uploads[30][3] == expected, "vertex input transition c30 reset");
    }
    auto noFog = base; noFog.fog = FogMode::Disabled;
    engine.ApplyPipeline(noFog); require(uploads[30][3] == 0, "fog-only descriptor change resets c30");
    engine.ApplyPipeline(base); require(uploads[30][3] == .73f, "fog-only descriptor restores c30");
    engine._aerialControl[3] = 0; engine.InvalidatePipelineCache(); engine.ApplyPipeline(base);
    require(uploads[30][3] == 0, "inactive frame weight explicitly uploads zero to eligible draw");
    engine._glContext = false; engine.InvalidatePipelineCache(); calls.clear(); engine.ApplyPipeline(base);
    require(calls.empty(), "no context pipeline is a no-op");
    require(rows.size() == 99, "upload transition cardinality");
}

struct SkyCase { const char* name; int variant; bool draw; float activation; };
void skyCases(bool skyEnabled, bool aerialEnabled) {
    const float nan = std::numeric_limits<float>::quiet_NaN(), inf = std::numeric_limits<float>::infinity();
    const SkyCase cases[] = {
        {"clear midday", 0, true, 1}, {"half clarity", 1, true, .5f},
        {"clarity clamps above one", 2, true, 1}, {"zero clarity", 3, false, 0},
        {"negative clarity", 4, false, 0}, {"NaN clarity", 5, false, 0}, {"infinite clarity", 6, false, 0},
        {"NVG", 7, false, 0}, {"night", 8, false, 0}, {"NaN night", 9, false, 0},
        {"negative night also excluded", 10, false, 0}, {"missing scene", 11, false, 0},
        {"missing sun", 12, false, 0}, {"missing camera", 13, false, 0},
        {"no context", 14, false, 0}, {"prior shader failure", 15, false, 0},
        {"below N sunrise", 16, false, 0}, {"partly active N", 17, true, 0},
        {"N fully active S lower endpoint", 18, true, 0}, {"S fade midpoint", 19, true, .5f},
        {"NaN sun elevation", 20, false, 0}, {"infinite sun X", 21, false, 0},
        {"invalid forward", 22, false, 0}, {"invalid right", 23, false, 0}, {"invalid up", 24, false, 0},
        {"invalid left extent", 25, false, 0}, {"invalid top extent", 26, false, 0},
        {"invalid sky color", 27, false, 0}, {"negative aureole", 28, false, 0},
        {"invalid fog", 29, false, 0}, {"foreign framebuffer", 30, false, 0},
        {"empty target", 31, false, 0}, {"invalid viewport", 32, false, 0},
        {"shader compile failure", 33, false, 0}, {"program link failure", 34, false, 0},
        {"VAO creation failure", 35, false, 0}, {"successful creation", 36, true, 1},
        {"S fade midpoint half clarity", 37, true, .25f}, {"infinite night", 38, false, 0},
    };
    for (const auto& c : cases) {
        resetRecording(); EngineGL33 engine; RecordingSun sun; RecordingCamera camera;
        RecordingScene scene{&sun, &camera}; GScene = &scene; float clarity = 1;
        engine._aerialControl[0] = 91; engine._aerialControl[1] = 92;
        engine._aerialControl[2] = 93; engine._aerialControl[3] = .73f;
        engine._lastApplied.valid = true;
        switch (c.variant) {
        case 0: break; case 1: clarity = .5f; break; case 2: clarity = 2; break;
        case 3: clarity = 0; break; case 4: clarity = -1; break; case 5: clarity = nan; break;
        case 6: clarity = inf; break; case 7: engine.nvg = true; break;
        case 8: sun.night = .01f; break; case 9: sun.night = nan; break; case 10: sun.night = -.01f; break;
        case 11: GScene = nullptr; break; case 12: scene.sun = nullptr; break; case 13: scene.camera = nullptr; break;
        case 14: engine._glContext = false; break; case 15: engine._experimentalSkyFailed = true; break;
        case 16: sun.direction.y = -.10f; break;
        case 17: sun.direction.y = -.216233615f; break;
        case 18: sun.direction.y = -.25881905f; break;
        case 19: sun.direction.y = -(.25881905f + .34202014f)*.5f; break;
        case 20: sun.direction.y = nan; break; case 21: sun.direction.x = inf; break;
        case 22: camera.forward.x = nan; break; case 23: camera.right.z = inf; break; case 24: camera.up.y = nan; break;
        case 25: camera.left = 0; break; case 26: camera.top = -1; break;
        case 27: sun.sky.r = nan; break; case 28: sun.aureole.g = -1; break; case 29: engine._fogColor.b = inf; break;
        case 30: boundFbo = 999; break; case 31: engine.width = 0; break;
        case 32: engine._aspectSettings.worldRight = 2; break;
        case 33: engine._experimentalSkyProgram = 0; shaderOkay = false; break;
        case 34: engine._experimentalSkyProgram = 0; programOkay = false; break;
        case 35: engine._experimentalSkyProgram = 0; vaoOkay = false; break;
        case 36: engine._experimentalSkyProgram = 0; break;
        case 37: sun.direction.y = -(.25881905f + .34202014f)*.5f; clarity = .5f; break;
        case 38: sun.night = inf; break;
        default: throw std::runtime_error("unhandled named sky case");
        }
        const bool expectedDraw = skyEnabled && c.draw;
        const float expectedWeight = skyEnabled && aerialEnabled ? c.activation : 0;
        const bool drawn = engine.DrawExperimentalSky(clarity);
        const std::string label = c.name;
        require(drawn == expectedDraw && skyDraws == unsigned(expectedDraw), label + " exact N success gate");
        near(engine._aerialControl[3], expectedWeight, label + " successful-sky S frame weight");
        require(!engine._lastApplied.valid, label + " previous S frame invalidates pipeline even on rejection");
        require(!calls.empty() && calls.front() == "invalidate-pipeline", label + " reset happens before any external work");
        if (drawn) {
            near(uniforms.at("aerialWeight"), expectedWeight, label + " horizon and geometry use same weight");
            near(uniforms.at("overlayWeight"), smoothOracle(-sun.direction.y, 10, 15), label + " N overlay weight");
            require(engine._aerialControl[0] == -sun.direction.x && engine._aerialControl[1] == -sun.direction.y &&
                    engine._aerialControl[2] == -sun.direction.z, label + " toward-sun direction committed after draw");
            const auto draw = std::find(calls.begin(), calls.end(), "draw-sky");
            const auto invalidate = std::find(draw, calls.end(), "invalidate-pipeline");
            require(invalidate != calls.end(), label + " successful sky invalidates pipeline");
        } else {
            require(uniforms.empty(), label + " rejected sky cannot upload active shader uniforms");
        }
        if (engine._glContext) {
            engine.ApplyPipeline(RenderPassDescriptor{});
            near(uploads.at(30)[3], expectedWeight, label + " following actual ApplyPipeline uploads c30");
            auto cockpit = RenderPassDescriptor{}; cockpit.pass = PassKind::CockpitOpaque;
            engine.ApplyPipeline(cockpit);
            require(uploads.at(30)[3] == 0, label + " following cockpit resets c30");
        }
    }
    // Same instance: successful frame -> failed/NVG attempt -> successful frame
    // -> frame boundary. These use actual invalidation and cache behavior.
    resetRecording(); EngineGL33 engine; RecordingSun sun; RecordingCamera camera;
    RecordingScene scene{&sun, &camera}; GScene = &scene;
    const RenderPassDescriptor world;
    engine.DrawExperimentalSky(1); engine.ApplyPipeline(world);
    const float expected = skyEnabled && aerialEnabled ? 1.0f : 0.0f;
    require(uploads.at(30)[3] == expected, "sequence successful sky weight");
    engine.nvg = true; engine.DrawExperimentalSky(1); engine.ApplyPipeline(world);
    require(uploads.at(30)[3] == 0, "sequence NVG resets cached world weight");
    engine.nvg = false; engine.DrawExperimentalSky(1); engine.ApplyPipeline(world);
    require(uploads.at(30)[3] == expected, "sequence success restores weight");
    engine.NextFrame(); engine.ApplyPipeline(world);
    require(uploads.at(30)[3] == 0 && engine._aerialControl[3] == 0, "NextFrame resets cached eligible world weight");
    require(std::find(calls.begin(), calls.end(), "base-next-frame") != calls.end(), "real NextFrame delegates to base");
    GScene = nullptr;
}
int main(int argc, char** argv) {
    try {
        if (argc == 1) {
            descriptorMatrix(); orthogonalFields(); buildCases(); activationCases(); uploadCases();
        } else {
            require(argc == 3, "expected independent sky/aerial parser oracle arguments");
            skyCases(std::stoi(argv[1]) != 0, std::stoi(argv[2]) != 0);
        }
        std::cout << checks << '\n'; return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
'''


def extract_sources(texts):
    signatures = [
        (SKY, r"GLuint\s+compileSky\([^)]*\)\s*\{"),
        (SKY, r"float\s+ExperimentalSkyActivation\([^)]*\)\s*\{"),
        (SKY, r"float\s+ExperimentalAerialActivation\([^)]*\)\s*\{"),
        (SKY, r"bool\s+finite3\([^)]*\)\s*\{"),
        (SKY, r"bool\s+EngineGL33::DrawExperimentalSky\([^)]*\)\s*\{"),
        (STATE, r"void\s+EngineGL33::ApplyPipeline\([^)]*\)\s*\{"),
        (LIFECYCLE, r"void\s+EngineGL33::NextFrame\(\s*\)\s*\{"),
    ]
    extracts = {signature: definition(texts[path], signature) for path, signature in signatures}
    # Preserve exact GL payloads too; recording compile calls do not execute GLSL.
    for name in ("skyVS", "skyFS"):
        matches = list(re.finditer(r'const char\* ' + name + r' = R"\(.*?\)";', texts[SKY], re.S))
        if len(matches) != 1:
            raise AssertionError(f"Expected one exact {name} literal")
        extracts[name] = matches[0].group()
    extracts["PipelineVertexInput"] = definition(texts[ENGINE], r"enum\s+class\s+PipelineVertexInput\s*\{") + ";"
    return extracts


def source_guards(texts, extracts):
    state = compact(next(value for key, value in extracts.items() if "ApplyPipeline" in key))
    sky = compact(next(value for key, value in extracts.items() if "DrawExperimentalSky" in key))
    frame = compact(next(value for key, value in extracts.items() if "NextFrame" in key))
    header = compact(texts[ENGINE])
    shaders = compact(texts[SHADERS])
    shadow = re.search(r"staticfloats_psShadow\[(\d+)\]=\{\};", shaders)
    upload = compact(definition(texts[SHADERS], r"void\s+EngineGL33::UploadPSConstant\([^)]*\)\s*\{"))
    flush = compact(definition(texts[SHADERS], r"void\s+EngineGL33::FlushPSConstants\(\s*\)\s*\{"))
    guards = {
        "production_classifier_included": "#include<Poseidon/Graphics/Rendering/AerialColorEligibility.hpp>" in compact(texts[STATE]),
        "explicit_mesh_not_implicit_active_pass": "IsAerialColorReceiver(d,vertexInput==PipelineVertexInput::Mesh)" in state,
        "dedicated_c30_before_cache_commit": state.index("UploadPSConstant(30,aerial);") < state.index("_lastApplied.d=d;"),
        "descriptor_and_vertex_input_in_cache_key": "vertexInput==_lastApplied.vertexInput&&d==_lastApplied.d" in state,
        "storage_defaults_zero": "_aerialControl[4]={}" in header,
        "production_cache_invalidation_matches_shell": "voidInvalidatePipelineCache(){_lastApplied.valid=false;}" in header,
        "c30_cpu_shadow_zero_initialized_and_large_enough": bool(shadow and int(shadow[1]) >= 124),
        "upload_slot_address_does_not_alias": "memcpy(s_psShadow+reg*4,data,16);" in upload,
        "whole_cpu_shadow_upload": "glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(s_psShadow),s_psShadow);" in flush,
        "attempt_resets_before_parsers_and_rejection": sky.index("_aerialControl[3]=0;") < sky.index("staticconstboolaerialEnabled="),
        "S_requires_fully_active_N_overlay": "aerialEnabled&&overlayWeight>=1.0f" in sky,
        "sky_success_commits_after_draw": sky.index("glDrawArrays(") < sky.index("_aerialControl[3]=aerialWeight;"),
        "frame_reset_before_base": frame.index("_aerialControl[3]=0;") < frame.index("base::NextFrame();"),
        "frame_reset_invalidates_cache": "if(_aerialControl[3]!=0)InvalidatePipelineCache();_aerialControl[3]=0;" in frame,
    }
    descriptor = code_mask(texts[HEADER_PATHS[1]])
    matrix = DESCRIPTOR_TEST[:DESCRIPTOR_TEST.index("void orthogonalFields")]
    for enum in ("PassKind", "DepthMode", "BlendMode", "FogMode", "ShaderFamily", "LightingMode", "SurfaceMode"):
        match = re.search(r"enum\s+class\s+" + enum + r"\s*:[^{]+\{([^}]+)\}", descriptor)
        if not match:
            raise AssertionError(f"Cannot inspect descriptor enum {enum}")
        members = {part.strip().split("=")[0].strip() for part in match[1].split(",") if part.strip()}
        covered = set(re.findall(r"\b" + enum + r"::(\w+)", matrix))
        guards[f"all_{enum}_values_enumerated"] = members == covered
    failed = [name for name, okay in guards.items() if not okay]
    if failed:
        raise AssertionError(f"Source wiring guards failed: {', '.join(failed)}")
    return guards


def run_checked(command, **kwargs):
    result = subprocess.run(command, capture_output=True, text=True, timeout=120, **kwargs)
    if result.returncode:
        raise RuntimeError(f"Command failed ({result.returncode}): {shlex.join(command)}\n{result.stdout}{result.stderr}")
    return result.stdout.strip()


def native_checks(root, compiler, extracts):
    sources = "\n".join(value for key, value in extracts.items() if key not in ("PipelineVertexInput", "skyVS", "skyFS"))
    probe = (PREAMBLE + DESCRIPTOR_TEST + RECORDING_SHELL + extracts["PipelineVertexInput"] +
             RECORDING_ENGINE_REST + extracts["skyVS"] + extracts["skyFS"] + sources + RUNTIME_TEST)
    with tempfile.TemporaryDirectory(prefix="cwr-aerial-eligibility-") as directory:
        work = Path(directory)
        source, executable = work / "probe.cpp", work / ("probe.exe" if os.name == "nt" else "probe")
        source.write_text(probe, encoding="utf-8")
        run_checked(shlex.split(compiler) + ["-std=c++17", "-Wall", "-Wextra", "-Werror", "-I", str(root),
                                            str(source), "-o", str(executable)])
        # Diagnostic modes are unrelated to S and must not contaminate the test.
        env = dict(os.environ)
        for key in ("CWR_TERRAIN_DIAGNOSTIC", "CWR_TERRAIN_RELIEF", "CWR_AERIAL_COLOR", "CWR_EXPERIMENTAL_SKY"):
            env.pop(key, None)
        contract_checks = int(run_checked([str(executable)], env=env))
        # Actual static parsers run in fresh processes. Only exact "1" enables
        # either feature; no prefix/whitespace/numeric coercion is permitted.
        values = [(None, False), ("", False), ("0", False), ("1", True), ("2", False),
                  ("01", False), ("10", False), ("-1", False), ("true", False),
                  ("1 ", False), (" 1", False), ("1\n", False)]
        runs = []
        for sky_value, sky_enabled in values:
            for aerial_value, aerial_enabled in values:
                process_env = dict(env)
                for key, value in (("CWR_EXPERIMENTAL_SKY", sky_value), ("CWR_AERIAL_COLOR", aerial_value)):
                    if value is not None:
                        process_env[key] = value
                checks = int(run_checked([str(executable), str(int(sky_enabled)), str(int(aerial_enabled))], env=process_env))
                runs.append({"sky_env": sky_value, "aerial_env": aerial_value, "checks": checks, "result": "PASS"})
    return {
        "contract_checks": contract_checks,
        "cartesian_receiver_rows": 50688,
        "allowed_receiver_tuples": 64,
        "allowed_rows_including_orthogonal_fields": 512,
        "named_material_cases": 31,
        "upload_transition_rows": 99,
        "activation_grid_samples": 2001,
        "named_sky_gate_cases_per_process": 39,
        "environment_processes": len(runs),
        "gate_checks": sum(run["checks"] for run in runs),
        "environment_results": runs,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--engine-root", type=Path, default=Path(__file__).resolve().parents[2] / "engine",
                        help="Source tree containing Poseidon and PoseidonGL33 (default: repository engine)")
    parser.add_argument("--compiler", default=os.environ.get("CXX", "c++"), help="C++17 GNU-style compiler command")
    parser.add_argument("--report", type=Path, help="Optional JSON evidence report")
    args = parser.parse_args()
    root = args.engine_root.resolve()
    missing = [name for name in HEADER_PATHS + SOURCE_PATHS if not (root / name).is_file()]
    if missing:
        parser.error("Incomplete engine snapshot; missing: " + ", ".join(missing))
    source_bytes = {name: (root / name).read_bytes() for name in HEADER_PATHS + SOURCE_PATHS}
    texts = {name: content.decode("utf-8") for name, content in source_bytes.items()}
    extracts = extract_sources(texts)
    guards = source_guards(texts, extracts)
    native = native_checks(root, args.compiler, extracts)
    changed = [name for name, content in source_bytes.items() if (root / name).read_bytes() != content]
    if changed:
        raise AssertionError(f"Source changed while testing; rerun a coherent snapshot: {changed}")
    report = {
        "result": "PASS", "scope": __doc__, "engine_root": str(root), "compiler": args.compiler,
        "source_guards": guards, "native": native,
        "source_sha256": {name: hashlib.sha256(content).hexdigest() for name, content in source_bytes.items()},
        "exact_extracted_source_sha256": {name: hashlib.sha256(body.encode()).hexdigest() for name, body in extracts.items()},
        "limits": (
            "CPU execution of exact headers and complete extracted production functions in a recording shell, "
            "not a full EngineGL33 instance. Scene, GL compilation/draw, pipeline helpers and upload effects "
            "are stubs. Source guards do not execute UBO storage or shader transport. No engine launch, "
            "GPU shader execution, real render queue, authored asset, screenshot, performance or visual "
            "acceptance evidence. Driver errors after submitted draws are outside this success contract."
        ),
    }
    output = json.dumps(report, indent=2) + "\n"
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(output, encoding="utf-8")
    print(output, end="")


if __name__ == "__main__":
    main()
