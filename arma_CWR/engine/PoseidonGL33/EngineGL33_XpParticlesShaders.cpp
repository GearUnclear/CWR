// Experiment xp-particles: the switches and the object program's main (the
// shader half; the engine half and the description are in
// EngineGL33_XpParticles.cpp). Kept apart from it so the unit tests, which
// link the shader sources but not the engine, can compile the mains.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>

#include <cstdlib>
#include <string>

namespace Poseidon::gl33lighting
{

int XpParticleFlags()
{
    static const int flags = []
    {
        auto on = [](const char* name)
        {
            const char* v = std::getenv(name);
            return v && v[0] == '1' && v[1] == '\0';
        };
        int f = 0;
        if (on("POSEIDON_XP_SOFT_PARTICLES"))
            f |= kXpSoftParticles;
        if (on("POSEIDON_XP_LIT_SMOKE"))
            f |= kXpLitSmoke;
        if (f)
            LOG_INFO(Graphics, "GL33: xp-particles on: soft particles {}, lit smoke {}",
                     (f & kXpSoftParticles) ? "on" : "off", (f & kXpLitSmoke) ? "on" : "off");
        return f;
    }();
    return flags;
}

namespace
{
// The pre-lit branch of the shipped object main, where the puff's colour is
// its texture times its vertex colour and before its glow is added.
constexpr const char* kXpAnchor =
    "        r0 *= constColor;\n        r0.rgb += vSpecColor.rgb;\n        // The sky clouds lit";
constexpr const char* kXpAnchorWith = "        r0 *= constColor;\n        xpParticle(r0);\n        r0.rgb += "
                                      "vSpecColor.rgb;\n        // The sky clouds lit";

// The puff's codes (EngineGL33::XpDecalCode): vUV1.y = 1000 + share * 1024
// + ox * 32 + oy, the share of its light from its dominant light in 31sts and
// that light's direction in camera space (x right, y up, z forward),
// octahedral in 32 x 32; vUV1.x = layer + radQ * 0.4 / 128, the soft fade's
// radius (m) as (radQ / 16)^2 (0: no fade). Every other draw leaves vUV1.y
// far below 999.5.
constexpr const char* kXpFunction = R"(
#if XP_SOFT
uniform sampler2D xpSceneDepth; // unit 17: the opaque world's depth (BeginTranslucentObjects)
#endif
// Lit smoke: the facing term's strength (the disc's mean kept).
const float kXpFacing = 0.7;
void xpParticle(inout vec4 r0) {
    float code = vUV1.y;
    if (code < 999.5) return;
    float c = floor(code - 1000.0 + 0.5);
    float shareQ = floor((c + 0.5) / 1024.0);
    float oct = c - shareQ * 1024.0;
    float ox = floor((oct + 0.5) / 32.0);
    float oy = oct - ox * 32.0;
    vec2 q = vUV0 * 2.0 - 1.0;
    float nz = sqrt(max(1.0 - dot(q, q), 0.0));
#if XP_LIT
    float share = shareQ / 31.0;
    if (share > 0.0) {
        vec2 f = vec2(ox, oy) / 31.0 * 2.0 - 1.0;
        vec3 L = vec3(f, 1.0 - abs(f.x) - abs(f.y));
        float t = max(-L.z, 0.0);
        L.x += L.x >= 0.0 ? -t : t;
        L.y += L.y >= 0.0 ? -t : t;
        L = normalize(L);
        // The puff as a ball facing the camera (v grows down the screen).
        vec3 n = normalize(vec3(q.x, -q.y, -nz - 1e-4));
        float D = max(1.0 + kXpFacing * (dot(n, L) + (2.0 / 3.0) * L.z), 0.25);
        // The vertex colour is display space: a linear scale of the shading.
        r0.rgb *= pow(mix(1.0, D, share), 1.0 / 2.2);
    }
#endif
#if XP_SOFT
    float layer = floor(vUV1.x + 0.5);
    float radQ = floor((vUV1.x - layer) * (128.0 / 0.4) + 0.5);
    if (radQ > 0.5) {
        float radius = (radQ / 16.0) * (radQ / 16.0);
        float d = texelFetch(xpSceneDepth, ivec2(gl_FragCoord.xy), 0).r;
        if (d < 0.999999) {
            // litPlanned: x proj _33, y proj _43, z depth scale, w depth bias.
            float sceneZ = litPlanned.y / ((d * litPlanned.z + litPlanned.w) - litPlanned.x);
            // The quad stands at the ball's near side (DrawDecal).
            float zc = 1.0 / gl_FragCoord.w + radius;
            float thick = max(radius * nz, 0.1 * radius);
            float fade = clamp((sceneZ - (zc - thick)) / thick, 0.0, 1.0);
            r0.a *= fade * fade * (3.0 - 2.0 * fade);
        }
    }
#endif
}
)";
} // namespace

std::string XpBuildNormalMain(const char* shipped, int flags)
{
    std::string main = shipped ? shipped : "";
    const size_t at = main.find(kXpAnchor);
    if (!flags || at == std::string::npos || main.find(kXpAnchor, at + 1) != std::string::npos)
        return std::string();
    main.replace(at, std::char_traits<char>::length(kXpAnchor), kXpAnchorWith);
    std::string head = "\n#define XP_SOFT ";
    head += (flags & kXpSoftParticles) ? "1" : "0";
    head += "\n#define XP_LIT ";
    head += (flags & kXpLitSmoke) ? "1" : "0";
    head += "\n";
    return head + kXpFunction + main;
}

const char* XpNormalMain(const char* shipped)
{
    static const std::string xp = [shipped]
    {
        const int f = XpParticleFlags();
        std::string built = XpBuildNormalMain(shipped, f);
        if (f && built.empty())
            LOG_ERROR(Graphics, "GL33: xp-particles: the object main's anchor moved; the shipped main stands");
        return built;
    }();
    return xp.empty() ? shipped : xp.c_str();
}

} // namespace Poseidon::gl33lighting
