#pragma once

#include <cstdint>

// Temporal anti-aliasing for the remastered lighting (INTEGRATION-PLAN.md step
// A5): the math the GL backend and the resolve shader share. Pure, no GL, no
// engine state.
//
// Every frame the world is drawn shifted by a sub-pixel offset (the jitter, a
// Halton(2, 3) sequence over 16 frames). The post pass then blends this frame
// into a history of the frames before it: the history is moved to where each
// pixel's surface was one frame ago (reprojection with the camera motion and
// the depth), clipped to the colours around the pixel in this frame, and
// mixed in at 1 - alpha. Over a few frames each pixel gathers many sample
// positions, so alpha-tested leaf edges and wire stop crawling, and effects
// that change their pattern every frame average into a smooth result.
//
// The camera convention is the one the backend uploads: a camera-relative
// world position p_rel maps to view space as p_view = p_rel * R (row vector,
// R the 3x3 rotation rows _11.._33 of FrameState::view), and the projection
// maps view space to NDC as x * p11 / z, y * p22 / z.

namespace Poseidon::remaster
{

// Frames before the jitter sequence repeats.
constexpr std::uint32_t kTaaJitterCycle = 16;
// A camera that moved further than this in one frame (a teleport, a cut to
// another camera) starts a fresh history.
constexpr double kTaaCutDistance = 50.0; // metres
// ... and so does one that turned by more than 60 degrees: the cosine between
// this frame's and the last frame's view direction must stay above this.
constexpr double kTaaCutMinCos = 0.5;
// The particle mask (the resolve's reactive term): a pixel that the
// translucent objects drawn after the opaque world changed by more than the
// low threshold (display levels / 255) starts to take more of this frame, all
// of kTaaReactiveMaxAlpha above the high one.
constexpr float kTaaReactiveLow = 2.0f / 255.0f;
constexpr float kTaaReactiveHigh = 24.0f / 255.0f;
constexpr float kTaaReactiveMaxAlpha = 0.8f;

// The radical inverse of `index` in `base` (Halton sequence), in [0, 1).
float Halton(std::uint32_t index, std::uint32_t base);

struct TaaOffset
{
    float x = 0.0f, y = 0.0f;
};

// Sub-pixel offset of TAA frame `frame` in render-target pixels, each axis in
// [-0.5, 0.5): Halton(2, 3) at index 1 + frame % kTaaJitterCycle, minus 0.5.
// Index 0 is skipped: it is the pixel centre on both axes, while the cycle
// should average to it.
TaaOffset TaaJitterPixels(std::uint32_t frame);

// The offset as the clip-space shift the remastered vertex shaders add
// (vpScale.zw, times w): 2 * pixels / size of the 3D viewport in pixels.
// Zero for an empty viewport.
TaaOffset TaaJitterNdc(TaaOffset pixels, int viewportW, int viewportH);

// The jitter a vertex shader gets in a pass: the frame's offset inside the
// world passes of a frame that the resolve will run on, zero everywhere else
// (the HUD, dialogs, portraits, 3D previews after the world, the shadow-map
// depth pass, frames without TAA). One function so the rule has one place.
TaaOffset TaaPassJitterNdc(bool worldPass, bool taaThisFrame, TaaOffset framePixels, int viewportW, int viewportH);

// One frame's camera as the resolve needs it.
struct TaaCamera
{
    double rot[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1}; // R, rows _11.._33
    double pos[3] = {0, 0, 0};                   // camera position (world, metres)
    double p11 = 1.0, p22 = 1.0;                 // projection _11 and _22
};

// The previous frame's view position of a point given in this frame's view
// space: p_prev = rows * (p, 1), as the resolve shader computes it.
struct TaaReprojection
{
    // False: no usable history this frame (a singular rotation, a cut).
    bool valid = false;
    float rows[3][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
    float prevP11 = 1.0f, prevP22 = 1.0f;
    double move = 0.0;       // metres the camera moved since the last frame
    double forwardCos = 1.0; // cosine between the two view directions
};

// Built in double precision: p_prev = p * (R^-1 * R_prev) + (C - C_prev) * R_prev
// (row vectors), handed over as the rows of the column-vector form. Invalid
// after a cut (kTaaCutDistance, kTaaCutMinCos) or for a singular rotation.
TaaReprojection BuildTaaReprojection(const TaaCamera& prev, const TaaCamera& cur);

// The resolve's mapping for one point, in double precision: this frame's view
// position `p` to the previous frame's NDC. False when the point lies behind
// the previous camera.
bool TaaReprojectNdc(const TaaReprojection& r, const double p[3], double ndc[2]);

// Whether the history written by the last resolve may be read by this one:
// it exists, it was written in the frame just before this one (a frame
// without a resolve, such as a vehicle interior, optics, night vision, the
// switch or the post effects off, breaks the chain), the targets kept their
// size, and the camera did not cut.
struct TaaHistory
{
    bool exists = false;
    std::uint32_t frame = 0; // engine frame counter of the resolve that wrote it
    int w = 0, h = 0;        // its size
};
bool TaaHistoryUsable(const TaaHistory& history, std::uint32_t frame, int w, int h, bool reprojectionValid);

} // namespace Poseidon::remaster
