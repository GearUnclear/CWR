// Hosted LAN startup, following the repository's hosted-first-join fixture.
// Mission is staged under this isolated user's content/MPMissions directory.
triAssertEq [triDisplay,0]
triClick 105
triAssertEq [triDisplay,8]
triInvokeButton 104
triAssertEq [triDisplay,17]
triAssertEq [triSelectListByData [101,"abel"],true]
triAssertEq [triSelectListByData [102,"WreckFire"],true]
triScreenshot "host_wreck_mission_selected"
triInvokeButton 1
triAssertEq [triDisplay,70]
triAssertEq [triMpAssignSelfSlot "WEST:3","OK"]
triBarrier hosted_setup
triMpWaitSlotTaken WEST:1
triMpWaitSlotTaken WEST:2
triBarrier hosted_slots_taken
triInvokeButton 1
triAssertNgsClient 12
triAssertNgsClient 13
triAssertEq [triMpClientReady 14,"OK"]
triAssertNgsClient 14
triAssertEq [triDisplay,52]
triInvokeButton 1
if (triDisplay == 204) then {triInvokeButton 1}
triWait 1500
triAssertMissionPlayable
triAssertEq [triPlayerAuthority,"OK"]
triAssertEq [player == lwHost,true]
player setCaptive true
removeAllWeapons player
triBarrier hosted_playable
// Actual host collision assertions are retained; client tank replication is observational only.
logInfo "LOBO_MP_TANK_SCOPE role=host client_replication_observational=true"
triAssertEq [local lwM1,true]
triAssertEq [local lwMerkava,true]
triAssertEq [typeOf lwM1,"LoBo_M1"]
triAssertEq [typeOf lwMerkava,"LoBoMerkava2"]
triAssertEq [getDammage lwM1,0]
triAssertEq [getDammage lwMerkava,0]
lwTankPos=getPos lwM1
lwTankRound="LoBo_105Sabot2" camCreate [lwTankPos select 0,lwTankPos select 1,(lwTankPos select 2)+12]
lwTankRound setVelocity [0,0,-300]
lwTankPos=getPos lwMerkava
lwTankRound="LoBo_105Sabot2" camCreate [lwTankPos select 0,lwTankPos select 1,(lwTankPos select 2)+12]
lwTankRound setVelocity [0,0,-300]
triAssertGt [getDammage lwM1,0]
triAssertGt [getDammage lwMerkava,0]
triAssertGt [(lwM1 getSelectionDamage "hull")+(lwM1 getSelectionDamage "turet"),0]
triAssertGt [(lwMerkava getSelectionDamage "hull")+(lwMerkava getSelectionDamage "turet"),0]
logInfo format ["LOBO_MP_TANK role=host method=vertical_LoBo_105Sabot2 class=%1 total=%2 hull=%3 turet=%4 names=%5",typeOf lwM1,getDammage lwM1,lwM1 getSelectionDamage "hull",lwM1 getSelectionDamage "turet",GetHitpointsNames lwM1]
logInfo format ["LOBO_MP_TANK role=host method=vertical_LoBo_105Sabot2 class=%1 total=%2 hull=%3 turet=%4 names=%5",typeOf lwMerkava,getDammage lwMerkava,lwMerkava getSelectionDamage "hull",lwMerkava getSelectionDamage "turet",GetHitpointsNames lwMerkava]
triBarrier tank_impacts_replicated
// The hosted owner controls the empty Toyota and local AI, while both victims are remote.
// No direct exec is used on the owner: setDammage invokes its installed killed handler.
triAssertNgsClient 14
logInfo format ["LOBO_MP_WRECK_PRECONDITION role=host player=%1 wreckClass=%2 wreckLocal=%3 aiLocal=%4 player1Local=%5 player2Local=%6 hostIsPlayer=%7",typeOf player,typeOf lwWreck,local lwWreck,local lwAI,local lwPlayer1,local lwPlayer2,player==lwHost]
triAssertEq [local lwWreck, true]
triAssertEq [local lwAI, true]
triAssertEq [local lwPlayer1, false]
triAssertEq [local lwPlayer2, false]
triAssertEq [(group lwAI) == (group lwPlayer1), false]
triAssertEq [(group lwAI) == (group lwPlayer2), false]
triAssertEq [(group lwPlayer1) == (group lwPlayer2), false]
lwBase = getPos lwWreck
lwGlowCount = {lwN=0; {if ((typeOf _x)=="M1Smoker") then {lwN=lwN+1}} forEach (nearestObjects [lwBase,[],30]); lwN}
triAssertEq [call lwGlowCount, 0]
lwAI setDammage 0
lwAI setCaptive true
removeAllWeapons lwAI
lwAI disableAI "MOVE"
triBarrier wreck_ready
lwWreck setDammage 1
lwStart = time
logInfo format ["LOBO_FIRE_OWNER killed=%1 owner=%2 player1local=%3 player2local=%4", getDammage lwWreck, local lwWreck, local lwPlayer1, local lwPlayer2]
triAssertEq [call lwGlowCount, 1]
// Native random secondary explosions are confined to the first 45 script seconds.
// Keep victims far away until this warmup completes, then measure only burn damage.
triAssertGe [time, lwStart + 50]
triBarrier fire_warm
lwAI setPos [(lwBase select 0)+7, lwBase select 1, 0]
lwAI setDammage 0
triBarrier victims_ready
triAssertGe [getDammage lwAI, 0.019]
lwFirst = time
logInfo format ["LOBO_FIRE_LOCAL_FIRST damage=%1 elapsed=%2 glow=%3", getDammage lwAI, time-lwStart, call lwGlowCount]
triAssertLe [getDammage lwAI, 0.021]
triAssertGe [time, lwFirst + 21]
logInfo format ["LOBO_FIRE_LOCAL_WINDOW damage=%1 elapsed=%2 glow=%3", getDammage lwAI, time-lwFirst, call lwGlowCount]
triAssertGe [getDammage lwAI, 0.049]
triAssertLe [getDammage lwAI, 0.081]
triAssertEq [call lwGlowCount, 1]
triBarrier burn_window_done
lwAI setPos [(lwBase select 0)-100, lwBase select 1, 0]
triAssertGe [time, lwStart + 165]
triAssertEq [call lwGlowCount, 0]
triAssertEq [isNull lwWreck, false]
logInfo format ["LOBO_FIRE_OWNER_CLEANUP elapsed=%1 glow=%2 wreck_exists=%3 damage=%4", time-lwStart, call lwGlowCount, not (isNull lwWreck), getDammage lwAI]
triBarrier natural_cleanup_done
triEndTest
