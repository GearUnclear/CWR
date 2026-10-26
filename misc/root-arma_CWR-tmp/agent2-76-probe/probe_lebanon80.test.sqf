// throwaway ground-truth probe for issue #76 (agent2), not a test
triSimUntil { GM_LIB_READY }
gtT0 = time
triSimUntil { time > gtT0 + 2 }
gtSites = [["ILS anchor RamatDavid", 8628, 2524], ["RamatDavid HAS", 8449, 2476], ["Tyre nabrezi", 8588, 9739], ["Tyre nabrezi_najezd", 8588, 9798], ["Tyre fortress2", 9056, 9690], ["Tyre fortress1", 9027, 9681], ["Tyre radiotower", 9066, 9664], ["Haifa crane", 6120, 5415], ["Beirut crane1", 12871, 18001], ["Beirut blok100", 12726, 18073], ["MtDov Names", 15416, 10684], ["Shaaba fortress2", 14353, 10605], ["Shaaba dhangar_radio", 14155, 10768]]
gtOut = ""
{gtS = _x; gtP = [gtS select 1, gtS select 2, 0]; gtNo = nearestObjects [gtP, [], 8]; gtO = "none"; gtTy = ""; gtA = []; if ((count gtNo) > 0) then {gtO = format ["%1", gtNo select 0]; gtTy = typeOf (gtNo select 0); gtA = getPosASL (gtNo select 0)}; logInfo format ["GT76|%1|%2|%3|%4|lm %5|geo %6", gtS select 0, gtO, gtTy, gtA, gmLandmass gtP, gmGeoGrid [gtP, 1]]} forEach gtSites
gtZ = ""
gtI = 0
while {gtI < gmZoneCount} do {gtR = gmZone gtI; logInfo format ["GT76Z|%1|%2|%3|%4|%5|%6", gtI, gtR select 0, gtR select 1, gtR select 2, gtR select 3, gtR select 8]; gtI = gtI + 1}
logInfo format ["GT76F|%1", gmOccupierForces]
triEndTest
