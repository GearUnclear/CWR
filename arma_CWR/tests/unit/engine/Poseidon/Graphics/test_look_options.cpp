#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Remaster/LookOptions.hpp>
#include <Poseidon/Graphics/Shadow/ShadowXp.hpp>

// The Graphics page's look options (shore water, lamp shadows, the sun shadow
// tier) and the environment's POSEIDON_XP_* overrides over them: an unset
// variable keeps the player's option, a set one wins, so the perf gate and the
// stills jobs can still pin the look the build drew before the options.

using namespace Poseidon::remaster;
namespace xp = Poseidon::shadow::xp;

TEST_CASE("LookOptions: the defaults turn the shore water and lamp shadows on at Normal shadows",
          "[Graphics][LookOptions]")
{
    const LookOptions o;
    CHECK(o.shoreWater);
    CHECK(o.lampShadows);
    CHECK(o.shadowTier == kShadowTierNormal);
    CHECK(kShadowTierNormal == 2);
    CHECK(kShadowTierHigh == 3);
}

TEST_CASE("LookOptions: the shore water follows the option without an override", "[Graphics][LookOptions]")
{
    const LookEnv none;
    CHECK(ShoreWaterParts(true, none) == kShoreWaterDefaultParts);
    CHECK(kShoreWaterDefaultParts == 7);
    CHECK(ShoreWaterParts(false, none) == 0);
}

TEST_CASE("LookOptions: POSEIDON_XP_WATER wins over the shore water option", "[Graphics][LookOptions]")
{
    LookEnv env;
    env.water = "0";
    CHECK(ShoreWaterParts(true, env) == 0);
    CHECK(ShoreWaterParts(false, env) == 0);
    env.water = "1";
    CHECK(ShoreWaterParts(false, env) == 7);
    CHECK(ShoreWaterParts(true, env) == 7);
}

TEST_CASE("LookOptions: POSEIDON_XP_WATER_PARTS picks the parts only while the shore water runs",
          "[Graphics][LookOptions]")
{
    LookEnv env;
    env.waterParts = "15";
    CHECK(ShoreWaterParts(true, env) == 15);
    CHECK(ShoreWaterParts(false, env) == 0);
    env.water = "1";
    CHECK(ShoreWaterParts(false, env) == 15);
    // The mask keeps its four bits.
    env.waterParts = "33";
    CHECK(ShoreWaterParts(true, env) == 1);
}

TEST_CASE("LookOptions: the lamp shadows option turns the lamp shadows and the bounce on together",
          "[Graphics][LookOptions]")
{
    const LookEnv none;
    const NightLightLook on = NightLightFor(true, none);
    CHECK(on.lampShadows);
    CHECK(on.bounceGain == 1.0f);
    const NightLightLook off = NightLightFor(false, none);
    CHECK_FALSE(off.lampShadows);
    CHECK(off.bounceGain == 0.0f);
}

TEST_CASE("LookOptions: POSEIDON_XP_NIGHTLIGHT wins over the lamp shadows option", "[Graphics][LookOptions]")
{
    LookEnv env;
    env.nightLight = "0";
    const NightLightLook off = NightLightFor(true, env);
    CHECK_FALSE(off.lampShadows);
    CHECK(off.bounceGain == 0.0f);
    env.nightLight = "1";
    const NightLightLook on = NightLightFor(false, env);
    CHECK(on.lampShadows);
    CHECK(on.bounceGain == 1.0f);
}

TEST_CASE("LookOptions: the single lamp switches each replace one part", "[Graphics][LookOptions]")
{
    LookEnv env;
    env.lampShadows = "0";
    NightLightLook n = NightLightFor(true, env);
    CHECK_FALSE(n.lampShadows);
    CHECK(n.bounceGain == 1.0f);

    env = LookEnv{};
    env.lightBounce = "0.5";
    n = NightLightFor(false, env);
    CHECK_FALSE(n.lampShadows);
    CHECK(n.bounceGain == Catch::Approx(0.5f));

    env.lightBounce = "0";
    CHECK(NightLightFor(true, env).bounceGain == 0.0f);
    env.lightBounce = "99";
    CHECK(NightLightFor(true, env).bounceGain == 8.0f);
    env.lightBounce = "yes";
    CHECK(NightLightFor(false, env).bounceGain == 1.0f);

    // The single switches apply on top of POSEIDON_XP_NIGHTLIGHT.
    env = LookEnv{};
    env.nightLight = "0";
    env.lampShadows = "1";
    n = NightLightFor(true, env);
    CHECK(n.lampShadows);
    CHECK(n.bounceGain == 0.0f);
}

TEST_CASE("LookOptions: POSEIDON_XP_SHADOW_TIER wins over the shadow detail option", "[Graphics][LookOptions]")
{
    // Unset (-1): the option.
    CHECK(xp::ResolveTier(kShadowTierNormal, -1) == 2);
    CHECK(xp::ResolveTier(kShadowTierHigh, -1) == 3);
    // Set: the variable, whatever the option.
    CHECK(xp::ResolveTier(kShadowTierHigh, 2) == 2);
    CHECK(xp::ResolveTier(kShadowTierNormal, 3) == 3);
    CHECK(xp::ResolveTier(kShadowTierNormal, 0) == 0);
    CHECK(xp::ResolveTier(kShadowTierNormal, 4) == 4);
    // An unknown option reads as Normal, an out-of-range variable as unset.
    CHECK(xp::ResolveTier(7, -1) == 2);
    CHECK(xp::ResolveTier(kShadowTierHigh, 9) == 3);
}

TEST_CASE("LookOptions: High is the ultra tier without the caster cull", "[Graphics][LookOptions]")
{
    const xp::TierParams high = xp::Tier(kShadowTierHigh);
    CHECK(high.resolution == 4096);
    CHECK(high.cascades == 4);
    CHECK(high.omni == 2);
    CHECK(high.reach == 1000.0f);
    CHECK(high.taaTaps == 0);
    CHECK(high.tight);
    // The per-cascade cull is pixel-identical but has no timing: no tier
    // turns it on (POSEIDON_XP_SMCULL does).
    for (int tier = 0; tier <= 4; ++tier)
        CHECK_FALSE(xp::Tier(tier).cull);
    // Normal draws the shadow distance as before the row.
    CHECK(xp::Tier(kShadowTierNormal).reach == 0.0f);
    CHECK(xp::MapFar(500.0f, 4000.0f, 2667.0f, xp::Tier(kShadowTierNormal).reach) == 500.0f);
    CHECK(xp::MapFar(500.0f, 4000.0f, 2667.0f, high.reach) == Catch::Approx(1000.0f));
}
