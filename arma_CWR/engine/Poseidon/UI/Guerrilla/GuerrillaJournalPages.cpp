// ============================================================================
//  GuerrillaJournalPages: the GATHER stage of the Resistance Dossier and its
//  entry point.
//
//  The dossier is built in three stages (GuerrillaJournalPages.hpp has the page
//  map and the slot table):
//
//    Gather   GatherGuerrillaJournalInputs() below.  The only stage that reads
//             the world: ZoneRegistry / AlertMachine / UndercoverSystem /
//             StashRegistry / GuerrillaBase / Market, the player's group and
//             the resistance side's other groups, the script-owned economy
//             globals and the engine's UI aspect.  Everything Compose needs is
//             copied into a JournalPageInputs value; Compose and Render never
//             touch a singleton, so the unit suite drives them world-less.
//    Compose  ComposeJournal() in JournalCompose.hpp (JournalCompose.cpp,
//             JournalComposeOps.cpp, JournalComposePeoplePlaces.cpp): pure,
//             from (Journal, JournalPageInputs) to a JournalDocument of pages,
//             blocks and runs in named voices and inks.
//    Render   RenderJournal() in JournalRender.hpp: binds the format slots and
//             lays the document into the CHTMLContainer, owning the page
//             budget and the "<name>_2" continuation pages.
//
//  BuildGuerrillaJournalPages(html, journal, in) is Compose then Render; it is
//  pure and null-safe on html.  The prose / format helpers shared with Compose
//  live in JournalText.hpp; the handbook table in JournalManual.hpp.
// ============================================================================

#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp>

#include <Poseidon/UI/Guerrilla/JournalCompose.hpp> // ComposeJournal
#include <Poseidon/UI/Guerrilla/JournalRender.hpp>  // RenderJournal
#include <Poseidon/UI/Guerrilla/JournalText.hpp>    // the prose / format helpers shared with Compose

#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>
#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp>
#include <Poseidon/Game/Guerrilla/Journal.hpp>
#include <Poseidon/Game/Guerrilla/LegendRegistry.hpp> // the character rows and the generated history
#include <Poseidon/Game/Guerrilla/Market.hpp>
#include <Poseidon/Game/Guerrilla/StashRegistry.hpp>
#include <Poseidon/Game/Guerrilla/Undercover.hpp>
#include <Poseidon/Game/Guerrilla/WorldNames.hpp> // FactionDisplayName (one copy, shared with the Game layer)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/Streams/QBStream.hpp> // QIFStreamB::FileExist (the portrait probe)

#include <Poseidon/AI/AICenter.hpp>
#include <Poseidon/AI/AIGroup.hpp>
#include <Poseidon/AI/AIUnit.hpp>
#include <Poseidon/AI/EntityAI.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/World/Entities/Weapons/Weapons.hpp> // WeaponType, MaskSlot*
#include <Poseidon/World/World.hpp>
#include <Evaluator/express.hpp> // GameState / GameValue (VarGet)

#include <Poseidon/Foundation/Common/FltOpts.hpp>  // toInt
#include <Poseidon/Foundation/Enums/EnumNames.hpp> // GetEnumValue<TargetSide>
#include <Poseidon/Foundation/platform.hpp>        // stricmp
#include <Poseidon/Graphics/Core/Engine.hpp>       // GEngine (Width2D / Height2D: the UI aspect)

#include <cmath>

namespace Poseidon::Guerrilla
{

namespace
{

// the prose and formatting helpers (Fmt, Sentence, Derive, ZoneBrief ...)
// live in JournalText.{hpp,cpp} so the Compose stage shares one copy
using namespace JournalText;

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

RString ReadString(GameState* gstate, const char* name)
{
    if (!gstate)
    {
        return RString();
    }
    GameValue value = gstate->VarGet(name);
    if (value.GetType() != GameString)
    {
        return RString();
    }
    return RString((GameStringType)value);
}

RString BearingOf(float dx, float dz)
{
    static const char* names[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    float deg = atan2f(dx, dz) * 180.0f / 3.14159265f;
    if (deg < 0)
    {
        deg += 360.0f;
    }
    int idx = (int)((deg + 22.5f) / 45.0f) % 8;
    return RString(names[idx]);
}

// The faction's displayName (optional descriptor key), else the class name with
// '_' rewritten to ' ' ("PLO_East" -> "PLO East"), else the side.  A forwarder
// onto the Game layer's copy: the history prose (FactionHistory) and the Legend
// registry name the same faction the same way as these pages, which they cannot
// do from two implementations.  DECISION: the underscore rewrite applies
// everywhere the journal shows a faction, not only in the generated history.
// The new-game cycler still shows the raw class name (it names a config class,
// not a faction as the journal talks about one), so
// ui/guerrilla_new_game_e2e's "OCCUPIER: PLO_East" is unaffected.
RString FactionDisplay(const ZoneRegistry& registry, const RString& className, const RString& side)
{
    return FactionDisplayName(registry, className, side);
}

// lower-case ASCII copy; the portrait key is a file name, so it is folded the
// same way on every platform and never through the C locale
RString LowerAscii(const RString& text)
{
    const int n = text.GetLength();
    if (n <= 0)
    {
        return RString();
    }
    AutoArray<char> buffer;
    buffer.Resize(n + 1);
    for (int i = 0; i < n; i++)
    {
        const char c = ((const char*)text)[i];
        buffer[i] = (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
    }
    buffer[n] = 0;
    return RString(buffer.Data());
}

// lower(bodyClass) + "__" + lower(face), empty when either half is missing or
// when the face is not one of the four the portrait catalogue ships (a body the
// face validation refused falls back to "Default", which has no photograph, so
// the dossier draws the unavailable treatment instead of a dead texture)
RString PortraitKeyOf(const LegendRow& row)
{
    if (row.bodyClass.GetLength() == 0 || row.face.GetLength() == 0)
    {
        return RString();
    }
    bool known = false;
    for (int i = 0; i < LegendRegistry::NPortraitFaces; i++)
    {
        if (stricmp(row.face, LegendRegistry::kPortraitFaces[i]) == 0)
        {
            known = true;
            break;
        }
    }
    if (!known)
    {
        return RString();
    }
    return LowerAscii(row.bodyClass) + RString("__") + LowerAscii(row.face);
}

// primary / launcher display names of a person
void ArmsOf(Person* person, RString& primary, RString& launcher)
{
    if (!person)
    {
        return;
    }
    for (int i = 0; i < person->NWeaponSystems(); i++)
    {
        const WeaponType* weapon = person->GetWeaponSystem(i);
        if (!weapon || weapon->_scope < 2)
        {
            continue;
        }
        if ((weapon->_weaponType & MaskSlotPrimary) && primary.GetLength() == 0)
        {
            primary = weapon->GetDisplayName();
        }
        else if ((weapon->_weaponType & MaskSlotSecondary) && launcher.GetLength() == 0)
        {
            launcher = weapon->GetDisplayName();
        }
    }
}

RString NearestZoneName(const ZoneRegistry& registry, Vector3Par pos)
{
    int best = -1;
    float bestDist = 0;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z)
        {
            continue;
        }
        float dist = (z->pos - pos).SizeXZ();
        if (best < 0 || dist < bestDist)
        {
            best = i;
            bestDist = dist;
        }
    }
    const ZoneRecord* z = best >= 0 ? registry.GetZone(best) : nullptr;
    return z ? z->name : RString();
}

} // namespace

// ===========================================================================
// public surface
// ===========================================================================

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
    in.resistanceName = FactionDisplay(registry, registry.ResistanceFaction(), registry.ResistanceSide());
    in.occupierName = FactionDisplay(registry, registry.OccupierFaction(), registry.OccupierSide());
    in.resistanceFactionClass = registry.ResistanceFaction();
    // portraits ship loose under the data root (Change 4); Render prefixes this
    // with a backslash so AddImage skips the briefing-relative search
    in.portraitDir = "gmcore\\portraits";
    // The UI aspect the notepad is projected into. Width2D/Height2D derive from
    // the aspect band the display settings pick (Engine.cpp Width2D/Height2D),
    // so this is display- and settings-dependent, not a constant, and it is NOT
    // GetAspectSettings().leftFOV/topFOV (a camera FOV, not the 2D region).
    // Render turns it into the portrait box that reads square on screen.
    in.uiAspect =
        (GEngine && GEngine->Height2D() > 0) ? (float)GEngine->Width2D() / (float)GEngine->Height2D() : 4.0f / 3.0f;
    JournalClockNow(in.day, in.minuteOfDay);
    in.supportFlip = registry.Tuning().supportFlip;

    GameState* gstate = GWorld ? GWorld->GetGameState() : nullptr;
    bool foundR = false;
    bool foundHR = false;
    in.resources = ReadScalar(gstate, "gmresources", 0, &foundR);
    in.manpower = ReadScalar(gstate, "gmmanpower", 0, &foundHR);
    in.manpowerCap = ReadScalar(gstate, "gm_manpower_cap", 0);
    in.warLevel = toInt(ReadScalar(gstate, "gmwarlevel", 1));
    in.economyKnown = foundR && foundHR;
    bool foundIncome = false;
    in.incomeR = ReadScalar(gstate, "gmecor", 0, &foundIncome);
    in.incomeKnown = foundIncome;
    in.incomeHR = ReadScalar(gstate, "gmecohr", 0);
    in.econTickSeconds = ReadScalar(gstate, "gm_econ_tick", 0);
    in.undercoverArmed = ReadBool(gstate, "gmundercover");
    if (in.undercoverArmed)
    {
        in.undercoverStatus = UndercoverSystem::Instance().Status();
        in.undercoverWitnesses = UndercoverSystem::Instance().WitnessCount();
    }
    in.stashCount = StashRegistry::Instance().Count();
    in.faction.doctrine = ReadString(gstate, "gmdoctrine");
    in.faction.outsideSupport = ReadString(gstate, "gmoutsidesupport");
    in.faction.alliedCells = toInt(ReadScalar(gstate, "gmalliedcells", 0));

    // occupier tier ladder (war levels where the next tier arrives)
    if (const FactionRecord* occ = registry.FindFactionForSide(registry.OccupierSide()))
    {
        for (int i = 0; i < occ->tierThresholds.Size(); i++)
        {
            in.occupierTierThresholds.Add(occ->tierThresholds[i]);
        }
    }

    // headquarters (GuerrillaBase) and the dealer market
    {
        const GuerrillaBase& base = GuerrillaBase::Instance();
        in.hqEstablished = base.IsEstablished();
        in.hqZone = base.ZoneName();
        in.hqIndoors = base.IsIndoors();
        in.garageCount = base.GarageCount();
        for (int i = 0; i < base.GarageCount(); i++)
        {
            EntityAI* veh = base.GarageVehicle(i);
            if (veh && veh->GetType())
            {
                in.garage.Add(veh->GetType()->GetDisplayName());
            }
        }
        const Market& market = Market::Instance();
        in.marketActive = market.IsActive();
        for (int i = 0; i < market.DealerCount(); i++)
        {
            const DealerRecord* dealer = market.Dealer(i);
            if (!dealer)
            {
                continue;
            }
            (dealer->kind == DKWeapon ? in.weaponDealerTowns : in.vehicleDealerTowns).Add(dealer->zoneName);
        }
    }

    // the player and their group
    Vector3 playerPos = VZero;
    bool havePlayer = false;
    AIGroup* playerGroup = nullptr;
    if (GWorld)
    {
        Person* player = GWorld->PlayerOn();
        if (player)
        {
            playerPos = player->Position();
            havePlayer = true;
            AIUnit* unit = player->Brain();
            playerGroup = unit ? unit->GetGroup() : nullptr;
        }
    }
    if (playerGroup)
    {
        for (int id = 1; id <= MAX_UNITS_PER_GROUP; id++)
        {
            AIUnit* unit = playerGroup->UnitWithID(id);
            if (!unit || !unit->IsUnit())
            {
                continue;
            }
            Person* person = unit->GetPerson();
            if (!person)
            {
                continue;
            }
            JournalRosterRow row;
            row.isPlayer = unit->IsPlayer();
            row.name = row.isPlayer ? RString("You") : person->GetInfo()._name;
            row.rank = ClampRankIndex(person->GetRank());
            row.role = row.isPlayer ? RString("Leader") : RString(person->GetType()->GetDisplayName());
            ArmsOf(person, row.primary, row.launcher);
            row.wounded = toInt(person->GetTotalDammage() * 100.0f);
            row.withPlayer = true;
            // the player first
            if (row.isPlayer)
            {
                in.roster.Insert(0, row);
            }
            else
            {
                in.roster.Add(row);
            }
        }
    }
    // other resistance groups: holding squads, one row per group
    if (GWorld)
    {
        using Poseidon::Foundation::GetEnumValue;
        TargetSide side = GetEnumValue<TargetSide>((const char*)registry.ResistanceSide());
        AICenter* center = ((int)side >= 0 && side < TSideUnknown) ? GWorld->GetCenter(side) : nullptr;
        for (int g = 0; center && g < center->NGroups(); g++)
        {
            AIGroup* grp = center->GetGroup(g);
            if (!grp || grp == playerGroup)
            {
                continue;
            }
            JournalRosterRow row;
            row.withPlayer = false;
            row.count = 0;
            Vector3 where = VZero;
            for (int id = 1; id <= MAX_UNITS_PER_GROUP; id++)
            {
                AIUnit* unit = grp->UnitWithID(id);
                if (!unit || !unit->IsUnit())
                {
                    continue;
                }
                Person* person = unit->GetPerson();
                if (!person)
                {
                    continue;
                }
                if (row.count == 0)
                {
                    row.role = person->GetType()->GetDisplayName();
                    ArmsOf(person, row.primary, row.launcher);
                    where = person->Position();
                }
                row.count++;
            }
            if (row.count == 0)
            {
                continue;
            }
            row.zone = NearestZoneName(registry, where);
            row.name = row.zone.GetLength() > 0 ? row.zone + RString(" squad") : RString("Holding squad");
            in.roster.Add(row);
        }
    }

    // zones
    const RString resistance = registry.ResistanceSide();
    const RString occupier = registry.OccupierSide();
    const AlertMachine& alerts = AlertMachine::Instance();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z)
        {
            continue;
        }
        JournalZoneRow row;
        row.name = z->name;
        row.type = z->type;
        const bool ours = stricmp(z->owner, resistance) == 0;
        const bool theirs = stricmp(z->owner, occupier) == 0;
        row.holder = ours ? 0 : theirs ? 1 : 2;
        row.revealed = z->revealed;
        row.support = z->support;
        row.capture = z->capture;
        row.heat = z->heat;
        row.garrison = toInt(z->garrison);
        row.alert = alerts.GetZoneState(i);
        row.seenDay = z->seenDay;
        row.seenMinute = z->seenMinute;
        if (havePlayer)
        {
            const float dx = z->pos.X() - playerPos.X();
            const float dz = z->pos.Z() - playerPos.Z();
            row.distance = sqrtf(dx * dx + dz * dz);
            row.bearing = BearingOf(dx, dz);
        }
        const bool isCity = stricmp(z->type, "CITY") == 0;
        const bool isCamp = stricmp(z->type, "CAMP") == 0;
        if (isCity)
        {
            in.townsTotal++;
            if (ours)
            {
                in.townsRisen++;
            }
        }
        else if (!isCamp)
        {
            in.militaryTotal++;
            if (ours)
            {
                in.militaryHeld++;
            }
        }
        in.zones.Add(row);
    }

    // ---- the named: companions, enemy Legends, memorials, and the history.
    // This is the ONLY place the Legend registry is read.  Compose and Render
    // stay pure (design D0), so a character view carries everything the
    // dossier pages need, resolved here.
    {
        const LegendRegistry& legends = LegendRegistry::Instance();
        // Row order is the registry's own contract (companions by compIndex
        // ascending, then the pre-rolled boss rows by id), so the People page
        // walks the rows straight through and never re-sorts them.
        for (int i = 0; i < legends.RowCount(); i++)
        {
            const LegendRow& row = legends.Row(i);
            JournalCharacterView view;
            view.id = row.id;
            view.displayName = legends.DisplayName(row);
            view.kind = row.kind;
            view.alive = row.alive;
            view.rank = row.rankSeen;
            view.legend = row.legend;
            view.bio = row.bio;
            view.defeated = row.defeated;
            if (row.deeds.Size() > 0)
            {
                view.deedLatest = row.deeds[row.deeds.Size() - 1].text;
            }
            if (row.kind == LKBoss)
            {
                // row.role is ALREADY the resolved role name ("Sniper" /
                // "Commander" / "Tank Commander"): ResolveBosses writes it once
                // at seeding, so the dossier never maps a role index back to a
                // string and this file needs nothing out of LegendPlacement.
                view.role = row.role;
                // lowercase, as the dossier caption reads it ("Commander, at
                // large"); a defeated Legend keeps its dossier page for good.
                // A commander with no stand (placement found nowhere to put
                // him, or the campaign was seeded before Change 3 existed and
                // never will) reads as missing intelligence rather than as an
                // elite standing somewhere: he has no marker and no objective
                // to disagree with, and he is never retried.
                //
                // The second clause is the spawn that FAILED.  row.spawned is
                // latched before the actors are built, so it is true on that
                // path too; bodySeen is the fact that a body ever existed and
                // is written immediately after BindRow, so a live commander can
                // never read as missing, and a row that has simply not reached
                // its spawn tick yet still carries spawned == false.
                const bool unlocated =
                    row.zoneName.GetLength() == 0 || (row.spawned && !row.bodySeen && !row.body.GetLink());
                view.status = row.defeated ? RString("defeated")
                              : unlocated  ? RString("whereabouts unknown")
                                           : RString("at large");
                view.zone = row.zoneName;
            }
            else
            {
                view.baseName = row.baseName;
                view.role = RankShort(row.rankSeen);
                Person* person = dyn_cast<Person>(row.body.GetLink());
                AIUnit* brain = person ? person->Brain() : nullptr;
                const bool liveBody = brain && brain->GetLifeState() == AIUnit::LSAlive;
                view.status = !row.alive                                       ? RString("fallen")
                              : (liveBody && brain->GetGroup() == playerGroup) ? RString("with me")
                                                                               : RString("unaccounted for");
                view.zone = row.lastZone;
            }
            view.portraitKey = PortraitKeyOf(row);
            // Probed once per character per rebuild, against the SAME path
            // Compose hands to Render minus its leading backslash, so the
            // "available" answer and the drawn source can never disagree.
            // portraitDir is empty in every unit test, which is what keeps the
            // suite off the file system.
            if (view.portraitKey.GetLength() > 0 && in.portraitDir.GetLength() > 0)
            {
                const RString path = in.portraitDir + RString("\\") + view.portraitKey + RString(".paa");
                view.portraitPresent = QIFStreamB::FileExist(path);
            }
            in.characters.Add(view);
        }

        const HistoryRecord& history = legends.History();
        in.history.present = history.Present();
        if (in.history.present)
        {
            in.history.opening1 = history.openingPage1;
            in.history.opening2 = history.openingPage2;
            for (int k = 0; k < kHistoryEvents && k < 3; k++)
            {
                in.history.events[k].title = history.eventTitle[k];
                in.history.events[k].text = history.eventText[k];
                in.history.events[k].place = history.eventPlace[k];
            }
        }
    }
    return in;
}

void BuildGuerrillaJournalPages(CHTMLContainer* html, const Journal& journal, const JournalPageInputs& in)
{
    if (!html)
    {
        return;
    }
    // Compose is pure and never sees the container; Render binds the slots,
    // lays the pages and owns the page budget (JournalRender.hpp)
    const JournalDocument doc = ComposeJournal(journal, in);
    RenderJournal(html, doc, in);
}

} // namespace Poseidon::Guerrilla
