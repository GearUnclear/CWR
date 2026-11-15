#pragma once

#include <string>
#include <vector>

// The camera path of motion clips (P0-10): the tri verb triCamPath
// (Game/Commands/GameStateExtTestRecord.cpp) sets it, World::Simulate steps
// it. Pure apart from the one process-wide path. Play never sets one: with
// no path active World::Simulate does what it did before.
//
// A path is a list of keys [t, east, upASL, north, heading, pitch] (seconds,
// metres, degrees in the setDir convention of triSetView: direction
// (sin h cos p, sin p, cos h cos p)), with an optional seventh element 1 for
// a smoothstep ease on the segment that ends at that key (0, the default, is
// linear). Its clock starts at 0 when it is set and advances by each
// simulation step, the accelerated one, so the same path plays at real speed
// in real time and exactly one fixed step per frame under triSetFixedStep.
// Before the first key the camera holds the first key, after the last one
// the last key; headings are interpolated as given (no wrap).

namespace Poseidon::tricam
{

struct CamKey
{
    float t = 0.0f;
    float pos[3] = {0.0f, 0.0f, 0.0f}; // east, up (ASL), north
    float headingDeg = 0.0f;
    float pitchDeg = 0.0f;
    bool ease = false;
};

struct CamPose
{
    float pos[3] = {0.0f, 0.0f, 0.0f};
    float dir[3] = {0.0f, 0.0f, 1.0f}; // unit length
};

// Keys must have strictly increasing times; at least one key.
bool ValidatePath(const std::vector<CamKey>& keys, std::string* why);
CamPose EvaluatePath(const std::vector<CamKey>& keys, float t);
float PathDuration(const std::vector<CamKey>& keys);

// The process-wide path.
bool SetCamPath(const std::vector<CamKey>& keys, std::string* why);
void StopCamPath();
bool CamPathActive();
// The path clock (seconds since the path was set; 0 when none was set).
float CamPathTime();
// Advances the clock by dt and gives the pose there; false (pose untouched)
// when no path is active.
bool StepCamPath(float dt, CamPose& out);

} // namespace Poseidon::tricam
