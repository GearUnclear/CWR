#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <Poseidon/Graphics/Remaster/CloudNoise.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// Remastered lighting (Graphics/Remaster + EngineGL33_Lighting*): the pure light
// rig, horizon map and cloud noise math, and the contracts between the C++
// uploads and the GLSL that reads them. The look itself is judged in the
// offline lab (tests/rendering/lighting_lab).

using namespace Poseidon::remaster;

namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kBrightness = 1.6f; // default user brightness carried by the materials

LightRigInputs ClearSky(float elevationDeg, float azimuthDeg = 200.0f)
{
    LightRigInputs in;
    const float e = elevationDeg * kPi / 180.0f;
    const float a = azimuthDeg * kPi / 180.0f;
    in.towardSun[0] = std::cos(e) * std::sin(a);
    in.towardSun[1] = std::sin(e);
    in.towardSun[2] = std::cos(e) * std::cos(a);
    in.fogColor = Rgb(0.73f, 0.79f, 0.85f);
    in.skyThrough = 1.0f;
    in.nightEffect = elevationDeg > 5.0f ? 0.0f : 1.0f;
    return in;
}

// On-screen fraction of the texture brightness for a surface that receives
// irradiance `e` (rig units), before the display gamma pass.
float Shown(float e, float exposure)
{
    return ToDisplay(e * kBrightness * exposure);
}

float RedOverBlue(const Rgb& c)
{
    return c.r / std::max(c.b, 1e-6f);
}
} // namespace

TEST_CASE("Remaster light rig: sunlit ground keeps its classic brightness through the day", "[Graphics][remaster]")
{
    const LightingTuning tuning;
    for (float elevation : {60.0f, 45.0f, 30.0f})
    {
        CAPTURE(elevation);
        const LightRigInputs in = ClearSky(elevation);
        const LightRig rig = ComputeLightRig(in, tuning);
        const float sinSun = in.towardSun[1];
        const float lit = Luminance(rig.sun) * sinSun + Luminance(rig.skyZenith);
        const float shade = Luminance(rig.skyZenith);
        // Classic clamps lit ground to the bare texture (1.0 on screen).
        CHECK(Shown(lit, rig.exposure) > 0.9f);
        CHECK(Shown(lit, rig.exposure) < 1.06f);
        // Cast shadow: bold but not crushed (reviewers rejected ~0.42).
        const float ratio = Shown(shade, rig.exposure) / Shown(lit, rig.exposure);
        CHECK(ratio > 0.5f);
        CHECK(ratio < 0.75f);
    }
}

TEST_CASE("Remaster light rig: golden hour is darker, warmer and more contrasty than noon", "[Graphics][remaster]")
{
    const LightingTuning tuning;
    const LightRig noon = ComputeLightRig(ClearSky(60.0f), tuning);
    const LightRigInputs lowIn = ClearSky(7.0f);
    const LightRig low = ComputeLightRig(lowIn, tuning);
    const float noonGround = Luminance(noon.sun) * std::sin(60.0f * kPi / 180.0f) + Luminance(noon.skyZenith);
    const float lowGround = Luminance(low.sun) * lowIn.towardSun[1] + Luminance(low.skyZenith);
    CHECK(Shown(lowGround, low.exposure) < 0.8f * Shown(noonGround, noon.exposure));
    // The sun is golden and the shade facing away from it stays cool.
    CHECK(RedOverBlue(low.sun) > 1.6f);
    CHECK(RedOverBlue(noon.sun) < 1.1f);
    CHECK(RedOverBlue(low.skyHorizonSun) > RedOverBlue(low.skyHorizon));
    // A wall facing a 7 degree sun is lit well above one facing away from it.
    const float sunWall = Luminance(low.sun) * std::cos(7.0f * kPi / 180.0f) + Luminance(low.skyHorizonSun);
    CHECK(sunWall > 4.0f * Luminance(low.skyHorizon));
}

TEST_CASE("Remaster light rig: twilight fill never outshines the visible sky", "[Graphics][remaster]")
{
    const LightingTuning tuning;
    LightRigInputs in = ClearSky(1.0f);
    in.fogColor = Rgb(0.20f, 0.12f, 0.12f); // dim dusk sky
    const LightRig rig = ComputeLightRig(in, tuning);
    const float skyShown = Luminance(ToLinear(in.fogColor));
    CHECK(Luminance(rig.skyZenith) * kBrightness * rig.exposure <= skyShown * 1.001f);
}

TEST_CASE("Remaster light rig: night keeps the classic ambient level", "[Graphics][remaster]")
{
    const LightingTuning tuning;
    const LightRig rig = ComputeLightRig(ClearSky(-30.0f), tuning);
    CHECK(Luminance(rig.sun) == 0.0f);
    CHECK(Luminance(rig.skyZenith) == Catch::Approx(0.0009f).margin(1e-6));
    CHECK(rig.exposure == Catch::Approx(1.0f));
}

TEST_CASE("Remaster light rig: cloud shadows carry the classic overcast dimming on average", "[Graphics][remaster]")
{
    const LightingTuning tuning;
    for (float skyThrough : {0.2f, 0.6f, 0.9f})
    {
        CAPTURE(skyThrough);
        LightRigInputs in = ClearSky(45.0f);
        in.skyThrough = skyThrough;
        in.cloudShadowsActive = false;
        const LightRig classicDim = ComputeLightRig(in, tuning);
        in.cloudShadowsActive = true;
        const LightRig field = ComputeLightRig(in, tuning);
        CHECK(field.cloudCover == Catch::Approx(1.0f - skyThrough));
        const float meanSun = Luminance(field.sun) * (1.0f - field.cloudCover * (1.0f - field.cloudTransmit));
        CHECK(meanSun == Catch::Approx(Luminance(classicDim.sun)).epsilon(1e-4));
    }
}

TEST_CASE("Remaster sun chroma and tonemap", "[Graphics][remaster]")
{
    float prev = 0.0f;
    for (float e : {60.0f, 30.0f, 15.0f, 8.0f, 3.0f, 0.0f})
    {
        CAPTURE(e);
        const Rgb c = SunChroma(e, 1.0f);
        CHECK(Luminance(c) == Catch::Approx(1.0f).epsilon(1e-4));
        CHECK(RedOverBlue(c) > prev); // warmer as the sun drops
        prev = RedOverBlue(c);
    }
    // Neutral Khronos curve without its toe: identity below 0.8, never above 1.
    const Rgb low = TonemapNeutral(Rgb(0.5f, 0.3f, 0.1f));
    CHECK(low.r == 0.5f);
    CHECK(low.b == 0.1f);
    float last = 0.0f;
    for (float v = 0.8f; v < 20.0f; v *= 1.3f)
    {
        const Rgb t = TonemapNeutral(Rgb(v, v, v));
        CHECK(t.r <= 1.0f);
        CHECK(t.r >= last);
        last = t.r;
    }
}

TEST_CASE("Remaster crown ellipsoid sits on the upper part of the shape", "[Graphics][remaster]")
{
    const float mn[3] = {-3.0f, 0.0f, -2.0f};
    const float mx[3] = {3.0f, 10.0f, 2.0f};
    float c[3], ih[3];
    CrownEllipsoid(mn, mx, c, ih);
    CHECK(c[0] == Catch::Approx(0.0f));
    CHECK(c[1] == Catch::Approx(6.0f));
    CHECK(c[2] == Catch::Approx(0.0f));
    CHECK(ih[0] == Catch::Approx(1.0f / 3.0f));
    CHECK(ih[1] == Catch::Approx(1.0f / 5.0f));
    CHECK(ih[2] == Catch::Approx(1.0f / 2.0f));
    // A flat or degenerate box stays finite.
    const float flat[3] = {1.0f, 1.0f, 1.0f};
    CrownEllipsoid(flat, flat, c, ih);
    CHECK(std::isfinite(ih[0]));
    CHECK(std::isfinite(ih[1]));
}

TEST_CASE("Remaster terrain horizon: a ridge shadows the valley behind it from a low sun", "[Graphics][remaster]")
{
    // 128 x 128 at 25 m; a 120 m north-south ridge along x = 40.
    const int n = 128;
    const float cell = 25.0f;
    std::vector<float> h(static_cast<size_t>(n) * n, 0.0f);
    for (int z = 0; z < n; ++z)
        for (int x = 0; x < n; ++x)
        {
            const float d = static_cast<float>(x - 40);
            h[static_cast<size_t>(z) * n + x] = 120.0f * std::exp(-d * d / 18.0f);
        }
    TerrainHorizonOptions opt;
    opt.threads = 2;
    TerrainHorizonMap map;
    REQUIRE(BuildTerrainHorizonMap(h.data(), n, cell, opt, map));
    REQUIRE(map.Valid());
    REQUIRE(map.size == n);
    REQUIRE(map.horizon.size() == static_cast<size_t>(TerrainHorizonMap::kLayers) * n * n * 4);

    // Sun in the west (-x) at 5 degrees; a point 250 m east of the crest sees
    // the ridge at ~25 degrees: in shadow. The same sun at 40 degrees clears it.
    const float e5 = 5.0f * kPi / 180.0f;
    const float e40 = 40.0f * kPi / 180.0f;
    CHECK(SunVisibility(map, 50, 64, -std::cos(e5), std::sin(e5), 0.0f, 1.2f) < 0.05f);
    CHECK(SunVisibility(map, 50, 64, -std::cos(e40), std::sin(e40), 0.0f, 1.2f) > 0.95f);
    // The far side of the ridge, facing the sun, stays lit.
    CHECK(SunVisibility(map, 30, 64, -std::cos(e5), std::sin(e5), 0.0f, 1.2f) > 0.95f);
    // Open plain far from the ridge sees nearly the whole sky; the foot of the
    // ridge sees less.
    CHECK(map.SkyVisibility(110, 64) > 0.9f);
    CHECK(map.SkyVisibility(46, 64) < map.SkyVisibility(110, 64));

    // The 16-bit encoding round-trips to well under the shader penumbra.
    for (float deg : {-5.0f, -1.3f, 0.0f, 2.7f, 12.0f, 45.0f})
        CHECK(DecodeHorizonDeg(EncodeHorizonDeg(deg)) == Catch::Approx(deg).margin(0.001f));
    CHECK(HashHeights(h.data(), n, cell) != HashHeights(h.data(), n, cell * 2.0f));
}

TEST_CASE("Remaster ground tint colours the bounce without changing its brightness", "[Graphics][remaster]")
{
    auto green = [](float r, float b) { return (1.0f - 0.2126f * r - 0.0722f * b) / 0.7152f; };
    struct Case
    {
        float r, g, b;
    };
    const Case cases[] = {{1, 1, 1},          {0.76f, 0.65f, 0.45f}, {0.30f, 0.42f, 0.18f}, {0.0f, 1.0f, 0.0f},
                          {1.0f, 0.0f, 1.0f}, {0.02f, 0.02f, 0.02f}, {0.20f, 0.35f, 0.60f}};
    for (const Case& c : cases)
    {
        CAPTURE(c.r, c.g, c.b);
        float tr = 0, tb = 0;
        GroundTint(c.r, c.g, c.b, 0.75f, tr, tb);
        const float tg = green(tr, tb);
        CHECK(0.2126f * tr + 0.7152f * tg + 0.0722f * tb == Catch::Approx(1.0f).epsilon(1e-4));
        for (float v : {tr, tg, tb})
        {
            CHECK(v >= 0.5f - 1e-4f);
            CHECK(v <= 1.6f + 1e-4f);
        }
    }
    float tr = 0, tb = 0;
    GroundTint(1, 1, 1, 0.75f, tr, tb);
    CHECK(tr == Catch::Approx(1.0f));
    CHECK(tb == Catch::Approx(1.0f));
    GroundTint(0.76f, 0.65f, 0.45f, 0.75f, tr, tb); // desert sand: warm bounce
    CHECK(tr > 1.15f);
    CHECK(tb < 0.7f);
    GroundTint(0.76f, 0.65f, 0.45f, 0.0f, tr, tb); // strength 0 = neutral
    CHECK(tr == Catch::Approx(1.0f));
    CHECK(tb == Catch::Approx(1.0f));

    // The builder downsamples the colours with the heights.
    const int n = 32;
    std::vector<float> h(static_cast<size_t>(n) * n, 0.0f);
    std::vector<float> rgb(static_cast<size_t>(n) * n * 3);
    for (size_t i = 0; i < static_cast<size_t>(n) * n; ++i)
    {
        rgb[i * 3 + 0] = 0.76f;
        rgb[i * 3 + 1] = 0.65f;
        rgb[i * 3 + 2] = 0.45f;
    }
    TerrainHorizonOptions opt;
    opt.maxSize = 16;
    opt.threads = 1;
    TerrainHorizonMap map;
    REQUIRE(BuildTerrainHorizonMap(h.data(), n, 10.0f, opt, map, rgb.data()));
    REQUIRE(map.size == 16);
    REQUIRE(map.groundTint.size() == static_cast<size_t>(16) * 16 * 2);
    GroundTint(0.76f, 0.65f, 0.45f, 0.75f, tr, tb);
    CHECK(map.groundTint[0] == Catch::Approx(tr));
    CHECK(map.groundTint[1] == Catch::Approx(tb));
    TerrainHorizonMap plain;
    REQUIRE(BuildTerrainHorizonMap(h.data(), n, 10.0f, opt, plain));
    CHECK(plain.groundTint.empty());
}

TEST_CASE("Remaster cloud noise is uniform, so a threshold sets the covered fraction exactly", "[Graphics][remaster]")
{
    const int n = 128;
    const std::vector<std::uint8_t> noise = BuildCloudNoise(n, 7);
    REQUIRE(noise.size() == static_cast<size_t>(n) * n);
    for (float cover : {0.1f, 0.4f, 0.8f})
    {
        CAPTURE(cover);
        int above = 0;
        for (std::uint8_t v : noise)
            if (v / 255.0f >= 1.0f - cover)
                ++above;
        CHECK(static_cast<float>(above) / static_cast<float>(noise.size()) == Catch::Approx(cover).margin(0.02f));
    }
    // Tileable: the seam between the last and first column is as smooth as
    // any other pair of neighbouring columns.
    auto columnStep = [&](int a, int b)
    {
        double sum = 0.0;
        for (int z = 0; z < n; ++z)
            sum += std::abs(static_cast<int>(noise[static_cast<size_t>(z) * n + a]) -
                            static_cast<int>(noise[static_cast<size_t>(z) * n + b]));
        return sum / n;
    };
    CHECK(columnStep(n - 1, 0) < 2.0 * columnStep(n / 2, n / 2 + 1) + 4.0);
}

// --- GLSL contracts --------------------------------------------------------

namespace
{
std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path Gl33Dir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine" / "PoseidonGL33";
}

std::string RawString(const std::string& src, const std::string& symbol)
{
    const std::string header = "static const char " + symbol + "[] = R\"(";
    const size_t start = src.find(header);
    if (start == std::string::npos)
        return {};
    const size_t body = start + header.size();
    const size_t end = src.find(")\"", body);
    return end == std::string::npos ? std::string() : src.substr(body, end - body);
}

std::vector<std::string> Symbols(const std::string& src)
{
    std::vector<std::string> out;
    static const std::regex header(R"(static\s+const\s+char\s+(s_\w+)\[\]\s*=\s*R\"\()");
    for (auto it = std::sregex_iterator(src.begin(), src.end(), header); it != std::sregex_iterator(); ++it)
        out.push_back((*it)[1].str());
    return out;
}

// Plain GLSL 3.30 core validation (glslangValidator's default). No SPIR-V
// client, unlike the I-28 harness: the post shaders use loose uniforms, which
// GL 3.3 allows and SPIR-V for OpenGL does not.
bool Compiles(const std::string& src, EShLanguage stage, std::string& log)
{
    glslang::TShader shader(stage);
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

// `<qualifier> <type> <name>;` declarations (varyings) of a GLSL block.
std::set<std::string> Varyings(const std::string& glsl, const std::string& qualifier)
{
    std::set<std::string> out;
    const std::regex decl("(?:flat\\s+)?" + qualifier + "\\s+(\\w+)\\s+(\\w+)\\s*;");
    for (auto it = std::sregex_iterator(glsl.begin(), glsl.end(), decl); it != std::sregex_iterator(); ++it)
    {
        const std::string name = (*it)[2].str();
        if (name != "fragColor")
            out.insert((*it)[1].str() + " " + name);
    }
    return out;
}
} // namespace

TEST_CASE("Remaster shaders compile under glslang and their varyings match", "[Graphics][Shaders][remaster]")
{
    GlslangInit init;
    const std::string lighting = ReadTextFile(Gl33Dir() / "EngineGL33_LightingShaders.cpp");
    REQUIRE_FALSE(lighting.empty());
    const std::string common = RawString(lighting, "s_psRemasterCommon");
    REQUIRE_FALSE(common.empty());

    int vertex = 0, fragment = 0;
    for (const std::string& sym : Symbols(lighting))
    {
        CAPTURE(sym);
        const std::string body = RawString(lighting, sym);
        std::string log;
        if (sym.rfind("s_vs", 0) == 0)
        {
            const bool ok = Compiles(body, EShLangVertex, log);
            CAPTURE(log);
            CHECK(ok);
            // Every remastered VS feeds the same varyings the common FS reads.
            CHECK(Varyings(body, "out") == Varyings(common, "in"));
            ++vertex;
        }
        else if (sym != "s_psRemasterCommon")
        {
            const bool ok = Compiles(common + body, EShLangFragment, log);
            CAPTURE(log);
            CHECK(ok);
            ++fragment;
        }
    }
    CHECK(vertex == 3);   // VSScreen, VSTransform, VSShadow
    CHECK(fragment == 4); // PSNormal, PSDetail, PSGrass, PSWater

    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    REQUIRE_FALSE(post.empty());
    int postCount = 0;
    for (const std::string& sym : Symbols(post))
    {
        CAPTURE(sym);
        std::string log;
        const bool ok =
            Compiles(RawString(post, sym), sym.rfind("s_vs", 0) == 0 ? EShLangVertex : EShLangFragment, log);
        CAPTURE(log);
        CHECK(ok);
        ++postCount;
    }
    CHECK(postCount == 7);
}

TEST_CASE("Remaster LitConstants GLSL block matches the LitConst slot layout", "[Graphics][Shaders][remaster]")
{
    namespace LC = Poseidon::gl33lighting::LitConst;
    const std::string lighting = ReadTextFile(Gl33Dir() / "EngineGL33_LightingShaders.cpp");
    const size_t open = lighting.find("uniform LitConstants {");
    REQUIRE(open != std::string::npos);
    const size_t close = lighting.find("};", open);
    REQUIRE(close != std::string::npos);
    const std::string block = lighting.substr(open, close - open);

    // Walk the members in order, counting vec4 slots.
    std::map<std::string, int> slot;
    int at = 0;
    static const std::regex member(R"(vec4\s+(\w+)(?:\[(\d+)\])?\s*;)");
    for (auto it = std::sregex_iterator(block.begin(), block.end(), member); it != std::sregex_iterator(); ++it)
    {
        slot[(*it)[1].str()] = at;
        at += (*it)[2].matched ? std::stoi((*it)[2].str()) : 1;
    }
    CHECK(at == LC::Count);
    CHECK(slot["litSunDir"] == LC::SunDir);
    CHECK(slot["litZenith"] == LC::Zenith);
    CHECK(slot["litMatDiffuse"] == LC::MatDiffuse);
    CHECK(slot["litTerrain"] == LC::Terrain);
    CHECK(slot["litCloud"] == LC::Cloud);
    CHECK(slot["litShadowDelta"] == LC::ShadowDelta);
    CHECK(slot["litFlags"] == LC::Flags);
    CHECK(slot["litLightCount"] == LC::LightCount);
    CHECK(slot["litHorizonSun"] == LC::HorizonSun);
    CHECK(slot["litFogSun"] == LC::FogSun);
    CHECK(slot["litFogCap"] == LC::FogCap);
    CHECK(slot["litLightPos"] == LC::LightPos);
    CHECK(slot["litLightColor"] == LC::LightColor);
    CHECK(slot["litLightAmb"] == LC::LightAmb);
    CHECK(slot["litLightDir"] == LC::LightDir);

    // The decoded horizon range in the shader is the encoder's.
    std::ostringstream range;
    range << "mix(" << std::fixed;
    range.precision(1);
    range << TerrainHorizonMap::kMinAngleDeg << ", " << TerrainHorizonMap::kMaxAngleDeg << ",";
    CHECK(lighting.find(range.str()) != std::string::npos);
}
