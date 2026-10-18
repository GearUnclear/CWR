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
// True while the backend wants a horizon map and the build for the current
// landscape has not been handed over yet (triSettleScene waits for it).
bool TerrainLightingPending();
// A landscape object was destroyed (Object::SetDestroyed): its footprint
// leaves the canopy of the long tree shadows (Graphics/Remaster/Canopy.hpp) on
// the next UpdateTerrainLighting, or as soon as the canopy is handed over.
void TerrainCanopyObjectDestroyed(int objectId);

} // namespace Poseidon
