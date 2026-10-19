// AI sight and the drawn fog (Graphics/Remaster/VolumetricFog.hpp, AI sight;
// World/Terrain/TerrainLighting.hpp SimulateFogSight), driven through the
// call the engine makes once per simulation step (World::SimulateLandscape ->
// SimulateFogSight(GLOB_LAND)) on a real Landscape, with an engine that
// reports each render path state in turn. The pure helpers' cases live in
// Graphics/test_volumetric_fog.cpp; these fail if any render path state
// (post effects, classic lighting, the fog's gates, an interior view, the fog
// quality knobs, what the renderer drew, the view distance, a missing
// renderer) reaches AI sight, or if the call order the link relies on breaks.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Core/Application.hpp>
#include <Poseidon/Core/Config/EngineConfig.hpp>
#include <Poseidon/Graphics/Dummy/EngineDummy.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/LightRig.hpp>
#include <Poseidon/Graphics/Remaster/VolumetricFog.hpp>
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

namespace
{

// The base engine with the render path state a backend reports: the lighting
// tuning (post effects, classic lighting, the volFog switch, the fog's
// quality knobs), the frame's feature gates and whether the remastered
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

// The fields AI sight's march reads (FogSightExtinctionAt).
void CheckSameSightAir(const rm::FogMedium& a, const rm::FogMedium& b)
{
    CHECK(a.groundSigma == b.groundSigma);
    CHECK(a.groundHeight == b.groundHeight);
    CHECK(a.mistSigma == b.mistSigma);
    CHECK(a.mistLayer == b.mistLayer);
    CHECK(a.mistFall == b.mistFall);
    CHECK(a.mistNoise == b.mistNoise);
}

// The renderer's own medium, made as EngineGL33::UpdateFogMedium makes it
// while its fog draws: the world's inputs plus its aridity, its tuning.
rm::FogMedium DrawnMedium(const Landscape& land, const rm::LightingTuning& t, float aridity)
{
    rm::FogInputs in = WorldFogInputs(&land);
    in.aridity = aridity;
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

TEST_CASE("AI fog sight: the simulation step ignores every render path state", "[remaster][fog][aisight]")
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
    REQUIRE(normal < 0.97f); // the mist takes a real share of a kilometre

    struct State
    {
        const char* name;
        std::function<void()> set;
    };
    const float tacticalZ = ENGINE_CONFIG.tacticalZ;
    const std::vector<State> states = {
        {"post effects off", [&] { engine.tuning.postEffects = false; }},
        {"classic lighting (owner call open: AI keeps the mist)", [&]
         {
             engine.tuning.remastered = false;
             engine.remasterActive = false;
         }},
        {"volFog switch off", [&] { engine.tuning.features.volFog = 0.0f; }},
        {"fog quality, sun and lamp knobs", [&]
         {
             engine.tuning.fogQuality = 0.0f;
             engine.tuning.fogSun = 0.2f;
             engine.tuning.fogLights = 0.0f;
         }},
        {"every gate off (SSAA off, interior view, fog unavailable)", [&] { engine.gatesComputed = true; }},
        {"the renderer drew another medium", [&]
         {
             rm::FogMedium other;
             other.mistSigma = 1.0e-2f;
             other.groundSigma = 1.0e-2f;
             rm::SetDrawnFogMedium(&other);
         }},
        {"the renderer drew nothing", [&] { rm::SetDrawnFogMedium(nullptr); }},
        {"a short view distance", [&] { ENGINE_CONFIG.tacticalZ = 300.0f; }},
        {"no renderer (dedicated server, headless client)", [&] { GEngine = nullptr; }},
    };
    for (const State& s : states)
    {
        INFO("render path state: " << s.name);
        engine.tuning = rm::LightingTuning{};
        engine.gatesComputed = false;
        engine.remasterActive = true;
        GEngine = &engine;
        ENGINE_CONFIG.tacticalZ = tacticalZ;
        s.set();
        SimulateFogSight(&land);
        CHECK(rm::FogSightActive());
        CheckSameSightAir(rm::FogSightMedium(), base);
        CHECK(rm::FogSightTransmittance(kFrom, kTo) == normal);
    }
    GEngine = &engine;
    ENGINE_CONFIG.tacticalZ = tacticalZ;
    rm::SetDrawnFogMedium(nullptr);
    ShutdownFogSight();
    CHECK_FALSE(rm::FogSightActive());
}

TEST_CASE("AI fog sight: the simulation step's air is the drawn fog's for the same weather",
          "[remaster][fog][aisight]")
{
    ShutdownFogSight();
    TestLand land;
    StageValley(land);
    RenderStateEngine engine;
    GEngineScope scope(&engine);
    REQUIRE(WorldFogInputs(&land).fog == 0.3f);

    // The knobs (the only per-machine inputs AI takes) move AI sight with the
    // drawn fog; the renderer's own extra input (the aridity, for the dust)
    // does not split them.
    for (float density : {1.0f, 0.5f, 2.0f})
        for (float mist : {1.0f, 0.5f, 0.0f})
        {
            INFO("fogDensity " << density << " fogMist " << mist);
            engine.tuning = rm::LightingTuning{};
            engine.tuning.fogDensity = density;
            engine.tuning.fogMist = mist;
            SimulateFogSight(&land);
            REQUIRE(rm::FogSightMapInUse());
            for (float aridity : {0.0f, 0.8f})
            {
                const rm::FogMedium drawn = DrawnMedium(land, engine.tuning, aridity);
                CheckSameSightAir(rm::FogSightMedium(), drawn);
                CHECK(rm::FogSightTransmittance(kFrom, kTo) ==
                      rm::FogSightTransmittanceThrough(drawn, *rm::FogSightMapInUse(), kFrom, kTo));
            }
        }
    // Thicker fog in the weather, thicker air for both.
    engine.tuning = rm::LightingTuning{};
    SimulateFogSight(&land);
    const float at03 = rm::FogSightTransmittance(kFrom, kTo);
    land.SetFog(0.6f);
    SimulateFogSight(&land);
    const float at06 = rm::FogSightTransmittance(kFrom, kTo);
    CHECK(at06 < at03);
    CHECK(at06 == rm::FogSightTransmittanceThrough(DrawnMedium(land, engine.tuning, 0.0f), *rm::FogSightMapInUse(),
                                                   kFrom, kTo));
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

    // A copy handed out does not follow the next step.
    const rm::FogMedium held = rm::FogSightMedium();
    server.SetFog(0.9f);
    SimulateFogSight(&server);
    CHECK(rm::FogSightMedium().mistSigma != held.mistSigma);
    CHECK(held.mistSigma == mServer.mistSigma);

    // No island (GLOB_LAND before a world loads): classic sight, no map.
    SimulateFogSight(nullptr);
    CHECK_FALSE(rm::FogSightActive());
    CHECK_FALSE(rm::FogSightMapInUse());
    CHECK(rm::FogSightTransmittance(kFrom, kTo) == 1.0f);
    ShutdownFogSight();
}

TEST_CASE("AI fog sight: the per-machine render inputs never reach AI's terms", "[remaster][fog][aisight]")
{
    // What WorldFogInputs hands the renderer beyond the synced weather and
    // date: the user's view distance, the classic fog's range, the map's
    // aridity; and the render-only knobs. None may move AI's air.
    rm::FogInputs in;
    in.sunElevationDeg = 4.0f;
    in.sunAzimuthDeg = 85.0f;
    in.skyThrough = 0.4f;
    in.fog = 0.3f;
    const rm::FogMedium base = rm::ComputeFogMedium(in, rm::LightingTuning{}, true);
    REQUIRE(base.mistSigma > 0.0f);
    rm::FogInputs other = in;
    other.viewDistance = 400.0f;
    other.fogStart = 50.0f;
    other.fogEnd = 400.0f;
    other.aridity = 1.0f;
    rm::LightingTuning t;
    t.postEffects = false;
    t.remastered = false;
    t.features.volFog = 0.0f;
    t.fogQuality = 0.0f;
    t.fogSun = 0.0f;
    t.fogLights = 0.0f;
    t.exposure = 3.0f;
    t.saturation = 0.5f;
    t.sun = 0.2f;
    CheckSameSightAir(rm::ComputeFogMedium(other, t, true), base);
}

TEST_CASE("AI fog sight: the engine's call path (source contract)", "[remaster][fog][aisight]")
{
    const std::filesystem::path engine = EngineDir();
    REQUIRE(std::filesystem::exists(engine / "Poseidon" / "World" / "WorldSetup.cpp"));

    // Who may set AI sight: only its own module and the simulation step. No
    // renderer file, no script verb.
    const std::vector<std::string> setters = {"SetFogSight(", "UpdateFogSightMedium(", "SimulateFogSight("};
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

    // The step reads the world and the knobs, never a render path state.
    const std::string lighting = ReadSource(engine / "Poseidon" / "World" / "Terrain" / "TerrainLighting.cpp");
    const std::string step = Body(lighting, "void SimulateFogSight(Landscape* land)");
    REQUIRE_FALSE(step.empty());
    CHECK(step.find("UpdateFogSightMedium(WorldFogInputs(land), tuning, s_sight.map)") != std::string::npos);
    const std::string inputs = Body(lighting, "remaster::FogInputs WorldFogInputs(const Landscape* land)");
    REQUIRE_FALSE(inputs.empty());
    for (const char* renderState : {"GetFeatureGates", "LightingRemasterActive", "GetDrawnFogMedium", "postEffects",
                                    "SSAA", "nterior", "GetFrameCounter", "volFog", "fogQuality"})
    {
        INFO("render path state " << renderState);
        CHECK(step.find(renderState) == std::string::npos);
        CHECK(inputs.find(renderState) == std::string::npos);
    }
    CHECK(inputs.find("GEngine") == std::string::npos);

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

    // AI's visibility takes the air.
    const std::string target = ReadSource(engine / "Poseidon" / "World" / "Detection" / "Target.cpp");
    const std::string vis = Body(target, "float EntityAI::CalcVisibility(");
    REQUIRE_FALSE(vis.empty());
    const size_t active = vis.find("Poseidon::remaster::FogSightActive()");
    const size_t mult = vis.find("vis *= Poseidon::remaster::FogSightTransmittance(from, to)");
    CHECK(active != std::string::npos);
    CHECK(mult != std::string::npos);
    CHECK(active < mult);

    // The renderer fills its medium from the same inputs and draws its fog
    // floor from AI sight's map; it only publishes what it drew.
    const std::string fog = ReadSource(engine / "PoseidonGL33" / "EngineGL33_PostFog.cpp");
    const std::string update = Body(fog, "void EngineGL33::UpdateFogMedium()");
    REQUIRE_FALSE(update.empty());
    CHECK(update.find("Poseidon::WorldFogInputs(GScene->GetLandscape())") != std::string::npos);
    CHECK(update.find("rm::ComputeFogMedium(in, _lighting, on)") != std::string::npos);
    CHECK(fog.find("rm::FogSightMapInUse()") != std::string::npos);
}
