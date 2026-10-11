// Phase 1: damage actual tank models and save their live component state.
// One vertical M833 projectile per tank is a controlled collision probe.
// It does not establish directional penetration, weapon accuracy or lethality.
// Track states are separately seeded with setSelectionDamage and labeled as such.
// The full ordered name list can include extra blank or light entries.
// Exposed damage values are read by name; duplicate blank names cannot be
// distinguished through getSelectionDamage and are not raw-array coverage.
triSimUntil { alive player }
setDate [1985,6,15,12,0]
0 setOvercast 0
0 setFog 0
player setCaptive true
player allowDammage false
setViewDistance 1800
showCinemaBorder false
tsBase=getPos player
tsTanks=[]
tsClasses=[]
tsModelLabels=[]
tsNames=[]
tsValues=[]
tsTotals=[]
tsImpactTotals=[]
tsReadDamage={tsRead=[]; {tsRead=tsRead+[tsTank getSelectionDamage _x]} forEach (GetHitpointsNames tsTank); tsRead}
tsCam="camera" camCreate tsBase
tsCam cameraEffect ["internal","back"]
tsAim={tsP=getPosASL tsTank; triSetView [(tsP select 0)+11,(tsP select 2)+5,(tsP select 1)+12,-11,-3.5,-12]}

// Vehicle LoBo_M1 uses model LoBo_M1A1E.
tsTank="LoBo_M1" createVehicle [(tsBase select 0)+-60,(tsBase select 1)+20,0]
triAssertEq [typeOf tsTank,"LoBo_M1"]
tsTank setDir 0
triSimFrames 10
triAssertGe [count (GetHitpointsNames tsTank),6]
triAssert ["engine" in (GetHitpointsNames tsTank)]
triAssert ["gun" in (GetHitpointsNames tsTank)]
triAssert ["hull" in (GetHitpointsNames tsTank)]
triAssert ["turet" in (GetHitpointsNames tsTank)]
triAssert ["pasl" in (GetHitpointsNames tsTank)]
triAssert ["pasp" in (GetHitpointsNames tsTank)]
triAssertEq [getDammage tsTank,0]
tsPos=getPos tsTank
tsShot="LoBo_105Sabot2" camCreate [tsPos select 0,tsPos select 1,(tsPos select 2)+12]
triAssert [not (isNull tsShot)]
tsShot setVelocity [0,0,-300]
tsImpactWait=time+1
triSimUntil {time >= tsImpactWait}
triAssertGt [getDammage tsTank,0]
triAssertGt [(tsTank getSelectionDamage "hull")+(tsTank getSelectionDamage "turet"),0]
triAssert [alive tsTank]
tsImpactTotals=tsImpactTotals+[getDammage tsTank]
logInfo format ["LOBO_TANK_IMPACT class=%1 model=LoBo_M1A1E method=vertical_LoBo_105Sabot2 total=%2 names=%3 values=%4",typeOf tsTank,getDammage tsTank,GetHitpointsNames tsTank,call tsReadDamage]
tsTank setSelectionDamage ["pasL",0.12]
tsTank setSelectionDamage ["pasP",0.23]
triAssertLe [abs ((tsTank getSelectionDamage "pasL")-0.12),0.0001]
triAssertLe [abs ((tsTank getSelectionDamage "pasP")-0.23),0.0001]
logInfo format ["LOBO_TANK_TRACK_SEED class=%1 method=setSelectionDamage pasL=%2 pasP=%3",typeOf tsTank,tsTank getSelectionDamage "pasL",tsTank getSelectionDamage "pasP"]
tsTanks=tsTanks+[tsTank]
tsClasses=tsClasses+[typeOf tsTank]
tsModelLabels=tsModelLabels+["LoBo_M1A1E"]
call tsAim
triSimFrames 3
triScreenshot "tank_01_LoBo_M1_damaged"

// Vehicle LoBo_M1G uses model LoBo_M1A1E_R.
tsTank="LoBo_M1G" createVehicle [(tsBase select 0)+-40,(tsBase select 1)+20,0]
triAssertEq [typeOf tsTank,"LoBo_M1G"]
tsTank setDir 0
triSimFrames 10
triAssertGe [count (GetHitpointsNames tsTank),6]
triAssert ["engine" in (GetHitpointsNames tsTank)]
triAssert ["gun" in (GetHitpointsNames tsTank)]
triAssert ["hull" in (GetHitpointsNames tsTank)]
triAssert ["turet" in (GetHitpointsNames tsTank)]
triAssert ["pasl" in (GetHitpointsNames tsTank)]
triAssert ["pasp" in (GetHitpointsNames tsTank)]
triAssertEq [getDammage tsTank,0]
tsPos=getPos tsTank
tsShot="LoBo_105Sabot2" camCreate [tsPos select 0,tsPos select 1,(tsPos select 2)+12]
triAssert [not (isNull tsShot)]
tsShot setVelocity [0,0,-300]
tsImpactWait=time+1
triSimUntil {time >= tsImpactWait}
triAssertGt [getDammage tsTank,0]
triAssertGt [(tsTank getSelectionDamage "hull")+(tsTank getSelectionDamage "turet"),0]
triAssert [alive tsTank]
tsImpactTotals=tsImpactTotals+[getDammage tsTank]
logInfo format ["LOBO_TANK_IMPACT class=%1 model=LoBo_M1A1E_R method=vertical_LoBo_105Sabot2 total=%2 names=%3 values=%4",typeOf tsTank,getDammage tsTank,GetHitpointsNames tsTank,call tsReadDamage]
tsTank setSelectionDamage ["pasL",0.12]
tsTank setSelectionDamage ["pasP",0.23]
triAssertLe [abs ((tsTank getSelectionDamage "pasL")-0.12),0.0001]
triAssertLe [abs ((tsTank getSelectionDamage "pasP")-0.23),0.0001]
logInfo format ["LOBO_TANK_TRACK_SEED class=%1 method=setSelectionDamage pasL=%2 pasP=%3",typeOf tsTank,tsTank getSelectionDamage "pasL",tsTank getSelectionDamage "pasP"]
tsTanks=tsTanks+[tsTank]
tsClasses=tsClasses+[typeOf tsTank]
tsModelLabels=tsModelLabels+["LoBo_M1A1E_R"]
call tsAim
triSimFrames 3
triScreenshot "tank_02_LoBo_M1G_damaged"

// Vehicle LoBo_M1MG_2 uses model LoBo_M1A2E.
tsTank="LoBo_M1MG_2" createVehicle [(tsBase select 0)+-20,(tsBase select 1)+20,0]
triAssertEq [typeOf tsTank,"LoBo_M1MG_2"]
tsTank setDir 0
triSimFrames 10
triAssertGe [count (GetHitpointsNames tsTank),6]
triAssert ["engine" in (GetHitpointsNames tsTank)]
triAssert ["gun" in (GetHitpointsNames tsTank)]
triAssert ["hull" in (GetHitpointsNames tsTank)]
triAssert ["turet" in (GetHitpointsNames tsTank)]
triAssert ["pasl" in (GetHitpointsNames tsTank)]
triAssert ["pasp" in (GetHitpointsNames tsTank)]
triAssertEq [getDammage tsTank,0]
tsPos=getPos tsTank
tsShot="LoBo_105Sabot2" camCreate [tsPos select 0,tsPos select 1,(tsPos select 2)+12]
triAssert [not (isNull tsShot)]
tsShot setVelocity [0,0,-300]
tsImpactWait=time+1
triSimUntil {time >= tsImpactWait}
triAssertGt [getDammage tsTank,0]
triAssertGt [(tsTank getSelectionDamage "hull")+(tsTank getSelectionDamage "turet"),0]
triAssert [alive tsTank]
tsImpactTotals=tsImpactTotals+[getDammage tsTank]
logInfo format ["LOBO_TANK_IMPACT class=%1 model=LoBo_M1A2E method=vertical_LoBo_105Sabot2 total=%2 names=%3 values=%4",typeOf tsTank,getDammage tsTank,GetHitpointsNames tsTank,call tsReadDamage]
tsTank setSelectionDamage ["pasL",0.12]
tsTank setSelectionDamage ["pasP",0.23]
triAssertLe [abs ((tsTank getSelectionDamage "pasL")-0.12),0.0001]
triAssertLe [abs ((tsTank getSelectionDamage "pasP")-0.23),0.0001]
logInfo format ["LOBO_TANK_TRACK_SEED class=%1 method=setSelectionDamage pasL=%2 pasP=%3",typeOf tsTank,tsTank getSelectionDamage "pasL",tsTank getSelectionDamage "pasP"]
tsTanks=tsTanks+[tsTank]
tsClasses=tsClasses+[typeOf tsTank]
tsModelLabels=tsModelLabels+["LoBo_M1A2E"]
call tsAim
triSimFrames 3
triScreenshot "tank_03_LoBo_M1MG_2_damaged"

// Vehicle LoBo_M1GMG_2 uses model LoBo_M1A2E_R.
tsTank="LoBo_M1GMG_2" createVehicle [(tsBase select 0)+0,(tsBase select 1)+20,0]
triAssertEq [typeOf tsTank,"LoBo_M1GMG_2"]
tsTank setDir 0
triSimFrames 10
triAssertGe [count (GetHitpointsNames tsTank),6]
triAssert ["engine" in (GetHitpointsNames tsTank)]
triAssert ["gun" in (GetHitpointsNames tsTank)]
triAssert ["hull" in (GetHitpointsNames tsTank)]
triAssert ["turet" in (GetHitpointsNames tsTank)]
triAssert ["pasl" in (GetHitpointsNames tsTank)]
triAssert ["pasp" in (GetHitpointsNames tsTank)]
triAssertEq [getDammage tsTank,0]
tsPos=getPos tsTank
tsShot="LoBo_105Sabot2" camCreate [tsPos select 0,tsPos select 1,(tsPos select 2)+12]
triAssert [not (isNull tsShot)]
tsShot setVelocity [0,0,-300]
tsImpactWait=time+1
triSimUntil {time >= tsImpactWait}
triAssertGt [getDammage tsTank,0]
triAssertGt [(tsTank getSelectionDamage "hull")+(tsTank getSelectionDamage "turet"),0]
triAssert [alive tsTank]
tsImpactTotals=tsImpactTotals+[getDammage tsTank]
logInfo format ["LOBO_TANK_IMPACT class=%1 model=LoBo_M1A2E_R method=vertical_LoBo_105Sabot2 total=%2 names=%3 values=%4",typeOf tsTank,getDammage tsTank,GetHitpointsNames tsTank,call tsReadDamage]
tsTank setSelectionDamage ["pasL",0.12]
tsTank setSelectionDamage ["pasP",0.23]
triAssertLe [abs ((tsTank getSelectionDamage "pasL")-0.12),0.0001]
triAssertLe [abs ((tsTank getSelectionDamage "pasP")-0.23),0.0001]
logInfo format ["LOBO_TANK_TRACK_SEED class=%1 method=setSelectionDamage pasL=%2 pasP=%3",typeOf tsTank,tsTank getSelectionDamage "pasL",tsTank getSelectionDamage "pasP"]
tsTanks=tsTanks+[tsTank]
tsClasses=tsClasses+[typeOf tsTank]
tsModelLabels=tsModelLabels+["LoBo_M1A2E_R"]
call tsAim
triSimFrames 3
triScreenshot "tank_04_LoBo_M1GMG_2_damaged"

// Vehicle LoBoMerkava1 uses model LoBoMerkava1.
tsTank="LoBoMerkava1" createVehicle [(tsBase select 0)+-60,(tsBase select 1)+40,0]
triAssertEq [typeOf tsTank,"LoBoMerkava1"]
tsTank setDir 0
triSimFrames 10
triAssertGe [count (GetHitpointsNames tsTank),6]
triAssert ["engine" in (GetHitpointsNames tsTank)]
triAssert ["gun" in (GetHitpointsNames tsTank)]
triAssert ["hull" in (GetHitpointsNames tsTank)]
triAssert ["turet" in (GetHitpointsNames tsTank)]
triAssert ["pasl" in (GetHitpointsNames tsTank)]
triAssert ["pasp" in (GetHitpointsNames tsTank)]
triAssertEq [getDammage tsTank,0]
tsPos=getPos tsTank
tsShot="LoBo_105Sabot2" camCreate [tsPos select 0,tsPos select 1,(tsPos select 2)+12]
triAssert [not (isNull tsShot)]
tsShot setVelocity [0,0,-300]
tsImpactWait=time+1
triSimUntil {time >= tsImpactWait}
triAssertGt [getDammage tsTank,0]
triAssertGt [(tsTank getSelectionDamage "hull")+(tsTank getSelectionDamage "turet"),0]
triAssert [alive tsTank]
tsImpactTotals=tsImpactTotals+[getDammage tsTank]
logInfo format ["LOBO_TANK_IMPACT class=%1 model=LoBoMerkava1 method=vertical_LoBo_105Sabot2 total=%2 names=%3 values=%4",typeOf tsTank,getDammage tsTank,GetHitpointsNames tsTank,call tsReadDamage]
tsTank setSelectionDamage ["pasL",0.12]
tsTank setSelectionDamage ["pasP",0.23]
triAssertLe [abs ((tsTank getSelectionDamage "pasL")-0.12),0.0001]
triAssertLe [abs ((tsTank getSelectionDamage "pasP")-0.23),0.0001]
logInfo format ["LOBO_TANK_TRACK_SEED class=%1 method=setSelectionDamage pasL=%2 pasP=%3",typeOf tsTank,tsTank getSelectionDamage "pasL",tsTank getSelectionDamage "pasP"]
tsTanks=tsTanks+[tsTank]
tsClasses=tsClasses+[typeOf tsTank]
tsModelLabels=tsModelLabels+["LoBoMerkava1"]
call tsAim
triSimFrames 3
triScreenshot "tank_05_LoBoMerkava1_damaged"

// Vehicle LoBoMerkava2 uses model LoBoMerkava2.
tsTank="LoBoMerkava2" createVehicle [(tsBase select 0)+-40,(tsBase select 1)+40,0]
triAssertEq [typeOf tsTank,"LoBoMerkava2"]
tsTank setDir 0
triSimFrames 10
triAssertGe [count (GetHitpointsNames tsTank),6]
triAssert ["engine" in (GetHitpointsNames tsTank)]
triAssert ["gun" in (GetHitpointsNames tsTank)]
triAssert ["hull" in (GetHitpointsNames tsTank)]
triAssert ["turet" in (GetHitpointsNames tsTank)]
triAssert ["pasl" in (GetHitpointsNames tsTank)]
triAssert ["pasp" in (GetHitpointsNames tsTank)]
triAssertEq [getDammage tsTank,0]
tsPos=getPos tsTank
tsShot="LoBo_105Sabot2" camCreate [tsPos select 0,tsPos select 1,(tsPos select 2)+12]
triAssert [not (isNull tsShot)]
tsShot setVelocity [0,0,-300]
tsImpactWait=time+1
triSimUntil {time >= tsImpactWait}
triAssertGt [getDammage tsTank,0]
triAssertGt [(tsTank getSelectionDamage "hull")+(tsTank getSelectionDamage "turet"),0]
triAssert [alive tsTank]
tsImpactTotals=tsImpactTotals+[getDammage tsTank]
logInfo format ["LOBO_TANK_IMPACT class=%1 model=LoBoMerkava2 method=vertical_LoBo_105Sabot2 total=%2 names=%3 values=%4",typeOf tsTank,getDammage tsTank,GetHitpointsNames tsTank,call tsReadDamage]
tsTank setSelectionDamage ["pasL",0.12]
tsTank setSelectionDamage ["pasP",0.23]
triAssertLe [abs ((tsTank getSelectionDamage "pasL")-0.12),0.0001]
triAssertLe [abs ((tsTank getSelectionDamage "pasP")-0.23),0.0001]
logInfo format ["LOBO_TANK_TRACK_SEED class=%1 method=setSelectionDamage pasL=%2 pasP=%3",typeOf tsTank,tsTank getSelectionDamage "pasL",tsTank getSelectionDamage "pasP"]
tsTanks=tsTanks+[tsTank]
tsClasses=tsClasses+[typeOf tsTank]
tsModelLabels=tsModelLabels+["LoBoMerkava2"]
call tsAim
triSimFrames 3
triScreenshot "tank_06_LoBoMerkava2_damaged"

// Vehicle LoBoMerkava3 uses model LoBoMerkava3.
tsTank="LoBoMerkava3" createVehicle [(tsBase select 0)+-20,(tsBase select 1)+40,0]
triAssertEq [typeOf tsTank,"LoBoMerkava3"]
tsTank setDir 0
triSimFrames 10
triAssertGe [count (GetHitpointsNames tsTank),6]
triAssert ["engine" in (GetHitpointsNames tsTank)]
triAssert ["gun" in (GetHitpointsNames tsTank)]
triAssert ["hull" in (GetHitpointsNames tsTank)]
triAssert ["turet" in (GetHitpointsNames tsTank)]
triAssert ["pasl" in (GetHitpointsNames tsTank)]
triAssert ["pasp" in (GetHitpointsNames tsTank)]
triAssertEq [getDammage tsTank,0]
tsPos=getPos tsTank
tsShot="LoBo_105Sabot2" camCreate [tsPos select 0,tsPos select 1,(tsPos select 2)+12]
triAssert [not (isNull tsShot)]
tsShot setVelocity [0,0,-300]
tsImpactWait=time+1
triSimUntil {time >= tsImpactWait}
triAssertGt [getDammage tsTank,0]
triAssertGt [(tsTank getSelectionDamage "hull")+(tsTank getSelectionDamage "turet"),0]
triAssert [alive tsTank]
tsImpactTotals=tsImpactTotals+[getDammage tsTank]
logInfo format ["LOBO_TANK_IMPACT class=%1 model=LoBoMerkava3 method=vertical_LoBo_105Sabot2 total=%2 names=%3 values=%4",typeOf tsTank,getDammage tsTank,GetHitpointsNames tsTank,call tsReadDamage]
tsTank setSelectionDamage ["pasL",0.12]
tsTank setSelectionDamage ["pasP",0.23]
triAssertLe [abs ((tsTank getSelectionDamage "pasL")-0.12),0.0001]
triAssertLe [abs ((tsTank getSelectionDamage "pasP")-0.23),0.0001]
logInfo format ["LOBO_TANK_TRACK_SEED class=%1 method=setSelectionDamage pasL=%2 pasP=%3",typeOf tsTank,tsTank getSelectionDamage "pasL",tsTank getSelectionDamage "pasP"]
tsTanks=tsTanks+[tsTank]
tsClasses=tsClasses+[typeOf tsTank]
tsModelLabels=tsModelLabels+["LoBoMerkava3"]
call tsAim
triSimFrames 3
triScreenshot "tank_07_LoBoMerkava3_damaged"

// Vehicle LoBoMerkava4DT uses model LoBoMerkava4D.
tsTank="LoBoMerkava4DT" createVehicle [(tsBase select 0)+0,(tsBase select 1)+40,0]
triAssertEq [typeOf tsTank,"LoBoMerkava4DT"]
tsTank setDir 0
triSimFrames 10
triAssertGe [count (GetHitpointsNames tsTank),6]
triAssert ["engine" in (GetHitpointsNames tsTank)]
triAssert ["gun" in (GetHitpointsNames tsTank)]
triAssert ["hull" in (GetHitpointsNames tsTank)]
triAssert ["turet" in (GetHitpointsNames tsTank)]
triAssert ["pasl" in (GetHitpointsNames tsTank)]
triAssert ["pasp" in (GetHitpointsNames tsTank)]
triAssertEq [getDammage tsTank,0]
tsPos=getPos tsTank
tsShot="LoBo_105Sabot2" camCreate [tsPos select 0,tsPos select 1,(tsPos select 2)+12]
triAssert [not (isNull tsShot)]
tsShot setVelocity [0,0,-300]
tsImpactWait=time+1
triSimUntil {time >= tsImpactWait}
triAssertGt [getDammage tsTank,0]
triAssertGt [(tsTank getSelectionDamage "hull")+(tsTank getSelectionDamage "turet"),0]
triAssert [alive tsTank]
tsImpactTotals=tsImpactTotals+[getDammage tsTank]
logInfo format ["LOBO_TANK_IMPACT class=%1 model=LoBoMerkava4D method=vertical_LoBo_105Sabot2 total=%2 names=%3 values=%4",typeOf tsTank,getDammage tsTank,GetHitpointsNames tsTank,call tsReadDamage]
tsTank setSelectionDamage ["pasL",0.12]
tsTank setSelectionDamage ["pasP",0.23]
triAssertLe [abs ((tsTank getSelectionDamage "pasL")-0.12),0.0001]
triAssertLe [abs ((tsTank getSelectionDamage "pasP")-0.23),0.0001]
logInfo format ["LOBO_TANK_TRACK_SEED class=%1 method=setSelectionDamage pasL=%2 pasP=%3",typeOf tsTank,tsTank getSelectionDamage "pasL",tsTank getSelectionDamage "pasP"]
tsTanks=tsTanks+[tsTank]
tsClasses=tsClasses+[typeOf tsTank]
tsModelLabels=tsModelLabels+["LoBoMerkava4D"]
call tsAim
triSimFrames 3
triScreenshot "tank_08_LoBoMerkava4DT_damaged"

// Live crew watch commands provide visual evidence of turret/gun movement.
// No angle getter is available; command completion alone is not an animation pass.
tsTank=tsTanks select 0
createCenter west
tsCrew=createGroup west
"SoldierWB" createUnit [[(tsBase select 0)-80,tsBase select 1,0],tsCrew,"tsGunner=this",0.8,"PRIVATE"]
triAssert [not (isNull tsGunner)]
tsGunner setCaptive true
tsCrew setCombatMode "BLUE"
tsGunner moveInGunner tsTank
triSimUntil {(gunner tsTank)==tsGunner}
tsPos=getPos tsTank
tsGunner doWatch [(tsPos select 0)+200,tsPos select 1,20]
tsAnimWait=time+4
triSimUntil {time >= tsAnimWait}
logInfo format ["LOBO_TANK_ANIMATION class=%1 command=doWatch_east gunner=%2 visual_review_required=true",typeOf tsTank,typeOf tsGunner]
call tsAim
triSimFrames 3
triScreenshot "tank_m1_turret_watch_east"
tsGunner doWatch [(tsPos select 0)-200,tsPos select 1,20]
tsAnimWait=time+4
triSimUntil {time >= tsAnimWait}
logInfo format ["LOBO_TANK_ANIMATION class=%1 command=doWatch_west gunner=%2 visual_review_required=true",typeOf tsTank,typeOf tsGunner]
call tsAim
triSimFrames 3
triScreenshot "tank_m1_turret_watch_west"
tsCam cameraEffect ["terminate","back"]
camDestroy tsCam
// Snapshot only after all impacts and observation steps have completed.
tsI=0
while {tsI < count tsTanks} do {
    tsTank=tsTanks select tsI;
    tsNames=tsNames+[GetHitpointsNames tsTank];
    tsValues=tsValues+[call tsReadDamage];
    tsTotals=tsTotals+[getDammage tsTank];
    logInfo format ["LOBO_TANK_SAVE class=%1 names=%2 values=%3 total=%4",typeOf tsTank,GetHitpointsNames tsTank,call tsReadDamage,getDammage tsTank];
    tsI=tsI+1;
}
triAssertEq [count tsTanks,8]
triAssertEq [triSaveGame "lobo_issue1_tanks","OK"]
logInfo "LOBO_TANK_SAVE_COMPLETE vehicles=8 next_phase_requires_fresh_process=true"
triEndTest
