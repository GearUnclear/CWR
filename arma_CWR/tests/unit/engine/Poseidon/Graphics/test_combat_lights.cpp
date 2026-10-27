#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Core/GpuTimings.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/CombatLights.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/FrameLights.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/PhotoCamera.hpp>
#include <Poseidon/Graphics/Remaster/PostStages.hpp>
#include <Poseidon/Graphics/Remaster/ScreenReflections.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Combat light (Graphics/Rendering/Lighting/CombatLights.hpp): the envelopes and their finish times, the day scaling,
// the strength from the ammo, the shadowed lights picked with hysteresis (no
// flip between two fires), the set aged by the simulation (a paused game holds
// it), the shadow mask's mapping at the screen centre and edges (an earlier
// version's seam down the centre column), the frame list's combat entries,
// the flash's exposure response, the gates and the backend contracts that keep
// the switch off equal to the frame before it.

using namespace Poseidon::lighting;
using Catch::Approx;

namespace
{

std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    if (!f.is_open())
        return {};
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path EngineDir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine";
}

std::string FunctionBody(const std::string& source, const std::string& signature)
{
    const size_t start = source.find(signature);
    if (start == std::string::npos)
        return {};
    const size_t end = source.find("\n}\n", start);
    return source.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

float Luminance(const float rgb[3])
{
    return 0.2126f * rgb[0] + 0.7152f * rgb[1] + 0.0722f * rgb[2];
}

CombatLightSample At(CombatLightKind kind, float age, float strength = 1.0f, std::uint32_t seed = 7)
{
    return EvaluateCombatLight(kind, strength, age, seed, CombatTuning{});
}

// A camera turned by `yawDeg` about the vertical (clockwise from +z toward
// +x) and pitched `pitchDeg`: world to view rows (x right, y up, z forward).
CombatMaskCamera MaskCamera(double x, double y, double z, float yawDeg, float pitchDeg, int w, int h)
{
    const float yaw = yawDeg * 3.14159265f / 180.0f, pitch = pitchDeg * 3.14159265f / 180.0f;
    const float f[3] = {std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch)};
    // right = up x forward, normalised; up = forward x right
    float r[3] = {f[2], 0.0f, -f[0]};
    const float rl = std::sqrt(r[0] * r[0] + r[2] * r[2]);
    r[0] /= rl;
    r[2] /= rl;
    const float u[3] = {f[1] * r[2] - f[2] * r[1], f[2] * r[0] - f[0] * r[2], f[0] * r[1] - f[1] * r[0]};
    CombatMaskCamera c;
    c.pos[0] = x;
    c.pos[1] = y;
    c.pos[2] = z;
    for (int i = 0; i < 3; ++i)
    {
        c.rot[0][i] = r[i];
        c.rot[1][i] = u[i];
        c.rot[2][i] = f[i];
    }
    c.p11 = 1.0f / std::tan(0.7f);
    c.p22 = c.p11 * static_cast<float>(w) / static_cast<float>(h);
    c.targetW = w;
    c.targetH = h;
    return c;
}

// The camera-relative point (to `cur`) that `cam` sees at frame-target pixel
// coordinate (px, py) at view depth z.
void PointAtPixel(const CombatMaskCamera& cam, const double cur[3], float px, float py, float z, float out[3])
{
    const float u = px / static_cast<float>(cam.targetW), v = py / static_cast<float>(cam.targetH);
    const float ndcX = (u - cam.rect[0]) / cam.rect[2] * 2.0f - 1.0f;
    const float ndcY = (v - cam.rect[1]) / cam.rect[3] * 2.0f - 1.0f;
    const float vp[3] = {ndcX * z / cam.p11, ndcY * z / cam.p22, z};
    for (int c = 0; c < 3; ++c)
    {
        // view to world: the transpose of the rows
        const double w = cam.rot[0][c] * vp[0] + cam.rot[1][c] * vp[1] + cam.rot[2][c] * vp[2] + cam.pos[c];
        out[c] = static_cast<float>(w - cur[c]);
    }
}

} // namespace

TEST_CASE("Combat lights: each kind's envelope, colour and finish", "[remaster][CombatLights]")
{
    for (int k = 0; k < kCombatLightKinds; ++k)
    {
        const auto kind = static_cast<CombatLightKind>(k);
        INFO(CombatLightKindName(kind));
        const CombatLightSample s = At(kind, 0.4f);
        // Colours of unit luminance: the intensity carries the power.
        CHECK(Luminance(s.rgb) == Approx(1.0f).margin(1e-4));
        CHECK(s.core > 0.0f);
        CHECK(s.range >= s.core * 4.0f - 1e-4f);
        CHECK(s.range <= kCombatFlareRange);
    }
    // Muzzle: full the frame the round leaves, e^-1 after 30 ms, done by 0.18 s.
    const CombatLightSample m0 = At(CombatLightKind::Muzzle, 0.0f);
    const CombatLightSample m1 = At(CombatLightKind::Muzzle, 0.030f);
    CHECK(m1.intensity / m0.intensity == Approx(std::exp(-1.0f)).epsilon(1e-3));
    CHECK_FALSE(At(CombatLightKind::Muzzle, 0.17f).finished);
    CHECK(At(CombatLightKind::Muzzle, 0.19f).finished);
    CHECK(m0.range <= 25.0f);
    // Cannon: a fast flash with a slower tail, done by 0.6 s.
    CHECK(At(CombatLightKind::Cannon, 0.1f).intensity < At(CombatLightKind::Cannon, 0.0f).intensity);
    CHECK(At(CombatLightKind::Cannon, 0.61f).finished);
    // Explosion: the white-hot flash at the peak, then the warm fireball;
    // the flash knob raises the first flash and not the window.
    const CombatLightSample e0 = At(CombatLightKind::Explosion, 0.0f);
    const CombatLightSample e3 = At(CombatLightKind::Explosion, 0.3f);
    CHECK(e0.intensity > 10.0f * e3.intensity);
    CHECK(e0.rgb[2] > e3.rgb[2]); // white-hot, then warm
    CHECK(At(CombatLightKind::Explosion, 0.91f).finished);
    CombatTuning brighter;
    brighter.flash = 2.0f;
    const CombatLightSample eb = EvaluateCombatLight(CombatLightKind::Explosion, 1.0f, 0.0f, 7, brighter);
    CHECK(eb.intensity > e0.intensity);
    // Continuous kinds never finish on their own.
    for (CombatLightKind k :
         {CombatLightKind::Fire, CombatLightKind::Tracer, CombatLightKind::Flare, CombatLightKind::Motor})
    {
        INFO(CombatLightKindName(k));
        CHECK_FALSE(At(k, 300.0f).finished);
        CHECK_FALSE(CombatLightIsBurst(k));
    }
    // A flare lights up over a third of a second.
    CHECK(At(CombatLightKind::Flare, 0.1f).intensity < At(CombatLightKind::Flare, 0.5f).intensity);
    // Before its birth a burst has finished (nothing to light).
    CHECK(At(CombatLightKind::Explosion, -0.1f).finished);
}

TEST_CASE("Combat lights: a fire breathes smoothly, never a new brightness every frame", "[remaster][CombatLights]")
{
    // The classic fire takes a random 0.8 +- 0.2 every frame (a mean step of
    // about 17% between 1/30 s frames, up to 67%); the combat fire breathes
    // and licks: a few percent per frame on average, a fifth at most.
    float worst = 0.0f, sum = 0.0f;
    float prev = At(CombatLightKind::Fire, 0.0f).intensity;
    for (int i = 1; i <= 900; ++i)
    {
        const float v = At(CombatLightKind::Fire, i / 30.0f).intensity;
        const float step = std::fabs(v - prev) / prev;
        worst = std::max(worst, step);
        sum += step;
        prev = v;
    }
    CHECK(sum / 900.0f < 0.06f);
    CHECK(worst < 0.25f);
    // The fire's light is less saturated than an earlier beige-making one,
    // and one step under (1.0, 0.66, 0.39), whose light read red over blue
    // 1.43 to 1.53 on the ground round a camp fire, over the 1.4 aimed for.
    const CombatLightSample f = At(CombatLightKind::Fire, 1.0f);
    CHECK(f.rgb[0] / f.rgb[2] < 1.0f / 0.14f);
    CHECK(f.rgb[0] / f.rgb[2] <= 1.0f / 0.45f + 1e-3f);
    CHECK(f.rgb[0] / f.rgb[1] <= 1.0f / 0.70f + 1e-3f);
}

TEST_CASE("Combat lights: a burst's flash lasts as long as Classic's whiteout and no burst lights the ground khaki",
          "[remaster][CombatLights]")
{
    // A classic shell burst still whitens the frame 0.13 s (four steps at 30
    // fps) after it: the flash holds through that frame (a 60 ms
    // fall left it 14 to 24 levels under Classic at eye height), and a quarter
    // of a second after the burst it has fallen well below its peak.
    // (The fireball under the flash fades from the start: within 6%.)
    const float e0 = At(CombatLightKind::Explosion, 0.0f).intensity;
    for (int step = 1; step <= 4; ++step)
    {
        INFO("step " << step);
        CHECK(At(CombatLightKind::Explosion, step / 30.0f).intensity > 0.94f * e0);
    }
    CHECK(At(CombatLightKind::Explosion, kExplosionFlashHold).intensity > 0.94f * e0);
    CHECK(At(CombatLightKind::Explosion, 8.0f / 30.0f).intensity < 0.3f * e0);
    // Where the flash response does not run (the photoreal camera's dusk:
    // CombatTuning::flashHold 0) the flash falls from its birth: the same
    // peak, about half of it a step later, and the frame 0.13 s after
    // the burst (firefight_2's) lit at under a fifth of the held flash.
    CombatTuning noHold;
    noHold.flashHold = 0.0f;
    CHECK(CombatTuning{}.flashHold == kExplosionFlashHold);
    const float n0 = EvaluateCombatLight(CombatLightKind::Explosion, 1.0f, 0.0f, 7, noHold).intensity;
    CHECK(n0 == Approx(e0));
    const float n1 = EvaluateCombatLight(CombatLightKind::Explosion, 1.0f, 1.0f / 30.0f, 7, noHold).intensity;
    CHECK(n1 < 0.7f * e0);
    CHECK(n1 > 0.35f * e0);
    const float n4 = EvaluateCombatLight(CombatLightKind::Explosion, 1.0f, 4.0f / 30.0f, 7, noHold).intensity;
    CHECK(n4 < 0.2f * At(CombatLightKind::Explosion, 4.0f / 30.0f).intensity);
    // Red over blue of the light: the fire's warmth (1 / 0.45) is what lit
    // ground takes at about 1.4 on screen; a burst stays at or under it at every
    // age, and the white-hot flash well under it (an earlier orange
    // fireball, 1 / 0.11, and muzzle, 1 / 0.42, turned the ground khaki).
    const CombatLightSample fire = At(CombatLightKind::Fire, 1.0f);
    const float fireRb = fire.rgb[0] / fire.rgb[2];
    for (CombatLightKind k : {CombatLightKind::Muzzle, CombatLightKind::Cannon, CombatLightKind::Explosion})
    {
        INFO(CombatLightKindName(k));
        for (int i = 0; i <= 27; ++i)
        {
            const CombatLightSample s = At(k, i / 30.0f);
            CHECK(s.rgb[0] / s.rgb[2] <= fireRb + 1e-3f);
        }
        const CombatLightSample s0 = At(k, 0.0f);
        CHECK(s0.rgb[0] / s0.rgb[2] < 0.75f * fireRb);
    }
}

TEST_CASE("Combat lights: by day continuous lights fade to 12%, flashes to a fifth", "[remaster][CombatLights]")
{
    CHECK(CombatDayScale(CombatLightKind::Fire, 0.0f) == Approx(0.12f));
    CHECK(CombatDayScale(CombatLightKind::Fire, 1.0f) == Approx(1.0f));
    CHECK(CombatDayScale(CombatLightKind::Explosion, 0.0f) == Approx(0.2f));
    CHECK(CombatDayScale(CombatLightKind::Muzzle, 1.0f) == Approx(1.0f));
    CHECK(CombatDayScale(CombatLightKind::Flare, 0.5f) == Approx(0.56f));
}

TEST_CASE("Combat lights: under the photoreal camera they follow its night, so a dusk is no full night",
          "[remaster][CombatLights]")
{
    namespace rm = Poseidon::remaster;
    // Without the camera: Classic's night effect.
    CHECK(CombatNight(1.0f, false, 0.0f) == 1.0f);
    CHECK(CombatNight(0.3f, false, 1.0f) == Approx(0.3f));
    // The dusk firefight (19:35, sun 1.7 degrees under): Classic's night is
    // already 1, the camera's still 0. A fire took its full night power there
    // and lit the plain behind it brighter than the sky; now the day scale.
    const float dusk = rm::CameraNight(-1.7f);
    CHECK(dusk == 0.0f);
    CHECK(CombatNight(1.0f, true, dusk) == 0.0f);
    CHECK(CombatDayScale(CombatLightKind::Fire, CombatNight(1.0f, true, dusk)) == Approx(0.12f));
    CHECK(CombatDayScale(CombatLightKind::Muzzle, CombatNight(1.0f, true, dusk)) == Approx(0.2f));
    // The night (a sun 50 degrees under, any sun 12 or more under)
    // keeps the night power.
    CHECK(CombatNight(1.0f, true, rm::CameraNight(-50.0f)) == 1.0f);
    CHECK(CombatNight(1.0f, true, rm::CameraNight(-12.0f)) == 1.0f);
    // The blue hour between: a share.
    const float blue = CombatNight(1.0f, true, rm::CameraNight(-8.0f));
    CHECK(blue > 0.0f);
    CHECK(blue < 1.0f);
    // Never more night than Classic's (a day frame stays a day frame).
    CHECK(CombatNight(0.0f, true, 1.0f) == 0.0f);
    CHECK(CombatNight(2.0f, false, 0.0f) == 1.0f);
}

TEST_CASE("Combat lights: a ball lights a surface by the part of it above that surface's horizon",
          "[remaster][CombatLights]")
{
    // Wholly above the horizon: the point light's cosine.
    CHECK(CombatBallCosine(0.8f, 1.0f, 10.0f) == Approx(0.8f));
    CHECK(CombatBallCosine(1.0f, 0.3f, 2.0f) == Approx(1.0f));
    // At its surface and inside it: (1 + cos) / 2, all round (the muzzle's
    // flash lights the shooter's arm).
    CHECK(CombatBallCosine(0.0f, 1.0f, 0.5f) == Approx(0.5f));
    CHECK(CombatBallCosine(-0.6f, 1.0f, 1.0f) == Approx(0.2f));
    // Wholly below: nothing.
    CHECK(CombatBallCosine(-0.5f, 1.0f, 10.0f) == 0.0f);
    // Continuous where the ball meets the horizon and at its surface, and
    // growing with the cosine.
    for (float d : {1.001f, 1.5f, 3.0f, 10.0f, 40.0f})
    {
        const float s = 1.0f / d;
        CHECK(CombatBallCosine(s - 1e-4f, 1.0f, d) == Approx(s).margin(1e-3));
        CHECK(CombatBallCosine(-s + 1e-4f, 1.0f, d) == Approx(0.0f).margin(1e-3));
        float prev = 0.0f;
        for (int i = -20; i <= 20; ++i)
        {
            const float v = CombatBallCosine(static_cast<float>(i) / 20.0f, 1.0f, d);
            CHECK(v >= prev - 1e-5f);
            CHECK(v <= 1.0f);
            prev = v;
        }
    }
    CHECK(CombatBallCosine(0.0f, 1.0f, 1.0001f) == Approx(0.5f).margin(0.02f));
    // A ball centred on flat ground lights it far off by the rim of its top:
    // 2 core / (3 pi d), a quarter to a fifth of the former wrap's (cos + core
    // / d) / (1 + core / d), which drew the plain behind a burst as a band.
    for (float d : {5.0f, 10.0f, 20.0f})
    {
        const float s = 1.0f / d;
        CHECK(CombatBallCosine(0.0f, 1.0f, d) == Approx(2.0f * s / (3.0f * 3.14159265f)).epsilon(0.03));
        CHECK(CombatBallCosine(0.0f, 1.0f, d) < 0.27f * s / (1.0f + s));
    }
    // The firefight's shell burst (core 2.6 m, 3.5 m up) over the plain 20 m
    // off: wholly above that ground's horizon, the point light's cosine, a
    // third under the wrap's.
    const float d = std::sqrt(20.0f * 20.0f + 3.5f * 3.5f), c = 3.5f / d;
    CHECK(CombatBallCosine(c, 2.6f, d) == Approx(c));
    CHECK(CombatBallCosine(c, 2.6f, d) < 0.7f * (c + 2.6f / d) / (1.0f + 2.6f / d));
}

TEST_CASE("Combat lights: a flash's strength comes from the fired ammo", "[remaster][CombatLights]")
{
    CHECK(CombatMuzzleStrength(10.0f, true, 0.004f) == Approx(1.0f));
    CHECK(CombatMuzzleStrength(20.0f, true, 0.4f) == Approx(2.0f));
    CHECK(CombatMuzzleStrength(600.0f, false, 1.0f) == Approx(2.0f));
    // Without the ammo, the classic intensities tell the sources apart.
    CHECK(CombatMuzzleStrength(-1.0f, true, 0.004f) == Approx(1.0f));
    CHECK(CombatMuzzleStrength(-1.0f, true, 0.4f) == Approx(1.8f));
    CHECK(CombatMuzzleStrength(-1.0f, false, 1.0f) == Approx(1.5f));
    // A stronger round lights more.
    CHECK(At(CombatLightKind::Muzzle, 0.0f, 2.0f).intensity > At(CombatLightKind::Muzzle, 0.0f, 1.0f).intensity);
}

TEST_CASE("Combat lights: the ball falloff and its window", "[remaster][CombatLights]")
{
    const CombatLightSample s = At(CombatLightKind::Fire, 1.0f);
    // Inside the ball the light is near its core intensity, far off it falls
    // as the inverse square, and it ends at the window.
    CHECK(CombatIrradiance(s, 0.0f) == Approx(s.intensity));
    const float d = 0.5f * s.range;
    const float i1 = s.intensity * s.core * s.core;
    // window (1 - (1/4)^2)^2, the ball adds r0^2 to d^2
    CHECK(CombatIrradiance(s, d * d) == Approx(i1 / (d * d + s.core * s.core) * 0.87890625f).epsilon(1e-3));
    CHECK(CombatIrradiance(s, s.range * s.range) == 0.0f);
    CHECK(CombatIrradiance(s, 2.0f * s.range * s.range) == 0.0f);
    // The window ends where the light (a steady one: a tracer) falls to the
    // range floor.
    const CombatLightSample tr = At(CombatLightKind::Tracer, 1.0f);
    CHECK(tr.intensity * tr.core * tr.core / (tr.range * tr.range) == Approx(kCombatRangeFloor).epsilon(1e-3));
}

TEST_CASE("Combat lights: smoke takes the light softly; no light leaves it as it was", "[remaster][CombatLights]")
{
    CHECK(CombatParticleChannel(0.3f, 0.0f, 1.0f) == 0.3f);
    CHECK(CombatParticleChannel(0.3f, 1.0f, 0.0f) == 0.3f);
    float prev = 0.3f;
    for (float e : {0.01f, 0.1f, 1.0f, 10.0f, 100.0f})
    {
        const float v = CombatParticleChannel(0.3f, e, 1.0f);
        CHECK(v > prev);
        prev = v;
    }
    // Soft saturation: a fireball's core glows without running away.
    CHECK(CombatParticleChannel(0.3f, 1e4f, 1.0f) < 2.0f);
}

TEST_CASE("Combat lights: a fire lights its smoke in its own colour however close, brighter at the foot",
          "[remaster][CombatLights]")
{
    // Saturated one by one, the channels went toward the same value, so the
    // foot of a camp fire's column went white (a plume at (179, 166, 151)), and the 8-bit colour clipped each channel.
    // The colour version saturates on the peak and keeps the light's hue.
    const CombatLightSample fire = At(CombatLightKind::Fire, 1.0f);
    const float ambient[3] = {0.08f, 0.08f, 0.09f}; // a puff under the night's light
    auto lit = [&](float d, float tint, float out[3])
    {
        const float e = CombatIrradiance(fire, d * d) * CombatSmokeTransmittance(CombatLightKind::Fire, fire, d, 0.7f);
        const float irr[3] = {fire.rgb[0] * e, fire.rgb[1] * e, fire.rgb[2] * e};
        CombatParticleLight(ambient, irr, 3.0f, tint, out);
    };
    // No light: as it was.
    const float none[3] = {0.0f, 0.0f, 0.0f};
    float same[3];
    CombatParticleLight(ambient, none, 3.0f, 0.85f, same);
    for (int i = 0; i < 3; ++i)
        CHECK(same[i] == ambient[i]);
    float foot[3], mid[3], top[3];
    lit(0.8f, 0.0f, foot);
    lit(3.0f, 0.0f, mid);
    lit(6.0f, 0.0f, top);
    // Warm through: red over blue stays well above grey at the foot, where
    // the per-channel form had all three near its ceiling.
    CHECK(foot[0] / foot[2] > 1.3f);
    const float perChannel = CombatParticleChannel(0.08f, fire.rgb[0] * 40.0f, 3.0f) /
                             CombatParticleChannel(0.09f, fire.rgb[2] * 40.0f, 3.0f);
    CHECK(perChannel < 1.25f);
    // The foot glows past white (the backend carries it as the decal's glow)
    // and well above the smoke 3 m up, which is above the smoke 6 m up.
    CHECK(foot[0] > 1.5f);
    CHECK(foot[0] > 1.6f * mid[0]);
    CHECK(mid[0] > top[0]);
    // Bounded: the ceiling of the soft saturation.
    float core[3];
    const float huge[3] = {1e5f, 7e4f, 4.5e4f};
    CombatParticleLight(ambient, huge, 3.0f, 0.0f, core);
    CHECK(core[0] < 3.3f);
    // The tint deepens the hue toward the light's own, never brightens red.
    float tinted[3];
    lit(0.8f, 0.85f, tinted);
    CHECK(tinted[0] == Catch::Approx(foot[0]));
    CHECK(tinted[1] / tinted[0] < foot[1] / foot[0]);
    CHECK(tinted[2] / tinted[0] < foot[2] / foot[0]);
    // Under one by the peak: the hue stays.
    float c[3] = {2.0f, 1.2f, 0.6f};
    KeepHueUnderOne(c);
    CHECK(c[0] == Catch::Approx(1.0f));
    CHECK(c[1] == Catch::Approx(0.6f));
    CHECK(c[2] == Catch::Approx(0.3f));
    float d[3] = {0.5f, 0.4f, 0.3f};
    KeepHueUnderOne(d);
    CHECK(d[0] == 0.5f);
}

TEST_CASE("Combat lights: a burst's cloud whitens at its core, a fire's smoke keeps the fire's colour",
          "[remaster][CombatLights]")
{
    // With every combat light kept in its hue, a dusk shell burst's cloud went one
    // flat mustard yellow. A fire's light keeps its hue; a burst's light
    // saturates channel by channel toward white, as an incandescent core does.
    const float ambient[3] = {0.08f, 0.08f, 0.09f};
    const float none[3] = {0.0f, 0.0f, 0.0f};
    const CombatLightSample burst = At(CombatLightKind::Explosion, 0.15f);
    const float eb = CombatIrradiance(burst, 2.0f * 2.0f);
    const float b[3] = {burst.rgb[0] * eb, burst.rgb[1] * eb, burst.rgb[2] * eb};
    float mixed[3], hueKept[3];
    CombatParticleMix(ambient, none, b, 1.0f, 0.0f, mixed);
    CombatParticleLight(ambient, b, 1.0f, 0.0f, hueKept);
    // The burst's cloud close to it is near white; kept in its hue it was not.
    CHECK(mixed[0] / mixed[2] < 1.15f);
    CHECK(hueKept[0] / hueKept[2] > 1.2f);
    // Each channel is the per-channel form's.
    for (int i = 0; i < 3; ++i)
        CHECK(mixed[i] == Catch::Approx(CombatParticleChannel(ambient[i], b[i], 1.0f)));
    // A fire's light alone is the hue-kept form's, with its tint.
    const CombatLightSample fire = At(CombatLightKind::Fire, 1.0f);
    const float ef = CombatIrradiance(fire, 0.8f * 0.8f);
    const float f[3] = {fire.rgb[0] * ef, fire.rgb[1] * ef, fire.rgb[2] * ef};
    float fireMix[3], fireOnly[3];
    CombatParticleMix(ambient, f, none, 3.0f, 0.85f, fireMix);
    CombatParticleLight(ambient, f, 3.0f, 0.85f, fireOnly);
    for (int i = 0; i < 3; ++i)
        CHECK(fireMix[i] == Catch::Approx(fireOnly[i]));
    // No light: as it was.
    float same[3];
    CombatParticleMix(ambient, none, none, 1.0f, 0.85f, same);
    for (int i = 0; i < 3; ++i)
        CHECK(same[i] == Catch::Approx(ambient[i]));
}

TEST_CASE("Combat lights: the smoke inside a fire's flames glows as the flame", "[remaster][CombatLights]")
{
    // The classic fire's light lit the puffs round it past
    // white, and that yellow column was the flame; the combat light on smoke
    // only scatters, so under the camera a camp fire had no flame. A puff in
    // the flames' body emits a yellow-orange light, strongest at the light,
    // none a metre or so out, and only for a fire.
    const CombatLightSample fire = At(CombatLightKind::Fire, 1.0f);
    float at[3], half[3], out[3];
    CombatFlameEmission(CombatLightKind::Fire, fire, 0.0f, at);
    CombatFlameEmission(CombatLightKind::Fire, fire, 0.5f, half);
    CombatFlameEmission(CombatLightKind::Fire, fire, 1.5f, out);
    CHECK(at[0] > 1.5f);
    CHECK(at[0] > at[1]);
    CHECK(at[1] > 2.0f * at[2]);
    CHECK(half[0] > 0.0f);
    CHECK(half[0] < at[0]);
    CHECK(out[0] == 0.0f);
    // A stronger fire's flames glow more; a flash, a flare or no fire not at all.
    float big[3];
    CombatFlameEmission(CombatLightKind::Fire, At(CombatLightKind::Fire, 1.0f, 2.0f), 0.0f, big);
    CHECK(big[0] > 1.5f * at[0]);
    float flash[3];
    CombatFlameEmission(CombatLightKind::Explosion, At(CombatLightKind::Explosion, 0.0f), 0.0f, flash);
    CHECK(flash[0] == 0.0f);
    CombatFlameEmission(CombatLightKind::Flare, At(CombatLightKind::Flare, 1.0f), 0.0f, flash);
    CHECK(flash[0] == 0.0f);
}

TEST_CASE("Combat lights: a burning wreck's black smoke glows at its base and its column stays dark",
          "[remaster][CombatLights]")
{
    // In an earlier version the embers lit every particle of the wreck's column
    // at full strength however dark or high it was, and at night the black
    // plume read at sky level (Classic 4.2, on 10.3, the sky 10.0). A fire's
    // light crosses its own smoke: black soot lets it reach the column's foot
    // only, pale smoke carries it higher.
    const CombatLightSample embers = At(CombatLightKind::Fire, 1.0f, 2.5f); // a Ural's: 0.9 x density x size
    const CombatLightSample camp = At(CombatLightKind::Fire, 1.0f, 1.0f);
    const float soot = 0.075f; // the wreck's (0.15, 0.15, 0.10) x a random 0..1 (Object::Destroy)
    const float pale = 0.7f;   // a camp fire's smoke as it greys and rises (its config table)
    const float night = 0.05f; // a particle's display level under the night ambient
    auto lit = [&](const CombatLightSample& s, CombatLightKind kind, float d, float albedo)
    {
        const float e = CombatIrradiance(s, d * d) * CombatSmokeTransmittance(kind, s, d, albedo);
        return CombatParticleChannel(night, e, 1.0f);
    };
    // The base round the embers glows.
    CHECK(lit(embers, CombatLightKind::Fire, 1.0f, soot) > 0.5f);
    // From 6 m up the column is the night's (without the cut 0.72: sky level).
    CHECK(lit(embers, CombatLightKind::Fire, 6.0f, soot) < 1.1f * night);
    CHECK(lit(embers, CombatLightKind::Fire, 10.0f, soot) < 1.02f * night);
    // A camp fire's pale smoke is still lit well above the flames.
    CHECK(lit(camp, CombatLightKind::Fire, 4.0f, pale) > 0.3f);
    // The reach: within the core all of it, then less, the paler the further.
    CHECK(CombatSmokeTransmittance(CombatLightKind::Fire, embers, embers.core * 0.5f, soot) == 1.0f);
    CHECK(CombatSmokeTransmittance(CombatLightKind::Fire, embers, 3.0f, soot) <
          CombatSmokeTransmittance(CombatLightKind::Fire, embers, 2.0f, soot));
    CHECK(CombatSmokeReach(embers.core, pale) > 4.0f * CombatSmokeReach(embers.core, soot));
    // A shell burst's light crosses its own cloud too: the
    // base round its ball is lit, the far side of its cloud less.
    const CombatLightSample burst = At(CombatLightKind::Explosion, 0.3f);
    CHECK(CombatSmokeTransmittance(CombatLightKind::Explosion, burst, burst.core * 0.5f, soot) == 1.0f);
    CHECK(CombatSmokeTransmittance(CombatLightKind::Explosion, burst, burst.core + 6.0f, 0.2f) < 0.5f);
    // A muzzle or cannon flash, a flare, a tracer or a motor lights the smoke
    // from the air.
    for (CombatLightKind k : {CombatLightKind::Muzzle, CombatLightKind::Cannon, CombatLightKind::Flare,
                              CombatLightKind::Tracer, CombatLightKind::Motor})
    {
        INFO(CombatLightKindName(k));
        CHECK(CombatSmokeTransmittance(k, At(k, 0.0f), 8.0f, soot) == 1.0f);
    }
}

TEST_CASE("Combat lights: a fireball's puff glows from white-hot to orange and cools to dark soot",
          "[remaster][CombatLights]")
{
    // CfgCloudlets Explosion: born at 2500 K, its table tops at 2200 K.
    float albedo[3], hot[3], warm[3], cool[3], cold[3];
    REQUIRE(FireballPuff(2500.0f, 2200.0f, albedo, hot));
    REQUIRE(FireballPuff(2000.0f, 2200.0f, albedo, warm));
    REQUIRE(FireballPuff(1600.0f, 2200.0f, albedo, cool));
    REQUIRE(FireballPuff(850.0f, 2200.0f, albedo, cold));
    // White-hot past the puff's white (its glow carries above 1): red and
    // green near the cap, blue well up, always brighter than the ground a
    // burst lights at dusk.
    CHECK(hot[0] > 1.9f);
    CHECK(hot[1] > 1.5f);
    CHECK(hot[2] > 0.5f);
    // 2000 K: orange, red leading.
    CHECK(warm[0] > 0.4f);
    CHECK(warm[1] < 0.5f * warm[0]);
    CHECK(warm[2] < 0.05f * warm[0]);
    // 1600 K: a dim red glow, far under the core.
    CHECK(cool[0] < 0.1f * hot[0]);
    CHECK(cool[1] < 0.3f * cool[0]);
    CHECK(cool[2] < 0.1f * cool[0]);
    // The black body: red-orange at 1500 K, yellower and bluer hotter.
    float b15[3], b25[3];
    BlackBodyRgb(1500.0f, b15);
    BlackBodyRgb(2500.0f, b25);
    CHECK(b15[1] == Catch::Approx(0.155f).margin(0.01));
    CHECK(b25[1] == Catch::Approx(0.36f).margin(0.02));
    CHECK(b25[2] == Catch::Approx(0.06f).margin(0.01));
    // Under the glow's start nothing: smoke.
    CHECK(cold[0] == 0.0f);
    CHECK(cold[1] == 0.0f);
    // What it scatters is dark grey-brown soot, never its table's colour.
    CHECK(albedo[0] < 0.3f);
    CHECK(albedo[0] >= albedo[1]);
    CHECK(albedo[1] >= albedo[2]);
    // Monotone in temperature.
    float prev = 0.0f;
    for (float t = 800.0f; t <= 2600.0f; t += 50.0f)
    {
        float a[3], e[3];
        FireballPuff(t, 2200.0f, a, e);
        CHECK(e[0] >= prev);
        CHECK(e[0] <= kFireballEmitCap);
        prev = e[0];
    }
    // Crater smoke (its table tops at 500) and a camp fire's or a smoke
    // shell's smoke (1000: orange at its foot, then grey) are no fireball.
    float a[3] = {9.0f, 9.0f, 9.0f}, e[3] = {9.0f, 9.0f, 9.0f};
    CHECK_FALSE(FireballPuff(500.0f, 500.0f, a, e));
    CHECK_FALSE(FireballPuff(1000.0f, 1000.0f, a, e));
    // A gun's fire (WeaponFireGun, its table to 4200 on its own scale).
    CHECK_FALSE(FireballPuff(4500.0f, 4200.0f, a, e));
    CHECK(a[0] == 9.0f);
    CHECK(e[0] == 9.0f);
}

TEST_CASE("Combat lights: a steady light reads against the twilight under the camera", "[remaster][CombatLights]")
{
    // The night as before.
    CHECK(CombatTwilight(-6.0f) == 1.0f);
    CHECK(CombatTwilight(-20.0f) == 1.0f);
    // Tenfold per 2.5 degrees of sun above it.
    CHECK(CombatTwilight(-3.5f) == Catch::Approx(0.1f).epsilon(1e-3));
    CHECK(CombatTwilight(-4.75f) == Catch::Approx(std::sqrt(0.1f)).epsilon(1e-3));
    // The firefights' dusk (sun 1.7 degrees under) and the day: the floor.
    CHECK(CombatTwilight(-1.7f) == Catch::Approx(kTwilightFloor));
    CHECK(CombatTwilight(40.0f) == Catch::Approx(kTwilightFloor));
    float prev = 2.0f;
    for (float e = -15.0f; e <= 10.0f; e += 0.25f)
    {
        const float s = CombatTwilight(e);
        CHECK(s <= prev);
        prev = s;
    }
}

TEST_CASE("Combat lights: the frame's list never cuts a fire, a flare or a motor for bursts and tracers",
          "[remaster][CombatLights]")
{
    // While combat light is on the classic light of a fire's source lights
    // nothing, so a fire cut from the twelve went dark: a camp fire 100 m off
    // (score 0.00175) behind twelve tracers 10 m off (0.0025 each).
    const CombatLightSample tracer = At(CombatLightKind::Tracer, 0.5f);
    const CombatLightSample fire = At(CombatLightKind::Fire, 1.0f);
    CHECK(ShadowScore(tracer, 100.0f) > ShadowScore(fire, 100.0f * 100.0f));
    std::vector<CombatLightKind> kinds(12, CombatLightKind::Tracer);
    kinds.push_back(CombatLightKind::Fire); // strongest first: the fire last
    std::vector<int> keep = KeepCombatLights(kinds.data(), static_cast<int>(kinds.size()), kMaxCombatFrameLights);
    REQUIRE(static_cast<int>(keep.size()) == kMaxCombatFrameLights);
    CHECK(std::find(keep.begin(), keep.end(), 12) != keep.end());
    CHECK(std::find(keep.begin(), keep.end(), 11) == keep.end()); // the weakest tracer goes
    CHECK(std::is_sorted(keep.begin(), keep.end()));              // strongest first still
    // Flares and missile motors too; bursts are cut by strength.
    for (CombatLightKind k : {CombatLightKind::Flare, CombatLightKind::Motor})
    {
        INFO(CombatLightKindName(k));
        CHECK(CombatLightIsSteady(k));
        std::vector<CombatLightKind> ks(13, CombatLightKind::Muzzle);
        ks.back() = k;
        keep = KeepCombatLights(ks.data(), 13, kMaxCombatFrameLights);
        CHECK(std::find(keep.begin(), keep.end(), 12) != keep.end());
    }
    for (CombatLightKind k :
         {CombatLightKind::Muzzle, CombatLightKind::Cannon, CombatLightKind::Explosion, CombatLightKind::Tracer})
        CHECK_FALSE(CombatLightIsSteady(k));
    // Under the cap all stay; over it in fires alone, the strongest fires.
    keep = KeepCombatLights(kinds.data(), 5, kMaxCombatFrameLights);
    CHECK(keep.size() == 5);
    std::vector<CombatLightKind> fires(14, CombatLightKind::Fire);
    keep = KeepCombatLights(fires.data(), 14, kMaxCombatFrameLights);
    REQUIRE(static_cast<int>(keep.size()) == kMaxCombatFrameLights);
    CHECK(keep.back() == kMaxCombatFrameLights - 1);
}

TEST_CASE("Combat lights: a flash at the camera opens the exposure, at night only, within a limit",
          "[remaster][CombatLights]")
{
    CHECK(FlashExposureGain(0.0f, 1.0f) == 1.0f);
    CHECK(FlashExposureGain(5.0f, 0.0f) == 1.0f);
    float prev = 1.0f;
    for (float e : {0.05f, 0.2f, 1.0f, 5.0f, 100.0f})
    {
        const float g = FlashExposureGain(e, 1.0f);
        CHECK(g > prev);
        CHECK(g < kFlashExposureMax);
        prev = g;
    }
    // Shell bursts and cannon open it, small arms do not.
    CHECK(CombatLightOpensExposure(CombatLightKind::Explosion));
    CHECK(CombatLightOpensExposure(CombatLightKind::Cannon));
    CHECK_FALSE(CombatLightOpensExposure(CombatLightKind::Muzzle));
    CHECK_FALSE(CombatLightOpensExposure(CombatLightKind::Fire));
    // Proportional to the irradiance for a faint flash.
    CHECK(FlashExposureGain(0.001f, 1.0f) - 1.0f == Approx(kFlashExposurePerIrradiance * 0.001f).epsilon(0.01));
    // The response holds a moment after the flash, then lets go; paused, it
    // holds.
    CHECK(FlashHeldIrradiance(0.0f, 1.0f, 1.0f / 30.0f) == 1.0f);
    const float held = FlashHeldIrradiance(1.0f, 0.2f, 1.0f / 30.0f);
    CHECK(held == Approx(std::exp(-1.0f / 30.0f / kFlashExposureHold)));
    CHECK(held > 0.2f);
    CHECK(FlashHeldIrradiance(1.0f, 0.0f, 0.0f) == 1.0f);
    CHECK(FlashHeldIrradiance(1.0f, 0.0f, 1.0f) < 0.001f);
}

TEST_CASE("Combat lights: two flickering fires keep their shadow channels", "[remaster][CombatLights]")
{
    // An earlier version picked the strongest light every frame: the
    // shadow flipped between the campfire and the burning truck whenever their
    // flicker crossed. With one channel, the shadowed fire keeps it unless the
    // other exceeds it 1.5x for 0.3 s.
    ShadowSelection sel;
    int flips = 0;
    std::uint32_t last = 0;
    for (int i = 0; i < 600; ++i)
    {
        const float t = i / 30.0f;
        const ShadowCandidate c[2] = {{1, 1.0f + 0.3f * std::sin(9.0f * t)}, {2, 1.0f + 0.3f * std::cos(7.0f * t)}};
        UpdateShadowSelection(sel, c, 2, 1.0f / 30.0f, 1);
        if (last != 0 && sel.ids[0] != last)
            ++flips;
        last = sel.ids[0];
    }
    CHECK(flips == 0);
    CHECK(last != 0);
}

TEST_CASE("Combat lights: a much stronger light takes the weakest channel after the hold time",
          "[remaster][CombatLights]")
{
    ShadowSelection sel;
    const ShadowCandidate a[2] = {{1, 1.0f}, {2, 0.9f}};
    int ch[2];
    UpdateShadowSelection(sel, a, 2, 0.0f, 2, ch);
    // Empty channels take the strongest at once.
    CHECK(ch[0] == 0);
    CHECK(ch[1] == 1);
    // A third light at 1.6x the weakest waits 0.3 s of simulation.
    const ShadowCandidate b[3] = {{1, 1.0f}, {2, 0.9f}, {3, 1.6f}};
    int ch3[3];
    float waited = 0.0f;
    for (int i = 0; i < 8; ++i)
    {
        UpdateShadowSelection(sel, b, 3, 1.0f / 30.0f, 2, ch3);
        waited += 1.0f / 30.0f;
        CHECK(ch3[2] == -1);
    }
    for (int i = 0; i < 4; ++i)
        UpdateShadowSelection(sel, b, 3, 1.0f / 30.0f, 2, ch3);
    CHECK(ch3[2] == 1); // it took the weaker light's channel
    CHECK(ch3[1] == -1);
    CHECK(ch3[0] == 0); // the stronger one kept its own
    // Paused (dt 0), a challenger never takes a channel.
    ShadowSelection paused;
    UpdateShadowSelection(paused, a, 2, 0.0f, 2);
    for (int i = 0; i < 100; ++i)
        UpdateShadowSelection(paused, b, 3, 0.0f, 2, ch3);
    CHECK(ch3[2] == -1);
    // Only 1.4x: never.
    ShadowSelection mild;
    const ShadowCandidate c[3] = {{1, 1.0f}, {2, 0.9f}, {3, 1.25f}};
    UpdateShadowSelection(mild, a, 2, 0.0f, 2);
    for (int i = 0; i < 100; ++i)
        UpdateShadowSelection(mild, c, 3, 1.0f / 30.0f, 2, ch3);
    CHECK(ch3[2] == -1);
    // A light that goes out frees its channel at once.
    const ShadowCandidate d[2] = {{2, 0.9f}, {4, 0.1f}};
    int ch2[2];
    UpdateShadowSelection(mild, d, 2, 1.0f / 30.0f, 2, ch2);
    CHECK(ch2[0] >= 0);
    CHECK(ch2[1] >= 0);
}

TEST_CASE("Combat lights: the set ages by the simulation step and drops finished bursts", "[remaster][CombatLights]")
{
    CombatLightSet set;
    const float p[3] = {10.0f, 2.0f, 20.0f};
    const std::uint32_t burst = set.Spawn(CombatLightKind::Muzzle, p, 1.0f);
    const std::uint32_t fire = set.Create(CombatLightKind::Fire, p, 1.0f);
    REQUIRE(set.Records().size() == 2);
    // Paused (no step) ages nothing.
    set.Tick(0.0f);
    set.Tick(-1.0f);
    CHECK(set.Find(burst)->age == 0.0f);
    CHECK(set.SimTime() == 0.0);
    for (int i = 0; i < 6; ++i)
        set.Tick(1.0f / 30.0f);
    CHECK(set.Find(burst) == nullptr); // a muzzle flash is over after 0.18 s
    REQUIRE(set.Find(fire) != nullptr);
    CHECK(set.Find(fire)->age == Approx(0.2f));
    CHECK(set.SimTime() == Approx(0.2));
    // The owner moves and removes its own light.
    const float q[3] = {1.0f, 2.0f, 3.0f};
    CHECK(set.Update(fire, q, 2.0f));
    CHECK(set.Find(fire)->pos[2] == 3.0f);
    set.Spawn(CombatLightKind::Explosion, p, 1.0f);
    set.ClearBursts();
    CHECK(set.Records().size() == 1);
    set.Remove(fire);
    CHECK(set.Records().empty());
    CHECK_FALSE(set.Update(fire, q, 1.0f));
}

TEST_CASE("Combat lights: a long step ages the set by the longest step, so bursts end under time acceleration",
          "[remaster][CombatLights]")
{
    // setAccTime 4 (the Showcase's escalation chapter) at a frame under 16 fps:
    // World::Simulate passes steps of about 0.27 s. Each ages the set by
    // kCombatMaxTickStep, so a muzzle burst (over after 0.18 s) is gone after
    // one step and an explosion within its envelope, and the set stays as
    // small as the shots fired since its bursts ended.
    CombatLightSet set;
    const float p[3] = {10.0f, 2.0f, 20.0f};
    const std::uint32_t muzzle = set.Spawn(CombatLightKind::Muzzle, p, 1.0f);
    const std::uint32_t fire = set.Create(CombatLightKind::Fire, p, 1.0f);
    set.Tick(0.27f);
    CHECK(set.Find(muzzle) == nullptr);
    REQUIRE(set.Find(fire) != nullptr);
    CHECK(set.Find(fire)->age == Approx(kCombatMaxTickStep));
    CHECK(set.SimTime() == Approx(kCombatMaxTickStep));
    // A clock jump (a load, a scripted time pin) moves the set on by no more.
    set.Tick(5.0f);
    CHECK(set.Find(fire)->age == Approx(2.0f * kCombatMaxTickStep));
    // A volley and a shell every long step: the bursts leave as they end.
    size_t most = 0;
    for (int i = 0; i < 40; ++i)
    {
        for (int k = 0; k < 5; ++k)
            set.Spawn(CombatLightKind::Muzzle, p, 1.0f);
        set.Spawn(CombatLightKind::Explosion, p, 1.0f);
        set.Tick(0.27f);
        most = std::max(most, set.Records().size());
    }
    CHECK(most < 30);
    for (int i = 0; i < 40; ++i)
        set.Tick(0.27f);
    CHECK(set.Records().size() == 1); // the fire alone
}

TEST_CASE("Combat lights: evaluation applies the day scale and a flare's tint", "[remaster][CombatLights]")
{
    CombatLightSet set;
    const float p[3] = {0.0f, 0.0f, 0.0f};
    const std::uint32_t flare = set.Create(CombatLightKind::Flare, p, 1.0f);
    set.SetTint(flare, 1.0f, 0.2f, 0.2f); // a red flare
    for (int i = 0; i < 30; ++i)
        set.Tick(1.0f / 30.0f);
    set.Evaluate(CombatTuning{}, 1.0f);
    const CombatLightSample night = set.Find(flare)->sample;
    CHECK(night.rgb[0] > 3.0f * night.rgb[1]);
    set.Evaluate(CombatTuning{}, 0.0f);
    CHECK(set.Find(flare)->sample.intensity == Approx(0.12f * night.intensity));
    CombatTuning t;
    t.smoke = 0.5f;
    set.Evaluate(t, 1.0f);
    CHECK(set.Tuning().smoke == 0.5f);
}

TEST_CASE("Combat lights: an owner's handle creates, moves and removes one light", "[remaster][CombatLights]")
{
    CombatLightSet& set = GCombatLights();
    const size_t before = set.Records().size();
    {
        CombatLightHandle h;
        const float p[3] = {1.0f, 2.0f, 3.0f};
        h.Set(CombatLightKind::Tracer, p, 1.0f);
        h.Set(CombatLightKind::Tracer, p, 0.5f);
        CHECK(set.Records().size() == before + 1);
        REQUIRE(set.Find(h.Id()) != nullptr);
        CHECK(set.Find(h.Id())->strength == 0.5f);
        // A copy of an owner does not take its light.
        CombatLightHandle copy(h);
        CHECK_FALSE(copy.Active());
        CHECK(set.Records().size() == before + 1);
    }
    CHECK(set.Records().size() == before);
}

TEST_CASE("Combat lights: the shadow mask maps the screen centre and edges without a seam", "[remaster][CombatLights]")
{
    // An earlier version showed a 3 to 4.5 level seam down the exact
    // screen-centre column. The mask is half resolution: texel i holds the
    // 2x2 block of frame-target pixels from 2i, so pixel coordinate x must map
    // to mask coordinate x / 2 exactly, at the centre (NDC 0) as at the edges,
    // for even and odd sizes; then the two pixel columns either side of the
    // centre read their own texels symmetrically.
    for (int w : {1920, 1921, 1440})
    {
        const int h = 1080;
        INFO("frame " << w << "x" << h);
        const CombatMaskCamera cam = MaskCamera(7600.0, 50.0, 10850.0, 30.0f, -12.0f, w, h);
        float rows[3][4];
        CombatMaskRows(cam, cam.pos, rows);
        const int hw = CombatMaskSize(w), hh = CombatMaskSize(h);
        for (float px : {0.5f, 1.5f, w * 0.5f - 0.5f, w * 0.5f, w * 0.5f + 0.5f, w - 0.5f})
        {
            for (float py : {0.5f, h * 0.5f, h - 0.5f})
            {
                float p[3], uv[2];
                PointAtPixel(cam, cam.pos, px, py, 25.0f, p);
                REQUIRE(CombatMaskUv(rows, p, uv));
                INFO("pixel " << px << ", " << py);
                CHECK(uv[0] * hw == Approx(px * 0.5f).margin(2e-3));
                CHECK(uv[1] * hh == Approx(py * 0.5f).margin(2e-3));
            }
        }
        // The centre: on an even width the two middle pixel columns sit a
        // quarter texel either side of the boundary between two texels.
        if (w % 2 == 0)
        {
            float p[3], uv[2];
            PointAtPixel(cam, cam.pos, w * 0.5f - 0.5f, h * 0.5f + 0.5f, 25.0f, p);
            REQUIRE(CombatMaskUv(rows, p, uv));
            const float left = uv[0] * hw;
            PointAtPixel(cam, cam.pos, w * 0.5f + 0.5f, h * 0.5f + 0.5f, 25.0f, p);
            REQUIRE(CombatMaskUv(rows, p, uv));
            const float right = uv[0] * hw;
            CHECK(right - left == Approx(0.5f).margin(2e-3));
            CHECK(0.5f * (left + right) == Approx(w * 0.25f).margin(2e-3));
        }
        // The mask pass's receiver of a texel maps back to that texel's centre.
        for (int i : {0, hw / 2 - 1, hw / 2, hw - 1})
        {
            float vp[3];
            CombatMaskTexelView(cam, i, hh / 2, 30.0f, vp);
            float p[3];
            for (int c = 0; c < 3; ++c)
                p[c] = static_cast<float>(cam.rot[0][c] * vp[0] + cam.rot[1][c] * vp[1] + cam.rot[2][c] * vp[2]);
            float uv[2];
            REQUIRE(CombatMaskUv(rows, p, uv));
            CHECK(uv[0] * hw == Approx(i + 0.5f).margin(2e-3));
            CHECK(uv[1] * hh == Approx(hh / 2 + 0.5f).margin(2e-3));
        }
    }
}

TEST_CASE("Combat lights: the mask mapping follows a moving camera and a pillarbox", "[remaster][CombatLights]")
{
    // A point the previous camera saw at a pixel maps to that pixel's mask
    // coordinate from wherever the camera is now (the rows carry the move).
    CombatMaskCamera prev = MaskCamera(7600.0, 50.0, 10850.0, 30.0f, -12.0f, 1920, 1080);
    prev.rect[0] = 0.125f;
    prev.rect[2] = 0.75f;
    const double cur[3] = {7601.3, 50.2, 10849.1};
    float rows[3][4];
    CombatMaskRows(prev, cur, rows);
    const int hw = CombatMaskSize(1920), hh = CombatMaskSize(1080);
    for (float px : {250.5f, 960.0f, 1669.5f})
    {
        float p[3], uv[2];
        PointAtPixel(prev, cur, px, 400.5f, 18.0f, p);
        REQUIRE(CombatMaskUv(rows, p, uv));
        CHECK(uv[0] * hw == Approx(px * 0.5f).margin(5e-3));
        CHECK(uv[1] * hh == Approx(400.5f * 0.5f).margin(5e-3));
    }
    // Behind the previous camera there is nothing to read.
    float behind[3];
    PointAtPixel(prev, cur, 960.0f, 540.0f, -5.0f, behind);
    float uv[2];
    CHECK_FALSE(CombatMaskUv(rows, behind, uv));
}

TEST_CASE("Combat lights: they lead the frame list, at most twelve, as HDR entries", "[remaster][CombatLights]")
{
    std::vector<FrameLight> classic(20);
    for (size_t i = 0; i < classic.size(); ++i)
    {
        classic[i].diffuse[0] = 0.5f;
        classic[i].start = 5.0f;
        classic[i].sortBrightness = 1.0f;
        classic[i].pos[2] = 10.0f + static_cast<float>(i);
    }
    std::vector<FrameLight> combat(15);
    for (size_t i = 0; i < combat.size(); ++i)
    {
        combat[i].kind = FrameLightKind::Combat;
        combat[i].diffuse[0] = 3.0f;
        combat[i].start = 0.5f;
        combat[i].range = 40.0f;
        combat[i].shadowSlot = i < 4 ? static_cast<int>(i) : -1;
    }
    std::vector<FrameLight> merged = classic;
    const int n = MergeCombatFrameLights(merged, combat);
    CHECK(n == kMaxCombatFrameLights);
    REQUIRE(static_cast<int>(merged.size()) == kMaxCombatFrameLights + 20);
    for (int i = 0; i < n; ++i)
    {
        CHECK(merged[i].hdr);
        CHECK_FALSE(merged[i].IsSpot());
    }
    CHECK_FALSE(merged[n].hdr);
    // A combat light is binned by its window, not 10 start distances.
    CHECK(LightBounds(merged[0]).r == 40.0f);
    CHECK(LightBounds(merged[n]).r == Approx(kRangeFactor * 5.0f));
    // Packed: w 2 marks it; x its window, y its shadow channel + 1.
    std::vector<float> texels;
    PackFrameLights(merged, texels);
    CHECK(texels[7] == 2.0f);
    CHECK(texels[8] == 40.0f);
    CHECK(texels[9] == 1.0f);
    CHECK(texels[4 * kTexelsPerLight * 4 + 9] == 0.0f); // the fifth: no channel
    CHECK(texels[n * kTexelsPerLight * 4 + 7] == 0.0f); // the first classic light keeps its layout
    CHECK(texels[n * kTexelsPerLight * 4 + 11] == 1.0f);
    // Never past the buffer.
    std::vector<FrameLight> many(kMaxFrameLights, classic[0]);
    CHECK(MergeCombatFrameLights(many, combat) == kMaxCombatFrameLights);
    CHECK(static_cast<int>(many.size()) == kMaxFrameLights);
}

TEST_CASE("Combat lights: the gates need the light list, a light within reach, the mask a night and post effects",
          "[remaster][CombatLights][FeatureGates]")
{
    using namespace Poseidon::remaster;
    LightingTuning t;
    GateInputs in;
    in.sunElevationDeg = -20.0f;
    in.nightEffect = 1.0f;
    in.combatLightsNear = 1;
    FeatureGates g = ComputeFeatureGates(in, t);
    CHECK(g[Feature::CombatLights].on);
    CHECK(g[Feature::CombatShadows].on);
    CHECK(g[Feature::LightTiles].on);
    // Without the light list the classic lights of the same sources light.
    LightingTuning noTiles = t;
    noTiles.features.lightTiles = 0.0f;
    g = ComputeFeatureGates(in, noTiles);
    CHECK_FALSE(g[Feature::CombatLights].on);
    CHECK_FALSE(g[Feature::CombatShadows].on);
    // The switch off does not open the light list by day for a combat light.
    GateInputs day = in;
    day.sunElevationDeg = 40.0f;
    day.nightEffect = 0.0f;
    LightingTuning off = t;
    off.features.combatLights = 0.0f;
    CHECK_FALSE(ComputeFeatureGates(day, off)[Feature::LightTiles].on);
    CHECK(ComputeFeatureGates(day, t)[Feature::LightTiles].on);
    // The mask traces through the post pass's linear depth.
    LightingTuning noPost = t;
    noPost.postEffects = false;
    CHECK_FALSE(ComputeFeatureGates(in, noPost)[Feature::CombatShadows].on);
    CHECK(ComputeFeatureGates(in, noPost)[Feature::CombatLights].on);
    GateInputs interior = in;
    interior.interior = true;
    CHECK_FALSE(ComputeFeatureGates(interior, t)[Feature::CombatShadows].on);
}

TEST_CASE("Combat lights: the mask asks for the linear depth, not in an interior",
          "[remaster][CombatLights][PostStages]")
{
    using namespace Poseidon::remaster;
    PostFrameInputs in;
    in.combatMask = true;
    PostStagePlan plan = PlanPostStages(in);
    CHECK(plan.Runs(PostStage::Resolve));
    CHECK(plan.Runs(PostStage::LinearDepth));
    CHECK(plan.Runs(PostStage::Composite));
    in.worldDepth = false;
    plan = PlanPostStages(in);
    CHECK_FALSE(plan.Runs(PostStage::LinearDepth));
    // Without the mask the linear depth stays off, as before.
    PostFrameInputs base;
    base.ambientOcclusion = 0.7f;
    CHECK_FALSE(PlanPostStages(base).Runs(PostStage::LinearDepth));
}

TEST_CASE("Combat lights: the switches have landed with their sub-knobs and their GPU sections",
          "[remaster][CombatLights][FeatureSwitches]")
{
    using namespace Poseidon::remaster;
    const FeatureInfo& lights = GetFeatureInfo(Feature::CombatLights);
    const FeatureInfo& shadows = GetFeatureInfo(Feature::CombatShadows);
    CHECK(lights.landed);
    CHECK(shadows.landed);
    CHECK(std::string(lights.step) == "combat light");
    CHECK(std::string(lights.gpuSection) == "world");
    CHECK(std::string(shadows.gpuSection) == "combatmask");
    CHECK(std::string(Poseidon::gpu::SectionName(Poseidon::gpu::Section::CombatMask)) == "combatmask");
    for (const char* key : {"combatFlash", "combatFire", "combatSmoke", "combatScatter"})
    {
        INFO(key);
        const LightingKey* k = FindLightingKey(key);
        REQUIRE(k != nullptr);
        CHECK(std::string(k->parent) == "combatLights");
        // The burst flash is lower under the photoreal
        // camera (a shell at dusk lit the plain white to the horizon).
        CHECK(KeyDefault(*k) == (std::string(key) == "combatFlash" ? 0.6f : 1.0f));
    }
    // The last debug view, after the reflections' two.
    CHECK(kDebugViewCombatMask == kDebugViewSsrReflection + 1);
    const LightingKey* debug = FindLightingKey("debug");
    REQUIRE(debug != nullptr);
    CHECK(debug->maxValue >= static_cast<float>(kDebugViewCombatMask)); // the surfaces' views follow
    // The lit shaders draw it from its own number (rm::ShaderDebugView leaves
    // it to the backend, as it does the light tiles).
    CHECK(ShaderDebugView(kDebugViewCombatMask) == 0.0f);
}

TEST_CASE("Combat lights: the shaders read the combat entries, the mask and the scatter as the C++ lays them out",
          "[remaster][CombatLights][Shaders]")
{
    namespace gl = Poseidon::gl33lighting;
    const std::string lights = gl::RemasterFragmentPiece(gl::FragmentPiece::Lights);
    // The block: the lights, then the combat members (UploadCombatFrame writes
    // them after kMaxFrameLights * kTexelsPerLight vec4).
    const size_t arr = lights.find("vec4 frameLights[" + std::to_string(kMaxFrameLights * kTexelsPerLight) + "];");
    const size_t frame = lights.find("vec4 combatFrame;");
    REQUIRE(arr != std::string::npos);
    REQUIRE(frame != std::string::npos);
    CHECK(arr < frame);
    CHECK(frame < lights.find("vec4 combatPrevU;"));
    CHECK(lights.find("vec4 combatPrevU;") < lights.find("vec4 combatPrevV;"));
    CHECK(lights.find("vec4 combatPrevV;") < lights.find("vec4 combatPrevZ;"));
    CHECK(lights.find("uniform sampler2D combatMask;") != std::string::npos);
    // The classic budget and the physical lamps see only the lights after
    // the combat head.
    CHECK(lights.find("while (head < count && tileEntry(head) <= nCombat) ++head;") != std::string::npos);
    CHECK(lights.find("int classic = count - head;") != std::string::npos);
    CHECK(lights.find("if (k < head) continue;\n            int id = tileEntry(k);\n"
                      "            if (physicalKind(id)) physicalLight(id, n, kd, ka);") != std::string::npos);
    // The mapping is CombatMaskUv's.
    CHECK(lights.find("float z = dot(combatPrevZ.xyz, vWorldRel) + combatPrevZ.w;") != std::string::npos);
    CHECK(lights.find("vec2 uv = vec2(dot(combatPrevU.xyz, vWorldRel) + combatPrevU.w, dot(combatPrevV.xyz, "
                      "vWorldRel) + combatPrevV.w) / z;") != std::string::npos);
    // The model is EvaluateCombatLight's: E = I * window / (1 + d^2 / r0^2).
    CHECK(lights.find("float e = win / (1.0 + d2 / max(pos.w * pos.w, 1e-4));") != std::string::npos);
    // A surface takes the part of the ball above its horizon, as
    // CombatBallCosine computes it (leaves keep the crown's wrap).
    const std::string ball = FunctionBody(lights, "float combatBallCosine(float c, float core, float d) {");
    REQUIRE(!ball.empty());
    CHECK(ball.find("float s = d > core ? core / d : 1.0;") != std::string::npos);
    CHECK(ball.find("if (c * c > s2 || s2 <= 0.0) return max(c, 0.0);") != std::string::npos);
    CHECK(ball.find("if (s2 >= 1.0) return 0.5 * (1.0 + c);") != std::string::npos);
    CHECK(ball.find("float e = (c * acos(y) - x * sy) * s2 + atan(sy / x);") != std::string::npos);
    CHECK(ball.find("return max(e, 0.0) / (3.14159265 * s2);") != std::string::npos);
    CHECK(lights.find(": combatBallCosine(dot(n, l), pos.w, d2 * invD);") != std::string::npos);
    CHECK(lights.find("(dot(n, l) + sa) / (1.0 + sa)") == std::string::npos);
    // Shade: combat light in linear light, outside the classic clamp.
    const std::string shade = gl::RemasterFragmentPiece(gl::FragmentPiece::Shade);
    CHECK(shade.find("vec3 combat = kd * combatLights(n, nLight, foliage, v);") != std::string::npos);
    // A seam down the screen centre: a test camera stood on
    // the terrain segment line x = 7600, and one segment's material is specular,
    // the other not; a low flash's glint drew the line. No flash glint on terrain.
    CHECK(shade.find("vec3 combatGloss = terrain ? vec3(0.0) : litMatSpecular.rgb * g_combatSpec * litHorizon.w;") !=
          std::string::npos);
    CHECK(shade.find("local + emissive + combat)") != std::string::npos);
    CHECK(shade.find("litFlags.z > 10.5") != std::string::npos); // debug view 11
    CHECK(shade.find("litFlags.z > 5.5 ? lightTileView(display) : display") != std::string::npos);
    // The wet weather's lamp streaks read the same list: a combat entry
    // streaks by its own model at its own linear light, not as a classic spot
    // light (its w is 2) scaled by the night factor.
    const std::string surface = gl::RemasterFragmentPiece(gl::FragmentPiece::Surface);
    const size_t streakCombat = surface.find("if (color.w > 1.5) {");
    REQUIRE(streakCombat != std::string::npos);
    CHECK(streakCombat < surface.find("bool spot = color.w > 0.5;"));
    CHECK(surface.find("return sum * litLightCount.z + combat;") != std::string::npos);
    const std::string finish = gl::RemasterFragmentPiece(gl::FragmentPiece::Finish);
    CHECK(finish.find("nightBlend = max(nightBlend, sqrt(g_combatShare));") != std::string::npos);
    // The mask pass: the receiver at the centre of its texel's 2x2 block, the
    // occluders at the texel under each step.
    const std::string mask = gl::PostShaderSource(gl::PostShader::CombatMask);
    CHECK(mask.find("vec2 ndc = ((2.0 * vec2(t) + 1.0) / maskSize.xy - maskRect.xy) / maskRect.zw * 2.0 - 1.0;") !=
          std::string::npos);
    CHECK(mask.find("ivec2 t = ivec2(floor(px * 0.5));") != std::string::npos);
    CHECK(std::string(gl::PostShaderName(gl::PostShader::CombatMask)) == "psCombatMask");
    CHECK(std::string(gl::PostShaderName(gl::PostShader::CombatMaskBlur)) == "psCombatMaskBlur");
    // The composite scatters only when a light is there.
    const std::string composite = gl::PostShaderSource(gl::PostShader::Composite);
    CHECK(composite.find("if (count <= 0) return vec3(0.0);") != std::string::npos);
    CHECK(composite.find("glow += combatScatter();") != std::string::npos);
    // The mask's unit and its stand-in.
    bool unit = false;
    for (const auto& s : gl::kLitSamplers)
        unit = unit || (std::string(s.name) == "combatMask" && s.unit == gl::kCombatMaskUnit);
    CHECK(unit);
}

TEST_CASE("Combat lights: with the combat light on, the classic lights of its sources light nothing",
          "[remaster][CombatLights]")
{
    // At light selection, not at spawn: the classic light stays (its glow
    // sprite, and the frame with the switch off), and the tiled list and the
    // smoke skip it while the combat lights light the frame.
    const std::string lighting = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_Lighting.cpp");
    const std::string tiles = FunctionBody(lighting, "void EngineGL33::UpdateLightTiles()");
    REQUIRE_FALSE(tiles.empty());
    CHECK(tiles.find("if (_combat.active &&") != std::string::npos);
    CHECK(tiles.find("light->Kind() == fl::FrameLightKind::Combat || light->Kind() == fl::FrameLightKind::Fire") !=
          std::string::npos);
    CHECK(tiles.find("_combat.count = fl::MergeCombatFrameLights(t.lights, combat);") != std::string::npos);
    const std::string object = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Scene" / "Object.cpp");
    const std::string decal = FunctionBody(object, "void Object::DrawDecal(");
    REQUIRE_FALSE(decal.empty());
    CHECK(decal.find("const bool combat = GEngine->CombatLightsActive();") != std::string::npos);
    CHECK(decal.find("if (combat &&") != std::string::npos);
    CHECK(decal.find("(kind == lighting::FrameLightKind::Combat || kind == lighting::FrameLightKind::Fire)") !=
          std::string::npos);
    // A fireball's glowing puff takes no detonation light.
    CHECK(decal.find("colorI = CombatLitParticle(colorI, pos.Position(), albedo, flameEmit, fireball);") !=
          std::string::npos);
    // A puff's colour comes under one by its peak (no
    // clipped white column) and the light past it goes to the decal's glow.
    CHECK(decal.find("lighting::KeepHueUnderOne(rgb);") != std::string::npos);
    CHECK(decal.find("GEngine->SetDecalGlow(") != std::string::npos);
    // A fire's light crosses its smoke to reach a particle (the wreck's plume).
    const std::string litParticle = FunctionBody(object, "Color CombatLitParticle(");
    CHECK(litParticle.find("irr *= lighting::CombatSmokeTransmittance(r.kind, r.sample, d, albedo);") !=
          std::string::npos);
    CHECK(litParticle.find("lighting::CombatParticleMix(base, e, o,") != std::string::npos);
    // A detonation's light keeps its hue on smoke as a fire's.
    CHECK(litParticle.find("r.kind == lighting::CombatLightKind::Fire || r.detonation ? e : o") != std::string::npos);
    CHECK(litParticle.find("if (ownGlow && r.detonation)") != std::string::npos);
    CHECK(litParticle.find("lighting::CombatFlameEmission(r.kind, r.sample, d, flame);") != std::string::npos);
    // The frame's list keeps the steady lights past the twelve, and a fire's
    // lit air is a small halo, not a 4.4 m ball over its smoke.
    const std::string post = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_PostCombat.cpp");
    const std::string gather = FunctionBody(post, "void EngineGL33::GatherCombatLights(");
    REQUIRE_FALSE(gather.empty());
    CHECK(gather.find("fl::KeepCombatLights(") != std::string::npos);
    CHECK(gather.find("cands.resize(") == std::string::npos);
    const std::string medium = FunctionBody(post, "float ScatterMedium(");
    CHECK(medium.find("case fl::CombatLightKind::Fire:\n            return 2.0f * core + 0.8f;") != std::string::npos);
    // The combat lights only with the tiled list; the flash's exposure only
    // while they light.
    const std::string upload = FunctionBody(lighting, "void EngineGL33::UploadLitFrameConstants()");
    CHECK(upload.find("_combat.active = _lightTilesActive && _featureGates[rm::Feature::CombatLights].on;") !=
          std::string::npos);
    CHECK(upload.find("_combat.flashGain = 1.0f;") != std::string::npos);
    // The game ages them by the simulation step, never by the frame.
    const std::string world = ReadTextFile(EngineDir() / "Poseidon" / "World" / "World.cpp");
    const std::string sim = FunctionBody(world, "void World::Simulate(");
    const size_t doSim = sim.find("if (doSim)\n    {");
    const size_t tick = sim.find("lighting::GCombatLights().Tick(deltaT);");
    REQUIRE(doSim != std::string::npos);
    REQUIRE(tick != std::string::npos);
    CHECK(doSim < tick);
}

TEST_CASE("Combat lights: a munition's charge sets its flash, fireball and column by the cube root",
          "[remaster][CombatLights]")
{
    // CfgAmmo of the Remaster: Shell120 (hit 200, indirectHit 150, range 8),
    // GrenadeHand (20, 18, 7), Heat120 (700, 300, 1).
    const Detonation he = DetonationFromAmmo(200.0f, 150.0f, 8.0f);
    const Detonation grenade = DetonationFromAmmo(20.0f, 18.0f, 7.0f);
    const Detonation heat = DetonationFromAmmo(700.0f, 300.0f, 1.0f);
    CHECK(he.chargeKg == Approx(2.5f));
    CHECK(grenade.chargeKg == Approx(126.0f / 480.0f));
    CHECK(heat.chargeKg == Approx(300.0f / 480.0f));
    CHECK_FALSE(he.shaped);
    CHECK_FALSE(grenade.shaped);
    CHECK(heat.shaped);
    // Hopkinson-Cranz: radius and times go with W^(1/3).
    CHECK(he.scale == Approx(std::cbrt(2.5f)));
    CHECK(he.radius == Approx(kFireballRadiusPerS * std::cbrt(2.5f)));
    CHECK(he.flashTime == Approx(kFlashTimePerS * std::cbrt(2.5f)));
    CHECK(he.radius / grenade.radius == Approx(std::cbrt(2.5f / grenade.chargeKg)));
    // The shell's flash is tens of milliseconds, the grenade's shorter; the
    // HEAT round's fireball is smaller and briefer than the HE shell's,
    // although its direct hit is 3.5 times the shell's (Classic: bigger).
    CHECK(he.flashTime > 0.03f);
    CHECK(he.flashTime < 0.06f);
    CHECK(grenade.flashTime < he.flashTime);
    CHECK(heat.radius < he.radius);
    CHECK(heat.flashTime + heat.burnTime < 0.5f * (he.flashTime + he.burnTime));
    CHECK(he.columnLife > 12.0f);
    CHECK(grenade.columnLife < he.columnLife);
    CHECK(grenade.columnHeight < he.columnHeight);
    // A blast without its round: the classic size stands for the charge.
    CHECK(DetonationFromBlastSize(kReferenceBlastSize).chargeKg == Approx(kReferenceChargeKg));
    CHECK(DetonationFromBlastSize(0.25f).radius < DetonationFromBlastSize(1.0f).radius);
}

TEST_CASE("Combat lights: the gas cools from the white-hot flash through the afterburn to soot",
          "[remaster][CombatLights]")
{
    const Detonation he = DetonationFromAmmo(200.0f, 150.0f, 8.0f);
    CHECK(DetonationTemperature(he, 0.0f) == Approx(kDetonationPeakK));
    // One flash time on, most of the flash's excess is gone; through the
    // afterburn the gas holds near kAfterburnK; after it, soot.
    const float t1 = DetonationTemperature(he, he.flashTime);
    CHECK(t1 < 2500.0f);
    CHECK(t1 > kAfterburnK);
    CHECK(DetonationTemperature(he, he.burnTime) == Approx(kAfterburnK).epsilon(0.03));
    CHECK(DetonationTemperature(he, he.burnTime + 0.4f) < 1000.0f);
    // The light: the flash's power at the peak goes with the radiating area
    // (W^(2/3)); a frame later it has fallen several-fold (Wien), and once the
    // gas is soot the burst is over.
    const CombatTuning tuning;
    const CombatLightSample s0 = EvaluateDetonationLight(he, 0.0f, 1u, tuning);
    const CombatLightSample s1 = EvaluateDetonationLight(he, 1.0f / 30.0f, 1u, tuning);
    const CombatLightSample s2 = EvaluateDetonationLight(he, 2.0f / 30.0f, 1u, tuning);
    const float p0 = s0.intensity * s0.core * s0.core;
    const float p1 = s1.intensity * s1.core * s1.core;
    const float p2 = s2.intensity * s2.core * s2.core;
    CHECK(p0 == Approx(kDetonationFlashI1 * std::pow(2.5f, 2.0f / 3.0f)));
    CHECK(p1 < 0.4f * p0);
    CHECK(p2 < 0.4f * p1);
    CHECK_FALSE(s0.finished);
    CHECK(EvaluateDetonationLight(he, he.burnTime + 0.4f, 1u, tuning).finished);
    // The flash is white-hot, the afterburn orange.
    CHECK(s0.rgb[2] / s0.rgb[0] > 0.12f);
    const CombatLightSample sb = EvaluateDetonationLight(he, 0.7f * he.burnTime, 1u, tuning);
    CHECK(sb.rgb[2] / sb.rgb[0] < s0.rgb[2] / s0.rgb[0]);
    // A puff born hot glows until the burn ends, then cools to soot.
    const float cool = DetonationPuffCooling(he, kDetonationPeakK);
    CHECK(cool < 0.0f);
    CHECK(kDetonationPeakK + cool * (he.flashTime + he.burnTime) == Approx(1100.0f));
    // A HEAT round's flash is shorter than a frame: its puffs glow at least
    // kMinGlowSpan, as a 1/30 s shutter holds the flash.
    const Detonation heat = DetonationFromAmmo(700.0f, 300.0f, 1.0f);
    CHECK(heat.flashTime + heat.burnTime < kMinGlowSpan);
    CHECK(DetonationPuffCooling(heat, kDetonationPeakK) == Approx(-(kDetonationPeakK - 1100.0f) / kMinGlowSpan));
    // The fireball's glow follows the bursts' day scale (by night five-fold).
    CombatLightSet set;
    set.Evaluate(tuning, 0.0f);
    CHECK(set.BurstDayScale() == Approx(CombatDayScale(CombatLightKind::Explosion, 0.0f)));
    set.Evaluate(tuning, 1.0f);
    CHECK(set.BurstDayScale() == Approx(1.0f));
}

TEST_CASE("Combat lights: a vehicle's fuel burns as a slow, cooler, rolling fireball", "[remaster][CombatLights]")
{
    // A jeep: explosives = 50 l x 0.1 = 5, times 0.5 .. 0.75: hit about 3.
    const Detonation fuel = DeflagrationFromFuel(kFuelKgPerHit * 3.0f);
    const Detonation he = DetonationFromAmmo(200.0f, 150.0f, 8.0f);
    CHECK(fuel.deflagration);
    // CCPS fireball: D = 5.8 M^(1/3), t = 0.45 M^(1/3).
    CHECK(2.0f * fuel.radius == Approx(kFuelDiameterPerS * std::cbrt(11.1f)));
    CHECK(fuel.burnTime == Approx(kFuelDurationPerS * std::cbrt(11.1f)));
    CHECK(fuel.burnTime > 0.9f);
    CHECK(fuel.radius > 2.0f * he.radius);
    // Cooler than a detonation all the way, and burning for its whole time.
    for (float t = 0.0f; t < fuel.burnTime; t += 0.05f)
    {
        CHECK(DetonationTemperature(fuel, t) <= kFuelFlameK + 1.0f);
        CHECK(DetonationTemperature(fuel, t) > 1600.0f);
    }
    CHECK(DetonationTemperature(fuel, 0.5f * fuel.burnTime) > DetonationTemperature(he, 0.5f * fuel.burnTime));
    const CombatTuning tuning;
    CHECK_FALSE(EvaluateDetonationLight(fuel, 0.9f * fuel.burnTime, 7u, tuning).finished);
    CHECK(EvaluateDetonationLight(fuel, 4.0f * fuel.burnTime + 1.0f, 7u, tuning).finished);
    // The fuel in the ball is bounded (a heavy vehicle's tank).
    CHECK(DeflagrationFromFuel(1e5f).chargeKg == Approx(kMaxFuelKg));
}

TEST_CASE("Combat lights: blasts take their round's charge, craters their column, the set its light",
          "[remaster][CombatLights]")
{
    const std::string shots = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Entities" / "Weapons" / "Shots.cpp");
    size_t n = 0, at = 0;
    while ((at = shots.find("new Explosion(nullptr, _parent, size);", at)) != std::string::npos)
    {
        ++n;
        const size_t next = shots.find(';', at + 40);
        CHECK(shots.substr(at, next - at + 160).find("DetonationFromAmmo(") != std::string::npos);
        at = next;
    }
    CHECK(n == 7);
    const std::string coll = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Simulation" / "Collisions.cpp");
    CHECK(coll.find("crater->SetDetonation(") != std::string::npos);
    const std::string smokes =
        ReadTextFile(EngineDir() / "Poseidon" / "Graphics" / "Rendering" / "Effects" / "Smokes.cpp");
    CHECK(smokes.find("lighting::GCombatLights().SpawnDetonation(p, _det);") != std::string::npos);
    CHECK(smokes.find("lighting::GCombatLights().SpawnDetonation(p, _deflagration);") != std::string::npos);
    // A fireball's puffs are drawn in the frame they are born, hot.
    CHECK(smokes.find("_fire.SetShowAtBirth(true);") != std::string::npos);
    const std::string post = ReadTextFile(EngineDir() / "PoseidonGL33" / "EngineGL33_PostCombat.cpp");
    CHECK(post.find("std::min(kBurstSurfaceReachCores * f.start, kBurstSurfaceReachMax)") != std::string::npos);
    // The set evaluates a detonation's light by its physics.
    CombatLightSet set;
    const float p[3] = {0.0f, 0.0f, 0.0f};
    const Detonation he = DetonationFromAmmo(200.0f, 150.0f, 8.0f);
    const std::uint32_t id = set.SpawnDetonation(p, he);
    set.Evaluate(CombatTuning(), 1.0f);
    REQUIRE(set.Find(id) != nullptr);
    CHECK(set.Find(id)->detonation);
    CHECK(set.Find(id)->sample.core == Approx(kDetonationCoreShare * he.radius));
    set.Tick(0.2f);
    set.Tick(0.25f);
    CHECK(set.Find(id) == nullptr);
}
