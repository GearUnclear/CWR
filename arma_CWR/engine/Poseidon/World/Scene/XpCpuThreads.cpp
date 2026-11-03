#include <Poseidon/World/Scene/XpCpuThreads.hpp>

#include <Poseidon/Core/TaskPool.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace Poseidon::XpCpuThreads
{

namespace
{
bool EnvSet(const char* name)
{
    const char* v = std::getenv(name);
    return v && *v && std::strcmp(v, "0") != 0;
}

unsigned ParseParts()
{
    const char* v = std::getenv("POSEIDON_XP_CPUTHREADS");
    if (!v || !*v || std::strcmp(v, "0") == 0)
    {
        return 0;
    }
    if (std::strcmp(v, "1") == 0 || std::strcmp(v, "all") == 0)
    {
        return kSort | kHill | kShadow | kLod | kPrep;
    }
    const std::string s(v);
    unsigned parts = 0;
    if (s.find("sort") != std::string::npos)
    {
        parts |= kSort;
    }
    if (s.find("hill") != std::string::npos)
    {
        parts |= kHill;
    }
    if (s.find("shadow") != std::string::npos)
    {
        parts |= kShadow;
    }
    if (s.find("lod") != std::string::npos)
    {
        parts |= kLod;
    }
    if (s.find("prep") != std::string::npos)
    {
        parts |= kPrep;
    }
    return parts;
}

constexpr int kStages = static_cast<int>(Stage::Count);
constexpr int kWindow = 256;
const char* const kStageNames[kStages] = {"sortocc", "sortshape", "sortsurf", "sortalpha", "hill",    "smcollect",
                                          "lod",     "prep",      "p1loop",   "p2surf",    "p2alpha", "smrender"};

struct Window
{
    double sum[kStages] = {};
    double max[kStages] = {};
    double frame[kStages] = {};
    long long mismatches[kStages] = {};
    int frames = 0;
    bool logged = false;
};

Window& GWindow()
{
    static Window w;
    return w;
}

long long NowNs()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}
} // namespace

unsigned Parts()
{
    static const unsigned s_parts = ParseParts();
    return s_parts;
}

bool VerifyOn()
{
    static const bool s_verify = Parts() != 0 && EnvSet("POSEIDON_XP_CPUTHREADS_VERIFY");
    return s_verify;
}

bool LogOn()
{
    static const bool s_log = EnvSet("POSEIDON_XP_CPUTHREADS_LOG");
    return s_log;
}

TaskPool* Pool(Part p)
{
    return On(p) ? GetGlobalTaskPool() : nullptr;
}

void AddStage(Stage s, double ms)
{
    GWindow().frame[static_cast<int>(s)] += ms;
}

void AddMismatches(Stage s, int n)
{
    GWindow().mismatches[static_cast<int>(s)] += n;
}

void FrameTick()
{
    Window& w = GWindow();
    if (!LogOn() && !VerifyOn())
    {
        return;
    }
    if (!w.logged)
    {
        w.logged = true;
        TaskPool* pool = GetGlobalTaskPool();
        LOG_INFO(Graphics, "XPCT config parts={} verify={} log={} poolThreads={}", Parts(), VerifyOn() ? 1 : 0,
                 LogOn() ? 1 : 0, pool ? pool->ThreadCount() : 0u);
    }
    // The stages timed since the last tick belong to the frame that just ended.
    for (int i = 0; i < kStages; i++)
    {
        w.sum[i] += w.frame[i];
        w.max[i] = std::max(w.max[i], w.frame[i]);
        w.frame[i] = 0.0;
    }
    if (++w.frames < kWindow)
    {
        return;
    }
    std::string line = "XPCT stages parts=" + std::to_string(Parts()) + " frames=" + std::to_string(w.frames);
    char buf[96];
    for (int i = 0; i < kStages; i++)
    {
        std::snprintf(buf, sizeof(buf), " %s=%.3f/%.3f", kStageNames[i], w.sum[i] / w.frames, w.max[i]);
        line += buf;
    }
    if (VerifyOn())
    {
        line += " | verify";
        for (int i = 0; i < kStages; i++)
        {
            line += " " + std::string(kStageNames[i]) + "=" + std::to_string(w.mismatches[i]);
        }
    }
    LOG_INFO(Graphics, "{}", line);
    const bool logged = w.logged;
    w = Window();
    w.logged = logged;
}

StageTimer::StageTimer(Stage s) : _s(s), _t0(LogOn() ? NowNs() : 0) {}

StageTimer::~StageTimer()
{
    Stop();
}

void StageTimer::Stop()
{
    if (_t0)
    {
        AddStage(_s, static_cast<double>(NowNs() - _t0) * 1e-6);
        _t0 = 0;
    }
}

} // namespace Poseidon::XpCpuThreads
