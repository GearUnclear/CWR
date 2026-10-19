#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>

#include <cstdio>

namespace Poseidon::remaster
{

namespace
{

Gate On(const char* reason, float scale = 1.0f)
{
    return Gate{true, scale, reason};
}

Gate Off(const char* reason, float scale = 1.0f)
{
    return Gate{false, scale, reason};
}

// The pass resolution of each feature (plan section 3.2): the air pass, the
// SSR trace and the combat shadow mask run at half resolution; everything else
// at full or has no screen pass of its own (the canopy's sun-visibility map is
// a world-space map around the camera, Canopy.hpp, not a screen buffer).
float FeatureScale(Feature f)
{
    switch (f)
    {
        case Feature::CombatShadows:
        case Feature::Ssr:
        case Feature::AirBeams:
        case Feature::AirHaze:
        case Feature::ValleyMist:
            return 0.5f;
        default:
            return 1.0f;
    }
}

bool FrameCostFeature(Feature f)
{
    return f == Feature::DrawGrouping || f == Feature::HillCull;
}

bool Morning(const GateInputs& in)
{
    const bool east = in.sunAzimuthDeg > 0.0f && in.sunAzimuthDeg < 180.0f;
    return east && in.sunElevationDeg > kGateMorningMinSunDeg && in.sunElevationDeg < kGateMorningMaxSunDeg;
}

// The moon as the key light: B3 routes it through the sun slots and the shadow
// map only when the sun is well under the horizon and the moon is up and
// bright enough.
Gate MoonKey(const GateInputs& in)
{
    if (in.nightEffect <= kGateNightMinEffect)
        return Off("day");
    if (in.sunElevationDeg >= kGateMoonKeyMaxSunDeg)
        return Off("sun above -3 deg");
    if (in.moonElevationDeg <= kGateMoonKeyMinMoonDeg)
        return Off("moon below 3 deg");
    if (in.moonPhase <= kGateMoonKeyMinPhase)
        return Off("moon phase below 0.25");
    return On("moon key");
}

// Lamps and headlights light the scene only at night: the light upload
// scales them by the night effect (EngineGL33::UploadLitLights skips them by
// day), so a lamp that is on at dusk adds nothing yet.
bool LampsLit(const GateInputs& in)
{
    return in.localLights > 0 && in.nightEffect > kGateNightMinEffect;
}

bool WorldVisible(const GateInputs& in, const char*& why)
{
    if (!in.worldDrawn)
        why = "no world pass";
    else if (in.interior)
        why = "vehicle interior";
    else if (in.optics)
        why = "optics overlay";
    else
        return true;
    return false;
}

// The feature's own condition, with its switch on and the gates in force.
Gate Condition(Feature f, const GateInputs& in, const LightingTuning& t)
{
    const float scale = FeatureScale(f);
    const char* why = "";
    switch (f)
    {
        case Feature::DrawGrouping:
        case Feature::HillCull:
        case Feature::LowSunRig:
        case Feature::CrownCanopy:
            return On("always");
        case Feature::SeaWaves:
            return On("per sea fragment");
        case Feature::TerrainDetail:
            return On("per terrain fragment within 300 m");
        case Feature::AirHaze:
            return On("beyond 150 m", scale);

        case Feature::Taa:
            // The resolve is a post stage: no post pass, no resolve (and an
            // unresolved jitter would shake the frame).
            if (!t.postEffects)
                return Off("post effects off");
            if (!WorldVisible(in, why))
                return Off(why);
            return On("world drawn");

        case Feature::LightTiles:
            // The combat lights shade from the tiled list (B4).
            if (in.combatLightsNear > 0 && FeatureOn(t.features, Feature::CombatLights))
                return On("combat light within 60 m");
            if (in.localLights <= 0)
                return Off("no local light");
            if (!LampsLit(in))
                return Off("day (lamps add no light)");
            return On("local light at night");

        case Feature::Atmosphere:
            if (in.sunElevationDeg <= kGateSkyTableMinSunDeg)
                return Off("sun below -9 deg");
            if (!in.skyTablesReady)
                return Off("atmosphere tables building");
            return On("sun above -9 deg");

        case Feature::TreeShadows:
        case Feature::SoftShadows:
            if (in.sunElevationDeg > kGateSunShadowMinSunDeg)
                return On("sun above 4 deg");
            if (FeatureOn(t.features, Feature::MoonShadows) && MoonKey(in).on)
                return On("moon key");
            return Off("sun below 4 deg");

        case Feature::NightLights:
            // The physical lamps and low beams are a model of the tiled light
            // list (the per-draw path keeps the classic one).
            if (in.nightEffect <= kGateNightMinEffect)
                return Off("day");
            if (!FeatureOn(t.features, Feature::LightTiles))
                return Off("light list off");
            return On("night");

        case Feature::Stars:
            if (in.nightEffect <= kGateNightMinEffect)
                return Off("day");
            return On("night");

        case Feature::MoonShadows:
            return MoonKey(in);

        case Feature::CombatLights:
            // The combat lights are entries of the tiled light list: without
            // it the classic lights of the same sources light the scene.
            if (!FeatureOn(t.features, Feature::LightTiles))
                return Off("light list off");
            if (in.combatLightsNear <= 0)
                return Off("no combat light within 60 m");
            return On("combat light within 60 m");

        case Feature::CombatShadows:
            if (!FeatureOn(t.features, Feature::CombatLights) || !FeatureOn(t.features, Feature::LightTiles))
                return Off("combat light off", scale);
            if (in.combatLightsNear <= 0)
                return Off("no combat light within 60 m", scale);
            if (in.nightEffect <= kGateCombatMaskMinNight)
                return Off("day (night below 0.3)", scale);
            // The mask traces through the post pass's half-resolution depth.
            if (!t.postEffects)
                return Off("post effects off", scale);
            if (!WorldVisible(in, why))
                return Off(why, scale);
            return On("combat light at night", scale);

        case Feature::Ssr:
        {
            if (!WorldVisible(in, why))
                return Off(why, scale);
            if (in.waterShare < 0.0f && in.wetShare < 0.0f)
                return On("screen share not measured", scale);
            // The ground reflects only while it is wet: the knob's wetness
            // when it forces one, else the weather's.
            const bool wet = (t.ssrWet >= 0.0f ? t.ssrWet : GateWetness(in)) > kGateWetMin;
            const float share =
                (in.waterShare > 0.0f ? in.waterShare : 0.0f) + (wet && in.wetShare > 0.0f ? in.wetShare : 0.0f);
            if (share <= kGateSsrMinShare)
                return Off(wet ? "water and wet ground below 1% of the screen" : "water below 1% of the screen, dry",
                           scale);
            return On("water or wet ground above 1% of the screen", scale);
        }

        case Feature::WetWeather:
            if (GateWetness(in) <= kGateWetMin)
                return Off("dry");
            return On("wet");

        case Feature::LampStreaks:
            // The streak is the wet film's reflection of the light.
            if (!FeatureOn(t.features, Feature::WetWeather))
                return Off("wet weather off");
            if (GateWetness(in) <= kGateWetMin)
                return Off("dry");
            if (!LampsLit(in))
                return Off("no lamp lit");
            return On("wet with lamps lit");

        case Feature::AirBeams:
        {
            const bool lowSunInView = in.sunElevationDeg > kGateBeamsMinSunDeg &&
                                      in.sunElevationDeg < kGateBeamsMaxSunDeg &&
                                      in.sunViewAngleDeg < kGateBeamsMaxViewAngleDeg;
            if (lowSunInView)
                return On("low sun in view", scale);
            if (in.fog > kGateMistMinFog || (FeatureOn(t.features, Feature::ValleyMist) && Morning(in)))
                return On("mist", scale);
            if (in.spotLights > 0 && in.nightEffect > kGateNightMinEffect)
                return On("headlights at night", scale);
            return Off("no low sun in view, mist or headlights", scale);
        }

        case Feature::ValleyMist:
            if (Morning(in))
                return On("morning", scale);
            if (in.fog > kGateMistMinFog)
                return On("fog", scale);
            return Off("not morning, no fog", scale);

        case Feature::CanopyShadows:
            if (in.sunElevationDeg <= kGateCanopyMinSunDeg || in.sunElevationDeg >= kGateCanopyMaxSunDeg)
                return Off("sun outside 2 to 25 deg", scale);
            if (in.skyThrough <= kGateCanopyMinSkyThrough)
                return Off("overcast (sky-through 0.5 or less)", scale);
            return On("low sun, clear sky", scale);

        case Feature::LitClouds:
            if (in.skyThrough >= kGateLitCloudsMaxSkyThrough)
                return Off("clear sky");
            if (in.sunElevationDeg <= kGateSkyTableMinSunDeg)
                return Off("sun below -9 deg");
            if (!in.skyDensityReady)
                return Off("sky density map building");
            return On("clouds in the sky");

        case Feature::SkyCloudShadows:
            if (in.cloudCover <= 0.0f)
                return Off("clear sky (cover below 0.05)");
            // Cloud shadows dim the key light: the sun, or the moon as B3's key.
            if (in.sunElevationDeg <= kGateSkyShadowsMinSunDeg &&
                !(FeatureOn(t.features, Feature::MoonShadows) && MoonKey(in).on))
                return Off("no sun or moon key");
            // Until the map exists the noise field casts them.
            if (!in.skyDensityReady)
                return Off("sky density map building");
            return On("cloud cover");

        case Feature::Sharpen:
            // One sharpen pass: TAA's own when it runs, this one otherwise.
            if (FeatureOn(t.features, Feature::Taa) && WorldVisible(in, why))
                return Off("TAA sharpens");
            return On("TAA off");

        case Feature::Count:
            break;
    }
    return Off("unknown feature");
}

std::string Slug(const char* text)
{
    std::string s = text ? text : "";
    for (char& c : s)
    {
        if (c == ' ' || c == ';' || c == '|')
            c = '-';
    }
    return s;
}

} // namespace

float GateWetness(const GateInputs& in)
{
    return in.wetness >= 0.0f ? in.wetness : in.rainDensity;
}

FeatureGates ComputeFeatureGates(const GateInputs& in, const LightingTuning& tuning)
{
    FeatureGates g;
    g.inputs = in;
    g.computed = true;
    for (int i = 0; i < kFeatureCount; ++i)
    {
        const Feature f = static_cast<Feature>(i);
        const float scale = FeatureScale(f);
        Gate& out = g.gate[i];
        if (!tuning.remastered)
            out = Off("classic lighting", scale);
        else if (!FeatureOn(tuning.features, f))
            out = Off("switch off", scale);
        else if (in.nightVision && !FrameCostFeature(f))
            out = Off("night vision", scale);
        else if (!tuning.gates)
            out = On("forced (gates off)", scale);
        else
            out = Condition(f, in, tuning);
    }
    return g;
}

std::string FeatureStateString(const LightingTuning& tuning, const FeatureGates& gates)
{
    const GateInputs& in = gates.inputs;
    int landed = 0;
    for (int i = 0; i < kFeatureCount; ++i)
        landed += GetFeatureInfo(static_cast<Feature>(i)).landed ? 1 : 0;
    char head[384];
    std::snprintf(
        head, sizeof(head),
        "mode=%s gates=%d computed=%d frame=%u sun=%.1f night=%.2f sky=%.2f rain=%.2f wet=%.2f fog=%.2f lights=%d "
        "landed=%d/%d |",
        tuning.remastered ? "remastered" : "classic", tuning.gates ? 1 : 0, gates.computed ? 1 : 0, gates.frame,
        static_cast<double>(in.sunElevationDeg), static_cast<double>(in.nightEffect),
        static_cast<double>(in.skyThrough), static_cast<double>(in.rainDensity), static_cast<double>(GateWetness(in)),
        static_cast<double>(in.fog), in.localLights, landed, kFeatureCount);
    std::string out = head;
    for (int i = 0; i < kFeatureCount; ++i)
    {
        const Feature f = static_cast<Feature>(i);
        const FeatureInfo& info = GetFeatureInfo(f);
        const Gate& gt = gates[f];
        char item[64];
        std::snprintf(item, sizeof(item), " %s=%g:%s", info.key, static_cast<double>(FeatureValue(tuning.features, f)),
                      gt.on ? "on" : "off");
        out += item;
        if (gt.scale != 1.0f)
        {
            std::snprintf(item, sizeof(item), "@%g", static_cast<double>(gt.scale));
            out += item;
        }
        out += ':';
        out += Slug(gt.reason);
        if (!info.landed)
            out += ":inert";
    }
    return out;
}

} // namespace Poseidon::remaster
