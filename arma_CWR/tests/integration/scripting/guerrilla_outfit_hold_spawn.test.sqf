// ============================================================================
//  Guerrilla Mode - CIVILIAN-outfit hold squad (issue #25, M2.3). Proves the
//  capture.sqs civilian branch on the native Abel fixture: with the civilian
//  outfit active, a captured military zone's hold garrison BYPASSES the
//  role-mixed native composer (gmFactionSquad) and spawns the holdClassCiv
//  monoculture through the tier-less path (capture.sqs "GM_HOLD_CIV != """
//  branch).
//
//  SCOPE SPLIT, stated honestly: the engine's capture DECISION and its
//  captured-event production (flip -> native handler -> gmEvtCaptured) are
//  pinned end to end by guerrilla_native_capture, whose garrison-clear +
//  secure-the-zone arc is timing-sensitive (live-fire AI spotting variance).
//  This test owns the CONSUMER side only, so it stays deterministic: it
//  clears the garrison (quiet zone), flips the owner through gmZoneSet (the
//  same script-surface write the save/reload seq uses), then appends the
//  captured event to gmEvtCaptured exactly as the native handler does
//  ([zoneIndex, zoneName, ownerString]); capture.sqs drain-swaps the queue
//  and reacts for real.
//
//  OUTFIT ACTIVATION, stated honestly too: this is a direct --test-mission
//  launch, so gmSelOutfit is nil and the managers boot warrior (GM_HOLD_CIV
//  = ""). The menu-time publish -> init.sqs fold -> manager fold chain is
//  pinned by ui/guerrilla_outfit_civilian_e2e (which asserts GM_HOLD_CIV ==
//  "SoldierGFakeC" after a real CIVILIAN launch). Here the test re-folds the
//  ONE tunable capture.sqs reads at event time - GM_HOLD_CIV - through the
//  SAME descriptor read the boot fold performs (gmFactionValue holdClassCiv,
//  authored in this fixture's description.ext), so the class name still
//  flows descriptor -> engine -> script, never from a test literal.
// ============================================================================

triSimUntil { GM_LIB_READY }

// -- capture.sqs boot block done (GM_cNextTT is its LAST boot write) ----------
triSimUntil { not (isNil "GM_cNextTT") }

// -- direct launch booted warrior; re-fold the event-time tunable through the
//    fixture descriptor (see header) -----------------------------------------
triAssertEq [(format ["%1", GM_OUTFIT_CIV]), "false"]
triAssertEq [GM_HOLD_CIV, ""]
GM_HOLD_CIV = gmFactionValue [gmResistanceSide, "holdClassCiv"]
triAssertEq [GM_HOLD_CIV, "SoldierGFakeC"]

// -- control: the warrior path would NOT have produced this monoculture - the
//    descriptor authors tiers[], so gmFactionSquad is non-empty and without
//    the bypass the hold squad would have been its role-mixed SoldierGB set --
ghSquad = gmFactionSquad [gmResistanceSide, gmWarLevel, 3]
triAssertGe [(count ghSquad), 1]

// -- quiet the zone: delete the cached garrison (no live-fire variance), then
//    flip ownership through the script surface -------------------------------
ghOut = gmZoneIndex "Outpost"
triAssertGe [ghOut, 0]
triSimUntil { gmGarrisonSpawned ghOut }
triSimUntil { (gmGarrisonLive ghOut) >= 6 }
ghUnits = []
"ghUnits = ghUnits + (units _x)" forEach (gmGarrisonGroups ghOut)
{deleteVehicle _x} forEach ghUnits
gmGarrisonForceDespawn ghOut
gmZoneSet [ghOut, "garrison", 0]
// belt-and-suspenders against a cache tick mid-flight when the deletes landed
triSimFrames 10
gmGarrisonForceDespawn ghOut
gmZoneSet [ghOut, "garrison", 0]
gmZoneSet [ghOut, "owner", gmResistanceSide]
triAssertEq [((gmZone ghOut) select 2), gmResistanceSide]

// -- inject the captured event exactly as the native one-line handler does ----
gmEvtCaptured = gmEvtCaptured + [[ghOut, "Outpost", gmResistanceSide]]

// -- the scripted reaction took the CIVILIAN branch: holdCount x holdClassCiv,
//    every body the plainclothes class (monoculture, NOT the role mix) --------
triSimUntil { not (isNil "GM_cGrp") }
triAssertEq [(count units GM_cGrp), 3]
ghClasses = []
"ghClasses = ghClasses + [typeOf _x]" forEach (units GM_cGrp)
triAssertEq [(format ["%1", ghClasses]), (format ["%1", ["SoldierGFakeC", "SoldierGFakeC", "SoldierGFakeC"]])]
// side weld: civilian-class bodies spawned into the resistance fight as GUER
triAssertEq [(format ["%1", side (leader GM_cGrp)]), "GUER"]

triEndTest
