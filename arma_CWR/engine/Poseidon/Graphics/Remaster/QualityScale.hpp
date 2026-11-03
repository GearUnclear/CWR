#pragma once

#include <Poseidon/Graphics/Remaster/LightRig.hpp>

// Graded graphics quality scale (experiment XP-presets, switch POSEIDON_XP_PRESET).
//
// Today every preset above Low draws the same frame: the Object LOD row is
// never read, the shadow tiers below Off are identical and nothing in the
// remastered features reads a tier (perf-audit S9). This module maps one
// level to concrete render settings, with High equal to today exactly:
//
//   Low     simpler at distance: objects drawn to 1,200 m, far LODs 3x coarser
//           from 300 m, sun shadows to 150 m from a 1,024 map, coarse fog grid,
//           no ambient occlusion, hard sun shadows, no long canopy shadows, no
//           air beams, no screen-space reflections.
//   Medium  objects to 2,000 m, far LODs 2x coarser from 600 m, sun shadows to
//           300 m, coarse fog grid; every feature on.
//   High    today's frame (identity: every hook below is a no-op).
//   Ultra   over budget extras: objects to 80% of the view distance (up to
//           4,000 m, past the 3,000 m objectsZ cap), sun shadows to 5/18 of the
//           view distance (up to 800 m) from a 4,096 map, fine fog grid.
//
// Everything here is draw-only: objectsZ and shadowsZ (which Guerrilla traffic
// and counter-attack perception read) are never written; the render cull and
// the shadow reach are derived beside them. Pure, no GL, no engine state
// except the one process-wide active level.

namespace Poseidon::remaster
{

enum class QualityLevel : int
{
    Low = 0,
    Medium = 1,
    High = 2,
    Ultra = 3,
};

struct QualityProfile
{
    QualityLevel level = QualityLevel::High;
    // High: no hook changes anything (pixel-identical to the switch off).
    bool identity = true;

    // Objects (render only). The drawn object distance is
    // min(objectsZ, renderObjectCap) when renderObjectViewRatio is 0, else
    // min(viewZ * renderObjectViewRatio, renderObjectCap) (Ultra may pass objectsZ).
    float renderObjectCap = 0.0f; // metres, 0 = no cap
    float renderObjectViewRatio = 0.0f;
    // Far LOD ramp: the LOD detail factor grows from 1 at farLodNear to farLodK
    // at farLodFar (and stays there): far objects pick coarser LODs and drop out
    // below the pixel limit sooner. 1 = today.
    float farLodK = 1.0f;
    float farLodNear = 600.0f;
    float farLodFar = 1500.0f;

    // Sun shadows. shadowResolution 0 keeps the tuning's map size. The reach is
    // min(shadowsZ, shadowFarCap) when shadowViewRatio is 0, else
    // min(viewZ * shadowViewRatio, shadowFarCap, drawn object distance).
    int shadowResolution = 0;
    float shadowFarCap = 0.0f; // metres, 0 = keep shadowsZ
    float shadowViewRatio = 0.0f;

    // Lighting tuning overrides (applied on every SetLightingTuning, so a test's
    // triSetLighting reset keeps them). fogQuality < 0 keeps the tuning's.
    float fogQuality = -1.0f;
    bool ao = true; // false: ambientOcclusion 0, which skips the AO pass (idempotent)
    bool softShadows = true;
    bool canopyShadows = true;
    bool airBeams = true;
    bool ssr = true;
};

QualityProfile ProfileFor(QualityLevel level);
const char* QualityLevelName(QualityLevel level);
// "low", "medium", "high", "ultra" (case-insensitive) or "0".."3"; false otherwise.
bool ParseQualityLevel(const char* text, QualityLevel& out);

// The LOD detail multiplier at a distance (1 = today). LevelFromDistance2
// multiplies its detail2 (distance squared times the LOD width squared) by the
// square of this.
float FarLodFactor(const QualityProfile& p, float distance);
// The drawn object distance for the profile.
float RenderObjectDistance(const QualityProfile& p, float objectsZ, float viewZ);
// The sun-shadow reach (cascade far plane and caster collection distance).
float ShadowReach(const QualityProfile& p, float shadowsZ, float viewZ, float objectsZ);
// Apply the profile's lighting overrides in place. No-op for an identity profile.
void ApplyQualityToTuning(const QualityProfile& p, LightingTuning& t);

// The process-wide active profile, or nullptr when the experiment is off (the
// default). On first use it reads POSEIDON_XP_PRESET (a level name: forces that
// level); SetActiveQualityLevel replaces it (the graphics options apply, when
// POSEIDON_XP_PRESETS=1 and no forced level is set).
const QualityProfile* ActiveQuality();
// Like ActiveQuality but nullptr also for an identity profile: the hot paths
// call this so High costs nothing and changes nothing.
const QualityProfile* ActiveQualityChange();
void SetActiveQualityLevel(const QualityLevel* level);
// True when POSEIDON_XP_PRESET names a level (it wins over the options).
bool QualityLevelForcedByEnv();

// The active profile's drawn object distance and sun-shadow reach; objectsZ
// and shadowsZ unchanged when the experiment is off or at High.
float ActiveRenderObjectDistance(float objectsZ, float viewZ);
float ActiveShadowReach(float shadowsZ, float viewZ, float objectsZ);

} // namespace Poseidon::remaster
