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
asGroups = []
asAdmitted = 0
asIndependent = true
asI = 0
while {asI < 8} do {asG = [asZone] call GM_fnCivSpawnTown; asU = leader asG; asU disableAI "TARGET"; asU disableAI "MOVE"; asM = "RESISTER"; if (asI < 2) then {asM = "ROGUE"}; if (gmAssailantRegister [asU, asM, player, "Village"]) then {asAdmitted = asAdmitted + 1}; asIndependent = asIndependent and (not ((group asU) in asGroups)) and ((count (units (group asU))) == 1); asGroups = asGroups + [group asU]; asBodies = asBodies + [asU]; asI = asI + 1}
// Surface aggregate results at top level; ignored nested assertion returns
// do not fail the Trident scenario.
triAssertEq [asAdmitted, 8]
triAssert [asIndependent]
triAssertEq [gmAssailantCount "ALL", 8]
triAssertEq [gmAssailantCount "ROGUE", 2]
triAssert [not (gmAssailantRegister [asBodies select 0, "ROGUE", objNull, "Village"])]
triAssertEq [count (magazines (asBodies select 0)), 3]
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
asRemoved = 0
{if (gmAssailantRemove _x) then {asRemoved = asRemoved + 1}} forEach asBodies
triAssertEq [asRemoved, 8]
triAssertEq [gmAssailantCount "ALL", 0]
asEmpty = true
{asEmpty = asEmpty and ((count (units _x)) == 0)} forEach asGroups
triAssert [asEmpty]

// Exercise the production candidate collector with one real ambient resident.
// Duplicate town rows must neither duplicate the body nor allocate profiles.
while {(count GM_CIV_GROUPS) > 0} do {[0] call GM_fnCivDespawnAt}
GM_CIV_PER_TOWN = 0
GM_CIV_GROUP_SIZE = 1
asG = [asZone] call GM_fnCivSpawnTown
asRogue = leader asG
asRogue disableAI "TARGET"
asRogue disableAI "AUTOTARGET"
asRogue disableAI "MOVE"
asRogue allowDammage false
asRogue setPos [(asPos select 0) + 30, asPos select 1, 0]
gmCiMenuBody = objNull
gmCiConfirmBody = objNull
GM_CI_REQUEST = []
[] call GM_CI_fnPrune
asProfiles = count GM_CI_PROFILES
GM_CIV_GROUPS = [asG, asG]
GM_CIV_GROUP_ZONE = ["Village", "Village"]
triAssertEq [count ([] call GM_AS_fnCandidates), 1]
triAssertEq [count GM_CI_PROFILES, asProfiles]
GM_CIV_GROUPS = [asG]
GM_CIV_GROUP_ZONE = ["Village"]
gmCiMenuBody = asRogue
triAssertEq [count ([] call GM_AS_fnCandidates), 0]
gmCiMenuBody = objNull
gmCiConfirmBody = asRogue
triAssertEq [count ([] call GM_AS_fnCandidates), 0]
gmCiConfirmBody = objNull
GM_CI_REQUEST = [asRogue, player, "PREVIEW"]
triAssertEq [count ([] call GM_AS_fnCandidates), 0]
GM_CI_REQUEST = []
GM_SHK_ACTIVE = true
GM_SHK_VICTIM = asRogue
triAssertEq [count ([] call GM_AS_fnCandidates), 0]
GM_SHK_ACTIVE = false
GM_SHK_VICTIM = objNull
GM_PANIC_ZONES = ["Village"]
GM_PANIC_UNTIL = [time + 1000]
triAssertEq [count ([] call GM_AS_fnCandidates), 0]

// The real manager pauses without eligible residents, then spends eligible
// simulation time and fires once. A reset leaves no overdue incident debt.
GM_AS_REMAINING = 20
asTick = GM_AS_TICKS
triSimUntil { GM_AS_TICKS > (asTick + 2) }
triAssertEq [GM_AS_REMAINING, 20]
GM_PANIC_ZONES = []
GM_PANIC_UNTIL = []
asTick = GM_AS_TICKS
triSimUntil { GM_AS_TICKS > (asTick + 2) }
triAssert [GM_AS_REMAINING < 20]
GM_AS_ROLL = {0.5}
GM_AS_REMAINING = 0
triSimUntil { (gmAssailantCount "ROGUE") == 1 }
triSimUntil { GM_AS_REMAINING > 0 }
triAssertEq [gmAssailantClass asRogue, "ROGUE"]
triAssertEq [GM_AS_REMAINING, gmAssailantInterval 0.5]
triAssert ["Village" in GM_PANIC_ZONES]
triAssert [not ((group asRogue) in GM_CIV_GROUPS)]

// Panic reaps the old population group, never its converted resident.
GM_PANIC_UNTIL = [time + GM_PANIC_EMPTY_T - 1]
asTick = gmCivTicks
triSimUntil { gmCivTicks > asTick }
triAssert [alive asRogue]
triAssertEq [gmAssailantCount "ROGUE", 1]
triAssertEq [count GM_CIV_GROUPS, 0]
triAssertEq [GM_AS_REMAINING, gmAssailantInterval 0.5]

// Native departure cleanup reaps the isolated assailant group as well.
asRogueGroup = group asRogue
player setPos [500, 500, 0]
triSimUntil { (gmAssailantCount "ALL") == 0 }
triAssert [isNull asRogue]
triAssertEq [count (units asRogueGroup), 0]
triAssertEq [GM_AS_REMAINING, gmAssailantInterval 0.5]
triEndTest
