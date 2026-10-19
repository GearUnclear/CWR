// ============================================================================
//  Occupier forces, the HQ and its fall (OCCUPIER-FORCES.md 9.1; D13, D14,
//  D22, D24).  Boots the SHIPPED Guerrilla.Abel template on the full
//  Remaster with no START TOWN (a direct launch):
//    p1  the default camp start has a REAL HQ (gmHqState): established at
//        the Camp, epoch 0, the market's "HQ (Camp)" marker placed, the
//        recruit menu anchored on it; a cache load and a garaged jeep;
//    p2  an occupier patrol (not an HQ_ASSAULT force) stands on the HQ with
//        nobody home: the HQ meter does not move (D14);
//    p3  the meter decays with no assault units (gmHqForceCapture lever);
//        a funded HQ_ASSAULT operation (gmCounterAttackLaunchFunded) fills
//        it, and at 100 the HQ is DESTROYED: cache
//        emptied, the garaged jeep wrecked (damage 1) where it stood, the
//        ring gone, the marker removed, the hqEstablish objective re-armed,
//        hqLost fired with the Camp, no recruiting at the old spot;
//    p4  the player re-establishes in Houdan, an occupier-administered town
//        whose garrison stands: free (no debit, no move counted), a fresh
//        empty cache, a new marker; the town keeps its owner and its live
//        defenders, with no exemption (D24).
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { not (isNil "GM_MKT_READY") }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
hqT0 = time
triSimUntil { time > hqT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
gmEvtHqLost = []
gmZoneOnEvent ["hqLost", {gmEvtHqLost = gmEvtHqLost + [_this]}]

// ---- p1: the default camp start is a real HQ -------------------------------------
hqCamp = gmZoneIndex "Camp"
triAssertGe [hqCamp, 0]
triSimUntil { gmHqEstablished }
hqS = gmHqState
triAssertEq [(format ["%1", hqS select 0]), "true"]
triAssertEq [(format ["%1", hqS select 1]), "false"]
triAssertEq [(hqS select 2), 0]
triAssertEq [(hqS select 3), 0]
triAssertEq [(hqS select 4), hqCamp]
triAssertEq [(count (hqS select 5)), 3]
triAssertEq [gmHqZone, "Camp"]
triAssertEq [gmHqMoveCount, 0]
hqPos = gmHqPos
// the market marks it, the objective is done
triSimUntil { GM_MKT_HQMARK }
triAssertLt [((markerPos "gmHqMarker") distance hqPos), 5]
triSimUntil { (gmJournalObjectiveState "hqEstablish") == "DONE" }
// the recruit anchor is the HQ
player setPos [(hqPos select 0) + 3, (hqPos select 1) + 3, 0]
triSimUntil { gmRecruitActive }
triAssertLt [(gmCampPos distance hqPos), 1]
// a cache load and a garaged jeep
triSimUntil { not (isNull gmHqCache) }
hqCache = gmHqCache
hqCache addWeaponCargo ["AK47", 2]
hqCache addMagazineCargo ["AK47", 3]
triAssertEq [(count (weaponCargo hqCache)), 2]
hqJeep = "Jeep" createVehicle gmHqGaragePos
triAssert [not (isNull hqJeep)]
triAssert [gmGarageLock [hqJeep, true]]
triAssertEq [gmGarageCount, 1]
hqJeepPos = getPos hqJeep

// ---- p2: a patrol on the HQ with nobody home does nothing to the meter ----------
player setPos [(hqPos select 0) + 450, (hqPos select 1) + 250, 0]
triSimUntil { not gmRecruitActive }
hqOcc = gmOccupierSide call GM_fnSideFromString
hqSq = [hqOcc, gmFactionSquad [gmOccupierSide, 1, 4], hqPos] call GM_fnSpawnSquad
hqSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units hqSq)
doStop (units hqSq)
triAssertEq [(count units hqSq), 4]
hqT1 = time
triSimUntil { time > hqT1 + 12 }
triAssertLt [((leader hqSq) distance hqPos), 150]
triAssertEq [((gmHqState) select 3), 0]
triAssertEq [(format ["%1", gmHqEstablished]), "true"]
{deleteVehicle _x} forEach (units hqSq)

// ---- p3: the meter; no assault units = decay; an HQ_ASSAULT force fills it ------
gmHqForceCapture 50
triAssertEq [((gmHqState) select 3), 50]
triSimUntil { ((gmHqState) select 3) < 50 }
triAssertGt [((gmHqState) select 3), 40]
gmHqForceCapture 0
// the deliberate assault (D14): a funded HQ_ASSAULT operation from the
// Harbour (its garrison is banked, so it can spare men; the Outpost's
// stands live beside the Camp and cannot), target the HQ zone (the
// destination is HqPos); the player waits by the Airfield, 4 km north and
// far off the Harbour's road to the Camp (that road passes west of the Camp,
// near Houdan, and a route point inside the player's bubble would birth the
// force live on the road), so the force arrives abstractly and holds at the
// HQ (test speed lever)
hqAf = gmZoneIndex "Airfield"
triAssertGe [hqAf, 0]
hqAfPos = (gmZone hqAf) select 8
player setPos [(hqAfPos select 0), (hqAfPos select 1) - 300, 0]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 30]]), "true"]
hqOp = gmCounterAttackLaunchFunded [hqCamp, "Le Port Harbour", "foot", 4, "HQ_ASSAULT"]
triAssertGt [hqOp, 0]
triSimUntil { ((gmHqState) select 3) > 0 }
triAssertEq [(format ["%1", (gmHqState) select 0]), "true"]
triSimUntil { not gmHqEstablished }
hqS = gmHqState
triAssertEq [(format ["%1", hqS select 0]), "false"]
triAssertEq [(format ["%1", hqS select 1]), "true"]
triAssertEq [(hqS select 2), 1]
triAssertEq [(hqS select 3), 0]
triAssertEq [(hqS select 4), -1]
triAssertEq [(count (hqS select 5)), 0]
triAssertEq [gmHqEpoch, 1]
triAssertEq [(format ["%1", gmHqEstablished]), "false"]
// the cache: emptied at once (the holder goes once nobody can see it)
triAssert [isNull gmHqCache]
hqCacheW = 0
if (not (isNull hqCache)) then {hqCacheW = count (weaponCargo hqCache)}
triAssertEq [hqCacheW, 0]
hqCacheM = 0
if (not (isNull hqCache)) then {hqCacheM = count (magazineCargo hqCache)}
triAssertEq [hqCacheM, 0]
// the garage (D22): wrecked where it stood, the ring gone
triAssertEq [gmGarageCount, 0]
triSimUntil { (damage hqJeep) >= 1 }
triAssertLt [((getPos hqJeep) distance hqJeepPos), 5]
// the market: marker removed, objective re-armed (no new messaging)
triSimUntil { not GM_MKT_HQMARK }
triAssertEq [((markerPos "gmHqMarker") select 0), 0]
triSimUntil { (gmJournalObjectiveState "hqEstablish") == "ACTIVE" }
// the zone event, with the zone the HQ stood in
triSimUntil { (count gmEvtHqLost) > 0 }
triAssertEq [((gmEvtHqLost select 0) select 0), hqCamp]
triAssertEq [((gmEvtHqLost select 0) select 1), "Camp"]
// the Camp itself keeps its owner (the zone around the HQ is unchanged)
triAssertEq [((gmZone hqCamp) select 2), gmResistanceSide]
// no HQ, no recruiting: not even at the old spot
player setPos [(hqPos select 0) + 3, (hqPos select 1) + 3, 0]
hqT2 = time
triSimUntil { time > hqT2 + 3 }
triAssertEq [(format ["%1", gmRecruitActive]), "false"]
// and the Camp does not become the HQ again by itself
triAssertEq [(format ["%1", gmHqEstablished]), "false"]

// ---- p4: re-establish in an enemy town; its garrison still defends it ----------
hqV = gmZoneIndex "Village"
triAssertGe [hqV, 0]
hqVPos = (gmZone hqV) select 8
player setPos [(hqVPos select 0) + 2, (hqVPos select 1) + 2, 0]
triSimUntil { gmGarrisonSpawned hqV }
triSimUntil { (gmGarrisonLive hqV) >= 4 }
{_x setCombatMode "BLUE"} forEach (gmGarrisonGroups hqV)
hqLive = gmGarrisonLive hqV
hqOwner = (gmZone hqV) select 2
triAssert [hqOwner != gmResistanceSide]
triAssertEq [(format ["%1", (gmZoneSim hqV) select 10]), "false"]
// the existing establish action: a first establishment, so it reads
// "Establish" and costs nothing
gmResources = 500
triSimUntil { gmMktHqActive }
hqR0 = gmResources
[player, player, gmMktHqAct] exec "\gmcore\scripts\market_action.sqs"
triSimUntil { gmHqEstablished }
triAssertEq [gmResources, hqR0]
triAssertEq [gmHqMoveCount, 0]
hqS = gmHqState
triAssertEq [(format ["%1", hqS select 0]), "true"]
triAssertEq [(format ["%1", hqS select 1]), "false"]
triAssertEq [(hqS select 2), 1]
triAssertEq [(hqS select 4), hqV]
triAssertEq [gmHqZone, "Village"]
// a fresh, empty cache and a fresh ring
triSimUntil { not (isNull gmHqCache) }
triAssert [gmHqCache != hqCache]
triAssertEq [(count (weaponCargo gmHqCache)), 0]
triAssertEq [gmGarageCount, 0]
// the market re-marks it and closes the objective again
triSimUntil { GM_MKT_HQMARK }
triAssertLt [((markerPos "gmHqMarker") distance gmHqPos), 5]
triSimUntil { (gmJournalObjectiveState "hqEstablish") == "DONE" }
// D24: the town keeps its owner and its defenders, no exemption
hqT3 = time
triSimUntil { time > hqT3 + 7 }
triAssertEq [((gmZone hqV) select 2), hqOwner]
triAssertEq [(format ["%1", (gmZoneSim hqV) select 10]), "false"]
triAssertEq [(gmGarrisonLive hqV), hqLive]
triAssertEq [(format ["%1", gmGarrisonSpawned hqV]), "true"]

triEndTest
