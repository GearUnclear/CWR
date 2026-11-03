#include <catch2/catch_test_macros.hpp>

#include <Poseidon/World/Scene/XpViewDist.hpp>

#include <cmath>
#include <random>

// xp-viewdist (World/Scene/XpViewDist.hpp). EARLYCULL drops an object before it
// gets a draw-list entry only when Scene::LevelFromDistance2 would hide it at
// every detail width the frame can pick and no shadow path reaches it, so the
// drawn set is unchanged. These cases pin that bar against a copy of the stock
// visibility test, on random objects, and pin the far ring's pixel limit.

using namespace Poseidon::xp;

namespace
{
float Sq(float v)
{
    return v * v;
}

// The stock Scene::LevelFromDistance2 visibility test before xp-viewdist
// (objectsZ cut, then the 1.25 px tiny-object cull), written as the engine does.
bool StockVisible(float distance2, float diameter, float objectsZ, float scale, float lodInvWidth)
{
    if (distance2 > Sq(objectsZ))
        return false;
    const float pixelLimit = 0.125f;
    const float detail2 = distance2 * Sq(lodInvWidth);
    return !(Sq(diameter) < Sq(pixelLimit * scale) * detail2);
}

// ObjectForDrawing's LOD distance: the near edge for an object whose radius is
// more than a quarter of its centre distance.
float LodDistance2(float centreDist2, float radius)
{
    if (Sq(radius) > centreDist2 * Sq(0.25f))
    {
        const float d = std::sqrt(centreDist2) - radius;
        return d > 0.0f ? Sq(d) : 0.0f;
    }
    return centreDist2;
}
} // namespace

TEST_CASE("LodVisible with the far ring off is the stock test", "[xpviewdist]")
{
    std::mt19937 rng(7);
    std::uniform_real_distribution<float> dist(0.0f, 6000.0f), dia(0.05f, 120.0f), objz(100.0f, 3000.0f);
    const float scale = 0.7f * 4.0f / 3.0f;
    for (int i = 0; i < 200000; ++i)
    {
        const float d2 = Sq(dist(rng));
        const float di = dia(rng);
        const float oz = objz(rng);
        const float w = (20.0f / 1920.0f) * (1.0f + (i % 4));
        REQUIRE(LodVisible(d2, di, oz, scale, w, 0.0f) == StockVisible(d2, di, oz, scale, w));
    }
}

TEST_CASE("BeyondDrawRange never drops an object the stock test draws", "[xpviewdist]")
{
    std::mt19937 rng(11);
    std::uniform_real_distribution<float> dist(1.0f, 6000.0f), rad(0.02f, 900.0f), objz(100.0f, 3000.0f),
        shadow(50.0f, 500.0f);
    const float scale = 0.7f * 4.0f / 3.0f;
    const float wMin = 20.0f / 1920.0f;
    int dropped = 0;
    for (int i = 0; i < 200000; ++i)
    {
        const float c2 = Sq(dist(rng));
        const float r = (i % 3) ? rad(rng) * 0.05f : rad(rng);
        const float di = 2.0f * r;
        const float oz = objz(rng);
        const float sz = std::fmin(shadow(rng), oz);
        if (!BeyondDrawRange(c2, r, di, oz, sz, scale, wMin, 0.0f))
            continue;
        ++dropped;
        // nothing inside the shadow range, nothing near-edge
        REQUIRE(c2 >= Sq(sz));
        REQUIRE(Sq(r) <= c2 * Sq(0.25f));
        // hidden at the finest width and at every coarser one (pins 1, 1.5, 2, 4)
        const float l2 = LodDistance2(c2, r);
        for (float pin : {1.0f, 1.5f, 2.0f, 4.0f})
            REQUIRE_FALSE(StockVisible(l2, di, oz, scale, wMin * pin));
    }
    CHECK(dropped > 10000);
}

TEST_CASE("BeyondDrawRange drops every small far object the stock test hides by distance", "[xpviewdist]")
{
    const float scale = 0.7f * 4.0f / 3.0f;
    const float wMin = 20.0f / 1920.0f;
    // a 6 m tree at 2.7 to 4 km with objectsZ 2,667 m and shadows at 500 m
    for (float d = 2668.0f; d < 4000.0f; d += 7.0f)
        CHECK(BeyondDrawRange(d * d, 3.0f, 6.0f, 2666.67f, 500.0f, scale, wMin, 0.0f));
    // inside the object distance a visible tree stays
    CHECK_FALSE(BeyondDrawRange(Sq(2000.0f), 3.0f, 6.0f, 2666.67f, 500.0f, scale, wMin, 0.0f));
    // inside the shadow range nothing is dropped, even a speck
    CHECK_FALSE(BeyondDrawRange(Sq(400.0f), 0.01f, 0.02f, 2666.67f, 500.0f, scale, wMin, 0.0f));
}

TEST_CASE("Far ring: pixel limit and what it keeps", "[xpviewdist]")
{
    ViewDistPolicy p;
    CHECK(p.FarPixelCoef() == 0.0f);
    p.farPx = 1.25f;
    // the stock tiny-object cull is 1.25 px at the finest width
    CHECK(std::fabs(p.FarPixelCoef() - 0.125f) < 1e-6f);
    p.farPx = 4.0f;
    const float coef = p.FarPixelCoef();
    const float scale = 0.7f * 4.0f / 3.0f;
    const float w = 20.0f / 1920.0f;
    const float oz = 2666.67f;
    // projected diameter in pixels at 1080p: d * 1920 / (2 * Left * dist)
    auto px = [&](float diameter, float distance) { return diameter * 1920.0f / (2.0f * scale * distance); };
    // a 60 m forest block at 3.5 km (about 18 px) is drawn, a 6 m tree (under 2 px) is not
    CHECK(px(60.0f, 3500.0f) > 4.0f);
    CHECK(LodVisible(Sq(3500.0f), 60.0f, oz, scale, w, coef));
    CHECK(px(6.0f, 3500.0f) < 4.0f);
    CHECK_FALSE(LodVisible(Sq(3500.0f), 6.0f, oz, scale, w, coef));
    // the limit sits at 4 px
    const float d4 = 4.0f * 2.0f * scale * 3000.0f / 1920.0f;
    CHECK(LodVisible(Sq(3000.0f), d4 * 1.01f, oz, scale, w, coef));
    CHECK_FALSE(LodVisible(Sq(3000.0f), d4 * 0.99f, oz, scale, w, coef));
    // inside objectsZ the far ring changes nothing
    CHECK(LodVisible(Sq(2000.0f), 6.0f, oz, scale, w, coef) == StockVisible(Sq(2000.0f), 6.0f, oz, scale, w));
    // EARLYCULL with the far ring keeps what the far ring draws
    CHECK_FALSE(BeyondDrawRange(Sq(3500.0f), 30.0f, 60.0f, oz, 500.0f, scale, w, coef));
    CHECK(BeyondDrawRange(Sq(3500.0f), 3.0f, 6.0f, oz, 500.0f, scale, w, coef));
}

TEST_CASE("CellBeyond: a skipped cell holds only objects BeyondDrawRange drops", "[xpviewdist]")
{
    std::mt19937 rng(23);
    std::uniform_real_distribution<float> u(-1.0f, 1.0f), cdist(0.0f, 6000.0f), crad(20.0f, 400.0f);
    const float scale = 0.7f * 4.0f / 3.0f;
    const float wMin = 20.0f / 1920.0f;
    const float oz = 2666.67f, sz = 500.0f;
    const float range = std::fmax(oz, sz);
    int skipped = 0;
    for (int i = 0; i < 20000; ++i)
    {
        const float cd = cdist(rng);
        const float cr = crad(rng);
        if (!CellBeyond(cd, cr, range))
            continue;
        ++skipped;
        for (int k = 0; k < 20; ++k)
        {
            // an object sphere inside the cell sphere: |p - c| + r <= cr
            const float r = std::fabs(u(rng)) * cr * 0.5f;
            const float off = std::fabs(u(rng)) * (cr - r);
            const float dir = u(rng);
            const float od = cd + off * dir; // along the camera ray: the extreme cases
            REQUIRE(BeyondDrawRange(od * od, r, 2.0f * r, oz, sz, scale, wMin, 0.0f));
        }
    }
    CHECK(skipped > 1000);
    CHECK_FALSE(CellBeyond(4000.0f, 100.0f, 0.0f)); // range 0: the far ring is on
}

TEST_CASE("SMALLPX: the tiny-object cull limit, and EARLYCULL follows it", "[xpviewdist]")
{
    ViewDistPolicy p;
    CHECK(p.SmallPixelCoef() == 0.125f); // the stock coefficient, bit for bit
    p.smallPx = 2.5f;
    const float coef = p.SmallPixelCoef();
    CHECK(std::fabs(coef - 0.25f) < 1e-6f);
    const float scale = 0.7f * 4.0f / 3.0f;
    const float w = 20.0f / 1920.0f;
    // a 4 m bush at 2 km is about 2.06 px: drawn by the stock cull, culled at 2.5 px
    CHECK(StockVisible(Sq(2000.0f), 4.0f, 2666.67f, scale, w));
    CHECK_FALSE(LodVisible(Sq(2000.0f), 4.0f, 2666.67f, scale, w, 0.0f, coef));
    CHECK(BeyondDrawRange(Sq(2000.0f), 2.0f, 4.0f, 2666.67f, 500.0f, scale, w, 0.0f, coef));
    CHECK_FALSE(BeyondDrawRange(Sq(2000.0f), 2.0f, 4.0f, 2666.67f, 500.0f, scale, w, 0.0f));
}
