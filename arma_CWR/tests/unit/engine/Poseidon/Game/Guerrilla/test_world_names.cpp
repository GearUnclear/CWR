// WorldNames: the one shared reader over CfgWorlds >> <world> >> Names, plus the
// faction display name the Game layer and the UI layer now share.
//
// Parser-only, like test_names_classify.cpp: a ParamFile built from a text
// buffer stands in for the world config, so nothing here needs a world, a
// landscape or a data package.
//
// The two contracts worth stating out loud, because both are easy to get wrong:
//   * position[] element [2] is the OFP map LABEL SIZE, not an elevation.  It is
//     discarded and pos.Y is always 0 (UIMap reads [0]/[1] and zeroes Y).
//   * ZoneRegistry::NamesEntryIsTown is asked for its BOOLEAN only.  It returns
//     false before writing its name / pos out-params for a typed non-town entry
//     or a short position[], and those rows are precisely what FeatureNames has
//     to hand the history generator with a usable name.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/WorldNames.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <Poseidon/Foundation/platform.hpp>

#include <string.h>
#include <string>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

struct Parsed
{
    ParamFile file;
    explicit Parsed(const char* text)
    {
        QIStream in(text, strlen(text));
        file.Parse(in);
    }
    const ParamEntry* Names() const { return file.FindEntry("Names"); }
    const ParamEntry* Zones() const { return file.FindEntry("CfgGuerrillaZones"); }
    const ParamEntry* Factions() const { return file.FindEntry("CfgGuerrillaFactions"); }
};

// One Names block carrying every shape the readers have to survive: an OFP
// 2-element position, a 3-element one whose [2] is a label size, an Arma-typed
// village, a typed non-town (the FeatureNames case), an entry with no `name`
// key at all, an entry whose position[] is too short to place, and a second
// entry that repeats a display name.
const char* kNames = "class Names\n"
                     "{\n"
                     "    class Houdan   { name=\"Houdan\"; position[]={5000.0, 6000.0}; };\n"
                     "    class Larche   { name=\"Larche\"; position[]={7000.0, 8000.0, 40.0}; };\n"
                     "    class Chapoi   { name=\"Chapoi\"; type=\"NameVillage\"; position[]={9000.0, 1000.0}; };\n"
                     "    class Cliffs   { name=\"The Cliffs\"; type=\"RockArea\"; position[]={2000.0, 3000.0}; };\n"
                     "    class Bourdeaux { position[]={4000.0, 4500.0}; };\n"
                     "    class Waypoint { name=\"Nowhere\"; position[]={1234.0}; };\n"
                     "    class HoudanToo { name=\"Houdan\"; position[]={5100.0, 6100.0}; };\n"
                     "};\n";

const char* kFactions = "class CfgGuerrillaZones { class Zones { class Camp { name=\"Camp\"; owner=\"GUER\"; "
                        "position[]={1000.0, 1000.0, 0.0}; }; }; };\n"
                        "class CfgGuerrillaFactions\n"
                        "{\n"
                        "    class PLO_East { side=\"GUER\"; };\n"
                        "    class East     { side=\"EAST\"; displayName=\"Soviet Army\"; };\n"
                        "    class Odd_One  { side=\"WEST\"; displayName=\"Task_Force Alpha\"; };\n"
                        "};\n";

} // namespace

TEST_CASE("WorldNames - the Names block reads in config order with the label size discarded",
          "[game][guerrilla][worldnames]")
{
    Parsed cfg(kNames);
    REQUIRE(cfg.Names() != nullptr);

    AutoArray<PlaceName> all;
    CollectPlaceNames(cfg.Names(), all);
    REQUIRE(all.Size() == 7);

    // config ORDER is the iteration order the history generator's place draws
    // depend on
    CHECK(Str(all[0].key) == "Houdan");
    CHECK(Str(all[1].key) == "Larche");
    CHECK(Str(all[2].key) == "Chapoi");
    CHECK(Str(all[3].key) == "Cliffs");
    CHECK(Str(all[4].key) == "Bourdeaux");
    CHECK(Str(all[5].key) == "Waypoint");
    CHECK(Str(all[6].key) == "HoudanToo");

    // a 2-element OFP position maps to engine axes with Y zeroed
    CHECK(all[0].pos.X() == 5000.0f);
    CHECK(all[0].pos.Y() == 0.0f);
    CHECK(all[0].pos.Z() == 6000.0f);
    // element [2] is the map LABEL SIZE, not an elevation: discarded
    CHECK(all[1].pos.X() == 7000.0f);
    CHECK(all[1].pos.Y() == 0.0f);
    CHECK(all[1].pos.Z() == 8000.0f);

    // the entry with no `name` key falls back to its class key
    CHECK(Str(all[4].name) == "Bourdeaux");
    // every row carries a display name, whatever its type or position shape
    for (int i = 0; i < all.Size(); i++)
    {
        CHECK(all[i].name.GetLength() > 0);
        CHECK(all[i].pos.Y() == 0.0f);
    }

    // the raw `type` value rides along untouched
    CHECK(Str(all[0].type).empty());
    CHECK(Str(all[2].type) == "NameVillage");
    CHECK(Str(all[3].type) == "RockArea");

    // `settlement` agrees with the shared classifier, entry for entry
    for (int i = 0; i < all.Size(); i++)
    {
        RString name;
        Vector3 pos;
        const bool town = ZoneRegistry::NamesEntryIsTown(cfg.Names()->GetEntry(i), name, pos);
        CHECK(all[i].settlement == town);
    }
    CHECK(all[0].settlement);  // type-less OFP entry
    CHECK(all[1].settlement);  // 3-element position, still a town
    CHECK(all[2].settlement);  // NameVillage
    CHECK(!all[3].settlement); // RockArea: a feature, not a settlement
    CHECK(all[4].settlement);  // no name key, still a town
    CHECK(!all[5].settlement); // position[] too short to place
    CHECK(all[6].settlement);
}

TEST_CASE("WorldNames - a typed feature and a short position still carry a name", "[game][guerrilla][worldnames]")
{
    Parsed cfg(kNames);
    AutoArray<PlaceName> all;
    CollectPlaceNames(cfg.Names(), all);
    REQUIRE(all.Size() == 7);

    // The classifier rejects both of these BEFORE it writes its out-params, so
    // taking the name and the position from it would hand the history generator
    // an empty string and a zero vector for exactly the rows FeatureNames is
    // made of.  CollectPlaceNames owns the parse instead.
    const PlaceName& rock = all[3];
    CHECK(Str(rock.name) == "The Cliffs");
    CHECK(rock.pos.X() == 2000.0f);
    CHECK(rock.pos.Z() == 3000.0f);
    CHECK(!rock.settlement);

    const PlaceName& shortPos = all[5];
    CHECK(Str(shortPos.name) == "Nowhere");
    CHECK(!shortPos.settlement);
    // nothing to place it with: the defined answer is the origin, not a read
    // past the end of the array
    CHECK(shortPos.pos.X() == 0.0f);
    CHECK(shortPos.pos.Y() == 0.0f);
    CHECK(shortPos.pos.Z() == 0.0f);
}

TEST_CASE("WorldNames - settlements and features partition, dropping repeated display names",
          "[game][guerrilla][worldnames]")
{
    Parsed cfg(kNames);
    AutoArray<PlaceName> all;
    CollectPlaceNames(cfg.Names(), all);

    AutoArray<PlaceName> towns;
    AutoArray<PlaceName> features;
    SettlementNames(all, towns);
    FeatureNames(all, features);

    // Houdan, Larche, Chapoi, Bourdeaux - HoudanToo repeats a display name
    REQUIRE(towns.Size() == 4);
    CHECK(Str(towns[0].name) == "Houdan");
    CHECK(Str(towns[1].name) == "Larche");
    CHECK(Str(towns[2].name) == "Chapoi");
    CHECK(Str(towns[3].name) == "Bourdeaux");

    REQUIRE(features.Size() == 2);
    CHECK(Str(features[0].name) == "The Cliffs");
    CHECK(Str(features[1].name) == "Nowhere");

    // no display name lands in both halves
    for (int i = 0; i < towns.Size(); i++)
    {
        for (int j = 0; j < features.Size(); j++)
        {
            CHECK(stricmp(towns[i].name, features[j].name) != 0);
        }
    }
    CHECK(towns.Size() + features.Size() == all.Size() - 1); // the one duplicate
}

TEST_CASE("WorldNames - a missing Names block yields an empty list", "[game][guerrilla][worldnames]")
{
    AutoArray<PlaceName> out;
    PlaceName stale;
    stale.name = "left over";
    out.Add(stale);

    CollectPlaceNames(nullptr, out);
    CHECK(out.Size() == 0);

    AutoArray<PlaceName> towns;
    AutoArray<PlaceName> features;
    SettlementNames(out, towns);
    FeatureNames(out, features);
    CHECK(towns.Size() == 0);
    CHECK(features.Size() == 0);
}

TEST_CASE("WorldNames - the faction display name prefers an authored displayName", "[game][guerrilla][worldnames]")
{
    ParamFile file;
    QIStream in(kFactions, strlen(kFactions));
    file.Parse(in);
    ZoneRegistry registry;
    registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), file.FindEntry("CfgGuerrillaFactions"));

    // an authored displayName wins over the class name
    CHECK(Str(FactionDisplayName(registry, "East", "EAST")) == "Soviet Army");
    // and is never rewritten, underscores and all
    CHECK(Str(FactionDisplayName(registry, "Odd_One", "WEST")) == "Task_Force Alpha");
    // the class-name fallback reads as prose: the LoBo descriptors author no
    // displayName, so "PLO_East" would otherwise reach the page verbatim
    CHECK(Str(FactionDisplayName(registry, "PLO_East", "GUER")) == "PLO East");
    // an unknown class still falls back to its own spaced name
    CHECK(Str(FactionDisplayName(registry, "No_Such_Faction", "GUER")) == "No Such Faction");
    // no class at all: the side's descriptor, else the side string itself
    CHECK(Str(FactionDisplayName(registry, RString(), "EAST")) == "Soviet Army");
    CHECK(Str(FactionDisplayName(registry, RString(), "GUER")) == "GUER");
    CHECK(Str(FactionDisplayName(registry, RString(), RString())).empty());
    // runs of separators collapse rather than leaving a double space
    CHECK(Str(FactionDisplayName(registry, "A__B", "GUER")) == "A B");
    CHECK(Str(FactionDisplayName(registry, "_Lead", "GUER")) == "Lead");
}
