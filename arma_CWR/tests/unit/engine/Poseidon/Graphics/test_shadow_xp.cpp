#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Shadow/ShadowMath.hpp>
#include <Poseidon/Graphics/Shadow/ShadowXp.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

// XP-shadows switches (graphics-experimental lane): the tier table keeps the
// tip at tier 2, the map reach formula, the minimal frustum-slice sphere and
// the conservative per-cascade caster test.

using namespace Poseidon::shadow;

TEST_CASE("ShadowXp: tier 2 is the tip's shadow set", "[Graphics][ShadowXp]")
{
    const xp::TierParams t = xp::Tier(2);
    CHECK(t.resolution == 2048);
    CHECK(t.cascades == 4);
    CHECK(t.omni == 2);
    CHECK(t.reach == 0.0f);
    CHECK(t.taaTaps == 0);
    CHECK_FALSE(t.cull);
    CHECK_FALSE(t.tight);
    // Out-of-range tiers fall back to the tip.
    CHECK(xp::Tier(9).resolution == 2048);
}

TEST_CASE("ShadowXp: map reach keeps the shadow distance without a cap", "[Graphics][ShadowXp]")
{
    // reach 0: the shadow distance as it is.
    CHECK(xp::MapFar(500.0f, 4000.0f, 2667.0f, 0.0f) == 500.0f);
    CHECK(xp::MapFar(222.0f, 800.0f, 533.0f, 0.0f) == 222.0f);
    // Ultra (1000 m cap): long views reach view * 5/18 up to the cap.
    CHECK(xp::MapFar(500.0f, 4000.0f, 2667.0f, 1000.0f) == Catch::Approx(1000.0f));
    CHECK(xp::MapFar(500.0f, 2500.0f, 1667.0f, 1000.0f) == Catch::Approx(694.444f).margin(0.01f));
    // Short views stay where they are (view * 5/18 == the shadow distance).
    CHECK(xp::MapFar(222.222f, 800.0f, 533.0f, 1000.0f) == Catch::Approx(222.222f).margin(0.01f));
    // The object distance caps it.
    CHECK(xp::MapFar(500.0f, 5000.0f, 900.0f, 1600.0f) == Catch::Approx(900.0f));
    // A cap below the shadow distance lowers it (low tier).
    CHECK(xp::MapFar(500.0f, 4000.0f, 2667.0f, 300.0f) == Catch::Approx(300.0f));
    CHECK(xp::MapFar(222.0f, 800.0f, 533.0f, 300.0f) == Catch::Approx(222.222f).margin(0.01f));
}

TEST_CASE("ShadowXp: the static caster cache grows with the reach", "[Graphics][ShadowXp]")
{
    CHECK(xp::CasterCacheCap(250.0f) == 1024);
    CHECK(xp::CasterCacheCap(500.0f) == 1024);
    CHECK(xp::CasterCacheCap(1000.0f) == 4096);
    CHECK(xp::CasterCacheCap(5000.0f) == 12288);
}

TEST_CASE("ShadowXp: minimal slice sphere holds every corner and beats the centroid sphere", "[Graphics][ShadowXp]")
{
    const Vec3 fw = Normalize(Vec3{0.3f, -0.1f, 1.0f});
    const Vec3 rt = Normalize(Cross(Vec3{0.0f, 1.0f, 0.0f}, fw));
    const Vec3 up = Cross(fw, rt);
    const float tx = 0.75f, ty = 0.42f;
    const float splits[][2] = {{0.5f, 30.0f}, {61.4f, 185.6f}, {185.6f, 500.0f}, {123.0f, 372.0f}, {372.0f, 1000.0f}};
    for (const auto& sp : splits)
    {
        const std::array<Vec3, 8> c = CameraFrustumCornersWorld({0.0f, 0.0f, 0.0f}, fw, rt, up, tx, ty, sp[0], sp[1]);
        const BoundingSphere tight = MinimalSliceSphere(fw, tx, ty, sp[0], sp[1]);
        const BoundingSphere loose = FrustumBoundingSphere(c);
        for (const Vec3& p : c)
            CHECK(Length(p - tight.center) <= tight.radius + 1e-3f);
        CHECK(tight.radius <= loose.radius + 1e-3f);
    }
}

TEST_CASE("ShadowXp: tight frustum tiers keep their selection distances", "[Graphics][ShadowXp]")
{
    CascadeBuildParams p;
    p.forward = {0.0f, 0.0f, -1.0f};
    p.right = {1.0f, 0.0f, 0.0f};
    p.up = {0.0f, 1.0f, 0.0f};
    p.tanHalfX = 0.75f;
    p.tanHalfY = 0.42f;
    p.nearD = 0.5f;
    p.farD = 500.0f;
    p.sunDir = {0.3f, -0.8f, 0.2f};
    p.distanceCoef = 1.0f;
    p.count = 4;
    p.omniCount = 2;
    p.omniCoef[0] = 0.08f;
    p.omniCoef[1] = 0.20f;
    const CascadeSet a = BuildShadowCascadesTiered(p);
    p.tightSphere = true;
    const CascadeSet b = BuildShadowCascadesTiered(p);
    for (int i = 0; i < 4; i++)
        CHECK(a.splitViewDist[i] == b.splitViewDist[i]);
    // Omni tiers untouched; frustum tiers get more texels per metre (larger x scale).
    for (int i = 0; i < 2; i++)
        CHECK(a.camRelVP[i].m == b.camRelVP[i].m);
    for (int i = 2; i < 4; i++)
        CHECK(
            std::fabs(b.camRelVP[i].at(0, 0)) + std::fabs(b.camRelVP[i].at(0, 1)) + std::fabs(b.camRelVP[i].at(0, 2)) >
            std::fabs(a.camRelVP[i].at(0, 0)) + std::fabs(a.camRelVP[i].at(0, 1)) + std::fabs(a.camRelVP[i].at(0, 2)));
}

TEST_CASE("ShadowXp: per-cascade caster test is conservative", "[Graphics][ShadowXp]")
{
    // An ortho light VP covering x, y in [-10, 10] (scale 0.1), z untouched.
    float vp[16] = {0.1f, 0, 0, 0, 0, 0.1f, 0, 0, 0, 0, 0.01f, 0, 0, 0, 0.5f, 1};
    const float inside[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    const float straddle[4] = {10.5f, 0.0f, 0.0f, 1.0f};
    const float outside[4] = {11.5f, 0.0f, 0.0f, 1.0f};
    const float outsideY[4] = {0.0f, -12.0f, 0.0f, 1.5f};
    const float farZ[4] = {0.0f, 0.0f, 1.0e5f, 1.0f}; // z never culls
    CHECK(xp::SphereInCascade(vp, inside));
    CHECK(xp::SphereInCascade(vp, straddle));
    CHECK_FALSE(xp::SphereInCascade(vp, outside));
    CHECK_FALSE(xp::SphereInCascade(vp, outsideY));
    CHECK(xp::SphereInCascade(vp, farZ));
}

TEST_CASE("ShadowXp: culled cascades rasterise the same depth as the full set", "[Graphics][ShadowXp]")
{
    // A tip-like tiered layout around a camera at the origin, and a few hundred
    // small casters scattered over 400 m. Each cascade is rasterised by the CPU
    // oracle from every caster, then only from the casters whose bounding
    // sphere SphereInCascade keeps: the two depth maps must be equal texel for
    // texel (the culled draw changes nothing in the map).
    CascadeBuildParams p;
    p.forward = Normalize(Vec3{0.2f, -0.05f, -1.0f});
    p.right = Normalize(Cross(p.forward, Vec3{0.0f, 1.0f, 0.0f}));
    p.up = Cross(p.right, p.forward);
    p.tanHalfX = 0.7f;
    p.tanHalfY = 0.39f;
    p.nearD = 0.5f;
    p.farD = 400.0f;
    p.sunDir = {0.35f, -0.6f, 0.25f};
    p.distanceCoef = 1.0f;
    p.count = 4;
    p.omniCount = 2;
    p.omniCoef[0] = 0.08f;
    p.omniCoef[1] = 0.20f;
    p.resolution = 128;
    const CascadeSet cs = BuildShadowCascadesTiered(p);

    struct Caster
    {
        std::array<Tri, 4> tris;
        float sphere[4];
    };
    std::vector<Caster> casters;
    unsigned seed = 12345u;
    auto rnd = [&]()
    {
        seed = seed * 1664525u + 1013904223u;
        return static_cast<float>((seed >> 8) & 0xFFFF) / 65535.0f;
    };
    for (int i = 0; i < 300; i++)
    {
        const Vec3 c{(rnd() - 0.5f) * 800.0f, (rnd() - 0.5f) * 20.0f, (rnd() - 0.5f) * 800.0f};
        const float s = 0.5f + rnd() * 6.0f;
        Caster k;
        float mn[3] = {1e30f, 1e30f, 1e30f}, mx[3] = {-1e30f, -1e30f, -1e30f};
        for (Tri& t : k.tris)
        {
            Vec3* v[3] = {&t.a, &t.b, &t.c};
            for (Vec3* q : v)
            {
                *q = c + Vec3{(rnd() - 0.5f) * s, rnd() * s * 2.0f, (rnd() - 0.5f) * s};
                const float xyz[3] = {q->x, q->y, q->z};
                for (int a = 0; a < 3; a++)
                {
                    mn[a] = std::min(mn[a], xyz[a]);
                    mx[a] = std::max(mx[a], xyz[a]);
                }
            }
        }
        const float hx = 0.5f * (mx[0] - mn[0]), hy = 0.5f * (mx[1] - mn[1]), hz = 0.5f * (mx[2] - mn[2]);
        k.sphere[0] = mn[0] + hx;
        k.sphere[1] = mn[1] + hy;
        k.sphere[2] = mn[2] + hz;
        k.sphere[3] = std::sqrt(hx * hx + hy * hy + hz * hz) * 1.001f + 0.01f;
        casters.push_back(k);
    }
    int culledSomewhere = 0;
    for (int c = 0; c < cs.count; c++)
    {
        std::vector<Tri> all, kept;
        for (const Caster& k : casters)
        {
            all.insert(all.end(), k.tris.begin(), k.tris.end());
            if (xp::SphereInCascade(cs.camRelVP[c].m.data(), k.sphere))
                kept.insert(kept.end(), k.tris.begin(), k.tris.end());
        }
        culledSomewhere += static_cast<int>(all.size() - kept.size());
        const DepthMap a = CpuRasterDepth(all.data(), static_cast<int>(all.size()), cs.camRelVP[c], p.resolution);
        const DepthMap b = CpuRasterDepth(kept.data(), static_cast<int>(kept.size()), cs.camRelVP[c], p.resolution);
        REQUIRE(a.depth.size() == b.depth.size());
        int diff = 0;
        for (size_t i = 0; i < a.depth.size(); i++)
            diff += a.depth[i] != b.depth[i] ? 1 : 0;
        CHECK(diff == 0);
    }
    // The near tiers drop most casters, so the cull really ran.
    CHECK(culledSomewhere > 300);
}
