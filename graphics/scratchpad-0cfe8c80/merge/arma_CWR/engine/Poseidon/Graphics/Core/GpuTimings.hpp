#pragma once

#include <array>
#include <cstdint>
#include <string>

// GPU pass timings from timestamp queries: pure bookkeeping, no GL.
//
// The backend drops a timestamp at fixed points of a frame (GpuMark) and reads
// the results back a few frames later, when the GPU has finished them, so the
// CPU never waits. Each finished frame hands its stamps to Submit, which turns
// them into per-section durations and keeps a smoothed average per section.
// Marks a frame did not reach (a pass that was off) are skipped: a post stage's
// section starts at the latest stage mark before its own start that the frame
// did record, so the stage sections still add up to the post pass when a stage
// is off.

namespace Poseidon::gpu
{

enum class Mark : int
{
    FrameBegin,
    // The remastered sky dome (the sky table's pass before the world): from
    // the hook opening it, past the sky-view table build when one runs, to the
    // end of the dome's draws.
    SkyBegin,
    SkyTableEnd,
    SkyEnd,
    // The canopy's sun-visibility map (the long tree shadows, B9), in the
    // frames that rebuild it, at the frame's first upload of the lit
    // constants: in the remastered sky's hook before SkyBegin when the sky
    // pass runs, otherwise at the world's first 3D pass just after
    // WorldBegin (the world and ground sections then include it too).
    CanopyBegin,
    CanopyEnd,
    // The combat shadow mask (B4): its trace and blur, at the start of the
    // world's first pass of a frame that shadows a combat light.
    CombatMaskBegin,
    CombatMaskEnd,
    WorldBegin, // first 3D pass of the world
    // Inside the world pass (Landscape::DrawRect, at its existing queue
    // flushes): the sky, terrain, water and horizon drawn; the opaque objects
    // (Scene::DrawObjectsAndShadowsPass1) drawn.
    GroundEnd,
    Pass1End,
    ShadowBegin, // shadow-map depth pass
    ShadowEnd,
    WorldEnd, // Engine::EndWorldScene, after the world's queued draws
    // The post pass: the end of each stage, in the order the stages run
    // (Poseidon::remaster::PostStage, Graphics/Remaster/PostStages.hpp, which
    // checks the order). A stage that does not run in a frame records none.
    ResolveEnd,       // resolve of the multisampled frame target
    LinearDepthEnd,   // half-resolution view depth
    TagReadoutEnd,    // SSR stencil tags (B6)
    AoEnd,            // ambient occlusion and its blur
    SsrEnd,           // screen-space reflections (B6)
    AirEnd,           // beams and haze (B8)
    BloomEnd,         // bloom chain
    ShaftsEnd,        // legacy sun shafts
    CombatScatterEnd, // combat light in smoke (B4)
    CompositeEnd,     // composite into the frame
    TemporalEnd,      // TAA resolve (A5)
    SharpenEnd,       // TAA sharpen or the luma sharpen (A5, B11)
    DebugViewEnd,     // a diagnostic view of a post buffer
    PostEnd,          // after the last stage that ran
    FrameEnd,         // before the swap
    Count
};

enum class Section : int
{
    Frame,
    World, // includes the shadow pass
    Shadow,
    // The world pass in three parts: WorldBegin to GroundEnd (sky, terrain,
    // water, horizon), GroundEnd to Pass1End (the opaque objects), Pass1End to
    // WorldEnd (the shadow-map pass, the alpha objects, the rest).
    Ground,
    Pass1,
    Rest,
    Sky,      // the remastered sky dome, with the table build when it runs
    SkyTable, // the sky-view table build (frames that rebuild it)
    Canopy,   // the canopy's sun-visibility map build (frames that rebuild it)
    CombatMask, // the combat shadow mask's trace and blur (frames that shadow a combat light)
    // One per post stage, in Mark order: from the end of the latest stage that
    // ran before it (or WorldEnd) to its own end.
    Resolve,
    LinearDepth,
    TagReadout,
    Ao,
    Ssr,
    Air,
    Bloom,
    Shafts,
    CombatScatter,
    Composite,
    Temporal,
    Sharpen,
    DebugView,
    Post, // WorldEnd to PostEnd: every stage
    Count
};

constexpr int kMarkCount = static_cast<int>(Mark::Count);
constexpr int kSectionCount = static_cast<int>(Section::Count);
// The backend records a frame's marks in a 32-bit mask.
static_assert(kMarkCount <= 32, "GpuTimings records the marks of a frame in a 32-bit mask");
// One post stage section per post stage mark, in the same order.
constexpr int kPostStageMarks = static_cast<int>(Mark::DebugViewEnd) - static_cast<int>(Mark::ResolveEnd) + 1;
static_assert(static_cast<int>(Section::DebugView) - static_cast<int>(Section::Resolve) + 1 == kPostStageMarks,
              "one post stage section per post stage mark");

const char* SectionName(Section s);
// The post stage section that ends at `m`; Section::Count for any other mark.
Section PostStageSection(Mark m);

namespace detail
{
constexpr std::array<float, kSectionCount> Unmeasured()
{
    std::array<float, kSectionCount> a{};
    for (float& v : a)
        v = -1.0f;
    return a;
}
} // namespace detail

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
    // The shortest frame since the last reset; false while never measured.
    // Other processes on the GPU only ever add time, so on a shared GPU the
    // minimum is closer to this program's own cost than the average.
    bool MinMs(Section s, float& ms) const;
    int Frames() const { return _frames; }

    // "frame 6.12 ms | world 4.80 (shadow 0.62) [ground 1.10 pass1 2.40 rest 1.30] | sky 0.05 (skytable
    // 0.02) | combatmask 0.40 | post 0.71: resolve 0.04 ao 0.27 bloom 0.21 shafts 0.05 composite 0.14 | min frame=5.80
    // world=4.55 ground=1.02 pass1=2.21 rest=1.21": the post stages in the order they run, each once it has
    // been measured; the minimum fields use '=' so a parser looking for "<section> <ms>" never reads them.
    std::string Summary() const;

  private:
    std::array<float, kSectionCount> _avg{};
    std::array<float, kSectionCount> _min{};
    std::array<float, kSectionCount> _last = detail::Unmeasured();
    std::array<int, kSectionCount> _count{};
    int _frames = 0;
};

} // namespace Poseidon::gpu
