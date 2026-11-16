#include <cmath>
#include <cstdlib>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/World/Scene/LodPop.hpp>

// The rules of the LOD hysteresis and the cross-fade's sample masks.

using Poseidon::LodPop::EnvFlagDefaultOn;
using Poseidon::LodPop::EnvPercentBandOr;
using Poseidon::LodPop::FaceInDissolve;
using Poseidon::LodPop::FadeMasks;
using Poseidon::LodPop::FadeSampleMasks;
using Poseidon::LodPop::HeldLevel;
using Poseidon::LodPop::kKeepMaxShadow;
using Poseidon::LodPop::MinShadowEyeDepth;
using Poseidon::LodPop::ShadowLength;

namespace
{
// Thresholds at resolution 10 and 20: level 0 below 10, 1 below 20, 2 above.
int Find(float resol2)
{
    return resol2 < 100.0f ? 0 : resol2 < 400.0f ? 1 : 2;
}
constexpr float kBand = 1.04f;
float Sq(float v)
{
    return v * v;
}

// Sets a variable for one test; an empty value removes it.
void SetTestEnv(const char* name, const char* value)
{
#ifdef _WIN32
    _putenv_s(name, value);
#else
    if (*value)
        setenv(name, value, 1);
    else
        unsetenv(name);
#endif
}
} // namespace

TEST_CASE("Shipped switches are on unless set to 0", "[lodpop]")
{
    const char* name = "UD_TEST_LODPOP_FLAG";
    SetTestEnv(name, "");
    CHECK(EnvFlagDefaultOn(name));
    SetTestEnv(name, "1");
    CHECK(EnvFlagDefaultOn(name));
    SetTestEnv(name, "0");
    CHECK_FALSE(EnvFlagDefaultOn(name));
    SetTestEnv(name, "");
}

TEST_CASE("EnvPercentBandOr falls back to the shipped band and 0 turns it off", "[lodpop]")
{
    const char* name = "UD_TEST_LODPOP_BAND";
    SetTestEnv(name, "");
    CHECK(EnvPercentBandOr(name, 10.0f) == Catch::Approx(1.10f));
    SetTestEnv(name, "25");
    CHECK(EnvPercentBandOr(name, 10.0f) == Catch::Approx(1.25f));
    SetTestEnv(name, "0");
    CHECK(EnvPercentBandOr(name, 10.0f) == 0.0f);
    SetTestEnv(name, "");
}

TEST_CASE("HeldLevel keeps the level inside the band", "[lodpop]")
{
    CHECK(HeldLevel(0, Sq(10.2f), kBand, Find) == 0); // past 10, inside the band
    CHECK(HeldLevel(1, Sq(9.8f), kBand, Find) == 1);  // under 10, inside the band
}

TEST_CASE("HeldLevel leaves the level past the band", "[lodpop]")
{
    CHECK(HeldLevel(0, Sq(10.5f), kBand, Find) == 1);
    CHECK(HeldLevel(1, Sq(9.5f), kBand, Find) == 0);
    CHECK(HeldLevel(0, Sq(30.0f), kBand, Find) == 2); // far past: straight to the stateless level
    CHECK(HeldLevel(2, Sq(5.0f), kBand, Find) == 0);
}

TEST_CASE("HeldLevel with no band is the stateless choice", "[lodpop]")
{
    for (float r : {5.0f, 9.99f, 10.01f, 19.99f, 20.01f, 30.0f})
    {
        for (int held = 0; held < 3; held++)
        {
            CHECK(HeldLevel(held, Sq(r), 1.0f, Find) == Find(Sq(r)));
        }
    }
}

TEST_CASE("FadeSampleMasks are complementary and both live until the end", "[lodpop]")
{
    for (int samples : {2, 4, 8})
    {
        const unsigned all = (1u << samples) - 1u;
        for (float t = 0.0f; t < 1.0f; t += 0.05f)
        {
            const FadeMasks m = FadeSampleMasks(t, samples);
            CHECK((m.now | m.was) == all);
            CHECK((m.now & m.was) == 0u);
            CHECK(m.now != 0u);
            CHECK(m.was != 0u);
        }
    }
}

TEST_CASE("FadeSampleMasks give the new level more samples as the fade goes", "[lodpop]")
{
    CHECK(FadeSampleMasks(0.0f, 4).now == 0x1u);
    CHECK(FadeSampleMasks(0.5f, 4).now == 0x3u);
    CHECK(FadeSampleMasks(0.95f, 4).now == 0x7u);
}

TEST_CASE("ShadowLength follows the sun height and stops at the cap", "[lodpop]")
{
    CHECK(ShadowLength(20.0f, 1.0f) == Catch::Approx(20.0f));
    CHECK(ShadowLength(20.0f, 0.5f) == Catch::Approx(40.0f));
    CHECK(ShadowLength(20.0f, 0.0698f) == Catch::Approx(286.5f).margin(0.1f));
    CHECK(ShadowLength(20.0f, 0.01f) == Catch::Approx(286.5f).margin(0.1f)); // the sun counts as 4 degrees up
    CHECK(ShadowLength(40.0f, 0.0698f) == kKeepMaxShadow);
}

TEST_CASE("MinShadowEyeDepth is the nearest end of the shadow", "[lodpop]")
{
    // sun from behind the object toward the camera: the shadow comes nearer
    CHECK(MinShadowEyeDepth(600.0f, 100.0f, -1.0f, 5.0f) == Catch::Approx(495.0f));
    // sun from behind the camera: the shadow runs away, the object is the nearest part
    CHECK(MinShadowEyeDepth(600.0f, 100.0f, 1.0f, 5.0f) == Catch::Approx(595.0f));
    // sideways sun: no change in depth
    CHECK(MinShadowEyeDepth(600.0f, 100.0f, 0.0f, 5.0f) == Catch::Approx(595.0f));
}

TEST_CASE("FaceInDissolve hands the faces over evenly and completely", "[lodpop]")
{
    constexpr int kFaces = 200;
    int lastIn = 0;
    for (float t : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f})
    {
        int in = 0, out = 0;
        for (int f = 0; f < kFaces; f++)
        {
            in += FaceInDissolve(f, t, true) ? 1 : 0;
            out += FaceInDissolve(f, t, false) ? 1 : 0;
        }
        CHECK(in + out == kFaces);                                     // each face belongs to one side
        CHECK(in == Catch::Approx(t * kFaces).margin(kFaces * 0.03f)); // and the share follows t
        CHECK(in >= lastIn);
        lastIn = in;
    }
    // a face that has come in stays in
    for (int f = 0; f < kFaces; f++)
    {
        if (FaceInDissolve(f, 0.3f, true))
        {
            CHECK(FaceInDissolve(f, 0.6f, true));
        }
    }
}

// Round 3: arrival by distance (POSEIDON_XP_LODBAND, POSEIDON_XP_SHADOWBAND).

TEST_CASE("BandBlend is the stateless level outside a band", "[lodpop]")
{
    using Poseidon::LodPop::BandBlend;
    const auto a = BandBlend(Sq(5.0f), 1.1f, Find);
    CHECK(a.fine == 0);
    CHECK(a.coarse == 0);
    const auto b = BandBlend(Sq(15.0f), 1.1f, Find);
    CHECK(b.fine == 1);
    CHECK(b.coarse == 1);
    const auto c = BandBlend(Sq(10.5f), 0.0f, Find); // off: no band at all
    CHECK(c.fine == 1);
    CHECK(c.coarse == 1);
}

TEST_CASE("BandBlend runs from the finer to the coarser level over the band before the switch", "[lodpop]")
{
    using Poseidon::LodPop::BandBlend;
    // the switch at 10, band 10 %: from 10 / 1.1 to 10
    const auto mid = BandBlend(Sq(10.0f / std::sqrt(1.1f)), 1.1f, Find);
    CHECK(mid.fine == 0);
    CHECK(mid.coarse == 1);
    CHECK(mid.t == Catch::Approx(0.5f).margin(0.01f));
    const auto nearEdge = BandBlend(Sq(10.0f / 1.1f * 1.001f), 1.1f, Find);
    CHECK(nearEdge.t < 0.02f);
    const auto atSwitch = BandBlend(Sq(10.0f * 0.9999f), 1.1f, Find);
    CHECK(atSwitch.t > 0.98f);
    const auto past = BandBlend(Sq(10.5f), 1.1f, Find); // past the switch: what it drew before, alone
    CHECK(past.fine == 1);
    CHECK(past.coarse == 1);
    float last = -1.0f;
    for (float r = 9.0f; r < 10.0f; r += 0.05f)
    {
        const float t = BandBlend(Sq(r), 1.1f, Find).t;
        CHECK(t >= last); // never back: the coarser share only grows with distance
        last = t;
    }
}

TEST_CASE("ShareSamples rounds a share to whole samples", "[lodpop]")
{
    using Poseidon::LodPop::ShareSamples;
    CHECK(ShareSamples(0.0f, 4) == 0);
    CHECK(ShareSamples(0.1f, 4) == 0);
    CHECK(ShareSamples(0.2f, 4) == 1);
    CHECK(ShareSamples(0.5f, 4) == 2);
    CHECK(ShareSamples(1.0f, 4) == 4);
    CHECK(ShareSamples(1.5f, 4) == 4);
}

TEST_CASE("EdgeShare fades an object in over the last band before an edge", "[lodpop]")
{
    using Poseidon::LodPop::EdgeShare;
    CHECK(EdgeShare(1000.0f, 1500.0f, 1.1f, false) == 1.0f);        // well inside the distance
    CHECK(EdgeShare(1500.0f, 1500.0f, 1.1f, false) == 0.0f);        // at the edge
    CHECK(EdgeShare(1500.0f / 1.05f, 1500.0f, 1.1f, false) > 0.4f); // half way in (log)
    CHECK(EdgeShare(1500.0f / 1.05f, 1500.0f, 1.1f, false) < 0.6f);
    CHECK(EdgeShare(2.0f, 1.0f, 1.1f, true) == 1.0f);        // big enough on screen
    CHECK(EdgeShare(1.0f, 1.0f, 1.1f, true) == 0.0f);        // at the size cut
    CHECK(EdgeShare(1500.0f, 1500.0f, 0.0f, false) == 1.0f); // off
}

TEST_CASE("MinShadowRadialDistance is the nearest point of the shadow", "[lodpop]")
{
    using Poseidon::LodPop::MinShadowRadialDistance;
    // object 600 m ahead, sun straight down: the shadow is under it
    CHECK(MinShadowRadialDistance(0.0f, 0.0f, 600.0f, 0.0f, -1.0f, 0.0f, 20.0f, 5.0f) ==
          Catch::Approx(595.0f).margin(0.01f));
    // sun from ahead, shadow toward the camera for 150 m: nearest end at 450 m
    CHECK(MinShadowRadialDistance(0.0f, 0.0f, 600.0f, 0.0f, 0.0f, -1.0f, 150.0f, 0.0f) ==
          Catch::Approx(450.0f).margin(0.01f));
    // sun from behind: the shadow runs away, the object itself is nearest
    CHECK(MinShadowRadialDistance(0.0f, 0.0f, 600.0f, 0.0f, 0.0f, 1.0f, 150.0f, 0.0f) ==
          Catch::Approx(600.0f).margin(0.01f));
    // the shadow passes beside the eye: the closest approach, not an end
    CHECK(MinShadowRadialDistance(100.0f, 0.0f, 50.0f, 0.0f, 0.0f, -1.0f, 200.0f, 0.0f) ==
          Catch::Approx(100.0f).margin(0.01f));
}

TEST_CASE("Dissolve: the two levels of a band caster split every cell between them", "[lodpop]")
{
    using Poseidon::LodPop::DissolveCellHash;
    using Poseidon::LodPop::DissolveKeeps;
    using Poseidon::LodPop::DissolveKey;
    for (float t : {0.0f, 0.1f, 0.37f, 0.5f, 0.9f, 1.0f})
    {
        int coarse = 0;
        const int n = 4000;
        for (int i = 0; i < n; i++)
        {
            const float h =
                DissolveCellHash(static_cast<float>(i % 97) * 0.0625f, static_cast<float>(i / 97) * 0.0625f, 3.0f);
            const bool c = DissolveKeeps(h, DissolveKey(t, true));
            const bool f = DissolveKeeps(h, DissolveKey(t, false));
            CHECK(c != f); // exactly one level owns each cell
            coarse += c ? 1 : 0;
        }
        // the coarser level's share of the cells follows t
        CHECK(static_cast<float>(coarse) / n == Catch::Approx(t).margin(0.03));
    }
}

TEST_CASE("Dissolve: a cell keeps its hash as the camera moves (world-anchored)", "[lodpop]")
{
    using Poseidon::LodPop::DissolveCellHash;
    using Poseidon::LodPop::DissolveHashCoord;
    // the same world point, written as world coordinates far apart in value
    CHECK(DissolveHashCoord(5000.03f) == Catch::Approx(DissolveHashCoord(5000.03f - 256.0f * 19.0f)).margin(1e-3));
    CHECK(DissolveHashCoord(-0.5f) == Catch::Approx(255.5f));
    // and the hash is a function of the cell only
    CHECK(DissolveCellHash(10.01f, 2.0f, 3.0f) == DissolveCellHash(10.05f, 2.0f, 3.0f));
    CHECK(DissolveCellHash(10.01f, 2.0f, 3.0f) != DissolveCellHash(10.07f, 2.0f, 3.0f));
}
