// ============================================================================
//  Guerrilla Mode NATIVE parity - capture-trigger flip (issue #3 item 5).
//    The retired zones.sqs capture poll is engine code now
//    (ZoneRegistry::EvaluateTick). This proves, on full CWA data (Abel):
//      1. pre-state: the Outpost is occupier-owned and its marker painted RED
//         natively (revealed: the resistance Camp is within revealRadius);
//      2. clearing the garrison the engine-legit way (kill/delete the live
//         cached bodies, zero the reserve) + a live resistance unit inside
//         zoneArea flips owner to gmResistanceSide on the next zone tick;
//      3. the marker recolors GREEN natively on the same tick;
//      4. the mission's scripted captured-event reaction ran: capture.sqs
//         consumed the native "captured" event and spawned the hold garrison
//         (GM_cGrp, holdCount=3 x holdClass from the GUER descriptor).
//
//  Determinism: the live occupier bodies are deleted while the cache stays
//  spawned (no respawn path: the reserve moved into the groups at spawn and
//  is 0), then gmGarrisonForceDespawn writes the honest survivor count (0)
//  back and stands the cache down - after which reserve<1 blocks any respawn.
//  This is exactly the state "you wiped out the garrison" produces.
// ============================================================================

triSimUntil { GM_LIB_READY }
gcOut = gmZoneIndex "Outpost"
triAssertGe [gcOut, 0]

// -- pre-state: occupier-owned, revealed -> native marker repaint painted RED -
triSimUntil { (getMarkerColor "gmZoneMarker_2") == "ColorRed" }
triAssertEq [((gmZone gcOut) select 2), gmOccupierSide]

// -- the garrison must be cached in before we can kill it ---------------------
triSimUntil { gmGarrisonSpawned gcOut }
triSimUntil { (gmGarrisonLive gcOut) >= 6 }

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

// -- native income tap stayed open (config income 25 >= defaultIncome path) ---
triAssertGe [((gmZone gcOut) select 5), 25]

// -- native heat spike on capture (heatCapSpike 40; GREEN decay is 1 per 10s) -
triAssertGe [((gmZone gcOut) select 6), 20]

// -- scripted REACTION: capture.sqs drained the native captured event and
//    spawned the hold garrison (3 x SoldierGB at the zone) -------------------
triSimUntil { not (isNil "GM_cGrp") }
triAssertEq [(count units GM_cGrp), 3]

triEndTest
