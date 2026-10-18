#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Graphics/Core/GpuTimings.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>

#include <set>
#include <string>
#include <string_view>

// The registry of remastered lighting keys and the per-frame feature gates
// (INTEGRATION-PLAN.md sections 3.1 and 3.2). The registry is the one table
// triSetLighting, the dev overlay and the capture tooling read, so these
// cases pin what every later step relies on: each feature has a key, a label
// and a default of 1; "baseline" turns every switch off and "remastered"
// turns them back on; a gate is off exactly where its feature cannot show.

using namespace Poseidon::remaster;

namespace
{

bool IsArea(const char* area)
{
    for (const char* a : kFeatureAreas)
    {
        if (std::string_view(a) == area)
            return true;
    }
    return false;
}

// Applies a triSetLighting setting to the shipped tuning; fails the test on a
// refusal.
LightingTuning Apply(const std::string& setting)
{
    LightingTuning t{};
    bool modeSet = false;
    const std::string why = ApplyLightingSetting(t, setting, modeSet);
    INFO(setting << ": " << why);
    REQUIRE(why.empty());
    return t;
}

// Inputs of a clear summer noon seen from the ground, no lights, dry.
GateInputs Noon()
{
    GateInputs in;
    in.sunElevationDeg = 60.0f;
    in.sunAzimuthDeg = 180.0f;
    in.sunViewAngleDeg = 90.0f;
    in.moonElevationDeg = -30.0f;
    in.nightEffect = 0.0f;
    in.skyThrough = 1.0f;
    return in;
}

// A moonless night with no light near.
GateInputs Night()
{
    GateInputs in = Noon();
    in.sunElevationDeg = -30.0f;
    in.nightEffect = 1.0f;
    in.moonElevationDeg = -10.0f;
    return in;
}

const Gate& G(const GateInputs& in, Feature f, const LightingTuning& t = LightingTuning{})
{
    static FeatureGates last;
    last = ComputeFeatureGates(in, t);
    return last[f];
}

} // namespace

TEST_CASE("Feature switches: every feature has a key, a label and a default of 1", "[remaster][FeatureSwitches]")
{
    std::set<std::string> keys;
    for (int i = 0; i < kFeatureCount; ++i)
    {
        const auto f = static_cast<Feature>(i);
        const FeatureInfo& info = GetFeatureInfo(f);
        INFO("feature " << i);
        CHECK(info.id == f);
        REQUIRE(info.key);
        INFO(info.key);
        CHECK(info.label[0] != '\0');
        CHECK(info.track[0] != '\0');
        CHECK(info.step[0] != '\0');
        CHECK(IsArea(info.area));
        CHECK(keys.insert(info.key).second);
        CHECK(FindFeature(info.key) == &info);

        const LightingKey* k = FindLightingKey(info.key);
        REQUIRE(k);
        CHECK(k->kind == KeyKind::Switch);
        CHECK(k->feature == f);
        CHECK(std::string_view(k->label) == info.label);
        CHECK(KeyDefault(*k) == 1.0f);
        CHECK(FeatureOn(LightingTuning{}.features, f));
    }
    CHECK(keys.size() == static_cast<size_t>(kFeatureCount));
}

TEST_CASE("Feature switches: each switch reads and writes its own FeatureSet member", "[remaster][FeatureSwitches]")
{
    FeatureSet set;
    for (int i = 0; i < kFeatureCount; ++i)
        SetFeatureValue(set, static_cast<Feature>(i), static_cast<float>(i + 2));
    for (int i = 0; i < kFeatureCount; ++i)
        CHECK(FeatureValue(set, static_cast<Feature>(i)) == static_cast<float>(i + 2));
    // The named members the features read are the registry's.
    CHECK(set.drawGrouping == 2.0f);
    CHECK(set.taa == 4.0f);
    CHECK(set.combatLights == 14.0f);
    CHECK(set.sharpen == static_cast<float>(kFeatureCount + 1));
}

TEST_CASE("Feature switches: the table holds every knob once, with a usable range", "[remaster][FeatureSwitches]")
{
    std::set<std::string> keys;
    int switches = 0;
    for (int i = 0; i < LightingKeyCount(); ++i)
    {
        const LightingKey& k = LightingKeyAt(i);
        INFO(k.key);
        CHECK(keys.insert(k.key).second);
        CHECK(k.label[0] != '\0');
        CHECK(FindLightingKey(k.key) == &k);
        if (k.kind == KeyKind::Switch)
        {
            ++switches;
            continue;
        }
        CHECK(k.minValue < k.maxValue);
        const float d = KeyDefault(k);
        CHECK(d >= k.minValue);
        CHECK(d <= k.maxValue);
        if (k.kind == KeyKind::Choice)
            CHECK(k.choices != nullptr);
    }
    CHECK(switches == kFeatureCount);
    // The keys triSetLighting took before the registry, plus the gates knob.
    for (const char* key : {"exposure", "sun", "sky", "bounce", "warmth", "saturation", "specular", "localLights",
                            "moonlight", "terrainShadows", "terrainOcclusion", "cloudShadows", "shadowSoftness", "ao",
                            "bloom", "shafts", "post", "debug", "gates"})
    {
        INFO(key);
        CHECK(FindLightingKey(key) != nullptr);
    }
    CHECK(FindLightingKey("remastered") == nullptr);
    CHECK(FindLightingKey("nonsense") == nullptr);
}

TEST_CASE("Feature switches: baseline turns every switch off, remastered turns them back on",
          "[remaster][FeatureSwitches]")
{
    const LightingTuning base = Apply("baseline");
    CHECK(base.remastered);
    for (int i = 0; i < LightingKeyCount(); ++i)
    {
        const LightingKey& k = LightingKeyAt(i);
        INFO(k.key);
        if (k.kind == KeyKind::Switch)
            CHECK(GetKeyValue(base, k) == 0.0f);
        else
            CHECK(GetKeyValue(base, k) == KeyDefault(k));
    }

    const LightingTuning back = Apply("bounce=2.5,baseline,remastered");
    CHECK(back.remastered);
    for (int i = 0; i < LightingKeyCount(); ++i)
    {
        const LightingKey& k = LightingKeyAt(i);
        INFO(k.key);
        CHECK(GetKeyValue(back, k) == KeyDefault(k));
    }

    const LightingTuning classic = Apply("baseline,classic");
    CHECK_FALSE(classic.remastered);
    for (int i = 0; i < kFeatureCount; ++i)
        CHECK(FeatureOn(classic.features, static_cast<Feature>(i)));
}

TEST_CASE("Feature switches: a switch key turns one feature off and leaves the rest", "[remaster][FeatureSwitches]")
{
    const LightingTuning t = Apply("remastered,combatLights=0,combatShadows=0");
    for (int i = 0; i < kFeatureCount; ++i)
    {
        const auto f = static_cast<Feature>(i);
        INFO(GetFeatureInfo(f).key);
        CHECK(FeatureOn(t.features, f) == (f != Feature::CombatLights && f != Feature::CombatShadows));
    }
    const LightingTuning on = Apply("baseline,taa=1");
    CHECK(FeatureOn(on.features, Feature::Taa));
    CHECK_FALSE(FeatureOn(on.features, Feature::Ssr));
}

TEST_CASE("Feature switches: a refused item leaves the tuning unchanged", "[remaster][FeatureSwitches]")
{
    LightingTuning t = Apply("remastered,taa=0");
    bool modeSet = false;
    CHECK(ApplyLightingSetting(t, "baseline,nonsense=1", modeSet).rfind("unknown knob nonsense", 0) == 0);
    CHECK(ApplyLightingSetting(t, "ssr=on", modeSet).rfind("bad value", 0) == 0);
    CHECK(ApplyLightingSetting(t, "", modeSet).rfind("expected", 0) == 0);
    CHECK_FALSE(modeSet);
    CHECK_FALSE(FeatureOn(t.features, Feature::Taa));
    CHECK(FeatureOn(t.features, Feature::Ssr));
}

TEST_CASE("Feature switches: the summary line is a setting that gives the tuning back", "[remaster][FeatureSwitches]")
{
    for (const char* setting :
         {"remastered", "classic", "baseline", "remastered,bounce=2.5,taa=0,combatLights=0,post=0,debug=3,gates=0",
          "baseline,ssr=1,exposure=1.3", "classic,sun=0.75,airBeams=0"})
    {
        INFO(setting);
        const LightingTuning want = Apply(setting);
        const std::string summary = TuningSummary(want);
        INFO(summary);
        LightingTuning got = Apply("remastered,sky=2,stars=0");
        bool modeSet = false;
        REQUIRE(ApplyLightingSetting(got, summary, modeSet).empty());
        CHECK(modeSet);
        CHECK(got.remastered == want.remastered);
        for (int i = 0; i < LightingKeyCount(); ++i)
        {
            const LightingKey& k = LightingKeyAt(i);
            INFO(k.key);
            CHECK(GetKeyValue(got, k) == GetKeyValue(want, k));
        }
    }
    CHECK(TuningSummary(Apply("baseline")).rfind("baseline,", 0) == 0);
    CHECK(TuningSummary(Apply("classic")).rfind("classic,", 0) == 0);
    CHECK(TuningSummary(Apply("remastered")).find("taa=") == std::string::npos);
}

// ---- frame gates: one case per gate row of plan section 3.2

TEST_CASE("Feature gates: frame-cost and constant features always run", "[remaster][FeatureGates]")
{
    for (const GateInputs& in : {Noon(), Night()})
    {
        for (Feature f : {Feature::DrawGrouping, Feature::HillCull, Feature::LowSunRig, Feature::CrownCanopy,
                          Feature::SeaWaves, Feature::TerrainDetail, Feature::AirHaze})
        {
            INFO(GetFeatureInfo(f).key);
            CHECK(G(in, f).on);
        }
    }
    GateInputs nv = Noon();
    nv.nightVision = true;
    CHECK(G(nv, Feature::DrawGrouping).on);
    CHECK(G(nv, Feature::HillCull).on);
}

TEST_CASE("Feature gates: TAA runs on a drawn world, not in interiors, optics or night vision",
          "[remaster][FeatureGates]")
{
    GateInputs in = Noon();
    CHECK(G(in, Feature::Taa).on);
    CHECK(G(in, Feature::Taa).scale == 1.0f);
    in.interior = true;
    CHECK_FALSE(G(in, Feature::Taa).on);
    in = Noon();
    in.optics = true;
    CHECK_FALSE(G(in, Feature::Taa).on);
    in = Noon();
    in.nightVision = true;
    CHECK_FALSE(G(in, Feature::Taa).on);
    CHECK(std::string_view(G(in, Feature::Taa).reason) == "night vision");
    in = Noon();
    in.worldDrawn = false;
    CHECK_FALSE(G(in, Feature::Taa).on);
    // The resolve is a post stage: with the post effects off nothing would
    // resolve the jittered frame.
    in = Noon();
    CHECK_FALSE(G(in, Feature::Taa, Apply("remastered,post=0")).on);
    CHECK(std::string_view(G(in, Feature::Taa, Apply("remastered,post=0")).reason) == "post effects off");
    CHECK(G(in, Feature::Taa, Apply("remastered,post=0,gates=0")).on); // the debugging override still wins
}

TEST_CASE("Feature switches: TAA has landed, with its sub-knobs under its switch", "[remaster][FeatureSwitches]")
{
    // A5 makes taa do something: it is no longer inert, and the overlay shows
    // its GPU section (the Temporal post stage's).
    const FeatureInfo& taa = GetFeatureInfo(Feature::Taa);
    CHECK(taa.landed);
    CHECK(std::string_view(taa.step) == "A5");
    CHECK(std::string_view(taa.gpuSection) == "taa");

    // Plan 3.1's sub-knobs: sharpen 0.5, texture LOD bias -0.5, alpha 0.1, each
    // under the taa switch, in its track's group.
    struct Sub
    {
        const char* key;
        float value;
    };
    for (const Sub& sub : {Sub{"taaSharpen", 0.5f}, Sub{"taaMipBias", -0.5f}, Sub{"taaAlpha", 0.1f}})
    {
        INFO(sub.key);
        const LightingKey* k = FindLightingKey(sub.key);
        REQUIRE(k != nullptr);
        CHECK(k->kind == KeyKind::Knob);
        REQUIRE(k->parent != nullptr);
        CHECK(std::string_view(k->parent) == "taa");
        CHECK(std::string_view(k->group) == "temporal");
        CHECK(KeyDefault(*k) == sub.value);
        // triSetLighting takes it, and baseline / remastered put it back.
        const LightingTuning t = Apply(std::string("remastered,") + sub.key + "=0.3");
        CHECK(GetKeyValue(t, *k) == 0.3f);
        CHECK(GetKeyValue(Apply(std::string(sub.key) + "=0.3,baseline"), *k) == sub.value);
    }
    // Every knob with a parent sits under a landed feature switch, in its
    // track's group: TAA's three, the sky table's three (A7, test_atmosphere.cpp),
    // texsharp's two (B11) and the sun shadows' three (B1, below).
    int children = 0;
    int taaChildren = 0;
    for (int i = 0; i < LightingKeyCount(); ++i)
    {
        const LightingKey& k = LightingKeyAt(i);
        if (!k.parent)
            continue;
        INFO(k.key);
        const std::string_view parent(k.parent);
        CHECK((parent == "taa" || parent == "atmosphere" || parent == "terrainDetail" || parent == "sharpen" ||
               parent == "softShadows" || parent == "treeShadows"));
        REQUIRE(FindFeature(k.parent) != nullptr);
        CHECK(FindFeature(k.parent)->landed);
        CHECK(std::string_view(k.group) == FindFeature(k.parent)->track);
        taaChildren += parent == "taa" ? 1 : 0;
        ++children;
    }
    CHECK(taaChildren == 3);
    CHECK(children == 11);
}

TEST_CASE("Feature switches: the terrain detail and the luma sharpen have landed, each with its strength",
          "[remaster][FeatureSwitches][TerrainDetail]")
{
    // B11 makes terrainDetail and sharpen do something: they are no longer
    // inert, and the overlay shows where their GPU time goes (the ground draw,
    // the post pass's sharpen stage).
    const FeatureInfo& detail = GetFeatureInfo(Feature::TerrainDetail);
    const FeatureInfo& sharpen = GetFeatureInfo(Feature::Sharpen);
    CHECK(detail.landed);
    CHECK(sharpen.landed);
    CHECK(std::string_view(detail.step) == "B11");
    CHECK(std::string_view(sharpen.step) == "B11");
    CHECK(std::string_view(detail.gpuSection) == "ground");
    CHECK(std::string_view(sharpen.gpuSection) == "sharpen");

    // Plan 3.1's sub-knob detailStrength 1.5 under terrainDetail, and the luma
    // sharpen's strength 0.3 (the research's CAS 0.3) under sharpen, both in
    // the texsharp group.
    struct Sub
    {
        const char* key;
        const char* parent;
        float value;
    };
    for (const Sub& sub : {Sub{"detailStrength", "terrainDetail", 1.5f}, Sub{"sharpenStrength", "sharpen", 0.3f}})
    {
        INFO(sub.key);
        const LightingKey* k = FindLightingKey(sub.key);
        REQUIRE(k != nullptr);
        CHECK(k->kind == KeyKind::Knob);
        REQUIRE(k->parent != nullptr);
        CHECK(std::string_view(k->parent) == sub.parent);
        CHECK(std::string_view(k->group) == "texsharp");
        CHECK(KeyDefault(*k) == sub.value);
        const LightingTuning t = Apply(std::string("remastered,") + sub.key + "=0.7");
        CHECK(GetKeyValue(t, *k) == 0.7f);
        CHECK(GetKeyValue(Apply(std::string(sub.key) + "=0.7,baseline"), *k) == sub.value);
    }

    // triFeatureState: the terrain detail runs by day and is not inert.
    const LightingTuning t = Apply("remastered");
    const FeatureGates g = ComputeFeatureGates(Noon(), t);
    const std::string s = FeatureStateString(t, g);
    INFO(s);
    CHECK(s.find(" terrainDetail=1:on:per-terrain-fragment-within-300-m ") != std::string::npos);
    CHECK(s.find(" sharpen=1:off:TAA-sharpens") != std::string::npos);
    CHECK(s.find("sharpen=1:off:TAA-sharpens:inert") == std::string::npos);
}

TEST_CASE("Feature switches: the sun shadows have landed, with the sun size and the leaf bias",
          "[remaster][FeatureSwitches]")
{
    // B1: plan 3.1's sub-knob sunSize (1 deg) and the reach of the softening
    // (40 m) under softShadows, and the leaf bias (6 texels toward the sun)
    // under treeShadows, all in the shadows track's group.
    struct Sub
    {
        const char* key;
        const char* parent;
        float value;
    };
    for (const Sub& sub : {Sub{"sunSize", "softShadows", 1.0f}, Sub{"shadowReach", "softShadows", 40.0f},
                           Sub{"leafBias", "treeShadows", 6.0f}})
    {
        INFO(sub.key);
        const LightingKey* k = FindLightingKey(sub.key);
        REQUIRE(k != nullptr);
        CHECK(k->kind == KeyKind::Knob);
        REQUIRE(k->parent != nullptr);
        CHECK(std::string_view(k->parent) == sub.parent);
        CHECK(std::string_view(k->group) == "shadows");
        CHECK(KeyDefault(*k) == sub.value);
        CHECK(GetKeyValue(Apply(std::string("remastered,") + sub.key + "=0.5"), *k) == 0.5f);
        CHECK(GetKeyValue(Apply(std::string(sub.key) + "=0.5,baseline"), *k) == sub.value);
    }
    for (Feature f : {Feature::TreeShadows, Feature::SoftShadows})
    {
        const FeatureInfo& info = GetFeatureInfo(f);
        INFO(info.key);
        CHECK(info.landed);
        CHECK(std::string_view(info.step) == "B1");
        CHECK(std::string_view(info.track) == "shadows");
    }
}

TEST_CASE("Feature gates: the light list runs with a lamp at night or a combat light", "[remaster][FeatureGates]")
{
    GateInputs in = Night();
    CHECK_FALSE(G(in, Feature::LightTiles).on);
    in.localLights = 1;
    CHECK(G(in, Feature::LightTiles).on);
    // By day a lamp that is on adds no light (the upload scales it by the night).
    GateInputs day = Noon();
    day.localLights = 3;
    CHECK_FALSE(G(day, Feature::LightTiles).on);
    day.combatLightsNear = 2;
    CHECK(G(day, Feature::LightTiles).on);
}

TEST_CASE("Feature gates: the sky table runs while the sun is above -9 degrees", "[remaster][FeatureGates]")
{
    GateInputs in = Night();
    in.sunElevationDeg = -8.5f;
    CHECK(G(in, Feature::Atmosphere).on);
    in.sunElevationDeg = -9.5f;
    CHECK_FALSE(G(in, Feature::Atmosphere).on);
}

TEST_CASE("Feature gates: sun shadows need the sun above 4 degrees or the moon key", "[remaster][FeatureGates]")
{
    GateInputs in = Noon();
    in.sunElevationDeg = 5.0f;
    CHECK(G(in, Feature::TreeShadows).on);
    CHECK(G(in, Feature::SoftShadows).on);
    in.sunElevationDeg = 3.0f;
    CHECK_FALSE(G(in, Feature::TreeShadows).on);
    CHECK_FALSE(G(in, Feature::SoftShadows).on);

    GateInputs moon = Night();
    moon.moonElevationDeg = 36.0f;
    moon.moonPhase = 1.0f;
    CHECK(G(moon, Feature::MoonShadows).on);
    CHECK(G(moon, Feature::TreeShadows).on);
    CHECK(std::string_view(G(moon, Feature::TreeShadows).reason) == "moon key");
    // No moon key without its switch.
    CHECK_FALSE(G(moon, Feature::TreeShadows, Apply("remastered,moonShadows=0")).on);
}

TEST_CASE("Feature gates: night features run only at night, the moon key only with a high, bright moon",
          "[remaster][FeatureGates]")
{
    for (Feature f : {Feature::NightLights, Feature::Stars, Feature::MoonShadows})
    {
        INFO(GetFeatureInfo(f).key);
        CHECK_FALSE(G(Noon(), f).on);
    }
    CHECK(G(Night(), Feature::NightLights).on);
    CHECK(G(Night(), Feature::Stars).on);

    GateInputs in = Night();
    in.moonElevationDeg = 36.0f;
    in.moonPhase = 1.0f;
    CHECK(G(in, Feature::MoonShadows).on);
    in.moonElevationDeg = 2.0f;
    CHECK_FALSE(G(in, Feature::MoonShadows).on);
    in.moonElevationDeg = 36.0f;
    in.moonPhase = 0.2f;
    CHECK_FALSE(G(in, Feature::MoonShadows).on);
    in.moonPhase = 1.0f;
    in.sunElevationDeg = -2.0f; // twilight: the sun is still the key
    CHECK_FALSE(G(in, Feature::MoonShadows).on);
}

TEST_CASE("Feature gates: combat light needs a combat light within 60 m, its shadow mask a night",
          "[remaster][FeatureGates]")
{
    GateInputs in = Night();
    CHECK_FALSE(G(in, Feature::CombatLights).on);
    CHECK_FALSE(G(in, Feature::CombatShadows).on);
    in.combatLightsNear = 1;
    CHECK(G(in, Feature::CombatLights).on);
    CHECK(G(in, Feature::CombatShadows).on);
    CHECK(G(in, Feature::CombatShadows).scale == 0.5f);
    in.nightEffect = 0.2f; // dusk: the light is weak, the mask is skipped
    CHECK(G(in, Feature::CombatLights).on);
    CHECK_FALSE(G(in, Feature::CombatShadows).on);
    in.nightEffect = 1.0f;
    CHECK_FALSE(G(in, Feature::CombatShadows, Apply("remastered,combatLights=0")).on);
}

TEST_CASE("Feature gates: SSR runs at half resolution while water or wet shows on 1% of the screen",
          "[remaster][FeatureGates]")
{
    GateInputs in = Noon();
    // Until B6's readout measures the shares, SSR stays on.
    CHECK(G(in, Feature::Ssr).on);
    CHECK(G(in, Feature::Ssr).scale == 0.5f);
    in.waterShare = 0.004f;
    in.wetShare = 0.0f;
    CHECK_FALSE(G(in, Feature::Ssr).on);
    in.waterShare = 0.03f;
    CHECK(G(in, Feature::Ssr).on);
    in.waterShare = 0.0f;
    in.wetShare = 0.02f;
    CHECK(G(in, Feature::Ssr).on);
    in.optics = true;
    CHECK_FALSE(G(in, Feature::Ssr).on);
}

TEST_CASE("Feature gates: wet shading runs above 0.02 wetness, lamp streaks also need a light",
          "[remaster][FeatureGates]")
{
    GateInputs in = Night();
    CHECK_FALSE(G(in, Feature::WetWeather).on);
    in.rainDensity = 0.5f; // no wetness state yet: the rain stands in
    CHECK(G(in, Feature::WetWeather).on);
    CHECK_FALSE(G(in, Feature::LampStreaks).on);
    in.localLights = 3;
    CHECK(G(in, Feature::LampStreaks).on);
    GateInputs wetDay = Noon();
    wetDay.rainDensity = 0.5f;
    wetDay.localLights = 3;
    CHECK(G(wetDay, Feature::WetWeather).on);
    CHECK_FALSE(G(wetDay, Feature::LampStreaks).on);
    CHECK_FALSE(G(in, Feature::LampStreaks, Apply("remastered,wetWeather=0")).on);
    in.wetness = 0.01f; // the integrator's state wins over the rain
    CHECK_FALSE(G(in, Feature::WetWeather).on);
    in.rainDensity = 0.0f;
    in.wetness = 0.4f; // drying after the rain
    CHECK(G(in, Feature::WetWeather).on);
}

TEST_CASE("Feature gates: beams need a low sun in view, mist or headlights; mist needs morning or fog",
          "[remaster][FeatureGates]")
{
    GateInputs in = Noon();
    CHECK_FALSE(G(in, Feature::AirBeams).on);
    CHECK_FALSE(G(in, Feature::ValleyMist).on);

    in.sunElevationDeg = 12.0f;
    in.sunAzimuthDeg = 270.0f; // evening sun
    in.sunViewAngleDeg = 30.0f;
    CHECK(G(in, Feature::AirBeams).on);
    CHECK(G(in, Feature::AirBeams).scale == 0.5f);
    in.sunViewAngleDeg = 120.0f; // sun behind the camera
    CHECK_FALSE(G(in, Feature::AirBeams).on);
    CHECK_FALSE(G(in, Feature::ValleyMist).on);

    in.sunAzimuthDeg = 90.0f; // the same sun in the morning
    CHECK(G(in, Feature::ValleyMist).on);
    CHECK(G(in, Feature::AirBeams).on);
    CHECK_FALSE(G(in, Feature::AirBeams, Apply("remastered,valleyMist=0")).on);
    in.sunElevationDeg = 40.0f; // mist burned off
    CHECK_FALSE(G(in, Feature::ValleyMist).on);
    in.fog = 0.3f;
    CHECK(G(in, Feature::ValleyMist).on);
    CHECK(G(in, Feature::AirBeams).on);

    GateInputs night = Night();
    CHECK_FALSE(G(night, Feature::AirBeams).on);
    night.spotLights = 1;
    night.localLights = 1;
    CHECK(G(night, Feature::AirBeams).on);
    CHECK(std::string_view(G(night, Feature::AirBeams).reason) == "headlights at night");
}

TEST_CASE("Feature gates: canopy shadows run with the sun between 2 and 25 degrees under a mostly clear sky",
          "[remaster][FeatureGates]")
{
    GateInputs in = Noon();
    CHECK_FALSE(G(in, Feature::CanopyShadows).on);
    in.sunElevationDeg = 10.0f;
    CHECK(G(in, Feature::CanopyShadows).on);
    CHECK(G(in, Feature::CanopyShadows).scale == 0.5f);
    in.sunElevationDeg = 1.5f;
    CHECK_FALSE(G(in, Feature::CanopyShadows).on);
    in.sunElevationDeg = 10.0f;
    in.skyThrough = 0.4f;
    CHECK_FALSE(G(in, Feature::CanopyShadows).on);
}

TEST_CASE("Feature gates: lit clouds need clouds in the sky, sky cloud shadows a cover", "[remaster][FeatureGates]")
{
    GateInputs in = Noon();
    CHECK_FALSE(G(in, Feature::LitClouds).on);
    CHECK_FALSE(G(in, Feature::SkyCloudShadows).on);
    in.skyThrough = 0.5f;
    in.cloudCover = 0.0f; // F4: no cloud shadows below 0.05 cover
    CHECK(G(in, Feature::LitClouds).on);
    CHECK_FALSE(G(in, Feature::SkyCloudShadows).on);
    in.cloudCover = 0.3f;
    CHECK(G(in, Feature::SkyCloudShadows).on);

    // At night: no sun to light the clouds or to cast their shadows, unless the
    // moon is the key light.
    GateInputs night = Night();
    night.skyThrough = 0.1f;
    night.cloudCover = 0.6f;
    CHECK_FALSE(G(night, Feature::LitClouds).on);
    CHECK_FALSE(G(night, Feature::SkyCloudShadows).on);
    night.moonElevationDeg = 40.0f;
    night.moonPhase = 1.0f;
    CHECK(G(night, Feature::SkyCloudShadows).on);
    CHECK_FALSE(G(night, Feature::SkyCloudShadows, Apply("remastered,moonShadows=0")).on);
}

TEST_CASE("Feature gates: one sharpen pass, TAA's or the luma one", "[remaster][FeatureGates]")
{
    GateInputs in = Noon();
    CHECK_FALSE(G(in, Feature::Sharpen).on);
    CHECK(G(in, Feature::Sharpen, Apply("remastered,taa=0")).on);
    in.interior = true; // TAA gated off in an interior: the luma sharpen takes over
    CHECK(G(in, Feature::Sharpen).on);
}

TEST_CASE("Feature gates: gates=0 forces every enabled feature on; switches and classic still win",
          "[remaster][FeatureGates]")
{
    const LightingTuning forced = Apply("remastered,gates=0,ssr=0");
    const FeatureGates g = ComputeFeatureGates(Noon(), forced);
    for (int i = 0; i < kFeatureCount; ++i)
    {
        const auto f = static_cast<Feature>(i);
        INFO(GetFeatureInfo(f).key);
        CHECK(g[f].on == (f != Feature::Ssr));
    }
    CHECK(g[Feature::AirBeams].scale == 0.5f);

    const FeatureGates classic = ComputeFeatureGates(Noon(), Apply("classic,gates=0"));
    const FeatureGates base = ComputeFeatureGates(Night(), Apply("baseline"));
    for (int i = 0; i < kFeatureCount; ++i)
    {
        const auto f = static_cast<Feature>(i);
        INFO(GetFeatureInfo(f).key);
        CHECK_FALSE(classic[f].on);
        CHECK_FALSE(base[f].on);
        CHECK(std::string_view(base[f].reason) == "switch off");
    }
}

TEST_CASE("Feature gates: the state line names every switch and never holds a ';'", "[remaster][FeatureGates]")
{
    GateInputs in = Noon();
    in.sunElevationDeg = 12.0f;
    in.sunViewAngleDeg = 20.0f;
    FeatureGates g = ComputeFeatureGates(in, Apply("remastered,stars=0"));
    g.frame = 42;
    const std::string s = FeatureStateString(Apply("remastered,stars=0"), g);
    INFO(s);
    CHECK(s.find(';') == std::string::npos);
    CHECK(s.rfind("mode=remastered gates=1 computed=1 frame=42 sun=12.0", 0) == 0);
    CHECK(s.find(" taa=1:on:world-drawn") != std::string::npos);
    CHECK(s.find(" stars=0:off:switch-off") != std::string::npos);
    CHECK(s.find(" airBeams=1:on@0.5:low-sun-in-view") != std::string::npos);
    for (int i = 0; i < kFeatureCount; ++i)
    {
        const FeatureInfo& info = GetFeatureInfo(static_cast<Feature>(i));
        INFO(info.key);
        CHECK(s.find(std::string(" ") + info.key + "=") != std::string::npos);
    }
    // Every switch that has not landed is marked inert; taa has landed (A5).
    CHECK(s.find(" taa=1:on:world-drawn:inert") == std::string::npos);
    size_t inert = 0;
    for (size_t p = s.find(":inert"); p != std::string::npos; p = s.find(":inert", p + 1))
        ++inert;
    size_t landed = 0;
    for (int i = 0; i < kFeatureCount; ++i)
        landed += GetFeatureInfo(static_cast<Feature>(i)).landed ? 1 : 0;
    CHECK(inert + landed == static_cast<size_t>(kFeatureCount));
}

TEST_CASE("Feature switches: the frame-cost switches, TAA, the sky table, texsharp and the sun shadows have landed, "
          "nothing else has",
          "[remaster][FeatureSwitches]")
{
    // A4 lands drawGrouping and hillCull (pixel-exact, both always on unless
    // classic): triFeatureState no longer marks them inert, and the overlay
    // shows the opaque objects' GPU section for them. A5 lands taa, A7
    // atmosphere, B11 terrainDetail and sharpen, B1 treeShadows (the shadow
    // depth pass) and softShadows (the world pass).
    std::set<std::string> sections;
    for (int i = 0; i < Poseidon::gpu::kSectionCount; ++i)
        sections.insert(Poseidon::gpu::SectionName(static_cast<Poseidon::gpu::Section>(i)));
    for (int i = 0; i < kFeatureCount; ++i)
    {
        const FeatureInfo& info = GetFeatureInfo(static_cast<Feature>(i));
        INFO(info.key);
        const bool perf = info.id == Feature::DrawGrouping || info.id == Feature::HillCull;
        const bool taa = info.id == Feature::Taa;
        const bool atmosphere = info.id == Feature::Atmosphere;
        const bool texsharp = info.id == Feature::TerrainDetail || info.id == Feature::Sharpen;
        const bool shadows = info.id == Feature::TreeShadows || info.id == Feature::SoftShadows;
        CHECK(info.landed == (perf || taa || atmosphere || texsharp || shadows));
        if (info.landed)
        {
            CHECK(std::string_view(info.step) == (perf         ? "A4"
                                                  : taa        ? "A5"
                                                  : atmosphere ? "A7"
                                                  : shadows    ? "B1"
                                                               : "B11"));
            CHECK(sections.count(info.gpuSection) == 1);
        }
    }
    CHECK(std::string_view(GetFeatureInfo(Feature::TreeShadows).gpuSection) == "shadow");
    CHECK(std::string_view(GetFeatureInfo(Feature::SoftShadows).gpuSection) == "world");
    const LightingTuning t = Apply("remastered");
    FeatureGates g = ComputeFeatureGates(GateInputs{}, t);
    const std::string s = FeatureStateString(t, g);
    INFO(s);
    CHECK(s.find(" drawGrouping=1:on:always ") != std::string::npos);
    CHECK(s.find(" hillCull=1:on:always ") != std::string::npos);
    CHECK(s.find(" treeShadows=1:on:sun-above-4-deg ") != std::string::npos);
    CHECK(s.find(" softShadows=1:on:sun-above-4-deg ") != std::string::npos);
    CHECK(s.find("landed=8/26") != std::string::npos);
    // Classic keeps its own draw path: the gates hold them off.
    const LightingTuning classic = Apply("classic");
    const FeatureGates gc = ComputeFeatureGates(GateInputs{}, classic);
    CHECK_FALSE(gc[Feature::DrawGrouping].on);
    CHECK_FALSE(gc[Feature::HillCull].on);
    // Night vision keeps them (they change no pixel).
    GateInputs nv;
    nv.nightVision = true;
    CHECK(ComputeFeatureGates(nv, t)[Feature::HillCull].on);
}
