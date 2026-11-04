#include <Poseidon/Graphics/Core/PerfCounters.hpp>

#include <Poseidon/Dev/Diag/FrameProfiler.hpp>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <vector>

#if defined(_M_X64) || defined(__x86_64__)
#include <immintrin.h>
#define POSEIDON_PERF_TSC 1
#endif

namespace Poseidon::perf
{

namespace
{
// Every FrameCounters member, int or float, in declaration order.
struct Field
{
    int FrameCounters::* i;
    float FrameCounters::* f;
};
using FC = FrameCounters;
constexpr Field kFieldList[] = {
    {&FC::lightSlots, nullptr},
    {&FC::activeLights, nullptr},
    {&FC::opaqueObjects, nullptr},
    {&FC::runs, nullptr},
    {&FC::runObjects, nullptr},
    {&FC::litObjects, nullptr},
    {&FC::hillRan, nullptr},
    {&FC::hillIdle, nullptr},
    {&FC::hillTested, nullptr},
    {&FC::hillHidden, nullptr},
    {&FC::hillBuilds, nullptr},
    {&FC::hillWedges, nullptr},
    {nullptr, &FC::hillMs},
    {nullptr, &FC::lodMs},
    {nullptr, &FC::occlusionMs},
    {&FC::litUploads, nullptr},
    {&FC::litSkips, nullptr},
    {&FC::litBytes, nullptr},
    {&FC::vsUploads, nullptr},
    {&FC::vsSkips, nullptr},
    {&FC::vsBytes, nullptr},
    {&FC::tileLights, nullptr},
    {&FC::tileRefs, nullptr},
    {nullptr, &FC::tileMs},
    // P0-15 (e)
    {&FC::landFitDraws, nullptr},
    {&FC::vbRewrites, nullptr},
    {nullptr, &FC::vbRewriteKB},
    {&FC::vbDynUploads, nullptr},
    {nullptr, &FC::vbDynKB},
    {&FC::occTested, nullptr},
    {&FC::occCulled, nullptr},
    {&FC::cloudlets, nullptr},
    {&FC::programBinds, nullptr},
    {&FC::textureBinds, nullptr},
    {&FC::samplerBinds, nullptr},
    {&FC::uboRangeBinds, nullptr},
    {&FC::refuseScene, nullptr},
    {&FC::refuseStatic, nullptr},
    {&FC::refuseOwnVertices, nullptr},
    {&FC::refuseLit, nullptr},
    {&FC::refuseShape, nullptr},
    {&FC::refuseLod, nullptr},
    {&FC::refusePass, nullptr},
    {&FC::refuseSpecial, nullptr},
    {&FC::refuseFogBand, nullptr},
    // P0-15 (c)
    {&FC::tris, nullptr},
    {&FC::lodSwitches, nullptr},
    {&FC::lodHist0, nullptr},
    {&FC::lodHist1, nullptr},
    {&FC::lodHist2, nullptr},
    {&FC::lodHist3, nullptr},
    {&FC::lodHist4, nullptr},
    {&FC::lodHist5, nullptr},
    {&FC::lodHist6, nullptr},
    {&FC::lodHist7, nullptr},
    {&FC::pipeGround, nullptr},
    {&FC::pipePass1, nullptr},
    {nullptr, &FC::groundPrims},
    {nullptr, &FC::groundClipIn},
    {nullptr, &FC::groundClipOut},
    {nullptr, &FC::groundVs},
    {nullptr, &FC::groundFs},
    {nullptr, &FC::pass1Prims},
    {nullptr, &FC::pass1ClipIn},
    {nullptr, &FC::pass1ClipOut},
    {nullptr, &FC::pass1Vs},
    {nullptr, &FC::pass1Fs},
    // P0-15 (f)
    {&FC::fogSightTarget, nullptr},
    {&FC::fogSightScript, nullptr},
    {&FC::fogSightOther, nullptr},
    {&FC::fogMarches, nullptr},
    {&FC::fogMarchesLandVis0, nullptr},
    {&FC::fogOutOfCone, nullptr},
    {&FC::combatLightRecords, nullptr},
    {nullptr, &FC::combatLightMs},
    {&FC::decalCalls, nullptr},
    {nullptr, &FC::decalMs},
    {&FC::simCloudlets, nullptr},
    {&FC::simHashes, nullptr},
    {&FC::sensorUpdates, nullptr},
    {nullptr, &FC::sensorMs},
};
// The list holds every member (all are 4 bytes), and the header sized the sums
// for it.
constexpr int kListed = static_cast<int>(sizeof(kFieldList) / sizeof(kFieldList[0]));
static_assert(kListed == Counters::kFields, "PerfCounters: one sum per listed field");
static_assert(sizeof(FrameCounters) == static_cast<size_t>(kListed) * 4, "PerfCounters: list every member");

double Value(const FrameCounters& c, const Field& f)
{
    return f.i ? static_cast<double>(c.*(f.i)) : static_cast<double>(c.*(f.f));
}

// A member's window sum, looked up by its pointer.
struct Sums
{
    double v[Counters::kFields];
    double operator()(int FrameCounters::* m) const
    {
        for (int i = 0; i < kListed; ++i)
            if (kFieldList[i].i == m)
                return v[i];
        return 0.0;
    }
    double operator()(float FrameCounters::* m) const
    {
        for (int i = 0; i < kListed; ++i)
            if (kFieldList[i].f == m)
                return v[i];
        return 0.0;
    }
};
} // namespace

static_assert(Counters::kWindow == Dev::FrameProfiler::kRingSize,
              "PerfCounters: the window covers the frames of the profiler's ring");

void CountRunRefusal(FrameCounters& c, RunRefusal r)
{
    switch (r)
    {
        case RunRefusal::Scene:
            ++c.refuseScene;
            break;
        case RunRefusal::Static:
            ++c.refuseStatic;
            break;
        case RunRefusal::OwnVertices:
            ++c.refuseOwnVertices;
            break;
        case RunRefusal::Lit:
            ++c.refuseLit;
            break;
        case RunRefusal::Shape:
            ++c.refuseShape;
            break;
        case RunRefusal::Lod:
            ++c.refuseLod;
            break;
        case RunRefusal::Pass:
            ++c.refusePass;
            break;
        case RunRefusal::Special:
            ++c.refuseSpecial;
            break;
        case RunRefusal::FogBand:
            ++c.refuseFogBand;
            break;
        case RunRefusal::Count:
            break;
    }
}

void CountLod(FrameCounters& c, int lod)
{
    if (lod < 0)
        return;
    int* const buckets[8] = {&c.lodHist0, &c.lodHist1, &c.lodHist2, &c.lodHist3,
                             &c.lodHist4, &c.lodHist5, &c.lodHist6, &c.lodHist7};
    ++*buckets[lod < 7 ? lod : 7];
}

void CountFogSight(FrameCounters& c, FogSightCaller who)
{
    switch (who)
    {
        case FogSightCaller::Target:
            ++c.fogSightTarget;
            break;
        case FogSightCaller::Script:
            ++c.fogSightScript;
            break;
        case FogSightCaller::Other:
            ++c.fogSightOther;
            break;
    }
}

Counters::Counters() : _ring(new FrameCounters[kWindow]()) {}

Counters& GPerfCounters()
{
    static Counters s_counters;
    return s_counters;
}

void Counters::EndFrame()
{
    _ring[_head] = _frame;
    _head = (_head + 1) % kWindow;
    if (_count < kWindow)
        ++_count;
    _last = _frame;
    _frame = FrameCounters();
    if (gCpuScopesOn && this == &GPerfCounters())
        GCpuScopes().EndFrame();
}

void Counters::ResetWindow()
{
    _head = 0;
    _count = 0;
}

void Counters::WindowSums(double (&sum)[kFields]) const
{
    for (double& v : sum)
        v = 0.0;
    for (int b = 0; b < _count; ++b)
    {
        const FrameCounters& c = WindowFrame(b);
        for (int i = 0; i < kListed; ++i)
            sum[i] += Value(c, kFieldList[i]);
    }
}

std::string Counters::Summary() const
{
    Sums sum;
    WindowSums(sum.v);
    const double n = _count > 0 ? static_cast<double>(_count) : 1.0;
    auto avg = [&](int FrameCounters::* m) { return sum(m) / n; };
    auto avgf = [&](float FrameCounters::* m) { return sum(m) / n; };
    char buf[640];
    std::snprintf(buf, sizeof(buf),
                  "objs opaque=%.0f runs=%.0f runobj=%.0f litobj=%.0f lights=%.0f/%.0f | hill ran=%.2f idle=%.2f "
                  "tested=%.0f hidden=%.0f builds=%.2f wedges=%.0f hillcpu=%.3f | prep lod=%.3f occ=%.3f | "
                  "ubo lit=%.0f/%.0f litKB=%.1f vs=%.0f/%.0f vsKB=%.1f | tiles lights=%.1f refs=%.0f tilecpu=%.3f",
                  avg(&FC::opaqueObjects), avg(&FC::runs), avg(&FC::runObjects), avg(&FC::litObjects),
                  avg(&FC::lightSlots), avg(&FC::activeLights), avg(&FC::hillRan), avg(&FC::hillIdle),
                  avg(&FC::hillTested), avg(&FC::hillHidden), avg(&FC::hillBuilds), avg(&FC::hillWedges),
                  avgf(&FC::hillMs), avgf(&FC::lodMs), avgf(&FC::occlusionMs), avg(&FC::litUploads), avg(&FC::litSkips),
                  avg(&FC::litBytes) / 1024.0, avg(&FC::vsUploads), avg(&FC::vsSkips), avg(&FC::vsBytes) / 1024.0,
                  avg(&FC::tileLights), avg(&FC::tileRefs), avgf(&FC::tileMs));
    std::string out = buf;
    std::snprintf(buf, sizeof(buf),
                  " | geo landfit=%.0f vbrw=%.0f vbKB=%.1f vbdyn=%.0f vbdynKB=%.1f occT=%.0f occC=%.0f cloudlets=%.0f"
                  " | binds prog=%.0f tex=%.0f smp=%.0f uboB=%.0f"
                  " | refuse=%.0f/%.0f/%.0f/%.0f/%.0f/%.0f/%.0f/%.0f/%.0f",
                  avg(&FC::landFitDraws), avg(&FC::vbRewrites), avgf(&FC::vbRewriteKB), avg(&FC::vbDynUploads),
                  avgf(&FC::vbDynKB), avg(&FC::occTested), avg(&FC::occCulled), avg(&FC::cloudlets),
                  avg(&FC::programBinds), avg(&FC::textureBinds), avg(&FC::samplerBinds), avg(&FC::uboRangeBinds),
                  avg(&FC::refuseScene), avg(&FC::refuseStatic), avg(&FC::refuseOwnVertices), avg(&FC::refuseLit),
                  avg(&FC::refuseShape), avg(&FC::refuseLod), avg(&FC::refusePass), avg(&FC::refuseSpecial),
                  avg(&FC::refuseFogBand));
    out += buf;
    std::snprintf(buf, sizeof(buf), " | tris=%.0f lodsw=%.0f lodhist=%.0f/%.0f/%.0f/%.0f/%.0f/%.0f/%.0f/%.0f",
                  avg(&FC::tris), avg(&FC::lodSwitches), avg(&FC::lodHist0), avg(&FC::lodHist1), avg(&FC::lodHist2),
                  avg(&FC::lodHist3), avg(&FC::lodHist4), avg(&FC::lodHist5), avg(&FC::lodHist6), avg(&FC::lodHist7));
    out += buf;
    // The pipeline statistics: over the frames that read a result back.
    const double ng = sum(&FC::pipeGround) > 0.0 ? sum(&FC::pipeGround) : 1.0;
    const double np = sum(&FC::pipePass1) > 0.0 ? sum(&FC::pipePass1) : 1.0;
    std::snprintf(buf, sizeof(buf),
                  " | pipe=%.0f/%.0f gprims=%.0f gclipin=%.0f gclipout=%.0f gvs=%.0f gfs=%.0f p1prims=%.0f "
                  "p1clipin=%.0f p1clipout=%.0f p1vs=%.0f p1fs=%.0f",
                  sum(&FC::pipeGround), sum(&FC::pipePass1), sum(&FC::groundPrims) / ng, sum(&FC::groundClipIn) / ng,
                  sum(&FC::groundClipOut) / ng, sum(&FC::groundVs) / ng, sum(&FC::groundFs) / ng,
                  sum(&FC::pass1Prims) / np, sum(&FC::pass1ClipIn) / np, sum(&FC::pass1ClipOut) / np,
                  sum(&FC::pass1Vs) / np, sum(&FC::pass1Fs) / np);
    out += buf;
    return out;
}

std::string Counters::SimSummary() const
{
    Sums sum;
    WindowSums(sum.v);
    const double n = _count > 0 ? static_cast<double>(_count) : 1.0;
    auto avg = [&](int FrameCounters::* m) { return sum(m) / n; };
    auto avgf = [&](float FrameCounters::* m) { return sum(m) / n; };
    char buf[640];
    std::snprintf(buf, sizeof(buf),
                  "fogsight target=%.1f script=%.1f other=%.1f marches=%.1f landvis0=%.1f outcone=%.1f | combat "
                  "records=%.1f cpu=%.3f | decals calls=%.1f cpu=%.3f | cloudlets=%.1f hashes=%.1f | sensors "
                  "updates=%.2f cpu=%.3f | n=%d",
                  avg(&FC::fogSightTarget), avg(&FC::fogSightScript), avg(&FC::fogSightOther), avg(&FC::fogMarches),
                  avg(&FC::fogMarchesLandVis0), avg(&FC::fogOutOfCone), avg(&FC::combatLightRecords),
                  avgf(&FC::combatLightMs), avg(&FC::decalCalls), avgf(&FC::decalMs), avg(&FC::simCloudlets),
                  avg(&FC::simHashes), avg(&FC::sensorUpdates), avgf(&FC::sensorMs), _count);
    return buf;
}

// --- CPU sub-scopes ---

bool gCpuScopesOn = false;

namespace
{
constexpr const char* kScopeNames[CpuScopes::kCount] = {
    "occtest", "occrender", "runbuild", "drawloop", "grass",   "p2sort",  "p2draw",    "shcollect",
    "shgl",    "census",    "obj.bld",  "obj.veg",  "obj.veh", "obj.man", "obj.other",
};

std::int64_t SteadyNs()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}
} // namespace

CpuScopes::CpuScopes() : _ring(new FrameRecord[kWindow]()) {}

const char* CpuScopes::Name(CpuScope s)
{
    const int i = static_cast<int>(s);
    return i >= 0 && i < kCount ? kScopeNames[i] : "?";
}

std::uint64_t CpuScopes::Now()
{
#ifdef POSEIDON_PERF_TSC
    return __rdtsc();
#else
    return static_cast<std::uint64_t>(SteadyNs());
#endif
}

void CpuScopes::FixTicksPerMs(double ticksPerMs)
{
    _ticksPerMs = ticksPerMs;
    _fixed = true;
}

void CpuScopes::SetEnabled(bool on)
{
    if (on)
    {
        ResetWindow();
        _calTicks = Now();
        _calNs = SteadyNs();
    }
    _enabled = on;
    if (this == &GCpuScopes())
        gCpuScopesOn = on;
}

void CpuScopes::Add(CpuScope s, std::uint64_t ticks, int weight)
{
    const int i = static_cast<int>(s);
    if (i < 0 || i >= kCount)
        return;
    _ticks[i] += ticks * static_cast<std::uint64_t>(weight > 0 ? weight : 1);
    ++_calls[i];
}

void CpuScopes::EndFrame()
{
    if (!_fixed)
    {
#ifdef POSEIDON_PERF_TSC
        // Calibrate the TSC against steady_clock over everything since the
        // window started; it settles within a frame or two.
        const double ms = static_cast<double>(SteadyNs() - _calNs) * 1e-6;
        if (ms > 0.5)
            _ticksPerMs = static_cast<double>(Now() - _calTicks) / ms;
#else
        _ticksPerMs = 1e6;
#endif
    }
    FrameRecord& r = _ring[_head];
    const double scale = _ticksPerMs > 0.0 ? 1.0 / _ticksPerMs : 0.0;
    for (int i = 0; i < kCount; ++i)
    {
        r.ms[i] = static_cast<float>(static_cast<double>(_ticks[i]) * scale);
        r.calls[i] = _calls[i];
        _ticks[i] = 0;
        _calls[i] = 0;
    }
    _head = (_head + 1) % kWindow;
    if (_count < kWindow)
        ++_count;
}

void CpuScopes::ResetWindow()
{
    _head = 0;
    _count = 0;
    for (int i = 0; i < kCount; ++i)
    {
        _ticks[i] = 0;
        _calls[i] = 0;
    }
}

float CpuScopes::WindowMs(int back, CpuScope s) const
{
    return _ring[(_head - 1 - back + 2 * kWindow) % kWindow].ms[static_cast<int>(s)];
}

std::string CpuScopes::Summary() const
{
    if (_count == 0)
        return _enabled ? "cpu: no frames measured yet" : "cpu: scopes off";
    std::string out;
    std::vector<float> vals(static_cast<size_t>(_count));
    char buf[96];
    for (int s = 0; s < kCount; ++s)
    {
        double sum = 0.0;
        long long calls = 0;
        for (int b = 0; b < _count; ++b)
        {
            const FrameRecord& r = _ring[(_head - 1 - b + 2 * kWindow) % kWindow];
            vals[static_cast<size_t>(b)] = r.ms[s];
            sum += r.ms[s];
            calls += r.calls[s];
        }
        const int nth = (_count * 95) / 100;
        std::nth_element(vals.begin(), vals.begin() + nth, vals.end());
        std::snprintf(buf, sizeof(buf), "%s%s=%.3f/%.3f/%.1f", out.empty() ? "" : " ", kScopeNames[s], sum / _count,
                      vals[static_cast<size_t>(nth)], static_cast<double>(calls) / _count);
        out += buf;
    }
    std::snprintf(buf, sizeof(buf), " n=%d", _count);
    out += buf;
    return out;
}

CpuScopes& GCpuScopes()
{
    static CpuScopes s_scopes;
    return s_scopes;
}

bool CpuObjectSample()
{
    static unsigned s_next = 0;
    if (!gCpuScopesOn)
        return false;
    return (s_next++ % CpuScopes::kObjectSampleEvery) == 0;
}

CpuMsTimer::~CpuMsTimer()
{
    if (!_t0)
        return;
    const double tpm = GCpuScopes().TicksPerMs();
    if (tpm > 0.0)
        _field += static_cast<float>(static_cast<double>(CpuScopes::Now() - _t0) / tpm);
}

} // namespace Poseidon::perf
