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
#include <Poseidon/Game/Guerrilla/Market.hpp>
#include <Poseidon/Game/Guerrilla/StashRegistry.hpp>
#include <Poseidon/Game/Guerrilla/Undercover.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

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

// the faction's displayName (optional descriptor key), else class name, else side
RString FactionDisplay(const ZoneRegistry& registry, const RString& className, const RString& side)
{
    if (className.GetLength() > 0)
    {
        RString dn = registry.FactionValue(className, "displayName");
        if (dn.GetLength() > 0)
        {
            return dn;
        }
        return className;
    }
    if (side.GetLength() > 0)
    {
        RString dn = registry.FactionValue(side, "displayName");
        if (dn.GetLength() > 0)
        {
            return dn;
        }
    }
    return side;
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
    // characters / history stay empty until Change 2 fills them from the registry
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
