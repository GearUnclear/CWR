#include <Poseidon/Graphics/Remaster/TemporalDeterminism.hpp>

#include <cmath>

namespace Poseidon::remaster
{

TemporalControlState& TemporalControl()
{
    static TemporalControlState state;
    return state;
}

void RequestTemporalReset(bool holdMist, float mistX, float mistZ)
{
    TemporalControlState& s = TemporalControl();
    ++s.resetSerial;
    if (s.resetSerial == 0) // 0 means "never requested"
        s.resetSerial = 1;
    s.mistHeld = holdMist;
    s.mist[0] = holdMist ? mistX : 0.0f;
    s.mist[1] = holdMist ? mistZ : 0.0f;
}

void SetTemporalDeterministic(bool on)
{
    TemporalControlState& s = TemporalControl();
    s.deterministic = on;
    if (!on)
        s.frameHold = false;
}

bool TemporalDeterministic()
{
    return TemporalControl().deterministic;
}

bool TemporalMistHeld(float& x, float& z)
{
    const TemporalControlState& s = TemporalControl();
    if (!s.mistHeld)
        return false;
    x = s.mist[0];
    z = s.mist[1];
    return true;
}

void NoteRandomSeed(int seed)
{
    TemporalControlState& s = TemporalControl();
    s.seeded = true;
    s.seed = seed;
}

void NoteTemporalResetApplied(std::uint32_t serial, std::uint32_t frame)
{
    TemporalControlState& s = TemporalControl();
    s.appliedSerial = serial;
    s.appliedFrame = frame;
}

void ArmCaptureHold()
{
    TemporalControlState& s = TemporalControl();
    if (s.deterministic)
        s.frameHold = true;
}

void ReleaseCaptureHold()
{
    TemporalControl().frameHold = false;
}

bool CaptureHoldActive()
{
    const TemporalControlState& s = TemporalControl();
    return s.frameHold && s.deterministic;
}

float DeterministicAdaptStep(float fixedStep)
{
    return fixedStep > 0.0f ? fixedStep : kDeterministicDefaultStep;
}

float ResetExposureLog2(float exposureLog2)
{
    return std::isfinite(exposureLog2) ? std::nearbyint(exposureLog2) : 0.0f;
}

bool SsrShareProbeDue(std::uint32_t frame, std::uint32_t base, int period, bool deterministic)
{
    if (period <= 1)
        return true;
    const std::uint32_t n = deterministic ? frame - base : frame;
    return n % static_cast<std::uint32_t>(period) == 0;
}

float RainStreakPhase(int simMs)
{
    const int ms = ((simMs % 1000) + 1000) % 1000;
    return static_cast<float>(ms) * 1e-3f;
}

} // namespace Poseidon::remaster
