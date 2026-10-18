#pragma once

#include <Poseidon/Graphics/Remaster/LightRig.hpp>

// Remastered sea (feature switch `seaWaves`, research/INTEGRATION-PLAN.md
// section 5 B5): the parts of the sea shading that are plain math, shared by
// the backend (EngineGL33_Sea.cpp), the landscape and the tests. No GL, no
// engine state.
//
// The landscape draws the sea as faces of its own (Landscape::DrawWater) with
// the animated sea texture (more_anim.01 to .07, one repeat per 50 m) and the
// specular detail texture (data\specular_dx.paa, a ripple pattern in its
// alpha). With the switch on, the backend draws them with the remastered water
// program (EngineGL33_LightingShaders.cpp, s_psWaterRemasterMain): the wave
// normal comes from the sea texture's own brightness, the sky is reflected by
// Fresnel on that normal, and the sun lays a broken path of glints on it.

namespace Poseidon::remaster
{

// ---- The sea texture's animation ------------------------------------------
//
// The landscape's phase runs 0 -> 1 -> 0 every 2 s (fmod(time, 2), mirrored)
// and PolyProperties::AnimateTexture binds frame floor(phase * n): a hard
// switch every 1/n s (0.14 s for the 7 frames of the Remaster sea), which the
// waves derived from the texture would show as a pop. SeaAnimationBlend
// spreads the frames evenly over the phase instead (frame k shows alone at
// phase k / (n - 1)) and cross-fades the two around the phase: the frame the
// landscape bound is one of them, the backend binds the other next to it and
// the shader mixes `otherWeight` of it in. The mix is continuous in the phase,
// so the waves flow from one frame into the next.

struct SeaFrameBlend
{
    int bound = 0;            // the frame the landscape bound (texture unit 0)
    int other = 0;            // the frame mixed in (the backend's next-frame unit)
    float otherWeight = 0.0f; // share of `other` in the colour (0 = the bound frame alone)
};
// `bound` is the frame the landscape's faces carry at `phase` (0 .. 1) of an
// animation of `frames` frames.
SeaFrameBlend SeaAnimationBlend(float phase, int frames, int bound);
// Frame `frame`'s share of the mix (the shares sum to 1).
float SeaFrameWeight(const SeaFrameBlend& blend, int frame);

// Consecutive frames of the Remaster sea are much alike: the correlation of
// their luminance gradients is 0.65 to 0.82 (more_anim.01 to .07). A mix of two
// frames loses a little relief halfway (the gradient's spread falls to
// sqrt((1 - w)^2 + w^2 + 2 w (1 - w) rho) of a frame's); the shader divides it
// out with this rho so the waves do not breathe at the frame rate.
constexpr float kSeaFrameCorrelation = 0.75f;
// The relief gain that keeps a mix with share `w` of the other frame as
// strong as a single frame.
float SeaCrossFadeGain(float w);

// ---- Per-frame constants of the water program ------------------------------
//
// LitConstants Water0 and Water1 (EngineGL33_Lighting.hpp). The layer heights,
// the lobes and the distance fades are constants of the shader.

// The share of the flat-water Fresnel reflection of the sky the sea shows on
// average (the part of the Fresnel term that comes from the waves goes in at
// full weight, which is where the definition comes from). Low, so the sea
// never turns into a pale mirror at grazing range.
constexpr float kSeaSkyMean = 0.3f;
// The material's own broad sun lobe: the original's sheen. The original lit
// it per vertex, with the flat normal, in its own sun colour (a broad
// unnormalised lobe of sun diffuse times material specular), which is what
// made its noon sea green-yellow; the remastered sheen keeps that hue
// (SeaSheenLight). On the wave normal it carries the waves; at a high sun the
// wave normal would split it into yellow facets and blue troughs (a mustard
// sea), so there it moves to the flat normal, a smooth wash as the original's.
constexpr float kSeaSheen = 0.35f;
// A high sun: the sea keeps more of its own colour, as the original's noon sea
// does. Between these elevations the mean sky weight falls to kSeaSkyMeanNoon
// of itself, the reflected sky takes kSeaSkyTintNoon of the original fog
// colour in place of its own, the sheen grows to kSeaSheenNoon and moves to
// the flat normal.
constexpr float kSeaHighSunStartDeg = 15.0f;
constexpr float kSeaHighSunFullDeg = 50.0f;
constexpr float kSeaSkyMeanNoon = 0.5f;
constexpr float kSeaSkyTintNoon = 0.6f;
constexpr float kSeaSheenNoon = 2.4f;

struct SeaWaveConstants
{
    float relief = 0.0f;    // wave relief scale (1 = the shader's layer heights)
    float crossFade = 0.0f; // share of the next-frame unit
    float skyMean = 0.0f;   // mean sky reflection weight
    float skyTint = 0.0f;   // reflected sky toward the original fog colour (0 none)
    float sheen = 0.0f;     // weight of the material's broad lobe
    float sheenFlat = 0.0f; // share of the flat normal in the sheen's normal (0 the wave normal)
};

// 0 for a sun below kSeaHighSunStartDeg, 1 above kSeaHighSunFullDeg, smooth in between.
float SeaHighSun(float sunElevationDeg);

// The constants for a sun `sunElevationDeg` above the horizon, the tuning's
// sea sub-knobs (seaRelief, seaSky, seaSheen) and the frame's animation blend.
SeaWaveConstants ComputeSeaWaveConstants(float sunElevationDeg, const LightingTuning& tuning,
                                         const SeaFrameBlend& blend);

// The light of the sheen: the remastered sun's radiance `rigSun` (linear) in
// the hue of the original sun `classicSunDiffuse` (LightSun::Diffuse(), display
// values), times `sheen`. The luminance is the rig's, so the sheen is as
// bright as the remastered sun makes it; only its hue is the original's. The
// original added its sheen to a near-grey sea in display space, and a small
// increment keeps its display chroma through the 2.2 power, so the hue is
// taken from the display values as they are.
Rgb SeaSheenLight(const Rgb& rigSun, const Rgb& classicSunDiffuse, float sheen);

} // namespace Poseidon::remaster
