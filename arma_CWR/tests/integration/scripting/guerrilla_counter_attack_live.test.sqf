// ============================================================================
//  Occupier counter-attack, LIVE path (issue #79 B2 follow-up; native
//  CounterAttack service, guerrilla-mode/OFFSCREEN-CONTRACT.md decision C).
//  Boots the SHIPPED Guerrilla.Abel template on the full Remaster and works
//  Houdan (the "Village" CITY zone, ~490 m from the Camp, inside the 800 m
//  cacheRadius) with the player standing at the Camp:
//    l1  kill Houdan's garrison and raise it with one fighter; the flip
//        schedules exactly one counter-attack (first town, k = 0; its 0.4
//        roll is tuned to 1 here so the answer is certain; the force is 6,
//        counterAttackSize x groupSize, scaled up by the war level);
//    l2  within the delay window (tuned to 5..10 s here) the Outpost, whose
//        garrison is live in the bubble, marches 5 of its 6 soldiers out
//        (keeping a quarter of its strength home): the row turns LIVE, the
//        squad is a real group of 5, the Outpost's live garrison drops by 5,
//        the counterAttack event fires, and the live-AI budget holds;
//    l3  the squad walks toward the town;
//    l4  (accelerated: the squad is set down in the street) with the fighter
//        gone the retake drain takes the town back below townRevertSupport,
//        and the squad stays on as Houdan's garrison (GarrisonCache adopts
//        it; the record is finished).
//  The companion roster and the QRF director are parked so nothing else
//  walks into the town; staged units are damage-shielded.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
caT0 = time
triSimUntil { time > caT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

caRes = gmResistanceSide call GM_fnSideFromString
caV = gmZoneIndex "Village"
caO = gmZoneIndex "Outpost"
triAssertGe [caV, 0]
triAssertGe [caO, 0]
caPos = (gmZone caV) select 8
triAssertEq [(format ["%1", gmCounterAttackTune ["delayMin", 5]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["delayMax", 10]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 1]]), "true"]
triAssertEq [(count gmCounterAttacks), 0]

// both garrisons are live: the player stands at the Camp, inside both bubbles
triSimUntil { gmGarrisonSpawned caV }
triSimUntil { (gmGarrisonLive caV) >= 4 }
triSimUntil { gmGarrisonSpawned caO }
triSimUntil { (gmGarrisonLive caO) >= 6 }
caOut0 = gmGarrisonLive caO

// ---- l1: kill the town garrison, raise the town, the fighter walks out -----
caUnits = []
"caUnits = caUnits + (units _x)" forEach (gmGarrisonGroups caV)
{_x setDamage 1} forEach caUnits
triSimUntil { (gmGarrisonLive caV) == 0 }
gmZoneOnEvent ["counterAttack", {gmEvtCounter = gmEvtCounter + [_this]; caSeen = _this}]
gmZoneOnEvent ["townLost", {gmEvtTownLost = gmEvtTownLost + [_this]; caLost = _this}]
caSq = [caRes, gmFactionSquad [gmResistanceSide, 1, 1], caPos] call GM_fnSpawnSquad
caSq setCombatMode "BLUE"
{_x allowDammage false} forEach (units caSq)
doStop (units caSq)
triSimUntil { ((gmZone caV) select 2) == gmResistanceSide }
{deleteVehicle _x} forEach (units caSq)
caFlipT = time
triAssertEq [(count gmCounterAttacks), 1]
caRow = gmCounterAttacks select 0
triAssertEq [(caRow select 0), "Village"]
triAssertEq [(caRow select 2), "SCHEDULED"]
// the force: 6 at war level 1, x (1 + 0.25 x (war level - 1)), one group at most
triAssertGe [(caRow select 5), 6]
triAssertGe [12, (caRow select 5)]
caExp = caRow select 5
if (gmWarLevel <= 1) then {caExp = 6}
triAssertEq [(caRow select 5), caExp]
triAssertEq [(caRow select 6), 0]

// ---- l2: LIVE within the window, drawn from the Outpost's own garrison -----
triSimUntil { (((gmCounterAttacks + [["", "", "NONE"]]) select 0) select 2) == "LIVE" }
triAssertGe [caFlipT + 20, time]
caRow = gmCounterAttacks select 0
triAssertEq [(caRow select 1), "Outpost"]
triAssertEq [(caRow select 4), 5]
caGrp = caRow select 7
triAssertEq [(count units caGrp), 5]
triSimUntil { (gmGarrisonLive caO) == caOut0 - 5 }
triSimUntil { not (isNil "caSeen") }
triAssertEq [(caSeen select 1), "Village"]
caDiag = gmSimDiag
caLive = (caDiag select 2) select 3
triAssertGe [130, caLive]
triAssertGe [40, ((caDiag select 3) select 3)]
gmSimDiagLog

// ---- l3: the squad walks on the town ----------------------------------------
caD0 = (getPos (leader caGrp)) distance caPos
triSimUntil { ((getPos (leader caGrp)) distance caPos) < (caD0 - 25) }

// ---- l4: set them down in the street; nobody holds it, the drain retakes it --
caGrp setCombatMode "BLUE"
{_x allowDammage false} forEach (units caGrp)
{_x setPos [(caPos select 0) + 4 - (random 8), (caPos select 1) + 4 - (random 8), 0]} forEach (units caGrp)
triSimUntil { ((gmZoneSim caV) select 2) >= 1 }
triSimUntil { ((gmZone caV) select 2) == "NEUTRAL" }
triAssertGe [30, ((gmZone caV) select 4)]
triSimUntil { not (isNil "caLost") }
triAssertEq [(caLost select 1), "Village"]
// the record is finished and the squad is Houdan's garrison now
triSimUntil { (count gmCounterAttacks) == 0 }
triSimUntil { gmGarrisonSpawned caV }
triAssertEq [(gmGarrisonLive caV), 5]
triAssertEq [((gmZone caV) select 3), 0]
gmSimDiagLog

triEndTest
