#include <Poseidon/UI/Settings/GraphicsApply.hpp>

#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp> // pulls Engine + extern GEngine
#include <Poseidon/Graphics/Remaster/QualityScale.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <Poseidon/World/Scene/Scene.hpp>

#include <cstdlib>
#include <optional>

namespace Poseidon
{

} // namespace Poseidon
#include <Poseidon/Core/Game/GameLoop.hpp>
namespace Poseidon
{
using Poseidon::gUserFpsCap;

float TerrainGridForTier(GraphicsConfig::Tier tier)
{
    switch (tier)
    {
        case GraphicsConfig::TierLow:
            return 50.0f;
        case GraphicsConfig::TierMedium:
            return 25.0f;
        case GraphicsConfig::TierHigh:
            return 12.5f;
        case GraphicsConfig::TierUltra:
            return 6.25f;
        case GraphicsConfig::TierExtreme:
            return 3.125f;
        default:
            return 6.25f;
    }
}

namespace
{
// Tier → projected-screen-size multiplier for LOD selection.  Low
// halves the apparent size (picks coarser LOD); Ultra doubles
// (picks finer LOD).  Range matches Scene::SetObjectLODBias clamp
// of [0.25, 4.0].
float ObjectLodTierToBias(GraphicsConfig::Tier t)
{
    switch (t)
    {
        case GraphicsConfig::TierLow:
            return 0.5f;
        case GraphicsConfig::TierMedium:
            return 0.75f;
        case GraphicsConfig::TierHigh:
            return 1.0f;
        case GraphicsConfig::TierUltra:
            return 2.0f;
        default:
            return 1.0f;
    }
}

// VsyncMode enum → SDL swap interval ints (0 / 1 / -1).
int VsyncToInterval(GraphicsConfig::VsyncMode v)
{
    switch (v)
    {
        case GraphicsConfig::VsyncOff:
            return 0;
        case GraphicsConfig::VsyncOn:
            return 1;
        case GraphicsConfig::VsyncAdaptive:
            return -1;
    }
    return 1;
}

// The config last pushed by ApplyGraphicsConfigToEngine.
std::optional<GraphicsConfig> s_appliedConfig;
} // namespace

void ApplyGraphicsShadowsToScene(const GraphicsConfig& cfg, Scene& scene)
{
    // Shadow tier: Off turns both off, Low and up turn both on.  Low/Med/High
    // discrimination is UI-only until a shadow-distance bias hook
    // lands.
    const bool shadowsOn = cfg.shadowQuality != GraphicsConfig::TierOff;
    scene.SetObjectShadows(shadowsOn);
    scene.SetVehicleShadows(shadowsOn);
    // Particles tier: Off turns cloudlets off, Low/High on.  Low vs
    // High is UI-only until the cloudlet system grows tiered density.
    scene.SetCloudlets(cfg.particlesQuality != GraphicsConfig::TierOff);
}

void ApplyGraphicsConfigToScene(const GraphicsConfig& cfg, Scene& scene)
{
    scene.SetPreferredTerrainGrid(TerrainGridForTier(cfg.terrainDetail));
    scene.SetObjectLODBias(ObjectLodTierToBias(cfg.objectLod));
    ApplyGraphicsShadowsToScene(cfg, scene);
}

const GraphicsConfig* AppliedGraphicsConfig()
{
    return s_appliedConfig ? &*s_appliedConfig : nullptr;
}

void ApplyGraphicsConfigToEngine(const GraphicsConfig& cfg)
{
    // Kept for a Scene built later: at boot this runs before the world exists.
    s_appliedConfig = cfg;
    // XP-presets (Graphics/Remaster/QualityScale.hpp): with POSEIDON_XP_PRESETS=1
    // the Quality Preset row picks the graded quality level (Custom reads as
    // High, today's frame). POSEIDON_XP_PRESET=<level> forces a level and wins.
    // Before the lighting set below, which applies the level's lighting caps.
    if (const char* xp = std::getenv("POSEIDON_XP_PRESETS"); xp && *xp && *xp != '0')
    {
        remaster::QualityLevel level = remaster::QualityLevel::High;
        if (cfg.qualityPreset >= GraphicsConfig::PresetLow && cfg.qualityPreset <= GraphicsConfig::PresetUltra)
            level = static_cast<remaster::QualityLevel>(static_cast<int>(cfg.qualityPreset));
        remaster::SetActiveQualityLevel(&level);
    }
    if (const remaster::QualityProfile* q = remaster::ActiveQuality())
        LOG_INFO(Graphics, "XP-presets: quality level {}{}", remaster::QualityLevelName(q->level),
                 remaster::QualityLevelForcedByEnv() ? " (POSEIDON_XP_PRESET)" : "");
    if (GScene)
        ApplyGraphicsConfigToScene(cfg, *GScene);
    if (GEngine)
    {
        GEngine->SetSwapInterval(VsyncToInterval(cfg.vsync));
        GEngine->SetBrightness(cfg.brightness);
        GEngine->SetGamma(cfg.gamma);
        GEngine->SetAlphaToCoverage(cfg.alphaToCoverage);
        GEngine->SetRenderScale(cfg.renderScale);
        GEngine->SetMsaaSamples(cfg.msaaSamples);
        GEngine->SetMultitexturing(cfg.multitexturing);

        // Lighting model. Remastered lighting takes its sun shadows from the
        // depth-buffer shadow maps (its shaders shadow only the direct sun);
        // Classic keeps the original projected shadows.
        remaster::LightingTuning lighting = GEngine->GetLightingTuning();
        lighting.remastered = cfg.lighting == GraphicsConfig::LightingRemastered;
        GEngine->SetLightingTuning(lighting);
        GEngine->SetShadowMapsEnabled(GEngine->LightingRemasterActive() &&
                                      cfg.shadowQuality != GraphicsConfig::TierOff);
    }
    gUserFpsCap = cfg.fpsCap;
}

} // namespace Poseidon
