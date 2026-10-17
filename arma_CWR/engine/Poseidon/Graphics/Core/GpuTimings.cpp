#include <Poseidon/Graphics/Core/GpuTimings.hpp>

#include <cstdio>

namespace Poseidon::gpu
{

namespace
{
struct SectionDef
{
    const char* name;
    Mark from;
    Mark to;
};

constexpr SectionDef kSections[kSectionCount] = {
    {"frame", Mark::FrameBegin, Mark::FrameEnd},
    {"world", Mark::WorldBegin, Mark::WorldEnd},
    {"shadow", Mark::ShadowBegin, Mark::ShadowEnd},
    {"ao", Mark::WorldEnd, Mark::AoEnd},
    {"bloom", Mark::AoEnd, Mark::BloomEnd},
    {"shafts", Mark::BloomEnd, Mark::ShaftsEnd},
    {"composite", Mark::ShaftsEnd, Mark::CompositeEnd},
    {"post", Mark::WorldEnd, Mark::CompositeEnd},
};

// Sections whose start may fall back to an earlier recorded mark: the post
// pass runs its steps in order and skips the ones that are off.
bool StartFallsBack(Section s)
{
    return s == Section::Bloom || s == Section::Shafts || s == Section::Composite;
}

constexpr float kSmoothing = 0.1f; // weight of the newest frame
} // namespace

const char* SectionName(Section s)
{
    return kSections[static_cast<int>(s)].name;
}

void GpuTimings::Reset()
{
    *this = GpuTimings();
}

void GpuTimings::Submit(const std::uint64_t stamps[kMarkCount], std::uint32_t recorded)
{
    auto has = [recorded](Mark m) { return (recorded >> static_cast<int>(m)) & 1u; };
    ++_frames;
    for (int i = 0; i < kSectionCount; ++i)
    {
        const Section s = static_cast<Section>(i);
        const SectionDef& d = kSections[i];
        _last[i] = -1.0f;
        if (!has(d.to))
            continue;
        int from = static_cast<int>(d.from);
        if (!has(d.from))
        {
            if (!StartFallsBack(s))
                continue;
            while (from > static_cast<int>(Mark::WorldEnd) && !has(static_cast<Mark>(from)))
                --from;
            if (!has(static_cast<Mark>(from)))
                continue;
        }
        const std::uint64_t a = stamps[from];
        const std::uint64_t b = stamps[static_cast<int>(d.to)];
        if (b < a)
            continue; // counter wrapped or a stale stamp: drop the sample
        const float ms = static_cast<float>(b - a) * 1e-6f;
        _last[i] = ms;
        _avg[i] = _count[i] == 0 ? ms : _avg[i] + (ms - _avg[i]) * kSmoothing;
        ++_count[i];
    }
}

bool GpuTimings::AverageMs(Section s, float& ms) const
{
    const int i = static_cast<int>(s);
    if (_count[i] == 0)
        return false;
    ms = _avg[i];
    return true;
}

std::string GpuTimings::Summary() const
{
    if (_frames == 0)
        return "gpu: no frames measured yet";
    std::string out;
    char buf[64];
    auto add = [&](Section s, const char* fmt)
    {
        float ms = 0.0f;
        if (!AverageMs(s, ms))
            return false;
        std::snprintf(buf, sizeof(buf), fmt, SectionName(s), ms);
        out += buf;
        return true;
    };
    add(Section::Frame, "%s %.2f ms");
    if (add(Section::World, " | %s %.2f"))
        add(Section::Shadow, " (%s %.2f)");
    if (add(Section::Post, " | %s %.2f:"))
    {
        add(Section::Ao, " %s %.2f");
        add(Section::Bloom, " %s %.2f");
        add(Section::Shafts, " %s %.2f");
        add(Section::Composite, " %s %.2f");
    }
    return out.empty() ? "gpu: no sections measured yet" : out;
}

} // namespace Poseidon::gpu
