// Portraits are prepared locally before gameplay, from player-provided assets.
gpInitial = triPortraitStats
triAssert [(gpInitial select 0) > 0]
triAssertEq [gpInitial select 0, gpInitial select 1]
triSimUntil { GM_LIB_READY }

// -- the registry has seeded and the first companion is alive and bound -------
triSimUntil { gmLegendCount >= 3 }
triSimUntil { (count GM_COMP_OBJ) >= 1 }
// the array is published before the body exists, so wait for the OBJECT
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triSimUntil { (gmLegendFace 0) != "" }

gpBody = typeOf (GM_COMP_OBJ select 0)
triAssertNe [gpBody, ""]
gpFace = gmLegendFace 0
triAssertNe [gpFace, ""]
triAssertNe [gpFace, "Default"]
// Match the opaque identity requested by the live body and recorded face.
gpExpect = format ["portrait:%1", triPortraitId [gpBody, gpFace]]

// -- the dossier anchor for row 0.  gmLegendId takes a COMPANION index, so for
//    the first companion the two agree; gmLegendInfo is the general form -------
gpInfo = gmLegendInfo 0
triAssertEq [(count gpInfo), 8]
gpAnchor = format ["GM_WHO_%1", gpInfo select 0]
triAssertNe [(gpInfo select 0), ""]

// -- open the real map the way the player does (see guerrilla_journal_pages) --
triAssertEq [(triOpenMap), "OK"]
triSendKey 16
triSimFrames 10

// -- land on that companion's dossier page.  Start from a sentinel that is not
//    an alias target, so a switch that misses is visible ------------------------
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triAssertEq [(triBriefingSwitch gpAnchor), gpAnchor]

// -- exactly one image field, and it is the photograph -----------------------
gpImgs = triBriefingImages
triAssertNe [gpImgs, ""]
triAssertExcludes [gpImgs, "FAIL:"]
// one field only: the verb joins fields with ';'
triAssertExcludes [gpImgs, ";"]
// "-" is the verb's word for "this field never resolved a texture", i.e. the
// loose .paa did not come through the bank-then-CWD chain
triAssertExcludes [gpImgs, "-|"]

// substr is [string, FROM, TO], both 0-based indices, not [string, start,
// count]: StrSub returns RString(str.Data() + from, to - from)
// (GameStateExtUi.cpp).  Passing a count crashes the game outright once
// to < from, because the negative length reaches strncpy unguarded.
gpSplit = {
    gpI = 0;
    gpN = sizeofstr _this;
    gpParts = [];
    gpCur = "";
    while "gpI < gpN" do {
        gpC = substr [_this, gpI, gpI + 1];
        if (gpC == "|") then { gpParts = gpParts + [gpCur]; gpCur = "" } else { gpCur = gpCur + gpC };
        gpI = gpI + 1
    };
    gpParts = gpParts + [gpCur];
    gpParts
}
gpF = gpImgs call gpSplit
triAssertEq [(count gpF), 3]

// the texture the notepad actually holds is the one the key names
gpName = gpF select 0
triAssertIncludes [gpName, "portrait:"]
triAssert [(gpName == gpExpect)]

// width and height are stored in PAGE units (AddImage divides by 640 and 480),
// so they are small fractions - but never zero, which is what an unsized or
// collapsed image field would report
gpW = gpF select 1
gpH = gpF select 2
triAssertNe [gpW, "0.0000"]
triAssertNe [gpH, "0.0000"]

// -- the box is inside the page.  triBriefingMetrics reports
//    x,y,w,h,scale,pageW,pageH; a portrait taller than the page would push the
//    caption and the footer off the paper -------------------------------------
gpMet = triBriefingMetrics
triAssertNe [gpMet, ""]
triAssertExcludes [gpMet, "FAIL:"]
triAssert [triBriefingFits]

// -- and it survives a repaint: the journal is rebuilt on every map open and on
//    every revision change, so a portrait that only resolves once would be a
//    texture-bank accident rather than a working path -------------------------
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triSimFrames 30
triAssertEq [(triBriefingSwitch gpAnchor), gpAnchor]
gpImgs2 = triBriefingImages
triAssertEq [gpImgs2, gpImgs]

triAssert [triPortraitIsolation [gpBody, gpFace]]
triSimFrames 3
triAssertEq [(triBriefingSection), gpAnchor]
triAssertEq [(triBriefingImages), gpImgs]
triAssert [triPortraitRecreate]
triSimFrames 3
triAssertEq [(triBriefingSection), gpAnchor]
triAssertEq [(triBriefingImages), gpImgs]
triScreenshot "portrait_dossier"
// Controlled capture is independent of campaign weather and time of day.
gpPixels = triPortraitPixels [gpBody, gpFace]
triAssertExcludes [gpPixels, "FAIL:"]
0 setOvercast 1
0 setRain 1
skipTime 12
triSimFrames 5
triAssertEq [(triPortraitPixels [gpBody, gpFace]), gpPixels]
triAssert [triPortraitIsolation [gpBody, gpFace]]
// An unprepared type exercises isolation during first asset/type construction.
triAssert [triPortraitIsolation ["Civilian3", "Face10"]]
triAssert [triPortraitIsolation []]
// A late unsupported face terminates without a render or blocking the queue.
gpBeforeFailure = triPortraitStats
gpUnsupported = triPortraitId [gpBody, "Default"]
triSimFrames 3
gpAfterFailure = triPortraitStats
triAssertEq [gpAfterFailure select 0, (gpBeforeFailure select 0) + 1]
triAssertEq [gpAfterFailure select 1, gpAfterFailure select 0]
triAssertEq [gpAfterFailure select 2, gpBeforeFailure select 2]
triAssertEq [(triPortraitId [gpBody, "Default"]), gpUnsupported]
triAssert [triPortraitCancel]
triSimFrames 3
triAssertEq [(triBriefingSection), gpAnchor]
triAssertEq [(triBriefingImages), gpImgs]
// Recompose while queued: placeholder first, then the same section's texture.
triAssert [triPortraitRefresh]
triSimFrames 3
triAssertEq [(triBriefingSection), gpAnchor]
triAssertEq [(triBriefingImages), gpImgs]
triAssert [triBriefingFits]
triEndTest
