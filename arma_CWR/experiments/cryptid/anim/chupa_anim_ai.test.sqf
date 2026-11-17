// ============================================================================
//  Chupacabra AI states per mode, speeds, flicker, overshoot, feet
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
chupa setPos [4780, 11000, 0]
chupa setDir 90
chupa setUnitPos "AUTO"
chG setBehaviour "SAFE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_SAFE_LIMITED_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4840, 11000, 0]
logInfo format ["CHUPA mode_AUTO_SAFE_LIMITED_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_SAFE_LIMITED_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4840, 11000, 0]
chupa setDir 270
chupa setUnitPos "AUTO"
chG setBehaviour "SAFE"
chG setSpeedMode "NORMAL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_SAFE_NORMAL_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4780, 11000, 0]
logInfo format ["CHUPA mode_AUTO_SAFE_NORMAL_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_SAFE_NORMAL_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4780, 11000, 0]
chupa setDir 90
chupa setUnitPos "AUTO"
chG setBehaviour "SAFE"
chG setSpeedMode "FULL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_SAFE_FULL_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4840, 11000, 0]
logInfo format ["CHUPA mode_AUTO_SAFE_FULL_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_SAFE_FULL_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4840, 11000, 0]
chupa setDir 270
chupa setUnitPos "AUTO"
chG setBehaviour "CARELESS"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_CARELESS_LIMITED_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4780, 11000, 0]
logInfo format ["CHUPA mode_AUTO_CARELESS_LIMITED_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_CARELESS_LIMITED_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4780, 11000, 0]
chupa setDir 90
chupa setUnitPos "AUTO"
chG setBehaviour "CARELESS"
chG setSpeedMode "NORMAL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_CARELESS_NORMAL_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4840, 11000, 0]
logInfo format ["CHUPA mode_AUTO_CARELESS_NORMAL_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_CARELESS_NORMAL_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4840, 11000, 0]
chupa setDir 270
chupa setUnitPos "AUTO"
chG setBehaviour "AWARE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_AWARE_LIMITED_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4780, 11000, 0]
logInfo format ["CHUPA mode_AUTO_AWARE_LIMITED_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_AWARE_LIMITED_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4780, 11000, 0]
chupa setDir 90
chupa setUnitPos "AUTO"
chG setBehaviour "AWARE"
chG setSpeedMode "NORMAL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_AWARE_NORMAL_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4840, 11000, 0]
logInfo format ["CHUPA mode_AUTO_AWARE_NORMAL_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_AWARE_NORMAL_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4840, 11000, 0]
chupa setDir 270
chupa setUnitPos "AUTO"
chG setBehaviour "AWARE"
chG setSpeedMode "FULL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_AWARE_FULL_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4780, 11000, 0]
logInfo format ["CHUPA mode_AUTO_AWARE_FULL_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_AWARE_FULL_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4780, 11000, 0]
chupa setDir 90
chupa setUnitPos "AUTO"
chG setBehaviour "COMBAT"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_COMBAT_LIMITED_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4840, 11000, 0]
logInfo format ["CHUPA mode_AUTO_COMBAT_LIMITED_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_COMBAT_LIMITED_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4840, 11000, 0]
chupa setDir 270
chupa setUnitPos "AUTO"
chG setBehaviour "COMBAT"
chG setSpeedMode "NORMAL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_COMBAT_NORMAL_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4780, 11000, 0]
logInfo format ["CHUPA mode_AUTO_COMBAT_NORMAL_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_COMBAT_NORMAL_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4780, 11000, 0]
chupa setDir 90
chupa setUnitPos "AUTO"
chG setBehaviour "COMBAT"
chG setSpeedMode "FULL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_COMBAT_FULL_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4840, 11000, 0]
logInfo format ["CHUPA mode_AUTO_COMBAT_FULL_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_COMBAT_FULL_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4840, 11000, 0]
chupa setDir 270
chupa setUnitPos "AUTO"
chG setBehaviour "STEALTH"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_STEALTH_LIMITED_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4780, 11000, 0]
logInfo format ["CHUPA mode_AUTO_STEALTH_LIMITED_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_STEALTH_LIMITED_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4780, 11000, 0]
chupa setDir 90
chupa setUnitPos "AUTO"
chG setBehaviour "STEALTH"
chG setSpeedMode "NORMAL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_STEALTH_NORMAL_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4840, 11000, 0]
logInfo format ["CHUPA mode_AUTO_STEALTH_NORMAL_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_STEALTH_NORMAL_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4840, 11000, 0]
chupa setDir 270
chupa setUnitPos "AUTO"
chG setBehaviour "STEALTH"
chG setSpeedMode "FULL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_AUTO_STEALTH_FULL_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4780, 11000, 0]
logInfo format ["CHUPA mode_AUTO_STEALTH_FULL_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_AUTO_STEALTH_FULL_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4780, 11000, 0]
chupa setDir 90
chupa setUnitPos "UP"
chG setBehaviour "SAFE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_UP_SAFE_LIMITED_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4840, 11000, 0]
logInfo format ["CHUPA mode_UP_SAFE_LIMITED_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_UP_SAFE_LIMITED_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4840, 11000, 0]
chupa setDir 270
chupa setUnitPos "UP"
chG setBehaviour "AWARE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_UP_AWARE_LIMITED_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4780, 11000, 0]
logInfo format ["CHUPA mode_UP_AWARE_LIMITED_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_UP_AWARE_LIMITED_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4780, 11000, 0]
chupa setDir 90
chupa setUnitPos "UP"
chG setBehaviour "COMBAT"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_UP_COMBAT_LIMITED_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4840, 11000, 0]
logInfo format ["CHUPA mode_UP_COMBAT_LIMITED_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_UP_COMBAT_LIMITED_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4840, 11000, 0]
chupa setDir 270
chupa setUnitPos "UP"
chG setBehaviour "SAFE"
chG setSpeedMode "FULL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_UP_SAFE_FULL_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4780, 11000, 0]
logInfo format ["CHUPA mode_UP_SAFE_FULL_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_UP_SAFE_FULL_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4780, 11000, 0]
chupa setDir 90
chupa setUnitPos "DOWN"
chG setBehaviour "SAFE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_DOWN_SAFE_LIMITED_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4840, 11000, 0]
logInfo format ["CHUPA mode_DOWN_SAFE_LIMITED_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_DOWN_SAFE_LIMITED_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4840, 11000, 0]
chupa setDir 270
chupa setUnitPos "DOWN"
chG setBehaviour "AWARE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_DOWN_AWARE_LIMITED_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4780, 11000, 0]
logInfo format ["CHUPA mode_DOWN_AWARE_LIMITED_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_DOWN_AWARE_LIMITED_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4780, 11000, 0]
chupa setDir 90
chupa setUnitPos "DOWN"
chG setBehaviour "COMBAT"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_DOWN_COMBAT_LIMITED_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4840, 11000, 0]
logInfo format ["CHUPA mode_DOWN_COMBAT_LIMITED_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_DOWN_COMBAT_LIMITED_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4840, 11000, 0]
chupa setDir 270
chupa setUnitPos "DOWN"
chG setBehaviour "SAFE"
chG setSpeedMode "FULL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA mode_DOWN_SAFE_FULL_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4780, 11000, 0]
logInfo format ["CHUPA mode_DOWN_SAFE_FULL_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA mode_DOWN_SAFE_FULL_moving %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4770, 11000, 0]
chupa setDir 90
chupa setUnitPos "DOWN"
chG setBehaviour "SAFE"
chG setSpeedMode "FULL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 2.5 }
logInfo format ["CHUPA speed_gallop_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4860, 11000, 0]
logInfo format ["CHUPA speed_gallop_go %1 %2 %3 %4 target 4860", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA speed_gallop_foot %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 4.5
cvE = 0.35
cvA = 0.15
cvO = 2
chFoot = [] call cvSide
triSetView chFoot
setAccTime 0.1923
logInfo format ["CHB %1 %2 %3 24 0.1923", "f_gallop", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_gallop"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "f_gallop", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 12 }
logInfo format ["CHUPA speed_gallop_late %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 4 }
logInfo format ["CHUPA speed_gallop_rest %1 %2 %3 %4 target 4860", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4770, 11000, 0]
chupa setDir 90
chupa setUnitPos "UP"
chG setBehaviour "SAFE"
chG setSpeedMode "FULL"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 2.5 }
logInfo format ["CHUPA speed_rush_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4860, 11000, 0]
logInfo format ["CHUPA speed_rush_go %1 %2 %3 %4 target 4860", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA speed_rush_foot %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 4.5
cvE = 0.45
cvA = 0.15
cvO = 1.8
chFoot = [] call cvSide
triSetView chFoot
setAccTime 0.1763
logInfo format ["CHB %1 %2 %3 24 0.1763", "f_rush", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_rush"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "f_rush", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 12 }
logInfo format ["CHUPA speed_rush_late %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 4 }
logInfo format ["CHUPA speed_rush_rest %1 %2 %3 %4 target 4860", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4770, 11000, 0]
chupa setDir 90
chupa setUnitPos "AUTO"
chG setBehaviour "AWARE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 2.5 }
logInfo format ["CHUPA speed_lope_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4810, 11000, 0]
logInfo format ["CHUPA speed_lope_go %1 %2 %3 %4 target 4810", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 4 }
logInfo format ["CHUPA speed_lope_foot %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 4.5
cvE = 0.45
cvA = 0.15
cvO = 1.2
chFoot = [] call cvSide
triSetView chFoot
setAccTime 0.2308
logInfo format ["CHB %1 %2 %3 30 0.2308", "f_lope", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_lope"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "f_lope", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA speed_lope_late %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 4 }
logInfo format ["CHUPA speed_lope_rest %1 %2 %3 %4 target 4810", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4770, 11000, 0]
chupa setDir 90
chupa setUnitPos "AUTO"
chG setBehaviour "SAFE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 2.5 }
logInfo format ["CHUPA speed_stalk_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4790, 11000, 0]
logInfo format ["CHUPA speed_stalk_go %1 %2 %3 %4 target 4790", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 4 }
logInfo format ["CHUPA speed_stalk_foot %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 4.5
cvE = 0.45
cvA = 0.15
cvO = 0.5
chFoot = [] call cvSide
triSetView chFoot
setAccTime 0.3462
logInfo format ["CHB %1 %2 %3 40 0.3462", "f_stalk", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_stalk", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_stalk"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "f_stalk", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA speed_stalk_late %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 4 }
logInfo format ["CHUPA speed_stalk_rest %1 %2 %3 %4 target 4790", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4770, 11000, 0]
chupa setDir 90
chupa setUnitPos "DOWN"
chG setBehaviour "AWARE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 2.5 }
logInfo format ["CHUPA speed_scuttle_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4810, 11000, 0]
logInfo format ["CHUPA speed_scuttle_go %1 %2 %3 %4 target 4810", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 4 }
logInfo format ["CHUPA speed_scuttle_foot %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 4.5
cvE = 0.35
cvA = 0.15
cvO = 1
chFoot = [] call cvSide
triSetView chFoot
setAccTime 0.2564
logInfo format ["CHB %1 %2 %3 24 0.2564", "f_scuttle", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_scuttle"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "f_scuttle", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA speed_scuttle_late %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 4 }
logInfo format ["CHUPA speed_scuttle_rest %1 %2 %3 %4 target 4810", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4770, 11000, 0]
chupa setDir 90
chupa setUnitPos "DOWN"
chG setBehaviour "SAFE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 2.5 }
logInfo format ["CHUPA speed_crawl_stopped %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa doMove [4785, 11000, 0]
logInfo format ["CHUPA speed_crawl_go %1 %2 %3 %4 target 4785", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 4 }
logInfo format ["CHUPA speed_crawl_foot %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 4.5
cvE = 0.35
cvA = 0.15
cvO = 0.4
chFoot = [] call cvSide
triSetView chFoot
setAccTime 0.2692
logInfo format ["CHB %1 %2 %3 40 0.2692", "f_crawl", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "f_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "f_crawl"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "f_crawl", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 7 }
logInfo format ["CHUPA speed_crawl_late %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 4 }
logInfo format ["CHUPA speed_crawl_rest %1 %2 %3 %4 target 4785", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos [4800, 11000, 0]
chupa setDir 90
chupa setUnitPos "DOWN"
chG setBehaviour "SAFE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA turn_down_still %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chAnimN = 0
chupa doMove [4775, 11000, 0]
logInfo format ["CHUPA turn_down_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 2 }
logInfo format ["CHUPA turn_down_count2 %1 %2 anims %3 dir %4", time, getMove chupa, chAnimN, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 2 }
logInfo format ["CHUPA turn_down_count4 %1 %2 anims %3 dir %4", time, getMove chupa, chAnimN, getDir chupa]
chupa setPos [4800, 11000, 0]
chupa setDir 90
chupa setUnitPos "UP"
chG setBehaviour "SAFE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA turn_up_still %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chAnimN = 0
chupa doMove [4775, 11000, 0]
logInfo format ["CHUPA turn_up_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 2 }
logInfo format ["CHUPA turn_up_count2 %1 %2 anims %3 dir %4", time, getMove chupa, chAnimN, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 2 }
logInfo format ["CHUPA turn_up_count4 %1 %2 anims %3 dir %4", time, getMove chupa, chAnimN, getDir chupa]
chupa setPos [4800, 11000, 0]
chupa setDir 90
chupa setUnitPos "AUTO"
chG setBehaviour "STEALTH"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA turn_stealth_still %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chAnimN = 0
chupa doMove [4775, 11000, 0]
logInfo format ["CHUPA turn_stealth_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 2 }
logInfo format ["CHUPA turn_stealth_count2 %1 %2 anims %3 dir %4", time, getMove chupa, chAnimN, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 2 }
logInfo format ["CHUPA turn_stealth_count4 %1 %2 anims %3 dir %4", time, getMove chupa, chAnimN, getDir chupa]
chupa setPos [4800, 11000, 0]
chupa setDir 90
chupa setUnitPos "DOWN"
chG setBehaviour "SAFE"
chG setSpeedMode "LIMITED"
chupa doMove (getPos chupa)
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
chAnimN = 0
chupa doWatch [4750, 11020, 0]
logInfo format ["CHUPA turn_watch_go %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 3 }
logInfo format ["CHUPA turn_watch_count3 %1 %2 anims %3 dir %4", time, getMove chupa, chAnimN, getDir chupa]
triAssert [alive chupa]
logInfo format ["CHUPA ai_end %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
CHW_QUIT = true
triEndTest
