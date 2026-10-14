// Issue #78: native cargo seating must establish the AI InCargo state for a
// live get-out as well as wreck cleanup (guerrilla_offscreen_convoy).
// Strict mode catches the AI_ERROR in DoGetOut if NativeMoveIn omits it.

triSimUntil { GM_LIB_READY }
player allowDammage false
ncOut = gmZoneIndex "Outpost"
ncVil = gmZoneIndex "Village"
triAssertGe [ncOut, 0]
triAssertGe [ncVil, 0]
ncPos = (gmZone ncVil) select 8
player setPos [ncPos select 0, ncPos select 1, 0]
triSimFrames 5

ncTruck = gmTrafficForceSpawn ["convoy", ncOut]
triAssertEq [(format ["%1", isNull ncTruck]), "false"]
ncDriver = driver ncTruck
triAssertEq [(format ["%1", alive ncDriver]), "true"]
ncCargo = []
{if ((_x != ncDriver) and (_x != (gunner ncTruck)) and (_x != (commander ncTruck))) then {ncCargo = ncCargo + [_x]}} forEach (crew ncTruck)
triAssertGt [count ncCargo, 0]
ncRider = ncCargo select 0
triAssertEq [(format ["%1", alive ncRider]), "true"]
triAssertEq [(format ["%1", (vehicle ncRider) == ncTruck]), "true"]

// Use the native passenger as spawned; moveInCargo would repair the missing
// state before the assertion and turn this into a false pass.
// Clear its assignment so the AI does not immediately try to board again.
unassignVehicle ncRider
ncRider action ["EJECT", ncTruck]
triSimUntil { (vehicle ncRider) == ncRider }
triAssertEq [(format ["%1", alive ncRider]), "true"]
triAssertEq [(format ["%1", ncRider in (crew ncTruck)]), "false"]
triAssertEq [(format ["%1", (driver ncTruck) == ncDriver]), "true"]
triAssertEq [gmTrafficCount "convoy", 1]

triEndTest
