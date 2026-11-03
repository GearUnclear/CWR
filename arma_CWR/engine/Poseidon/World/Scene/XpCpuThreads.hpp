#pragma once

// XP-cputhreads (overnight experiment, default on in graphics-experimental): parts of the CPU render
// preparation run on the process TaskPool's workers (Core/TaskPool.hpp) while
// the frame stays exactly what the serial code draws.
//
//   POSEIDON_XP_CPUTHREADS         0: off (the tip's serial code).
//                                  unset, empty, 1 or all: every part (the
//                                  default). Otherwise a list of
//                                  part names: sort, hill, shadow, lod, prep.
//   POSEIDON_XP_CPUTHREADS_VERIFY  1: also run the serial code beside each
//                                  parallel part and count the differences
//                                  (logged; slower, for proof runs only).
//   POSEIDON_XP_CPUTHREADS_LOG     1: log the timed stages every 256 frames,
//                                  on or off, so one exe compares both sides.
//
// Parts:
//   sort    the four draw-list sorts (DrawListSort::SortByKeysParallel)
//   hill    the hill cull's horizon tests (TerrainOcclusion TestConcurrent)
//   shadow  the shadow-map pass's copies of cached static casters
//   lod     Scene::AdjustComplexity's per-object covered areas and LOD finds
//   prep    the per-object decisions of the object queue (ObjectForDrawing)

namespace Poseidon
{
class TaskPool;
}

namespace Poseidon::XpCpuThreads
{

enum Part : unsigned
{
    kSort = 1u,
    kHill = 2u,
    kShadow = 4u,
    kLod = 8u,
    kPrep = 16u,
};

// The parts the environment turned on (read once).
unsigned Parts();
inline bool On(Part p)
{
    return (Parts() & p) != 0;
}
bool VerifyOn();
bool LogOn();

// The process pool when part p is on and the pool exists, else nullptr.
TaskPool* Pool(Part p);

enum class Stage : int
{
    SortOccluders,
    SortShape,
    SortSurface,
    SortAlpha,
    Hill,
    ShadowCollect,
    Lod,
    Prep,
    // timed only (no parallel part): where the rest of the main thread goes
    Pass1Loop,    // Pass1's opaque draw loop (runs and draws)
    Pass2Surface, // Pass2's road and decal blend loop
    Pass2Alpha,   // Pass2's alpha loop
    ShadowRender, // the shadow-map pass after the caster collection (cascades and GL)
    Count
};

void AddStage(Stage s, double ms);
void AddMismatches(Stage s, int n);
// Once per frame (Scene::DrawObjectsAndShadowsPass1): every 256 frames logs
// "XPCT stages ..." with each stage's mean and max ms when logging is on.
void FrameTick();

// Times one stage while logging is on.
class StageTimer
{
  public:
    explicit StageTimer(Stage s);
    ~StageTimer();
    void Stop(); // ends the stage early (once)
    StageTimer(const StageTimer&) = delete;
    StageTimer& operator=(const StageTimer&) = delete;

  private:
    Stage _s;
    long long _t0;
};

} // namespace Poseidon::XpCpuThreads
