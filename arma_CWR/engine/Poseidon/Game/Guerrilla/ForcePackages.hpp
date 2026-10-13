#pragma once

// Guerrilla Mode occupier force packages (guerrilla-mode/OCCUPIER-FORCES.md
// section 6.5, decisions D1 and D2; phase 3).  The director (OccupierForces)
// buys every operation as a package: infantry on foot, or infantry carried
// by a hull, or (later phases) armour and air.  This file holds the pure
// half: the hull classification, the catalogue, the prices, the lethality
// index, the real abstract speeds (3.2) and the package choice.  The only
// world-touching piece is ConfigHullInspector, which reads the facts of a
// CfgVehicles class from the merged game config.
//
// CLASSIFICATION (6.5).  Each class of the occupier faction's vehicles[]
// ladder is inspected (HullInspector) and classified, first match wins:
//   APC        tracked or wheeled armour, transportSoldier >= 4, a crew
//              weapon (tested BEFORE Tank, so a BMP is an APC);
//   Tank       tank simulation with a crew weapon (its main gun);
//   Armed car  wheeled (car simulation) with a crew weapon;
//   Truck      a ground vehicle with transportSoldier >= 6 and no crew weapon.
// A crew weapon is a weapons[] entry that takes a magazine (a horn,
// CarHorn / TruckHorn, takes none).  Wheeled armour is a car with armor >=
// WheeledArmour.  Air is never classified from the ladder: it must be
// authored.  The descriptor keys forcesTruck, forcesArmedCar, forcesApc,
// forcesTank, forcesTransportHeli, forcesAttackHeli and forcesJet name a
// class for their package and override the ladder.  The first ladder class
// of a package is its hull; a package with no hull is not offered.  No
// war-level gate applies (D2): war level only picks the infantry tier.
//
// PRICES AND LETHALITY (6.5, pool units; one unit = one soldier).
//   body 1; truck 3; armed car 4; APC 8; tank 14; transport helicopter 10
//   (15 off-map); attack helicopter 18 (27); jet 22 (33).  Crew bodies are
//   priced as bodies on top of the hull.
//   Lethality: each infantry body is worth the tier factor (1.0 / 1.25 /
//   1.5 by infantry tier); carried infantry x 1.25 in a truck, an APC or a
//   transport helicopter; a hull adds armed car +3, APC +6, tank +12,
//   attack helicopter +14, jet +16.  Crew add no lethality.
//
// THE CHOICE (6.5).  L_req = forcesLethalityMargin x counterAttackDefenderWeight
// x knownFighters (the director's sightings, never the true census, D9).
// Minimum bodies: max(forcesRetakeMin, garrisonMax(target)) for a town or
// base retake, 4 for an alarm, 6 for an HQ assault.  Per offered package the
// infantry is sized to meet L_req (never below the minimum), capped at one
// group (or the hull's cargo) and at the source's spare; the source is the
// nearest occupier base by travel time at the package's speed whose spare
// reaches the minimum.  The cheapest package meeting both the minimum and
// L_req is chosen; an URGENT target takes the fastest package costing at
// most forcesUrgentCostShare x that cheapest one.  If no package meets
// L_req, the most lethal affordable package goes (the INFO line says so).
// Infantry is paid by the source base (its spare); the hull and its crew are
// paid by the pool (poolPrice).
//
// PHASES.  Every package is classified now, but phase 3 offers only Foot and
// Truck to the chooser (OfferedMask = Phase3Packages); phase 5 adds armour,
// phase 6 air.

#include <Poseidon/Game/Guerrilla/CounterAttackOrders.hpp>

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

namespace Poseidon
{
namespace Guerrilla
{

// What the classification needs to know about one CfgVehicles class
struct HullFacts
{
    bool exists = false;
    RString simulation;      // "car", "tank", "helicopter", "airplane", ...
    int cargo = 0;           // transportSoldier
    float maxSpeedKmh = 0;   // maxSpeed (km/h)
    float armor = 0;         // armor
    bool crewWeapon = false; // a weapons[] entry that takes a magazine
    int crew = 0;            // hasDriver + hasGunner + hasCommander
};

// The fact source.  The engine reads the merged config (ConfigHullInspector);
// unit tests feed fake facts.
class HullInspector
{
  public:
    virtual ~HullInspector() = default;
    // false (out.exists false) when the class is unknown
    virtual bool Inspect(const char* className, HullFacts& out) const = 0;
};

// Pars >> CfgVehicles >> class (engine path)
class ConfigHullInspector final : public HullInspector
{
  public:
    bool Inspect(const char* className, HullFacts& out) const override;
};

// One package's offer for a target (PackageChoice and the quote table)
struct PackageQuote
{
    int package = FPFoot;
    int source = -1;   // the zone the infantry comes from
    int bodies = 0;    // infantry, crew excluded
    int crew = 0;      // hull crew (pool-paid bodies)
    int hullPrice = 0; // pool units for the hull
    int poolPrice = 0; // hull + crew: what the pool pays
    int price = 0;     // bodies + crew + hull: the package's whole cost
    float lethality = 0;
    float speed = 0;     // m/s, the abstract leg (3.2)
    float seconds = 0;   // straight distance / speed from the source
    bool lethal = false; // lethality >= L_req
    bool affordable = false;
    RString hullClass;
};

// A base the infantry may come from
struct PackageSource
{
    int zone = -1;
    float distance = 0; // straight distance to the target (m)
    int spare = 0;      // bodies the base can spare now (CounterAttack::Spare, live or stored)
    // bodies its STORED reserve can spare: a hull package's infantry is
    // always debited from the reserve (CounterAttack::LaunchFunded); -1 =
    // the same as spare
    int spareStored = -1;
    int SpareFor(int package) const { return package != FPFoot && spareStored >= 0 ? spareStored : spare; }
};

struct PackageRequest
{
    float fighters = 0;          // knownFighters(target): sightings only (D9)
    float defenderWeight = 3.0f; // counterAttackDefenderWeight
    float margin = 1.2f;         // forcesLethalityMargin
    int minBodies = 4;
    bool urgent = false;
    float urgentCostShare = 1.5f; // forcesUrgentCostShare
    float tierFactor = 1.0f;      // 1.0 / 1.25 / 1.5
    int pool = 0;                 // pool units available for hulls and crews
    bool poolActive = true;       // false: the pool is inert (nothing is debited)
    unsigned int offered = 0;     // OfferedMask bits (ForcePackages::Phase3Packages)
};

struct PackageChoice
{
    bool ok = false;
    bool shortOfLethality = false; // no package met L_req: the most lethal affordable one went
    float lReq = 0;
    PackageQuote quote;
    int nQuotes = 0;
    PackageQuote quotes[NForcePackages]; // every package considered (diagnostics, tests)
    RString why;                         // when !ok
};

class ForcePackages
{
  public:
    // bit masks of ForcePackage values offered to the chooser
    static constexpr unsigned int Bit(int package) { return 1u << (unsigned int)package; }
    static constexpr unsigned int Phase3Packages = (1u << FPFoot) | (1u << FPTruck);
    static constexpr unsigned int AllPackages = (1u << NForcePackages) - 1u;

    // classification thresholds (6.5)
    static constexpr int TruckCargo = 6;
    static constexpr int ApcCargo = 4;
    static constexpr float WheeledArmour = 50.0f; // a car this armoured is wheeled armour
    // the largest infantry group (MAX_UNITS_PER_GROUP)
    static constexpr int MaxGroup = 12;
    // minimum bodies for the non-retake kinds (6.5)
    static constexpr int AlarmMinBodies = 4;
    static constexpr int HqMinBodies = 6;
    // abstract speeds (3.2): foot m/s, ground and air shares of maxSpeed
    static constexpr float FootSpeed = 1.5f;
    static constexpr float GroundCruise = 0.6f;
    static constexpr float AirCruise = 0.8f;
    static constexpr float OffMapPriceFactor = 1.5f;
    // the 40-group budget and forcesLiveHullsMax (6.5)
    static constexpr int GroupBudget = 40;
    static constexpr int LiveHullsMax = 6;

    // the descriptor override key of a package ("" for foot)
    static const char* OverrideKey(int package);
    static const char* PackageName(int package); // "FOOT", "TRUCK", ... (gmOccupierOps)
    static const char* KindName(int kind);       // "RETAKE", "BASE", ...
    static const char* OriginName(int origin);   // "LEGACY", "DIRECTOR", ...

    // the classification of one class from its facts: FPTruck, FPArmedCar,
    // FPApc, FPTank, or -1 (unclassified: air, unarmed small cars, statics)
    static int Classify(const HullFacts& f);
    static bool IsAir(const HullFacts& f);
    static ForceHull HullOf(const char* className, const HullFacts& f);
    // the catalogue: ladder classes in order (first per package wins), then
    // the overrides (overrides[package] a class name or empty) on top.
    // An override naming an unknown class is ignored (logged when log).
    static void BuildCatalogue(const AutoArray<RString>& ladder, const RString overrides[NForcePackages],
                               const HullInspector& inspector, ForcePackageCatalogue& out, bool log = false);

    // prices and lethality (6.5)
    static int HullPrice(int package, bool offMap = false);
    static float LethalityAdd(int package);
    static float CarriedFactor(int package); // 1.25 for carried infantry, else 1
    static bool CarriesInfantry(int package);
    static float TierFactor(int tierIndex); // 1.0 / 1.25 / 1.5 (index clamped 0..2)
    static int CrewOf(int package, const ForcePackageCatalogue& cat);
    // the abstract leg's speed (m/s): foot 1.5, ground 0.6 x maxSpeed, air 0.8 x
    static float SpeedOf(int package, const ForcePackageCatalogue& cat);
    static float Lethality(int package, int bodies, float tierFactor);
    // groups a package adds to the side's group budget: infantry 1, each
    // crewed hull 1 (a helicopter insertion: crew + cargo = 2)
    static int GroupsOf(int package);
    static int HullsOf(int package) { return package == FPFoot ? 0 : 1; }
    // a launch fits the 40-group budget and forcesLiveHullsMax
    static bool WithinBudgets(int package, int occGroups, int liveHulls, int groupBudget = GroupBudget,
                              int liveHullsMax = LiveHullsMax);

    // L_req = margin * defenderWeight * fighters
    static float RequiredLethality(const PackageRequest& req);
    // one package's quote for the sources (sorted or not); quote.source -1
    // when no source can spare the minimum
    static PackageQuote Quote(int package, const PackageRequest& req, const ForcePackageCatalogue& cat,
                              const AutoArray<PackageSource>& sources);
    // the choice (6.5)
    static PackageChoice Choose(const PackageRequest& req, const ForcePackageCatalogue& cat,
                                const AutoArray<PackageSource>& sources);
};

} // namespace Guerrilla
} // namespace Poseidon
