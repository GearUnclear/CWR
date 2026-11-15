#pragma once

#include <cstdint>
#include <memory>
#include <string>

// Frame-cost counters of the object pass and the constant uploads: how many
// objects the instanced runs grouped, how many the hill cull hid, what the
// horizon table cost, how many uniform-buffer uploads the mirrors skipped. The scene and the GL33 backend count into
// the frame being drawn; World::Simulate closes the frame next to the frame profiler. triPerfStats prints the averages
// over the profiler's window, so they cover the same frames as its CPU phases (triPerfReset restarts both). Pure
// bookkeeping, no engine state. The window is the last kWindow closed frames, the frame profiler's ring
// (Dev::FrameProfiler::kRingSize), so a long window still covers the same frames as its CPU phases.
//
// The perf squad's counters (PERF-PLAN P0-15) follow the original ones: the geometry and bind counters, the
// instanced-run refusals, the pipeline statistics, the simulation counters (triSimStats) and the CPU sub-scopes
// (CpuScopes, triCpuStats). A counter reads 0 until its owner adds the increment at its site; the sites are listed
// in the perf squad's INFRA-C1 notes. Counting is a plain add, on in every build; the CPU sub-scopes and the
// millisecond fields of the simulation counters are timed only while triCpuStats is on.

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

    // --- P0-15 (e): land fits, vertex buffer rewrites, occlusion, cloudlets, binds (M2's scratch counters) ---
    int landFitDraws = 0;     // Object::Animate: draws that fitted ClipLandKeep/ClipLandOn points to the terrain
    int vbRewrites = 0;       // static vertex buffers rewritten (VertexBufferGL33::CopyVertices, not dynamic)
    float vbRewriteKB = 0.0f; // their vertex bytes, kB
    int vbDynUploads = 0;     // dynamic vertex buffer uploads (CopyVertices, dynamic)
    float vbDynKB = 0.0f;     // their vertex bytes, kB
    int occTested = 0;        // objects tested against the occluders (Pass1's object occlusion)
    int occCulled = 0;        // of those, hidden
    int cloudlets = 0;        // whole-alpha objects drawn in Pass2 (passNum 2: cloudlets, objects under 0xd0)
    int programBinds = 0;     // glUseProgram calls that changed the program
    int textureBinds = 0;     // texture unit binds
    int samplerBinds = 0;     // sampler object binds
    int uboRangeBinds = 0;    // glBindBufferRange on a uniform block
    // Instanced-run refusals by the first rule that failed (RunRefusal below)
    int refuseScene = 0;       // a scene or routing gate (lights, proxy, OnSurface, IsColored, camera object)
    int refuseStatic = 0;      // not Static()
    int refuseOwnVertices = 0; // its draw rewrites its own vertices (land fit, damage, animation)
    int refuseLit = 0;         // a local light lights it
    int refuseShape = 0;       // another shape than the run's head
    int refuseLod = 0;         // another draw level
    int refusePass = 0;        // another pass
    int refuseSpecial = 0;     // other render flags
    int refuseFogBand = 0;     // outside the head's fog band
    // --- P0-15 (c): geometry ---
    int tris = 0;        // triangles submitted by the world's draws
    int lodSwitches = 0; // objects whose draw level differs from their last frame's
    // Draws per draw level (CountLod): level 0 .. 6, and 7 or coarser.
    int lodHist0 = 0;
    int lodHist1 = 0;
    int lodHist2 = 0;
    int lodHist3 = 0;
    int lodHist4 = 0;
    int lodHist5 = 0;
    int lodHist6 = 0;
    int lodHist7 = 0;
    // GL_ARB_pipeline_statistics_query (triPipeStats, gpu::SetPipelineStatsWanted): the ground and the
    // opaque-object spans of the world pass, added by the frame that read them back (pipeGround / pipePass1 = 1).
    int pipeGround = 0;
    int pipePass1 = 0;
    float groundPrims = 0.0f;   // GL_PRIMITIVES_SUBMITTED
    float groundClipIn = 0.0f;  // GL_CLIPPING_INPUT_PRIMITIVES
    float groundClipOut = 0.0f; // GL_CLIPPING_OUTPUT_PRIMITIVES
    float groundVs = 0.0f;      // GL_VERTEX_SHADER_INVOCATIONS
    float groundFs = 0.0f;      // GL_FRAGMENT_SHADER_INVOCATIONS
    float pass1Prims = 0.0f;
    float pass1ClipIn = 0.0f;
    float pass1ClipOut = 0.0f;
    float pass1Vs = 0.0f;
    float pass1Fs = 0.0f;
    // --- P0-15 (f): simulation (triSimStats) ---
    int fogSightTarget = 0;     // AI sight through the fog (FogSightTransmittance) from target tracking
    int fogSightScript = 0;     // ... from scripts and tri verbs
    int fogSightOther = 0;      // ... from anywhere else
    int fogMarches = 0;         // fog-sight ray marches
    int fogMarchesLandVis0 = 0; // of those, with the landscape visibility already 0
    int fogOutOfCone = 0;       // sight checks outside the observer's cone
    int combatLightRecords = 0; // combat lights recorded this frame (CombatLights)
    float combatLightMs = 0.0f; // CPU: their records and upkeep (timed while triCpuStats is on)
    int decalCalls = 0;         // Object::DrawDecal calls
    float decalMs = 0.0f;       // CPU: Object::DrawDecal (timed while triCpuStats is on)
    int simCloudlets = 0;       // cloudlets simulated (smoke, dust)
    int simHashes = 0;          // spatial hash inserts and lookups of the simulation
    int sensorUpdates = 0;      // SensorList::SmartUpdateAll calls
    float sensorMs = 0.0f;      // CPU: SensorList::SmartUpdateAll, outside every profiled phase today
};

// The first rule an object failed to join or head an instanced run
// (InstancedRunRules.hpp CanHead / CanJoin, plus the caller's scene gates).
enum class RunRefusal : int
{
    Scene,
    Static,
    OwnVertices,
    Lit,
    Shape,
    Lod,
    Pass,
    Special,
    FogBand,
    Count
};
// Adds one refusal to the frame's counter of rule r.
void CountRunRefusal(FrameCounters& c, RunRefusal r);
// Adds one draw at draw level `lod` to the frame's level histogram (7 or more: the last bucket; below 0: ignored).
void CountLod(FrameCounters& c, int lod);

// Who asked for AI sight through the fog.
enum class FogSightCaller : int
{
    Target,
    Script,
    Other
};
void CountFogSight(FrameCounters& c, FogSightCaller who);

class Counters
{
  public:
    // The window: as many frames as the frame profiler's ring holds.
    static constexpr int kWindow = 256;

    Counters();
    // The frame being drawn: the scene and the backend add to it.
    FrameCounters& Frame() { return _frame; }
    // The last closed frame.
    const FrameCounters& Last() const { return _last; }
    // Closes the frame: it becomes Last() and joins the window (the oldest
    // frame leaves it once it holds kWindow frames). Closes the CPU sub-scopes'
    // frame too (GCpuScopes, when on).
    void EndFrame();
    // Starts a new window (triPerfReset).
    void ResetWindow();
    int WindowFrames() const { return _count; }
    // The window's frames: newest = WindowFrame(0), oldest = WindowFrame(WindowFrames() - 1).
    const FrameCounters& WindowFrame(int back) const { return _ring[(_head - 1 - back + 2 * kWindow) % kWindow]; }
    // Per-frame averages over the window, for triPerfStats:
    // "objs opaque=<n> runs=<n> runobj=<n> litobj=<n> lights=<slots>/<on> | hill ran=<share> idle=<share>
    //  tested=<n> hidden=<n> kept=<n> builds=<n> wedges=<n> hillcpu=<ms> | prep lod=<ms> occ=<ms> |
    //  ubo lit=<uploads>/<skips> litKB=<kB> vs=<uploads>/<skips> vsKB=<kB> | tiles lights=<n> refs=<n>
    //  tilecpu=<ms> | geo landfit=<n> vbrw=<n> vbKB=<kB> vbdyn=<n> vbdynKB=<kB> occT=<n> occC=<n> cloudlets=<n>
    //  | binds prog=<n> tex=<n> smp=<n> uboB=<n> | refuse=<scene>/<static>/<own>/<lit>/<shape>/<lod>/<pass>/
    //  <special>/<fog> | tris=<n> lodsw=<n> lodhist=<l0>/.../<l7+> | pipe=<ground frames>/<pass1 frames>
    //  gprims=<n> gclipin=<n> gclipout=<n> gvs=<n> gfs=<n> p1prims=<n> p1clipin=<n> p1clipout=<n> p1vs=<n>
    //  p1fs=<n>" (the pipeline fields average over the frames that read a result back, the rest over every
    //  frame of the window).
    std::string Summary() const;
    // Per-frame averages of the simulation counters, for triSimStats:
    // "fogsight target=<n> script=<n> other=<n> marches=<n> landvis0=<n> outcone=<n> | combat records=<n>
    //  cpu=<ms> | decals calls=<n> cpu=<ms> | cloudlets=<n> hashes=<n> | sensors updates=<n> cpu=<ms>
    //  | n=<window frames>".
    std::string SimSummary() const;

    // One per FrameCounters member (PerfCounters.cpp lists them; a static_assert
    // there keeps the count).
    static constexpr int kFields = 81;

  private:
    // Sums over the window, in double so a long window does not round.
    void WindowSums(double (&sum)[kFields]) const;

    FrameCounters _frame;
    FrameCounters _last;
    std::unique_ptr<FrameCounters[]> _ring; // kWindow frames
    int _head = 0;
    int _count = 0;
};

Counters& GPerfCounters();

// --- P0-15 (b): CPU sub-scopes (triCpuStats) ---

// The parts of the frame the CPU phases of the frame profiler do not split.
// The Object* scopes are per object class, timed on one object in
// CpuScopes::kObjectSampleEvery (CpuObjectSample) and scaled up by that factor.
enum class CpuScope : int
{
    OcclusionTest,   // Pass1: the object occlusion tests
    OcclusionRender, // Pass1: drawing the occluders
    RunBuild,        // Pass1: building the instanced runs
    DrawLoop,        // Pass1: the opaque draw loop
    Grass,           // the grass (clutter) pass
    Pass2Sort,       // Pass2: sorting the alpha objects
    Pass2Draw,       // Pass2: drawing them
    ShadowCollect,   // the shadow-map casters' collection
    ShadowGL,        // the shadow-map depth pass's GL work
    Census,          // the per-frame object census
    ObjBuilding,     // one object's draw, sampled: buildings
    ObjVegetation,   // trees and bushes
    ObjVehicle,      // vehicles
    ObjMan,          // soldiers and civilians
    ObjOther,        // anything else
    Count
};

class CpuScopes
{
  public:
    static constexpr int kCount = static_cast<int>(CpuScope::Count);
    static constexpr int kObjectSampleEvery = 16;
    static constexpr int kWindow = 256;

    CpuScopes();
    // On (true) starts a new window; off stops timing.
    void SetEnabled(bool on);
    bool Enabled() const { return _enabled; }
    // A scope's ticks (Now() units); weight scales a sampled scope up.
    void Add(CpuScope s, std::uint64_t ticks, int weight = 1);
    // Closes the frame (Counters::EndFrame calls it while on).
    void EndFrame();
    void ResetWindow();
    int WindowFrames() const { return _count; }
    // The frame's ms of scope s, newest = back 0.
    float WindowMs(int back, CpuScope s) const;
    // "<name>=<avg ms>/<p95 ms>/<calls per frame> ..." over the window, then " frames=<n>"
    // ("off" while off and never measured).
    std::string Summary() const;
    static const char* Name(CpuScope s);

    // The clock: the TSC on x86-64, steady_clock nanoseconds elsewhere.
    static std::uint64_t Now();
    // Ticks per millisecond, calibrated against steady_clock since SetEnabled(true); tests fix it.
    double TicksPerMs() const { return _ticksPerMs; }
    void FixTicksPerMs(double ticksPerMs);

  private:
    struct FrameRecord
    {
        float ms[kCount];
        int calls[kCount];
    };
    std::unique_ptr<FrameRecord[]> _ring;
    std::uint64_t _ticks[kCount] = {};
    int _calls[kCount] = {};
    int _head = 0;
    int _count = 0;
    double _ticksPerMs = 0.0;
    bool _fixed = false;
    bool _enabled = false;
    std::uint64_t _calTicks = 0;
    std::int64_t _calNs = 0;
};

CpuScopes& GCpuScopes();
// Read inline by the timers below: one load and a branch while off.
extern bool gCpuScopesOn;

// Times one scope while triCpuStats is on.
class CpuScopeTimer
{
  public:
    explicit CpuScopeTimer(CpuScope s, int weight = 1)
        : _s(s), _weight(weight), _t0(gCpuScopesOn ? CpuScopes::Now() : 0)
    {
    }
    ~CpuScopeTimer()
    {
        if (_t0)
            GCpuScopes().Add(_s, CpuScopes::Now() - _t0, _weight);
    }
    CpuScopeTimer(const CpuScopeTimer&) = delete;
    CpuScopeTimer& operator=(const CpuScopeTimer&) = delete;

  private:
    CpuScope _s;
    int _weight;
    std::uint64_t _t0;
};

// True for one object in CpuScopes::kObjectSampleEvery (a running count), and
// only while triCpuStats is on: wrap that object's draw in
// CpuScopeTimer(CpuScope::Obj..., CpuScopes::kObjectSampleEvery).
bool CpuObjectSample();

// Adds the elapsed milliseconds to `field` (a FrameCounters ms field) while
// triCpuStats is on.
class CpuMsTimer
{
  public:
    explicit CpuMsTimer(float& field) : _field(field), _t0(gCpuScopesOn ? CpuScopes::Now() : 0) {}
    ~CpuMsTimer();
    CpuMsTimer(const CpuMsTimer&) = delete;
    CpuMsTimer& operator=(const CpuMsTimer&) = delete;

  private:
    float& _field;
    std::uint64_t _t0;
};

} // namespace Poseidon::perf
