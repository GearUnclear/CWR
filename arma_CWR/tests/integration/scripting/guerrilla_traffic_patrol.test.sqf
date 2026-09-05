// ============================================================================
//  Guerrilla Mode ambient road traffic - occupier patrol vehicles between
//  occupier-held zones.  guerrilla_native.abel ships ONE occupier zone (the
//  Outpost), so the Camp is flipped to the occupier first to give the route a
//  destination; then gmTrafficForceSpawn ["patrol", Outpost] places the
//  faction's light hull (vehicles[0]) on the road near the Outpost with an
//  occupier-side crew (driver + gunner when the hull has a turret), SAFE
//  behaviour (still road pathing), registered as kind "patrol".
// ============================================================================

triSimUntil { GM_LIB_READY }

gtOut = gmZoneIndex "Outpost"
gtCamp = gmZoneIndex "Camp"
gtVil = gmZoneIndex "Village"
triAssertGe [gtOut, 0]
triAssertGe [gtCamp, 0]
triAssertGe [gtVil, 0]
gmZoneSet [gtCamp, "owner", gmOccupierSide]
triAssertEq [(gmZone gtCamp) select 2, gmOccupierSide]

// -- player at the Village: ~540 m from the Outpost, so the Outpost's road net
//    sits inside the spawn band --------------------------------------------------
gtPos = (gmZone gtVil) select 8
player setPos [gtPos select 0, gtPos select 1, 0]
triSimFrames 5

gtCar = gmTrafficForceSpawn ["patrol", gtOut]
triAssertEq [(format ["%1", isNull gtCar]), "false"]

// -- on the road ----------------------------------------------------------------
gtRoad = gmRoadNearest (getPos gtCar)
triAssertGe [count gtRoad, 3]
gtDx = ((getPos gtCar) select 0) - (gtRoad select 0)
gtDy = ((getPos gtCar) select 1) - (gtRoad select 1)
triAssertGe [10, sqrt ((gtDx * gtDx) + (gtDy * gtDy))]

// -- occupier crew, registered as a patrol -------------------------------------
triAssertEq [(format ["%1", isNull (driver gtCar)]), "false"]
triAssertEq [(format ["%1", side (driver gtCar)]), gmOccupierSide]
triAssertEq [gmTrafficCount "patrol", 1]
gtInfo = gmTrafficInfo gtCar
triAssertEq [gtInfo select 0, "patrol"]
triAssertEq [gtInfo select 1, gtOut]
triAssertEq [gtInfo select 2, gtCamp]
triAssertGe [count gmEvtTrSpawned, 1]
triAssertEq [((gmEvtTrSpawned select 0) select 1), "patrol"]

// -- it drives ------------------------------------------------------------------
gtP0 = getPos gtCar
triSimUntil { ((getPos gtCar) distance gtP0) >= 15 }

triEndTest
