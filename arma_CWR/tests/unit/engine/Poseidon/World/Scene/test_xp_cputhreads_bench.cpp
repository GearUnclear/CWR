#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Core/TaskPool.hpp>
#include <Poseidon/World/Scene/DrawListSort.hpp>
#include <Poseidon/World/Scene/TerrainOcclusion.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>
#include <random>
#include <thread>
#include <vector>

// XP-cputhreads offline micro-benchmark (hidden: run with "[.xpbench]"). Times
// the serial and pool versions of the draw-list sort and the horizon tests on
// lists shaped like the valley's (8,000 entries spread over the heap), with an
// idle gap before each repetition so the pool's workers have gone to sleep, as
// they have between two stages of a real frame. Prints medians; asserts nothing
// about speed.

using Poseidon::Foundation::Ref;
using Poseidon::Foundation::RefCount;
namespace dls = Poseidon::DrawListSort;
using namespace std::chrono;

namespace
{
struct BenchShape
{
    int faces;
};

struct BenchEntry : RefCount
{
    char pad[200]; // SortObjects and their objects are spread over the heap
    signed char passNum = 0;
    signed char drawLOD = 0;
    BenchShape* shape = nullptr;
    float distance2 = 0.0f;
};

double Median(std::vector<double> v)
{
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}

double P90(std::vector<double> v)
{
    std::sort(v.begin(), v.end());
    return v[v.size() * 9 / 10];
}

template <class F>
std::vector<double> TimeReps(int reps, int gapUs, F&& f)
{
    std::vector<double> ms;
    for (int r = 0; r < reps; r++)
    {
        if (gapUs > 0)
        {
            std::this_thread::sleep_for(microseconds(gapUs));
        }
        const auto t0 = steady_clock::now();
        f();
        ms.push_back(duration<double, std::milli>(steady_clock::now() - t0).count());
    }
    return ms;
}
} // namespace

TEST_CASE("XP-cputhreads bench: pool dispatch, draw-list sorts, horizon tests", "[.xpbench]")
{
    Poseidon::TaskPool pool(8);
    const int n = 8000;
    const int reps = 150;
    std::mt19937 rng(42);
    std::vector<BenchShape> shapes(300);
    for (auto& s : shapes)
    {
        s.faces = static_cast<int>(rng() % 2000);
    }
    std::vector<std::unique_ptr<BenchEntry[]>> blocks;
    std::vector<Ref<BenchEntry>> list;
    for (int i = 0; i < n; i++)
    {
        BenchEntry* e = new BenchEntry;
        e->passNum = static_cast<signed char>(rng() % 2);
        e->drawLOD = static_cast<signed char>(rng() % 4);
        e->shape = &shapes[rng() % shapes.size()];
        e->distance2 = static_cast<float>(rng() % 16000000);
        list.emplace_back(e);
    }
    std::shuffle(list.begin(), list.end(), rng);

    for (int gap : {0, 2000})
    {
        auto d = TimeReps(2000, gap > 0 ? 500 : 0, [&] { pool.ParallelFor(8, [](uint32_t, uint32_t) {}); });
        std::printf("XPBENCH dispatch gap=%dus median=%.4f p90=%.4f ms\n", gap > 0 ? 500 : 0, Median(d), P90(d));
    }

    auto distKey = [](const BenchEntry* o) { return o->distance2; };
    auto distCmp = [](float a, float b) { return dls::CompareRevDistance(a, b); };
    auto shapeKey = [](const BenchEntry* o)
    { return dls::ShapeKey{{o->passNum, o->shape, o->drawLOD, o->distance2}, o->shape->faces}; };
    auto shapeCmp = [](const dls::ShapeKey& a, const dls::ShapeKey& b) { return dls::CompareShapeKeys(a, b); };

    for (int gapUs : {0, 1000})
    {
        dls::Scratch<float, BenchEntry> s1;
        dls::ParallelSortScratch<float, BenchEntry> p1;
        dls::Scratch<dls::ShapeKey, BenchEntry> s2;
        dls::ParallelSortScratch<dls::ShapeKey, BenchEntry> p2;
        std::vector<Ref<BenchEntry>> work;
        auto reset = [&] { work = list; };
        std::vector<double> a, b, c, e;
        for (int r = 0; r < reps; r++)
        {
            reset();
            a.push_back(TimeReps(1, gapUs, [&] { dls::SortByKeys(work.data(), n, distKey, distCmp, s1); })[0]);
            reset();
            b.push_back(
                TimeReps(1, gapUs, [&] { dls::SortByKeysParallel(work.data(), n, distKey, distCmp, p1, pool); })[0]);
            reset();
            c.push_back(TimeReps(1, gapUs, [&] { dls::SortByKeys(work.data(), n, shapeKey, shapeCmp, s2); })[0]);
            reset();
            e.push_back(
                TimeReps(1, gapUs, [&] { dls::SortByKeysParallel(work.data(), n, shapeKey, shapeCmp, p2, pool); })[0]);
        }
        std::printf("XPBENCH sort n=%d gap=%dus dist serial=%.4f parallel=%.4f | shape serial=%.4f parallel=%.4f ms "
                    "(median)\n",
                    n, gapUs, Median(a), Median(b), Median(c), Median(e));
        // Pass2 re-sorts a list that Pass1 already shape-sorted: nearly sorted input.
        std::vector<double> f, g;
        for (int r = 0; r < reps; r++)
        {
            reset();
            dls::SortByKeys(work.data(), n, shapeKey, shapeCmp, s2);
            std::vector<Ref<BenchEntry>> sorted = work;
            f.push_back(TimeReps(1, gapUs, [&] { dls::SortByKeys(work.data(), n, distKey, distCmp, s1); })[0]);
            work = sorted;
            g.push_back(
                TimeReps(1, gapUs, [&] { dls::SortByKeysParallel(work.data(), n, distKey, distCmp, p1, pool); })[0]);
        }
        std::printf("XPBENCH resort-after-shape n=%d gap=%dus serial=%.4f parallel=%.4f ms\n", n, gapUs, Median(f),
                    Median(g));
    }

    // The shape sort in parts: key reads, quicksort, on the pool or not.
    {
        using E = dls::Entry<dls::ShapeKey>;
        std::vector<E> keys(n), work(n);
        auto readKeys = [&](uint32_t s, uint32_t e)
        {
            for (uint32_t i = s; i < e; ++i)
            {
                keys[i].key = shapeKey(list[i].GetRef());
                keys[i].index = static_cast<int>(i);
            }
        };
        auto cmp = [](const E* a, const E* b, const decltype(shapeCmp)* c) { return (*c)(a->key, b->key); };
        dls::ParallelScratch ps;
        std::vector<double> k1, k2, q1, q2, q3, q4;
        for (int r = 0; r < reps; r++)
        {
            k1.push_back(TimeReps(1, 1000, [&] { readKeys(0, n); })[0]);
            k2.push_back(TimeReps(1, 1000, [&] { pool.ParallelFor(n, readKeys); })[0]);
            work = keys;
            q1.push_back(
                TimeReps(1, 1000, [&] { Poseidon::Foundation::QSortWithContext(work.data(), n, &shapeCmp, cmp); })[0]);
            work = keys;
            q2.push_back(
                TimeReps(1, 1000, [&] { dls::ParallelQSortWithContext(work.data(), n, &shapeCmp, cmp, pool, ps); })[0]);
            work = keys;
            q3.push_back(TimeReps(
                1, 1000, [&] { dls::ParallelQSortWithContext(work.data(), n, &shapeCmp, cmp, pool, ps, 4, 256); })[0]);
            work = keys;
            q4.push_back(TimeReps(
                1, 1000, [&] { dls::ParallelQSortWithContext(work.data(), n, &shapeCmp, cmp, pool, ps, 2, 256); })[0]);
        }
        std::printf("XPBENCH shape parts: keys serial=%.4f pool=%.4f | qsort serial=%.4f pool(10,256)=%.4f "
                    "pool(4,256)=%.4f pool(2,256)=%.4f ms\n",
                    Median(k1), Median(k2), Median(q1), Median(q2), Median(q3), Median(q4));
        // leaf balance at 4 levels
        work = keys;
        dls::ParallelQSortWithContext(work.data(), n, &shapeCmp, cmp, pool, ps);
        std::printf("XPBENCH shape leaves:");
        for (const auto& r : ps.cur)
            std::printf(" %d", r.hi - r.lo + 1);
        std::printf("\n");
    }

    // Horizon tests: a 512 x 512 grid of rolling hills, the eye 2 m up, 8,000
    // objects out to 2.5 km; wedges built (static camera) and rebuilt every
    // repetition (moving camera).
    using namespace Poseidon::TerrainOcclusion;
    const int gs = 512;
    std::vector<float> v(static_cast<size_t>(gs) * gs);
    for (int z = 0; z < gs; z++)
        for (int x = 0; x < gs; x++)
            v[static_cast<size_t>(z) * gs + x] =
                40.0f * std::sin(x * 0.05f) * std::cos(z * 0.043f) + 15.0f * std::sin((x + z) * 0.11f);
    HeightGrid grid;
    grid.BuildFromVertices(gs, 12.5f, -100.0f, v);
    struct Obj
    {
        float x, y, z, r;
    };
    std::vector<Obj> objs;
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    const float ex = 3200.0f, ez = 3200.0f, ey = grid.SurfaceY(ex, ez) + 2.0f;
    // In view only: a 90 degree cone, as the draw list holds (plus a few behind for shadows).
    for (int i = 0; i < n; i++)
    {
        const float ang = (unit(rng) - 0.5f) * 1.5707963f, dist = 30.0f + unit(rng) * 2400.0f;
        const float px = ex + std::sin(ang) * dist, pz = ez + std::cos(ang) * dist;
        objs.push_back({px, grid.SurfaceY(px, pz) + 3.0f, pz, 4.0f + unit(rng) * 6.0f});
    }
    std::vector<Result> res(objs.size());
    for (bool rebuild : {false, true})
    {
        HorizonTable ts, tc;
        ts.Build(grid, ex, ey, ez, 2500.0f, Params());
        tc.Build(grid, ex, ey, ez, 2500.0f, Params());
        std::vector<double> a, b;
        for (int r = 0; r < 60; r++)
        {
            if (rebuild)
            {
                ts.Build(grid, ex, ey + 0.001f * (r + 1), ez, 2500.0f, Params());
                tc.Build(grid, ex, ey + 0.001f * (r + 1), ez, 2500.0f, Params());
            }
            a.push_back(TimeReps(1, 1000,
                                 [&]
                                 {
                                     for (size_t i = 0; i < objs.size(); i++)
                                         res[i] = ts.Test(objs[i].x, objs[i].y, objs[i].z, objs[i].r);
                                 })[0]);
            b.push_back(TimeReps(1, 1000,
                                 [&]
                                 {
                                     tc.BeginConcurrent();
                                     pool.ParallelFor(static_cast<uint32_t>(objs.size()),
                                                      [&](uint32_t s, uint32_t e)
                                                      {
                                                          for (uint32_t i = s; i < e; i++)
                                                              res[i] = tc.TestConcurrent(objs[i].x, objs[i].y,
                                                                                         objs[i].z, objs[i].r);
                                                      });
                                     tc.EndConcurrent();
                                 })[0]);
        }
        std::printf("XPBENCH hill n=%d %s serial=%.4f concurrent=%.4f ms (median), wedges %d\n", n,
                    rebuild ? "rebuilt-each-frame" : "static", Median(a), Median(b), ts.WedgesBuilt());
    }
    SUCCEED();
}
