// throwaway probe 2 (#76 agent2)
triSimUntil { GM_LIB_READY }
gtT0 = time
triSimUntil { time > gtT0 + 2 }
gtSites = [["Sharm pier0", 10713, 3494], ["Sharm pier1p", 10813, 3444], ["Sharm pier0b", 10563, 3544], ["ElTor pier1p", 5984, 3194], ["ElTor pier2u", 5884, 3144]]
{gtS = _x; gtP = [gtS select 1, gtS select 2, 0]; gtNo = nearestObjects [gtP, [], 45]; gtL = []; {gtL = gtL + [format ["%1 @%2", _x, getPosASL _x]]} forEach gtNo; logInfo format ["GT76B|%1|n %2|%3", gtS select 0, count gtNo, gtL]} forEach gtSites
triEndTest
