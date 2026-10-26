#pragma once

#include <Poseidon/Graphics/Remaster/LightRig.hpp>

#include <cstdint>
#include <string>
#include <string_view>

// One registry for every remastered lighting key: the triSetLighting items,
// the dev overlay's Lighting tab, the copy-back summary line and the tests all
// read this table, so a key added here exists everywhere at once. Pure, no GL,
// no engine state.
//
// Two kinds of entries:
//
//   - knobs: the art-direction floats of LightingTuning (exposure, sun, ...),
//     the post toggle, the debug view and the `gates` dev knob;
//   - feature switches: the members of FeatureSet (taa, combatLights, ...),
//     0 off and 1 on, every one on by default. Each feature also has a frame
//     gate (FeatureGates.hpp) that decides whether an enabled feature runs.
//
// Mode words reset the whole tuning (TuningForMode): "classic" is the original
// look bit for bit, "remastered" the shipped defaults (every feature on) and
// "baseline" the remastered look with every feature switch off, which is the
// look of the graphics-remaster branch cut. A feature's "off" in the
// side-by-side video is "remastered,<its keys>=0", and the A/B capture checks
// that it matches "remastered" on the build before the feature landed.
//
// Defaults live in one place, the member initialisers of LightingTuning and
// FeatureSet; the table reads them from a default-constructed tuning.

namespace Poseidon::remaster
{

enum class Feature : std::uint8_t
{
    DrawGrouping,
    HillCull,
    Taa,
    LightTiles,
    Atmosphere,
    TreeShadows,
    SoftShadows,
    LowSunRig,
    CrownCanopy,
    NightLights,
    MoonShadows,
    Stars,
    CombatLights,
    CombatShadows,
    SeaWaves,
    Ssr,
    WetWeather,
    LampStreaks,
    AirBeams,
    AirHaze,
    ValleyMist,
    CanopyShadows,
    LitClouds,
    SkyCloudShadows,
    TerrainDetail,
    Sharpen,
    PhotoLight,
    Count
};

constexpr int kFeatureCount = static_cast<int>(Feature::Count);

// What the overlay, triFeatureState and the capture tooling know about a
// feature switch.
struct FeatureInfo
{
    Feature id;
    const char* key;   // triSetLighting key, also the FeatureSet member name
    const char* label; // overlay checkbox
    const char* track; // research track (tmp/research/<track>, research/<track>.md)
    const char* area;  // the owner's four areas, plus "Frame cost" for perf
    const char* step;  // integration step that makes the switch do something
    // False while nothing reads the switch: it exists (keys, gates, captures)
    // but on and off draw the same frame. The step that lands the feature sets
    // it to true.
    bool landed;
    float FeatureSet::* member;
    // GpuTimings section name of the feature's own GPU pass ("" = none yet).
    const char* gpuSection;
};

const FeatureInfo& GetFeatureInfo(Feature f);
// nullptr when `key` is not a feature switch.
const FeatureInfo* FindFeature(std::string_view key);
float FeatureValue(const FeatureSet& set, Feature f);
void SetFeatureValue(FeatureSet& set, Feature f, float value);
// A switch counts as on above 0.
bool FeatureOn(const FeatureSet& set, Feature f);
// The overlay's area headings in display order (each FeatureInfo::area is one
// of these).
constexpr const char* kFeatureAreas[] = {"Light and shadow", "Sky and air", "Water and shine", "Night and combat",
                                         "Frame cost"};

enum class KeyKind : std::uint8_t
{
    Knob,   // float in LightingTuning
    Toggle, // bool in LightingTuning, set as 0 / 1
    Choice, // int in LightingTuning (the debug view)
    Switch, // feature switch, a float in FeatureSet
};

struct LightingKey
{
    const char* key;   // triSetLighting key
    const char* label; // overlay label
    // Overlay section for knobs ("Light", "World", "Post effects",
    // "Diagnostics", "Gates"); the track for switches.
    const char* group;
    const char* hint; // one overlay line under the control ("" = none)
    KeyKind kind;
    float minValue, maxValue; // overlay slider range
    float LightingTuning::* knob = nullptr;
    bool LightingTuning::* toggle = nullptr;
    int LightingTuning::* choice = nullptr;
    Feature feature = Feature::Count;
    // A sub-knob tunes one feature: the switch key it belongs to (the overlay
    // shows it under that switch). nullptr for top-level knobs.
    const char* parent = nullptr;
    // Choice: the option names, each ended by '\0' (an ImGui combo list).
    const char* choices = nullptr;
};

int LightingKeyCount();
const LightingKey& LightingKeyAt(int i);
// nullptr when `key` is not in the table.
const LightingKey* FindLightingKey(std::string_view key);
float GetKeyValue(const LightingTuning& t, const LightingKey& k);
// Stores `value` as the key's type (a toggle is on for any value but 0, a
// choice is truncated). No clamping: research values outside the overlay
// range stay possible from triSetLighting.
void SetKeyValue(LightingTuning& t, const LightingKey& k, float value);
// The key's value in a default-constructed LightingTuning.
float KeyDefault(const LightingKey& k);

enum class LightingMode : std::uint8_t
{
    Classic,
    Remastered,
    Baseline,
};

// "classic", "remastered" or "baseline"; false for anything else.
bool ParseLightingMode(std::string_view word, LightingMode& mode);
// The whole tuning a mode word sets: every knob at its default; Baseline also
// turns every feature switch off.
LightingTuning TuningForMode(LightingMode mode);

// One triSetLighting item applied to `t`: a mode word replaces the whole
// tuning (`modeSet` becomes true), "key=value" sets one key. Returns "" or why
// the item is refused (then `t` is unchanged).
std::string ApplyLightingItem(LightingTuning& t, const std::string& item, bool& modeSet);
// A comma list of items applied in order, all or none: on a refusal `t` is
// unchanged and the reason is returned.
std::string ApplyLightingSetting(LightingTuning& t, const std::string& setting, bool& modeSet);

// The tuning as a triSetLighting setting: the mode word, every knob, and the
// feature switches that are not at their default. Applying it to any tuning
// gives `t` back. The overlay's copy-back line.
std::string TuningSummary(const LightingTuning& t);

} // namespace Poseidon::remaster
