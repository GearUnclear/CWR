#pragma once

#include <cstdlib>
#include <cstring>

// The look options of the Graphics page (GraphicsConfig shoreWater,
// lampShadows, shadowDetail, sunGlare and groundCover, persisted in graphics.cfg; GraphicsApply hands
// them to Engine::SetLookOptions) and the environment's overrides over them.
// Pure: no engine state, no GL.
//
// The perf gate and the stills jobs pin a look with environment variables, so
// a variable that is set wins over the player's option, and one that is unset
// or empty leaves the option as the player chose it:
//
//   POSEIDON_XP_WATER         0: shore water off; any other number: on
//   POSEIDON_XP_WATER_PARTS   while the shore water runs, its parts mask
//                             (EngineGL33_Sea.cpp; 15 adds wave masking)
//   POSEIDON_XP_NIGHTLIGHT    0: lamp shadows and light bounce off; anything
//                             else: both on
//   POSEIDON_XP_LAMPSHADOWS   the lamp shadows alone: 0 off, anything else on
//   POSEIDON_XP_LIGHTBOUNCE   the light bounce alone: 0 off, a number above 0
//                             its gain (at most 8), other text 1
//   POSEIDON_XP_SHADOW_TIER   0..4: the sun shadows' tier (2 Normal, 3 High;
//                             Graphics/Shadow/ShadowXp.hpp, which also reads the
//                             single shadow knobs on top of the tier)
//   POSEIDON_XP_SUNGLARE      the low sun's glare: 0 off, a number above 0
//                             its strength (at most 8), other text 1
//   POSEIDON_XP_GRASS         the near ground cover: 0 off, anything else on
//   POSEIDON_XP_SURF          the surf's style while the shore water's surf
//                             runs: 0 or painted Painted, 1 or ribbons
//                             Ribbons (other text keeps the option)
//
// So POSEIDON_XP_WATER=0 POSEIDON_XP_NIGHTLIGHT=0 POSEIDON_XP_SHADOW_TIER=2
// POSEIDON_XP_SUNGLARE=0 POSEIDON_XP_GRASS=0 draws the frame of the build before these options,
// whatever graphics.cfg says.

namespace Poseidon::remaster
{

// The sun shadow tiers the Shadow Detail row offers (ShadowXp.hpp Tier).
constexpr int kShadowTierNormal = 2; // 2048 maps reaching the shadow distance (500 m)
constexpr int kShadowTierHigh = 3;   // 4096 maps reaching up to 1000 m

// The shore's parts when the shore water runs and POSEIDON_XP_WATER_PARTS is
// unset: shallow water, surf and wet sand (1 | 2 | 4).
constexpr int kShoreWaterDefaultParts = 7;

// The surf styles of the Surf row. Painted: the sea and land programs paint
// the waves (EngineGL33_LightingShaders.cpp, no CPU work). Ribbons: breaking
// waves drawn as ribbons along a coastline traced the first time they draw on
// an island (EngineGL33_ShoreRibbons.cpp), within about 320 m of the camera.
constexpr int kSurfPainted = 0;
constexpr int kSurfRibbons = 1;

struct LookOptions
{
    bool shoreWater = true;
    bool lampShadows = true;
    int shadowTier = kShadowTierNormal;
    // Off by default: the glare is temporal (it follows what covers the sun)
    // and has only been judged in stills.
    bool sunGlare = false;
    // Off by default: short grass blades on the grassy ground near the camera.
    // Drawn only: nothing in the simulation, AI sight, saves or the network
    // reads it.
    bool groundCover = false;
    // The surf's style under Shore Water (kSurfPainted or kSurfRibbons).
    // Painted by default: it reaches the whole visible shore and costs least.
    int surfStyle = kSurfPainted;
};

// The environment's look switches as read once (null = unset or empty).
struct LookEnv
{
    const char* water = nullptr;
    const char* waterParts = nullptr;
    const char* nightLight = nullptr;
    const char* lampShadows = nullptr;
    const char* lightBounce = nullptr;
    const char* sunGlare = nullptr;
    const char* groundCover = nullptr;
    const char* surfStyle = nullptr;
};

inline const char* LookEnvValue(const char* name)
{
    const char* v = std::getenv(name);
    return (v && *v) ? v : nullptr;
}

inline LookEnv ReadLookEnv()
{
    LookEnv e;
    e.water = LookEnvValue("POSEIDON_XP_WATER");
    e.waterParts = LookEnvValue("POSEIDON_XP_WATER_PARTS");
    e.nightLight = LookEnvValue("POSEIDON_XP_NIGHTLIGHT");
    e.lampShadows = LookEnvValue("POSEIDON_XP_LAMPSHADOWS");
    e.lightBounce = LookEnvValue("POSEIDON_XP_LIGHTBOUNCE");
    e.sunGlare = LookEnvValue("POSEIDON_XP_SUNGLARE");
    e.groundCover = LookEnvValue("POSEIDON_XP_GRASS");
    e.surfStyle = LookEnvValue("POSEIDON_XP_SURF");
    return e;
}

// An on/off switch: null keeps `option`, "0" is off, anything else on.
inline bool LookSwitch(const char* value, bool option)
{
    if (!value)
        return option;
    return std::strcmp(value, "0") != 0;
}

// A gain switch: null keeps `option` (1 on, 0 off), a number its value (0 or
// below off, capped at 8), other text 1.
inline float LookGain(const char* value, bool option)
{
    if (!value)
        return option ? 1.0f : 0.0f;
    char* end = nullptr;
    const float g = std::strtof(value, &end);
    if (end == value)
        return 1.0f;
    if (!(g > 0.0f))
        return 0.0f;
    return g < 8.0f ? g : 8.0f;
}

// The shore's parts mask (0 = off): the option, or POSEIDON_XP_WATER when it is
// set (a number, 0 off), then POSEIDON_XP_WATER_PARTS while it runs.
inline int ShoreWaterParts(bool option, const LookEnv& env)
{
    bool on = option;
    if (env.water)
        on = std::atoi(env.water) != 0;
    if (!on)
        return 0;
    if (env.waterParts)
        return std::atoi(env.waterParts) & 15;
    return kShoreWaterDefaultParts;
}

// The lamp shadows and the light bounce the Lamp shadows option turns on
// together; POSEIDON_XP_NIGHTLIGHT replaces the option for both, then
// POSEIDON_XP_LAMPSHADOWS and POSEIDON_XP_LIGHTBOUNCE each replace one.
struct NightLightLook
{
    float bounceGain = 0.0f;
    bool lampShadows = false;
};

inline NightLightLook NightLightFor(bool option, const LookEnv& env)
{
    const bool all = LookSwitch(env.nightLight, option);
    NightLightLook n;
    n.bounceGain = LookGain(env.lightBounce, all);
    n.lampShadows = LookSwitch(env.lampShadows, all);
    return n;
}

// The low sun's glare strength (0 = off; EngineGL33_XpAtmosphere.cpp): the
// Sun Glare option at strength 1, or POSEIDON_XP_SUNGLARE when it is set.
inline float SunGlareGain(bool option, const LookEnv& env)
{
    return LookGain(env.sunGlare, option);
}

// The near ground cover (Landscape::DrawRect, the remastered grass program):
// the Ground Cover option, or POSEIDON_XP_GRASS when it is set.
inline bool GroundCoverOn(bool option, const LookEnv& env)
{
    return LookSwitch(env.groundCover, option);
}

// The surf's style: the Surf option (an unknown value is Painted), or
// POSEIDON_XP_SURF when it names one ("0" or "painted", "1" or "ribbons", any
// case; other text keeps the option).
inline int SurfStyleFor(int option, const LookEnv& env)
{
    int style = option == kSurfRibbons ? kSurfRibbons : kSurfPainted;
    if (!env.surfStyle)
        return style;
    auto is = [&](const char* word)
    {
        const char* v = env.surfStyle;
        for (; *word; ++word, ++v)
            if (*v == '\0' || (*v | 0x20) != *word)
                return false;
        return *v == '\0';
    };
    if (std::strcmp(env.surfStyle, "0") == 0 || is("painted"))
        return kSurfPainted;
    if (std::strcmp(env.surfStyle, "1") == 0 || is("ribbons"))
        return kSurfRibbons;
    return style;
}

} // namespace Poseidon::remaster
