// ============================================================================
//  Chupacabra joints in motion (mesh A/B)
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
chupa disableAI "MOVE"
chupa disableAI "ANIM"
chupa stop true
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaStalkF"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
cvD = 2.4
cvE = 0.55
cvA = 0.45
cvO = 0
triSetView ([] call cvSide)
setAccTime 0.4190
logInfo format ["CHB %1 %2 %3 48 0.4190", "j_stalk_knee", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_stalk_knee"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "j_stalk_knee", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaStalkF"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
cvD = 2.6
cvE = 0.7
cvA = 0.45
cvO = 0
triSetView ([] call cv34)
setAccTime 0.4190
logInfo format ["CHB %1 %2 %3 48 0.4190", "j_stalk_knee34", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_stalk_knee34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_stalk_knee34"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "j_stalk_knee34", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaGallop"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
cvD = 2.8
cvE = 0.45
cvA = 0.35
cvO = 0
triSetView ([] call cvSide)
setAccTime 0.1310
logInfo format ["CHB %1 %2 %3 24 0.1310", "j_gallop_limbs", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "j_gallop_limbs"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "j_gallop_limbs", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaGallop"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
cvD = 3
cvE = 0.6
cvA = 0.35
cvO = 0
triSetView ([] call cv34)
setAccTime 0.1310
logInfo format ["CHB %1 %2 %3 24 0.1310", "j_gallop_limbs34", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_gallop_limbs34", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_gallop_limbs34"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "j_gallop_limbs34", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaCrawlF"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
cvD = 2.6
cvE = 0.7
cvA = 0.35
cvO = 0
triSetView ([] call cv34)
setAccTime 0.3143
logInfo format ["CHB %1 %2 %3 40 0.3143", "j_crawl_elbow", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_crawl_elbow", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_crawl_elbow"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "j_crawl_elbow", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaScuttle"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
cvD = 2.6
cvE = 0.7
cvA = 0.45
cvO = 0
triSetView ([] call cv34)
setAccTime 0.2072
logInfo format ["CHB %1 %2 %3 26 0.2072", "j_scuttle_neck", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_scuttle_neck", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_scuttle_neck"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "j_scuttle_neck", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaFeed"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
cvD = 2.2
cvE = 0.8
cvA = 0.3
cvO = 0
triSetView ([] call cv34)
setAccTime 0.3929
logInfo format ["CHB %1 %2 %3 48 0.3929", "j_feed_wrist", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_feed_wrist", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_feed_wrist"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "j_feed_wrist", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.3 }
chupa setPos [4792, 11000, 0]
chupa setDir 90
chupa switchMove "ChupaGallop"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.6 }
cvD = 3.6
cvE = 1.2
cvA = 0.6
cvO = 2.2
chupa playMove "ChupaPounce"
chupa playMove "ChupaFeed"
logInfo format ["CHUPA call_pounce %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
setAccTime 0.1571
logInfo format ["CHB %1 %2 %3 40 0.1571", "j_pounce_shoulder", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_pounce_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cv34)
triScreenshot "j_pounce_shoulder"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "j_pounce_shoulder", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 3.4 }
cvD = 3.4
cvE = 1.1
cvA = 0.6
cvO = -0.4
triSetView ([] call cv34)
chupa playMove "ChupaRecoil"
logInfo format ["CHUPA call_recoil %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
setAccTime 0.2308
logInfo format ["CHB %1 %2 %3 40 0.2308", "j_recoil_shoulder", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
logInfo format ["CHF %1 %2 %3 %4 %5", "j_recoil_shoulder", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "j_recoil_shoulder"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "j_recoil_shoulder", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 1 }
triAssert [alive chupa]
logInfo format ["CHUPA joints_end %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
CHW_QUIT = true
triEndTest
