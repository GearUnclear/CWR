// The same conversion and targeting path must work with mod-owned civilians
// and a different spare side. Camp is the open wadi south of the IDF airbase.
triSimUntil { not (isNil "GM_AS_READY") }
GM_AS_REMAINING = 100000
GM_SHK_CHANCE = -100
GM_COMP_ALIVE = [false]
GM_CIV_RADIUS = 0
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false
removeAllWeapons player
player setCaptive true
player setPos [8350, 2950, 0]
triAssertEq [gmOccupierSide, "WEST"]
triAssertEq [gmResistanceSide, "EAST"]
triAssertEq [gmAssailantSide, "GUER"]
triAssertEq [gmAssailantDiagnostic "RESISTER", ""]
asZone = gmZoneIndex "El Tor"
gmCvClassAt = 0
asOldGroup = [asZone] call GM_fnCivSpawnTown
asShooter = leader asOldGroup
triAssertEq [typeOf asShooter, "LoBo_Civ_01"]
asShooter setPos [8330, 2950, 0]
asShooter setDir 90
asShooter disableAI "MOVE"
asShooter allowDammage false
asShots = 0
asShooter addEventHandler ["fired", {asShots = asShots + 1}]
asTG = createGroup west
(gmFactionTierClass [gmOccupierSide, 1]) createUnit [[8350,2950,0],asTG,"asTarget = this",0.5,"PRIVATE"]
triAssertEq [typeOf asTarget, "LoBoGolaniWB"]
triAssertEq [format ["%1", side asTarget], "WEST"]
// Keep the personal extorter off the shooter/target line: a bullet aimed
// at the player must not hit the fixture soldier on its way past.
player setPos [8330,3250,0]
asTarget disableAI "TARGET"
asTarget disableAI "AUTOTARGET"
asTarget disableAI "MOVE"
asTarget stop true
asTG setCombatMode "BLUE"
// A shot anywhere in the live Sinai campaign is not targeting evidence.
// Hit reports [victim, shooter, damage]; only this assailant hitting this
// soldier counts. Leave damage enabled so the native Hit event can fire.
asTargetHits = 0
asTarget addEventHandler ["hit", {if ((_this select 1) == asShooter) then {asTargetHits = asTargetHits + 1}}]
triAssert [gmAssailantRegister [asShooter,"RESISTER",player,"El Tor"]]
triAssertEq [typeOf asShooter, "LoBo_Civ_01"]
triAssertEq [format ["%1", side asShooter], "GUER"]
triAssert [not ((group asShooter) == asOldGroup)]
triAssertEq [count (units (group asShooter)), 1]
triAssertEq [count (weapons asShooter), 1]
triAssertEq [count (magazines asShooter), 3]
triSimUntil { asTargetHits > 0 }
triAssertGe [asTargetHits, 1]
triAssertGt [asShots, 0]
triAssert [captive player]
triAssertEq [gmUndercoverStatus, 0]
triAssertGe [((group asShooter) knowsAbout asTarget), 0.1]
triEndTest
