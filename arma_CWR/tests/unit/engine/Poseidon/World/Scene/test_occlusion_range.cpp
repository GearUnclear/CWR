#include <catch2/catch_test_macros.hpp>

#include <Poseidon/World/Scene/OcclusionRange.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <vector>

// W3-1(b): beyond a range the object occlusion pass (Scene::DrawObjectsAndShadowsPass1)
// stops testing objects that are not occluders. The bar is that the set of
// drawn objects only grows (an object the test would have hidden may now be
// drawn, behind its occluders), never shrinks, and that the occlusion buffer,
// which also feeds the flares and the projected shadows, stays identical. These
// tests pin the decision rules and run a model of the pass (same loop shape as
// SceneDraw.cpp, a one-dimensional nearest-depth buffer) on random scenes with
// and without the range.

using namespace Poseidon::OcclusionRange;

TEST_CASE("ParseRange: default, numbers, zero and junk", "[occrange]")
{
    CHECK(ParseRange(nullptr) == kDefaultRange);
    CHECK(ParseRange("") == kDefaultRange);
    CHECK(ParseRange("abc") == kDefaultRange);
    CHECK(ParseRange("nan") == kDefaultRange);
    CHECK(ParseRange("300") == 300.0f);
    CHECK(ParseRange("1000.5") == 1000.5f);
    CHECK(ParseRange("0") == 0.0f);
    CHECK(ParseRange("-5") == 0.0f);
}

TEST_CASE("TestRange2: no range, raised to the caster reach, squared", "[occrange]")
{
    CHECK(TestRange2(0.0f, 250.0f) == 0.0f);
    CHECK(TestRange2(-1.0f, 250.0f) == 0.0f);
    CHECK(TestRange2(600.0f, 250.0f) == 600.0f * 600.0f);
    CHECK(TestRange2(600.0f, 600.0f) == 600.0f * 600.0f);
    // a caster reach beyond the range wins: no untested object casts into the shadow map
    CHECK(TestRange2(300.0f, 500.0f) == 500.0f * 500.0f);
    CHECK(TestRange2(600.0f, 0.0f) == 600.0f * 600.0f);
}

TEST_CASE("SkipTest: only far non-occluders skip, the boundary is inclusive", "[occrange]")
{
    const float r2 = 600.0f * 600.0f;
    CHECK_FALSE(SkipTest(599.0f * 599.0f, r2, false));
    CHECK(SkipTest(r2, r2, false));
    CHECK(SkipTest(2000.0f * 2000.0f, r2, false));
    // an occluder keeps its test at any distance
    CHECK_FALSE(SkipTest(r2, r2, true));
    CHECK_FALSE(SkipTest(2000.0f * 2000.0f, r2, true));
    // no range: nothing skips
    CHECK_FALSE(SkipTest(1.0e12f, 0.0f, false));
}

namespace
{
constexpr int kPixels = 64;

struct ModelObject
{
    float distance2;
    int x0, x1;             // covered pixels [x0, x1] of its test box and of its view geometry
    float depth;            // nearest depth of the box
    bool canBeOccluded;     // LODShape::CanBeOccluded
    bool farEnough;         // FarEnoughForOcclusion
    bool occluderIfVisible; // OccluderViewGeometry != null
};

struct PassResult
{
    std::vector<int> culled;   // indices hidden by the pass
    std::vector<int> rendered; // indices rendered into the buffer, in order
    std::vector<float> buffer;
    Stats stats;
};

// The occlusion loop of Scene::DrawObjectsAndShadowsPass1 on a model buffer.
// `objs` is sorted near to far, as the front-to-back walk of occSort visits it.
PassResult RunPass(const std::vector<ModelObject>& objs, float range2)
{
    PassResult r;
    r.buffer.assign(kPixels, std::numeric_limits<float>::infinity());
    for (int i = 0; i < static_cast<int>(objs.size()); i++)
    {
        const ModelObject& o = objs[i];
        bool occluded = false;
        if (o.canBeOccluded && o.farEnough)
        {
            const bool beyond = range2 > 0.0f && o.distance2 >= range2;
            if (beyond && SkipTest(o.distance2, range2, o.occluderIfVisible))
            {
                ++r.stats.skipped;
            }
            else
            {
                ++r.stats.tested;
                r.stats.farTested += beyond ? 1 : 0;
                bool hidden = true;
                for (int x = o.x0; x <= o.x1; x++)
                {
                    hidden = hidden && r.buffer[x] < o.depth;
                }
                if (hidden)
                {
                    occluded = true;
                    ++r.stats.culled;
                }
            }
        }
        if (!occluded)
        {
            if (o.occluderIfVisible)
            {
                r.rendered.push_back(i);
                for (int x = o.x0; x <= o.x1; x++)
                {
                    r.buffer[x] = std::min(r.buffer[x], o.depth);
                }
            }
        }
        else
        {
            r.culled.push_back(i);
        }
    }
    return r;
}

std::vector<ModelObject> RandomScene(std::mt19937& rng, int count)
{
    std::uniform_real_distribution<float> dist(30.0f, 3000.0f);
    std::uniform_int_distribution<int> px(0, kPixels - 1);
    std::uniform_int_distribution<int> width(0, 12);
    std::uniform_int_distribution<int> pct(0, 99);
    std::vector<ModelObject> objs;
    for (int i = 0; i < count; i++)
    {
        ModelObject o{};
        // a few exact duplicates of a distance, and some on the range boundary
        float d = dist(rng);
        if (pct(rng) < 5)
        {
            d = 600.0f;
        }
        else if (pct(rng) < 5 && !objs.empty())
        {
            d = std::sqrt(objs.back().distance2);
        }
        o.distance2 = d * d;
        o.depth = d;
        o.x0 = px(rng);
        o.x1 = std::min(kPixels - 1, o.x0 + width(rng));
        o.canBeOccluded = pct(rng) < 85;
        o.farEnough = pct(rng) < 95;
        o.occluderIfVisible = pct(rng) < 25;
        objs.push_back(o);
    }
    std::stable_sort(objs.begin(), objs.end(),
                     [](const ModelObject& a, const ModelObject& b) { return a.distance2 < b.distance2; });
    return objs;
}
} // namespace

TEST_CASE("Occlusion pass with a range: same buffer, drawn set only grows", "[occrange]")
{
    std::mt19937 rng(0x0CC5A9u);
    const float ranges[] = {100.0f, 300.0f, 600.0f, 1000.0f};
    int grewSomewhere = 0;
    for (int scene = 0; scene < 400; scene++)
    {
        const std::vector<ModelObject> objs = RandomScene(rng, 20 + scene % 180);
        const PassResult full = RunPass(objs, 0.0f);
        CHECK(full.stats.skipped == 0);
        for (float range : ranges)
        {
            const float range2 = TestRange2(range, 0.0f);
            const PassResult part = RunPass(objs, range2);

            // the buffer the flares and the projected shadows read is identical,
            // and so is every occluder render in its order
            REQUIRE(part.rendered == full.rendered);
            REQUIRE(part.buffer == full.buffer);

            // nothing is hidden that the full test drew
            for (int i : part.culled)
            {
                REQUIRE(std::find(full.culled.begin(), full.culled.end(), i) != full.culled.end());
            }
            // what the range draws in addition: far, not an occluder, not tested
            for (int i : full.culled)
            {
                if (std::find(part.culled.begin(), part.culled.end(), i) == part.culled.end())
                {
                    REQUIRE(objs[i].distance2 >= range2);
                    REQUIRE_FALSE(objs[i].occluderIfVisible);
                    ++grewSomewhere;
                }
            }
            // inside the range every verdict is the full test's
            for (int i = 0; i < static_cast<int>(objs.size()); i++)
            {
                if (objs[i].distance2 < range2)
                {
                    const bool a = std::find(full.culled.begin(), full.culled.end(), i) != full.culled.end();
                    const bool b = std::find(part.culled.begin(), part.culled.end(), i) != part.culled.end();
                    REQUIRE(a == b);
                }
            }
            // counters: every eligible object is either tested or skipped
            REQUIRE(part.stats.tested + part.stats.skipped == full.stats.tested);
            REQUIRE(part.stats.culled == static_cast<int>(part.culled.size()));
            REQUIRE(part.stats.farTested <= part.stats.tested);
        }
    }
    // the model does exercise the case the range changes
    CHECK(grewSomewhere > 0);
}

TEST_CASE("Occlusion pass with no range is the full test", "[occrange]")
{
    std::mt19937 rng(42u);
    for (int scene = 0; scene < 50; scene++)
    {
        const std::vector<ModelObject> objs = RandomScene(rng, 100);
        const PassResult a = RunPass(objs, 0.0f);
        const PassResult b = RunPass(objs, TestRange2(ParseRange("0"), 250.0f));
        REQUIRE(a.culled == b.culled);
        REQUIRE(a.rendered == b.rendered);
        REQUIRE(a.buffer == b.buffer);
    }
}
