// ============================================================================
//  Guerrilla Mode - capture-trigger flip on the NATIVE engine surface.
//    Demo-data twin of guerrilla_native_capture.test.sqf (full-CWA/Abel); the
//    retired zones.sqs capture poll is ZoneRegistry::EvaluateTick now. Proves:
//      1. pre-state: the Outpost is occupier-owned, its marker painted RED
//         natively (revealed: the resistance Camp is within revealRadius);
//      2. clearing the garrison the engine-legit way (delete the live cached
//         bodies, force-despawn for the survivor write-back, zero the
//         reserve) + a resistance unit inside zoneArea flips the owner to
//         gmResistanceSide on the next native zone tick;
//      3. the marker recolors GREEN natively;
//      4. the scripted reaction ran: capture.sqs consumed the native
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

// -- stand the player (a live resistance unit) inside the zone area -----------
gcPos = (gmZone gcOut) select 8
player setPos [gcPos select 0, gcPos select 1, 0]

// belt-and-suspenders against a cache tick that was mid-flight on the sim
// thread when the deletes landed
triSimFrames 10
gmGarrisonForceDespawn gcOut
gmZoneSet [gcOut, "garrison", 0]

// -- THE FLIP: native zone tick sees liveOccupiers<1 && garrison<1 && a
//    resistance unit in zoneArea -> owner becomes the resistance side --------
triSimUntil { ((gmZone gcOut) select 2) == gmResistanceSide }

// -- native marker recolor on the capture path --------------------------------
triSimUntil { (getMarkerColor "gmZoneMarker_2") == "ColorGreen" }

// -- native income tap stayed open + heat spiked on capture -------------------
triAssertGe [((gmZone gcOut) select 5), 25]
triAssertGe [((gmZone gcOut) select 6), 20]

// -- scripted REACTION: capture.sqs spawned the hold garrison -----------------
triSimUntil { not (isNil "GM_cGrp") }
triAssertEq [(count units GM_cGrp), 3]

triEndTest
