// ============================================================================
//  Point-blank AI combat — regression for the "glass" bug: with the preloaded
//  black texture unavailable (1.99 data, no Remaster CfgPreloadTextures),
//  every bullet-vs-man collision compared equal to the null glass texture and
//  was skipped as "shoot-through glass": soldiers at any range soaked entire
//  magazines with only component chip damage ("we can't hurt each other").
//
//  Two hostile AI soldiers are spawned 4 m apart and allowed to engage
//  through the normal AI fire loop (real CalculateAimWeapon aim — NOT the
//  `fire` command, whose no-target force-fire deliberately discharges
//  skyward, dir[1]=10 in AimWeaponForceFire).
//
//  Pass = real damage lands at point-blank range on either combatant.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

// stage the duel ~80 m from the player so neither AI engages him instead
udDuelX = (getPos player select 0) + 80
udDuelY = getPos player select 1

// resistance vs occupier: the two sides this mission's AI centers exist for
// (createGroup west returns grpNull here - no WEST center in the mission)
udGrpW = createGroup resistance
"SoldierGB" createUnit [[udDuelX, udDuelY, 0], udGrpW, "udW = this", 0.5, "PRIVATE"]
triSimUntil { !(isNil "udW") }
triSimUntil { alive udW }

udGrpE = createGroup east
"SoldierEB" createUnit [[udDuelX + 4, udDuelY, 0], udGrpE, "udE = this", 0.5, "PRIVATE"]
triSimUntil { !(isNil "udE") }
triSimUntil { alive udE }

// make sure both know about each other and are willing to shoot
udW setBehaviour "COMBAT"
udE setBehaviour "COMBAT"
udGrpW setCombatMode "RED"
udGrpE setCombatMode "RED"
udW reveal udE
udE reveal udW

// real body damage on either duelist (chip damage from the broken glass
// path only ever crept up in hundredths — 0.35 discriminates)
triSimUntil { ((getDammage udW) > 0.35) || ((getDammage udE) > 0.35) }

triEndTest
