// Names-block classification for the CITY auto-seed (issue #54 C3): the
// seedCities key is a tri-state (absent = Auto with the settlement probe,
// 1 = legacy seed-everything, 0 = seed nothing), so a theatre-map Names
// block no longer needs seedCities=0 plus hand-placed towns.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <string.h>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

// a landscape stand-in: only the listed names have houses around them
struct FakeSettlement final : SettlementProbe
{
    std::vector<std::string> towns;
    mutable std::vector<std::string> asked;
    bool IsSettlement(const char* name, const Vector3& /*pos*/) const override
    {
        asked.push_back(name);
        for (const std::string& t : towns)
        {
            if (stricmp(t.c_str(), name) == 0)
            {
                return true;
            }
        }
        return false;
    }
};

struct Parsed
{
    ParamFile file;
    explicit Parsed(const char* text)
    {
        QIStream in(text, strlen(text));
        file.Parse(in);
    }
    const ParamEntry* Zones() const { return file.FindEntry("CfgGuerrillaZones"); }
    const ParamEntry* Names() const { return file.FindEntry("Names"); }
};

// Lebanon80's shape: a type-less theatre map. Tyre is a real town with an
// authored zone on top of it (name dedup), Saida a real town 400 m off its
// authored zone (distance dedup misses, name dedup catches), Ghajar a real
// town nobody authored, the rest labels.
const char* kZonesAuto =
    "class CfgGuerrillaZones\n"
    "{\n"
    "    class Zones\n"
    "    {\n"
    "        class Camp  { name=\"Camp\";  type=\"CAMP\"; owner=\"GUER\"; position[]={1000.0, 1000.0, 0.0}; };\n"
    "        class Tyre  { name=\"Tyre\";  type=\"CITY\"; position[]={5000.0, 5000.0, 0.0}; };\n"
    "        class Saida { name=\"Saida\"; type=\"CITY\"; position[]={9000.0, 9000.0, 0.0}; };\n"
    "    };\n"
    "};\n";
const char* kZonesAll = "class CfgGuerrillaZones\n"
                        "{\n"
                        "    seedCities = 1;\n"
                        "    class Zones { class Camp { name=\"Camp\"; type=\"CAMP\"; owner=\"GUER\"; "
                        "position[]={1000.0, 1000.0, 0.0}; }; };\n"
                        "};\n";
const char* kZonesOff = "class CfgGuerrillaZones\n"
                        "{\n"
                        "    seedCities = 0;\n"
                        "    class Zones { class Camp { name=\"Camp\"; type=\"CAMP\"; owner=\"GUER\"; "
                        "position[]={1000.0, 1000.0, 0.0}; }; };\n"
                        "};\n";
const char* kNames = "class Names\n"
                     "{\n"
                     "    class MedSea   { name=\"Mediterranean Sea\"; position[]={7847.0, 16012.0}; };\n"
                     "    class Galilee  { name=\"Sea of Galilee\";    position[]={13856.0, 4724.0}; };\n"
                     "    class Lebanon  { name=\"Lebanon\";           position[]={12000.0, 12000.0}; };\n"
                     "    class Hermon   { name=\"Mt. Hermon\";        position[]={20000.0, 8000.0}; };\n"
                     "    class Damascus { name=\"Damascus\";          position[]={24000.0, 3000.0}; };\n"
                     "    class Tyre     { name=\"Tyre\";              position[]={5100.0, 5100.0}; };\n"
                     "    class Saida    { name=\"Saida\";             position[]={9400.0, 9000.0}; };\n"
                     "    class Ghajar   { name=\"Ghajar\";            position[]={17000.0, 6000.0}; };\n"
                     "    class Rocks    { name=\"Rocks\"; type=\"RockArea\"; position[]={3000.0, 3000.0}; };\n"
                     "};\n";

std::vector<std::string> ZoneNames(const ZoneRegistry& reg)
{
    std::vector<std::string> out;
    for (int i = 0; i < reg.NZones(); i++)
    {
        out.push_back(Str(reg.GetZone(i)->name));
    }
    return out;
}

} // namespace

TEST_CASE("seedCities tri-state: absent = Auto, 0 = Off, 1 = All", "[game][guerrilla][names]")
{
    Parsed autoCfg(kZonesAuto);
    Parsed allCfg(kZonesAll);
    Parsed offCfg(kZonesOff);
    REQUIRE(ZoneRegistry::ReadSeedCitiesMode(autoCfg.Zones()) == ZoneTuning::SeedCities::Auto);
    REQUIRE(ZoneRegistry::ReadSeedCitiesMode(allCfg.Zones()) == ZoneTuning::SeedCities::All);
    REQUIRE(ZoneRegistry::ReadSeedCitiesMode(offCfg.Zones()) == ZoneTuning::SeedCities::Off);
    REQUIRE(ZoneRegistry::ReadSeedCitiesMode(nullptr) == ZoneTuning::SeedCities::Auto);
}

TEST_CASE("Auto seeding asks the settlement probe and seeds only what it accepts", "[game][guerrilla][names]")
{
    Parsed cfg(kZonesAuto);
    Parsed names(kNames);
    FakeSettlement land;
    land.towns = {"Tyre", "Saida", "Ghajar"};

    ZoneRegistry reg;
    reg.LoadFromParams(cfg.Zones(), nullptr, nullptr, nullptr, names.Names(), nullptr, &land);
    REQUIRE(reg.Tuning().seedCities == ZoneTuning::SeedCities::Auto);
    // the three authored zones plus Ghajar, and nothing spurious: the seas,
    // the country, the mountain and the deep-rear city were all refused, the
    // typed non-town never reached the probe, Tyre/Saida were deduped by name
    REQUIRE(ZoneNames(reg) == std::vector<std::string>{"Camp", "Tyre", "Saida", "Ghajar"});
    REQUIRE(Str(reg.GetZone(3)->type) == "CITY");
    // the probe never sees the typed non-town (type filter runs first)
    for (const std::string& n : land.asked)
    {
        REQUIRE(n != "Rocks");
    }
}

TEST_CASE("All (seedCities = 1) keeps the legacy seed-everything behaviour, probe ignored", "[game][guerrilla][names]")
{
    Parsed cfg(kZonesAll);
    Parsed names(kNames);
    FakeSettlement land; // accepts nothing
    ZoneRegistry reg;
    reg.LoadFromParams(cfg.Zones(), nullptr, nullptr, nullptr, names.Names(), nullptr, &land);
    REQUIRE(reg.Tuning().seedCities == ZoneTuning::SeedCities::All);
    REQUIRE(reg.NZones() == 1 + 8); // Camp + every entry but the typed RockArea
    REQUIRE(land.asked.empty());
    REQUIRE(reg.FindZoneIndex("Mediterranean Sea") >= 0);
}

TEST_CASE("Off (seedCities = 0) seeds nothing; Auto without a probe seeds like All", "[game][guerrilla][names]")
{
    Parsed names(kNames);
    {
        Parsed cfg(kZonesOff);
        FakeSettlement land;
        land.towns = {"Tyre"};
        ZoneRegistry reg;
        reg.LoadFromParams(cfg.Zones(), nullptr, nullptr, nullptr, names.Names(), nullptr, &land);
        REQUIRE(reg.NZones() == 1);
        REQUIRE(land.asked.empty());
    }
    {
        // no landscape to classify against (headless config tests): the
        // pre-C3 answer, so every existing fixture keeps its zone count
        Parsed cfg(kZonesAuto);
        ZoneRegistry reg;
        reg.LoadFromParams(cfg.Zones(), nullptr, nullptr, nullptr, names.Names(), nullptr, nullptr);
        REQUIRE(reg.NZones() == 3 + 6); // Camp/Tyre/Saida + 8 entries minus the two name-deduped
    }
}

TEST_CASE("CollectTownNames at menu time: absent key lists the town-typed entries, 0 lists none",
          "[game][guerrilla][names]")
{
    Parsed names(kNames);
    AutoArray<RString> out;
    {
        Parsed cfg(kZonesAuto);
        ZoneRegistry::CollectTownNames(cfg.Zones(), names.Names(), out);
        // authored CITY zones first, then the Names towns the mission MAY seed
        // (the classifier needs the landscape, which the menu has not loaded)
        REQUIRE(out.Size() == 2 + 6);
        REQUIRE(Str(out[0]) == "Tyre");
        REQUIRE(Str(out[1]) == "Saida");
    }
    {
        Parsed cfg(kZonesOff);
        ZoneRegistry::CollectTownNames(cfg.Zones(), names.Names(), out);
        REQUIRE(out.Size() == 0);
    }
    {
        Parsed cfg(kZonesAll);
        ZoneRegistry::CollectTownNames(cfg.Zones(), names.Names(), out);
        REQUIRE(out.Size() == 8);
    }
}
