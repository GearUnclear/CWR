#include <Poseidon/Dev/Diag/FrameProfiler.hpp>

#include <cstdio>

namespace Poseidon
{
namespace Dev
{

namespace
{
int Clamp(int n, int count)
{
    return n <= 0 || n > count ? count : n;
}
} // namespace

std::string FrameProfiler::RingLog(int n) const
{
    n = Clamp(n, _count);
    std::string s;
    s.reserve(static_cast<size_t>(n) * 36);
    char b[96];
    for (int i = n - 1; i >= 0; --i)
    {
        const FrameRecord& r = Frame(i);
        std::snprintf(b, sizeof(b), "%.2f,%.2f,%.2f,%.2f,%.2f;", r.totalMs, r.ms[PhaseDrawObjects], r.ms[PhaseSwap],
                      r.ms[PhaseDrawLandObjects], r.ms[PhaseDrawLandscape]);
        s += b;
    }
    return s;
}

std::string FrameProfiler::RingCsv(int n) const
{
    n = Clamp(n, _count);
    std::string s = "i,total";
    for (int p = 0; p < PhaseCount; ++p)
    {
        s += ',';
        s += PhaseName(p);
    }
    s += ",draws\n";
    s.reserve(s.size() + static_cast<size_t>(n) * (12 + PhaseCount * 7));
    char b[32];
    for (int i = n - 1; i >= 0; --i)
    {
        const FrameRecord& r = Frame(i);
        std::snprintf(b, sizeof(b), "%d,%.2f", n - 1 - i, r.totalMs);
        s += b;
        for (int p = 0; p < PhaseCount; ++p)
        {
            std::snprintf(b, sizeof(b), ",%.2f", r.ms[p]);
            s += b;
        }
        std::snprintf(b, sizeof(b), ",%d\n", r.drawCalls);
        s += b;
    }
    return s;
}

FrameProfiler& GFrameProfiler()
{
    static FrameProfiler instance;
    return instance;
}

} // namespace Dev
} // namespace Poseidon
