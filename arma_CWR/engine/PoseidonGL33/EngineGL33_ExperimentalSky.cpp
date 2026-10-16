#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLPipelineState.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <glad/gl.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <chrono>

namespace
{
const char* skyVS = R"(#version 330 core
out vec2 ndc;
void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);ndc=p*2.0-1.0;gl_Position=vec4(ndc,1,1);}
)";
const char* skyFS = R"(#version 330 core
in vec2 ndc;
uniform vec3 forwardRay,rightRay,upRay,sunRay,horizonColor,skyColor,sunColor;
uniform float clearSky, overlayWeight, aerialWeight;
out vec4 fragColor;
// S: change only scattering color; preserve the authored fog transmittance.
vec3 directionalAerialColor(vec3 base, vec3 ray, vec3 towardSun, float weight) {
    if (weight <= 0.0) return base;
    if (any(isnan(base)) || any(isinf(base)) || any(lessThan(base,vec3(0.0))) ||
        any(isnan(ray)) || any(isinf(ray)) || any(isnan(towardSun)) || any(isinf(towardSun))) return base;
    float rayLength2=dot(ray,ray), sunLength2=dot(towardSun,towardSun);
    if (rayLength2<=1e-12 || sunLength2<=1e-12) return base;
    vec3 luma=vec3(.2126,.7152,.0722);
    float originalLuma=dot(base,luma);
    if (originalLuma<=1e-8) return base;
    vec3 cool=base*vec3(.80,.96,1.14), warm=base*vec3(1.12,1.02,.86);
    cool*=originalLuma/max(dot(cool,luma),1e-8);
    warm*=originalLuma/max(dot(warm,luma),1e-8);
    float mu=max(dot(ray*inversesqrt(rayLength2),towardSun*inversesqrt(sunLength2)),0.0);
    float lobe=mu*mu; lobe*=lobe;
    return mix(base,mix(cool,warm,lobe),clamp(weight,0.0,1.0));
}

void main(){
 vec3 ray=normalize(forwardRay+ndc.x*rightRay+ndc.y*upRay);
 if(ray.y<0.0) discard;
 // Authored SkyColor is a near-white texture modulation, not the dome's
 // blue chromaticity. An explicit restrained scattering palette supplies
 // that missing texture hue; input color still controls tint and brightness.
 // These are artistic coefficients, not calibrated atmospheric radiance.
 float luminance=dot(skyColor,vec3(.2126,.7152,.0722));
 vec3 tint=skyColor/max(max(skyColor.r,skyColor.g),max(skyColor.b,1e-6));
 float daylight=luminance/(luminance+.5);
 vec3 zenith=tint*vec3(.38,.62,1.0)*daylight;
 vec3 haze=tint*vec3(.75,.85,1.0)*(luminance/(luminance+.25));
 float altitude=smoothstep(.04,.70,ray.y);
 float mu=max(dot(ray,sunRay),0.0);
 vec3 sunTint=sunColor/max(max(sunColor.r,sunColor.g),max(sunColor.b,1e-6));
 vec3 directional=mix(haze,zenith,altitude)+sunTint*(.08*daylight*pow(mu,8.0));
 // Exact true-horizon fog match, with zero first derivative and a smooth
 // two-degree transition, rather than a broad five-degree gray lock.
 float horizonBlend=smoothstep(0.0,.035,ray.y)*clearSky;
 vec3 horizon=directionalAerialColor(horizonColor,ray,sunRay,aerialWeight);
 vec3 color=mix(horizon,directional,horizonBlend);
 fragColor=vec4(clamp(color,0.0,1.0),overlayWeight);
}
)";
GLuint compileSky(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "N sky shader failed: {}", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
float ExperimentalSkyActivation(float sunY)
{
    if (!std::isfinite(sunY)) return 0.0f;
    const float t = std::clamp((sunY - .17364818f) / (.25881905f - .17364818f), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float ExperimentalAerialActivation(float sunY)
{
    if (!std::isfinite(sunY)) return 0.0f;
    const float t = std::clamp((sunY - .25881905f) / (.34202014f - .25881905f), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

bool finite3(Vector3Par v)
{
    return std::isfinite(v.X()) && std::isfinite(v.Y()) && std::isfinite(v.Z());
}
} // namespace

bool EngineGL33::DrawExperimentalSky(float skyThrough)
{
    // A failed/unsupported sky attempt cannot leave an earlier frame's atmosphere active.
    if (_aerialControl[3] != 0) InvalidatePipelineCache();
    _aerialControl[3] = 0;
    static const bool aerialEnabled = [] {
        const char* v = std::getenv("CWR_AERIAL_COLOR");
        return v && v[0] == '1' && v[1] == '\0';
    }();
    static const bool enabled = []
    {
        const char* v = std::getenv("CWR_EXPERIMENTAL_SKY");
        return v && v[0] == '1' && v[1] == '\0';
    }();
    if (!enabled || !_glContext || _experimentalSkyFailed || !GScene || GetNightVision())
        return false;
    const auto* sun = GScene->MainLight();
    const auto* camera = GScene->GetCamera();
    if (!sun || !camera || !std::isfinite(sun->NightEffect()) || sun->NightEffect() != 0 ||
        !std::isfinite(skyThrough) || skyThrough <= 0)
        return false;
    // Fade in over 10..15 degrees in either time direction. The authored dome
    // is already queued underneath; weight zero leaves it untouched without GL.
    const float sunY = -sun->SunDirection().Y();
    if (!std::isfinite(sunY)) return false;
    const float overlayWeight = ExperimentalSkyActivation(sunY);
    if (overlayWeight <= 0.0f) return false;
    // S starts only once N3 is fully opaque, avoiding an authored-underlay horizon mismatch.
    const float aerialWeight = aerialEnabled && overlayWeight >= 1.0f
        ? ExperimentalAerialActivation(sunY) * std::clamp(skyThrough,0.0f,1.0f) : 0.0f;
    const Vector3 forward = camera->Direction(), right = camera->DirectionAside() * camera->Left(),
                  up = camera->DirectionUp() * camera->Top();
    if (!finite3(forward) || !finite3(right) || !finite3(up) || !finite3(sun->SunDirection()) || camera->Left() <= 0 ||
        camera->Top() <= 0)
        return false;
    const Color sky = sun->SkyColor() * GetAccomodateEye(), aureole = sun->SunSkyColor() * GetAccomodateEye();
    static unsigned inputLogs = 0;
    static auto lastInputLog = std::chrono::steady_clock::time_point{};
    const auto now = std::chrono::steady_clock::now();
    if (inputLogs < 24 && now - lastInputLog > std::chrono::seconds(2))
    {
        const Color accommodation = GetAccomodateEye();
        LOG_INFO(Graphics,
                 "N sky inputs rawSky=({},{},{}), rawSunSky=({},{},{}), accommodation=({},{},{}), fog=({},{},{}), "
                 "skyThrough={}, sunY={}",
                 sun->SkyColor().R(), sun->SkyColor().G(), sun->SkyColor().B(), sun->SunSkyColor().R(),
                 sun->SunSkyColor().G(), sun->SunSkyColor().B(), accommodation.R(), accommodation.G(),
                 accommodation.B(), _fogColor.R(), _fogColor.G(), _fogColor.B(), skyThrough, -sun->SunDirection().Y());
        lastInputLog = now;
        ++inputLogs;
    }
    const float values[] = {sky.R(),     sky.G(),       sky.B(),       aureole.R(),  aureole.G(),
                            aureole.B(), _fogColor.R(), _fogColor.G(), _fogColor.B()};
    for (float x : values)
        if (!std::isfinite(x) || x < 0)
            return false;
    FlushQueues();
    GLint drawFbo = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
    if (drawFbo != static_cast<GLint>(_ssaaFbo))
        return false; // no portrait/foreign targets
    int width, height;
    RenderTargetSize(width, height);
    const auto& a = _aspectSettings;
    const int x0 = int(a.worldLeft * width + .5f), x1 = int(a.worldRight * width + .5f);
    const int y0 = int(a.worldTop * height + .5f), y1 = int(a.worldBottom * height + .5f);
    if (width <= 0 || height <= 0 || x0 < 0 || y0 < 0 || x1 > width || y1 > height || x1 <= x0 || y1 <= y0)
        return false;
    if (!_experimentalSkyProgram)
    {
        GLuint vs = compileSky(GL_VERTEX_SHADER, skyVS), fs = compileSky(GL_FRAGMENT_SHADER, skyFS);
        GLuint program = 0;
        if (vs && fs)
        {
            program = glCreateProgram();
            glAttachShader(program, vs);
            glAttachShader(program, fs);
            glLinkProgram(program);
            GLint ok = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &ok);
            if (!ok)
            {
                glDeleteProgram(program);
                program = 0;
            }
        }
        if (vs)
            glDeleteShader(vs);
        if (fs)
            glDeleteShader(fs);
        if (!program)
        {
            _experimentalSkyFailed = true;
            return false;
        }
        _experimentalSkyProgram = program;
        glGenVertexArrays(1, &_experimentalSkyVao);
        if (!_experimentalSkyVao)
        {
            glDeleteProgram(program);
            _experimentalSkyProgram = 0;
            _experimentalSkyFailed = true;
            return false;
        }
    }
    GLint oldProgram = 0, oldVao = 0, viewport[4], polygon[2];
    GLboolean depthMask, colorMask[4];
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_POLYGON_MODE, polygon);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);
    const GLenum caps[] = {GL_CULL_FACE, GL_SCISSOR_TEST, GL_SAMPLE_ALPHA_TO_COVERAGE,
                           GL_COLOR_LOGIC_OP, GL_RASTERIZER_DISCARD};
    GLboolean states[5];
    const GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST), stencilEnabled = glIsEnabled(GL_STENCIL_TEST);
    for (int i = 0; i < 5; ++i)
    {
        states[i] = glIsEnabled(caps[i]);
        glDisable(caps[i]);
    }
    GLint clipCount = 0;
    glGetIntegerv(GL_MAX_CLIP_DISTANCES, &clipCount);
    clipCount = std::min(clipCount, 8);
    GLboolean clip[8];
    for (int i = 0; i < clipCount; ++i)
    {
        clip[i] = glIsEnabled(GL_CLIP_DISTANCE0 + i);
        glDisable(GL_CLIP_DISTANCE0 + i);
    }
    glViewport(x0, height - y1, x1 - x0, y1 - y0);
    Poseidon::render::depthstencil::ScopedBackground backgroundState;
    Poseidon::render::pipeline::ScopedRgbOnly colorWriteState;
    Poseidon::render::blend::ScopedAlphaOverlay blendState;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(_experimentalSkyProgram);
    glBindVertexArray(_experimentalSkyVao);
    const auto uniform3 = [&](const char* name, float x, float y, float z)
    { glUniform3f(glGetUniformLocation(_experimentalSkyProgram, name), x, y, z); };
    uniform3("forwardRay", forward.X(), forward.Y(), forward.Z());
    uniform3("rightRay", right.X(), right.Y(), right.Z());
    uniform3("upRay", up.X(), up.Y(), up.Z());
    uniform3("sunRay", -sun->SunDirection().X(), -sun->SunDirection().Y(), -sun->SunDirection().Z());
    uniform3("horizonColor", _fogColor.R(), _fogColor.G(), _fogColor.B());
    uniform3("skyColor", sky.R(), sky.G(), sky.B());
    uniform3("sunColor", aureole.R(), aureole.G(), aureole.B());
    glUniform1f(glGetUniformLocation(_experimentalSkyProgram, "clearSky"), std::clamp(skyThrough, 0.0f, 1.0f));
    glUniform1f(glGetUniformLocation(_experimentalSkyProgram, "overlayWeight"), overlayWeight);
    glUniform1f(glGetUniformLocation(_experimentalSkyProgram, "aerialWeight"), aerialWeight);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    _aerialControl[0] = -sun->SunDirection().X();
    _aerialControl[1] = -sun->SunDirection().Y();
    _aerialControl[2] = -sun->SunDirection().Z();
    _aerialControl[3] = aerialWeight;
    static bool aerialLogged = false;
    if (aerialWeight > 0 && !aerialLogged) {
        LOG_INFO(Graphics,"S aerial color: weight={}, sunY={}, skyThrough={}, distanceFade=120..350m, activation=15..20deg, luminance-normalized cool/warm; transmittance unchanged",aerialWeight,sunY,skyThrough);
        aerialLogged = true;
    }
    glUseProgram(oldProgram);
    glBindVertexArray(oldVao);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    backgroundState.Restore();
    colorWriteState.Restore();
    blendState.Restore();
    glPolygonMode(GL_FRONT_AND_BACK, polygon[0]);
    for (int i = 0; i < 5; ++i)
    {
        if (states[i])
            glEnable(caps[i]);
        else
            glDisable(caps[i]);
    }
    for (int i = 0; i < clipCount; ++i)
    {
        if (clip[i])
            glEnable(GL_CLIP_DISTANCE0 + i);
        else
            glDisable(GL_CLIP_DISTANCE0 + i);
    }
    _pixelShaderSel = PSNone;
    GL33Bind::Invalidate();
    InvalidatePipelineCache();
    static bool logged = false;
    if (!logged)
    {
        GLint program = 0, vao = 0, fb = 0, vp[4], poly[2];
        GLboolean dm, cm[4];
        glGetIntegerv(GL_CURRENT_PROGRAM, &program);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fb);
        glGetIntegerv(GL_VIEWPORT, vp);
        glGetIntegerv(GL_POLYGON_MODE, poly);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &dm);
        glGetBooleanv(GL_COLOR_WRITEMASK, cm);
        bool restored = blendState.MatchesSavedState() && glIsEnabled(GL_DEPTH_TEST) == depthEnabled && glIsEnabled(GL_STENCIL_TEST) == stencilEnabled &&
                        program == oldProgram && vao == oldVao && fb == drawFbo && dm == depthMask &&
                        poly[0] == polygon[0] && poly[1] == polygon[1];
        for (int i = 0; i < 4; ++i)
            restored = restored && vp[i] == viewport[i] && cm[i] == colorMask[i];
        for (int i = 0; i < 5; ++i)
            restored = restored && glIsEnabled(caps[i]) == states[i];
        for (int i = 0; i < clipCount; ++i)
            restored = restored && glIsEnabled(GL_CLIP_DISTANCE0 + i) == clip[i];
        LOG_INFO(Graphics, "N sky first-draw touched-state restoration verified={}", restored);
    }
    if (!logged)
    {
        LOG_INFO(Graphics, "N sky active: target={}, viewport={}x{}, sunElevationY={}, clearSky={}, overlayWeight={}, exact analytic fog horizon",
                 drawFbo, x1 - x0, y1 - y0, -sun->SunDirection().Y(), skyThrough, overlayWeight);
        logged = true;
    }
    return true;
}
