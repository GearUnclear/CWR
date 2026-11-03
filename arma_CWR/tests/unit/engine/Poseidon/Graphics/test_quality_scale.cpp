// QualityScale (XP-presets): the graded quality levels. High must be the
// identity (every hook a no-op, so the switch at High draws today's frame);
// the lower levels only ever shrink, Ultra only ever grows.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <Poseidon/Graphics/Remaster/QualityScale.hpp>

using namespace Poseidon::remaster;
using Catch::Matchers::WithinAbs;

TEST_CASE("QualityScale: High is the identity", "[Graphics][QualityScale]")
{
    const QualityProfile p = ProfileFor(QualityLevel::High);
    CHECK(p.identity);
    for (float d : {0.0f, 100.0f, 600.0f, 1500.0f, 4000.0f, 9000.0f})
        CHECK(FarLodFactor(p, d) == 1.0f);
    for (float vd : {800.0f, 900.0f, 2500.0f, 4000.0f, 5000.0f})
    {
        const float objects = vd * 600.0f / 900.0f;
        CHECK(RenderObjectDistance(p, objects, vd) == objects);
        CHECK(ShadowReach(p, 250.0f, vd, objects) == 250.0f);
    }
    LightingTuning t;
    t.fogQuality = 1.0f;
    const LightingTuning before = t;
    ApplyQualityToTuning(p, t);
    CHECK(t.fogQuality == before.fogQuality);
    CHECK(t.ambientOcclusion == before.ambientOcclusion);
    CHECK(t.features.ssr == before.features.ssr);
    CHECK(t.features.softShadows == before.features.softShadows);
}

TEST_CASE("QualityScale: lower levels shrink, Ultra grows", "[Graphics][QualityScale]")
{
    const float vd = 4000.0f, objects = 2666.7f, shadows = 500.0f;
    const QualityProfile low = ProfileFor(QualityLevel::Low);
    const QualityProfile med = ProfileFor(QualityLevel::Medium);
    const QualityProfile ultra = ProfileFor(QualityLevel::Ultra);

    CHECK(RenderObjectDistance(low, objects, vd) == 1200.0f);
    CHECK(RenderObjectDistance(med, objects, vd) == 2000.0f);
    CHECK_THAT(RenderObjectDistance(ultra, objects, vd), WithinAbs(3200.0f, 0.01f));
    CHECK(RenderObjectDistance(ultra, 3000.0f, 5000.0f) == 4000.0f);
    // A short view distance: the lower caps do not bite, nothing passes the view.
    CHECK(RenderObjectDistance(low, 533.0f, 800.0f) == 533.0f);
    CHECK(RenderObjectDistance(ultra, 533.0f, 800.0f) <= 800.0f);

    CHECK(ShadowReach(low, shadows, vd, objects) == 150.0f);
    CHECK(ShadowReach(med, shadows, vd, objects) == 300.0f);
    CHECK(ShadowReach(ultra, shadows, vd, objects) == 800.0f);
    CHECK(ShadowReach(ultra, 222.0f, 800.0f, 533.0f) >= 222.0f);
    CHECK(ShadowReach(low, 100.0f, 360.0f, 240.0f) == 100.0f);

    // Far LOD: 1 up to the ramp start, k from its end, monotonic between.
    CHECK(FarLodFactor(med, 500.0f) == 1.0f);
    CHECK(FarLodFactor(med, 1500.0f) == 2.0f);
    CHECK(FarLodFactor(med, 3000.0f) == 2.0f);
    float last = 1.0f;
    for (float d = 600.0f; d <= 1500.0f; d += 50.0f)
    {
        const float f = FarLodFactor(med, d);
        CHECK(f >= last);
        last = f;
    }
    CHECK(FarLodFactor(ultra, 3000.0f) == 1.0f);
    CHECK(FarLodFactor(low, 1000.0f) == 3.0f);
}

TEST_CASE("QualityScale: lighting caps are idempotent and keep other knobs", "[Graphics][QualityScale]")
{
    const QualityProfile low = ProfileFor(QualityLevel::Low);
    LightingTuning t;
    ApplyQualityToTuning(low, t);
    const LightingTuning once = t;
    ApplyQualityToTuning(low, t);
    CHECK(t.fogQuality == once.fogQuality);
    CHECK(t.ambientOcclusion == 0.0f);
    CHECK(t.features.ssr == 0.0f);
    CHECK(t.features.softShadows == 0.0f);
    CHECK(t.features.canopyShadows == 0.0f);
    CHECK(t.features.airBeams == 0.0f);
    CHECK(t.features.volFog == LightingTuning{}.features.volFog);
    CHECK(t.exposure == LightingTuning{}.exposure);

    LightingTuning u;
    ApplyQualityToTuning(ProfileFor(QualityLevel::Ultra), u);
    CHECK(u.fogQuality == 2.0f);
    CHECK(u.features.ssr == LightingTuning{}.features.ssr);
}

TEST_CASE("QualityScale: level names parse", "[Graphics][QualityScale]")
{
    QualityLevel l = QualityLevel::High;
    CHECK(ParseQualityLevel("low", l));
    CHECK(l == QualityLevel::Low);
    CHECK(ParseQualityLevel("ULTRA", l));
    CHECK(l == QualityLevel::Ultra);
    CHECK(ParseQualityLevel("1", l));
    CHECK(l == QualityLevel::Medium);
    CHECK_FALSE(ParseQualityLevel("", l));
    CHECK_FALSE(ParseQualityLevel(nullptr, l));
    CHECK_FALSE(ParseQualityLevel("epic", l));
    for (int i = 0; i < 4; ++i)
    {
        const QualityLevel q = static_cast<QualityLevel>(i);
        QualityLevel back = QualityLevel::High;
        CHECK(ParseQualityLevel(QualityLevelName(q), back));
        CHECK(back == q);
    }
}
