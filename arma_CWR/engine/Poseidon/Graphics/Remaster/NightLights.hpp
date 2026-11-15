#pragma once

#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>

#include <cstdint>

// The remastered night: the rules the GL backend, the scene and the shaders
// share. Pure, no GL, no engine state. Three feature switches, each with its
// frame gate
// (FeatureGates.hpp):
//
//   moonShadows  the moon as the key light: once the sun is well under the
//                horizon and a bright moon is up, the shader's sun slots carry
//                the moon (LightRig.hpp KeyLight), so the shadow map, the
//                terrain horizon, the cloud shadows and the glossy highlight
//                follow it, and the scene renders the shadow map from the moon.
//                The moon keeps its calibration; only its shadows are new. The
//                sky stays on the sun (its table, aureole and fog glow).
//   nightLights  street lamps and headlights as physical lights on the frame's
//                tiled light list (FrameLights.hpp: inverse-square falloff from
//                the bulb, a low-beam pattern aimed from the vehicle's body,
//                rough diffuse, foliage wrapped as the sun wraps it), and the
//                same lights on smoke and dust (ParticleLight).
//   stars        stars as small soft splats with a faint colour each, instead
//                of a two-pixel bilinear dot.
//
// The shaders read two LitConstants slots (EngineGL33_Lighting.hpp):
// LitConst::Night, x 1 while the physical lamps and low beams light the frame
// (the nightLights gate on and the tiled list lighting the pass), y the lamp
// intensity per metre of start distance (times nightLamp), z the low beam's
// hot-spot intensity (times nightSpot), w the moon key's share (0: the sun is
// the key); and LitConst::SkySun, the sun's own direction and elevation, which
// the sky reads whatever the key (LightRig.hpp KeyLight).

namespace Poseidon::remaster
{

// The moon key fades in over this many degrees above each of its gate edges
// (the sun kGateMoonKeyMaxSunDeg under the horizon, the moon
// kGateMoonKeyMinMoonDeg up) and over this much of the phase above
// kGateMoonKeyMinPhase, so the moon's shadows fade in instead of switching on.
constexpr float kMoonKeyFadeDeg = 3.0f;
constexpr float kMoonKeyFadePhase = 0.1f;

// The share (0..1) of the rig's moon that the key carries this frame: 0 where
// the moon key's gate is off, rising to 1 within kMoonKeyFadeDeg of the sun
// and moon edges and kMoonKeyFadePhase of the phase edge.
float MoonKeyShare(const FeatureGates& gates);

// LitConst::Night for the frame. `tiledLights`: the frame's tiled light list
// lights the pass (the physical model lives in the tiled path only).
// `moonShare`: the frame's KeyLight::moonShare, which w carries whatever the
// lamps do (the sky's sun radiance is 0 while it is above 0).
struct NightConstants
{
    float night[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};
NightConstants ComputeNightConstants(const LightingTuning& tuning, const FeatureGates& gates, bool tiledLights,
                                     float moonShare = 0.0f);

// A star as a splat: a fan of four triangles around the star's screen point
// (x, y in pixels), its centre kStarCentreGain times the star's alpha (0..255,
// at most 255) falling to 0 at the corners of a square kStarSplatRadius pixels
// out, and a colour tint between cool and warm picked from the star's index,
// at about unit luminance. The bilinear dot it replaces spread the alpha over
// two pixels; at 1080p and render scale 1 most stars were a faint pixel.
constexpr float kStarSplatRadius = 1.6f;
constexpr float kStarCentreGain = 1.8f;
struct StarSplat
{
    float x[5] = {}, y[5] = {}; // 0 the centre, 1..4 the corners
    float alpha[5] = {};        // 0..255
    float tint[3] = {1.0f, 1.0f, 1.0f};
};
StarSplat MakeStarSplat(float x, float y, float alpha, std::uint32_t index);

} // namespace Poseidon::remaster
