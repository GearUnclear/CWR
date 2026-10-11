// LoBo issue #1 landing-craft number visuals.
// Both native config Init handlers run the repaired numbering scripts.
// The legacy auxiliary speed/flag initializer expects a mission object named
// Server. Use its native non-server branch here; this probe covers numbers only.
// No textures are assigned by this probe, and no numbering script is called directly.
// Screenshot review is required: this engine has no object-texture getter.
triSimUntil { alive player }
setDate [1985,6,15,12,0]
0 setOvercast 0
0 setFog 0
setViewDistance 1800
showCinemaBorder false
player setCaptive true
player allowDammage false
Server = objNull
logInfo "LOBO_LCM_SCOPE native_number_init=true auxiliary_server_branch=false visual_review_required=true"
lcCam = "camera" camCreate [7800,5700,10]
lcCam cameraEffect ["internal","back"]
lcAim = {
    lcP=getPosASL lcBoat;
    lcCx=(lcP select 0)+(lcOff select 0);
    lcCy=(lcP select 2)+(lcOff select 1);
    lcCz=(lcP select 1)+(lcOff select 2);
    triSetView [lcCx,lcCy,lcCz,(lcP select 0)-lcCx,(lcP select 2)+lcHeight-lcCy,(lcP select 1)+lcAimN-lcCz]
}
lcBoats = []

lcBoat = "LoBo_LCM_IDF" createVehicle [7800,5700,0]
triAssertEq [isNull lcBoat, false]
triAssertEq [typeOf lcBoat, "LoBo_LCM_IDF"]
lcBoats = lcBoats + [lcBoat]
lcBoat setDir 0
lcBoat allowDammage false
lcBoat setVelocity [0,0,0]
lcReady = time + 1
triSimUntil { time >= lcReady }
logInfo format ["LOBO_LCM_NATIVE_INIT class=%1 local=%2 alive=%3 position=%4 expected_digit_slots=2,3,4",typeOf lcBoat,local lcBoat,alive lcBoat,getPos lcBoat]

lcOff = [-25,5,0]
lcAimN = 0
lcHeight = 1.5
call lcAim
triSimFrames 3
triScreenshot "lcm_idf_port_overview"

lcOff = [25,5,0]
lcAimN = 0
lcHeight = 1.5
call lcAim
triSimFrames 3
triScreenshot "lcm_idf_starboard_overview"

lcOff = [20,6,23]
lcAimN = 0
lcHeight = 1.5
call lcAim
triSimFrames 3
triScreenshot "lcm_idf_bow_quarter"

lcOff = [-20,6,-23]
lcAimN = 0
lcHeight = 1.5
call lcAim
triSimFrames 3
triScreenshot "lcm_idf_stern_quarter"

lcOff = [-12,3.2,8]
lcAimN = 8
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_idf_port_forward_digits"

lcOff = [12,3.2,8]
lcAimN = 8
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_idf_starboard_forward_digits"

lcOff = [-12,3.2,-8]
lcAimN = -8
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_idf_port_aft_digits"

lcOff = [12,3.2,-8]
lcAimN = -8
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_idf_starboard_aft_digits"

lcOff = [0,3.5,24]
lcAimN = 10
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_idf_bow_digits"

lcOff = [0,3.5,-24]
lcAimN = -10
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_idf_stern_digits"

lcBoat = "LoBo_LCM_Egy" createVehicle [7860,5700,0]
triAssertEq [isNull lcBoat, false]
triAssertEq [typeOf lcBoat, "LoBo_LCM_Egy"]
lcBoats = lcBoats + [lcBoat]
lcBoat setDir 0
lcBoat allowDammage false
lcBoat setVelocity [0,0,0]
lcReady = time + 1
triSimUntil { time >= lcReady }
logInfo format ["LOBO_LCM_NATIVE_INIT class=%1 local=%2 alive=%3 position=%4 expected_digit_slots=2,3,4",typeOf lcBoat,local lcBoat,alive lcBoat,getPos lcBoat]

lcOff = [-25,5,0]
lcAimN = 0
lcHeight = 1.5
call lcAim
triSimFrames 3
triScreenshot "lcm_egypt_port_overview"

lcOff = [25,5,0]
lcAimN = 0
lcHeight = 1.5
call lcAim
triSimFrames 3
triScreenshot "lcm_egypt_starboard_overview"

lcOff = [20,6,23]
lcAimN = 0
lcHeight = 1.5
call lcAim
triSimFrames 3
triScreenshot "lcm_egypt_bow_quarter"

lcOff = [-20,6,-23]
lcAimN = 0
lcHeight = 1.5
call lcAim
triSimFrames 3
triScreenshot "lcm_egypt_stern_quarter"

lcOff = [-12,3.2,8]
lcAimN = 8
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_egypt_port_forward_digits"

lcOff = [12,3.2,8]
lcAimN = 8
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_egypt_starboard_forward_digits"

lcOff = [-12,3.2,-8]
lcAimN = -8
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_egypt_port_aft_digits"

lcOff = [12,3.2,-8]
lcAimN = -8
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_egypt_starboard_aft_digits"

lcOff = [0,3.5,24]
lcAimN = 10
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_egypt_bow_digits"

lcOff = [0,3.5,-24]
lcAimN = -10
lcHeight = 1.6
call lcAim
triSimFrames 3
triScreenshot "lcm_egypt_stern_digits"

logInfo format ["LOBO_LCM_CAPTURE_COMPLETE count=%1 visual_review_required=true",count lcBoats]
lcCam cameraEffect ["terminate","back"]
camDestroy lcCam
// Whole-process teardown ends any native auxiliary scripts without deleting
// their objects while they are still running.
triEndTest
