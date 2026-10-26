// throwaway ground-truth probe for issue #76 (agent2), not a test
triSimUntil { GM_LIB_READY }
gtT0 = time
triSimUntil { time > gtT0 + 2 }
gtSites = [["ILS anchor IsrAB", 12176, 11476], ["IsrAB tmphanger", 11928, 11245], ["Wadi hangar", 8079, 3767], ["Wadi start_turm", 8045, 3732], ["Wadi HAS", 8067, 3598], ["EgAB hangar01", 3573, 1583], ["EgAB Names", 3443, 1479], ["EgAB brown_turm", 3606, 1441], ["ElTor crane", 5908, 3158], ["ElTor pier1p", 5984, 3194], ["ElTor pier2", 5934, 3244], ["ElTor Names", 6827, 2808], ["ElTor garrison hlaska", 6333, 3055], ["ElTor garrison fortress1", 6351, 3042], ["Sharm crane", 10587, 3382], ["Sharm pier0", 10713, 3494], ["Sharm pier1", 10713, 3444], ["Sharm hangar_2", 10708, 3614], ["Sharm Names", 9284, 3763]]
gtOut = ""
{gtS = _x; gtP = [gtS select 1, gtS select 2, 0]; gtNo = nearestObjects [gtP, [], 8]; gtO = "none"; gtTy = ""; gtA = []; if ((count gtNo) > 0) then {gtO = format ["%1", gtNo select 0]; gtTy = typeOf (gtNo select 0); gtA = getPosASL (gtNo select 0)}; logInfo format ["GT76|%1|%2|%3|%4|lm %5|geo %6", gtS select 0, gtO, gtTy, gtA, gmLandmass gtP, gmGeoGrid [gtP, 1]]} forEach gtSites
gtZ = ""
gtI = 0
while {gtI < gmZoneCount} do {gtR = gmZone gtI; logInfo format ["GT76Z|%1|%2|%3|%4|%5|%6", gtI, gtR select 0, gtR select 1, gtR select 2, gtR select 3, gtR select 8]; gtI = gtI + 1}
logInfo format ["GT76F|%1", gmOccupierForces]
triEndTest
