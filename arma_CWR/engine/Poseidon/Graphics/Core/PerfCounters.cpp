#include <Poseidon/Graphics/Core/PerfCounters.hpp>

#include <cstdio>

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
    {&FC::lightSlots, nullptr},  {&FC::activeLights, nullptr}, {&FC::opaqueObjects, nullptr},
    {&FC::runs, nullptr},        {&FC::runObjects, nullptr},   {&FC::litObjects, nullptr},
    {&FC::hillRan, nullptr},     {&FC::hillIdle, nullptr},     {&FC::hillTested, nullptr},
    {&FC::hillHidden, nullptr},  {&FC::hillKept, nullptr},     {&FC::hillBuilds, nullptr},
    {&FC::hillWedges, nullptr},  {nullptr, &FC::hillMs},       {nullptr, &FC::lodMs},
    {nullptr, &FC::occlusionMs}, {&FC::litUploads, nullptr},   {&FC::litSkips, nullptr},
    {&FC::litBytes, nullptr},    {&FC::vsUploads, nullptr},    {&FC::vsSkips, nullptr},
    {&FC::vsBytes, nullptr},     {&FC::tileLights, nullptr},   {&FC::tileRefs, nullptr},
    {nullptr, &FC::tileMs},
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
} // namespace

Counters& GPerfCounters()
{
    static Counters s_counters;
    return s_counters;
}

void Counters::EndFrame()
{
    for (int i = 0; i < kListed; ++i)
        _sum[i] += Value(_frame, kFieldList[i]);
    ++_frames;
    _last = _frame;
    _frame = FrameCounters();
}

void Counters::ResetWindow()
{
    for (double& s : _sum)
        s = 0.0;
    _frames = 0;
}

std::string Counters::Summary() const
{
    const double n = _frames > 0 ? static_cast<double>(_frames) : 1.0;
    auto avg = [&](int FrameCounters::* m)
    {
        for (int i = 0; i < kListed; ++i)
            if (kFieldList[i].i == m)
                return _sum[i] / n;
        return 0.0;
    };
    auto avgf = [&](float FrameCounters::* m)
    {
        for (int i = 0; i < kListed; ++i)
            if (kFieldList[i].f == m)
                return _sum[i] / n;
        return 0.0;
    };
    char buf[640];
    std::snprintf(buf, sizeof(buf),
                  "objs opaque=%.0f runs=%.0f runobj=%.0f litobj=%.0f lights=%.0f/%.0f | hill ran=%.2f idle=%.2f "
                  "tested=%.0f hidden=%.0f kept=%.1f builds=%.2f wedges=%.0f hillcpu=%.3f | prep lod=%.3f occ=%.3f | "
                  "ubo lit=%.0f/%.0f litKB=%.1f vs=%.0f/%.0f vsKB=%.1f | tiles lights=%.1f refs=%.0f tilecpu=%.3f",
                  avg(&FC::opaqueObjects), avg(&FC::runs), avg(&FC::runObjects), avg(&FC::litObjects),
                  avg(&FC::lightSlots), avg(&FC::activeLights), avg(&FC::hillRan), avg(&FC::hillIdle),
                  avg(&FC::hillTested), avg(&FC::hillHidden), avg(&FC::hillKept), avg(&FC::hillBuilds),
                  avg(&FC::hillWedges), avgf(&FC::hillMs), avgf(&FC::lodMs), avgf(&FC::occlusionMs),
                  avg(&FC::litUploads), avg(&FC::litSkips), avg(&FC::litBytes) / 1024.0, avg(&FC::vsUploads),
                  avg(&FC::vsSkips), avg(&FC::vsBytes) / 1024.0, avg(&FC::tileLights), avg(&FC::tileRefs),
                  avgf(&FC::tileMs));
    return buf;
}

} // namespace Poseidon::perf
