// Dumps the light rig at a few sun heights (the unit test's ClearSky inputs).
#include <Poseidon/Graphics/Remaster/LightRig.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace Poseidon::remaster;

constexpr float kPi = 3.14159265358979323846f;

LightRigInputs ClearSky(float elevationDeg, float azimuthDeg = 200.0f)
{
    LightRigInputs in;
    const float e = elevationDeg * kPi / 180.0f;
    const float a = azimuthDeg * kPi / 180.0f;
    in.towardSun[0] = std::cos(e) * std::sin(a);
    in.towardSun[1] = std::sin(e);
    in.towardSun[2] = std::cos(e) * std::cos(a);
    in.fogColor = Rgb(0.73f, 0.79f, 0.85f);
    in.skyThrough = 1.0f;
    in.nightEffect = elevationDeg > 5.0f ? 0.0f : 1.0f;
    return in;
}

// Classic's sun colour at an elevation (LightSun::Recalculate): colorFull from
// FullSunColor (0.85, 0.75, 0.4) to SunsetColor (0.8, 0.3, 0.23) below 25 deg.
Rgb ClassicSun(float e)
{
    const float s = std::sin(std::max(e, 0.0f) * kPi / 180.0f);
    const float t = std::min(std::max(1.0f - s / 0.42262f, 0.0f), 1.0f);
    return Mix(Rgb(0.85f, 0.75f, 0.4f), Rgb(0.8f, 0.30f, 0.23f), t);
}

float Shown(float e, float exposure)
{
    return ToDisplay(e * 1.6f * exposure);
}

int main(int argc, char** argv)
{
    const bool on = argc > 1 && std::strcmp(argv[1], "on") == 0;
    LightingTuning t;
#ifdef HAS_FEATURES
    t.features.lowSunRig = on ? 1.0f : 0.0f;
#endif
    for (float e : {60.0f, 30.0f, 25.0f, 14.0f, 12.7f, 9.0f, 7.5f, 7.0f, 4.8f, 3.0f, 1.0f})
    {
        LightRigInputs in = ClearSky(e);
        in.classicSunDiffuse = ClassicSun(e);
        const LightRig r = ComputeLightRig(in, t);
        std::printf("e=%5.1f sun=(%.9g,%.9g,%.9g) zen=(%.9g,%.9g,%.9g) hor=(%.9g,%.9g,%.9g) horS=(%.9g,%.9g,%.9g) "
                    "gnd=(%.9g,%.9g,%.9g) exp=%.9g\n",
                    e, r.sun.r, r.sun.g, r.sun.b, r.skyZenith.r, r.skyZenith.g, r.skyZenith.b, r.skyHorizon.r,
                    r.skyHorizon.g, r.skyHorizon.b, r.skyHorizonSun.r, r.skyHorizonSun.g, r.skyHorizonSun.b,
                    r.skyGround.r, r.skyGround.g, r.skyGround.b, r.exposure);
        // Side-lit hull: lit face 45 deg from the sun azimuth, shaded face 135.
        const float ce = std::cos(e * kPi / 180.0f);
        auto face = [&](float cosPhi)
        {
            const float toward = 0.5f + 0.5f * cosPhi;
            const float sunTerm = std::max(ce * cosPhi, 0.0f);
            return Rgb(r.sun.r * sunTerm + r.skyHorizon.r + (r.skyHorizonSun.r - r.skyHorizon.r) * toward,
                       r.sun.g * sunTerm + r.skyHorizon.g + (r.skyHorizonSun.g - r.skyHorizon.g) * toward,
                       r.sun.b * sunTerm + r.skyHorizon.b + (r.skyHorizonSun.b - r.skyHorizon.b) * toward);
        };
        const Rgb lit = face(0.70710678f);
        const Rgb shade = face(-0.70710678f);
        const float fl = Shown(Luminance(lit), r.exposure);
        const float fs = Shown(Luminance(shade), r.exposure);
        // Facing the sun and facing away (the README calibration table).
        const Rgb sunWall = face(1.0f);
        const Rgb awayWall = face(-1.0f);
        const float sinSun = std::sin(e * kPi / 180.0f);
        const float ground = Luminance(r.sun) * sinSun + Luminance(r.skyZenith);
        std::printf("   hull lit F=%.3f shade F=%.3f shade/lit=%.3f lit r:b=%.3f g:r=%.3f | ground %.3f cast %.3f "
                    "wallSun %.3f wallAway %.3f\n",
                    fl, fs, fs / fl, ToDisplay(lit.r) / ToDisplay(lit.b), ToDisplay(lit.g) / ToDisplay(lit.r),
                    Shown(ground, r.exposure), Shown(Luminance(r.skyZenith), r.exposure),
                    Shown(Luminance(sunWall), r.exposure), Shown(Luminance(awayWall), r.exposure));
    }
    return 0;
}
