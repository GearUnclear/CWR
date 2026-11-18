// Lebanon80 promo scout: road points and houses around the shoot places, and an AI movement check.
triSimUntil { GM_LIB_READY }
triSimUntil { alive player }
player allowDammage false
player setCaptive true
call compile preprocessFile "promo_lib.sqf"
call { gmTrafficTune ["maxCiv", 0]; gmTrafficTune ["maxPatrols", 0]; gmTrafficTune ["maxConvoys", 0]; gmTrafficTune ["maxParked", 0] }
scDump = { private ["_c", "_r", "_s"]; _c = _this select 1; _r = _c nearestRoads (_this select 2); _s = ""; { _s = _s + format [" %1,%2", (_x select 0) - ((_x select 0) mod 1), (_x select 1) - ((_x select 1) mod 1)] } forEach _r; logInfo format ["SC road %1%2", _this select 0, _s]; _s = ""; { _s = _s + format [" %1,%2", ((getPos _x) select 0) - (((getPos _x) select 0) mod 1), ((getPos _x) select 1) - (((getPos _x) select 1) mod 1)] } forEach (nearestObjects [_c, ["House"], _this select 2]); logInfo format ["SC house %1%2", _this select 0, _s] }
call { ["ghajar", [14150, 11250, 0], 450] call scDump }
call { ["tyre", [8950, 9800, 0], 600] call scDump }
call { ["marj", [13043, 12140, 0], 400] call scDump }
call { ["northrd", [9600, 11000, 0], 700] call scDump }
call { ["coast", [10300, 8500, 0], 900] call scDump }
call { ["camp", [11500, 12000, 0], 500] call scDump }
call { { logInfo format ["SC path %1 %2 -> %3", count (gmRoadPath _x), _x select 0, _x select 1] } forEach [[[9550, 10500, 0], [9800, 12300, 0]], [[11900, 8480, 0], [8800, 8520, 0]], [[14150, 11250, 0], [13043, 12140, 0]], [[9550, 10500, 0], [9600, 11000, 0]]] }
// movement: a group in the olive plain, leader doMove 120 m; a second group with move
scG = [west, ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB"], [10500, 9200, 0], 300, 0.7] call psInf
call { scG setFormation "FILE"; scG setBehaviour "SAFE"; scG setSpeedMode "LIMITED"; (leader scG) doMove [10400, 9300, 0]; logInfo format ["SC g1 leader %1 units %2", leader scG, units scG] }
scH = [east, ["LoBo_HizballahLeader", "LoBo_HizballahRifle4", "LoBo_HizballahMG1"], [10600, 9100, 0], 300, 0.7] call psInf
call { scH setBehaviour "AWARE"; scH setSpeedMode "LIMITED"; scH move [10500, 9200, 0] }
scT0 = time
call { for "_k" from 1 to 6 do { triSimFrames 90 } }
triSimFrames 150
call { logInfo format ["SC t %1 g1 leader %2 last %3 | g2 leader %4 last %5", time - scT0, getPos (leader scG), getPos ((units scG) select 3), getPos (leader scH), getPos ((units scH) select 2)] }
triSimFrames 300
call { logInfo format ["SC t %1 g1 leader %2 last %3 | g2 leader %4 last %5", time - scT0, getPos (leader scG), getPos ((units scG) select 3), getPos (leader scH), getPos ((units scH) select 2)] }
triSimFrames 300
call { logInfo format ["SC t %1 g1 leader %2 last %3 | g2 leader %4 last %5 | %6", time - scT0, getPos (leader scG), getPos ((units scG) select 3), getPos (leader scH), getPos ((units scH) select 2), daytime] }
logInfo "SC END"
triEndTest
