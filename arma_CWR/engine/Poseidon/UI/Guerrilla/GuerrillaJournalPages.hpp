#pragma once

// Guerrilla Mode field journal (the Resistance Dossier) for the map screen's
// briefing notepad.
//
// DisplayMap::ReloadBriefingContent calls BuildGuerrillaJournalPages right
// after the mission's briefing.html (if any) is parsed, whenever the
// ZoneRegistry is active: at display construction (once per mission,
// DisplayMission::InitUI) and through DisplayMap::RefreshGuerrillaJournal on
// every map-key open (ResetHUD), on a journal revision change while the map
// is open (OnSimulate), on a Notes / Plan tab press and after an in-place
// load.
//
// The work is split in three stages:
//
//   Gather   GatherGuerrillaJournalInputs() (this file): the only stage that
//            reads the world.  It fills JournalPageInputs from the
//            ZoneRegistry / AlertMachine / UndercoverSystem / StashRegistry /
//            GuerrillaBase / Market, the player's group and the resistance
//            side's other groups, the script-owned economy globals
//            (gmResources, gmManpower, gmWarLevel, gmDayCount, gmUndercover,
//            gmEcoR / gmEcoHR, GM_MANPOWER_CAP, GM_ECON_TICK) and the engine's
//            UI aspect.  Safe without a world.
//   Compose  ComposeJournal() (JournalCompose.hpp): pure, from (Journal,
//            JournalPageInputs) to a JournalDocument of pages, blocks and
//            runs in named voices and inks.  Owns the content limits (five
//            list entries per page, 25-word hand remarks) and never measures
//            a pixel.
//   Render   RenderJournal() (JournalRender.hpp): binds the format slots and
//            lays the document into the CHTMLContainer document model
//            (AddText / AddBreak / AddImage with per-field ink, table cells
//            and hanging indents), so the pages are drawn by the stock
//            briefing control in the stock notepad look.  Owns the page
//            budget: a page whose blocks overrun the paper continues on
//            "<name>_2", "<name>_3" ... with prev / next links in the footer.
//
// Page map (section names; legacy anchors in brackets stay resolvable as
// aliases of the new page so old links and the tab wiring keep working):
//
//   "Main"          [GM_CONTENTS]   CONTENTS: the six sections below as
//                                   links with a one-line description
//                                   (aliased __BRIEFING by the caller, the
//                                   Notes tab)
//   "GM_DISPATCH"                   DISPATCHES: threat, top objective, the
//                                   latest development, the cover line
//   "Plan"          [GM_OPERATIONS] OPERATIONS hub (copied into __PLAN by
//                                   UpdatePlan): links to
//                                   "GM_OBJECTIVES", "GM_ACTIONS",
//                                   "GM_SUPPLY" and "GM_FACTION"
//   "GM_PEOPLE"     [GM_CELL]       PEOPLE index -> "GM_ROSTER" (the
//                                   fighters, arms); character dossiers
//                                   "GM_WHO_<id>" arrive with Change 2
//   "GM_PLACES"     [GM_ZONES]      PLACES index -> "GM_ZONE_<i>", one page
//                                   per zone (facts, the cell's presence,
//                                   the zone's latest lines)
//   "GM_CHRONICLES"                 CHRONICLES hub -> "GM_RECORD" [GM_LOG],
//                                   every entry in the hand, newest first
//   "GM_REFERENCE"  [GM_MAN_INDEX]  REFERENCE: the handbook chapters
//                                   "GM_MAN_<n>" (JournalManual.hpp)
//
// Typography (the slot table; P is the engine-shared typed body and is never
// resized or rebound): H3 = title face, "garamond" at 1.45 x P; H4 = head
// voice, "couriernewb" at 1.15 x P; H5 = narrative serif, "garamond" at
// 1.1 x P; H6 = the hand, "cwrpen" at 1.6 x P; P = typed body and small type
// (pencil ink), also the blank spacer.  H1 / H2 are not bound and not used.
// The journal never touches the control's colours; it inks fields
// (blue-black hand, red pen, pencil, faded hand).
//
// Every page ends in the same typed footer pinned to the page foot:
// Contents, the parent section, prev / next when the page continues.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/UI/Guerrilla/JournalManual.hpp> // GuerrillaManualTopic* (re-exported for tests / docs)

class CHTMLContainer;

namespace Poseidon
{
namespace Guerrilla
{
class Journal;

// one zone ledger row (every zone, revealed or not)
struct JournalZoneRow
{
    RString name;
    RString type;   // "CAMP" | "AIRFIELD" | "SEAPORT" | "OUTPOST" | "CITY"
    int holder = 2; // 0 ours / 1 occupier / 2 neutral or other
    bool revealed = false;
    float support = 0;   // CITY: 0..100
    float capture = 0;   // military: 0..100 (0 = no capture in progress)
    float heat = 0;      // 0..100
    int garrison = 0;    // known occupier headcount (despawned count)
    int alert = 0;       // 0 GREEN / 1 YELLOW / 2 RED
    float distance = -1; // m from the player, -1 unknown
    RString bearing;     // "NE" (empty when distance unknown)
    int seenDay = 0;     // last inside the player's bubble; 0 = never
    int seenMinute = 0;
};

// one roster row (the player's group, one row per fighter; other resistance
// groups collapse to one row per group with count > 1)
struct JournalRosterRow
{
    RString name;
    int rank = 0;     // Rank enum index (0 private .. 6 colonel), -1 unknown
    RString role;     // type display name ("Rifleman"); "Leader" for the player
    RString primary;  // primary weapon display name
    RString launcher; // secondary (launcher) display name, "" none
    int wounded = 0;  // 0..100 % damage taken
    bool withPlayer = true;
    RString zone;  // nearest zone for a holding group
    int count = 1; // fighters this row stands for
    bool isPlayer = false;
};

// faction-management stubs (RESISTANCE page).  Nothing native fills these
// yet: they read optional script globals (gmDoctrine, gmOutsideSupport,
// gmAlliedCells) so a future faction manager can publish through them
// without an engine change; empty / zero = not shown.
struct JournalFactionStubs
{
    RString doctrine;       // gmDoctrine
    RString outsideSupport; // gmOutsideSupport
    int alliedCells = 0;    // gmAlliedCells
};

// one named character for the People pages (companions, enemy legends).
// Change 2 fills these from the LegendRegistry; declared now so Compose is
// written once against the final struct (design D1 Gather)
struct JournalCharacterView
{
    RString id;          // stable "comp:<i>:<slug>" / "boss:<n>"
    RString displayName; // five-slot name as bound on the body
    RString baseName;    // "Petra"
    int kind = 0;        // 0 companion, 1 boss
    RString role;        // "Rifleman" / "Sniper" ...
    RString status;      // "with me" / "holding Outpost" / "Defeated" ...
    bool alive = true;
    int rank = -1; // ladder index, -1 unknown
    bool legend = false;
    RString bio;         // 35-45 words: the dossier page's budget (Change 2)
    RString deedLatest;  // one notable deed, <= 25 words
    RString portraitKey; // lower(bodyClass) + "__" + lower(face)
    bool portraitPresent = false;
    RString zone;
    bool defeated = false;
};

// the generated faction history for the Chronicles (Change 2)
struct JournalHistoryView
{
    bool present = false;
    RString opening1, opening2;
    struct Event
    {
        RString title, text, place;
    } events[3];
};

struct JournalPageInputs
{
    RString islandName;
    RString resistanceName; // descriptor displayName, else class name, else side string
    RString occupierName;
    int day = 0; // campaign day (1-based); 0 = no clock
    int minuteOfDay = 0;

    float resources = 0;
    float manpower = 0;
    float manpowerCap = 0; // GM_MANPOWER_CAP (0: unknown)
    float incomeR = 0;     // last economy tick (gmEcoR / gmEcoHR)
    float incomeHR = 0;
    float econTickSeconds = 0; // GM_ECON_TICK (0: unknown)
    bool incomeKnown = false;
    int warLevel = 1;
    int warLevelMax = 10;                    // escalation ladder (gmWarLevel runs 1..10)
    bool economyKnown = false;               // false when the script globals are not up yet
    AutoArray<float> occupierTierThresholds; // war levels where the occupier fields the next tier

    int militaryTotal = 0;  // non-CITY zones, the Camp excluded
    int militaryHeld = 0;   // ... owned by the resistance
    int townsTotal = 0;     // CITY zones
    int townsRisen = 0;     // ... owned by the resistance
    float supportFlip = 60; // town rise threshold

    bool undercoverArmed = false; // gmUndercover true
    int undercoverStatus = 0;     // 0 clean / 1 suspected / 2 compromised
    int undercoverWitnesses = 0;

    int stashCount = 0;

    // headquarters (GuerrillaBase) + dealer market (Market)
    bool hqEstablished = false;
    RString hqZone;            // zone name while established
    bool hqIndoors = false;    // building HQ vs edge-of-town
    int garageCount = 0;       // vehicles locked in the garage
    AutoArray<RString> garage; // their display names (may be shorter than garageCount)
    bool marketActive = false;
    AutoArray<RString> weaponDealerTowns;
    AutoArray<RString> vehicleDealerTowns;

    // zone ledger, registry order
    AutoArray<JournalZoneRow> zones;
    // roster: player's group first, then holding groups
    AutoArray<JournalRosterRow> roster;

    JournalFactionStubs faction;

    RString resistanceFactionClass;             // ZoneRegistry::ResistanceFaction() (class, not display name)
    RString portraitDir;                        // "gmcore\\portraits" in game; "" in tests
    float uiAspect = 4.0f / 3.0f;               // GEngine->Width2D() / Height2D(); 4:3 without an engine
    AutoArray<JournalCharacterView> characters; // empty in Change 1
    JournalHistoryView history;                 // present == false in Change 1
};

// true when the mission runs Guerrilla Mode (CfgGuerrillaZones present)
bool GuerrillaJournalActive();

// world-dependent half: collect the live facts (safe without a world)
JournalPageInputs GatherGuerrillaJournalInputs();

// entry point: ComposeJournal(journal, in) -> RenderJournal(html, doc, in); pure, null-safe on html
void BuildGuerrillaJournalPages(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in);

} // namespace Guerrilla
} // namespace Poseidon
