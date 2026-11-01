// GPU timestamp queries (Engine::SetGpuTimingEnabled): a timestamp at fixed
// points of the frame, read back kGpuFrames frames later. A result the GPU has
// not produced yet is dropped instead of waited for, so measuring never stalls
// the frame. The durations and their averages live in Poseidon::gpu::GpuTimings.

#include <PoseidonGL33/EngineGL33.hpp>

#include <cstring>

using Poseidon::gpu::kMarkCount;
using Poseidon::gpu::Mark;

namespace
{
// Slots whose frame was being recorded when the window restarted: their stamps
// straddle the restart, so the read-back drops them instead of submitting them.
std::uint32_t s_gpuDropSlots = 0;
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
}
