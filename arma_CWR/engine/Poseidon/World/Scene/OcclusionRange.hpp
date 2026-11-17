#pragma once

#include <cstdlib>

namespace Poseidon::OcclusionRange
{

// W3-1(b): a range for the object occlusion test (Scene::DrawObjectsAndShadowsPass1).
//
// The pass walks the occludable objects front to back. Each object far enough
// from the camera is tested against the occlusion buffer (Object::AnimatedMinMax
// then Occlusion::TestBBox); a hidden one is not drawn, a visible one that is a
// big enough occluder is rendered into the buffer. At long views the test runs
// on 4,000 to 8,000 objects a frame and hides 6 to 16 of them (M2 3b), so most
// of the pass's 2 to 5 ms buys nothing. Beyond the range an object is no longer
// tested and stays drawn, with two exceptions that keep everything else in the
// frame exactly as before:
//
// - An object that would render into the buffer when it is not hidden (an
//   occluder: CanOcclude, OcclusionView, a view geometry and the area test) keeps
//   its test at any distance. Its verdict decides whether it renders, and the
//   buffer feeds the later tests, the sun and light flares (Scene::DrawFlares) and
//   the projected shadows (Scene::DrawExShadow). Keeping every occluder's test
//   keeps the buffer, and so all of those, identical.
// - The range is never shorter than the shadow caster reach (TestRange2's
//   keepWithin). The shadow-map pass casts a drawn object from its draw LOD and a
//   hidden one from its shadow LOD; inside the caster reach an untested hidden
//   object would cast from a different LOD. Beyond it nothing casts.
//
// So the only change is that a far, non-occluder object the test would have
// hidden is drawn: it lies behind occluders the buffer already holds and the
// depth test removes it. It never hides an object that was drawn before.

// The range M2 measured (EXP-L10, with a cruder cut that also dropped the far
// occluders): it kept the town's culls (923 draws against 888 with the full
// test) and cut the pass's cost at the valley and Eden from about 4 ms to about
// 1 ms. Keeping the far occluders' tests culls at least as much as that cut.
constexpr float kDefaultRange = 600.0f;

// POSEIDON_OCC_RANGE (metres), read once by the scene. Unset, empty or not a
// number: kDefaultRange. Zero or negative: no range (every object is tested, as
// before W3-1).
inline float ParseRange(const char* text)
{
    if (!text || !*text)
    {
        return kDefaultRange;
    }
    char* end = nullptr;
    const float v = std::strtof(text, &end);
    if (end == text || !(v == v))
    {
        return kDefaultRange;
    }
    return v > 0.0f ? v : 0.0f;
}

// The squared test range for a frame: `range` raised to `keepWithin` (the shadow
// caster reach). 0 means no range: every object is tested.
inline float TestRange2(float range, float keepWithin)
{
    if (!(range > 0.0f))
    {
        return 0.0f;
    }
    const float r = range > keepWithin ? range : keepWithin;
    return r * r;
}

// Whether an occludable object far enough to be tested skips its test.
// `distance2` is the same SortObject::distance2 the pass sorts by.
inline bool SkipTest(float distance2, float range2, bool rendersAsOccluder)
{
    return range2 > 0.0f && distance2 >= range2 && !rendersAsOccluder;
}

// Per-pass counters (POSEIDON_OCC_STATS=1 logs their averages). INFRA-C1's
// PerfCounters fields occTested and occCulled are `tested` and `culled`.
struct Stats
{
    int tested = 0;    // objects tested against the buffer
    int culled = 0;    // of those, hidden (not drawn)
    int skipped = 0;   // beyond the range, not tested (drawn)
    int farTested = 0; // beyond the range but tested, because they are occluders
};

} // namespace Poseidon::OcclusionRange
