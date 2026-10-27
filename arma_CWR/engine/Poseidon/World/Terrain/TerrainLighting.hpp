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
namespace remaster
{
struct FogInputs;
}

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

// AI sight's air (Graphics/Remaster/VolumetricFog.hpp, AI sight), a part of
// the simulation: called once per simulation step after the weather and the
// clock moved (World::SimulateLandscape). When the landscape's heights change
// (an island loads) the sight map, with the island's aridity from its ground
// colours, is built at once on the calling thread, whatever the render
// settings; every step publishes a snapshot (MakeFogSightSnapshot) from
// WorldFogInputs, the shipped knobs and FogSightClassic. Deterministic: every
// machine follows the same synced weather and date and the same island.
void SimulateFogSight(Landscape* land);
// The simulating machine's Lighting row is Classic (the Options value last
// applied): AI sight then takes the classic tactical fog only. False on a
// machine that never applied one (a dedicated server, a headless client).
bool FogSightClassic();
// Clears AI sight (classic) and drops the map. Called on world teardown.
void ShutdownFogSight();
// The fog's inputs from the world: the sun, the weather, the classic fog's
// range, the view distance, the date and the island's aridity (AI sight's
// map's, 0 before one is built). The renderer's fog fills its medium from the
// same function, so the drawn air and AI sight cannot drift apart.
remaster::FogInputs WorldFogInputs(const Landscape* land);

} // namespace Poseidon
