#include <Evaluator/express.hpp>
#include <Poseidon/Core/BuildProvenance.hpp>
#include <Poseidon/Core/Game/GameLoop.hpp>
#include <Poseidon/Dev/Diag/FrameProfiler.hpp>
#include <Poseidon/Foundation/Logging/Logging.hpp>
#include <Poseidon/Graphics/Core/GpuTimings.hpp>
#include <Poseidon/Graphics/Core/PerfCounters.hpp>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

using namespace Poseidon;

// Measurement verbs for the perf bench (tests/rendering/captures/gate): the
// per-frame rings, the build id, the pinned configuration and the counters. GLOBAL scope like the rest of the tri verb
// surface (registered, with forward decls, in GameStateExtTestAudio.cpp). Each verb only reads or resets bookkeeping;
// with none of them called, play is unchanged.

std::string GetTriOutputDir();

namespace
{
// The label part of a ring file name: letters, digits, '.', '-' and '_' kept,
// anything else '_'; empty when no label was given.
std::string FileLabel(const std::string& label)
{
    std::string out;
    out.reserve(label.size());
    for (char c : label)
    {
        const bool keep = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' ||
                          c == '-' || c == '_';
        out += keep ? c : '_';
    }
    return out;
}

// "<n>" or [<n>, "<label>"]: the frame count and the file label of a ring verb.
void RingArgs(GameValuePar arg, int& n, std::string& label)
{
    n = 0;
    label.clear();
    if (arg.GetType() == GameArray)
    {
        const GameArrayType& a = arg;
        if (a.Size() > 0 && a[0].GetType() == GameScalar)
            n = toInt(static_cast<float>(static_cast<GameScalarType>(a[0])));
        if (a.Size() > 1 && a[1].GetType() == GameString)
        {
            GameStringType s = static_cast<GameStringType>(a[1]);
            label = (const char*)s ? (const char*)s : "";
        }
    }
    else if (arg.GetType() == GameScalar)
    {
        n = toInt(static_cast<float>(static_cast<GameScalarType>(arg)));
    }
}

// Writes `csv` to <TRI_OUTPUT_DIR>/<stem>-<label>.csv (or <stem>-<k>.csv, k
// counting the unlabelled files of this stem); returns the path, empty when
// the file could not be written.
std::string WriteRingCsv(const char* stem, const std::string& label, int& unlabelled, const std::string& csv)
{
    const std::string dir = GetTriOutputDir();
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    const std::string tag = label.empty() ? std::to_string(unlabelled++) : FileLabel(label);
    const std::string path = dir + "/" + stem + "-" + tag + ".csv";
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f)
        return {};
    f << csv;
    return f ? path : std::string();
}
} // namespace

/// triPerfRing <n> or [<n>, "<label>"]: the last n frames of the CPU frame
/// profiler's ring (n <= 0: all it holds, at most 256), oldest first. Logs
/// "[tri] triPerfRing total,drw:obj,swap,land:obj,drw:land;..." (the round-1
/// format) and writes every phase and the draw calls per frame to
/// <TRI_OUTPUT_DIR>/perfring-<label>.csv (Dev::FrameProfiler::RingCsv; an
/// unlabelled call numbers its file instead). Returns "OK:<n>" (frames
/// written), "FAIL:no_frames" or "FAIL:csv".
GameValue TriPerfRing(const GameState* /*state*/, GameValuePar arg)
{
    static int s_unlabelled = 0;
    const Dev::FrameProfiler& perf = Dev::GFrameProfiler();
    if (perf.FrameCount() == 0)
        return GameValue("FAIL:no_frames");
    int n = 0;
    std::string label;
    RingArgs(arg, n, label);
    if (n <= 0 || n > perf.FrameCount())
        n = perf.FrameCount();
    LOG_INFO(Core, "[tri] triPerfRing {}", perf.RingLog(n));
    const std::string path = WriteRingCsv("perfring", label, s_unlabelled, perf.RingCsv(n));
    if (path.empty())
        return GameValue("FAIL:csv");
    LOG_INFO(Core, "[tri] triPerfRing csv={} frames={}", path, n);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "OK:%d", n);
    return GameValue(buf);
}

/// triGpuRing <n> or [<n>, "<label>"]: the last n frames' GPU times
/// (Poseidon::gpu::GpuFrameRing: frame, world, pass1, ground, rest, post, up
/// to 256 frames since triGpuTimes last started a window), oldest first. Logs
/// "[tri] triGpuRing frame,world,pass1,ground,rest,post;..." and writes
/// <TRI_OUTPUT_DIR>/gpuring-<label>.csv (an empty field: not measured that
/// frame). The records trail the CPU frame by the read-back delay. Returns
/// "OK:<n>", "FAIL:no_frames" (timing off or nothing read back yet) or
/// "FAIL:csv".
GameValue TriGpuRing(const GameState* /*state*/, GameValuePar arg)
{
    static int s_unlabelled = 0;
    const gpu::GpuFrameRing& ring = gpu::GGpuFrameRing();
    if (ring.Count() == 0)
        return GameValue("FAIL:no_frames");
    int n = 0;
    std::string label;
    RingArgs(arg, n, label);
    if (n <= 0 || n > ring.Count())
        n = ring.Count();
    LOG_INFO(Core, "[tri] triGpuRing {}", ring.LogText(n));
    const std::string path = WriteRingCsv("gpuring", label, s_unlabelled, ring.CsvText(n));
    if (path.empty())
        return GameValue("FAIL:csv");
    LOG_INFO(Core, "[tri] triGpuRing csv={} frames={}", path, n);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "OK:%d", n);
    return GameValue(buf);
}

/// triBuildId: the build provenance of this binary (Core/BuildProvenance.hpp):
/// "<git describe --always --dirty>" of the source it was built from, plus
/// "+<12 hex>" when engine/ or apps/ held local changes. Logs "[tri]
/// build_id=<id>" and returns the id.
GameValue TriBuildId(const GameState* /*state*/)
{
    const char* id = BuildProvenance::BuildId();
    LOG_INFO(Core, "[tri] build_id={}", id);
    return GameValue(id);
}

/// triSetFpsCap <n>: the frame cap RenderFrame paces to (Poseidon::gUserFpsCap,
/// the Graphics screen's fpsCap; 0 = unlimited), for benches that pin the
/// configuration instead of inheriting the user's. Session-only: graphics.cfg
/// is not written, and applying the Graphics screen sets it again. n is
/// clamped to 0..1000. Returns "OK:<cap>"; read it back with triGetFpsCap.
GameValue TriSetFpsCap(const GameState* /*state*/, GameValuePar arg)
{
    const int cap = std::clamp(toInt(static_cast<float>(static_cast<GameScalarType>(arg))), 0, 1000);
    gUserFpsCap = cap;
    LOG_INFO(Core, "[tri] triSetFpsCap {}", cap);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "OK:%d", cap);
    return GameValue(buf);
}

/// triCpuStats <on>: the CPU sub-scopes (Graphics/Core/PerfCounters.hpp
/// CpuScopes: occlusion tests and occluder draws, run building, the opaque
/// draw loop, grass, Pass2 sort and draw, shadow collection and GL, the
/// census, and per object class one object in 16, scaled up). It returns the
/// summary of the window so far, "<scope>=<avg ms>/<p95 ms>/<calls per frame>
/// ... n=<frames>", then applies <on>: true starts a new window (turning the
/// timers on when off), false stops timing. Off by default; while off each
/// timer costs one load and a branch. A scope reads 0 until its site is
/// instrumented. The millisecond fields of triSimStats are timed only while
/// this is on.
GameValue TriCpuStats(const GameState* /*state*/, GameValuePar arg)
{
    perf::CpuScopes& scopes = perf::GCpuScopes();
    const std::string summary = scopes.Summary();
    const bool on = static_cast<bool>(arg);
    scopes.SetEnabled(on);
    LOG_INFO(Core, "[tri] triCpuStats {} -> {}", summary, on ? "on" : "off");
    return GameValue(summary.c_str());
}

/// triSimStats: the simulation counters averaged per frame over the frame
/// profiler's window (triPerfReset restarts it): AI sight through the fog by
/// caller, its marches, the marches with the landscape visibility already 0,
/// sight checks out of the cone, combat-light records, DrawDecal calls,
/// simulated cloudlets, spatial hash use and SensorList::SmartUpdateAll (which
/// runs outside every profiled phase), with CPU ms where triCpuStats is on.
/// "fogsight target=<n> script=<n> other=<n> marches=<n> landvis0=<n>
/// outcone=<n> | combat records=<n> cpu=<ms> | decals calls=<n> cpu=<ms> |
/// cloudlets=<n> hashes=<n> | sensors updates=<n> cpu=<ms> | n=<frames>".
/// A counter reads 0 until its site is instrumented.
GameValue TriSimStats(const GameState* /*state*/)
{
    const std::string s = perf::GPerfCounters().SimSummary();
    LOG_INFO(Core, "[tri] triSimStats {}", s);
    return GameValue(s.c_str());
}

/// triPipeStats <on>: GL_ARB_pipeline_statistics_query around the world
/// pass's ground and opaque-object spans (primitives submitted, clipping in
/// and out, vertex and fragment shader invocations), read back a few frames
/// later without waiting into the counters triPerfStats prints ("pipe=..."
/// onwards). Samples only while GPU timing is on (triGpuTimes true), from the
/// next frame. Off by default: no query is issued. Returns "OK:on" or
/// "OK:off".
GameValue TriPipeStats(const GameState* /*state*/, GameValuePar arg)
{
    const bool on = static_cast<bool>(arg);
    gpu::SetPipelineStatsWanted(on);
    LOG_INFO(Core, "[tri] triPipeStats {}", on ? "on" : "off");
    return GameValue(on ? "OK:on" : "OK:off");
}
