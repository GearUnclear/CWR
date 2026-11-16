#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/SeaWaves.hpp>

#include <cmath>
#include <string>
#include <string_view>
#include <vector>

// The remastered sea's plain math (Graphics/Remaster/SeaWaves.hpp): the
// cross-fade of the sea texture's animation frames, the relief gain that keeps
// a mix of two frames as strong as one, the per-frame constants of the water
// program and the sheen's light.

using namespace Poseidon::remaster;

namespace
{

// The frame PolyProperties::AnimateTexture binds: floor(phase * n), wrapped
// once (phase 1 exactly wraps to frame 0).
int AnimateTextureFrame(float phase, int n)
{
    int i = static_cast<int>(std::floor(phase * static_cast<float>(n)));
    if (i < 0)
        i += n;
    else if (i >= n)
        i -= n;
    return i;
}

std::vector<float> Weights(const SeaFrameBlend& b, int n)
{
    std::vector<float> w(static_cast<size_t>(n));
    for (int k = 0; k < n; ++k)
        w[static_cast<size_t>(k)] = SeaFrameWeight(b, k);
    return w;
}

} // namespace

TEST_CASE("Sea animation: the frames cross-fade continuously instead of switching every 1/n s", "[remaster][SeaWaves]")
{
    for (const int n : {2, 7, 8})
    {
        INFO("frames " << n);
        const int steps = 7000;
        std::vector<float> prev;
        for (int i = 0; i <= steps; ++i)
        {
            const float phase = static_cast<float>(i) / steps;
            const int bound = AnimateTextureFrame(phase, n);
            const SeaFrameBlend b = SeaAnimationBlend(phase, n, bound);
            INFO("phase " << phase << " bound " << bound << " other " << b.other << " w " << b.otherWeight);
            // The landscape's frame is on unit 0; the backend binds the other.
            CHECK(b.bound == bound);
            CHECK(b.other >= 0);
            CHECK(b.other < n);
            CHECK(b.otherWeight >= 0.0f);
            CHECK(b.otherWeight <= 1.0f);
            const std::vector<float> w = Weights(b, n);
            float sum = 0.0f;
            for (float x : w)
                sum += x;
            CHECK(sum == Catch::Approx(1.0f).margin(1e-5));
            // The mix is the even spread: frame k alone at phase k / (n - 1),
            // the two around the phase in between.
            const float s = phase * static_cast<float>(n - 1);
            for (int k = 0; k < n; ++k)
            {
                const float want = std::max(0.0f, 1.0f - std::fabs(s - static_cast<float>(k)));
                CHECK(w[static_cast<size_t>(k)] == Catch::Approx(want).margin(1e-4));
            }
            // Continuous: no frame's share jumps between neighbouring phases
            // (a hard switch moves a whole frame's share at once).
            if (!prev.empty())
            {
                const float step = static_cast<float>(n - 1) / steps;
                for (int k = 0; k < n; ++k)
                    CHECK(std::fabs(w[static_cast<size_t>(k)] - prev[static_cast<size_t>(k)]) <= step * 1.01f + 1e-5f);
            }
            prev = w;
        }
    }
}

TEST_CASE("Sea animation: a bound frame off the pair still gives the even spread", "[remaster][SeaWaves]")
{
    // AnimateTexture's rounding floor can bind the frame below at an exact frame
    // edge, and phase 1 wraps to frame 0: the mix then shows the nearer frame of
    // the pair through the other unit alone, never the bound frame.
    const int n = 7;
    const SeaFrameBlend top = SeaAnimationBlend(1.0f, n, 0);
    CHECK(top.other == n - 1);
    CHECK(top.otherWeight == 1.0f);
    CHECK(SeaFrameWeight(top, 0) == 0.0f);
    CHECK(SeaFrameWeight(top, n - 1) == 1.0f);
    // Phase 3 / 6 is frame 3 alone; AnimateTexture binds frame 3 there
    // (floor(3.5)), its rounding could give 2.
    const SeaFrameBlend mid = SeaAnimationBlend(0.5f, n, 3);
    CHECK(SeaFrameWeight(mid, 3) == Catch::Approx(1.0f));
    const SeaFrameBlend off = SeaAnimationBlend(0.5f, n, 1);
    CHECK(SeaFrameWeight(off, 3) == Catch::Approx(1.0f));
    CHECK(SeaFrameWeight(off, 1) == 0.0f);
    // No animation: the bound frame alone.
    const SeaFrameBlend still = SeaAnimationBlend(0.3f, 1, 0);
    CHECK(still.otherWeight == 0.0f);
}

TEST_CASE("Sea animation: a mix of two frames keeps the relief of one", "[remaster][SeaWaves]")
{
    CHECK(SeaCrossFadeGain(0.0f) == Catch::Approx(1.0f));
    CHECK(SeaCrossFadeGain(1.0f) == Catch::Approx(1.0f));
    // Halfway the gradient's spread of two frames with correlation rho is
    // sqrt(0.5 + 0.5 rho) of one frame's.
    CHECK(SeaCrossFadeGain(0.5f) == Catch::Approx(1.0f / std::sqrt(0.5f + 0.5f * kSeaFrameCorrelation)));
    float last = 0.0f;
    for (int i = 0; i <= 50; ++i)
    {
        const float g = SeaCrossFadeGain(static_cast<float>(i) / 100.0f);
        CHECK(g >= 1.0f);
        CHECK(g >= last);
        last = g;
    }
    // Measured on the Remaster sea (more_anim.01 to .07): gradient correlation
    // 0.65 to 0.82 between consecutive frames.
    CHECK(kSeaFrameCorrelation >= 0.65f);
    CHECK(kSeaFrameCorrelation <= 0.82f);
}

TEST_CASE("Sea constants: a low sun keeps the tuned look, a high sun keeps more of the sea's own colour",
          "[remaster][SeaWaves]")
{
    const LightingTuning t{};
    const SeaFrameBlend blend{2, 3, 0.25f};
    const SeaWaveConstants sunset = ComputeSeaWaveConstants(8.4f, t, blend);
    CHECK(sunset.relief == 1.0f);
    CHECK(sunset.crossFade == 0.25f);
    CHECK(sunset.skyMean == Catch::Approx(kSeaSkyMean));
    CHECK(sunset.skyTint == 0.0f);
    CHECK(sunset.sheen == Catch::Approx(kSeaSheen));
    CHECK(sunset.sheenFlat == 0.0f);
    const SeaWaveConstants noon = ComputeSeaWaveConstants(71.9f, t, blend);
    CHECK(noon.skyMean == Catch::Approx(kSeaSkyMean * kSeaSkyMeanNoon));
    CHECK(noon.skyTint == Catch::Approx(kSeaSkyTintNoon));
    CHECK(noon.sheen == Catch::Approx(kSeaSheenNoon));
    // At a high sun the sheen is the original's flat wash, not yellow facets.
    CHECK(noon.sheenFlat == Catch::Approx(1.0f));
    CHECK(noon.skyMean < sunset.skyMean);
    CHECK(noon.skyTint > 0.0f);
    // Monotone and smooth with the sun's height in between.
    float lastMean = 1.0f, lastTint = -1.0f;
    for (int e = 0; e <= 90; ++e)
    {
        const SeaWaveConstants c = ComputeSeaWaveConstants(static_cast<float>(e), t, blend);
        CHECK(c.skyMean <= lastMean + 1e-6f);
        CHECK(c.skyTint >= lastTint - 1e-6f);
        lastMean = c.skyMean;
        lastTint = c.skyTint;
    }
    // The sub-knobs scale what they name.
    LightingTuning k{};
    k.seaRelief = 0.0f;
    k.seaSky = 2.0f;
    k.seaSheen = 0.5f;
    const SeaWaveConstants knobs = ComputeSeaWaveConstants(8.4f, k, blend);
    CHECK(knobs.relief == 0.0f);
    CHECK(knobs.skyMean == Catch::Approx(2.0f * kSeaSkyMean));
    CHECK(knobs.sheen == Catch::Approx(0.5f * kSeaSheen));
}

TEST_CASE("Sea sheen: the rig sun's brightness in the original sun's hue", "[remaster][SeaWaves]")
{
    // The classic noon sun, display values times the 1.6 brightness.
    const Rgb classic = Rgb(0.85f, 0.75f, 0.4f) * 1.6f;
    const Rgb rig(1.0f, 0.98f, 0.95f);
    const Rgb s = SeaSheenLight(rig, classic, 0.5f);
    CHECK(Luminance(s) == Catch::Approx(0.5f * Luminance(rig)));
    // Its hue is the original's display chroma (a small increment on a
    // near-grey sea keeps it through the 2.2 power), not the more saturated
    // chroma of the linear colour.
    CHECK(s.r / s.g == Catch::Approx(classic.r / classic.g));
    CHECK(s.b / s.g == Catch::Approx(classic.b / classic.g));
    CHECK(s.r > s.g);
    CHECK(s.g > s.b);
    // A black original sun leaves the rig's luminance, neutral.
    const Rgb n = SeaSheenLight(rig, Rgb(0.0f, 0.0f, 0.0f), 1.0f);
    CHECK(n.r == Catch::Approx(n.b));
}

TEST_CASE("Sea switch: landed, gated per sea fragment, with its sub-knobs under it",
          "[remaster][SeaWaves][FeatureSwitches]")
{
    const FeatureInfo& info = GetFeatureInfo(Feature::SeaWaves);
    CHECK(info.landed);
    CHECK(std::string_view(info.key) == "seaWaves");
    CHECK(std::string_view(info.step) == "sea");
    CHECK(std::string_view(info.gpuSection) == "ground");
    for (const char* key : {"seaRelief", "seaSky", "seaSheen"})
    {
        INFO(key);
        const LightingKey* k = FindLightingKey(key);
        REQUIRE(k != nullptr);
        CHECK(k->kind == KeyKind::Knob);
        REQUIRE(k->parent != nullptr);
        CHECK(std::string_view(k->parent) == "seaWaves");
        CHECK(std::string_view(k->group) == "water");
        CHECK(KeyDefault(*k) == 1.0f);
        // Baseline puts it back with the switch off.
        LightingTuning t{};
        bool modeSet = false;
        REQUIRE(ApplyLightingSetting(t, std::string(key) + "=0.5,baseline", modeSet).empty());
        CHECK(GetKeyValue(t, *k) == 1.0f);
        CHECK_FALSE(FeatureOn(t.features, Feature::SeaWaves));
    }
    // Day and night, the sea runs wherever it is drawn; classic holds it off.
    GateInputs noon;
    noon.sunElevationDeg = 60.0f;
    CHECK(ComputeFeatureGates(noon, LightingTuning{})[Feature::SeaWaves].on);
    GateInputs night = noon;
    night.sunElevationDeg = -30.0f;
    night.nightEffect = 1.0f;
    CHECK(ComputeFeatureGates(night, LightingTuning{})[Feature::SeaWaves].on);
    LightingTuning classic{};
    classic.remastered = false;
    CHECK_FALSE(ComputeFeatureGates(noon, classic)[Feature::SeaWaves].on);
}
