#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Core/GpuTimings.hpp>

#include <cstdint>
#include <set>
#include <string>

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
    void Drop(Mark m) { recorded &= ~(1u << static_cast<int>(m)); }
};

// A frame of today's post pass: resolve, AO, bloom, shafts, composite.
Frame FullFrame()
{
    Frame f;
    f.Set(Mark::FrameBegin, 0.0);
    f.Set(Mark::WorldBegin, 0.2);
    f.Set(Mark::ShadowBegin, 1.0);
    f.Set(Mark::ShadowEnd, 1.6);
    f.Set(Mark::WorldEnd, 5.0);
    f.Set(Mark::ResolveEnd, 5.05);
    f.Set(Mark::AoEnd, 5.3);
    f.Set(Mark::BloomEnd, 5.5);
    f.Set(Mark::ShaftsEnd, 5.55);
    f.Set(Mark::CompositeEnd, 5.7);
    f.Set(Mark::PostEnd, 5.7);
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
    CHECK(t.LastMs(Section::Resolve) == Catch::Approx(0.05).margin(1e-3));
    CHECK(t.LastMs(Section::Ao) == Catch::Approx(0.25).margin(1e-3)); // from the resolve, not WorldEnd
    CHECK(t.LastMs(Section::Bloom) == Catch::Approx(0.2).margin(1e-3));
    CHECK(t.LastMs(Section::Shafts) == Catch::Approx(0.05).margin(1e-3));
    CHECK(t.LastMs(Section::Composite) == Catch::Approx(0.15).margin(1e-3));
    CHECK(t.LastMs(Section::Post) == Catch::Approx(0.7).margin(1e-3));
    CHECK(t.LastMs(Section::LinearDepth) < 0.0f); // did not run
    CHECK(t.LastMs(Section::DebugView) < 0.0f);
    REQUIRE(t.AverageMs(Section::Frame, ms));
    CHECK(ms == Catch::Approx(6.0).margin(1e-3));
    CHECK(t.Summary() == "frame 6.00 ms | world 4.80 (shadow 0.60) | post 0.70: resolve 0.05 ao 0.25 bloom 0.20 "
                         "shafts 0.05 composite 0.15 | min frame=6.00 world=4.80");
}

TEST_CASE("GpuTimings: a skipped post step folds into the next one", "[Graphics][gpu]")
{
    Frame f = FullFrame();
    f.Drop(Mark::AoEnd);     // AO off
    f.Drop(Mark::ShaftsEnd); // sun behind the camera
    GpuTimings t;
    t.Submit(f.stamps, f.recorded);
    CHECK(t.LastMs(Section::Ao) < 0.0f);
    CHECK(t.LastMs(Section::Bloom) == Catch::Approx(0.45).margin(1e-3)); // from ResolveEnd
    CHECK(t.LastMs(Section::Shafts) < 0.0f);
    CHECK(t.LastMs(Section::Composite) == Catch::Approx(0.2).margin(1e-3)); // from BloomEnd
    CHECK(t.LastMs(Section::Post) == Catch::Approx(0.7).margin(1e-3));
}

TEST_CASE("GpuTimings: every post stage has its mark and section, in run order", "[Graphics][gpu][PostStages]")
{
    // The stage marks are contiguous after WorldEnd and the stage sections
    // follow them one for one, so each section runs from the stage before it.
    CHECK(kMarkCount <= 32);
    CHECK(static_cast<int>(Mark::ResolveEnd) == static_cast<int>(Mark::WorldEnd) + 1);
    CHECK(static_cast<int>(Mark::PostEnd) == static_cast<int>(Mark::DebugViewEnd) + 1);
    std::set<std::string> names;
    for (int i = 0; i < kSectionCount; ++i)
        names.insert(SectionName(static_cast<Section>(i)));
    CHECK(names.size() == static_cast<size_t>(kSectionCount)); // unique: GpuTimingSectionMs looks them up by name
    for (int i = 0; i < kPostStageMarks; ++i)
    {
        const auto m = static_cast<Mark>(static_cast<int>(Mark::ResolveEnd) + i);
        CAPTURE(i);
        CHECK(PostStageSection(m) == static_cast<Section>(static_cast<int>(Section::Resolve) + i));
    }
    CHECK(PostStageSection(Mark::WorldEnd) == Section::Count);
    CHECK(PostStageSection(Mark::PostEnd) == Section::Count);

    // Every stage runs: each section measures its own stage, and together they
    // make the post pass.
    Frame f;
    f.Set(Mark::FrameBegin, 0.0);
    f.Set(Mark::WorldBegin, 0.1);
    f.Set(Mark::WorldEnd, 4.0);
    double t0 = 4.0;
    for (int i = 0; i < kPostStageMarks; ++i)
    {
        t0 += 0.01 * (i + 1);
        f.Set(static_cast<Mark>(static_cast<int>(Mark::ResolveEnd) + i), t0);
    }
    f.Set(Mark::PostEnd, t0);
    f.Set(Mark::FrameEnd, t0 + 0.5);
    GpuTimings t;
    t.Submit(f.stamps, f.recorded);
    double sum = 0.0;
    for (int i = 0; i < kPostStageMarks; ++i)
    {
        const auto s = static_cast<Section>(static_cast<int>(Section::Resolve) + i);
        CAPTURE(SectionName(s));
        CHECK(t.LastMs(s) == Catch::Approx(0.01 * (i + 1)).margin(1e-4));
        sum += t.LastMs(s);
    }
    CHECK(t.LastMs(Section::Post) == Catch::Approx(sum).margin(1e-3));
    // The summary lists them in run order.
    const std::string summary = t.Summary();
    size_t at = 0;
    for (int i = static_cast<int>(Section::Resolve); i <= static_cast<int>(Section::DebugView); ++i)
    {
        const std::string name = std::string(" ") + SectionName(static_cast<Section>(i)) + " ";
        CAPTURE(name, summary);
        const size_t found = summary.find(name, at);
        REQUIRE(found != std::string::npos);
        at = found;
    }
}

TEST_CASE("GpuTimings: a stage after the composite counts in the post pass", "[Graphics][gpu][PostStages]")
{
    // The post section ends at PostEnd, after whichever stage ran last (the
    // debug view here; the TAA resolve and sharpen once they land).
    Frame f = FullFrame();
    f.Set(Mark::LinearDepthEnd, 5.1);
    f.Set(Mark::DebugViewEnd, 5.9);
    f.Set(Mark::PostEnd, 5.9);
    GpuTimings t;
    t.Submit(f.stamps, f.recorded);
    CHECK(t.LastMs(Section::LinearDepth) == Catch::Approx(0.05).margin(1e-3));
    CHECK(t.LastMs(Section::Ao) == Catch::Approx(0.2).margin(1e-3)); // from LinearDepthEnd
    CHECK(t.LastMs(Section::DebugView) == Catch::Approx(0.2).margin(1e-3));
    CHECK(t.LastMs(Section::Post) == Catch::Approx(0.9).margin(1e-3));
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
    CHECK_FALSE(t.AverageMs(Section::Resolve, ms));
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
    CHECK(t.LastMs(Section::Frame) < 0.0f);
}

TEST_CASE("GpuTimings: the world pass splits into ground, opaque objects and the rest", "[Graphics][gpu]")
{
    // A4: Landscape::DrawRect marks the end of the ground (sky, terrain, water,
    // horizon) and of the opaque objects (Pass1); the rest is the shadow-map
    // pass, the alpha objects and what follows.
    Frame f = FullFrame();
    f.Set(Mark::GroundEnd, 0.9);
    f.Set(Mark::Pass1End, 3.1);
    GpuTimings t;
    t.Submit(f.stamps, f.recorded);
    CHECK(t.LastMs(Section::Ground) == Catch::Approx(0.7).margin(1e-3));
    CHECK(t.LastMs(Section::Pass1) == Catch::Approx(2.2).margin(1e-3));
    CHECK(t.LastMs(Section::Rest) == Catch::Approx(1.9).margin(1e-3));
    CHECK(t.LastMs(Section::Ground) + t.LastMs(Section::Pass1) + t.LastMs(Section::Rest) ==
          Catch::Approx(t.LastMs(Section::World)).margin(1e-3));
    const std::string s = t.Summary();
    INFO(s);
    CHECK(s.find("world 4.80 (shadow 0.60) [ground 0.70 pass1 2.20 rest 1.90] | post 0.70:") != std::string::npos);
    // make_pairs.py reads "<section> <ms>"; the minimum fields use '=' so
    // they never shadow the averages.
    CHECK(s.find(" | min frame=6.00 world=4.80 ground=0.70 pass1=2.20 rest=1.90") != std::string::npos);
    // The two marks are world marks: never post stage marks.
    CHECK(PostStageSection(Mark::GroundEnd) == Section::Count);
    CHECK(PostStageSection(Mark::Pass1End) == Section::Count);
}

TEST_CASE("GpuTimings: the minimum is the shortest frame since the reset", "[Graphics][gpu]")
{
    GpuTimings t;
    float ms = 0.0f;
    CHECK_FALSE(t.MinMs(Section::Frame, ms));
    for (double frame : {9.0, 6.0, 12.0, 7.0})
    {
        Frame f;
        f.Set(Mark::FrameBegin, 0.0);
        f.Set(Mark::FrameEnd, frame);
        t.Submit(f.stamps, f.recorded);
    }
    REQUIRE(t.MinMs(Section::Frame, ms));
    CHECK(ms == Catch::Approx(6.0).margin(1e-3));
    REQUIRE(t.AverageMs(Section::Frame, ms));
    CHECK(ms > 6.5f); // the average is not the minimum
    t.Reset();
    CHECK_FALSE(t.MinMs(Section::Frame, ms));
}
