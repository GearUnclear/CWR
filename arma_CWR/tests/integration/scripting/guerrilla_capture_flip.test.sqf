// ============================================================================
//  Guerrilla Mode - capture-flip integration test (Phase-1 MVP core loop).
//    Clear the EAST Outpost garrison + stand a friendly (the player) inside the
//    zone, then assert zones.sqs flips GM_Z_OWNER to "GUER" AND recolors the
//    zone marker (red -> green).
//
//  Runs against the REAL mode: guerrilla_capture.Demo bootstraps init.sqs +
//  every manager, so this exercises the shipped zones.sqs capture path (not a
//  reimplementation). Outpost is zone index 2 (seed order Camp,Village,Outpost).
//
//  Determinism: the garrison is *cleared* by deleting the live occupier bodies
//  and zeroing the cached reserve GM_Z_GAR - the exact state "you killed the
//  garrison" produces - rather than relying on an AI firefight. All commands are
//  confirmed (setPos :1238, deleteVehicle :1028, units :979, getMarkerColor
//  :955, count/select/set/forEach). GM_Z_* index constants come from init.sqs.
//  gc* globals persist across the per-statement harness evaluation.
// ============================================================================

// -- mode must be fully booted (init.sqs published the helper table) ----------
triSimUntil { GM_LIB_READY }
triAssertEq [(count GM_ZONES), 3]

// -- zones.sqs is live once it has created + colored the Outpost marker. The
//    Outpost is within reveal range of the GUER Camp, so it paints red (revealed
//    EAST) on the first tick. This also asserts the pre-capture owner state.
triSimUntil { (getMarkerColor "gmZoneMarker_2") == "ColorRed" }
triAssertEq [((GM_ZONES select 2) select GM_Z_OWNER), "EAST"]

// -- stand the player inside the Outpost (ground z=0; GM_Z_POS is getPos order) -
gcPos = [((GM_ZONES select 2) select GM_Z_POS) select 0, ((GM_ZONES select 2) select GM_Z_POS) select 1, 0]
player setPos gcPos

// -- CLEAR the garrison: zero the reserve first (so spawning.sqs cannot re-spawn:
//    its spawn gate needs GM_Z_GAR>0), then delete every live cached occupier and
//    empty the live-group cache. This is the "garrison wiped out" state. -------
(GM_ZONES select 2) set [GM_Z_GAR, 0]
gcUnits = []
"gcUnits = gcUnits + units _x" forEach GM_CACHE_GROUPS
{deleteVehicle _x} forEach gcUnits
GM_CACHE_GROUPS = []
GM_CACHE_ZONEIDX = []

// belt-and-suspenders against a spawn that was mid-flight on the sim thread
triSimFrames 10
(GM_ZONES select 2) set [GM_Z_GAR, 0]
gcUnits2 = []
"gcUnits2 = gcUnits2 + units _x" forEach GM_CACHE_GROUPS
{deleteVehicle _x} forEach gcUnits2
GM_CACHE_GROUPS = []
GM_CACHE_ZONEIDX = []

// -- ASSERT the flip: zones.sqs (~3s tick) sees no live occupiers, no cached
//    reserve, and a friendly in the area -> GM_Z_OWNER becomes "GUER". ---------
triSimUntil { ((GM_ZONES select 2) select GM_Z_OWNER) == "GUER" }

// -- ASSERT the marker recolored to the GUER color on the same capture path ---
triSimFrames 10
triAssertEq [(getMarkerColor "gmZoneMarker_2"), "ColorGreen"]

triEndTest
