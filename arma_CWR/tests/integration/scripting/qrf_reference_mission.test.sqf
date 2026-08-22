// ============================================================================
//  Qrf.Abel reference mission - the alert -> QRF chain, end to end, on the
//  campaign's REAL policy script (scripts/qrf.sqs, byte-identical core copy)
//  driven by the native AlertMachine + GarrisonCache. Proves, on full CWA
//  data (Abel):
//      1. the mission boots its own bootstrap (QRF_BOOTED), the shared-core
//         subset handshakes (GM_LIB_READY) and qrf.sqs is running
//         (GM_QRF_ACTIVE defined, false);
//      2. the Outpost garrison is natively spawned at boot (player start is
//         inside the 800 m cacheRadius);
//      3. the mission's own debug action (act_reveal.sqs - `reveal` to every
//         garrison group every 5 s) drives the native FSM GREEN -> YELLOW
//         (knowsAbout >= 0.5) -> RED (the 20 s YELLOW window expiring);
//      4. RED makes qrf.sqs field a QRF on that zone: GM_QRF_ACTIVE, the
//         zone index matches, the convoy group holds >= 5 units (role
//         squad + officer) and a staging position was chosen;
//      5. the disengage half: with the only perception source gone (player
//         1 km away, garrisons force-despawned - fresh respawns know nothing)
//         the zone calms to GREEN and qrf.sqs stands the convoy down
//         (GM_QRF_ACTIVE false, the group gone).
//
//  Timing: alert tick 5 s, YELLOW window 20 s, qrf.sqs tick 5 s - the
//  reveal -> QRF leg completes in ~35 s; the calm-down waits on the next
//  alert tick after the knowers are gone. Generous windows in the .toml.
// ============================================================================

triSimUntil { not (isNil "QRF_BOOTED") }
triSimUntil { not (isNil "GM_LIB_READY") }
triSimUntil { GM_LIB_READY }
triSimUntil { not (isNil "GM_QRF_ACTIVE") }
triAssert [not GM_QRF_ACTIVE]

// -- zones + native garrison at boot ------------------------------------------
qrZO = gmZoneIndex "Outpost"
qrZD = gmZoneIndex "Depot"
triAssertGe [qrZO, 0]
triAssertGe [qrZD, 0]
triAssertEq [(gmZone qrZO) select GM_Z_OWNER, gmOccupierSide]
triSimUntil { (gmGarrisonSpawned qrZO) and ((gmGarrisonLive qrZO) > 0) }
triAssertEq [(gmZoneAlert qrZO), 0]

// -- the observer must survive the convoy's arrival for the whole run ---------
player allowDammage false

// -- the mission's debug reveal drives the real FSM ----------------------------
[] exec "act_reveal.sqs"
triSimUntil { QRF_REVEAL_ACTIVE }
triSimUntil { (gmZoneAlert qrZO) >= 1 }
triSimUntil { (gmZoneAlert qrZO) >= 2 }

// -- RED -> qrf.sqs fields ONE convoy on that zone ------------------------------
triSimUntil { GM_QRF_ACTIVE }
triAssertEq [GM_QRF_ZONE, qrZO]
triAssertGe [(count units GM_QRF_GROUP), 5]
triAssertEq [(count GM_QRF_OPOS), 3]
triAssertGt [(GM_QRF_OPOS distance ((gmZone qrZO) select GM_Z_POS)), 50]

// -- disengage: remove every perception source, expect calm + stand-down -------
triSimUntil { not QRF_REVEAL_ACTIVE }
player setPos [7205, 7300, 0]
gmGarrisonForceDespawn qrZO
gmGarrisonForceDespawn qrZD
triSimUntil { (gmZoneAlert qrZO) == 0 }
triSimUntil { not GM_QRF_ACTIVE }
triAssertEq [(count units GM_QRF_GROUP), 0]

triEndTest
