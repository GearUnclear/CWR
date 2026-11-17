// ============================================================================
//  Occupier forces, the WIPED-GARRISON RELEASE (OCCUPIER-FORCES.md 3.3,
//  invariant S5; native GarrisonCache pending deposit).  Boots the SHIPPED
//  Guerrilla.Abel template (occupierForces = 1) on the full Remaster:
//    r1  a real camera at the default view distance in daylight; the
//        Outpost's garrison is live with a reserve of three beside it; the
//        player kills the last defender from 250 m (inside the deposit
//        bubble): the three are pending, nothing appears beside him;
//    r2  he walks out past 850 m (cacheRadius + the despawn hysteresis): the
//        deposit releases into the reserve;
//    r3  he walks back inside 800 m: the reserve materializes as the post's
//        garrison.
//  The companion roster and the QRF director are parked; the player is
//  damage-shielded.
// ============================================================================

triSimUntil { GM_LIB_READY }

GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
wpT0 = time
triSimUntil { time > wpT0 + 6 }
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

// ---- r1: the last defender dies with the player inside the bubble -------------
wpB = gmTrafficPercept (getPos player)
triAssertEq [(wpB select 0), 1]
triAssertGe [(wpB select 3), 750]
wpO = gmZoneIndex "Outpost"
triAssertGe [wpO, 0]
wpPos = (gmZone wpO) select 8
triSimUntil { gmGarrisonSpawned wpO }
triSimUntil { (gmGarrisonLive wpO) >= 6 }
gmZoneSet [wpO, "garrison", 3]
player setPos [(wpPos select 0) - 150, (wpPos select 1) - 200, 0]
wpUnits = []
"wpUnits = wpUnits + (units _x)" forEach (gmGarrisonGroups wpO)
{_x setDamage 1} forEach wpUnits
triSimUntil { (gmGarrisonLive wpO) == 0 }
triSimUntil { ((gmZoneSim wpO) select 13) == 3 }
triAssertEq [((gmZone wpO) select 3), 0]
triAssertEq [(format ["%1", gmGarrisonSpawned wpO]), "false"]

// ---- r2: out past 850 m: the deposit releases -----------------------------------
player setPos [(wpPos select 0) - 600, (wpPos select 1) - 1000, 0]
triAssertGe [(player distance wpPos), 900]
triSimUntil { ((gmZoneSim wpO) select 13) == 0 }
triAssertGe [((gmZone wpO) select 3), 3]
triAssertEq [(format ["%1", gmGarrisonSpawned wpO]), "false"]

// ---- r3: back inside 800 m: the garrison materializes ---------------------------
player setPos [(wpPos select 0) - 400, (wpPos select 1) - 500, 0]
triAssertGe [790, (player distance wpPos)]
triSimUntil { gmGarrisonSpawned wpO }
triSimUntil { (gmGarrisonLive wpO) >= 3 }
triAssertEq [((gmZoneSim wpO) select 13), 0]

triEndTest
