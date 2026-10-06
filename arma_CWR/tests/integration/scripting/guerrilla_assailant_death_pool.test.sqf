triSimUntil { not (isNil "GM_AS_READY") }
GM_AS_REMAINING = 100000
GM_SHK_CHANCE = -100
GM_CIV_RADIUS = 0
GM_CIV_PER_TOWN = 0
GM_QRF_TICK = 999999
GM_COMP_ALIVE = [false]
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
asDeathZone = gmZoneIndex "Village"
asDeathPos = (gmZone asDeathZone) select GM_Z_POS
player setPos asDeathPos
asDeathCycle = 0
asDeathOK = true
asDeathCaptured = 0
asDeathBodies = []
asDeathGroups = []

// Repeated deaths must not retain private AI groups. Dead brains remain in
// their groups until casualty reporting unless assailant retirement reaps
// them explicitly; keeping corpses here also rules out deletion as a fix.
triSimUntil { if ((asDeathCycle < 70) and asDeathOK) then {asDeathG = createGroup GM_CIV_SIDE; (GM_CIV_CLASSES select 0) createUnit [asDeathPos, asDeathG, "asDeathBody = this", 0.5, "PRIVATE"]; asDeathBody disableAI "TARGET"; asDeathBody disableAI "AUTOTARGET"; asDeathBody disableAI "MOVE"; asDeathOK = gmAssailantRegister [asDeathBody, "RESISTER", player, "Village"]; if (asDeathOK) then {asDeathBodies = asDeathBodies + [asDeathBody]; asDeathGroups = asDeathGroups + [group asDeathBody]; asDeathBody addEventHandler ["killed", {if ((gmAssailantClass (_this select 0)) == "RESISTER") then {asDeathCaptured = asDeathCaptured + 1}}]; asDeathBody setDammage 1; asDeathCycle = asDeathCycle + 1}}; (asDeathCycle >= 70) or (not asDeathOK) }
triAssert [asDeathOK]
triAssertEq [asDeathCycle, 70]
triAssertEq [asDeathCaptured, 70]
triAssertEq [gmAssailantCount "ALL", 0]
triSimUntil { (gmAssailantClass asDeathBody) == "" }
asDeathCorpses = true
{asDeathCorpses = asDeathCorpses and (not (isNull _x)) and (not (alive _x))} forEach asDeathBodies
triAssert [asDeathCorpses]
asDeathEmpty = true
{asDeathEmpty = asDeathEmpty and ((count (units _x)) == 0)} forEach asDeathGroups
triAssert [asDeathEmpty]
{deleteVehicle _x} forEach asDeathBodies
triEndTest
