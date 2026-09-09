// ============================================================================
//  Portrait-shoot verbs on Classic 1.99 data.
//    The Guerrilla journal dossier reserves a portrait box and names a
//    `<bodyClass>__<face>.paa` for it. Producing that catalogue is a dev-time
//    shoot, and the shoot needs four seams that did not exist:
//      * triListFaces      - which CfgFaces tokens the loaded package will
//                            actually accept on a MAN body. The four
//                            LegendRegistry::kPortraitFaces tokens are a
//                            design guess until this lane runs them against
//                            real data.
//      * triSetUnitFaceView- the repeatable chest-up rig (EffectStandStill +
//                            mimic Default + Man::CalculateCameraPosition) for
//                            an ARBITRARY spawned unit. Before this the rig was
//                            reachable only through triSetRoleFaceView, i.e.
//                            only for a live multiplayer player.
//      * triFaceTexture    - the measurement that proves setFace took.
//                            Head::SetFace fails silently twice over: an
//                            unknown token degrades to "Default", and a
//                            woman/man mismatch simply returns. A shoot that
//                            trusts the setFace call photographs 128 identical
//                            heads and never notices.
//      * triCaptureAtPresent - capture the PRESENTED frame (post gamma pass,
//                            post overlay) instead of the mid-frame default.
//                            Default gamma is 1.2, so the two differ in tone.
//    Windowed 800x600: nothing here reads pixels, it only proves the file is
//    written, so resolution buys nothing.
// ============================================================================

triSimFrames 10

// -- triListFaces: non-empty, and every kPortraitFaces token is in it ---------
pvFaces = triListFaces
triAssertGt [(count pvFaces), 0]
pvMissing = ""
{ if (!(_x in pvFaces)) then { pvMissing = pvMissing + _x + " " } } forEach ["Face10", "Face18", "Face27", "Face33"]
triAssertEq [pvMissing, ""]

// -- a spawned unit, on a created centre (createGroup returns grpNull for a
//    side with no centre, and the units then silently never appear) -----------
createCenter west
pvGrp = createGroup west
"SoldierWB" createUnit [[(getPos player select 0) + 8, (getPos player select 1), 0], pvGrp, "pvUnit = this", 0.5, "PRIVATE"]
triSimUntil { not (isNil "pvUnit") }
pvUnit setCaptive true
pvUnit disableAI "MOVE"
pvUnit disableAI "AUTOTARGET"
pvUnit setBehaviour "CARELESS"
triSimFrames 5

// -- triSetUnitFaceView frames a unit with no role and no player -------------
triAssertEq [(triSetUnitFaceView [pvUnit, 2.0]), "OK"]
triAssertEq [(triSetUnitFaceView [objNull, 2.0]), "FAIL:no_object"]

// -- triFaceTexture reports a real texture, and setFace changes it ------------
pvUnit setFace "Face27"
triSimFrames 5
pvTex27 = triFaceTexture pvUnit
triAssertNe [pvTex27, ""]
triAssertExcludes [pvTex27, "FAIL"]
pvUnit setFace "Face18"
triSimFrames 5
pvTex18 = triFaceTexture pvUnit
triAssertExcludes [pvTex18, "FAIL"]
triAssertNe [pvTex18, pvTex27]

// -- the presented-frame capture still writes a file. The scene change and the
//    capture stay in separate statements even in this mode: the deferred
//    capture moves to the end of the frame, not to a later frame, so the sim
//    still has to settle the pose first. --------------------------------------
triCaptureAtPresent true
triSimFrames 5
triScreenshot "portrait_verbs_present"
triCaptureAtPresent false
triSimFrames 5
triScreenshot "portrait_verbs_midframe"

triClearView
triEndTest
