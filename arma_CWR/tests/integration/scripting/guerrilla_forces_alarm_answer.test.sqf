// ============================================================================
//  Occupier forces, the native alarm answer (OCCUPIER-FORCES.md 8.2, D20;
//  12.2).  Boots the SHIPPED Guerrilla.Abel template (occupierForces = 1) on
//  the full Remaster:
//    a1  the pool is sized; gmLegacyQrf is false (a new campaign with the
//        key on: alarms are answered natively); the director stays DORMANT
//        (wakeTowns 99), the legacy first-town roll is 0 and the abstract
//        leg runs at 100 m/s;
//    a2  the Outpost garrison (515 m from the Camp, spawned at boot) is
//        revealed the player every frame: GREEN, YELLOW, RED;
//    a3  the director answers the RED zone with ONE CAKAlarm order: a
//        gmOccupierOps row of kind "ALARM", origin "ALARM", target the
//        Outpost, sourced from another occupier base; the alarm-answer
//        counter (gmOccupierForces slot 26) rises;
//    a4  the answer is born: its phase reaches HOLDING or LIVE with a birth
//        reason; the Outpost sat in the player's bubble at launch, so a live
//        squad may be born anywhere on its road (even at its base, km out),
//        and its march is checked over 150 s: the leader leaves its birth
//        point by more than 150 m and ends nearer the Outpost;
//        through a2 to a4 qrf.sqs fields nothing (GM_QRF_ACTIVE stays
//        false);
//    a5  the legacy path: gmForcesForce ["legacySave", 1] makes gmLegacyQrf
//        true and qrf.sqs fields its script QRF on the RED zone.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { not (isNil "GM_QRF_ACTIVE") }

GM_COMP_ALIVE = [false]
{deleteVehicle _x} forEach ((units group player) - [player])
aaT0 = time
triSimUntil { time > aaT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- a1: the pool, native answers -------------------------------------------
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [(format ["%1", gmLegacyQrf]), "false"]
triAssertEq [(format ["%1", (gmOccupierForces) select 27]), "false"]
triAssertEq [(format ["%1", (gmOccupierForces) select 28]), "false"]
triAssertEq [(format ["%1", gmForcesTune ["wakeTowns", 99]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["firstTownChance", 0]]), "true"]
triAssertEq [(format ["%1", gmCounterAttackTune ["speed", 100]]), "true"]
triAssertEq [(format ["%1", GM_QRF_ACTIVE]), "false"]
aaAns0 = (gmOccupierForces) select 26
aaQrfSeen = false

// ---- a2: the Outpost goes RED ------------------------------------------------
aaO = gmZoneIndex "Outpost"
triAssertGe [aaO, 0]
triAssertEq [((gmZone aaO) select 2), gmOccupierSide]
triSimUntil { (gmGarrisonSpawned aaO) and ((gmGarrisonLive aaO) > 0) }
triSimUntil { {_x reveal player} forEach (gmGarrisonGroups aaO); if (GM_QRF_ACTIVE) then {aaQrfSeen = true}; (gmZoneAlert aaO) >= 2 }

// ---- a3: one native answer ---------------------------------------------------
aaOp = []
triSimUntil { {_x reveal player} forEach (gmGarrisonGroups aaO); if (GM_QRF_ACTIVE) then {aaQrfSeen = true}; aaOp = []; {if (((_x select 1) == "ALARM") and ((_x select 5) == "Outpost")) then {aaOp = _x}} forEach gmOccupierOps; (count aaOp) > 0 }
triAssertEq [(aaOp select 2), "ALARM"]
triAssertEq [(format ["%1", (aaOp select 4) != ""]), "true"]
triAssertEq [(format ["%1", (aaOp select 4) != "Outpost"]), "true"]
triAssertGe [(aaOp select 0), 1]
triAssertGe [(aaOp select 7), 1]
triAssertGe [((gmOccupierForces) select 26), aaAns0 + 1]
aaId = aaOp select 0
aaN = 0
{if ((_x select 1) == "ALARM") then {aaN = aaN + 1}} forEach gmOccupierOps
triAssertEq [aaN, 1]

// ---- a4: the answer arrives --------------------------------------------------
triSimUntil { {_x reveal player} forEach (gmGarrisonGroups aaO); if (GM_QRF_ACTIVE) then {aaQrfSeen = true}; aaOp = []; {if ((_x select 0) == aaId) then {aaOp = _x}} forEach gmOccupierOps; ((count aaOp) > 0) and ((aaOp select 6) in ["HOLDING", "LIVE"]) }
triAssertEq [(format ["%1", (aaOp select 10) != ""]), "true"]
// the Outpost was in the player's bubble at launch, so the force may be born
// live anywhere on its road (at its base, kilometres out): the march on the
// Outpost is checked as the live leader moving off and closing on it
aaOPos = (gmZone aaO) select 8
aaLeader = objNull
triSimUntil { aaLeader = objNull; {if ((_x select 10) == aaId) then {aaLeader = _x select 20}} forEach gmCounterAttacks; (not (isNull aaLeader)) or (not ((aaOp select 6) == "LIVE")) }
aaD0 = 0
aaP0 = [0, 0, 0]
if (not (isNull aaLeader)) then {aaD0 = aaLeader distance aaOPos; aaP0 = getPos aaLeader}
aaT1 = time
triSimUntil { {_x reveal player} forEach (gmGarrisonGroups aaO); if (GM_QRF_ACTIVE) then {aaQrfSeen = true}; (time > aaT1 + 150) or (isNull aaLeader) }
aaClosed = true
if (not (isNull aaLeader)) then {aaClosed = ((aaLeader distance aaP0) > 150) and ((aaLeader distance aaOPos) < aaD0)}
triAssertEq [(format ["%1", aaClosed]), "true"]
triAssertEq [(format ["%1", aaQrfSeen]), "false"]
triAssertEq [(format ["%1", GM_QRF_ACTIVE]), "false"]
triAssertEq [(format ["%1", gmLegacyQrf]), "false"]

// ---- a5: a legacy save: qrf.sqs fields its script QRF ------------------------
triAssertEq [(format ["%1", gmForcesForce ["legacySave", 1]]), "true"]
triAssertEq [(format ["%1", gmLegacyQrf]), "true"]
triAssertEq [(format ["%1", (gmOccupierForces) select 27]), "true"]
triAssertEq [(format ["%1", (gmOccupierForces) select 28]), "true"]
triSimUntil { {_x reveal player} forEach (gmGarrisonGroups aaO); GM_QRF_ACTIVE }
triAssertGe [(gmZoneAlert GM_QRF_ZONE), 2]
triAssertGe [(count units GM_QRF_GROUP), 1]
gmSimDiagLog

triEndTest
