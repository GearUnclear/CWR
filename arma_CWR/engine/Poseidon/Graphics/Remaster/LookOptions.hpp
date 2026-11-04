#pragma once

#include <cstdlib>
#include <cstring>

// The look options of the Graphics page (GraphicsConfig shoreWater,
// lampShadows, shadowDetail and sunGlare, persisted in graphics.cfg; GraphicsApply hands
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
//
// So POSEIDON_XP_WATER=0 POSEIDON_XP_NIGHTLIGHT=0 POSEIDON_XP_SHADOW_TIER=2
// POSEIDON_XP_SUNGLARE=0 draws the frame of the build before these options,
// whatever graphics.cfg says.

namespace Poseidon::remaster
{

// The sun shadow tiers the Shadow Detail row offers (ShadowXp.hpp Tier).
constexpr int kShadowTierNormal = 2; // 2048 maps reaching the shadow distance (500 m)
constexpr int kShadowTierHigh = 3;   // 4096 maps reaching up to 1000 m

// The shore's parts when the shore water runs and POSEIDON_XP_WATER_PARTS is
// unset: shallow water, surf and wet sand (1 | 2 | 4).
constexpr int kShoreWaterDefaultParts = 7;

struct LookOptions
{
    bool shoreWater = true;
    bool lampShadows = true;
    int shadowTier = kShadowTierNormal;
    // Off by default: the glare is temporal (it follows what covers the sun)
    // and has only been judged in stills.
    bool sunGlare = false;
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

} // namespace Poseidon::remaster
