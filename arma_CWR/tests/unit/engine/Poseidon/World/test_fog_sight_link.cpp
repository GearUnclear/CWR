// AI sight and the drawn fog (Graphics/Remaster/VolumetricFog.hpp, AI sight;
// World/Terrain/TerrainLighting.hpp SimulateFogSight), driven through the
// call the engine makes once per simulation step (World::SimulateLandscape ->
// SimulateFogSight(GLOB_LAND)) on a real Landscape, with an engine that
// reports each render path state in turn. The pure helpers' cases live in
// Graphics/test_volumetric_fog.cpp ([aisight]); these fail if any render path
// state (the fog's gates, an interior view, the fog's dev and quality knobs,
// what the renderer drew, a missing renderer) reaches AI sight, if AI sight
// stops being the whole drawn medium, if the Lighting row's Classic setting,
// the classic shaders or the post effects off on a machine that applied the
// Options stop reducing it to the classic fog, or if the call order the link
// relies on breaks.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Core/Application.hpp>
#include <Poseidon/Core/Config/EngineConfig.hpp>
#include <Poseidon/Core/Game/GameLoop.hpp>
#include <Poseidon/Graphics/Dummy/EngineDummy.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <Poseidon/Graphics/Remaster/VolumetricFog.hpp>
#include <Poseidon/UI/Settings/GraphicsApply.hpp>
#include <Poseidon/UI/Settings/GraphicsConfig.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/World/Terrain/TerrainLighting.hpp>

#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

using namespace Poseidon;
namespace rm = Poseidon::remaster;

// No caller may hold a reference into the state the next step rewrites.
static_assert(!std::is_reference_v<decltype(rm::FogSightMedium())>, "FogSightMedium hands out a copy");
static_assert(std::is_same_v<decltype(rm::FogSightNow()), std::shared_ptr<const rm::FogSightSnapshot>>,
              "FogSightNow hands out an immutable snapshot");

namespace
{

// The base engine with the render path state a backend reports: the lighting
// tuning (post effects, the shader switch, the volFog switch, the fog's dev
// and quality knobs), the frame's feature gates and whether the remastered
// shaders run.
class RenderStateEngine : public EngineDummy
{
  public:
    rm::LightingTuning tuning;
    bool gatesComputed = false;
    bool remasterActive = true;

    rm::LightingTuning GetLightingTuning() const override { return tuning; }
    bool LightingRemasterActive() const override { return remasterActive; }
    bool GetFeatureGates(rm::FeatureGates& out) const override
    {
        if (!gatesComputed)
            return false;
        out = rm::FeatureGates{}; // every gate off: SSAA off, an interior, the fog unavailable
        out.computed = true;
        out.inputs.interior = true;
        return true;
    }
};

class GEngineScope
{
  public:
    explicit GEngineScope(Engine* e) : _prev(GEngine) { GEngine = e; }
    ~GEngineScope() { GEngine = _prev; }

  private:
    Engine* _prev;
};

// Applies the Graphics options' Lighting row as boot and the Options page do
// (ApplyGraphicsConfigToEngine), with no world and no engine to push into.
void ApplyLightingRow(GraphicsConfig::LightingMode row)
{
    GraphicsConfig cfg;
    cfg.LoadDefaults();
    cfg.lighting = row;
    Scene* scene = GScene;
    Engine* engine = GEngine;
    const int fpsCap = gUserFpsCap;
    GScene = nullptr;
    GEngine = nullptr;
    ApplyGraphicsConfigToEngine(cfg);
    GEngine = engine;
    GScene = scene;
    gUserFpsCap = fpsCap;
}

// The landscape with its terrain setters open (the engine's island loader
// sizes and fills it through them).
class TestLand : public Landscape
{
  public:
    TestLand() : Landscape(nullptr, nullptr) {}
    using Landscape::Dim;
    using Landscape::SetData;
};

// A 64 x 64 island of 50 m cells: walls at 90 m, a valley floor at 20 m
// between x 20 and 44 (1000..2200 m east), open north to south. Fog 0.3.
void StageValley(TestLand& land, float fog = 0.3f)
{
    land.Dim(64, 64, 64, 64, 50.0f);
    for (int z = 0; z < 64; ++z)
        for (int x = 0; x < 64; ++x)
            land.SetData(x, z, (x >= 20 && x <= 44) ? 20.0f : 90.0f);
    land.SetFog(fog);
}

// A kilometre along the valley floor, 1.7 m over it (a man's eye to a man).
const float kFrom[3] = {1100.0f, 21.7f, 500.0f};
const float kTo[3] = {1100.0f, 21.7f, 1500.0f};

// Every field AI sight's march reads (FogSightTransmittanceThrough).
void CheckSameSightAir(const rm::FogMedium& a, const rm::FogMedium& b)
{
    CHECK(a.hazeSigma == b.hazeSigma);
    CHECK(a.hazeHeight == b.hazeHeight);
    CHECK(a.hazeNearKeep == b.hazeNearKeep);
    CHECK(a.hazeNearTo == b.hazeNearTo);
    CHECK(a.rayleigh == b.rayleigh);
    CHECK(a.dustSigma == b.dustSigma);
    CHECK(a.dustHeight == b.dustHeight);
    CHECK(a.weatherSigma == b.weatherSigma);
    CHECK(a.weatherHeight == b.weatherHeight);
    CHECK(a.groundSigma == b.groundSigma);
    CHECK(a.groundHeight == b.groundHeight);
    CHECK(a.mistSigma == b.mistSigma);
    CHECK(a.mistLayer == b.mistLayer);
    CHECK(a.mistFall == b.mistFall);
    CHECK(a.fogStart == b.fogStart);
    CHECK(a.fogEnd == b.fogEnd);
    CHECK(a.fogEdgeScale == b.fogEdgeScale);
}

// The renderer's own medium, made as EngineGL33::UpdateFogMedium makes it
// while its fog draws: the world's inputs, the sight map's aridity, the
// machine's tuning.
rm::FogMedium DrawnMedium(const Landscape& land, const rm::LightingTuning& t)
{
    rm::FogInputs in = WorldFogInputs(&land);
    const std::shared_ptr<const rm::FogSightMap> map = rm::FogSightMapInUse();
    in.aridity = map ? map->aridity : 0.0f;
    return rm::ComputeFogMedium(in, t, true);
}

std::filesystem::path EngineDir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine";
}

std::string ReadSource(const std::filesystem::path& p)
{
    std::ifstream f(p);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// The body of `signature { ... }` (to the closing brace at the start of a line).
std::string Body(const std::string& src, const std::string& signature)
{
    const size_t at = src.find(signature);
    if (at == std::string::npos)
        return {};
    const size_t end = src.find("\n}\n", at);
    return src.substr(at, end == std::string::npos ? std::string::npos : end - at);
}

} // namespace

TEST_CASE("AI fog sight: the simulation step ignores every render path state but what this machine draws",
          "[remaster][fog][aisight]")
{
    ShutdownFogSight();
    TestLand land;
    StageValley(land);
    RenderStateEngine engine;
    GEngineScope scope(&engine);

    SimulateFogSight(&land);
    REQUIRE(rm::FogSightActive());
    REQUIRE(rm::FogSightMapInUse());
    const rm::FogMedium base = rm::FogSightMedium();
    const float normal = rm::FogSightTransmittance(kFrom, kTo);
    INFO("T along the valley at fog 0.3 = " << normal);
    REQUIRE(normal < 0.9f); // the fog and the mist take a real share of a kilometre

    struct State
    {
        const char* name;
        std::function<void()> set;
    };
    const std::vector<State> states = {
        {"the engine's Lighting switch alone (the remastered shaders still drawn)",
         [&] { engine.tuning.remastered = false; }},
        {"volFog switch off", [&] { engine.tuning.features.volFog = 0.0f; }},
        {"the fog's dev knobs (density, mist)",
         [&]
         {
             engine.tuning.fogDensity = 2.5f;
             engine.tuning.fogMist = 0.0f;
         }},
        {"fog quality, sun and lamp knobs",
         [&]
         {
             engine.tuning.fogQuality = 0.0f;
             engine.tuning.fogSun = 0.2f;
             engine.tuning.fogLights = 0.0f;
         }},
        {"every gate off (SSAA off, interior view, fog unavailable)", [&] { engine.gatesComputed = true; }},
        {"the renderer drew another medium",
         [&]
         {
             rm::FogMedium other;
             other.mistSigma = 1.0e-2f;
             other.groundSigma = 1.0e-2f;
             other.hazeSigma = 1.0e-2f;
             rm::SetDrawnFogMedium(&other);
         }},
        {"the renderer drew nothing", [&] { rm::SetDrawnFogMedium(nullptr); }},
        {"no renderer (dedicated server, headless client)", [&] { GEngine = nullptr; }},
    };
    for (const State& s : states)
    {
        INFO("render path state: " << s.name);
        engine.tuning = rm::LightingTuning{};
        engine.gatesComputed = false;
        engine.remasterActive = true;
        GEngine = &engine;
        s.set();
        SimulateFogSight(&land);
        CHECK(rm::FogSightActive());
        CheckSameSightAir(rm::FogSightMedium(), base);
        CHECK(rm::FogSightTransmittance(kFrom, kTo) == normal);
    }
    GEngine = &engine;
    rm::SetDrawnFogMedium(nullptr);
    ShutdownFogSight();
    CHECK_FALSE(rm::FogSightActive());
    CHECK_FALSE(rm::FogSightNow());
}

TEST_CASE("AI fog sight: the simulation step's air is the whole drawn medium for the same weather",
          "[remaster][fog][aisight]")
{
    ShutdownFogSight();
    TestLand land;
    StageValley(land);
    RenderStateEngine engine;
    GEngineScope scope(&engine);
    REQUIRE(WorldFogInputs(&land).fog == 0.3f);

    // The shipped knobs: AI sight is the drawn medium, part for part, and the
    // same transmittance along the same line.
    SimulateFogSight(&land);
    REQUIRE(rm::FogSightMapInUse());
    const rm::FogMedium drawn = DrawnMedium(land, rm::LightingTuning{});
    CheckSameSightAir(rm::FogSightMedium(), drawn);
    REQUIRE(drawn.weatherSigma > 0.0f);
    REQUIRE(drawn.hazeSigma > 0.0f);
    const float ai = rm::FogSightTransmittance(kFrom, kTo);
    CHECK(ai == rm::FogSightTransmittanceThrough(drawn, *rm::FogSightMapInUse(), kFrom, kTo));
    // The old link (the classic tactical fog times the mist and the ground
    // air) left more than the whole medium does: the weather fog and the
    // haze it now takes.
    rm::FogMedium wetOnly = drawn;
    wetOnly.hazeSigma = wetOnly.rayleigh = wetOnly.weatherSigma = wetOnly.dustSigma = 0.0f;
    const float wet = rm::FogSightTransmittanceThrough(wetOnly, *rm::FogSightMapInUse(), kFrom, kTo);
    CHECK(ai < 0.8f * wet);

    // Thicker fog in the weather, thicker air for both.
    land.SetFog(0.6f);
    SimulateFogSight(&land);
    const float at06 = rm::FogSightTransmittance(kFrom, kTo);
    CHECK(at06 < ai);
    CHECK(at06 == rm::FogSightTransmittanceThrough(DrawnMedium(land, rm::LightingTuning{}), *rm::FogSightMapInUse(),
                                                   kFrom, kTo));

    // The step's view distance and classic fog range (the simulating
    // machine's, which set its classic tactical fog too) move AI sight as they
    // move the drawn edge and weather fog.
    const float tacticalZ = ENGINE_CONFIG.tacticalZ;
    ENGINE_CONFIG.tacticalZ = 600.0f;
    SimulateFogSight(&land);
    CheckSameSightAir(rm::FogSightMedium(), DrawnMedium(land, rm::LightingTuning{}));
    ENGINE_CONFIG.tacticalZ = tacticalZ;
    ShutdownFogSight();
}

TEST_CASE("AI fog sight: the Lighting row on Classic leaves the classic fog alone", "[remaster][fog][aisight]")
{
    ShutdownFogSight();
    TestLand land;
    StageValley(land);
    RenderStateEngine engine;
    GEngineScope scope(&engine);

    // No row applied (a dedicated server, a headless client): the default,
    // Remastered.
    SimulateFogSight(&land);
    CHECK(rm::FogSightActive());

    ApplyLightingRow(GraphicsConfig::LightingClassic);
    CHECK(FogSightClassic());
    SimulateFogSight(&land);
    CHECK_FALSE(rm::FogSightActive());
    CHECK(rm::FogSightTransmittance(kFrom, kTo) == 1.0f);
    REQUIRE(rm::FogSightNow());
    CHECK_FALSE(rm::FogSightNow()->volumetric);
    CHECK(rm::FogSightMapInUse()); // the map stays for the renderer's fog floor
    // The engine's own switch does not override the row either way.
    engine.tuning.remastered = true;
    SimulateFogSight(&land);
    CHECK_FALSE(rm::FogSightActive());

    ApplyLightingRow(GraphicsConfig::LightingRemastered);
    CHECK_FALSE(FogSightClassic());
    engine.tuning.remastered = false;
    SimulateFogSight(&land);
    CHECK(rm::FogSightActive());
    CHECK(rm::FogSightTransmittance(kFrom, kTo) < 0.9f);
    ShutdownFogSight();
}

TEST_CASE("AI fog sight: a machine drawing the classic fog leaves the classic fog alone, like the Classic row",
          "[remaster][fog][aisight]")
{
    ShutdownFogSight();
    TestLand land;
    StageValley(land);
    RenderStateEngine engine;
    GEngineScope scope(&engine);
    ApplyLightingRow(GraphicsConfig::LightingRemastered);

    // The shipped switches: the whole medium.
    CHECK_FALSE(FogSightDrawsClassic());
    SimulateFogSight(&land);
    REQUIRE(rm::FogSightActive());
    const float medium = rm::FogSightTransmittance(kFrom, kTo);
    CHECK(medium < 0.9f);

    // Post effects off: this machine draws the classic fog (the volumetric
    // fog is one of the post effects), so AI sight takes the classic tactical
    // fog only, as with the Lighting row on Classic.
    engine.tuning.postEffects = false;
    CHECK(FogSightDrawsClassic());
    CHECK(FogSightClassic());
    SimulateFogSight(&land);
    CHECK_FALSE(rm::FogSightActive());
    CHECK(rm::FogSightTransmittance(kFrom, kTo) == 1.0f);
    REQUIRE(rm::FogSightNow());
    CHECK_FALSE(rm::FogSightNow()->volumetric);
    CHECK(rm::FogSightMapInUse()); // the map stays for the renderer's fog floor
    // The engine's Lighting switch does not bring the medium back.
    engine.tuning.remastered = true;
    SimulateFogSight(&land);
    CHECK_FALSE(rm::FogSightActive());

    // The classic shaders with the row on Remastered (the remastered ones
    // failed to build, or the engine's Lighting switch): the classic fog is
    // drawn, so AI sight takes it.
    engine.tuning.postEffects = true;
    engine.remasterActive = false;
    CHECK(FogSightDrawsClassic());
    SimulateFogSight(&land);
    CHECK_FALSE(rm::FogSightActive());
    engine.remasterActive = true;

    // Back on: the medium again, the same as before.
    CHECK_FALSE(FogSightClassic());
    SimulateFogSight(&land);
    CHECK(rm::FogSightActive());
    CHECK(rm::FogSightTransmittance(kFrom, kTo) == medium);

    // The fog's own knobs with post effects on still do not count.
    engine.tuning.features.volFog = 0.0f;
    engine.tuning.fogDensity = 2.5f;
    SimulateFogSight(&land);
    CHECK(rm::FogSightActive());
    CHECK(rm::FogSightTransmittance(kFrom, kTo) == medium);

    // No engine draws nothing: the default, Remastered.
    GEngine = nullptr;
    CHECK_FALSE(FogSightDrawsClassic());
    CHECK_FALSE(FogSightClassic());
    SimulateFogSight(&land);
    CHECK(rm::FogSightActive());
    GEngine = &engine;

    // Both off together: classic, and the row alone keeps it classic when
    // post effects come back.
    engine.tuning = rm::LightingTuning{};
    engine.tuning.postEffects = false;
    ApplyLightingRow(GraphicsConfig::LightingClassic);
    SimulateFogSight(&land);
    CHECK_FALSE(rm::FogSightActive());
    engine.tuning.postEffects = true;
    SimulateFogSight(&land);
    CHECK_FALSE(rm::FogSightActive());
    ApplyLightingRow(GraphicsConfig::LightingRemastered);
    SimulateFogSight(&land);
    CHECK(rm::FogSightActive());
    ShutdownFogSight();
}

TEST_CASE("AI fog sight: deterministic from synced state, rebuilt only when the ground changes",
          "[remaster][fog][aisight]")
{
    ShutdownFogSight();
    RenderStateEngine engine;
    GEngineScope scope(&engine);

    // The server: a renderer with its fog on.
    TestLand server;
    StageValley(server);
    SimulateFogSight(&server);
    REQUIRE(rm::FogSightActive());
    const rm::FogMedium mServer = rm::FogSightMedium();
    const std::shared_ptr<const rm::FogSightMap> mapServer = rm::FogSightMapInUse();
    REQUIRE(mapServer);
    const float tServer = rm::FogSightTransmittance(kFrom, kTo);

    // The same step again: the map is kept, the air the same.
    SimulateFogSight(&server);
    CHECK(rm::FogSightMapInUse() == mapServer);
    CHECK(rm::FogSightTransmittance(kFrom, kTo) == tServer);

    // A client: another landscape object with the same synced heights and
    // weather, a headless process (no renderer), from a cold start.
    ShutdownFogSight();
    {
        TestLand client;
        StageValley(client);
        GEngine = nullptr;
        SimulateFogSight(&client);
        GEngine = &engine;
        REQUIRE(rm::FogSightActive());
        const std::shared_ptr<const rm::FogSightMap> mapClient = rm::FogSightMapInUse();
        REQUIRE(mapClient);
        CHECK(mapClient != mapServer); // built afresh
        CHECK(mapClient->size == mapServer->size);
        CHECK(mapClient->cellSize == mapServer->cellSize);
        CHECK(mapClient->ground == mapServer->ground);
        CHECK(mapClient->floorY == mapServer->floorY);
        CHECK(mapClient->layer == mapServer->layer);
        CHECK(mapClient->aridity == mapServer->aridity);
        CheckSameSightAir(rm::FogSightMedium(), mServer);
        CHECK(rm::FogSightTransmittance(kFrom, kTo) == tServer);
        ShutdownFogSight();
    }

    // Reshaped ground (a new island at the same size): the map is rebuilt.
    SimulateFogSight(&server);
    const std::shared_ptr<const rm::FogSightMap> before = rm::FogSightMapInUse();
    server.SetData(30, 30, 60.0f);
    SimulateFogSight(&server);
    REQUIRE(rm::FogSightMapInUse());
    CHECK(rm::FogSightMapInUse() != before);
    CHECK(rm::FogSightMapInUse()->ground[30 * 64 + 30] == 60.0f);

    // A snapshot handed out does not follow the next step.
    const std::shared_ptr<const rm::FogSightSnapshot> held = rm::FogSightNow();
    REQUIRE(held);
    const rm::FogMedium heldMedium = held->medium;
    const float heldT = rm::FogSightTransmittance(*held, kFrom, kTo);
    server.SetFog(0.9f);
    SimulateFogSight(&server);
    CHECK(rm::FogSightNow() != held);
    CHECK(rm::FogSightMedium().weatherSigma != heldMedium.weatherSigma);
    CHECK(held->medium.weatherSigma == heldMedium.weatherSigma);
    CHECK(rm::FogSightTransmittance(*held, kFrom, kTo) == heldT);

    // No island (GLOB_LAND before a world loads): classic sight, no map.
    SimulateFogSight(nullptr);
    CHECK_FALSE(rm::FogSightActive());
    CHECK_FALSE(rm::FogSightMapInUse());
    CHECK(rm::FogSightTransmittance(kFrom, kTo) == 1.0f);
    ShutdownFogSight();
}

TEST_CASE("AI fog sight: the dust is the island's, not the renderer's input", "[remaster][fog][aisight]")
{
    // WorldFogInputs carries no aridity; the snapshot takes the sight map's,
    // whatever the inputs say.
    const int n = 32;
    std::vector<float> h(static_cast<size_t>(n) * n, 30.0f);
    std::vector<float> sand(static_cast<size_t>(n) * n * 3);
    for (size_t i = 0; i < h.size(); ++i)
    {
        sand[i * 3 + 0] = 0.8f;
        sand[i * 3 + 1] = 0.66f;
        sand[i * 3 + 2] = 0.45f;
    }
    const std::shared_ptr<const rm::FogSightMap> desert = rm::BuildFogSightMap(h.data(), n, 50.0f, 1024, sand.data());
    const std::shared_ptr<const rm::FogSightMap> bare = rm::BuildFogSightMap(h.data(), n, 50.0f);
    REQUIRE(desert);
    REQUIRE(bare);
    CHECK(desert->aridity > 0.5f);
    CHECK(bare->aridity == 0.0f);
    rm::FogInputs in;
    in.aridity = 1.0f; // a renderer's own value never reaches AI sight
    CHECK(rm::MakeFogSightSnapshot(in, false, bare)->medium.dustSigma == 0.0f);
    in.aridity = 0.0f;
    CHECK(rm::MakeFogSightSnapshot(in, false, desert)->medium.dustSigma > 0.0f);
}

TEST_CASE("AI fog sight: the engine's call path (source contract)", "[remaster][fog][aisight]")
{
    const std::filesystem::path engine = EngineDir();
    REQUIRE(std::filesystem::exists(engine / "Poseidon" / "World" / "WorldSetup.cpp"));

    // Who may set AI sight: only its own module and the simulation step. No
    // renderer file, no script verb.
    const std::vector<std::string> setters = {"PublishFogSight(", "MakeFogSightSnapshot(", "SimulateFogSight("};
    int scanned = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(engine))
    {
        if (!entry.is_regular_file())
            continue;
        const std::string ext = entry.path().extension().string();
        if (ext != ".cpp" && ext != ".hpp" && ext != ".h")
            continue;
        const std::string name = entry.path().filename().string();
        const std::string src = ReadSource(entry.path());
        ++scanned;
        for (const std::string& call : setters)
        {
            if (src.find(call) == std::string::npos)
                continue;
            INFO(entry.path().string() << " calls " << call);
            const bool own = name == "VolumetricFog.hpp" || name == "VolumetricFog.cpp" ||
                             name == "TerrainLighting.hpp" || name == "TerrainLighting.cpp";
            const bool step = name == "WorldSetup.cpp" && call == "SimulateFogSight(";
            CHECK((own || step));
        }
    }
    CHECK(scanned > 500);

    // The step reads the world, the island, the applied Lighting row and the
    // post effects switch, never another render path state or live knob.
    const std::string lighting = ReadSource(engine / "Poseidon" / "World" / "Terrain" / "TerrainLighting.cpp");
    const std::string step = Body(lighting, "void SimulateFogSight(Landscape* land)");
    REQUIRE_FALSE(step.empty());
    CHECK(step.find("remaster::PublishFogSight(remaster::MakeFogSightSnapshot(WorldFogInputs(land), "
                    "FogSightClassic(), s_sight.map))") != std::string::npos);
    const std::string classic = Body(lighting, "bool FogSightClassic()");
    REQUIRE_FALSE(classic.empty());
    CHECK(classic.find("AppliedGraphicsConfig()") != std::string::npos);
    CHECK(classic.find("if (!cfg)\n        return false;") != std::string::npos);
    CHECK(classic.find("FogSightDrawsClassic()") != std::string::npos);
    // What this machine draws: the remastered shaders and the post effects
    // switch, nothing else.
    const std::string post = Body(lighting, "bool FogSightDrawsClassic()");
    REQUIRE_FALSE(post.empty());
    CHECK(post.find("!GEngine->LightingRemasterActive() || !GEngine->GetLightingTuning().postEffects") !=
          std::string::npos);
    for (const char* other : {"GetFeatureGates", "GetDrawnFogMedium", "SSAA", "nterior", "GetFrameCounter", "volFog",
                              "fogQuality", "fogDensity", ".remastered"})
    {
        INFO("post effects switch reads " << other);
        CHECK(post.find(other) == std::string::npos);
    }
    const std::string inputs = Body(lighting, "remaster::FogInputs WorldFogInputs(const Landscape* land)");
    REQUIRE_FALSE(inputs.empty());
    for (const char* renderState :
         {"GetFeatureGates", "LightingRemasterActive", "GetDrawnFogMedium", "postEffects", "SSAA", "nterior",
          "GetFrameCounter", "volFog", "fogQuality", "GetLightingTuning", "GEngine"})
    {
        INFO("render path state " << renderState);
        CHECK(step.find(renderState) == std::string::npos);
        CHECK(classic.find(renderState) == std::string::npos);
        CHECK(inputs.find(renderState) == std::string::npos);
    }
    // The snapshot takes the shipped knobs.
    const std::string fogSrc = ReadSource(engine / "Poseidon" / "Graphics" / "Remaster" / "VolumetricFog.cpp");
    const std::string make = Body(fogSrc, "std::shared_ptr<const FogSightSnapshot> MakeFogSightSnapshot(");
    REQUIRE_FALSE(make.empty());
    CHECK(make.find("ComputeFogMedium(world, LightingTuning{}, true)") != std::string::npos);

    // The step runs at the end of the landscape's simulation (after the clock,
    // the sun and the weather moved) ...
    const std::string setup = ReadSource(engine / "Poseidon" / "World" / "WorldSetup.cpp");
    const std::string landscape = Body(setup, "void World::SimulateLandscape(float deltaT)");
    REQUIRE_FALSE(landscape.empty());
    const size_t sight = landscape.find("SimulateFogSight(GLOB_LAND)");
    REQUIRE(sight != std::string::npos);
    CHECK(landscape.find("sun->Recalculate(") < sight);
    CHECK(landscape.find("GLOB_LAND->Simulate(") < sight);
    CHECK(landscape.rfind("SetFog(") < sight);
    // ... which runs before PerformAI in the same frame.
    const std::string world = ReadSource(engine / "Poseidon" / "World" / "World.cpp");
    const std::string frame = Body(world, "void World::Simulate(float deltaT, bool& enableDraw)");
    REQUIRE_FALSE(frame.empty());
    const size_t simLand = frame.find("SimulateLandscape(deltaT)");
    const size_t ai = frame.find("PerformAI(deltaT");
    REQUIRE(simLand != std::string::npos);
    REQUIRE(ai != std::string::npos);
    CHECK(simLand < ai);

    // AI's visibility takes one snapshot and, when it is volumetric, the
    // drawn air in place of the classic tactical fog; else the classic fog.
    const std::string target = ReadSource(engine / "Poseidon" / "World" / "Detection" / "Target.cpp");
    const std::string vis = Body(target, "float EntityAI::CalcVisibility(");
    REQUIRE_FALSE(vis.empty());
    const size_t now = vis.find("Poseidon::remaster::FogSightNow()");
    const size_t volumetric = vis.find("sight && sight->volumetric");
    const size_t take = vis.find("vis = Poseidon::remaster::FogSightTransmittance(*sight, from, to)");
    const size_t classicFog = vis.find("vis = 1 - GLOB_SCENE->TacticalFog8(dist2)");
    CHECK(now != std::string::npos);
    CHECK(volumetric != std::string::npos);
    CHECK(take != std::string::npos);
    CHECK(classicFog != std::string::npos);
    CHECK(now < volumetric);
    CHECK(volumetric < take);
    CHECK(take < classicFog);
    CHECK(vis.find("vis *= Poseidon::remaster::FogSightTransmittance") == std::string::npos);

    // The renderer fills its medium from the same inputs and draws its fog
    // floor from AI sight's map; it only publishes what it drew.
    const std::string fog = ReadSource(engine / "PoseidonGL33" / "EngineGL33_PostFog.cpp");
    const std::string update = Body(fog, "void EngineGL33::UpdateFogMedium()");
    REQUIRE_FALSE(update.empty());
    CHECK(update.find("Poseidon::WorldFogInputs(GScene->GetLandscape())") != std::string::npos);
    CHECK(fog.find("rm::FogSightMapInUse()") != std::string::npos);
    // The dust's aridity has one source, AI sight's map through
    // WorldFogInputs: no fog view overrides it with a renderer's own.
    CHECK(inputs.find("remaster::FogSightMapInUse()") != std::string::npos);
    CHECK(inputs.find("in.aridity = map->aridity") != std::string::npos);
    for (const char* file : {"EngineGL33_PostFog.cpp", "EngineGL33_PostFogViews.cpp", "EngineGL33.hpp"})
    {
        INFO(file);
        const std::string src = ReadSource(engine / "PoseidonGL33" / file);
        REQUIRE_FALSE(src.empty());
        CHECK(src.find("in.aridity") == std::string::npos);
        CHECK(src.find("FogAridity(") == std::string::npos);
    }
}
