#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/ScreenReflections.hpp>
#include <Poseidon/Graphics/Remaster/SurfaceMaterials.hpp>
#include <Poseidon/Graphics/Remaster/Wetness.hpp>
#include <Poseidon/Graphics/Rendering/BuildRenderPassDescriptor.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// The remastered wet weather (feature switches wetWeather and lampStreaks,
// INTEGRATION-PLAN.md section 5 B7; Graphics/Remaster/Wetness.hpp and
// SurfaceMaterials.hpp): the world's wetness that the rain wets and the sun and
// wind dry, the surface classes from texture names and object kinds, and the
// contracts that tie the shaders, the reflections' pass and the backend to
// them.

using namespace Poseidon::remaster;
namespace render = Poseidon::render;
namespace gl = Poseidon::gl33lighting;
using K = render::DrawObjectKind;

namespace
{

std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path Gl33Dir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine" / "PoseidonGL33";
}

int CountOf(const std::string& haystack, const std::string& needle)
{
    int n = 0;
    for (size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + needle.size()))
        ++n;
    return n;
}

LightingTuning Apply(const std::string& setting)
{
    LightingTuning t{};
    bool modeSet = false;
    const std::string why = ApplyLightingSetting(t, setting, modeSet);
    INFO(setting << ": " << why);
    REQUIRE(why.empty());
    return t;
}

// Steps the state `seconds` long in steps of `dt`.
void Run(WetnessState& s, const WetnessInputs& in, float seconds, float dt = 0.05f)
{
    for (float t = 0.0f; t < seconds; t += dt)
        StepWetness(s, in, dt);
}

WetnessInputs Rain(float rain)
{
    WetnessInputs in;
    in.rain = rain;
    in.sunElevationDeg = 20.0f;
    in.skyThrough = 0.1f;
    in.windSpeed = 3.0f;
    return in;
}

// A night after the rain: no sun, still air.
WetnessInputs StillNight()
{
    WetnessInputs in;
    in.rain = 0.0f;
    in.sunElevationDeg = -30.0f;
    in.skyThrough = 1.0f;
    in.windSpeed = 0.0f;
    return in;
}

SurfaceClass Classify(std::string_view path, K kind, bool onSurface = false)
{
    static const SurfaceTable table = DefaultSurfaceTable();
    return ClassifySurface(table, path, kind, onSurface);
}

} // namespace

// ---- The world's wetness ------------------------------------------------------

TEST_CASE("Wetness: rain wets surfaces within minutes and fills puddles slower", "[remaster][wet]")
{
    WetnessState s;
    const WetnessInputs heavy = Rain(1.0f);
    Run(s, heavy, 60.0f);
    // A minute of heavy rain: surfaces half wet or more, the puddles barely started.
    CHECK(s.surface > 0.5f);
    CHECK(s.puddles < 0.3f);
    CHECK(s.puddles < s.surface);
    Run(s, heavy, 540.0f);
    // Ten minutes: surfaces soaked; the puddles fill on.
    CHECK(s.surface == Catch::Approx(1.0f).margin(0.01));
    CHECK(s.puddles > 0.5f);
    Run(s, heavy, 3000.0f);
    CHECK(s.puddles == Catch::Approx(WetPuddleTarget(1.0f)).margin(0.01));
    // Light rain wets slower than heavy rain.
    WetnessState light;
    Run(light, Rain(0.1f), 60.0f);
    WetnessState hard;
    Run(hard, Rain(0.6f), 60.0f);
    CHECK(light.surface < hard.surface);
    // No rain wets nothing.
    WetnessState dry;
    Run(dry, StillNight(), 600.0f);
    CHECK(dry.surface == 0.0f);
    CHECK(dry.puddles == 0.0f);
}

TEST_CASE("Wetness: surfaces dry over minutes after the rain, faster in sun and wind; puddles outlast them",
          "[remaster][wet]")
{
    const WetnessState soaked = SettledWetness(1.0f);
    CHECK(soaked.surface == 1.0f);
    CHECK(soaked.puddles == 1.0f);

    // A still, overcast or dark place: still damp after ten minutes.
    WetnessState night = soaked;
    Run(night, StillNight(), 600.0f);
    CHECK(night.surface > 0.5f);
    CHECK(night.puddles > night.surface);

    // A high clear sun and a breeze dry surfaces much faster.
    WetnessInputs sunny;
    sunny.rain = 0.0f;
    sunny.sunElevationDeg = 60.0f;
    sunny.skyThrough = 1.0f;
    sunny.windSpeed = 6.0f;
    CHECK(DryingFactor(sunny) > 2.5f * DryingFactor(StillNight()));
    WetnessState noon = soaked;
    Run(noon, sunny, 600.0f);
    CHECK(noon.surface < 0.3f);
    CHECK(noon.surface < night.surface);
    // The puddles lag: still standing when the surfaces are nearly dry.
    CHECK(noon.puddles > 0.5f);
    Run(noon, sunny, 3600.0f);
    CHECK(noon.surface < 0.01f);
    CHECK(noon.puddles < 0.3f);

    // A long frame never overshoots below dry (an exact step, not Euler).
    WetnessState jump = soaked;
    StepWetness(jump, sunny, 1e6f);
    CHECK(jump.surface == Catch::Approx(0.0f).margin(1e-6));
    CHECK(jump.puddles == Catch::Approx(0.0f).margin(1e-6));
    // No step, no change.
    WetnessState same = soaked;
    StepWetness(same, sunny, 0.0f);
    CHECK(same.surface == soaked.surface);
}

TEST_CASE("Wetness: a rain change made at once snaps the world to what that rain settles at", "[remaster][wet]")
{
    // `0 setRain 1` at a mission start: wet at once.
    CHECK(WetnessSnaps(0.0f, 1.0f, 0.0f));
    CHECK(WetnessSnaps(0.0f, 1.0f, 0.5f));
    // Over a minute: the world wets with it instead.
    CHECK_FALSE(WetnessSnaps(0.0f, 1.0f, 60.0f));
    // The same rain again (a script re-applying its weather) changes nothing.
    CHECK_FALSE(WetnessSnaps(0.5f, 0.5f, 0.0f));
    CHECK_FALSE(WetnessSnaps(0.5f, 0.505f, 0.0f));
    const WetnessState s = SettledWetness(0.5f);
    CHECK(s.surface == Catch::Approx(WetSurfaceTarget(0.5f)));
    CHECK(s.puddles == Catch::Approx(WetPuddleTarget(0.5f)));
    // Drizzle soaks surfaces but fills few puddles; no rain, no wetness.
    CHECK(WetSurfaceTarget(0.2f) > 0.5f);
    CHECK(WetPuddleTarget(0.2f) < 0.1f);
    CHECK(WetSurfaceTarget(0.0f) == 0.0f);
    CHECK(WetPuddleTarget(0.0f) == 0.0f);
}

TEST_CASE("Wetness: the knob forces surfaces and puddles, the puddles knob scales them", "[remaster][wet]")
{
    const WetnessState world{0.4f, 0.2f};
    const WetnessFrame weather = FrameWetness(world, -1.0f, 1.0f);
    CHECK_FALSE(weather.forced);
    CHECK(weather.surface == Catch::Approx(0.4f));
    CHECK(weather.puddles == Catch::Approx(0.2f));
    const WetnessFrame forced = FrameWetness(world, 1.0f, 1.0f);
    CHECK(forced.forced);
    CHECK(forced.surface == 1.0f);
    CHECK(forced.puddles == 1.0f);
    CHECK(FrameWetness(world, 1.0f, 0.0f).puddles == 0.0f);
    CHECK(FrameWetness(world, -1.0f, 2.0f).puddles == Catch::Approx(0.4f));
    // The gates see the wetter of the two: puddles keep the wet path on after
    // the surfaces have dried.
    CHECK(FrameWetnessGate(WetnessFrame{0.0f, 0.3f, false}) == Catch::Approx(0.3f));
}

// ---- Surface classes ----------------------------------------------------------

TEST_CASE("Surface classes: the research log's stock textures classify as the prototype did", "[remaster][wet]")
{
    // Names and classes from the research prototype's log of every texture of
    // three islands (materials/classification_log_r6.txt), by object kind.
    struct Case
    {
        const char* path;
        K kind;
        SurfaceClass expected;
    };
    const Case cases[] = {
        {"data\\00000mc_vojakg2.paa", K::Man, SurfaceClass::Fabric},
        {"data\\m16_pmp_1.pac", K::Man, SurfaceClass::Weapon},
        {"data\\m4_1.pac", K::Man, SurfaceClass::Fabric}, // a known miss of the token list
        {"data\\abrams_roury_frt.pac", K::LandVehicle, SurfaceClass::Paint},
        {"data\\jepp4x4_glass.paa", K::LandVehicle, SurfaceClass::Glass},
        {"data\\m113_kolo2.pac", K::LandVehicle, SurfaceClass::Rubber},
        {"data\\m60kolozub.paa", K::LandVehicle, SurfaceClass::Rubber},
        {"data\\m113_svetla_front.pac", K::LandVehicle, SurfaceClass::Lens},
        {"data\\m1a1_hlaven.pac", K::LandVehicle, SurfaceClass::Paint}, // painted gun barrels
        {"data\\m60_lauf.pac", K::LandVehicle, SurfaceClass::Paint},
        {"data\\m113panel.pac", K::LandVehicle, SurfaceClass::Paint},
        {"data\\astan_side1.pac", K::Fabric, SurfaceClass::Fabric},
        {"data\\drevo_dirkate.pac", K::Fabric, SurfaceClass::Fabric},
        {"data\\asfaltka.paa", K::Static, SurfaceClass::Road},
        {"data\\silnice.paa", K::Static, SurfaceClass::Road},
        {"data\\cesta.paa", K::Static, SurfaceClass::Road},
        {"data\\benzinka_stojan.pac", K::Static, SurfaceClass::Metal}, // "zink" in benzinka
        {"data\\c_s_stecha_plech90.pac", K::Static, SurfaceClass::Metal},
        {"data\\plechbud_back.pac", K::Static, SurfaceClass::Metal},
        {"data\\blok_a_strecha.pac", K::Static, SurfaceClass::Roof},
        {"data\\dum_05_strecha_01.pac", K::Static, SurfaceClass::Roof},
        {"data\\kmen_borovice.pac", K::Static, SurfaceClass::Wood},
        {"data\\maskovaci_sit_new.paa", K::Static, SurfaceClass::Fabric},
        {"data\\stena_sklocihly.pac", K::Static, SurfaceClass::Glass},
        {"baracken\\glass.paa", K::Static, SurfaceClass::Glass},
        {"data\\abelpruchlod.pac", K::Static, SurfaceClass::Masonry},
    };
    for (const Case& c : cases)
    {
        INFO(c.path << " kind " << static_cast<int>(c.kind));
        CHECK(Classify(c.path, c.kind) == c.expected);
    }
    // Case and slashes do not matter.
    CHECK(Classify("DATA/JEPP4X4_GLASS.PAA", K::LandVehicle) == SurfaceClass::Glass);
    // A window frame is not glass; a window is.
    CHECK(Classify("data\\okno1.paa", K::Static) == SurfaceClass::Glass);
    CHECK(Classify("data\\okno_ram.paa", K::Static) == SurfaceClass::Masonry);
    // A hand is skin, a sleeve (rukav) cloth.
    CHECK(Classify("data\\ruka.pac", K::Man) == SurfaceClass::Skin);
    CHECK(Classify("data\\rukav.pac", K::Man) == SurfaceClass::Fabric);
}

TEST_CASE("Surface classes: each kind's default, roads under on-surface draws", "[remaster][wet]")
{
    CHECK(Classify("data\\anything.pac", K::LandVehicle) == SurfaceClass::Paint);
    CHECK(Classify("data\\anything.pac", K::Aircraft) == SurfaceClass::Paint);
    CHECK(Classify("data\\anything.pac", K::Ship) == SurfaceClass::Paint);
    CHECK(Classify("data\\anything.pac", K::Man) == SurfaceClass::Fabric);
    CHECK(Classify("data\\anything.pac", K::Fabric) == SurfaceClass::Fabric);
    CHECK(Classify("data\\anything.pac", K::Weapon) == SurfaceClass::Weapon);
    CHECK(Classify("data\\anything.pac", K::Static) == SurfaceClass::Masonry);
    CHECK(Classify("data\\anything.pac", K::Unknown) == SurfaceClass::Unknown);
    // A road or track decal on the ground with an anonymous texture.
    CHECK(Classify("data\\anything.pac", K::Static, true) == SurfaceClass::Road);
    CHECK(Classify("data\\anything.pac", K::Unknown, true) == SurfaceClass::Road);
    // Names still win on the ground.
    CHECK(Classify("data\\drevo.pac", K::Static, true) == SurfaceClass::Wood);
    // Vehicle rules do not reach men or houses, and the other way round.
    CHECK(Classify("data\\m113_kolo2.pac", K::Static) == SurfaceClass::Masonry);
    CHECK(Classify("data\\xicht_a.paa", K::LandVehicle) == SurfaceClass::Paint);
    CHECK(Classify("data\\xicht_a.paa", K::Man) == SurfaceClass::Skin);
}

TEST_CASE("Surface classes: a mod's CfgSurfaceMaterials rules go first or replace a stock rule by name",
          "[remaster][wet]")
{
    SurfaceTable t = DefaultSurfaceTable();
    const size_t stock = t.rules.size();
    SurfaceRule lobo;
    std::string why;
    REQUIRE(MakeSurfaceRule("LoBo_Rubber", "rubber", {"_Tyres"}, {}, {"vehicle"}, lobo, &why));
    CHECK(lobo.any == std::vector<std::string>{"_tyres"});
    CHECK(lobo.kinds == kVehicleKinds);
    SurfaceRule glass;
    REQUIRE(MakeSurfaceRule("glass", "metal", {"sklo"}, {}, {}, glass, &why)); // replaces the stock "glass" rule
    CHECK(glass.kinds == kAllKinds);
    MergeSurfaceRules(t, {lobo, glass});
    CHECK(t.rules.size() == stock + 1);
    CHECK(t.rules.front().name == "LoBo_Rubber");
    CHECK(ClassifySurface(t, "lobo\\hmmwv_TYRES.paa", K::LandVehicle, false) == SurfaceClass::Rubber);
    CHECK(ClassifySurface(t, "lobo\\hmmwv_tyres.paa", K::Man, false) == SurfaceClass::Fabric);
    CHECK(ClassifySurface(t, "data\\sklo.paa", K::Static, false) == SurfaceClass::Metal);
    // Unknown class or kind names are refused with the reason.
    SurfaceRule bad;
    CHECK_FALSE(MakeSurfaceRule("Bad", "plastic", {"x"}, {}, {}, bad, &why));
    CHECK(why.find("plastic") != std::string::npos);
    CHECK_FALSE(MakeSurfaceRule("Bad", "glass", {"x"}, {}, {"submarine"}, bad, &why));
    CHECK(why.find("submarine") != std::string::npos);
}

TEST_CASE("Surface classes: object kinds from the config's simulation and class", "[remaster][wet]")
{
    CHECK(DrawObjectKindFromNames("car", "Jeep") == K::LandVehicle);
    CHECK(DrawObjectKindFromNames("Tank", "M113") == K::LandVehicle);
    CHECK(DrawObjectKindFromNames("motorcycle", "Kolo") == K::LandVehicle);
    CHECK(DrawObjectKindFromNames("soldier", "SoldierWB") == K::Man);
    CHECK(DrawObjectKindFromNames("helicopter", "UH60") == K::Aircraft);
    CHECK(DrawObjectKindFromNames("airplane", "A10") == K::Aircraft);
    CHECK(DrawObjectKindFromNames("ship", "BoatW") == K::Ship);
    CHECK(DrawObjectKindFromNames("proxyweapon", "") == K::Weapon);
    CHECK(DrawObjectKindFromNames("house", "Housedum01") == K::Static);
    CHECK(DrawObjectKindFromNames("StreetLamp", "StreetLampMetal") == K::Static);
    // Tents and nets by their class, whatever their simulation.
    CHECK(DrawObjectKindFromNames("thing", "CampEast") == K::Fabric);
    CHECK(DrawObjectKindFromNames("house", "LoBo_camnet") == K::Fabric);
    CHECK(ParseKindMask("vehicle") == kVehicleKinds);
    CHECK(ParseKindMask("MAN") == KindBit(K::Man));
    CHECK(ParseKindMask("nothing") == 0u);
}

TEST_CASE("Surface classes: the shader's porosity table is the C++ one, in class order", "[remaster][wet]")
{
    const std::string surface = gl::RemasterFragmentPiece(gl::FragmentPiece::Surface);
    const std::regex table(R"(const float kWetPorosity\[(\d+)\] = float\[\]\(([^)]*)\);)");
    std::smatch m;
    REQUIRE(std::regex_search(surface, m, table));
    CHECK(std::stoi(m[1].str()) == kSurfaceClassCount);
    std::stringstream values(m[2].str());
    std::string item;
    int i = 0;
    while (std::getline(values, item, ','))
    {
        REQUIRE(i < kSurfaceClassCount);
        INFO(SurfaceClassName(static_cast<SurfaceClass>(i)));
        CHECK(std::stof(item) == Catch::Approx(SurfacePorosity(static_cast<SurfaceClass>(i))));
        ++i;
    }
    CHECK(i == kSurfaceClassCount);
    // Sealed surfaces form a film, cloth soaks the rain up.
    CHECK(SurfacePorosity(SurfaceClass::Paint) < 0.1f);
    CHECK(SurfacePorosity(SurfaceClass::Glass) == 0.0f);
    CHECK(SurfacePorosity(SurfaceClass::Fabric) > 0.8f);
    SurfaceClass parsed;
    CHECK(ParseSurfaceClass("Masonry", parsed));
    CHECK(parsed == SurfaceClass::Masonry);
    CHECK_FALSE(ParseSurfaceClass("plastic", parsed));
}

TEST_CASE("Draw descriptor: carries the object kind, which changes no pipeline state", "[remaster][wet]")
{
    render::LegacySpec spec;
    render::BuildContext ctx;
    const render::RenderPassDescriptor none = render::BuildRenderPassDescriptor(spec, ctx);
    CHECK(none.objectKind == K::Unknown);
    ctx.objectKind = K::LandVehicle;
    const render::RenderPassDescriptor jeep = render::BuildRenderPassDescriptor(spec, ctx);
    CHECK(jeep.objectKind == K::LandVehicle);
    // ApplyPipeline skips an unchanged descriptor: a new object kind alone
    // must not make it rebind the same state.
    CHECK(jeep == none);
}

// ---- The switches and their gates ---------------------------------------------

TEST_CASE("Wet weather switches: landed with the wetness and puddles sub-knobs", "[remaster][wet][FeatureSwitches]")
{
    for (Feature f : {Feature::WetWeather, Feature::LampStreaks})
    {
        const FeatureInfo& info = GetFeatureInfo(f);
        INFO(info.key);
        CHECK(info.landed);
        CHECK(std::string_view(info.step) == "B7");
        CHECK(std::string_view(info.gpuSection) == "world");
    }
    struct Sub
    {
        const char* key;
        float value;
    };
    for (const Sub& s : {Sub{"wetness", -1.0f}, Sub{"puddles", 1.0f}})
    {
        const LightingKey* k = FindLightingKey(s.key);
        REQUIRE(k != nullptr);
        INFO(s.key);
        CHECK(k->kind == KeyKind::Knob);
        CHECK(std::string_view(k->parent) == "wetWeather");
        CHECK(KeyDefault(*k) == s.value);
    }
    const LightingTuning t = Apply("remastered,wetness=1,puddles=0.5");
    CHECK(t.wetness == 1.0f);
    CHECK(t.puddles == 0.5f);
    // A mode word puts them back.
    CHECK(Apply("remastered,wetness=1,remastered").wetness == -1.0f);
}

TEST_CASE("Wet weather gates: the world's wetness drives them, puddles keep them on, dry holds them off",
          "[remaster][wet]")
{
    GateInputs in;
    in.sunElevationDeg = 30.0f;
    in.rainDensity = 0.0f;
    in.wetness = 0.0f;
    const LightingTuning t = Apply("remastered");
    CHECK_FALSE(ComputeFeatureGates(in, t)[Feature::WetWeather].on);
    // After the rain: no rain, the world still wet.
    in.wetness = FrameWetnessGate(WetnessFrame{0.0f, 0.4f, false});
    CHECK(ComputeFeatureGates(in, t)[Feature::WetWeather].on);
    // Lamps at night add the streaks.
    in.nightEffect = 1.0f;
    in.localLights = 3;
    CHECK(ComputeFeatureGates(in, t)[Feature::LampStreaks].on);
    CHECK_FALSE(ComputeFeatureGates(in, Apply("remastered,wetWeather=0"))[Feature::LampStreaks].on);
    CHECK_FALSE(ComputeFeatureGates(in, Apply("remastered,lampStreaks=0"))[Feature::LampStreaks].on);
    // The state line names the wetness.
    const FeatureGates g = ComputeFeatureGates(in, t);
    CHECK(FeatureStateString(t, g).find(" wet=0.40 ") != std::string::npos);
}

// ---- The shaders --------------------------------------------------------------

TEST_CASE("Wet weather shaders: nothing changes while dry, one puddle field for the forward pass and the trace",
          "[remaster][wet][Shaders]")
{
    const std::string shade = gl::RemasterFragmentPiece(gl::FragmentPiece::Shade);
    // The wet response runs only while the Weather slot is set (the gate on).
    CHECK(shade.find("if (litWeather.x > 0.0 || litWeather.z > 0.0)\n        wetSurface(") != std::string::npos);
    // ... and its terms enter as identities while it does not.
    CHECK(shade.find("float wetKeep = 1.0;") != std::string::npos);
    CHECK(shade.find("vec3 wetLit = vec3(0.0);") != std::string::npos);
    CHECK(shade.find("vec3 wetEnv = vec3(0.0);") != std::string::npos);
    CHECK(shade.find("float wetMirror = 0.0;") != std::string::npos);

    // The puddle field is its own piece, in the prelude once and pasted into
    // the reflections' trace once, so both draw the same puddles.
    const std::string puddles = gl::RemasterFragmentPiece(gl::FragmentPiece::Puddles);
    CHECK(puddles.find("vec2 wetPuddle(") != std::string::npos);
    CHECK(CountOf(gl::RemasterFragmentCommon(), puddles) == 1);
    const std::string trace = gl::PostShaderSource(gl::PostShader::SsrTrace);
    CHECK(CountOf(trace, puddles) == 1);
    // In the trace the wet weather's puddles replace B6's own only while it
    // shades the ground, and the trace then leaves the darkening to it.
    CHECK(trace.find("puddle = wetPuddle(posW, footT, road ? 1.0 : 0.0, grass, ssrWetMode.y).x;") != std::string::npos);
    CHECK(trace.find("float base = wetWeather ? 1.0 : darken * mix(1.0, kSsrPuddleDark, puddle);") !=
          std::string::npos);
    // The forward puddle darkening is the trace's own (B6's) puddle colour.
    const std::string surface = gl::RemasterFragmentPiece(gl::FragmentPiece::Surface);
    std::ostringstream dark;
    dark << "const float kWetPuddleDark = " << kSsrPuddleDark << ";";
    CHECK(surface.find(dark.str()) != std::string::npos);
    // The puddle thresholds on roads and bare ground are B6's.
    std::ostringstream road, ground;
    road << "const float kPuddleRoad = " << kSsrPuddleRoad << ";";
    ground << "const float kPuddleGround = " << kSsrPuddleGround << ";";
    CHECK(puddles.find(road.str()) != std::string::npos);
    CHECK(puddles.find(ground.str()) != std::string::npos);

    // Every main that shades the sea through shadeRemastered keeps it dry, and
    // the land and the grass say what they are.
    const std::string detail = gl::RemasterFragmentMain(1); // PSDetail
    CHECK(detail.find("g_wetDraw = litDetail.w > 0.5 ? 1 : 3;") != std::string::npos);
    const std::string water = gl::RemasterFragmentMain(3); // PSWater
    CHECK(water.find("g_wetDraw = 3;") != std::string::npos);
}

TEST_CASE("Wet weather shaders: the sun's glint in the film is capped far below a lamp's and softened on bare ground",
          "[remaster][wet][Shaders]")
{
    const std::string surface = gl::RemasterFragmentPiece(gl::FragmentPiece::Surface);
    const auto constant = [&](const char* name)
    {
        const std::regex re(std::string("const float ") + name + R"( = ([0-9.]+);)");
        std::smatch m;
        INFO(name);
        REQUIRE(std::regex_search(surface, m, re));
        return std::stof(m[1].str());
    };
    // The low sun's lobe on a wet film peaks at well over a hundred times the
    // sun (grazing light, grazing view): under the lamps' cap it clipped a
    // white column from the horizon to the eye. Its own cap keeps the glint
    // at a few times the sun at most, even in a puddle.
    const float sunCap = constant("kWetSunCap");
    const float lampCap = constant("kWetSpecCap");
    CHECK(sunCap <= 4.0f);
    CHECK(sunCap * 10.0f < lampCap);
    // Bare ground breaks the film: a rougher lobe on a smaller share.
    CHECK(constant("kWetSunGroundRough") > constant("kWetSunRough"));
    CHECK(constant("kWetSunGroundFilm") <= 0.5f);
    CHECK(sunCap * constant("kWetSunGroundFilm") <= 2.0f);
    // A pixel's footprint widens the lobe (far wet ground: no white dashes).
    CHECK(constant("kWetSunFoot") > 0.0f);
    CHECK(surface.find("float sunFoot = kWetSunFoot * length(fwidth(posAbs));") != std::string::npos);
    // Every sun lobe takes the sun's cap, every lamp lobe the lamps' (the tiled
    // list's, the draw's own list's, and the combat lights' of B4).
    CHECK(CountOf(surface, "wetLobe(litSunDir.xyz, 1e4, nf, v, nv, ") == 2);
    CHECK(CountOf(surface, ", kWetSunCap)") == 2);
    CHECK(CountOf(surface, ", kWetSpecCap)") == 3);
    CHECK(surface.find("min(3.14159265 * D * vis * nl * norm, cap) * F") != std::string::npos);
}

TEST_CASE("Wet weather backend: the gate sets the Weather slot, the per-draw class uploads on change only",
          "[remaster][wet]")
{
    const std::string lighting = ReadTextFile(Gl33Dir() / "EngineGL33_Lighting.cpp");
    // The gates read the wet weather's state only while its switch is on: off,
    // the rain stands in, as before the step (the reflections' gate too).
    CHECK(lighting.find("if (rm::FeatureOn(_lighting.features, rm::Feature::WetWeather))\n        in.wetness = "
                        "rm::FrameWetnessGate(_wetFrame);") != std::string::npos);
    // Dry: the slot is zero.
    CHECK(lighting.find("SetSlot(LitConst::Weather, 0.0f, 0.0f, 0.0f, 0.0f);") != std::string::npos);
    CHECK(lighting.find("if (m[0] == x && m[1] == y)\n        return;") != std::string::npos);
    const std::string queue = ReadTextFile(Gl33Dir() / "EngineGL33_Queue.cpp");
    CHECK(queue.find("ctx.objectKind = GetDrawObjectKind();") != std::string::npos);
    CHECK(queue.find("SetLitSurface(tex, d);") != std::string::npos);
    // The reflections' pass takes the ground's wetness from the wet weather
    // while its gate is on, else the rain.
    const std::string ssr = ReadTextFile(Gl33Dir() / "EngineGL33_PostSsr.cpp");
    CHECK(
        ssr.find("const float weatherWet = wetWeather ? _wetFrame.surface : (land ? land->GetRainDensity() : 0.0f);") !=
        std::string::npos);
}
