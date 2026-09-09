// ============================================================================
//  Guerrilla Legends on a live campaign (guerrilla_native.abel).
//    The native LegendRegistry (Game/Guerrilla/LegendRegistry) seeds the
//    campaign on its first active Simulate tick: it draws the one campaign
//    seed, generates the faction history and pre-rolls the three enemy
//    Legends. From then on it OBSERVES the GM_COMP_* arrays once a second and
//    owns the canonical display name, which companions.sqs stamps onto the
//    body through gmLegendBind at spawn.
//    The unit suite drives the registry with handed-in observations and no
//    world; this lane is the half that needs the real thing:
//      * the seed is drawn and the history generated exactly once, in game;
//      * the rolled face token actually resolves against Classic's CfgFaces
//        (the four kPortraitFaces tokens are a design guess until this runs);
//      * ONE write of the display name really does make the roster, the HUD
//        and the cursor label agree (they all read Person::GetInfo()._name);
//      * an award fires once per threshold against the live 1 Hz poll, and
//        the second award ADDS a slot rather than replacing the first.
//    No literal seed is pinned anywhere: the draw depends on how many other
//    systems drew from GRandGen that frame. What is pinned is reproducibility
//    and the invariants that hold for every seed.
// ============================================================================

triSimUntil { GM_LIB_READY }

// -- the seeding tick: history + the three enemy Legends exist before any
//    companion has been polled ------------------------------------------------
triSimUntil { gmLegendCount >= 3 }
gmLegHist = gmLegendHistory
triAssertEq [(gmLegHist select 0), 1]
triAssertNe [(gmLegHist select 1), 0]
triAssertNe [(gmLegHist select 2), ""]
triAssertNe [(gmLegHist select 3), ""]
gmLegOpen1 = gmLegHist select 2
gmLegOpen2 = gmLegHist select 3
gmLegSeed = gmLegHist select 1
gmLegEv0 = gmLegendHistoryEvent 0
triAssertEq [(count gmLegEv0), 3]
triAssertNe [(gmLegEv0 select 0), ""]
triAssertNe [(gmLegEv0 select 1), ""]
triAssertNe [(gmLegEv0 select 2), ""]
gmLegEv2 = gmLegendHistoryEvent 2
triAssertEq [(count gmLegEv2), 3]
triAssertNe [(gmLegEv2 select 1), ""]
// out of range is an empty array, never a fabricated beat
triAssertEq [(count (gmLegendHistoryEvent 3)), 0]

// -- the companion body. GM_fnCompSpawn calls gmLegendBind after createUnit
//    returns, so the row exists and the body wears the Legend name from the
//    frame it exists rather than from the next poll -----------------------------
triSimUntil { not (isNull (GM_COMP_OBJ select 0)) }
triSimUntil { (gmLegendId 0) != "" }
gmLegBase = GM_COMP_NAMES select 0
triAssertEq [gmLegBase, "Petra"]
gmLegPetra = gmLegendName 0
triAssertNe [gmLegPetra, ""]
triAssertNe [gmLegPetra, gmLegBase]
triAssertIncludes [gmLegPetra, gmLegBase]
// before any award the display name is exactly "<base> <last>", so the tail is
// the persisted surname: the token both awards below have to leave standing
gmLegSur = substr [gmLegPetra, (sizeofstr gmLegBase) + 1, sizeofstr gmLegPetra]
triAssertNe [gmLegSur, ""]
gmLegId = gmLegendId 0
triAssertNe [gmLegId, ""]

// -- the face. THIS is the confirmation that the four kPortraitFaces tokens
//    exist in Classic's CfgFaces: the binder validates the rolled token and
//    degrades to "Default" when the package refuses it ------------------------
gmLegFace = gmLegendFace 0
triAssertNe [gmLegFace, ""]
triAssertNe [gmLegFace, "Default"]
triAssert [((["Face10", "Face18", "Face27", "Face33"] find gmLegFace) >= 0)]

// -- one write, so the HUD, the cursor label, the briefing roster and the
//    journal roster all agree (every one of them reads _name) ------------------
triAssertEq [(name (GM_COMP_OBJ select 0)), gmLegPetra]

// -- gmLegendInfo 0 is the first COMPANION for the life of the campaign: rows
//    are kept companions-first --------------------------------------------------
gmLegInfo = gmLegendInfo 0
triAssertEq [(count gmLegInfo), 8]
triAssertEq [(gmLegInfo select 0), gmLegId]
triAssertEq [(gmLegInfo select 1), gmLegPetra]
triAssertEq [(gmLegInfo select 2), 0]
triAssert [(gmLegInfo select 5)]
triAssert [not (gmLegInfo select 6)]
triAssertEq [(gmLegInfo select 7), 0]
triAssertEq [(count (gmLegendInfo 999)), 0]

// -- the enemy Legends carry real identities from campaign start, all three
//    latched so the companion award machinery can never fire on them ----------
triAssertGe [gmLegendCount, 4]
gmLegBossN = 0
gmLegI = 0
while {gmLegI < gmLegendCount} do {gmLegRow = gmLegendInfo gmLegI; if ((gmLegRow select 2) == 1) then {triAssertNe [(gmLegRow select 1), ""]; triAssert [(gmLegRow select 6)]; triAssertEq [(gmLegRow select 7), 3]; gmLegBossN = gmLegBossN + 1}; gmLegI = gmLegI + 1}
triAssertEq [gmLegBossN, 3]

// -- FIRST AWARD at SERGEANT (ladder index 2, XP 250). The live 1 Hz poll
//    sees the ladder step, awards one slot and writes D2.5's two lines --------
gmLegJ0 = gmJournalCount
GM_COMP_XP set [0, 250]
triSimUntil { (gmLegendName 0) != gmLegPetra }
gmLegAward1 = gmLegendName 0
triSimFrames 200
// the latch: a threshold crossed once stays crossed, however many polls follow
triAssertEq [(gmLegendName 0), gmLegAward1]
triAssertEq [((gmLegendInfo 0) select 7), 1]
triAssert [not ((gmLegendInfo 0) select 6)]
triAssertIncludes [gmLegAward1, gmLegBase]
triAssertIncludes [gmLegAward1, gmLegSur]
triAssertEq [(name (GM_COMP_OBJ select 0)), gmLegAward1]

// the diary lines the award owes, each attributed to this character's id
gmLegNew = ""
gmLegAttr = 0
gmLegI = gmLegJ0
while {gmLegI < gmJournalCount} do {gmLegNew = gmLegNew + " " + ((gmJournalEntry gmLegI) select 1); if ((gmJournalEntryChar gmLegI) == gmLegId) then {gmLegAttr = gmLegAttr + 1}; gmLegI = gmLegI + 1}
triAssertGe [gmLegAttr, 2]
triAssertIncludes [gmLegNew, "is now known as"]
triAssertIncludes [gmLegNew, "promoted to SERGEANT"]

// -- SECOND AWARD at COLONEL (ladder index 6, XP 1900). The second slot is
//    ADDED, never swapped: the substring relation between the two display
//    names does not survive a middle insertion, so the durable invariants are
//    the mask, the Legend flag and the two surviving tokens -------------------
gmLegJ1 = gmJournalCount
GM_COMP_XP set [0, 1900]
triSimUntil { (gmLegendName 0) != gmLegAward1 }
gmLegAward2 = gmLegendName 0
triSimFrames 200
triAssertEq [(gmLegendName 0), gmLegAward2]
triAssertNe [gmLegAward2, gmLegAward1]
triAssertEq [((gmLegendInfo 0) select 7), 3]
triAssert [((gmLegendInfo 0) select 6)]
triAssertIncludes [gmLegAward2, gmLegBase]
triAssertIncludes [gmLegAward2, gmLegSur]
triAssertEq [(name (GM_COMP_OBJ select 0)), gmLegAward2]
gmLegNew2 = ""
gmLegI = gmLegJ1
while {gmLegI < gmJournalCount} do {gmLegNew2 = gmLegNew2 + " " + ((gmJournalEntry gmLegI) select 1); gmLegI = gmLegI + 1}
triAssertIncludes [gmLegNew2, "become a legend of the resistance"]

// -- nothing rerolls. The history is generated once, in the seeding tick, and
//    persisted as resolved prose ------------------------------------------------
triSimFrames 300
gmLegHist2 = gmLegendHistory
triAssertEq [(gmLegHist2 select 1), gmLegSeed]
triAssertEq [(gmLegHist2 select 2), gmLegOpen1]
triAssertEq [(gmLegHist2 select 3), gmLegOpen2]
triAssertEq [((gmLegendHistoryEvent 0) select 1), (gmLegEv0 select 1)]
// and no further award can fire past the top of the ladder
triAssertEq [(gmLegendName 0), gmLegAward2]
triAssertEq [((gmLegendInfo 0) select 7), 3]
triAssertEq [gmLegFace, (gmLegendFace 0)]
triAssertEq [gmLegId, (gmLegendId 0)]

triEndTest
