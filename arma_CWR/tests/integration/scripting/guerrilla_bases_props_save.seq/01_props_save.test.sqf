// ============================================================================
//  Generated-base props across a save (OCCUPIER-FORCES.md 5.7 step 2),
//  phase 1 of 2: Guerrilla.Abel builds its outposts' layouts
//  (guerrilla_bases_props_abel has the layout rules).  One prop is deleted
//  and must come back exactly once at its planned spot within a re-create
//  pass (BaseProps::TickInterval, 5 s).  The expected rows ride the save as
//  script globals (ps*), so phase 2 compares against what this phase saw.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { ((gmOccupierForces) select 2) != "OFF" }
psClasses = ["ViewTower", "Fortress1", "Wire", "Camp", "CampEast", "FlagCarrier", "FuelStationNew2"]
psGen = gmGeneratedBases
triAssertEq [(count psGen), 2]
psProps = gmBaseProps
triAssertGe [(count psProps), 16]
triAssertLe [(count psProps), 24]
// the world count of layout objects around each base: [name, x, z, count]
psFnWorld = {psWL = []; {psWC = _x select 2; psWL = psWL + [[_x select 0, psWC select 0, psWC select 1, count (nearestObjects [[psWC select 0, psWC select 1, 0], psClasses, 60])]]} forEach psGen; psWL}
psWorld0 = call psFnWorld

// ---- delete one prop: it comes back once, at its spot ----------------------
psVictim = psProps select 0
psVPos = psVictim select 2
deleteVehicle (psVictim select 7)
triSimFrames 2
triAssertEq [(((gmBaseProps) select 0) select 4), false]
triSimUntil { ((gmBaseProps) select 0) select 4 }
psBack = (gmBaseProps) select 0
triAssertEq [(psBack select 6), 2]
triAssertLe [([(psBack select 2) select 0, (psBack select 2) select 1, 0] distance [psVPos select 0, psVPos select 1, 0]), 0.5]
psT0 = time
triSimUntil { time > psT0 + 12 }
psMade = 0
{psMade = psMade + (_x select 6)} forEach gmBaseProps
triAssertEq [psMade, (count psProps) + 1]
triAssertEq [(format ["%1", call psFnWorld]), (format ["%1", psWorld0])]

// ---- what phase 2 must find --------------------------------------------------
psRows = []
{psRows = psRows + [[_x select 0, _x select 1, (_x select 2) select 0, (_x select 2) select 1]]} forEach gmBaseProps
psWorld = call psFnWorld
triAssertEq [(triSaveGame "props57"), "OK"]
triEndTest
