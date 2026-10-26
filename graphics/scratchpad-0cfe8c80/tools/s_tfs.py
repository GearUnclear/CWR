R = ['''    // texsharp's two (B11), the sun shadows' three (B1, below), the sea's three
    // (B5), the beams' contrast (B8, below), the night's lamp and headlight
    // strengths (B3, test_night_lights.cpp) and the volumetric fog's five
    // (look-fog).
''',
'''               parent == "softShadows" || parent == "treeShadows" || parent == "seaWaves" || parent == "airBeams" ||
               parent == "nightLights" || parent == "volFog"));
''',
'''    CHECK(children == 22);
''',
'''        const bool fog = info.id == Feature::VolFog;
        CHECK(info.landed == (perf || taa || lightList || atmosphere || texsharp || shadows || shade || night || sea ||
                              air || canopy || clouds || fog));
''',
'''    // The night's three hold off by day.
    CHECK(s.find(" nightLights=1:off:day ") != std::string::npos);
    CHECK(s.find(" moonShadows=1:off:day ") != std::string::npos);
    CHECK(s.find(" stars=1:off:day ") != std::string::npos);
    CHECK(s.find("landed=22/27") != std::string::npos);
    // The volumetric fog runs on any drawn world: it replaces the distance fog,
    // and B8's air pass, which it supersedes, holds off.
    CHECK(s.find(" volFog=1:on:world-drawn-(replaces-the-distance-fog)") != std::string::npos);
''']
