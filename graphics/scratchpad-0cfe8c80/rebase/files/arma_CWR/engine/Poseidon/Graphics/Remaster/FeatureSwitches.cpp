#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>

#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <vector>

namespace Poseidon::remaster
{

namespace
{

using FS = FeatureSet;
using LT = LightingTuning;

// One row per Feature, in enum order (checked below). `landed` turns true in
// the step that makes the switch do something.
constexpr FeatureInfo kFeatures[] = {
    // A4 (perf): both pixel-exact; their GPU time shows in the opaque objects'
    // section of the world pass.
    {Feature::DrawGrouping, "drawGrouping", "Draw grouping", "perf", "Frame cost", "A4", true, &FS::drawGrouping,
     "pass1"},
    {Feature::HillCull, "hillCull", "Hill culling", "perf", "Frame cost", "A4", true, &FS::hillCull, "pass1"},
    {Feature::Taa, "taa", "Temporal anti-aliasing", "temporal", "Light and shadow", "A5", true, &FS::taa, "taa"},
    // A6: no pass of its own; the per-fragment light loop is part of the
    // world pass's shading.
    {Feature::LightTiles, "lightTiles", "Tiled light list", "light list", "Night and combat", "A6", true,
     &FS::lightTiles, "world"},
    {Feature::Atmosphere, "atmosphere", "Sky table (dome and fog colour)", "atmosphere", "Sky and air", "A7", true,
     &FS::atmosphere, "sky"},
    // B1 (shadows): the casters' cost shows in the shadow depth pass, the soft
    // filter's in the world pass that samples it.
    {Feature::TreeShadows, "treeShadows", "Tree and building sun shadows", "shadows", "Light and shadow", "B1", true,
     &FS::treeShadows, "shadow"},
    {Feature::SoftShadows, "softShadows", "Soft sun shadows", "shadows", "Light and shadow", "B1", true,
     &FS::softShadows, "world"},
    // B2 (shade): the low-sun rig is constants computed once a frame (no GPU
    // cost of its own; the world pass is what it lights); the crown canopy is
    // a few terms on the world pass's foliage.
    {Feature::LowSunRig, "lowSunRig", "Low-sun rig", "shade", "Light and shadow", "B2", true, &FS::lowSunRig, "world"},
    {Feature::CrownCanopy, "crownCanopy", "Tree crowns as a canopy", "shade", "Light and shadow", "B2", true,
     &FS::crownCanopy, "world"},
    {Feature::NightLights, "nightLights", "Lamps and low beams", "night", "Night and combat", "B3", false,
     &FS::nightLights, ""},
    {Feature::MoonShadows, "moonShadows", "Moon shadows", "night", "Night and combat", "B3", false, &FS::moonShadows,
     ""},
    {Feature::Stars, "stars", "Stars", "night", "Night and combat", "B3", false, &FS::stars, ""},
    {Feature::CombatLights, "combatLights", "Combat light", "combat", "Night and combat", "B4", false,
     &FS::combatLights, ""},
    {Feature::CombatShadows, "combatShadows", "Combat light shadows", "combat", "Night and combat", "B4", false,
     &FS::combatShadows, ""},
    // B5 (water): the sea's draws, in the ground section of the world pass
    // (sky, terrain, water, horizon).
    {Feature::SeaWaves, "seaWaves", "Sea waves", "water", "Water and shine", "B5", true, &FS::seaWaves, "ground"},
    {Feature::Ssr, "ssr", "Screen-space reflections", "ssr", "Water and shine", "B6", false, &FS::ssr, ""},
    {Feature::WetWeather, "wetWeather", "Wet weather and puddles", "materials", "Water and shine", "B7", false,
     &FS::wetWeather, ""},
    {Feature::LampStreaks, "lampStreaks", "Lamp streaks on wet ground", "materials", "Water and shine", "B7", false,
     &FS::lampStreaks, ""},
    {Feature::AirBeams, "airBeams", "Sun beams and headlight cones", "volumetric", "Sky and air", "B8", false,
     &FS::airBeams, ""},
    {Feature::AirHaze, "airHaze", "Aerial haze", "haze", "Sky and air", "B8", false, &FS::airHaze, ""},
    {Feature::ValleyMist, "valleyMist", "Valley mist", "haze", "Sky and air", "B8", false, &FS::valleyMist, ""},
    // B9 (hfrt): the sun-visibility map's build shows in its own section on
    // the frames that rebuild it; the one read per fragment is in the world pass.
    {Feature::CanopyShadows, "canopyShadows", "Long canopy shadows", "hfrt", "Light and shadow", "B9", true,
     &FS::canopyShadows, "canopy"},
    // B10 (clouds): lit clouds cost in the sky pass (the dome and the sky
    // clouds), the sky-matched shadows one density tap per lit world fragment.
    {Feature::LitClouds, "litClouds", "Lit sky clouds", "clouds", "Sky and air", "B10", true, &FS::litClouds, "sky"},
    {Feature::SkyCloudShadows, "skyCloudShadows", "Sky-matched cloud shadows", "clouds", "Sky and air", "B10", true,
     &FS::skyCloudShadows, "world"},
    // B11 (texsharp): the terrain detail runs inside the ground draw, the luma
    // sharpen in the post pass's sharpen stage (TAA's own sharpen when TAA runs).
    {Feature::TerrainDetail, "terrainDetail", "Terrain detail", "texsharp", "Light and shadow", "B11", true,
     &FS::terrainDetail, "ground"},
    {Feature::Sharpen, "sharpen", "Sharpen (with TAA off)", "texsharp", "Light and shadow", "B11", true, &FS::sharpen,
     "sharpen"},
    // Look lane "light" (PhotoCamera.hpp): the physical rig is constants once a
    // frame (plus the sky integral when the sun has moved), the camera is the
    // meter pass and a few terms of the composite.
    {Feature::PhotoLight, "photoLight", "Photoreal light and camera", "look-light", "Light and shadow", "L1", true,
     &FS::photoLight, "composite"},
};
static_assert(sizeof(kFeatures) / sizeof(kFeatures[0]) == kFeatureCount, "one FeatureInfo per Feature");

constexpr bool FeaturesInEnumOrder()
{
    for (int i = 0; i < kFeatureCount; ++i)
    {
        if (static_cast<int>(kFeatures[i].id) != i)
            return false;
    }
    return true;
}
static_assert(FeaturesInEnumOrder(), "kFeatures rows follow the Feature enum");

LightingKey MakeKnob(const char* key, const char* label, const char* group, const char* hint, float LT::* field,
                     float lo, float hi)
{
    LightingKey k{key, label, group, hint, KeyKind::Knob, lo, hi};
    k.knob = field;
    return k;
}

LightingKey MakeToggle(const char* key, const char* label, const char* group, const char* hint, bool LT::* field)
{
    LightingKey k{key, label, group, hint, KeyKind::Toggle, 0.0f, 1.0f};
    k.toggle = field;
    return k;
}

LightingKey MakeChoice(const char* key, const char* label, const char* group, const char* choices, int LT::* field,
                       int lo, int hi)
{
    LightingKey k{key, label, group, "", KeyKind::Choice, static_cast<float>(lo), static_cast<float>(hi)};
    k.choice = field;
    k.choices = choices;
    return k;
}

LightingKey MakeSwitch(const FeatureInfo& f)
{
    LightingKey k{f.key, f.label, f.track, "", KeyKind::Switch, 0.0f, 1.0f};
    k.feature = f.id;
    return k;
}

// A knob that tunes one feature: the overlay shows it under the feature's
// switch, in the switch's track.
LightingKey MakeSubKnob(Feature parent, const char* key, const char* label, const char* hint, float LT::* field,
                        float lo, float hi)
{
    const FeatureInfo& f = kFeatures[static_cast<int>(parent)];
    LightingKey k = MakeKnob(key, label, f.track, hint, field, lo, hi);
    k.parent = f.key;
    return k;
}

struct KeyTable
{
    std::vector<LightingKey> rows;
    int count = 0;

    KeyTable()
    {
        // The overlay draws the knob groups in this order. A feature's
        // sub-knobs go after the switches, with `parent` set.
        const LightingKey knobs[] = {
            MakeKnob("exposure", "Exposure", "Light", "", &LT::exposure, 0.4f, 2.5f),
            MakeKnob("sun", "Sun", "Light", "", &LT::sun, 0.0f, 2.5f),
            MakeKnob("sky", "Sky fill", "Light", "shade brightness; lower = more contrast between sun and shade",
                     &LT::sky, 0.0f, 2.5f),
            MakeKnob("bounce", "Ground bounce", "Light", "", &LT::bounce, 0.0f, 3.0f),
            MakeKnob("warmth", "Sun warmth", "Light", "0 = neutral sun at any height, 1 = golden low sun", &LT::warmth,
                     0.0f, 2.0f),
            MakeKnob("saturation", "Saturation", "Light", "", &LT::saturation, 0.5f, 1.6f),
            MakeKnob("specular", "Specular", "Light", "", &LT::specular, 0.0f, 3.0f),
            MakeKnob("localLights", "Local lights", "Light", "", &LT::localLights, 0.0f, 4.0f),
            MakeKnob("moonlight", "Moonlight", "Light", "", &LT::moonlight, 0.0f, 4.0f),
            MakeKnob("terrainShadows", "Terrain shadows", "World", "hills shading the valleys (horizon map)",
                     &LT::terrainShadows, 0.0f, 1.0f),
            MakeKnob("terrainOcclusion", "Terrain occlusion", "World", "", &LT::terrainOcclusion, 0.0f, 1.0f),
            MakeKnob("cloudShadows", "Cloud shadows", "World", "drifting shadows in broken overcast", &LT::cloudShadows,
                     0.0f, 1.0f),
            MakeKnob("shadowSoftness", "Shadow softness", "World", "", &LT::shadowSoftness, 0.25f, 4.0f),
            MakeToggle("post", "Enabled (AO, bloom, shafts)", "Post effects", "", &LT::postEffects),
            MakeKnob("ao", "Ambient occlusion", "Post effects", "", &LT::ambientOcclusion, 0.0f, 2.0f),
            MakeKnob("bloom", "Bloom", "Post effects", "", &LT::bloom, 0.0f, 2.0f),
            MakeKnob("shafts", "Sun shafts", "Post effects", "", &LT::sunShafts, 0.0f, 2.0f),
            MakeChoice("debug", "View", "Diagnostics",
                       "Final\0Sun visibility\0Terrain sky visibility\0Normals\0Ambient occlusion\0"
                       "Linear depth (half res)\0Light tiles (lights per tile)\0Albedo\0",
                       &LT::debugView, 0, kDebugViewAlbedo),
            MakeToggle("gates", "Frame gates", "Gates", "off = enabled features run on every frame (debugging)",
                       &LT::gates),
        };
        rows.assign(std::begin(knobs), std::end(knobs));
        for (const FeatureInfo& f : kFeatures)
            rows.push_back(MakeSwitch(f));
        // Sub-knobs, in the order their features land.
        const LightingKey subKnobs[] = {
            MakeSubKnob(Feature::Taa, "taaSharpen", "TAA sharpen", "0 = soft; the overshoot stays bounded (no halos)",
                        &LT::taaSharpen, 0.0f, 1.5f),
            MakeSubKnob(Feature::Taa, "taaMipBias", "TAA texture LOD bias",
                        "world textures while TAA runs; negative = sharper", &LT::taaMipBias, -1.5f, 0.5f),
            MakeSubKnob(Feature::Taa, "taaAlpha", "TAA current frame weight", "lower = steadier and softer",
                        &LT::taaAlpha, 0.02f, 1.0f),
            MakeSubKnob(Feature::Atmosphere, "atmoHue", "Sky hue",
                        "0 = the original sky's hue relit, 1 = the physical hue", &LT::atmoHue, 0.0f, 1.0f),
            MakeSubKnob(Feature::Atmosphere, "atmoSat", "Sky saturation",
                        "display-space saturation of the dome and fog colour", &LT::atmoSat, 0.5f, 1.6f),
            MakeSubKnob(Feature::Atmosphere, "atmoHaze", "Haze",
                        "Mie density of the air (rebuilds the atmosphere tables)", &LT::atmoHaze, 0.25f, 4.0f),
            MakeSubKnob(Feature::TerrainDetail, "detailStrength", "Terrain grain strength",
                        "grain from the square's own texture; 2 starts to look scaly", &LT::detailStrength, 0.0f, 3.0f),
            MakeSubKnob(Feature::Sharpen, "sharpenStrength", "Luma sharpen strength",
                        "contrast-adaptive, brightness only; runs only with TAA off", &LT::sharpenStrength, 0.0f, 1.0f),
            MakeSubKnob(Feature::TreeShadows, "leafBias", "Leaf shadow bias",
                        "shadow-map texels a leaf's own lookup moves toward the sun; 0 = striped crowns", &LT::leafBias,
                        0.0f, 8.0f),
            MakeSubKnob(Feature::SoftShadows, "sunSize", "Sun size",
                        "apparent diameter (deg): 0.53 real, 1 shipped; wider = softer far shadow ends", &LT::sunSize,
                        0.0f, 3.0f),
            MakeSubKnob(Feature::SoftShadows, "shadowReach", "Soft shadow reach",
                        "metres along the sun ray over which a shadow keeps softening", &LT::shadowReach, 2.0f, 80.0f),
            MakeSubKnob(Feature::SeaWaves, "seaRelief", "Wave relief", "0 = a flat sea; 1 = the shipped waves",
                        &LT::seaRelief, 0.0f, 2.0f),
            MakeSubKnob(Feature::SeaWaves, "seaSky", "Sea sky reflection",
                        "the mean share of sky the sea reflects; the waves' share stays", &LT::seaSky, 0.0f, 2.0f),
            MakeSubKnob(Feature::SeaWaves, "seaSheen", "Sea sheen",
                        "the broad sheen toward the sun, in the original hue", &LT::seaSheen, 0.0f, 3.0f),
            MakeSubKnob(Feature::PhotoLight, "camEv", "Exposure compensation", "EV on top of the metered exposure",
                        &LT::camEv, -3.0f, 3.0f),
            MakeSubKnob(Feature::PhotoLight, "camAdapt", "Eye adaptation time",
                        "seconds; opening up to the dark takes 1.5 x as long", &LT::camAdapt, 0.0f, 5.0f),
            MakeSubKnob(Feature::PhotoLight, "camCurve", "Tone curve",
                        "0 filmic hue-keeping, 1 filmic per channel, 2 ACES fit, 3 Khronos Neutral", &LT::camCurve,
                        0.0f, 3.0f),
            MakeSubKnob(Feature::PhotoLight, "camContrast", "Curve contrast", "the toe: higher = deeper shadows",
                        &LT::camContrast, 0.8f, 2.0f),
            MakeSubKnob(Feature::PhotoLight, "camShoulder", "Curve shoulder", "lower = a longer highlight roll-off",
                        &LT::camShoulder, 0.8f, 1.0f),
            MakeSubKnob(Feature::PhotoLight, "camWhite", "Curve white", "scene light reaching white, in middle greys",
                        &LT::camWhite, 4.0f, 64.0f),
            MakeSubKnob(Feature::PhotoLight, "camSat", "Scene saturation", "1 = as lit", &LT::camSat, 0.5f, 1.6f),
            MakeSubKnob(Feature::PhotoLight, "camWb", "White balance",
                        "how far the camera adapts to the light's colour", &LT::camWb, 0.0f, 1.0f),
            MakeSubKnob(Feature::PhotoLight, "camSky", "Clear sky brightness",
                        "against the physical sky's (1); the dome, fog and the sea's sky", &LT::camSky, 0.2f, 3.0f),
            MakeSubKnob(Feature::PhotoLight, "camSkyHue", "Clear sky hue",
                        "under the camera, in place of atmoHue: 0 the original's, 1 physical", &LT::camSkyHue, 0.0f,
                        1.0f),
            MakeSubKnob(Feature::PhotoLight, "camNight", "Night exposure", "EV at full night", &LT::camNight, -3.0f,
                        1.0f),
            MakeSubKnob(Feature::PhotoLight, "camVignette", "Vignette", "corner darkening of the lens",
                        &LT::camVignette, 0.0f, 0.6f),
            MakeSubKnob(Feature::PhotoLight, "camBloom", "Highlight bloom", "glare around light brighter than white",
                        &LT::camBloom, 0.0f, 3.0f),
        };
        rows.insert(rows.end(), std::begin(subKnobs), std::end(subKnobs));
        count = static_cast<int>(rows.size());
    }
};

const KeyTable& Keys()
{
    static const KeyTable table;
    return table;
}

std::string Trim(const std::string& s)
{
    const size_t a = s.find_first_not_of(' ');
    if (a == std::string::npos)
        return {};
    const size_t b = s.find_last_not_of(' ');
    return s.substr(a, b - a + 1);
}

void AppendNumber(std::string& out, float v)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.6g", static_cast<double>(v));
    out += buf;
}

} // namespace

const FeatureInfo& GetFeatureInfo(Feature f)
{
    const int i = static_cast<int>(f);
    return kFeatures[i >= 0 && i < kFeatureCount ? i : 0];
}

const FeatureInfo* FindFeature(std::string_view key)
{
    for (const FeatureInfo& f : kFeatures)
    {
        if (key == f.key)
            return &f;
    }
    return nullptr;
}

float FeatureValue(const FeatureSet& set, Feature f)
{
    return set.*(GetFeatureInfo(f).member);
}

void SetFeatureValue(FeatureSet& set, Feature f, float value)
{
    set.*(GetFeatureInfo(f).member) = value;
}

bool FeatureOn(const FeatureSet& set, Feature f)
{
    return FeatureValue(set, f) > 0.0f;
}

int LightingKeyCount()
{
    return Keys().count;
}

const LightingKey& LightingKeyAt(int i)
{
    const KeyTable& t = Keys();
    return t.rows[i >= 0 && i < t.count ? i : 0];
}

const LightingKey* FindLightingKey(std::string_view key)
{
    const KeyTable& t = Keys();
    for (int i = 0; i < t.count; ++i)
    {
        if (key == t.rows[i].key)
            return &t.rows[i];
    }
    return nullptr;
}

float GetKeyValue(const LightingTuning& t, const LightingKey& k)
{
    switch (k.kind)
    {
        case KeyKind::Knob:
            return t.*(k.knob);
        case KeyKind::Toggle:
            return t.*(k.toggle) ? 1.0f : 0.0f;
        case KeyKind::Choice:
            return static_cast<float>(t.*(k.choice));
        case KeyKind::Switch:
            return FeatureValue(t.features, k.feature);
    }
    return 0.0f;
}

void SetKeyValue(LightingTuning& t, const LightingKey& k, float value)
{
    switch (k.kind)
    {
        case KeyKind::Knob:
            t.*(k.knob) = value;
            break;
        case KeyKind::Toggle:
            t.*(k.toggle) = value != 0.0f;
            break;
        case KeyKind::Choice:
            t.*(k.choice) = static_cast<int>(value);
            break;
        case KeyKind::Switch:
            SetFeatureValue(t.features, k.feature, value);
            break;
    }
}

float KeyDefault(const LightingKey& k)
{
    return GetKeyValue(LightingTuning{}, k);
}

bool ParseLightingMode(std::string_view word, LightingMode& mode)
{
    if (word == "classic")
        mode = LightingMode::Classic;
    else if (word == "remastered")
        mode = LightingMode::Remastered;
    else if (word == "baseline")
        mode = LightingMode::Baseline;
    else
        return false;
    return true;
}

LightingTuning TuningForMode(LightingMode mode)
{
    LightingTuning t{};
    t.remastered = mode != LightingMode::Classic;
    if (mode == LightingMode::Baseline)
    {
        for (int i = 0; i < kFeatureCount; ++i)
            SetFeatureValue(t.features, static_cast<Feature>(i), 0.0f);
    }
    return t;
}

std::string ApplyLightingItem(LightingTuning& t, const std::string& item, bool& modeSet)
{
    LightingMode mode;
    if (ParseLightingMode(item, mode))
    {
        t = TuningForMode(mode);
        modeSet = true;
        return {};
    }
    const size_t eq = item.find('=');
    if (eq == std::string::npos)
        return "expected classic, remastered, baseline or key=value";
    const std::string key = item.substr(0, eq);
    const char* valueText = item.c_str() + eq + 1;
    char* valueEnd = nullptr;
    const float v = std::strtof(valueText, &valueEnd);
    if (valueEnd == valueText || *valueEnd != '\0')
        return "bad value in " + item;
    const LightingKey* k = FindLightingKey(key);
    if (!k)
        return "unknown knob " + key;
    SetKeyValue(t, *k, v);
    return {};
}

std::string ApplyLightingSetting(LightingTuning& t, const std::string& setting, bool& modeSet)
{
    LightingTuning work = t;
    bool workModeSet = false;
    for (size_t start = 0;;)
    {
        const size_t comma = setting.find(',', start);
        const std::string item =
            Trim(setting.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
        const std::string why = ApplyLightingItem(work, item, workModeSet);
        if (!why.empty())
            return why;
        if (comma == std::string::npos)
            break;
        start = comma + 1;
    }
    t = work;
    modeSet = modeSet || workModeSet;
    return {};
}

std::string TuningSummary(const LightingTuning& t)
{
    // The mode word whose switch values need the fewest overrides.
    int off = 0;
    for (int i = 0; i < kFeatureCount; ++i)
        off += FeatureOn(t.features, static_cast<Feature>(i)) ? 0 : 1;
    const LightingMode mode = !t.remastered             ? LightingMode::Classic
                              : off * 2 > kFeatureCount ? LightingMode::Baseline
                                                        : LightingMode::Remastered;
    const LightingTuning base = TuningForMode(mode);
    std::string out = mode == LightingMode::Classic    ? "classic"
                      : mode == LightingMode::Baseline ? "baseline"
                                                       : "remastered";
    for (int i = 0; i < LightingKeyCount(); ++i)
    {
        const LightingKey& k = LightingKeyAt(i);
        const float v = GetKeyValue(t, k);
        if (k.kind == KeyKind::Switch && v == GetKeyValue(base, k))
            continue;
        out += ',';
        out += k.key;
        out += '=';
        AppendNumber(out, v);
    }
    return out;
}

} // namespace Poseidon::remaster
