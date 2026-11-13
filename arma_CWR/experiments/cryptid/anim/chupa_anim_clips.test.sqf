// ============================================================================
//  Chupacabra clips forced one by one (check 2, loop wraps, joints)
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
triSetView ([] call cvSide)
triSimFrames 6
triScreenshot "01_spawn_idle"
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaIdle"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_idle_twitch %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 6
cvE = 1.1
cvA = 0.8
cvO = 0
triSetView ([] call cv34)
setAccTime 0.4274
logInfo format ["CHB %1 %2 %3 90 0.4274", "c_idle_twitch", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_idle_twitch", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_idle_twitch"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_idle_twitch", time, getMove chupa]
logInfo format ["CHUPA after_idle_twitch %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaStalkF"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_stalk_crouch %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 6
cvE = 1.1
cvA = 0.8
cvO = 0
triSetView ([] call cvSide)
setAccTime 0.3571
logInfo format ["CHB %1 %2 %3 64 0.3571", "c_stalk_crouch", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_stalk_crouch"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_stalk_crouch", time, getMove chupa]
logInfo format ["CHUPA after_stalk_crouch %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaTurnL"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_stalk_crouch_turnl %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 6
cvE = 1.1
cvA = 0.8
cvO = 0
triSetView ([] call cv34)
setAccTime 0.3205
logInfo format ["CHB %1 %2 %3 30 0.3205", "c_stalk_crouch_turnl", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnl"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_stalk_crouch_turnl", time, getMove chupa]
logInfo format ["CHUPA after_stalk_crouch_turnl %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaTurnR"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_stalk_crouch_turnr %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 6
cvE = 1.1
cvA = 0.8
cvO = 0
triSetView ([] call cv34)
setAccTime 0.3205
logInfo format ["CHB %1 %2 %3 30 0.3205", "c_stalk_crouch_turnr", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_stalk_crouch_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_stalk_crouch_turnr"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_stalk_crouch_turnr", time, getMove chupa]
logInfo format ["CHUPA after_stalk_crouch_turnr %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaLope"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_lope %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 6
cvE = 1.1
cvA = 0.8
cvO = 0
triSetView ([] call cvSide)
setAccTime 0.2392
logInfo format ["CHB %1 %2 %3 32 0.2392", "c_lope", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lope", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_lope"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_lope", time, getMove chupa]
logInfo format ["CHUPA after_lope %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaRush"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_sprint_rush %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 6
cvE = 1.1
cvA = 0.8
cvO = 0
triSetView ([] call cvSide)
setAccTime 0.1576
logInfo format ["CHB %1 %2 %3 24 0.1576", "c_sprint_rush", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_sprint_rush", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_sprint_rush"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_sprint_rush", time, getMove chupa]
logInfo format ["CHUPA after_sprint_rush %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaFreeze"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_freeze %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 5.5
cvE = 0.7
cvA = 0.45
cvO = 0
triSetView ([] call cv34)
setAccTime 0.5769
logInfo format ["CHB %1 %2 %3 100 0.5769", "c_freeze", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_freeze", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_freeze"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_freeze", time, getMove chupa]
logInfo format ["CHUPA after_freeze %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaCrawlF"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_crawl %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 5.5
cvE = 0.7
cvA = 0.45
cvO = 0
triSetView ([] call cvSide)
setAccTime 0.2857
logInfo format ["CHB %1 %2 %3 50 0.2857", "c_crawl", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_crawl"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_crawl", time, getMove chupa]
logInfo format ["CHUPA after_crawl %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaCrawlTurnL"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_crawl_turnl %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 5.5
cvE = 0.7
cvA = 0.45
cvO = 0
triSetView ([] call cv34)
setAccTime 0.3205
logInfo format ["CHB %1 %2 %3 36 0.3205", "c_crawl_turnl", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnl", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnl"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_crawl_turnl", time, getMove chupa]
logInfo format ["CHUPA after_crawl_turnl %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaCrawlTurnR"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_crawl_turnr %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 5.5
cvE = 0.7
cvA = 0.45
cvO = 0
triSetView ([] call cv34)
setAccTime 0.3205
logInfo format ["CHB %1 %2 %3 36 0.3205", "c_crawl_turnr", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_crawl_turnr", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_crawl_turnr"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_crawl_turnr", time, getMove chupa]
logInfo format ["CHUPA after_crawl_turnr %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaScuttle"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_flee_scuttle %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 5.5
cvE = 0.7
cvA = 0.45
cvO = 0
triSetView ([] call cvSide)
setAccTime 0.2187
logInfo format ["CHB %1 %2 %3 28 0.2187", "c_flee_scuttle", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_flee_scuttle", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_flee_scuttle"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_flee_scuttle", time, getMove chupa]
logInfo format ["CHUPA after_flee_scuttle %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaGallop"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_gallop %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 5.5
cvE = 0.7
cvA = 0.45
cvO = 0
triSetView ([] call cvSide)
setAccTime 0.1488
logInfo format ["CHB %1 %2 %3 24 0.1488", "c_gallop", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_gallop", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_gallop"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_gallop", time, getMove chupa]
logInfo format ["CHUPA after_gallop %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa switchMove "ChupaFeed"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.05 }
logInfo format ["CHUPA forced_feed %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 5.5
cvE = 0.7
cvA = 0.45
cvO = 0
triSetView ([] call cv34)
setAccTime 0.3606
logInfo format ["CHB %1 %2 %3 80 0.3606", "c_feed", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_feed", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_feed"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_feed", time, getMove chupa]
logInfo format ["CHUPA after_feed %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
cvD = 6
cvE = 1
cvA = 0.7
cvO = 0
triSetView ([] call cv34)
chupa switchMove "ChupaSnapUp"
logInfo format ["CHUPA call_snap_up %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
setAccTime 0.2564
logInfo format ["CHB %1 %2 %3 30 0.2564", "c_snap_up", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_snap_up", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_snap_up"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_snap_up", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.5 }
chupa setPos chAt
chupa setDir 90
cvD = 6
cvE = 1.1
cvA = 0.8
cvO = 0.8
triSetView ([] call cvSide)
chupa switchMove "ChupaLungeStrike"
logInfo format ["CHUPA call_lunge %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
setAccTime 0.2564
logInfo format ["CHB %1 %2 %3 36 0.2564", "c_lunge_strike", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_lunge_strike", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_lunge_strike"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_lunge_strike", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 1 }
chupa setPos [4792, 11000, 0]
chupa setDir 90
chupa switchMove "ChupaGallop"
if (true) then { t0 = time }
triSimUntil { time > t0 + 0.6 }
cvD = 7
cvE = 0.9
cvA = 0.6
cvO = 1.5
chupa playMove "ChupaPounce"
chupa playMove "ChupaFeed"
logInfo format ["CHUPA call_pounce %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
setAccTime 0.1857
logInfo format ["CHB %1 %2 %3 40 0.1857", "c_pounce", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_pounce", time, getMove chupa, getPosASL chupa, getDir chupa]
triSetView ([] call cvSide)
triScreenshot "c_pounce"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_pounce", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 3.2 }
logInfo format ["CHUPA after_pounce %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
cvD = 6
cvE = 1
cvA = 0.6
cvO = -0.6
triSetView ([] call cvSide)
chupa playMove "ChupaRecoil"
logInfo format ["CHUPA call_recoil %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
setAccTime 0.2692
logInfo format ["CHB %1 %2 %3 40 0.2692", "c_recoil", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_recoil", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_recoil"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_recoil", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 1 }
logInfo format ["CHUPA after_recoil %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
chupa setPos chAt
chupa setDir 90
chupa enableAI "ANIM"
chupa switchMove "ChupaIdle"
if (true) then { t0 = time }
triSimUntil { time > t0 + 1 }
cvD = 6
cvE = 1
cvA = 0.5
cvO = 0
triSetView ([] call cv34)
chupa setDammage 1
logInfo format ["CHUPA call_die %1 %2 %3 %4", time, getMove chupa, getPosASL chupa, getDir chupa]
setAccTime 0.3385
logInfo format ["CHB %1 %2 %3 50 0.3385", "c_die", time, getMove chupa]
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
logInfo format ["CHF %1 %2 %3 %4 %5", "c_die", time, getMove chupa, getPosASL chupa, getDir chupa]
triScreenshot "c_die"
setAccTime 1
logInfo format ["CHE %1 %2 %3", "c_die", time, getMove chupa]
if (true) then { t0 = time }
triSimUntil { time > t0 + 1 }
logInfo format ["CHUPA dead %1 %2 alive %3", time, getMove chupa, alive chupa]
triAssert [!(alive chupa)]
triSetView ([] call cv34)
triSimFrames 6
triScreenshot "c_dead"
CHW_QUIT = true
triEndTest
