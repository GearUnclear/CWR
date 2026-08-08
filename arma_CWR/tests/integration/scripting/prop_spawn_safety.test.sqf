// Regression: createVehicle of a third-party static scenery prop must never take
// the process down. Guerrilla Mode's premise is mounting arbitrary island/faction
// packs, so a 2005-era model or a partially resolved class has to degrade to
// objNull and a log line, never to an access violation.
//
// Two crashes this pins, both reproduced with @LoBo's wreck props:
//   * Building::Building sized _locks from NPos(), which reads
//     BuildingType::_positions through a static_cast of the type object. A class
//     whose owner addon was denied (or whose scope is <= 0) loads as a bare
//     EntityAIType, so that read returned garbage and AutoArray::Resize memmove'd
//     an arbitrary range - 0xC0000005 inside VCRUNTIME memmove.
//   * Building::DrawProxies dereferenced WeaponProxy::obj after IsPresent() said
//     the proxy was present; obj is a loose link that reads back null once its
//     target is released.
//
// And the config plumbing they exposed: the @propsafety fixture's preload roster
// has to reach World::ActivateAddons even though it is mounted after another
// config-bearing mod, and even though the stock config locks CfgAddons.
//
// PRECONDITIONS: full CWA 1.99 + @LoBo, and the one-time @LoBo repairs
// (tools/lobo/fix-lobo-scope.ps1, plus
// tests/fixtures/mods-lobo/@lobofixup/gen-patched-pbos.ps1 if @LoBo's ammo pbos
// still carry the "0.0.1" tracer floats).
triSimUntil { GM_LIB_READY }
triSimUntil { alive player }

// -- every mounted mod's preload roster took effect ---------------------------
//    map_editorupgrade + lobowreck come from @propsafety (mounted LAST, the slot
//    the single-slot deferred merge used to discard); lobolebobject comes from
//    @lobofixup (the slot that used to win). gwbuild1 is named by nobody.
triAssertEq [(format ["%1 || %2 || %3", (triAddonActive "map_editorupgrade"), (triAddonActive "lobowreck"), (triAddonActive "lobolebobject")]), "true || true || true"]
triAssertEq [(format ["%1", (triAddonActive "gwbuild1")]), "false"]

psPos = getPos player

// -- CASE 1: owner addon not activated ----------------------------------------
//    CheckAccessCreate deliberately continues past a denial (VehicleTypes.cpp),
//    so what comes back is data-dependent: GWRubble inherits a positive scope
//    from a visible base and still builds, other classes resolve abstract and
//    yield objNull. Either is fine - what is NOT fine is a partially resolved
//    class reaching a constructor that static_casts its type. Spawn it, DRAW it,
//    tear it down, and require the process to still be here afterwards.
psDenied = "GWRubble" createVehicle psPos
triSimFrames 20
deleteVehicle psDenied
triSimFrames 4

// -- CASE 2: owner activated but the class is scope<=0 (abstract) -------------
//    This is the one that crashed: an abstract type is not the BuildingType the
//    "house" simulation constructor static_casts it to, so Building::Building
//    sized _locks from whatever sat where _positions would be.
//
//    "Strategic" is the stock CfgVehicles base (Strategic: Building: Static:
//    All) - it inherits scope 0 from All and simulation "house" from Static, so
//    it hits exactly that path and, unlike a mod class, it cannot be repaired
//    out from under this assertion. It used to be LoBo_uralwreck01, which was
//    only abstract because @LoBo's LoBoWreck.pbo omitted the #define public 2
//    header its sibling configs carry; that is fixed at source now
//    (tools/lobo/fix-lobo-scope.ps1) and CASE 2b below pins the fix.
psAbstract = "Strategic" createVehicle psPos
triAssertEq [(format ["%1", isNull psAbstract]), "true"]
// and the objNull the caller is handed survives the usual follow-up calls
psAbstract setDir 90
psAbstract setPos psPos
deleteVehicle psAbstract
triSimFrames 4

// -- CASE 2b: the repaired LoBoWreck / LoBoPalObj classes are createable ------
//    Regression on the content fix, not on the engine. All ten LoBoWreck
//    CfgVehicles classes and LoBoPalObj's LoBo_Poster_01 wrote `scope = public;`
//    in a file that never defined `public`, so the config reader stored the
//    string, scope read back as 0, and every one of them was refused with
//    "Cannot create '<class>': type is abstract". A red here means
//    tools/lobo/fix-lobo-scope.ps1 has not been run against this @LoBo install
//    (or that @LoBo was reinstalled over the repair).
psWreck = "LoBo_uralwreck01" createVehicle psPos
triAssertEq [(typeOf psWreck), "LoBo_uralwreck01"]
triSimFrames 8
deleteVehicle psWreck

psWreck = "LoBo_M60A1_wreck" createVehicle psPos
triAssertEq [(typeOf psWreck), "LoBo_M60A1_wreck"]
deleteVehicle psWreck

psWreck = "LoBo_BTR60wreck1" createVehicle psPos
triAssertEq [(typeOf psWreck), "LoBo_BTR60wreck1"]
deleteVehicle psWreck

psWreck = "LoBo_Shot_Wreck1" createVehicle psPos
triAssertEq [(typeOf psWreck), "LoBo_Shot_Wreck1"]
deleteVehicle psWreck

//    LoBo_t54wrck is the base every other LoBoWreck class inherits from, so it
//    carried the defect for all of them; the posters inherit theirs the same way
//    from LoBo_Poster_01 in a second pbo with the same missing header.
psWreck = "LoBo_t54wrck" createVehicle psPos
triAssertEq [(typeOf psWreck), "LoBo_t54wrck"]
triSimFrames 8
deleteVehicle psWreck

psWreck = "LoBo_Poster_01" createVehicle psPos
triAssertEq [(typeOf psWreck), "LoBo_Poster_01"]
triSimFrames 8
deleteVehicle psWreck
triSimFrames 4

// -- CASE 3: a well-formed prop from a mod-activated addon still works --------
//    Placed at the player so it is inside the frustum and actually drawn: the
//    DrawProxies guard only means anything if the renderer reaches it.
psProp = "MAP_Barrel1" createVehicle [(psPos select 0) + 4, (psPos select 1) + 4, 0]
triAssertEq [(format ["%1", isNull psProp]), "false"]
triAssertEq [(typeOf psProp), "MAP_Barrel1"]
triSimFrames 30
deleteVehicle psProp
triSimFrames 4

// -- the world is still simulating and the player is still alive --------------
psF0 = triFrameCount
triSimUntil { triFrameCount > psF0 + 30 }
triAssertEq [(format ["%1", alive player]), "true"]

triEndTest
