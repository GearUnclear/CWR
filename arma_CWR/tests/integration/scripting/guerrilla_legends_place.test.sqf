// ============================================================================
//  Enemy Legends, half one: WHERE they stand and WHAT they are.
//    The native LegendRegistry resolves the three enemy commanders in the
//    seeding tick (role from the occupier's faction descriptor, stand from a
//    350..950 m annulus around a military zone) and then spawns one per
//    second. The unit suite drives the picker and the role resolver with no
//    world; this lane is the half that needs the real one:
//      * exactly THREE commanders are placed, each with a resolved role, a
//        zone and a stand out of the water;
//      * every body is a class THIS faction can field, on the occupier side -
//        a wrong-faction crew class would spawn happily and only a side check
//        would catch it;
//      * every stand is at least 300 m off every zone centre, which is the
//        observable form of the design decision this feature rests on: a
//        commander stands OUTSIDE the 150 m zone presence radius, so he never
//        garrisons his zone;
//      * ... and therefore the outpost is still capturable while he lives,
//        proved by running the meter to a flip with all three alive;
//      * the boss is pinned with DAMove and nothing else, so he holds his
//        ground but still acquires and fires;
//      * boss groups are invisible to the garrison cache and to the QRF;
//      * the marker, the objective and the dossier row all say the same thing,
//        because all three read the same registry row.
//    The kill half (the pin under fire, the tank rule, the defeat latch and
//    the no-respawn window) is guerrilla_legends_defeat: splitting them keeps
//    one long negative window from eating another one's budget, and keeps a
//    stray bullet in the fire test from invalidating these assertions.
//
//  ASSERTIONS INSIDE A while/forEach BODY ARE SILENT: the harness only reads
//  the value of the whole LINE, and a loop returns Nothing. Every loop below
//  therefore accumulates a diagnostic STRING and the assertion is made on the
//  aggregate afterwards, which also makes a failure say which commander broke.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { gmLegendCount >= 3 }
// the companion row is INSERTED ahead of the boss rows, so let it exist before
// any row index is captured
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }

// -- the three boss rows, by kind, and their actors --------------------------
glBoss = []
glI = 0
while {glI < gmLegendCount} do {if (((gmLegendInfo glI) select 2) == 1) then {glBoss = glBoss + [glI]}; glI = glI + 1}
triAssertEq [(count glBoss), 3]
triAssertEq [gmLegendCount, 4]
// one commander per tick, so three ticks of the 1 Hz poll
triSimUntil { (not (isNull (gmLegendBody (glBoss select 0)))) and (not (isNull (gmLegendBody (glBoss select 1)))) and (not (isNull (gmLegendBody (glBoss select 2)))) }

// -- identity, role and zone. The role assertion is CONJUNCTIVE on purpose:
//    "Commander" is both the Elite role name and the pre-roll sentinel, so the
//    role string alone cannot prove that resolution ran. A non-empty zone and
//    a live body can. -----------------------------------------------------------
glBad = ""
glRoles = ""
glI = 0
while {glI < 3} do {glRow = gmLegendInfo (glBoss select glI); glTag = format ["[%1]", glRow select 0]; if ((glRow select 1) == "") then {glBad = glBad + glTag + "noname "}; if ((["Sniper", "Commander", "Tank Commander"] find (glRow select 3)) < 0) then {glBad = glBad + glTag + "role=" + (glRow select 3) + " "}; if ((glRow select 4) == "") then {glBad = glBad + glTag + "nozone "}; if (not (glRow select 6)) then {glBad = glBad + glTag + "notlegend "}; glRoles = glRoles + (glRow select 3) + "|"; glI = glI + 1}
triAssertEq [glBad, ""]
// this fixture's EAST descriptor fields a sniper rung, an officer and a tank
// hull, so all three roles must RESOLVE - a skipped branch would be a silent
// hole exactly where the degrade path hides
triAssertIncludes [glRoles, "Sniper|"]
triAssertIncludes [glRoles, "Commander|"]
triAssertIncludes [glRoles, "Tank Commander|"]

// -- the bodies: a class this faction can field, on the occupier's side ------
glBad = ""
glI = 0
while {glI < 3} do {glObj = gmLegendBody (glBoss select glI); glTag = format ["[%1]", glI]; if (isNull glObj) then {glBad = glBad + glTag + "nobody "}; if ((typeOf glObj) == "") then {glBad = glBad + glTag + "notype "}; if (not (gmClassExists (typeOf glObj))) then {glBad = glBad + glTag + "noclass=" + (typeOf glObj) + " "}; if ((format ["%1", side glObj]) != gmOccupierSide) then {glBad = glBad + glTag + "side=" + (format ["%1", side glObj]) + " "}; if ((getDammage glObj) >= 1) then {glBad = glBad + glTag + "dead "}; glI = glI + 1}
triAssertEq [glBad, ""]

// -- the stands. OUT OF THE WATER, at least 300 m off EVERY zone centre (the
//    presence radius is 150 m), and at least 350 m from each other. --------------
glPos0 = gmLegendPos (glBoss select 0)
glPos1 = gmLegendPos (glBoss select 1)
glPos2 = gmLegendPos (glBoss select 2)
triAssertEq [(count glPos0), 3]
glBad = ""
glI = 0
while {glI < 3} do {glP = gmLegendPos (glBoss select glI); glTag = format ["[%1]", glI]; if (((glP select 0) == 0) and ((glP select 1) == 0)) then {glBad = glBad + glTag + "nostand "}; if ((glP select 2) < 0.1) then {glBad = glBad + glTag + "underwater "}; glZi = 0; while {glZi < gmZoneCount} do {glZp = (gmZone glZi) select 8; glD = sqrt ((((glP select 0) - (glZp select 0)) * ((glP select 0) - (glZp select 0))) + (((glP select 1) - (glZp select 1)) * ((glP select 1) - (glZp select 1)))); if (glD < 300) then {glBad = glBad + glTag + "zone" + (format ["%1", glZi]) + "@" + (format ["%1", glD]) + " "}; glZi = glZi + 1}; glI = glI + 1}
triAssertEq [glBad, ""]
glD01 = sqrt ((((glPos0 select 0) - (glPos1 select 0)) * ((glPos0 select 0) - (glPos1 select 0))) + (((glPos0 select 1) - (glPos1 select 1)) * ((glPos0 select 1) - (glPos1 select 1))))
glD02 = sqrt ((((glPos0 select 0) - (glPos2 select 0)) * ((glPos0 select 0) - (glPos2 select 0))) + (((glPos0 select 1) - (glPos2 select 1)) * ((glPos0 select 1) - (glPos2 select 1))))
glD12 = sqrt ((((glPos1 select 0) - (glPos2 select 0)) * ((glPos1 select 0) - (glPos2 select 0))) + (((glPos1 select 1) - (glPos2 select 1)) * ((glPos1 select 1) - (glPos2 select 1))))
triAssertGe [glD01, 350]
triAssertGe [glD02, 350]
triAssertGe [glD12, 350]

// -- the body really is where the row says it is (the row is re-read from the
//    body after createUnit's own free-position search, so these agree to a
//    stride, not to a metre) ----------------------------------------------------
glBad = ""
glI = 0
while {glI < 3} do {glP = gmLegendPos (glBoss select glI); glB = getPos (gmLegendBody (glBoss select glI)); glD = sqrt ((((glP select 0) - (glB select 0)) * ((glP select 0) - (glB select 0))) + (((glP select 1) - (glB select 1)) * ((glP select 1) - (glB select 1)))); if (glD > 30) then {glBad = glBad + (format ["[%1]drift=%2 ", glI, glD])}; glI = glI + 1}
triAssertEq [glBad, ""]

// -- the DOSSIER, the MARKER and the OBJECTIVE all read one row --------------
glBad = ""
glI = 0
while {glI < 3} do {glRow = gmLegendInfo (glBoss select glI); glMk = gmLegendMarker (glBoss select glI); glTag = format ["[%1]", glRow select 0]; if ((count glMk) != 4) then {glBad = glBad + glTag + "nomarker "}; if ((count glMk) == 4) then {if ((glMk select 0) != ("gmLegend_" + (glRow select 0))) then {glBad = glBad + glTag + "mname=" + (glMk select 0) + " "}; if ((glMk select 1) != "Warning") then {glBad = glBad + glTag + "mtype=" + (glMk select 1) + " "}; if ((glMk select 2) != "ColorRed") then {glBad = glBad + glTag + "mcolor=" + (glMk select 2) + " "}; if ((glMk select 3) != ((glRow select 1) + ", " + (glRow select 3))) then {glBad = glBad + glTag + "mtext=" + (glMk select 3) + " "}}; if ((gmJournalObjectiveState ("legend_" + (glRow select 0))) != "ACTIVE") then {glBad = glBad + glTag + "obj=" + (gmJournalObjectiveState ("legend_" + (glRow select 0))) + " "}; if (gmLegendDefeated (glBoss select glI)) then {glBad = glBad + glTag + "defeated "}; glI = glI + 1}
triAssertEq [glBad, ""]

// the dossier's zone name is the zone he was PLACED to watch, and the stand is
// on that zone's annulus: this is what keeps the map, the Plan page and the
// People page from ever naming three different places
glBad = ""
glI = 0
while {glI < 3} do {glRow = gmLegendInfo (glBoss select glI); glZi = gmZoneIndex (glRow select 4); glTag = format ["[%1]", glRow select 0]; if (glZi < 0) then {glBad = glBad + glTag + "zone?" + (glRow select 4) + " "}; if (glZi >= 0) then {glP = gmLegendPos (glBoss select glI); glZp = (gmZone glZi) select 8; glD = sqrt ((((glP select 0) - (glZp select 0)) * ((glP select 0) - (glZp select 0))) + (((glP select 1) - (glZp select 1)) * ((glP select 1) - (glZp select 1)))); if (glD > 1000) then {glBad = glBad + glTag + "far=" + (format ["%1", glD]) + " "}; if (glD < 340) then {glBad = glBad + glTag + "near=" + (format ["%1", glD]) + " "}}; glI = glI + 1}
triAssertEq [glBad, ""]

// -- THE PIN. DAMove (bit 2) and nothing else: DATarget would stop target
//    assignment, DAAutoTarget his own acquisition and DAAnim his stance, and
//    the whole point of this commander is that he holds his ground and still
//    shoots back. -----------------------------------------------------------------
glB0 = gmLegendBody (glBoss select 0)
glB1 = gmLegendBody (glBoss select 1)
glB2 = gmLegendBody (glBoss select 2)
triAssertEq [(triUnitAIDisabled "glB0"), 2]
triAssertEq [(triUnitAIDisabled "glB1"), 2]
triAssertEq [(triUnitAIDisabled "glB2"), 2]

// -- ISOLATION. GarrisonCache only ever touches groups it created and pushed
//    into its own state, and qrf.sqs only reads GM_QRF_GROUP, so a group
//    registered with neither is unreachable by both - by construction, which
//    this asserts rather than assumes. Boss groups are deliberately NOT hidden
//    from the undercover layer (a Legend's bodyguards blowing the player's
//    cover is fair play), but they must not manufacture witnesses on their own
//    from 900 m away either. -------------------------------------------------------
glGarUnits = []
glZi = 0
while {glZi < gmZoneCount} do {"glGarUnits = glGarUnits + (units _x)" forEach (gmGarrisonGroups glZi); glZi = glZi + 1}
glBad = ""
glI = 0
while {glI < 3} do {glProbe = gmLegendBody (glBoss select glI); glHit = 0; "if (_x == glProbe) then {glHit = glHit + 1}" forEach glGarUnits; if (glHit > 0) then {glBad = glBad + (format ["[%1]ingarrison ", glI])}; if (not (isNil "GM_QRF_GROUP")) then {if ((group glProbe) == GM_QRF_GROUP) then {glBad = glBad + (format ["[%1]isqrf ", glI])}}; glI = glI + 1}
triAssertEq [glBad, ""]
triAssertEq [gmUndercoverWitnesses, 0]

// -- CAPTURE IS STILL POSSIBLE WHILE HE LIVES. This is the decision the whole
//    ring geometry exists to make true: taking the outpost and killing its
//    commander are two objectives in either order. The garrison is cleared the
//    way guerrilla_native_capture clears it (the capture predicate reads live
//    bodies inside zoneArea plus the despawned garrison count), and then the
//    meter has to run all the way to a FLIP with all three commanders alive.
//    THE ATTACKERS ARE SPAWNED, NOT THE PLAYER: this mission's undercover
//    manager sets gmUndercover at boot and keeps it for the whole campaign,
//    and an undercover player is deliberately excluded from his side's zone
//    presence (ZoneRegistry::GatherInputs), so a lone player can never move
//    this meter here. A resistance fire team standing in the zone can.
glOut = gmZoneIndex "Outpost"
triAssertGe [glOut, 0]
glOutPos = (gmZone glOut) select 8
triSimUntil { gmGarrisonSpawned glOut }
triSimUntil { (gmGarrisonLive glOut) >= 6 }
glGarrison = []
"glGarrison = glGarrison + (units _x)" forEach (gmGarrisonGroups glOut)
{deleteVehicle _x} forEach glGarrison
gmGarrisonForceDespawn glOut
gmZoneSet [glOut, "garrison", 0]
triSimFrames 10
gmGarrisonForceDespawn glOut
gmZoneSet [glOut, "garrison", 0]
glAtk = [gmResistanceSide call GM_fnSideFromString, gmFactionTierClass [gmResistanceSide, 1], 3, glOutPos] call GM_fnSpawnGroup
triSimUntil { ((gmZone glOut) select 9) > 0 }
triAssertEq [(gmLegendDefeated (glBoss select 0)), false]
triSimUntil { ((gmZone glOut) select 2) == gmResistanceSide }

// -- ... AND NOTHING MOVED. Placement is resolved once and never recomputed,
//    so a captured zone leaves the commander standing where he was, still
//    marked, still an objective: a hostile pocket behind the line. -------------
glT0 = time
triSimUntil { time > glT0 + 6 }
glBad = ""
glI = 0
while {glI < 3} do {glP = gmLegendPos (glBoss select glI); glWas = glPos0; if (glI == 1) then {glWas = glPos1}; if (glI == 2) then {glWas = glPos2}; if (((glP select 0) != (glWas select 0)) or ((glP select 1) != (glWas select 1))) then {glBad = glBad + (format ["[%1]moved ", glI])}; glMk = gmLegendMarker (glBoss select glI); if ((glMk select 2) != "ColorRed") then {glBad = glBad + (format ["[%1]repainted ", glI])}; if ((gmJournalObjectiveState ("legend_" + ((gmLegendInfo (glBoss select glI)) select 0))) != "ACTIVE") then {glBad = glBad + (format ["[%1]objgone ", glI])}; glI = glI + 1}
triAssertEq [glBad, ""]
triAssertEq [gmLegendCount, 4]

triEndTest
