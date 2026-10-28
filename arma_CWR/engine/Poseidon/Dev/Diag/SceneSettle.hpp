#pragma once

#include <algorithm>
#include <deque>

namespace Poseidon
{
namespace Dev
{

// Decides when a freshly framed view has finished loading, for scripted
// captures (triSettleScene). Fed once per frame with that frame's draw-call
// count, the scene's object LOD width and whether anything is still being
// built for the view (the remastered terrain horizon map, a frame that did not
// render). The view counts as settled once the last `window` frames were all
// idle, all had the same LOD width, and their draw counts spread by no more
// than `tolerance` (a fraction of the largest; 0 = exactly equal).
//
// A steady draw count alone does not prove the view is loaded when the object
// detail is adaptive: Scene::AdjustComplexity refuses to refine for five
// seconds after it last coarsened, so a scene can sit at a coarse LOD with a
// perfectly steady count for longer than any short window. Captures pin the
// detail first (triPinObjectDetail), which leaves only real loading to wait
// for.
class SceneSettle
{
  public:
    SceneSettle(int window, float tolerance) : _window(std::max(window, 1)), _tolerance(std::max(tolerance, 0.0f)) {}

    void Add(int draws, float lodInvWidth, bool busy)
    {
        _samples.push_back({draws, lodInvWidth, busy});
        if (static_cast<int>(_samples.size()) > _window)
            _samples.pop_front();
        ++_frames;
    }

    bool Settled() const
    {
        if (static_cast<int>(_samples.size()) < _window)
            return false;
        const float lod = _samples.back().lodInvWidth;
        for (const Sample& s : _samples)
        {
            if (s.busy || s.lodInvWidth != lod)
                return false;
        }
        const int hi = MaxDraws();
        return static_cast<float>(hi - MinDraws()) <= _tolerance * static_cast<float>(hi);
    }

    int Frames() const { return _frames; }
    // Draw-count range over the current window (0 before the first frame).
    int MinDraws() const
    {
        int v = _samples.empty() ? 0 : _samples.front().draws;
        for (const Sample& s : _samples)
            v = std::min(v, s.draws);
        return v;
    }
    int MaxDraws() const
    {
        int v = _samples.empty() ? 0 : _samples.front().draws;
        for (const Sample& s : _samples)
            v = std::max(v, s.draws);
        return v;
    }

  private:
    struct Sample
    {
        int draws;
        float lodInvWidth;
        bool busy;
    };
    int _window;
    float _tolerance;
    int _frames = 0;
    std::deque<Sample> _samples;
};

} // namespace Dev
} // namespace Poseidon
