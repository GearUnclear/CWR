#include <Poseidon/Graphics/Remaster/PostStages.hpp>

#include <Poseidon/Graphics/Remaster/ScreenReflections.hpp>

#include <algorithm>

namespace Poseidon::remaster
{

namespace
{

using gpu::Mark;
using gpu::Section;

// One row per PostStage, in list order (checked below).
constexpr PostStageInfo kStages[] = {
    {PostStage::Resolve, "resolve", "post pass", true, false, Mark::ResolveEnd, Section::Resolve},
    {PostStage::LinearDepth, "lindepth", "post pass", true, false, Mark::LinearDepthEnd, Section::LinearDepth},
    {PostStage::TagReadout, "tags", "reflections", true, false, Mark::TagReadoutEnd, Section::TagReadout},
    // AO reads the full-resolution depth.
    {PostStage::Ao, "ao", "post pass", true, false, Mark::AoEnd, Section::Ao},
    // The reflections march the half-resolution linear depth.
    {PostStage::Ssr, "ssr", "reflections", true, true, Mark::SsrEnd, Section::Ssr},
    // The air pass marches at half resolution, on the linear depth; the
    // volumetric fog (EngineGL33_PostFog.cpp) runs on the same stage in its
    // place.
    {PostStage::Air, "air", "air", true, true, Mark::AirEnd, Section::Air},
    {PostStage::Bloom, "bloom", "post pass", true, false, Mark::BloomEnd, Section::Bloom},
    {PostStage::Shafts, "shafts", "post pass", true, false, Mark::ShaftsEnd, Section::Shafts},
    {PostStage::CombatScatter, "scatter", "combat light", false, false, Mark::CombatScatterEnd, Section::CombatScatter},
    {PostStage::Composite, "composite", "post pass", true, false, Mark::CompositeEnd, Section::Composite},
    {PostStage::Temporal, "taa", "temporal AA", true, false, Mark::TemporalEnd, Section::Temporal},
    // TAA's sharpen, or with TAA off the luma sharpen.
    {PostStage::Sharpen, "sharpen", "temporal AA", true, false, Mark::SharpenEnd, Section::Sharpen},
    {PostStage::DebugView, "debugview", "post pass", true, true, Mark::DebugViewEnd, Section::DebugView},
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
                       in.temporal || in.lumaSharpen || in.air || in.ssrTags || in.combatMask || in.camera || in.fog;
    if (!asked)
        return plan;

    plan.runs |= Bit(PostStage::Resolve);
    // The reflections: the tags are read out (and their screen shares
    // measured for the gate) whenever the world wrote them and kept its depth;
    // the trace runs when the gate is on and no screen overlay covers the
    // world, or when one of its diagnostic views is selected.
    if (in.ssrTags && in.worldDepth)
    {
        plan.runs |= Bit(PostStage::TagReadout);
        const bool debugSsr = in.debugView == kDebugViewSsrTags || in.debugView == kDebugViewSsrReflection;
        if ((in.ssr && in.glow) || debugSsr)
            plan.runs |= Bit(PostStage::Ssr);
    }
    // Occlusion needs the world depth, or the surfaces' interior shares.
    plan.aoStrength = (in.worldDepth || in.surfaceInterior) ? in.ambientOcclusion : 0.0f;
    if (plan.aoStrength > 0.0f)
        plan.runs |= Bit(PostStage::Ao);
    // A screen overlay in the world pass turns the glow off.
    plan.bloomStrength = in.glow ? in.bloom : 0.0f;
    if (plan.bloomStrength > 0.0f)
        plan.runs |= Bit(PostStage::Bloom);
    // The air pass needs the world depth, and over a screen overlay its haze
    // would grey the overlay's black surround. The volumetric fog takes the
    // stage in its place.
    // The volumetric fog also runs under a screen overlay (a rifle's iron
    // sight, a gunner's optic): its apply leaves the overlay's black mask as
    // drawn; switching the mist off whenever the player aims would make it pop
    // away and let the player see through it. In a vehicle interior it runs on the
    // world depth saved before the interior cleared it (fogWorldDepth), so
    // the air is drawn in every view AI sight is cut by it.
    plan.fog = in.fog && (in.worldDepth || in.fogWorldDepth);
    const bool air = (in.air && in.glow && in.worldDepth) || plan.fog;
    if (air)
        plan.runs |= Bit(PostStage::Air);
    // The shafts are depth tested glow toward a sun on screen; the air pass's
    // beams and the fog's own sun light in the air replace them, and so does
    // the photoreal camera (its composite adds none: their radial march draws
    // a fan of streaks round a low sun).
    if (in.worldDepth && in.glow && in.sunShafts > 0.0f && in.shaftWeight > kShaftMinWeight &&
        !(air && (in.airBeams || plan.fog)) && !in.camera)
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
    plan.fog = plan.fog && plan.Runs(PostStage::Air);
    for (const PostStageInfo& s : kStages)
        if (s.readsLinearDepth && plan.Runs(s.id))
            plan.runs |= Bit(PostStage::LinearDepth);
    // The next frame's combat shadow mask traces through the linear depth (a
    // vehicle interior has no world depth to give it).
    if (in.combatMask && in.worldDepth)
        plan.runs |= Bit(PostStage::LinearDepth);
    return plan;
}

float ForwardFogFactor(float distance, float start, float invRange)
{
    return std::clamp(1.0f - (distance - start) * invRange, 0.0f, 1.0f);
}

ForwardFogRange ScaleForwardFogRange(float start, float invRange, float scale)
{
    ForwardFogRange r{start, invRange};
    if (!(invRange > 0.0f) || !(scale > 1.0f))
        return r;
    // 1 - (d - start') * inv' = scale * (1 - (d - start) * inv), with
    // inv' = inv * scale: start' = end - (end - start) / scale.
    const float end = start + 1.0f / invRange;
    r.invRange = invRange * scale;
    r.start = end - 1.0f / r.invRange;
    return r;
}

bool InteriorGlassDraw(bool blended, bool glassClass)
{
    return blended && glassClass;
}

} // namespace Poseidon::remaster
