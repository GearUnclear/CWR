#include <catch2/catch_test_macros.hpp>

#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <Poseidon/Graphics/Remaster/SeaFlow.hpp>
#include <Poseidon/Graphics/Remaster/SeaMirror.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cmath>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>

// XP sea mirror (perf squad lane EX5-SEA, plan R-2; Graphics/Remaster/SeaMirror.hpp,
// EngineGL33_XpSeaMirror.cpp): the switch's parsing, and the two shader
// variants it builds from the stock sources.

namespace gl = Poseidon::gl33lighting;
namespace rm = Poseidon::remaster;

namespace
{

bool CompilesFragment(const std::string& src, std::string& log)
{
    glslang::TShader shader(EShLangFragment);
    const char* strings[1] = {src.c_str()};
    shader.setStrings(strings, 1);
    const bool ok = shader.parse(GetDefaultResources(), 330, false, EShMsgDefault);
    log = shader.getInfoLog();
    return ok;
}

struct GlslangInit
{
    GlslangInit() { glslang::InitializeProcess(); }
    ~GlslangInit() { glslang::FinalizeProcess(); }
};

size_t CountOf(const std::string& s, const std::string& what)
{
    size_t n = 0;
    for (size_t at = s.find(what); at != std::string::npos; at = s.find(what, at + what.size()))
        ++n;
    return n;
}

} // namespace

TEST_CASE("XP sea mirror: the switch's value", "[Graphics][SeaMirrorXp]")
{
    CHECK(rm::ParseSeaMirrorXp(nullptr) == 0.0f);
    CHECK(rm::ParseSeaMirrorXp("") == 0.0f);
    CHECK(rm::ParseSeaMirrorXp("0") == 0.0f);
    CHECK(rm::ParseSeaMirrorXp("-1") == 0.0f);
    CHECK(rm::ParseSeaMirrorXp("on") == 0.0f);
    CHECK(rm::ParseSeaMirrorXp("1") == 1.0f);
    CHECK(rm::ParseSeaMirrorXp("0.5") == 0.5f);
    CHECK(rm::ParseSeaMirrorXp("9") == 4.0f);
    CHECK(rm::ParseSeaMirrorWeight(nullptr) == 1.0f);
    CHECK(rm::ParseSeaMirrorWeight("") == 1.0f);
    CHECK(rm::ParseSeaMirrorWeight("x") == 1.0f);
    CHECK(rm::ParseSeaMirrorWeight("0") == 0.0f);
    CHECK(rm::ParseSeaMirrorWeight("0.5") == 0.5f);
    CHECK(rm::ParseSeaMirrorWeight("3") == 1.0f);
    CHECK(rm::ParseSeaMirrorWeight("-2") == 0.0f);
}

TEST_CASE("XP sea mirror: off keeps the stock water main", "[Graphics][SeaMirrorXp]")
{
    // The test process runs without POSEIDON_XP_SEAMIRROR and POSEIDON_XP_SEAFLOW,
    // so the flow is on (its shipped default) and the mirror off.
    if (rm::SeaMirrorXpOn())
        return; // a switch is set in this environment
    if (rm::SeaFlowXpOn())
    {
        // The flow's main replaces the stock one and writes no slope.
        REQUIRE(gl::SeaWaterMainXp() != nullptr);
        CHECK(std::string(gl::RemasterFragmentMain(3)) == gl::SeaWaterMainXp());
        CHECK(std::string(gl::RemasterFragmentMain(3)).find("fragWave") == std::string::npos);
        return;
    }
    CHECK(gl::SeaWaterMainXp() == nullptr);
    CHECK(std::string(gl::RemasterFragmentMain(3)) == gl::RemasterWaterMainStock());
    CHECK(std::string(gl::RemasterFragmentMain(3)).find("fragWave") == std::string::npos);
}

TEST_CASE("XP sea mirror: the water main writes its slope and compiles", "[Graphics][SeaMirrorXp]")
{
    const char* xp = gl::SeaMirrorWaterMainXp();
    REQUIRE(xp != nullptr);
    const std::string main = xp;
    const std::string stock = gl::RemasterWaterMainStock();
    CHECK(CountOf(main, "layout(location = 2) out vec4 fragWave;") == 1);
    CHECK(CountOf(main, "fragWave = vec4(0.0, 0.0, 0.0, 1.0);") == 1);
    // The slope that built n, after the shore's calming, before the surface target.
    const size_t calm = main.find("if ((xpParts & 1) != 0) grad *= mix(kXpCalm");
    const size_t normal = main.find("vec3 n = normalize(vec3(-grad.x, 1.0, -grad.y));");
    const size_t write = main.find("fragWave = vec4(grad, 0.0, 1.0);");
    const size_t surf = main.find("g_surfShare = ambientShare * (1.0 - wSky);");
    REQUIRE(calm != std::string::npos);
    REQUIRE(write != std::string::npos);
    CHECK(calm < normal);
    CHECK(normal < write);
    CHECK(write < surf);
    // Nothing else changes: the stock text with the added lines taken out.
    std::string back = main;
    for (const char* added :
         {"// XP sea mirror: the wave slope (the gradient that built the wave normal) for the\n"
          "// reflections' trace; a sea-only target, in the draw buffers only while the sea draws.\n"
          "layout(location = 2) out vec4 fragWave;\n",
          "    fragWave = vec4(0.0, 0.0, 0.0, 1.0);\n",
          "        // XP sea mirror: the slope the trace moves the mirror image by.\n"
          "        fragWave = vec4(grad, 0.0, 1.0);\n"})
    {
        const size_t at = back.find(added);
        REQUIRE(at != std::string::npos);
        back.erase(at, std::string(added).size());
    }
    CHECK(back == stock);

    GlslangInit init;
    std::string log;
    const bool ok = CompilesFragment(std::string(gl::RemasterFragmentCommon()) + main, log);
    CAPTURE(log);
    CHECK(ok);
}

TEST_CASE("XP sea mirror: the trace reads the sea's slope and compiles", "[Graphics][SeaMirrorXp]")
{
    const char* xp = gl::SeaMirrorTraceSourceXp();
    REQUIRE(xp != nullptr);
    const std::string trace = xp;
    CHECK(CountOf(trace, "uniform sampler2DMS seaWave;") == 1);
    CHECK(CountOf(trace, "uniform vec4 ssrSea;") == 1);
    CHECK(CountOf(trace, "vec3 seaWaveNormal(ivec2 ij, ivec2 lim, vec3 vW, float dist, out float lostVar, out "
                         "float meanW) {") == 1);
    CHECK(CountOf(trace, "texelFetch(seaWave, q, 0)") == 1);
    // The ripples remain the fallback; the sea's normal comes first.
    CHECK(CountOf(trace, "vec3 nWave = ssrSea.x > 0.5 ? seaWaveNormal(ij, lim, vW, dist, lostVar, seaW)") == 1);
    CHECK(CountOf(trace, ": rippleNormal(posW.xz, ssrParams.z, foot, lostVar);") == 1);
    // Each pixel's sky weight as the sea program has it, Fresnel F0 0.02.
    CHECK(CountOf(trace, "float F = 0.02 + 0.98 * pow(clamp(1.0 - max(dot(nk, vW), 0.0), 0.0, 1.0), 5.0);") == 1);
    CHECK(CountOf(trace, "float w = clamp(skyMean * Ff + (F - Ff), 0.0, 0.95);") == 1);
    CHECK(CountOf(trace, "if (masking) w *= mix(0.6, 1.0, smoothstep(-0.08, 0.0, reflect(-vW, nk).y));") == 1);
    CHECK(CountOf(trace, "wS = mix(wS, seaW, ssrSea.z);") == 1);
    // A wave facet whose flat mirror ray met nothing still mirrors what stands
    // on screen in its own direction (never the sky or the sea).
    CHECK(CountOf(trace, "vec2 uvD = dV.z > 0.02 ? uvOfView(dV) : vec2(-1.0);") == 1);
    CHECK(CountOf(trace, "if (inFrame(uvP) && (tp & 0xC0) != 0x80 && zP < kSsrSkyDepth && zP > z * 0.98) {") == 1);
    CHECK(CountOf(trace, "float share = clamp(wS * dot(S, kLuma) / max(seaLum, 1e-4), 0.0, 1.0);") == 1);
    // The water program weighs its sky the same way.
    const std::string water = gl::RemasterWaterMainStock();
    CHECK(water.find("float F = 0.02 + 0.98 * pow(clamp(1.0 - nv, 0.0, 1.0), 5.0);") != std::string::npos);
    CHECK(water.find("float wSky = clamp(skyMean * Ff + kSeaSkyVar * (F - Ff), 0.0, 0.95);") != std::string::npos);
    CHECK(water.find("const float kSeaSkyVar = 1.0;") != std::string::npos);
    CHECK(
        water.find("if ((xpParts & 8) != 0) wSky *= mix(kXpMaskFloor, 1.0, smoothstep(-kXpMaskSpan, 0.0, xpRawY));") !=
        std::string::npos);
    CHECK(water.find("const float kXpMaskFloor = 0.6;") != std::string::npos);
    CHECK(water.find("const float kXpMaskSpan = 0.08;") != std::string::npos);
    // The stock trace is untouched.
    const std::string stock = gl::PostShaderSource(gl::PostShader::SsrTrace);
    CHECK(stock.find("seaWave") == std::string::npos);

    GlslangInit init;
    std::string log;
    const bool ok = CompilesFragment(trace, log);
    CAPTURE(log);
    CHECK(ok);
}

// XP sea flow (rounds 2 and 3; Graphics/Remaster/SeaFlow.hpp): the sea's
// animation no longer plays forward and backward, and its layers neither loop
// nor flow one way.

TEST_CASE("XP sea flow: the switches' values", "[Graphics][SeaFlowXp]")
{
    CHECK(rm::ParseSeaFlowXp(nullptr) == 0.0f);
    CHECK(rm::ParseSeaFlowXp("") == 0.0f);
    CHECK(rm::ParseSeaFlowXp("0") == 0.0f);
    CHECK(rm::ParseSeaFlowXp("-2") == 0.0f);
    CHECK(rm::ParseSeaFlowXp("on") == 0.0f);
    CHECK(rm::ParseSeaFlowXp("1") == 1.0f);
    CHECK(rm::ParseSeaFlowXp("1.5") == 1.5f);
    CHECK(rm::ParseSeaFlowXp("40") == 4.0f);
    // Unset, the flow is on (the shipped look); 0 turns it off.
    CHECK(rm::ParseSeaFlowXpEnv(nullptr) == 1.0f);
    CHECK(rm::ParseSeaFlowXpEnv("") == 1.0f);
    CHECK(rm::ParseSeaFlowXpEnv("0") == 0.0f);
    CHECK(rm::ParseSeaFlowXpEnv("2") == 2.0f);
    CHECK(rm::ParseSeaFlowField(nullptr) == rm::SeaFlowField::Sea);
    CHECK(rm::ParseSeaFlowField("sea") == rm::SeaFlowField::Sea);
    CHECK(rm::ParseSeaFlowField("x") == rm::SeaFlowField::Sea);
    CHECK(rm::ParseSeaFlowField("river") == rm::SeaFlowField::River);
    const rm::SeaFlowField sea = rm::SeaFlowField::Sea, river = rm::SeaFlowField::River;
    CHECK(rm::kSeaFlowSeaMorphSec == 0.0f); // the dissolve is a switch, not a default (its cost)
    CHECK(rm::ParseSeaFlowMorph(nullptr, sea) == rm::kSeaFlowSeaMorphSec);
    CHECK(rm::ParseSeaFlowMorph("0.8", sea) == 0.8f);
    CHECK(rm::ParseSeaFlowMorph("x", sea) == rm::kSeaFlowSeaMorphSec);
    CHECK(rm::ParseSeaFlowMorph(nullptr, river) == 0.0f);
    CHECK(rm::ParseSeaFlowMorph("0", sea) == 0.0f);
    CHECK(rm::ParseSeaFlowMorph("0.1", sea) == 0.0f);
    CHECK(rm::ParseSeaFlowMorph("6", river) == 6.0f);
    CHECK(rm::ParseSeaFlowMorph("999", sea) == 120.0f);
    CHECK(rm::ParseSeaFlowRho(nullptr, sea) == rm::kSeaFlowSeaRho);
    CHECK(rm::ParseSeaFlowRho(nullptr, river) == 0.3f);
    CHECK(rm::ParseSeaFlowRho("x", river) == 0.3f);
    CHECK(rm::ParseSeaFlowRho("0.5", sea) == 0.5f);
    CHECK(rm::ParseSeaFlowRho("2", sea) == 0.95f);
    // The drawn sea's share of the sea level's wave: a quarter with the flow
    // on, today's with it off, unless the variable says otherwise.
    CHECK(rm::ParseSeaFlowHeave(nullptr, true) == 0.25f);
    CHECK(rm::ParseSeaFlowHeave("x", true) == 0.25f);
    CHECK(rm::ParseSeaFlowHeave(nullptr, false) == 1.0f);
    CHECK(rm::ParseSeaFlowHeave("", false) == 1.0f);
    CHECK(rm::ParseSeaFlowHeave("x", false) == 1.0f);
    CHECK(rm::ParseSeaFlowHeave("0", false) == 0.0f);
    CHECK(rm::ParseSeaFlowHeave("0", true) == 0.0f);
    CHECK(rm::ParseSeaFlowHeave("1", true) == 1.0f);
    CHECK(rm::ParseSeaFlowHeave("7", true) == 1.0f);
    CHECK(rm::ParseSeaFlowFreezeProbe("freeze"));
    CHECK_FALSE(rm::ParseSeaFlowFreezeProbe(nullptr));
    CHECK_FALSE(rm::ParseSeaFlowFreezeProbe("1"));
    // With the switches unset, the drawn sea level's wave is today's.
    if (!rm::SeaFlowXpOn() && !std::getenv("POSEIDON_XP_SEAFLOW_HEAVE"))
        CHECK(rm::SeaFlowXpHeave() == 1.0f);
}

TEST_CASE("XP sea flow: the frames never play backward and never repeat", "[Graphics][SeaFlowXp]")
{
    CHECK(rm::SeaFlowFrameStride(7) == 3);
    CHECK(rm::SeaFlowFrameStride(2) == 1);
    CHECK(rm::SeaFlowFrameStride(5) == 2);
    // No dissolve: two different frames that are not neighbours, for ever.
    for (double t : {0.0, 0.7, 1.0, 1.9, 2.0, 117.3})
    {
        const rm::SeaFlowFrames f = rm::SeaFlowFrameBlend(t, 7, 0.0f);
        CHECK(f.bound == 0);
        CHECK(f.other == 3);
        CHECK(f.otherWeight == 0.0f);
    }
    // One frame: nothing to pick.
    CHECK(rm::SeaFlowFrameBlend(3.0, 1, 4.0f).other == 0);
    // The dissolve's order: one or two frames forward round the 7-frame cycle,
    // never back, and the run of steps has no period.
    std::vector<int> steps;
    for (long long s = 0; s < 400; s++)
    {
        const int d = (rm::SeaFlowMorphFrame(s + 1, 7) - rm::SeaFlowMorphFrame(s, 7) + 7) % 7;
        REQUIRE((d == 1 || d == 2));
        steps.push_back(d);
    }
    for (size_t period = 1; period <= 60; period++)
    {
        bool periodic = true;
        for (size_t i = 0; i + period < steps.size() && periodic; i++)
            periodic = steps[i] == steps[i + period];
        CHECK_FALSE(periodic);
    }
    // The dissolve: each frame's share is continuous in time, every step goes
    // from the frame just reached to a new one (never back to the one before),
    // and it visits all seven.
    const float step = 0.8f;
    std::set<int> seen;
    int prevBound = -1, prevOther = -1;
    double prevShare[7] = {1, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i <= 20 * 400; i++)
    {
        const double t = i * (step / 400.0);
        const rm::SeaFlowFrames f = rm::SeaFlowFrameBlend(t, 7, step);
        REQUIRE(f.bound != f.other);
        double share[7] = {0, 0, 0, 0, 0, 0, 0};
        share[f.bound] += 1.0 - f.otherWeight;
        share[f.other] += f.otherWeight;
        for (int k = 0; k < 7; k++)
            REQUIRE(std::abs(share[k] - prevShare[k]) < 0.004);
        if (f.bound != prevBound && prevBound >= 0)
        {
            REQUIRE(f.bound == prevOther); // the frame just reached carries on
            REQUIRE(f.other != prevBound); // and the next is not the one it came from
        }
        prevBound = f.bound;
        prevOther = f.other;
        for (int k = 0; k < 7; k++)
            prevShare[k] = share[k];
        seen.insert(f.bound);
    }
    CHECK(seen.size() == 7);
    // AnimateTexture binds floor(phase * n).
    for (int k = 0; k < 7; k++)
        CHECK(static_cast<int>(std::floor(rm::SeaFlowAnimationPhase(k, 7) * 7)) == k);
}

TEST_CASE("XP sea flow: the sea field's layers cross their crests and cancel", "[Graphics][SeaFlowXp]")
{
    for (const rm::SeaFlowField field : {rm::SeaFlowField::Sea, rm::SeaFlowField::River})
    {
        const rm::SeaFlowRates r = rm::ComputeSeaFlowRates(1.0f, field);
        const float* all[5] = {r.base, r.swell, r.chop, r.ripple, r.swell2};
        for (const float* v : all)
        {
            for (int c = 0; c < 2; c++)
            {
                // Whole numbers of 1 / 4096: fract(rate * t) is continuous where the clock wraps.
                const float k = v[c] * rm::kSeaFlowTimeWrap;
                CHECK(k == std::round(k));
            }
        }
        for (const float* v : {r.base, r.swell, r.chop})
            CHECK(std::hypot(v[0], v[1]) > 0.0f);
        // Both fields keep the stock sea's slope spread, on the fine pair and on the swell.
        const rm::SeaFlowShape& sh = rm::SeaFlowFieldShape(field);
        CHECK(std::abs(sh.baseK * sh.baseK + sh.chopFreq * sh.chopFreq * sh.chopK * sh.chopK - 0.8f * 0.8f) < 0.01f);
        CHECK(std::abs(sh.swellK * sh.swellK + sh.swell2Freq * sh.swell2Freq * sh.swell2K * sh.swell2K - 1.2f * 1.2f) <
              0.01f);
    }
    // Round 2's ripples went one way and it had no swell partner; round 3's
    // ripples stay where they are (as today's) and its swell has a partner.
    const rm::SeaFlowRates rr = rm::ComputeSeaFlowRates(1.0f, rm::SeaFlowField::River);
    CHECK(std::hypot(rr.ripple[0], rr.ripple[1]) > 0.0f);
    CHECK(rr.swell2[0] == 0.0f);
    CHECK(rr.swell2[1] == 0.0f);
    CHECK(rm::kSeaFlowRiverShape.swell2K == 0.0f);
    const rm::SeaFlowRates rs = rm::ComputeSeaFlowRates(1.0f, rm::SeaFlowField::Sea);
    CHECK(rs.ripple[0] == 0.0f);
    CHECK(rs.ripple[1] == 0.0f);
    CHECK(std::hypot(rs.swell2[0], rs.swell2[1]) > 0.0f);
    // The swell partner's map: 37 - 8 degrees, 0.27 x 0.88.
    float m2[4];
    rm::SeaFlowSwell2Map(rm::kSeaFlowSeaShape, m2);
    const float f2 = 0.27f * 0.88f;
    CHECK(std::abs(m2[0] - std::cos(29.0f * 0.01745329252f) * f2) < 1e-4f);
    CHECK(std::abs(m2[1] - std::sin(29.0f * 0.01745329252f) * f2) < 1e-4f);
    CHECK(std::abs(m2[2] + std::sin(29.0f * 0.01745329252f) * f2) < 1e-4f);
    CHECK(std::abs(m2[3] - m2[0]) < 1e-6f);
    // The third layer's map turns and scales the texture's coordinates
    // (round 2's literal map: 0.61 x, turned -16 degrees).
    float m[4];
    rm::SeaFlowChopMap(rm::kSeaFlowRiverShape, m);
    CHECK(std::abs(m[0] - 0.9613f * 0.61f) < 1e-4f);
    CHECK(std::abs(m[1] + 0.2756f * 0.61f) < 1e-4f);
    CHECK(std::abs(m[2] - 0.2756f * 0.61f) < 1e-4f);
    CHECK(std::abs(m[3] - 0.9613f * 0.61f) < 1e-4f);

    // The crests each layer shows on the world: the texture's, turned back by
    // the layer's map.
    auto crestsOf = [](const rm::SeaFlowShape& sh, float out[5])
    {
        out[0] = rm::kSeaFlowTexCrestDeg;
        out[1] = rm::kSeaFlowTexCrestDeg - rm::kSeaFlowSwellTurnDeg;
        out[2] = rm::kSeaFlowTexCrestDeg - sh.chopTurnDeg;
        out[3] = rm::kSeaFlowRippleCrestDeg;
        out[4] = rm::kSeaFlowTexCrestDeg - rm::kSeaFlowSwellTurnDeg - sh.swell2TurnDeg;
    };
    auto offCrest = [](float heading, float crest)
    {
        float off = std::fmod(std::abs(heading - crest), 180.0f);
        return std::min(off, 180.0f - off); // 0 along the crests, 90 across
    };
    const rm::SeaFlowLayers& L = rm::kSeaFlowSea;
    const rm::SeaFlowShape& S = rm::kSeaFlowSeaShape;
    float crests[5];
    crestsOf(S, crests);
    // Round 3: each moving layer travels square to its crests (the ripples
    // stand still).
    const rm::SeaFlowLayer layers[5] = {L.base, L.swell, L.chop, L.ripple, L.swell2};
    for (int i = 0; i < 5; i++)
    {
        if (i == 3)
            CHECK(layers[i].speedMps == 0.0f);
        else
            CHECK(std::abs(offCrest(layers[i].headingDeg, crests[i]) - 90.0f) < 0.5f);
    }
    // Round 2's moved along them (within 12 degrees): the streaks slid lengthwise.
    float riverCrests[5];
    crestsOf(rm::kSeaFlowRiverShape, riverCrests);
    const rm::SeaFlowLayer river[4] = {rm::kSeaFlowRiver.base, rm::kSeaFlowRiver.swell, rm::kSeaFlowRiver.chop,
                                       rm::kSeaFlowRiver.ripple};
    for (int i = 0; i < 4; i++)
        CHECK(offCrest(river[i].headingDeg, riverCrests[i]) < 12.0f);

    // The fine detail: the texture's layer and the third one. In the sea field
    // they have the same slope share, near the same size and speed, and go
    // opposite ways, so their motions cancel; in round 2's they went the same
    // way.
    auto pairDrift = [](const rm::SeaFlowLayers& f, const rm::SeaFlowShape& sh)
    {
        const double wb = sh.baseK * f.base.speedMps, wc = sh.chopK * sh.chopFreq * f.chop.speedMps;
        const double ab = f.base.headingDeg * 0.017453292519943295, ac = f.chop.headingDeg * 0.017453292519943295;
        return std::hypot(wb * std::cos(ab) + wc * std::cos(ac), wb * std::sin(ab) + wc * std::sin(ac)) / (wb + wc);
    };
    CHECK(std::abs(S.baseK - S.chopK * S.chopFreq) < 0.01f);
    CHECK(S.chopFreq > 0.8f);
    CHECK(pairDrift(rm::kSeaFlowSea, S) < 0.1);
    CHECK(pairDrift(rm::kSeaFlowRiver, rm::kSeaFlowRiverShape) > 0.9);
    // The swell and its partner the same way: same slope share, opposite ways.
    const double ws = S.swellK * rm::kSeaFlowSwellFreq * L.swell.speedMps,
                 ws2 = S.swell2K * S.swell2Freq * rm::kSeaFlowSwellFreq * L.swell2.speedMps;
    const double as = L.swell.headingDeg * 0.017453292519943295, as2 = L.swell2.headingDeg * 0.017453292519943295;
    CHECK(std::abs(S.swellK - S.swell2K * S.swell2Freq) < 0.01f);
    CHECK(std::hypot(ws * std::cos(as) + ws2 * std::cos(as2), ws * std::sin(as) + ws2 * std::sin(as2)) / (ws + ws2) <
          0.1);

    // Speeds by feature size: sqrt of the size ratio, and far slower than round 2's.
    CHECK(std::abs(L.swell.speedMps / L.base.speedMps - std::sqrt(1.0f / rm::kSeaFlowSwellFreq)) < 0.01f);
    CHECK(std::abs(L.chop.speedMps / L.base.speedMps - std::sqrt(1.0f / S.chopFreq)) < 0.01f);
    CHECK(std::abs(L.swell2.speedMps / L.swell.speedMps - std::sqrt(1.0f / S.swell2Freq)) < 0.01f);
    CHECK(L.base.speedMps < 0.5f * rm::kSeaFlowRiver.base.speedMps);
    CHECK(L.swell.speedMps < 0.5f * rm::kSeaFlowRiver.swell.speedMps);
    // The texture's layer: the base speed over a 50 m repeat, along its
    // heading (the offset runs against the travel).
    const rm::SeaFlowRates r = rm::ComputeSeaFlowRates(1.0f, rm::SeaFlowField::Sea);
    CHECK(std::abs(std::hypot(r.base[0], r.base[1]) - rm::kSeaFlowSeaBaseMps / 50.0f) < 0.0005f);
    CHECK(r.base[1] < 0.0f);
    // The layers come round at their own times, and no two come round
    // together inside 4 minutes.
    const double tb = 50.0 / L.base.speedMps;
    const double ts = 50.0 / rm::kSeaFlowSwellFreq / L.swell.speedMps;
    const double tc = 50.0 / S.chopFreq / L.chop.speedMps;
    const double ts2 = 50.0 / (rm::kSeaFlowSwellFreq * S.swell2Freq) / L.swell2.speedMps;
    CHECK(tb > 30.0);
    const double pairs[6][2] = {{ts, tb}, {tc, tb}, {ts, tc}, {ts2, tb}, {ts2, tc}, {ts2, ts}};
    for (const auto& pr : pairs)
        for (int p = 1; p <= 4; p++)
            for (int q = 1; q <= 4; q++)
                if (std::abs(pr[0] / pr[1] - static_cast<double>(p) / q) <= 0.02)
                    CHECK(q * pr[0] > 240.0);
    // The speed scale scales the rates.
    const rm::SeaFlowRates r2 = rm::ComputeSeaFlowRates(2.0f, rm::SeaFlowField::Sea);
    CHECK(std::abs(r2.base[0] - 2.0f * r.base[0]) < 2.0f / rm::kSeaFlowTimeWrap);
}

TEST_CASE("XP sea flow: the water main moves every layer and compiles", "[Graphics][SeaFlowXp]")
{
    const std::string stock = gl::RemasterWaterMainStock();
    // The stock main is what plays the loop: fixed coordinates, one mixed frame.
    CHECK(stock.find("vec4 t0 = seaTex(vUV0);") != std::string::npos);
    CHECK(stock.find("flowT") == std::string::npos);

    GlslangInit init;
    for (const bool mirror : {false, true})
    {
        // Every field with and without the dissolve: the shipped variant is the
        // sea field without it.
        for (const int variant : {0, 1, 2, 3})
        {
            const bool morph = (variant & 1) != 0;
            const rm::SeaFlowField field = (variant & 2) != 0 ? rm::SeaFlowField::River : rm::SeaFlowField::Sea;
            CAPTURE(mirror, morph, variant);
            const std::string main = gl::SeaFlowWaterMainXp(mirror, 1.0f, 0.3f, morph, field);
            REQUIRE(!main.empty());
            CHECK(CountOf(main, "float flowT = litWater2.y;") == 1);
            CHECK(CountOf(main, "vec2 uvB = vUV0 + fract(kFlowRateB * flowT);") == 1);
            CHECK(CountOf(main, "vec4 t0 = seaTex(uvB);") == 1);
            CHECK(CountOf(main, "vec2 grad = kFlowBaseK * seaGrad(uvB, dot(t0.rgb, kLuma), gU, gV, e0);") == 1);
            CHECK(CountOf(main, "+ fract(kFlowRateS * flowT);") == 1);
            CHECK(CountOf(main, "grad += kFlowSwellK * seaGradS(uvS, seaLumS(uvS), gUs, gVs, eS);") == 1);
            // The sea field's swell has a partner; round 2's has none.
            CHECK(CountOf(main, "grad += kFlowSwell2K * seaGradS(uvS2, seaLumS(uvS2), gUs2, gVs2, eS2);") ==
                  (field == rm::SeaFlowField::Sea ? 1u : 0u));
            CHECK(CountOf(main, "grad += kFlowChopK * seaGrad(uvC, seaLum(uvC), gUc, gVc, eC);") == 1);
            CHECK(CountOf(main, "grad += kSeaRippleK * seaRippleGrad(uvR, t1.a, gU * 32.0, gV * 32.0, e1);") == 1);
            CHECK(CountOf(main, "vec4 t1 = texture(tex1, uvR);") == 1);
            // With the dissolve the swell mixes the frames the same way round as
            // the other layers (the other way, it would jump at each step).
            CHECK(CountOf(main, "float seaLumS(vec2 uv) {\n    return dot(mix(texture(tex0, uv), texture(seaNext, uv), "
                                "litWater0.y).rgb, kLuma);\n}") == (morph ? 1u : 0u));
            // Without it the swell takes seaNext alone, in one fetch.
            CHECK(CountOf(main, "float seaLumS(vec2 uv) {\n    return dot(texture(seaNext, uv).rgb, kLuma);\n}") ==
                  (morph ? 0u : 1u));
            // No wave layer is left on the still coordinates.
            CHECK(main.find("seaGrad(vUV0") == std::string::npos);
            CHECK(main.find("seaTex(vUV0)") == std::string::npos);
            CHECK(main.find("seaRippleGrad(vUV1") == std::string::npos);
            CHECK(CountOf(main, "fragWave = vec4(grad, 0.0, 1.0);") == (mirror ? 1u : 0u));
            std::string log;
            const bool ok = CompilesFragment(std::string(gl::RemasterFragmentCommon()) + main, log);
            CAPTURE(log);
            CHECK(ok);
        }
    }
}
