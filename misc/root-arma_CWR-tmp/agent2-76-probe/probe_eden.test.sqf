// throwaway ground-truth probe for issue #76 (agent2), not a test
triSimUntil { GM_LIB_READY }
gtT0 = time
triSimUntil { time > gtT0 + 2 }
gtSites = [["ILS anchor", 4772, 10923], ["runway mid", 4772, 11360], ["znacka_letiste", 5007, 11048], ["budova5 airfield", 4477, 11203], ["budova1 airfield", 4457, 11233], ["hlaska base", 5101, 10499], ["budova2 base", 5122, 10517], ["molo_beton", 7392, 6835], ["molo_krychle", 7381, 6828], ["vez pier", 7299, 6798], ["budova1 pier", 7280, 6785], ["Outpost 1", 7535, 8760], ["Outpost 2", 11439, 1557], ["Outpost 3", 9267, 4432], ["Saint Phillippe", 4674, 10730]]
gtOut = ""
{gtS = _x; gtP = [gtS select 1, gtS select 2, 0]; gtNo = nearestObjects [gtP, [], 8]; gtO = "none"; gtTy = ""; gtA = []; if ((count gtNo) > 0) then {gtO = format ["%1", gtNo select 0]; gtTy = typeOf (gtNo select 0); gtA = getPosASL (gtNo select 0)}; logInfo format ["GT76|%1|%2|%3|%4|lm %5|geo %6", gtS select 0, gtO, gtTy, gtA, gmLandmass gtP, gmGeoGrid [gtP, 1]]} forEach gtSites
gtZ = ""
gtI = 0
while {gtI < gmZoneCount} do {gtR = gmZone gtI; logInfo format ["GT76Z|%1|%2|%3|%4|%5|%6", gtI, gtR select 0, gtR select 1, gtR select 2, gtR select 3, gtR select 8]; gtI = gtI + 1}
logInfo format ["GT76F|%1", gmOccupierForces]
triEndTest
