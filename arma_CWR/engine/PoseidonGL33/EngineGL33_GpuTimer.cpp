// GPU timestamp queries (Engine::SetGpuTimingEnabled): a timestamp at fixed
// points of the frame, read back kGpuFrames frames later. A result the GPU has
// not produced yet is dropped instead of waited for, so measuring never stalls
// the frame. The durations and their averages live in Poseidon::gpu::GpuTimings.
// While timing is on and Poseidon::gpu::PipelineStatsWanted() (triPipeStats),
// the same marks also bracket GL_ARB_pipeline_statistics_query sets around the
// world pass's ground and opaque-object spans, read back the same way into the
// frame-cost counters (PerfCounters.hpp).

#include <PoseidonGL33/EngineGL33.hpp>

#include <Poseidon/Graphics/Core/PerfCounters.hpp>

#include <cstring>

using Poseidon::gpu::kMarkCount;
using Poseidon::gpu::Mark;

namespace
{
// Slots whose frame was being recorded when the window restarted: their stamps
// straddle the restart, so the read-back drops them instead of submitting them.
std::uint32_t s_gpuDropSlots = 0;

// Pipeline statistics (triPipeStats): per frame slot, a query set per span
// (0 = ground: WorldBegin to GroundEnd, 1 = opaque objects: GroundEnd to
// Pass1End). Only one query per target may be active, and the two spans follow
// each other, so one span is open at a time.
constexpr int kPipeSlots = EngineGL33::kGpuFrames;
constexpr int kPipeSpans = 2;
constexpr int kPipeQueries = 5;
constexpr GLenum kPipeTargets[kPipeQueries] = {GL_PRIMITIVES_SUBMITTED_ARB, GL_CLIPPING_INPUT_PRIMITIVES_ARB,
                                               GL_CLIPPING_OUTPUT_PRIMITIVES_ARB, GL_VERTEX_SHADER_INVOCATIONS_ARB,
                                               GL_FRAGMENT_SHADER_INVOCATIONS_ARB};
GLuint s_pipeQ[kPipeSlots][kPipeSpans][kPipeQueries] = {};
std::uint32_t s_pipeDone[kPipeSlots] = {}; // bit k: span k began and ended at its own marks
bool s_pipeActive = false;                 // this frame samples them
int s_pipeOpen = -1;                       // the span whose queries are active, or -1
int s_pipeOpenSlot = 0;

void PipeEnd(bool complete)
{
    if (s_pipeOpen < 0)
        return;
    for (GLenum target : kPipeTargets)
        glEndQuery(target);
    if (complete)
        s_pipeDone[s_pipeOpenSlot] |= 1u << s_pipeOpen;
    s_pipeOpen = -1;
}

void PipeBegin(int slot, int span)
{
    for (int k = 0; k < kPipeQueries; ++k)
        glBeginQuery(kPipeTargets[k], s_pipeQ[slot][span][k]);
    s_pipeOpen = span;
    s_pipeOpenSlot = slot;
}

// At a timestamp mark of the frame in `slot`: open or close the spans.
void PipeAtMark(int slot, Mark mark)
{
    switch (mark)
    {
        case Mark::WorldBegin:
            PipeEnd(false);
            PipeBegin(slot, 0);
            break;
        case Mark::GroundEnd:
        {
            const bool ground = s_pipeOpen == 0;
            PipeEnd(ground);
            if (ground)
                PipeBegin(slot, 1);
            break;
        }
        case Mark::Pass1End:
            PipeEnd(s_pipeOpen == 1);
            break;
        case Mark::WorldEnd:
        case Mark::FrameEnd:
            PipeEnd(false);
            break;
        default:
            break;
    }
}

// Reads the slot's finished spans into the counters of the frame being drawn;
// a span the GPU has not finished is dropped, never waited for.
void PipeReadBack(int slot)
{
    if (!s_pipeDone[slot])
        return;
    Poseidon::perf::FrameCounters& c = Poseidon::perf::GPerfCounters().Frame();
    for (int span = 0; span < kPipeSpans; ++span)
    {
        if (!((s_pipeDone[slot] >> span) & 1u))
            continue;
        GLuint64 v[kPipeQueries] = {};
        bool ready = true;
        for (int k = 0; k < kPipeQueries && ready; ++k)
        {
            GLint available = 0;
            glGetQueryObjectiv(s_pipeQ[slot][span][k], GL_QUERY_RESULT_AVAILABLE, &available);
            ready = available != 0;
            if (ready)
                glGetQueryObjectui64v(s_pipeQ[slot][span][k], GL_QUERY_RESULT, &v[k]);
        }
        if (!ready)
            continue;
        float* const out[2][kPipeQueries] = {
            {&c.groundPrims, &c.groundClipIn, &c.groundClipOut, &c.groundVs, &c.groundFs},
            {&c.pass1Prims, &c.pass1ClipIn, &c.pass1ClipOut, &c.pass1Vs, &c.pass1Fs}};
        for (int k = 0; k < kPipeQueries; ++k)
            *out[span][k] += static_cast<float>(v[k]);
        ++(span == 0 ? c.pipeGround : c.pipePass1);
    }
    s_pipeDone[slot] = 0;
}

void PipeDestroy(bool haveContext)
{
    if (s_pipeQ[0][0][0] && haveContext)
    {
        if (s_pipeOpen >= 0)
            PipeEnd(false);
        glDeleteQueries(kPipeSlots * kPipeSpans * kPipeQueries, &s_pipeQ[0][0][0]);
    }
    std::memset(s_pipeQ, 0, sizeof(s_pipeQ));
    std::memset(s_pipeDone, 0, sizeof(s_pipeDone));
    s_pipeOpen = -1;
    s_pipeActive = false;
}
} // namespace

void EngineGL33::SetGpuTimingEnabled(bool enabled)
{
    if (enabled == _gpuTiming)
    {
        if (!enabled)
            return;
        // Already on: start a new window. The frames still in flight were
        // issued before it, so they are dropped too: the in-flight slots are
        // forgotten and the frame being recorded is dropped at its read-back.
        _gpuTimings.Reset();
        Poseidon::gpu::GGpuFrameRing().Reset();
        for (int i = 0; i < kGpuFrames; ++i)
            if (i != _gpuSlot)
                _gpuRecorded[i] = 0;
        s_gpuDropSlots = _gpuRecorded[_gpuSlot] ? 1u << _gpuSlot : 0u;
        std::memset(s_pipeDone, 0, sizeof(s_pipeDone));
        LOG_DEBUG(Graphics, "GL33: GPU timing window restarted");
        return;
    }
    _gpuTiming = enabled;
    _gpuTimings.Reset();
    Poseidon::gpu::GGpuFrameRing().Reset();
    s_gpuDropSlots = 0;
    if (!enabled)
        DestroyGpuQueries();
    LOG_INFO(Graphics, "GL33: GPU timing {}", enabled ? "on" : "off");
}

std::string EngineGL33::GpuTimingSummary() const
{
    if (!_gpuTiming)
        return "gpu: timing off";
    return _gpuTimings.Summary();
}

bool EngineGL33::GpuTimingSectionMs(const char* name, float& ms) const
{
    if (!_gpuTiming || !name)
        return false;
    for (int i = 0; i < Poseidon::gpu::kSectionCount; ++i)
    {
        const auto s = static_cast<Poseidon::gpu::Section>(i);
        if (std::strcmp(Poseidon::gpu::SectionName(s), name) == 0)
            return _gpuTimings.AverageMs(s, ms);
    }
    return false;
}

void EngineGL33::GpuMarkWorldPhase(WorldPhase phase)
{
    // Only inside the world's own pass (not a portrait or a 3D preview).
    if (!_worldPassOpen)
        return;
    GpuMark(phase == WorldPhase::GroundDrawn ? Mark::GroundEnd : Mark::Pass1End);
}

void EngineGL33::DestroyGpuQueries()
{
    if (_gpuQueries[0][0] && _glContext)
        glDeleteQueries(kGpuFrames * kMarkCount, &_gpuQueries[0][0]);
    std::memset(_gpuQueries, 0, sizeof(_gpuQueries));
    std::memset(_gpuRecorded, 0, sizeof(_gpuRecorded));
    PipeDestroy(_glContext != nullptr);
}

void EngineGL33::GpuFrameBegin()
{
    if (!_gpuTiming || !_glContext)
        return;
    if (!_gpuQueries[0][0])
        glGenQueries(kGpuFrames * kMarkCount, &_gpuQueries[0][0]);
    _gpuSlot = (_gpuSlot + 1) % kGpuFrames;
    // The slot about to be reused holds the frame issued kGpuFrames ago.
    const std::uint32_t issued = _gpuRecorded[_gpuSlot];
    if (issued)
    {
        std::uint64_t stamps[kMarkCount] = {};
        std::uint32_t read = 0;
        for (int m = 0; m < kMarkCount; ++m)
        {
            if (!((issued >> m) & 1u))
                continue;
            GLint available = 0;
            glGetQueryObjectiv(_gpuQueries[_gpuSlot][m], GL_QUERY_RESULT_AVAILABLE, &available);
            if (!available)
                continue;
            GLuint64 value = 0;
            glGetQueryObjectui64v(_gpuQueries[_gpuSlot][m], GL_QUERY_RESULT, &value);
            stamps[m] = value;
            read |= 1u << m;
        }
        if (s_gpuDropSlots & (1u << _gpuSlot))
            s_gpuDropSlots &= ~(1u << _gpuSlot);
        else
        {
            _gpuTimings.Submit(stamps, read);
            Poseidon::gpu::GGpuFrameRing().Push(_gpuTimings);
        }
    }
    _gpuRecorded[_gpuSlot] = 0;
    // Pipeline statistics: a span left open by a frame that never reached its
    // closing mark is dropped; the slot's finished spans are read back; then
    // this frame samples them only while they are wanted and supported.
    PipeEnd(false);
    PipeReadBack(_gpuSlot);
    const bool wantPipe = Poseidon::gpu::PipelineStatsWanted() && GLAD_GL_ARB_pipeline_statistics_query;
    if (wantPipe && !s_pipeQ[0][0][0])
        glGenQueries(kPipeSlots * kPipeSpans * kPipeQueries, &s_pipeQ[0][0][0]);
    else if (!wantPipe && s_pipeQ[0][0][0])
        PipeDestroy(true);
    s_pipeActive = wantPipe;
    GpuMark(Mark::FrameBegin);
}

void EngineGL33::GpuMark(Mark mark)
{
    if (!_gpuTiming || !_gpuQueries[0][0])
        return;
    const int m = static_cast<int>(mark);
    if ((_gpuRecorded[_gpuSlot] >> m) & 1u)
        return; // the first occurrence in a frame counts
    glQueryCounter(_gpuQueries[_gpuSlot][m], GL_TIMESTAMP);
    _gpuRecorded[_gpuSlot] |= 1u << m;
    if (s_pipeActive)
        PipeAtMark(_gpuSlot, mark);
}
