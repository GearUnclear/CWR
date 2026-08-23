#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp>

#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>
#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp>
#include <Poseidon/Game/Guerrilla/Journal.hpp>
#include <Poseidon/Game/Guerrilla/Market.hpp>
#include <Poseidon/Game/Guerrilla/StashRegistry.hpp>
#include <Poseidon/Game/Guerrilla/Undercover.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/UI/Controls/UIControlsBase.hpp> // CHTMLContainer

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/AI/AI.hpp>    // AIUnit / AIGroup
#include <Evaluator/express.hpp> // GameState / GameValue (VarGet)

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/platform.hpp>

#include <cstdio>
#include <cstring>

namespace Poseidon::Guerrilla
{

// ===========================================================================
// field manual - static player text.  Plain sentences, no markup: the HTML
// model takes text fields directly.  Keep paragraphs short; the notepad
// page is narrow and long sections auto-paginate.
// ===========================================================================

namespace
{

struct ManualTopic
{
    const char* anchor;
    const char* title;
    const char* paragraphs[8]; // null-terminated
};

const ManualTopic kManual[] = {
    {"GM_MAN_MODE",
     "What this is",
     {"Uslu dur! Guerrilla Mode is an open-ended insurgency. You are one resistance fighter on an occupied "
      "island. There is no mission script to follow: liberate the island zone by zone, build a cell, and hold "
      "what you take against an occupier that grows stronger as you do.",
      "Everything you see on this notepad is written as you play. Notes shows the campaign situation, the "
      "manual and the latest diary lines. Plan shows the standing goal, your objectives and suggested next "
      "steps. Gear and Group work as in any mission.",
      "Open the map at any time; the pages are rebuilt every time you open it, so the numbers are current.",
      "Time passes and the day counter climbs. Nothing is lost between sessions as long as you save (see "
      "Saving).",
      nullptr}},
    {"GM_MAN_ZONES",
     "Zones: the three kinds",
     {"Every place that matters is a zone with a flag marker on the map. Green means yours, red means the "
      "occupier, yellow means neutral, white means contested right now.",
      "The Camp is yours from the start. Recruiting, training and the Save action live there. Keep it.",
      "Military zones (outposts, airfields, seaports) are garrisoned by the occupier. They are taken by "
      "force: clear the garrison and hold the ground until the capture meter fills. A captured military zone "
      "pays income and gets a friendly holding squad.",
      "Towns are won by support, not by force. The townspeople rise when support is high enough and your "
      "fighters are present without occupier troops around. Towns pay income too and count toward the war.",
      "Markers are revealed as you move within sight of them. The label on a marker shows the capture or "
      "support progress.",
      nullptr}},
    {"GM_MAN_CAPTURE",
     "Capturing a military zone",
     {"A military zone carries a capture meter from 0 to 100. It climbs while your fighters stand inside the "
      "zone and no live occupier soldier is inside it. More fighters climb it faster, up to a small crew "
      "cap.",
      "If an occupier soldier is inside the zone the meter is contested and freezes. Kill or drive them out "
      "to resume. If you leave, the meter fades; if the defenders hold it alone, it drops quickly.",
      "At 100 the zone flips: the marker turns green, income opens, a holding squad of your faction spawns "
      "there, and regional Heat spikes.",
      "Meters only move while you are within about 800 m of the zone. Nothing is captured while you are "
      "away, and nothing is lost either.",
      "A patrol or a QRF that walks into the zone contests it like a garrison does: a capture is never safe "
      "until it is done.",
      nullptr}},
    {"GM_MAN_TOWNS",
     "Towns and support",
     {"Each town has a support value from 0 to 100. It rises while your fighters stand in the town with no "
      "occupier troops present, and it bleeds away while only occupier troops are present (never below a "
      "floor).",
      "When support crosses the threshold the town is READY TO RISE: the diary and a hint say so. The rise "
      "itself needs your fighters in the town while it is free of occupier troops. Then the town flips to "
      "your side and raises your flag.",
      "A disguised, undercover player counts for neither side. Bring fighters if you want a town to move.",
      "Risen towns pay income, raise the War Level and can be taken back by the occupier if you leave them "
      "exposed. Occupied towns can still be organised underground and won later.",
      nullptr}},
    {"GM_MAN_WAR",
     "War Level, Heat and alert",
     {"War Level runs from 1 upward and follows the share of the island you hold. Higher levels field better "
      "occupier troops, heavier vehicles and sharper eyes for disguises. It is the price of progress.",
      "Heat is per zone. Your actions (captures, compromises, fights) raise it; it decays slowly while the "
      "zone is quiet. High Heat means the garrison is on edge.",
      "Each garrison runs an alert state: GREEN (safe), YELLOW (aware: they investigate your last known "
      "position) and RED (combat: a quick reaction force is dispatched toward you). Break contact and stay "
      "out of sight to let it calm down; it does not calm while they can see you.",
      "The Situation block on Notes shows the worst alert and the hottest zone at a glance.", nullptr}},
    {"GM_MAN_CELL",
     "Resources, manpower and recruiting",
     {"The cell lives on Resources (R) and Manpower (HR). Both tick in from the zones you hold: military "
      "zones and risen towns pay, the Camp does not. A panicked town pays less.",
      "At the Camp the action menu offers Recruit Fighter, Recruit Specialist and Train Squad; each shows "
      "its price. Recruits join your group (12 per group; overflow forms a new group) and arrive with the "
      "best gear you have unlocked for their role.",
      "Manpower is bodies: one HR is one recruit. Resources buy specialists and training. Spend them; they "
      "do nothing in the treasury.",
      "Training raises the squad's skill up to a cap that grows with the War Level.", nullptr}},
    {"GM_MAN_LOOT",
     "Loot, unlocks and stashes",
     {"Every occupier soldier you kill drops what he carried into the cell's pool. Collect enough of one "
      "weapon or magazine and it is UNLOCKED: from then on recruits and companions can be kitted with it "
      "freely.",
      "The diary records each unlock; the Situation block lists what is unlocked so far.",
      "Weapon holders you register as stashes are kept even when emptied, so an arms cache you build stays "
      "where you left it across saves. The count of stashes shows on Notes.",
      nullptr}},
    {"GM_MAN_COMPANIONS",
     "Companions",
     {"Named companions fight in your group, gain experience from kills and survival, and are promoted "
      "through the ranks. Each promotion raises their skill and shows live in the Group page.",
      "Death is permanent. A fallen companion is gone for good and the diary records it. Keep them alive.",
      "Companions are rebuilt beside you after a load from their saved record (rank, experience, kit).", nullptr}},
    {"GM_MAN_UNDERCOVER",
     "Undercover",
     {"You begin undercover: to occupier eyes you are a civilian. Cover is judged by each occupier group "
      "separately, from what they can actually see.",
      "Unarmed you read as a civilian. A rifle slung on your back gives you away close up or from behind. A "
      "weapon in your hands makes you suspect at once and identified soon after. Firing a shot blows your "
      "cover for every group that can see you.",
      "A group that has identified you remembers it. Groups that never saw it still take you for a "
      "civilian, so cover is re-established by walking away, stowing the weapon, or leaving no witnesses. "
      "Nobody calls it in for you: the Situation block shows clean, suspected or blown and how many witness "
      "groups know your face.",
      "Vehicles: a civilian car is anonymous. A stolen occupier vehicle passes at range and is recognised "
      "close up; a witnessed getaway marks that car for those witnesses.",
      nullptr}},
    {"GM_MAN_SAVE",
     "Saving and the diary",
     {"Save Game is always in your action menu. The whole campaign is saved: zones, garrisons, alert "
      "states, your cell, companions, unlocks, stashes and this journal.",
      "The diary on this notepad is the running record of the campaign: captures, risings, quick reaction "
      "forces, blown cover, promotions, losses, unlocks, saves and loads. It survives saving and loading.",
      "Load from the Esc menu as usual. After a load the diary notes the restore and everything resumes "
      "where it was.",
      nullptr}},
};

constexpr int kManualCount = sizeof(kManual) / sizeof(kManual[0]);

// ---------------------------------------------------------------------------
// small emitters over the document model
// ---------------------------------------------------------------------------

void Heading(CHTMLContainer* html, int section, const char* text, HTMLFormat format)
{
    html->AddText(section, RString(text), format, HALeft, false, false, RString(""));
    html->AddBreak(section, false);
}

void Para(CHTMLContainer* html, int section, const char* text)
{
    html->AddText(section, RString(text), HFP, HALeft, false, false, RString(""));
    html->AddBreak(section, false);
}

void Line(CHTMLContainer* html, int section, const char* label, const char* value)
{
    html->AddText(section, RString(label) + RString(": "), HFP, HALeft, false, true, RString(""));
    html->AddText(section, RString(value), HFP, HALeft, false, false, RString(""));
    html->AddBreak(section, false);
}

void Link(CHTMLContainer* html, int section, const char* text, const char* href)
{
    html->AddText(section, RString(text), HFP, HALeft, false, false, RString(href));
    html->AddBreak(section, false);
}

void Gap(CHTMLContainer* html, int section)
{
    html->AddBreak(section, false);
}

// a section by name, created (and named) when absent
int SectionNamed(CHTMLContainer* html, const char* name)
{
    int section = html->FindSection(name);
    if (section < 0)
    {
        section = html->AddSection();
        html->AddName(section, RString(name));
    }
    return section;
}

// one objective row: stock icon + indented text, or a text bullet when no
// texture loader is available (unit tests)
void ObjectiveRow(CHTMLContainer* html, int section, int state, const char* text, bool useImages)
{
    if (state == JOHidden)
    {
        return;
    }
    if (useImages)
    {
        const char* picture = "mission_dot.paa";
        if (state == JODone)
        {
            picture = "mission_done.paa";
        }
        else if (state == JOFailed)
        {
            picture = "mission_uncomplete.paa";
        }
        html->AddBreak(section, false);
        float imgHeight = 640.0f * 1.5f * html->GetPHeight();
        HTMLField* fld = html->AddImage(section, RString(picture), HALeft, false, -1, imgHeight, RString(""));
        if (fld)
        {
            fld->exclude = true;
            html->SetIndent(1.2f * fld->width);
        }
        html->AddText(section, RString(text), HFP, HALeft, false, false, RString(""));
        html->SetIndent(0);
        html->AddBreak(section, false);
        return;
    }
    const char* bullet = "[ ] ";
    if (state == JODone)
    {
        bullet = "[x] ";
    }
    else if (state == JOFailed)
    {
        bullet = "[-] ";
    }
    html->AddText(section, RString(bullet) + RString(text), HFP, HALeft, false, false, RString(""));
    html->AddBreak(section, false);
}

RString Num(float v)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", toInt(v));
    return RString(buffer);
}

RString Fraction(int a, int b)
{
    char buffer[48];
    snprintf(buffer, sizeof(buffer), "%d of %d", a, b);
    return RString(buffer);
}

RString JoinNames(const AutoArray<RString>& names, int limit)
{
    RString out;
    for (int i = 0; i < names.Size() && i < limit; i++)
    {
        if (i > 0)
        {
            out = out + RString(", ");
        }
        out = out + names[i];
    }
    if (names.Size() > limit)
    {
        out = out + RString(" ...");
    }
    return out;
}

const char* AlertName(int state)
{
    switch (state)
    {
        case 2:
            return "RED";
        case 1:
            return "YELLOW";
        default:
            return "GREEN";
    }
}

// ---------------------------------------------------------------------------
// world-dependent helpers (all null-safe)
// ---------------------------------------------------------------------------

float ReadScalar(GameState* gstate, const char* name, float fallback, bool* found = nullptr)
{
    if (found)
    {
        *found = false;
    }
    if (!gstate)
    {
        return fallback;
    }
    GameValue value = gstate->VarGet(name);
    if (value.GetType() != GameScalar)
    {
        return fallback;
    }
    if (found)
    {
        *found = true;
    }
    return (float)value;
}

bool ReadBool(GameState* gstate, const char* name)
{
    if (!gstate)
    {
        return false;
    }
    GameValue value = gstate->VarGet(name);
    return value.GetType() == GameBool && (GameBoolType)value;
}

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

bool GuerrillaJournalActive()
{
    return ZoneRegistry::Instance().IsActive();
}

JournalPageInputs GatherGuerrillaJournalInputs()
{
    JournalPageInputs in;
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive())
    {
        return in;
    }

    in.islandName = IslandDisplayName();
    in.resistanceName = registry.ResistanceFaction();
    if (in.resistanceName.GetLength() == 0)
    {
        in.resistanceName = registry.ResistanceSide();
    }
    in.occupierName = registry.OccupierFaction();
    if (in.occupierName.GetLength() == 0)
    {
        in.occupierName = registry.OccupierSide();
    }
    in.stamp = JournalStampNow();
    in.useImages = true;

    GameState* gstate = GWorld ? GWorld->GetGameState() : nullptr;
    bool foundR = false;
    bool foundHR = false;
    in.resources = ReadScalar(gstate, "gmresources", 0, &foundR);
    in.manpower = ReadScalar(gstate, "gmmanpower", 0, &foundHR);
    in.warLevel = toInt(ReadScalar(gstate, "gmwarlevel", 1));
    in.economyKnown = foundR && foundHR;
    in.undercoverArmed = ReadBool(gstate, "gmundercover");
    if (in.undercoverArmed)
    {
        in.undercoverStatus = UndercoverSystem::Instance().Status();
        in.undercoverWitnesses = UndercoverSystem::Instance().WitnessCount();
    }
    in.stashCount = StashRegistry::Instance().Count();
    // headquarters (GuerrillaBase) and the dealer market
    {
        const GuerrillaBase& base = GuerrillaBase::Instance();
        in.hqEstablished = base.IsEstablished();
        in.hqZone = base.ZoneName();
        in.hqIndoors = base.IsIndoors();
        in.garageCount = base.GarageCount();
        const Market& market = Market::Instance();
        in.marketActive = market.IsActive();
        for (int i = 0; i < market.DealerCount(); i++)
        {
            const DealerRecord* d = market.Dealer(i);
            if (!d)
            {
                continue;
            }
            (d->kind == DKWeapon ? in.weaponDealers : in.vehicleDealers)++;
        }
    }

    // player position (distance ordering of targets) and cell size
    Vector3 playerPos = VZero;
    bool havePlayer = false;
    if (GWorld)
    {
        Person* player = GWorld->PlayerOn();
        if (player)
        {
            playerPos = player->Position();
            havePlayer = true;
            AIUnit* unit = player->Brain();
            AIGroup* grp = unit ? unit->GetGroup() : nullptr;
            if (grp)
            {
                in.cellSize = grp->NUnits();
            }
        }
    }

    const RString resistance = registry.ResistanceSide();
    const RString occupier = registry.OccupierSide();
    const float supportFlip = registry.Tuning().supportFlip;
    const AlertMachine& alerts = AlertMachine::Instance();

    // occupier military zones, collected then ordered nearest-first
    AutoArray<int> targetIdx;
    AutoArray<float> targetDist;

    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z)
        {
            continue;
        }
        const bool ours = stricmp(z->owner, resistance) == 0;
        const bool theirs = stricmp(z->owner, occupier) == 0;
        const bool isCity = stricmp(z->type, "CITY") == 0;
        const bool isCamp = stricmp(z->type, "CAMP") == 0;

        if (isCity)
        {
            in.townsTotal++;
            if (ours)
            {
                in.townsRisen++;
            }
            else if (z->support >= supportFlip)
            {
                in.townsReady.Add(z->name);
            }
        }
        else if (!isCamp)
        {
            in.militaryTotal++;
            if (ours)
            {
                in.militaryHeld++;
            }
            if (!ours && z->capture > 0)
            {
                char buffer[96];
                snprintf(buffer, sizeof(buffer), "%s %d%%", (const char*)z->name, toInt(z->capture));
                in.securing.Add(RString(buffer));
            }
            if (theirs)
            {
                float dist = havePlayer ? (z->pos - playerPos).Size() : 0;
                targetIdx.Add(i);
                targetDist.Add(dist);
            }
        }

        if (z->heat > in.heatMax)
        {
            in.heatMax = z->heat;
            in.heatZone = z->name;
        }
        int alert = alerts.GetZoneState(i);
        if (alert > in.alertMax || (alert == in.alertMax && in.alertZone.GetLength() == 0 && alert > 0))
        {
            in.alertMax = alert;
            in.alertZone = z->name;
        }
    }

    // selection sort is plenty for MaxZones = 64
    for (int a = 0; a < targetIdx.Size(); a++)
    {
        int best = a;
        for (int b = a + 1; b < targetIdx.Size(); b++)
        {
            if (targetDist[b] < targetDist[best])
            {
                best = b;
            }
        }
        if (best != a)
        {
            int ti = targetIdx[a];
            targetIdx[a] = targetIdx[best];
            targetIdx[best] = ti;
            float td = targetDist[a];
            targetDist[a] = targetDist[best];
            targetDist[best] = td;
        }
    }
    for (int a = 0; a < targetIdx.Size(); a++)
    {
        const ZoneRecord* z = registry.GetZone(targetIdx[a]);
        if (!z)
        {
            continue;
        }
        char buffer[160];
        if (havePlayer)
        {
            snprintf(buffer, sizeof(buffer), "%s: %s, garrison %d, %d m away", (const char*)z->name,
                     (const char*)z->type, toInt(z->garrison), toInt(targetDist[a]));
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "%s: %s, garrison %d", (const char*)z->name, (const char*)z->type,
                     toInt(z->garrison));
        }
        in.targets.Add(RString(buffer));
    }
    return in;
}

void BuildGuerrillaJournalPages(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in)
{
    if (!html)
    {
        return;
    }

    // ---- Notes ("Main") ----------------------------------------------------
    int notes = SectionNamed(html, "Main");
    Heading(html, notes, "Uslu dur! Field journal", HFH1);
    {
        RString line;
        if (in.islandName.GetLength() > 0)
        {
            line = in.islandName + RString(" campaign. ");
        }
        else
        {
            line = RString("Guerrilla campaign. ");
        }
        if (in.resistanceName.GetLength() > 0 && in.occupierName.GetLength() > 0)
        {
            line = line + in.resistanceName + RString(" resistance against the ") + in.occupierName +
                   RString(" occupier.");
        }
        Para(html, notes, line);
    }
    if (in.stamp.GetLength() > 0)
    {
        Para(html, notes, in.stamp);
    }
    Gap(html, notes);

    Heading(html, notes, "Situation", HFH2);
    if (in.economyKnown)
    {
        Line(html, notes, "Treasury", Num(in.resources) + RString(" R"));
        Line(html, notes, "Manpower", Num(in.manpower) + RString(" HR"));
    }
    Line(html, notes, "War Level", Num((float)in.warLevel));
    Line(html, notes, "Military zones held", Fraction(in.militaryHeld, in.militaryTotal));
    Line(html, notes, "Towns risen", Fraction(in.townsRisen, in.townsTotal));
    if (in.townsReady.Size() > 0)
    {
        Line(html, notes, "Ready to rise", JoinNames(in.townsReady, 6));
    }
    if (in.securing.Size() > 0)
    {
        Line(html, notes, "Securing", JoinNames(in.securing, 6));
    }
    if (in.alertMax > 0 && in.alertZone.GetLength() > 0)
    {
        Line(html, notes, "Alert", RString(AlertName(in.alertMax)) + RString(" at ") + in.alertZone);
    }
    else
    {
        Line(html, notes, "Alert", "all garrisons GREEN");
    }
    if (in.heatZone.GetLength() > 0)
    {
        Line(html, notes, "Heat", Num(in.heatMax) + RString(" at ") + in.heatZone);
    }
    if (in.undercoverArmed)
    {
        RString cover = "intact";
        if (in.undercoverStatus == 1)
        {
            cover = "suspected";
        }
        else if (in.undercoverStatus == 2)
        {
            cover = RString("blown, ") + Num((float)in.undercoverWitnesses) + RString(" witness group(s)");
        }
        Line(html, notes, "Cover", cover);
    }
    if (in.cellSize > 0)
    {
        Line(html, notes, "Fighters with you", Num((float)in.cellSize));
    }
    if (in.stashCount > 0)
    {
        Line(html, notes, "Arms stashes", Num((float)in.stashCount));
    }
    if (in.hqEstablished)
    {
        RString hq = in.hqZone.GetLength() > 0 ? in.hqZone : RString("unknown zone");
        hq = hq + RString(in.hqIndoors ? " (building)" : " (outdoors)");
        if (in.garageCount > 0)
        {
            hq = hq + RString(", ") + Num((float)in.garageCount) + RString(" vehicle(s) garaged");
        }
        Line(html, notes, "Headquarters", hq);
    }
    else
    {
        Line(html, notes, "Headquarters", "none - establish one");
    }
    if (in.marketActive)
    {
        Line(html, notes, "Dealers",
             Num((float)in.weaponDealers) + RString(" arms / ") + Num((float)in.vehicleDealers) + RString(" vehicle"));
    }
    for (int i = 0; i < journal.StatusCount(); i++)
    {
        Line(html, notes, journal.Status(i).key, journal.Status(i).text);
    }
    Gap(html, notes);

    Heading(html, notes, "Field manual", HFH2);
    for (int i = 0; i < kManualCount; i++)
    {
        Link(html, notes, kManual[i].title, RString("#") + RString(kManual[i].anchor));
    }
    Gap(html, notes);

    Heading(html, notes, "Diary", HFH2);
    const int recent = 8;
    if (journal.EntryCount() == 0)
    {
        Para(html, notes, "Nothing recorded yet.");
    }
    for (int i = journal.EntryCount() - 1, n = 0; i >= 0 && n < recent; i--, n++)
    {
        const JournalEntry& e = journal.Entry(i);
        if (e.stamp.GetLength() > 0)
        {
            Line(html, notes, e.stamp, e.text);
        }
        else
        {
            Para(html, notes, e.text);
        }
    }
    if (journal.EntryCount() > recent)
    {
        Link(html, notes, "Full diary", "#GM_LOG");
    }
    html->FormatSection(notes);

    // ---- Plan --------------------------------------------------------------
    int plan = SectionNamed(html, "Plan");
    {
        RString goal = "Liberate ";
        goal = goal + (in.islandName.GetLength() > 0 ? in.islandName : RString("the island"));
        goal = goal + RString(". Hold every military zone and raise every town; the occupier escalates as you "
                              "grow, so keep the cell supplied and your cover usable.");
        Para(html, plan, goal);
    }
    Gap(html, plan);
    ObjectiveRow(html, plan, in.militaryTotal > 0 && in.militaryHeld >= in.militaryTotal ? JODone : JOActive,
                 RString("Hold every military zone (") + Fraction(in.militaryHeld, in.militaryTotal) + RString(")"),
                 in.useImages);
    ObjectiveRow(html, plan, in.townsTotal > 0 && in.townsRisen >= in.townsTotal ? JODone : JOActive,
                 RString("Raise every town (") + Fraction(in.townsRisen, in.townsTotal) + RString(")"), in.useImages);
    for (int i = 0; i < journal.ObjectiveCount(); i++)
    {
        const JournalObjective& o = journal.Objective(i);
        ObjectiveRow(html, plan, o.state, o.text, in.useImages);
    }
    Gap(html, plan);
    Heading(html, plan, "Next steps", HFH2);
    int steps = 0;
    if (in.alertMax == 2 && in.alertZone.GetLength() > 0)
    {
        Para(html, plan, RString("Break contact: ") + in.alertZone + RString(" is RED and a QRF is coming."));
        steps++;
    }
    if (in.undercoverArmed && in.undercoverStatus == 2)
    {
        Para(html, plan,
             "Your face is known to some groups: stow the weapon and avoid or eliminate the witnesses "
             "to move unseen again.");
        steps++;
    }
    for (int i = 0; i < in.townsReady.Size() && i < 3; i++)
    {
        Para(html, plan, in.townsReady[i] + RString(" is ready to rise: get fighters into the town."));
        steps++;
    }
    for (int i = 0; i < in.securing.Size() && i < 3; i++)
    {
        Para(html, plan, RString("Finish securing ") + in.securing[i] + RString("."));
        steps++;
    }
    for (int i = 0; i < in.targets.Size() && i < 3; i++)
    {
        Para(html, plan, RString("Target: ") + in.targets[i] + RString("."));
        steps++;
    }
    if (in.economyKnown && in.manpower >= 1)
    {
        Para(html, plan, RString("Recruit at the Camp: ") + Num(in.manpower) + RString(" HR available."));
        steps++;
    }
    if (in.heatMax >= 50 && in.heatZone.GetLength() > 0)
    {
        Para(html, plan, RString("Lie low near ") + in.heatZone + RString(": Heat is high there."));
        steps++;
    }
    if (!in.hqEstablished)
    {
        // standing advice, not a tactical step: it does not count against
        // the scout-the-island fallback below
        Para(html, plan,
             "Establish a headquarters: walk into a town (or the Camp) and use the action menu. It gives you a "
             "weapon cache and a garage where vehicles can be locked away.");
    }
    if (steps == 0)
    {
        Para(html, plan, "Scout the island: reveal zones by moving within sight of them.");
    }
    html->FormatSection(plan);

    // ---- full diary --------------------------------------------------------
    int log = SectionNamed(html, "GM_LOG");
    Heading(html, log, "Diary", HFH1);
    if (journal.EntryCount() == 0)
    {
        Para(html, log, "Nothing recorded yet.");
    }
    for (int i = journal.EntryCount() - 1; i >= 0; i--)
    {
        const JournalEntry& e = journal.Entry(i);
        if (e.stamp.GetLength() > 0)
        {
            Line(html, log, e.stamp, e.text);
        }
        else
        {
            Para(html, log, e.text);
        }
    }
    Gap(html, log);
    Link(html, log, "Back to notes", "#Main");
    html->FormatSection(log);

    // ---- manual pages ------------------------------------------------------
    for (int t = 0; t < kManualCount; t++)
    {
        int page = SectionNamed(html, kManual[t].anchor);
        Heading(html, page, kManual[t].title, HFH1);
        for (int p = 0; kManual[t].paragraphs[p]; p++)
        {
            Para(html, page, kManual[t].paragraphs[p]);
            Gap(html, page);
        }
        if (t + 1 < kManualCount)
        {
            Link(html, page, RString("Next: ") + RString(kManual[t + 1].title),
                 RString("#") + RString(kManual[t + 1].anchor));
        }
        Link(html, page, "Back to notes", "#Main");
        html->FormatSection(page);
    }
}

} // namespace Poseidon::Guerrilla
