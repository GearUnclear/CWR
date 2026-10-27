#pragma once

#include <string>

// Frame-cost counters of the object pass and the constant uploads: how many
// objects the instanced runs grouped, how many the hill cull hid, what the
// horizon table cost, how many uniform-buffer uploads the mirrors skipped. The scene and the GL33 backend count into the frame
// being drawn; World::Simulate closes the frame next to the frame profiler.
// triPerfStats prints the averages over the profiler's window, so they cover
// the same frames as its CPU phases (triPerfReset restarts both). Pure
// bookkeeping, no engine state.

namespace Poseidon::perf
{

struct FrameCounters
{
    // Scene::DrawObjectsAndShadowsPass1
    int lightSlots = 0;    // Scene::NLights: light slots (a lamp that was ever on keeps one)
    int activeLights = 0;  // lights on this frame
    int opaqueObjects = 0; // objects through the opaque run loop
    int runs = 0;          // instanced runs drawn (four objects or more)
    int runObjects = 0;    // objects drawn in those runs
    int litObjects = 0;    // objects kept out of runs because their own light list is not empty
    // Hill culling (TerrainOcclusion.hpp)
    int hillRan = 0;          // 1 when the cull ran this frame
    int hillIdle = 0;         // 1 when the governor held it idle
    int hillTested = 0;       // objects far enough to test
    int hillHidden = 0;       // of those, hidden behind the terrain
    int hillKept = 0;         // of those, drawn anyway: the last draw before the shadow-map pass
    int hillBuilds = 0;       // horizon tables started (the eye moved)
    int hillWedges = 0;       // wedges built on demand
    float hillMs = 0.0f;      // CPU: table and tests
    float lodMs = 0.0f;       // CPU: Scene::AdjustComplexity (LOD selection)
    float occlusionMs = 0.0f; // CPU: the object occlusion pass
    // Uniform-buffer uploads (EngineGL33: LitConstants and VSConstants)
    int litUploads = 0;
    int litSkips = 0; // flushes the mirror found unchanged
    int litBytes = 0;
    int vsUploads = 0;
    int vsSkips = 0;
    int vsBytes = 0;
    // The tiled light list (FrameLights.hpp, EngineGL33::UpdateLightTiles), on
    // the frames that build one
    int tileLights = 0;  // lights in the frame's list
    int tileRefs = 0;    // tile entries: a light counts once per tile its reach covers
    float tileMs = 0.0f; // CPU: gather, rank, bin and upload
};

class Counters
{
  public:
    // The frame being drawn: the scene and the backend add to it.
    FrameCounters& Frame() { return _frame; }
    // The last closed frame.
    const FrameCounters& Last() const { return _last; }
    // Closes the frame: it becomes Last() and joins the window's sums.
    void EndFrame();
    // Starts a new window (triPerfReset).
    void ResetWindow();
    int WindowFrames() const { return _frames; }
    // Per-frame averages over the window, for triPerfStats:
    // "objs opaque=<n> runs=<n> runobj=<n> litobj=<n> lights=<slots>/<on> | hill ran=<share> idle=<share>
    //  tested=<n> hidden=<n> kept=<n> builds=<n> wedges=<n> hillcpu=<ms> | prep lod=<ms> occ=<ms> |
    //  ubo lit=<uploads>/<skips> litKB=<kB> vs=<uploads>/<skips> vsKB=<kB> | tiles lights=<n> refs=<n>
    //  tilecpu=<ms>"
    std::string Summary() const;

    // One per FrameCounters member (PerfCounters.cpp lists them; a static_assert
    // there keeps the count).
    static constexpr int kFields = 25;

  private:
    FrameCounters _frame;
    FrameCounters _last;
    // Sums over the window, in double so a long window does not round.
    double _sum[kFields] = {};
    int _frames = 0;
};

Counters& GPerfCounters();

} // namespace Poseidon::perf
