#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Core/GpuTimings.hpp>

#include <cstdint>

// GpuTimings turns a frame's GPU timestamps into per-section durations
// (EngineGL33_GpuTimer.cpp feeds it the query results).

using namespace Poseidon::gpu;

namespace
{
struct Frame
{
    std::uint64_t stamps[kMarkCount] = {};
    std::uint32_t recorded = 0;

    void Set(Mark m, double ms)
    {
        stamps[static_cast<int>(m)] = static_cast<std::uint64_t>(static_cast<std::int64_t>(1e9 + ms * 1e6));
        recorded |= 1u << static_cast<int>(m);
    }
};

Frame FullFrame()
{
    Frame f;
    f.Set(Mark::FrameBegin, 0.0);
    f.Set(Mark::WorldBegin, 0.2);
    f.Set(Mark::ShadowBegin, 1.0);
    f.Set(Mark::ShadowEnd, 1.6);
    f.Set(Mark::WorldEnd, 5.0);
    f.Set(Mark::AoEnd, 5.3);
    f.Set(Mark::BloomEnd, 5.5);
    f.Set(Mark::ShaftsEnd, 5.55);
    f.Set(Mark::CompositeEnd, 5.7);
    f.Set(Mark::FrameEnd, 6.0);
    return f;
}
} // namespace

TEST_CASE("GpuTimings: sections from one full frame", "[Graphics][gpu]")
{
    GpuTimings t;
    float ms = 0.0f;
    CHECK_FALSE(t.AverageMs(Section::Frame, ms));
    const Frame f = FullFrame();
    t.Submit(f.stamps, f.recorded);
    CHECK(t.Frames() == 1);
    CHECK(t.LastMs(Section::Frame) == Catch::Approx(6.0).margin(1e-3));
    CHECK(t.LastMs(Section::World) == Catch::Approx(4.8).margin(1e-3));
    CHECK(t.LastMs(Section::Shadow) == Catch::Approx(0.6).margin(1e-3));
    CHECK(t.LastMs(Section::Ao) == Catch::Approx(0.3).margin(1e-3));
    CHECK(t.LastMs(Section::Bloom) == Catch::Approx(0.2).margin(1e-3));
    CHECK(t.LastMs(Section::Shafts) == Catch::Approx(0.05).margin(1e-3));
    CHECK(t.LastMs(Section::Composite) == Catch::Approx(0.15).margin(1e-3));
    CHECK(t.LastMs(Section::Post) == Catch::Approx(0.7).margin(1e-3));
    REQUIRE(t.AverageMs(Section::Frame, ms));
    CHECK(ms == Catch::Approx(6.0).margin(1e-3));
    CHECK(t.Summary().rfind("frame 6.00 ms", 0) == 0);
}

TEST_CASE("GpuTimings: a skipped post step folds into the next one", "[Graphics][gpu]")
{
    Frame f = FullFrame();
    f.recorded &= ~(1u << static_cast<int>(Mark::AoEnd));     // AO off
    f.recorded &= ~(1u << static_cast<int>(Mark::ShaftsEnd)); // sun behind the camera
    GpuTimings t;
    t.Submit(f.stamps, f.recorded);
    CHECK(t.LastMs(Section::Ao) < 0.0f);
    CHECK(t.LastMs(Section::Bloom) == Catch::Approx(0.5).margin(1e-3)); // from WorldEnd
    CHECK(t.LastMs(Section::Shafts) < 0.0f);
    CHECK(t.LastMs(Section::Composite) == Catch::Approx(0.2).margin(1e-3)); // from BloomEnd
    CHECK(t.LastMs(Section::Post) == Catch::Approx(0.7).margin(1e-3));
}

TEST_CASE("GpuTimings: missing or out-of-order stamps are dropped, averages smooth", "[Graphics][gpu]")
{
    GpuTimings t;
    Frame classic; // no post pass, no shadow maps
    classic.Set(Mark::FrameBegin, 0.0);
    classic.Set(Mark::WorldBegin, 0.1);
    classic.Set(Mark::WorldEnd, 3.0);
    classic.Set(Mark::FrameEnd, 4.0);
    t.Submit(classic.stamps, classic.recorded);
    float ms = 0.0f;
    CHECK_FALSE(t.AverageMs(Section::Shadow, ms));
    CHECK_FALSE(t.AverageMs(Section::Post, ms));
    CHECK(t.LastMs(Section::World) == Catch::Approx(2.9).margin(1e-3));

    Frame bad = classic;
    bad.Set(Mark::FrameEnd, -1.0); // earlier than FrameBegin: a stale result
    t.Submit(bad.stamps, bad.recorded);
    CHECK(t.LastMs(Section::Frame) < 0.0f);
    REQUIRE(t.AverageMs(Section::Frame, ms));
    CHECK(ms == Catch::Approx(4.0).margin(1e-3)); // the bad frame did not move it

    Frame slow = classic;
    slow.Set(Mark::FrameEnd, 14.0);
    t.Submit(slow.stamps, slow.recorded);
    REQUIRE(t.AverageMs(Section::Frame, ms));
    CHECK(ms > 4.0f);
    CHECK(ms < 14.0f); // smoothed, not the last frame

    t.Reset();
    CHECK(t.Frames() == 0);
    CHECK_FALSE(t.AverageMs(Section::Frame, ms));
}
