// ============================================================================
//  Generated-base props across a save (OCCUPIER-FORCES.md 5.7 step 2),
//  phase 2 of 2: a fresh process boots Abel (generating and building again,
//  from a new seed), loads the phase 1 save and finds the saved props
//  re-bound: the same rows in order at the same spots, every one standing,
//  none created in this session (made 0), the same world count of layout
//  objects around each saved base (nothing duplicated), and the count still
//  holds after two re-create passes.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
triAssertEq [(triLoadGame "props57"), "OK"]
triSimFrames 5
// the world count of layout objects around each saved base: [name, x, z, count]
psFnWorld = {psWL = []; {psWC = _x select 2; psWL = psWL + [[_x select 0, psWC select 0, psWC select 1, count (nearestObjects [[psWC select 0, psWC select 1, 0], psClasses, 60])]]} forEach psGen; psWL}
prProps = gmBaseProps
triAssertEq [(count prProps), (count psRows)]
prBad = ""
prK = 0
while {prK < (count psRows)} do {prS = psRows select prK; prL = prProps select prK; if (((prL select 0) != (prS select 0)) or ((prL select 1) != (prS select 1))) then {prBad = prBad + format ["[row %1 %2 %3, saved %4 %5]", prK, prL select 0, prL select 1, prS select 0, prS select 1]}; if (not (prL select 4)) then {prBad = prBad + format ["[row %1]gone ", prK]}; if ((prL select 6) != 0) then {prBad = prBad + format ["[row %1]made %2 ", prK, prL select 6]}; if (([(prL select 2) select 0, (prL select 2) select 1, 0] distance [prS select 2, prS select 3, 0]) > 0.5) then {prBad = prBad + format ["[row %1]moved ", prK]}; prK = prK + 1}
triAssertEq [prBad, ""]
triAssertEq [(format ["%1", call psFnWorld]), (format ["%1", psWorld])]
prT0 = time
triSimUntil { time > prT0 + 12 }
prMade = 0
{prMade = prMade + (_x select 6)} forEach gmBaseProps
triAssertEq [prMade, 0]
triAssertEq [(count gmBaseProps), (count psRows)]
triAssertEq [(format ["%1", call psFnWorld]), (format ["%1", psWorld])]
triEndTest
