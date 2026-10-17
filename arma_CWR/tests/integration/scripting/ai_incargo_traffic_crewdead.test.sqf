// ============================================================================
//  AI InCargo state for natively seated cargo (round 3 B, issue #78 family).
//
//  Traffic seats its crews through NativeMoveIn.  A cargo rider seated that
//  way used to keep his previous brain state, so the first get-out through
//  AIUnit::DoGetOut tripped "check failed: _state == InCargo" and strict mode
//  shut the game down (exit 3).  The occupier-forces comparison video hit it
//  when a traffic patrol lost its driver: Traffic's crewDead despawn steps the
//  living seated rider out with DoGetOut.
//
//  Case 1: a convoy rider ejected by action (the plain get-out path).
//  Case 2: a patrol whose driver dies; the crewDead despawn dismounts the
//  living rider (the path from the video).
//  Both riders must leave their seat alive with no strict-mode error.
// ============================================================================

triSimUntil { GM_LIB_READY }
player allowDammage false

icOut = gmZoneIndex "Outpost"
icCamp = gmZoneIndex "Camp"
icVil = gmZoneIndex "Village"
triAssertGe [icOut, 0]
triAssertGe [icCamp, 0]
triAssertGe [icVil, 0]
// a patrol needs a second occupier zone to drive to
gmZoneSet [icCamp, "owner", gmOccupierSide]
icPos = (gmZone icVil) select 8
player setPos [icPos select 0, icPos select 1, 0]
triSimFrames 5

// -- case 1: convoy rider, ejected by action ----------------------------------
icTruck = gmTrafficForceSpawn ["convoy", icOut]
triAssertEq [(format ["%1", isNull icTruck]), "false"]
icTDriver = driver icTruck
icTCargo = []
{if ((_x != icTDriver) and (_x != (gunner icTruck)) and (_x != (commander icTruck))) then {icTCargo = icTCargo + [_x]}} forEach (crew icTruck)
triAssertGt [count icTCargo, 0]
icTRider = icTCargo select 0
icTRider allowDammage false
triAssertEq [(format ["%1", alive icTRider]), "true"]
triAssertEq [(format ["%1", (vehicle icTRider) == icTruck]), "true"]
// unassigned so the AI does not order him straight back aboard
unassignVehicle icTRider
icTRider action ["EJECT", icTruck]
triSimUntil { (vehicle icTRider) == icTRider }
triAssertEq [(format ["%1", alive icTRider]), "true"]
triAssertEq [(format ["%1", icTRider in (crew icTruck)]), "false"]
triAssertEq [(format ["%1", (driver icTruck) == icTDriver]), "true"]

// -- case 2: patrol rider, dismounted by the crewDead despawn ------------------
icCar = gmTrafficForceSpawn ["patrol", icOut]
triAssertEq [(format ["%1", isNull icCar]), "false"]
triAssertEq [gmTrafficCount "patrol", 1]
icDriver = driver icCar
icCargo = []
{if ((_x != icDriver) and (_x != (gunner icCar)) and (_x != (commander icCar))) then {icCargo = icCargo + [_x]}} forEach (crew icCar)
triAssertGt [count icCargo, 0]
icRider = icCargo select 0
icRider allowDammage false
if (!isNull (gunner icCar)) then {(gunner icCar) allowDammage false}
triAssertEq [(format ["%1", (vehicle icRider) == icCar]), "true"]

icDriver setDammage 1
triAssertEq [(format ["%1", alive icDriver]), "false"]
// Traffic files the entry as crewDead and steps the living rider out (the
// ambient spawner may put a new patrol on the road at once, so wait on this
// hull leaving the registry rather than on the patrol count)
triSimUntil { !(icCar in gmTrafficVehicles) }
triSimUntil { (vehicle icRider) == icRider }
triSimFrames 30
triAssertEq [(format ["%1", alive icRider]), "true"]
triAssertEq [(format ["%1", icRider in (crew icCar)]), "false"]

triEndTest
