#pragma once

// Keeps the remastered lighting's terrain horizon map in step with the current
// landscape (see Graphics/Remaster/TerrainHorizon.hpp). Called once per frame
// before the world draws: when the graphics backend wants a map and the
// landscape's heights changed, the heights are copied on the calling thread and
// the map is built on a worker thread; the finished map is handed to the engine
// on a later call, on the calling (render) thread.

namespace Poseidon
{
class Landscape;

void UpdateTerrainLighting(Landscape* land);
// Cancels and joins any build in flight. Called on world teardown, so no
// worker outlives the world (or runs on into static destruction).
void ShutdownTerrainLighting();

} // namespace Poseidon
