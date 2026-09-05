// Issue #41: real ambient bodies, action dispatcher, atomic economic effects,
// boundaries, lazy state, cooldown/recovery and departure/panic/invalid gates.
triSimUntil { not (isNil "GM_CI_READY") }
triSimUntil { not (isNil "gmShkTicks") }
GM_SHK_CHANCE = -100
GM_CIV_WANDER = 100000
GM_CI_RECOVERY = 0
GM_CI_ROLL = {0.25}
ciZone = gmZoneIndex "Village"
ciPos = (gmZone ciZone) select GM_Z_POS
player setPos ciPos
triSimUntil { (count GM_CIV_GROUPS) > 0 }
ciBody = leader (GM_CIV_GROUPS select 0)
{_x disableAI "MOVE"} forEach (units (group ciBody))
// Outside the native 150 m support-accrual area, inside the 300 m visit.
ciBody setPos [(ciPos select 0) + 200, ciPos select 1, 0]
player setPos [(ciPos select 0) + 201, ciPos select 1, 0]
triAssertEq [([ciBody] call GM_CI_fnZone), ciZone]
triAssertGe [([ciBody, player] call GM_CI_fnEligible), 0]
triSimUntil { gmCiMenuBody == ciBody }
triAssertEq [count GM_CI_PROFILES, 0]
triAssertGe [gmCiMenuAsk, 0]
triAssertGe [gmCiMenuExtort, 0]

// Boundary rolls: exactly 75 qualifies, exactly 0.5 does not; fear 65 pays,
// fear <20 resists. No randomness on any asserted result.
triAssertEq [([74.99, 100, "SOLICIT", 0] call GM_CI_fnOutcome), "REFUSED"]
triAssertEq [([75, 0, "SOLICIT", 0.499] call GM_CI_fnOutcome), "DONATED"]
triAssertEq [([100, 0, "SOLICIT", 0.5] call GM_CI_fnOutcome), "REFUSED"]
triAssertEq [([0, 65, "EXTORT", 0.99] call GM_CI_fnOutcome), "EXTORTED"]
triAssertEq [([100, 64.99, "EXTORT", 0] call GM_CI_fnOutcome), "REFUSED"]
triAssertEq [([100, 19.99, "EXTORT", 0.99] call GM_CI_fnOutcome), "RESISTED"]
triAssertEq [([100, 20, "EXTORT", 0] call GM_CI_fnOutcome), "REFUSED"]

// Occupation suppresses personal baseline, not the native support ledger.
GM_CI_SPREAD = 0
gmZoneSet [ciZone, "owner", gmOccupierSide]
gmZoneSet [ciZone, "support", 80]
ciIndex = [ciBody, ciZone] call GM_CI_fnProfile
triAssertEq [((GM_CI_PROFILES select ciIndex) select 2), 40]
triAssertEq [((gmZone ciZone) select GM_Z_SUPPORT), 80]
triAssertGe [100, ((GM_CI_PROFILES select ciIndex) select 4)]
triAssertGe [((GM_CI_PROFILES select ciIndex) select 4), 0]
triAssertEq [([ciBody, ciZone] call GM_CI_fnProfile), ciIndex]
triAssertEq [count GM_CI_PROFILES, 1]

// Exercise the actual menu dispatcher, not a separate test-only payout path.
GM_CI_PROFILES set [ciIndex, [ciBody, "Village", 100, 100, 65, time, 0]]
ciMoney = gmResources
ciSupport = (gmZone ciZone) select GM_Z_SUPPORT
[ciBody, player, gmCiMenuAsk] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { (count GM_CI_LAST) > 0 }
triAssertEq [GM_CI_LAST select 0, "DONATED"]
triAssertEq [gmResources, ciMoney + GM_CI_DONATION]
triAssertEq [((gmZone ciZone) select GM_Z_SUPPORT), ciSupport - 0.5]
triAssertEq [GM_CI_LAST select 3, 90]
ciResult = [ciBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [ciResult select 0, "COOLDOWN"]
triAssertEq [gmResources, ciMoney + GM_CI_DONATION]

// A high-fear extortion pays even with low opinion and a losing donate roll.
GM_CI_ROLL = {0.99}
GM_CI_PROFILES set [ciIndex, [ciBody, "Village", 100, 40, 65, time, 0]]
ciMoney = gmResources
ciSupport = (gmZone ciZone) select GM_Z_SUPPORT
[ciBody, player, gmCiMenuExtort] exec "\gmcore\scripts\civilian_interaction_action.sqs"
triSimUntil { (GM_CI_LAST select 0) == "EXTORTED" }
ciResult = GM_CI_LAST
triAssertEq [ciResult select 0, "EXTORTED"]
triAssertEq [ciResult select 3, 10]
triAssertEq [gmResources, ciMoney + GM_CI_EXTORTION]
triAssertEq [((gmZone ciZone) select GM_Z_SUPPORT), ciSupport - 1.5]

// Fight-back is an exact-once hook for #42; no payment and no faction change.
ciResists = 0
GM_CI_ON_RESIST = {ciResists = ciResists + 1; ciResistBody = _this select 0; ciResistZone = _this select 2}
GM_CI_PROFILES set [ciIndex, [ciBody, "Village", 100, 100, 19, time, 0]]
ciMoney = gmResources
ciResult = [ciBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [ciResult select 0, "RESISTED"]
triAssertEq [ciResists, 1]
triAssertEq [ciResistBody, ciBody]
triAssertEq [ciResistZone, "Village"]
triAssertEq [gmResources, ciMoney]
ciResult = [ciBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [ciResists, 1]
triAssertEq [ciResult select 0, "COOLDOWN"]

// Lazy recovery uses elapsed simulation time, caps at the original baseline,
// and does not regenerate town support. A cooldown query updates only self.
GM_CI_RECOVERY = 1
GM_CI_PROFILES set [ciIndex, [ciBody, "Village", 80, 79, 65, time - 600, time + 60]]
ciSupport = (gmZone ciZone) select GM_Z_SUPPORT
ciResult = [ciBody, player, "SOLICIT"] call GM_CI_fnInteract
triAssertEq [ciResult select 3, 80]
triAssertEq [((gmZone ciZone) select GM_Z_SUPPORT), ciSupport]

// Panic and an in-progress shakedown invalidate even a previously shown menu.
GM_PANIC_ZONES = ["Village"]
GM_PANIC_UNTIL = [time + 100]
ciResult = [ciBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [ciResult select 0, "INVALID"]
GM_PANIC_ZONES = []
GM_PANIC_UNTIL = []
GM_SHK_ACTIVE = true
GM_SHK_VICTIM = ciBody
triAssertEq [([ciBody, player] call GM_CI_fnEligible), -1]
GM_SHK_ACTIVE = false
GM_SHK_VICTIM = objNull
triAssertEq [([objNull, player, "EXTORT"] call GM_CI_fnInteract) select 0, "INVALID"]
triAssertEq [([player, player, "EXTORT"] call GM_CI_fnInteract) select 0, "INVALID"]
triAssertEq [([ciBody, ciBody, "EXTORT"] call GM_CI_fnInteract) select 0, "INVALID"]
triAssertEq [([ciBody, player, "FAKE"] call GM_CI_fnInteract) select 0, "INVALID"]
triAssertEq [gmResources, ciMoney]

// Cap refuses new profiles rather than evicting an existing visitor's state.
GM_CI_MAX_PROFILES = 1
ciOther = (units (group ciBody)) select 1
ciOther setPos [(ciPos select 0) + 200, ciPos select 1, 0]
ciResult = [ciOther, player, "SOLICIT"] call GM_CI_fnInteract
triAssertEq [ciResult select 0, "CAPACITY"]
triAssertEq [count GM_CI_PROFILES, 1]
GM_CI_MAX_PROFILES = 64

// Within 300 m the opinion/fear stay stable even out of conversational reach.
player setPos [(ciPos select 0) + 250, ciPos select 1, 0]
[] call GM_CI_fnPrune
triAssertEq [count GM_CI_PROFILES, 1]
ciResult = [ciBody, player, "EXTORT"] call GM_CI_fnInteract
triAssertEq [ciResult select 0, "INVALID"]
// Leaving the local visit evicts the personal row but preserves support/money.
player setPos [500, 500, 0]
[] call GM_CI_fnPrune
triAssertEq [count GM_CI_PROFILES, 0]
triAssertEq [((gmZone ciZone) select GM_Z_SUPPORT), ciSupport]
triAssertEq [gmResources, ciMoney]
triSimUntil { isNull gmCiMenuBody }
triAssertEq [gmCiMenuAsk, -1]
triAssertEq [gmCiMenuExtort, -1]
triEndTest
