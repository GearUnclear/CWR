// ============================================================================
//  Phase 1 of the NATIVE save/reload round-trip (guerrilla_native.abel).
//    Boot the full mode, stamp DISTINCTIVE sentinels into BOTH persistence
//    layers - script globals (GGameState) and native zone state (ZoneRegistry,
//    written through the gmZoneSet command surface) - then triSaveGame to the
//    shared user dir. Phase 2 fresh-boots, reloads and diffs everything,
//    including the natively-serialized campaignLoaded event handler.
//
//  The zone-owner sentinel goes through gmZoneSet ["owner"], the exact write
//  the mission scripts use; the native serializer must carry it by NAME across
//  the config rebuild a load performs.
// ============================================================================

triSimUntil { GM_LIB_READY }
gsOut = gmZoneIndex "Outpost"
gsVil = gmZoneIndex "Village"
triAssertGe [gsOut, 0]
triAssertGe [gsVil, 0]
triAssertEq [((gmZone gsOut) select 2), gmOccupierSide]

// -- the companion (Petra) must have a live body before the snapshot: phase 2
//    asserts the post-load reconciliation rebuilds/keeps her alive ------------
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triAssertEq [(format ["%1", alive (GM_COMP_OBJ select 0)]), "true"]

// -- stamp sentinels: script-global layer --------------------------------------
gmResources = 777
gmManpower = 9
gmWarLevel = 5
GM_COMP_XP set [0, 12345]
GM_GEAR_UNLOCKED = ["AK47"]

// -- stamp sentinels: native field journal (Journal::Serialize, the map's
//    Notes/Plan pages). The boot scripts already wrote the opening diary
//    line and the starter objectives; add a distinctive entry + an objective
//    flip + a status line through the command surface the managers use -----
gsJ0 = gmJournalCount
triAssertGe [gsJ0, 1]
gmJournalLog "GNAT sentinel diary line"
gmJournalObjective ["firstRecruit", "", "DONE"]
gmJournalStatus ["GnatStatus", "stamped before save"]
triAssertEq [gmJournalCount, gsJ0 + 1]
triAssertEq [((gmJournalEntry gsJ0) select 1), "GNAT sentinel diary line"]
triAssertEq [(gmJournalObjectiveState "firstRecruit"), "DONE"]

// -- stamp sentinels: native zone-registry layer -------------------------------
gmZoneSet [gsVil, "support", 55]
gmZoneSet [gsOut, "income", 99]
// owner FIRST: an owner write resets the capture meter (ownership
// discontinuity), so the capture sentinel must land after it
gmZoneSet [gsOut, "owner", gmResistanceSide]
gmZoneSet [gsOut, "capture", 50]

// -- the civilian layer must be up and fully seeded before we stamp its
//    ledgers (gmCivTicks/gmShkTicks are the LAST state seeds of each manager) -
triSimUntil { not (isNil "GM_CIV_READY") }
triSimUntil { not (isNil "gmCivTicks") }
triSimUntil { not (isNil "gmShkTicks") }

// -- stamp sentinels: civilian-layer ledgers (issue #8) ------------------------
//    Names + times only (the binding persistence policy). The kill-queue
//    record carries NULL handles ON PURPOSE - the load-degraded case the
//    consumer must absorb. Its pos sits far outside GM_CIV_EFFECT_R of the
//    Village, so the post-load drain takes the effect-radius DROP path and
//    cannot disturb the support sentinel (55) phase 2 diffs.
GM_PANIC_ZONES = ["Village"]
GM_PANIC_UNTIL = [time + 900]
GM_RESENT_ZONES = ["Village"]
GM_RESENT_DUE = [time + 900]
GM_RESENT_AMT = [7]
gmDayCount = 3
gmCivKilled = [[objNull, objNull, [500, 500, 0], gsVil]]
gsCivT = gmCivTicks

// -- ambient traffic: force a civilian car from the Village so a live traffic
//    row (hull + group + driver refs, zone names) rides the GuerrillaTraffic
//    save block; the OBJECT handle itself rides GGameState -----------------------
gsCar = gmTrafficForceSpawn ["civ", gsVil]
triAssertEq [(format ["%1", isNull gsCar]), "false"]
triAssertGe [gmTrafficCount "civ", 1]
triAssertEq [((gmTrafficInfo gsCar) select 1), gsVil]

// -- write the binary save into the shared UserDir/Saved/Tmp/gnat.fps ---------
triAssertEq [(triSaveGame "gnat"), "OK"]

triEndTest
