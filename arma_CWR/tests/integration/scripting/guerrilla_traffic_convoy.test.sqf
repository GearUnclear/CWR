// ============================================================================
//  Guerrilla Mode ambient road traffic - occupier supply CONVOY discipline
//  (the first rehearsal of the 2026-09-01 convoy work, issue #55).
//    gmTrafficForceSpawn ["convoy", Outpost] places the faction's truck rung
//    (vehicles[1]) on the road near the Outpost with the escort rung
//    (vehicles[0]) 14 m behind it, one occupier-side group for both crews
//    (truck driver + cargo; escort driver + gunner + up to 2 riflemen),
//    SAFE behaviour, SpeedLimited, FormColumn on the drivers' own subgroup
//    so the engine's native convoy-follow engages: the escort TAILS the truck
//    down the road instead of racing it.
//  Quiet loss: an escort destroyed with nothing disclosed (no shot fired at
//  the group; a setDammage has no enemy owner, so Landscape::Disclose never
//  reaches the truck's group) is NOT a rout - the escort hull is released to
//  the roadside table, the truck drives on unescorted as a live convoy
//  entry, and no "bailed" event fires.  (The bail-under-fire branch needs a
//  disclosed group and is left to the human playtest.)
//  Staging mirrors guerrilla_traffic_patrol: the Camp is flipped to the
//  occupier first so the Outpost has a destination; the player stands at the
//  Village (~540 m from the Outpost) so the Outpost's roads sit inside the
//  spawn band.
// ============================================================================

triSimUntil { GM_LIB_READY }

gtBailed = []
gmTrafficOnEvent ["bailed", {gtBailed = gtBailed + [_this]}]

gtOut = gmZoneIndex "Outpost"
gtCamp = gmZoneIndex "Camp"
gtVil = gmZoneIndex "Village"
triAssertGe [gtOut, 0]
triAssertGe [gtCamp, 0]
triAssertGe [gtVil, 0]
gmZoneSet [gtCamp, "owner", gmOccupierSide]
triAssertEq [(gmZone gtCamp) select 2, gmOccupierSide]

gtPos = (gmZone gtVil) select 8
player setPos [gtPos select 0, gtPos select 1, 0]
triSimFrames 5

// -- spawn: truck + escort, one entry ------------------------------------------
gtTruck = gmTrafficForceSpawn ["convoy", gtOut]
triAssertEq [(format ["%1", isNull gtTruck]), "false"]
gtEsc = gmTrafficEscort gtTruck
triAssertEq [(format ["%1", isNull gtEsc]), "false"]
triAssertEq [gmTrafficCount "convoy", 1]
gtInfo = gmTrafficInfo gtTruck
triAssertEq [gtInfo select 0, "convoy"]
triAssertEq [gtInfo select 1, gtOut]
triAssertEq [gtInfo select 2, gtCamp]
// the escort answers to the same entry
triAssertEq [(gmTrafficInfo gtEsc) select 0, "convoy"]

// -- occupier crews, one group, riflemen aboard the escort ---------------------
triAssertEq [(format ["%1", isNull (driver gtTruck)]), "false"]
triAssertEq [(format ["%1", isNull (driver gtEsc)]), "false"]
triAssertEq [(format ["%1", side (driver gtTruck)]), gmOccupierSide]
triAssertEq [(format ["%1", side (driver gtEsc)]), gmOccupierSide]
triAssertEq [(format ["%1", (group (driver gtTruck)) == (group (driver gtEsc))]), "true"]
triAssertGe [count (crew gtEsc), 2]
// column spacing at spawn: the escort sits behind the truck
triAssertGe [30, gtEsc distance gtTruck]

// -- both hulls drive -------------------------------------------------------------
gtP0 = getPos gtTruck
gtE0 = getPos gtEsc
triSimUntil { ((getPos gtTruck) distance gtP0) >= 15 }
triSimUntil { ((getPos gtEsc) distance gtE0) >= 15 }

// -- column discipline: after 30 s of road the escort still tails the truck
//    (a stuck or racing escort would be hundreds of metres off by now) --------
gtT0 = time
triSimUntil { time > gtT0 + 30 }
triAssertGe [120, gtEsc distance gtTruck]
triAssertEq [(format ["%1", gtTruck in gmTrafficVehicles]), "true"]

// -- quiet escort loss: released, no rout ----------------------------------------
gtEsc setDammage 1
triSimUntil { isNull (gmTrafficEscort gtTruck) }
triAssertEq [(format ["%1", gtTruck in gmTrafficVehicles]), "true"]
triAssertEq [(gmTrafficInfo gtTruck) select 0, "convoy"]
triAssertEq [(format ["%1", alive (driver gtTruck)]), "true"]
triAssertEq [count gtBailed, 0]
// the wreck joined the roadside table (gmTrafficDiag: [.., released, ..])
triAssertGe [(gmTrafficDiag) select 4, 1]

triEndTest
