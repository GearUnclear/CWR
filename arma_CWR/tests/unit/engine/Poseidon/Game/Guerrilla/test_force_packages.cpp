// Occupier force packages (OCCUPIER-FORCES.md 6.5, D1 and D2; ForcePackages):
// the ladder classification for WEST, EAST and LoBo Hizballah from config
// facts (APC before Tank, horns are not weapons, air never from the ladder),
// the descriptor overrides, the price and lethality table, the real abstract
// speeds (3.2), the package choice (the cheapest lethal package, the fastest
// within the urgent cost share, the most lethal affordable fallback, the
// minimum bodies, the source's spare), both worked examples of 6.5 with the
// crew included, no war-level gate, and the group and hull budgets.  Phase 5:
// the default offer of every ground package, the armour ladders and crews,
// the seat cap, the choice with armour affordable and not, the town-edge
// role, and the downgrade to foot.  Pure:
// the class facts come from a fake inspector, not the game config.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Game/Guerrilla/ForcePackages.hpp>

#include <string.h>
#include <map>
#include <string>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

// facts read from the Remaster config (CfgVehicles, 2026-10-13) and, for the
// LoBo Hizballah ladder, the shape of its classes: weapons that take a
// magazine are crew weapons, horns (CarHorn / TruckHorn) take none
struct FakeInspector final : HullInspector
{
    std::map<std::string, HullFacts> facts;

    void Add(const char* cls, const char* sim, int cargo, float speed, float armor, bool weapon, int crew)
    {
        HullFacts f;
        f.exists = true;
        f.simulation = sim;
        f.cargo = cargo;
        f.maxSpeedKmh = speed;
        f.armor = armor;
        f.crewWeapon = weapon;
        f.crew = crew;
        facts[cls] = f;
    }
    bool Inspect(const char* className, HullFacts& out) const override
    {
        out = HullFacts();
        auto it = facts.find(className ? className : "");
        if (it == facts.end())
        {
            return false;
        }
        out = it->second;
        return true;
    }
};

FakeInspector VanillaAndLoBo()
{
    FakeInspector in;
    // WEST {Jeep, Truck5t, M113, M60}
    in.Add("Jeep", "car", 3, 100, 20, false, 1);
    in.Add("Truck5t", "car", 12, 80, 20, false, 1);
    in.Add("M113", "tank", 8, 60, 150, true, 2);
    in.Add("M60", "tank", 0, 54, 300, true, 3);
    // EAST {UAZ, Ural, BMP, T72}
    in.Add("UAZ", "car", 3, 90, 20, false, 1);
    in.Add("Ural", "car", 12, 80, 20, false, 1);
    in.Add("BMP", "tank", 8, 70, 200, true, 3);
    in.Add("T72", "tank", 0, 60, 400, true, 3);
    // LoBo Hizballah {Ural, JeepMG, Ural zsu23, JeepAT}
    in.Add("LoBo_Ural_Hizballah", "car", 12, 80, 20, false, 1);
    in.Add("LoBo_JeepMG_Hizballah", "car", 1, 100, 20, true, 2);
    in.Add("LoBo_ural_Hizbal_zsu23", "car", 0, 80, 20, true, 2);
    in.Add("LoBo_JeepAT_Hizballah", "car", 1, 100, 20, true, 2);
    // air, for the overrides
    in.Add("UH60", "helicopter", 10, 300, 40, true, 2);
    in.Add("AH1", "helicopter", 0, 280, 40, true, 2);
    in.Add("A10", "airplane", 0, 650, 60, true, 1);
    return in;
}

AutoArray<RString> Ladder(std::initializer_list<const char*> names)
{
    AutoArray<RString> out;
    for (const char* n : names)
    {
        out.Add(RString(n));
    }
    return out;
}

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

// a catalogue with a Truck5t (cargo 12, 80 km/h, one driver)
ForcePackageCatalogue WestCatalogue()
{
    const FakeInspector in = VanillaAndLoBo();
    ForcePackageCatalogue cat;
    ForcePackages::BuildCatalogue(Ladder({"Jeep", "Truck5t", "M113", "M60"}), nullptr, in, cat);
    return cat;
}

AutoArray<PackageSource> OneSource(int zone, float distance, int spare)
{
    AutoArray<PackageSource> s;
    PackageSource src;
    src.zone = zone;
    src.distance = distance;
    src.spare = spare;
    s.Add(src);
    return s;
}

} // namespace

TEST_CASE("ForcePackages - the WEST ladder: no armed car, Truck5t truck, M113 APC, M60 tank",
          "[game][guerrilla][occupierforces][packages]")
{
    const FakeInspector in = VanillaAndLoBo();
    ForcePackageCatalogue cat;
    ForcePackages::BuildCatalogue(Ladder({"Jeep", "Truck5t", "M113", "M60"}), nullptr, in, cat);
    CHECK(cat.Offered(FPFoot));
    CHECK_FALSE(cat.Offered(FPArmedCar)); // the Jeep carries only a horn
    CHECK(Str(cat.hulls[FPTruck].className) == "Truck5t");
    CHECK(Str(cat.hulls[FPApc].className) == "M113");
    CHECK(Str(cat.hulls[FPTank].className) == "M60");
    CHECK_FALSE(cat.Offered(FPTransportHeli));
    CHECK_FALSE(cat.Offered(FPAttackHeli));
    CHECK_FALSE(cat.Offered(FPJet));
    CHECK(cat.hulls[FPTruck].cargo == 12);
    CHECK(cat.hulls[FPTruck].crew == 1);
    CHECK(cat.hulls[FPTruck].maxSpeedKmh == Approx(80.0f));
    CHECK(cat.hulls[FPTank].crew == 3);
}

TEST_CASE("ForcePackages - the EAST ladder: rung 0 UAZ is no package, BMP is an APC (before Tank)",
          "[game][guerrilla][occupierforces][packages]")
{
    const FakeInspector in = VanillaAndLoBo();
    HullFacts uaz;
    REQUIRE(in.Inspect("UAZ", uaz));
    CHECK(ForcePackages::Classify(uaz) == -1);
    ForcePackageCatalogue cat;
    ForcePackages::BuildCatalogue(Ladder({"UAZ", "Ural", "BMP", "T72"}), nullptr, in, cat);
    CHECK_FALSE(cat.Offered(FPArmedCar));
    CHECK(Str(cat.hulls[FPTruck].className) == "Ural");
    CHECK(Str(cat.hulls[FPApc].className) == "BMP"); // gun + transportSoldier 8: APC, not tank
    CHECK(Str(cat.hulls[FPTank].className) == "T72");
}

TEST_CASE("ForcePackages - LoBo Hizballah: Ural truck, JeepMG / JeepAT / Ural zsu23 armed cars, no armour",
          "[game][guerrilla][occupierforces][packages]")
{
    const FakeInspector in = VanillaAndLoBo();
    const char* names[] = {"LoBo_Ural_Hizballah", "LoBo_JeepMG_Hizballah", "LoBo_ural_Hizbal_zsu23",
                           "LoBo_JeepAT_Hizballah"};
    const int expected[] = {FPTruck, FPArmedCar, FPArmedCar, FPArmedCar};
    for (int i = 0; i < 4; i++)
    {
        HullFacts f;
        REQUIRE(in.Inspect(names[i], f));
        CHECK(ForcePackages::Classify(f) == expected[i]);
    }
    ForcePackageCatalogue cat;
    ForcePackages::BuildCatalogue(Ladder({names[0], names[1], names[2], names[3]}), nullptr, in, cat);
    CHECK(Str(cat.hulls[FPTruck].className) == "LoBo_Ural_Hizballah");
    CHECK(Str(cat.hulls[FPArmedCar].className) == "LoBo_JeepMG_Hizballah"); // the lowest rung wins
    CHECK_FALSE(cat.Offered(FPApc));
    CHECK_FALSE(cat.Offered(FPTank));
}

TEST_CASE("ForcePackages - air is never classified from the ladder; the descriptor keys override",
          "[game][guerrilla][occupierforces][packages]")
{
    const FakeInspector in = VanillaAndLoBo();
    HullFacts heli;
    REQUIRE(in.Inspect("UH60", heli));
    CHECK(ForcePackages::IsAir(heli));
    CHECK(ForcePackages::Classify(heli) == -1);
    ForcePackageCatalogue cat;
    ForcePackages::BuildCatalogue(Ladder({"UH60", "Jeep", "Truck5t"}), nullptr, in, cat);
    CHECK_FALSE(cat.Offered(FPTransportHeli));

    RString overrides[NForcePackages];
    overrides[FPTruck] = "Ural";         // replaces the ladder's Truck5t
    overrides[FPArmedCar] = "M113";      // an override wins over the classification
    overrides[FPTransportHeli] = "UH60"; // air must be authored
    overrides[FPAttackHeli] = "AH1";
    overrides[FPJet] = "NoSuchJet"; // an unknown class is ignored
    ForcePackages::BuildCatalogue(Ladder({"Jeep", "Truck5t", "M113", "M60"}), overrides, in, cat);
    CHECK(Str(cat.hulls[FPTruck].className) == "Ural");
    CHECK(Str(cat.hulls[FPArmedCar].className) == "M113");
    CHECK(Str(cat.hulls[FPTransportHeli].className) == "UH60");
    CHECK(Str(cat.hulls[FPAttackHeli].className) == "AH1");
    CHECK_FALSE(cat.Offered(FPJet));
    CHECK(Str(cat.hulls[FPTank].className) == "M60");
    CHECK(std::string(ForcePackages::OverrideKey(FPTruck)) == "forcesTruck");
    CHECK(std::string(ForcePackages::OverrideKey(FPArmedCar)) == "forcesArmedCar");
    CHECK(std::string(ForcePackages::OverrideKey(FPApc)) == "forcesApc");
    CHECK(std::string(ForcePackages::OverrideKey(FPTank)) == "forcesTank");
    CHECK(std::string(ForcePackages::OverrideKey(FPTransportHeli)) == "forcesTransportHeli");
    CHECK(std::string(ForcePackages::OverrideKey(FPAttackHeli)) == "forcesAttackHeli");
    CHECK(std::string(ForcePackages::OverrideKey(FPJet)) == "forcesJet");
    CHECK(std::string(ForcePackages::OverrideKey(FPFoot)).empty());
}

TEST_CASE("ForcePackages - wheeled armour with cargo is an APC; an unarmed APC hull with cargo is a truck",
          "[game][guerrilla][occupierforces][packages]")
{
    HullFacts brdm;
    brdm.exists = true;
    brdm.simulation = "car";
    brdm.armor = 120;
    brdm.cargo = 6;
    brdm.crewWeapon = true;
    CHECK(ForcePackages::Classify(brdm) == FPApc);
    brdm.cargo = 2;
    CHECK(ForcePackages::Classify(brdm) == FPArmedCar);
    HullFacts carrier;
    carrier.exists = true;
    carrier.simulation = "tank";
    carrier.armor = 150;
    carrier.cargo = 10;
    carrier.crewWeapon = false;
    CHECK(ForcePackages::Classify(carrier) == FPTruck);
    HullFacts man;
    man.exists = true;
    man.simulation = "soldier";
    CHECK(ForcePackages::Classify(man) == -1);
}

TEST_CASE("ForcePackages - prices and lethality follow the 6.5 table", "[game][guerrilla][occupierforces][packages]")
{
    CHECK(ForcePackages::HullPrice(FPFoot) == 0);
    CHECK(ForcePackages::HullPrice(FPTruck) == 3);
    CHECK(ForcePackages::HullPrice(FPArmedCar) == 4);
    CHECK(ForcePackages::HullPrice(FPApc) == 8);
    CHECK(ForcePackages::HullPrice(FPTank) == 14);
    CHECK(ForcePackages::HullPrice(FPTransportHeli) == 10);
    CHECK(ForcePackages::HullPrice(FPTransportHeli, true) == 15);
    CHECK(ForcePackages::HullPrice(FPAttackHeli) == 18);
    CHECK(ForcePackages::HullPrice(FPAttackHeli, true) == 27);
    CHECK(ForcePackages::HullPrice(FPJet) == 22);
    CHECK(ForcePackages::HullPrice(FPJet, true) == 33);

    CHECK(ForcePackages::TierFactor(0) == Approx(1.0f));
    CHECK(ForcePackages::TierFactor(1) == Approx(1.25f));
    CHECK(ForcePackages::TierFactor(2) == Approx(1.5f));
    CHECK(ForcePackages::TierFactor(7) == Approx(1.5f));
    CHECK(ForcePackages::Lethality(FPFoot, 4, 1.0f) == Approx(4.0f));
    CHECK(ForcePackages::Lethality(FPFoot, 4, 1.5f) == Approx(6.0f));
    CHECK(ForcePackages::Lethality(FPTruck, 9, 1.0f) == Approx(11.25f));
    CHECK(ForcePackages::Lethality(FPArmedCar, 1, 1.0f) == Approx(4.0f));
    CHECK(ForcePackages::Lethality(FPApc, 8, 1.0f) == Approx(16.0f));
    CHECK(ForcePackages::Lethality(FPTank, 0, 1.0f) == Approx(12.0f));
    CHECK(ForcePackages::Lethality(FPTransportHeli, 8, 1.0f) == Approx(10.0f));
    CHECK(ForcePackages::Lethality(FPAttackHeli, 0, 1.0f) == Approx(14.0f));
    CHECK(ForcePackages::Lethality(FPJet, 0, 1.0f) == Approx(16.0f));
}

TEST_CASE("ForcePackages - real abstract speeds: foot 1.5 m/s, a truck 0.6 x its maxSpeed",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue cat = WestCatalogue();
    CHECK(ForcePackages::SpeedOf(FPFoot, cat) == Approx(1.5f));
    CHECK(ForcePackages::SpeedOf(FPTruck, cat) == Approx(0.6f * 80.0f / 3.6f));
    // a package the faction lacks moves (if ever asked) at a walk
    CHECK(ForcePackages::SpeedOf(FPArmedCar, cat) == Approx(1.5f));
    ForcePackageCatalogue air;
    air.hulls[FPTransportHeli].className = "UH60";
    air.hulls[FPTransportHeli].maxSpeedKmh = 300;
    CHECK(ForcePackages::SpeedOf(FPTransportHeli, air) == Approx(0.8f * 300.0f / 3.6f));
}

TEST_CASE("ForcePackages - worked example 1: a quiet empty town 1 km from a base goes on foot",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue cat = WestCatalogue();
    PackageRequest req;
    req.fighters = 0;
    req.minBodies = 4;
    req.urgent = false;
    req.pool = 60;
    req.offered = ForcePackages::Phase3Packages;
    const PackageChoice c = ForcePackages::Choose(req, cat, OneSource(7, 1000.0f, 9));
    REQUIRE(c.ok);
    CHECK_FALSE(c.shortOfLethality);
    CHECK(c.lReq == Approx(0.0f));
    REQUIRE(c.nQuotes == 2);
    // foot: 4 units, about 11 min at 1.5 m/s (the doc rounds to 15 with the road)
    CHECK(c.quotes[0].package == FPFoot);
    CHECK(c.quotes[0].bodies == 4);
    CHECK(c.quotes[0].price == 4);
    CHECK(c.quotes[0].poolPrice == 0);
    // motorized: 4 bodies + a driver + a truck = 8 units, about 75 s
    CHECK(c.quotes[1].package == FPTruck);
    CHECK(c.quotes[1].bodies == 4);
    CHECK(c.quotes[1].crew == 1);
    CHECK(c.quotes[1].price == 8);
    CHECK(c.quotes[1].poolPrice == 4);
    CHECK(c.quotes[1].seconds < c.quotes[0].seconds);
    CHECK(c.quote.package == FPFoot);
    CHECK(c.quote.source == 7);
    CHECK(c.quote.bodies == 4);
}

TEST_CASE("ForcePackages - worked example 2: a loud flip 3.5 km out with 3 fighters goes motorized",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue cat = WestCatalogue();
    PackageRequest req;
    req.fighters = 3;
    req.defenderWeight = 3;
    req.margin = 1.2f;
    req.minBodies = 4;
    req.urgent = true;
    req.urgentCostShare = 1.5f;
    req.pool = 60;
    req.offered = ForcePackages::Phase3Packages;
    const PackageChoice c = ForcePackages::Choose(req, cat, OneSource(3, 3500.0f, 12));
    REQUIRE(c.ok);
    CHECK(c.lReq == Approx(10.8f));
    REQUIRE(c.nQuotes == 2);
    // foot 11: 11 units, about 39 min
    CHECK(c.quotes[0].bodies == 11);
    CHECK(c.quotes[0].price == 11);
    CHECK(c.quotes[0].lethality == Approx(11.0f));
    CHECK(c.quotes[0].seconds == Approx(3500.0f / 1.5f));
    // motorized 9 + a driver: 13 units, lethality 11.25, about 5 min
    CHECK(c.quotes[1].bodies == 9);
    CHECK(c.quotes[1].crew == 1);
    CHECK(c.quotes[1].price == 13);
    CHECK(c.quotes[1].lethality == Approx(11.25f));
    CHECK(c.quotes[1].seconds == Approx(3500.0f / (0.6f * 80.0f / 3.6f)));
    // 13 <= 1.5 x 11: urgent takes the faster package
    CHECK(c.quote.package == FPTruck);
    CHECK(Str(c.quote.hullClass) == "Truck5t");
    CHECK(c.quote.poolPrice == 4); // hull 3 + one crew body; the 9 come from the base
    CHECK(c.quote.speed == Approx(0.6f * 80.0f / 3.6f));
    // the same target not urgent: the cheapest lethal package, on foot
    req.urgent = false;
    const PackageChoice calm = ForcePackages::Choose(req, cat, OneSource(3, 3500.0f, 12));
    REQUIRE(calm.ok);
    CHECK(calm.quote.package == FPFoot);
    CHECK(calm.quote.bodies == 11);
    // urgent but over the cost share: a cheapest of 11 allows 16.5; a share
    // of 1.1 allows 12.1, and the truck's 13 is too dear
    req.urgent = true;
    req.urgentCostShare = 1.1f;
    const PackageChoice tight = ForcePackages::Choose(req, cat, OneSource(3, 3500.0f, 12));
    REQUIRE(tight.ok);
    CHECK(tight.quote.package == FPFoot);
}

TEST_CASE("ForcePackages - the minimum bodies, the source's spare and the most lethal fallback",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue cat = WestCatalogue();
    PackageRequest req;
    req.fighters = 3; // L_req 10.8
    req.minBodies = 4;
    req.pool = 60;
    req.offered = ForcePackages::Phase3Packages;
    // a base that can spare only 6: foot 6 (6.0) and motorized 6 (7.5) both
    // fall short; the most lethal affordable one goes and says so
    PackageChoice c = ForcePackages::Choose(req, cat, OneSource(2, 2000.0f, 6));
    REQUIRE(c.ok);
    CHECK(c.shortOfLethality);
    CHECK(c.quote.package == FPTruck);
    CHECK(c.quote.bodies == 6);
    CHECK(c.quote.lethality == Approx(7.5f));
    // a pool that cannot pay the truck (4 units): foot 6 goes, still short
    req.pool = 3;
    c = ForcePackages::Choose(req, cat, OneSource(2, 2000.0f, 6));
    REQUIRE(c.ok);
    CHECK(c.shortOfLethality);
    CHECK(c.quote.package == FPFoot);
    CHECK(c.quote.poolPrice == 0);
    // no base can spare the minimum: no operation
    req.pool = 60;
    c = ForcePackages::Choose(req, cat, OneSource(2, 2000.0f, 3));
    CHECK_FALSE(c.ok);
    CHECK(c.why.GetLength() > 0);
    // the nearest base with the minimum, not the nearest base
    AutoArray<PackageSource> two;
    PackageSource nearPoor;
    nearPoor.zone = 1;
    nearPoor.distance = 500;
    nearPoor.spare = 3;
    PackageSource farRich;
    farRich.zone = 4;
    farRich.distance = 3000;
    farRich.spare = 9;
    two.Add(nearPoor);
    two.Add(farRich);
    req.fighters = 0;
    c = ForcePackages::Choose(req, cat, two);
    REQUIRE(c.ok);
    CHECK(c.quote.source == 4);
    // a garrison of 6 to retake: at least 6 bodies whatever L_req says
    req.minBodies = 6;
    c = ForcePackages::Choose(req, cat, two);
    REQUIRE(c.ok);
    CHECK(c.quote.bodies == 6);
    // a hull package draws its infantry from the stored reserve
    AutoArray<PackageSource> liveOnly = OneSource(5, 3500.0f, 9);
    liveOnly[0].spareStored = 0;
    req.minBodies = 4;
    req.fighters = 3;
    req.urgent = true;
    c = ForcePackages::Choose(req, cat, liveOnly);
    REQUIRE(c.ok);
    CHECK(c.quote.package == FPFoot);
}

TEST_CASE("ForcePackages - phase 3 offers foot and truck only; no war-level gate",
          "[game][guerrilla][occupierforces][packages]")
{
    const FakeInspector in = VanillaAndLoBo();
    ForcePackageCatalogue cat;
    ForcePackages::BuildCatalogue(Ladder({"Jeep", "Truck5t", "M113", "M60"}), nullptr, in, cat);
    PackageRequest req;
    req.fighters = 4; // L_req 14.4: one group of 12 on foot (12.0) cannot meet it
    req.minBodies = 4;
    req.pool = 100;
    req.offered = ForcePackages::Phase3Packages;
    PackageChoice c = ForcePackages::Choose(req, cat, OneSource(1, 1000.0f, 12));
    REQUIRE(c.ok);
    CHECK(c.nQuotes == 2);
    for (int i = 0; i < c.nQuotes; i++)
    {
        CHECK((c.quotes[i].package == FPFoot || c.quotes[i].package == FPTruck));
    }
    // motorized 12 (15.0) meets 14.4 where foot 12 (12.0) cannot
    CHECK(c.quote.package == FPTruck);
    CHECK(c.quote.bodies == 12);
    // every package offered: the APC (8 + 2 crew, +6, x1.25) is chosen when
    // it is the cheapest lethal one; the request carries no war level at all,
    // so nothing gates armour on it (D2) - only the tier factor
    req.offered = ForcePackages::AllPackages;
    c = ForcePackages::Choose(req, cat, OneSource(1, 1000.0f, 12));
    REQUIRE(c.ok);
    // foot, truck, APC, tank, then the columns truck+APC, truck+tank,
    // APC+APC, APC+tank (no armed car, no air in this ladder)
    CHECK(c.nQuotes == 8);
    // truck 12 + 1 + 3 = 16 (15.0); APC needs ceil(8.4 / 1.25) = 7: 7 + 2 + 8 = 17 (14.75);
    // every column costs more than the plain truck
    CHECK(c.quote.package == FPTruck);
    // a better tier: foot 10 x 1.5 = 15.0 meets it for 10 units
    req.tierFactor = 1.5f;
    c = ForcePackages::Choose(req, cat, OneSource(1, 1000.0f, 12));
    REQUIRE(c.ok);
    CHECK(c.quote.package == FPFoot);
    CHECK(c.quote.bodies == 10);
}

TEST_CASE("ForcePackages - group and hull budgets", "[game][guerrilla][occupierforces][packages]")
{
    CHECK(ForcePackages::GroupsOf(FPFoot) == 1);
    CHECK(ForcePackages::GroupsOf(FPTruck) == 2);
    CHECK(ForcePackages::GroupsOf(FPTransportHeli) == 2);
    CHECK(ForcePackages::HullsOf(FPFoot) == 0);
    CHECK(ForcePackages::HullsOf(FPTruck) == 1);
    CHECK(ForcePackages::WithinBudgets(FPFoot, 39, 6));
    CHECK_FALSE(ForcePackages::WithinBudgets(FPFoot, 40, 0));
    CHECK(ForcePackages::WithinBudgets(FPTruck, 38, 5));
    CHECK_FALSE(ForcePackages::WithinBudgets(FPTruck, 39, 0)); // two groups do not fit in one
    CHECK_FALSE(ForcePackages::WithinBudgets(FPTruck, 10, 6)); // forcesLiveHullsMax
    CHECK(ForcePackages::WithinBudgets(FPFoot, 10, 6));        // foot needs no hull
}

TEST_CASE("ForcePackages - names for gmOccupierOps", "[game][guerrilla][occupierforces][packages]")
{
    CHECK(std::string(ForcePackages::PackageName(FPFoot)) == "FOOT");
    CHECK(std::string(ForcePackages::PackageName(FPTruck)) == "TRUCK");
    CHECK(std::string(ForcePackages::PackageName(FPJet)) == "JET");
    CHECK(std::string(ForcePackages::KindName(CAKRetake)) == "RETAKE");
    CHECK(std::string(ForcePackages::KindName(CAKHqAssault)) == "HQ_ASSAULT");
    CHECK(std::string(ForcePackages::OriginName(CAOLegacy)) == "LEGACY");
    CHECK(std::string(ForcePackages::OriginName(CAODirector)) == "DIRECTOR");
    CHECK(std::string(ForcePackages::OriginName(CAOPushout)) == "PUSHOUT");
}

// ---------------------------------------------------------------------------
// phase 5: armour packages (armed car, APC, tank)
// ---------------------------------------------------------------------------

namespace
{

// a ladder with all three armour packages, in the shape of LoBo's Egyptian
// frontier roster {Toyota MG, BTR-60, BMP, T-55}: an armed pickup with two
// cargo seats, a wheeled APC (car simulation, armoured, eight seats), a
// tracked APC and a tank.  The facts are fakes in that shape, not config
// reads.
FakeInspector ArmourLadder()
{
    FakeInspector in = VanillaAndLoBo();
    in.Add("ToyotaMG", "car", 2, 110, 20, true, 2);
    in.Add("BTR60", "car", 8, 80, 120, true, 2);
    in.Add("BMPE", "tank", 8, 65, 200, true, 3);
    in.Add("T55", "tank", 0, 50, 350, true, 3);
    // an IDF-shaped ladder: no truck at all
    in.Add("HMMWVPatrol", "car", 3, 100, 30, true, 2);
    in.Add("Zelda", "tank", 10, 60, 150, true, 2);
    in.Add("M60A1", "tank", 0, 50, 300, true, 3);
    return in;
}

ForcePackageCatalogue ArmourCatalogue()
{
    const FakeInspector in = ArmourLadder();
    ForcePackageCatalogue cat;
    ForcePackages::BuildCatalogue(Ladder({"ToyotaMG", "BTR60", "BMPE", "T55"}), nullptr, in, cat);
    return cat;
}

ForcePackageCatalogue IdfCatalogue()
{
    const FakeInspector in = ArmourLadder();
    ForcePackageCatalogue cat;
    ForcePackages::BuildCatalogue(Ladder({"HMMWVPatrol", "Zelda", "M60A1"}), nullptr, in, cat);
    return cat;
}

// the plain (escort-less) quote of a package
const PackageQuote* QuoteOf(const PackageChoice& c, int package)
{
    for (int i = 0; i < c.nQuotes; i++)
    {
        if (c.quotes[i].package == package && !c.quotes[i].HasEscort())
        {
            return &c.quotes[i];
        }
    }
    return nullptr;
}

// a column's quote: carrier + escort
const PackageQuote* ColumnOf(const PackageChoice& c, int carrier, int escort)
{
    for (int i = 0; i < c.nQuotes; i++)
    {
        if (c.quotes[i].package == carrier && c.quotes[i].escort == escort)
        {
            return &c.quotes[i];
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("ForcePackages - phase 5 offers every ground package by default, air not yet",
          "[game][guerrilla][occupierforces][packages]")
{
    const PackageRequest req;
    CHECK(req.offered == ForcePackages::DefaultPackages);
    CHECK(ForcePackages::DefaultPackages == ForcePackages::GroundPackages);
    for (int p = FPFoot; p <= FPTank; p++)
    {
        CHECK((ForcePackages::DefaultPackages & ForcePackages::Bit(p)) != 0);
    }
    for (int p = FPTransportHeli; p < NForcePackages; p++)
    {
        CHECK((ForcePackages::DefaultPackages & ForcePackages::Bit(p)) == 0);
        CHECK((ForcePackages::AirPackages & ForcePackages::Bit(p)) != 0);
    }
    CHECK((ForcePackages::GroundPackages | ForcePackages::AirPackages) == ForcePackages::AllPackages);
    CHECK((ForcePackages::Phase3Packages & ForcePackages::GroundPackages) == ForcePackages::Phase3Packages);
}

TEST_CASE("ForcePackages - the armour ladders: armed car, wheeled and tracked APC, tank; the lowest rung wins",
          "[game][guerrilla][occupierforces][packages]")
{
    const FakeInspector in = ArmourLadder();
    const char* names[] = {"ToyotaMG", "BTR60", "BMPE", "T55", "HMMWVPatrol", "Zelda", "M60A1"};
    const int expected[] = {FPArmedCar, FPApc, FPApc, FPTank, FPArmedCar, FPApc, FPTank};
    for (int i = 0; i < 7; i++)
    {
        HullFacts f;
        REQUIRE(in.Inspect(names[i], f));
        CHECK(ForcePackages::Classify(f) == expected[i]);
    }
    const ForcePackageCatalogue cat = ArmourCatalogue();
    CHECK_FALSE(cat.Offered(FPTruck));
    CHECK(Str(cat.hulls[FPArmedCar].className) == "ToyotaMG");
    CHECK(Str(cat.hulls[FPApc].className) == "BTR60"); // before the BMP: the lower rung
    CHECK(Str(cat.hulls[FPTank].className) == "T55");
    // crew per class: what hasDriver + hasGunner + hasCommander count
    CHECK(cat.hulls[FPArmedCar].crew == 2);
    CHECK(cat.hulls[FPApc].crew == 2);
    CHECK(cat.hulls[FPTank].crew == 3);
    CHECK(ForcePackages::CrewOf(FPArmedCar, cat) == 2);
    CHECK(ForcePackages::CrewOf(FPTank, cat) == 3);
    CHECK(ForcePackages::CrewOf(FPTruck, cat) == 0); // not offered
    // the WEST ladder: the M113 has two crew, the M60 three
    const ForcePackageCatalogue west = WestCatalogue();
    CHECK(ForcePackages::CrewOf(FPApc, west) == 2);
    CHECK(ForcePackages::CrewOf(FPTank, west) == 3);
    // an override names an armour class directly
    RString overrides[NForcePackages];
    overrides[FPTank] = "M60A1";
    overrides[FPApc] = "Zelda";
    ForcePackageCatalogue over;
    ForcePackages::BuildCatalogue(Ladder({"ToyotaMG", "BTR60", "BMPE", "T55"}), overrides, in, over);
    CHECK(Str(over.hulls[FPTank].className) == "M60A1");
    CHECK(Str(over.hulls[FPApc].className) == "Zelda");
    CHECK(over.hulls[FPApc].cargo == 10);
}

TEST_CASE("ForcePackages - armour prices and lethality in a quote: bodies + crew + hull",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue cat = ArmourCatalogue();
    PackageRequest req;
    req.fighters = 0;
    req.minBodies = 2; // an armed car's two seats hold the minimum
    req.pool = 100;
    const AutoArray<PackageSource> src = OneSource(1, 2000.0f, 12);
    const PackageQuote car = ForcePackages::Quote(FPArmedCar, req, cat, src);
    CHECK(car.source == 1);
    CHECK(car.bodies == 2);
    CHECK(car.crew == 2);
    CHECK(car.hullPrice == 4);
    CHECK(car.poolPrice == 6);
    CHECK(car.price == 8);
    CHECK(car.lethality == Approx(2.0f + 3.0f)); // no carried factor, +3
    CHECK(car.speed == Approx(0.6f * 110.0f / 3.6f));
    const PackageQuote apc = ForcePackages::Quote(FPApc, req, cat, src);
    CHECK(apc.bodies == 2);
    CHECK(apc.crew == 2);
    CHECK(apc.hullPrice == 8);
    CHECK(apc.poolPrice == 10);
    CHECK(apc.price == 12);
    CHECK(apc.lethality == Approx(2.0f * 1.25f + 6.0f));
    // a tank carries nobody: its quote is seats-short at any minimum >= 1
    const PackageQuote tank = ForcePackages::Quote(FPTank, req, cat, src);
    CHECK(tank.seatsShort);
    CHECK(tank.source == -1);
    CHECK(tank.hullPrice == 14);
    CHECK(tank.poolPrice == 17);
    CHECK(ForcePackages::Lethality(FPTank, 0, 1.0f) == Approx(12.0f));
    CHECK(ForcePackages::Lethality(FPArmedCar, 2, 1.5f) == Approx(6.0f));
    CHECK(ForcePackages::Lethality(FPApc, 8, 1.25f) == Approx(8.0f * 1.25f * 1.25f + 6.0f));
}

TEST_CASE("ForcePackages - the seat cap: a hull carries at most its cargo seats, one group at most",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue west = WestCatalogue();
    CHECK(ForcePackages::SeatCap(FPFoot, west) == ForcePackages::MaxGroup);
    CHECK(ForcePackages::SeatCap(FPTruck, west) == 12);
    CHECK(ForcePackages::SeatCap(FPApc, west) == 8);
    CHECK(ForcePackages::SeatCap(FPTank, west) == 0);
    CHECK(ForcePackages::SeatCap(FPArmedCar, west) == 0); // not offered
    const ForcePackageCatalogue idf = IdfCatalogue();
    CHECK(ForcePackages::SeatCap(FPApc, idf) == 10);
    CHECK(ForcePackages::SeatCap(FPArmedCar, idf) == 3);

    PackageRequest req;
    req.fighters = 5; // L_req 18: the APC wants ceil(12 / 1.25) = 10 but has 8 seats
    req.minBodies = 4;
    req.pool = 100;
    const AutoArray<PackageSource> src = OneSource(2, 1500.0f, 12);
    const PackageQuote apc = ForcePackages::Quote(FPApc, req, west, src);
    CHECK(apc.bodies == 8);
    CHECK_FALSE(apc.seatsShort);
    CHECK(apc.lethality == Approx(16.0f));
    CHECK_FALSE(apc.lethal);
    // a minimum above the seats: the APC is quoted, short, and never chosen
    req.minBodies = 9;
    const PackageQuote big = ForcePackages::Quote(FPApc, req, west, src);
    CHECK(big.seatsShort);
    CHECK(big.source == -1);
    const PackageChoice c = ForcePackages::Choose(req, west, src);
    REQUIRE(c.ok);
    CHECK(c.quote.package != FPApc);
    CHECK(c.quote.package != FPTank);
    // the IDF-shaped armed car (3 seats) cannot carry a retake's 4
    req.minBodies = 4;
    req.fighters = 0;
    const PackageQuote car = ForcePackages::Quote(FPArmedCar, req, idf, src);
    CHECK(car.seatsShort);
    req.minBodies = 3;
    const PackageQuote car3 = ForcePackages::Quote(FPArmedCar, req, idf, src);
    CHECK_FALSE(car3.seatsShort);
    CHECK(car3.bodies == 3);
}

TEST_CASE("ForcePackages - the choice with armour affordable and not", "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue west = WestCatalogue();
    PackageRequest req;
    req.fighters = 5; // L_req 18: foot 12 (12.0), truck 12 (15.0), APC 8 (16.0): no single hull meets it
    req.minBodies = 4;
    req.pool = 100;
    const AutoArray<PackageSource> src = OneSource(2, 1500.0f, 12);
    PackageChoice c = ForcePackages::Choose(req, west, src);
    REQUIRE(c.ok);
    CHECK(c.nQuotes == 8); // foot, truck, APC, tank + truck+APC, truck+tank, APC+APC, APC+tank
    // a column meets it: truck + APC escort, 10 + 1 + 3 + 2 + 8 = 24 units,
    // 10 x 1.25 + 6 = 18.5 (truck + tank 26, APC + APC 25, APC + tank 31)
    CHECK_FALSE(c.shortOfLethality);
    CHECK(c.quote.package == FPTruck);
    CHECK(c.quote.escort == FPApc);
    CHECK(c.quote.price == 24);
    // the pool cannot pay any column (the cheapest needs 14): the most
    // lethal affordable single package goes, the APC
    req.pool = 13;
    c = ForcePackages::Choose(req, west, src);
    REQUIRE(c.ok);
    CHECK(c.shortOfLethality);
    CHECK(c.quote.package == FPApc);
    CHECK_FALSE(c.quote.HasEscort());
    CHECK(c.quote.bodies == 8);
    CHECK(c.quote.crew == 2);
    CHECK(c.quote.poolPrice == 10);
    CHECK(c.quote.price == 18);
    CHECK(c.quote.lethality == Approx(16.0f));
    CHECK(Str(c.quote.hullClass) == "M113");
    // the pool cannot pay the APC (10) but can the truck (4)
    req.pool = 9;
    c = ForcePackages::Choose(req, west, src);
    REQUIRE(c.ok);
    CHECK(c.shortOfLethality);
    CHECK(c.quote.package == FPTruck);
    CHECK_FALSE(c.quote.HasEscort());
    CHECK(c.quote.lethality == Approx(15.0f));
    // the pool inert: everything is affordable, the cheapest lethal column
    req.pool = 0;
    req.poolActive = false;
    c = ForcePackages::Choose(req, west, src);
    REQUIRE(c.ok);
    CHECK(c.quote.package == FPTruck);
    CHECK(c.quote.escort == FPApc);

    // an IDF-shaped ladder (no truck): the APC is the only carrier.  A loud
    // urgent flip with 3 fighters (L_req 10.8): foot 11 is the cheapest at
    // 11; the APC (4 + 2 crew + 8 = 14, lethality 11.0) costs at most 1.5 x
    // that and is faster, so it goes
    const ForcePackageCatalogue idf = IdfCatalogue();
    PackageRequest loud;
    loud.fighters = 3;
    loud.minBodies = 4;
    loud.urgent = true;
    loud.pool = 60;
    c = ForcePackages::Choose(loud, idf, OneSource(3, 3500.0f, 12));
    REQUIRE(c.ok);
    CHECK_FALSE(c.shortOfLethality);
    CHECK(c.quote.package == FPApc);
    CHECK(c.quote.bodies == 4);
    CHECK(c.quote.price == 14);
    CHECK(c.quote.lethality == Approx(11.0f));
    const PackageQuote* car = QuoteOf(c, FPArmedCar);
    REQUIRE(car != nullptr);
    CHECK(car->seatsShort); // three seats, four needed
    const PackageQuote* tank = QuoteOf(c, FPTank);
    REQUIRE(tank != nullptr);
    CHECK(tank->seatsShort);
    // the same flip with a pool that cannot pay the APC: on foot
    loud.pool = 9;
    c = ForcePackages::Choose(loud, idf, OneSource(3, 3500.0f, 12));
    REQUIRE(c.ok);
    CHECK(c.quote.package == FPFoot);
    CHECK(c.quote.bodies == 11);
    // not urgent: the cheapest lethal package, on foot
    loud.pool = 60;
    loud.urgent = false;
    c = ForcePackages::Choose(loud, idf, OneSource(3, 3500.0f, 12));
    REQUIRE(c.ok);
    CHECK(c.quote.package == FPFoot);

    // an armed car is quoted where its seats hold the minimum (3 here):
    // 1 fighter (L_req 3.6), foot 4 (4 units) against the armed car's
    // 3 + 2 crew + 4 = 9 units (lethality 6.0).  Urgent, the ceiling is
    // 1.5 x 4 = 6, so the armed car is too dear and foot goes.
    PackageRequest small;
    small.fighters = 1;
    small.minBodies = 3;
    small.urgent = true;
    small.pool = 60;
    c = ForcePackages::Choose(small, idf, OneSource(3, 3000.0f, 12));
    REQUIRE(c.ok);
    const PackageQuote* smallCar = QuoteOf(c, FPArmedCar);
    REQUIRE(smallCar != nullptr);
    CHECK(smallCar->source == 3);
    CHECK(smallCar->bodies == 3);
    CHECK(smallCar->price == 9);
    CHECK(smallCar->lethality == Approx(6.0f));
    CHECK(c.quote.package == FPFoot);
    CHECK(c.quote.bodies == 4);
}

TEST_CASE("ForcePackages - both worked examples hold with every ground package offered",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue cat = WestCatalogue();
    // example 1: a quiet empty town 1 km from a base
    PackageRequest quiet;
    quiet.fighters = 0;
    quiet.minBodies = 4;
    quiet.pool = 60;
    PackageChoice c = ForcePackages::Choose(quiet, cat, OneSource(7, 1000.0f, 9));
    REQUIRE(c.ok);
    CHECK(c.nQuotes == 8);
    const PackageQuote* apc = QuoteOf(c, FPApc);
    REQUIRE(apc != nullptr);
    CHECK(apc->bodies == 4);
    CHECK(apc->price == 14); // 4 bodies + 2 crew + the APC's 8
    CHECK(ColumnOf(c, FPTruck, FPApc)->price == 18);
    CHECK(c.quote.package == FPFoot);
    CHECK_FALSE(c.quote.HasEscort());
    CHECK(c.quote.price == 4);
    // example 2: a loud flip 3.5 km from the Harbour, 3 fighters, urgent
    PackageRequest loud;
    loud.fighters = 3;
    loud.minBodies = 4;
    loud.urgent = true;
    loud.pool = 60;
    c = ForcePackages::Choose(loud, cat, OneSource(3, 3500.0f, 12));
    REQUIRE(c.ok);
    CHECK(c.lReq == Approx(10.8f));
    apc = QuoteOf(c, FPApc);
    REQUIRE(apc != nullptr);
    const PackageQuote* truck = QuoteOf(c, FPTruck);
    REQUIRE(truck != nullptr);
    // the APC: 4 + 2 crew + 8 = 14 units, lethality 11.0, within 1.5 x 11
    // but slower than the truck (0.6 x 60 km/h against 0.6 x 80)
    CHECK(apc->bodies == 4);
    CHECK(apc->price == 14);
    CHECK(apc->lethality == Approx(11.0f));
    CHECK(apc->lethal);
    CHECK(apc->seconds > truck->seconds);
    CHECK(QuoteOf(c, FPTank)->seatsShort);
    // the columns: truck + APC 4 + 1 + 3 + 2 + 8 = 18 and truck + tank
    // 4 + 1 + 3 + 3 + 14 = 25, both above 1.5 x 11 = 16.5
    const PackageQuote* truckApc = ColumnOf(c, FPTruck, FPApc);
    REQUIRE(truckApc != nullptr);
    CHECK(truckApc->bodies == 4);
    CHECK(truckApc->price == 18);
    CHECK(truckApc->lethality == Approx(11.0f));
    const PackageQuote* truckTank = ColumnOf(c, FPTruck, FPTank);
    REQUIRE(truckTank != nullptr);
    CHECK(truckTank->price == 25);
    CHECK(c.quote.package == FPTruck);
    CHECK_FALSE(c.quote.HasEscort());
    CHECK(c.quote.bodies == 9);
    CHECK(c.quote.price == 13);
}

TEST_CASE("ForcePackages - at the town edge armed hulls support and trucks hold back",
          "[game][guerrilla][occupierforces][packages]")
{
    CHECK(ForcePackages::EdgeRole(FPFoot) == FERNone);
    CHECK(ForcePackages::EdgeRole(FPTruck) == FERHoldBack);
    CHECK(ForcePackages::EdgeRole(FPArmedCar) == FERSupport);
    CHECK(ForcePackages::EdgeRole(FPApc) == FERSupport);
    CHECK(ForcePackages::EdgeRole(FPTank) == FERSupport);
    CHECK(ForcePackages::EdgeRole(FPTransportHeli) == FERNone);
    CHECK(ForcePackages::EdgeRole(FPAttackHeli) == FERNone);
    CHECK(ForcePackages::EdgeRole(FPJet) == FERNone);
    CHECK_FALSE(ForcePackages::SupportsAtEdge(FPTruck));
    CHECK(ForcePackages::SupportsAtEdge(FPApc));
    CHECK(ForcePackages::IsGroundHull(FPTruck));
    CHECK(ForcePackages::IsGroundHull(FPTank));
    CHECK_FALSE(ForcePackages::IsGroundHull(FPFoot));
    CHECK_FALSE(ForcePackages::IsGroundHull(FPTransportHeli));
    // every ground hull package counts one crewed hull and two groups
    for (int p = FPTruck; p <= FPTank; p++)
    {
        CHECK(ForcePackages::GroupsOf(p) == 2);
        CHECK(ForcePackages::HullsOf(p) == 1);
    }
}

TEST_CASE("ForcePackages - a hull package downgrades to foot when the stored reserve cannot pay",
          "[game][guerrilla][occupierforces][packages]")
{
    // stored 0, live 9: the truck's 9 go on foot
    CHECK(ForcePackages::MustDowngrade(FPTruck, 9, 0, 9));
    CHECK(ForcePackages::MustDowngrade(FPApc, 4, 3, 6));
    CHECK(ForcePackages::MustDowngrade(FPTank, 1, 0, 4));
    // the stored reserve pays: no downgrade
    CHECK_FALSE(ForcePackages::MustDowngrade(FPTruck, 9, 9, 9));
    // nobody can pay at all: no downgrade (the launch is refused instead)
    CHECK_FALSE(ForcePackages::MustDowngrade(FPTruck, 9, 0, 5));
    // foot and air never downgrade
    CHECK_FALSE(ForcePackages::MustDowngrade(FPFoot, 4, 0, 9));
    CHECK_FALSE(ForcePackages::MustDowngrade(FPTransportHeli, 8, 0, 12));

    // worked example 2's truck quote, downgraded
    const ForcePackageCatalogue cat = WestCatalogue();
    PackageRequest loud;
    loud.fighters = 3;
    loud.minBodies = 4;
    loud.urgent = true;
    loud.pool = 60;
    loud.offered = ForcePackages::Phase3Packages;
    const PackageChoice c = ForcePackages::Choose(loud, cat, OneSource(3, 3500.0f, 12));
    REQUIRE(c.ok);
    REQUIRE(c.quote.package == FPTruck);
    int refund = -1;
    const PackageQuote f = ForcePackages::Downgrade(c.quote, 1.0f, c.lReq, refund);
    CHECK(refund == 4); // the truck's 3 + its driver
    CHECK(f.package == FPFoot);
    CHECK(f.source == 3);
    CHECK(f.bodies == 9);
    CHECK(f.crew == 0);
    CHECK(f.hullPrice == 0);
    CHECK(f.poolPrice == 0);
    CHECK(f.price == 9);
    CHECK(f.hullClass.GetLength() == 0);
    CHECK(f.speed == Approx(1.5f));
    CHECK(f.seconds == Approx(3500.0f / 1.5f).epsilon(0.001));
    CHECK(f.lethality == Approx(9.0f));
    CHECK_FALSE(f.lethal); // 9 on foot fall short of 10.8
    CHECK(f.affordable);
    // a foot quote stays as it is, nothing refunded
    const PackageQuote same = ForcePackages::Downgrade(f, 1.0f, c.lReq, refund);
    CHECK(refund == 0);
    CHECK(same.package == FPFoot);
    CHECK(same.bodies == 9);
}

// ---------------------------------------------------------------------------
// phase 5: columns (a carrier + one escort hull, the owner's option b)
// ---------------------------------------------------------------------------

namespace
{

// a ladder with every ground role: a truck (80 km/h), a slow armoured car
// (40 km/h, two seats), a tracked APC (65 km/h, eight seats, three crew)
// and a tank (60 km/h).  Fakes in that shape.
ForcePackageCatalogue ColumnCatalogue()
{
    FakeInspector in = VanillaAndLoBo();
    in.Add("ColTruck", "car", 12, 80, 20, false, 1);
    in.Add("ColArmouredCar", "car", 2, 40, 20, true, 2);
    in.Add("ColApc", "tank", 8, 65, 200, true, 3);
    in.Add("ColTank", "tank", 0, 60, 400, true, 3);
    ForcePackageCatalogue cat;
    ForcePackages::BuildCatalogue(Ladder({"ColTruck", "ColArmouredCar", "ColApc", "ColTank"}), nullptr, in, cat);
    return cat;
}

} // namespace

TEST_CASE("ForcePackages - a column: carrier + escort, its price, lethality and pace",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue cat = WestCatalogue();
    PackageRequest req;
    req.fighters = 5; // L_req 18
    req.minBodies = 4;
    req.pool = 100;
    const AutoArray<PackageSource> src = OneSource(2, 1500.0f, 12);
    const PackageQuote q = ForcePackages::Quote(FPTruck, FPApc, req, cat, src);
    CHECK(q.package == FPTruck);
    CHECK(q.escort == FPApc);
    CHECK(q.HasEscort());
    CHECK(Str(q.hullClass) == "Truck5t");
    CHECK(Str(q.escortClass) == "M113");
    CHECK(q.bodies == 10); // ceil((18 - 6) / 1.25)
    CHECK(q.crew == 1);
    CHECK(q.hullPrice == 3);
    CHECK(q.escortCrew == 2);
    CHECK(q.escortHullPrice == 8);
    CHECK(q.EscortPoolPrice() == 10);
    CHECK(q.poolPrice == 14);
    CHECK(q.price == 24);
    CHECK(q.lethality == Approx(10.0f * 1.25f + 6.0f));
    CHECK(q.lethal);
    // the slower hull sets the pace: the M113's 60 km/h, not the truck's 80
    CHECK(q.speed == Approx(0.6f * 60.0f / 3.6f));
    CHECK(q.seconds == Approx(1500.0f / (0.6f * 60.0f / 3.6f)));
    CHECK(ForcePackages::GroupsOf(q) == 3);
    CHECK(ForcePackages::HullsOf(q) == 2);
    CHECK(ForcePackages::ColumnLethality(FPApc, FPTank, 4, 1.0f) == Approx(4.0f * 1.25f + 6.0f + 12.0f));
    // an APC carrier with an APC escort: the escort adds its +6 and carries nobody
    const PackageQuote aa = ForcePackages::Quote(FPApc, FPApc, req, cat, src);
    CHECK(aa.bodies == 5); // ceil((18 - 12) / 1.25), the carrier's 8 seats
    CHECK(aa.lethality == Approx(5.0f * 1.25f + 12.0f));
    CHECK(aa.price == 5 + 2 + 8 + 2 + 8);
    // invalid pairs quote no source: foot never takes an escort, a tank never carries
    CHECK(ForcePackages::Quote(FPFoot, FPApc, req, cat, src).source == -1);
    CHECK(ForcePackages::Quote(FPTank, FPApc, req, cat, src).source == -1);
    CHECK(ForcePackages::Quote(FPTruck, FPTruck, req, cat, src).source == -1);
    CHECK(ForcePackages::Quote(FPTruck, FPArmedCar, req, cat, src).source == -1); // WEST has no armed car
    // the plain overload is the escort-less column
    const PackageQuote plain = ForcePackages::Quote(FPTruck, req, cat, src);
    CHECK_FALSE(plain.HasEscort());
    CHECK(plain.EscortPoolPrice() == 0);
    CHECK(ForcePackages::GroupsOf(plain) == 2);
    CHECK(ForcePackages::HullsOf(plain) == 1);
}

TEST_CASE("ForcePackages - a column is chosen when L_req exceeds what a truck squad gives",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue cat = WestCatalogue();
    PackageRequest req;
    req.fighters = 4.5f; // L_req 16.2: a full truck squad gives 15.0
    req.minBodies = 4;
    req.pool = 100;
    const AutoArray<PackageSource> src = OneSource(2, 2000.0f, 12);
    PackageChoice c = ForcePackages::Choose(req, cat, src);
    REQUIRE(c.ok);
    CHECK_FALSE(QuoteOf(c, FPTruck)->lethal);
    // the APC alone: ceil(10.2 / 1.25) = 9 but 8 seats: 16.0, short.
    // truck + APC: ceil(10.2 / 1.25) = 9 bodies, 17.25, 9 + 4 + 10 = 23;
    // truck + tank: 4 bodies, 17.0, 4 + 4 + 17 = 25; APC + APC: 4, 17.0,
    // 4 + 10 + 10 = 24
    CHECK_FALSE(c.shortOfLethality);
    CHECK(c.quote.package == FPTruck);
    CHECK(c.quote.escort == FPApc);
    CHECK(c.quote.bodies == 9);
    CHECK(c.quote.price == 23);
    CHECK(c.quote.lethality == Approx(17.25f));
    // with the truck squad enough, the plain truck stays the choice
    req.fighters = 4; // L_req 14.4
    c = ForcePackages::Choose(req, cat, src);
    REQUIRE(c.ok);
    CHECK(c.quote.package == FPTruck);
    CHECK_FALSE(c.quote.HasEscort());
}

TEST_CASE("ForcePackages - a lone tank is never chosen, even when nothing else meets L_req",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue cat = WestCatalogue();
    PackageRequest req;
    req.fighters = 30; // L_req 108: nothing meets it
    req.minBodies = 4;
    req.pool = 1000;
    PackageChoice c = ForcePackages::Choose(req, cat, OneSource(2, 1500.0f, 12));
    REQUIRE(c.ok);
    CHECK(c.shortOfLethality);
    CHECK(c.quote.package != FPTank);
    REQUIRE(QuoteOf(c, FPTank) != nullptr);
    CHECK(QuoteOf(c, FPTank)->seatsShort);
    // the tank goes only as an escort: the most lethal column is
    // APC + tank, 8 x 1.25 + 6 + 12 = 28
    CHECK(c.quote.package == FPApc);
    CHECK(c.quote.escort == FPTank);
    CHECK(c.quote.lethality == Approx(28.0f));
    // only foot and the tank offered: foot goes, never the tank
    req.offered = ForcePackages::Bit(FPFoot) | ForcePackages::Bit(FPTank);
    c = ForcePackages::Choose(req, cat, OneSource(2, 1500.0f, 12));
    REQUIRE(c.ok);
    CHECK(c.quote.package == FPFoot);
    // a seats-short armed car (one seat) is never chosen either
    const FakeInspector in = VanillaAndLoBo();
    ForcePackageCatalogue hiz;
    ForcePackages::BuildCatalogue(Ladder({"LoBo_JeepMG_Hizballah"}), nullptr, in, hiz);
    req.offered = ForcePackages::DefaultPackages;
    req.fighters = 3;
    c = ForcePackages::Choose(req, hiz, OneSource(2, 1500.0f, 12));
    REQUIRE(c.ok);
    CHECK(c.quote.package == FPFoot);
    CHECK(QuoteOf(c, FPArmedCar)->seatsShort);
}

TEST_CASE("ForcePackages - URGENT takes the fastest within 1.5 x across columns",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue cat = ColumnCatalogue();
    PackageRequest req;
    req.fighters = 5; // L_req 18: truck 12 (15.0) short; the APC alone 8 (16.0) short
    req.minBodies = 4;
    req.pool = 100;
    const AutoArray<PackageSource> src = OneSource(4, 3000.0f, 12);
    PackageChoice c = ForcePackages::Choose(req, cat, src);
    REQUIRE(c.ok);
    // the cheapest lethal: truck + armoured car, 12 + 1 + 3 + 2 + 4 = 22
    // (18.0), at the armoured car's 40 km/h
    const PackageQuote* car = ColumnOf(c, FPTruck, FPArmedCar);
    REQUIRE(car != nullptr);
    CHECK(car->price == 22);
    CHECK(car->lethal);
    CHECK(car->speed == Approx(0.6f * 40.0f / 3.6f));
    CHECK(c.quote.package == FPTruck);
    CHECK(c.quote.escort == FPArmedCar);
    // urgent: anything up to 1.5 x 22 = 33 may go, and the fastest does:
    // truck + APC (10 bodies, 10 + 4 + 11 = 25, 65 km/h) beats truck + tank
    // (60 km/h) and ties APC + APC (65 km/h, 27) on pace but is cheaper
    req.urgent = true;
    c = ForcePackages::Choose(req, cat, src);
    REQUIRE(c.ok);
    const PackageQuote* apc = ColumnOf(c, FPTruck, FPApc);
    REQUIRE(apc != nullptr);
    CHECK(apc->price == 25);
    CHECK(c.quote.package == FPTruck);
    CHECK(c.quote.escort == FPApc);
    CHECK(c.quote.speed == Approx(0.6f * 65.0f / 3.6f));
    // a tighter share (1.1 x 22 = 24.2): the faster columns are too dear
    req.urgentCostShare = 1.1f;
    c = ForcePackages::Choose(req, cat, src);
    REQUIRE(c.ok);
    CHECK(c.quote.escort == FPArmedCar);
}

TEST_CASE("ForcePackages - a column goes on foot whole: the downgrade refunds both hulls",
          "[game][guerrilla][occupierforces][packages]")
{
    const ForcePackageCatalogue cat = WestCatalogue();
    PackageRequest req;
    req.fighters = 4.5f;
    req.minBodies = 4;
    req.pool = 100;
    const PackageChoice c = ForcePackages::Choose(req, cat, OneSource(2, 2000.0f, 12));
    REQUIRE(c.ok);
    REQUIRE(c.quote.HasEscort());
    CHECK(ForcePackages::MustDowngrade(c.quote.package, c.quote.bodies, 0, 12));
    int refund = -1;
    const PackageQuote f = ForcePackages::Downgrade(c.quote, 1.0f, c.lReq, refund);
    CHECK(refund == 14); // truck 3 + driver 1 + APC 8 + its crew 2
    CHECK(f.package == FPFoot);
    CHECK_FALSE(f.HasEscort());
    CHECK(f.escortClass.GetLength() == 0);
    CHECK(f.escortCrew == 0);
    CHECK(f.escortHullPrice == 0);
    CHECK(f.EscortPoolPrice() == 0);
    CHECK(f.poolPrice == 0);
    CHECK(f.bodies == 9);
    CHECK(f.price == 9);
    CHECK(f.speed == Approx(1.5f));
    CHECK(f.seconds == Approx(2000.0f / 1.5f).epsilon(0.001));
    CHECK(f.lethality == Approx(9.0f));
    CHECK(ForcePackages::GroupsOf(f) == 1);
    CHECK(ForcePackages::HullsOf(f) == 0);
}
