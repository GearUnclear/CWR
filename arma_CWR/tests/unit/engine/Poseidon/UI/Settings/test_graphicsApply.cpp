// GraphicsApply: the shadow and smoke tiers of the Graphics options reach a
// Scene that is built after they were applied. Boot applies graphics.cfg
// before the world (and its Scene) exists, so the shadow tier never reached
// the scene: trees and buildings cast no sun shadows until the player opened
// the Graphics page. The terrain grid and object LOD bias tiers stay out of
// that path: the terrain grid subdivides the terrain heights that SurfaceY
// reads, so a Scene built at boot keeps its own values for both.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Core/Config/EngineConfig.hpp>
#include <Poseidon/Core/Game/GameLoop.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/UI/Settings/GraphicsApply.hpp>
#include <Poseidon/UI/Settings/GraphicsConfig.hpp>
#include <Poseidon/World/Scene/Scene.hpp>

#include <memory>

using Poseidon::GraphicsConfig;
using Poseidon::Scene;

namespace
{
// Applies cfg the way boot does: no world, so no scene. The engine half is
// left out (GEngine is the shared test dummy); this test is about the scene.
struct BootApply
{
    explicit BootApply(const GraphicsConfig& cfg)
    {
        Poseidon::GScene = nullptr;
        Poseidon::GEngine = nullptr;
        Poseidon::ApplyGraphicsConfigToEngine(cfg);
        Poseidon::GEngine = engine;
        Poseidon::GScene = scene;
    }
    ~BootApply() { Poseidon::gUserFpsCap = fpsCap; }

    Scene* scene = Poseidon::GScene;
    Poseidon::Engine* engine = Poseidon::GEngine;
    int fpsCap = Poseidon::gUserFpsCap;
};

// A Scene as World builds one. Its constructor and destructor touch the far
// clip and the profile's legacy keys; the far clip is put back.
struct TestScene
{
    TestScene() : scene(std::make_unique<Scene>()) {}
    ~TestScene()
    {
        scene.reset();
        ENGINE_CONFIG.horizontZ = horizontZ;
    }

    float horizontZ = ENGINE_CONFIG.horizontZ;
    std::unique_ptr<Scene> scene;
};
} // namespace

TEST_CASE("GraphicsApply: a Scene built after the boot apply takes the shadow and smoke tiers only",
          "[Settings][GraphicsApply][graphics]")
{
    // The Scene's own terrain grid (its default, or the profile's legacy
    // terrainGrid key) and object LOD bias, from a Scene built the same way.
    float sceneGrid = 0.0f;
    float sceneLodBias = 0.0f;
    {
        TestScene reference;
        sceneGrid = reference.scene->GetPreferredTerrainGrid();
        sceneLodBias = reference.scene->GetObjectLODBias();
    }

    GraphicsConfig cfg;
    cfg.LoadDefaults();
    cfg.shadowQuality = GraphicsConfig::TierLow;
    cfg.particlesQuality = GraphicsConfig::TierOff;
    cfg.terrainDetail = GraphicsConfig::TierLow; // 50 m
    cfg.objectLod = GraphicsConfig::TierLow;     // bias 0.5
    // Both tiers must differ from the Scene's values, or the checks below
    // could not tell an apply from none.
    REQUIRE(Poseidon::TerrainGridForTier(cfg.terrainDetail) != Catch::Approx(sceneGrid));
    REQUIRE(sceneLodBias != Catch::Approx(0.5f));
    BootApply boot(cfg);

    TestScene world;
    CHECK(world.scene->GetObjectShadows()); // trees and buildings cast
    CHECK(world.scene->GetVehicleShadows());
    CHECK_FALSE(world.scene->GetCloudlets());
    // Not the config's 50 m and 0.5: the terrain grid would subdivide the
    // physical terrain.
    CHECK(world.scene->GetPreferredTerrainGrid() == Catch::Approx(sceneGrid));
    CHECK(world.scene->GetObjectLODBias() == Catch::Approx(sceneLodBias));
}

TEST_CASE("GraphicsApply: shadow quality Off turns the scene's shadows off", "[Settings][GraphicsApply][graphics]")
{
    GraphicsConfig cfg;
    cfg.LoadDefaults();
    cfg.shadowQuality = GraphicsConfig::TierOff;
    BootApply boot(cfg);

    TestScene world;
    CHECK_FALSE(world.scene->GetObjectShadows());
    CHECK_FALSE(world.scene->GetVehicleShadows());
}

TEST_CASE("GraphicsApply: a profile switch keeps the Graphics options over the profile's legacy keys",
          "[Settings][GraphicsApply][graphics]")
{
    GraphicsConfig cfg;
    cfg.LoadDefaults();
    cfg.shadowQuality = GraphicsConfig::TierHigh;
    BootApply boot(cfg);

    TestScene world;
    // A profile last saved with object shadows off (every session before this
    // fix saved them so), then read back as a profile switch does.
    world.scene->SetObjectShadows(false);
    world.scene->SaveConfig();
    world.scene->LoadConfig();
    CHECK(world.scene->GetObjectShadows());
}
