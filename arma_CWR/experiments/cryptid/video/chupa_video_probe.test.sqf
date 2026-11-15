// ============================================================================
//  Chupacabra video probe: runway, apron, classes, stepped clock
//  Written by gen_chupa_video.py (edit that, not this file).
//  Quiet: no weapons, nobody fires, --nosound.
// ============================================================================
triSimUntil { alive player }
triCaptureAtPresent true
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [4800, 9000, 0]
setDate [1985, 5, 10, 13, 0]
0 setOvercast 0.35
0 setFog 0
setViewDistance 1500
showCinemaBorder false
cvC = createCenter civilian
cvHud = "camera" camCreate [4800, 9000, 40]
cvHud cameraEffect ["internal", "back"]
cvHud camSetTarget [4800, 12000, 40]
cvHud camSetFov 0.7
cvHud camCommit 0
cvProbe = "HeliHEmpty" createVehicle [4800, 9000, 0]
cvGnd = { cvProbe setPos [_this select 0, _this select 1, 0]; (getPosASL cvProbe) select 2 }
logInfo format ["CVP class Hangar %1", gmClassExists "Hangar"]
logInfo format ["CVP class Hangar_Hall %1", gmClassExists "Hangar_Hall"]
logInfo format ["CVP class Hangar_Office %1", gmClassExists "Hangar_Office"]
logInfo format ["CVP class Barracks %1", gmClassExists "Barracks"]
logInfo format ["CVP class Office %1", gmClassExists "Office"]
logInfo format ["CVP class Shed %1", gmClassExists "Shed"]
logInfo format ["CVP class Su25 %1", gmClassExists "Su25"]
logInfo format ["CVP class A10 %1", gmClassExists "A10"]
logInfo format ["CVP class Cessna %1", gmClassExists "Cessna"]
logInfo format ["CVP class UH60 %1", gmClassExists "UH60"]
logInfo format ["CVP class Mi17 %1", gmClassExists "Mi17"]
logInfo format ["CVP class Mi24 %1", gmClassExists "Mi24"]
logInfo format ["CVP class Truck5tRefuel %1", gmClassExists "Truck5tRefuel"]
logInfo format ["CVP class UralRefuel %1", gmClassExists "UralRefuel"]
logInfo format ["CVP class Truck5t %1", gmClassExists "Truck5t"]
logInfo format ["CVP class Ural %1", gmClassExists "Ural"]
logInfo format ["CVP class Jeep %1", gmClassExists "Jeep"]
logInfo format ["CVP class HMMWV %1", gmClassExists "HMMWV"]
logInfo format ["CVP class Barrels %1", gmClassExists "Barrels"]
logInfo format ["CVP class Barrel4 %1", gmClassExists "Barrel4"]
logInfo format ["CVP class Civilian %1", gmClassExists "Civilian"]
logInfo format ["CVP class Civilian2 %1", gmClassExists "Civilian2"]
logInfo format ["CVP class Civilian3 %1", gmClassExists "Civilian3"]
logInfo format ["CVP class SoldierWCrew %1", gmClassExists "SoldierWCrew"]
logInfo format ["CVP class SoldierWPilot %1", gmClassExists "SoldierWPilot"]
logInfo format ["CVP class UDChupacabraAnim %1", gmClassExists "UDChupacabraAnim"]
logInfo format ["CVP class Camp %1", gmClassExists "Camp"]
logInfo format ["CVP class FieldHospital %1", gmClassExists "FieldHospital"]
triSimFrames 60
cvHud camSetFov 0.7
cvHud camCommit 0
triSetView [4600, 650, 10600, 0, -1, 0.02]
triSimFrames 30
logInfo format ["CVP tile 0 4600 10600 ground %1", [4600, 10600] call cvGnd]
triScreenshot "top_0_4600_10600"
triSetView [5100, 650, 10600, 0, -1, 0.02]
triSimFrames 30
logInfo format ["CVP tile 1 5100 10600 ground %1", [5100, 10600] call cvGnd]
triScreenshot "top_1_5100_10600"
triSetView [4600, 650, 11100, 0, -1, 0.02]
triSimFrames 30
logInfo format ["CVP tile 2 4600 11100 ground %1", [4600, 11100] call cvGnd]
triScreenshot "top_2_4600_11100"
triSetView [5100, 650, 11100, 0, -1, 0.02]
triSimFrames 30
logInfo format ["CVP tile 3 5100 11100 ground %1", [5100, 11100] call cvGnd]
triScreenshot "top_3_5100_11100"
triSetView [5500, 650, 10300, 0, -1, 0.02]
triSimFrames 30
logInfo format ["CVP tile 4 5500 10300 ground %1", [5500, 10300] call cvGnd]
triScreenshot "top_4_5500_10300"
triSetView [5500, 650, 10850, 0, -1, 0.02]
triSimFrames 30
logInfo format ["CVP tile 5 5500 10850 ground %1", [5500, 10850] call cvGnd]
triScreenshot "top_5_5500_10850"
triSetView [4300, 650, 10850, 0, -1, 0.02]
triSimFrames 30
logInfo format ["CVP tile 6 4300 10850 ground %1", [4300, 10850] call cvGnd]
triScreenshot "top_6_4300_10850"
cvG = [4800, 11000] call cvGnd
triSetView [4800, cvG + 2.0, 11000, 0, -0.03, 1]
triSimFrames 20
triScreenshot "look_4800_11000_h000"
cvG = [4800, 11000] call cvGnd
triSetView [4800, cvG + 2.0, 11000, 1, -0.03, 6.12323e-17]
triSimFrames 20
triScreenshot "look_4800_11000_h090"
cvG = [4800, 11000] call cvGnd
triSetView [4800, cvG + 2.0, 11000, 1.22465e-16, -0.03, -1]
triSimFrames 20
triScreenshot "look_4800_11000_h180"
cvG = [4800, 11000] call cvGnd
triSetView [4800, cvG + 2.0, 11000, -1, -0.03, -1.83697e-16]
triSimFrames 20
triScreenshot "look_4800_11000_h270"
cvG = [5050, 10650] call cvGnd
triSetView [5050, cvG + 2.0, 10650, 0, -0.03, 1]
triSimFrames 20
triScreenshot "look_5050_10650_h000"
cvG = [5050, 10650] call cvGnd
triSetView [5050, cvG + 2.0, 10650, 1, -0.03, 6.12323e-17]
triSimFrames 20
triScreenshot "look_5050_10650_h090"
cvG = [5050, 10650] call cvGnd
triSetView [5050, cvG + 2.0, 10650, 1.22465e-16, -0.03, -1]
triSimFrames 20
triScreenshot "look_5050_10650_h180"
cvG = [5050, 10650] call cvGnd
triSetView [5050, cvG + 2.0, 10650, -1, -0.03, -1.83697e-16]
triSimFrames 20
triScreenshot "look_5050_10650_h270"
cvG0 = createGroup civilian
cvMen = []
"Civilian" createUnit [[4790, 11000, 0], cvG0, "cvMen = cvMen + [this]", 0.5, "PRIVATE"]
"Civilian2" createUnit [[4791.6, 11000, 0], cvG0, "cvMen = cvMen + [this]", 0.5, "PRIVATE"]
"Civilian3" createUnit [[4793.2, 11000, 0], cvG0, "cvMen = cvMen + [this]", 0.5, "PRIVATE"]
"SoldierWCrew" createUnit [[4794.8, 11000, 0], cvG0, "cvMen = cvMen + [this]", 0.5, "PRIVATE"]
"SoldierWPilot" createUnit [[4796.4, 11000, 0], cvG0, "cvMen = cvMen + [this]", 0.5, "PRIVATE"]
triSimFrames 5
{ removeAllWeapons _x; _x setCaptive true; _x allowDammage false; _x setDir 180; _x disableAI "MOVE" } forEach cvMen
cvG0 setBehaviour "CARELESS"
chupa = objNull
"UDChupacabraAnim" createUnit [[4800, 11000, 0], cvG0, "chupa = this", 0.5, "PRIVATE"]
triSimFrames 5
chupa setCaptive true
chupa setPos [4801, 10998, 0]
chupa setDir 180
triSimFrames 60
{ _x switchMove "Civil" } forEach cvMen
triSimFrames 10
cvG = [4794, 10990] call cvGnd
triSetView [4794, cvG + 1.6, 10990, 0, -0.02, 1]
triSimFrames 6
triScreenshot "lineup"
cvHud camSetTarget [4794, 30, 12000]
cvHud camSetFov 0.35
cvHud camCommit 0
triSimFrames 6
triSetView [4794, cvG + 1.6, 10985, 0, -0.02, 1]
triSimFrames 6
triScreenshot "lineup_long"
chupa setUnitPos "DOWN"
cvG0 setSpeedMode "FULL"
chupa doMove [4801, 10900, 0]
triSimFrames 30
call { setAccTime 0 }
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 0, time, getMove chupa, getPosASL chupa]
triSetView [(getPosASL chupa select 0) + 6, (getPosASL chupa select 2) + 1, (getPosASL chupa select 1), -1, -0.1, 0]
triScreenshot "step_00"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 1, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 2, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 3, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 4, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 5, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 6, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 7, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 8, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 9, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 10, time, getMove chupa, getPosASL chupa]
triSetView [(getPosASL chupa select 0) + 6, (getPosASL chupa select 2) + 1, (getPosASL chupa select 1), -1, -0.1, 0]
triScreenshot "step_10"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 11, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 12, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 13, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 14, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 15, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 16, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 17, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 18, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 19, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 20, time, getMove chupa, getPosASL chupa]
triSetView [(getPosASL chupa select 0) + 6, (getPosASL chupa select 2) + 1, (getPosASL chupa select 1), -1, -0.1, 0]
triScreenshot "step_20"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 21, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 22, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 23, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 24, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 25, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 26, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 27, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 28, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 29, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 30, time, getMove chupa, getPosASL chupa]
triSetView [(getPosASL chupa select 0) + 6, (getPosASL chupa select 2) + 1, (getPosASL chupa select 1), -1, -0.1, 0]
triScreenshot "step_30"
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 31, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 32, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 33, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 34, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 35, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 36, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 37, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 38, time, getMove chupa, getPosASL chupa]
call { triWaitFrames 60; setAccTime (1/9); triSimFrames 1; setAccTime 0 }
logInfo format ["CVF step %1 %2 %3 %4", 39, time, getMove chupa, getPosASL chupa]
setAccTime 1
triEndTest
