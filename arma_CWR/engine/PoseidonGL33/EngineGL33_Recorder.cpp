// P0-10 real-time frame recorder, the GL half (the state and the rules are in
// Poseidon/Graphics/Shared/FrameRecorder.hpp, the verbs triRecord and
// triRecordState in Game/Commands/GameStateExtTestRecord.cpp).
//
// BackToFront calls GL33RecordPresentedFrame right after
// CaptureScreenshotIfPending and before the swap, so a recorded frame is the
// image triScreenshot reads at present: the default framebuffer after the
// gamma pass and the overlay. Each frame is read into one of kRingDepth
// pixel-pack buffers and collected kRingDepth frames later (the read never
// waits on the GPU), repacked to RGB24 into one block of memory, and written
// out once after the last frame. Idle (play): one test and return.

#include <Poseidon/Foundation/Logging/Logging.hpp>
#include <Poseidon/Graphics/Shared/FrameRecorder.hpp>
#include <Poseidon/World/TriCamPath.hpp>

#include <glad/gl.h>
#include <SDL3/SDL.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace fr = Poseidon::framerec;

// Called from EngineGL33::BackToFront (EngineGL33_VertexBuffer.cpp, which
// declares it the same way).
void GL33RecordPresentedFrame(SDL_Window* window, int fallbackW, int fallbackH, bool screenshotFrame);

namespace
{

struct RecorderGL
{
    GLuint pbo[fr::kRingDepth] = {};
    int pboFrame[fr::kRingDepth] = {};
    int w = 0, h = 0, ow = 0, oh = 0, issued = 0;
    std::uint32_t serial = 0;
    // CRT malloc: the engine heap refuses blocks of several GB.
    std::uint8_t* data = nullptr;
    std::vector<double> wallMs;
    std::vector<float> pathT;
    std::vector<std::uint8_t> shot;
    std::vector<std::uint8_t> staging; // one RGBA frame read back from a ring buffer
    std::chrono::steady_clock::time_point t0;
};

RecorderGL gRec;

} // namespace

// File-local helpers. (DebugLog.hpp defines a Fail() macro, hence RecorderFail.)

static void Release()
{
    if (gRec.pbo[0])
        glDeleteBuffers(fr::kRingDepth, gRec.pbo);
    std::free(gRec.data);
    gRec = RecorderGL();
}

static void Collect(int slot)
{
    const int f = gRec.pboFrame[slot];
    if (f < 0)
        return;
    // glGetBufferSubData, as the camera meter readback does (the backend maps no buffer directly,
    // test_buffer_usage_audit I-21).
    glBindBuffer(GL_PIXEL_PACK_BUFFER, gRec.pbo[slot]);
    const GLsizeiptr bytes = static_cast<GLsizeiptr>(gRec.w) * gRec.h * 4;
    gRec.staging.resize(static_cast<std::size_t>(bytes));
    glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, bytes, gRec.staging.data());
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    const fr::RecordMode m = fr::Recorder().mode;
    std::uint8_t* dst = gRec.data + static_cast<std::size_t>(f) * gRec.ow * gRec.oh * 3;
    fr::PackFrame(gRec.staging.data(), gRec.w, gRec.h, fr::IsHalfMode(m), dst);
    gRec.pboFrame[slot] = -1;
}

static void Finish()
{
    for (int s = 0; s < fr::kRingDepth; ++s)
        Collect(s);
    fr::RecorderControl& rc = fr::Recorder();
    const std::size_t bytes = static_cast<std::size_t>(gRec.ow) * gRec.oh * 3 * gRec.issued;
    bool ok = false;
    if (FILE* fp = std::fopen(rc.path.c_str(), "wb"))
    {
        ok = gRec.data && std::fwrite(gRec.data, 1, bytes, fp) == bytes;
        ok = std::fclose(fp) == 0 && ok;
    }
    const std::string meta = rc.path + ".txt";
    if (FILE* fp = std::fopen(meta.c_str(), "w"))
    {
        std::fprintf(fp, "%s\n", fr::SidecarHeader(gRec.ow, gRec.oh, gRec.issued, rc.mode).c_str());
        for (int i = 0; i < gRec.issued; ++i)
            std::fprintf(fp, "%d %.3f %.5f %d\n", i, gRec.wallMs[i], static_cast<double>(gRec.pathT[i]),
                         static_cast<int>(gRec.shot[i]));
        ok = std::fclose(fp) == 0 && ok;
    }
    else
        ok = false;
    const int frames = gRec.issued;
    LOG_INFO(Graphics, "GL33: recorder {} {} frames {}x{} to {}", ok ? "wrote" : "FAILED writing", frames, gRec.ow,
             gRec.oh, rc.path);
    Release();
    if (ok)
        fr::MarkRecorderWritten(frames);
    else
        fr::MarkRecorderFailed("write failed");
}

static void RecorderFail(const char* why)
{
    LOG_ERROR(Graphics, "GL33: recorder failed: {}", why);
    Release();
    fr::MarkRecorderFailed(why);
}

static bool Begin(int w, int h)
{
    fr::RecorderControl& rc = fr::Recorder();
    const std::uint64_t bytes = fr::RecordBytes(rc.mode, w, h, rc.want);
    if (bytes == 0 || bytes > fr::kMaxRecordBytes)
    {
        RecorderFail("recording too large for memory");
        return false;
    }
    gRec.w = w;
    gRec.h = h;
    gRec.ow = fr::OutWidth(rc.mode, w);
    gRec.oh = fr::OutHeight(rc.mode, h);
    gRec.serial = rc.serial;
    gRec.data = static_cast<std::uint8_t*>(std::malloc(static_cast<std::size_t>(bytes)));
    if (!gRec.data)
    {
        RecorderFail("out of memory");
        return false;
    }
    gRec.wallMs.assign(rc.want, 0.0);
    gRec.pathT.assign(rc.want, 0.0f);
    gRec.shot.assign(rc.want, 0);
    glGenBuffers(fr::kRingDepth, gRec.pbo);
    for (int s = 0; s < fr::kRingDepth; ++s)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, gRec.pbo[s]);
        glBufferData(GL_PIXEL_PACK_BUFFER, static_cast<GLsizeiptr>(w) * h * 4, nullptr, GL_STREAM_READ);
        gRec.pboFrame[s] = -1;
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    gRec.t0 = std::chrono::steady_clock::now();
    LOG_INFO(Graphics, "GL33: recorder armed {} frames mode {} {}x{} -> {}", rc.want, static_cast<int>(rc.mode),
             gRec.ow, gRec.oh, rc.path);
    return true;
}

void GL33RecordPresentedFrame(SDL_Window* window, int fallbackW, int fallbackH, bool screenshotFrame)
{
    fr::RecorderControl& rc = fr::Recorder();
    if (rc.state != fr::RecordState::Recording)
        return;
    if (gRec.issued > 0 && gRec.serial != rc.serial)
        Release(); // re-armed while a recording was open: start over
    if (rc.stopRequested)
    {
        if (gRec.issued > 0)
            Finish();
        else
        {
            Release();
            fr::MarkRecorderWritten(0);
        }
        return;
    }
    if (!fr::RecordsFrame(rc.mode, screenshotFrame))
        return;
    int w = fallbackW, h = fallbackH;
    if (window)
        SDL_GetWindowSizeInPixels(window, &w, &h);
    if (w <= 0 || h <= 0)
        return;
    if (gRec.issued == 0 && !Begin(w, h))
        return;
    if (w != gRec.w || h != gRec.h)
    {
        RecorderFail("window size changed while recording");
        return;
    }
    const int slot = gRec.issued % fr::kRingDepth;
    Collect(slot);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, gRec.pbo[slot]);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    const int i = gRec.issued;
    gRec.pboFrame[slot] = i;
    gRec.wallMs[i] = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - gRec.t0).count();
    gRec.pathT[i] = Poseidon::tricam::CamPathTime();
    gRec.shot[i] = screenshotFrame ? 1 : 0;
    gRec.issued = i + 1;
    rc.got = gRec.issued;
    if (gRec.issued >= rc.want)
        Finish();
}
