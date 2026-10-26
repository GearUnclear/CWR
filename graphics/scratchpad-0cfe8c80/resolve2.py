import importlib.util
import os

here = os.path.dirname(os.path.abspath(__file__))
src = open(os.path.join(here, 'resolve.py'), encoding='utf-8').read()
# reuse resolve() and lines() only
ns = {}
ns['__file__'] = os.path.join(here, 'resolve.py')
exec(src.split('E = \'arma_CWR/engine/\'')[0], ns)
resolve = ns['resolve']


def lines(text):
    return text.strip('\n').split('\n')


T = 'arma_CWR/tests/unit/engine/Poseidon/Graphics/'

resolve(T + 'test_feature_switches.cpp', [
    lambda t, b: lines('''
    // texsharp's two (B11), the sun shadows' three (B1, below), the sea's three
    // (B5), the beams' contrast (B8, below), the night's lamp and headlight
    // strengths (B3, test_night_lights.cpp), the reflections' two (B6), the
    // wet weather's two (B7, test_wet_weather.cpp) and combat light's four (B4,
    // test_combat_lights.cpp).'''),
    lambda t, b: lines('''
               parent == "softShadows" || parent == "treeShadows" || parent == "seaWaves" || parent == "airBeams" ||
               parent == "nightLights" || parent == "ssr" || parent == "wetWeather" || parent == "combatLights"));'''),
    lambda t, b: ['    CHECK(children == 25);'],
    lambda t, b: lines('''
TEST_CASE("Feature switches: every feature switch has landed: the frame-cost switches, TAA, the light list, the sky "
          "table, texsharp, the sun shadows, shade, the night, combat light, the sea, the reflections, the wet "
          "weather, the air pass, the canopy shadows and the clouds",'''),
    lambda t, b: t[:-1] + [t[-1][:-1] + ',', '    // and B4 combatLights (the world pass) and combatShadows (its mask pass).'],
    lambda t, b: lines('''
        const bool shade = info.id == Feature::LowSunRig || info.id == Feature::CrownCanopy;
        const bool night =
            info.id == Feature::NightLights || info.id == Feature::MoonShadows || info.id == Feature::Stars;
        const bool combat = info.id == Feature::CombatLights || info.id == Feature::CombatShadows;
        const bool sea = info.id == Feature::SeaWaves;
        const bool ssr = info.id == Feature::Ssr;
        const bool wet = info.id == Feature::WetWeather || info.id == Feature::LampStreaks;
        const bool air = info.id == Feature::AirBeams || info.id == Feature::AirHaze || info.id == Feature::ValleyMist;
        const bool canopy = info.id == Feature::CanopyShadows;
        const bool clouds = info.id == Feature::LitClouds || info.id == Feature::SkyCloudShadows;
        CHECK(info.landed == (perf || taa || lightList || atmosphere || texsharp || shadows || shade || night ||
                              combat || sea || ssr || wet || air || canopy || clouds));'''),
    lambda t, b: t[:2] + ['                                                  : combat     ? "B4"'] + t[2:],
    lambda t, b: t[:10] + lines('''
    // Combat light holds off with no combat light near, its shadows by day.
    CHECK(s.find(" combatLights=1:off:") != std::string::npos);
    CHECK(s.find(" combatShadows=1:off:") != std::string::npos);
    CHECK(s.find("landed=26/26") != std::string::npos);''') + t[11:] + lines('''
    CHECK(std::string_view(GetFeatureInfo(Feature::CombatLights).gpuSection) == "world");
    CHECK(std::string_view(GetFeatureInfo(Feature::CombatShadows).gpuSection) == "combatmask");'''),
])

resolve(T + 'test_frame_lights.cpp', [lambda t, b: lines('''
    // (B3: the budget counts the lights of the classic model; with the physical
    // lamps and low beams on they leave it, test_night_lights.cpp. B4: the
    // combat lights at the head of the tile leave it too.)
    CHECK(lights.find("float w = classic > kClassicBudget ? clamp(0.5 + (lightImportance(id) - mid) / band, 0.0, 1.0) "
                      ": 1.0;") != std::string::npos);
    CHECK(lights.find("int classic = count - head;") != std::string::npos);''')])


def post_debug(t, b):
    out = []
    for ln in t:
        if ln.startswith('    // and 10, B6).'):
            out += ['    // and 10, B6), and the combat shadow mask after them (debug=11, B4).']
            continue
        if ln.strip() == 'CHECK(debug->maxValue == static_cast<float>(kDebugViewSsrReflection));':
            out += ['    CHECK(Poseidon::lighting::kDebugViewCombatMask == kDebugViewSsrReflection + 1);',
                    '    CHECK(debug->maxValue == static_cast<float>(Poseidon::lighting::kDebugViewCombatMask));']
            continue
        out.append(ln)
    return out


resolve(T + 'test_post_stages.cpp', [post_debug])

resolve(T + 'test_remaster_lighting.cpp', [lambda t, b: lines('''
    // two passes, the luma sharpen, the canopy's sun-visibility pass, the air
    // pass's march, blur, mean and apply, the reflections' tag readout,
    // shares, trace, rough blur and composite, and the combat shadow mask and
    // its blur
    CHECK(post == 26);''')])
