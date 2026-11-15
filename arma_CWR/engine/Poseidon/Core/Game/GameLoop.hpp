#pragma once

namespace Poseidon
{

extern int gUserFpsCap;

void ProcessMessagesNoWait();
void RenderFrame(float deltaT, bool enableDraw);
bool AppIdle();

// Fixed simulation step for scripted captures (triSetFixedStep): while above
// 0, every AppIdle hands World::Simulate exactly this many seconds instead of
// the measured real time, so a paired capture advances the world by the same
// amount whatever the frame rate. 0 = real time (the default).
constexpr float kMaxSimStep = 0.3f; // the longest real-time step AppIdle simulates at once
void SetFixedSimStep(float seconds);
float FixedSimStep();
// The World::Simulate delta for a measured real-time delta: the fixed step
// when one is set, else the real delta capped at kMaxSimStep.
float SimulationDelta(float realSeconds);

} // namespace Poseidon
