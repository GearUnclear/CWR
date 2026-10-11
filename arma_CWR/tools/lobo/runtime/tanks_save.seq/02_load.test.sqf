// Phase 2: fresh process baseline, then load phase 1's binary world save.
// Saved object references and immutable snapshots must rehydrate together.
// Compare the entire ordered name list and all exposed by-name damage values.
// Duplicate blank names expose the first matching entry, not distinct raw slots.
triSimUntil {alive player}
triAssert [isNil "tsTanks"]
logInfo "LOBO_TANK_LOAD_BASELINE fresh_process=true tank_globals_absent=true"
triAssertEq [triLoadGame "lobo_issue1_tanks","OK"]
triSimFrames 3
triAssert [not (isNil "tsTanks")]
triAssertEq [count tsTanks,8]
triAssertEq [count tsNames,8]
triAssertEq [count tsValues,8]
triAssertEq [count tsTotals,8]
tsMismatch=[]
tsI=0
while {tsI < count tsTanks} do {
    tsTank=tsTanks select tsI;
    tsActualNames=GetHitpointsNames tsTank;
    tsExpectedNames=tsNames select tsI;
    tsExpectedValues=tsValues select tsI;
    if (isNull tsTank) then {tsMismatch=tsMismatch+[format ["null tank index=%1",tsI]]};
    if ((typeOf tsTank)!=(tsClasses select tsI)) then {tsMismatch=tsMismatch+[format ["class mismatch index=%1",tsI]]};
    if ((format ["%1",tsActualNames])!=(format ["%1",tsExpectedNames])) then {tsMismatch=tsMismatch+[format ["component order mismatch class=%1",typeOf tsTank]]};
    if (abs ((getDammage tsTank)-(tsTotals select tsI))>0.0001) then {tsMismatch=tsMismatch+[format ["total mismatch class=%1",typeOf tsTank]]};
    tsJ=0;
    tsActualValues=[];
    while {tsJ < count tsExpectedNames} do {
        tsActual=tsTank getSelectionDamage (tsExpectedNames select tsJ);
        tsExpected=tsExpectedValues select tsJ;
        tsActualValues=tsActualValues+[tsActual];
        if (abs (tsActual-tsExpected)>0.0001) then {tsMismatch=tsMismatch+[format ["component mismatch class=%1 selection=%2 actual=%3 expected=%4",typeOf tsTank,tsExpectedNames select tsJ,tsActual,tsExpected]]};
        tsJ=tsJ+1;
    };
    logInfo format ["LOBO_TANK_RELOAD class=%1 model=%2 names=%3 actual=%4 expected=%5 total=%6 expected_total=%7",typeOf tsTank,tsModelLabels select tsI,tsActualNames,tsActualValues,tsExpectedValues,getDammage tsTank,tsTotals select tsI];
    tsI=tsI+1;
}
logInfo format ["LOBO_TANK_RELOAD_MISMATCHES %1",tsMismatch]
triAssertEq [count tsMismatch,0]
triAssertEq [gunner (tsTanks select 0),tsGunner]
tsLoadCam="camera" camCreate (getPos player)
tsLoadCam cameraEffect ["internal","back"]

tsTank=tsTanks select 0
triAssertEq [typeOf tsTank,"LoBo_M1"]
triAssert [alive tsTank]
call tsAim
triSimFrames 3
triScreenshot "tank_01_LoBo_M1_reloaded"

tsTank=tsTanks select 1
triAssertEq [typeOf tsTank,"LoBo_M1G"]
triAssert [alive tsTank]
call tsAim
triSimFrames 3
triScreenshot "tank_02_LoBo_M1G_reloaded"

tsTank=tsTanks select 2
triAssertEq [typeOf tsTank,"LoBo_M1MG_2"]
triAssert [alive tsTank]
call tsAim
triSimFrames 3
triScreenshot "tank_03_LoBo_M1MG_2_reloaded"

tsTank=tsTanks select 3
triAssertEq [typeOf tsTank,"LoBo_M1GMG_2"]
triAssert [alive tsTank]
call tsAim
triSimFrames 3
triScreenshot "tank_04_LoBo_M1GMG_2_reloaded"

tsTank=tsTanks select 4
triAssertEq [typeOf tsTank,"LoBoMerkava1"]
triAssert [alive tsTank]
call tsAim
triSimFrames 3
triScreenshot "tank_05_LoBoMerkava1_reloaded"

tsTank=tsTanks select 5
triAssertEq [typeOf tsTank,"LoBoMerkava2"]
triAssert [alive tsTank]
call tsAim
triSimFrames 3
triScreenshot "tank_06_LoBoMerkava2_reloaded"

tsTank=tsTanks select 6
triAssertEq [typeOf tsTank,"LoBoMerkava3"]
triAssert [alive tsTank]
call tsAim
triSimFrames 3
triScreenshot "tank_07_LoBoMerkava3_reloaded"

tsTank=tsTanks select 7
triAssertEq [typeOf tsTank,"LoBoMerkava4DT"]
triAssert [alive tsTank]
call tsAim
triSimFrames 3
triScreenshot "tank_08_LoBoMerkava4DT_reloaded"

tsLoadCam cameraEffect ["terminate","back"]
camDestroy tsLoadCam
logInfo "LOBO_TANK_ROUNDTRIP_COMPLETE vehicles=8 supported_components_per_vehicle=6 ordered_names_preserved=true tolerance=0.0001 content_same=true"
triEndTest
