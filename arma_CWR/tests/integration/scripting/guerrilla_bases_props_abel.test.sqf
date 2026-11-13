// ============================================================================
//  Generated occupier bases, step 2: the outpost layout (OCCUPIER-FORCES.md
//  5.7, ISLAND-PACKS.md 11), Abel.  Both generated OUTPOSTs (La Riviere and
//  Vigny on the 2026-10-16 ground truth; names move with the seed) get one
//  layout of stock statics at new game.
//    p1  each generated base has 8 to 12 props in gmBaseProps, every one
//        standing, every one within 60 m (2D) of its base's centre; the
//        world agrees: nearestObjects of the layout classes within 60 m of
//        the centre counts the same objects (nothing duplicated).
//    p2  no prop has a road within 6 m (nearestRoads 6 is empty at it).
//    p3  no prop reaches into the 25 m yard (centre distance minus the
//        footprint radius is 25 m or more).
//    p4  each prop stands on the ground, neither floating nor buried: the
//        model's lowest vertex sits between 1.5 m under and 0.5 m over the
//        terrain under its origin (gmBaseProps "bottom").  Ground truth
//        2026-10-16: tents, wire, the flag and the fuel point read about 0;
//        Fortress1 and ViewTower read -1.03 to -1.06, their authored dug-in
//        foundations (a buried model reads its whole height under).
//    p5  an LZ probe around the outpost (OccupierAir's own pick, ring 40 to
//        250 m) lands 60 m or more from every prop.
//    p6  the player walks into the bubble of one outpost: its garrison is
//        released and every soldier stands outside every prop's box
//        (gmBasePropsInside is 0 at each).
//    p7  each outpost keeps its gate (step 3, the object-free footprint):
//        both gate bunkers (Fortress1, rows 0 and 1, base-local [-10, 40]
//        and [10, 40]) stand within 12 m of the gate point 40 m from the
//        centre towards the road.  Ground truth 2026-10-16: Vigny Outpost
//        lost both to trees; with the scorer preferring clear footprints
//        neither base drops them.
//    p8  the gate is passable: a soldier created 80 m out on the road side
//        and sent (doMove) to the yard walks through the gate and arrives
//        within 10 m of the centre.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
GM_COMP_ALIVE = [false]
GM_QRF_TICK = 999999
{deleteVehicle _x} forEach ((units group player) - [player])
player allowDammage false

bpGen = gmGeneratedBases
triAssertEq [(count bpGen), 2]
bpProps = gmBaseProps
bpClasses = ["ViewTower", "Fortress1", "Wire", "Camp", "CampEast", "FlagCarrier", "FuelStationNew2"]
// 2D distance: [[x, z], [x, z]] call bpFnD
bpFnD = {bpDA = _this select 0; bpDB = _this select 1; [bpDA select 0, bpDA select 1, 0] distance [bpDB select 0, bpDB select 1, 0]}

// ---- p1: 8 to 12 props per base, standing, within 60 m; the world agrees ---
bpBad = ""
bpB = 0
while {bpB < (count bpGen)} do {bpRow = bpGen select bpB; bpName = bpRow select 0; bpC = bpRow select 2; if ((bpRow select 1) != "OUTPOST") then {bpBad = bpBad + format ["[%1]type ", bpName]}; bpN = 0; bpK = 0; while {bpK < (count bpProps)} do {bpP = bpProps select bpK; if ((bpP select 0) == bpName) then {bpN = bpN + 1; if (not (bpP select 4)) then {bpBad = bpBad + format ["[%1 %2]gone ", bpName, bpP select 1]}; if (([bpP select 2, bpC] call bpFnD) > 60) then {bpBad = bpBad + format ["[%1 %2]far ", bpName, bpP select 1]}}; bpK = bpK + 1}; if (bpN < 8) then {bpBad = bpBad + format ["[%1]only %2 ", bpName, bpN]}; if (bpN > 12) then {bpBad = bpBad + format ["[%1]%2 props ", bpName, bpN]}; bpW = count (nearestObjects [[bpC select 0, bpC select 1, 0], bpClasses, 60]); if (bpW != bpN) then {bpBad = bpBad + format ["[%1]world %2 vs %3 ", bpName, bpW, bpN]}; bpB = bpB + 1}
triAssertEq [bpBad, ""]

// ---- p2 .. p4: the road margin, the yard, seated -----------------------------
bpBad = ""
bpK = 0
while {bpK < (count bpProps)} do {bpP = bpProps select bpK; bpPos = bpP select 2; bpBi = gmZoneIndex (bpP select 0); bpC = (gmZone bpBi) select 8; if ((count ([bpPos select 0, bpPos select 1, 0] nearestRoads 6)) > 0) then {bpBad = bpBad + format ["[%1 %2]road ", bpP select 0, bpP select 1]}; if ((([bpPos, bpC] call bpFnD) - (bpP select 5)) < 25) then {bpBad = bpBad + format ["[%1 %2]yard %3 ", bpP select 0, bpP select 1, ([bpPos, bpC] call bpFnD)]}; bpSeat = bpP select 8; if ((bpSeat < -1.5) or (bpSeat > 0.5)) then {bpBad = bpBad + format ["[%1 %2]seat %3 ", bpP select 0, bpP select 1, bpSeat]}; bpK = bpK + 1}
triAssertEq [bpBad, ""]

// ---- p5: an LZ probe near the outpost lands outside 60 m of the props -------
bpBad = ""
bpB = 0
while {bpB < (count bpGen)} do {bpName = (bpGen select bpB) select 0; bpC = (bpGen select bpB) select 2; bpLz = gmBasePropsLz [bpC select 0, bpC select 1, 40, 250]; if ((count bpLz) != 2) then {bpBad = bpBad + format ["[%1]nolz ", bpName]} else {bpK = 0; while {bpK < (count bpProps)} do {bpP = bpProps select bpK; bpDd = [bpLz, bpP select 2] call bpFnD; if (bpDd < 60) then {bpBad = bpBad + format ["[%1]lz %2 m from %3 ", bpName, bpDd, bpP select 1]}; bpK = bpK + 1}}; bpB = bpB + 1}
triAssertEq [bpBad, ""]

// ---- p6: a released garrison stands clear of every prop box ------------------
bpName = (bpGen select 0) select 0
bpC = (bpGen select 0) select 2
bpI = gmZoneIndex bpName
triAssertGe [bpI, 0]
player setPos [(bpC select 0) + 450, (bpC select 1) + 450, 0]
triSimUntil { gmGarrisonSpawned bpI }
triSimUntil { (gmGarrisonLive bpI) >= 4 }
bpUnits = []
"bpUnits = bpUnits + (units _x)" forEach (gmGarrisonGroups bpI)
triAssertGe [(count bpUnits), 4]
bpBad = ""
{bpU = getPos _x; if ((gmBasePropsInside [bpU select 0, bpU select 1]) > 0) then {bpBad = bpBad + format ["[%1 at %2]", _x, bpU]}} forEach bpUnits
triAssertEq [bpBad, ""]

// ---- p7: each outpost keeps its gate -------------------------------------------
bpBad = ""
{bpName = _x select 0; bpC = _x select 2; bpH = _x select 7; bpGate = [(bpC select 0) + 40 * (sin bpH), (bpC select 1) + 40 * (cos bpH)]; bpN = 0; {if (((_x select 0) == bpName) and ((_x select 1) == "Fortress1") and (([_x select 2, bpGate] call bpFnD) < 12)) then {bpN = bpN + 1}} forEach bpProps; if (bpN != 2) then {bpBad = bpBad + format ["[%1]gate %2 ", bpName, bpN]}} forEach bpGen
triAssertEq [bpBad, ""]

// ---- p8: the gate is passable --------------------------------------------------
// the other outpost (p6 woke the first one's garrison; the player stays by
// the first, so this one's garrison is still abstract)
bpRow = bpGen select 1
bpC = bpRow select 2
bpH = bpRow select 7
bpWalker = objNull
bpGrp = createGroup east
"SoldierEB" createUnit [[(bpC select 0) + 80 * (sin bpH), (bpC select 1) + 80 * (cos bpH), 0], bpGrp, "bpWalker = this", 0.5, "PRIVATE"]
triSimUntil { not (isNull bpWalker) }
bpWalker allowDammage false
bpWalker doMove [bpC select 0, bpC select 1, 0]
triSimUntil { ([getPos bpWalker, bpC] call bpFnD) < 10 }
triAssertLt [([getPos bpWalker, bpC] call bpFnD), 10]

triEndTest
