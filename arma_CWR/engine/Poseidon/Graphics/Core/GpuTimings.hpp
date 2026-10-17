#pragma once

#include <cstdint>
#include <string>

// GPU pass timings from timestamp queries: pure bookkeeping, no GL.
//
// The backend drops a timestamp at fixed points of a frame (GpuMark) and reads
// the results back a few frames later, when the GPU has finished them, so the
// CPU never waits. Each finished frame hands its stamps to Submit, which turns
// them into per-section durations and keeps a smoothed average per section.
// Marks a frame did not reach (a pass that was off) are skipped: a section
// starts at the latest mark before its own start that the frame did record,
// so the post-pass sections still add up to the post pass when AO is off.

namespace Poseidon::gpu
{

enum class Mark : int
{
    FrameBegin,
    WorldBegin,  // first 3D pass of the world
    ShadowBegin, // shadow-map depth pass
    ShadowEnd,
    WorldEnd,     // Engine::EndWorldScene, after the world's queued draws
    AoEnd,        // post: ambient occlusion and its blur
    BloomEnd,     // post: bloom chain
    ShaftsEnd,    // post: sun shafts
    CompositeEnd, // post: composite into the frame
    FrameEnd,     // before the swap
    Count
};

enum class Section : int
{
    Frame,
    World, // includes the shadow pass
    Shadow,
    Ao,
    Bloom,
    Shafts,
    Composite,
    Post, // Ao + Bloom + Shafts + Composite
    Count
};

constexpr int kMarkCount = static_cast<int>(Mark::Count);
constexpr int kSectionCount = static_cast<int>(Section::Count);

const char* SectionName(Section s);

class GpuTimings
{
  public:
    // stamps[i] is valid when bit i of `recorded` is set (nanoseconds, any
    // common origin). Frames that never reached FrameEnd are still used.
    void Submit(const std::uint64_t stamps[kMarkCount], std::uint32_t recorded);
    void Reset();

    // Smoothed duration in milliseconds; false while the section has never
    // been measured.
    bool AverageMs(Section s, float& ms) const;
    // Last frame's duration, -1 when that frame did not measure it.
    float LastMs(Section s) const { return _last[static_cast<int>(s)]; }
    int Frames() const { return _frames; }

    // "frame 6.12 ms | world 4.80 (shadow 0.62) | post 0.71: ao 0.30 bloom 0.21 shafts 0.05 composite 0.15"
    std::string Summary() const;

  private:
    float _avg[kSectionCount] = {};
    float _last[kSectionCount] = {-1, -1, -1, -1, -1, -1, -1, -1};
    int _count[kSectionCount] = {};
    int _frames = 0;
};

} // namespace Poseidon::gpu
