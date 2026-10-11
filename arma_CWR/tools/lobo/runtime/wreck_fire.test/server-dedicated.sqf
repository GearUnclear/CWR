// Historical dedicated role reference, not selected by the active fixture.
// The mission owns an empty Toyota on the dedicated server.
// No direct exec is used on the owner: setDammage invokes its installed killed handler.
triAssertNgs 14
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
