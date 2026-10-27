#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Graphics/Core/GpuTimings.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/CombatLights.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/PostStages.hpp>
#include <Poseidon/Graphics/Remaster/ScreenReflections.hpp>
#include <Poseidon/Graphics/Remaster/VolumetricFog.hpp>

#include <algorithm>
#include <cmath>

#include <filesystem>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>
#include <string>

// The post pass as a fixed list of stages (INTEGRATION-PLAN.md section 3.6,
// Graphics/Remaster/PostStages.hpp): the list's order and GPU marks, and the
// plan of a frame, which decides the early return and every stage's gate. The
// plan must reproduce the pass as it ran before the list (same stages, same
// strengths), because A3 changes no pixel.

using namespace Poseidon::remaster;
namespace gpu = Poseidon::gpu;

namespace
{

// The shipped post knobs on a frame with the world depth and no overlay.
PostFrameInputs Shipped()
{
    const LightingTuning t{};
    PostFrameInputs in;
    in.ambientOcclusion = t.ambientOcclusion;
    in.bloom = t.bloom;
    in.sunShafts = t.sunShafts;
    in.debugView = t.debugView;
    in.shaftWeight = 0.5f; // a low sun in view
    return in;
}

std::uint32_t Mask(std::initializer_list<PostStage> stages)
{
    std::uint32_t m = 0;
    for (PostStage s : stages)
        m |= 1u << static_cast<int>(s);
    return m;
}

std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    if (!f.is_open())
        return {};
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path Gl33Dir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine" / "PoseidonGL33";
}

int CountOf(const std::string& text, const std::string& needle)
{
    int n = 0;
    for (size_t at = text.find(needle); at != std::string::npos; at = text.find(needle, at + needle.size()))
        ++n;
    return n;
}

// The enumerator spelling of each stage, as RunPostStage's cases name them.
const char* const kStageEnumerators[] = {"Resolve",  "LinearDepth", "TagReadout",    "Ao",        "Ssr",      "Air",
                                         "Bloom",    "Shafts",      "CombatScatter", "Composite", "Temporal", "Sharpen",
                                         "DebugView"};
static_assert(sizeof(kStageEnumerators) / sizeof(kStageEnumerators[0]) == kPostStageCount, "one name per stage");

} // namespace

TEST_CASE("Post stages: the list of plan 3.6, each with its own GPU mark and section",
          "[Graphics][remaster][PostStages]")
{
    // The order the plan fixes; later steps land stages, never reorder them.
    const PostStage order[] = {PostStage::Resolve,   PostStage::LinearDepth, PostStage::TagReadout,
                               PostStage::Ao,        PostStage::Ssr,         PostStage::Air,
                               PostStage::Bloom,     PostStage::Shafts,      PostStage::CombatScatter,
                               PostStage::Composite, PostStage::Temporal,    PostStage::Sharpen,
                               PostStage::DebugView};
    REQUIRE(sizeof(order) / sizeof(order[0]) == kPostStageCount);
    std::set<std::string> names;
    std::set<int> marks;
    int landed = 0;
    for (int i = 0; i < kPostStageCount; ++i)
    {
        const PostStageInfo& s = GetPostStageInfo(order[i]);
        CAPTURE(i, s.name);
        CHECK(static_cast<int>(s.id) == i);
        // The stage's section is the GpuTimings section of its mark, under the
        // stage's name, so triGpuTimes and GpuTimingSectionMs find it.
        CHECK(gpu::PostStageSection(s.endMark) == s.section);
        CHECK(std::string(gpu::SectionName(s.section)) == s.name);
        CHECK(static_cast<int>(s.endMark) > static_cast<int>(gpu::Mark::WorldEnd));
        CHECK(static_cast<int>(s.endMark) < static_cast<int>(gpu::Mark::PostEnd));
        CHECK(s.step[0] != '\0');
        names.insert(s.name);
        marks.insert(static_cast<int>(s.endMark));
        if (s.landed)
            ++landed;
    }
    CHECK(names.size() == static_cast<size_t>(kPostStageCount));
    CHECK(marks.size() == static_cast<size_t>(kPostStageCount));
    CHECK(gpu::kMarkCount <= 32);
    // A3 lands the passes that existed (resolve, AO, bloom, shafts, composite)
    // plus the linear depth and the debug view that shows it; A5 the TAA
    // resolve and its sharpen; B6 the tag readout and the reflections; B8 the
    // air pass.
    CHECK(landed == 12);
    CHECK(GetPostStageInfo(PostStage::Temporal).landed);
    CHECK(GetPostStageInfo(PostStage::Sharpen).landed);
    CHECK(GetPostStageInfo(PostStage::TagReadout).landed);
    CHECK(GetPostStageInfo(PostStage::Ssr).landed);
    CHECK(GetPostStageInfo(PostStage::Air).landed);
    for (PostStage s : {PostStage::CombatScatter})
    {
        CAPTURE(GetPostStageInfo(s).name);
        CHECK_FALSE(GetPostStageInfo(s).landed);
    }
}

TEST_CASE("Post stages: the shipped knobs run the pass as before", "[Graphics][remaster][PostStages]")
{
    const LightingTuning t{};
    PostFrameInputs in = Shipped();
    PostStagePlan plan = PlanPostStages(in);
    CHECK(plan.runs ==
          Mask({PostStage::Resolve, PostStage::Ao, PostStage::Bloom, PostStage::Shafts, PostStage::Composite}));
    CHECK(plan.aoStrength == t.ambientOcclusion);
    CHECK(plan.bloomStrength == t.bloom);
    // The linear depth feeds nothing yet: it does not run for the final image.
    CHECK_FALSE(plan.Runs(PostStage::LinearDepth));
    CHECK_FALSE(plan.Runs(PostStage::DebugView));

    // The sun behind the camera, under the horizon or the weight below the
    // threshold: no shafts, the rest as before.
    for (float weight : {0.0f, kShaftMinWeight, 0.5f * kShaftMinWeight})
    {
        CAPTURE(weight);
        in.shaftWeight = weight;
        plan = PlanPostStages(in);
        CHECK(plan.runs == Mask({PostStage::Resolve, PostStage::Ao, PostStage::Bloom, PostStage::Composite}));
    }
}

TEST_CASE("Post stages: the early return asks every stage", "[Graphics][remaster][PostStages]")
{
    // Every knob off: the frame stays as drawn.
    PostFrameInputs in = Shipped();
    in.ambientOcclusion = 0.0f;
    in.bloom = 0.0f;
    in.sunShafts = 0.0f;
    CHECK(PlanPostStages(in).Empty());
    in.ambientOcclusion = -1.0f; // a research value from triSetLighting
    CHECK(PlanPostStages(in).Empty());
    in.ambientOcclusion = 0.0f;

    // Any one knob keeps the pass, with the resolve and the composite.
    const std::uint32_t frame = Mask({PostStage::Resolve, PostStage::Composite});
    in.ambientOcclusion = 0.7f;
    CHECK(PlanPostStages(in).runs == (frame | Mask({PostStage::Ao})));
    in.ambientOcclusion = 0.0f;
    in.bloom = 0.35f;
    CHECK(PlanPostStages(in).runs == (frame | Mask({PostStage::Bloom})));
    in.bloom = 0.0f;
    in.sunShafts = 1.0f;
    CHECK(PlanPostStages(in).runs == (frame | Mask({PostStage::Shafts})));
    // A knob that is on keeps the pass even when the frame holds its stage off:
    // the composite still copies the resolved frame back, as before the list.
    in.shaftWeight = 0.0f;
    CHECK(PlanPostStages(in).runs == frame);
    in.sunShafts = 0.0f;
    in.ambientOcclusion = 0.7f;
    in.worldDepth = false;
    CHECK(PlanPostStages(in).runs == frame);
    CHECK(PlanPostStages(in).aoStrength == 0.0f);

    // The linear depth debug view keeps the pass on its own.
    in.ambientOcclusion = 0.0f;
    in.worldDepth = true;
    in.debugView = kDebugViewLinearDepth;
    CHECK(PlanPostStages(in).runs == (frame | Mask({PostStage::LinearDepth, PostStage::DebugView})));
    // The other debug views do not.
    for (int view : {1, 2, 3, 4})
    {
        CAPTURE(view);
        in.debugView = view;
        CHECK(PlanPostStages(in).Empty());
    }
}

TEST_CASE("Post stages: interior and optics frames hold their stages off", "[Graphics][remaster][PostStages]")
{
    // A vehicle interior clears the world depth: no occlusion and no shafts,
    // bloom stays.
    PostFrameInputs in = Shipped();
    in.worldDepth = false;
    PostStagePlan plan = PlanPostStages(in);
    CHECK(plan.runs == Mask({PostStage::Resolve, PostStage::Bloom, PostStage::Composite}));
    CHECK(plan.aoStrength == 0.0f);
    CHECK(plan.bloomStrength == LightingTuning{}.bloom);

    // A screen overlay (gunner optics): no glow, so no bloom and no shafts;
    // occlusion stays.
    in = Shipped();
    in.glow = false;
    plan = PlanPostStages(in);
    CHECK(plan.runs == Mask({PostStage::Resolve, PostStage::Ao, PostStage::Composite}));
    CHECK(plan.bloomStrength == 0.0f);
    CHECK(plan.aoStrength == LightingTuning{}.ambientOcclusion);
}

TEST_CASE("Post stages: the fog runs in a vehicle interior on the world depth saved before it",
          "[Graphics][remaster][PostStages][fog]")
{
    // The interior cleared the world depth after the world's depth and colour
    // were saved (EngineGL33::Clear): the fog still runs (the Air stage and
    // the linear depth it reads, made from the saved depth), and every other
    // stage that needs the world depth keeps holding off.
    PostFrameInputs in = Shipped();
    in.fog = true;
    in.worldDepth = false;
    in.fogWorldDepth = true;
    in.temporal = true;
    in.ssrTags = true;
    in.ssr = true;
    in.sunShafts = 1.0f;
    in.shaftWeight = 1.0f;
    PostStagePlan plan = PlanPostStages(in);
    CHECK(plan.fog);
    CHECK(plan.Runs(PostStage::Air));
    CHECK(plan.Runs(PostStage::LinearDepth));
    CHECK(plan.Runs(PostStage::Composite));
    CHECK_FALSE(plan.Runs(PostStage::Ao));
    CHECK(plan.aoStrength == 0.0f);
    CHECK_FALSE(plan.Runs(PostStage::Temporal));
    CHECK_FALSE(plan.Runs(PostStage::TagReadout));
    CHECK_FALSE(plan.Runs(PostStage::Ssr));
    CHECK_FALSE(plan.Runs(PostStage::Shafts));
    // An optic overlay over the interior keeps it too.
    in.glow = false;
    CHECK(PlanPostStages(in).fog);
    // No saved depth (the save failed): no fog, and B8's air pass never runs
    // without the world depth either.
    in.glow = true;
    in.fogWorldDepth = false;
    in.air = true;
    plan = PlanPostStages(in);
    CHECK_FALSE(plan.fog);
    CHECK_FALSE(plan.Runs(PostStage::Air));
    // The saved depth alone asks for nothing: the air pass needs the real one.
    in.fog = false;
    in.fogWorldDepth = true;
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Air));
}

TEST_CASE("Post stages: the fog's gate stays on in every view the player can take",
          "[Graphics][remaster][FeatureGates][fog]")
{
    // AI sight is cut by the whole medium in every view, so the drawn fog
    // must not switch off when the player sits in a cockpit, raises an optic
    // or turns on night vision (each a pop and a see-through before).
    const LightingTuning t{};
    GateInputs in;
    in.sunElevationDeg = 5.0f;
    in.sunAzimuthDeg = 90.0f;
    in.fog = 0.4f;
    for (int view = 0; view < 8; ++view)
    {
        GateInputs v = in;
        v.interior = (view & 1) != 0;
        v.optics = (view & 2) != 0;
        v.nightVision = (view & 4) != 0;
        CAPTURE(v.interior, v.optics, v.nightVision);
        CHECK(ComputeFeatureGates(v, t)[Feature::VolFog].on);
        // The Lighting row on Classic: no overhaul fog in any view.
        LightingTuning classic = t;
        classic.remastered = false;
        CHECK_FALSE(ComputeFeatureGates(v, classic)[Feature::VolFog].on);
        // Post effects off: the forward fog's classic edge, in every view.
        LightingTuning noPost = t;
        noPost.postEffects = false;
        CHECK_FALSE(ComputeFeatureGates(v, noPost)[Feature::VolFog].on);
    }
    // Under night vision every other feature keeps holding off.
    GateInputs nv = in;
    nv.nightVision = true;
    const FeatureGates g = ComputeFeatureGates(nv, t);
    for (Feature f : {Feature::Taa, Feature::Ssr, Feature::PhotoLight, Feature::AirHaze, Feature::ValleyMist})
    {
        CAPTURE(GetFeatureInfo(f).key);
        CHECK_FALSE(g[f].on);
    }
}

TEST_CASE("Post stages: night vision's forward fog keeps the same edge as the remastered programs and AI sight",
          "[Graphics][remaster][PostStages][fog]")
{
    // The classic programs (night vision) get the forward fog range that
    // gives min(factor * fogEdgeScale, 1): the remastered programs' litHaze.x
    // edge, and AI sight's forward edge, clamp((end - d) / ((1 -
    // kFogEdgeStart) end)). The weather fog itself is the volume's.
    for (float start : {10.0f, 300.0f, 900.0f})
        for (float end : {1000.0f, 2500.0f})
        {
            const float inv = 1.0f / (end - start);
            const float scale = VolFogEdgeScale(start, end);
            const ForwardFogRange r = ScaleForwardFogRange(start, inv, scale);
            CAPTURE(start, end, scale);
            for (float d = 0.0f; d <= end * 1.2f; d += end / 64.0f)
            {
                CAPTURE(d);
                const float remastered = std::min(ForwardFogFactor(d, start, inv) * scale, 1.0f);
                const float edge = std::clamp((end - d) / ((1.0f - kFogEdgeStart) * end), 0.0f, 1.0f);
                CHECK(std::fabs(ForwardFogFactor(d, r.start, r.invRange) - remastered) < 1e-4f);
                CHECK(std::fabs(ForwardFogFactor(d, r.start, r.invRange) - edge) < 1e-4f);
            }
        }
    // No scale or no range: the classic fog as it was.
    const ForwardFogRange same = ScaleForwardFogRange(100.0f, 1.0f / 900.0f, 1.0f);
    CHECK(same.start == 100.0f);
    CHECK(same.invRange == 1.0f / 900.0f);
    const ForwardFogRange empty = ScaleForwardFogRange(100.0f, 0.0f, 5.0f);
    CHECK(empty.start == 100.0f);
    CHECK(empty.invRange == 0.0f);
}

TEST_CASE("Post stages: the TAA resolve and its sharpen run together on a jittered frame",
          "[Graphics][remaster][PostStages][TemporalAA]")
{
    // Without the jitter (taa switch or gate off) nothing changes: the plan of
    // before, which is what makes the switch's "off" the look before A5.
    PostFrameInputs in = Shipped();
    const std::uint32_t before = PlanPostStages(in).runs;
    CHECK(before ==
          Mask({PostStage::Resolve, PostStage::Ao, PostStage::Bloom, PostStage::Shafts, PostStage::Composite}));

    in.temporal = true;
    PostStagePlan plan = PlanPostStages(in);
    CHECK(plan.runs == (before | Mask({PostStage::Temporal, PostStage::Sharpen})));
    // The other stages and their strengths are untouched.
    CHECK(plan.aoStrength == LightingTuning{}.ambientOcclusion);
    CHECK(plan.bloomStrength == LightingTuning{}.bloom);
    CHECK_FALSE(plan.Runs(PostStage::LinearDepth));

    // TAA asks for the pass on its own: with every knob off the frame still
    // gets the resolve, the composite and the two TAA stages.
    PostFrameInputs bare;
    bare.temporal = true;
    CHECK(PlanPostStages(bare).runs ==
          Mask({PostStage::Resolve, PostStage::Composite, PostStage::Temporal, PostStage::Sharpen}));

    // A vehicle interior (no world depth to reproject with) or a screen
    // overlay drawn over the jittered world: the frame goes out as drawn.
    in.worldDepth = false;
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Temporal));
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Sharpen));
    in.worldDepth = true;
    in.glow = false;
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Temporal));
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Sharpen));
    CHECK(PlanPostStages(in).Runs(PostStage::Composite));

    // The debug view still draws last, over the resolved frame.
    in.glow = true;
    in.debugView = kDebugViewLinearDepth;
    plan = PlanPostStages(in);
    CHECK(plan.Runs(PostStage::Temporal));
    CHECK(plan.Runs(PostStage::DebugView));
    CHECK(static_cast<int>(PostStage::DebugView) > static_cast<int>(PostStage::Sharpen));
    CHECK(static_cast<int>(PostStage::Temporal) > static_cast<int>(PostStage::Composite));
}

TEST_CASE("Post stages: with TAA off the luma sharpen runs the Sharpen stage alone",
          "[Graphics][remaster][PostStages][LumaSharpen]")
{
    // Off (the sharpen switch, its gate or its strength): the plan of before.
    PostFrameInputs in = Shipped();
    const std::uint32_t before = PlanPostStages(in).runs;
    in.lumaSharpen = true;
    PostStagePlan plan = PlanPostStages(in);
    CHECK(plan.runs == (before | Mask({PostStage::Sharpen})));
    CHECK_FALSE(plan.Runs(PostStage::Temporal));
    CHECK(plan.aoStrength == LightingTuning{}.ambientOcclusion);
    CHECK(plan.bloomStrength == LightingTuning{}.bloom);

    // It asks for the pass on its own.
    PostFrameInputs bare;
    bare.lumaSharpen = true;
    CHECK(PlanPostStages(bare).runs == Mask({PostStage::Resolve, PostStage::Composite, PostStage::Sharpen}));

    // One sharpen: a jittered frame gets TAA's (the backend never asks for both).
    in.temporal = true;
    plan = PlanPostStages(in);
    CHECK(plan.Runs(PostStage::Temporal));
    CHECK(plan.Runs(PostStage::Sharpen));

    // Where TAA holds off (a vehicle interior, an optics overlay), the luma
    // sharpen takes over (FeatureGates: "TAA gated off in an interior").
    in.temporal = false;
    in.worldDepth = false;
    CHECK(PlanPostStages(in).Runs(PostStage::Sharpen));
    in.worldDepth = true;
    in.glow = false;
    CHECK(PlanPostStages(in).Runs(PostStage::Sharpen));
}

TEST_CASE("Post stages: the luma sharpen reads the composite and writes the frame target",
          "[Graphics][remaster][PostStages][LumaSharpen][GL33]")
{
    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    const std::string temporal = ReadTextFile(Gl33Dir() / "EngineGL33_PostTemporal.cpp");
    REQUIRE_FALSE(post.empty());
    REQUIRE_FALSE(temporal.empty());
    // The composite goes to the TAA input whenever the Sharpen stage runs.
    CHECK(CountOf(post, "DrawTo(plan.Runs(rm::PostStage::Sharpen) ? p.taaInFbo : _ssaaFbo, f.w, f.h);") == 1);
    // The luma sharpen asks only on a frame without TAA, with its gate on.
    CHECK(CountOf(post, "in.lumaSharpen = !taaFrame && _featureGates.computed") == 1);
    CHECK(CountOf(post, "_featureGates[rm::Feature::Sharpen].on && _lighting.sharpenStrength > 0.0f;") == 1);
    // The stage picks the luma sharpen when the resolve did not run, reading
    // the composite's output.
    const size_t at = temporal.find("void EngineGL33::PostSharpen(const PostFrame& f)");
    REQUIRE(at != std::string::npos);
    const std::string body = temporal.substr(at, temporal.find("\n}\n", at) - at);
    CHECK(body.find("if (!f.plan.Runs(rm::PostStage::Temporal))") != std::string::npos);
    CHECK(body.find("glUseProgram(p.progLumaSharpen);") != std::string::npos);
    CHECK(body.find("BindTex(0, p.taaInTex);") != std::string::npos);
    CHECK(body.find("glUseProgram(p.progTaaSharpen);") != std::string::npos);
}

TEST_CASE("Post stages: the linear depth runs exactly when a stage reads it", "[Graphics][remaster][PostStages]")
{
    // Its readers: the debug view, the reflections (B6) and the air pass (B8);
    // the stages that land with a half-resolution pass set readsLinearDepth.
    int readers = 0;
    for (int i = 0; i < kPostStageCount; ++i)
    {
        const PostStageInfo& s = GetPostStageInfo(static_cast<PostStage>(i));
        if (s.readsLinearDepth)
        {
            CAPTURE(s.name);
            CHECK(s.landed);
            ++readers;
        }
    }
    CHECK(readers == 3);
    CHECK(GetPostStageInfo(PostStage::DebugView).readsLinearDepth);
    CHECK(GetPostStageInfo(PostStage::Ssr).readsLinearDepth);
    CHECK(GetPostStageInfo(PostStage::Air).readsLinearDepth);

    PostFrameInputs in = Shipped();
    // 6 is the forward shaders' light-tile view (A6) and 7 their albedo view
    // (B2): no post stage of their own; 8 is the air pass's (B8), which needs
    // its switches, 9 and 10 the reflections' (B6), which need the tags.
    for (int view = 0; view <= kDebugViewSsrReflection; ++view)
    {
        CAPTURE(view);
        in.debugView = view;
        const PostStagePlan plan = PlanPostStages(in);
        CHECK(plan.Runs(PostStage::LinearDepth) == (view == kDebugViewLinearDepth));
        CHECK(plan.Runs(PostStage::DebugView) == (view == kDebugViewLinearDepth));
        // The debug view adds its stages and changes nothing else.
        CHECK((plan.runs & ~Mask({PostStage::LinearDepth, PostStage::DebugView})) ==
              Mask({PostStage::Resolve, PostStage::Ao, PostStage::Bloom, PostStage::Shafts, PostStage::Composite}));
    }
    // With the tags written and the reflections' gate on, the reflections
    // bring the linear depth.
    in.debugView = 0;
    in.ssrTags = true;
    in.ssr = true;
    CHECK(PlanPostStages(in).Runs(PostStage::LinearDepth));
    // The registry offers the view (the overlay's combo, triSetLighting debug=5),
    // the light tiles after it (debug=6), B2's albedo view (debug=7), the air
    // pass's in-scatter (debug=8, B8) and the reflections' two last (debug=9
    // and 10, B6), and the combat shadow mask after them (debug=11, B4), then the surfaces lane's three
    // forward views (12 to 14).
    const LightingKey* debug = FindLightingKey("debug");
    REQUIRE(debug != nullptr);
    CHECK(debug->maxValue >= static_cast<float>(kDebugViewLinearDepth));
    CHECK(kDebugViewAlbedo == 7);
    CHECK(kDebugViewAir == kDebugViewAlbedo + 1);
    CHECK(kDebugViewSsrTags == kDebugViewAir + 1);
    CHECK(kDebugViewSsrReflection == kDebugViewSsrTags + 1);
    CHECK(Poseidon::lighting::kDebugViewCombatMask == kDebugViewSsrReflection + 1);
    CHECK(kDebugViewSkyLight == Poseidon::lighting::kDebugViewCombatMask + 1);
    CHECK(debug->maxValue == static_cast<float>(kDebugViewSurfaceSpecular));
}

TEST_CASE("Post stages: the reflections read the tags out whenever they were written, and trace when gated on",
          "[Graphics][remaster][PostStages][ssr]")
{
    // Off (the switch: no tags): the plan of before, which is what makes the
    // switch's "off" the look before B6.
    PostFrameInputs in = Shipped();
    const std::uint32_t before = PlanPostStages(in).runs;
    in.ssr = true; // a gate without tags cannot run anything
    CHECK(PlanPostStages(in).runs == before);

    // Tags written, gate off (dry, no water on screen): the readout alone
    // measures the shares, so the gate can open again; nothing else changes.
    in.ssr = false;
    in.ssrTags = true;
    PostStagePlan plan = PlanPostStages(in);
    CHECK(plan.runs == (before | Mask({PostStage::TagReadout})));
    CHECK(plan.aoStrength == LightingTuning{}.ambientOcclusion);
    CHECK(plan.bloomStrength == LightingTuning{}.bloom);

    // Gate on: the trace, with the linear depth it marches.
    in.ssr = true;
    plan = PlanPostStages(in);
    CHECK(plan.runs == (before | Mask({PostStage::TagReadout, PostStage::LinearDepth, PostStage::Ssr})));
    // Before bloom, so reflected highlights bloom; the readout before both.
    CHECK(static_cast<int>(PostStage::Ssr) < static_cast<int>(PostStage::Bloom));
    CHECK(static_cast<int>(PostStage::TagReadout) < static_cast<int>(PostStage::Ssr));

    // The tags ask for the pass on their own (the readout keeps the gate fed).
    PostFrameInputs bare;
    bare.ssrTags = true;
    CHECK(PlanPostStages(bare).runs == Mask({PostStage::Resolve, PostStage::TagReadout, PostStage::Composite}));

    // A vehicle interior: no world depth, no readout and no trace. Optics: the
    // readout, no trace.
    in.worldDepth = false;
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::TagReadout));
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Ssr));
    in.worldDepth = true;
    in.glow = false;
    CHECK(PlanPostStages(in).Runs(PostStage::TagReadout));
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Ssr));

    // The diagnostic views trace whatever the gate says, while there are tags.
    in.glow = true;
    in.ssr = false;
    for (int view : {kDebugViewSsrTags, kDebugViewSsrReflection})
    {
        CAPTURE(view);
        in.debugView = view;
        CHECK(PlanPostStages(in).Runs(PostStage::Ssr));
        in.ssrTags = false;
        CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Ssr));
        in.ssrTags = true;
    }
}

TEST_CASE("Post stages: a stage that has not landed never runs", "[Graphics][remaster][PostStages]")
{
    std::uint32_t landed = 0;
    for (int i = 0; i < kPostStageCount; ++i)
        if (GetPostStageInfo(static_cast<PostStage>(i)).landed)
            landed |= 1u << i;
    // Every combination of the inputs that decide a plan.
    for (float ao : {0.0f, 0.7f})
        for (float bloom : {0.0f, 0.35f})
            for (float shafts : {0.0f, 1.0f})
                for (int view = 0; view <= kDebugViewSsrReflection; ++view)
                    for (bool depth : {false, true})
                        for (bool glow : {false, true})
                            for (float weight : {0.0f, 0.5f})
                                for (bool temporal : {false, true})
                                    for (bool luma : {false, true})
                                        for (int extra = 0; extra < 12; ++extra)
                                        {
                                            PostFrameInputs in;
                                            in.ambientOcclusion = ao;
                                            in.bloom = bloom;
                                            in.sunShafts = shafts;
                                            in.debugView = view;
                                            in.worldDepth = depth;
                                            in.glow = glow;
                                            in.shaftWeight = weight;
                                            in.temporal = temporal;
                                            in.lumaSharpen = luma;
                                            const int air = extra % 3;
                                            const int ssr = extra / 3;
                                            in.air = air > 0;
                                            in.airBeams = air > 1;
                                            in.ssrTags = (ssr & 1) != 0;
                                            in.ssr = (ssr & 2) != 0;
                                            const PostStagePlan plan = PlanPostStages(in);
                                            CAPTURE(ao, bloom, shafts, view, depth, glow, weight, temporal, luma, air,
                                                    ssr);
                                            // The reflections run only on the tags the world wrote,
                                            // with its depth; the trace only after the readout.
                                            if (!in.ssrTags || !depth)
                                            {
                                                CHECK_FALSE(plan.Runs(PostStage::TagReadout));
                                                CHECK_FALSE(plan.Runs(PostStage::Ssr));
                                            }
                                            if (plan.Runs(PostStage::Ssr))
                                            {
                                                CHECK(plan.Runs(PostStage::TagReadout));
                                                CHECK(plan.Runs(PostStage::LinearDepth));
                                            }
                                            CHECK((plan.runs & ~landed) == 0u);
                                            // A pass that runs resolves first and composites.
                                            if (!plan.Empty())
                                            {
                                                CHECK(plan.Runs(PostStage::Resolve));
                                                CHECK(plan.Runs(PostStage::Composite));
                                            }
                                            // The resolve brings its sharpen, and never runs on a
                                            // frame drawn without the jitter. The sharpen runs
                                            // alone only as the luma sharpen.
                                            if (plan.Runs(PostStage::Temporal))
                                                CHECK(plan.Runs(PostStage::Sharpen));
                                            if (!temporal)
                                                CHECK_FALSE(plan.Runs(PostStage::Temporal));
                                            if (plan.Runs(PostStage::Sharpen) && !plan.Runs(PostStage::Temporal))
                                                CHECK(luma);
                                            if (!luma)
                                                CHECK(plan.Runs(PostStage::Temporal) == plan.Runs(PostStage::Sharpen));
                                            // The air pass runs exactly when asked and the
                                            // world is there to march; its beams replace the
                                            // shafts; it reads the linear depth.
                                            CHECK(plan.Runs(PostStage::Air) == (air > 0 && depth && glow));
                                            if (plan.Runs(PostStage::Air))
                                                CHECK(plan.Runs(PostStage::LinearDepth));
                                            if (plan.Runs(PostStage::Air) && air > 1)
                                                CHECK_FALSE(plan.Runs(PostStage::Shafts));
                                        }
}

TEST_CASE("Post stages: the air pass asks for the pass, and its beams replace the shafts",
          "[Graphics][remaster][PostStages][AirMedium]")
{
    // Off (the three switches or their gates): the plan of before.
    PostFrameInputs in = Shipped();
    const std::uint32_t before = PlanPostStages(in).runs;
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Air));

    // Haze or mist only: the air pass and its linear depth, the shafts stay.
    in.air = true;
    PostStagePlan plan = PlanPostStages(in);
    CHECK(plan.runs == (before | Mask({PostStage::LinearDepth, PostStage::Air})));
    CHECK(plan.aoStrength == LightingTuning{}.ambientOcclusion);
    CHECK(plan.bloomStrength == LightingTuning{}.bloom);
    // With the beams the legacy shafts hold off.
    in.airBeams = true;
    plan = PlanPostStages(in);
    CHECK(plan.Runs(PostStage::Air));
    CHECK_FALSE(plan.Runs(PostStage::Shafts));
    CHECK(plan.runs == ((before & ~Mask({PostStage::Shafts})) | Mask({PostStage::LinearDepth, PostStage::Air})));

    // It asks for the pass by itself.
    PostFrameInputs bare;
    bare.air = true;
    CHECK(PlanPostStages(bare).runs ==
          Mask({PostStage::Resolve, PostStage::LinearDepth, PostStage::Air, PostStage::Composite}));

    // A vehicle interior (no world depth) or a screen overlay (the haze would
    // grey its black surround): no air, and the shafts follow their own rule.
    in.worldDepth = false;
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Air));
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::LinearDepth));
    in.worldDepth = true;
    in.glow = false;
    CHECK_FALSE(PlanPostStages(in).Runs(PostStage::Air));
    CHECK(PlanPostStages(in).Runs(PostStage::Composite));

    // The air pass runs before the bloom and the composite, which read its
    // output (EngineGL33_PostFX.cpp).
    CHECK(static_cast<int>(PostStage::Air) > static_cast<int>(PostStage::Ao));
    CHECK(static_cast<int>(PostStage::Air) < static_cast<int>(PostStage::Bloom));
    CHECK(GetPostStageInfo(PostStage::Air).section == gpu::Section::Air);
    CHECK(std::string(GetPostStageInfo(PostStage::Air).name) == "air");
}

TEST_CASE("Post stages: the backend runs every landed stage and marks it from the table",
          "[Graphics][remaster][PostStages][GL33]")
{
    // EndWorldScene walks the list and stamps each stage's mark from the table:
    // the only GPU marks in the post pass are WorldEnd, the table's and PostEnd.
    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    REQUIRE_FALSE(post.empty());
    CHECK(CountOf(post, "GpuMark(") == 3);
    CHECK(CountOf(post, "GpuMark(Poseidon::gpu::Mark::WorldEnd);") == 1);
    CHECK(CountOf(post, "GpuMark(rm::GetPostStageInfo(stage).endMark);") == 1);
    CHECK(CountOf(post, "GpuMark(Poseidon::gpu::Mark::PostEnd);") == 1);
    CHECK(CountOf(post, "rm::PlanPostStages(in)") == 1);
    // RunPostStage has a case for every landed stage and none for the others.
    for (int i = 0; i < kPostStageCount; ++i)
    {
        const PostStageInfo& s = GetPostStageInfo(static_cast<PostStage>(i));
        const std::string label = std::string("case rm::PostStage::") + kStageEnumerators[i] + ":";
        CAPTURE(label);
        CHECK(CountOf(post, label) == (s.landed ? 1 : 0));
    }
}
