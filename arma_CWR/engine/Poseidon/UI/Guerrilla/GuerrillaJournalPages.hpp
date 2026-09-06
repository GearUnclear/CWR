#pragma once

// Guerrilla Mode field journal pages for the map screen's briefing notepad.
//
// DisplayMap::ReloadBriefingContent calls BuildGuerrillaJournalPages right
// after the mission's briefing.html (if any) is parsed, whenever the
// ZoneRegistry is active: at display construction (once per mission,
// DisplayMission::InitUI) and through DisplayMap::RefreshGuerrillaJournal on
// every map-key open (ResetHUD), on a journal revision change while the map
// is open (OnSimulate), on a Notes / Plan tab press and after an in-place
// load.  The renderer emits into the CHTMLContainer document model (AddText /
// AddBreak with table cells, per-field ink and hanging indents), so the
// pages are drawn by the stock briefing control in the stock notepad look:
// the control's own format slots (Courier type in H1-H4 and P, Garamond in
// H5, the handwriting face in H6), black type and stock links on the paper
// of the notepad model.  The journal never touches the control's colours;
// it inks fields (blue-black hand, red pen, pencil) and binds only the H6
// slot to "cwrpen", the handwriting face with a heavier stroke, at 1.6x
// the typed body size.
//
//   "Main"           NOTES (aliased __BRIEFING by the caller, the Notes
//                    tab): the day's page in the hand - the threat, the
//                    cell, the ground held, the latest diary lines
//   "Plan"           PLAN (copied into __PLAN by UpdatePlan): objectives,
//                    the done list, the next moves in priority order
//   "GM_ZONES"       ZONES: a typed index (state group, brief, range)
//   "GM_ZONE_<i>"    one page per zone: typed facts, what the cell has
//                    there, the zone's latest diary lines (the rest in the
//                    Diary)
//   "GM_CELL"        CELL: roster, arms, supply
//   "GM_FACTION"     RESISTANCE: war-level ladder, ground, organisation (the
//                    faction-management stubs live here)
//   "GM_LOG"         DIARY: every entry in the hand, newest first, by day
//   "GM_MAN_INDEX"   HANDBOOK index; "GM_MAN_<n>" one typed page per chapter
//
// Every page ends in the same typed footer (the journal's pages as links,
// pinned to the page foot).  The live facts come in through
// JournalPageInputs so the renderer itself is pure (unit-testable against a
// parser-only CHTMLContainer); the world-dependent half is
// GatherGuerrillaJournalInputs(), which reads the ZoneRegistry / AlertMachine
// / UndercoverSystem / StashRegistry / GuerrillaBase / Market, the player's
// group and the resistance side's other groups, plus the script-owned
// economy globals (gmResources, gmManpower, gmWarLevel, gmDayCount,
// gmUndercover, gmEcoR / gmEcoHR, GM_MANPOWER_CAP, GM_ECON_TICK).

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

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
};

// true when the mission runs Guerrilla Mode (CfgGuerrillaZones present)
bool GuerrillaJournalActive();

// world-dependent half: collect the live facts (safe without a world)
JournalPageInputs GatherGuerrillaJournalInputs();

// pure half: emit the pages into the document model
void BuildGuerrillaJournalPages(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in);

// handbook table (exposed for tests / docs): chapter count, titles, anchors
int GuerrillaManualTopicCount();
const char* GuerrillaManualTopicTitle(int i);
const char* GuerrillaManualTopicAnchor(int i);

} // namespace Guerrilla
} // namespace Poseidon
