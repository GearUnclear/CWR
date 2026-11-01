#include <catch2/catch_test_macros.hpp>
#include <Evaluator/express.hpp>
#include <Poseidon/Core/BuildProvenance.hpp>
#include <Poseidon/Dev/Diag/FrameProfiler.hpp>
#include <Poseidon/Graphics/Core/GpuTimings.hpp>
#include <Poseidon/Graphics/Core/PerfCounters.hpp>
#include <Poseidon/Graphics/Dummy/EngineDummy.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// The measurement verbs of the perf bench (tests/rendering/captures/gate) and
// the bookkeeping behind them: the CPU and GPU per-frame rings and their CSV
// files, the GPU timer's window restart, the counters' window.

using namespace Poseidon;

GameValue TriPerfRing(const GameState*, GameValuePar);
GameValue TriGpuRing(const GameState*, GameValuePar);
GameValue TriGpuTimes(const GameState*, GameValuePar);
GameValue TriBuildId(const GameState*);

namespace
{

struct InitFixture
{
    InitFixture()
    {
        static bool done = false;
        if (!done)
        {
            GGameState.Init();
            done = true;
        }
    }
};

struct GEngineGuard
{
    Engine* prev;
    explicit GEngineGuard(Engine* tmp) : prev(GEngine) { GEngine = tmp; }
    ~GEngineGuard() { GEngine = prev; }
};

std::string Text(const GameValue& v)
{
    return std::string(((RString)(GameStringType)v).Data());
}

GameValue Pair(float n, const char* label)
{
    GameValue arr = GGameState.CreateGameValue(GameArray);
    GameArrayType& a = arr;
    a.Resize(2);
    a[0] = GameValue(n);
    a[1] = GameValue(RString(label));
    return arr;
}

void SetEnv(const char* name, const std::string& value)
{
#ifdef _WIN32
    _putenv_s(name, value.c_str());
#else
    setenv(name, value.c_str(), 1);
#endif
}

// Points TRI_OUTPUT_DIR at a fresh folder for one case and puts it back after.
struct OutputDirGuard
{
    std::string dir;
    std::string prev;
    bool hadPrev = false;
    explicit OutputDirGuard(const char* name)
    {
        if (const char* p = std::getenv("TRI_OUTPUT_DIR"))
        {
            prev = p;
            hadPrev = true;
        }
        dir = (std::filesystem::temp_directory_path() / "ud-tri-perf-verbs" / name).string();
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
        SetEnv("TRI_OUTPUT_DIR", dir);
    }
    ~OutputDirGuard()
    {
        SetEnv("TRI_OUTPUT_DIR", hadPrev ? prev : std::string());
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }
};

std::vector<std::string> Lines(const std::string& text)
{
    std::vector<std::string> out;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line))
        out.push_back(line);
    return out;
}

std::string ReadFile(const std::string& path)
{
    std::ifstream f(path, std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string LastField(const std::string& line)
{
    return line.substr(line.rfind(',') + 1);
}

// One frame of GPU stamps (ms from a common origin) for GpuTimings::Submit.
struct GpuFrame
{
    std::uint64_t stamps[gpu::kMarkCount] = {};
    std::uint32_t recorded = 0;
    void Set(gpu::Mark m, double ms)
    {
        stamps[static_cast<int>(m)] = static_cast<std::uint64_t>(1e9 + ms * 1e6);
        recorded |= 1u << static_cast<int>(m);
    }
};

GpuFrame WorldFrame(double scale)
{
    GpuFrame f;
    f.Set(gpu::Mark::FrameBegin, 0.0);
    f.Set(gpu::Mark::WorldBegin, 0.5 * scale);
    f.Set(gpu::Mark::GroundEnd, 1.5 * scale);
    f.Set(gpu::Mark::Pass1End, 4.0 * scale);
    f.Set(gpu::Mark::WorldEnd, 5.0 * scale);
    f.Set(gpu::Mark::PostEnd, 6.0 * scale);
    f.Set(gpu::Mark::FrameEnd, 7.0 * scale);
    return f;
}

// Engine stand-in for the GPU timer: counts the calls the verb makes and
// answers a summary that names how many windows were started.
class TimingEngine : public EngineDummy
{
  public:
    bool on = false;
    int windows = 0;
    std::vector<bool> calls;
    void SetGpuTimingEnabled(bool enabled) override
    {
        calls.push_back(enabled);
        if (enabled)
            ++windows; // turning on and restarting both start a window
        on = enabled;
    }
    bool GpuTimingEnabled() const override { return on; }
    std::string GpuTimingSummary() const override
    {
        return on ? "window " + std::to_string(windows) : "gpu: timing off";
    }
};

} // namespace

TEST_CASE("FrameProfiler ring: log and CSV list the last frames oldest first", "[tri][perf][triPerfRing]")
{
    Dev::FrameProfiler p;
    for (int draws : {10, 20, 30})
    {
        p.BeginFrame();
        p.Mark(Dev::FrameProfiler::PhaseSetup);
        p.EndFrame(draws);
    }
    const std::vector<std::string> all = Lines(p.RingCsv(0));
    REQUIRE(all.size() == 4);
    CHECK(all[0].rfind("i,total,setup,drw:init,", 0) == 0);
    CHECK(LastField(all[0]) == "draws");
    // i, total, one column per phase, draws.
    CHECK(std::count(all[0].begin(), all[0].end(), ',') == Dev::FrameProfiler::PhaseCount + 2);
    CHECK(all[1].rfind("0,", 0) == 0);
    CHECK(LastField(all[1]) == "10");
    CHECK(LastField(all[3]) == "30");

    const std::vector<std::string> last2 = Lines(p.RingCsv(2));
    REQUIRE(last2.size() == 3);
    CHECK(LastField(last2[1]) == "20");
    CHECK(LastField(last2[2]) == "30");

    const std::string log = p.RingLog(2);
    CHECK(std::count(log.begin(), log.end(), ';') == 2);
    CHECK(std::count(log.begin(), log.end(), ',') == 8); // five fields per frame
    CHECK(p.RingLog(99) == p.RingLog(0));
}

TEST_CASE("GpuFrameRing: one record per read-back frame, empty where unmeasured", "[tri][perf][triGpuRing]")
{
    gpu::GpuTimings t;
    gpu::GpuFrameRing ring;
    GpuFrame a = WorldFrame(1.0);
    t.Submit(a.stamps, a.recorded);
    ring.Push(t);
    GpuFrame b = WorldFrame(2.0);
    b.recorded &= ~(1u << static_cast<int>(gpu::Mark::PostEnd)); // no post pass this frame
    t.Submit(b.stamps, b.recorded);
    ring.Push(t);
    REQUIRE(ring.Count() == 2);

    const std::vector<std::string> csv = Lines(ring.CsvText(0));
    REQUIRE(csv.size() == 3);
    CHECK(csv[0] == "i,frame,world,pass1,ground,rest,post");
    // frame, world, pass1, ground, rest, post: the raw last-frame times, not the averages.
    CHECK(csv[1] == "0,7.000,4.500,2.500,1.000,1.000,1.000");
    CHECK(csv[2] == "1,14.000,9.000,5.000,2.000,2.000,");
    CHECK(ring.LogText(1) == "14.000,9.000,5.000,2.000,2.000,;");

    for (int i = 0; i < gpu::GpuFrameRing::kSize + 10; ++i)
        ring.Push(t);
    CHECK(ring.Count() == gpu::GpuFrameRing::kSize);
    ring.Reset();
    CHECK(ring.Count() == 0);
    CHECK(Lines(ring.CsvText(0)).size() == 1);
}

TEST_CASE("PerfCounters window: the last 256 frames, the profiler's ring", "[tri][perf][PerfCounters]")
{
    perf::Counters c;
    for (int i = 0; i < 44; ++i)
    {
        c.Frame().runs = 1;
        c.EndFrame();
    }
    for (int i = 0; i < perf::Counters::kWindow; ++i)
    {
        c.Frame().runs = 3;
        c.EndFrame();
    }
    CHECK(c.WindowFrames() == Dev::FrameProfiler::kRingSize);
    CHECK(c.WindowFrame(0).runs == 3);
    CHECK(c.Summary().find("runs=3 ") != std::string::npos);
    c.ResetWindow();
    CHECK(c.WindowFrames() == 0);
    c.Frame().runs = 5;
    c.EndFrame();
    CHECK(c.Summary().find("runs=5 ") != std::string::npos);
}

TEST_CASE("triPerfRing writes the CPU ring as CSV under TRI_OUTPUT_DIR", "[tri][perf][triPerfRing]")
{
    InitFixture fix;
    OutputDirGuard out("perfring");
    Dev::FrameProfiler& p = Dev::GFrameProfiler();
    p.Reset();
    CHECK(Text(TriPerfRing(nullptr, GameValue(0.0f))) == "FAIL:no_frames");
    for (int draws : {7, 8, 9})
    {
        p.BeginFrame();
        p.EndFrame(draws);
    }
    CHECK(Text(TriPerfRing(nullptr, Pair(2.0f, "noon fields"))) == "OK:2");
    const std::vector<std::string> csv = Lines(ReadFile(out.dir + "/perfring-noon_fields.csv"));
    REQUIRE(csv.size() == 3);
    CHECK(LastField(csv[1]) == "8");
    CHECK(LastField(csv[2]) == "9");
    // More than the ring holds: all of it; unlabelled files are numbered.
    CHECK(Text(TriPerfRing(nullptr, GameValue(500.0f))) == "OK:3");
    p.Reset();
}

TEST_CASE("triGpuRing writes the GPU ring as CSV under TRI_OUTPUT_DIR", "[tri][perf][triGpuRing]")
{
    InitFixture fix;
    OutputDirGuard out("gpuring");
    gpu::GpuFrameRing& ring = gpu::GGpuFrameRing();
    ring.Reset();
    CHECK(Text(TriGpuRing(nullptr, GameValue(0.0f))) == "FAIL:no_frames");
    gpu::GpuTimings t;
    GpuFrame f = WorldFrame(1.0);
    t.Submit(f.stamps, f.recorded);
    ring.Push(t);
    CHECK(Text(TriGpuRing(nullptr, Pair(0.0f, "a"))) == "OK:1");
    const std::vector<std::string> csv = Lines(ReadFile(out.dir + "/gpuring-a.csv"));
    REQUIRE(csv.size() == 2);
    CHECK(csv[1] == "0,7.000,4.500,2.500,1.000,1.000,1.000");
    ring.Reset();
}

TEST_CASE("triGpuTimes true returns the window so far, then starts a new one", "[tri][perf][triGpuTimes]")
{
    InitFixture fix;
    TimingEngine eng;
    GEngineGuard guard(&eng);
    // Off: turning on starts the first window.
    CHECK(Text(TriGpuTimes(nullptr, GameValue(true))) == "gpu: timing off");
    CHECK(eng.on);
    CHECK(eng.windows == 1);
    // Already on: the summary is the window's, read before the restart, and
    // the restart reaches the backend (it used to be skipped when already on).
    CHECK(Text(TriGpuTimes(nullptr, GameValue(true))) == "window 1");
    CHECK(eng.windows == 2);
    CHECK(Text(TriGpuTimes(nullptr, GameValue(true))) == "window 2");
    // The legacy reset still works: false then true.
    CHECK(Text(TriGpuTimes(nullptr, GameValue(false))) == "window 3");
    CHECK_FALSE(eng.on);
    CHECK(Text(TriGpuTimes(nullptr, GameValue(true))) == "gpu: timing off");
    CHECK(eng.windows == 4);
    CHECK(eng.calls == std::vector<bool>{true, true, true, false, true});
}

TEST_CASE("triBuildId answers the build provenance baked in at build time", "[tri][perf][triBuildId]")
{
    InitFixture fix;
    const std::string id = Text(TriBuildId(nullptr));
    CHECK(id == BuildProvenance::BuildId());
    REQUIRE_FALSE(id.empty());
    // It rides in UDCAP key=value lines: no separators or spaces inside.
    CHECK(id.find_first_of("; |=\t\r\n") == std::string::npos);
    // "+<12 hex>" marks local changes under engine/ or apps/.
    const size_t plus = id.find('+');
    if (plus != std::string::npos)
    {
        const std::string hash = id.substr(plus + 1);
        CHECK(hash.size() == 12);
        CHECK(hash.find_first_not_of("0123456789abcdef") == std::string::npos);
    }
}

TEST_CASE("capture_lib logs build= on every mode and shot line", "[tri][perf][triBuildId]")
{
    const auto root = std::filesystem::path(TESTS_ROOT_DIR).parent_path();
    const std::string lib = ReadFile((root / "tests" / "rendering" / "captures" / "capture_lib.sqf").string());
    REQUIRE_FALSE(lib.empty());
    CHECK(lib.find("capBuild = triBuildId;") != std::string::npos);
    CHECK(lib.find("event=mode;mode=%2;lit=%3;detail=%4;settle=%5;%6;build=%7") != std::string::npos);
    CHECK(lib.find("\"%1;build=%2;perf=%3;gpu=%4\", _head, capBuild, capPerf, capGpu") != std::string::npos);
}
