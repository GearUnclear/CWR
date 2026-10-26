#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Graphics/Core/GpuTimings.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/PostStages.hpp>

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
    // plus the linear depth and the debug view that shows it.
    CHECK(landed == 7);
    for (PostStage s : {PostStage::TagReadout, PostStage::Ssr, PostStage::Air, PostStage::CombatScatter,
                        PostStage::Temporal, PostStage::Sharpen})
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

TEST_CASE("Post stages: the linear depth runs exactly when a stage reads it", "[Graphics][remaster][PostStages]")
{
    // Today the debug view is its only reader; the stages that land with a
    // half-resolution pass set readsLinearDepth.
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
    CHECK(readers == 1);
    CHECK(GetPostStageInfo(PostStage::DebugView).readsLinearDepth);

    PostFrameInputs in = Shipped();
    for (int view = 0; view <= 5; ++view)
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
    // The registry offers the view (the overlay's combo, triSetLighting debug=5).
    const LightingKey* debug = FindLightingKey("debug");
    REQUIRE(debug != nullptr);
    CHECK(debug->maxValue == static_cast<float>(kDebugViewLinearDepth));
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
                for (int view = 0; view <= 6; ++view)
                    for (bool depth : {false, true})
                        for (bool glow : {false, true})
                            for (float weight : {0.0f, 0.5f})
                            {
                                PostFrameInputs in;
                                in.ambientOcclusion = ao;
                                in.bloom = bloom;
                                in.sunShafts = shafts;
                                in.debugView = view;
                                in.worldDepth = depth;
                                in.glow = glow;
                                in.shaftWeight = weight;
                                const PostStagePlan plan = PlanPostStages(in);
                                CAPTURE(ao, bloom, shafts, view, depth, glow, weight);
                                CHECK((plan.runs & ~landed) == 0u);
                                // A pass that runs resolves first and composites.
                                if (!plan.Empty())
                                {
                                    CHECK(plan.Runs(PostStage::Resolve));
                                    CHECK(plan.Runs(PostStage::Composite));
                                }
                            }
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
