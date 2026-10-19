#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/FrameLights.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <Poseidon/Graphics/Remaster/NightLights.hpp>
#include <Poseidon/Graphics/Remaster/SunShadows.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// The remastered night (INTEGRATION-PLAN.md section 5 B3, research track
// night; Graphics/Remaster/NightLights.hpp): the moon as a shadow-casting key
// light that keeps F9's calibration, street lamps and headlights as physical
// lights on the tiled list (inverse square from the bulb, a low beam aimed
// from the body), the same lights on smoke, and stars as soft splats. Each
// case pins one contract the shaders, the backend or the scene rely on.

using namespace Poseidon::remaster;
namespace lt = Poseidon::lighting;

namespace
{

constexpr float kPi = 3.14159265f;

LightingTuning Apply(const std::string& setting)
{
    LightingTuning t{};
    bool modeSet = false;
    const std::string why = ApplyLightingSetting(t, setting, modeSet);
    INFO(setting << ": " << why);
    REQUIRE(why.empty());
    return t;
}

// A clear night with a full moon `moonDeg` up and the sun `sunDeg` under.
GateInputs MoonNight(float sunDeg = -30.0f, float moonDeg = 36.0f, float phase = 1.0f)
{
    GateInputs in;
    in.sunElevationDeg = sunDeg;
    in.moonElevationDeg = moonDeg;
    in.moonPhase = phase;
    in.nightEffect = 1.0f;
    in.skyThrough = 1.0f;
    return in;
}

float Share(const GateInputs& in, const LightingTuning& t = LightingTuning{})
{
    return MoonKeyShare(ComputeFeatureGates(in, t));
}

// The rig inputs of the same night: the sun and moon toward their elevations
// (the moon in the south-east), the classic night light.
LightRigInputs RigNight(float sunDeg, float moonDeg, float phase)
{
    LightRigInputs in;
    const float s = sunDeg * kPi / 180.0f, m = moonDeg * kPi / 180.0f;
    in.towardSun[0] = 0.0f;
    in.towardSun[1] = std::sin(s);
    in.towardSun[2] = -std::cos(s);
    in.towardMoon[0] = 0.6f * std::cos(m);
    in.towardMoon[1] = std::sin(m);
    in.towardMoon[2] = -0.8f * std::cos(m);
    in.moonPhaseLight = phase;
    in.classicSunDiffuse = Rgb(0.0f, 0.0f, 0.0f);
    in.classicAmbient = Rgb(0.03f, 0.03f, 0.04f);
    in.fogColor = Rgb(0.02f, 0.02f, 0.03f);
    in.skyThrough = 1.0f;
    in.nightEffect = 1.0f;
    return in;
}

bool SameRgb(const Rgb& a, const Rgb& b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

lt::FrameLight Lamp(float x, float y, float z, float start = 50.0f)
{
    lt::FrameLight l;
    l.kind = lt::FrameLightKind::Lamp;
    l.pos[0] = x;
    l.pos[1] = y;
    l.pos[2] = z;
    l.start = start;
    l.diffuse[0] = 0.9f;
    l.diffuse[1] = 0.8f;
    l.diffuse[2] = 0.6f;
    l.sortBrightness = 1.0f;
    return l;
}

lt::FrameLight Headlight(float x, float y, float z, const float dir[3])
{
    lt::FrameLight l = Lamp(x, y, z, 110.0f);
    l.kind = lt::FrameLightKind::Spot;
    const float n = std::sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
    for (int i = 0; i < 3; ++i)
        l.dir[i] = dir[i] / n;
    return l;
}

float Luma(const float c[3])
{
    return 0.2126f * c[0] + 0.7152f * c[1] + 0.0722f * c[2];
}

// The direction at `pitch` radians and heading 0 (+z).
void Dir(float pitch, float out[3])
{
    out[0] = 0.0f;
    out[1] = std::sin(pitch);
    out[2] = std::cos(pitch);
}

// Where the hot band's centre line of a lamp `h` metres above a road that
// rises at `slopeDeg` meets the road, in metres along it, for the aimed axis.
float HotBandReach(const float axis[3], float h, float slopeDeg)
{
    // The hot band: the axis turned kLowBeamHotBand down in its vertical plane.
    const float pitch = std::atan2(axis[1], std::sqrt(axis[0] * axis[0] + axis[2] * axis[2])) + lt::kLowBeamHotBand;
    // Road: y = tan(slope) * z; lamp at (0, h). Ray: y = h + tan(pitch) * z.
    const float ts = std::tan(slopeDeg * kPi / 180.0f), tp = std::tan(pitch);
    if (tp >= ts)
        return 1e9f; // the band never meets the road
    const float z = h / (ts - tp);
    return z / std::cos(slopeDeg * kPi / 180.0f);
}

std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path EngineDir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine";
}

std::string FunctionBody(const std::string& source, const std::string& signature)
{
    const size_t start = source.find(signature);
    if (start == std::string::npos)
        return {};
    const size_t end = source.find("\n}\n", start);
    return source.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

std::string Fixed(float v, int digits)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.*f", digits, static_cast<double>(v));
    std::string s = buf;
    // Trailing zeros off, one digit after the point kept (GLSL float literal).
    while (s.size() > 1 && s.back() == '0' && s[s.size() - 2] != '.')
        s.pop_back();
    return s;
}

} // namespace

TEST_CASE("Night: the moon key carries the moon only with the sun 3 deg under and a bright moon up",
          "[remaster][NightLights]")
{
    // Day, twilight, a low moon, a thin moon: the sun stays the key.
    GateInputs day = MoonNight(40.0f);
    day.nightEffect = 0.0f;
    CHECK(Share(day) == 0.0f);
    CHECK(Share(MoonNight(-2.0f)) == 0.0f);
    CHECK(Share(MoonNight(-30.0f, 2.0f)) == 0.0f);
    CHECK(Share(MoonNight(-30.0f, 36.0f, 0.2f)) == 0.0f);
    // A full moon high in a dark sky: the whole moon is the key.
    CHECK(Share(MoonNight()) == 1.0f);
    // Not without its switch, nor under night vision.
    CHECK(Share(MoonNight(), Apply("remastered,moonShadows=0")) == 0.0f);
    GateInputs nv = MoonNight();
    nv.nightVision = true;
    CHECK(Share(nv) == 0.0f);
    // It fades in over 3 degrees above each edge and over 0.1 of the phase, so
    // the shadows do not switch on: monotone, continuous at the edges.
    float prev = 0.0f;
    for (float m = 3.0f; m <= 7.0f; m += 0.25f)
    {
        const float s = Share(MoonNight(-30.0f, m));
        CHECK(s >= prev);
        CHECK(s - prev < 0.2f);
        prev = s;
    }
    CHECK(prev == 1.0f);
    CHECK(Share(MoonNight(-30.0f, 3.1f)) < 0.01f);
    CHECK(Share(MoonNight(-3.1f)) < 0.01f);
    CHECK(Share(MoonNight(-4.5f)) == Catch::Approx(0.5f).margin(1e-4f));
    CHECK(Share(MoonNight(-30.0f, 36.0f, 0.3f)) == Catch::Approx(0.5f).margin(1e-4f));
}

TEST_CASE("Night: the key light is the sun bit for bit unless the moon key is on", "[remaster][NightLights]")
{
    const LightingTuning t{};
    for (const float sunDeg : {50.0f, 8.0f, -1.0f, -30.0f})
    {
        INFO(sunDeg);
        const LightRigInputs in = RigNight(sunDeg, 36.0f, 1.0f);
        const LightRig before = ComputeLightRig(in, t);
        LightRig rig = before;
        const KeyLight key = ComputeKeyLight(in, rig, 0.0f);
        CHECK_FALSE(key.IsMoon());
        CHECK(SameRgb(key.radiance, before.sun));
        CHECK(key.elevationDeg == before.sunElevationDeg);
        for (int i = 0; i < 3; ++i)
            CHECK(key.toward[i] == in.towardSun[i]);
        // The rig is left alone: the moon keeps its shadowless term.
        CHECK(SameRgb(rig.moon, before.moon));
        CHECK(SameRgb(rig.sun, before.sun));
    }
}

TEST_CASE("Night: the moon key moves F9's moon into the key, nothing more", "[remaster][NightLights]")
{
    const LightingTuning t{};
    const LightRigInputs in = RigNight(-30.0f, 36.0f, 1.0f);
    const LightRig before = ComputeLightRig(in, t);
    // F9's moon is there to move (kMoonFull 0.02 at a full moon high up).
    REQUIRE(Luminance(before.moon) == Catch::Approx(0.02f).epsilon(0.02f));
    CHECK(Luminance(before.sun) == 0.0f); // the sun is fully set 3 deg under
    LightRig rig = before;
    const KeyLight key = ComputeKeyLight(in, rig, 1.0f);
    CHECK(key.IsMoon());
    for (int i = 0; i < 3; ++i)
        CHECK(key.toward[i] == in.towardMoon[i]);
    CHECK(key.elevationDeg == Catch::Approx(36.0f).margin(1e-3f));
    CHECK(SameRgb(key.radiance, before.moon));
    CHECK(Luminance(rig.moon) == 0.0f);
    // The moon's sky fill and everything else in the rig stay as F9 set them.
    CHECK(SameRgb(rig.skyZenith, before.skyZenith));
    CHECK(SameRgb(rig.skyHorizon, before.skyHorizon));
    CHECK(SameRgb(rig.skyGround, before.skyGround));
    // A partial share splits the moon between the key and the shadowless term.
    LightRig half = before;
    const KeyLight k2 = ComputeKeyLight(in, half, 0.25f);
    CHECK(Luminance(k2.radiance) + Luminance(half.moon) == Catch::Approx(Luminance(before.moon)).epsilon(1e-5f));
    CHECK(Luminance(k2.radiance) == Catch::Approx(0.25f * Luminance(before.moon)).epsilon(1e-5f));
}

TEST_CASE("Night: the sky stays on the sun under the moon key", "[remaster][NightLights]")
{
    // The sky table and its twilight glow, the fog toward the sun, the warm
    // side of the sky light and the lit clouds read the sun's own direction
    // and elevation (litSkySun): with the moon key on from 3 degrees under the
    // horizon and the sky table on down to 9, the dusk glow would otherwise
    // jump to the moon's side of the sky.
    const LightingTuning t{};
    for (const float sunDeg : {-4.5f, -7.8f, -30.0f})
    {
        INFO(sunDeg);
        const LightRigInputs in = RigNight(sunDeg, 20.0f, 1.0f);
        const LightRig before = ComputeLightRig(in, t);
        LightRig rig = before;
        const KeyLight key = ComputeKeyLight(in, rig, 1.0f);
        REQUIRE(key.IsMoon());
        for (int i = 0; i < 3; ++i)
        {
            CHECK(key.toward[i] == in.towardMoon[i]);
            CHECK(key.skySunToward[i] == in.towardSun[i]);
        }
        CHECK(key.skySunElevationDeg == before.sunElevationDeg);
    }
    // With the sun as the key the sky's sun is the key, bit for bit.
    for (const float sunDeg : {50.0f, 8.0f, -1.0f, -30.0f})
    {
        INFO(sunDeg);
        const LightRigInputs in = RigNight(sunDeg, 36.0f, 1.0f);
        LightRig rig = ComputeLightRig(in, t);
        const KeyLight key = ComputeKeyLight(in, rig, 0.0f);
        for (int i = 0; i < 3; ++i)
            CHECK(key.skySunToward[i] == key.toward[i]);
        CHECK(key.skySunElevationDeg == key.elevationDeg);
    }
    // The night slot's w carries the moon key's share whatever the lamps do
    // (the sky's sun radiance is 0 while it is above 0).
    const FeatureGates day = ComputeFeatureGates(GateInputs{}, t);
    const NightConstants n = ComputeNightConstants(t, day, false, 0.6f);
    CHECK(n.night[0] == 0.0f);
    CHECK(n.night[3] == 0.6f);
    CHECK(ComputeNightConstants(t, day, false, 1.5f).night[3] == 1.0f);
    // The backend uploads the sky's sun and the share beside the key.
    const std::string lighting = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_Lighting.cpp");
    const std::string frame = FunctionBody(lighting, "void EngineGL33::UploadLitFrameConstants()");
    CHECK(frame.find("SetSlot(LitConst::SkySun, key.skySunToward[0], key.skySunToward[1], key.skySunToward[2], "
                     "key.skySunElevationDeg);") != std::string::npos);
    CHECK(frame.find("rm::ComputeNightConstants(_lighting, _featureGates, _lightTilesActive, key.moonShare);") !=
          std::string::npos);
    // The sky's GLSL reads the sun only through skySunDir, skySunElevation and
    // skySunRadiance; the key slots stay with the surfaces' light.
    namespace gl = Poseidon::gl33lighting;
    const std::string sky = gl::RemasterFragmentPiece(gl::FragmentPiece::Sky);
    REQUIRE_FALSE(sky.empty());
    CHECK(sky.find("vec3 skySunDir() { return litSkySun.xyz; }") != std::string::npos);
    CHECK(sky.find("float skySunElevation() { return litSkySun.w; }") != std::string::npos);
    CHECK(sky.find("vec3 skySunRadiance() { return litNight.w > 0.0 ? vec3(0.0) : litSun.rgb; }") != std::string::npos);
    std::string code; // the piece without its comments
    for (size_t at = 0; at < sky.size();)
    {
        const size_t comment = sky.find("//", at);
        const size_t stop = comment == std::string::npos ? sky.size() : comment;
        code += sky.substr(at, stop - at);
        at = comment == std::string::npos ? sky.size() : sky.find('\n', comment);
    }
    size_t keyReads = 0;
    for (size_t p = code.find("litSun"); p != std::string::npos; p = code.find("litSun", p + 1))
        ++keyReads;
    CHECK(keyReads == 1); // litSun.rgb in skySunRadiance
    CHECK(code.find("litSunDir") == std::string::npos);
    // The crowns' low-sun factors follow the sun too (off and on alike at night).
    const std::string shade = gl::RemasterFragmentPiece(gl::FragmentPiece::Shade);
    CHECK(shade.find("float lowSun = 1.0 - smoothstep(12.0, 35.0, skySunElevation());") != std::string::npos);
}

TEST_CASE("Night: the backend's sun slots, hills and shadow pass follow the key light", "[remaster][NightLights]")
{
    const std::string lighting = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_Lighting.cpp");
    const std::string frame = FunctionBody(lighting, "void EngineGL33::UploadLitFrameConstants()");
    REQUIRE_FALSE(frame.empty());
    CHECK(frame.find("rm::ComputeKeyLight(in, rig, gatesNow ? rm::MoonKeyShare(_featureGates) : 0.0f)") !=
          std::string::npos);
    CHECK(frame.find("SetSlot(LitConst::SunDir, key.toward[0], key.toward[1], key.toward[2]") != std::string::npos);
    CHECK(frame.find("SetSlot(LitConst::Sun, key.radiance.r, key.radiance.g, key.radiance.b, key.elevationDeg);") !=
          std::string::npos);
    CHECK(frame.find("std::atan2(key.toward[0], key.toward[2])") != std::string::npos);
    // The moon slot carries what the key left of the moon (after the key).
    CHECK(frame.find("ComputeKeyLight") < frame.find("SetSlot(LitConst::Moon, rig.moon.r"));
    // The scene renders the shadow map at night only for the moon key, from the moon.
    const std::string draw = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Scene" / "SceneDraw.cpp");
    CHECK(draw.find("if (sunFactor > 0.01f || remasterSunUp || GEngine->RemasterMoonKey())") != std::string::npos);
    const std::string pass = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Scene" / "SceneShadowPass.cpp");
    CHECK(pass.find("sd = GEngine->RemasterMoonKey() ? _mainLight->MoonDirection() : _mainLight->SunDirection();") !=
          std::string::npos);
    // ... and asks the frame's gate, not a value a later pass may reset.
    const std::string key = FunctionBody(lighting, "bool EngineGL33::RemasterMoonKey() const");
    CHECK(key.find("rm::MoonKeyShare(_featureGates) > 0.0f") != std::string::npos);
    CHECK(key.find("_featureGates.frame == GetFrameCounter()") != std::string::npos);
}

TEST_CASE("Night: the moon's shadows take the stock filter, not the soft one", "[remaster][NightLights]")
{
    // The moon key opens the sun shadow gates; the soft filter (B1) stays off
    // there (a fiftieth of the noon sun shows no penumbra, and the soft taps
    // cost the night's world pass), the leaves keep their lift.
    const LightingTuning t{};
    const FeatureGates moon = ComputeFeatureGates(MoonNight(), t);
    REQUIRE(moon[Feature::SoftShadows].on);
    REQUIRE(std::string_view(moon[Feature::SoftShadows].reason) == "moon key");
    const SunShadowConstants c = ComputeSunShadowConstants(t, moon, false, 0u);
    CHECK(c.pcss[2] == 0.0f);
    CHECK(c.pcss[3] == t.leafBias);
    // By day the soft filter fades in from the gate's edge as before, and
    // forced on (gates=0) below the edge it runs whole, unless the moon is the
    // key.
    GateInputs low = MoonNight(5.0f, -20.0f);
    low.nightEffect = 0.0f;
    CHECK(ComputeSunShadowConstants(t, ComputeFeatureGates(low, t), false, 0u).pcss[2] == PcssStrength(5.0f));
    const LightingTuning forced = Apply("remastered,gates=0");
    low.sunElevationDeg = 3.0f;
    CHECK(ComputeSunShadowConstants(forced, ComputeFeatureGates(low, forced), false, 0u).pcss[2] == 1.0f);
    CHECK(ComputeSunShadowConstants(forced, ComputeFeatureGates(MoonNight(), forced), false, 0u).pcss[2] == 0.0f);
}

TEST_CASE("Night: a physical light reaches as far as it lights the darkest ground", "[remaster][NightLights]")
{
    const lt::PhysicalLightParams params;
    const lt::FrameLight lamp = Lamp(0.0f, 10.0f, 0.0f);
    // Where the lamp's intensity falls to kPhysicalMinIrradiance, inside the
    // classic reach of 10 start distances.
    const float reach = lt::PhysicalReach(lamp, params);
    CHECK(reach == Catch::Approx(std::sqrt(lt::kLampIntensityPerStart * 50.0f / lt::kPhysicalMinIrradiance)));
    CHECK(reach < lt::kRangeFactor * lamp.start);
    // A dim lamp keeps the classic reach as its limit.
    const lt::FrameLight tiny = Lamp(0.0f, 10.0f, 0.0f, 0.5f);
    CHECK(lt::PhysicalReach(tiny, params) == Catch::Approx(lt::kRangeFactor * 0.5f));
    // The tiles bin a physical light by it, a classic one by its classic reach.
    CHECK(lt::LightBounds(lamp, &params).r == Catch::Approx(reach));
    CHECK(lt::LightBounds(lamp).r == Catch::Approx(lt::kRangeFactor * lamp.start));
    lt::FrameLight fire = lamp;
    fire.kind = lt::FrameLightKind::Fire;
    CHECK(lt::LightBounds(fire, &params).r == Catch::Approx(lt::kRangeFactor * lamp.start));
    // Light ends there: nothing at the reach, about kPhysicalMinIrradiance at
    // half its square (the window's start), where the light is twice it.
    const float colourLuma =
        0.2126f * std::pow(0.9f, 2.2f) + 0.7152f * std::pow(0.8f, 2.2f) + 0.0722f * std::pow(0.6f, 2.2f);
    float e[3];
    const float atEnd[3] = {reach * 1.0001f, 10.0f, 0.0f};
    lt::PhysicalIrradiance(lamp, atEnd, params, e);
    CHECK(Luma(e) == 0.0f);
    const float atHalf[3] = {reach / std::sqrt(2.0f), 10.0f, 0.0f};
    lt::PhysicalIrradiance(lamp, atHalf, params, e);
    CHECK(Luma(e) / colourLuma == Catch::Approx(2.0f * lt::kPhysicalMinIrradiance).epsilon(0.01f));
    // A headlight reaches by its hot spot.
    float flat[3];
    Dir(0.0f, flat);
    CHECK(lt::PhysicalReach(Headlight(0.0f, 0.9f, 0.0f, flat), params) ==
          Catch::Approx(std::sqrt(lt::kLowBeamPeak / lt::kPhysicalMinIrradiance)));
    // The shaders end the light at the same reach, and the backend bins by it
    // with the physical model on.
    namespace gl = Poseidon::gl33lighting;
    const std::string lights = gl::RemasterFragmentPiece(gl::FragmentPiece::Lights);
    CHECK(lights.find("const float kNightMinIrradiance = " + Fixed(lt::kPhysicalMinIrradiance, 3) + ";") !=
          std::string::npos);
    CHECK(lights.find("float end2 = min(kReachFactor * kReachFactor * pos.w * pos.w, peak / kNightMinIrradiance);") !=
          std::string::npos);
    const std::string lighting = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_Lighting.cpp");
    CHECK(lighting.find("fl::BinLightsToTiles(t.lights, view, t.grid, aimBeams ? &physical : nullptr);") !=
          std::string::npos);
}

TEST_CASE("Night: the low beam is wide, has a hot band 1.7 deg down and a hard top edge", "[remaster][NightLights]")
{
    float beam[3];
    Dir(0.0f, beam);
    auto at = [&](float horizontal, float vertical)
    {
        // A direction `horizontal` radians to the right and `vertical` up.
        const float d[3] = {std::sin(horizontal) * std::cos(vertical), std::sin(vertical),
                            std::cos(horizontal) * std::cos(vertical)};
        return lt::LowBeamPattern(d, beam);
    };
    // The hot band's centre line straight ahead is the brightest point: the
    // band, the floor under it, the core and the spill at full.
    const float peak = at(0.0f, lt::kLowBeamHotBand);
    CHECK(peak == Catch::Approx((1.0f + lt::kLowBeamFloor) * (1.0f + lt::kLowBeamSpill)).epsilon(1e-5f));
    for (float v = -0.3f; v <= 0.1f; v += 0.005f)
        CHECK(at(0.0f, v) <= peak + 1e-5f);
    // The top edge: nothing above 1.43 deg (kLowBeamCutHigh), most of the light
    // within 2 deg under the axis.
    CHECK(at(0.0f, lt::kLowBeamCutHigh + 1e-4f) == 0.0f);
    CHECK(at(0.0f, 0.04f) == 0.0f);
    CHECK(at(0.0f, -0.035f) > 0.9f);
    CHECK(at(0.0f, -0.004f) > 0.4f);
    // From the axis to the cut the light falls by more than half within 1.7 deg.
    CHECK(at(0.0f, 0.015f) < 0.5f * at(0.0f, -0.015f));
    // Wide: the spill reaches 34 deg to either side, symmetric.
    CHECK(at(0.6f, lt::kLowBeamHotBand) > 0.05f);
    CHECK(at(0.6f, lt::kLowBeamHotBand) == Catch::Approx(at(-0.6f, lt::kLowBeamHotBand)).epsilon(1e-5f));
    CHECK(at(0.25f, lt::kLowBeamHotBand) > at(0.6f, lt::kLowBeamHotBand));
    // Nothing behind the lamp's plane (its own vehicle).
    CHECK(at(1.6f, 0.0f) == 0.0f);
    CHECK(at(3.0f, -0.3f) == 0.0f);
    // A faint floor under the hot band down to 34 deg, none below.
    CHECK(at(0.0f, -0.3f) > 0.0f);
    CHECK(at(0.0f, -0.3f) < 0.03f);
    CHECK(at(0.0f, -0.7f) < 1e-3f);
}

TEST_CASE("Night: a headlight is aimed from its body, 1% down, on flat and sloped roads", "[remaster][NightLights]")
{
    // Flat road, lamp 0.9 m up: the hot band reaches about 23 m.
    float flat[3];
    Dir(0.0f, flat);
    float axis[3];
    lt::LowBeamAim(flat, flat, axis);
    CHECK(std::sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]) == Catch::Approx(1.0f));
    CHECK(axis[1] / axis[2] == Catch::Approx(-lt::kLowBeamTilt).margin(1e-5f));
    const float onFlat = HotBandReach(axis, 0.9f, 0.0f);
    CHECK(onFlat > 18.0f);
    CHECK(onFlat < 28.0f);
    // The model's own beam axis can point degrees up or down (the research's
    // Jeep: 4.7 deg up on a 14 deg slope): only its heading counts.
    for (const float modelPitchDeg : {-9.0f, 4.7f, 20.0f})
    {
        float model[3];
        Dir(modelPitchDeg * kPi / 180.0f, model);
        float a[3];
        lt::LowBeamAim(model, flat, a);
        for (int i = 0; i < 3; ++i)
            CHECK(a[i] == Catch::Approx(axis[i]).margin(1e-6f));
    }
    // A steady 14 deg climb (the research's hill road), the body pitched with
    // it: the hot band lands the same distance up the road, not over it.
    for (const float slopeDeg : {14.0f, -14.0f, 6.0f})
    {
        INFO(slopeDeg);
        float body[3];
        Dir(slopeDeg * kPi / 180.0f, body);
        float a[3];
        lt::LowBeamAim(flat, body, a);
        const float reach = HotBandReach(a, 0.9f, slopeDeg);
        CHECK(reach > 18.0f);
        CHECK(reach < 28.0f);
    }
    // The heading is the lamp's: a reversing light points back, 1% down from
    // the body seen from behind (on a climb, down the slope behind).
    float back[3] = {0.0f, 0.0f, -1.0f};
    float a[3];
    lt::LowBeamAim(back, flat, a);
    CHECK(a[2] < -0.99f);
    CHECK(a[1] / -a[2] == Catch::Approx(-lt::kLowBeamTilt).margin(1e-5f));
    float climb[3];
    Dir(14.0f * kPi / 180.0f, climb);
    lt::LowBeamAim(back, climb, a);
    CHECK(a[1] / -a[2] == Catch::Approx(-std::tan(14.0f * kPi / 180.0f) - lt::kLowBeamTilt).margin(1e-5f));
}

TEST_CASE("Night: lamps and low beams fall off with the squared distance from the bulb", "[remaster][NightLights]")
{
    const lt::PhysicalLightParams params;
    auto light = [&](const lt::FrameLight& l, float x, float y, float z)
    {
        const float p[3] = {x, y, z};
        std::vector<float> out(3);
        lt::PhysicalIrradiance(l, p, params, out.data());
        return out;
    };
    const lt::FrameLight lamp = Lamp(0.0f, 10.0f, 0.0f);
    // Inverse square, softened by kPhysicalSoftening: no flat disc of full
    // light out to the 50 m start distance.
    const std::vector<float> under = light(lamp, 0.0f, 0.0f, 0.0f);
    const std::vector<float> twice = light(lamp, 0.0f, -10.0f, 0.0f);
    const float colourLuma =
        0.2126f * std::pow(0.9f, 2.2f) + 0.7152f * std::pow(0.8f, 2.2f) + 0.0722f * std::pow(0.6f, 2.2f);
    const float intensity = lt::kLampIntensityPerStart * 50.0f;
    CHECK(Luma(under.data()) ==
          Catch::Approx(intensity / (100.0f + lt::kPhysicalSoftening) * colourLuma).epsilon(1e-4f));
    CHECK(Luma(under.data()) / Luma(twice.data()) == Catch::Approx((400.0f + 0.25f) / (100.0f + 0.25f)).epsilon(1e-4f));
    // The authored colour, taken to linear (no warming).
    CHECK(under[0] / under[2] == Catch::Approx(std::pow(0.9f / 0.6f, 2.2f)).epsilon(1e-4f));
    // The intensity follows the configured brightness (start distance) linearly.
    CHECK(Luma(light(Lamp(0.0f, 10.0f, 0.0f, 100.0f), 0.0f, 0.0f, 0.0f).data()) ==
          Catch::Approx(2.0f * Luma(under.data())).epsilon(1e-4f));
    // Off its own housing and post, and gone at the classic reach.
    CHECK(Luma(light(lamp, 0.0f, 9.8f, 0.0f).data()) == 0.0f);
    CHECK(Luma(light(lamp, 0.0f, 10.0f, 10.0f * lamp.start).data()) == 0.0f);
    CHECK(Luma(light(lamp, 0.0f, 9.0f, 0.0f).data()) > 0.0f);
    // The other kinds keep their classic model: nothing here.
    lt::FrameLight fire = lamp;
    fire.kind = lt::FrameLightKind::Fire;
    CHECK(Luma(light(fire, 0.0f, 0.0f, 0.0f).data()) == 0.0f);
    CHECK(lt::UsesPhysicalModel(lt::FrameLightKind::Lamp));
    CHECK(lt::UsesPhysicalModel(lt::FrameLightKind::Spot));
    CHECK_FALSE(lt::UsesPhysicalModel(lt::FrameLightKind::Fire));
    CHECK_FALSE(lt::UsesPhysicalModel(lt::FrameLightKind::Combat));
    CHECK_FALSE(lt::UsesPhysicalModel(lt::FrameLightKind::Classic));

    // A headlight 0.9 m up, aimed on a flat road: on the hot band 20 m out it
    // gives the peak over the squared distance; nothing behind it.
    float flat[3];
    Dir(0.0f, flat);
    float axis[3];
    lt::LowBeamAim(flat, flat, axis);
    const lt::FrameLight spot = Headlight(0.0f, 0.9f, 0.0f, axis);
    const float bandPitch = std::atan2(axis[1], axis[2]) + lt::kLowBeamHotBand;
    const float d = 20.0f;
    const std::vector<float> onBand = light(spot, 0.0f, 0.9f + d * std::sin(bandPitch), d * std::cos(bandPitch));
    const float pattern = (1.0f + lt::kLowBeamFloor) * (1.0f + lt::kLowBeamSpill); // the band's centre line
    CHECK(Luma(onBand.data()) ==
          Catch::Approx(params.spotPeak * pattern / (d * d + lt::kPhysicalSoftening) * colourLuma).epsilon(0.02f));
    CHECK(Luma(light(spot, 0.0f, 0.5f, -3.0f).data()) == 0.0f);
    // A headlight is white at its colour's linear luminance: the configured
    // rosy beam turned the khaki grass of a verge orange-tan. A lamp keeps its
    // authored colour (F9b).
    CHECK(onBand[0] == onBand[1]);
    CHECK(onBand[1] == onBand[2]);
    float beamColour[3], lampColour[3];
    lt::PhysicalColour(spot, beamColour);
    lt::PhysicalColour(lamp, lampColour);
    CHECK(Luma(beamColour) == Catch::Approx(colourLuma).epsilon(1e-5f));
    CHECK(beamColour[0] == beamColour[2]);
    CHECK(lampColour[0] == Catch::Approx(std::pow(0.9f, 2.2f)).epsilon(1e-6f));
    CHECK(lampColour[2] == Catch::Approx(std::pow(0.6f, 2.2f)).epsilon(1e-6f));
    // Above the cut-off, nothing (a wall 20 m out above the lamp's height).
    CHECK(Luma(light(spot, 0.0f, 0.9f + d * std::tan(0.03f), d).data()) == 0.0f);
}

TEST_CASE("Night: smoke takes the same physical lamps and low beams", "[remaster][NightLights]")
{
    const lt::PhysicalLightParams params;
    std::vector<lt::FrameLight> lights = {Lamp(0.0f, 10.0f, 0.0f), Lamp(20.0f, 8.0f, 0.0f)};
    lt::FrameLight fire = Lamp(5.0f, 1.0f, 5.0f, 3.0f);
    fire.kind = lt::FrameLightKind::Fire;
    lights.push_back(fire);
    const float p[3] = {4.0f, 3.0f, 2.0f};
    float sum[3];
    lt::ParticleLight(lights, p, params, sum);
    float each[3] = {0.0f, 0.0f, 0.0f};
    for (const lt::FrameLight& l : lights)
    {
        float e[3];
        lt::PhysicalIrradiance(l, p, params, e);
        for (int i = 0; i < 3; ++i)
            each[i] += e[i];
    }
    for (int i = 0; i < 3; ++i)
        CHECK(sum[i] == Catch::Approx(each[i]).epsilon(1e-6f));
    CHECK(Luma(sum) > 0.0f);
    // The classic loop skips exactly these kinds while the physical model runs.
    const std::string object = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Scene" / "Object.cpp");
    const std::string decal = FunctionBody(object, "void Object::DrawDecal(");
    REQUIRE_FALSE(decal.empty());
    CHECK(decal.find("const bool night = GEngine->ParticleNightLight(pos.Position(), physical);") != std::string::npos);
    CHECK(decal.find("if (night && IsPhysicalNightLight(*lights[index]))") != std::string::npos);
    CHECK(decal.find("colorI = AddParticleNightLight(colorI, physical, addLightsFactor);") != std::string::npos);
    CHECK(object.find("return lighting::UsesPhysicalModel(desc.type == LTSpotLight ? lighting::FrameLightKind::Spot "
                      ": light.Kind());") != std::string::npos);
}

TEST_CASE("Night: the night slot is on only with its gate and the tiled list", "[remaster][NightLights]")
{
    GateInputs night = MoonNight(-30.0f, -10.0f);
    night.localLights = 2;
    const LightingTuning t{};
    const FeatureGates g = ComputeFeatureGates(night, t);
    REQUIRE(g[Feature::NightLights].on);
    const NightConstants on = ComputeNightConstants(t, g, true);
    CHECK(on.night[0] == 1.0f);
    CHECK(on.night[1] == lt::kLampIntensityPerStart); // nightLamp 1 shipped
    CHECK(on.night[2] == lt::kLowBeamPeak);           // nightSpot 1 shipped
    CHECK(on.night[3] == 0.0f);
    // Off: zero, which the shader reads as "every light classic".
    GateInputs day = MoonNight(40.0f);
    day.nightEffect = 0.0f;
    const FeatureGates switchedOff = ComputeFeatureGates(night, Apply("remastered,nightLights=0"));
    for (const NightConstants& off : {ComputeNightConstants(t, g, false), ComputeNightConstants(t, switchedOff, true),
                                      ComputeNightConstants(t, ComputeFeatureGates(day, t), true)})
    {
        for (float v : off.night)
            CHECK(v == 0.0f);
    }
    CHECK(ComputeNightConstants(Apply("remastered,nightSpot=0"), g, true).night[2] == 0.0f);
    CHECK(ComputeNightConstants(Apply("remastered,nightLamp=0"), g, true).night[1] == 0.0f);
    CHECK(ComputeNightConstants(Apply("remastered,nightLamp=2"), g, true).night[1] ==
          2.0f * lt::kLampIntensityPerStart);
    CHECK(ComputeNightConstants(Apply("remastered,nightSpot=3"), g, true).night[2] == lt::kLowBeamPeak * 3.0f);
}

TEST_CASE("Night: the shaders' physical model is FrameLights.hpp's", "[Graphics][Shaders][remaster][NightLights]")
{
    namespace gl = Poseidon::gl33lighting;
    const std::string lights = gl::RemasterFragmentPiece(gl::FragmentPiece::Lights);
    REQUIRE_FALSE(lights.empty());
    struct Pin
    {
        const char* name;
        float value;
        int digits;
    };
    for (const Pin& p :
         {Pin{"kNightSoftening", lt::kPhysicalSoftening, 2}, Pin{"kNightNearStart", lt::kPhysicalNearStart, 2},
          Pin{"kNightNearEnd", lt::kPhysicalNearEnd, 1}, Pin{"kBeamCutLow", lt::kLowBeamCutLow, 3},
          Pin{"kBeamCutHigh", lt::kLowBeamCutHigh, 3}, Pin{"kBeamHotBand", lt::kLowBeamHotBand, 2},
          Pin{"kBeamHotWidth", lt::kLowBeamHotWidth, 2}, Pin{"kBeamFloor", lt::kLowBeamFloor, 2},
          Pin{"kBeamFloorEnd", lt::kLowBeamFloorEnd, 1}, Pin{"kBeamFloorStart", lt::kLowBeamFloorStart, 1},
          Pin{"kBeamCoreWidth", lt::kLowBeamCoreWidth, 2}, Pin{"kBeamSpill", lt::kLowBeamSpill, 2},
          Pin{"kBeamSpillWidth", lt::kLowBeamSpillWidth, 1}})
    {
        const std::string decl = std::string("const float ") + p.name + " = " + Fixed(p.value, p.digits) + ";";
        INFO(decl);
        CHECK(lights.find(decl) != std::string::npos);
    }
    // The model: intensity over the softened squared distance, the near fade,
    // the reach window, lamps by their start distance and headlights by the
    // pattern times the slot's peak, the configured colour taken to linear.
    CHECK(lights.find("float peak = spot ? litNight.z : litNight.y * pos.w;") != std::string::npos);
    CHECK(lights.find("float I = spot ? peak * lowBeamPattern(-L, frameLights[at + 3].xyz) : peak;") !=
          std::string::npos);
    CHECK(lights.find("I *= (1.0 - smoothstep(0.5 * end2, end2, d2)) * smoothstep(kNightNearStart, kNightNearEnd, "
                      "d);") != std::string::npos);
    CHECK(lights.find("float E = I / (d2 + kNightSoftening);") != std::string::npos);
    // A lamp's colour taken to linear, a headlight's white at its luminance.
    const size_t colour = lights.find("vec3 col = toLinear(color.rgb);");
    REQUIRE(colour != std::string::npos);
    const size_t white = lights.find("if (spot) col = vec3(dot(col, kLuma));", colour);
    CHECK(white != std::string::npos);
    CHECK(white < lights.find("col *= E;", colour));
    CHECK(lights.find("col *= E;", colour) < lights.find("if (g_lightLeaves) {", colour));
    // Leaves wrap a lamp's light as the sun's (the black crowns by the lamps).
    CHECK(lights.find("float wrap = clamp((dot(g_lightLeafNormal, L) + 0.35) / 1.35, 0.0, 1.0);") != std::string::npos);
    // Lamps and headlights by kind (FrameLightKind Lamp 1, Spot 2), only while on.
    CHECK(static_cast<int>(lt::FrameLightKind::Lamp) == 1);
    CHECK(static_cast<int>(lt::FrameLightKind::Spot) == 2);
    CHECK(lights.find("return litNight.x > 0.5 && kind > 0.5 && kind < 2.5;") != std::string::npos);
    // They leave the classic budget, and their sum takes the night factor.
    CHECK(lights.find("if (physicalKind(id)) physicalLight(id, n, kd, ka);") != std::string::npos);
    CHECK(lights.find("g_physicalLocal *= litLightCount.z;") != std::string::npos);
    // The shade adds them outside the classic clamp, and hands the leaves'
    // normal over.
    const std::string shade = gl::RemasterFragmentPiece(gl::FragmentPiece::Shade);
    CHECK(shade.find("vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.0))) + g_physicalLocal * "
                     "litGround.w;") != std::string::npos);
    CHECK(shade.find("g_lightLeafNormal = nLight;") != std::string::npos);
    CHECK(shade.find("g_physicalSpec * litGround.w") != std::string::npos);
}

TEST_CASE("Night: the sea takes the physical lamps and low beams", "[Graphics][Shaders][remaster][NightLights]")
{
    // While the model is on, localLights() hands lamps and headlights to
    // physicalLight() and leaves them out of its classic sum, so a caller that
    // drops g_physicalLocal loses them: with nightLights on, a harbour lamp or
    // a headlight would light the B5 sea not at all, where the classic model
    // lit it with the switch off.
    namespace gl = Poseidon::gl33lighting;
    const std::string water = gl::RemasterFragmentMain(3); // PSWater
    REQUIRE_FALSE(water.empty());
    const std::string sea =
        FunctionBody(water, "vec3 seaBody(vec3 albedo, out float ambientShare, out float sunVis) {");
    REQUIRE_FALSE(sea.empty());
    // The view direction for the rough diffuse and the gleam, set before the
    // lights are summed; the physical light outside the classic clamp; the
    // gleam on the radiance, as in shadeRemastered.
    const size_t view = sea.find("g_lightView = -vWorldRel * inversesqrt(max(dot(vWorldRel, vWorldRel), 1e-12));");
    const size_t local = sea.find("vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.0))) + "
                                  "g_physicalLocal * litGround.w;");
    CHECK(view != std::string::npos);
    CHECK(local != std::string::npos);
    CHECK(view < local);
    const std::string radiance = "vec3 radiance = (a * (direct + ambient + moon + local + emissive) + "
                                 "g_physicalSpec * litGround.w) * exposure;";
    CHECK(sea.find(radiance) != std::string::npos);

    // Every call of localLights() in the remastered fragment programs adds the
    // physical sum on the same line (today shadeRemastered and seaBody). The
    // pixel shader ids end at NPixelShaders (6); past them the table answers
    // nullptr.
    std::vector<std::string> sources{gl::RemasterFragmentCommon()};
    for (int ps = 0; ps < 16; ++ps)
        if (const char* main = gl::RemasterFragmentMain(ps))
            sources.emplace_back(main);
    int calls = 0;
    for (const std::string& src : sources)
    {
        for (size_t at = src.find("localLights("); at != std::string::npos; at = src.find("localLights(", at + 1))
        {
            if (at >= 5 && src.compare(at - 5, 5, "vec3 ") == 0)
                continue; // the definition
            const size_t begin = src.rfind('\n', at);
            const std::string line = src.substr(begin + 1, src.find('\n', at) - begin - 1);
            INFO(line);
            CHECK(line.find("+ g_physicalLocal * litGround.w") != std::string::npos);
            ++calls;
        }
    }
    // shadeRemastered once in the prelude, seaBody once in the water program.
    CHECK(calls == 2);
}

TEST_CASE("Night: the backend aims the low beams only with the physical model on", "[remaster][NightLights]")
{
    const std::string lighting = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_Lighting.cpp");
    const std::string tiles = FunctionBody(lighting, "void EngineGL33::UpdateLightTiles()");
    REQUIRE_FALSE(tiles.empty());
    CHECK(tiles.find("_featureGates[rm::Feature::NightLights].on") != std::string::npos);
    CHECK(tiles.find("if (spot && aimBeams)") != std::string::npos);
    CHECK(tiles.find("fl::LowBeamAim(lampDir, bodyForward, f.dir);") != std::string::npos);
    const std::string frame = FunctionBody(lighting, "void EngineGL33::UploadLitFrameConstants()");
    CHECK(frame.find("rm::ComputeNightConstants(_lighting, _featureGates, _lightTilesActive, key.moonShare)") !=
          std::string::npos);
    CHECK(frame.find("SetSlot(LitConst::Night, night.night[0], night.night[1], night.night[2], night.night[3]);") !=
          std::string::npos);
    const std::string post = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_PostFX.cpp");
    CHECK(FunctionBody(post, "void EngineGL33::EndWorldScene()").find("_nightLightsActive = false;") !=
          std::string::npos);
}

TEST_CASE("Night: a star is a soft splat with a faint colour of its own", "[remaster][NightLights]")
{
    const StarSplat s = MakeStarSplat(100.25f, 50.5f, 100.0f, 7u);
    CHECK(s.x[0] == 100.25f);
    CHECK(s.y[0] == 50.5f);
    CHECK(s.alpha[0] == Catch::Approx(100.0f * kStarCentreGain));
    for (int c = 1; c < 5; ++c)
    {
        CHECK(s.alpha[c] == 0.0f);
        CHECK(std::fabs(s.x[c] - s.x[0]) == Catch::Approx(kStarSplatRadius));
        CHECK(std::fabs(s.y[c] - s.y[0]) == Catch::Approx(kStarSplatRadius));
    }
    // The four corners go round (a fan: 1, 2, 3, 4 and back to 1).
    CHECK(s.x[1] < s.x[2]);
    CHECK(s.y[2] < s.y[3]);
    CHECK(s.x[3] > s.x[4]);
    CHECK(s.y[4] > s.y[1]);
    // A bright star's centre saturates instead of wrapping.
    CHECK(MakeStarSplat(0.0f, 0.0f, 250.0f, 1u).alpha[0] == 255.0f);
    // The tint: unit luminance, between a cool and a warm white, the same for
    // the same star, different across stars.
    std::set<float> reds;
    for (std::uint32_t i = 0; i < 64; ++i)
    {
        const StarSplat t = MakeStarSplat(0.0f, 0.0f, 50.0f, i);
        CHECK(Luma(t.tint) == Catch::Approx(1.0f).epsilon(1e-5f));
        CHECK(t.tint[2] / t.tint[0] >= 0.74f - 1e-4f);
        CHECK(t.tint[2] / t.tint[0] <= 1.25f + 1e-4f);
        CHECK(MakeStarSplat(9.0f, 9.0f, 50.0f, i).tint[0] == t.tint[0]);
        reds.insert(t.tint[0]);
    }
    CHECK(reds.size() > 32);
    // The backend draws it only with the stars gate, and keeps the dot without.
    const std::string shared = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_DrawShared.cpp");
    const std::string points = FunctionBody(shared, "void EngineGL33::DrawPoints(int beg, int end)");
    REQUIRE_FALSE(points.empty());
    CHECK(points.find("_featureGates[Poseidon::remaster::Feature::Stars].on") != std::string::npos);
    CHECK(points.find("if (splats)") < points.find("TLVertex vs[4];"));
}

TEST_CASE("Night: the three switches have landed, with the lamp and headlight strengths", "[remaster][NightLights]")
{
    for (Feature f : {Feature::NightLights, Feature::MoonShadows, Feature::Stars})
    {
        const FeatureInfo& info = GetFeatureInfo(f);
        INFO(info.key);
        CHECK(info.landed);
        CHECK(std::string_view(info.step) == "B3");
        CHECK(std::string_view(info.track) == "night");
        CHECK(std::string_view(info.area) == "Night and combat");
    }
    struct Sub
    {
        const char* key;
        float value;
    };
    for (const Sub& sub : {Sub{"nightSpot", 1.0f}, Sub{"nightLamp", 1.0f}})
    {
        INFO(sub.key);
        const LightingKey* k = FindLightingKey(sub.key);
        REQUIRE(k != nullptr);
        CHECK(k->kind == KeyKind::Knob);
        REQUIRE(k->parent != nullptr);
        CHECK(std::string_view(k->parent) == "nightLights");
        CHECK(std::string_view(k->group) == "night");
        CHECK(KeyDefault(*k) == sub.value);
        CHECK(GetKeyValue(Apply(std::string("remastered,") + sub.key + "=0.5"), *k) == 0.5f);
        CHECK(GetKeyValue(Apply(std::string(sub.key) + "=0.5,baseline"), *k) == sub.value);
    }
}
