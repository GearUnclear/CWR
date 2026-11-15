#include <Poseidon/Graphics/Rendering/Lighting/CombatLights.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::lighting
{

namespace
{

constexpr float kTwoPi = 6.28318530718f;

// The shipped look: the tuned knob values, baked (CombatTuning).
constexpr float kFlashGain = 6.0f;  // white-hot first flash of a burst, x the base envelope
constexpr float kFireGain = 2.5f;   // fire and ember power
constexpr float kSmokeGain = 0.35f; // combat light on smoke particles
// Material brightness the remastered shaders give a white surface: a particle
// lit by irradiance e shows what a white surface lit by e shows.
constexpr float kParticleBrightness = 1.6f;
// The fire's light deepens toward its own hue in the smoke it lights
// (CombatTuning::smokeTint): each channel's share of the peak to this power,
// reached as the light's luminance on the particle grows at this rate (1.6
// left the lit column peach, 3 turned it salmon red).
constexpr float kSmokeTintPower = 2.2f;
// The flame's body: a fire's flames are its own smoke
// glowing, incandescent soot in a volume round the light. The classic fire's
// light lit the puffs there past white, and that yellow column was the
// flame; the combat light on smoke only scatters, so under the camera the
// fire had no flame. A puff within kFlameBodyCores core radii of a fire's
// light emits the flame's colour (linear, kFlameRgb at its peak channel)
// up to kFlameEmission at the light, falling off squared, times the fire's
// strength and flicker.
constexpr float kFlameBodyCores = 3.0f;
constexpr float kFlameEmission = 8.0f;
constexpr float kFlameRgb[3] = {1.0f, 0.55f, 0.16f};
// The fire's own power at strength 1 (EvaluateCombatLight's Fire).
constexpr float kFirePower = 7.0f;
constexpr float kSmokeTintRate = 0.6f;
// The ceiling of the colour version's soft saturation (CombatParticleLight),
// linear light added at most: the scalar's 4 held the foot of a camp fire's
// column at the level of the smoke 3 m up, a flat glow with no flame in it.
constexpr float kParticleLightCap = 12.0f;
// The fire's colour, and the warmth a fireball cools to: (1.0, 0.66, 0.39)
// lit the ground round a camp fire at red over blue 1.43 to 1.53 on screen,
// over the 1.4 that keeps it from reading khaki; one step less saturated.
constexpr float kFireRgb[3] = {1.0f, 0.70f, 0.45f};

float Hash01(std::uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return static_cast<float>(x & 0xffffffU) / static_cast<float>(0x1000000);
}

// Smooth value noise in [-1, 1] over time.
float Noise1(float t, std::uint32_t seed)
{
    const float f = std::floor(t);
    const float u = t - f;
    const auto i = static_cast<std::int64_t>(f);
    const float a = Hash01(static_cast<std::uint32_t>(i) * 747796405U + seed) * 2.0f - 1.0f;
    const float b = Hash01(static_cast<std::uint32_t>(i + 1) * 747796405U + seed) * 2.0f - 1.0f;
    const float s = u * u * (3.0f - 2.0f * u);
    return a + (b - a) * s;
}

// Linear colour of unit luminance.
void SetColour(CombatLightSample& s, float r, float g, float b)
{
    const float lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
    const float inv = lum > 1e-6f ? 1.0f / lum : 0.0f;
    s.rgb[0] = r * inv;
    s.rgb[1] = g * inv;
    s.rgb[2] = b * inv;
}

// i1 = the intensity at 1 m for d >> core (i1 = intensity * core^2); the
// window ends where the irradiance falls to kCombatRangeFloor.
void SetPower(CombatLightSample& s, float i1, float core, float maxRange)
{
    s.core = core;
    s.intensity = i1 / (core * core);
    const float range = std::sqrt(std::max(i1, 0.0f) / kCombatRangeFloor);
    s.range = std::min(std::max(range, core * 4.0f), maxRange);
}

float Clamp(float v, float lo, float hi)
{
    return std::min(std::max(v, lo), hi);
}

} // namespace

const char* CombatLightKindName(CombatLightKind kind)
{
    switch (kind)
    {
        case CombatLightKind::Muzzle:
            return "muzzle";
        case CombatLightKind::Cannon:
            return "cannon";
        case CombatLightKind::Explosion:
            return "explosion";
        case CombatLightKind::Fire:
            return "fire";
        case CombatLightKind::Tracer:
            return "tracer";
        case CombatLightKind::Flare:
            return "flare";
        case CombatLightKind::Motor:
            return "motor";
    }
    return "unknown";
}

bool CombatLightIsBurst(CombatLightKind kind)
{
    return kind == CombatLightKind::Muzzle || kind == CombatLightKind::Cannon || kind == CombatLightKind::Explosion;
}

CombatLightSample EvaluateCombatLight(CombatLightKind kind, float strength, float age, std::uint32_t seed,
                                      const CombatTuning& tuning)
{
    CombatLightSample s;
    if (age < 0.0f)
    {
        s.finished = CombatLightIsBurst(kind);
        return s;
    }
    const float jitter = 0.8f + 0.4f * Hash01(seed); // shot to shot variation
    switch (kind)
    {
        case CombatLightKind::Muzzle:
        {
            // There the frame the round leaves, gone in a few frames. A small
            // pool: an earlier (1.6 at 1 m, (1.0, 0.50, 0.16), 45 m) lit
            // the whole foreground of two riflemen orange, and (1.0, 0.70,
            // 0.42) still turned the lit ground khaki (red over blue 1.50).
            // A step whiter than the fire, as the fire moved a step.
            const float st = Clamp(strength, 0.3f, 3.0f);
            SetColour(s, 1.0f, 0.80f, 0.62f);
            SetPower(s, 0.9f * st * jitter, 0.30f, 25.0f);
            s.intensity *= std::exp(-age / 0.030f);
            s.finished = age > 0.18f;
            break;
        }
        case CombatLightKind::Cannon:
        {
            const float st = Clamp(strength, 0.5f, 3.0f);
            SetColour(s, 1.0f, 0.78f, 0.61f);
            SetPower(s, 70.0f * st * jitter, 0.8f, kCombatMaxRange);
            s.intensity *= 0.8f * std::exp(-age / 0.045f) + 0.2f * std::exp(-age / 0.16f);
            s.finished = age > 0.6f;
            break;
        }
        case CombatLightKind::Explosion:
        {
            // A white-hot flash, then the fireball: warm, flickering, fading
            // over about a third of a second. Strength is the engine's
            // explosion size (0.25 grenade .. 1 heavy shell, 1.25 fuel). The
            // flash holds for kExplosionFlashHold, steady (a classic burst
            // still whitens the frame 0.13 s after it; a flash falling and
            // flickering from its birth left that frame 14 to 24 levels under
            // Classic), then falls in 50 ms while the flicker fades in. The
            // fireball cools to the fire's warmth (kFireRgb), not to the
            // saturated orange (1.0, 0.40, 0.11) tried first, whose light
            // turned the ground round a crater khaki (red over blue 1.55).
            const float e = Clamp(strength, 0.2f, 1.5f);
            const float cool = std::min(age / 0.45f, 1.0f);
            const float c = cool * cool * (3.0f - 2.0f * cool);
            SetColour(s, 1.0f, 0.90f + (kFireRgb[1] - 0.90f) * c, 0.74f + (kFireRgb[2] - 0.74f) * c);
            const float flash = std::max(kFlashGain * tuning.flash, 0.0f);
            const float peak = 0.65f * flash + 0.35f;
            const float afterHold = std::max(age - std::max(tuning.flashHold, 0.0f), 0.0f);
            const float flicker =
                1.0f + std::min(afterHold / 0.05f, 1.0f) *
                           (0.22f * Noise1(age * 14.0f, seed) + 0.1f * Noise1(age * 31.0f, seed ^ 0x51ed27U));
            const float flashEnv = std::exp(-afterHold / 0.05f);
            const float env = (0.65f * flash * flashEnv + 0.35f * std::exp(-age / 0.16f)) * flicker;
            // The window is sized for the peak.
            SetPower(s, 420.0f * e * e * jitter * peak, 0.8f + 1.8f * e, kCombatMaxRange);
            s.intensity *= std::max(env, 0.0f) / peak;
            s.finished = age > 0.9f;
            break;
        }
        case CombatLightKind::Fire:
        {
            // A slow breathing, faster licks and a crackle, never a new random
            // brightness every frame. Warm but far less saturated than an
            // earlier (1.0, 0.46, 0.14), whose light turned lit ground
            // khaki (red over blue 1.4 to 1.7 on the lit ground).
            const float st = std::max(strength, 0.0f);
            SetColour(s, kFireRgb[0], kFireRgb[1], kFireRgb[2]);
            const float ph = Hash01(seed) * kTwoPi;
            const float f = 0.80f + 0.10f * std::sin(kTwoPi * 1.3f * age + ph) +
                            0.07f * std::sin(kTwoPi * 3.7f * age + 2.1f * ph) + 0.10f * Noise1(age * 6.0f, seed) +
                            0.04f * Noise1(age * 11.0f, seed ^ 0x2545f491U);
            SetPower(s, kFirePower * kFireGain * std::max(tuning.fire, 0.0f) * st, 0.45f, kCombatMaxRange);
            s.intensity *= std::max(f, 0.2f);
            break;
        }
        case CombatLightKind::Flare:
        {
            // A magnesium flare under its parachute: lights up over a third of
            // a second, then burns with a sputtering flicker.
            const float st = std::max(strength, 0.0f);
            SetColour(s, 1.0f, 0.97f, 0.90f);
            const float on = std::min(age / 0.35f, 1.0f);
            const float f = 0.84f + 0.08f * Noise1(age * 7.0f, seed) + 0.05f * Noise1(age * 19.0f, seed ^ 0x6c8e9cf5U) +
                            0.03f * std::sin(kTwoPi * 2.3f * age);
            SetPower(s, 2500.0f * st, 0.6f, kCombatFlareRange);
            s.intensity *= on * on * std::max(f, 0.3f);
            break;
        }
        case CombatLightKind::Tracer:
        {
            const float st = Clamp(strength, 0.0f, 2.0f);
            SetColour(s, 1.0f, 0.62f, 0.30f);
            SetPower(s, 0.25f * st, 0.15f, 12.0f);
            break;
        }
        case CombatLightKind::Motor:
        {
            const float st = Clamp(strength, 0.0f, 1.0f);
            SetColour(s, 0.7f, 0.8f, 1.0f);
            SetPower(s, 3.0f * st, 0.3f, 30.0f);
            s.intensity *= 0.9f + 0.1f * Noise1(age * 17.0f, seed);
            break;
        }
    }
    return s;
}

float CombatDayScale(CombatLightKind kind, float nightEffect)
{
    const float night = Clamp(nightEffect, 0.0f, 1.0f);
    return CombatLightIsBurst(kind) ? 0.2f + 0.8f * night : 0.12f + 0.88f * night;
}

float CombatNight(float classicNight, bool photoCamera, float cameraNight)
{
    const float classic = Clamp(classicNight, 0.0f, 1.0f);
    return photoCamera ? std::min(classic, Clamp(cameraNight, 0.0f, 1.0f)) : classic;
}

float CombatBallCosine(float cosTheta, float core, float d)
{
    const float c = Clamp(cosTheta, -1.0f, 1.0f);
    // The ball's angular radius, sin sigma; at its surface and inside it 1.
    const float sinSigma = d > core ? std::max(core, 0.0f) / d : 1.0f;
    const float s2 = sinSigma * sinSigma;
    if (c * c > s2 || s2 <= 0.0f)
        return std::max(c, 0.0f);
    if (s2 >= 1.0f)
        return 0.5f * (1.0f + c);
    const float sinTheta = std::sqrt(std::max(1.0f - c * c, 1e-12f));
    const float x = std::sqrt(1.0f / s2 - 1.0f);
    const float y = Clamp(-x * c / sinTheta, -1.0f, 1.0f);
    const float sy = sinTheta * std::sqrt(std::max(1.0f - y * y, 0.0f));
    const float e = (c * std::acos(y) - x * sy) * s2 + std::atan(sy / x);
    return std::max(e, 0.0f) / (3.14159265f * s2);
}

bool CombatLightOpensExposure(CombatLightKind kind)
{
    return kind == CombatLightKind::Explosion || kind == CombatLightKind::Cannon;
}

float CombatMuzzleStrength(float ammoHit, bool machineGun, float classicIntensity)
{
    if (machineGun)
        return ammoHit > 0.0f ? ammoHit / 10.0f : (classicIntensity > 0.1f ? 1.8f : 1.0f);
    return ammoHit > 0.0f ? std::sqrt(ammoHit / 150.0f) : 1.5f;
}

float CombatIrradiance(const CombatLightSample& s, float d2)
{
    const float r2 = s.range * s.range;
    if (d2 >= r2 || s.intensity <= 0.0f)
        return 0.0f;
    const float x = d2 / r2;
    float win = 1.0f - x * x;
    win *= win;
    return s.intensity * win / (1.0f + d2 / std::max(s.core * s.core, 1e-4f));
}

float CombatParticleChannel(float display, float irradiance, float smokeGain)
{
    const float x = std::max(irradiance, 0.0f) * kParticleBrightness * kSmokeGain * std::max(smokeGain, 0.0f);
    if (x <= 0.0f)
        return display;
    const float lin = std::pow(std::max(display, 0.0f), 2.2f) + x / (1.0f + x * 0.25f);
    return std::pow(lin, 1.0f / 2.2f);
}

void CombatParticleLight(const float display[3], const float irradiance[3], float smokeGain, float tint, float out[3])
{
    const float k = kParticleBrightness * kSmokeGain * std::max(smokeGain, 0.0f);
    float x[3];
    for (int i = 0; i < 3; ++i)
        x[i] = std::max(irradiance[i], 0.0f) * k;
    const float peak = std::max(x[0], std::max(x[1], x[2]));
    if (peak <= 0.0f)
    {
        for (int i = 0; i < 3; ++i)
            out[i] = display[i];
        return;
    }
    // A soft saturation on the peak channel (to kParticleLightCap), the same
    // share off every channel: the light keeps its hue however strong.
    const float keep = 1.0f / (1.0f + peak / kParticleLightCap);
    const float lum = (0.2126f * x[0] + 0.7152f * x[1] + 0.0722f * x[2]) * keep;
    const float w = Clamp(tint, 0.0f, 1.0f) * (1.0f - std::exp(-lum * kSmokeTintRate));
    for (int i = 0; i < 3; ++i)
    {
        const float hue = std::pow(x[i] / peak, kSmokeTintPower);
        const float add = x[i] * keep * (1.0f + (hue - 1.0f) * w);
        out[i] = std::pow(std::pow(std::max(display[i], 0.0f), 2.2f) + add, 1.0f / 2.2f);
    }
}

void CombatParticleMix(const float display[3], const float fire[3], const float other[3], float smokeGain, float tint,
                       float out[3])
{
    CombatParticleLight(display, fire, smokeGain, tint, out);
    for (int i = 0; i < 3; ++i)
        out[i] = CombatParticleChannel(out[i], other[i], smokeGain);
}

void CombatFlameEmission(CombatLightKind kind, const CombatLightSample& s, float d, float out[3])
{
    out[0] = out[1] = out[2] = 0.0f;
    if (kind != CombatLightKind::Fire || s.intensity <= 0.0f)
        return;
    const float body = kFlameBodyCores * std::max(s.core, 0.05f);
    if (d >= body)
        return;
    const float t = 1.0f - d / body;
    // The fire's strength times its flicker (and the fire knob): the light's
    // power over a strength 1 fire's.
    const float level = s.intensity * s.core * s.core / (kFirePower * kFireGain);
    const float e = kFlameEmission * level * t * t;
    for (int i = 0; i < 3; ++i)
        out[i] = kFlameRgb[i] * e;
}

void BlackBodyRgb(float temperatureK, float out[3])
{
    const float t = Clamp(temperatureK, 1000.0f, 3000.0f);
    out[0] = 1.0f;
    out[1] = std::max(0.155f + (t - 1500.0f) * 0.000207f, 0.05f);
    out[2] = 0.15f * std::pow(Clamp((t - 1900.0f) / 1100.0f, 0.0f, 1.0f), 1.5f);
}

bool FireballPuff(float temperatureK, float hottestK, float albedo[3], float emit[3])
{
    if (!(hottestK >= kFireballTableK && hottestK <= kFireballTableMaxK))
        return false;
    const float t = std::max(temperatureK, 0.0f);
    const float fade = Clamp((t - kFireballGlowFromK) / (kFireballGlowFullK - kFireballGlowFromK), 0.0f, 1.0f);
    const float glow = kFireballEmission *
                       std::exp(kFireballWienK * (1.0f / kFireballRefK - 1.0f / std::max(t, 1.0f))) * fade * fade *
                       (3.0f - 2.0f * fade);
    float body[3];
    BlackBodyRgb(t, body);
    for (int i = 0; i < 3; ++i)
    {
        albedo[i] = kFireballSoot[i];
        // A soft per-channel saturation: the hottest core goes white-yellow.
        emit[i] = kFireballEmitCap * (1.0f - std::exp(-body[i] * glow / kFireballEmitCap));
    }
    return true;
}

namespace
{
// The visible power of a black body at `k` against one at `refK` (Wien at
// 580 nm, kFireballWienK = c2 / 580 nm).
float WienRatio(float k, float refK)
{
    return std::exp(kFireballWienK * (1.0f / std::max(refK, 1.0f) - 1.0f / std::max(k, 1.0f)));
}
} // namespace

Detonation DetonationFromAmmo(float hit, float indirectHit, float indirectHitRange)
{
    Detonation d;
    const float energy = std::max(indirectHit, 0.0f) * std::max(indirectHitRange, 0.0f);
    d.chargeKg = Clamp(energy / kChargeEnergyPerKg, kMinChargeKg, kMaxChargeKg);
    d.scale = std::cbrt(d.chargeKg);
    d.shaped = hit > energy;
    d.radius = kFireballRadiusPerS * d.scale * (d.shaped ? kShapedRadiusShare : 1.0f);
    d.flashTime = kFlashTimePerS * d.scale * (d.shaped ? kShapedFlashShare : 1.0f);
    d.burnTime = kAfterburnTimePerS * d.scale * (d.shaped ? kShapedAfterburnShare : 1.0f);
    d.peakK = kDetonationPeakK;
    d.burnK = kAfterburnK;
    d.columnHeight = kColumnHeightPerS * d.scale * (d.shaped ? kShapedColumnShare : 1.0f);
    d.columnLife = kColumnLifePerS * d.scale;
    return d;
}

Detonation DetonationFromBlastSize(float blastSize)
{
    // The reference shell's charge at its classic size, the cube root linear
    // in the size: W = 2.5 kg x (size / 0.6)^3.
    const float r = std::max(blastSize, 0.05f) / kReferenceBlastSize;
    const float kg = kReferenceChargeKg * r * r * r;
    // indirectHit x indirectHitRange of that charge, no shaped charge.
    return DetonationFromAmmo(0.0f, kg * kChargeEnergyPerKg, 1.0f);
}

Detonation DeflagrationFromFuel(float fuelKg)
{
    Detonation d;
    d.deflagration = true;
    d.chargeKg = Clamp(fuelKg, kMinFuelKg, kMaxFuelKg);
    d.scale = std::cbrt(d.chargeKg);
    d.radius = 0.5f * kFuelDiameterPerS * d.scale;
    d.burnTime = kFuelDurationPerS * d.scale;
    d.flashTime = kFuelRiseShare * d.burnTime;
    d.peakK = kFuelFlameK;
    d.burnK = kFuelFlameK;
    d.columnHeight = 3.0f * d.radius;
    d.columnLife = kFuelColumnLife;
    return d;
}

float DetonationTemperature(const Detonation& d, float age)
{
    const float t = std::max(age, 0.0f);
    if (d.deflagration)
    {
        // Ignition, a rolling rise to the flame, the burn, then the soot.
        const float up = std::min(t / std::max(d.flashTime, 1e-3f), 1.0f);
        const float fall = std::max(0.3f * d.burnTime, kGlowFallTime);
        const float hold = t <= d.burnTime ? 1.0f : std::exp(-(t - d.burnTime) / fall);
        return kSootK + (d.burnK - kSootK) * (0.75f + 0.25f * up) * hold;
    }
    const float hold = t <= d.burnTime ? 1.0f : std::exp(-(t - d.burnTime) / kGlowFallTime);
    return kSootK + (d.burnK - kSootK) * hold + (d.peakK - d.burnK) * std::exp(-t / std::max(d.flashTime, 1e-3f));
}

float DetonationPuffCooling(const Detonation& d, float bornK)
{
    const float glowEndK = 1100.0f;
    const float span = d.deflagration ? 0.6f * d.burnTime : d.flashTime + d.burnTime;
    return -std::max(bornK - glowEndK, 100.0f) / std::max(span, kMinGlowSpan);
}

CombatLightSample EvaluateDetonationLight(const Detonation& d, float age, std::uint32_t seed,
                                          const CombatTuning& tuning)
{
    CombatLightSample s;
    if (age < 0.0f)
    {
        s.finished = true;
        return s;
    }
    const float k = DetonationTemperature(d, age);
    float body[3];
    BlackBodyRgb(k, body);
    SetColour(s, body[0], body[1], body[2]);
    // The radiating area goes with the charge to the 2/3 (a ball of the
    // fireball's radius), its visible power with Wien against the peak.
    const float area = std::pow(d.chargeKg, 2.0f / 3.0f);
    float i1 = d.deflagration ? kDeflagrationI1 * area * WienRatio(k, kFuelFlameK)
                              : kDetonationFlashI1 * area * WienRatio(k, d.peakK) *
                                    std::max(kFlashGain * tuning.flash, 0.0f) / kFlashGain;
    // A burning fuel ball flickers as it rolls; a detonation's flash does not.
    if (d.deflagration)
        i1 *= 1.0f + 0.15f * Noise1(age * 9.0f, seed) + 0.08f * Noise1(age * 23.0f, seed ^ 0x51ed27U);
    const float core = std::max(kDetonationCoreShare * d.radius, 0.2f);
    SetPower(s, std::max(i1, 0.0f), core, kCombatMaxRange);
    // Over once the gas is soot (its light a thousandth of the flash).
    s.finished = k < 1150.0f && age > d.burnTime;
    return s;
}

float CombatTwilight(float sunElevationDeg)
{
    const float above = std::max(sunElevationDeg - kTwilightNightDeg, 0.0f);
    return std::max(std::pow(10.0f, -above / kTwilightDecadeDeg), kTwilightFloor);
}

void KeepHueUnderOne(float rgb[3])
{
    const float peak = std::max(rgb[0], std::max(rgb[1], rgb[2]));
    if (peak > 1.0f)
        for (int i = 0; i < 3; ++i)
            rgb[i] /= peak;
}

float CombatSmokeReach(float core, float albedo)
{
    const float a = Clamp(albedo, 0.0f, 1.0f);
    return std::max(core, 0.05f) * (kSootReachCores + kPaleReachCores * a * a);
}

float CombatSmokeTransmittance(CombatLightKind kind, const CombatLightSample& s, float d, float albedo)
{
    if (kind != CombatLightKind::Fire && kind != CombatLightKind::Explosion)
        return 1.0f;
    const float inside = std::max(d - s.core, 0.0f);
    return std::exp(-inside / CombatSmokeReach(s.core, albedo));
}

bool CombatLightIsSteady(CombatLightKind kind)
{
    return kind == CombatLightKind::Fire || kind == CombatLightKind::Flare || kind == CombatLightKind::Motor;
}

std::vector<int> KeepCombatLights(const CombatLightKind* kinds, int n, int cap)
{
    std::vector<int> keep;
    n = std::max(n, 0);
    cap = std::max(cap, 0);
    int steady = 0;
    for (int i = 0; i < n; ++i)
        steady += CombatLightIsSteady(kinds[i]) ? 1 : 0;
    const int steadySlots = std::min(steady, cap);
    const int otherSlots = cap - steadySlots;
    int steadyTaken = 0, otherTaken = 0;
    for (int i = 0; i < n; ++i)
    {
        if (CombatLightIsSteady(kinds[i]))
        {
            if (steadyTaken < steadySlots)
            {
                keep.push_back(i);
                ++steadyTaken;
            }
        }
        else if (otherTaken < otherSlots)
        {
            keep.push_back(i);
            ++otherTaken;
        }
    }
    return keep;
}

float FlashHeldIrradiance(float held, float flashNow, float dt)
{
    const float decayed = held * std::exp(-std::max(dt, 0.0f) / kFlashExposureHold);
    return std::max(std::max(flashNow, 0.0f), decayed);
}

float FlashExposureGain(float flashIrradianceAtCamera, float nightEffect)
{
    const float x =
        kFlashExposurePerIrradiance * std::max(flashIrradianceAtCamera, 0.0f) * Clamp(nightEffect, 0.0f, 1.0f);
    // 1 + x for a faint flash, kFlashExposureMax at most.
    return 1.0f + x / (1.0f + x / (kFlashExposureMax - 1.0f));
}

float ShadowScore(const CombatLightSample& s, float d2)
{
    return s.intensity * s.core * s.core / std::max(d2, 100.0f);
}

void UpdateShadowSelection(ShadowSelection& sel, const ShadowCandidate* candidates, int n, float dt, int slots,
                           int* channel)
{
    slots = std::min(std::max(slots, 1), kMaxShadowedCombatLights);
    auto scoreOf = [&](std::uint32_t id) -> float
    {
        for (int i = 0; i < n; ++i)
            if (candidates[i].id == id)
                return candidates[i].score;
        return -1.0f;
    };
    // A light that went out or out of view frees its channel, and so does a
    // channel past `slots`.
    for (int c = 0; c < kMaxShadowedCombatLights; ++c)
    {
        if (sel.ids[c] != 0 && (c >= slots || scoreOf(sel.ids[c]) <= 0.0f))
            sel.ids[c] = 0;
    }
    auto shadowed = [&](std::uint32_t id)
    {
        for (int c = 0; c < slots; ++c)
            if (sel.ids[c] == id)
                return true;
        return false;
    };
    // The strongest candidate without a channel (score above 0).
    auto strongestFree = [&]() -> int
    {
        int best = -1;
        for (int i = 0; i < n; ++i)
        {
            if (candidates[i].id == 0 || candidates[i].score <= 0.0f || shadowed(candidates[i].id))
                continue;
            if (best < 0 || candidates[i].score > candidates[best].score)
                best = i;
        }
        return best;
    };
    // Empty channels take the strongest at once, lowest channel first.
    for (int c = 0; c < slots; ++c)
    {
        if (sel.ids[c] != 0)
            continue;
        const int b = strongestFree();
        if (b < 0)
            break;
        sel.ids[c] = candidates[b].id;
    }
    // All channels full: the strongest of the rest takes the weakest channel
    // once it has exceeded it by the ratio for the hold time.
    const int b = strongestFree();
    int weakest = -1;
    for (int c = 0; c < slots; ++c)
    {
        if (sel.ids[c] == 0)
        {
            weakest = -1;
            break;
        }
        if (weakest < 0 || scoreOf(sel.ids[c]) < scoreOf(sel.ids[weakest]))
            weakest = c;
    }
    if (b >= 0 && weakest >= 0 && candidates[b].score > kShadowSwapRatio * scoreOf(sel.ids[weakest]))
    {
        if (sel.challenger == candidates[b].id)
            sel.challengeTime += std::max(dt, 0.0f);
        else
        {
            sel.challenger = candidates[b].id;
            sel.challengeTime = 0.0f;
        }
        if (sel.challengeTime >= kShadowSwapTime)
        {
            sel.ids[weakest] = candidates[b].id;
            sel.challenger = 0;
            sel.challengeTime = 0.0f;
        }
    }
    else
    {
        sel.challenger = 0;
        sel.challengeTime = 0.0f;
    }
    if (channel)
    {
        for (int i = 0; i < n; ++i)
        {
            channel[i] = -1;
            for (int c = 0; c < slots; ++c)
                if (candidates[i].id != 0 && sel.ids[c] == candidates[i].id)
                    channel[i] = c;
        }
    }
}

std::uint32_t CombatLightSet::Add(CombatLightKind kind, const float pos[3], float strength, bool owned)
{
    CombatLightRecord r;
    r.id = _nextId++;
    if (_nextId == 0)
        _nextId = 1;
    r.kind = kind;
    r.strength = strength;
    for (int i = 0; i < 3; ++i)
        r.pos[i] = pos[i];
    r.seed = _seed;
    _seed = _seed * 1664525U + 1013904223U;
    r.owned = owned;
    _records.push_back(r);
    return r.id;
}

std::uint32_t CombatLightSet::Spawn(CombatLightKind kind, const float pos[3], float strength)
{
    return Add(kind, pos, strength, false);
}

std::uint32_t CombatLightSet::SpawnDetonation(const float pos[3], const Detonation& d)
{
    const std::uint32_t id = Add(CombatLightKind::Explosion, pos, d.scale, false);
    CombatLightRecord& r = _records.back();
    r.detonation = true;
    r.det = d;
    return id;
}

std::uint32_t CombatLightSet::Create(CombatLightKind kind, const float pos[3], float strength)
{
    return Add(kind, pos, strength, true);
}

CombatLightRecord* CombatLightSet::FindMutable(std::uint32_t id)
{
    for (CombatLightRecord& r : _records)
        if (r.id == id)
            return &r;
    return nullptr;
}

const CombatLightRecord* CombatLightSet::Find(std::uint32_t id) const
{
    for (const CombatLightRecord& r : _records)
        if (r.id == id)
            return &r;
    return nullptr;
}

bool CombatLightSet::Update(std::uint32_t id, const float pos[3], float strength)
{
    CombatLightRecord* r = FindMutable(id);
    if (!r)
        return false;
    for (int i = 0; i < 3; ++i)
        r->pos[i] = pos[i];
    r->strength = strength;
    return true;
}

void CombatLightSet::SetTint(std::uint32_t id, float r, float g, float b)
{
    if (CombatLightRecord* rec = FindMutable(id))
    {
        rec->tint[0] = r;
        rec->tint[1] = g;
        rec->tint[2] = b;
    }
}

void CombatLightSet::Remove(std::uint32_t id)
{
    _records.erase(
        std::remove_if(_records.begin(), _records.end(), [id](const CombatLightRecord& r) { return r.id == id; }),
        _records.end());
}

namespace
{
// A record's envelope: a blast with its detonation physics, or its kind's.
CombatLightSample SampleRecord(const CombatLightRecord& r, const CombatTuning& tuning)
{
    if (r.detonation)
        return EvaluateDetonationLight(r.det, r.age, r.seed, tuning);
    return EvaluateCombatLight(r.kind, r.strength, r.age, r.seed, tuning);
}
} // namespace

void CombatLightSet::Tick(float dt)
{
    if (!(dt > 0.0f))
        return;
    // A longer step (time acceleration at a low frame rate, a clock jump) ages
    // the set by the longest step, so bursts still end.
    const float step = std::min(dt, kCombatMaxTickStep);
    _simTime += step;
    for (CombatLightRecord& r : _records)
        r.age += step;
    // Bursts whose envelope is over leave (the tuning does not change when).
    const CombatTuning neutral;
    _records.erase(std::remove_if(_records.begin(), _records.end(), [&neutral](const CombatLightRecord& r)
                                  { return !r.owned && SampleRecord(r, neutral).finished; }),
                   _records.end());
}

void CombatLightSet::Evaluate(const CombatTuning& tuning, float nightEffect)
{
    _tuning = tuning;
    _burstDayScale = CombatDayScale(CombatLightKind::Explosion, nightEffect);
    for (CombatLightRecord& r : _records)
    {
        CombatLightSample s = SampleRecord(r, tuning);
        if (r.kind == CombatLightKind::Flare || r.kind == CombatLightKind::Motor)
        {
            const float lum = 0.2126f * r.tint[0] + 0.7152f * r.tint[1] + 0.0722f * r.tint[2];
            if (lum > 1e-4f)
                SetColour(s, r.tint[0], r.tint[1], r.tint[2]);
        }
        s.intensity *= CombatDayScale(r.kind, nightEffect);
        r.sample = s;
    }
}

void CombatLightSet::ClearBursts()
{
    _records.erase(
        std::remove_if(_records.begin(), _records.end(), [](const CombatLightRecord& r) { return !r.owned; }),
        _records.end());
}

void CombatLightSet::Clear()
{
    _records.clear();
}

CombatLightSet& GCombatLights()
{
    static CombatLightSet set;
    return set;
}

void CombatLightHandle::Set(CombatLightKind kind, const float pos[3], float strength)
{
    CombatLightSet& set = GCombatLights();
    if (_id != 0 && set.Update(_id, pos, strength))
        return;
    _id = set.Create(kind, pos, strength);
}

void CombatLightHandle::SetTint(float r, float g, float b)
{
    if (_id != 0)
        GCombatLights().SetTint(_id, r, g, b);
}

void CombatLightHandle::Release()
{
    if (_id != 0)
        GCombatLights().Remove(_id);
    _id = 0;
}

int CombatMaskSize(int fullSize)
{
    return (fullSize + 1) / 2;
}

void CombatMaskRows(const CombatMaskCamera& prev, const double curPos[3], float rows[3][4])
{
    // The current camera's position in the previous view: the translation of
    // every camera-relative point.
    const double d[3] = {curPos[0] - prev.pos[0], curPos[1] - prev.pos[1], curPos[2] - prev.pos[2]};
    double t[3];
    for (int r = 0; r < 3; ++r)
        t[r] = prev.rot[r][0] * d[0] + prev.rot[r][1] * d[1] + prev.rot[r][2] * d[2];
    // Mask u = frame-target pixel / (2 * half width) = sx * (rect.x + (ndc.x / 2 + 1 / 2) * rect.w)
    // with ndc.x = p11 * x / z, so u = (au * z + bu * x) / z; v likewise.
    const int hw = CombatMaskSize(prev.targetW), hh = CombatMaskSize(prev.targetH);
    const double sx = hw > 0 ? static_cast<double>(prev.targetW) / (2.0 * hw) : 1.0;
    const double sy = hh > 0 ? static_cast<double>(prev.targetH) / (2.0 * hh) : 1.0;
    const double au = sx * (prev.rect[0] + 0.5 * prev.rect[2]), bu = sx * 0.5 * prev.rect[2] * prev.p11;
    const double av = sy * (prev.rect[1] + 0.5 * prev.rect[3]), bv = sy * 0.5 * prev.rect[3] * prev.p22;
    for (int c = 0; c < 3; ++c)
    {
        rows[0][c] = static_cast<float>(au * prev.rot[2][c] + bu * prev.rot[0][c]);
        rows[1][c] = static_cast<float>(av * prev.rot[2][c] + bv * prev.rot[1][c]);
        rows[2][c] = prev.rot[2][c];
    }
    rows[0][3] = static_cast<float>(au * t[2] + bu * t[0]);
    rows[1][3] = static_cast<float>(av * t[2] + bv * t[1]);
    rows[2][3] = static_cast<float>(t[2]);
}

bool CombatMaskUv(const float rows[3][4], const float p[3], float uv[2])
{
    float v[3];
    for (int r = 0; r < 3; ++r)
        v[r] = rows[r][0] * p[0] + rows[r][1] * p[1] + rows[r][2] * p[2] + rows[r][3];
    if (v[2] <= 1e-4f)
        return false;
    uv[0] = v[0] / v[2];
    uv[1] = v[1] / v[2];
    return true;
}

void CombatMaskTexelView(const CombatMaskCamera& prev, int i, int j, float z, float out[3])
{
    // The block's centre is frame-target pixel coordinate (2i + 1, 2j + 1).
    const float u = (2.0f * static_cast<float>(i) + 1.0f) / static_cast<float>(std::max(prev.targetW, 1));
    const float v = (2.0f * static_cast<float>(j) + 1.0f) / static_cast<float>(std::max(prev.targetH, 1));
    const float ndcX = (u - prev.rect[0]) / prev.rect[2] * 2.0f - 1.0f;
    const float ndcY = (v - prev.rect[1]) / prev.rect[3] * 2.0f - 1.0f;
    out[0] = ndcX * z / prev.p11;
    out[1] = ndcY * z / prev.p22;
    out[2] = z;
}

} // namespace Poseidon::lighting
