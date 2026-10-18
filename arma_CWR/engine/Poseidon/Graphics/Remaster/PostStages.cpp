#include <Poseidon/Graphics/Remaster/PostStages.hpp>

namespace Poseidon::remaster
{

namespace
{

using gpu::Mark;
using gpu::Section;

// One row per PostStage, in list order (checked below).
constexpr PostStageInfo kStages[] = {
    {PostStage::Resolve, "resolve", "A3", true, false, Mark::ResolveEnd, Section::Resolve},
    {PostStage::LinearDepth, "lindepth", "A3", true, false, Mark::LinearDepthEnd, Section::LinearDepth},
    {PostStage::TagReadout, "tags", "B6", false, false, Mark::TagReadoutEnd, Section::TagReadout},
    // AO still reads the full-resolution depth; it moves onto the linear depth
    // in its own step (a pixel change).
    {PostStage::Ao, "ao", "A3", true, false, Mark::AoEnd, Section::Ao},
    {PostStage::Ssr, "ssr", "B6", false, false, Mark::SsrEnd, Section::Ssr},
    // B8: the air pass marches at half resolution, on the linear depth.
    {PostStage::Air, "air", "B8", true, true, Mark::AirEnd, Section::Air},
    {PostStage::Bloom, "bloom", "A3", true, false, Mark::BloomEnd, Section::Bloom},
    {PostStage::Shafts, "shafts", "A3", true, false, Mark::ShaftsEnd, Section::Shafts},
    {PostStage::CombatScatter, "scatter", "B4", false, false, Mark::CombatScatterEnd, Section::CombatScatter},
    {PostStage::Composite, "composite", "A3", true, false, Mark::CompositeEnd, Section::Composite},
    {PostStage::Temporal, "taa", "A5", true, false, Mark::TemporalEnd, Section::Temporal},
    // A5 landed TAA's sharpen; B11's luma sharpen (with TAA off) joined it.
    {PostStage::Sharpen, "sharpen", "A5", true, false, Mark::SharpenEnd, Section::Sharpen},
    {PostStage::DebugView, "debugview", "A3", true, true, Mark::DebugViewEnd, Section::DebugView},
};
static_assert(sizeof(kStages) / sizeof(kStages[0]) == kPostStageCount, "one PostStageInfo per PostStage");
static_assert(kPostStageCount == gpu::kPostStageMarks, "one GPU mark per post stage");

constexpr bool StagesInOrder()
{
    // Rows follow the enum, and the marks and sections follow the rows, so a
    // stage's section starts where the stage before it ends.
    for (int i = 0; i < kPostStageCount; ++i)
    {
        const PostStageInfo& s = kStages[i];
        if (static_cast<int>(s.id) != i)
            return false;
        if (static_cast<int>(s.endMark) != static_cast<int>(Mark::ResolveEnd) + i)
            return false;
        if (static_cast<int>(s.section) != static_cast<int>(Section::Resolve) + i)
            return false;
    }
    return true;
}
static_assert(StagesInOrder(), "kStages rows, marks and sections follow the PostStage order");
static_assert(kPostStageCount <= 32, "PostStagePlan::runs has a bit per stage");

std::uint32_t Bit(PostStage s)
{
    return 1u << static_cast<int>(s);
}

std::uint32_t LandedMask()
{
    std::uint32_t mask = 0;
    for (const PostStageInfo& s : kStages)
        if (s.landed)
            mask |= Bit(s.id);
    return mask;
}

} // namespace

const PostStageInfo& GetPostStageInfo(PostStage s)
{
    const int i = static_cast<int>(s);
    return kStages[i >= 0 && i < kPostStageCount ? i : 0];
}

PostStagePlan PlanPostStages(const PostFrameInputs& in)
{
    PostStagePlan plan;
    const bool debugDepth = in.debugView == kDebugViewLinearDepth;
    // The stages that can ask for the pass ask by their knob, switch or debug
    // view, whatever this frame's conditions then decide for them.
    const bool asked = in.ambientOcclusion > 0.0f || in.bloom > 0.0f || in.sunShafts > 0.0f || debugDepth ||
                       in.temporal || in.lumaSharpen || in.air;
    if (!asked)
        return plan;

    plan.runs |= Bit(PostStage::Resolve);
    // Occlusion needs the world depth.
    plan.aoStrength = in.worldDepth ? in.ambientOcclusion : 0.0f;
    if (plan.aoStrength > 0.0f)
        plan.runs |= Bit(PostStage::Ao);
    // A screen overlay in the world pass turns the glow off.
    plan.bloomStrength = in.glow ? in.bloom : 0.0f;
    if (plan.bloomStrength > 0.0f)
        plan.runs |= Bit(PostStage::Bloom);
    // The air pass needs the world depth, and over a screen overlay its haze
    // would grey the overlay's black surround.
    const bool air = in.air && in.worldDepth && in.glow;
    if (air)
        plan.runs |= Bit(PostStage::Air);
    // The shafts are depth tested glow toward a sun on screen; the air pass's
    // beams replace them.
    if (in.worldDepth && in.glow && in.sunShafts > 0.0f && in.shaftWeight > kShaftMinWeight && !(air && in.airBeams))
        plan.runs |= Bit(PostStage::Shafts);
    plan.runs |= Bit(PostStage::Composite);
    // The resolve reprojects with the world depth (a vehicle interior cleared
    // it), and a screen overlay in the world pass was drawn jittered over the
    // world: such a frame goes out as drawn and the history starts again.
    if (in.temporal && in.worldDepth && in.glow)
        plan.runs |= Bit(PostStage::Temporal) | Bit(PostStage::Sharpen);
    // With TAA off, the luma sharpen: the same stage, alone.
    else if (in.lumaSharpen)
        plan.runs |= Bit(PostStage::Sharpen);
    if (debugDepth)
        plan.runs |= Bit(PostStage::DebugView);

    plan.runs &= LandedMask();
    for (const PostStageInfo& s : kStages)
        if (s.readsLinearDepth && plan.Runs(s.id))
            plan.runs |= Bit(PostStage::LinearDepth);
    return plan;
}

} // namespace Poseidon::remaster
