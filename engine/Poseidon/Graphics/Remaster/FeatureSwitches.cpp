#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>

#include <Poseidon/Graphics/Remaster/PostStages.hpp>
#include <Poseidon/Graphics/Remaster/ScreenReflections.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/CombatLights.hpp>

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

// One row per Feature, in enum order (checked below). `landed` is true once
// the switch has an effect. The last field is the GpuTimings section the
// feature's cost shows in.
constexpr FeatureInfo kFeatures[] = {
    // Both pixel-exact; their GPU time shows in the opaque objects' section of
    // the world pass.
    {Feature::DrawGrouping, "drawGrouping", "Draw grouping", "perf", "Frame cost", "frame cost", true,
     &FS::drawGrouping, "pass1"},
    {Feature::HillCull, "hillCull", "Hill culling", "perf", "Frame cost", "frame cost", true, &FS::hillCull, "pass1"},
    {Feature::Taa, "taa", "Temporal anti-aliasing", "temporal", "Light and shadow", "temporal AA", true, &FS::taa,
     "taa"},
    // No pass of its own; the per-fragment light loop is part of the world
    // pass's shading.
    {Feature::LightTiles, "lightTiles", "Tiled light list", "light list", "Night and combat", "light list", true,
     &FS::lightTiles, "world"},
    {Feature::Atmosphere, "atmosphere", "Sky table (dome and fog colour)", "atmosphere", "Sky and air", "sky table",
     true, &FS::atmosphere, "sky"},
    // The casters' cost shows in the shadow depth pass, the soft filter's in
    // the world pass that samples it.
    {Feature::TreeShadows, "treeShadows", "Tree and building sun shadows", "shadows", "Light and shadow", "sun shadows",
     true, &FS::treeShadows, "shadow"},
    {Feature::SoftShadows, "softShadows", "Soft sun shadows", "shadows", "Light and shadow", "sun shadows", true,
     &FS::softShadows, "world"},
    // The low-sun rig is constants computed once a frame (no GPU cost of its
    // own; the world pass is what it lights); the crown canopy is a few terms
    // on the world pass's foliage.
    {Feature::LowSunRig, "lowSunRig", "Low-sun rig", "shade", "Light and shadow", "low sun", true, &FS::lowSunRig,
     "world"},
    {Feature::CrownCanopy, "crownCanopy", "Tree crowns as a canopy", "shade", "Light and shadow", "low sun", true,
     &FS::crownCanopy, "world"},
    // The lamps and low beams shade in the world pass, the moon's shadow map
    // renders in the shadow depth pass, the stars draw with the sky (part of
    // the world section).
    {Feature::NightLights, "nightLights", "Lamps and low beams", "night", "Night and combat", "night", true,
     &FS::nightLights, "world"},
    {Feature::MoonShadows, "moonShadows", "Moon shadows", "night", "Night and combat", "night", true, &FS::moonShadows,
     "shadow"},
    {Feature::Stars, "stars", "Stars", "night", "Night and combat", "night", true, &FS::stars, "world"},
    // The combat lights shade in the world pass (their scattering in the air
    // adds to the composite); their shadow mask has its own pass at the start
    // of the world (combatmask).
    {Feature::CombatLights, "combatLights", "Combat light", "combat", "Night and combat", "combat light", true,
     &FS::combatLights, "world"},
    {Feature::CombatShadows, "combatShadows", "Combat light shadows", "combat", "Night and combat", "combat light",
     true, &FS::combatShadows, "combatmask"},
    // The sea's draws, in the ground section of the world pass (sky, terrain,
    // water, horizon).
    {Feature::SeaWaves, "seaWaves", "Sea waves", "water", "Water and shine", "sea", true, &FS::seaWaves, "ground"},
    // The tag readout and the reflections, two post stages.
    {Feature::Ssr, "ssr", "Screen-space reflections", "ssr", "Water and shine", "reflections", true, &FS::ssr, "ssr"},
    // The wet surfaces, the puddles' darkening and the lamp streaks shade in
    // the world pass; the puddles' reflections are the ssr stage's.
    {Feature::WetWeather, "wetWeather", "Wet weather and puddles", "materials", "Water and shine", "wet weather", true,
     &FS::wetWeather, "world"},
    {Feature::LampStreaks, "lampStreaks", "Lamp streaks on wet ground", "materials", "Water and shine", "wet weather",
     true, &FS::lampStreaks, "world"},
    // One post stage, the air pass, for all three.
    {Feature::AirBeams, "airBeams", "Sun beams and headlight cones", "volumetric", "Sky and air", "air", true,
     &FS::airBeams, "air"},
    {Feature::AirHaze, "airHaze", "Aerial haze", "haze", "Sky and air", "air", true, &FS::airHaze, "air"},
    {Feature::ValleyMist, "valleyMist", "Valley mist", "haze", "Sky and air", "air", true, &FS::valleyMist, "air"},
    // The sun-visibility map's build shows in its own section on the frames
    // that rebuild it; the one read per fragment is in the world pass.
    {Feature::CanopyShadows, "canopyShadows", "Long canopy shadows", "hfrt", "Light and shadow", "canopy shadows", true,
     &FS::canopyShadows, "canopy"},
    // Lit clouds cost in the sky pass (the dome and the sky clouds), the
    // sky-matched shadows one density tap per lit world fragment.
    {Feature::LitClouds, "litClouds", "Lit sky clouds", "clouds", "Sky and air", "lit clouds", true, &FS::litClouds,
     "sky"},
    {Feature::SkyCloudShadows, "skyCloudShadows", "Sky-matched cloud shadows", "clouds", "Sky and air", "lit clouds",
     true, &FS::skyCloudShadows, "world"},
    // The terrain detail runs inside the ground draw, the luma sharpen in the
    // post pass's sharpen stage (TAA's own sharpen when TAA runs).
    {Feature::TerrainDetail, "terrainDetail", "Terrain detail", "texsharp", "Light and shadow", "detail and sharpen",
     true, &FS::terrainDetail, "ground"},
    {Feature::Sharpen, "sharpen", "Sharpen (with TAA off)", "texsharp", "Light and shadow", "detail and sharpen", true,
     &FS::sharpen, "sharpen"},
    // The horizon occlusion is the post pass's ao stage; the sky light, canopy
    // occlusion and specular are terms of the world pass's shading.
    {Feature::Surfaces, "surfaces", "Surfaces and ambient light", "surface", "Light and shadow", "surfaces", true,
     &FS::surfaces, "ao"},
    // PhotoCamera.hpp: the physical rig is constants once a frame (plus the sky
    // integral when the sun has moved), the camera is the meter pass and a few
    // terms of the composite.
    {Feature::PhotoLight, "photoLight", "Photoreal light and camera", "look-light", "Light and shadow",
     "photoreal camera", true, &FS::photoLight, "composite"},
    // The volumetric fog (VolumetricFog.hpp): a froxel volume of haze, weather
    // fog and valley mist lit by the sun, the sky and the local lights,
    // applied in the post pass's Air stage in place of the flat distance fog
    // (which keeps only the view-distance edge). It supersedes the plain air
    // pass (airBeams, airHaze, valleyMist) on the same stage.
    {Feature::VolFog, "volFog", "Volumetric fog and atmosphere", "fog", "Sky and air", "volumetric fog", true,
     &FS::volFog, "air"},
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
                       "Linear depth (half res)\0Light tiles (lights per tile)\0Albedo\0Air scattering\0"
                       "Reflection tags\0Reflections only\0Combat shadow mask\0"
                       "Sky light (surfaces)\0Canopy sky visibility (surfaces)\0Specular (surfaces)\0",
                       &LT::debugView, 0, kDebugViewSurfaceSpecular),
            MakeToggle("gates", "Frame gates", "Gates", "off = enabled features run on every frame (debugging)",
                       &LT::gates),
        };
        rows.assign(std::begin(knobs), std::end(knobs));
        for (const FeatureInfo& f : kFeatures)
            rows.push_back(MakeSwitch(f));
        // Sub-knobs, grouped by feature.
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
            MakeSubKnob(Feature::VolFog, "fogDensity", "Fog density",
                        "every part of the air: haze, weather fog and mist; 1 = shipped", &LT::fogDensity, 0.0f, 4.0f),
            MakeSubKnob(Feature::VolFog, "fogQuality", "Fog quality",
                        "froxel grid: 0 = 16 px cells, 48 slices; 1 = 12 px, 64; 2 = 8 px, 96", &LT::fogQuality, 0.0f,
                        2.0f),
            MakeSubKnob(Feature::VolFog, "fogMist", "Valley mist",
                        "the morning, evening and weather mist pooled on low ground", &LT::fogMist, 0.0f, 4.0f),
            MakeSubKnob(Feature::VolFog, "fogSun", "Sun in the air",
                        "the sun's light scattered by the air: beams and the glow toward the sun", &LT::fogSun, 0.0f,
                        4.0f),
            MakeSubKnob(Feature::VolFog, "fogLights", "Lights in the fog",
                        "lamps, headlights and fires glowing in the air", &LT::fogLights, 0.0f, 4.0f),
            MakeSubKnob(Feature::AirBeams, "volGain", "Beam contrast",
                        "-1 = from the weather (4 in clear air, 2.5 in mist); 1 = no boost of the shafts", &LT::volGain,
                        -1.0f, 6.0f),
            MakeSubKnob(Feature::NightLights, "nightLamp", "Street lamp strength",
                        "the lamps' intensity; 0 = street lamps light nothing", &LT::nightLamp, 0.0f, 4.0f),
            MakeSubKnob(Feature::NightLights, "nightSpot", "Headlight strength",
                        "the low beam's hot spot; 0 = headlights light nothing", &LT::nightSpot, 0.0f, 4.0f),
            MakeSubKnob(Feature::Ssr, "ssrWater", "Water reflection",
                        "the share of the sea's light a reflection takes, times the sky's estimated share",
                        &LT::ssrWater, 0.0f, 2.0f),
            MakeSubKnob(Feature::Ssr, "ssrWet", "Ground wetness",
                        "-1 = the weather (the wetness state, the rain with wetWeather off); 0..1 forces it",
                        &LT::ssrWet, -1.0f, 1.0f),
            MakeSubKnob(Feature::WetWeather, "wetness", "Wetness",
                        "-1 = the weather (rain wets, sun and wind dry); 0..1 forces it, also after the rain",
                        &LT::wetness, -1.0f, 1.0f),
            MakeSubKnob(Feature::WetWeather, "puddles", "Puddles", "the puddles' amount; 0 = none, 1 = the weather's",
                        &LT::puddles, 0.0f, 2.0f),
            MakeSubKnob(Feature::CombatLights, "combatFlash", "Burst flash",
                        "the white-hot first flash of shells and grenades; also the flash's exposure response",
                        &LT::combatFlash, 0.0f, 3.0f),
            MakeSubKnob(Feature::CombatLights, "combatFire", "Fire light", "camp fires and burning wrecks",
                        &LT::combatFire, 0.0f, 3.0f),
            MakeSubKnob(Feature::CombatLights, "combatSmoke", "Lit smoke", "combat light on smoke and dust",
                        &LT::combatSmoke, 0.0f, 3.0f),
            MakeSubKnob(Feature::CombatLights, "combatScatter", "Lit air", "combat light scattered in the air",
                        &LT::combatScatter, 0.0f, 3.0f),
            MakeSubKnob(Feature::Surfaces, "surfAo", "Horizon occlusion",
                        "contact darkening of the sky light; 0 = the stock occlusion", &LT::surfAo, 0.0f, 4.0f),
            MakeSubKnob(Feature::Surfaces, "surfAoRadius", "Occlusion radius",
                        "metres the horizon search reaches around each pixel", &LT::surfAoRadius, 0.5f, 6.0f),
            MakeSubKnob(Feature::Surfaces, "surfCanopyAo", "Canopy occlusion",
                        "sky light lost under tree crowns, forests and building blocks", &LT::surfCanopyAo, 0.0f, 1.5f),
            MakeSubKnob(Feature::Surfaces, "surfSkyLight", "Directional sky light",
                        "share of the sky light from the sky table; 0 = the rig's hemisphere (default)",
                        &LT::surfSkyLight, 0.0f, 1.0f),
            MakeSubKnob(Feature::Surfaces, "surfSkyTint", "Sky light colour",
                        "0 = the rig's own colour toward each face, 1 = the physical sky's", &LT::surfSkyTint, 0.0f,
                        1.0f),
            MakeSubKnob(Feature::Surfaces, "surfSkyLevel", "Sky light level",
                        "0 = the rig's shade fill, 1 = the sky the air gives against the sun (deeper shade)",
                        &LT::surfSkyLevel, 0.0f, 1.0f),
            MakeSubKnob(Feature::Surfaces, "surfSpec", "Surface specular",
                        "Fresnel reflection of sun and sky on every surface", &LT::surfSpec, 0.0f, 2.0f),
            MakeSubKnob(Feature::Surfaces, "surfContact", "Contact shadows",
                        "screen-space sun shadows of small details", &LT::surfContact, 0.0f, 1.0f),
            MakeSubKnob(Feature::Surfaces, "surfRelief", "Surface relief",
                        "relief the textures give the land and objects at a low sun; 0 = flat", &LT::surfRelief, 0.0f,
                        4.0f),
            MakeSubKnob(Feature::Surfaces, "surfShade", "Shade depth",
                        "share of the sky fill shaded surfaces keep under a clear day sky; 1 = the original's",
                        &LT::surfShade, 0.2f, 1.0f),
            MakeSubKnob(Feature::Surfaces, "surfCavity", "Surface cavities",
                        "sun and sky the texture's pits lose (stones, ruts, mortar); 0 = none", &LT::surfCavity, 0.0f,
                        3.0f),
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
                        "the sky table's sky against its physical brightness (1); the dome, fog and the sea's sky",
                        &LT::camSky, 0.2f, 3.0f),
            MakeSubKnob(Feature::PhotoLight, "camSkyHue", "Clear sky hue",
                        "under the camera, in place of atmoHue: 0 the original's, 1 physical", &LT::camSkyHue, 0.0f,
                        1.0f),
            MakeSubKnob(Feature::PhotoLight, "camNight", "Night exposure", "EV at full night", &LT::camNight, -5.0f,
                        1.0f),
            MakeSubKnob(Feature::PhotoLight, "camVignette", "Vignette", "corner darkening of the lens",
                        &LT::camVignette, 0.0f, 0.6f),
            MakeSubKnob(Feature::PhotoLight, "camBloom", "Highlight bloom", "glare around light brighter than white",
                        &LT::camBloom, 0.0f, 3.0f),
            MakeSubKnob(Feature::PhotoLight, "camHaze", "Air haze",
                        "the sky table's haze under the camera, times atmoHaze (rebuilds the atmosphere tables)",
                        &LT::camHaze, 1.0f, 40.0f),
            MakeSubKnob(Feature::PhotoLight, "camScotopic", "Night rod shift",
                        "dim night light toward a cool grey; 0 = none", &LT::camScotopic, 0.0f, 1.5f),
            MakeSubKnob(Feature::PhotoLight, "camAdditive", "Additive particle scale",
                        "fire, flashes and flares; 1 = as drawn", &LT::camAdditive, 0.3f, 2.0f),
            MakeSubKnob(Feature::PhotoLight, "camAerial", "Aerial perspective",
                        "the air pass's haze density under the camera; 1 = the air pass's own", &LT::camAerial, 0.0f,
                        10.0f),
            MakeSubKnob(Feature::PhotoLight, "camLocal", "Night local lights",
                        "lamps, fires and headlights at full night; 1 = as lit", &LT::camLocal, 0.5f, 6.0f),
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
