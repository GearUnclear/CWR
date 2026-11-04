#include <Poseidon/Graphics/Rendering/Frame/RuntimeChecks.hpp>

namespace Poseidon
{

namespace render::frame
{

std::optional<RuntimeViolation> DetectBlackFrame(bool had3DContent, const std::uint8_t centerPixelRGB[3], int threshold)
{
    if (!had3DContent)
        return std::nullopt;

    const int r = centerPixelRGB[0];
    const int g = centerPixelRGB[1];
    const int b = centerPixelRGB[2];
    if (r <= threshold && g <= threshold && b <= threshold)
    {
        RuntimeViolation v;
        v.ruleId = "I-29";
        v.detail = "scene had 3D draws but center pixel RGB=(" + std::to_string(r) + "," + std::to_string(g) + "," +
                   std::to_string(b) + ") <= " + std::to_string(threshold);
        return v;
    }
    return std::nullopt;
}

void BlackFrameTracker::ExpectSample(std::uint32_t frame)
{
    Event e;
    e.frame = frame;
    e.kind = Kind::Sample;
    _events.push_back(e);
}

void BlackFrameTracker::NoContent(std::uint32_t frame)
{
    Event e;
    e.frame = frame;
    e.kind = Kind::NoContent;
    e.state = State::Resolved;
    _events.push_back(e);
}

void BlackFrameTracker::Resolve(std::uint32_t frame, const std::uint8_t rgb[3])
{
    for (Event& e : _events)
    {
        if (e.kind != Kind::Sample || e.state != State::Waiting)
            continue;
        const auto age = static_cast<std::int32_t>(e.frame - frame);
        if (age > 0)
            break; // newer than this result: still in flight
        if (age == 0)
        {
            e.rgb[0] = rgb[0];
            e.rgb[1] = rgb[1];
            e.rgb[2] = rgb[2];
            e.state = State::Resolved;
            break;
        }
        // Older than this result: results arrive in queue order, so its
        // readback was lost.
        e.state = State::Dropped;
    }
}

std::vector<BlackFrameTracker::Log> BlackFrameTracker::Advance(std::uint32_t nowFrame)
{
    std::vector<Log> out;
    while (!_events.empty())
    {
        Event& e = _events.front();
        if (e.kind == Kind::NoContent)
        {
            _consecutive = 0;
            _events.pop_front();
            continue;
        }
        if (e.state == State::Waiting)
        {
            if (static_cast<std::int32_t>(nowFrame - e.frame) <= static_cast<std::int32_t>(kMaxDelayFrames))
                break; // its result may still come: later events wait behind it
            e.state = State::Dropped;
        }
        if (e.state == State::Resolved)
        {
            if (auto vio = DetectBlackFrame(true, e.rgb))
            {
                ++_consecutive;
                if (_consecutive >= kThreshold && _logs < kLogCap)
                {
                    out.push_back({std::move(*vio), _consecutive});
                    ++_logs;
                }
            }
            else
            {
                _consecutive = 0;
            }
        }
        _events.pop_front();
    }
    return out;
}

std::optional<RuntimeViolation> DetectMissingMeshHandles(unsigned int tlDrawCount, unsigned int tlDrawsWithZeroVao)
{
    if (tlDrawsWithZeroVao == 0)
        return std::nullopt;
    RuntimeViolation v;
    v.ruleId = "I-22";
    v.detail = std::to_string(tlDrawsWithZeroVao) + " of " + std::to_string(tlDrawCount) +
               " TL draws reached the frame layer with mesh.vao == 0";
    return v;
}

std::optional<RuntimeViolation> DetectViewportMismatch(int expectedX, int expectedY, int expectedW, int expectedH,
                                                       const int liveRect[4], int tolerancePx)
{
    auto absDiff = [](int a, int b) { return a > b ? a - b : b - a; };
    if (absDiff(expectedX, liveRect[0]) > tolerancePx || absDiff(expectedY, liveRect[1]) > tolerancePx ||
        absDiff(expectedW, liveRect[2]) > tolerancePx || absDiff(expectedH, liveRect[3]) > tolerancePx)
    {
        RuntimeViolation v;
        v.ruleId = "I-24";
        v.detail = "GL viewport (" + std::to_string(liveRect[0]) + "," + std::to_string(liveRect[1]) + "," +
                   std::to_string(liveRect[2]) + "," + std::to_string(liveRect[3]) + ") != SceneInputs (" +
                   std::to_string(expectedX) + "," + std::to_string(expectedY) + "," + std::to_string(expectedW) + "," +
                   std::to_string(expectedH) + ")";
        return v;
    }
    return std::nullopt;
}

std::optional<RuntimeViolation> DetectBindCacheDivergence(int unit, unsigned int cachedHandle,
                                                          unsigned int liveGLHandle)
{
    if (cachedHandle == liveGLHandle)
        return std::nullopt;
    RuntimeViolation v;
    v.ruleId = "B-007";
    v.detail = "bind cache says unit " + std::to_string(unit) + " holds texture " + std::to_string(cachedHandle) +
               " but GL has " + std::to_string(liveGLHandle) + " (deleted/recycled handle or bind outside the cache)";
    return v;
}

} // namespace render::frame

} // namespace Poseidon
