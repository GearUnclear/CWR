// RESEARCH (spawned-vehicles bug fix): cost of OptimizeLate. Spawns vehicle
// classes the mission does not place, in two bursts, with the debug log on,
// so the PERF lines of ShapeBank::New and ShapeBank::OptimizeLate show.
#include "capture_lib.sqf"

triSimUntil { alive player }
player setCaptive true
player allowDamage false
capView = "hitch"
triSimFrames 120
call { "view=hitch;event=burst1" call capLog }
call {
    capH1 = [];
    { capH1 = capH1 + [_x createVehicle [7300 + 20 * (count capH1), 10900, 0]] } forEach ["M113", "Jeep", "T72", "BMP", "Ural", "M60", "UH60", "Truck5t"];
}
triSimFrames 60
call { "view=hitch;event=burst2" call capLog }
call {
    capH2 = [];
    { capH2 = capH2 + [_x createVehicle [7300 + 20 * (count capH2), 10950, 0]] } forEach ["M1Abrams", "T80", "BRDM", "UAZ", "Mi17", "Skoda", "Bus", "Scud"];
}
triSimFrames 60
call { format ["view=hitch;event=done;n1=%1;n2=%2", count capH1, count capH2] call capLog }
triEndTest