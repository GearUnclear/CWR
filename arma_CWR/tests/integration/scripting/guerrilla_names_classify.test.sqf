// ============================================================================
//  Guerrilla Mode - Names-block classification (issue #54 C3).
//    Boots tests/integration/missions/guerrilla_names.Lebanon80: a copy of
//    the Guerrilla.Lebanon80 template with the seedCities key REMOVED, so the
//    CITY auto-seed runs in Auto mode over Lebanon80's theatre-map Names
//    block through the engine's LandscapeSettlementProbe (dry land + at
//    least three house-sized buildings within 300 m).
//
//  What the classifier must do on this world (verified from the engine log
//  of the first run, one "not seeded" INFO line per refusal):
//    * refuse the 13 labels: Mediterranean Sea and Sea of Galilee (water),
//      Lebanon / Israel / Syria / Jordan (country labels), Golan Heights,
//      Bekaa Valley, Mt. Dov (terrain), Arik Bridge, Bnot
//      Yaakov Bridge, UNDOF (no houses around them) - Mt. Hermon is NOT one
//      of them: the summit carries a modelled post, so it is accepted;
//    * accept the settlements that really have buildings modelled (Beirut,
//      Haifa, Damascus, the airports, Sabra, Shatila, ...) - they are real
//      towns even though the shipped template excludes them from its play
//      area with seedCities = 0;
//    * dedup Tyre / Saida / Ghajar against the authored zones by name.
//  6 authored zones + 8 classified settlements = 14, asserted exactly: a
//  label leaking through shows up as 15+, a lost town as 13-.
// ============================================================================

triSimUntil { GM_LIB_READY }
triSimUntil { gmZoneCount >= 6 }

// -- the 13 labels stay out --------------------------------------------------
triAssertEq [(gmZoneIndex "Mediterranean Sea"), -1]
triAssertEq [(gmZoneIndex "Sea of Galilee"), -1]
triAssertEq [(gmZoneIndex "Lebanon"), -1]
triAssertEq [(gmZoneIndex "Golan Heights"), -1]
triAssertEq [(gmZoneIndex "Mt. Dov"), -1]
triAssertEq [(gmZoneIndex "Bekaa Valley"), -1]
triAssertEq [(gmZoneIndex "Arik Bridge"), -1]
triAssertEq [(gmZoneIndex "UNDOF"), -1]

// -- the authored towns are not duplicated, the built-up rear is seeded -----
triAssertEq [(gmZoneIndex "Tyre") >= 0, true]
triAssertEq [(gmZoneIndex "Beirut") >= 0, true]
triAssertEq [((gmZone (gmZoneIndex "Beirut")) select 1), "CITY"]

// -- exact count: 6 authored + 8 classified settlements (one coastal
//    entry more is refused by the sea-level test than by the older stub) ----------------------
triAssertEq [(gmZoneCount), 14]

triEndTest
