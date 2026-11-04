#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

// The real-time frame recorder for motion clips (P0-10): the state the tri
// verbs triRecord and triRecordState set and read
// (Game/Commands/GameStateExtTestRecord.cpp) and the GL backend fills
// (PoseidonGL33/EngineGL33_Recorder.cpp). Pure, no GL. Play never arms it:
// with the state idle the backend returns at its first test.
//
// - Armed with a path, a frame count and a mode, the backend reads the
//   default framebuffer at the triScreenshot capture point (BackToFront,
//   after the gamma pass and the overlay, just before the swap) into a ring
//   of kRingDepth pixel-pack buffers and collects each one kRingDepth
//   frames later, so recording never waits on the GPU. The frames stay in
//   memory and are written once, after the last one: raw RGB24, rows
//   bottom-up as glReadPixels returns them, frame after frame, plus a text
//   sidecar <path>.txt (size, mode, per frame the wall ms since the first
//   frame, the camera path time and whether a triScreenshot captured it).
// - Mode EveryFrame records every presented frame, ShotFramesOnly only the
//   frames a triScreenshot captured at present (the bit-identity check
//   against triScreenshot), EveryFrameHalf every presented frame as a 2x2
//   box-filtered half-size copy.

namespace Poseidon::framerec
{

constexpr int kRingDepth = 4;
// The largest recording held in memory (frames x width x height x 3).
constexpr std::uint64_t kMaxRecordBytes = 24ull << 30;

enum class RecordMode : int
{
    EveryFrame = 0,
    ShotFramesOnly = 1,
    EveryFrameHalf = 2,
};

enum class RecordState : int
{
    Idle = 0,
    Recording = 1,
    Written = 2,
    Failed = 3,
};

struct RecorderControl
{
    RecordState state = RecordState::Idle;
    RecordMode mode = RecordMode::EveryFrame;
    std::string path;
    int want = 0;
    int got = 0;
    bool stopRequested = false;
    std::uint32_t serial = 0; // bumped by every Arm
    std::string failReason;
};

RecorderControl& Recorder();

// Arms a new recording. Refuses (false, *why set) a non-positive frame count,
// an unknown mode, an empty path or a recording in progress.
bool ArmRecorder(const std::string& path, int frames, int mode, std::string* why);
// Asks the backend to write what it has at the next presented frame.
void RequestRecorderStop();
// Called by the backend.
void MarkRecorderFailed(const char* reason);
void MarkRecorderWritten(int frames);

const char* RecordStateName(RecordState s);
bool IsHalfMode(RecordMode m);
// Whether a presented frame is recorded in mode m.
bool RecordsFrame(RecordMode m, bool screenshotFrame);
// Output frame size for a w x h framebuffer.
int OutWidth(RecordMode m, int w);
int OutHeight(RecordMode m, int h);
std::uint64_t RecordBytes(RecordMode m, int w, int h, int frames);

// RGBA8 rows (as read back, w x h) to RGB24 in the same row order; in a half
// mode a 2x2 box filter, (a + b + c + d + 2) / 4 per channel, to w/2 x h/2.
void PackFrame(const std::uint8_t* rgba, int w, int h, bool half, std::uint8_t* rgbOut);

// The sidecar's first line.
std::string SidecarHeader(int outW, int outH, int frames, RecordMode m);

} // namespace Poseidon::framerec
