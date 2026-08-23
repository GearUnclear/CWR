#include <Poseidon/Game/Guerrilla/Market.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFile/LocalizedString.hpp> // displayName ($STR_) resolution
#include <Poseidon/IO/ParamFileExt.hpp>              // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp> // GLOB_LAND surface Y
#include <Poseidon/World/Terrain/Roads.hpp>     // GRoadNet road avoidance
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/VehicleAI.hpp> // Rank

#include <Random/randomGen.hpp>

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <string.h>

using namespace Poseidon;

// Shared command internal (Game/Commands/GameStateExtWorld.cpp) - the body
// of createUnit without the script-value parsing.  Global namespace, same
// forward-declaration idiom as GarrisonCache.cpp / Traffic.cpp.
void CreateUnit(AIGroup* group, RString type, Vector3Par position, RString init, float skill, Rank rank);

namespace Poseidon::Guerrilla
{

// Defined in MarketCommands.cpp.  Referencing it from here forces the
// command TU (whose only other content is an INIT_MODULE registration) into
// the link - same pattern as EnsureStashRegistryCommandsLinked.
void EnsureMarketCommandsLinked();

// Process-lifetime singleton - no global constructor (see express.hpp's
// GGameState for the convention).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
Market& Market::Instance()
{
    EnsureMarketCommandsLinked();
    static Market instance;
    return instance;
}
#pragma clang diagnostic pop

static float Dist2D(Vector3Par a, Vector3Par b)
{
    float dx = a.X() - b.X();
    float dz = a.Z() - b.Z();
    return sqrtf(dx * dx + dz * dz);
}

// ---------------------------------------------------------------------------
// lifecycle / config
// ---------------------------------------------------------------------------

void Market::Clear()
{
    _tuning = MarketTuning();
    _configured = false;
    _configLoaded = false;
    _weapons.Clear();
    _vehicles.Clear();
    for (int k = 0; k < NDealerKinds; k++)
    {
        _authoredDealers[k].Clear();
        _groups[k].Clear();
    }
    _rows.Clear();
    _assigned = false;
    _seed = 0;
    _accum = 0;
    _pending.Clear();
}

void Market::InitMission()
{
    Clear();
    LoadFromConfig();
}

void Market::LoadFromConfig()
{
    const ParamEntry* market = ExtParsMission.FindEntry("CfgGuerrillaMarket");
    if (!market)
    {
        market = Pars.FindEntry("CfgGuerrillaMarket");
    }
    const ParamEntry* factions = ExtParsMission.FindEntry("CfgGuerrillaFactions");
    if (!factions)
    {
        factions = Pars.FindEntry("CfgGuerrillaFactions");
    }
    // the engine path always resolves classnames against the loaded data
    // package (plan 15): an authored row the package lacks is dropped with a
    // log line instead of a fatal createVehicle / addWeaponCargo downstream
    ParsClassProbe probe;
    LoadFromParams(market, factions, &probe);
    DropUncreatableClasses();
    ResolveDisplayNames();
    _configLoaded = true;
}

// existence is not creatability: CfgVehicles carries abstract bases (the
// Classic 1.99 "Motorcycle" is scope 0 - the rideable bike is an addon class)
// and NewVehicle refuses them ("type is abstract").  The engine path reads
// the package's (inherited) scope and drops such rows / falls back on the
// dealer body, so a purchase never creates nothing.  Pars-only: the unit
// tests' probe fakes existence and never reach this.
static bool VehicleClassCreatable(const RString& className)
{
    const ParamEntry* bank = Pars.FindEntry("CfgVehicles");
    const ParamEntry* cls = bank ? bank->FindEntry(className) : nullptr;
    if (!cls)
    {
        return false;
    }
    return cls->ReadValue("scope", 2.0f) > 0.0f;
}

void Market::DropUncreatableClasses()
{
    for (int i = _vehicles.Size() - 1; i >= 0; i--)
    {
        if (!VehicleClassCreatable(_vehicles[i].vehicle))
        {
            LOG_WARN(Core, "Market: vehicle '{}' (row '{}') is abstract in the loaded package - dropped",
                     (const char*)_vehicles[i].vehicle, (const char*)_vehicles[i].name);
            _vehicles.Delete(i);
        }
    }
    if (_tuning.dealerClass.GetLength() > 0 && !VehicleClassCreatable(_tuning.dealerClass))
    {
        LOG_WARN(Core, "Market: dealer body '{}' is abstract in the loaded package - using Civilian",
                 (const char*)_tuning.dealerClass);
        _tuning.dealerClass = "Civilian";
    }
}

// string array entries of an optional key (missing / non-array = empty)
static void ReadStringArray(const ParamEntry* cls, const char* key, AutoArray<RString>& out)
{
    out.Clear();
    if (!cls)
    {
        return;
    }
    const ParamEntry* arr = cls->FindEntry(key);
    if (!arr || !arr->IsArray())
    {
        return;
    }
    for (int i = 0; i < arr->GetSize(); i++)
    {
        RString value = (*arr)[i];
        if (value.GetLength() > 0)
        {
            out.Add(value);
        }
    }
}

// the CIV descriptor's first civilian body (civClass1), "" when absent
static RString CivDescriptorClass(const ParamEntry* factionsCfg)
{
    if (!factionsCfg)
    {
        return RString();
    }
    for (int i = 0; i < factionsCfg->GetEntryCount(); i++)
    {
        const ParamEntry& e = factionsCfg->GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        RString side = e.ReadValue("side", RString(e.GetName()));
        if (stricmp(side, "CIV") != 0)
        {
            continue;
        }
        return e.ReadValue("civClass1", RString());
    }
    return RString();
}

void Market::LoadFromParams(const ParamEntry* marketCfg, const ParamEntry* factionsCfg, const ClassProbe* probe)
{
    _tuning = MarketTuning();
    _configured = false;
    _weapons.Clear();
    _vehicles.Clear();
    for (int k = 0; k < NDealerKinds; k++)
    {
        _authoredDealers[k].Clear();
    }
    if (!marketCfg)
    {
        return;
    }
    _configured = true;
    _tuning.dealerShare = marketCfg->ReadValue("dealerShare", _tuning.dealerShare);
    _tuning.dealerRespawnSeconds = marketCfg->ReadValue("dealerRespawnSeconds", _tuning.dealerRespawnSeconds);
    _tuning.hqMoveCost = marketCfg->ReadValue("hqMoveCost", _tuning.hqMoveCost);
    if (_tuning.dealerShare < 0.0f)
    {
        _tuning.dealerShare = 0.0f;
    }
    if (_tuning.dealerShare > 1.0f)
    {
        _tuning.dealerShare = 1.0f;
    }
    if (_tuning.dealerRespawnSeconds < 1.0f)
    {
        _tuning.dealerRespawnSeconds = 1.0f;
    }
    if (_tuning.hqMoveCost < 0.0f)
    {
        _tuning.hqMoveCost = 0.0f;
    }

    // dealer body: authored key > the CIV descriptor's civClass1 > the stock
    // "Civilian"; each rung is package-probed when a probe is supplied
    RString authoredBody = marketCfg->ReadValue("dealerClass", RString());
    RString civBody = CivDescriptorClass(factionsCfg);
    const RString rungs[] = {authoredBody, civBody, RString("Civilian")};
    for (const RString& rung : rungs)
    {
        if (rung.GetLength() == 0)
        {
            continue;
        }
        if (probe && !probe->Exists("CfgVehicles", rung))
        {
            LOG_WARN(Core, "Market: dealer body '{}' is not in the loaded package - trying the next fallback",
                     (const char*)rung);
            continue;
        }
        _tuning.dealerClass = rung;
        break;
    }
    if (_tuning.dealerClass.GetLength() == 0)
    {
        _tuning.dealerClass = "Civilian"; // nothing probed clean: keep the stock body, spawn may degrade
    }

    ReadStringArray(marketCfg, "weaponDealers", _authoredDealers[DKWeapon]);
    ReadStringArray(marketCfg, "vehicleDealers", _authoredDealers[DKVehicle]);

    if (const ParamEntry* weapons = marketCfg->FindEntry("Weapons"))
    {
        for (int i = 0; i < weapons->GetEntryCount(); i++)
        {
            const ParamEntry& e = weapons->GetEntry(i);
            if (!e.IsClass())
            {
                continue;
            }
            MarketWeaponRow row;
            row.name = e.GetName();
            row.weapon = e.ReadValue("weapon", RString());
            row.magazine = e.ReadValue("magazine", RString());
            row.mags = toInt(e.ReadValue("mags", 0.0f));
            row.price = e.ReadValue("price", 0.0f);
            if (row.weapon.GetLength() == 0 && row.magazine.GetLength() == 0)
            {
                LOG_WARN(Core, "Market: stock row '{}' names neither a weapon nor a magazine - dropped",
                         (const char*)row.name);
                continue;
            }
            if (probe && row.weapon.GetLength() > 0 && !probe->Exists("CfgWeapons", row.weapon))
            {
                LOG_WARN(Core, "Market: weapon '{}' (row '{}') is not in the loaded package - dropped",
                         (const char*)row.weapon, (const char*)row.name);
                continue;
            }
            // OFP convention: magazines are CfgWeapons classes (a simple weapon
            // IS its own magazine); a CfgMagazines bank is accepted when a
            // package ships one
            if (probe && row.magazine.GetLength() > 0 && !probe->Exists("CfgWeapons", row.magazine) &&
                !probe->Exists("CfgMagazines", row.magazine))
            {
                if (row.weapon.GetLength() > 0)
                {
                    LOG_WARN(Core, "Market: magazine '{}' (row '{}') is not in the loaded package - weapon sold bare",
                             (const char*)row.magazine, (const char*)row.name);
                    row.magazine = RString();
                    row.mags = 0;
                }
                else
                {
                    LOG_WARN(Core, "Market: magazine '{}' (row '{}') is not in the loaded package - dropped",
                             (const char*)row.magazine, (const char*)row.name);
                    continue;
                }
            }
            if (row.magazine.GetLength() > 0 && row.mags < 1)
            {
                row.mags = 1;
            }
            if (row.magazine.GetLength() == 0)
            {
                row.mags = 0;
            }
            if (row.price < 0.0f)
            {
                row.price = 0.0f;
            }
            row.displayName = row.weapon.GetLength() > 0 ? row.weapon : row.magazine;
            _weapons.Add(row);
        }
    }

    if (const ParamEntry* vehicles = marketCfg->FindEntry("Vehicles"))
    {
        for (int i = 0; i < vehicles->GetEntryCount(); i++)
        {
            const ParamEntry& e = vehicles->GetEntry(i);
            if (!e.IsClass())
            {
                continue;
            }
            MarketVehicleRow row;
            row.name = e.GetName();
            row.vehicle = e.ReadValue("vehicle", RString());
            row.price = e.ReadValue("price", 0.0f);
            if (row.vehicle.GetLength() == 0)
            {
                LOG_WARN(Core, "Market: vehicle row '{}' names no vehicle - dropped", (const char*)row.name);
                continue;
            }
            if (probe && !probe->Exists("CfgVehicles", row.vehicle))
            {
                LOG_WARN(Core, "Market: vehicle '{}' (row '{}') is not in the loaded package - dropped",
                         (const char*)row.vehicle, (const char*)row.name);
                continue;
            }
            if (row.price < 0.0f)
            {
                row.price = 0.0f;
            }
            row.displayName = row.vehicle;
            _vehicles.Add(row);
        }
    }
}

// the package config's (localized) displayName for a class of a bank; the
// class name itself when the class or the key is missing
static RString ConfigDisplayName(const char* bank, const RString& className)
{
    if (className.GetLength() == 0)
    {
        return RString();
    }
    const ParamEntry* bankEntry = Pars.FindEntry(bank);
    const ParamEntry* cls = bankEntry ? bankEntry->FindEntry(className) : nullptr;
    const ParamEntry* dn = cls ? cls->FindEntry("displayName") : nullptr;
    if (!dn)
    {
        return className;
    }
    LocalizedString text;
    text.Bind(*dn);
    RString resolved = (const char*)text.Get();
    return resolved.GetLength() > 0 ? resolved : className;
}

void Market::ResolveDisplayNames()
{
    for (int i = 0; i < _weapons.Size(); i++)
    {
        MarketWeaponRow& row = _weapons[i];
        row.displayName = row.weapon.GetLength() > 0 ? ConfigDisplayName("CfgWeapons", row.weapon)
                                                     : ConfigDisplayName("CfgWeapons", row.magazine);
    }
    for (int i = 0; i < _vehicles.Size(); i++)
    {
        MarketVehicleRow& row = _vehicles[i];
        row.displayName = ConfigDisplayName("CfgVehicles", row.vehicle);
    }
}

// ---------------------------------------------------------------------------
// queries
// ---------------------------------------------------------------------------

bool Market::IsActive() const
{
    return _configured && ZoneRegistry::Instance().IsActive();
}

float Market::Value(const char* key) const
{
    if (!key)
    {
        return 0.0f;
    }
    if (stricmp(key, "dealerShare") == 0)
    {
        return _tuning.dealerShare;
    }
    if (stricmp(key, "dealerRespawnSeconds") == 0)
    {
        return _tuning.dealerRespawnSeconds;
    }
    if (stricmp(key, "hqMoveCost") == 0)
    {
        return _tuning.hqMoveCost;
    }
    return 0.0f;
}

int Market::NAuthoredDealers(int kind) const
{
    if (kind < 0 || kind >= NDealerKinds)
    {
        return 0;
    }
    return _authoredDealers[kind].Size();
}

RString Market::AuthoredDealer(int kind, int i) const
{
    if (kind < 0 || kind >= NDealerKinds || i < 0 || i >= _authoredDealers[kind].Size())
    {
        return RString();
    }
    return _authoredDealers[kind][i];
}

const DealerRecord* Market::Dealer(int i) const
{
    if (i < 0 || i >= _rows.Size())
    {
        return nullptr;
    }
    return &_rows[i];
}

int Market::NearestDealer(int kind, Vector3Par pos) const
{
    int best = -1;
    float bestDist = 0;
    for (int i = 0; i < _rows.Size(); i++)
    {
        if (_rows[i].kind != kind)
        {
            continue;
        }
        float d = Dist2D(_rows[i].pos, pos);
        if (best < 0 || d < bestDist)
        {
            best = i;
            bestDist = d;
        }
    }
    return best;
}

// ---------------------------------------------------------------------------
// pure logic (unit-tested)
// ---------------------------------------------------------------------------

int Market::DealerQuota(int nCities, float share)
{
    if (nCities <= 0)
    {
        return 0;
    }
    int quota = toInt(floorf((float)nCities * share + 0.5f));
    if (quota < 1)
    {
        quota = 1;
    }
    if (quota > nCities)
    {
        quota = nCities;
    }
    return quota;
}

static int PosHash(Vector3Par pos)
{
    int x = toInt(pos.X());
    int z = toInt(pos.Z());
    return (x * 73856093) ^ (z * 19349663);
}

void Market::ShuffleOrder(int seed, int salt, const AutoArray<Vector3>& cityPos, AutoArray<int>& order)
{
    order.Clear();
    AutoArray<float> keys;
    for (int i = 0; i < cityPos.Size(); i++)
    {
        order.Add(i);
        // RandomValue(int) is the table-driven pure generator: same seed,
        // same value, no generator state touched
        keys.Add(GRandGen.RandomValue(seed ^ salt ^ PosHash(cityPos[i])));
    }
    // insertion sort by key, stable by index (city lists are small)
    for (int i = 1; i < order.Size(); i++)
    {
        int idx = order[i];
        float key = keys[idx];
        int j = i - 1;
        while (j >= 0 && keys[order[j]] > key)
        {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = idx;
    }
}

static int FindCity(const AutoArray<DealerCity>& cities, const char* name)
{
    for (int i = 0; i < cities.Size(); i++)
    {
        if (stricmp(cities[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

static void PlanKind(const AutoArray<DealerCity>& cities, int seed, int salt, float share, int kind,
                     const AutoArray<RString>& authored, AutoArray<DealerRecord>& out)
{
    AutoArray<int> picks;
    if (authored.Size() > 0)
    {
        for (int i = 0; i < authored.Size(); i++)
        {
            int c = FindCity(cities, authored[i]);
            if (c < 0)
            {
                LOG_WARN(Core, "Market: authored {} dealer town '{}' is not a CITY zone - skipped",
                         Market::KindName(kind), (const char*)authored[i]);
                continue;
            }
            bool dup = false;
            for (int j = 0; j < picks.Size() && !dup; j++)
            {
                dup = picks[j] == c;
            }
            if (!dup)
            {
                picks.Add(c);
            }
        }
    }
    else
    {
        AutoArray<Vector3> pos;
        for (int i = 0; i < cities.Size(); i++)
        {
            pos.Add(cities[i].pos);
        }
        AutoArray<int> order;
        Market::ShuffleOrder(seed, salt, pos, order);
        int quota = Market::DealerQuota(cities.Size(), share);
        for (int i = 0; i < quota && i < order.Size(); i++)
        {
            picks.Add(order[i]);
        }
    }
    for (int i = 0; i < picks.Size(); i++)
    {
        DealerRecord row;
        row.zoneName = cities[picks[i]].name;
        row.kind = kind;
        row.pos = cities[picks[i]].pos;
        row.lotPos = row.pos;
        out.Add(row);
    }
}

void Market::PlanDealers(const AutoArray<DealerCity>& cities, int seed, float share,
                         const AutoArray<RString>& authoredWeapon, const AutoArray<RString>& authoredVehicle,
                         AutoArray<DealerRecord>& out)
{
    out.Clear();
    PlanKind(cities, seed, SaltWeapon, share, DKWeapon, authoredWeapon, out);
    PlanKind(cities, seed, SaltVehicle, share, DKVehicle, authoredVehicle, out);
}

const char* Market::KindName(int kind)
{
    switch (kind)
    {
        case DKWeapon:
            return "WEAPON";
        case DKVehicle:
            return "VEHICLE";
        default:
            return "";
    }
}

int Market::KindFromName(const char* name)
{
    if (!name)
    {
        return -1;
    }
    if (stricmp(name, "WEAPON") == 0 || stricmp(name, "weapons") == 0 || stricmp(name, "arms") == 0)
    {
        return DKWeapon;
    }
    if (stricmp(name, "VEHICLE") == 0 || stricmp(name, "vehicles") == 0)
    {
        return DKVehicle;
    }
    return -1;
}

void Market::AssignForTest(const AutoArray<DealerCity>& cities, int seed)
{
    _seed = seed;
    PlanDealers(cities, _seed, _tuning.dealerShare, _authoredDealers[DKWeapon], _authoredDealers[DKVehicle], _rows);
    _assigned = true;
}

// ---------------------------------------------------------------------------
// world-touching internals (engine path only)
// ---------------------------------------------------------------------------

void Market::Assign()
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    AutoArray<DealerCity> cities;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || stricmp(z->type, "CITY") != 0)
        {
            continue;
        }
        DealerCity c;
        c.name = z->name;
        c.pos = z->pos;
        cities.Add(c);
    }
    if (_seed == 0)
    {
        // one draw from the stateful generator, then serialized: a reload
        // keeps the same towns
        _seed = toInt(GRandGen.RandomValue() * 1073741823.0f) | 1;
    }
    PlanDealers(cities, _seed, _tuning.dealerShare, _authoredDealers[DKWeapon], _authoredDealers[DKVehicle], _rows);
    _assigned = true;
    int weapons = 0;
    int vehicles = 0;
    for (int i = 0; i < _rows.Size(); i++)
    {
        (_rows[i].kind == DKWeapon ? weapons : vehicles)++;
    }
    LOG_INFO(Core, "Market: {} cities -> {} weapon dealer(s), {} vehicle dealer(s) (seed {})", cities.Size(), weapons,
             vehicles, _seed);
}

// deterministic ring samples inside the town: radii 0.3/0.4/0.5 of the zone
// area, weapon dealers on the cardinal bearings, vehicle dealers on the
// diagonals.  The dealer takes the first dry off-road sample; a vehicle
// dealer's LOT is the next acceptable sample at least LotMinDist away.
bool Market::ComputeDealerSpot(DealerRecord& row) const
{
    if (!GLandscape)
    {
        return false;
    }
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    int zoneIndex = registry.FindZoneIndex(row.zoneName);
    const ZoneRecord* z = registry.GetZone(zoneIndex);
    if (!z)
    {
        return false;
    }
    row.zoneIndex = zoneIndex;
    float area = registry.Tuning().zoneArea;
    const float fractions[] = {0.3f, 0.4f, 0.5f};
    const float offset = row.kind == DKVehicle ? 45.0f : 0.0f;
    AutoArray<Vector3> accepted;
    for (float fraction : fractions)
    {
        float radius = fraction * area;
        for (int b = 0; b < 4; b++)
        {
            float angle = (offset + 90.0f * (float)b) * (H_PI / 180.0f);
            float x = z->pos.X() + radius * cosf(angle);
            float zz = z->pos.Z() + radius * sinf(angle);
            float y = GLOB_LAND->SurfaceY(x, zz);
            if (y < 0.1f)
            {
                continue; // coastal towns reach the sea
            }
            Vector3 p(x, y, zz);
            if (GRoadNet && GRoadNet->IsOnRoad(p, RoadClearance))
            {
                continue;
            }
            accepted.Add(p);
        }
    }
    if (accepted.Size() == 0)
    {
        return false;
    }
    row.pos = accepted[0];
    row.lotPos = row.pos;
    if (row.kind == DKVehicle)
    {
        for (int i = 1; i < accepted.Size(); i++)
        {
            if (Dist2D(accepted[i], row.pos) >= LotMinDist)
            {
                row.lotPos = accepted[i];
                break;
            }
        }
    }
    row.placed = true;
    return true;
}

AIGroup* Market::DealerGroup(int kind)
{
    if (kind < 0 || kind >= NDealerKinds)
    {
        return nullptr;
    }
    LLinkArray<AIGroup>& groups = _groups[kind];
    for (int i = groups.Size() - 1; i >= 0; i--)
    {
        if (!groups[i])
        {
            groups.Delete(i); // dead link (group dissolved) - forget it
        }
    }
    for (int i = 0; i < groups.Size(); i++)
    {
        if (groups[i]->NUnits() < MAX_UNITS_PER_GROUP)
        {
            return groups[i];
        }
    }
    // dealers are CIV-side: neutral to both campaign sides, so no captive
    // flag is needed (and the undercover layer never reads them)
    AIGroup* grp = CreateSideGroup(EnsureSideCenter("CIV"));
    if (grp)
    {
        groups.Add(grp);
    }
    return grp;
}

void Market::SpawnDealer(DealerRecord& row)
{
    if (!GWorld)
    {
        return;
    }
    if (!row.placed && !ComputeDealerSpot(row))
    {
        if (!row.warned)
        {
            LOG_WARN(Core, "Market: no dry off-road dealer spot in zone '{}' - dealer placed on the zone centre",
                     (const char*)row.zoneName);
            row.warned = true;
        }
        row.placed = true; // degrade: the zone centre stands in
    }
    AIGroup* grp = DealerGroup(row.kind);
    if (!grp)
    {
        if (!row.warned)
        {
            LOG_WARN(Core, "Market: no CIV group for the {} dealer in '{}'", KindName(row.kind),
                     (const char*)row.zoneName);
            row.warned = true;
        }
        return;
    }
    // snapshot so the new unit can be told apart (createUnit returns nothing)
    AIUnit* before[MAX_UNITS_PER_GROUP];
    for (int i = 0; i < MAX_UNITS_PER_GROUP; i++)
    {
        before[i] = grp->UnitWithID(i + 1);
    }
    ::CreateUnit(grp, _tuning.dealerClass, row.pos, RString(), 0.2f, RankPrivate);
    AIUnit* unit = nullptr;
    for (int i = 0; i < MAX_UNITS_PER_GROUP; i++)
    {
        AIUnit* u = grp->UnitWithID(i + 1);
        if (u && u != before[i])
        {
            unit = u;
            break;
        }
    }
    Person* person = unit ? unit->GetPerson() : nullptr;
    if (!person)
    {
        if (!row.warned)
        {
            LOG_WARN(Core, "Market: dealer body '{}' did not materialize in '{}'", (const char*)_tuning.dealerClass,
                     (const char*)row.zoneName);
            row.warned = true;
        }
        return;
    }
    // a shopkeeper: stays put, picks no targets
    unit->SetAIDisabled(AIUnit::DAMove | AIUnit::DATarget | AIUnit::DAAutoTarget);
    const bool first = !row.spawned;
    row.npc = person;
    row.spawned = true;
    row.respawnIn = -1;
    LOG_INFO(Core, "Market: {} dealer {} in '{}' at [{:.0f},{:.0f}]", KindName(row.kind),
             first ? "spawned" : "respawned", (const char*)row.zoneName, row.pos.X(), row.pos.Z());
}

// ---------------------------------------------------------------------------
// simulation
// ---------------------------------------------------------------------------

void Market::Simulate(float deltaT)
{
    if (!_configLoaded && ZoneRegistry::Instance().IsActive())
    {
        // a save written without our block (pre-feature) cleared us without
        // a reload; the mission config is valid for the whole mission
        LoadFromConfig();
    }
    if (!IsActive())
    {
        return;
    }
    _accum += deltaT;
    if (_accum < TickInterval)
    {
        return;
    }
    _accum = 0;
    if (!GWorld)
    {
        return;
    }
    if (!_assigned)
    {
        Assign();
    }
    for (int i = 0; i < _rows.Size(); i++)
    {
        DealerRecord& row = _rows[i];
        Person* person = dyn_cast<Person>(row.npc.GetLink());
        AIUnit* unit = person ? person->Brain() : nullptr;
        bool alive = unit && unit->GetLifeState() == AIUnit::LSAlive;
        if (alive)
        {
            row.respawnIn = -1;
            continue;
        }
        if (!row.spawned)
        {
            SpawnDealer(row);
            continue;
        }
        if (row.respawnIn < 0)
        {
            row.respawnIn = _tuning.dealerRespawnSeconds;
            LOG_INFO(Core, "Market: {} dealer in '{}' is gone - respawn in {:.0f} s", KindName(row.kind),
                     (const char*)row.zoneName, row.respawnIn);
            continue;
        }
        row.respawnIn -= TickInterval;
        if (row.respawnIn <= 0)
        {
            SpawnDealer(row);
            if (!row.npc.GetLink())
            {
                row.respawnIn = _tuning.dealerRespawnSeconds; // try again later
            }
        }
    }
}

// ---------------------------------------------------------------------------
// serialization
// ---------------------------------------------------------------------------

LSError DealerRecord::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("zone", zoneName, 1, RString()))
    PARAM_CHECK(ar.Serialize("kind", kind, 1, 0))
    PARAM_CHECK(ar.Serialize("pos", pos, 1, VZero))
    PARAM_CHECK(ar.Serialize("lotPos", lotPos, 1, VZero))
    PARAM_CHECK(ar.Serialize("placed", placed, 1, false))
    PARAM_CHECK(ar.Serialize("spawned", spawned, 1, false))
    PARAM_CHECK(ar.Serialize("respawnIn", respawnIn, 1, -1.0f))
    // the NPC ref resolves on the second load pass (SerializeRef), after the
    // world's vehicle serializer has recreated the body
    PARAM_CHECK(ar.SerializeRef("npc", npc, 1))
    return LSOK;
}

void Market::ApplyPendingLoad()
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    _rows.Clear();
    for (int r = 0; r < _pending.Size(); r++)
    {
        DealerRecord row = _pending[r];
        // rows are matched to the rebuilt zone table by NAME; a row whose
        // town is no longer a zone is dropped (config changed under the save)
        row.zoneIndex = registry.FindZoneIndex(row.zoneName);
        if (row.zoneIndex < 0 && registry.IsActive())
        {
            LOG_INFO(Core, "Market: dealer town '{}' is no longer a zone - row dropped", (const char*)row.zoneName);
            continue;
        }
        row.warned = false;
        _rows.Add(row);
    }
}

LSError Market::Serialize(ParamArchive& ar)
{
    if (ar.IsLoading() && ar.GetPass() == ParamArchive::PassFirst)
    {
        // a load never reruns InitMission; the mission config was reparsed
        // during this pass, so the stock + tuning are rebuilt here
        LoadFromConfig();
    }
    if (ar.IsSaving())
    {
        _pending = _rows;
    }
    PARAM_CHECK(ar.Serialize("assigned", _assigned, 1, false))
    PARAM_CHECK(ar.Serialize("seed", _seed, 1, 0))
    PARAM_CHECK(ar.Serialize("Dealers", _pending, 1))
    PARAM_CHECK(ar.SerializeRefs("GroupsWeapon", _groups[DKWeapon], 1))
    PARAM_CHECK(ar.SerializeRefs("GroupsVehicle", _groups[DKVehicle], 1))

    if (ar.IsSaving())
    {
        _pending.Clear();
    }
    else if (ar.GetPass() == ParamArchive::PassSecond)
    {
        // the ZoneRegistry rebuilt its zone table before this block (its
        // subclass precedes ours in World::Serialize), so rows can be
        // matched now; NPC/group refs were resolved by this pass's SerializeRef
        ApplyPendingLoad();
        _pending.Clear();
        _accum = 0;
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
