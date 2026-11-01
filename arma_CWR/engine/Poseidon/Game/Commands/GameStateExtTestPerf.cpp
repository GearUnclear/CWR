#include <Evaluator/express.hpp>
#include <Poseidon/Dev/Diag/FrameProfiler.hpp>
#include <Poseidon/Foundation/Logging/Logging.hpp>
#include <Poseidon/Graphics/Core/GpuTimings.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

using namespace Poseidon;

// Measurement verbs for the perf bench (tests/rendering/captures/gate): the
// per-frame rings. GLOBAL scope like the rest of the tri verb surface
// (registered, with forward decls, in GameStateExtTestAudio.cpp). Each verb
// only reads or resets bookkeeping; with none of them called, play is
// unchanged.

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
