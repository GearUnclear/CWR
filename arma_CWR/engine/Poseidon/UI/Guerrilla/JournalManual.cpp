#include <Poseidon/UI/Guerrilla/JournalManual.hpp>

namespace Poseidon::Guerrilla
{

namespace
{

// ===========================================================================
// handbook - static in-universe text, typed.  Mini markup per line:
//   "#Heading"        section head        "!A|B|C"   table header
//   "|a|b|c"          table row; a cell starting with ~r is in red ink, ~w
//                     is bold type, ~g / ~y are plain type (kept so the
//                     text reads the same on paper as it did on the mock)
//   "- text"          bullet              "@standing" the live cover row
//   anything else     paragraph
//
// The table came over from GuerrillaJournalPages.cpp; the prose that named
// the retired Notes / Plan / Zones / Cell / Resistance / Diary pages was
// rewritten for the dossier's page map (Contents, Dispatches, Operations,
// People, Places, Chronicles, Reference), which test_journal_compose.cpp
// pins against the Contents menu.
// ===========================================================================

const ManualTopic kManual[] = {
    {"GM_MAN_MODE",
     "The campaign",
     "what this is",
     {"One fighter, one camp, an occupied island. There is no script to follow: take the island zone by zone, "
      "build a cell, and hold what you take against an occupier that grows stronger as you do.",
      "This dossier is written as the campaign runs. Dispatches is the day's page; Operations carries the "
      "objectives and the next moves; People and Places are the ledgers; Chronicles is the whole record; "
      "Reference is this handbook.",
      "Open it whenever you like. It is rewritten every time, so the figures are current.", "#Pages",
      "|~wCONTENTS|the six sections, one line each",
      "|~wDISPATCHES|the day's page: the threat, the top objective, the latest development, your cover",
      "|~wOPERATIONS|objectives with progress, suggested moves, supplies, resistance strength",
      "|~wPEOPLE|the roster and the named", "|~wPLACES|towns, bases, the headquarters, a page for each",
      "|~wCHRONICLES|the record of the campaign, newest first", "|~wREFERENCE|these handbook topics", nullptr}},
    {"GM_MAN_ZONES",
     "Zones",
     "camp, bases, towns",
     {"Every place that matters is a zone with a flag on the map. Green is ours, red the occupier, yellow "
      "neutral, white contested.",
      "!TYPE|WHAT IT IS|HOW IT IS WON", "|~wCAMP|yours from the start; recruit, train and keep the record here|keep it",
      "|~wBASE|outpost, airfield or port with an occupier garrison|clear the garrison, hold the ground",
      "|~wTOWN|civilians with a support figure 0 to 100|support past 60, then fighters in the town",
      "A held base pays income and gets a holding squad. A risen town pays too and counts toward the war.",
      "Zones show on Places once they are within reach of ground you hold. Their meters only move while "
      "you are near (about 800 m), which is what last seen records.",
      nullptr}},
    {"GM_MAN_CAPTURE",
     "Taking a base",
     "the capture meter",
     {"A base carries a capture meter from 0 to 100. It climbs while your fighters stand inside the zone and no "
      "live occupier is inside it. More fighters climb it faster, up to a small crew.",
      "!IF|THEN", "|~yoccupier inside|the meter is contested and freezes; kill or drive them out",
      "|~yyou leave|the meter fades; alone, the defenders drive it down fast",
      "|~gmeter at 100|the base flips: income opens, a holding squad forms, regional heat spikes",
      "A patrol or a QRF that walks in contests the base like a garrison does. A capture is never safe until it "
      "is done.",
      nullptr}},
    {"GM_MAN_TOWNS",
     "Towns and support",
     "how a town rises",
     {"Each town has a support figure. It rises while your fighters stand in the town with no occupier "
      "present, and bleeds while only occupier troops are present, never below a floor.",
      "!SUPPORT|STATE|MEANS", "|~wunder 60|~wNEUTRAL|the town is being worked",
      "|~w60 and up|~gRISING|ready: fighters in the town while it is free of occupier troops and it flips",
      "|~wrisen|~gHELD|pays income, raises the war level, can be retaken if left exposed",
      "A disguised, undercover fighter counts for neither side. Bring fighters if you want a town to move.", nullptr}},
    {"GM_MAN_WAR",
     "War level, heat, alert",
     "what the occupier does back",
     {"The war level follows the share of the island you hold: 20, 40, 55, 70 and 85 percent step it up. Higher "
      "levels field better troops, heavier vehicles and sharper eyes for disguises.",
      "Heat is per zone, 0 to 100. Captures, blown cover and fights raise it; it decays while the zone is quiet. "
      "Past 30 the garrison is on edge, past 50 expect a sweep.",
      "!ALERT|MEANS|DO", "|~gGREEN|garrison calm|work", "|~yYELLOW|aware: they check your last known position|move",
      "|~rRED|combat: a quick reaction force is out toward your last known position|break contact, stay unseen",
      "Alert does not calm while they can see you.", nullptr}},
    {"GM_MAN_CELL",
     "The cell",
     "resources, manpower, recruiting",
     {"The cell lives on resources (R) and manpower (HR). Both come in from the zones you hold: bases and risen "
      "towns pay, the Camp does not. A panicked town pays less.",
      "!AT THE CAMP|COSTS|GIVES", "|~wRecruit fighter|1 HR|a rifleman in your group with the best pattern in issue",
      "|~wRecruit specialist|R|a medic, gunner or anti-tank man",
      "|~wTrain squad|R|skill for the whole group, up to a cap that grows with the war level",
      "Manpower is bodies: one HR is one recruit, and the pool has a ceiling. Resources buy specialists and "
      "training; they do nothing in the treasury.",
      nullptr}},
    {"GM_MAN_LOOT",
     "Arms and caches",
     "patterns, standard issue, the cache",
     {"Every occupier you kill drops what he carried into the cell's pool. Capture enough of one pattern and it "
      "becomes standard issue: recruits and companions can be kitted with it freely. The roster tracks the "
      "next pattern.",
      "The headquarters holds a cache and a garage. Weapons stored in the cache and vehicles locked in the "
      "garage stay where you left them, across saves and across an HQ move.",
      "Arms dealers and vehicle dealers trade in some towns for R; a dealer in an occupied town is a risk.", nullptr}},
    {"GM_MAN_COMPANIONS",
     "Companions",
     "named fighters",
     {"Named companions fight in your group, gain standing from kills and survival, and are promoted through the "
      "ranks. Each promotion raises their skill; the roster shows the progress to the next rank.",
      "Death is permanent. A fallen companion is gone for good and the record keeps the line.",
      "Companions are rebuilt beside you after a load from their saved record: rank, standing, kit.", nullptr}},
    {"GM_MAN_UNDERCOVER",
     "Undercover",
     "how the occupier reads you",
     {"To occupier eyes you begin as a civilian. Each occupier group judges you separately, from what it can see.",
      "!YOU ARE|SEEN AS|RANGE", "|~wUnarmed|~gCIVILIAN|any", "|~wRifle slung|~ySUSPECTED|under 20 m, or from behind",
      "|~wWeapon in hand|~ySUSPECTED, then BLOWN|any, with line of sight", "|~wFiring|~rBLOWN|every group in view",
      "@standing", "|~gCLEAN|no group has identified you", "|~ySUSPECTED|a group is checking you",
      "|~rBLOWN|identified; the journal counts the patrols who know you", "#Cover returns when",
      "- You walk away from every group that identified you.", "- The weapon is stowed and no witness is left.",
      "#Cover also fails when", "- A stolen occupier vehicle is seen close up.",
      "- A getaway is witnessed: that car is marked for those witnesses.", nullptr}},
    {"GM_MAN_SAVE",
     "Keeping the record",
     "saving and the record",
     {"Save whenever you like: the whole campaign is kept. Zones, garrisons, alert, the cell, companions, "
      "patterns, caches, this dossier.",
      "The record under Chronicles is the running account of the campaign: captures, risings, quick reaction "
      "forces, blown cover, promotions, losses, patterns, saves and restores. It survives saving and loading.",
      "After a restore the record notes it and everything resumes where it was.", nullptr}},
};

constexpr int kManualCount = sizeof(kManual) / sizeof(kManual[0]);

} // namespace

// ===========================================================================
// public surface
// ===========================================================================

int GuerrillaManualTopicCount()
{
    return kManualCount;
}

const char* GuerrillaManualTopicTitle(int i)
{
    if (i < 0 || i >= kManualCount)
    {
        return "";
    }
    return kManual[i].title;
}

const char* GuerrillaManualTopicAnchor(int i)
{
    if (i < 0 || i >= kManualCount)
    {
        return "";
    }
    return kManual[i].anchor;
}

const ManualTopic& GuerrillaManualTopic(int i)
{
    if (i < 0)
    {
        i = 0;
    }
    if (i >= kManualCount)
    {
        i = kManualCount - 1;
    }
    return kManual[i];
}

} // namespace Poseidon::Guerrilla
