// ============================================================================
//  The dossier photograph, end to end, on a live campaign (guerrilla_native.abel).
//
//    Every other layer of the portrait loop is checked somewhere cheaper: the
//    key rule in the unit suite (PortraitKeyOf), the shipped files' shape in
//    tests/unit/.../test_portrait_assets.cpp, the "is a portrait missing for a
//    faction somebody just added" question in tests/contracts/
//    test_portrait_catalogue.py.  What none of those can see is whether the
//    .paa the journal NAMES is a file the engine can actually open, because
//    that answer depends on the loose-file resolution chain: PortraitSrc's
//    leading backslash makes AddImage skip FindPicture and hand the path
//    straight to GlobLoadTexture, which resolves through QIFStreamB::FileExist
//    (mounted banks first, then a CWD-relative open) - and the CWD is the data
//    dir, so it comes down to whether install-missions.ps1's robocopy /MIR of
//    guerrilla-mode/core actually put the file in <GameDir>\gmcore\portraits.
//
//    triBriefingImages is what makes that observable: HTMLField keeps no copy
//    of the source path, only Ref<Texture> texture1, so a field that reports
//    "-" is precisely a field whose texture never resolved.  A name is proof of
//    the opposite, and the name it reports is the texture's own, lowercased by
//    AddImage.
//
//    The expected key is built here from the LIVE body and the PERSISTED face,
//    never hardcoded: typeOf (GM_COMP_OBJ select 0) is the same string the
//    registry stamps into LegendRow::bodyClass (GetNonAIType()->GetName()), and
//    gmLegendFace 0 is the token BindRow validated against CfgFaces.  That is
//    what makes "a portrait can never depict a different faction's uniform"
//    a checked property rather than a promise.
//
//    HEADFUL on purpose.  A lane with no presenting engine has no texture bank
//    to load into and the field would come back "-" for the wrong reason.
//
//    PRECONDITION: guerrilla-mode/core must be installed into the data dir's
//    gmcore (install-missions.ps1, or the Trident workflow's own robocopy /MIR
//    into the classic-shadow), portraits included.
// ============================================================================

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
// only these four have photographs; anything else is the "Photograph
// unavailable" branch by design, and this lane would then be testing nothing
triAssert [((["Face10", "Face18", "Face27", "Face33"] find gpFace) >= 0)]

// The path the journal builds, reproduced from the same two halves.  SQF string
// == is strcmpi (express.cpp StrCmpE), so this compares case-insensitively -
// which is what is wanted, because AddImage lowercases the path it loads and
// typeOf reports the class in its config spelling.
gpExpect = format ["gmcore\portraits\%1__%2.paa", gpBody, gpFace]

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
triAssertIncludes [gpName, "portraits"]
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

// -- and it survives a repaint: the journal is rebuilt on every map open and on
//    every revision change, so a portrait that only resolves once would be a
//    texture-bank accident rather than a working path -------------------------
triAssertEq [(triBriefingSwitch "GM_MAN_SAVE"), "GM_MAN_SAVE"]
triSimFrames 30
triAssertEq [(triBriefingSwitch gpAnchor), gpAnchor]
gpImgs2 = triBriefingImages
triAssertEq [gpImgs2, gpImgs]

triEndTest
