#pragma once

#include "Frame.hpp"

#include <vector>

namespace Poseidon
{
class Engine;
namespace render::frame
{

void ObserveRenderedFrame(Engine& engine, Scene& scene);

// Whether the next ObserveRenderedFrame call observes its frame (every frame
// under the tri harness, 1 in 16 in play). The backend records its per-draw
// DrawItems for the observer only while this is true (S3-28): on the other
// frames nothing reads them. True until the first observation.
bool ObserverRecordsThisFrame();

// Pass shape of the most recently observed frame — kind + draw count
// per emitted pass, in emission order.  Read by the tri harness
// (`triGetFrameShape` / `triAssertFrameShape`) to pin the pipeline's
// pass structure in integration tests.
struct ObservedPass
{
    FramePassKind kind = FramePassKind::WorldOpaque;
    int draws = 0;
};
const std::vector<ObservedPass>& LastObservedFrameShape();

} // namespace render::frame

} // namespace Poseidon
