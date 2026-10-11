// LoBo issue #1 aircraft runtime acceptance probe.
// Actual native fire commands and Fired events, with screenshot evidence.
// Controlled airborne launches exercise script labels and pylon updates, not flight guidance.
// No installed addon data is modified. All helpers are observed after scripts run.
// Observe screenshots before claiming that the pylon visuals are correct.
triSimUntil { alive player }
setDate [1985, 6, 15, 12, 0]
0 setOvercast 0
0 setFog 0
setViewDistance 1800
showCinemaBorder false
player allowDammage false
player setCaptive true
laStart = getPos player
laPos = [(laStart select 0) + 40, (laStart select 1) + 40, 250]
laCam = "camera" camCreate laPos
laCam cameraEffect ["internal", "back"]
laCountHelpers = {
    laN = 0;
    {if ((typeOf _x) == "EmptyDetector") then {laN = laN + 1}} forEach (nearestObjects [laPos, [], 250]);
    laN
}
laAim = {
    laPlane setPos laPos;
    laPlane setVelocity [0,0,0];
    laPlane setVectorDirAndUp [[0,1,0],[0,0,1]];
    laP = getPosASL laPlane;
    triSetView [(laP select 0) + 7, (laP select 2) - 3, (laP select 1) + 9, -7, 3, -9]
}
laBaseline = call laCountHelpers
laSeed = "EmptyDetector" createVehicle laPos
triSimFrames 2
laSeedCount = call laCountHelpers
laHelpersObservable = laSeedCount == (laBaseline + 1)
logInfo format ["LOBO_AIRCRAFT_HELPER_CONTROL before=%1 seeded=%2 observable=%3", laBaseline, laSeedCount, laHelpersObservable]
deleteVehicle laSeed
triSimFrames 2
triAssertEq [call laCountHelpers, laBaseline]
laAllShots = 0

// mig21_ic1: LoBo_E_mig21bis_IC1
laPlane = "LoBo_E_mig21bis_IC1" createVehicle laPos
triAssertEq [typeOf laPlane, "LoBo_E_mig21bis_IC1"]
laPlane allowDammage false
laPlane setDir 0
laPlane setPos laPos
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane setVelocity [0,0,0]
player moveInDriver laPlane
triAssertEq [vehicle player, laPlane]
laPlane engineOn false
laFired = []
laPlane addEventHandler ["fired", {laFired = laFired + [[_this select 1, _this select 4]]; laAllShots = laAllShots + 1; logInfo format ["LOBO_AIRCRAFT_FIRED class=%1 weapon=%2 ammo=%3 remaining=%4", typeOf (_this select 0), _this select 1, _this select 4, (_this select 0) ammo (_this select 1)]}]
triSimFrames 40
call laAim
triSimFrames 2
triScreenshot "mig21_ic1_before"

triAssertEq [laPlane ammo "LoBoAIM9Rail6", 6]
laPlane selectWeapon "LoBoAIM9Rail6"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM9Rail6"
triSimUntil {(laPlane ammo "LoBoAIM9Rail6") == 5}
triAssertEq [count laFired, 1]
triAssertEq [(laFired select 0) select 0, "LoBoAIM9Rail6"]
triAssertEq [(laFired select 0) select 1, "LoBoAIM9"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic1 ordinal=1 expected_hidden_slot=8 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM9Rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic1_shot_01_slot_08"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM9Rail6"
triSimUntil {(laPlane ammo "LoBoAIM9Rail6") == 4}
triAssertEq [count laFired, 2]
triAssertEq [(laFired select 1) select 0, "LoBoAIM9Rail6"]
triAssertEq [(laFired select 1) select 1, "LoBoAIM9"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic1 ordinal=2 expected_hidden_slot=9 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM9Rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic1_shot_02_slot_09"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM9Rail6"
triSimUntil {(laPlane ammo "LoBoAIM9Rail6") == 3}
triAssertEq [count laFired, 3]
triAssertEq [(laFired select 2) select 0, "LoBoAIM9Rail6"]
triAssertEq [(laFired select 2) select 1, "LoBoAIM9"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic1 ordinal=3 expected_hidden_slot=10 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM9Rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic1_shot_03_slot_10"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM9Rail6"
triSimUntil {(laPlane ammo "LoBoAIM9Rail6") == 2}
triAssertEq [count laFired, 4]
triAssertEq [(laFired select 3) select 0, "LoBoAIM9Rail6"]
triAssertEq [(laFired select 3) select 1, "LoBoAIM9"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic1 ordinal=4 expected_hidden_slot=11 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM9Rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic1_shot_04_slot_11"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM9Rail6"
triSimUntil {(laPlane ammo "LoBoAIM9Rail6") == 1}
triAssertEq [count laFired, 5]
triAssertEq [(laFired select 4) select 0, "LoBoAIM9Rail6"]
triAssertEq [(laFired select 4) select 1, "LoBoAIM9"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic1 ordinal=5 expected_hidden_slot=12 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM9Rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic1_shot_05_slot_12"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM9Rail6"
triSimUntil {(laPlane ammo "LoBoAIM9Rail6") == 0}
triAssertEq [count laFired, 6]
triAssertEq [(laFired select 5) select 0, "LoBoAIM9Rail6"]
triAssertEq [(laFired select 5) select 1, "LoBoAIM9"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic1 ordinal=6 expected_hidden_slot=13 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM9Rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic1_shot_06_slot_13"

triAssertEq [laPlane ammo "LoBoAIM7Rail2", 2]
laPlane selectWeapon "LoBoAIM7Rail2"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM7Rail2"
triSimUntil {(laPlane ammo "LoBoAIM7Rail2") == 1}
triAssertEq [count laFired, 7]
triAssertEq [(laFired select 6) select 0, "LoBoAIM7Rail2"]
triAssertEq [(laFired select 6) select 1, "LoBoAIM7"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic1 ordinal=7 expected_hidden_slot=14 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM7Rail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic1_shot_07_slot_14"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM7Rail2"
triSimUntil {(laPlane ammo "LoBoAIM7Rail2") == 0}
triAssertEq [count laFired, 8]
triAssertEq [(laFired select 7) select 0, "LoBoAIM7Rail2"]
triAssertEq [(laFired select 7) select 1, "LoBoAIM7"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic1 ordinal=8 expected_hidden_slot=15 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM7Rail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic1_shot_08_slot_15"

logInfo "LOBO_AIRCRAFT_COMPLETE case=mig21_ic1 fired=8"
call laAim
triSimFrames 2
triScreenshot "mig21_ic1_after"
laPlane setPos [(laStart select 0) + 40, (laStart select 1) + 40, 0]
laPlane setVelocity [0,0,0]
player action ["GETOUT", laPlane]
triSimUntil {vehicle player == player}
player setPos laStart
deleteVehicle laPlane
triSimFrames 3

// mig21_ic2: LoBo_E_mig21bis_IC2
laPlane = "LoBo_E_mig21bis_IC2" createVehicle laPos
triAssertEq [typeOf laPlane, "LoBo_E_mig21bis_IC2"]
laPlane allowDammage false
laPlane setDir 0
laPlane setPos laPos
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane setVelocity [0,0,0]
player moveInDriver laPlane
triAssertEq [vehicle player, laPlane]
laPlane engineOn false
laFired = []
laPlane addEventHandler ["fired", {laFired = laFired + [[_this select 1, _this select 4]]; laAllShots = laAllShots + 1; logInfo format ["LOBO_AIRCRAFT_FIRED class=%1 weapon=%2 ammo=%3 remaining=%4", typeOf (_this select 0), _this select 1, _this select 4, (_this select 0) ammo (_this select 1)]}]
triSimFrames 40
call laAim
triSimFrames 2
triScreenshot "mig21_ic2_before"

triAssertEq [laPlane ammo "LoBoMagic2rail6", 6]
laPlane selectWeapon "LoBoMagic2rail6"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMagic2rail6"
triSimUntil {(laPlane ammo "LoBoMagic2rail6") == 5}
triAssertEq [count laFired, 1]
triAssertEq [(laFired select 0) select 0, "LoBoMagic2rail6"]
triAssertEq [(laFired select 0) select 1, "LoBo_Magic"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic2 ordinal=1 expected_hidden_slot=8 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMagic2rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic2_shot_01_slot_08"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMagic2rail6"
triSimUntil {(laPlane ammo "LoBoMagic2rail6") == 4}
triAssertEq [count laFired, 2]
triAssertEq [(laFired select 1) select 0, "LoBoMagic2rail6"]
triAssertEq [(laFired select 1) select 1, "LoBo_Magic"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic2 ordinal=2 expected_hidden_slot=9 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMagic2rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic2_shot_02_slot_09"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMagic2rail6"
triSimUntil {(laPlane ammo "LoBoMagic2rail6") == 3}
triAssertEq [count laFired, 3]
triAssertEq [(laFired select 2) select 0, "LoBoMagic2rail6"]
triAssertEq [(laFired select 2) select 1, "LoBo_Magic"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic2 ordinal=3 expected_hidden_slot=10 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMagic2rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic2_shot_03_slot_10"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMagic2rail6"
triSimUntil {(laPlane ammo "LoBoMagic2rail6") == 2}
triAssertEq [count laFired, 4]
triAssertEq [(laFired select 3) select 0, "LoBoMagic2rail6"]
triAssertEq [(laFired select 3) select 1, "LoBo_Magic"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic2 ordinal=4 expected_hidden_slot=11 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMagic2rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic2_shot_04_slot_11"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMagic2rail6"
triSimUntil {(laPlane ammo "LoBoMagic2rail6") == 1}
triAssertEq [count laFired, 5]
triAssertEq [(laFired select 4) select 0, "LoBoMagic2rail6"]
triAssertEq [(laFired select 4) select 1, "LoBo_Magic"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic2 ordinal=5 expected_hidden_slot=12 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMagic2rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic2_shot_05_slot_12"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMagic2rail6"
triSimUntil {(laPlane ammo "LoBoMagic2rail6") == 0}
triAssertEq [count laFired, 6]
triAssertEq [(laFired select 5) select 0, "LoBoMagic2rail6"]
triAssertEq [(laFired select 5) select 1, "LoBo_Magic"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic2 ordinal=6 expected_hidden_slot=13 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMagic2rail6", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic2_shot_06_slot_13"

triAssertEq [laPlane ammo "LoBoAIM7Rail2", 2]
laPlane selectWeapon "LoBoAIM7Rail2"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM7Rail2"
triSimUntil {(laPlane ammo "LoBoAIM7Rail2") == 1}
triAssertEq [count laFired, 7]
triAssertEq [(laFired select 6) select 0, "LoBoAIM7Rail2"]
triAssertEq [(laFired select 6) select 1, "LoBoAIM7"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic2 ordinal=7 expected_hidden_slot=14 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM7Rail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic2_shot_07_slot_14"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM7Rail2"
triSimUntil {(laPlane ammo "LoBoAIM7Rail2") == 0}
triAssertEq [count laFired, 8]
triAssertEq [(laFired select 7) select 0, "LoBoAIM7Rail2"]
triAssertEq [(laFired select 7) select 1, "LoBoAIM7"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig21_ic2 ordinal=8 expected_hidden_slot=15 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM7Rail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig21_ic2_shot_08_slot_15"

logInfo "LOBO_AIRCRAFT_COMPLETE case=mig21_ic2 fired=8"
call laAim
triSimFrames 2
triScreenshot "mig21_ic2_after"
laPlane setPos [(laStart select 0) + 40, (laStart select 1) + 40, 0]
laPlane setVelocity [0,0,0]
player action ["GETOUT", laPlane]
triSimUntil {vehicle player == player}
player setPos laStart
deleteVehicle laPlane
triSimFrames 3

// mig25_cap1: LoBo_Mig25PD_Syria
laPlane = "LoBo_Mig25PD_Syria" createVehicle laPos
triAssertEq [typeOf laPlane, "LoBo_Mig25PD_Syria"]
laPlane allowDammage false
laPlane setDir 0
laPlane setPos laPos
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane setVelocity [0,0,0]
player moveInDriver laPlane
triAssertEq [vehicle player, laPlane]
laPlane engineOn false
laFired = []
laPlane addEventHandler ["fired", {laFired = laFired + [[_this select 1, _this select 4]]; laAllShots = laAllShots + 1; logInfo format ["LOBO_AIRCRAFT_FIRED class=%1 weapon=%2 ammo=%3 remaining=%4", typeOf (_this select 0), _this select 1, _this select 4, (_this select 0) ammo (_this select 1)]}]
triSimFrames 40
call laAim
triSimFrames 2
triScreenshot "mig25_cap1_before"

triAssertEq [laPlane ammo "LoBo_R60Rail4", 4]
laPlane selectWeapon "LoBo_R60Rail4"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBo_R60Rail4"
triSimUntil {(laPlane ammo "LoBo_R60Rail4") == 3}
triAssertEq [count laFired, 1]
triAssertEq [(laFired select 0) select 0, "LoBo_R60Rail4"]
triAssertEq [(laFired select 0) select 1, "LoBo_R60"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig25_cap1 ordinal=1 expected_hidden_slot=3 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBo_R60Rail4", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig25_cap1_shot_01_slot_03"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBo_R60Rail4"
triSimUntil {(laPlane ammo "LoBo_R60Rail4") == 2}
triAssertEq [count laFired, 2]
triAssertEq [(laFired select 1) select 0, "LoBo_R60Rail4"]
triAssertEq [(laFired select 1) select 1, "LoBo_R60"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig25_cap1 ordinal=2 expected_hidden_slot=4 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBo_R60Rail4", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig25_cap1_shot_02_slot_04"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBo_R60Rail4"
triSimUntil {(laPlane ammo "LoBo_R60Rail4") == 1}
triAssertEq [count laFired, 3]
triAssertEq [(laFired select 2) select 0, "LoBo_R60Rail4"]
triAssertEq [(laFired select 2) select 1, "LoBo_R60"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig25_cap1 ordinal=3 expected_hidden_slot=5 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBo_R60Rail4", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig25_cap1_shot_03_slot_05"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBo_R60Rail4"
triSimUntil {(laPlane ammo "LoBo_R60Rail4") == 0}
triAssertEq [count laFired, 4]
triAssertEq [(laFired select 3) select 0, "LoBo_R60Rail4"]
triAssertEq [(laFired select 3) select 1, "LoBo_R60"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=mig25_cap1 ordinal=4 expected_hidden_slot=6 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBo_R60Rail4", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "mig25_cap1_shot_04_slot_06"

logInfo "LOBO_AIRCRAFT_COMPLETE case=mig25_cap1 fired=4"
call laAim
triSimFrames 2
triScreenshot "mig25_cap1_after"
laPlane setPos [(laStart select 0) + 40, (laStart select 1) + 40, 0]
laPlane setVelocity [0,0,0]
player action ["GETOUT", laPlane]
triSimUntil {vehicle player == player}
player setPos laStart
deleteVehicle laPlane
triSimFrames 3

// f5_firemissiles2: LoBo_F5_mir_Jor
laPlane = "LoBo_F5_mir_Jor" createVehicle laPos
triAssertEq [typeOf laPlane, "LoBo_F5_mir_Jor"]
laPlane allowDammage false
laPlane setDir 0
laPlane setPos laPos
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane setVelocity [0,0,0]
player moveInDriver laPlane
triAssertEq [vehicle player, laPlane]
laPlane engineOn false
laFired = []
laPlane addEventHandler ["fired", {laFired = laFired + [[_this select 1, _this select 4]]; laAllShots = laAllShots + 1; logInfo format ["LOBO_AIRCRAFT_FIRED class=%1 weapon=%2 ammo=%3 remaining=%4", typeOf (_this select 0), _this select 1, _this select 4, (_this select 0) ammo (_this select 1)]}]
triSimFrames 40
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles2_before"

triAssertEq [laPlane ammo "LoBoAIM9Rail2", 2]
laPlane selectWeapon "LoBoAIM9Rail2"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM9Rail2"
triSimUntil {(laPlane ammo "LoBoAIM9Rail2") == 1}
triAssertEq [count laFired, 1]
triAssertEq [(laFired select 0) select 0, "LoBoAIM9Rail2"]
triAssertEq [(laFired select 0) select 1, "LoBoAIM9"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles2 ordinal=1 expected_hidden_slot=0 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM9Rail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles2_shot_01_slot_00"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM9Rail2"
triSimUntil {(laPlane ammo "LoBoAIM9Rail2") == 0}
triAssertEq [count laFired, 2]
triAssertEq [(laFired select 1) select 0, "LoBoAIM9Rail2"]
triAssertEq [(laFired select 1) select 1, "LoBoAIM9"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles2 ordinal=2 expected_hidden_slot=1 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM9Rail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles2_shot_02_slot_01"

triAssertEq [laPlane ammo "LoBoMavRail2", 2]
laPlane selectWeapon "LoBoMavRail2"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMavRail2"
triSimUntil {(laPlane ammo "LoBoMavRail2") == 1}
triAssertEq [count laFired, 3]
triAssertEq [(laFired select 2) select 0, "LoBoMavRail2"]
triAssertEq [(laFired select 2) select 1, "LoBoMav"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles2 ordinal=3 expected_hidden_slot=2 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMavRail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles2_shot_03_slot_02"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMavRail2"
triSimUntil {(laPlane ammo "LoBoMavRail2") == 0}
triAssertEq [count laFired, 4]
triAssertEq [(laFired select 3) select 0, "LoBoMavRail2"]
triAssertEq [(laFired select 3) select 1, "LoBoMav"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles2 ordinal=4 expected_hidden_slot=3 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMavRail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles2_shot_04_slot_03"

triAssertEq [laPlane ammo "LoBo_Mk20Rail2", 2]
laPlane selectWeapon "LoBo_Mk20Rail2"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBo_Mk20Rail2"
triSimUntil {(laPlane ammo "LoBo_Mk20Rail2") == 1}
triAssertEq [count laFired, 5]
triAssertEq [(laFired select 4) select 0, "LoBo_Mk20Rail2"]
triAssertEq [(laFired select 4) select 1, "LoBo_mk20Bomb"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles2 ordinal=5 expected_hidden_slot=4 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBo_Mk20Rail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles2_shot_05_slot_04"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBo_Mk20Rail2"
triSimUntil {(laPlane ammo "LoBo_Mk20Rail2") == 0}
triAssertEq [count laFired, 6]
triAssertEq [(laFired select 5) select 0, "LoBo_Mk20Rail2"]
triAssertEq [(laFired select 5) select 1, "LoBo_mk20Bomb"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles2 ordinal=6 expected_hidden_slot=5 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBo_Mk20Rail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles2_shot_06_slot_05"

logInfo "LOBO_AIRCRAFT_COMPLETE case=f5_firemissiles2 fired=6"
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles2_after"
laPlane setPos [(laStart select 0) + 40, (laStart select 1) + 40, 0]
laPlane setVelocity [0,0,0]
player action ["GETOUT", laPlane]
triSimUntil {vehicle player == player}
player setPos laStart
deleteVehicle laPlane
triSimFrames 3

// f5_firemissiles3: LoBo_F5_AT_Jor
laPlane = "LoBo_F5_AT_Jor" createVehicle laPos
triAssertEq [typeOf laPlane, "LoBo_F5_AT_Jor"]
laPlane allowDammage false
laPlane setDir 0
laPlane setPos laPos
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane setVelocity [0,0,0]
player moveInDriver laPlane
triAssertEq [vehicle player, laPlane]
laPlane engineOn false
laFired = []
laPlane addEventHandler ["fired", {laFired = laFired + [[_this select 1, _this select 4]]; laAllShots = laAllShots + 1; logInfo format ["LOBO_AIRCRAFT_FIRED class=%1 weapon=%2 ammo=%3 remaining=%4", typeOf (_this select 0), _this select 1, _this select 4, (_this select 0) ammo (_this select 1)]}]
triSimFrames 40
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles3_before"

triAssertEq [laPlane ammo "LoBoAIM9Rail2", 2]
laPlane selectWeapon "LoBoAIM9Rail2"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM9Rail2"
triSimUntil {(laPlane ammo "LoBoAIM9Rail2") == 1}
triAssertEq [count laFired, 1]
triAssertEq [(laFired select 0) select 0, "LoBoAIM9Rail2"]
triAssertEq [(laFired select 0) select 1, "LoBoAIM9"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles3 ordinal=1 expected_hidden_slot=0 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM9Rail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles3_shot_01_slot_00"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoAIM9Rail2"
triSimUntil {(laPlane ammo "LoBoAIM9Rail2") == 0}
triAssertEq [count laFired, 2]
triAssertEq [(laFired select 1) select 0, "LoBoAIM9Rail2"]
triAssertEq [(laFired select 1) select 1, "LoBoAIM9"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles3 ordinal=2 expected_hidden_slot=1 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoAIM9Rail2", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles3_shot_02_slot_01"

triAssertEq [laPlane ammo "LoBoMavRail4", 4]
laPlane selectWeapon "LoBoMavRail4"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMavRail4"
triSimUntil {(laPlane ammo "LoBoMavRail4") == 3}
triAssertEq [count laFired, 3]
triAssertEq [(laFired select 2) select 0, "LoBoMavRail4"]
triAssertEq [(laFired select 2) select 1, "LoBoMav"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles3 ordinal=3 expected_hidden_slot=2 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMavRail4", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles3_shot_03_slot_02"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMavRail4"
triSimUntil {(laPlane ammo "LoBoMavRail4") == 2}
triAssertEq [count laFired, 4]
triAssertEq [(laFired select 3) select 0, "LoBoMavRail4"]
triAssertEq [(laFired select 3) select 1, "LoBoMav"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles3 ordinal=4 expected_hidden_slot=3 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMavRail4", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles3_shot_04_slot_03"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMavRail4"
triSimUntil {(laPlane ammo "LoBoMavRail4") == 1}
triAssertEq [count laFired, 5]
triAssertEq [(laFired select 4) select 0, "LoBoMavRail4"]
triAssertEq [(laFired select 4) select 1, "LoBoMav"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles3 ordinal=5 expected_hidden_slot=4 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMavRail4", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles3_shot_05_slot_04"

laPlane setPos laPos
laPlane setVelocity [0,0,0]
laPlane setVectorDirAndUp [[0,1,0],[0,0,1]]
laPlane fire "LoBoMavRail4"
triSimUntil {(laPlane ammo "LoBoMavRail4") == 0}
triAssertEq [count laFired, 6]
triAssertEq [(laFired select 5) select 0, "LoBoMavRail4"]
triAssertEq [(laFired select 5) select 1, "LoBoMav"]
triSimFrames 4
triAssertEq [call laCountHelpers, laBaseline]
logInfo format ["LOBO_AIRCRAFT_SHOT case=f5_firemissiles3 ordinal=6 expected_hidden_slot=5 remaining=%1 helpers=%2 observable=%3", laPlane ammo "LoBoMavRail4", call laCountHelpers, laHelpersObservable]
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles3_shot_06_slot_05"

logInfo "LOBO_AIRCRAFT_COMPLETE case=f5_firemissiles3 fired=6"
call laAim
triSimFrames 2
triScreenshot "f5_firemissiles3_after"
laPlane setPos [(laStart select 0) + 40, (laStart select 1) + 40, 0]
laPlane setVelocity [0,0,0]
player action ["GETOUT", laPlane]
triSimUntil {vehicle player == player}
player setPos laStart
deleteVehicle laPlane
triSimFrames 3

triAssertEq [laAllShots, 32]
logInfo format ["LOBO_AIRCRAFT_RESULT shots=%1 helper_observable=%2 helpers=%3 baseline=%4", laAllShots, laHelpersObservable, call laCountHelpers, laBaseline]
laCam cameraEffect ["terminate", "back"]
camDestroy laCam
triEndTest
