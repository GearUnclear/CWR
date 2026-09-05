// ============================================================================
//  Market.Abel reference mission - the money loop end to end, on the
//  campaign's REAL policy script (\gmcore\scripts\market.sqs out of the ONE
//  shared core) driven by the native GuerrillaBase + Market. Proves, on full CWA
//  data (Abel):
//      1. the mission boots its own bootstrap (MKT_BOOTED), the shared-core
//         subset handshakes (GM_LIB_READY) and market.sqs is running
//         (GM_MKT_READY); the market is active, no HQ yet, the hqEstablish
//         objective is ACTIVE;
//      2. the first market tick draws the dealers over the CITY zones: per
//         kind exactly round(cities * dealerShare) (at least 1), every dealer
//         NPC spawns alive;
//      3. the mission's debug teleport (act_tp_arms.sqs) puts the player at
//         the arms dealer, market.sqs mounts the BUY menu; driving its
//         dispatcher (market_action.sqs) debits the price and drops a
//         WeaponHolder at the player's feet carrying the class;
//      4. inside the nearest town, ESTABLISH HEADQUARTERS elects the HQ in
//         that CITY zone, the cache holder exists and is a registered stash,
//         the objective flips DONE;
//      5. at the cache: STASH moves the rifle + its magazines into the holder
//         (removeWeapon + cargo), the script-fallback retrieval path
//         (removeWeaponCargo + addWeapon) brings it back, the emptied holder
//         survives (keep-when-empty);
//      6. delivery toggled to HQ: a weapon purchase lands in the cache cargo,
//         a vehicle purchase lands in the garage ring LOCKED (gmGarageCount,
//         locked, within 30 m); gmGarageLock unlock/relock round-trips;
//      7. MOVE HEADQUARTERS at the Camp: the outdoor fallback (not indoors),
//         the move is debited hqMoveCost, the cache travels with its cargo,
//         and the hull left in the old town is released from the garage.
// ============================================================================

triSimUntil { not (isNil "MKT_BOOTED") }
triSimUntil { not (isNil "GM_LIB_READY") }
triSimUntil { GM_LIB_READY }
triSimUntil { not (isNil "GM_MKT_READY") }
triAssert [gmMarketActive]
triAssert [not gmHqEstablished]
triAssertEq [(gmJournalObjectiveState "hqEstablish"), "ACTIVE"]

// -- dealers drawn over the towns on the first market tick ---------------------
triSimUntil { gmDealerCount >= 2 }
mkCities = 0; mkI = 0; while {mkI < gmZoneCount} do {if (((gmZone mkI) select 1) == "CITY") then {mkCities = mkCities + 1}; mkI = mkI + 1}
triAssertGe [mkCities, 3]
mkQuota = mkCities * (gmMarketValue "dealerShare"); mkQuota = (mkQuota + 0.5) - ((mkQuota + 0.5) mod 1); if (mkQuota < 1) then {mkQuota = 1}
mkW = 0; mkV = 0; mkI = 0; while {mkI < gmDealerCount} do {if (((gmDealer mkI) select 1) == "WEAPON") then {mkW = mkW + 1} else {mkV = mkV + 1}; mkI = mkI + 1}
triAssertEq [mkW, mkQuota]
triAssertEq [mkV, mkQuota]
// every dealer body stands (CIV class, alive) once the spawn tick has run
GM_tstDealersUp = {GM_tstAll = true; GM_tstI = 0; while {GM_tstI < gmDealerCount} do {GM_tstN = (gmDealer GM_tstI) select 3; if ((isNull GM_tstN) or (not (alive GM_tstN))) then {GM_tstAll = false}; GM_tstI = GM_tstI + 1}; GM_tstAll}
triSimUntil { call GM_tstDealersUp }

// -- buy at the arms dealer, delivery "here" ------------------------------------
[] exec "act_tp_arms.sqs"
triSimUntil { (count gmMktBuyActs) > 0 }
triAssertEq [gmMktDealerKind, "WEAPON"]
mkStock = gmDealerStock "WEAPON"
triAssertGe [(count mkStock), 10]
mkRow = mkStock select 0
mkPrice = mkRow select 2
triAssertGt [mkPrice, 0]
mkR0 = gmResources
[aP, aP, gmMktBuyActs select 0] exec "\gmcore\scripts\market_action.sqs"
triSimUntil { gmResources == (mkR0 - mkPrice) }
// the holder is dropped at the player's feet; createVehicle's free-position
// nudge can shove it a few metres in a built-up town, so look a bit wider
triSimUntil { (count (nearestObjects [aP, ["WeaponHolder"], 15])) >= 1 }
mkHolders = nearestObjects [aP, ["WeaponHolder"], 15]
triAssert [(mkRow select 0) in (weaponCargo (mkHolders select 0))]

// -- establish the HQ in the nearest town --------------------------------------
[] exec "act_tp_town.sqs"
triSimUntil { gmHqCanEstablish (getPos aP) }
triSimUntil { gmMktHqActive }
[aP, aP, gmMktHqAct] exec "\gmcore\scripts\market_action.sqs"
triSimUntil { gmHqEstablished }
triAssertEq [gmHqMoveCount, 0]
triAssertEq [((gmZone (gmZoneIndex gmHqZone)) select 1), "CITY"]
triSimUntil { not (isNull gmHqCache) }
triAssertGe [gmStashCount, 1]
triSimUntil { (gmJournalObjectiveState "hqEstablish") == "DONE" }

// -- stash the rifle, take it back, the emptied holder survives ------------------
mkWpn = primaryWeapon aP
triAssert [mkWpn != ""]
aP setPos [(gmHqCachePos select 0) + 1, (gmHqCachePos select 1) + 1, 0]
triSimUntil { gmMktCacheActive }
triAssertGe [(gmMktCacheActs select 0), 0]
[aP, aP, gmMktCacheActs select 0] exec "\gmcore\scripts\market_action.sqs"
triSimUntil { not (aP hasWeapon mkWpn) }
triAssert [mkWpn in (weaponCargo gmHqCache)]
gmHqCache removeWeaponCargo mkWpn
aP addWeapon mkWpn
triAssert [aP hasWeapon mkWpn]
triAssert [not (mkWpn in (weaponCargo gmHqCache))]
triSimFrames 5
triAssert [not (isNull gmHqCache)]

// -- delivery to the HQ: weapon -> cache cargo, vehicle -> garage (locked) -------
[] exec "act_tp_arms.sqs"
triSimUntil { (count gmMktBuyActs) > 0 }
triAssertGe [gmMktDelivAct, 0]
[aP, aP, gmMktDelivAct] exec "\gmcore\scripts\market_action.sqs"
triSimUntil { gmMktDelivHq and gmMktBuyActive and (not gmMktBuyDirty) }
mkCargo0 = count (weaponCargo gmHqCache)
mkRow = (gmDealerStock "WEAPON") select 1
mkR0 = gmResources
[aP, aP, gmMktBuyActs select 1] exec "\gmcore\scripts\market_action.sqs"
triSimUntil { gmResources == (mkR0 - (mkRow select 2)) }
triAssertGe [(count (weaponCargo gmHqCache)), mkCargo0 + 1]

[] exec "act_tp_vehicle.sqs"
triSimUntil { ((count gmMktBuyActs) > 0) and (gmMktDealerKind == "VEHICLE") }
triAssert [gmMktDelivHq]
mkRow = (gmDealerStock "VEHICLE") select 0
mkR0 = gmResources
[aP, aP, gmMktBuyActs select 0] exec "\gmcore\scripts\market_action.sqs"
triSimUntil { gmResources == (mkR0 - (mkRow select 2)) }
triSimUntil { gmGarageCount == 1 }
mkVeh = gmGarageVehicle 0
triAssert [not (isNull mkVeh)]
triAssert [locked mkVeh]
triAssertLt [(mkVeh distance gmHqGaragePos), 30]
triAssert [gmGarageLock [mkVeh, false]]
triAssertEq [gmGarageCount, 0]
triAssert [not (locked mkVeh)]
triAssert [gmGarageLock [mkVeh, true]]
triAssertEq [gmGarageCount, 1]
triAssert [locked mkVeh]

// -- move the HQ to the Camp: outdoor fallback, debited, cache travels ----------
[] exec "act_tp_camp.sqs"
triSimUntil { gmMktHqActive and (gmHqCanEstablish (getPos aP)) }
mkR0 = gmResources
[aP, aP, gmMktHqAct] exec "\gmcore\scripts\market_action.sqs"
triSimUntil { gmHqMoveCount == 1 }
triAssertEq [gmResources, mkR0 - (gmMarketValue "hqMoveCost")]
triAssertEq [gmHqZone, "Camp"]
triAssert [not gmHqIndoors]
triSimFrames 3
triAssert [not (isNull gmHqCache)]
triAssertLt [(gmHqCache distance gmHqCachePos), 10]
triAssertGe [(count (weaponCargo gmHqCache)), 1]
// the hull stayed in the old town, far outside the relocated ring -> released
triSimUntil { gmGarageCount == 0 }
triAssert [not (locked mkVeh)]

triEndTest
