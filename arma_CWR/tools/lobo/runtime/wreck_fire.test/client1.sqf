triAssertEq [triDisplay,0]
triBarrier hosted_setup
// Direct harness join is restricted to the local host's exact ephemeral port.
triMpJoin 127.0.0.1 ${ports.game}
triAssertEq [triDisplay,70]
triAssertEq [triMpAssignSelfSlot "WEST:1","OK"]
triBarrier hosted_slots_taken
triAssertNgsClient 12
triAssertNgsClient 14
triInvokeButton 1
if (triDisplay == 204) then {triInvokeButton 1}
triWait 1500
triAssertMissionPlayable
triBarrier hosted_playable
// Observational tank phase: a passing fire test does not establish damage replication.
triWait 3000
logInfo "LOBO_MP_TANK_SCOPE role=client1 observational=true replication_asserted=false"
logInfo format ["LOBO_MP_TANK role=client1 method=observe_after_host_impact class=%1 local=%2 total=%3 hull=%4 turet=%5 names=%6",typeOf lwM1,local lwM1,getDammage lwM1,lwM1 getSelectionDamage "hull",lwM1 getSelectionDamage "turet",GetHitpointsNames lwM1]
logInfo format ["LOBO_MP_TANK role=client1 method=observe_after_host_impact class=%1 local=%2 total=%3 hull=%4 turet=%5 names=%6",typeOf lwMerkava,local lwMerkava,getDammage lwMerkava,lwMerkava getSelectionDamage "hull",lwMerkava getSelectionDamage "turet",GetHitpointsNames lwMerkava]
triBarrier tank_impacts_replicated
triAssertNgsClient 14
triAssertMissionPlayable
triAssertEq [triPlayerAuthority, "OK"]
triAssertEq [player == lwPlayer1, true]
logInfo format ["LOBO_MP_WRECK_PRECONDITION role=client1 player=%1 wreckClass=%2 wreckLocal=%3 aiLocal=%4 player1Local=%5 player2Local=%6 player1IsPlayer=%7 player2IsPlayer=%8",typeOf player,typeOf lwWreck,local lwWreck,local lwAI,local lwPlayer1,local lwPlayer2,player==lwPlayer1,player==lwPlayer2]
triAssertEq [local lwWreck, false]
triAssertEq [local lwPlayer2, false]
triAssertEq [local lwAI, false]
triAssertEq [(group player) == (group lwAI), false]
player setCaptive true
removeAllWeapons player
player setDammage 0
lwBase = getPos lwWreck
lwGlowCount = {lwN=0; {if ((typeOf _x)=="M1Smoker") then {lwN=lwN+1}} forEach (nearestObjects [lwBase,[],30]); lwN}
lwCam = "camera" camCreate lwBase
lwCam cameraEffect ["internal", "back"]
lwAim = {lwP=getPosASL lwWreck; triSetView [(lwP select 0)+15,(lwP select 2)+5,(lwP select 1)+15,-15,-3,-15]}
triBarrier wreck_ready
triAssertEq [alive lwWreck, false]
triAssertEq [call lwGlowCount, 1]
call lwAim
triWait 500
triScreenshot "client1_owner_killed_fire"
triBarrier fire_warm
// Deliberately execute the installed script on one non-owner too. This
// exercises its sound fallback and proves this branch adds no damage scheduler.
[lwWreck] exec "\LoBoTer\scripts\fire.sqs"
lwRemoteStart = time
player setPos [(lwBase select 0)+-7,(lwBase select 1)+0,0]
player setDammage 0
triBarrier victims_ready
triAssertGe [getDammage player, 0.019]
lwFirst = time
logInfo format ["LOBO_FIRE_REMOTE_FIRST client=1 damage=%1 local=%2 wreck_local=%3 glow=%4", getDammage player, local player, local lwWreck, call lwGlowCount]
triAssertLe [getDammage player, 0.021]
call lwAim
triWait 250
triScreenshot "client1_remote_burn"
triAssertGe [time, lwFirst + 21]
logInfo format ["LOBO_FIRE_REMOTE_WINDOW client=1 damage=%1 elapsed=%2 glow=%3", getDammage player, time-lwFirst, call lwGlowCount]
triAssertGe [getDammage player, 0.049]
triAssertLe [getDammage player, 0.081]
triAssertEq [call lwGlowCount, 1]
triBarrier burn_window_done
player setPos [(lwBase select 0)-100,(lwBase select 1)+10,0]
// Wait past both the owner's 150 s lifetime and the explicit remote lifetime.
triAssertGe [time, lwRemoteStart + 165]
triAssertEq [call lwGlowCount, 0]
triAssertEq [isNull lwWreck, false]
logInfo format ["LOBO_FIRE_REMOTE_CLEANUP client=1 elapsed=%1 glow=%2 wreck_exists=%3 damage=%4", time-lwRemoteStart, call lwGlowCount, not (isNull lwWreck), getDammage player]
call lwAim
triWait 250
triScreenshot "client1_natural_cleanup"
lwCam cameraEffect ["terminate","back"]
camDestroy lwCam
triBarrier natural_cleanup_done
triEndTest
