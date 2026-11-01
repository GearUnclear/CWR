#pragma once

#include <cstdint>

// Temporal determinism for captures (P0-6): the state the tri verbs
// triResetTemporal, triSeedRandom and triDeterministic set
// (Game/Commands/GameStateExtTestDeterminism.cpp) and the GL backend reads.
// Pure, no GL. Play never calls the setters: with no reset requested and the
// flag off, every reader takes the path it took before.
//
// - A reset request bumps a serial. The backend applies each new serial once,
//   at the first of its next frame's passes that reads temporal state
//   (TemporalResetLatch): the TAA jitter index and the fog's frame sequence
//   start again at 0, the TAA, fog and surface-occlusion histories are
//   dropped, and the photoreal camera snaps to the first meter of that frame.
//   When that snap lands the histories are dropped once more, so everything
//   they hold was drawn at the snapped exposure.
// - The mist hold keeps the volumetric fog's mist-bank drift at a fixed offset
//   (metres) until a reset without one releases it.
// - Deterministic mode (the flag): the readbacks a frame depends on, the
//   camera meter and the reflections' screen shares, are taken at a fixed age
//   of kDeterministicReadbackLag frames, waiting on their fence when the GPU
//   is behind, instead of whichever has arrived by then (that choice moved the
//   exposure, so the whole frame, by 1 to 2 levels whenever GPU time changed);
//   the eye adaptation steps by the fixed simulation step instead of the wall
//   clock; a reset rounds the exposure in use to a whole stop, so the frame the
//   snap meters was stored at the same pre-exposure in every run; the
//   reflections' probe frames count from the last reset; the rain streak
//   overlay follows the simulation clock instead of a per-frame float sum.

namespace Poseidon::remaster
{

// The age (frames) at which deterministic mode reads a readback: the driver
// runs up to three frames ahead, so the fence of frame k - 3 has nearly
// always passed by frame k, and the wait is rare.
constexpr int kDeterministicReadbackLag = 3;
// The eye adaptation step when deterministic mode runs without a fixed
// simulation step (triSetFixedStep 0).
constexpr float kDeterministicDefaultStep = 1.0f / 60.0f;

struct TemporalControlState
{
    bool deterministic = false;
    std::uint32_t resetSerial = 0; // 0: never requested
    bool mistHeld = false;
    float mist[2] = {0.0f, 0.0f};
    bool seeded = false;
    int seed = 0;
    // The last reset the backend applied and the engine frame it applied at.
    std::uint32_t appliedSerial = 0;
    std::uint32_t appliedFrame = 0;
};

// Process-wide, main thread only.
TemporalControlState& TemporalControl();
// A reset at the next frame; holdMist keeps the mist drift at (mistX, mistZ)
// from then on, false releases a hold.
void RequestTemporalReset(bool holdMist, float mistX, float mistZ);
void SetTemporalDeterministic(bool on);
bool TemporalDeterministic();
bool TemporalMistHeld(float& x, float& z);
void NoteRandomSeed(int seed);
void NoteTemporalResetApplied(std::uint32_t serial, std::uint32_t frame);

// The backend's side of a reset request: true once for each new serial.
struct TemporalResetLatch
{
    std::uint32_t applied = 0;
    bool Take(std::uint32_t serial)
    {
        if (serial == applied)
            return false;
        applied = serial;
        return true;
    }
};

// A readback queued at engine frame `slotFrame` is due at `frame` once it is
// at least `lag` frames old (wrap-safe; a slot from the future never is).
constexpr bool DeterministicReadbackDue(std::uint32_t slotFrame, std::uint32_t frame,
                                        int lag = kDeterministicReadbackLag)
{
    return static_cast<std::int32_t>(frame - slotFrame) >= lag;
}

// The eye adaptation step in deterministic mode: the fixed simulation step
// when one is set, else kDeterministicDefaultStep.
float DeterministicAdaptStep(float fixedStep);

// The exposure (log2) a deterministic reset leaves in use until its snap
// lands: rounded to a whole stop (0 when not finite). The pre-exposure only
// sets the scale the world's light is stored at, so a stop off is harmless
// for the three frames it lasts.
float ResetExposureLog2(float exposureLog2);

// The reflections read their screen shares every `period`-th frame while
// their gate is off: counted from the engine's frame 0 in play, from the last
// reset (`base`) in deterministic mode.
bool SsrShareProbeDue(std::uint32_t frame, std::uint32_t base, int period, bool deterministic);

// The rain streak overlay's vertical phase (0..1) from the simulation clock
// in milliseconds (deterministic mode; play keeps its per-frame float sum).
float RainStreakPhase(int simMs);

} // namespace Poseidon::remaster
