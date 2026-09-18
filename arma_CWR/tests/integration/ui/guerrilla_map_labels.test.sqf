// ============================================================================
//  Map labels must not overprint each other (UI/Map/MapLabelLayout.hpp).
//    Every Guerrilla zone marker sits on the Names anchor of the town it
//    seeds, so the island's own name and the zone label ("Houdan" under
//    "Houdan - support 20/60") used to share one baseline, and neighbouring
//    zone labels had no thinning at all.  The engine now lets marker labels
//    claim their screen space first: a town name under a zone label yields,
//    a name under a bare (unrevealed) flag slides to the flag's right edge,
//    and a later marker label under an earlier one is thinned for the frame.
//
//    Only the thinning is SQF-observable (triMapHiddenLabels); the name
//    handling is captured for eyeballing: 01 at the default zoom around
//    Houdan (Village revealed, seeded towns with support text), 02 zoomed
//    out until zone labels collide, 03 zoomed all the way in.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { gmZoneCount >= 4 }

// stand on Houdan: Village/Camp/Outpost and the towns within revealRadius
// reveal on the next zone ticks; give two seeded towns a support line so
// their labels carry the long suffix
mlVillage = gmZoneIndex "Village"
mlPos = (gmZone mlVillage) select GM_Z_POS
player setPos [mlPos select 0, mlPos select 1, 0]
mlDourdan = gmZoneIndex "Dourdan"
mlArudy = gmZoneIndex "Arudy"
triAssertGt [mlDourdan, -1]
triAssertGt [mlArudy, -1]
gmZoneSet [mlDourdan, "support", 40]
gmZoneSet [mlArudy, "support", 45]
triSimUntil { time > 8 }

triAssertEq [triOpenMap, "OK"]
triShowMap 1
triSimFrames 10
mlScale = triMapGetScale
triAssertGt [mlScale, 0]
triScreenshot "01_map_labels_default"

// zoomed out far enough that the three hand-authored zones (500 m apart)
// and the seeded towns run into each other: at least one label is thinned
triMapSetScale (mlScale * 6)
triSimFrames 10
mlHiddenOut = triMapHiddenLabels
triAssertGt [mlHiddenOut, 0]
triScreenshot "02_map_labels_zoomed_out"

// zoomed in nothing collides: every marker keeps its label (a jump to the
// map's minimum scale stalls the draw for tens of seconds - stock behaviour,
// the label pass is per-marker - so zoom in moderately)
triMapSetScale (mlScale / 4)
triSimFrames 10
triAssertEq [triMapHiddenLabels, 0]
triScreenshot "03_map_labels_zoomed_in"

triMapSetScale mlScale
triSimFrames 5
triEndTest
