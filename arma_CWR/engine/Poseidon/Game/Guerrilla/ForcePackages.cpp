#include <Poseidon/Game/Guerrilla/ForcePackages.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp> // stricmp

#include <math.h>

namespace Poseidon::Guerrilla
{

static int MinInt(int a, int b)
{
    return a < b ? a : b;
}

static int MaxInt(int a, int b)
{
    return a > b ? a : b;
}

// ---------------------------------------------------------------------------
// names
// ---------------------------------------------------------------------------

const char* ForcePackages::OverrideKey(int package)
{
    switch (package)
    {
        case FPTruck:
            return "forcesTruck";
        case FPArmedCar:
            return "forcesArmedCar";
        case FPApc:
            return "forcesApc";
        case FPTank:
            return "forcesTank";
        case FPTransportHeli:
            return "forcesTransportHeli";
        case FPAttackHeli:
            return "forcesAttackHeli";
        case FPJet:
            return "forcesJet";
        default:
            return "";
    }
}

const char* ForcePackages::PackageName(int package)
{
    switch (package)
    {
        case FPFoot:
            return "FOOT";
        case FPTruck:
            return "TRUCK";
        case FPArmedCar:
            return "ARMED_CAR";
        case FPApc:
            return "APC";
        case FPTank:
            return "TANK";
        case FPTransportHeli:
            return "TRANSPORT_HELI";
        case FPAttackHeli:
            return "ATTACK_HELI";
        case FPJet:
            return "JET";
        default:
            return "";
    }
}

const char* ForcePackages::KindName(int kind)
{
    switch (kind)
    {
        case CAKRetake:
            return "RETAKE";
        case CAKBase:
            return "BASE";
        case CAKHqAssault:
            return "HQ_ASSAULT";
        case CAKAlarm:
            return "ALARM";
        case CAKReinforce:
            return "REINFORCE";
        case CAKStrike:
            return "STRIKE";
        default:
            return "";
    }
}

const char* ForcePackages::OriginName(int origin)
{
    switch (origin)
    {
        case CAOLegacy:
            return "LEGACY";
        case CAODirector:
            return "DIRECTOR";
        case CAOPushout:
            return "PUSHOUT";
        case CAOWave:
            return "WAVE";
        case CAOAlarm:
            return "ALARM";
        default:
            return "";
    }
}

// ---------------------------------------------------------------------------
// classification and the catalogue
// ---------------------------------------------------------------------------

bool ForcePackages::IsAir(const HullFacts& f)
{
    return stricmp(f.simulation, "helicopter") == 0 || stricmp(f.simulation, "airplane") == 0;
}

int ForcePackages::Classify(const HullFacts& f)
{
    if (!f.exists || IsAir(f))
    {
        return -1; // air must be authored (the descriptor keys)
    }
    const bool tracked = stricmp(f.simulation, "tank") == 0;
    const bool wheeled = stricmp(f.simulation, "car") == 0;
    if (!tracked && !wheeled)
    {
        return -1; // a man, a static, a boat, a motorcycle
    }
    const bool armour = tracked || f.armor >= WheeledArmour;
    // APC before Tank: a BMP (gun + transportSoldier 8) is an APC
    if (armour && f.crewWeapon && f.cargo >= ApcCargo)
    {
        return FPApc;
    }
    if (tracked && f.crewWeapon)
    {
        return FPTank;
    }
    if (wheeled && f.crewWeapon)
    {
        return FPArmedCar;
    }
    if (!f.crewWeapon && f.cargo >= TruckCargo)
    {
        return FPTruck;
    }
    return -1;
}

ForceHull ForcePackages::HullOf(const char* className, const HullFacts& f)
{
    ForceHull h;
    h.className = className;
    h.maxSpeedKmh = f.maxSpeedKmh;
    h.cargo = f.cargo;
    h.crew = f.crew > 0 ? f.crew : 1; // a hull always needs a driver or a pilot
    return h;
}

void ForcePackages::BuildCatalogue(const AutoArray<RString>& ladder, const RString overrides[NForcePackages],
                                   const HullInspector& inspector, ForcePackageCatalogue& out, bool log)
{
    out = ForcePackageCatalogue();
    for (int i = 0; i < ladder.Size(); i++)
    {
        HullFacts f;
        if (ladder[i].GetLength() == 0 || !inspector.Inspect(ladder[i], f))
        {
            continue;
        }
        const int p = Classify(f);
        if (p < 0 || out.hulls[p].className.GetLength() > 0)
        {
            continue; // unclassified, or the package already has its (lower-rung) hull
        }
        out.hulls[p] = HullOf(ladder[i], f);
    }
    if (overrides)
    {
        for (int p = FPTruck; p < NForcePackages; p++)
        {
            if (overrides[p].GetLength() == 0)
            {
                continue;
            }
            HullFacts f;
            if (!inspector.Inspect(overrides[p], f))
            {
                if (log)
                {
                    LOG_WARN(Core, "OccupierForces: {} = \"{}\" names no vehicle class, ignored", OverrideKey(p),
                             (const char*)overrides[p]);
                }
                continue;
            }
            out.hulls[p] = HullOf(overrides[p], f);
        }
    }
    if (log)
    {
        for (int p = FPTruck; p < NForcePackages; p++)
        {
            if (out.hulls[p].className.GetLength() > 0)
            {
                LOG_INFO(Core, "OccupierForces: package {} uses {} (cargo {}, crew {}, {:.0f} km/h)", PackageName(p),
                         (const char*)out.hulls[p].className, out.hulls[p].cargo, out.hulls[p].crew,
                         (double)out.hulls[p].maxSpeedKmh);
            }
        }
    }
}

// a weapon takes a magazine: its own magazines[], or a muzzle's
static bool WeaponTakesMagazine(const ParamEntry* weapons, const char* name)
{
    if (!weapons || !name || !*name)
    {
        return false;
    }
    const ParamEntry* w = weapons->FindEntry(name);
    if (!w || !w->IsClass())
    {
        return false;
    }
    const ParamEntry* mags = w->FindEntry("magazines");
    if (mags && mags->IsArray() && mags->GetSize() > 0)
    {
        return true;
    }
    const ParamEntry* muzzles = w->FindEntry("muzzles");
    if (muzzles && muzzles->IsArray())
    {
        for (int i = 0; i < muzzles->GetSize(); i++)
        {
            const RString muzzle = RString((RStringB)(*muzzles)[i]);
            if (stricmp(muzzle, "this") == 0)
            {
                continue;
            }
            const ParamEntry* m = w->FindEntry(muzzle);
            const ParamEntry* mm = m && m->IsClass() ? m->FindEntry("magazines") : nullptr;
            if (mm && mm->IsArray() && mm->GetSize() > 0)
            {
                return true;
            }
        }
    }
    return false;
}

bool ConfigHullInspector::Inspect(const char* className, HullFacts& out) const
{
    out = HullFacts();
    if (!className || !*className)
    {
        return false;
    }
    const ParamEntry* vehicles = Pars.FindEntry("CfgVehicles");
    const ParamEntry* c = vehicles ? vehicles->FindEntry(className) : nullptr;
    if (!c || !c->IsClass())
    {
        return false;
    }
    out.exists = true;
    out.simulation = c->ReadValue("simulation", RString());
    out.cargo = (int)c->ReadValue("transportSoldier", 0.0f);
    out.maxSpeedKmh = c->ReadValue("maxSpeed", 0.0f);
    out.armor = c->ReadValue("armor", 0.0f);
    out.crew = (c->ReadValue("hasDriver", 1.0f) != 0.0f ? 1 : 0) + (c->ReadValue("hasGunner", 0.0f) != 0.0f ? 1 : 0) +
               (c->ReadValue("hasCommander", 0.0f) != 0.0f ? 1 : 0);
    const ParamEntry* weapons = c->FindEntry("weapons");
    const ParamEntry* cfgWeapons = Pars.FindEntry("CfgWeapons");
    if (weapons && weapons->IsArray())
    {
        for (int i = 0; i < weapons->GetSize() && !out.crewWeapon; i++)
        {
            out.crewWeapon = WeaponTakesMagazine(cfgWeapons, RString((RStringB)(*weapons)[i]));
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// prices, lethality, speeds, budgets
// ---------------------------------------------------------------------------

int ForcePackages::HullPrice(int package, bool offMap)
{
    int price = 0;
    switch (package)
    {
        case FPTruck:
            return 3;
        case FPArmedCar:
            return 4;
        case FPApc:
            return 8;
        case FPTank:
            return 14;
        case FPTransportHeli:
            price = 10;
            break;
        case FPAttackHeli:
            price = 18;
            break;
        case FPJet:
            price = 22;
            break;
        default:
            return 0;
    }
    return offMap ? (int)floorf((float)price * OffMapPriceFactor + 0.5f) : price;
}

float ForcePackages::LethalityAdd(int package)
{
    switch (package)
    {
        case FPArmedCar:
            return 3.0f;
        case FPApc:
            return 6.0f;
        case FPTank:
            return 12.0f;
        case FPAttackHeli:
            return 14.0f;
        case FPJet:
            return 16.0f;
        default:
            return 0.0f;
    }
}

bool ForcePackages::CarriesInfantry(int package)
{
    return package == FPTruck || package == FPApc || package == FPTransportHeli;
}

float ForcePackages::CarriedFactor(int package)
{
    return CarriesInfantry(package) ? 1.25f : 1.0f;
}

float ForcePackages::TierFactor(int tierIndex)
{
    const int t = tierIndex < 0 ? 0 : (tierIndex > 2 ? 2 : tierIndex);
    return 1.0f + 0.25f * (float)t;
}

int ForcePackages::CrewOf(int package, const ForcePackageCatalogue& cat)
{
    if (package <= FPFoot || package >= NForcePackages || !cat.Offered(package))
    {
        return 0;
    }
    return cat.hulls[package].crew > 0 ? cat.hulls[package].crew : 1;
}

float ForcePackages::SpeedOf(int package, const ForcePackageCatalogue& cat)
{
    if (package <= FPFoot || package >= NForcePackages || !cat.Offered(package))
    {
        return FootSpeed;
    }
    const float kmh = cat.hulls[package].maxSpeedKmh;
    if (kmh <= 0)
    {
        return FootSpeed; // a hull with no config speed moves at a walk
    }
    const bool air = package == FPTransportHeli || package == FPAttackHeli || package == FPJet;
    return (air ? AirCruise : GroundCruise) * kmh / 3.6f;
}

float ForcePackages::Lethality(int package, int bodies, float tierFactor)
{
    return (float)(bodies > 0 ? bodies : 0) * tierFactor * CarriedFactor(package) + LethalityAdd(package);
}

int ForcePackages::GroupsOf(int package)
{
    return package == FPFoot ? 1 : 2; // the infantry group plus the crewed hull's group
}

bool ForcePackages::WithinBudgets(int package, int occGroups, int liveHulls, int groupBudget, int liveHullsMax)
{
    if (groupBudget > 0 && occGroups + GroupsOf(package) > groupBudget)
    {
        return false;
    }
    if (HullsOf(package) > 0 && liveHulls + HullsOf(package) > liveHullsMax)
    {
        return false;
    }
    return true;
}

float ForcePackages::RequiredLethality(const PackageRequest& req)
{
    const float f = req.fighters > 0 ? req.fighters : 0;
    return req.margin * req.defenderWeight * f;
}

// ---------------------------------------------------------------------------
// the choice
// ---------------------------------------------------------------------------

PackageQuote ForcePackages::Quote(int package, const PackageRequest& req, const ForcePackageCatalogue& cat,
                                  const AutoArray<PackageSource>& sources)
{
    PackageQuote q;
    q.package = package;
    q.speed = SpeedOf(package, cat);
    q.hullClass = package > FPFoot && package < NForcePackages ? cat.hulls[package].className : RString();
    const float tf = req.tierFactor > 0 ? req.tierFactor : 1.0f;
    const float lReq = RequiredLethality(req);
    const float mult = CarriedFactor(package);
    const float add = LethalityAdd(package);
    int need = req.minBodies > 0 ? req.minBodies : 1;
    if (lReq > add)
    {
        need = MaxInt(need, (int)ceilf((lReq - add) / (tf * mult) - 1e-4f));
    }
    // infantry rides in the hull: one group at most, and no more than its
    // cargo (an armed car or a tank carries what its cargo seats hold; phase
    // 5 revisits armour escorting infantry on foot)
    const int cap = package == FPFoot ? MaxGroup : MinInt(MaxGroup, cat.hulls[package].cargo);
    // the nearest base (by travel time at this package's speed, so by
    // distance) whose spare reaches the minimum
    int best = -1;
    for (int i = 0; i < sources.Size(); i++)
    {
        const PackageSource& s = sources[i];
        if (s.zone < 0 || s.SpareFor(package) < req.minBodies)
        {
            continue;
        }
        if (best < 0 || s.distance < sources[best].distance ||
            (s.distance == sources[best].distance && s.zone < sources[best].zone))
        {
            best = i;
        }
    }
    q.crew = CrewOf(package, cat);
    q.hullPrice = HullPrice(package);
    q.poolPrice = q.hullPrice + q.crew;
    if (best < 0 || cap < req.minBodies)
    {
        q.source = -1;
        q.price = q.poolPrice;
        return q;
    }
    q.source = sources[best].zone;
    q.bodies = MinInt(MinInt(need, cap), sources[best].SpareFor(package));
    q.price = q.bodies + q.poolPrice;
    q.lethality = Lethality(package, q.bodies, tf);
    q.lethal = q.lethality + 1e-4f >= lReq;
    q.affordable = !req.poolActive || q.poolPrice <= req.pool;
    q.seconds = q.speed > 0 ? sources[best].distance / q.speed : 0;
    return q;
}

PackageChoice ForcePackages::Choose(const PackageRequest& req, const ForcePackageCatalogue& cat,
                                    const AutoArray<PackageSource>& sources)
{
    PackageChoice c;
    c.lReq = RequiredLethality(req);
    bool anySource = false;
    bool anyAffordable = false;
    for (int p = 0; p < NForcePackages; p++)
    {
        if ((req.offered & Bit(p)) == 0 || !cat.Offered(p))
        {
            continue;
        }
        const PackageQuote q = Quote(p, req, cat, sources);
        c.quotes[c.nQuotes++] = q;
        anySource = anySource || q.source >= 0;
        anyAffordable = anyAffordable || (q.source >= 0 && q.affordable);
    }
    auto valid = [&req](const PackageQuote& q) { return q.source >= 0 && q.bodies >= req.minBodies && q.affordable; };
    // the cheapest package meeting the minimum and L_req
    int cheapest = -1;
    for (int i = 0; i < c.nQuotes; i++)
    {
        const PackageQuote& q = c.quotes[i];
        if (!valid(q) || !q.lethal)
        {
            continue;
        }
        if (cheapest < 0 || q.price < c.quotes[cheapest].price ||
            (q.price == c.quotes[cheapest].price && q.seconds < c.quotes[cheapest].seconds))
        {
            cheapest = i;
        }
    }
    if (cheapest >= 0)
    {
        int pick = cheapest;
        if (req.urgent)
        {
            // the fastest package costing at most share x the cheapest
            const float ceiling = req.urgentCostShare * (float)c.quotes[cheapest].price + 1e-4f;
            for (int i = 0; i < c.nQuotes; i++)
            {
                const PackageQuote& q = c.quotes[i];
                if (!valid(q) || !q.lethal || (float)q.price > ceiling)
                {
                    continue;
                }
                if (q.seconds < c.quotes[pick].seconds ||
                    (q.seconds == c.quotes[pick].seconds && q.price < c.quotes[pick].price))
                {
                    pick = i;
                }
            }
        }
        c.ok = true;
        c.quote = c.quotes[pick];
        return c;
    }
    // nothing meets L_req: the most lethal affordable package goes
    int lethal = -1;
    for (int i = 0; i < c.nQuotes; i++)
    {
        const PackageQuote& q = c.quotes[i];
        if (!valid(q))
        {
            continue;
        }
        if (lethal < 0 || q.lethality > c.quotes[lethal].lethality ||
            (q.lethality == c.quotes[lethal].lethality && q.price < c.quotes[lethal].price))
        {
            lethal = i;
        }
    }
    if (lethal >= 0)
    {
        c.ok = true;
        c.shortOfLethality = true;
        c.quote = c.quotes[lethal];
        return c;
    }
    c.ok = false;
    if (!anySource)
    {
        c.why = "no occupier base can spare the minimum";
    }
    else if (!anyAffordable)
    {
        c.why = "the pool cannot pay for a hull";
    }
    else
    {
        c.why = "no package carries the minimum";
    }
    return c;
}

} // namespace Poseidon::Guerrilla
