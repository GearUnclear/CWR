// throwaway ground-truth probe for issue #76 (agent2), not a test
triSimUntil { GM_LIB_READY }
gtT0 = time
triSimUntil { time > gtT0 + 2 }
gtSites = [["ILS anchor", 7922, 9590], ["runway mid", 7922, 10020], ["budova1 hangar row", 8088, 9922], ["pletivo_wired", 7799, 9593], ["zone Airfield", 8030, 9905], ["molo_beton", 8476, 3754], ["molo_krychle", 8472, 3774], ["zone Harbour", 8410, 3690], ["molo_drevo_bs", 7491, 3602], ["majak LePort", 8269, 2860], ["znacka_leport", 8174, 3168], ["hlaska compound", 9639, 4006], ["garaz compound", 9657, 3914], ["garaz_s_tankem", 9745, 4002], ["strazni_vez", 9771, 4006], ["zone Compound", 9715, 3940]]
gtOut = ""
{gtS = _x; gtP = [gtS select 1, gtS select 2, 0]; gtNo = nearestObjects [gtP, [], 8]; gtO = "none"; gtTy = ""; gtA = []; if ((count gtNo) > 0) then {gtO = format ["%1", gtNo select 0]; gtTy = typeOf (gtNo select 0); gtA = getPosASL (gtNo select 0)}; logInfo format ["GT76|%1|%2|%3|%4|lm %5|geo %6", gtS select 0, gtO, gtTy, gtA, gmLandmass gtP, gmGeoGrid [gtP, 1]]} forEach gtSites
gtZ = ""
gtI = 0
while {gtI < gmZoneCount} do {gtR = gmZone gtI; logInfo format ["GT76Z|%1|%2|%3|%4|%5|%6", gtI, gtR select 0, gtR select 1, gtR select 2, gtR select 3, gtR select 8]; gtI = gtI + 1}
logInfo format ["GT76F|%1", gmOccupierForces]
triEndTest
