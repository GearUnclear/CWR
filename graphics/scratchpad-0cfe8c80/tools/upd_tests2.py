import os
os.chdir(r'D:\Arma_CWA_worktrees\look-fog\arma_CWR')


def edit(p, pairs):
    s = open(p, encoding='utf-8', newline='').read()
    nl = '\r\n' if '\r\n' in s else '\n'
    for a, b in pairs:
        a2, b2 = a.replace('\n', nl), b.replace('\n', nl)
        assert s.count(a2) == 1, (p, a[:70], s.count(a2))
        s = s.replace(a2, b2)
    open(p, 'w', encoding='utf-8', newline='').write(s)


edit('tests/unit/engine/Poseidon/Graphics/test_feature_switches.cpp', [
    ('''const Gate& G(const GateInputs& in, Feature f, const LightingTuning& t = LightingTuning{})
{''',
     '''// The shipped tuning with the volumetric fog off: the gate rows below are each
// feature's own condition, and the fog (on by default) supersedes B8's air
// pass (airBeams, airHaze, valleyMist), whose gates it turns off while it runs
// (test_volumetric_fog.cpp checks that hand-over).
LightingTuning NoVolFog()
{
    LightingTuning t{};
    t.features.volFog = 0.0f;
    return t;
}

const Gate& G(const GateInputs& in, Feature f, const LightingTuning& t = NoVolFog())
{'''),
    ('''    // triFeatureState: the haze runs at noon and is not inert; the beams and
    // the mist wait for a low sun, mist or headlights.
    const LightingTuning t = Apply("remastered");''',
     '''    // triFeatureState: the haze runs at noon and is not inert; the beams and
    // the mist wait for a low sun, mist or headlights (the volumetric fog,
    // which runs in their place, off).
    const LightingTuning t = Apply("remastered,volFog=0");'''),
])

s = open('tests/unit/engine/Poseidon/Graphics/test_feature_switches.cpp', encoding='utf-8').read()
print('Apply("remastered,valleyMist=0") uses:', s.count('Apply("remastered,valleyMist=0")'))

edit('tests/unit/engine/Poseidon/Graphics/test_air_medium.cpp', [
    ('''    // The bloom and the composite read the frame with the air applied only
    // when the Air stage ran; otherwise the resolved frame, as before.
    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    REQUIRE_FALSE(post.empty());
    CHECK(CountOf(post, "f.plan.Runs(rm::PostStage::Air) ? p.airOutTex : p.colorTex") == 1);
    CHECK(CountOf(post, "BindTex(0, plan.Runs(rm::PostStage::Air) ? p.airOutTex : p.colorTex);") == 1);''',
     '''    // The bloom and the composite read the frame with the air applied only
    // when the Air stage ran; otherwise the resolved frame, as before
    // (PostSceneTex, which picks the volumetric fog's output when the fog ran
    // the stage instead).
    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    REQUIRE_FALSE(post.empty());
    CHECK(CountOf(post, "first ? PostSceneTex(f) : p.bloomTex[i - 1]") == 1);
    CHECK(CountOf(post, "BindTex(0, PostSceneTex(f));") == 1);
    const std::string scene =
        FunctionBody(ReadTextFile(Gl33Dir() / "EngineGL33_PostFog.cpp"), "unsigned int EngineGL33::PostSceneTex");
    REQUIRE_FALSE(scene.empty());
    CHECK(scene.find("if (!f.plan.Runs(rm::PostStage::Air))\\n        return _post.colorTex;") != std::string::npos);
    CHECK(scene.find("if (!f.plan.fog)\\n        return _post.airOutTex;") != std::string::npos);'''),
])

edit('tests/unit/engine/Poseidon/Graphics/test_volumetric_fog.cpp', [
    ('''    CHECK(inside > 8.0f * ridge);
    CHECK(inside ==
          Catch::Approx''', '''    CHECK(inside > 4.0f * ridge);
    CHECK(inside ==
          Catch::Approx'''),
    ('''            h[static_cast<size_t>(z) * n + x] = x < 6     ? -20.0f''', '''            h[static_cast<size_t>(z) * n + x] = x < 20    ? -20.0f'''),
    ('''TEST_CASE("Volumetric fog: the switch has landed with its knobs; its gate needs the post pass and a drawn world",
          "[remaster][fog][FeatureGates]")
{''', '''TEST_CASE("Volumetric fog: it supersedes B8's air pass, whose gates it turns off while it runs",
          "[remaster][fog][FeatureGates]")
{
    GateInputs in;
    in.sunElevationDeg = 8.0f;
    in.sunAzimuthDeg = 90.0f; // a morning sun in view: B8's beams and mist would run
    in.sunViewAngleDeg = 20.0f;
    LightingTuning t{};
    const FeatureGates on = ComputeFeatureGates(in, t);
    CHECK(on[Feature::VolFog].on);
    for (Feature f : {Feature::AirBeams, Feature::AirHaze, Feature::ValleyMist})
    {
        CAPTURE(GetFeatureInfo(f).key);
        CHECK_FALSE(on[f].on);
        CHECK(std::string(on[f].reason) == "volFog runs in its place");
    }
    t.features.volFog = 0.0f;
    const FeatureGates off = ComputeFeatureGates(in, t);
    CHECK(off[Feature::AirHaze].on);
    CHECK(off[Feature::ValleyMist].on);
    CHECK(off[Feature::AirBeams].on);
    // With the gates forced on (gates=0) the fog still takes their place.
    LightingTuning forced{};
    forced.gates = false;
    CHECK_FALSE(ComputeFeatureGates(in, forced)[Feature::AirHaze].on);
}

TEST_CASE("Volumetric fog: the switch has landed with its knobs; its gate needs the post pass and a drawn world",
          "[remaster][fog][FeatureGates]")
{'''),
])
print('ok')
