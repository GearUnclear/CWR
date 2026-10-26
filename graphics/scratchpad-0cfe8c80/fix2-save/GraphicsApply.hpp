#pragma once

// GraphicsConfig → live engine bridge.
//
// The Settings/ layer is allowed to know about engine globals
// (GScene + GEngine) here because this module is the boundary
// between "config value" and "running renderer".  Two callers:
//
//   - GameApplication::LoadAndApplyGraphicsConfig at boot.
//   - GraphicsPage's row handlers on every live row change.
//
// A Scene built after the boot apply takes the shadow and smoke tiers of
// the last applied config in Scene::LoadConfig (AppliedGraphicsConfig).
//
// Tier→engine-value mappings live in this TU so both callers see the
// same translation table; changing a mapping touches one place.

#include <Poseidon/UI/Settings/GraphicsConfig.hpp>

// Push every cfg field into the live engine state.  Idempotent — safe
// to call repeatedly, e.g. after every row change.  Reads GScene +
// GEngine + writes to gUserFpsCap (the FPS-cap global defined in
// engine/Poseidon/Core/Game/GameLoop.cpp).

namespace Poseidon
{
class Scene;

float TerrainGridForTier(GraphicsConfig::Tier tier);
void ApplyGraphicsConfigToEngine(const GraphicsConfig& cfg);

// The scene's whole share of cfg: terrain grid, object LOD bias, and the
// shadow and smoke tiers below.  ApplyGraphicsConfigToEngine calls it when
// GScene exists, which today means the Graphics page's live apply.
void ApplyGraphicsConfigToScene(const GraphicsConfig& cfg, Scene& scene);

// The draw-only part of the scene's share: object and vehicle shadows (the
// shadow tier) and smoke (the particles tier).  Scene::LoadConfig applies
// only this part.  The terrain grid stays out because it drives
// Landscape::SubdivideTerrain, which writes new heights (fractal relief)
// that SurfaceY reads: applied when a Scene is built, it would change the
// physical terrain of every single-player session and test run.  The
// object LOD bias stays out with it.  Both reach the scene through a live
// apply (the Graphics page), and the terrain grid also through its legacy
// profile key.
void ApplyGraphicsShadowsToScene(const GraphicsConfig& cfg, Scene& scene);

// The config ApplyGraphicsConfigToEngine last pushed, or null before the
// first push.  Boot applies graphics.cfg before the world and its Scene
// exist, so a Scene takes the shadow and smoke tiers from here when it
// loads its config: at construction (boot, mod re-mount) and on a profile
// switch.
const GraphicsConfig* AppliedGraphicsConfig();

} // namespace Poseidon
