triSimUntil { not (isNil "GM_AS_READY") }
GM_AS_REMAINING = 100000
GM_SHK_CHANCE = -100
GM_CIV_WANDER = 100000
GM_CI_RECOVERY = 0
asZone = gmZoneIndex "Village"
asPos = (gmZone asZone) select GM_Z_POS
player setPos asPos
triSimUntil { count GM_CIV_GROUPS > 0 }
asBody = leader (GM_CIV_GROUPS select 0)
asBody disableAI "MOVE"
asBody disableAI "TARGET"
asBody setPos asPos
player setPos [(asPos select 0) + 2, asPos select 1, 0]
triAssertEq [gmAssailantDiagnostic "RESISTER", ""]
triAssertEq [gmAssailantSide, "WEST"]
asIndex = [asBody, asZone] call GM_CI_fnProfile
GM_CI_PROFILES set [asIndex, [asBody, "Village", 100, 100, 19, time, 0, false]]
triSimUntil { gmCiMenuBody == asBody }
asMoney = gmResources
asSupport = (gmZone asZone) select GM_Z_SUPPORT
[asBody, player, "PREVIEW"] call GM_CI_fnRequest
triAssertEq [gmAssailantCount "ALL", 0]
[asBody, player, "CANCEL"] call GM_CI_fnRequest
triAssertEq [gmResources, asMoney]
triAssertEq [((gmZone asZone) select GM_Z_SUPPORT), asSupport]
asOld = group asBody
asType = typeOf asBody
asResult = [asBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [asResult select 0, "RESISTED"]
triAssertEq [gmResources, asMoney]
triAssertEq [((gmZone asZone) select GM_Z_SUPPORT), asSupport - 1.5]
triAssertEq [gmAssailantClass asBody, "RESISTER"]
triAssertEq [typeOf asBody, asType]
triAssertEq [format ["%1", side asBody], gmAssailantSide]
triAssert [not ((group asBody) == asOld)]
triAssertEq [count (units (group asBody)), 1]
triAssertEq [count (weapons asBody), 1]
triAssertEq [count (magazines asBody), 3]
triAssertEq [([asBody, player] call GM_CI_fnEligible), -1]
triAssertEq [([asBody, player, "EXTORT"] call GM_CI_fnInteract) select 0, "INVALID"]
triAssertEq [gmResources, asMoney]
triAssertEq [((gmZone asZone) select GM_Z_SUPPORT), asSupport - 1.5]
triAssert [not (gmAssailantRegister [asBody, "ROGUE", objNull, "Village"])]
triAssert ["Village" in GM_PANIC_ZONES]

// Death records capture classification before prune; no murder debit or resentment.
asSupport = (gmZone asZone) select GM_Z_SUPPORT
asResent = count GM_RESENT_ZONES
asBody setDammage 1
triAssertEq [((gmCivKilled select ((count gmCivKilled) - 1)) select 4), "RESISTER"]
triSimUntil { gmAssailantCount "ALL" == 0 }
triSimUntil { count gmCivKilled == 0 }
triAssertEq [((gmZone asZone) select GM_Z_SUPPORT), asSupport]
triAssertEq [count GM_RESENT_ZONES, asResent]

// Hard cap revalidation; no shared groups, no extra magazines on duplicates.
GM_PANIC_ZONES = []
GM_PANIC_UNTIL = []
GM_CIV_PER_TOWN = 8
GM_CIV_MAX_GROUPS = 8
asBodies = []
asI = 0
while {asI < 8} do {asG = [asZone] call GM_fnCivSpawnTown; asU = leader asG; asU disableAI "TARGET"; asU disableAI "MOVE"; asM = "RESISTER"; if (asI < 2) then {asM = "ROGUE"}; triAssert [gmAssailantRegister [asU, asM, player, "Village"]]; asBodies = asBodies + [asU]; asI = asI + 1}
triAssertEq [gmAssailantCount "ALL", 8]
triAssertEq [gmAssailantCount "ROGUE", 2]
asG = [asZone] call GM_fnCivSpawnTown
asU = leader asG
asU setPos asPos
asIndex = [asU, asZone] call GM_CI_fnProfile
GM_CI_PROFILES set [asIndex, [asU, "Village", 100, 100, 80, time, 0, false]]
asSupport = (gmZone asZone) select GM_Z_SUPPORT
triAssertEq [([asU, player, "EXTORT"] call GM_CI_fnInteract) select 0, "ASSAILANT_UNAVAILABLE"]
triAssertEq [gmResources, asMoney]
triAssertEq [((gmZone asZone) select GM_Z_SUPPORT), asSupport]
triAssertEq [([asU, player] call GM_CI_fnInspect) select 0, "READY"]
triAssert [not (gmAssailantRegister [asU, "ROGUE", objNull, "Village"])]
triAssertEq [gmAssailantAdvance [10, 100, false], 10]
triAssertEq [gmAssailantAdvance [10, 100, true], 0]
{triAssert [gmAssailantRemove _x]} forEach asBodies
triAssertEq [gmAssailantCount "ALL", 0]
triEndTest
