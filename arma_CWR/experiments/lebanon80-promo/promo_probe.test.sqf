// Lebanon80 promo probe: classes, zones, village map, candidate views, hint test.
triSimUntil { GM_LIB_READY }
triSimUntil { alive player }
triCaptureAtPresent true
player allowDammage false
call { prLit = triSetLighting "remastered"; logInfo format ["PR lit %1", prLit] }
setViewDistance 3500
logInfo format ["PR player %1 daytime %2 side %3 type %4 zones %5", getPos player, daytime, side player, typeOf player, gmZoneCount]
call { for "_i" from 0 to (gmZoneCount - 1) do { logInfo format ["PR zone %1 %2", _i, gmZone _i] } }
prHas = { (triAssertConfigClass (format ["CfgVehicles/%1", _this])) == "OK" }
call { { logInfo format ["PR class %1 %2", _x, _x call prHas] } forEach ["LoBo_M60A1_IDF", "LoBo_Magach7A", "LoBo_Magach7C", "LoBoMerkava1", "LoBoMerkava2", "LoBoMerkava3", "LoBo_Achzarit", "LoBo_Zelda", "LoBo_Zelda2", "LoBo_HMWV_Mag_Patrol", "LoBoHMWV_Mag_Patrol", "LoBo_HMWV_Mag_M240", "LoBo_HMWVUP_IDF_M240", "LoBoM60A1", "LoBo_Centurion2", "LoBo_M35", "LoBo_Truck5tcovered", "LoBo_AH1_IAF", "LoBo_AH1", "LoBo_B212_g", "LoBoB212", "LoBo_CH53B", "LoBo_jeep_mg_Hizbal", "LoBo_JeepMG_Hizballah", "LoBo_Ural_Hizballah", "LoBo_ural_Hizbal", "LoBo_v3s_Hizbal", "LoBo_JeepAT_Hizballah", "LoBoGolaniWB", "LoBoGolaniWMG", "LoBoGolaniWLAW", "LoBoGolaniWBo", "LoBoParaWB", "LoBo_HizballahRifle4", "LoBo_HizballahRifle1", "LoBo_HizballahMG1", "LoBo_Hizballah_RPG", "LoBo_HizballahLeader", "LoBo_HizballahSniper2", "LoBo_Terror_01E", "LoBo_Civ_01", "LoBo_S1203", "LoBo_Uaz3741", "Skoda", "Bus", "FlagCarrier", "LoBo_M60A1_wreck"] }
prProbe = "HeliHEmpty" createVehicle [0, 0, 0]
prGnd = { prProbe setPos [_this select 0, _this select 1, 0]; (getPosASL prProbe) select 2 }
call { for "_y" from 16000 to 6000 step -500 do { _r = ""; for "_x" from 5000 to 19000 step 500 do { _r = _r + format [" %1", (([_x, _y] call prGnd) - (([_x, _y] call prGnd) mod 1))] }; logInfo format ["PR H %1%2", _y, _r] } }
call { for "_y" from 16000 to 6000 step -500 do { _r = ""; for "_x" from 5000 to 19000 step 500 do { _r = _r + format [" %1", count (nearestObjects [[_x, _y, 0], ["House"], 250])] }; logInfo format ["PR B %1%2", _y, _r] } }
call { for "_y" from 16000 to 6000 step -500 do { _r = ""; for "_x" from 5000 to 19000 step 500 do { _r = _r + format [" %1", count ([_x, _y, 0] nearestRoads 250)] }; logInfo format ["PR R %1%2", _y, _r] } }
call { logInfo format ["PR traffic %1", gmTrafficDiag] }
prRd = [12000, 12000, 0] nearestRoads 400
call { logInfo format ["PR roads near checkpoint %1 first %2 last %3", count prRd, prRd select 0, prRd select ((count prRd) - 1)] }
prT = "LoBoMerkava2" createVehicle [11700, 11800, 0]
call { logInfo format ["PR spawn merkava %1 %2", typeOf prT, getPos prT] }
prHe = "LoBoB212" createVehicle [11800, 11750, 0]
call { logInfo format ["PR spawn heli %1 %2", typeOf prHe, getPos prHe] }
prZ = "LoBo_Zelda" createVehicle (prRd select 0)
prG = createGroup west
call { "LoBoGolaniWB" createUnit [getPos prZ, prG, "prD = this", 0.7, "PRIVATE"]; prD moveInDriver prZ; prG setBehaviour "CARELESS"; prD doMove (prRd select ((count prRd) - 1)) }
prZ0 = getPos prZ
triSimFrames 900
call { logInfo format ["PR zelda moved %1 m, driver %2, speed %3", prZ distance prZ0, driver prZ, speed prZ] }
prCam = "camera" camCreate [11500, 11700, 80]
prCam cameraEffect ["internal", "back"]
showCinemaBorder false
enableRadio false
prV = { prCam camSetPos (_this select 0); prCam camSetTarget (_this select 1); prCam camSetFov (_this select 2); prCam camCommit 0 }
call { [[11000, 9000, 1200], [11800, 11800, 0], 0.7] call prV }
triSimFrames 90
triScreenshot "overview_south_high"
call { [[11800, 14800, 1500], [11800, 11000, 0], 0.7] call prV }
triSimFrames 90
triScreenshot "overview_north_high"
call { [[9700, 9300, 150], [9100, 9800, 0], 0.7] call prV }
triSimFrames 90
triScreenshot "tyre_aerial"
call { [[8500, 9000, 60], [9300, 10500, 0], 0.7] call prV }
triSimFrames 90
triScreenshot "tyre_coast_low"
call { [[11100, 12600, 150], [10500, 13050, 0], 0.7] call prV }
triSimFrames 90
triScreenshot "saida_aerial"
call { [[13450, 11800, 150], [13043, 12140, 0], 0.7] call prV }
triSimFrames 90
triScreenshot "marj_aerial"
call { [[14450, 10950, 150], [14071, 11262, 0], 0.7] call prV }
triSimFrames 90
triScreenshot "ghajar_aerial"
call { [[11250, 11650, 80], [11500, 12000, 0], 0.7] call prV }
triSimFrames 90
triScreenshot "camp_aerial"
call { [[11820, 11850, 30], [12000, 12000, 0], 0.7] call prV }
triSimFrames 90
triScreenshot "checkpoint_low"
call { [[(getPos player select 0) - 6, (getPos player select 1) - 6, 2], getPos player, 0.7] call prV }
triSimFrames 30
hint "PROBE HINT TEXT"
triSimFrames 10
triScreenshot "player_hint_test"
call { logInfo format ["PR garrison live %1 %2", gmGarrisonLive 1, gmGarrisonLive 2] }
call { prMen = nearestObjects [getPos player, ["Man"], 1500]; logInfo format ["PR men near player %1 west %2 east %3", count prMen, {side _x == west} count prMen, {side _x == east} count prMen] }
logInfo "PR END"
triEndTest
