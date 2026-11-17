#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <Poseidon/Game/Kaiju/KaijuMath.hpp>
#include <Poseidon/Game/Kaiju/Kaiju.hpp>
#include <cmath>

using namespace Poseidon::Kaiju;

TEST_CASE("Kaiju traits default to stock behaviour", "[kaiju]")
{
    Traits t;
    REQUIRE(t.farDraw == 0);
    REQUIRE(t.wadeDepth == 0);
    REQUIRE_FALSE(t.obstacleBlind);
    REQUIRE_FALSE(t.noSpeedDamage);
    REQUIRE(t.damageCeiling < 0);
    REQUIRE_FALSE(t.hitReport);
    REQUIRE(t.stepShake == 0);
    REQUIRE(t.gridRadius == 0);
}

TEST_CASE("Kaiju far fog floor is a silhouette that thins and ends at the limit", "[kaiju][fardraw]")
{
    const float fogEnd = 900, limit = 6000, contrast = 0.5f;
    // off: no limit, no contrast, past the limit
    REQUIRE(FarFogFloor(2000, fogEnd, 0, contrast) == 0);
    REQUIRE(FarFogFloor(2000, fogEnd, limit, 0) == 0);
    REQUIRE(FarFogFloor(limit, fogEnd, limit, contrast) == 0);
    REQUIRE(FarFogFloor(limit + 1, fogEnd, limit, contrast) == 0);
    // at the fog end the floor is the contrast asked for
    REQUIRE(FarFogFloor(fogEnd, fogEnd, limit, contrast) == Catch::Approx(contrast));
    // and never above it, closer in
    REQUIRE(FarFogFloor(100, fogEnd, limit, contrast) == Catch::Approx(contrast));
    // it only falls with distance, and reaches zero without a step
    float prev = contrast;
    for (float d = fogEnd; d <= limit; d += 50)
    {
        float f = FarFogFloor(d, fogEnd, limit, contrast);
        REQUIRE(f <= prev + 1e-5f);
        REQUIRE(f >= 0);
        prev = f;
    }
    REQUIRE(FarFogFloor(limit - 1, fogEnd, limit, contrast) < 0.01f);
    // still a readable shape half way out
    REQUIRE(FarFogFloor(3000, fogEnd, limit, contrast) > 0.3f);
    // contrast is clamped to a visibility
    REQUIRE(FarFogFloor(fogEnd, fogEnd, limit, 5) == Catch::Approx(1.0f));
}

TEST_CASE("Kaiju far pass takes over where the far plane would cut the unit", "[kaiju][fardraw]")
{
    REQUIRE_FALSE(FarNeedsOwnPass(700, 25, 900));
    REQUIRE(FarNeedsOwnPass(870, 25, 900));
    REQUIRE(FarNeedsOwnPass(3000, 25, 900));

    ClipBracket b = FarClipBracket(3000, 25);
    REQUIRE(b.zNear < 3000 - 25);
    REQUIRE(b.zFar > 3000 + 25);
    REQUIRE(b.zNear > 0);
    // a unit the camera stands inside of still gets a valid bracket
    ClipBracket c = FarClipBracket(10, 25);
    REQUIRE(c.zNear >= 5);
    REQUIRE(c.zFar > c.zNear);
    // the depth split leaves both parts a real range
    REQUIRE(kFarDepthSplit > 0.5f);
    REQUIRE(kFarDepthSplit < 1.0f);
}

TEST_CASE("Kaiju far ground haze grows from nothing and stops at half the body", "[kaiju][fardraw]")
{
    const float clipFar = 900, radius = 40, height = 40, feetRelY = 14;
    // where the far pass takes over the band covers nothing: its top is at the feet
    HazeBand start = FarHazeBand(clipFar, radius, feetRelY, height, clipFar, clipFar);
    const float scale0 = (clipFar - radius) / clipFar;
    REQUIRE(start.topY == Catch::Approx(feetRelY * scale0));
    REQUIRE(start.midY < start.topY);
    // far out it is solid to his shins and gone a little over half the body
    HazeBand full = FarHazeBand(3000, radius, feetRelY, height, 3000, clipFar);
    const float scale = (3000.0f - radius) / 3000.0f;
    REQUIRE(full.z == Catch::Approx(3000.0f - radius));
    REQUIRE(full.midY == Catch::Approx((feetRelY + 0.12f * height) * scale));
    REQUIRE(full.topY == Catch::Approx((feetRelY + 0.55f * height) * scale));
    // and it is all there a tenth of a view distance past the far plane
    HazeBand soon = FarHazeBand(1.1f * clipFar, radius, feetRelY, height, 1.1f * clipFar, clipFar);
    REQUIRE(soon.topY == Catch::Approx((feetRelY + 0.55f * height) * (1.1f * clipFar - radius) / (1.1f * clipFar)));
    // it only rises with distance
    float prevTop = -1e9f;
    for (float d = clipFar; d < 2000; d += 25)
    {
        HazeBand b = FarHazeBand(d, radius, feetRelY, height, d, clipFar);
        REQUIRE(b.topY >= prevTop - 1e-3f);
        REQUIRE(b.topY > b.midY);
        prevTop = b.topY;
    }
    // a unit almost on the camera still gets a band in front of the near plane
    REQUIRE(FarHazeBand(20, radius, feetRelY, height, 1000, clipFar).z >= 10);
}

TEST_CASE("Kaiju tremor gain falls with distance and dies with time", "[kaiju][shake]")
{
    // full strength inside the full radius, just after the attack
    REQUIRE(TremorGain(2, 10, 50, 300, 0.03f, 3) == Catch::Approx(2 * 0.99f * 0.99f));
    // nothing before it starts, after it ends, or past the zero radius
    REQUIRE(TremorGain(2, 10, 50, 300, -0.1f, 3) == 0);
    REQUIRE(TremorGain(2, 10, 50, 300, 3, 3) == 0);
    REQUIRE(TremorGain(2, 300, 50, 300, 0.5f, 3) == 0);
    REQUIRE(TremorGain(2, 1000, 50, 300, 0.5f, 3) == 0);
    REQUIRE(TremorGain(0, 10, 50, 300, 0.5f, 3) == 0);
    // monotone in distance and, after the attack, in time
    float prev = 1e9f;
    for (float d = 0; d <= 320; d += 10)
    {
        float g = TremorGain(2, d, 50, 300, 0.5f, 3);
        REQUIRE(g <= prev);
        prev = g;
    }
    prev = 1e9f;
    for (float age = 0.05f; age < 3; age += 0.05f)
    {
        float g = TremorGain(2, 10, 50, 300, age, 3);
        REQUIRE(g <= prev);
        prev = g;
    }
    // a zero radius not past the full radius is a hard edge, not a divide by zero
    REQUIRE(TremorGain(1, 60, 50, 50, 0.5f, 3) == 0);
}

TEST_CASE("Kaiju shake offset is bounded and scales with gain", "[kaiju][shake]")
{
    ShakeOffset none = ShakeAt(12.3f, 0);
    REQUIRE(none.up == 0);
    REQUIRE(none.pitch == 0);
    float maxUp1 = 0, maxUp3 = 0, maxPitch = 0;
    for (int i = 0; i < 2000; i++)
    {
        float t = i * 0.0137f;
        ShakeOffset a = ShakeAt(t, 1);
        ShakeOffset b = ShakeAt(t, 3);
        ShakeOffset huge = ShakeAt(t, 100);
        maxUp1 = std::fmax(maxUp1, std::fabs(a.up));
        maxUp3 = std::fmax(maxUp3, std::fabs(b.up));
        maxPitch = std::fmax(maxPitch, std::fabs(huge.pitch));
        // the gain is capped: a runaway sum of tremors cannot spin the camera
        REQUIRE(std::fabs(huge.up) <= 0.035f * kShakeMaxGain + 1e-5f);
        REQUIRE(std::fabs(huge.roll) <= 0.0085f * kShakeMaxGain + 1e-5f);
    }
    REQUIRE(maxUp1 > 0.02f);
    REQUIRE(maxUp1 <= 0.035f + 1e-5f);
    REQUIRE(maxUp3 > 2.5f * maxUp1);
    REQUIRE(maxPitch < 0.04f); // about two degrees at the very most
}

TEST_CASE("Kaiju steer heading error takes the short way round", "[kaiju][steer]")
{
    const float pi = 3.14159265f;
    REQUIRE(HeadingError(0, 0.5f) == Catch::Approx(0.5f));
    REQUIRE(HeadingError(0.5f, 0) == Catch::Approx(-0.5f));
    REQUIRE(HeadingError(pi - 0.1f, -pi + 0.1f) == Catch::Approx(0.2f).margin(1e-4));
    REQUIRE(HeadingError(-pi + 0.1f, pi - 0.1f) == Catch::Approx(-0.2f).margin(1e-4));
    REQUIRE(std::fabs(HeadingError(0, 7 * pi)) <= pi + 1e-4f);
}

TEST_CASE("Kaiju damage ceiling clamps only when set", "[kaiju][damage]")
{
    REQUIRE(ClampDamage(0.9f, -1) == Catch::Approx(0.9f));
    REQUIRE(ClampDamage(0.9f, 0.4f) == Catch::Approx(0.4f));
    REQUIRE(ClampDamage(0.2f, 0.4f) == Catch::Approx(0.2f));
    REQUIRE(ClampDamage(5.0f, 0) == 0);
}

TEST_CASE("Kaiju finishing traits default to stock behaviour", "[kaiju]")
{
    Traits t;
    REQUIRE(t.glow < 0);
    REQUIRE(t.farGlow == 0);
    REQUIRE_FALSE(t.plantBlend);
    REQUIRE(t.pace == 1.0f);
    REQUIRE(t.bodySink == 0);
    REQUIRE_FALSE(t.lodPin);
    REQUIRE(IsShiningMaterial(200));
    REQUIRE(IsShiningMaterial(220));
    REQUIRE_FALSE(IsShiningMaterial(0));
    REQUIRE_FALSE(IsShiningMaterial(201));
    REQUIRE_FALSE(IsShiningMaterial(70));
}

TEST_CASE("Kaiju glow splits into a dimmer and a white-hot share", "[kaiju][glow]")
{
    float scale = -1, heat = -1;
    GlowSplit(0, scale, heat);
    REQUIRE(scale == 0);
    REQUIRE(heat == 0);
    GlowSplit(0.4f, scale, heat);
    REQUIRE(scale == Catch::Approx(0.4f));
    REQUIRE(heat == 0);
    GlowSplit(1, scale, heat);
    REQUIRE(scale == 1);
    REQUIRE(heat == 0);
    GlowSplit(1.5f, scale, heat);
    REQUIRE(scale == 1);
    REQUIRE(heat == Catch::Approx(0.5f));
    GlowSplit(7, scale, heat);
    REQUIRE(scale == 1);
    REQUIRE(heat == 1);
    GlowSplit(-3, scale, heat);
    REQUIRE(scale == 0);
    REQUIRE(heat == 0);
}

TEST_CASE("Kaiju far fog floor rises at night for a unit that carries its own light", "[kaiju][fardraw]")
{
    const float limit = 6000;
    // off by day, off without the trait, off past the limit
    REQUIRE(FarNightFloor(0.45f, 2300, limit, 0.9f, 0) == Catch::Approx(0.45f));
    REQUIRE(FarNightFloor(0.45f, 2300, limit, 0, 1) == Catch::Approx(0.45f));
    REQUIRE(FarNightFloor(0, limit + 1, limit, 0.9f, 1) == 0);
    // full night: the night floor, inside the fade
    REQUIRE(FarNightFloor(0.45f, 2300, limit, 0.9f, 1) == Catch::Approx(0.9f));
    // dusk: in between, never under the day floor
    const float dusk = FarNightFloor(0.45f, 2300, limit, 0.9f, 0.5f);
    REQUIRE(dusk > 0.45f);
    REQUIRE(dusk < 0.9f);
    // a night floor under the day floor changes nothing
    REQUIRE(FarNightFloor(0.45f, 2300, limit, 0.2f, 1) == Catch::Approx(0.45f));
    // it falls to nothing at the limit without a step
    float prev = 1;
    for (float d = 0.8f * limit; d <= limit; d += 25)
    {
        const float f = FarNightFloor(FarFogFloor(d, 900, limit, 0.5f), d, limit, 0.9f, 1);
        REQUIRE(f <= prev + 1e-5f);
        prev = f;
    }
    REQUIRE(prev < 0.01f);
}

TEST_CASE("Kaiju planted blend keeps the standing foot where it is", "[kaiju][anim]")
{
    // A walk of step S per cycle blends in from a stand over the first
    // quarter of the cycle. The planted foot sits at -S * t in the clip, so
    // in the blended pose it is at body - f * S * t. Summed over the frames
    // the body must have advanced f * S * t for the foot to stay at zero.
    const float S = 14.0f;
    const int frames = 60;
    float body = 0, bodyStock = 0, t = 0, f = 0;
    float worstFoot = 0, worstFootStock = 0;
    for (int i = 0; i < frames; i++)
    {
        const float t1 = t + 0.25f / frames;
        const float f1 = f + 1.0f / frames;
        body += S * PlantedBlendShare(t, t1, f, f1);
        bodyStock += S * (t1 - t) * f;
        t = t1;
        f = f1;
        worstFoot = std::fmax(worstFoot, std::fabs(body - f * S * t));
        worstFootStock = std::fmax(worstFootStock, std::fabs(bodyStock - f * S * t));
    }
    REQUIRE(worstFoot < 1e-3f);
    // the stock share leaves the foot dragged back by about half the step taken
    REQUIRE(worstFootStock > 1.5f);
    // once the blend is over the share is the stock one
    REQUIRE(PlantedBlendShare(0.5f, 0.6f, 1, 1) == Catch::Approx(0.1f));
}
