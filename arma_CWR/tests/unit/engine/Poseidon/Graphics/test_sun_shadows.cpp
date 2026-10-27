#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/SunShadows.hpp>
#include <Poseidon/Graphics/Shadow/ShadowMath.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

// Sun shadows of the remastered lighting (Graphics/Remaster/SunShadows.hpp):
// the soft filter's blocker-to-penumbra conversion against the real cascade
// matrices, the frame constants the shader reads, the caster rule of
// treeShadows, the near tiers of softShadows, and the shader and shadow-pass
// contracts that keep both switches' "off" the look before the soft shadows.

using namespace Poseidon::remaster;
namespace sm = Poseidon::shadow;
namespace gl = Poseidon::gl33lighting;
using Catch::Approx;

namespace
{

std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    if (!f.is_open())
        return {};
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path EngineDir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine";
}

LightingTuning Apply(const std::string& setting)
{
    LightingTuning t{};
    bool modeSet = false;
    const std::string why = ApplyLightingSetting(t, setting, modeSet);
    INFO(setting << ": " << why);
    REQUIRE(why.empty());
    return t;
}

GateInputs SunAt(float elevationDeg)
{
    GateInputs in;
    in.sunElevationDeg = elevationDeg;
    in.moonElevationDeg = -30.0f;
    return in;
}

// The length of row `r` (xyz) of a light view-projection, as the shader reads
// it from the column-major uniform (cascadeVP[c][0][r], [1][r], [2][r]).
float RowLength(const sm::Mat4& m, int r)
{
    return std::sqrt(m.at(r, 0) * m.at(r, 0) + m.at(r, 1) * m.at(r, 1) + m.at(r, 2) * m.at(r, 2));
}

std::string Num(float v)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f", static_cast<double>(v));
    return buf;
}

} // namespace

TEST_CASE("Sun shadows: the soft filter's penumbra from a real cascade matrix", "[remaster][SunShadows]")
{
    // The first near tier as the shadow pass builds it: a camera-centred
    // sphere of 30.7 m at 2048 texels (3 cm per texel), the sun 20 degrees up
    // in the south-west.
    const float elev = 20.0f * 3.14159265f / 180.0f;
    const float az = 225.0f * 3.14159265f / 180.0f;
    const sm::Vec3 toSun{std::cos(elev) * std::sin(az), std::sin(elev), std::cos(elev) * std::cos(az)};
    const sm::Vec3 travel{-toSun.x, -toSun.y, -toSun.z};
    const int res = 2048;
    const float radius = 30.72f;
    const sm::Mat4 vp = sm::OmniSphereLightVP(sm::Vec3{6940.0f, 120.0f, 9964.0f}, radius, travel,
                                              sm::Vec3{0.0f, 1.0f, 0.0f}, res, 50.0f);

    // One texel in metres, the shader's way (2 ts / |row 0|): the sphere's
    // diameter plus the two snap texels over the resolution.
    const float ts = 1.0f / static_cast<float>(res);
    const float texelWorld = 2.0f * ts / RowLength(vp, 0);
    CHECK(texelWorld == Approx((2.0f * radius) / res * (1.0f + 2.0f / res)).epsilon(1e-4));

    // A blocker 10 m toward the sun from a receiver (camera-relative points):
    // their depth difference over the z row's length is the 10 m.
    const sm::Vec3 receiver{3.0f, -1.7f, 5.0f};
    const float d = 10.0f;
    const sm::Vec3 blocker{receiver.x + toSun.x * d, receiver.y + toSun.y * d, receiver.z + toSun.z * d};
    const float zR = sm::TransformPoint(vp, receiver).z;
    const float zB = sm::TransformPoint(vp, blocker).z;
    CHECK(zB < zR); // nearer the light
    const float depthPerMetre = RowLength(vp, 2);
    CHECK(PcssBlockerMetres(zR, zB, depthPerMetre) == Approx(d).epsilon(1e-3));
    // Both points fall on the same light-plane texel: the ray is the light's.
    const sm::Vec3 pr = sm::TransformPoint(vp, receiver);
    const sm::Vec3 pb = sm::TransformPoint(vp, blocker);
    CHECK(pr.x == Approx(pb.x).margin(1e-5));
    CHECK(pr.y == Approx(pb.y).margin(1e-5));
    // A blocker behind the receiver never counts.
    CHECK(PcssBlockerMetres(zB, zR, depthPerMetre) == 0.0f);

    // The 1 deg sun: tan(0.5 deg). The penumbra radius of the 10 m blocker is
    // 10 m * tan(0.5 deg) = 8.7 cm, about 3 texels of 3 cm.
    const float tanA = SunTanRadius(1.0f);
    CHECK(tanA == Approx(std::tan(0.5 * 3.14159265358979 / 180.0)).epsilon(1e-6));
    CHECK(SunTanRadius(0.0f) == 0.0f);
    CHECK(SunTanRadius(-1.0f) == 0.0f);
    const float searchT = PcssSearchTexels(tanA, texelWorld);
    CHECK(searchT == Approx(kPcssSearchMetres * tanA / texelWorld).epsilon(1e-5)); // 11.6 texels, inside the clamp
    const float penumbra = PcssPenumbraTexels(d, tanA, texelWorld, searchT);
    CHECK(penumbra == Approx(d * tanA / texelWorld).epsilon(1e-5));
    CHECK(penumbra * texelWorld == Approx(0.0873f).epsilon(1e-2));
    // At contact the filter keeps its smallest radius; far blockers stop at
    // the search radius; a coarse far tier searches at least 2 texels.
    CHECK(PcssPenumbraTexels(0.0f, tanA, texelWorld, searchT) == kPcssMinFilterTexels);
    CHECK(PcssPenumbraTexels(500.0f, tanA, texelWorld, searchT) == searchT);
    CHECK(PcssSearchTexels(tanA, 0.6f) == kPcssMinSearchTexels);
    CHECK(PcssSearchTexels(tanA, 0.001f) == kPcssMaxSearchTexels);
}

TEST_CASE("Sun shadows: taps by tier, width and TAA", "[remaster][SunShadows]")
{
    // Without TAA: 16 search taps on the omni tiers, 8 on the frustum tiers;
    // the filter 12, 24 or 32 by its radius. With TAA: 4 and 4, every frame.
    CHECK(PcssSearchTaps(0, 2, false) == 16);
    CHECK(PcssSearchTaps(1, 2, false) == 16);
    CHECK(PcssSearchTaps(2, 2, false) == 8);
    CHECK(PcssSearchTaps(3, 2, false) == 8);
    CHECK(PcssSearchTaps(0, 2, true) == 4);
    CHECK(PcssFilterTaps(0.6f, false) == 12);
    CHECK(PcssFilterTaps(2.99f, false) == 12);
    CHECK(PcssFilterTaps(3.0f, false) == 24);
    CHECK(PcssFilterTaps(10.0f, false) == 32);
    CHECK(PcssFilterTaps(30.0f, true) == 4);
    // The turn is the post pass's golden-ratio step.
    CHECK(PcssFrameTurn(0) == 0.0f);
    CHECK(PcssFrameTurn(1) == Approx(0.618034f).epsilon(1e-5));
    CHECK(PcssFrameTurn(2) == Approx(0.236068f).epsilon(1e-4));
    for (std::uint32_t n : {3u, 100u, 65535u, 4000000000u})
    {
        CHECK(PcssFrameTurn(n) >= 0.0f);
        CHECK(PcssFrameTurn(n) < 1.0f);
    }
}

TEST_CASE("Sun shadows: the frame constants follow the gates, the switches and TAA", "[remaster][SunShadows]")
{
    const LightingTuning t = Apply("remastered");
    const float tanA = SunTanRadius(1.0f);

    // High sun, no TAA: the soft filter whole, fixed taps; leaves lifted.
    SunShadowConstants c = ComputeSunShadowConstants(t, ComputeFeatureGates(SunAt(30.0f), t), false, 7);
    CHECK(c.taa[0] == 0.0f);
    CHECK(c.taa[1] == 0.0f);
    CHECK(c.taa[2] == static_cast<float>(kPcssTemporalTaps));
    CHECK(c.taa[3] == Approx(tanA));
    CHECK(t.shadowReach == kPcssSearchMetres); // the knob's default is the header's
    CHECK(c.pcss[0] == kPcssSearchMetres);
    CHECK(c.pcss[1] == kPcssMinFilterTexels);
    CHECK(c.pcss[2] == 1.0f);
    CHECK(t.leafBias > 0.0f);
    CHECK(c.pcss[3] == t.leafBias);

    // A TAA frame turns the pattern by the frame's golden-ratio step.
    c = ComputeSunShadowConstants(t, ComputeFeatureGates(SunAt(30.0f), t), true, 7);
    CHECK(c.taa[0] == Approx(PcssFrameTurn(7)));
    CHECK(c.taa[1] == 1.0f);

    // The soft filter fades in above the gate's 4 deg; below it both are off.
    CHECK(PcssStrength(4.0f) == 0.0f);
    CHECK(PcssStrength(5.0f) == Approx(0.5f));
    CHECK(PcssStrength(kPcssFullFromSunDeg) == 1.0f);
    c = ComputeSunShadowConstants(t, ComputeFeatureGates(SunAt(5.0f), t), false, 0);
    CHECK(c.taa[3] == Approx(tanA));
    CHECK(c.pcss[2] == Approx(0.5f));
    c = ComputeSunShadowConstants(t, ComputeFeatureGates(SunAt(3.0f), t), false, 0);
    CHECK(c.taa[3] == 0.0f);
    CHECK(c.pcss[2] == 0.0f);
    CHECK(c.pcss[3] == 0.0f);
    // Forced on (gates=0) the soft filter runs whole at any sun.
    c = ComputeSunShadowConstants(t, ComputeFeatureGates(SunAt(3.0f), Apply("remastered,gates=0")), false, 0);
    CHECK(c.taa[3] == Approx(tanA));
    CHECK(c.pcss[2] == 1.0f);

    // The switches: each zeroes its own part only, and both off is the stock
    // filter without a lift (the look before the soft shadows), as are baseline and classic.
    const GateInputs day = SunAt(30.0f);
    auto with = [&](const std::string& s)
    {
        const LightingTuning tt = Apply(s);
        return ComputeSunShadowConstants(tt, ComputeFeatureGates(day, tt), true, 5);
    };
    c = with("remastered,softShadows=0");
    CHECK(c.taa[3] == 0.0f);
    CHECK(c.taa[1] == 0.0f);
    CHECK(c.pcss[3] == t.leafBias);
    c = with("remastered,treeShadows=0");
    CHECK(c.taa[3] == Approx(tanA));
    CHECK(c.pcss[3] == 0.0f);
    for (const char* off :
         {"remastered,treeShadows=0,softShadows=0", "baseline", "classic", "remastered,sunSize=0,leafBias=0"})
    {
        INFO(off);
        c = with(off);
        for (float v : c.taa)
            CHECK(v == 0.0f);
        for (float v : c.pcss)
            CHECK(v == 0.0f);
    }
    // The sun size sets the penumbra per metre, the reach where it stops
    // growing (never below a metre: a reach of 0 would search nothing).
    CHECK(with("remastered,sunSize=2").taa[3] == Approx(SunTanRadius(2.0f)));
    CHECK(with("remastered,shadowReach=20").pcss[0] == 20.0f);
    CHECK(with("remastered,shadowReach=0").pcss[0] == kPcssMinReachMetres);
    CHECK(PcssSearchTexels(tanA, 0.0166f, 20.0f) == Approx(20.0f * tanA / 0.0166f));
    CHECK(PcssSearchTexels(tanA, 0.0166f, 0.0f) == kPcssMinSearchTexels);
}

TEST_CASE("Sun shadows: static objects cast with treeShadows, the rest keep their own say", "[remaster][SunShadows]")
{
    CHECK(CastsIntoSunShadowMap(true, false, false));
    CHECK(CastsIntoSunShadowMap(true, true, false));
    CHECK_FALSE(CastsIntoSunShadowMap(false, true, false)); // the legacy toggle off, the switch off: as before
    CHECK(CastsIntoSunShadowMap(false, true, true));        // trees and buildings, whatever the toggle
    CHECK_FALSE(CastsIntoSunShadowMap(false, false, true)); // a vehicle or man keeps the vehicle toggle

    // The shadow pass asks the rule with the treeShadows gate, never the
    // object alone, and only in Remastered.
    const std::string pass = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Scene" / "SceneShadowPass.cpp");
    REQUIRE_FALSE(pass.empty());
    CHECK(pass.find("remaster::CastsIntoSunShadowMap(oi->object->CastShadow(), oi->object->Static(), smTreeShadows)") !=
          std::string::npos);
    CHECK(pass.find("!oi->object->CastShadow())") == std::string::npos);
    CHECK(pass.find("smGates[remaster::Feature::TreeShadows].on") != std::string::npos);
}

TEST_CASE("Sun shadows: the soft filter runs on the stock near tiers", "[remaster][SunShadows]")
{
    // A tighter first tier (0.04 of the shadow range instead of 0.08) drew a
    // solid shadow over a Sinai camp's foreground with the stock filter as much
    // as with the soft one; the cause was not found, so the soft shadows keep
    // the tuned tiers whatever the switches say.
    const std::string pass = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Scene" / "SceneShadowPass.cpp");
    REQUIRE_FALSE(pass.empty());
    CHECK(pass.find("bp.omniCoef[0] = tune.omniCoef0;") != std::string::npos);
    CHECK(pass.find("bp.omniCoef[1] = tune.omniCoef1;") != std::string::npos);
    CHECK(pass.find("SoftShadows") == std::string::npos);
}

TEST_CASE("Sun shadows: the shader keeps the stock filter and mirrors the soft filter's rules",
          "[remaster][SunShadows][Shaders]")
{
    const std::string shadows = gl::RemasterFragmentPiece(gl::FragmentPiece::Shadows);
    const std::string shade = gl::RemasterFragmentPiece(gl::FragmentPiece::Shade);
    const std::string inputs = gl::RemasterFragmentPiece(gl::FragmentPiece::Inputs);
    REQUIRE_FALSE(shadows.empty());

    // The stock filter is the one the look before the soft shadows had: 12
    // comparisons on the Poisson disc, run alone while the soft strength is 0.
    CHECK(shadows.find("lit += texture(shadowMap, vec4(suv + kPoisson[k] * radius, float(c), zRef));") !=
          std::string::npos);
    CHECK(shadows.find("float soft = litTaa.w > 0.0 ? litPcss.z : 0.0;") != std::string::npos);
    CHECK(shadows.find("if (soft < 1.0) lit = pcfVisibility(suv, c, sc.z - bias, radius);") != std::string::npos);
    CHECK(shadows.find("if (leafLift > 0.0) at += litSunDir.xyz * (leafLift * texelWorld);") != std::string::npos);

    // The raw depth sampler on unit 6 feeds the search.
    CHECK(inputs.find("uniform sampler2DArray shadowDepth;") != std::string::npos);
    CHECK(shadows.find("texture(shadowDepth, vec3(suv, float(c))).r") != std::string::npos);

    // SunShadows.hpp's rules in the shader: the search clamp, the tap ladders
    // and the blocker-to-metres conversion through the z row.
    CHECK(shadows.find("clamp(litPcss.x * tanA / texelWorld, " + Num(kPcssMinSearchTexels) + ", " +
                       Num(kPcssMaxSearchTexels) + ")") != std::string::npos);
    CHECK(shadows.find("int nSearch = temporal ? int(litTaa.z) : (c < int(cascadeCtl.w) ? " +
                       std::to_string(kPcssSearchTapsNear) + " : " + std::to_string(kPcssSearchTapsFar) + ");") !=
          std::string::npos);
    CHECK(shadows.find("int nTaps = temporal ? int(litTaa.z) : (rT < " + Num(kPcssTaps24FromTexels) + " ? 12 : (rT < " +
                       Num(kPcssTaps32FromTexels) + " ? 24 : 32));") != std::string::npos);
    CHECK(
        shadows.find("float depthPerM = max(length(vec3(cascadeVP[c][0][2], cascadeVP[c][1][2], cascadeVP[c][2][2])), "
                     "1e-9);") != std::string::npos);
    CHECK(shadows.find("float dB = max(zRef - bSum / bN, 0.0) / depthPerM;") != std::string::npos);
    CHECK(shadows.find("float rT = clamp(dB * tanA / texelWorld, litPcss.y, searchT);") != std::string::npos);
    CHECK(shadows.find("float turn = 6.2831853 * (shadowNoise(gl_FragCoord.xy) + litTaa.x);") != std::string::npos);

    // Leaves lift, grass (shaded as terrain) does not.
    CHECK(shade.find("float leafLift = (foliage && !terrain) ? litPcss.w : 0.0;") != std::string::npos);
    CHECK(shade.find("shadowMapVisibility(nCast, nlCast, terrain ? 0.0 : 1.0, leafLift)") != std::string::npos);
}
