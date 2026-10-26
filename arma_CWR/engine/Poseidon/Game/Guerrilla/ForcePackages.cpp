#include <Poseidon/Game/Guerrilla/ForcePackages.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Game/Guerrilla/DirectorLog.hpp> // GM_DIRECTOR_INFO (the package catalogue line)
#include <Poseidon/Foundation/platform.hpp> // stricmp

#include <math.h>
#include <stdio.h> // snprintf (the refusal names the shortfall)

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
                GM_DIRECTOR_INFO(Core, "OccupierForces: package {} uses {} (cargo {}, crew {}, {:.0f} km/h)", PackageName(p),
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

int ForcePackages::SeatCap(int package, const ForcePackageCatalogue& cat)
{
    if (package == FPFoot)
    {
        return MaxGroup;
    }
    if (package < FPFoot || package >= NForcePackages || !cat.Offered(package))
    {
        return 0;
    }
    if (package == FPAttackHeli || package == FPJet)
    {
        return 0; // a strike airframe inserts nobody (6.5: strikes are CAKStrike orders)
    }
    const int cargo = cat.hulls[package].cargo;
    return MinInt(MaxGroup, cargo > 0 ? cargo : 0);
}

int ForcePackages::EdgeRole(int package)
{
    switch (package)
    {
        case FPTruck:
            return FERHoldBack;
        case FPArmedCar:
        case FPApc:
        case FPTank:
            return FERSupport;
        default:
            return FERNone;
    }
}

bool ForcePackages::MustDowngrade(int package, int bodies, int storedSpare, int spare)
{
    if (!IsGroundHull(package) || bodies < 1)
    {
        return false;
    }
    return storedSpare < bodies && spare >= bodies;
}

PackageQuote ForcePackages::Downgrade(const PackageQuote& q, float tierFactor, float lReq, int& refund)
{
    refund = 0;
    if (q.package == FPFoot || !IsGroundHull(q.package))
    {
        return q;
    }
    PackageQuote f = q;
    refund = q.poolPrice > 0 ? q.poolPrice : 0; // carrier and escort, both crews
    // the same leg on foot (the road quote's travel; a pre-road quote has
    // only its seconds)
    const float distance = q.travel > 0 ? q.travel : q.seconds * q.speed;
    const float tf = tierFactor > 0 ? tierFactor : 1.0f;
    f.package = FPFoot;
    f.crew = 0;
    f.hullPrice = 0;
    f.poolPrice = 0;
    f.hullClass = RString();
    f.seatsShort = false;
    f.escort = -1;
    f.escortCrew = 0;
    f.escortHullPrice = 0;
    f.escortClass = RString();
    f.bodies = MinInt(q.bodies, MaxGroup);
    f.price = f.bodies;
    f.lethality = Lethality(FPFoot, f.bodies, tf);
    f.lethal = f.lethality + 1e-4f >= lReq;
    f.affordable = true; // foot costs the pool nothing
    f.speed = FootSpeed;
    f.travel = distance;
    f.seconds = distance / FootSpeed;
    return f;
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

float ForcePackages::ColumnLethality(int carrier, int escort, int bodies, float tierFactor)
{
    return Lethality(carrier, bodies, tierFactor) + (escort >= 0 ? LethalityAdd(escort) : 0.0f);
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
// the road quote (6.5, 2026-10-16)
// ---------------------------------------------------------------------------

namespace
{
float& TheatreRoadFactor()
{
    static float factor = ForcePackages::DefaultRoadFactor;
    return factor;
}
} // namespace

float ForcePackages::RoadFactor()
{
    return TheatreRoadFactor();
}

void ForcePackages::SetRoadFactor(float factor)
{
    TheatreRoadFactor() = factor < MinRoadFactor ? MinRoadFactor : (factor > MaxRoadFactor ? MaxRoadFactor : factor);
}

float ForcePackages::GroundLeg(float straightDistance, float roadLength, float factor)
{
    if (roadLength > 0)
    {
        return roadLength; // a real road route already is the road: no factor on top
    }
    const float straight = straightDistance > 0 ? straightDistance : 0;
    return straight * (factor > MinRoadFactor ? factor : MinRoadFactor);
}

const char* ForcePackages::ReasonWords(int reason)
{
    switch (reason)
    {
        case PackageChoice::ReasonGroundCheapest:
            return "ground: cheapest under slowEta";
        case PackageChoice::ReasonGroundSlow:
            return "ground: slow but no air case";
        case PackageChoice::ReasonUrgentShare:
            return "urgent: fastest within forcesUrgentCostShare";
        case PackageChoice::ReasonUrgentSlow:
            return "urgent: fastest under slowEta";
        case PackageChoice::ReasonAirWater:
            return "air: across water";
        case PackageChoice::ReasonAirEta:
            return "air: past forcesAirEta";
        case PackageChoice::ReasonAirFighters:
            return "air: fighters reported";
        default:
            return "";
    }
}

float ForcePackages::TravelOf(int package, const PackageSource& s, const PackageRequest& req, int* eta, float* factor)
{
    const float straight = s.distance > 0 ? s.distance : 0;
    int kind = FEQLine;
    float f = 1.0f;
    float travel = straight;
    if (!IsAirPackage(package))
    {
        if (s.road > 0)
        {
            kind = FEQRoad;
            travel = s.road;
        }
        else if (req.roadFactor > 1.0f + 1e-4f)
        {
            kind = FEQLineFactor;
            f = req.roadFactor;
            travel = GroundLeg(straight, -1.0f, f);
        }
    }
    if (eta)
    {
        *eta = kind;
    }
    if (factor)
    {
        *factor = f;
    }
    return travel;
}

RString ForcePackages::EtaNote(const PackageQuote& q)
{
    char text[48];
    const double seconds = q.seconds > 0 ? (double)q.seconds : 0.0;
    if (q.eta == FEQRoad)
    {
        snprintf(text, sizeof(text), "road %.0f s", seconds);
    }
    else if (q.eta == FEQLineFactor)
    {
        snprintf(text, sizeof(text), "line x%.1f %.0f s", (double)q.etaFactor, seconds);
    }
    else
    {
        snprintf(text, sizeof(text), "line %.0f s", seconds);
    }
    return RString(text);
}

namespace
{
struct NoteRing
{
    ForcePackages::EtaNoteRow rows[ForcePackages::EtaNoteRing];
    int next = 0;  // the slot the next note takes
    int count = 0; // at most EtaNoteRing
};

NoteRing& Notes()
{
    static NoteRing ring;
    return ring;
}
} // namespace

RString ForcePackages::LogEtaNote(const PackageQuote& q)
{
    const RString text = EtaNote(q);
    NoteRing& ring = Notes();
    EtaNoteRow& row = ring.rows[ring.next];
    row.text = text;
    row.package = q.package;
    row.source = q.source;
    row.eta = q.eta;
    row.seconds = q.seconds;
    ring.next = (ring.next + 1) % EtaNoteRing;
    ring.count = MinInt(ring.count + 1, EtaNoteRing);
    return text;
}

int ForcePackages::EtaNoteCount()
{
    return Notes().count;
}

const ForcePackages::EtaNoteRow& ForcePackages::EtaNoteAt(int i)
{
    const NoteRing& ring = Notes();
    const int n = ring.count;
    const int k = i < 0 ? 0 : (i >= n ? n - 1 : i);
    const int first = (ring.next - n + EtaNoteRing) % EtaNoteRing;
    return ring.rows[(first + (k < 0 ? 0 : k)) % EtaNoteRing];
}

void ForcePackages::ClearEtaNotes()
{
    Notes() = NoteRing();
}

// ---------------------------------------------------------------------------
// the choice
// ---------------------------------------------------------------------------

PackageQuote ForcePackages::Quote(int package, const PackageRequest& req, const ForcePackageCatalogue& cat,
                                  const AutoArray<PackageSource>& sources)
{
    return Quote(package, -1, req, cat, sources);
}

PackageQuote ForcePackages::Quote(int package, int escort, const PackageRequest& req, const ForcePackageCatalogue& cat,
                                  const AutoArray<PackageSource>& sources)
{
    PackageQuote q;
    q.package = package;
    q.speed = SpeedOf(package, cat);
    q.hullClass = package > FPFoot && package < NForcePackages ? cat.hulls[package].className : RString();
    if (escort >= 0)
    {
        q.escort = escort;
        if (!IsCarrier(package) || !IsEscort(escort) || !cat.Offered(package) || !cat.Offered(escort))
        {
            q.source = -1; // not a column
            return q;
        }
        q.escortClass = cat.hulls[escort].className;
        q.escortCrew = CrewOf(escort, cat);
        q.escortHullPrice = HullPrice(escort);
        const float es = SpeedOf(escort, cat);
        q.speed = es < q.speed ? es : q.speed; // the slower hull sets the pace (3.2)
    }
    const float tf = req.tierFactor > 0 ? req.tierFactor : 1.0f;
    const float lReq = RequiredLethality(req);
    const float mult = CarriedFactor(package);
    const float add = LethalityAdd(package) + (escort >= 0 ? LethalityAdd(escort) : 0.0f);
    int need = req.minBodies > 0 ? req.minBodies : 1;
    if (lReq > add)
    {
        need = MaxInt(need, (int)ceilf((lReq - add) / (tf * mult) - 1e-4f));
    }
    // infantry rides in the hull: one group at most, and no more than its
    // cargo seats (SeatCap).  An armed car or a tank whose seats cannot hold
    // the minimum is quoted (diagnostics) but never chosen.
    const int cap = SeatCap(package, cat);
    // the nearest base by travel time at this package's speed (so by its
    // leg: the road route for a ground package, the straight line for air)
    // whose spare covers the bodies the package needs to meet L_req (2026-10-16
    // phase 1 C: a defended target takes the Airfield's 9 rather than the
    // nearer Outpost's 5); when none does, the nearest whose spare reaches
    // the minimum (the quote is then short of L_req)
    const int full = MinInt(need, cap);
    int best = -1;
    float bestTravel = 0;
    for (int pass = 0; pass < 2 && best < 0; pass++)
    {
        const int bar = pass == 0 ? MaxInt(full, req.minBodies) : req.minBodies;
        if (pass == 1 && bar >= MaxInt(full, req.minBodies))
        {
            break; // the same bar as the first pass: nothing more to find
        }
        for (int i = 0; i < sources.Size(); i++)
        {
            const PackageSource& s = sources[i];
            if (s.zone < 0 || s.SpareFor(package) < bar || !s.Serves(package))
            {
                continue; // an air-only base (another landmass) sources only a helicopter insertion
            }
            const float travel = TravelOf(package, s, req);
            if (best < 0 || travel < bestTravel || (travel == bestTravel && s.zone < sources[best].zone))
            {
                best = i;
                bestTravel = travel;
            }
        }
    }
    q.crew = CrewOf(package, cat);
    const bool air = IsAirPackage(package);
    q.hullPrice = HullPrice(package, air && req.airOffMap);
    q.poolPrice = q.hullPrice + q.crew + q.EscortPoolPrice();
    q.seatsShort = cap < req.minBodies;
    if (best < 0 || q.seatsShort)
    {
        q.source = -1;
        q.price = q.poolPrice;
        return q;
    }
    q.source = sources[best].zone;
    q.bodies = MinInt(MinInt(need, cap), sources[best].SpareFor(package));
    q.price = q.bodies + q.poolPrice;
    q.lethality = ColumnLethality(package, escort, q.bodies, tf);
    q.lethal = q.lethality + 1e-4f >= lReq;
    q.affordable = !req.poolActive || q.poolPrice <= req.pool;
    q.travel = TravelOf(package, sources[best], req, &q.eta, &q.etaFactor);
    q.seconds = q.speed > 0 ? q.travel / q.speed : 0;
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
    // the columns: an offered carrier with one offered escort (phase 5)
    for (int p = 0; p < NForcePackages; p++)
    {
        if (!IsCarrier(p) || (req.offered & Bit(p)) == 0 || !cat.Offered(p))
        {
            continue;
        }
        for (int e = 0; e < NForcePackages && c.nQuotes < PackageChoice::MaxQuotes; e++)
        {
            if (!IsEscort(e) || (req.offered & Bit(e)) == 0 || !cat.Offered(e))
            {
                continue;
            }
            const PackageQuote q = Quote(p, e, req, cat, sources);
            c.quotes[c.nQuotes++] = q;
            anySource = anySource || q.source >= 0;
            anyAffordable = anyAffordable || (q.source >= 0 && q.affordable);
        }
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
    const bool rule = req.slowEta > 0 || req.airEta > 0;
    if (cheapest >= 0 && !rule)
    {
        // the plain choice (the arrival rule off: the pure chooser, wave
        // strikes): the cheapest lethal package; URGENT takes the fastest
        // within forcesUrgentCostShare x its price
        int pick = cheapest;
        const PackageQuote& cq = c.quotes[cheapest];
        c.cheapestPackage = cq.package;
        c.cheapestSeconds = cq.seconds;
        c.cheapestIndex = cheapest;
        if (req.urgent)
        {
            const float ceiling = req.urgentCostShare * (float)cq.price + 1e-4f;
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
    if (cheapest >= 0)
    {
        // THE ARRIVAL RULE (6.5; reworked 2026-10-16, phase 1 C).  T: the
        // threshold a ground arrival must meet (slowEta, else airEta)
        const float threshold = req.slowEta > 0 ? req.slowEta : req.airEta;
        const float bound = req.slowCostShare > 1.0f ? req.slowCostShare : 1.0f;
        // the yardsticks: the cheapest lethal ground package, the fastest
        // lethal ground package, the walk (the foot quote), any ground quote
        // at all, and the cheapest lethal insertion
        int ground = -1;
        int fastest = -1;
        int heli = -1;
        int walk = -1;
        bool anyGround = false;
        for (int i = 0; i < c.nQuotes; i++)
        {
            const PackageQuote& q = c.quotes[i];
            if (q.package == FPFoot && !q.HasEscort() && q.source >= 0 && q.bodies >= req.minBodies)
            {
                walk = i; // lethal or not: how long the infantry would walk
            }
            if (!valid(q))
            {
                continue;
            }
            if (IsAirPackage(q.package))
            {
                if (q.package == FPTransportHeli && q.lethal && (heli < 0 || q.price < c.quotes[heli].price))
                {
                    heli = i;
                }
                continue;
            }
            anyGround = true;
            if (!q.lethal)
            {
                continue;
            }
            if (ground < 0 || q.price < c.quotes[ground].price ||
                (q.price == c.quotes[ground].price && q.seconds < c.quotes[ground].seconds))
            {
                ground = i;
            }
            if (fastest < 0 || q.seconds < c.quotes[fastest].seconds ||
                (q.seconds == c.quotes[fastest].seconds && q.price < c.quotes[fastest].price))
            {
                fastest = i;
            }
        }
        // the line's "the cheapest": the cheapest lethal ground package, or
        // the cheapest lethal package of all when no ground one is lethal
        const int shown = ground >= 0 ? ground : cheapest;
        c.cheapestPackage = c.quotes[shown].package;
        c.cheapestSeconds = c.quotes[shown].seconds;
        c.cheapestIndex = shown;
        c.walkSeconds = walk >= 0 ? c.quotes[walk].seconds : -1.0f;
        c.groundSeconds = fastest >= 0 ? c.quotes[fastest].seconds : -1.0f;
        // the air cases: across water (no ground package can reach, the
        // insertion's infantry stands on another landmass), past airEta by
        // the fastest lethal ground package (or none is lethal), or fighters
        // reported on a target out of walking reach
        bool heliAcross = false;
        if (heli >= 0)
        {
            for (int i = 0; i < sources.Size(); i++)
            {
                if (sources[i].zone == c.quotes[heli].source)
                {
                    heliAcross = sources[i].airOnly;
                    break;
                }
            }
        }
        const bool across = heli >= 0 && !anyGround && heliAcross;
        const bool pastAir = req.airEta > 0 && (fastest < 0 || c.quotes[fastest].seconds > req.airEta);
        const bool farWalk = walk < 0 || c.quotes[walk].seconds > threshold;
        const bool defended = req.fighters + 1e-4f >= (float)AirFightersMin;
        const bool fighters = defended && (farWalk || ground < 0);
        if (heli >= 0 && (across || pastAir || fighters))
        {
            c.reason = across ? PackageChoice::ReasonAirWater
                              : (pastAir ? PackageChoice::ReasonAirEta : PackageChoice::ReasonAirFighters);
            c.etaRule = PackageChoice::EtaAir;
            c.ok = true;
            c.quote = c.quotes[heli];
            return c;
        }
        if (ground >= 0)
        {
            const PackageQuote& cq = c.quotes[ground];
            int pick = ground;
            if (cq.seconds <= threshold)
            {
                // the cheapest meets T: it goes; URGENT, the fastest ground
                // package within forcesUrgentCostShare x its price
                c.reason = PackageChoice::ReasonGroundCheapest;
                if (req.urgent)
                {
                    const float ceiling = req.urgentCostShare * (float)cq.price + 1e-4f;
                    for (int i = 0; i < c.nQuotes; i++)
                    {
                        const PackageQuote& q = c.quotes[i];
                        if (!valid(q) || !q.lethal || IsAirPackage(q.package) || (float)q.price > ceiling)
                        {
                            continue;
                        }
                        if (q.seconds < c.quotes[pick].seconds ||
                            (q.seconds == c.quotes[pick].seconds && q.price < c.quotes[pick].price))
                        {
                            pick = i;
                        }
                    }
                    if (pick != ground)
                    {
                        c.reason = PackageChoice::ReasonUrgentShare;
                    }
                }
            }
            else
            {
                // slow: the CHEAPEST lethal ground package that meets T, at
                // most bound x the cheapest (URGENT: the FASTEST meeting T);
                // none: the fastest within the bound
                const float ceiling = bound * (float)cq.price + 1e-4f;
                int meet = -1;
                int meetFast = -1;
                int inBoundFast = -1;
                for (int i = 0; i < c.nQuotes; i++)
                {
                    const PackageQuote& q = c.quotes[i];
                    if (!valid(q) || !q.lethal || IsAirPackage(q.package) || (float)q.price > ceiling)
                    {
                        continue;
                    }
                    if (inBoundFast < 0 || q.seconds < c.quotes[inBoundFast].seconds ||
                        (q.seconds == c.quotes[inBoundFast].seconds && q.price < c.quotes[inBoundFast].price))
                    {
                        inBoundFast = i;
                    }
                    if (q.seconds > threshold)
                    {
                        continue;
                    }
                    if (meet < 0 || q.price < c.quotes[meet].price ||
                        (q.price == c.quotes[meet].price && q.seconds < c.quotes[meet].seconds))
                    {
                        meet = i;
                    }
                    if (meetFast < 0 || q.seconds < c.quotes[meetFast].seconds ||
                        (q.seconds == c.quotes[meetFast].seconds && q.price < c.quotes[meetFast].price))
                    {
                        meetFast = i;
                    }
                }
                if (meet >= 0)
                {
                    pick = req.urgent ? meetFast : meet;
                    c.reason = pick != meet ? PackageChoice::ReasonUrgentSlow : PackageChoice::ReasonGroundCheapest;
                }
                else
                {
                    pick = inBoundFast >= 0 ? inBoundFast : ground;
                    c.reason = PackageChoice::ReasonGroundSlow;
                }
                if (pick != ground)
                {
                    c.etaRule = PackageChoice::EtaSlow;
                }
            }
            c.ok = true;
            c.quote = c.quotes[pick];
            return c;
        }
        // no lethal ground package and no air case: the fallback below
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
        // name the shortfall: the minimum against the largest spare one
        // source offers to a package it can serve
        int largest = 0;
        for (int i = 0; i < sources.Size(); i++)
        {
            const PackageSource& s = sources[i];
            if (s.zone < 0)
            {
                continue;
            }
            for (int p = 0; p < NForcePackages; p++)
            {
                if ((req.offered & Bit(p)) != 0 && cat.Offered(p) && SeatCap(p, cat) > 0 && s.Serves(p))
                {
                    largest = MaxInt(largest, s.SpareFor(p));
                }
            }
        }
        c.largestSpare = largest;
        const int minimum = req.minBodies > 0 ? req.minBodies : 1;
        char why[160];
        snprintf(why, sizeof(why),
                 "no occupier base can spare the minimum (%d needed, the largest single-base spare is %d: short %d)",
                 minimum, largest, minimum > largest ? minimum - largest : 0);
        c.why = why;
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
