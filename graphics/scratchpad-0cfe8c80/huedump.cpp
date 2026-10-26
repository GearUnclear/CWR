// The unit test's evening hull hue model over several sun heights, rig on and off.
#include <Poseidon/Graphics/Remaster/LightRig.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>

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
Rgb ClassicSun(float e)
{
    const float s = std::sin(std::max(e, 0.0f) * kPi / 180.0f);
    const float t = std::min(std::max(1.0f - s / 0.42262f, 0.0f), 1.0f);
    return Mix(Rgb(0.85f, 0.75f, 0.4f), Rgb(0.8f, 0.30f, 0.23f), t);
}
float Shown(float e, float exposure) { return ToDisplay(e * 1.6f * exposure); }

int main()
{
    for (float e : {3.0f, 4.8f, 7.0f, 7.5f, 9.0f, 12.7f, 14.0f, 20.0f})
    {
        const Rgb cs = ClassicSun(e);
        const float ambientI = std::sin(e * kPi / 180.0f) * 1.5f;
        const Rgb ca(0.55f * ambientI, 0.6f * ambientI, 0.8f * ambientI);
        LightRigInputs in = ClearSky(e);
        in.classicSunDiffuse = cs;
        constexpr float kNdotL = 0.5f;
        auto ch = [&](float sun, float amb) { return std::min((sun * kNdotL + amb) * 1.6f, 1.0f); };
        const Rgb classic(ch(cs.r, ca.r), ch(cs.g, ca.g), ch(cs.b, ca.b));
        auto hull = [&](const LightRig& rig)
        {
            const float toward = 0.5f + 0.5f * kNdotL;
            auto c = [&](float sun, float h, float hs) { return Shown(sun * kNdotL + h + (hs - h) * toward, rig.exposure); };
            return Rgb(c(rig.sun.r, rig.skyHorizon.r, rig.skyHorizonSun.r), c(rig.sun.g, rig.skyHorizon.g, rig.skyHorizonSun.g),
                       c(rig.sun.b, rig.skyHorizon.b, rig.skyHorizonSun.b));
        };
        LightingTuning on, off;
        off.features.lowSunRig = 0.0f;
        const Rgb hOn = hull(ComputeLightRig(in, on));
        const Rgb hOff = hull(ComputeLightRig(in, off));
        std::printf("e=%5.1f classic r/b %.3f g/r %.3f | off r/b %.3f g/r %.3f | on r/b %.3f g/r %.3f | on/classic r/b %.3f g/r %.3f\n",
                    e, classic.r / classic.b, classic.g / classic.r, hOff.r / hOff.b, hOff.g / hOff.r, hOn.r / hOn.b,
                    hOn.g / hOn.r, (hOn.r / hOn.b) / (classic.r / classic.b), (hOn.g / hOn.r) / (classic.g / classic.r));
    }
    // Twilight: a dim dusk sky.
    for (float e : {-1.0f, 0.0f, 1.0f, 2.0f, 3.0f, 5.0f})
    {
        LightRigInputs in = ClearSky(e);
        in.fogColor = Rgb(0.20f, 0.12f, 0.12f);
        in.classicSunDiffuse = ClassicSun(e);
        LightingTuning on, off;
        off.features.lowSunRig = 0.0f;
        const LightRig a = ComputeLightRig(in, on), b = ComputeLightRig(in, off);
        const float sky = Luminance(ToLinear(in.fogColor));
        std::printf("dusk e=%4.1f sky %.5f | zen on %.5f off %.5f | hor on %.5f off %.5f | horS on %.5f off %.5f\n", e, sky,
                    Luminance(a.skyZenith) * 1.6f * a.exposure, Luminance(b.skyZenith) * 1.6f * b.exposure,
                    Luminance(a.skyHorizon) * 1.6f * a.exposure, Luminance(b.skyHorizon) * 1.6f * b.exposure,
                    Luminance(a.skyHorizonSun) * 1.6f * a.exposure, Luminance(b.skyHorizonSun) * 1.6f * b.exposure);
    }
    return 0;
}
