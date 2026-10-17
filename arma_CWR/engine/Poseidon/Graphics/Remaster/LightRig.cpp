#include <Poseidon/Graphics/Remaster/LightRig.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::remaster
{

namespace
{
constexpr float kPi = 3.14159265358979323846f;

float Clamp01(float v)
{
    return std::min(1.0f, std::max(0.0f, v));
}

float Smoothstep(float e0, float e1, float x)
{
    const float t = Clamp01((x - e0) / (e1 - e0));
    return t * t * (3.0f - 2.0f * t);
}

Rgb NormaliseLuminance(const Rgb& c)
{
    const float l = Luminance(c);
    return l > 1e-6f ? c * (1.0f / l) : Rgb(1.0f, 1.0f, 1.0f);
}

// Relative air mass (Kasten and Young 1989) for an elevation in degrees.
float AirMass(float elevationDeg)
{
    const float e = std::max(elevationDeg, 0.0f);
    return 1.0f / (std::sin(e * kPi / 180.0f) + 0.50572f * std::pow(e + 6.07995f, -1.6364f));
}

// Calibration, in linear units before the material terms. With the default
// brightness the materials carry 1.6, so a surface lit by irradiance E shows
// its texture times 1.6 E. At noon sunlit ground gets about 0.67 (texture
// brightness, as classic) and cast shadow 0.15 (about 0.58 of lit on screen
// after the display gamma): bold shadows that still keep their detail. The
// exposure meter holds the lit level as the sun drops.
constexpr float kSunNoon = 0.55f;
constexpr float kSkyNoon = 0.15f;
constexpr float kSkyNight = 0.0009f; // display parity with the classic night ambient
constexpr float kMoonFull = 0.0035f;
constexpr float kGroundAlbedo = 0.2f;
constexpr float kExtinction = 0.05f; // optical depth per air mass beyond the zenith
constexpr float kCloudTransmit = 0.2f;
constexpr float kBrightness = 1.6f; // default user brightness folded into the materials
constexpr float kMaxMeterGain = 1.6f;

// Clear-sky fill as a fraction of noon: falls more slowly than the sun on the
// ground, so morning shade stays readable, but fast enough that a low sun
// still out-lights the sky on what it faces (golden hour stays a contrasty,
// darker frame, not a dim noon). Fades through twilight below the horizon.
float SkyDayFraction(float sinSun)
{
    const float day = std::pow(std::min(std::max(sinSun / 0.866f, 0.0f), 1.15f), 0.6f);
    return std::max(day, 0.12f * Smoothstep(-0.10f, 0.03f, sinSun));
}
} // namespace

float Luminance(const Rgb& c)
{
    return 0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b;
}

Rgb Mix(const Rgb& a, const Rgb& b, float t)
{
    return {a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t};
}

float ToLinear(float display)
{
    return std::pow(std::max(display, 0.0f), 2.2f);
}

Rgb ToLinear(const Rgb& display)
{
    return {ToLinear(display.r), ToLinear(display.g), ToLinear(display.b)};
}

float ToDisplay(float linear)
{
    return std::pow(std::max(linear, 0.0f), 1.0f / 2.2f);
}

Rgb TonemapNeutral(const Rgb& in)
{
    // The Khronos curve without its toe: the toe subtracts up to 0.04 to deepen
    // blacks, which would push the classic night levels (~0.0015 linear) to
    // zero and black out every night mission.
    constexpr float startCompression = 0.8f;
    constexpr float desaturation = 0.15f;
    Rgb c = in;
    const float peak = std::max(c.r, std::max(c.g, c.b));
    if (peak < startCompression)
        return c;
    constexpr float d = 1.0f - startCompression;
    const float newPeak = 1.0f - d * d / (peak + d - startCompression);
    c = c * (newPeak / peak);
    const float g = 1.0f - 1.0f / (desaturation * (peak - newPeak) + 1.0f);
    return Mix(c, Rgb(newPeak, newPeak, newPeak), g);
}

Rgb SunChroma(float elevationDeg, float warmth)
{
    // Keyframes eyeballed against clear-sky photographs: near-neutral white
    // from mid morning (a yellow midday sun turns OFP's olive textures to
    // mustard), gold in the last hour, deep orange on the horizon.
    struct Key
    {
        float deg;
        Rgb c;
    };
    static const Key kKeys[] = {
        {0.0f, {1.00f, 0.40f, 0.17f}},  {3.0f, {1.00f, 0.53f, 0.28f}},  {8.0f, {1.00f, 0.71f, 0.46f}},
        {15.0f, {1.00f, 0.86f, 0.70f}}, {30.0f, {1.00f, 0.96f, 0.90f}}, {60.0f, {1.00f, 0.985f, 0.96f}},
    };
    const float e = std::max(elevationDeg, 0.0f);
    Rgb c = kKeys[5].c;
    for (int i = 0; i < 5; ++i)
    {
        if (e <= kKeys[i + 1].deg)
        {
            const float t = (e - kKeys[i].deg) / (kKeys[i + 1].deg - kKeys[i].deg);
            c = Mix(kKeys[i].c, kKeys[i + 1].c, Clamp01(t));
            break;
        }
    }
    const Rgb neutral(1.0f, 0.985f, 0.96f);
    c = Mix(neutral, c, std::max(0.0f, warmth));
    c.r = std::max(c.r, 0.0f);
    c.g = std::max(c.g, 0.0f);
    c.b = std::max(c.b, 0.0f);
    return NormaliseLuminance(c);
}

void CrownEllipsoid(const float boxMin[3], const float boxMax[3], float center[3], float invHalf[3])
{
    constexpr float kMinHalf = 0.05f;
    for (int i = 0; i < 3; ++i)
    {
        const float size = std::max(boxMax[i] - boxMin[i], 0.0f);
        center[i] = boxMin[i] + size * (i == 1 ? 0.6f : 0.5f);
        invHalf[i] = 1.0f / std::max(0.5f * size, kMinHalf);
    }
}

LightRig ComputeLightRig(const LightRigInputs& in, const LightingTuning& tuning)
{
    LightRig rig;
    const float sinSun = std::max(-1.0f, std::min(1.0f, in.towardSun[1]));
    const float elevation = std::asin(sinSun) * 180.0f / kPi;
    rig.sunElevationDeg = elevation;

    // --- sun ---------------------------------------------------------------
    // Classic dims the sun by overcast (diffuse = full * (skyThrough*0.8+0.2)).
    // With cloud shadows on, that dimming moves into a moving cloud field whose
    // mean transmission matches it: cover c with transmission T under a cloud
    // averages 1 - c(1 - T), so c = (1 - skyThrough) for T = 0.2.
    const float skyThrough = Clamp01(in.skyThrough);
    const float cover = 1.0f - skyThrough;
    const bool clouds = in.cloudShadowsActive && tuning.cloudShadows > 0.0f && cover > 0.02f;
    const float overcastDim = clouds ? 1.0f : skyThrough * 0.8f + 0.2f;
    rig.cloudCover = clouds ? cover : 0.0f;
    rig.cloudTransmit = clouds ? 1.0f - (1.0f - kCloudTransmit) * std::min(1.0f, tuning.cloudShadows) : 1.0f;

    const float extinction = std::exp(-kExtinction * (AirMass(elevation) - 1.0f));
    const float aboveHorizon = Smoothstep(-1.5f, 1.0f, elevation);
    const float clearSun = kSunNoon * extinction * aboveHorizon;
    const float sunPower = clearSun * overcastDim * tuning.sun;
    rig.sun = SunChroma(elevation, tuning.warmth) * sunPower;

    // --- sky hemisphere ------------------------------------------------------
    const float clearSky = kSkyNoon * SkyDayFraction(sinSun);
    // Overcast spreads the lost sunlight across the dome.
    float skyPower = std::max(clearSky * (1.0f + 0.9f * cover), kSkyNight) * tuning.sky;

    // --- exposure meter ------------------------------------------------------
    // Clear-sky irradiance on flat ground, in the units where 1/brightness shows
    // the bare texture. A lower sun opens the exposure up to kMaxMeterGain, so
    // morning ground stays as bright as classic's while its shadows get deeper.
    // Metered from the clear sky: an overcast day stays darker, as it should.
    // The meter lets go again below about 20 degrees: the last hour before
    // sunset is meant to darken into a warm, contrasty frame.
    const float flatGround = clearSun * std::max(sinSun, 0.0f) + kSkyNoon * SkyDayFraction(sinSun);
    float meter = 1.0f;
    if (flatGround > 1e-5f)
        meter = std::min(std::max(std::pow((1.0f / kBrightness) / flatGround, 0.85f), 1.0f), kMaxMeterGain);
    meter = 1.0f + (meter - 1.0f) * Smoothstep(0.08f, 0.40f, sinSun);
    rig.exposure = meter * tuning.exposure;

    // In twilight the fill must not light a white wall brighter than the sky
    // behind it: cap it by the visible sky (the fog colour carries the sky
    // texture, the sky lighting and the eye accommodation).
    const Rgb fogLinear = ToLinear(in.fogColor);
    const float skyCap = Luminance(fogLinear) / (kBrightness * rig.exposure);
    const float twilight = 1.0f - Smoothstep(0.05f, 0.25f, sinSun);
    if (skyCap < skyPower)
        skyPower = std::max(skyPower + (skyCap - skyPower) * twilight, kSkyNight * tuning.sky);

    const Rgb clearZenith = NormaliseLuminance(Rgb(0.60f, 0.69f, 0.88f));
    const Rgb greyZenith = NormaliseLuminance(Rgb(0.95f, 1.00f, 1.06f));
    const Rgb zenithChroma = Mix(greyZenith, clearZenith, skyThrough);
    // The sunward horizon takes the engine fog colour (sky texture times sky
    // lighting), softened toward grey so a strongly tinted sky does not stain
    // everything; the anti-sun horizon keeps the cool zenith colour.
    const Rgb sunwardChroma = Luminance(fogLinear) > 1e-4f
                                  ? NormaliseLuminance(Mix(NormaliseLuminance(fogLinear), Rgb(1, 1, 1), 0.35f))
                                  : zenithChroma;

    rig.skyZenith = zenithChroma * skyPower;
    // Ground bounce: the ground below lit by sun and sky, near neutral.
    const float sunOnGround = Luminance(rig.sun) * std::max(sinSun, 0.0f);
    const float bouncePower = kGroundAlbedo * (sunOnGround + skyPower) * tuning.bounce;
    const Rgb bounceChroma = NormaliseLuminance(Rgb(1.0f, 1.0f, 0.96f));
    rig.skyGround = bounceChroma * bouncePower;
    // A vertical surface sees half the dome and half the ground. The sky toward
    // a low sun is brighter than the sky opposite it.
    const float low = 1.0f - Clamp01(sinSun);
    rig.skyHorizonSun = sunwardChroma * (0.55f * skyPower * (1.0f + 0.4f * low)) + rig.skyGround * 0.5f;
    rig.skyHorizon = zenithChroma * (0.55f * skyPower * (1.0f - 0.25f * low)) + rig.skyGround * 0.5f;

    // --- moon ----------------------------------------------------------------
    const float moonUp = Smoothstep(0.0f, 0.2f, in.towardMoon[1]);
    const float night = Clamp01(in.nightEffect);
    const float moonPower = kMoonFull * Clamp01(in.moonPhaseLight) * moonUp * night * tuning.moonlight;
    rig.moon = NormaliseLuminance(Rgb(0.70f, 0.80f, 1.00f)) * moonPower;
    return rig;
}

} // namespace Poseidon::remaster
