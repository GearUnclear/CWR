// ============================================================================
//  Guerrilla Mode ambient road traffic - view-distance perception gate
//  (issue #53).  The fixed spawn/despawn band breaks above ~2250 m view
//  distance (the whole band sits inside draw range); the perception gate
//  derives the band from the live object-cull distance instead.  Proven here
//  on full CWA data (Abel):
//    1. at view distance 5000 (objectsZ capped at 3000) the effective band
//       reported by gmTrafficPercept follows the cull: spawn floor beyond
//       3000 m, band ordered minSpawn <= radius <= despawnEdge;
//    2. THE FENCE: a live civ car watched from 2000 m - inside the cull,
//       in frustum, beyond the pre-#53 fixed despawn edge (1800 m) - stays
//       alive and tracked across several traffic passes (the old code
//       far-despawned it in plain view on the first pass),
//       and every natural "spawned" event fired during the watch was
//       imperceptible at that instant (beyond the effective floor,
//       terrain-hidden, out of the view cone, or an alibi curb spawn
//       already in the depart machinery);
//    3. at view distance 500 the config values carry the band as floors
//       (no collapse below minSpawnDist/radius/despawn edge), and a spawn
//       still lands inside the config band [300, 1500] of the player.
//  Determinism: gmTrafficForceSpawn bypasses the chance roll, the caps AND
//  the perception gate (the stage-1 decision: tests spawn deliberately
//  close); the fence therefore watches the DESPAWN side on the forced car
//  and audits whatever natural spawns the chance rolls produce.
// ============================================================================

triSimUntil { GM_LIB_READY }

gtVil = gmZoneIndex "Village"
triAssertGe [gtVil, 0]
gtPos = (gmZone gtVil) select 8
player setPos [gtPos select 0, (gtPos select 1) - 400, 0]
triSimFrames 5

// -- phase 1: max view distance - the whole config band is inside draw range --
triSetViewDistance 5000
triSimFrames 5

// effective band follows the cull (config values are floors, never caps)
gtB = gmTrafficPercept (getPos player)
triAssertEq [count gtB, 6]
triAssertEq [gtB select 0, 1]
triAssertGe [gtB select 3, 3000]
triAssertGe [gtB select 4, gtB select 3]
triAssertGe [gtB select 5, gtB select 4]

// forced spawn: the documented gate bypass still places close to the player
gtCar = gmTrafficForceSpawn ["civ", gtVil]
triAssertEq [(format ["%1", isNull gtCar]), "false"]

// record every later (natural) spawn WITH its perception verdict at the
// instant the event fires (handlers run at dispatch, same frame as the pass)
gtSpawnObs = []
gmTrafficOnEvent ["spawned", {gtSpawnObs = gtSpawnObs + [[(_this select 0) distance player, gmTrafficPercept (getPos (_this select 0)), (gmTrafficInfo (_this select 0)) select 3]]}]

// -- the fence: watch the car from 2000 m, facing it - inside the 3000 m
//    cull and in frustum, but beyond the pre-#53 fixed edge (1800 m).  The
//    perception gate must hold the far-despawn; the old code deleted the
//    car in plain view on the first pass. --------------------------------------
gtCp = getPos gtCar
player setPos [(gtCp select 0) - 2000, gtCp select 1, 0]
player setDir 90
triSimFrames 5
gtP = gmTrafficPercept (getPos gtCar)
triAssertEq [gtP select 2, 1]
triAssertGe [2999, player distance gtCar]
gtT0 = time
triSimUntil { time >= gtT0 + 12 }
// the fence is THE WATCHED CAR surviving (the old code far-despawned it in
// plain view on the first pass).  Deliberately NOT count gmEvtTrDespawned==0:
// natural chance-roll cars are live in this mission, and one pushed beyond
// the widened edge by the 2 km teleport may legally far-despawn out of the
// cone during the watch.
triAssertEq [(format ["%1", isNull gtCar]), "false"]
triAssertEq [(format ["%1", gtCar in gmTrafficVehicles]), "true"]

// every natural spawn during the watch was imperceptible at its instant:
// beyond the effective floor, terrain-hidden, out of the cone, or an alibi
// curb spawn already departing (state 7 = TSDeparting)
gtBad = 0
{ if ((((_x select 0) < ((_x select 1) select 3)) and (((_x select 1) select 1) == 0)) and ((((_x select 1) select 2) == 1) and ((_x select 2) != 7))) then {gtBad = gtBad + 1} } forEach gtSpawnObs
triAssertEq [gtBad, 0]

// -- phase 2: low view distance - the config values are floors, the band
//    never collapses below them, and spawning still works ---------------------
triSetViewDistance 500
triSimFrames 5
gtB2 = gmTrafficPercept (getPos player)
triAssertGe [gtB2 select 3, 300]
triAssertGe [1200, gtB2 select 3]
triAssertGe [gtB2 select 4, 1500]
triAssertGe [gtB2 select 5, 1800]

player setPos [gtPos select 0, (gtPos select 1) - 400, 0]
triSimFrames 5
gtCar2 = gmTrafficForceSpawn ["civ", gtVil]
triAssertEq [(format ["%1", isNull gtCar2]), "false"]
gtD = player distance gtCar2
triAssertGe [gtD, 300]
triAssertGe [1500, gtD]

triEndTest
