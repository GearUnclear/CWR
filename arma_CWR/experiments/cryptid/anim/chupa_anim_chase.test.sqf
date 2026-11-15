// ============================================================================
//  Chupacabra video chase: gallop, pounce, feed, recoil, rush, lunge
//  Written by gen_chupa_anim_tests.py (edit that, not this file).
//  UDChupacabraAnim (CfgMovesChupa) on the Everon airfield edge at 13:00.
//  Quiet: no weapons, nobody fires, --nosound. CHUPA/CHF/CHW lines carry
//  sim time, move, ASL position and heading for the offline checks.
// ============================================================================
triSimUntil { alive player }
triCaptureAtPresent true
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [4800, 10000, 0]
setDate [1985, 5, 10, 13, 0]
0 setOvercast 0.3
0 setFog 0
setViewDistance 1200
showCinemaBorder false
chC = createCenter civilian
chG = createGroup civilian
chupa = objNull
chAt = [4800, 11000, 0]
"UDChupacabraAnim" createUnit [chAt, chG, "chupa = this", 0.5, "PRIVATE"]
logInfo format ["CHUPA spawn %1 %2", time, getMove chupa]
triSimFrames 2
triAssert [!(isNull chupa)]
triAssertEq [typeOf chupa, "UDChupacabraAnim"]
triAssert [alive chupa]
chAnimN = 0
chupa addEventHandler ["AnimChanged", {chAnimN = chAnimN + 1; logInfo format ["CHUPA anim %1 %2", time, _this select 1]}]
chupa setCaptive true
chupa setDir 90
chG setBehaviour "SAFE"
CHW_ON = true
CHW_QUIT = false
[chupa, 0.05] exec "chupa_watch.sqs"
chCam = "camera" camCreate [4800, 10500, 30]
chCam cameraEffect ["internal", "back"]
chCam camSetTarget [4800, 10700, 30]
chCam camSetFov 0.5
chCam camCommit 0
cvD = 6
cvO = 0
cvE = 1.1
cvA = 0.8
cvSide = { private ["_p"]; _p = getPosASL chupa; [(_p select 0) + cvO, (_p select 2) + cvE, (_p select 1) - cvD, -cvO, cvA - cvE, cvD] }
cv34 = { private ["_p"]; _p = getPosASL chupa; [(_p select 0) + cvD * 0.64, (_p select 2) + cvE, (_p select 1) - cvD * 0.77, -cvD * 0.64, cvA - cvE, cvD * 0.77] }
triSimFrames 150
logInfo format ["CHUPA settled %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triSimFrames 6
triScreenshot "00_warmup_discard"
chV = createGroup civilian
victim = objNull
CHS_POUNCE_AT = 3.5
CHS_DROP_AFTER = 0.35
CHS_LUNGE_AT = 3.0
CHS_FEED_HOLD = 3
CHS_RUSH_DELAY = 1.5
CHS_CHASE_MAX = 20
CHS_REORDER = 30
CHS_SIDE = 2.2
CHS_RUSH_GAP = 6
if (!(isNull victim)) then { deleteVehicle victim }
triSimFrames 2
"Civilian" createUnit [[4754, 11000, 0], chV, "victim = this", 0.5, "PRIVATE"]
triSimFrames 2
triAssert [!(isNull victim)]
victim setCaptive true
victim allowDammage false
victim setDir 90
chV setBehaviour "CARELESS"
chupa stop false
chupa enableAI "MOVE"
chupa setPos [4740, 10997.8, 0]
chupa setDir 90
chupa setUnitPos "DOWN"
chG setBehaviour "SAFE"
chG setSpeedMode "FULL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA chase_rt_ready %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
logInfo format ["CHUPA chase_rt_victim %1 %2 %3", time, getMove victim, getPosASL victim]
CHS_DONE = false
[chupa, victim] exec "chupa_chase.sqs"
triSimUntil { CHS_DONE }
logInfo format ["CHUPA chase_rt_done %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
logInfo format ["CHUPA chase_rt_victim_end %1 %2 %3 damage %4", time, getMove victim, getPosASL victim, damage victim]
if (true) then { t0 = time }
triSimUntil { time > t0 + 2 }
if (!(isNull victim)) then { deleteVehicle victim }
triSimFrames 2
"Civilian" createUnit [[4754, 11000, 0], chV, "victim = this", 0.5, "PRIVATE"]
triSimFrames 2
triAssert [!(isNull victim)]
victim setCaptive true
victim allowDammage false
victim setDir 90
chV setBehaviour "CARELESS"
chupa stop false
chupa enableAI "MOVE"
chupa setPos [4740, 10997.8, 0]
chupa setDir 90
chupa setUnitPos "DOWN"
chG setBehaviour "SAFE"
chG setSpeedMode "FULL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA chase_slow_ready %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
logInfo format ["CHUPA chase_slow_victim %1 %2 %3", time, getMove victim, getPosASL victim]
CHS_DONE = false
[chupa, victim] exec "chupa_chase.sqs"
chMid = { private ["_a", "_b", "_mx", "_mz", "_h"]; _a = getPosASL chupa; _b = getPosASL victim; _mx = ((_a select 0) + (_b select 0)) / 2; _mz = ((_a select 1) + (_b select 1)) / 2; _h = ((_a select 2) + (_b select 2)) / 2; [_mx, _h + 1.3, _mz - 9, 0, -0.6, 9] }
triSetView ([] call chMid)
setAccTime 0.2286
logInfo format ["CHB %1 %2 %3 500 0.2286", "chase", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
logInfo format ["CHF %1 %2 %3 %4 %5", "chase", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call chMid)
triScreenshot "chase"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "chase", time, getMove chupa]
triSimUntil { CHS_DONE }
logInfo format ["CHUPA chase_slow_done %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
logInfo format ["CHUPA chase_slow_victim_end %1 %2 %3 damage %4", time, getMove victim, getPosASL victim, damage victim]
if (true) then { t0 = time }
triSimUntil { time > t0 + 2 }
triAssert [alive chupa]
logInfo format ["CHUPA chase_end %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
CHW_QUIT = true
triEndTest
