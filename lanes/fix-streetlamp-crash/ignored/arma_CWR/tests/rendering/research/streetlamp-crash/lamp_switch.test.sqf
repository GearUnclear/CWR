// ============================================================================
//  Research-only repro for bugfix "streetlamp-crash" (not a regression test in
//  tests/integration; run it through tests/rendering/captures/run-captures.ps1
//  with -Tests). Saint Louis on Abel, 22:00 on 27 February, the town_night
//  view of abel_lighting.test.sqf: six street lamps (lampazel) 30-90 m ahead.
//
//  switchLight / lightIsOn find their lamp with dyn_cast<StreetLamp>. Before
//  the fix StreetLamp had no casting id of its own, so the cast accepted any
//  Entity (every house, soldier and car) and SwitchLight ran the lamp code on
//  it. The steps below switch a spawned soldier, then a spawned Jeep, then
//  every object within 200 m, each behind its own log line, so the last
//  "event=switch" line before a crash names the step that crashed.
// ============================================================================

#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Abel"
capFov = 0.7
capSimT = 0
capPinTime = false
capUseView = false
capProps = []
capView = "setup"
capModeName = "none"
capLit = "none"
capGpu = "none"
capPerf = "none"

triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [7152, 8915, 0]
capCam = "camera" camCreate [7152, 8915, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }

["lamps", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin
"remastered" call capMode
call {
    capJeep = [["Jeep"], 12, -3, 70, "veh"] call capSpawn;
    capMan = [["SoldierWB"], 8, 1.5, 160, "man"] call capSpawn;
}
// A lamp in AUTO only re-evaluates on its 12 s simulation step.
triSimFrames 1200
[] call capPin
triSimFrames 3
triScreenshot "lamps_auto"
"lamps_auto" call capRecord

// Census: lightIsOn answers "ERROR" for anything dyn_cast<StreetLamp> rejects.
call {
    capNear = nearestObjects [[7143, 8975, 0], [], 200];
    capLamps = [];
    { if ((lightIsOn _x) != "ERROR") then { capLamps = capLamps + [_x] } } forEach capNear;
    format ["view=lamps;event=census;near=%1;lamps=%2;jeep=%3;man=%4", count capNear, count capLamps, lightIsOn capJeep, lightIsOn capMan] call capLog;
}

call { "view=lamps;event=switch;target=soldier;state=OFF" call capLog; capMan switchLight "OFF"; "view=lamps;event=switched;target=soldier" call capLog }
call { "view=lamps;event=switch;target=jeep;state=OFF" call capLog; capJeep switchLight "OFF"; "view=lamps;event=switched;target=jeep" call capLog }
call { "view=lamps;event=switch;target=near200;state=OFF" call capLog; { _x switchLight "OFF" } forEach capNear; "view=lamps;event=switched;target=near200" call capLog }
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "lamps_off"
"lamps_off" call capRecord
call {
    capOff = 0;
    { if ((lightIsOn _x) == "OFF") then { capOff = capOff + 1 } } forEach capLamps;
    format ["view=lamps;event=state;want=OFF;lamps=%1;off=%2", count capLamps, capOff] call capLog;
}

call { "view=lamps;event=switch;target=near200;state=ON" call capLog; { _x switchLight "ON" } forEach capNear; "view=lamps;event=switched;target=near200" call capLog }
triSimFrames 30
[] call capPin
triSimFrames 3
triScreenshot "lamps_on"
"lamps_on" call capRecord
call {
    capOn = 0;
    { if ((lightIsOn _x) == "ON") then { capOn = capOn + 1 } } forEach capLamps;
    format ["view=lamps;event=state;want=ON;lamps=%1;on=%2;jeep=%3;man=%4", count capLamps, capOn, lightIsOn capJeep, lightIsOn capMan] call capLog;
}

triAssertEq [lightIsOn capMan, "ERROR"]
triAssertEq [lightIsOn capJeep, "ERROR"]
triAssertEq [capOff, count capLamps]
triAssertEq [capOn, count capLamps]
triAssertLt [0, count capLamps]

call { { _x switchLight "AUTO" } forEach capNear; "view=done;event=done" call capLog }
triEndTest
