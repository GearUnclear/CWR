#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/AI/EntityAI.hpp>      // complete EntityAI for the npc LLink reads
#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/Market.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // ExtParsMission (load-pass config rebuild)
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <filesystem>
#include <string.h>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

// fake data package for the plan-15 resolution pass: a class exists when its
// name is on the list for the bank (CfgWeapons carries OFP magazines too)
struct FakeProbe final : ClassProbe
{
    std::vector<std::string> vehicles;
    std::vector<std::string> weapons;
    std::vector<std::string> magazines; // a package that ships a CfgMagazines bank

    bool Exists(const char* bank, const char* className) const override
    {
        const std::vector<std::string>* list = &vehicles;
        if (stricmp(bank, "CfgWeapons") == 0)
        {
            list = &weapons;
        }
        else if (stricmp(bank, "CfgMagazines") == 0)
        {
            list = &magazines;
        }
        for (const std::string& name : *list)
        {
            if (stricmp(name.c_str(), className) == 0)
            {
                return true;
            }
        }
        return false;
    }
};

// keeps the parsed ParamFile alive for the duration of a test
struct MarketFixture
{
    ParamFile file;
    Market market;

    void Load(const char* config, const ClassProbe* probe = nullptr)
    {
        QIStream in(config, strlen(config));
        file.Parse(in);
        market.LoadFromParams(file.FindEntry("CfgGuerrillaMarket"), file.FindEntry("CfgGuerrillaFactions"), probe);
    }
};

DealerCity City(const char* name, float x, float z)
{
    DealerCity c;
    c.name = name;
    c.pos = Vector3(x, 0, z);
    return c;
}

const char* kStockConfig = "class CfgGuerrillaMarket\n"
                           "{\n"
                           "    dealerShare = 0.5; dealerRespawnSeconds = 120; hqMoveCost = 750;\n"
                           "    weaponDealers[] = {\"Houdan\"};\n"
                           "    class Weapons\n"
                           "    {\n"
                           "        class AK   { weapon=\"AK47\"; magazine=\"AK47\"; mags=4; price=300; };\n"
                           "        class PK   { weapon=\"PK\";   magazine=\"PK\";   mags=2; price=900; };\n"
                           "        class Nade { weapon=\"\"; magazine=\"HandGrenade\"; mags=3; price=60; };\n"
                           "        class Bino { weapon=\"Binocular\"; magazine=\"\"; price=80; };\n"
                           "        class Ghost { weapon=\"G36a\"; magazine=\"G36a\"; mags=3; price=1200; };\n"
                           "        class BareM16 { weapon=\"M16\"; magazine=\"M16Mag\"; mags=3; price=400; };\n"
                           "        class Nothing { price=10; };\n"
                           "        class LostMag { weapon=\"\"; magazine=\"NoSuchMag\"; mags=2; price=20; };\n"
                           "        class Free { weapon=\"AK74\"; magazine=\"AK74\"; price=-5; };\n"
                           "    };\n"
                           "    class Vehicles\n"
                           "    {\n"
                           "        class Jeep  { vehicle=\"Jeep\";  price=2500; };\n"
                           "        class Mini  { vehicle=\"Mini\";  price=900; };\n"
                           "        class Blank { price=5; };\n"
                           "        class Ural  { vehicle=\"Ural\";  price=6000; };\n"
                           "    };\n"
                           "};\n"
                           "class CfgGuerrillaFactions\n"
                           "{\n"
                           "    class EAST { side=\"EAST\"; };\n"
                           "    class CIV  { side=\"CIV\"; civClassCount=1; civClass1=\"Civilian2\"; };\n"
                           "};\n";

} // namespace

TEST_CASE("Market - dealer quota per kind", "[game][guerrilla][market]")
{
    CHECK(Market::DealerQuota(0, 0.34f) == 0);
    CHECK(Market::DealerQuota(1, 0.34f) == 1);
    CHECK(Market::DealerQuota(2, 0.34f) == 1);
    CHECK(Market::DealerQuota(3, 0.34f) == 1);
    CHECK(Market::DealerQuota(6, 0.34f) == 2);
    CHECK(Market::DealerQuota(10, 0.34f) == 3);
    CHECK(Market::DealerQuota(10, 0.0f) == 1);  // never zero on a live island
    CHECK(Market::DealerQuota(10, 1.0f) == 10); // never above the city count
    CHECK(Market::DealerQuota(4, 2.0f) == 4);
}

TEST_CASE("Market - shuffle order is deterministic per seed and independent per salt", "[game][guerrilla][market]")
{
    AutoArray<Vector3> cities;
    cities.Add(Vector3(1000, 0, 1000));
    cities.Add(Vector3(5000, 0, 2000));
    cities.Add(Vector3(3000, 0, 8000));
    cities.Add(Vector3(9000, 0, 9000));
    cities.Add(Vector3(2000, 0, 6000));
    cities.Add(Vector3(7000, 0, 4000));

    AutoArray<int> a, b, c, d;
    Market::ShuffleOrder(12345, Market::SaltWeapon, cities, a);
    Market::ShuffleOrder(12345, Market::SaltWeapon, cities, b);
    Market::ShuffleOrder(12345, Market::SaltVehicle, cities, c);
    Market::ShuffleOrder(54321, Market::SaltWeapon, cities, d);

    REQUIRE(a.Size() == 6);
    // a permutation
    for (int i = 0; i < 6; i++)
    {
        bool found = false;
        for (int j = 0; j < a.Size() && !found; j++)
        {
            found = a[j] == i;
        }
        CHECK(found);
    }
    // same seed + salt -> same order
    for (int i = 0; i < 6; i++)
    {
        CHECK(a[i] == b[i]);
    }
    // a different salt or seed is a different draw (six cities: identical
    // orders from two independent draws are 1/720 - these seeds differ)
    bool saltDiffers = false;
    bool seedDiffers = false;
    for (int i = 0; i < 6; i++)
    {
        saltDiffers = saltDiffers || a[i] != c[i];
        seedDiffers = seedDiffers || a[i] != d[i];
    }
    CHECK(saltDiffers);
    CHECK(seedDiffers);
}

TEST_CASE("Market - dealer plan: quota per kind, overlap allowed, authored towns override", "[game][guerrilla][market]")
{
    AutoArray<DealerCity> cities;
    cities.Add(City("Houdan", 7100, 6000));
    cities.Add(City("Larche", 5900, 8600));
    cities.Add(City("Sainte Marie", 3200, 5100));
    cities.Add(City("Le Port", 8100, 2900));
    cities.Add(City("Dourdan", 9900, 7000));
    cities.Add(City("Arudy", 2100, 9400));

    SECTION("drawn: round(n * share) per kind, each dealer names a city")
    {
        AutoArray<RString> none;
        AutoArray<DealerRecord> rows;
        Market::PlanDealers(cities, 777, 0.34f, none, none, rows);
        int weapons = 0;
        int vehicles = 0;
        for (int i = 0; i < rows.Size(); i++)
        {
            bool known = false;
            for (int c = 0; c < cities.Size(); c++)
            {
                known = known || stricmp(cities[c].name, rows[i].zoneName) == 0;
            }
            CHECK(known);
            CHECK_FALSE(rows[i].placed);
            CHECK_FALSE(rows[i].spawned);
            (rows[i].kind == DKWeapon ? weapons : vehicles)++;
        }
        CHECK(weapons == 2);
        CHECK(vehicles == 2);
        // the same seed replans identically (the save carries the seed)
        AutoArray<DealerRecord> again;
        Market::PlanDealers(cities, 777, 0.34f, none, none, again);
        REQUIRE(again.Size() == rows.Size());
        for (int i = 0; i < rows.Size(); i++)
        {
            CHECK(Str(again[i].zoneName) == Str(rows[i].zoneName));
            CHECK(again[i].kind == rows[i].kind);
        }
    }

    SECTION("share 1.0 puts both kinds in every town (overlap is allowed)")
    {
        AutoArray<RString> none;
        AutoArray<DealerRecord> rows;
        Market::PlanDealers(cities, 1, 1.0f, none, none, rows);
        CHECK(rows.Size() == 12);
    }

    SECTION("authored lists override the draw for their kind; unknown towns are skipped")
    {
        AutoArray<RString> weaponTowns;
        weaponTowns.Add(RString("Larche"));
        weaponTowns.Add(RString("Nowhere"));
        weaponTowns.Add(RString("larche")); // duplicate, case-insensitive
        AutoArray<RString> none;
        AutoArray<DealerRecord> rows;
        Market::PlanDealers(cities, 5, 0.34f, weaponTowns, none, rows);
        int weapons = 0;
        int vehicles = 0;
        for (int i = 0; i < rows.Size(); i++)
        {
            if (rows[i].kind == DKWeapon)
            {
                weapons++;
                CHECK(Str(rows[i].zoneName) == "Larche");
                CHECK(rows[i].pos.X() == Approx(5900.0f));
            }
            else
            {
                vehicles++;
            }
        }
        CHECK(weapons == 1);
        CHECK(vehicles == 2); // the vehicle kind is still drawn
    }

    SECTION("no cities, no dealers")
    {
        AutoArray<DealerCity> empty;
        AutoArray<RString> none;
        AutoArray<DealerRecord> rows;
        Market::PlanDealers(empty, 9, 0.34f, none, none, rows);
        CHECK(rows.Size() == 0);
    }
}

TEST_CASE("Market - kind names", "[game][guerrilla][market]")
{
    CHECK(Str(Market::KindName(DKWeapon)) == "WEAPON");
    CHECK(Str(Market::KindName(DKVehicle)) == "VEHICLE");
    CHECK(Market::KindFromName("weapon") == DKWeapon);
    CHECK(Market::KindFromName("VEHICLE") == DKVehicle);
    CHECK(Market::KindFromName("arms") == DKWeapon);
    CHECK(Market::KindFromName("food") == -1);
    CHECK(Market::KindFromName(nullptr) == -1);
}

TEST_CASE("Market - unconfigured market is inert", "[game][guerrilla][market]")
{
    Market market;
    market.LoadFromParams(nullptr, nullptr, nullptr);
    CHECK_FALSE(market.IsConfigured());
    CHECK_FALSE(market.IsActive());
    CHECK_FALSE(market.IsAssigned());
    CHECK(market.NWeapons() == 0);
    CHECK(market.NVehicles() == 0);
    CHECK(market.DealerCount() == 0);
    CHECK(market.Dealer(0) == nullptr);
    CHECK(market.NearestDealer(DKWeapon, VZero) == -1);
    CHECK(market.Value("hqMoveCost") == Approx(500.0f)); // defaults still readable
    CHECK(market.Value("nothing") == Approx(0.0f));
    market.Simulate(10.0f); // no registry, no config: no-op
}

TEST_CASE("Market - stock parse with the package probe", "[game][guerrilla][market]")
{
    FakeProbe probe;
    probe.weapons = {"AK47", "PK", "HandGrenade", "Binocular", "M16", "AK74"};
    probe.vehicles = {"Jeep", "Ural", "Civilian"}; // no Mini, no Civilian2
    MarketFixture f;
    f.Load(kStockConfig, &probe);

    CHECK(f.market.IsConfigured());
    CHECK(f.market.Tuning().dealerShare == Approx(0.5f));
    CHECK(f.market.Value("dealerRespawnSeconds") == Approx(120.0f));
    CHECK(f.market.Value("hqMoveCost") == Approx(750.0f));
    // dealer body: no authored key; the CIV descriptor's Civilian2 fails the
    // probe, so the stock Civilian stands in
    CHECK(Str(f.market.Tuning().dealerClass) == "Civilian");
    CHECK(f.market.NAuthoredDealers(DKWeapon) == 1);
    CHECK(Str(f.market.AuthoredDealer(DKWeapon, 0)) == "Houdan");
    CHECK(f.market.NAuthoredDealers(DKVehicle) == 0);

    // weapons: AK, PK, Nade (magazine-only), Bino (weapon-only), BareM16
    // (unknown magazine -> sold bare), Free (price floored) survive; Ghost
    // (unknown weapon), Nothing (neither), LostMag (unknown magazine, no
    // weapon) are dropped. Config order is kept.
    REQUIRE(f.market.NWeapons() == 6);
    CHECK(Str(f.market.Weapon(0).name) == "AK");
    CHECK(Str(f.market.Weapon(0).weapon) == "AK47");
    CHECK(Str(f.market.Weapon(0).magazine) == "AK47");
    CHECK(f.market.Weapon(0).mags == 4);
    CHECK(f.market.Weapon(0).price == Approx(300.0f));
    CHECK(Str(f.market.Weapon(0).displayName) == "AK47"); // class name until the engine path localizes
    CHECK(Str(f.market.Weapon(1).name) == "PK");
    CHECK(Str(f.market.Weapon(2).name) == "Nade");
    CHECK(f.market.Weapon(2).weapon.GetLength() == 0);
    CHECK(Str(f.market.Weapon(2).magazine) == "HandGrenade");
    CHECK(f.market.Weapon(2).mags == 3);
    CHECK(Str(f.market.Weapon(2).displayName) == "HandGrenade");
    CHECK(Str(f.market.Weapon(3).name) == "Bino");
    CHECK(f.market.Weapon(3).magazine.GetLength() == 0);
    CHECK(f.market.Weapon(3).mags == 0);
    CHECK(Str(f.market.Weapon(4).name) == "BareM16");
    CHECK(Str(f.market.Weapon(4).weapon) == "M16");
    CHECK(f.market.Weapon(4).magazine.GetLength() == 0);
    CHECK(f.market.Weapon(4).mags == 0);
    CHECK(Str(f.market.Weapon(5).name) == "Free");
    CHECK(f.market.Weapon(5).mags == 1); // a magazine with no count ships one
    CHECK(f.market.Weapon(5).price == Approx(0.0f));

    // vehicles: Jeep, Ural survive; Mini (unknown) and Blank (no class) drop
    REQUIRE(f.market.NVehicles() == 2);
    CHECK(Str(f.market.Vehicle(0).vehicle) == "Jeep");
    CHECK(f.market.Vehicle(0).price == Approx(2500.0f));
    CHECK(Str(f.market.Vehicle(1).vehicle) == "Ural");
}

TEST_CASE("Market - stock parse without a probe keeps every well-formed row", "[game][guerrilla][market]")
{
    MarketFixture f;
    f.Load(kStockConfig, nullptr);
    // only the structurally broken rows go: Nothing (no class at all)
    CHECK(f.market.NWeapons() == 8);
    CHECK(f.market.NVehicles() == 3);
    // no probe: the CIV descriptor's civClass1 is trusted
    CHECK(Str(f.market.Tuning().dealerClass) == "Civilian2");
}

TEST_CASE("Market - dealerClass key and a CfgMagazines bank are honoured", "[game][guerrilla][market]")
{
    const char* config = "class CfgGuerrillaMarket\n"
                         "{\n"
                         "    dealerClass = \"LoBo_Civ_01\";\n"
                         "    class Weapons { class R { weapon=\"LoBoAK47\"; magazine=\"JAM_E762_30mag\"; mags=4; "
                         "price=350; }; };\n"
                         "};\n";
    FakeProbe probe;
    probe.vehicles = {"LoBo_Civ_01"};
    probe.weapons = {"LoBoAK47"};
    probe.magazines = {"JAM_E762_30mag"};
    MarketFixture f;
    f.Load(config, &probe);
    CHECK(Str(f.market.Tuning().dealerClass) == "LoBo_Civ_01");
    REQUIRE(f.market.NWeapons() == 1);
    CHECK(Str(f.market.Weapon(0).magazine) == "JAM_E762_30mag");
}

TEST_CASE("Market - nearest dealer of a kind", "[game][guerrilla][market]")
{
    MarketFixture f;
    f.Load("class CfgGuerrillaMarket { dealerShare = 1.0; };");
    AutoArray<DealerCity> cities;
    cities.Add(City("Near", 1000, 1000));
    cities.Add(City("Far", 9000, 9000));
    f.market.AssignForTest(cities, 42);
    REQUIRE(f.market.IsAssigned());
    REQUIRE(f.market.DealerCount() == 4);
    int w = f.market.NearestDealer(DKWeapon, Vector3(1200, 0, 900));
    REQUIRE(w >= 0);
    CHECK(Str(f.market.Dealer(w)->zoneName) == "Near");
    CHECK(f.market.Dealer(w)->kind == DKWeapon);
    int v = f.market.NearestDealer(DKVehicle, Vector3(8000, 0, 8000));
    REQUIRE(v >= 0);
    CHECK(Str(f.market.Dealer(v)->zoneName) == "Far");
    CHECK(f.market.Dealer(v)->kind == DKVehicle);
    CHECK(f.market.NearestDealer(5, VZero) == -1);
}

TEST_CASE("Market - assignment and rows survive save/load by zone name", "[game][guerrilla][market][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "market-roundtrip.bin";

    // the load pass rebuilds stock + tuning from the mission config and
    // matches rows to the rebuilt zone table by name
    const char* config = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    class Zones\n"
                         "    {\n"
                         "        class Houdan { name=\"Houdan\"; type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={7100.0, 6000.0, 35.0}; };\n"
                         "        class Larche { name=\"Larche\"; type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={5900.0, 8600.0, 20.0}; };\n"
                         "    };\n"
                         "};\n"
                         "class CfgGuerrillaMarket\n"
                         "{\n"
                         "    dealerShare = 1.0; hqMoveCost = 321;\n"
                         "    class Weapons { class AK { weapon=\"AK47\"; magazine=\"AK47\"; mags=2; price=100; }; };\n"
                         "};\n";
    {
        QIStream in(config, strlen(config));
        ExtParsMission.Parse(in);
    }
    ZoneRegistry::Instance().InitMission();
    REQUIRE(ZoneRegistry::Instance().FindZoneIndex("Houdan") >= 0);

    {
        Market market;
        market.LoadFromConfig();
        REQUIRE(market.IsConfigured());
        AutoArray<DealerCity> cities;
        cities.Add(City("Houdan", 7100, 6000));
        cities.Add(City("Larche", 5900, 8600));
        cities.Add(City("Ghost", 100, 100)); // not a zone of the config above
        market.AssignForTest(cities, 4242);
        REQUIRE(market.DealerCount() == 6);

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(market.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
        CHECK(market.DealerCount() == 6); // saving does not disturb the rows
    }

    Market loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }

    CHECK(loaded.IsAssigned());
    CHECK(loaded.IsConfigured());
    CHECK(loaded.Value("hqMoveCost") == Approx(321.0f));
    // (the stock rows are re-read through the ENGINE probe on the load pass;
    // a unit test has no Pars, so every classname fails it and the stock
    // comes back empty - the parse itself is pinned by the probe cases above)
    // the Ghost rows (two kinds) were dropped - their town is no zone
    CHECK(loaded.DealerCount() == 4);
    for (int i = 0; i < loaded.DealerCount(); i++)
    {
        const DealerRecord* d = loaded.Dealer(i);
        REQUIRE(d != nullptr);
        CHECK((Str(d->zoneName) == "Houdan" || Str(d->zoneName) == "Larche"));
        CHECK(d->npc.GetLink() == nullptr); // no live world: NPCs respawn on the first tick
    }

    // scrub the process-wide state other tests expect to be empty
    ExtParsMission.Clear();
    ZoneRegistry::Instance().Clear();
    std::filesystem::remove(archivePath);
}
