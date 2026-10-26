// research probe (night track): road centres near two areas with ground heights
#include "capture_lib.sqf"
triSimUntil { alive player }
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
triSimFrames 30
call { { capR = gmRoadsNear [_x, 500]; { format ["ROADPT;%1;%2;%3;%4", _x select 0, _x select 1, [_x select 0, _x select 1] call capGround, _x select 2] call capLog } forEach capR } forEach [[7152, 8915], [7264, 8368], [6500, 9500]] }
call { "view=done;event=done" call capLog }
triEndTest