
namespace
{
// The value of `const float <name> = <value>;` in a GLSL source, or NaN.
float GlslConst(const std::string& source, const std::string& name)
{
    const std::regex decl("const float " + name + R"(\s*=\s*(-?[0-9.]+)\s*;)");
    std::smatch m;
    if (!std::regex_search(source, m, decl))
        return std::numeric_limits<float>::quiet_NaN();
    return std::stof(m[1].str());
}

// A float as the shaders write it: the shortest of 1 to 4 decimals that reads
// back exactly.
std::string GlslNumber(float v)
{
    for (int digits = 1; digits <= 4; ++digits)
    {
        std::ostringstream s;
        s << std::fixed;
        s.precision(digits);
        s << v;
        if (std::abs(std::stof(s.str()) - v) < 1e-5f)
            return s.str();
    }
    std::ostringstream s;
    s << v;
    return s.str();
}
} // namespace

TEST_CASE("Remaster sky clouds: with their slots at zero every draw is the one before them",
          "[Graphics][Shaders][remaster][clouds]")
{
    // The clouds switches are off (switch, gate, classic, night vision) exactly
    // when the engine uploads litSky as zero (EngineGL33_SkyClouds.cpp
    // UploadLitClouds). Then the dome keeps its texture coordinate and its
    // draw, the sky clouds and the sun disk pass through, and the shadows come
    // from the noise field: the frame before B10.
    const std::string screen = gl::RemasterVertexSource(0); // VSScreen
    CHECK(screen.find("if (dome && litSky.z != 0.0) vUV0.x += litSky.z;") != std::string::npos);
    CHECK(CountOf(screen, "vUV0 ") == 1); // set once ...
    CHECK(CountOf(screen, "vUV0.x += ") == 1); // ... and turned once
    const size_t sheets = screen.find("} else if (litSkyFwd.w > 1.5 && litSkyFwd.w < 2.5 && litSky.x > 0.0) {");
    REQUIRE(sheets != std::string::npos);
    CHECK(screen.find("vLitMode = -2.0;", sheets) != std::string::npos);

    const std::string normal = gl::RemasterFragmentMain(0); // PSNormal
    CHECK(normal.find("else if (litSky.x > 0.0) rgb = litCloudsOverClassic(rgb, litCloudTermsDome(vUV0));") !=
          std::string::npos);
    CHECK(normal.find("if (vLitMode < -1.5) r0.rgb = litCloudsOverClassic(r0.rgb, litCloudTermsSheet(t0.a));") !=
          std::string::npos);
    CHECK(normal.find("if (litSkyFwd.w > 2.5 && litSky.y > 0.5)") != std::string::npos);
    CHECK(normal.find("mix(1.0, kSkySunBehindCloud, skyShadowCoverToward(litSunDir.xyz))") != std::string::npos);
    // Only PSNormal draws the sky.
    for (int ps : {1, 2, 3})
    {
        CAPTURE(ps);
        CHECK(std::string(gl::RemasterFragmentMain(ps)).find("litSky") == std::string::npos);
    }

    // The shadows: the dome only behind litSky.y, before the noise lookup.
    const std::string terrain = Piece(gl::FragmentPiece::TerrainClouds);
    const size_t dome = terrain.find("if (litSky.y > 0.5) return mix(1.0, litCloud.y, domeCloudCover(");
    REQUIRE(dome != std::string::npos);
    CHECK(dome < terrain.find("texture(cloudNoise,"));
    // The previous anchor's lookup only while it fades.
    CHECK(terrain.find("if (litSkyShadow.w > 0.0) {") != std::string::npos);

    // Lit clouds: on the dome only behind litSky.x, and the terms weigh
    // nothing at zero strength.
    const std::string sky = Piece(gl::FragmentPiece::Sky);
    const size_t lit = sky.find("if (litSky.x > 0.0) {\n        vec4 lc = litCloudTermsDome(vUV0);");
    REQUIRE(lit != std::string::npos);
    CHECK(lit < sky.find("vec3 disp = mix(skyDisplay(a), skyDisplay(cloudLin), cloud);"));
    CHECK(sky.find("return vec4(gain, warm, silver, litSky.x * smoothstep(0.02, 0.25, d));") != std::string::npos);
    CHECK(sky.find("if (t.w <= 0.0) return lin;") != std::string::npos);
    CHECK(sky.find("if (t.w <= 0.0) return disp;") != std::string::npos);
    CHECK(sky.find("if (litSky.x <= 0.0) return vec4(1.0, 0.0, 0.0, 0.0);") != std::string::npos);
}

TEST_CASE("Remaster sky clouds: the shaders use SkyDensity.hpp's constants and dome mapping",
          "[Graphics][Shaders][remaster][clouds]")
{
    const std::string terrain = Piece(gl::FragmentPiece::TerrainClouds);
    const std::string sky = Piece(gl::FragmentPiece::Sky);
    struct Const
    {
        const std::string* source;
        const char* glsl;
        float value;
    };
    const Const consts[] = {
        {&terrain, "kSkyShadowDeck", kSkyShadowDeckAltitude},
        {&terrain, "kSkyShadowThreshold", kSkyShadowThreshold},
        {&terrain, "kSkyShadowEdge", kSkyShadowEdge},
        {&terrain, "kSkyShadowAnchorRadius", kSkyShadowAnchorRadius},
        {&terrain, "kSkySunBehindCloud", kSkySunBehindCloud},
        {&sky, "kLitCloudRelief", kLitCloudRelief},
        {&sky, "kLitCloudSilver", kLitCloudSilver},
        {&sky, "kLitCloudStepDeg", kLitCloudStepDeg},
        {&sky, "kLitCloudFadeStartDeg", kLitCloudFadeStartDeg},
        {&sky, "kLitCloudFadeEndDeg", kLitCloudFadeEndDeg},
    };
    for (const Const& c : consts)
    {
        CAPTURE(c.glsl);
        CHECK(GlslConst(*c.source, c.glsl) == Catch::Approx(c.value));
    }

    // skyDomeUv: the rings of SkyDomeV, segment by segment.
    const float* e = kSkyDomeRingElevationDeg;
    const float* v = kSkyDomeRingV;
    REQUIRE(kSkyDomeRings == 5);
    const std::string segments[] = {
        "if (e < " + GlslNumber(e[1]) + ") v = mix(" + GlslNumber(v[0]) + ", " + GlslNumber(v[1]) + ", clamp((e + " +
            GlslNumber(-e[0]) + ") / " + GlslNumber(e[1] - e[0]) + ", 0.0, 1.0));",
        "else if (e < " + GlslNumber(e[2]) + ") v = mix(" + GlslNumber(v[1]) + ", " + GlslNumber(v[2]) + ", (e - " +
            GlslNumber(e[1]) + ") / " + GlslNumber(e[2] - e[1]) + ");",
        "else if (e < " + GlslNumber(e[3]) + ") v = mix(" + GlslNumber(v[2]) + ", " + GlslNumber(v[3]) + ", (e - " +
            GlslNumber(e[2]) + ") / " + GlslNumber(e[3] - e[2]) + ");",
        "else v = mix(" + GlslNumber(v[3]) + ", " + GlslNumber(v[4]) + ", clamp((e - " + GlslNumber(e[3]) + ") / " +
            GlslNumber(e[4] - e[3]) + ", 0.0, 1.0));",
    };
    for (const std::string& seg : segments)
    {
        CAPTURE(seg);
        CHECK(terrain.find(seg) != std::string::npos);
    }
    // u: azimuth from north toward east over 180 degrees, plus the turn.
    CHECK(terrain.find("float az = atan(w.x, w.z) * 57.29578;") != std::string::npos);
    CHECK(terrain.find("return vec2(az / 180.0 + litSky.z, v);") != std::string::npos);
    // The density and the deck weight as SkyCloudDensity and SkyDeckWeight.
    CHECK(terrain.find("float deck = m.g * (1.0 - smoothstep(0.35, 0.75, litSky.w));") != std::string::npos);
    CHECK(terrain.find("return max(m.r, deck) * mix(0.75 + 0.25 * m.b, 0.5 + 0.5 * m.b, deck);") != std::string::npos);
    CHECK(SkyCloudDensity(0.6f, 1.0f, 0.4f, SkyDeckWeight(0.5f)) ==
          Catch::Approx(std::max(0.6f, 0.625f) * ((0.75f + 0.25f * 0.4f) * 0.375f + (0.5f + 0.5f * 0.4f) * 0.625f)));
    // The march steps as LitCloudStepAngleDeg, the low-sun fade as LitCloudLowSun.
    CHECK(sky.find("for (int k = 1; k <= " + std::to_string(kLitCloudSteps) + "; ++k) {") != std::string::npos);
    CHECK(sky.find("float a = radians(kLitCloudStepDeg) * fk * (0.5 + 0.5 * fk);") != std::string::npos);
    CHECK(sky.find("1.0 - smoothstep(kLitCloudFadeStartDeg, kLitCloudFadeEndDeg, litSun.w)") != std::string::npos);
    // The shadow edge as SkyShadowCover.
    CHECK(terrain.find("smoothstep(kSkyShadowThreshold - kSkyShadowEdge, kSkyShadowThreshold + kSkyShadowEdge, d)") !=
          std::string::npos);

    // The engine builds the map, its coverage and the turn from SkyDensity.hpp.
    const std::string engine = ReadTextFile(Gl33Dir() / "EngineGL33_SkyClouds.cpp");
    for (const char* use : {"rm::BuildSkyDensityMap(", "rm::SkyShadowCoverage(", "rm::SkyDomeTurnU(",
                            "rm::MixSkyTexels(", "rm::SkyDeckWeight(", "s_clouds.anchor.Update("})
    {
        CAPTURE(use);
        CHECK(engine.find(use) != std::string::npos);
    }
}
