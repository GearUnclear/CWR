#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Core/PerfCounters.hpp>

#include <string>

// The frame-cost counters triPerfStats prints: the scene and the backend add to
// the frame being drawn, World::Simulate closes it, and the summary averages
// over the profiler's window.

using Poseidon::perf::Counters;

TEST_CASE("PerfCounters: frames close into per-frame averages over the window", "[gpu][PerfCounters]")
{
    Counters c;
    c.Frame().runs = 10;
    c.Frame().runObjects = 400;
    c.Frame().hillTested = 7000;
    c.Frame().hillHidden = 6000;
    c.Frame().hillMs = 0.5f;
    c.Frame().litBytes = 2048;
    c.EndFrame();
    CHECK(c.Last().runs == 10);
    CHECK(c.Frame().runs == 0); // a new frame starts empty
    c.Frame().runs = 20;
    c.Frame().runObjects = 600;
    c.Frame().hillTested = 5000;
    c.Frame().hillHidden = 4000;
    c.Frame().hillMs = 1.5f;
    c.Frame().litBytes = 4096;
    c.Frame().tileLights = 9;
    c.Frame().tileRefs = 3000;
    c.Frame().tileMs = 0.2f;
    c.EndFrame();
    CHECK(c.WindowFrames() == 2);
    const std::string s = c.Summary();
    INFO(s);
    CHECK(s.find("runs=15 runobj=500") != std::string::npos);
    CHECK(s.find("tested=6000 hidden=5000 builds=") != std::string::npos);
    CHECK(s.find("hillcpu=1.000") != std::string::npos);
    CHECK(s.find("litKB=3.0") != std::string::npos);
    // The tiled light list: averaged over every frame of the window.
    CHECK(s.find("| tiles lights=4.5 refs=1500 tilecpu=0.100") != std::string::npos);
    // It rides in UDCAP lines after triPerfStats' own fields: no ';', and none
    // of the names the capture parser reads there (fps=, frame=, calls=).
    CHECK(s.find(';') == std::string::npos);
    CHECK(s.find("fps=") == std::string::npos);
    CHECK(s.find("frame=") == std::string::npos);
    CHECK(s.find("calls=") == std::string::npos);

    c.ResetWindow();
    CHECK(c.WindowFrames() == 0);
    CHECK(c.Summary().find("runs=0 ") != std::string::npos);
    CHECK(c.Last().runs == 20); // the last frame stays readable
}
