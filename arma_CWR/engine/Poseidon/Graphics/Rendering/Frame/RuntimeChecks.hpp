#pragma once

#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <vector>

// Runtime invariant helpers — pure, testable functions that operate
// on values read back from the live engine after Execute.  Kept
// separate from `ValidateFrame` (which is purely structural on the
// Frame) so each side can be unit-tested in isolation: the structural
// validator never needs a framebuffer, the runtime checks never need
// a Frame.


namespace Poseidon
{
namespace render::frame
{

struct RuntimeViolation
{
    const char* ruleId;   // "I-29" etc.
    std::string detail;   // human-readable specifics
};

// A non-empty 3D scene must produce non-trivial framebuffer
// content.  Caller has already established `had3DContent` (typically
// `worldOpaqueDraws.size() + worldTransparentDraws.size() > 0`) and
// read back a single representative pixel from the post-Execute
// framebuffer.  Returns a violation when the pixel is at or below
// `threshold` on all three channels and content was expected — the
// signature of a clear that ran too late, a scene matrix bug, or a
// blit failure.
//
// Threshold of 8 matches the screenshot suite's standard "near
// black" margin and avoids false positives on intentionally dark
// scenes (forest interior, night missions); the readback uses an
// 8-bit-per-channel format so 8 corresponds to ~3% intensity.
std::optional<RuntimeViolation> DetectBlackFrame(
    bool had3DContent,
    const std::uint8_t centerPixelRGB[3],
    int threshold = 8);

// I-29 over time (W1-2). A loading screen, scene swap, pre-dawn cutscene or
// short fade can give a string of legitimately near-black frames, so the
// observer escalates only after kThreshold consecutive black observed frames
// (about half a second under tri, which observes every frame) and logs at
// most kLogCap times. The centre pixel arrives asynchronously (a fenced
// readback, a few frames late, or dropped when no readback slot is free), so
// the tracker keeps the observer's per-frame events in frame order and
// applies each one only when its outcome is known: the counts and log lines
// are the ones the old synchronous readback gave, at most kMaxDelayFrames
// later. A dropped sample changes nothing, as a failed SamplePixel did.
class BlackFrameTracker
{
  public:
    static constexpr int kThreshold = 30;
    static constexpr int kLogCap = 5;
    // A queued sample whose result is still missing this many frames after
    // its frame is treated as dropped (its readback was lost).
    static constexpr std::uint32_t kMaxDelayFrames = 8;

    struct Log
    {
        RuntimeViolation violation;
        int consecutive = 0; // the run length the log line reports
    };

    // An observed frame with 3D content whose centre-pixel probe was queued.
    void ExpectSample(std::uint32_t frame);
    // An observed frame without 3D content: ends the run, in frame order.
    void NoContent(std::uint32_t frame);
    // A probe result (results arrive oldest first). Earlier samples still
    // waiting can no longer arrive and are dropped.
    void Resolve(std::uint32_t frame, const std::uint8_t rgb[3]);
    // Applies every leading event whose outcome is known and drops samples
    // older than kMaxDelayFrames; returns the log lines to emit, in order.
    std::vector<Log> Advance(std::uint32_t nowFrame);

    int Consecutive() const { return _consecutive; }
    int LogsEmitted() const { return _logs; }
    size_t Pending() const { return _events.size(); }

  private:
    enum class Kind : std::uint8_t
    {
        Sample,
        NoContent
    };
    enum class State : std::uint8_t
    {
        Waiting,
        Resolved,
        Dropped
    };
    struct Event
    {
        std::uint32_t frame = 0;
        Kind kind = Kind::Sample;
        State state = State::Waiting;
        std::uint8_t rgb[3] = {0, 0, 0};
    };
    std::deque<Event> _events;
    int _consecutive = 0;
    int _logs = 0;
};

// The active GL viewport rect at observation time must match
// the rect the SceneInputs carried.  Mismatch means a resize, fullscreen
// transition, or scissor rect leaked into the viewport — exactly the
// class of bug a screenshot test might miss because the framebuffer
// can still be non-black with a wrong viewport.
//
// `expected{X,Y,W,H}` come from `SceneInputs.camera.viewport`;
// `liveRect` is `Engine::GetGLViewport`'s return.  Any axis disagreement
// of more than `tolerancePx` is a violation.  Default tolerance 0 —
// the GL state is authoritative and shouldn't drift.
std::optional<RuntimeViolation> DetectViewportMismatch(
    int expectedX, int expectedY, int expectedW, int expectedH,
    const int liveRect[4],
    int tolerancePx = 0);

// Every captured TL draw must reach the frame layer with a
// non-zero VAO handle — `EmitDraw` skips a zero VAO, so a zero here
// means a draw was captured that emission silently dropped.
// `tlDrawCount` is the number of indexed
// draws in this frame; `tlDrawsWithZeroVao` is how many of those
// arrived with `mesh.vao == 0`.  Non-zero means SceneExtractor saw
// a DrawItem the backend hadn't populated — either a path that
// doesn't go through `DrawSectionTL` and shouldn't have isTLDraw
// set, or a regression in the capture site.  Threshold is hard 0;
// even one stray draw is a structural bug worth flagging.
std::optional<RuntimeViolation> DetectMissingMeshHandles(
    unsigned int tlDrawCount,
    unsigned int tlDrawsWithZeroVao);

// When a GL texture bind-skip is taken (the bind cache claims `unit`
// already holds `cachedHandle`, so glBindTexture is elided), the handle read
// back from the driver must equal that claim.  A mismatch is bind-cache
// divergence: the cached handle was deleted and its GL id recycled, or
// something bound the unit outside the cache without invalidating — so the
// skip leaves the wrong texture (often the GL default, sampling as white)
// bound.  `cachedHandle` is `g_tex[unit]`; `liveGLHandle` is
// `glGetIntegerv(GL_TEXTURE_BINDING_2D)` with `unit` active.  Any disagreement
// is a violation.
std::optional<RuntimeViolation> DetectBindCacheDivergence(
    int unit,
    unsigned int cachedHandle,
    unsigned int liveGLHandle);

} // namespace render::frame

} // namespace Poseidon
