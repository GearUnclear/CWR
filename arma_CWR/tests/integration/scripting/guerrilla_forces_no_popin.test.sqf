// ============================================================================
//  Occupier forces, NO POP-IN (OCCUPIER-FORCES.md 3.3, invariant S5: the
//  wiped-garrison remainder "no longer appears beside the player who just
//  killed the last defender"; native GarrisonCache pending deposit).  Boots
//  the SHIPPED Guerrilla.Abel template (occupierForces = 1) on the full
//  Remaster:
//    n1  a real camera at the default view distance in daylight; the
//        Outpost's garrison is live (the player starts 520 m away) and a
//        reserve of three waits beside it (a top-up written while the
//        garrison was live);
//    n2  the player walks to 250 m of the Outpost (inside its deposit
//        bubble, outside its 150 m capture circle) and kills the last
//        defender: the waiting three become a PENDING deposit
//        (gmZoneSim slot 13), not bodies beside him;
//    n3  for 20 s nothing materializes while he stays: no cache garrison,
//        no live body, the deposit still pending.
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
npT0 = time
triSimUntil { time > npT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- n1: a real camera; the Outpost live with a reserve beside it -------------
npB = gmTrafficPercept (getPos player)
triAssertEq [(npB select 0), 1]
triAssertGe [(npB select 3), 750]
npO = gmZoneIndex "Outpost"
triAssertGe [npO, 0]
npPos = (gmZone npO) select 8
triSimUntil { gmGarrisonSpawned npO }
triSimUntil { (gmGarrisonLive npO) >= 6 }
gmZoneSet [npO, "garrison", 3]
triAssertEq [((gmZone npO) select 3), 3]

// ---- n2: 250 m out, the last defender dies --------------------------------------
player setPos [(npPos select 0) - 150, (npPos select 1) - 200, 0]
npUnits = []
"npUnits = npUnits + (units _x)" forEach (gmGarrisonGroups npO)
{_x setDamage 1} forEach npUnits
triSimUntil { (gmGarrisonLive npO) == 0 }
triSimUntil { ((gmZoneSim npO) select 13) == 3 }
triAssertEq [((gmZone npO) select 3), 0]
triAssertEq [((gmZone npO) select 2), gmOccupierSide]

// ---- n3: nothing appears beside him ---------------------------------------------
npT1 = time
triSimUntil { (gmGarrisonSpawned npO) or ((gmGarrisonLive npO) > 0) or (time > npT1 + 20) }
triAssertEq [(format ["%1", gmGarrisonSpawned npO]), "false"]
triAssertEq [(gmGarrisonLive npO), 0]
triAssertEq [((gmZoneSim npO) select 13), 3]
triAssertEq [((gmZone npO) select 3), 0]

triEndTest
