// look-light motion captures on Abel: gen_look_motion.py (pan across a low sun, shade to sky).
// Generated, do not edit: 1920x1080 windowed, render scale 1.0, 4x MSAA, presented frame.
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
capFire = objNull
capBurnV = objNull
capJeep = objNull
capDrv = objNull

call { format ["view=setup;event=window_boot;size=%1", triGetWindowSize] call capLog }
triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.0
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capGrpE = createGroup east
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call { format ["view=setup;event=setup;world=Abel;msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }


// ---- pan_sun__on
call { capRain = 0; capEyeAbs = false }
["pan_sun__on", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call { capSimT = 2000; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"remastered" call capMode
call { capH = 92.5; [] call capPinView }
triSimFrames 120
call { capH = 92.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=0;offset=-60;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_000"
call { capH = 94; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=1;offset=-58.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 95.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=2;offset=-57;feat=%2", capView, triFeatureState] call capLog }
call { capH = 97; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=3;offset=-55.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 98.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=4;offset=-54;feat=%2", capView, triFeatureState] call capLog }
call { capH = 100; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=5;offset=-52.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 101.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=6;offset=-51;feat=%2", capView, triFeatureState] call capLog }
call { capH = 103; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=7;offset=-49.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 104.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=8;offset=-48;feat=%2", capView, triFeatureState] call capLog }
call { capH = 106; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=9;offset=-46.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 107.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=10;offset=-45;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_010"
call { capH = 109; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=11;offset=-43.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 110.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=12;offset=-42;feat=%2", capView, triFeatureState] call capLog }
call { capH = 112; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=13;offset=-40.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 113.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=14;offset=-39;feat=%2", capView, triFeatureState] call capLog }
call { capH = 115; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=15;offset=-37.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 116.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=16;offset=-36;feat=%2", capView, triFeatureState] call capLog }
call { capH = 118; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=17;offset=-34.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 119.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=18;offset=-33;feat=%2", capView, triFeatureState] call capLog }
call { capH = 121; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=19;offset=-31.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 122.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=20;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_020"
call { capH = 124; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=21;offset=-28.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 125.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=22;offset=-27;feat=%2", capView, triFeatureState] call capLog }
call { capH = 127; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=23;offset=-25.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 128.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=24;offset=-24;feat=%2", capView, triFeatureState] call capLog }
call { capH = 130; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=25;offset=-22.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 131.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=26;offset=-21;feat=%2", capView, triFeatureState] call capLog }
call { capH = 133; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=27;offset=-19.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 134.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=28;offset=-18;feat=%2", capView, triFeatureState] call capLog }
call { capH = 136; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=29;offset=-16.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 137.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=30;offset=-15;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_030"
call { capH = 139; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=31;offset=-13.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 140.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=32;offset=-12;feat=%2", capView, triFeatureState] call capLog }
call { capH = 142; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=33;offset=-10.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 143.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=34;offset=-9;feat=%2", capView, triFeatureState] call capLog }
call { capH = 145; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=35;offset=-7.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 146.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=36;offset=-6;feat=%2", capView, triFeatureState] call capLog }
call { capH = 148; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=37;offset=-4.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 149.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=38;offset=-3;feat=%2", capView, triFeatureState] call capLog }
call { capH = 151; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=39;offset=-1.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 152.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=40;offset=0;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_040"
call { capH = 154; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=41;offset=1.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 155.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=42;offset=3;feat=%2", capView, triFeatureState] call capLog }
call { capH = 157; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=43;offset=4.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 158.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=44;offset=6;feat=%2", capView, triFeatureState] call capLog }
call { capH = 160; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=45;offset=7.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 161.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=46;offset=9;feat=%2", capView, triFeatureState] call capLog }
call { capH = 163; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=47;offset=10.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 164.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=48;offset=12;feat=%2", capView, triFeatureState] call capLog }
call { capH = 166; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=49;offset=13.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 167.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=50;offset=15;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_050"
call { capH = 169; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=51;offset=16.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 170.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=52;offset=18;feat=%2", capView, triFeatureState] call capLog }
call { capH = 172; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=53;offset=19.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 173.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=54;offset=21;feat=%2", capView, triFeatureState] call capLog }
call { capH = 175; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=55;offset=22.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 176.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=56;offset=24;feat=%2", capView, triFeatureState] call capLog }
call { capH = 178; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=57;offset=25.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 179.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=58;offset=27;feat=%2", capView, triFeatureState] call capLog }
call { capH = 181; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=59;offset=28.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=60;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_060"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=61;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=62;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=63;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=64;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=65;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=66;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=67;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=68;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=69;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=70;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_070"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=71;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=72;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=73;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=74;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=75;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=76;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=77;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=78;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=79;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=80;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_080"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=81;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=82;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=83;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=84;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=85;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=86;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=87;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=88;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=89;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=90;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_090"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=91;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=92;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=93;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=94;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=95;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=96;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=97;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=98;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=99;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=100;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_100"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=101;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=102;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=103;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=104;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=105;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=106;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=107;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=108;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=109;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=110;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_110"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=111;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=112;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=113;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=114;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=115;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=116;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=117;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=118;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=119;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=120;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_120"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=121;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=122;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=123;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=124;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=125;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=126;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=127;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=128;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=129;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=130;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_130"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=131;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=132;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=133;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=134;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=135;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=136;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=137;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=138;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=139;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=140;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_140"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=141;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=142;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=143;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=144;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=145;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=146;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=147;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=148;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=149;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=on;frame=150;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__on_150"
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- pan_sun__off
call { capRain = 0; capEyeAbs = false }
["pan_sun__off", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin
call { capSimT = 2600; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;
    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;
    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;
    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;
}
triSimFrames 150
"remastered,photoLight=0" call capMode
call { capH = 92.5; [] call capPinView }
triSimFrames 120
call { capH = 92.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=0;offset=-60;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_000"
call { capH = 94; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=1;offset=-58.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 95.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=2;offset=-57;feat=%2", capView, triFeatureState] call capLog }
call { capH = 97; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=3;offset=-55.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 98.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=4;offset=-54;feat=%2", capView, triFeatureState] call capLog }
call { capH = 100; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=5;offset=-52.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 101.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=6;offset=-51;feat=%2", capView, triFeatureState] call capLog }
call { capH = 103; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=7;offset=-49.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 104.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=8;offset=-48;feat=%2", capView, triFeatureState] call capLog }
call { capH = 106; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=9;offset=-46.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 107.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=10;offset=-45;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_010"
call { capH = 109; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=11;offset=-43.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 110.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=12;offset=-42;feat=%2", capView, triFeatureState] call capLog }
call { capH = 112; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=13;offset=-40.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 113.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=14;offset=-39;feat=%2", capView, triFeatureState] call capLog }
call { capH = 115; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=15;offset=-37.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 116.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=16;offset=-36;feat=%2", capView, triFeatureState] call capLog }
call { capH = 118; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=17;offset=-34.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 119.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=18;offset=-33;feat=%2", capView, triFeatureState] call capLog }
call { capH = 121; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=19;offset=-31.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 122.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=20;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_020"
call { capH = 124; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=21;offset=-28.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 125.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=22;offset=-27;feat=%2", capView, triFeatureState] call capLog }
call { capH = 127; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=23;offset=-25.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 128.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=24;offset=-24;feat=%2", capView, triFeatureState] call capLog }
call { capH = 130; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=25;offset=-22.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 131.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=26;offset=-21;feat=%2", capView, triFeatureState] call capLog }
call { capH = 133; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=27;offset=-19.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 134.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=28;offset=-18;feat=%2", capView, triFeatureState] call capLog }
call { capH = 136; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=29;offset=-16.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 137.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=30;offset=-15;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_030"
call { capH = 139; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=31;offset=-13.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 140.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=32;offset=-12;feat=%2", capView, triFeatureState] call capLog }
call { capH = 142; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=33;offset=-10.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 143.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=34;offset=-9;feat=%2", capView, triFeatureState] call capLog }
call { capH = 145; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=35;offset=-7.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 146.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=36;offset=-6;feat=%2", capView, triFeatureState] call capLog }
call { capH = 148; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=37;offset=-4.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 149.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=38;offset=-3;feat=%2", capView, triFeatureState] call capLog }
call { capH = 151; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=39;offset=-1.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 152.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=40;offset=0;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_040"
call { capH = 154; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=41;offset=1.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 155.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=42;offset=3;feat=%2", capView, triFeatureState] call capLog }
call { capH = 157; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=43;offset=4.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 158.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=44;offset=6;feat=%2", capView, triFeatureState] call capLog }
call { capH = 160; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=45;offset=7.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 161.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=46;offset=9;feat=%2", capView, triFeatureState] call capLog }
call { capH = 163; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=47;offset=10.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 164.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=48;offset=12;feat=%2", capView, triFeatureState] call capLog }
call { capH = 166; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=49;offset=13.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 167.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=50;offset=15;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_050"
call { capH = 169; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=51;offset=16.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 170.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=52;offset=18;feat=%2", capView, triFeatureState] call capLog }
call { capH = 172; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=53;offset=19.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 173.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=54;offset=21;feat=%2", capView, triFeatureState] call capLog }
call { capH = 175; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=55;offset=22.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 176.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=56;offset=24;feat=%2", capView, triFeatureState] call capLog }
call { capH = 178; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=57;offset=25.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 179.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=58;offset=27;feat=%2", capView, triFeatureState] call capLog }
call { capH = 181; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=59;offset=28.5;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=60;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_060"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=61;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=62;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=63;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=64;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=65;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=66;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=67;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=68;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=69;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=70;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_070"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=71;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=72;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=73;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=74;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=75;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=76;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=77;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=78;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=79;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=80;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_080"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=81;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=82;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=83;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=84;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=85;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=86;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=87;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=88;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=89;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=90;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_090"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=91;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=92;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=93;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=94;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=95;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=96;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=97;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=98;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=99;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=100;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_100"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=101;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=102;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=103;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=104;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=105;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=106;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=107;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=108;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=109;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=110;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_110"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=111;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=112;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=113;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=114;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=115;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=116;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=117;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=118;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=119;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=120;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_120"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=121;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=122;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=123;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=124;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=125;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=126;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=127;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=128;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=129;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=130;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_130"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=131;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=132;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=133;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=134;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=135;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=136;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=137;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=138;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=139;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=140;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_140"
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=141;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=142;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=143;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=144;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=145;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=146;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=147;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=148;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=149;offset=30;feat=%2", capView, triFeatureState] call capLog }
call { capH = 182.5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=pan_sun;mode=off;frame=150;offset=30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "pan_sun__off_150"
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- shade_open__on
call { capRain = 0; capEyeAbs = false }
["shade_open__on", 6940, 9964, 1.75, 240, -2, [1985, 7, 5, 17, 30], 0, 0, 1500] call capBegin
call { capSimT = 3200; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 14, 2, 70, "veh"] call capSpawn;
    [["SoldierWB"], 9, -2, 150, "man"] call capSpawn;
}
triSimFrames 150
"remastered" call capMode
call { capP = -32; [] call capPinView }
triSimFrames 120
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=0;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_000"
call { capP = -31; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=1;offset=-29;feat=%2", capView, triFeatureState] call capLog }
call { capP = -30; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=2;offset=-28;feat=%2", capView, triFeatureState] call capLog }
call { capP = -29; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=3;offset=-27;feat=%2", capView, triFeatureState] call capLog }
call { capP = -28; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=4;offset=-26;feat=%2", capView, triFeatureState] call capLog }
call { capP = -27; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=5;offset=-25;feat=%2", capView, triFeatureState] call capLog }
call { capP = -26; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=6;offset=-24;feat=%2", capView, triFeatureState] call capLog }
call { capP = -25; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=7;offset=-23;feat=%2", capView, triFeatureState] call capLog }
call { capP = -24; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=8;offset=-22;feat=%2", capView, triFeatureState] call capLog }
call { capP = -23; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=9;offset=-21;feat=%2", capView, triFeatureState] call capLog }
call { capP = -22; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=10;offset=-20;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_010"
call { capP = -21; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=11;offset=-19;feat=%2", capView, triFeatureState] call capLog }
call { capP = -20; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=12;offset=-18;feat=%2", capView, triFeatureState] call capLog }
call { capP = -19; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=13;offset=-17;feat=%2", capView, triFeatureState] call capLog }
call { capP = -18; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=14;offset=-16;feat=%2", capView, triFeatureState] call capLog }
call { capP = -17; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=15;offset=-15;feat=%2", capView, triFeatureState] call capLog }
call { capP = -16; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=16;offset=-14;feat=%2", capView, triFeatureState] call capLog }
call { capP = -15; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=17;offset=-13;feat=%2", capView, triFeatureState] call capLog }
call { capP = -14; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=18;offset=-12;feat=%2", capView, triFeatureState] call capLog }
call { capP = -13; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=19;offset=-11;feat=%2", capView, triFeatureState] call capLog }
call { capP = -12; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=20;offset=-10;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_020"
call { capP = -11; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=21;offset=-9;feat=%2", capView, triFeatureState] call capLog }
call { capP = -10; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=22;offset=-8;feat=%2", capView, triFeatureState] call capLog }
call { capP = -9; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=23;offset=-7;feat=%2", capView, triFeatureState] call capLog }
call { capP = -8; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=24;offset=-6;feat=%2", capView, triFeatureState] call capLog }
call { capP = -7; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=25;offset=-5;feat=%2", capView, triFeatureState] call capLog }
call { capP = -6; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=26;offset=-4;feat=%2", capView, triFeatureState] call capLog }
call { capP = -5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=27;offset=-3;feat=%2", capView, triFeatureState] call capLog }
call { capP = -4; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=28;offset=-2;feat=%2", capView, triFeatureState] call capLog }
call { capP = -3; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=29;offset=-1;feat=%2", capView, triFeatureState] call capLog }
call { capP = -2; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=30;offset=0;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_030"
call { capP = -1; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=31;offset=1;feat=%2", capView, triFeatureState] call capLog }
call { capP = 0; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=32;offset=2;feat=%2", capView, triFeatureState] call capLog }
call { capP = 1; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=33;offset=3;feat=%2", capView, triFeatureState] call capLog }
call { capP = 2; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=34;offset=4;feat=%2", capView, triFeatureState] call capLog }
call { capP = 3; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=35;offset=5;feat=%2", capView, triFeatureState] call capLog }
call { capP = 4; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=36;offset=6;feat=%2", capView, triFeatureState] call capLog }
call { capP = 5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=37;offset=7;feat=%2", capView, triFeatureState] call capLog }
call { capP = 6; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=38;offset=8;feat=%2", capView, triFeatureState] call capLog }
call { capP = 7; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=39;offset=9;feat=%2", capView, triFeatureState] call capLog }
call { capP = 8; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=40;offset=10;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_040"
call { capP = 9; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=41;offset=11;feat=%2", capView, triFeatureState] call capLog }
call { capP = 10; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=42;offset=12;feat=%2", capView, triFeatureState] call capLog }
call { capP = 11; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=43;offset=13;feat=%2", capView, triFeatureState] call capLog }
call { capP = 12; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=44;offset=14;feat=%2", capView, triFeatureState] call capLog }
call { capP = 13; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=45;offset=15;feat=%2", capView, triFeatureState] call capLog }
call { capP = 14; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=46;offset=16;feat=%2", capView, triFeatureState] call capLog }
call { capP = 15; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=47;offset=17;feat=%2", capView, triFeatureState] call capLog }
call { capP = 16; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=48;offset=18;feat=%2", capView, triFeatureState] call capLog }
call { capP = 17; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=49;offset=19;feat=%2", capView, triFeatureState] call capLog }
call { capP = 18; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=50;offset=20;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_050"
call { capP = 19; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=51;offset=21;feat=%2", capView, triFeatureState] call capLog }
call { capP = 20; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=52;offset=22;feat=%2", capView, triFeatureState] call capLog }
call { capP = 21; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=53;offset=23;feat=%2", capView, triFeatureState] call capLog }
call { capP = 22; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=54;offset=24;feat=%2", capView, triFeatureState] call capLog }
call { capP = 23; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=55;offset=25;feat=%2", capView, triFeatureState] call capLog }
call { capP = 23; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=56;offset=25;feat=%2", capView, triFeatureState] call capLog }
call { capP = 22; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=57;offset=24;feat=%2", capView, triFeatureState] call capLog }
call { capP = 21; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=58;offset=23;feat=%2", capView, triFeatureState] call capLog }
call { capP = 20; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=59;offset=22;feat=%2", capView, triFeatureState] call capLog }
call { capP = 19; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=60;offset=21;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_060"
call { capP = 18; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=61;offset=20;feat=%2", capView, triFeatureState] call capLog }
call { capP = 17; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=62;offset=19;feat=%2", capView, triFeatureState] call capLog }
call { capP = 16; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=63;offset=18;feat=%2", capView, triFeatureState] call capLog }
call { capP = 15; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=64;offset=17;feat=%2", capView, triFeatureState] call capLog }
call { capP = 14; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=65;offset=16;feat=%2", capView, triFeatureState] call capLog }
call { capP = 13; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=66;offset=15;feat=%2", capView, triFeatureState] call capLog }
call { capP = 12; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=67;offset=14;feat=%2", capView, triFeatureState] call capLog }
call { capP = 11; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=68;offset=13;feat=%2", capView, triFeatureState] call capLog }
call { capP = 10; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=69;offset=12;feat=%2", capView, triFeatureState] call capLog }
call { capP = 9; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=70;offset=11;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_070"
call { capP = 8; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=71;offset=10;feat=%2", capView, triFeatureState] call capLog }
call { capP = 7; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=72;offset=9;feat=%2", capView, triFeatureState] call capLog }
call { capP = 6; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=73;offset=8;feat=%2", capView, triFeatureState] call capLog }
call { capP = 5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=74;offset=7;feat=%2", capView, triFeatureState] call capLog }
call { capP = 4; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=75;offset=6;feat=%2", capView, triFeatureState] call capLog }
call { capP = 3; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=76;offset=5;feat=%2", capView, triFeatureState] call capLog }
call { capP = 2; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=77;offset=4;feat=%2", capView, triFeatureState] call capLog }
call { capP = 1; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=78;offset=3;feat=%2", capView, triFeatureState] call capLog }
call { capP = 0; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=79;offset=2;feat=%2", capView, triFeatureState] call capLog }
call { capP = -1; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=80;offset=1;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_080"
call { capP = -2; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=81;offset=0;feat=%2", capView, triFeatureState] call capLog }
call { capP = -3; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=82;offset=-1;feat=%2", capView, triFeatureState] call capLog }
call { capP = -4; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=83;offset=-2;feat=%2", capView, triFeatureState] call capLog }
call { capP = -5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=84;offset=-3;feat=%2", capView, triFeatureState] call capLog }
call { capP = -6; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=85;offset=-4;feat=%2", capView, triFeatureState] call capLog }
call { capP = -7; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=86;offset=-5;feat=%2", capView, triFeatureState] call capLog }
call { capP = -8; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=87;offset=-6;feat=%2", capView, triFeatureState] call capLog }
call { capP = -9; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=88;offset=-7;feat=%2", capView, triFeatureState] call capLog }
call { capP = -10; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=89;offset=-8;feat=%2", capView, triFeatureState] call capLog }
call { capP = -11; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=90;offset=-9;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_090"
call { capP = -12; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=91;offset=-10;feat=%2", capView, triFeatureState] call capLog }
call { capP = -13; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=92;offset=-11;feat=%2", capView, triFeatureState] call capLog }
call { capP = -14; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=93;offset=-12;feat=%2", capView, triFeatureState] call capLog }
call { capP = -15; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=94;offset=-13;feat=%2", capView, triFeatureState] call capLog }
call { capP = -16; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=95;offset=-14;feat=%2", capView, triFeatureState] call capLog }
call { capP = -17; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=96;offset=-15;feat=%2", capView, triFeatureState] call capLog }
call { capP = -18; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=97;offset=-16;feat=%2", capView, triFeatureState] call capLog }
call { capP = -19; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=98;offset=-17;feat=%2", capView, triFeatureState] call capLog }
call { capP = -20; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=99;offset=-18;feat=%2", capView, triFeatureState] call capLog }
call { capP = -21; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=100;offset=-19;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_100"
call { capP = -22; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=101;offset=-20;feat=%2", capView, triFeatureState] call capLog }
call { capP = -23; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=102;offset=-21;feat=%2", capView, triFeatureState] call capLog }
call { capP = -24; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=103;offset=-22;feat=%2", capView, triFeatureState] call capLog }
call { capP = -25; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=104;offset=-23;feat=%2", capView, triFeatureState] call capLog }
call { capP = -26; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=105;offset=-24;feat=%2", capView, triFeatureState] call capLog }
call { capP = -27; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=106;offset=-25;feat=%2", capView, triFeatureState] call capLog }
call { capP = -28; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=107;offset=-26;feat=%2", capView, triFeatureState] call capLog }
call { capP = -29; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=108;offset=-27;feat=%2", capView, triFeatureState] call capLog }
call { capP = -30; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=109;offset=-28;feat=%2", capView, triFeatureState] call capLog }
call { capP = -31; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=110;offset=-29;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_110"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=111;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=112;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=113;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=114;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=115;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=116;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=117;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=118;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=119;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=120;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_120"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=121;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=122;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=123;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=124;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=125;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=126;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=127;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=128;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=129;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=130;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_130"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=131;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=132;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=133;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=134;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=135;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=136;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=137;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=138;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=139;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=140;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_140"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=141;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=142;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=143;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=144;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=145;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=146;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=147;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=148;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=149;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=150;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_150"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=151;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=152;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=153;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=154;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=155;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=156;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=157;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=158;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=159;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=160;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_160"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=161;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=162;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=163;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=164;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=165;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=166;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=167;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=168;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=169;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=170;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_170"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=171;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=172;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=173;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=174;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=175;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=176;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=177;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=178;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=179;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=180;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_180"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=181;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=182;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=183;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=184;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=185;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=186;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=187;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=188;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=189;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=190;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_190"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=191;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=192;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=193;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=194;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=195;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=196;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=197;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=198;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=199;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=200;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__on_200"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=on;frame=201;offset=-30;feat=%2", capView, triFeatureState] call capLog }
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

// ---- shade_open__off
call { capRain = 0; capEyeAbs = false }
["shade_open__off", 6940, 9964, 1.75, 240, -2, [1985, 7, 5, 17, 30], 0, 0, 1500] call capBegin
call { capSimT = 3800; capSimRes = triSetSimTime capSimT; format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }
call { capDriftRes = triPinCloudDrift [1830, 970, 0]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }
call {
    [["Jeep"], 14, 2, 70, "veh"] call capSpawn;
    [["SoldierWB"], 9, -2, 150, "man"] call capSpawn;
}
triSimFrames 150
"remastered,photoLight=0" call capMode
call { capP = -32; [] call capPinView }
triSimFrames 120
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=0;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_000"
call { capP = -31; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=1;offset=-29;feat=%2", capView, triFeatureState] call capLog }
call { capP = -30; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=2;offset=-28;feat=%2", capView, triFeatureState] call capLog }
call { capP = -29; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=3;offset=-27;feat=%2", capView, triFeatureState] call capLog }
call { capP = -28; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=4;offset=-26;feat=%2", capView, triFeatureState] call capLog }
call { capP = -27; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=5;offset=-25;feat=%2", capView, triFeatureState] call capLog }
call { capP = -26; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=6;offset=-24;feat=%2", capView, triFeatureState] call capLog }
call { capP = -25; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=7;offset=-23;feat=%2", capView, triFeatureState] call capLog }
call { capP = -24; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=8;offset=-22;feat=%2", capView, triFeatureState] call capLog }
call { capP = -23; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=9;offset=-21;feat=%2", capView, triFeatureState] call capLog }
call { capP = -22; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=10;offset=-20;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_010"
call { capP = -21; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=11;offset=-19;feat=%2", capView, triFeatureState] call capLog }
call { capP = -20; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=12;offset=-18;feat=%2", capView, triFeatureState] call capLog }
call { capP = -19; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=13;offset=-17;feat=%2", capView, triFeatureState] call capLog }
call { capP = -18; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=14;offset=-16;feat=%2", capView, triFeatureState] call capLog }
call { capP = -17; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=15;offset=-15;feat=%2", capView, triFeatureState] call capLog }
call { capP = -16; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=16;offset=-14;feat=%2", capView, triFeatureState] call capLog }
call { capP = -15; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=17;offset=-13;feat=%2", capView, triFeatureState] call capLog }
call { capP = -14; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=18;offset=-12;feat=%2", capView, triFeatureState] call capLog }
call { capP = -13; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=19;offset=-11;feat=%2", capView, triFeatureState] call capLog }
call { capP = -12; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=20;offset=-10;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_020"
call { capP = -11; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=21;offset=-9;feat=%2", capView, triFeatureState] call capLog }
call { capP = -10; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=22;offset=-8;feat=%2", capView, triFeatureState] call capLog }
call { capP = -9; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=23;offset=-7;feat=%2", capView, triFeatureState] call capLog }
call { capP = -8; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=24;offset=-6;feat=%2", capView, triFeatureState] call capLog }
call { capP = -7; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=25;offset=-5;feat=%2", capView, triFeatureState] call capLog }
call { capP = -6; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=26;offset=-4;feat=%2", capView, triFeatureState] call capLog }
call { capP = -5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=27;offset=-3;feat=%2", capView, triFeatureState] call capLog }
call { capP = -4; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=28;offset=-2;feat=%2", capView, triFeatureState] call capLog }
call { capP = -3; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=29;offset=-1;feat=%2", capView, triFeatureState] call capLog }
call { capP = -2; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=30;offset=0;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_030"
call { capP = -1; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=31;offset=1;feat=%2", capView, triFeatureState] call capLog }
call { capP = 0; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=32;offset=2;feat=%2", capView, triFeatureState] call capLog }
call { capP = 1; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=33;offset=3;feat=%2", capView, triFeatureState] call capLog }
call { capP = 2; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=34;offset=4;feat=%2", capView, triFeatureState] call capLog }
call { capP = 3; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=35;offset=5;feat=%2", capView, triFeatureState] call capLog }
call { capP = 4; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=36;offset=6;feat=%2", capView, triFeatureState] call capLog }
call { capP = 5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=37;offset=7;feat=%2", capView, triFeatureState] call capLog }
call { capP = 6; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=38;offset=8;feat=%2", capView, triFeatureState] call capLog }
call { capP = 7; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=39;offset=9;feat=%2", capView, triFeatureState] call capLog }
call { capP = 8; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=40;offset=10;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_040"
call { capP = 9; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=41;offset=11;feat=%2", capView, triFeatureState] call capLog }
call { capP = 10; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=42;offset=12;feat=%2", capView, triFeatureState] call capLog }
call { capP = 11; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=43;offset=13;feat=%2", capView, triFeatureState] call capLog }
call { capP = 12; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=44;offset=14;feat=%2", capView, triFeatureState] call capLog }
call { capP = 13; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=45;offset=15;feat=%2", capView, triFeatureState] call capLog }
call { capP = 14; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=46;offset=16;feat=%2", capView, triFeatureState] call capLog }
call { capP = 15; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=47;offset=17;feat=%2", capView, triFeatureState] call capLog }
call { capP = 16; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=48;offset=18;feat=%2", capView, triFeatureState] call capLog }
call { capP = 17; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=49;offset=19;feat=%2", capView, triFeatureState] call capLog }
call { capP = 18; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=50;offset=20;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_050"
call { capP = 19; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=51;offset=21;feat=%2", capView, triFeatureState] call capLog }
call { capP = 20; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=52;offset=22;feat=%2", capView, triFeatureState] call capLog }
call { capP = 21; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=53;offset=23;feat=%2", capView, triFeatureState] call capLog }
call { capP = 22; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=54;offset=24;feat=%2", capView, triFeatureState] call capLog }
call { capP = 23; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=55;offset=25;feat=%2", capView, triFeatureState] call capLog }
call { capP = 23; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=56;offset=25;feat=%2", capView, triFeatureState] call capLog }
call { capP = 22; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=57;offset=24;feat=%2", capView, triFeatureState] call capLog }
call { capP = 21; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=58;offset=23;feat=%2", capView, triFeatureState] call capLog }
call { capP = 20; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=59;offset=22;feat=%2", capView, triFeatureState] call capLog }
call { capP = 19; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=60;offset=21;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_060"
call { capP = 18; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=61;offset=20;feat=%2", capView, triFeatureState] call capLog }
call { capP = 17; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=62;offset=19;feat=%2", capView, triFeatureState] call capLog }
call { capP = 16; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=63;offset=18;feat=%2", capView, triFeatureState] call capLog }
call { capP = 15; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=64;offset=17;feat=%2", capView, triFeatureState] call capLog }
call { capP = 14; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=65;offset=16;feat=%2", capView, triFeatureState] call capLog }
call { capP = 13; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=66;offset=15;feat=%2", capView, triFeatureState] call capLog }
call { capP = 12; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=67;offset=14;feat=%2", capView, triFeatureState] call capLog }
call { capP = 11; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=68;offset=13;feat=%2", capView, triFeatureState] call capLog }
call { capP = 10; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=69;offset=12;feat=%2", capView, triFeatureState] call capLog }
call { capP = 9; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=70;offset=11;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_070"
call { capP = 8; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=71;offset=10;feat=%2", capView, triFeatureState] call capLog }
call { capP = 7; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=72;offset=9;feat=%2", capView, triFeatureState] call capLog }
call { capP = 6; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=73;offset=8;feat=%2", capView, triFeatureState] call capLog }
call { capP = 5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=74;offset=7;feat=%2", capView, triFeatureState] call capLog }
call { capP = 4; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=75;offset=6;feat=%2", capView, triFeatureState] call capLog }
call { capP = 3; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=76;offset=5;feat=%2", capView, triFeatureState] call capLog }
call { capP = 2; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=77;offset=4;feat=%2", capView, triFeatureState] call capLog }
call { capP = 1; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=78;offset=3;feat=%2", capView, triFeatureState] call capLog }
call { capP = 0; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=79;offset=2;feat=%2", capView, triFeatureState] call capLog }
call { capP = -1; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=80;offset=1;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_080"
call { capP = -2; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=81;offset=0;feat=%2", capView, triFeatureState] call capLog }
call { capP = -3; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=82;offset=-1;feat=%2", capView, triFeatureState] call capLog }
call { capP = -4; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=83;offset=-2;feat=%2", capView, triFeatureState] call capLog }
call { capP = -5; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=84;offset=-3;feat=%2", capView, triFeatureState] call capLog }
call { capP = -6; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=85;offset=-4;feat=%2", capView, triFeatureState] call capLog }
call { capP = -7; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=86;offset=-5;feat=%2", capView, triFeatureState] call capLog }
call { capP = -8; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=87;offset=-6;feat=%2", capView, triFeatureState] call capLog }
call { capP = -9; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=88;offset=-7;feat=%2", capView, triFeatureState] call capLog }
call { capP = -10; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=89;offset=-8;feat=%2", capView, triFeatureState] call capLog }
call { capP = -11; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=90;offset=-9;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_090"
call { capP = -12; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=91;offset=-10;feat=%2", capView, triFeatureState] call capLog }
call { capP = -13; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=92;offset=-11;feat=%2", capView, triFeatureState] call capLog }
call { capP = -14; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=93;offset=-12;feat=%2", capView, triFeatureState] call capLog }
call { capP = -15; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=94;offset=-13;feat=%2", capView, triFeatureState] call capLog }
call { capP = -16; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=95;offset=-14;feat=%2", capView, triFeatureState] call capLog }
call { capP = -17; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=96;offset=-15;feat=%2", capView, triFeatureState] call capLog }
call { capP = -18; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=97;offset=-16;feat=%2", capView, triFeatureState] call capLog }
call { capP = -19; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=98;offset=-17;feat=%2", capView, triFeatureState] call capLog }
call { capP = -20; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=99;offset=-18;feat=%2", capView, triFeatureState] call capLog }
call { capP = -21; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=100;offset=-19;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_100"
call { capP = -22; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=101;offset=-20;feat=%2", capView, triFeatureState] call capLog }
call { capP = -23; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=102;offset=-21;feat=%2", capView, triFeatureState] call capLog }
call { capP = -24; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=103;offset=-22;feat=%2", capView, triFeatureState] call capLog }
call { capP = -25; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=104;offset=-23;feat=%2", capView, triFeatureState] call capLog }
call { capP = -26; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=105;offset=-24;feat=%2", capView, triFeatureState] call capLog }
call { capP = -27; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=106;offset=-25;feat=%2", capView, triFeatureState] call capLog }
call { capP = -28; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=107;offset=-26;feat=%2", capView, triFeatureState] call capLog }
call { capP = -29; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=108;offset=-27;feat=%2", capView, triFeatureState] call capLog }
call { capP = -30; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=109;offset=-28;feat=%2", capView, triFeatureState] call capLog }
call { capP = -31; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=110;offset=-29;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_110"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=111;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=112;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=113;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=114;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=115;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=116;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=117;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=118;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=119;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=120;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_120"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=121;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=122;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=123;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=124;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=125;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=126;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=127;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=128;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=129;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=130;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_130"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=131;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=132;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=133;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=134;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=135;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=136;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=137;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=138;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=139;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=140;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_140"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=141;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=142;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=143;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=144;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=145;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=146;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=147;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=148;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=149;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=150;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_150"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=151;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=152;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=153;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=154;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=155;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=156;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=157;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=158;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=159;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=160;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_160"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=161;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=162;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=163;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=164;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=165;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=166;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=167;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=168;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=169;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=170;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_170"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=171;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=172;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=173;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=174;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=175;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=176;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=177;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=178;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=179;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=180;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_180"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=181;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=182;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=183;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=184;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=185;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=186;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=187;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=188;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=189;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=190;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_190"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=191;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=192;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=193;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=194;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=195;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=196;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=197;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=198;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=199;offset=-30;feat=%2", capView, triFeatureState] call capLog }
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=200;offset=-30;feat=%2", capView, triFeatureState] call capLog }
triScreenshot "shade_open__off_200"
call { capP = -32; [] call capPinView }
triSimFrames 1
call { format ["view=%1;event=motion;move=shade_open;mode=off;frame=201;offset=-30;feat=%2", capView, triFeatureState] call capLog }
[] call capClear
call { capDriftRes = triPinCloudDrift [] }

call { "view=done;event=done" call capLog }
triEndTest
