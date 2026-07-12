// ============================================================================
//  Guerrilla Mode - consolidation capture on the NATIVE engine surface.
//    Demo-data twin of guerrilla_native_capture.test.sqf (full-CWA/Abel); the
//    capture decision is ZoneRegistry::EvaluateTick. A military zone flips
//    only after a sustained hold (the capture meter, gmZone element 9), and
//    ANY live occupier unit inside zoneArea freezes progress. Proves
//    (captureRate=20 via description.ext):
//      1. pre-state: the Outpost is occupier-owned, its marker painted RED
//         natively (revealed: the resistance Camp is within revealRadius);
//      2. clearing the garrison + a resistance unit inside zoneArea starts
//         the meter WITHOUT flipping the zone (the non-instant pin);
//      3. one live occupier spawned inside the zone FREEZES the meter and
//         blocks the flip (the capture-with-enemies-present fix);
//      4. removing him resumes securing; at meter 100 the owner flips and
//         the marker recolors GREEN natively;
//      5. the scripted reaction ran: capture.sqs consumed the native
//         "captured" event and spawned the hold garrison (GM_cGrp, 3 men).
//
//  Runs against the REAL mode: guerrilla_capture.Demo's init.sqs + scripts/
//  are byte-identical to the canonical Guerrilla.Demo core (enforced by
//  test_mission_script_core.cpp), so this exercises the shipped scripts.
// ============================================================================

triSimUntil { GM_LIB_READY }
gcOut = gmZoneIndex "Outpost"
triAssertGe [gcOut, 0]

// -- pre-state: occupier-owned, revealed -> native marker repaint painted RED -
triSimUntil { (getMarkerColor "gmZoneMarker_2") == "ColorRed" }
triAssertEq [((gmZone gcOut) select 2), gmOccupierSide]

// -- the garrison must be cached in before we can kill it (the player boots
//    at the Camp, ~290 m from the Outpost - inside the 800 m cacheRadius) ----
triSimUntil { gmGarrisonSpawned gcOut }
triSimUntil { (gmGarrisonLive gcOut) >= 8 }

// -- CLEAR the garrison: delete every live cached occupier (spawned stays
//    true, reserve is already 0, so nothing can respawn mid-sequence), then
//    force the despawn for the authoritative survivor write-back (0). --------
gcUnits = []
"gcUnits = gcUnits + (units _x)" forEach (gmGarrisonGroups gcOut)
{deleteVehicle _x} forEach gcUnits
gmGarrisonForceDespawn gcOut
gmZoneSet [gcOut, "garrison", 0]

// -- stand the player (a live resistance unit) inside the zone area; shield
//    him - the staged contest below spawns a live enemy at his feet ----------
player allowDammage false
gcPos = (gmZone gcOut) select 8
player setPos [gcPos select 0, gcPos select 1, 0]

// belt-and-suspenders against a cache tick that was mid-flight on the sim
// thread when the deletes landed
triSimFrames 10
gmGarrisonForceDespawn gcOut
gmZoneSet [gcOut, "garrison", 0]

// -- NON-INSTANT: the meter starts climbing on the zone ticks, but the zone
//    does NOT flip the moment it reads empty ----------------------------------
triSimUntil { ((gmZone gcOut) select 9) > 0 }
triAssertEq [((gmZone gcOut) select 2), gmOccupierSide]

// -- CONTEST: one live occupier inside zoneArea freezes the meter. Spawned
//    into the occupier center via the shipped lib helper; BLUE = never fires,
//    presence is all the engine reads. ---------------------------------------
gcOccGrp = [gmOccupierSide call GM_fnSideFromString, gmFactionTierClass [gmOccupierSide, 1], 1, gcPos] call GM_fnSpawnGroup
gcOccGrp setCombatMode "BLUE"
// let the first post-spawn zone tick land, then hold across two more ticks
gcT0 = time
triSimUntil { time > gcT0 + 4 }
gcCap1 = (gmZone gcOut) select 9
gcT0 = time
triSimUntil { time > gcT0 + 7 }
triAssertEq [((gmZone gcOut) select 9), gcCap1]
triAssertEq [((gmZone gcOut) select 2), gmOccupierSide]

// -- clear the contest -> securing resumes -> THE FLIP at meter 100 -----------
{deleteVehicle _x} forEach (units gcOccGrp)
triSimUntil { ((gmZone gcOut) select 2) == gmResistanceSide }
player allowDammage true

// -- native marker recolor on the capture path --------------------------------
triSimUntil { (getMarkerColor "gmZoneMarker_2") == "ColorGreen" }

// -- native income tap stayed open + heat spiked on capture -------------------
triAssertGe [((gmZone gcOut) select 5), 25]
triAssertGe [((gmZone gcOut) select 6), 20]

// -- the meter reset on the flip ----------------------------------------------
triAssertEq [((gmZone gcOut) select 9), 0]

// -- scripted REACTION: capture.sqs spawned the hold garrison -----------------
triSimUntil { not (isNil "GM_cGrp") }
triAssertEq [(count units GM_cGrp), 3]

triEndTest
