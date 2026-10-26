triSimUntil { GM_LIB_READY }
triSimUntil { gmJournalCount >= 1 }
triSimUntil { (gmLegendId 0) != "" }
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triCaptureAtPresent true
triAssertEq [(triOpenMap), "OK"]
triSendKey 16
triSimFrames 15
triAssertEq [(triBriefingSwitch "Main"), "Main"]
triSimFrames 10
triScreenshot "journal_contents"
triAssertExcludes [(triBriefingMetrics), "FAIL"]
jaWho = format ["GM_WHO_%1", gmLegendId 0]
triAssertEq [(triBriefingSwitch jaWho), jaWho]
triSimFrames 15
triScreenshot "journal_first_dossier"
triEndTest
