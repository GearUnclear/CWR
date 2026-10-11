// Actual projectile collision and blast observations, not a physical lethality model.
triSimUntil { alive player }
player allowDammage false
player setCaptive true
createCenter east
laEG=createGroup east
laBase=getPos player
laImpact=[(laBase select 0)-110,(laBase select 1)+100,0]
// LoBo_Carbomb/SoldierEB/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/SoldierEB/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_Carbomb/UAZ/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/UAZ/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_Carbomb/M1Abrams/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/M1Abrams/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_Carbomb/SoldierEB/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/SoldierEB/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_Carbomb/UAZ/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/UAZ/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_Carbomb/M1Abrams/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/M1Abrams/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_Carbomb/SoldierEB/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/SoldierEB/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_Carbomb/UAZ/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/UAZ/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_Carbomb/M1Abrams/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/M1Abrams/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_Carbomb/SoldierEB/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/SoldierEB/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_Carbomb/UAZ/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/UAZ/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_Carbomb/M1Abrams/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/M1Abrams/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_Carbomb/SoldierEB/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/SoldierEB/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_Carbomb/UAZ/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/UAZ/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_Carbomb/M1Abrams/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/M1Abrams/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_Carbomb/SoldierEB/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/SoldierEB/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_Carbomb/UAZ/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/UAZ/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_Carbomb/M1Abrams/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_Carbomb" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_Carbomb/M1Abrams/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82_LDGB/SoldierEB/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/SoldierEB/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82_LDGB/UAZ/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/UAZ/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82_LDGB/M1Abrams/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/M1Abrams/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82_LDGB/SoldierEB/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/SoldierEB/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82_LDGB/UAZ/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/UAZ/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82_LDGB/M1Abrams/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/M1Abrams/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82_LDGB/SoldierEB/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/SoldierEB/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82_LDGB/UAZ/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/UAZ/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82_LDGB/M1Abrams/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/M1Abrams/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82_LDGB/SoldierEB/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/SoldierEB/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82_LDGB/UAZ/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/UAZ/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82_LDGB/M1Abrams/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/M1Abrams/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82_LDGB/SoldierEB/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/SoldierEB/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82_LDGB/UAZ/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/UAZ/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82_LDGB/M1Abrams/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/M1Abrams/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82_LDGB/SoldierEB/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/SoldierEB/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82_LDGB/UAZ/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/UAZ/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82_LDGB/M1Abrams/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82_LDGB" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82_LDGB/M1Abrams/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
logInfo "LOBO_BLAST_VARIANTS_COMPLETE"
triEndTest
