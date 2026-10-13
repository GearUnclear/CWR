// Occupier force packages (OCCUPIER-FORCES.md 6.5, D1 and D2; ForcePackages):
// the ladder classification for WEST, EAST and LoBo Hizballah from config
// facts (APC before Tank, horns are not weapons, air never from the ladder),
// the descriptor overrides, the price and lethality table, the real abstract
// speeds (3.2), the package choice (the cheapest lethal package, the fastest
// within the urgent cost share, the most lethal affordable fallback, the
// minimum bodies, the source's spare), both worked examples of 6.5 with the
// crew included, no war-level gate, and the group and hull budgets.  Pure:
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
    CHECK(c.nQuotes == 4); // foot, truck, APC, tank (no armed car, no air in this ladder)
    // truck 12 + 1 + 3 = 16 (15.0); APC needs ceil(8.4 / 1.25) = 7: 7 + 2 + 8 = 17 (14.75)
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
