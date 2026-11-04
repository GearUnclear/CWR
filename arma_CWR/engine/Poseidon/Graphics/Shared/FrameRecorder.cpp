#include <Poseidon/Graphics/Shared/FrameRecorder.hpp>

#include <cstdio>

namespace Poseidon::framerec
{

RecorderControl& Recorder()
{
    static RecorderControl control;
    return control;
}

bool ArmRecorder(const std::string& path, int frames, int mode, std::string* why)
{
    RecorderControl& r = Recorder();
    auto refuse = [why](const char* text)
    {
        if (why)
            *why = text;
        return false;
    };
    if (r.state == RecordState::Recording)
        return refuse("busy");
    if (path.empty())
        return refuse("empty path");
    if (frames <= 0)
        return refuse("frames must be positive");
    if (mode < static_cast<int>(RecordMode::EveryFrame) || mode > static_cast<int>(RecordMode::EveryFrameHalf))
        return refuse("mode must be 0 (every frame), 1 (triScreenshot frames) or 2 (every frame, half size)");
    r.state = RecordState::Recording;
    r.mode = static_cast<RecordMode>(mode);
    r.path = path;
    r.want = frames;
    r.got = 0;
    r.stopRequested = false;
    r.failReason.clear();
    ++r.serial;
    return true;
}

void RequestRecorderStop()
{
    RecorderControl& r = Recorder();
    if (r.state == RecordState::Recording)
        r.stopRequested = true;
}

void MarkRecorderFailed(const char* reason)
{
    RecorderControl& r = Recorder();
    r.state = RecordState::Failed;
    r.stopRequested = false;
    r.failReason = reason ? reason : "unknown";
}

void MarkRecorderWritten(int frames)
{
    RecorderControl& r = Recorder();
    r.state = RecordState::Written;
    r.stopRequested = false;
    r.got = frames;
}

const char* RecordStateName(RecordState s)
{
    switch (s)
    {
        case RecordState::Idle:
            return "idle";
        case RecordState::Recording:
            return "recording";
        case RecordState::Written:
            return "written";
        case RecordState::Failed:
            return "failed";
    }
    return "unknown";
}

bool IsHalfMode(RecordMode m)
{
    return m == RecordMode::EveryFrameHalf;
}

bool RecordsFrame(RecordMode m, bool screenshotFrame)
{
    return m != RecordMode::ShotFramesOnly || screenshotFrame;
}

int OutWidth(RecordMode m, int w)
{
    return IsHalfMode(m) ? w / 2 : w;
}

int OutHeight(RecordMode m, int h)
{
    return IsHalfMode(m) ? h / 2 : h;
}

std::uint64_t RecordBytes(RecordMode m, int w, int h, int frames)
{
    if (w <= 0 || h <= 0 || frames <= 0)
        return 0;
    return static_cast<std::uint64_t>(OutWidth(m, w)) * static_cast<std::uint64_t>(OutHeight(m, h)) * 3ull *
           static_cast<std::uint64_t>(frames);
}

void PackFrame(const std::uint8_t* rgba, int w, int h, bool half, std::uint8_t* rgbOut)
{
    const std::size_t sw = static_cast<std::size_t>(w);
    if (!half)
    {
        const std::size_t n = sw * static_cast<std::size_t>(h);
        for (std::size_t i = 0; i < n; ++i)
        {
            rgbOut[i * 3 + 0] = rgba[i * 4 + 0];
            rgbOut[i * 3 + 1] = rgba[i * 4 + 1];
            rgbOut[i * 3 + 2] = rgba[i * 4 + 2];
        }
        return;
    }
    const std::size_t ow = static_cast<std::size_t>(w / 2), oh = static_cast<std::size_t>(h / 2);
    for (std::size_t y = 0; y < oh; ++y)
        for (std::size_t x = 0; x < ow; ++x)
            for (std::size_t c = 0; c < 3; ++c)
            {
                const std::size_t a = ((2 * y) * sw + 2 * x) * 4 + c;
                const std::size_t b = ((2 * y + 1) * sw + 2 * x) * 4 + c;
                rgbOut[(y * ow + x) * 3 + c] =
                    static_cast<std::uint8_t>((rgba[a] + rgba[a + 4] + rgba[b] + rgba[b + 4] + 2) / 4);
            }
}

std::string SidecarHeader(int outW, int outH, int frames, RecordMode m)
{
    char buf[160];
    std::snprintf(buf, sizeof(buf), "w=%d h=%d frames=%d mode=%d half=%d rows=bottom-up format=rgb24", outW, outH,
                  frames, static_cast<int>(m), IsHalfMode(m) ? 1 : 0);
    return buf;
}

} // namespace Poseidon::framerec
