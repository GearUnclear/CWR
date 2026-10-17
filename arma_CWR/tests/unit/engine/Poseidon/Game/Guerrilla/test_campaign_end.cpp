// Guerrilla campaign end state (issue #79 B3): the win rule, its latch, the
// end clock and the save contract.  Pure: the registry is built from inline
// config, ownership is flipped by hand, and CampaignEnd is driven through
// Evaluate / AdvanceEnd (Simulate is the engine wrapper over the same calls).

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // ExtParsMission (load-pass tuning reread)
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <filesystem>
#include <string.h>
#include <string>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

// Two enemy bases (one by the OCCUPIER token, one by the occupier's literal
// side), an enemy airfield, plus every zone the rule must leave out: the Camp,
// a town, a resistance-authored outpost and a neutral seaport.
const char* kWarConfig = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    campaignEndDelay = 30;\n"
                         "    class Zones\n"
                         "    {\n"
                         "        class Camp    { name=\"Camp\";    type=\"CAMP\";     owner=\"RESISTANCE\"; "
                         "position[]={1000, 1000, 0}; };\n"
                         "        class Town    { name=\"Town\";    type=\"CITY\";     owner=\"NEUTRAL\";    "
                         "support=20; position[]={1500, 1000, 0}; };\n"
                         "        class Outpost { name=\"Outpost\"; type=\"OUTPOST\";  owner=\"OCCUPIER\";   "
                         "garrison=6; position[]={2000, 1000, 0}; };\n"
                         "        class Field   { name=\"Field\";   type=\"airfield\"; owner=\"EAST\";       "
                         "garrison=12; position[]={3000, 1000, 0}; };\n"
                         "        class Ours    { name=\"Ours\";    type=\"OUTPOST\";  owner=\"RESISTANCE\"; "
                         "position[]={4000, 1000, 0}; };\n"
                         "        class Harbour { name=\"Harbour\"; type=\"SEAPORT\";  owner=\"NEUTRAL\";    "
                         "position[]={5000, 1000, 0}; };\n"
                         "    };\n"
                         "};\n";

// Nothing the occupier holds that is a base: a campaign that can never be won.
const char* kNoBaseConfig = "class CfgGuerrillaZones\n"
                            "{\n"
                            "    class Zones\n"
                            "    {\n"
                            "        class Camp { name=\"Camp\"; type=\"CAMP\"; owner=\"RESISTANCE\"; "
                            "position[]={1000, 1000, 0}; };\n"
                            "        class Town { name=\"Town\"; type=\"CITY\"; owner=\"OCCUPIER\"; "
                            "position[]={1500, 1000, 0}; };\n"
                            "        class Ours { name=\"Ours\"; type=\"OUTPOST\"; owner=\"RESISTANCE\"; "
                            "position[]={4000, 1000, 0}; };\n"
                            "    };\n"
                            "};\n";

struct Fixture
{
    ParamFile file;
    ZoneRegistry registry;
    CampaignEnd end;

    void Load(const char* config)
    {
        QIStream in(config, strlen(config));
        file.Parse(in);
        // no faction table: the built-in EAST / GUER campaign sides
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), nullptr);
        end.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
    }

    void SetOwner(const char* zone, const char* owner)
    {
        const int index = registry.FindZoneIndex(zone);
        REQUIRE(index >= 0);
        registry.GetZoneMutable(index)->owner = owner;
    }
};

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

} // namespace

TEST_CASE("CampaignEnd - military types are everything but CAMP and CITY", "[game][guerrilla][campaignend]")
{
    CHECK(CampaignEnd::IsMilitaryType("OUTPOST"));
    CHECK(CampaignEnd::IsMilitaryType("AIRFIELD"));
    CHECK(CampaignEnd::IsMilitaryType("seaport"));
    CHECK(CampaignEnd::IsMilitaryType("COMPOUND")); // a type no code knows yet still counts
    CHECK_FALSE(CampaignEnd::IsMilitaryType("CAMP"));
    CHECK_FALSE(CampaignEnd::IsMilitaryType("city"));
    CHECK_FALSE(CampaignEnd::IsMilitaryType(""));
    CHECK_FALSE(CampaignEnd::IsMilitaryType(nullptr));
}

TEST_CASE("CampaignEnd - the roster is the authored enemy military zones", "[game][guerrilla][campaignend]")
{
    Fixture f;
    f.Load(kWarConfig);
    REQUIRE(f.registry.IsActive());
    CHECK(f.end.Tuning().endDelay == Approx(30.0f));

    CHECK_FALSE(f.end.HasRoster());
    CHECK_FALSE(f.end.Evaluate(f.registry));
    REQUIRE(f.end.HasRoster());
    REQUIRE(f.end.RosterSize() == 2);
    CHECK(Str(f.end.RosterName(0)) == "Outpost");
    CHECK(Str(f.end.RosterName(1)) == "Field");
    // the Camp, the town, the resistance outpost and the neutral harbour are out
    CHECK(Str(f.end.RosterName(2)) == "");
    CHECK(f.end.RosterHeld(f.registry) == 0);
    CHECK(f.end.RosterPresent(f.registry) == 2);
}

TEST_CASE("CampaignEnd - zero enemy military zones never wins", "[game][guerrilla][campaignend]")
{
    Fixture f;
    f.Load(kNoBaseConfig);
    REQUIRE(f.registry.IsActive());

    CHECK_FALSE(f.end.Evaluate(f.registry));
    CHECK(f.end.HasRoster());
    CHECK(f.end.RosterSize() == 0);
    // every zone on the map in resistance hands, the occupier town included
    f.SetOwner("Town", "GUER");
    f.SetOwner("Ours", "GUER");
    for (int tick = 0; tick < 5; tick++)
    {
        CHECK_FALSE(f.end.Evaluate(f.registry));
    }
    CHECK_FALSE(f.end.Won());
    CHECK_FALSE(f.end.AllHeld(f.registry));
    CHECK_FALSE(f.end.AdvanceEnd(100.0f));
}

TEST_CASE("CampaignEnd - campaignEnd = 0 switches the rule off", "[game][guerrilla][campaignend]")
{
    std::string config = kWarConfig;
    const std::string key = "    campaignEndDelay = 30;\n";
    const size_t at = config.find(key);
    REQUIRE(at != std::string::npos);
    config.insert(at, "    campaignEnd = 0;\n");

    Fixture f;
    f.Load(config.c_str());
    CHECK_FALSE(f.end.Tuning().enabled);
    f.SetOwner("Outpost", "GUER");
    f.SetOwner("Field", "GUER");
    CHECK_FALSE(f.end.Evaluate(f.registry));
    CHECK_FALSE(f.end.HasRoster()); // nothing to save either
    CHECK_FALSE(f.end.HasState());
    CHECK_FALSE(f.end.Won());

    Fixture on;
    on.Load(kWarConfig);
    CHECK(on.end.Tuning().enabled); // the default
}

TEST_CASE("CampaignEnd - a partial hold does not win", "[game][guerrilla][campaignend]")
{
    Fixture f;
    f.Load(kWarConfig);
    CHECK_FALSE(f.end.Evaluate(f.registry));

    f.SetOwner("Outpost", "GUER");
    CHECK_FALSE(f.end.Evaluate(f.registry));
    CHECK(f.end.RosterHeld(f.registry) == 1);
    // towns are not part of the rule: a risen town changes nothing
    f.SetOwner("Town", "GUER");
    CHECK_FALSE(f.end.Evaluate(f.registry));
    CHECK_FALSE(f.end.Won());
    CHECK_FALSE(f.end.EndPending());
}

TEST_CASE("CampaignEnd - holding every enemy base latches the win once", "[game][guerrilla][campaignend]")
{
    Fixture f;
    f.Load(kWarConfig);
    CHECK_FALSE(f.end.Evaluate(f.registry, 2, 600));
    f.SetOwner("Outpost", "GUER");
    CHECK_FALSE(f.end.Evaluate(f.registry, 2, 610));
    f.SetOwner("Field", "GUER");

    CHECK(f.end.Evaluate(f.registry, 3, 75)); // the latch edge
    CHECK(f.end.Won());
    CHECK(f.end.WonDay() == 3);
    CHECK(f.end.WonMinute() == 75);
    CHECK(Str(f.end.LastTaken()) == "Field");
    CHECK(f.end.EndPending());
    CHECK(f.end.EndClock() == Approx(30.0f));
    // the edge never repeats
    CHECK_FALSE(f.end.Evaluate(f.registry, 3, 80));
    CHECK_FALSE(f.end.Evaluate(f.registry, 3, 85));
    CHECK(f.end.WonDay() == 3);
}

TEST_CASE("CampaignEnd - the latch survives a later loss", "[game][guerrilla][campaignend]")
{
    Fixture f;
    f.Load(kWarConfig);
    f.SetOwner("Outpost", "GUER");
    f.SetOwner("Field", "GUER");
    REQUIRE(f.end.Evaluate(f.registry, 4, 0));
    CHECK(Str(f.end.LastTaken()) == ""); // won on the first check: nobody fell "last"

    // a base changes hands back: the win stands and does not fire again
    f.SetOwner("Field", "EAST");
    CHECK_FALSE(f.end.AllHeld(f.registry));
    CHECK_FALSE(f.end.Evaluate(f.registry, 4, 10));
    CHECK(f.end.Won());
    f.SetOwner("Field", "GUER");
    CHECK_FALSE(f.end.Evaluate(f.registry, 4, 20));
    CHECK(f.end.WonDay() == 4);
    CHECK(f.end.WonMinute() == 0);
}

TEST_CASE("CampaignEnd - the authored owner fixes the roster, not the live one", "[game][guerrilla][campaignend]")
{
    // a campaign derived from a save written before the service existed: the
    // Outpost already fell, but it was the enemy's and stays on the roster
    Fixture f;
    f.Load(kWarConfig);
    f.SetOwner("Outpost", "GUER");
    CHECK_FALSE(f.end.Evaluate(f.registry));
    CHECK(f.end.RosterSize() == 2);
    CHECK(f.end.RosterHeld(f.registry) == 1);

    // and the roster is fixed once taken: a base the occupier wins later
    // (here the neutral harbour) is not added
    f.SetOwner("Harbour", "EAST");
    f.SetOwner("Field", "GUER");
    CHECK(f.end.Evaluate(f.registry));
}

TEST_CASE("CampaignEnd - the end clock fires exactly once", "[game][guerrilla][campaignend]")
{
    Fixture f;
    f.Load(kWarConfig);
    CHECK_FALSE(f.end.AdvanceEnd(1000.0f)); // nothing to end before the win
    f.SetOwner("Outpost", "GUER");
    f.SetOwner("Field", "GUER");
    REQUIRE(f.end.Evaluate(f.registry));

    CHECK_FALSE(f.end.AdvanceEnd(10.0f));
    CHECK(f.end.EndClock() == Approx(20.0f));
    // the script knob restarts a pending clock
    f.end.SetEndDelay(5.0f);
    CHECK(f.end.EndClock() == Approx(5.0f));
    CHECK_FALSE(f.end.AdvanceEnd(4.0f));
    CHECK(f.end.AdvanceEnd(2.0f));
    CHECK(f.end.EndIssued());
    CHECK_FALSE(f.end.EndPending());
    CHECK(f.end.EndClock() == Approx(0.0f));
    CHECK_FALSE(f.end.AdvanceEnd(10.0f));

    // a zero delay ends on the latch tick itself
    Fixture g;
    g.Load(kWarConfig);
    g.end.SetEndDelay(-3.0f); // clamped to 0
    CHECK(g.end.Tuning().endDelay == Approx(0.0f));
    g.SetOwner("Outpost", "GUER");
    g.SetOwner("Field", "GUER");
    REQUIRE(g.end.Evaluate(g.registry));
    CHECK(g.end.AdvanceEnd(0.0f));
}

TEST_CASE("CampaignEnd - roster, latch and end clock survive save/load", "[game][guerrilla][campaignend][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path wonPath = dir / "campaign-end-won.bin";
    const std::filesystem::path openPath = dir / "campaign-end-open.bin";
    const std::filesystem::path emptyPath = dir / "campaign-end-empty.bin";

    // the load's second pass rereads the tuning from the mission config,
    // exactly as SetMission's description.ext reparse would leave it
    {
        QIStream in(kWarConfig, strlen(kWarConfig));
        ExtParsMission.Parse(in);
    }

    Fixture f;
    f.Load(kWarConfig);
    // an open campaign: the roster is taken, one base held
    f.SetOwner("Outpost", "GUER");
    CHECK_FALSE(f.end.Evaluate(f.registry));
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.end.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(openPath.string().c_str()));
    }
    // the same campaign won inside the end window, 12 s left on the clock
    f.SetOwner("Field", "GUER");
    REQUIRE(f.end.Evaluate(f.registry, 6, 900));
    CHECK_FALSE(f.end.AdvanceEnd(18.0f));
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.end.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(wonPath.string().c_str()));
    }

    auto load = [](const std::filesystem::path& path, CampaignEnd& into)
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(into.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(into.Serialize(ar) == LSOK);
    };

    SECTION("the won campaign keeps its latch and resumes the clock")
    {
        CampaignEnd loaded;
        load(wonPath, loaded);
        CHECK(loaded.HasRoster());
        REQUIRE(loaded.RosterSize() == 2);
        CHECK(Str(loaded.RosterName(0)) == "Outpost");
        CHECK(Str(loaded.RosterName(1)) == "Field");
        CHECK(loaded.Won());
        CHECK(loaded.WonDay() == 6);
        CHECK(loaded.WonMinute() == 900);
        CHECK(Str(loaded.LastTaken()) == "Field");
        CHECK(loaded.EndPending());
        CHECK(loaded.EndClock() == Approx(12.0f));
        CHECK(loaded.Tuning().endDelay == Approx(30.0f)); // reread from the mission config

        // a base lost after the load cannot undo the win or fire it again
        f.SetOwner("Outpost", "EAST");
        CHECK_FALSE(loaded.Evaluate(f.registry));
        CHECK(loaded.Won());
        CHECK_FALSE(loaded.AdvanceEnd(11.0f));
        CHECK(loaded.AdvanceEnd(1.0f));
        CHECK_FALSE(loaded.AdvanceEnd(1.0f));
    }

    SECTION("the open campaign keeps its roster, not the registry's current owners")
    {
        CampaignEnd loaded;
        load(openPath, loaded);
        CHECK(loaded.HasRoster());
        CHECK(loaded.RosterSize() == 2);
        CHECK_FALSE(loaded.Won());
        CHECK_FALSE(loaded.EndPending());
        // a fresh registry: the saved roster is what counts, so holding its
        // two bases wins
        Fixture other;
        other.Load(kWarConfig);
        other.SetOwner("Outpost", "GUER");
        CHECK_FALSE(loaded.Evaluate(other.registry));
        other.SetOwner("Field", "GUER");
        CHECK(loaded.Evaluate(other.registry));
        CHECK(Str(loaded.LastTaken()) == "Field");
    }

    SECTION("a zero-base campaign stays unwinnable across a load")
    {
        Fixture none;
        none.Load(kNoBaseConfig);
        CHECK_FALSE(none.end.Evaluate(none.registry));
        {
            ParamArchiveSave ar(WorldSerializeVersion);
            REQUIRE(none.end.Serialize(ar) == LSOK);
            REQUIRE(ar.SaveBin(emptyPath.string().c_str()));
        }
        CampaignEnd loaded;
        load(emptyPath, loaded);
        CHECK(loaded.HasRoster());
        CHECK(loaded.RosterSize() == 0);
        // the roster is NOT retaken from a registry that now has enemy bases
        f.SetOwner("Outpost", "GUER");
        f.SetOwner("Field", "GUER");
        CHECK_FALSE(loaded.Evaluate(f.registry));
        CHECK_FALSE(loaded.Won());
    }

    ExtParsMission.Clear();
    std::filesystem::remove(wonPath);
    std::filesystem::remove(openPath);
    std::filesystem::remove(emptyPath);
}

TEST_CASE("CampaignEnd - the victory and debriefing text", "[game][guerrilla][campaignend]")
{
    const std::string line = Str(CampaignEnd::VictoryLine("on Malden", "Outpost"));
    CHECK(line == "Outpost is ours. It was the last base they held on Malden.");
    CHECK(Str(CampaignEnd::VictoryLine("in the Sinai", "")) ==
          "We took their last base. They hold nothing in the Sinai now.");
    CHECK(Str(CampaignEnd::VictoryLine(nullptr, nullptr)) ==
          "We took their last base. They hold nothing on the island now.");
    CHECK(Str(CampaignEnd::DebriefingTitle("Malden")) == "Malden is free.");
    CHECK(Str(CampaignEnd::DebriefingTitle("")) == "The island is free.");
    CHECK(Str(CampaignEnd::DebriefingBody("on Malden", 4, 9)) ==
          "Every occupier base on Malden is in resistance hands, 4 taken in all. The campaign was won on Day 9.");

    // house style: no em dashes in player-facing text
    for (const std::string& s : {line, Str(CampaignEnd::DebriefingBody("on Malden", 1, 0))})
    {
        CHECK(s.find("\xE2\x80\x94") == std::string::npos);
        CHECK(s.find(" -- ") == std::string::npos);
    }
}
