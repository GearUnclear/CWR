// throwaway probe 2 (#76 agent2)
triSimUntil { GM_LIB_READY }
gtT0 = time
triSimUntil { time > gtT0 + 2 }
gtSites = [["Tyre nabrezi", 8588, 9739], ["Tyre najezd", 8588, 9798], ["Tyre nabrezi2", 8588, 9857], ["Haifa pier area", 6120, 5415], ["Beirut crane1", 12871, 18001], ["Beirut pier_a1", 13019, 17964]]
{gtS = _x; gtP = [gtS select 1, gtS select 2, 0]; gtNo = nearestObjects [gtP, [], 45]; gtL = []; {gtL = gtL + [format ["%1 @%2", _x, getPosASL _x]]} forEach gtNo; logInfo format ["GT76B|%1|n %2|%3", gtS select 0, count gtNo, gtL]} forEach gtSites
triEndTest
