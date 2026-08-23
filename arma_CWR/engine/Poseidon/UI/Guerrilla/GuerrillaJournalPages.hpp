#pragma once

// Guerrilla Mode journal pages for the map screen's briefing notepad.
//
// DisplayMap::ReloadBriefingContent calls BuildGuerrillaJournalPages right
// after the mission's briefing.html (if any) is parsed, whenever the
// ZoneRegistry is active: at display construction (once per mission,
// DisplayMission::InitUI) and through DisplayMap::RefreshGuerrillaJournal on
// every map-key open (ResetHUD), on a journal revision change while the map
// is open (OnSimulate), on a Notes/Plan tab press and after an in-place load.  It emits into the existing CHTMLContainer
// document model (the same AddSection / AddText / AddImage calls the
// engine's own UpdatePlan / UpdateUnitsInBriefing use):
//
//   "Main"        the Notes page (aliased __BRIEFING by the caller): campaign
//                 header, the live Situation block, the field-manual index
//                 and the latest diary lines
//   "Plan"        copied into __PLAN by UpdatePlan: the standing goal, the
//                 scripted objectives and the engine-derived next steps
//   "GM_LOG"      the full diary, newest first
//   "GM_MAN_<n>"  one page per field-manual topic (static player text)
//
// The live facts come in through JournalPageInputs so the renderer itself is
// pure (unit-testable against a parser-only CHTMLContainer): the world-
// dependent half is GatherGuerrillaJournalInputs(), which reads the
// ZoneRegistry / AlertMachine / UndercoverSystem / StashRegistry plus the
// script-owned economy globals (gmResources, gmManpower, gmWarLevel,
// gmDayCount, gmUndercover).

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

class CHTMLContainer;

namespace Poseidon
{
namespace Guerrilla
{
class Journal;

struct JournalPageInputs
{
    RString islandName;
    RString resistanceName; // descriptor class name, else side string
    RString occupierName;
    RString stamp; // "Day N HH:MM" (empty: not shown)

    float resources = 0;
    float manpower = 0;
    int warLevel = 1;
    bool economyKnown = false; // false when the script globals are not up yet

    int militaryTotal = 0;         // non-CITY zones, the Camp excluded
    int militaryHeld = 0;          // ... owned by the resistance
    int townsTotal = 0;            // CITY zones
    int townsRisen = 0;            // ... owned by the resistance
    AutoArray<RString> townsReady; // support past the threshold, not yet risen
    AutoArray<RString> securing;   // "Outpost 45%" meters in progress
    AutoArray<RString> targets;    // occupier military zones, nearest first

    int alertMax = 0; // 0 GREEN / 1 YELLOW / 2 RED, the worst zone
    RString alertZone;
    float heatMax = 0;
    RString heatZone;

    bool undercoverArmed = false; // gmUndercover true
    int undercoverStatus = 0;     // 0 clean / 1 suspected / 2 compromised
    int undercoverWitnesses = 0;

    int cellSize = 0; // units in the player's group, player included
    int stashCount = 0;

    // headquarters (GuerrillaBase) + dealer market (Market)
    bool hqEstablished = false;
    RString hqZone;          // zone name while established
    bool hqIndoors = false;  // building HQ vs edge-of-town
    int garageCount = 0;     // vehicles locked in the garage
    bool marketActive = false;
    int weaponDealers = 0;
    int vehicleDealers = 0;

    // stock objective icons need a texture loader; off in unit tests
    bool useImages = false;
};

// true when the mission runs Guerrilla Mode (CfgGuerrillaZones present)
bool GuerrillaJournalActive();

// world-dependent half: collect the live facts (safe without a world)
JournalPageInputs GatherGuerrillaJournalInputs();

// pure half: emit the pages into the document model
void BuildGuerrillaJournalPages(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in);

// manual table (exposed for tests / docs): topic count and titles
int GuerrillaManualTopicCount();
const char* GuerrillaManualTopicTitle(int i);

} // namespace Guerrilla
} // namespace Poseidon
