#include <catch2/catch_test_macros.hpp>
#include <Evaluator/express.hpp>
#include <Poseidon/Dev/Diag/SceneSettle.hpp>
#include <Poseidon/Graphics/Dummy/EngineDummy.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <string>

// The verbs and helpers the lighting capture harness (tests/rendering/captures)
// relies on for trustworthy A/B pictures: triSetLighting must put every knob
// back to its default when it switches mode and must apply a comma list whole
// or not at all, and SceneSettle must not call a still-loading view settled.

using namespace Poseidon;

GameValue TriSetLighting(const GameState*, GameValuePar);

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

    void SetLightingTuning(const remaster::LightingTuning& t) override { tuning = t; }
    remaster::LightingTuning GetLightingTuning() const override { return tuning; }
    bool LightingRemasterActive() const override { return tuning.remastered; }
    void SetShadowMapsEnabled(bool enabled) override { shadowMaps = enabled; }
    bool ShadowMapsEnabled() const override { return shadowMaps; }
};

struct GEngineGuard
{
    Engine* prev;
    explicit GEngineGuard(Engine* tmp) : prev(GEngine) { GEngine = tmp; }
    ~GEngineGuard() { GEngine = prev; }
};

std::string Set(const char* setting)
{
    const GameValue v = TriSetLighting(nullptr, GameValue(setting));
    return std::string(((RString)(GameStringType)v).Data());
}

// Every knob of `t` is at its shipped default (the mode flag aside).
bool AllKnobsDefault(const remaster::LightingTuning& t)
{
    const remaster::LightingTuning d{};
    return t.postEffects == d.postEffects && t.exposure == d.exposure && t.saturation == d.saturation &&
           t.sun == d.sun && t.sky == d.sky && t.bounce == d.bounce && t.warmth == d.warmth &&
           t.terrainShadows == d.terrainShadows && t.terrainOcclusion == d.terrainOcclusion &&
           t.cloudShadows == d.cloudShadows && t.specular == d.specular && t.localLights == d.localLights &&
           t.moonlight == d.moonlight && t.ambientOcclusion == d.ambientOcclusion && t.bloom == d.bloom &&
           t.sunShafts == d.sunShafts && t.shadowSoftness == d.shadowSoftness && t.debugView == d.debugView;
}

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
    REQUIRE(eng.tuning.bounce == 2.5f);

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
    // The research run's sunrise_ridge: the first shot drew 1,465 calls while
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
