// Lebanon80 promo scout 2: infantry and vehicle movement variants, isolated (no enemies near), fixed 1/30 s step.
triSimUntil { GM_LIB_READY }
triSimUntil { alive player }
player allowDammage false
player setCaptive true
call compile preprocessFile "promo_lib.sqf"
call { gmTrafficTune ["maxCiv", 0]; gmTrafficTune ["maxPatrols", 0]; gmTrafficTune ["maxConvoys", 0]; gmTrafficTune ["maxParked", 0] }
call { player setPos [10300, 8600, 0] }
call { prStep = triSetFixedStep 0.0333333; setAccTime 1 }
// A: leader doMove, FILE, SAFE, LIMITED (Ghajar north road, walking south into town)
sA = [west, ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB"], [14115, 11400, 0], 180, 0.7] call psInf
call { sA setFormation "FILE"; sA setBehaviour "SAFE"; sA setSpeedMode "LIMITED"; (leader sA) doMove [14116, 11110, 0] }
// B: every unit doMove, AWARE, LIMITED (olive plain)
sB = [west, ["LoBoGolaniWBo", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWB"], [10500, 9200, 0], 300, 0.7] call psInf
call { sB setBehaviour "AWARE"; sB setSpeedMode "LIMITED"; { _x doMove [10380 + 3 * (random 4), 9300 + 3 * (random 4), 0] } forEach (units sB) }
// C: group move, SAFE, NORMAL (Tyre south road)
sC = [east, ["LoBo_HizballahLeader", "LoBo_HizballahRifle4", "LoBo_HizballahMG1"], [9150, 9300, 0], 0, 0.7] call psInf
call { sC setBehaviour "SAFE"; sC setSpeedMode "NORMAL"; sC move [9190, 9520, 0] }
// D: three tanks one group, leader doMove, coast highway eastward
call { sT1 = ["LoBoMerkava2", [10000, 8307, 0], 90] call psVeh; sT2 = ["LoBoMerkava3", [9960, 8307, 0], 90] call psVeh; sT3 = ["LoBo_Magach7A", [9920, 8307, 0], 90] call psVeh; sTG = [sT1, west, "LoBoGolaniWB", true, true] call psCrew; [sT2, sTG, "LoBoGolaniWB", true, true] call psCrewG; [sT3, sTG, "LoBoGolaniWB", true, true] call psCrewG; sTG setFormation "COLUMN"; sTG setBehaviour "SAFE"; sTG setSpeedMode "NORMAL"; (leader sTG) doMove [11150, 8307, 0] }
// E: tank alone doMove (each driver)
call { sE = ["LoBoMerkava2", [10500, 8307, 0], 90] call psVeh; sEG = [sE, west, "LoBoGolaniWB", true, true] call psCrew; sEG setBehaviour "CARELESS"; (driver sE) doMove [11150, 8307, 0] }
sLog = { logInfo format ["SC2 t %1 | A %2 %3 | B %4 %5 | C %6 %7 | T %8 %9 %10 | E %11", time, getPos (leader sA), getPos ((units sA) select 3), getPos ((units sB) select 0), getPos ((units sB) select 3), getPos (leader sC), getPos ((units sC) select 2), getPos sT1, getPos sT2, getPos sT3, getPos sE] }
call { [] call sLog }
triSimFrames 150
call { [] call sLog }
triSimFrames 300
call { [] call sLog }
triSimFrames 300
call { [] call sLog }
triSimFrames 450
call { [] call sLog; logInfo format ["SC2 behaviours %1 %2 %3 %4", behaviour (leader sA), behaviour (leader sB), behaviour (leader sC), behaviour (leader sTG)] }
logInfo "SC2 END"
triEndTest
