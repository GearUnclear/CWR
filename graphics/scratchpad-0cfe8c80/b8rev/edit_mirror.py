from pathlib import Path

R = Path(r"D:\Arma_CWA_worktrees\gr-B8\arma_CWR")


def rep(path, old, new):
    s = path.read_text(encoding="utf-8")
    assert s.count(old) == 1, (path.name, old[:80], s.count(old))
    path.write_text(s.replace(old, new), encoding="utf-8", newline="\n")


h = R / "engine/Poseidon/Graphics/Remaster/AirMedium.hpp"
rep(h, """//   - beams (airBeams): single scattering of the sun (and of headlights) in a
//     ground mist, marched at half resolution with the sun's visibility from
//     the shadow map, the terrain horizon and the cloud field, so crowns,
//     roofs and ridges cut light shafts out of it. The mist is thin in clear
//     weather and thick in setFog and rain (fans of god rays).
""", """//   - beams (airBeams): single scattering of the sun (and of headlights) in a
//     ground mist, marched at half resolution with the sun's visibility from
//     the shadow map, the terrain horizon and the cloud field, so crowns,
//     roofs and ridges cut light shafts out of it. The mist is thin in clear
//     weather and thick in setFog and rain (fans of god rays). The shafts'
//     contrast works on the share of the scattering that sees the sun
//     (BeamShaft), so a ray that only runs longer through the mist, as over
//     open ground toward the horizon, is not boosted into a band.
""")
rep(h, """// `gainKnob` below 0 takes the gain from the weather (kBeamGainClear in clear
// air, kBeamGainMisty in mist); 0 or more is used as given.
AirMedium ComputeAirMedium(const AirInputs& in, const AirParts& parts, float gainKnob);
""", """// `gainKnob` below 0 takes the gain from the weather (kBeamGainClear in clear
// air, kBeamGainMisty in mist); 0 or more is used as given.
AirMedium ComputeAirMedium(const AirInputs& in, const AirParts& parts, float gainKnob);

// The sun beams' contrast at one pixel (the apply's beam step, which runs the
// same steps on a vec2 with the headlight cones). `lit` is the ray's
// scattering that sees the sun, `open` the same ray's scattering as if all of
// it did, `meanLit` and `meanOpen` their depth-matched local means, `conf` the
// upsample's depth match (0..1). The contrast works on the share that sees the
// sun, lit / open, against the mean share: what stands above it is boosted by
// gain - 1, the rest loses up to `highpass` of itself but never more, and the
// result goes back to scattering (times open). A field that sees the sun all
// over keeps (1 - highpass) of its scattering, however that varies with the
// ray's length through the mist.
float BeamShaft(float lit, float open, float meanLit, float meanOpen, float conf, float highpass, float gain);
""")
c = R / "engine/Poseidon/Graphics/Remaster/AirMedium.cpp"
rep(c, """float ExpOpticalLength(float yA, float yB, float len, float scaleHeight)""", """float BeamShaft(float lit, float open, float meanLit, float meanOpen, float conf, float highpass, float gain)
{
    float v = std::clamp(lit / std::max(open, 1e-7f), 0.0f, 1.0f);
    const float local = std::clamp(meanLit / std::max(meanOpen, 1e-7f), 0.0f, 1.0f);
    // No depth match: clamped to the mean share and no boost.
    const float low = std::min(v, local);
    v = low + (v - low) * conf;
    const float boost = (gain - 1.0f) * conf;
    const float s = std::max(v - highpass * local, (1.0f - highpass) * v) + boost * std::max(v - local, 0.0f);
    return s * open;
}

float ExpOpticalLength(float yA, float yB, float len, float scaleHeight)""")

t = R / "tests/unit/engine/Poseidon/Graphics/test_air_medium.cpp"
rep(t, """    CHECK(apply.find("shaft = max(shaft - airBeams.y * mean, floorAt) + boost * max(shaft - mean, vec2(0.0));") !=
          std::string::npos);""", """    CHECK(apply.find("shaft = max(shaft - airBeams.y * local, floorAt) + boost * max(shaft - local, vec2(0.0));") !=
          std::string::npos);""")
rep(t, """    // The haze's chroma, the valley floor's rings, the mist's top and banks.
    CHECK(apply.find("vec3(" + Num(kHazeChromaR) + ", 1.0, " + Num(kHazeChromaB) + ")") != std::string::npos);""",
    """    // The haze's chroma and the sky light of shaded air, the valley floor's
    // rings, the mist's top and banks.
    CHECK(apply.find("vec3(" + Num(kHazeChromaR) + ", 1.0, " + Num(kHazeChromaB) + ")") != std::string::npos);
    CHECK(apply.find("const float kAirHazeShadowAmbient = " + Num(kHazeShadowAmbient) + ";") != std::string::npos);
    CHECK(apply.find("zen * (skyLum * kAirHazeShadowAmbient)") != std::string::npos);""")
rep(t, """TEST_CASE("Air pass: the march and the apply paste the remastered prelude's pieces, once",""",
    """TEST_CASE("Air medium: the beams' contrast works on the share of the scattering that sees the sun",
          "[remaster][AirMedium][Shaders]")
{
    const float hp = kBeamHighpassClear;
    const float gain = kBeamGainClear;
    // Open ground toward a low sun (lowsun_fp): every ray sees the sun, but a
    // ray to the far plain runs longer through the ground mist than its
    // depth-matched neighbours (the mean leans to the nearer ground), so its
    // scattering stands above the mean, 0.18 against 0.15. Its share does
    // not: no boost, (1 - high-pass) of it, as anywhere the sun reaches the
    // whole field. A contrast on the scattering itself gave this ray
    // 0.18 - 0.85 * 0.15 + 3 * 0.03 = 0.14, five times as much: the pale band
    // along the plain of the first B8 review.
    CHECK(BeamShaft(0.18f, 0.18f, 0.15f, 0.15f, 1.0f, hp, gain) == Catch::Approx((1.0f - hp) * 0.18f));
    CHECK(BeamShaft(0.05f, 0.05f, 0.15f, 0.15f, 1.0f, hp, gain) == Catch::Approx((1.0f - hp) * 0.05f));
    // A beam through a gap in the crowns: the ray sees the sun, its
    // neighbours mostly do not (the mean share is 0.3). It is boosted above
    // the plain scattering of the ray: a shaft.
    const float beam = BeamShaft(0.18f, 0.18f, 0.054f, 0.18f, 1.0f, hp, gain);
    CHECK(beam == Catch::Approx((1.0f - hp * 0.3f + (gain - 1.0f) * 0.7f) * 0.18f));
    CHECK(beam > 0.18f);
    // A shaded ray among lit ones keeps (1 - high-pass) of its own share.
    CHECK(BeamShaft(0.036f, 0.18f, 0.18f, 0.18f, 1.0f, hp, gain) == Catch::Approx((1.0f - hp) * 0.036f));
    // No depth match (a thin branch against the glow): the share is clamped
    // to the mean share and gets no boost.
    CHECK(BeamShaft(0.18f, 0.18f, 0.054f, 0.18f, 0.0f, hp, gain) == Catch::Approx((1.0f - hp) * 0.3f * 0.18f));
    // In mist the same rule with the misty high-pass and gain.
    CHECK(BeamShaft(0.6f, 0.6f, 0.4f, 0.4f, 1.0f, kBeamHighpassMisty, kBeamGainMisty) ==
          Catch::Approx((1.0f - kBeamHighpassMisty) * 0.6f));
    // Nothing to scatter: nothing added; never below zero.
    CHECK(BeamShaft(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, hp, gain) == 0.0f);
    CHECK(BeamShaft(0.01f, 0.2f, 0.2f, 0.2f, 1.0f, hp, gain) >= 0.0f);

    // The apply runs the same steps (the sun in x, the headlight cones in y).
    const std::string apply = gl::PostShaderSource(gl::PostShader::AirApply);
    CHECK(apply.find("vec2 shaft = vec2(clamp(air.r / max(air.a, 1e-7), 0.0, 1.0), air.g);") != std::string::npos);
    CHECK(apply.find("vec2 local = vec2(clamp(mean.x / max(mean.z, 1e-7), 0.0, 1.0), mean.y);") != std::string::npos);
    CHECK(apply.find("shaft = mix(min(shaft, local), shaft, conf);") != std::string::npos);
    CHECK(apply.find("glow = airSunColor.rgb * (shaft.x * air.a) + airSpotTint.rgb * shaft.y;") != std::string::npos);
    // The march hands it both scatterings and the view depth in the second
    // target; the blur and the mean match depths from there, and the mean
    // averages both scatterings.
    const std::string march = gl::PostShaderSource(gl::PostShader::AirMarch);
    CHECK(march.find("sumOpen += w;") != std::string::npos);
    CHECK(march.find("fragColor = vec4(sunBeams, spots, vis, sunOpen);") != std::string::npos);
    CHECK(march.find("fragMist = vec4(top, mistDensity, min(z, 6e4), 1.0);") != std::string::npos);
    const std::string blur = gl::PostShaderSource(gl::PostShader::AirBlur);
    CHECK(blur.find("float zc = texture(airDepthTex, vUV).b;") != std::string::npos);
    CHECK(blur.find("fragColor = sum / wsum;") != std::string::npos);
    const std::string mean = gl::PostShaderSource(gl::PostShader::AirMean);
    CHECK(mean.find("float zc = texture(airDepthTex, vUV).b;") != std::string::npos);
    CHECK(mean.find("mean += texture(airTex, uv).rga * wk;") != std::string::npos);
    CHECK(apply.find("float dn = abs(m.b - z) / (0.05 * z + 0.3);") != std::string::npos);
    // The driver binds the depth on a post unit (never unit 1) for both.
    const std::string air = ReadTextFile(Gl33Dir() / "EngineGL33_PostAir.cpp");
    CHECK(CountOf(air, "\\"airDepthTex\\", 3);") == 2);
    CHECK(CountOf(air, "BindTex(3, p.airMistTex);") == 2);
}

TEST_CASE("Air pass: the march and the apply paste the remastered prelude's pieces, once",""")
print("ok")
