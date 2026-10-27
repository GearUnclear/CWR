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
    {"ground", Mark::WorldBegin, Mark::GroundEnd},
    {"pass1", Mark::GroundEnd, Mark::Pass1End},
    {"rest", Mark::Pass1End, Mark::WorldEnd},
    {"sky", Mark::SkyBegin, Mark::SkyEnd},
    {"skytable", Mark::SkyBegin, Mark::SkyTableEnd},
    {"canopy", Mark::CanopyBegin, Mark::CanopyEnd},
    {"combatmask", Mark::CombatMaskBegin, Mark::CombatMaskEnd},
    {"resolve", Mark::WorldEnd, Mark::ResolveEnd},
    {"lindepth", Mark::ResolveEnd, Mark::LinearDepthEnd},
    {"tags", Mark::LinearDepthEnd, Mark::TagReadoutEnd},
    {"ao", Mark::TagReadoutEnd, Mark::AoEnd},
    {"ssr", Mark::AoEnd, Mark::SsrEnd},
    {"air", Mark::SsrEnd, Mark::AirEnd},
    {"bloom", Mark::AirEnd, Mark::BloomEnd},
    {"shafts", Mark::BloomEnd, Mark::ShaftsEnd},
    {"scatter", Mark::ShaftsEnd, Mark::CombatScatterEnd},
    {"composite", Mark::CombatScatterEnd, Mark::CompositeEnd},
    {"taa", Mark::CompositeEnd, Mark::TemporalEnd},
    {"sharpen", Mark::TemporalEnd, Mark::SharpenEnd},
    {"debugview", Mark::SharpenEnd, Mark::DebugViewEnd},
    {"post", Mark::WorldEnd, Mark::PostEnd},
};

constexpr bool PostStageSectionsChain()
{
    // Each post stage section ends at its own mark and starts at the mark of
    // the stage before it, the first one at WorldEnd.
    for (int i = 0; i < kPostStageMarks; ++i)
    {
        const SectionDef& d = kSections[static_cast<int>(Section::Resolve) + i];
        if (static_cast<int>(d.to) != static_cast<int>(Mark::ResolveEnd) + i)
            return false;
        if (static_cast<int>(d.from) != static_cast<int>(Mark::WorldEnd) + i)
            return false;
    }
    return true;
}
static_assert(PostStageSectionsChain(), "post stage sections run from the previous stage mark to their own");

bool IsPostStage(Section s)
{
    return static_cast<int>(s) >= static_cast<int>(Section::Resolve) &&
           static_cast<int>(s) <= static_cast<int>(Section::DebugView);
}

constexpr float kSmoothing = 0.1f; // weight of the newest frame
} // namespace

const char* SectionName(Section s)
{
    return kSections[static_cast<int>(s)].name;
}

Section PostStageSection(Mark m)
{
    const int i = static_cast<int>(m) - static_cast<int>(Mark::ResolveEnd);
    if (i < 0 || i >= kPostStageMarks)
        return Section::Count;
    return static_cast<Section>(static_cast<int>(Section::Resolve) + i);
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
            // A post stage that did not run leaves no mark: the next stage
            // starts at the latest one that did, at the latest at WorldEnd.
            if (!IsPostStage(s))
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
        _min[i] = _count[i] == 0 || ms < _min[i] ? ms : _min[i];
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

bool GpuTimings::MinMs(Section s, float& ms) const
{
    const int i = static_cast<int>(s);
    if (_count[i] == 0)
        return false;
    ms = _min[i];
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
    {
        add(Section::Shadow, " (%s %.2f)");
        float ms = 0.0f;
        if (AverageMs(Section::Ground, ms) || AverageMs(Section::Pass1, ms) || AverageMs(Section::Rest, ms))
        {
            out += " [";
            const size_t open = out.size();
            for (Section s : {Section::Ground, Section::Pass1, Section::Rest})
                add(s, out.size() == open ? "%s %.2f" : " %s %.2f");
            out += "]";
        }
    }
    if (add(Section::Sky, " | %s %.2f"))
        add(Section::SkyTable, " (%s %.2f)");
    add(Section::Canopy, " | %s %.2f");
    add(Section::CombatMask, " | %s %.2f");
    if (add(Section::Post, " | %s %.2f:"))
    {
        for (int i = static_cast<int>(Section::Resolve); i <= static_cast<int>(Section::DebugView); ++i)
            add(static_cast<Section>(i), " %s %.2f");
    }
    // The best frame of the window for the frame, the world and its parts.
    bool first = true;
    for (Section s : {Section::Frame, Section::World, Section::Ground, Section::Pass1, Section::Rest})
    {
        float ms = 0.0f;
        if (!MinMs(s, ms))
            continue;
        std::snprintf(buf, sizeof(buf), "%s%s=%.2f", first ? " | min " : " ", SectionName(s), ms);
        out += buf;
        first = false;
    }
    return out.empty() ? "gpu: no sections measured yet" : out;
}

} // namespace Poseidon::gpu
