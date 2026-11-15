#include <catch2/catch_test_macros.hpp>
#include <Evaluator/express.hpp>
#include <Poseidon/Core/Game/GameLoop.hpp>
#include <Poseidon/Dev/Diag/SceneSettle.hpp>
#include <Poseidon/Graphics/Dummy/EngineDummy.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <string>

// The verbs and helpers the lighting capture harness (tests/rendering/captures)
// relies on for trustworthy A/B pictures: triSetLighting must put every knob
// back to its default when it switches mode and must apply a comma list whole
// or not at all, "baseline" must turn every feature switch off (an honest
// "off" for before and after pictures), triSetFixedStep must hold the simulation step,
// and SceneSettle must not call a still-loading view settled.

using namespace Poseidon;

GameValue TriSetLighting(const GameState*, GameValuePar);
GameValue TriFeatureState(const GameState*);
GameValue TriSetFixedStep(const GameState*, GameValuePar);
GameValue TriPinCloudDrift(const GameState*, GameValuePar);

namespace
{

struct InitFixture
{
    InitFixture()
    {
        static bool done = false;
        if (!done)
        {
            GGameState.Init();
            done = true;
        }
    }
};

// Keeps the tuning the verb hands over, as the GL33 backend does, and reports
// the remastered shaders as available.
class LightingEngine : public EngineDummy
{
  public:
    remaster::LightingTuning tuning;
    bool shadowMaps = false;
    bool cloudsPinned = false;
    float drift[2] = {1830.0f, 970.0f};

    void SetLightingTuning(const remaster::LightingTuning& t) override { tuning = t; }
    remaster::LightingTuning GetLightingTuning() const override { return tuning; }
    bool LightingRemasterActive() const override { return tuning.remastered; }
    void SetShadowMapsEnabled(bool enabled) override { shadowMaps = enabled; }
    bool ShadowMapsEnabled() const override { return shadowMaps; }
    void PinCloudDrift(bool pinned, float x, float z) override
    {
        cloudsPinned = pinned;
        if (pinned)
        {
            drift[0] = x;
            drift[1] = z;
        }
    }
    bool GetCloudDrift(float& x, float& z) const override
    {
        x = drift[0];
        z = drift[1];
        return true;
    }
};

struct GEngineGuard
{
    Engine* prev;
    explicit GEngineGuard(Engine* tmp) : prev(GEngine) { GEngine = tmp; }
    ~GEngineGuard() { GEngine = prev; }
};

std::string Text(const GameValue& v)
{
    return std::string(((RString)(GameStringType)v).Data());
}

std::string Set(const char* setting)
{
    return Text(TriSetLighting(nullptr, GameValue(setting)));
}

// Every key of the registry (knobs and feature switches) is at its shipped
// default; the mode flag aside.
bool AllKnobsDefault(const remaster::LightingTuning& t)
{
    for (int i = 0; i < remaster::LightingKeyCount(); ++i)
    {
        const remaster::LightingKey& k = remaster::LightingKeyAt(i);
        if (remaster::GetKeyValue(t, k) != remaster::KeyDefault(k))
            return false;
    }
    return true;
}

// Restores real time after a triSetFixedStep case.
struct FixedStepGuard
{
    ~FixedStepGuard() { Poseidon::SetFixedSimStep(0.0f); }
};

} // namespace

TEST_CASE("triSetLighting mode words reset every knob to its default", "[tri][triSetLighting][remaster]")
{
    InitFixture fix;
    LightingEngine eng;
    GEngineGuard guard(&eng);

    REQUIRE(Set("remastered") == "OK");
    REQUIRE(Set("bounce=2.5") == "OK");
    REQUIRE(Set("saturation=1.4") == "OK");
    REQUIRE(Set("post=0") == "OK");
    REQUIRE(Set("debug=3") == "OK");
    REQUIRE(Set("gates=0,taa=0,combatLights=0") == "OK");
    REQUIRE(eng.tuning.bounce == 2.5f);
    REQUIRE(eng.tuning.features.taa == 0.0f);

    SECTION("remastered")
    {
        REQUIRE(Set("remastered") == "OK");
        CHECK(eng.tuning.remastered);
        CHECK(eng.shadowMaps);
        CHECK(AllKnobsDefault(eng.tuning));
    }
    SECTION("classic")
    {
        REQUIRE(Set("classic") == "OK");
        CHECK_FALSE(eng.tuning.remastered);
        CHECK_FALSE(eng.shadowMaps);
        CHECK(AllKnobsDefault(eng.tuning));
    }
}

TEST_CASE("triSetLighting baseline turns every feature switch off and remastered turns them back on",
          "[tri][triSetLighting][remaster]")
{
    InitFixture fix;
    LightingEngine eng;
    GEngineGuard guard(&eng);

    REQUIRE(Set("remastered,bounce=2.5") == "OK");
    REQUIRE(Set("baseline") == "OK");
    CHECK(eng.tuning.remastered);
    CHECK(eng.shadowMaps);
    for (int i = 0; i < remaster::LightingKeyCount(); ++i)
    {
        const remaster::LightingKey& k = remaster::LightingKeyAt(i);
        INFO(k.key);
        CHECK(remaster::GetKeyValue(eng.tuning, k) ==
              (k.kind == remaster::KeyKind::Switch ? 0.0f : remaster::KeyDefault(k)));
    }
    REQUIRE(Set("remastered") == "OK");
    CHECK(AllKnobsDefault(eng.tuning));
    for (int i = 0; i < remaster::kFeatureCount; ++i)
        CHECK(remaster::FeatureOn(eng.tuning.features, static_cast<remaster::Feature>(i)));
}

TEST_CASE("triSetLighting takes every registry key, so <key>=0 is every feature's off",
          "[tri][triSetLighting][remaster]")
{
    InitFixture fix;
    LightingEngine eng;
    GEngineGuard guard(&eng);

    for (int i = 0; i < remaster::LightingKeyCount(); ++i)
    {
        const remaster::LightingKey& k = remaster::LightingKeyAt(i);
        const std::string setting = std::string("remastered,") + k.key + "=0";
        INFO(setting);
        REQUIRE(Set(setting.c_str()) == "OK");
        CHECK(remaster::GetKeyValue(eng.tuning, k) == 0.0f);
        // The A/B off of a feature changes that key only.
        for (int j = 0; j < remaster::LightingKeyCount(); ++j)
        {
            if (j == i)
                continue;
            const remaster::LightingKey& other = remaster::LightingKeyAt(j);
            INFO(other.key);
            CHECK(remaster::GetKeyValue(eng.tuning, other) == remaster::KeyDefault(other));
        }
    }
}

TEST_CASE("triFeatureState reports the switches without a ';'", "[tri][triSetLighting][remaster]")
{
    InitFixture fix;
    LightingEngine eng;
    GEngineGuard guard(&eng);

    REQUIRE(Set("remastered,ssr=0") == "OK");
    const std::string s = Text(TriFeatureState(nullptr));
    INFO(s);
    // The dummy backend computes no gates: computed=0, every gate off.
    CHECK(s.rfind("mode=remastered gates=1 computed=0", 0) == 0);
    CHECK(s.find(" ssr=0:off") != std::string::npos);
    CHECK(s.find(" taa=1:off") != std::string::npos);
    CHECK(s.find(';') == std::string::npos);
}

TEST_CASE("triSetFixedStep holds the simulation step until 0 returns to real time", "[tri][triSetFixedStep]")
{
    InitFixture fix;
    FixedStepGuard restore;

    CHECK(Poseidon::FixedSimStep() == 0.0f);
    CHECK(Poseidon::SimulationDelta(0.005f) == 0.005f);
    CHECK(Poseidon::SimulationDelta(1.0f) == Poseidon::kMaxSimStep);

    CHECK(Text(TriSetFixedStep(nullptr, GameValue(0.0333f))) == "OK:0.0333");
    CHECK(Poseidon::FixedSimStep() == 0.0333f);
    CHECK(Poseidon::SimulationDelta(0.005f) == 0.0333f);
    CHECK(Poseidon::SimulationDelta(0.25f) == 0.0333f);

    // Out of range: refused, the step stays.
    CHECK(Text(TriSetFixedStep(nullptr, GameValue(-1.0f))).rfind("FAIL:", 0) == 0);
    CHECK(Text(TriSetFixedStep(nullptr, GameValue(0.5f))).rfind("FAIL:", 0) == 0);
    CHECK(Poseidon::FixedSimStep() == 0.0333f);

    CHECK(Text(TriSetFixedStep(nullptr, GameValue(0.0f))) == "OK:0");
    CHECK(Poseidon::FixedSimStep() == 0.0f);
    CHECK(Poseidon::SimulationDelta(0.005f) == 0.005f);
}

TEST_CASE("triPinCloudDrift holds the cloud field until [] releases it", "[tri][triPinCloudDrift]")
{
    InitFixture fix;
    LightingEngine eng;
    GEngineGuard guard(&eng);

    auto pin = [](std::initializer_list<float> v)
    {
        GameArrayType a;
        for (float f : v)
            a.Add(GameValue(f));
        return Text(TriPinCloudDrift(nullptr, GameValue(a)));
    };
    // No scene in the unit test: the sky clouds report -1.
    CHECK(pin({100.0f, -50.0f}) == "OK:pinned 100.00 -50.00 sky -1.000");
    CHECK(eng.cloudsPinned);
    CHECK(eng.drift[0] == 100.0f);
    CHECK(pin({}) == "OK:released 100.00 -50.00 sky -1.000");
    CHECK_FALSE(eng.cloudsPinned);
    CHECK(pin({1.0f}).rfind("FAIL:", 0) == 0);
    CHECK(Text(TriPinCloudDrift(nullptr, GameValue(1.0f))).rfind("FAIL:", 0) == 0);
    CHECK_FALSE(eng.cloudsPinned);
}

TEST_CASE("triSetLighting applies every item of a comma list", "[tri][triSetLighting][remaster]")
{
    InitFixture fix;
    LightingEngine eng;
    GEngineGuard guard(&eng);

    REQUIRE(Set("remastered") == "OK");
    REQUIRE(Set("bounce=2.5, warmth=1.5") == "OK");
    CHECK(eng.tuning.bounce == 2.5f);
    CHECK(eng.tuning.warmth == 1.5f);

    // A mode word inside the list resets what came before it.
    REQUIRE(Set("sky=1.8,remastered,sun=1.2") == "OK");
    CHECK(eng.tuning.sky == remaster::LightingTuning{}.sky);
    CHECK(eng.tuning.bounce == remaster::LightingTuning{}.bounce);
    CHECK(eng.tuning.sun == 1.2f);
    CHECK(eng.tuning.remastered);
}

TEST_CASE("triSetLighting refuses a bad item and applies nothing", "[tri][triSetLighting][remaster]")
{
    InitFixture fix;
    LightingEngine eng;
    GEngineGuard guard(&eng);
    REQUIRE(Set("remastered") == "OK");

    CHECK(Set("bounce=2.0,nonsense=1").rfind("FAIL:unknown knob nonsense", 0) == 0);
    CHECK(Set("bounce=abc").rfind("FAIL:bad value", 0) == 0);
    CHECK(Set("bounce=2.0x").rfind("FAIL:bad value", 0) == 0);
    CHECK(Set("bounce").rfind("FAIL:expected", 0) == 0);
    CHECK(Set("").rfind("FAIL:expected", 0) == 0);
    CHECK(eng.tuning.remastered);
    CHECK(AllKnobsDefault(eng.tuning));
}

TEST_CASE("SceneSettle waits for a full window of steady frames", "[SceneSettle][tri]")
{
    Dev::SceneSettle s(5, 0.02f);
    for (int i = 0; i < 4; ++i)
        s.Add(1000, 0.5f, false);
    CHECK_FALSE(s.Settled());
    s.Add(1000, 0.5f, false);
    CHECK(s.Settled());
    CHECK(s.Frames() == 5);
}

TEST_CASE("SceneSettle does not settle while the draw count is still climbing", "[SceneSettle][tri]")
{
    // A sunrise view over a ridge: the first shot drew 1,465 calls while
    // the scene went on to draw about 9,500.
    Dev::SceneSettle s(5, 0.02f);
    for (int draws = 1465; draws < 9500; draws += 800)
    {
        s.Add(draws, 0.5f, false);
        CHECK_FALSE(s.Settled());
    }
    for (int i = 0; i < 5; ++i)
        s.Add(9500 + (i % 2) * 100, 0.5f, false); // 1% flicker, inside the 2% tolerance
    CHECK(s.Settled());
    CHECK(s.MinDraws() == 9500);
    CHECK(s.MaxDraws() == 9600);
}

TEST_CASE("SceneSettle tolerance is a fraction of the largest count", "[SceneSettle][tri]")
{
    Dev::SceneSettle tight(3, 0.02f);
    Dev::SceneSettle exact(3, 0.0f);
    for (int draws : {1000, 1030, 1000})
    {
        tight.Add(draws, 0.5f, false);
        exact.Add(draws, 0.5f, false);
    }
    CHECK_FALSE(tight.Settled()); // 30 of 1030 is 2.9%
    CHECK_FALSE(exact.Settled());
    for (int i = 0; i < 3; ++i)
        exact.Add(0, 0.5f, false); // nothing drawn at all is steady too
    CHECK(exact.Settled());
}

TEST_CASE("SceneSettle restarts on a LOD change or a busy frame", "[SceneSettle][tri]")
{
    Dev::SceneSettle s(3, 0.02f);
    s.Add(1000, 0.5f, false);
    s.Add(1000, 0.5f, false);
    s.Add(1000, 0.4f, false); // the adaptive detail moved
    CHECK_FALSE(s.Settled());
    s.Add(1000, 0.4f, false);
    s.Add(1000, 0.4f, true); // horizon map still building
    CHECK_FALSE(s.Settled());
    s.Add(1000, 0.4f, false);
    s.Add(1000, 0.4f, false);
    CHECK_FALSE(s.Settled());
    s.Add(1000, 0.4f, false);
    CHECK(s.Settled());
}
