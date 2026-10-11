// Actual projectile collision and blast observations, not a physical lethality model.
triSimUntil { alive player }
player allowDammage false
player setCaptive true
createCenter east
laEG=createGroup east
laBase=getPos player
laImpact=[(laBase select 0)-110,(laBase select 1)+100,0]
laJor="LoBo_M60A1_jor" createVehicle [(laBase select 0)+30,laBase select 1,0]
triAssert [not isNull laJor]
player moveInGunner laJor
triAssertEq [vehicle player,laJor]
laM833Shots=0
laJor addEventHandler ["Fired",{logInfo format ["LOBO_M833_FIRED weapon=%1 ammo=%2",_this select 1,_this select 4]; if ((_this select 4)=="LoBo_105Sabot2") then {laM833Shots=laM833Shots+1}}]
laJor selectWeapon "LoBo_105Gun"
laJor fire "LoBo_105Gun"
triSimUntil {laM833Shots==1}
player action ["GETOUT",laJor]
triSimUntil {vehicle player==player}
deleteVehicle laJor
laVictim="M1Abrams" createVehicle laImpact
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 20
laP=getPos laVictim
laBefore=getDammage laVictim
laPj="LoBo_105Sabot2" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_M833_IMPACT target=M1Abrams before=%1 after=%2 hitpoints=%3 hull=%4 turret=%5 leftTrack=%6 rightTrack=%7",laBefore,getDammage laVictim,GetHitpointsNames laVictim,laVictim getSelectionDamage "hull",laVictim getSelectionDamage "turet",laVictim getSelectionDamage "pasL",laVictim getSelectionDamage "pasP"]
triAssertGe [getDammage laVictim,laBefore+0.0001]
deleteVehicle laVictim
triSimFrames 5
laVictim="LoBo_M1" createVehicle laImpact
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 20
laP=getPos laVictim
laBefore=getDammage laVictim
laPj="LoBo_105Sabot2" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_M833_IMPACT target=LoBo_M1 before=%1 after=%2 hitpoints=%3 hull=%4 turret=%5 leftTrack=%6 rightTrack=%7",laBefore,getDammage laVictim,GetHitpointsNames laVictim,laVictim getSelectionDamage "hull",laVictim getSelectionDamage "turet",laVictim getSelectionDamage "pasL",laVictim getSelectionDamage "pasP"]
triAssertGe [getDammage laVictim,laBefore+0.0001]
deleteVehicle laVictim
triSimFrames 5
laVictim="LoBoMerkava2" createVehicle laImpact
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 20
laP=getPos laVictim
laBefore=getDammage laVictim
laPj="LoBo_105Sabot2" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_M833_IMPACT target=LoBoMerkava2 before=%1 after=%2 hitpoints=%3 hull=%4 turret=%5 leftTrack=%6 rightTrack=%7",laBefore,getDammage laVictim,GetHitpointsNames laVictim,laVictim getSelectionDamage "hull",laVictim getSelectionDamage "turet",laVictim getSelectionDamage "pasL",laVictim getSelectionDamage "pasP"]
triAssertGe [getDammage laVictim,laBefore+0.0001]
deleteVehicle laVictim
triSimFrames 5
laVictim="LoBoMerkava3" createVehicle laImpact
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 20
laP=getPos laVictim
laBefore=getDammage laVictim
laPj="LoBo_105Sabot2" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_M833_IMPACT target=LoBoMerkava3 before=%1 after=%2 hitpoints=%3 hull=%4 turret=%5 leftTrack=%6 rightTrack=%7",laBefore,getDammage laVictim,GetHitpointsNames laVictim,laVictim getSelectionDamage "hull",laVictim getSelectionDamage "turet",laVictim getSelectionDamage "pasL",laVictim getSelectionDamage "pasP"]
triAssertGe [getDammage laVictim,laBefore+0.0001]
deleteVehicle laVictim
triSimFrames 5
laVictim="LoBoMerkava4DT" createVehicle laImpact
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 20
laP=getPos laVictim
laBefore=getDammage laVictim
laPj="LoBo_105Sabot2" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_M833_IMPACT target=LoBoMerkava4DT before=%1 after=%2 hitpoints=%3 hull=%4 turret=%5 leftTrack=%6 rightTrack=%7",laBefore,getDammage laVictim,GetHitpointsNames laVictim,laVictim getSelectionDamage "hull",laVictim getSelectionDamage "turet",laVictim getSelectionDamage "pasL",laVictim getSelectionDamage "pasP"]
triAssertGe [getDammage laVictim,laBefore+0.0001]
deleteVehicle laVictim
triSimFrames 5
laVictim="UAZ" createVehicle laImpact
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 20
laP=getPos laVictim
laBefore=getDammage laVictim
laPj="LoBo_105Sabot2" camCreate [laP select 0,laP select 1,10]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-300]
triSimFrames 60
logInfo format ["LOBO_M833_IMPACT target=UAZ before=%1 after=%2 hitpoints=%3 hull=%4 turret=%5 leftTrack=%6 rightTrack=%7",laBefore,getDammage laVictim,GetHitpointsNames laVictim,laVictim getSelectionDamage "hull",laVictim getSelectionDamage "turet",laVictim getSelectionDamage "pasL",laVictim getSelectionDamage "pasP"]
triAssertGe [getDammage laVictim,laBefore+0.0001]
deleteVehicle laVictim
triSimFrames 5
// LoBo_60HVMSHE/SoldierEB/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/SoldierEB/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_60HVMSHE/UAZ/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/UAZ/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_60HVMSHE/M1Abrams/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/M1Abrams/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_60HVMSHE/SoldierEB/5m/M113-cover
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
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/SoldierEB/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_60HVMSHE/UAZ/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/UAZ/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_60HVMSHE/M1Abrams/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/M1Abrams/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_60HVMSHE/SoldierEB/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/SoldierEB/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_60HVMSHE/UAZ/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/UAZ/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_60HVMSHE/M1Abrams/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/M1Abrams/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_60HVMSHE/SoldierEB/15m/M113-cover
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
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/SoldierEB/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_60HVMSHE/UAZ/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/UAZ/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_60HVMSHE/M1Abrams/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/M1Abrams/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_60HVMSHE/SoldierEB/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/SoldierEB/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_60HVMSHE/UAZ/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/UAZ/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_60HVMSHE/M1Abrams/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/M1Abrams/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_60HVMSHE/SoldierEB/30m/M113-cover
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
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/SoldierEB/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_60HVMSHE/UAZ/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/UAZ/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_60HVMSHE/M1Abrams/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_60HVMSHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_60HVMSHE/M1Abrams/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_120MortarShellHE/SoldierEB/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/SoldierEB/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_120MortarShellHE/UAZ/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/UAZ/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_120MortarShellHE/M1Abrams/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/M1Abrams/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_120MortarShellHE/SoldierEB/5m/M113-cover
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
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/SoldierEB/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_120MortarShellHE/UAZ/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/UAZ/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_120MortarShellHE/M1Abrams/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/M1Abrams/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_120MortarShellHE/SoldierEB/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/SoldierEB/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_120MortarShellHE/UAZ/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/UAZ/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_120MortarShellHE/M1Abrams/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/M1Abrams/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_120MortarShellHE/SoldierEB/15m/M113-cover
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
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/SoldierEB/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_120MortarShellHE/UAZ/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/UAZ/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_120MortarShellHE/M1Abrams/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/M1Abrams/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_120MortarShellHE/SoldierEB/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/SoldierEB/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_120MortarShellHE/UAZ/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/UAZ/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_120MortarShellHE/M1Abrams/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/M1Abrams/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_120MortarShellHE/SoldierEB/30m/M113-cover
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
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/SoldierEB/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_120MortarShellHE/UAZ/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/UAZ/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_120MortarShellHE/M1Abrams/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_120MortarShellHE" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_120MortarShellHE/M1Abrams/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_M160MortarShell/SoldierEB/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/SoldierEB/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_M160MortarShell/UAZ/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/UAZ/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_M160MortarShell/M1Abrams/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/M1Abrams/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_M160MortarShell/SoldierEB/5m/M113-cover
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
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/SoldierEB/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_M160MortarShell/UAZ/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/UAZ/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_M160MortarShell/M1Abrams/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/M1Abrams/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_M160MortarShell/SoldierEB/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/SoldierEB/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_M160MortarShell/UAZ/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/UAZ/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_M160MortarShell/M1Abrams/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/M1Abrams/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_M160MortarShell/SoldierEB/15m/M113-cover
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
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/SoldierEB/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_M160MortarShell/UAZ/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/UAZ/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_M160MortarShell/M1Abrams/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/M1Abrams/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_M160MortarShell/SoldierEB/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/SoldierEB/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_M160MortarShell/UAZ/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/UAZ/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_M160MortarShell/M1Abrams/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/M1Abrams/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_M160MortarShell/SoldierEB/30m/M113-cover
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
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/SoldierEB/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_M160MortarShell/UAZ/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/UAZ/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_M160MortarShell/M1Abrams/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_M160MortarShell" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_M160MortarShell/M1Abrams/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_C4/SoldierEB/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/SoldierEB/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_C4/UAZ/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/UAZ/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_C4/M1Abrams/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/M1Abrams/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_C4/SoldierEB/5m/M113-cover
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
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/SoldierEB/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_C4/UAZ/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/UAZ/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_C4/M1Abrams/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/M1Abrams/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_C4/SoldierEB/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/SoldierEB/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_C4/UAZ/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/UAZ/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_C4/M1Abrams/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/M1Abrams/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_C4/SoldierEB/15m/M113-cover
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
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/SoldierEB/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_C4/UAZ/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/UAZ/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_C4/M1Abrams/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/M1Abrams/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_C4/SoldierEB/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/SoldierEB/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_C4/UAZ/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/UAZ/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_C4/M1Abrams/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/M1Abrams/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_C4/SoldierEB/30m/M113-cover
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
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/SoldierEB/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_C4/UAZ/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/UAZ/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_C4/M1Abrams/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_C4" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_C4/M1Abrams/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82/SoldierEB/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/SoldierEB/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82/UAZ/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/UAZ/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82/M1Abrams/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/M1Abrams/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82/SoldierEB/5m/M113-cover
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
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/SoldierEB/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82/UAZ/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/UAZ/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82/M1Abrams/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/M1Abrams/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82/SoldierEB/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/SoldierEB/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82/UAZ/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/UAZ/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82/M1Abrams/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/M1Abrams/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82/SoldierEB/15m/M113-cover
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
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/SoldierEB/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82/UAZ/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/UAZ/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82/M1Abrams/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/M1Abrams/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82/SoldierEB/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/SoldierEB/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82/UAZ/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/UAZ/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82/M1Abrams/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/M1Abrams/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_MK82/SoldierEB/30m/M113-cover
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
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/SoldierEB/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82/UAZ/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/UAZ/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_MK82/M1Abrams/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_MK82" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_MK82/M1Abrams/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_GBU12/SoldierEB/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/SoldierEB/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_GBU12/UAZ/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/UAZ/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_GBU12/M1Abrams/5m/exposed
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/M1Abrams/5m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_GBU12/SoldierEB/5m/M113-cover
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
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/SoldierEB/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_GBU12/UAZ/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/UAZ/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_GBU12/M1Abrams/5m/M113-cover
laTargetPos=[(laImpact select 0)+5,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(5*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/M1Abrams/5m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_GBU12/SoldierEB/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/SoldierEB/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_GBU12/UAZ/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/UAZ/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_GBU12/M1Abrams/15m/exposed
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/M1Abrams/15m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_GBU12/SoldierEB/15m/M113-cover
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
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/SoldierEB/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_GBU12/UAZ/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/UAZ/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_GBU12/M1Abrams/15m/M113-cover
laTargetPos=[(laImpact select 0)+15,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(15*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/M1Abrams/15m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_GBU12/SoldierEB/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
"SoldierEB" createUnit [laTargetPos,laEG,"laVictim=this",1,"PRIVATE"]
removeAllWeapons laVictim
laVictim disableAI "MOVE"
laVictim setUnitPos "UP"
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/SoldierEB/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_GBU12/UAZ/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/UAZ/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_GBU12/M1Abrams/30m/exposed
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/M1Abrams/30m/exposed damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
triSimFrames 5
// LoBo_GBU12/SoldierEB/30m/M113-cover
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
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/SoldierEB/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_GBU12/UAZ/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="UAZ" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/UAZ/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
// LoBo_GBU12/M1Abrams/30m/M113-cover
laTargetPos=[(laImpact select 0)+30,laImpact select 1,0]
laVictim="M1Abrams" createVehicle laTargetPos
triAssert [not isNull laVictim]
laVictim setDir 0
laCover="M113" createVehicle [(laImpact select 0)+(30*0.5),laImpact select 1,0]
triAssert [not isNull laCover]
laCover allowDammage false
laCover setDir 0
triSimFrames 10
laPj="LoBo_GBU12" camCreate [laImpact select 0,laImpact select 1,8]
triAssert [not isNull laPj]
laPj setVelocity [0,0,-100]
triSimFrames 60
logInfo format ["LOBO_BLAST case=LoBo_GBU12/M1Abrams/30m/M113-cover damage=%1 alive=%2 actualDistance=%3 position=%4",getDammage laVictim,alive laVictim,laVictim distance laImpact,getPosASL laVictim]
deleteVehicle laVictim
deleteVehicle laCover
triSimFrames 5
logInfo "LOBO_DAMAGE_MATRIX_COMPLETE"
triEndTest
