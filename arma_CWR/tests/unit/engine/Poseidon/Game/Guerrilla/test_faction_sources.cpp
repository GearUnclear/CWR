// FactionSources (issue #54 A1): the union-merge of the global
// CfgGuerrillaFactions (Pars) with the island template's own block, island
// winning on a class-name collision. The three engine consumers all build
// their table through this helper, so these tests pin the semantics once.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/FactionSources.hpp>
#include <Poseidon/Game/Guerrilla/OutfitSelect.hpp>
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

struct ParsedConfig
{
    ParamFile file;
    explicit ParsedConfig(const char* text)
    {
        QIStream in(text, strlen(text));
        file.Parse(in);
    }
    const ParamEntry* Factions() const { return file.FindEntry("CfgGuerrillaFactions"); }
};

std::vector<std::string> ClassNames(const ParamEntry* table)
{
    std::vector<std::string> out;
    if (!table)
    {
        return out;
    }
    for (int i = 0; i < table->GetEntryCount(); i++)
    {
        const ParamEntry& e = table->GetEntry(i);
        if (e.IsClass())
        {
            out.push_back(Str(RString(e.GetName())));
        }
    }
    return out;
}

// what an addon-shipped faction pack looks like once merged into Pars: a
// vanilla pair plus a mod roster, one of them via inheritance
const char* kGlobal = "class CfgGuerrillaFactions\n"
                      "{\n"
                      "    class WEST { side=\"WEST\"; tiers[]={\"SoldierWB\",\"SoldierWG\"}; officer=\"OfficerW\"; "
                      "vehicleThreshold=3; captureRate=1.5; };\n"
                      "    class EAST { side=\"EAST\"; tiers[]={\"SoldierEB\"}; };\n"
                      "    class LoBoBase { tiers[]={\"LoBoGolaniWB\"}; officer=\"LoBoOfficer\"; };\n"
                      "    class IDF : LoBoBase { side=\"WEST\"; flag=\"\\\\lobo\\\\idf.paa\"; };\n"
                      "    class CIV { side=\"CIV\"; civClassCount=1; civClass1=\"Civilian\"; };\n"
                      "};\n";

// an island template: its own GUER roster, a CIV block of its own, and a
// redeclared WEST that deliberately drops the global's officer key
const char* kIsland = "class CfgGuerrillaFactions\n"
                      "{\n"
                      "    class GUER { side=\"GUER\"; tiers[]={\"SoldierGB\"}; playerClassWarrior=\"SoldierGB\"; };\n"
                      "    class WEST { side=\"WEST\"; tiers[]={\"SoldierWCrew\"}; };\n"
                      "    class CIV { side=\"CIV\"; civClassCount=2; civClass1=\"Civilian\"; civClass2=\"Civilian2\"; "
                      "};\n"
                      "};\n";

} // namespace

TEST_CASE("FactionSources: neither source present yields no table", "[guerrilla][factions]")
{
    FactionSources s;
    s.Build(nullptr, nullptr);
    REQUIRE(s.Factions() == nullptr);
    REQUIRE(s.Records().empty());
}

TEST_CASE("FactionSources: a lone global block is copied verbatim, inheritance flattened", "[guerrilla][factions]")
{
    ParsedConfig global(kGlobal);
    FactionSources s;
    s.Build(global.Factions(), nullptr);
    REQUIRE(s.Factions() != nullptr);
    REQUIRE(ClassNames(s.Factions()) == std::vector<std::string>{"WEST", "EAST", "LoBoBase", "IDF", "CIV"});

    const ParamEntry* west = s.Factions()->FindEntry("WEST");
    REQUIRE(west != nullptr);
    REQUIRE(Str(west->ReadValue("side", RString())) == "WEST");
    REQUIRE(Str(west->ReadValue("officer", RString())) == "OfficerW");
    REQUIRE(west->ReadValue("vehicleThreshold", 0) == 3);
    REQUIRE(west->ReadValue("captureRate", 0.0f) == 1.5f);
    const ParamEntry* tiers = west->FindEntry("tiers");
    REQUIRE(tiers != nullptr);
    REQUIRE(tiers->IsArray());
    REQUIRE(tiers->GetSize() == 2);
    REQUIRE(Str(RString((RStringB)(*tiers)[1])) == "SoldierWG");

    // IDF : LoBoBase - the copy carries the inherited keys as its OWN
    // entries, so a GetEntry(i) walk (LoadFactions) sees them
    const ParamEntry* idf = s.Factions()->FindEntry("IDF");
    REQUIRE(idf != nullptr);
    REQUIRE(Str(idf->ReadValue("officer", RString())) == "LoBoOfficer");
    REQUIRE(Str(idf->ReadValue("side", RString())) == "WEST");
    bool ownOfficer = false;
    for (int i = 0; i < idf->GetEntryCount(); i++)
    {
        ownOfficer |= stricmp(idf->GetEntry(i).GetName(), "officer") == 0;
    }
    REQUIRE(ownOfficer);
    const ParamEntry* idfTiers = idf->FindEntry("tiers");
    REQUIRE(idfTiers != nullptr);
    REQUIRE(Str(RString((RStringB)(*idfTiers)[0])) == "LoBoGolaniWB");

    for (const FactionSources::Record& r : s.Records())
    {
        REQUIRE(r.origin == FactionSources::Origin::Global);
        REQUIRE_FALSE(r.overrodeGlobal);
    }
}

TEST_CASE("FactionSources: a lone island block is copied verbatim", "[guerrilla][factions]")
{
    ParsedConfig island(kIsland);
    FactionSources s;
    s.Build(nullptr, island.Factions());
    REQUIRE(ClassNames(s.Factions()) == std::vector<std::string>{"GUER", "WEST", "CIV"});
    REQUIRE(s.FindRecord("GUER") != nullptr);
    REQUIRE(s.FindRecord("GUER")->origin == FactionSources::Origin::Island);
    REQUIRE(s.FindRecord("guer") != nullptr); // case-insensitive, like FindFaction
    REQUIRE(s.FindRecord("IDF") == nullptr);
}

TEST_CASE("FactionSources: union - island first, then global-only classes; island wins on collision",
          "[guerrilla][factions]")
{
    ParsedConfig global(kGlobal);
    ParsedConfig island(kIsland);
    FactionSources s;
    s.Build(global.Factions(), island.Factions());

    REQUIRE(ClassNames(s.Factions()) == std::vector<std::string>{"GUER", "WEST", "CIV", "EAST", "LoBoBase", "IDF"});

    // WEST: the island's whole class replaced the global one - no per-key
    // merge, so the global's officer key is GONE
    const ParamEntry* west = s.Factions()->FindEntry("WEST");
    REQUIRE(west != nullptr);
    REQUIRE(west->FindEntry("officer") == nullptr);
    REQUIRE(Str(RString((RStringB)(*west->FindEntry("tiers"))[0])) == "SoldierWCrew");
    REQUIRE(s.FindRecord("WEST")->origin == FactionSources::Origin::Island);
    REQUIRE(s.FindRecord("WEST")->overrodeGlobal);

    // CIV stays island-owned even when a (misbehaving) global block ships one
    const ParamEntry* civ = s.Factions()->FindEntry("CIV");
    REQUIRE(civ != nullptr);
    REQUIRE(civ->ReadValue("civClassCount", 0) == 2);

    // global-only classes come through untouched
    REQUIRE(s.FindRecord("EAST")->origin == FactionSources::Origin::Global);
    REQUIRE_FALSE(s.FindRecord("EAST")->overrodeGlobal);
    REQUIRE(s.FindRecord("GUER")->origin == FactionSources::Origin::Island);
    REQUIRE_FALSE(s.FindRecord("GUER")->overrodeGlobal);
}

TEST_CASE("FactionSources: the merged table outlives its sources and survives a rebuild", "[guerrilla][factions]")
{
    FactionSources s;
    {
        ParsedConfig global(kGlobal);
        ParsedConfig island(kIsland);
        s.Build(global.Factions(), island.Factions());
    } // both sources destroyed
    REQUIRE(s.Factions() != nullptr);
    const ParamEntry* idf = s.Factions()->FindEntry("IDF");
    REQUIRE(idf != nullptr);
    REQUIRE(Str(RString((RStringB)(*idf->FindEntry("tiers"))[0])) == "LoBoGolaniWB");
    // config strings keep their backslashes verbatim (no escape processing)
    REQUIRE(Str(idf->ReadValue("flag", RString())) == "\\\\lobo\\\\idf.paa");

    // a rebuild with different sources replaces the table wholesale
    ParsedConfig island(kIsland);
    s.Build(nullptr, island.Factions());
    REQUIRE(s.Factions()->FindEntry("IDF") == nullptr);
    REQUIRE(s.Factions()->FindEntry("GUER") != nullptr);
    s.Clear();
    REQUIRE(s.Factions() == nullptr);
}

TEST_CASE("FactionSources: the merged table feeds the registry and the outfit seam unchanged", "[guerrilla][factions]")
{
    // the issue-#26 gap, end to end at the config level: a global (addon)
    // IDF on an island that never authored it is now a selectable roster
    ParsedConfig global(kGlobal);
    ParsedConfig island(
        "class CfgGuerrillaZones\n"
        "{\n"
        "    defaultOccupier = \"EAST\";\n"
        "    defaultResistance = \"GUER\";\n"
        "    playerSide = \"GUER\";\n"
        "    class Zones { class Camp { name=\"Camp\"; type=\"CAMP\"; owner=\"RESISTANCE\"; "
        "position[]={100.0, 100.0, 0.0}; }; };\n"
        "};\n"
        "class CfgGuerrillaFactions\n"
        "{\n"
        "    class GUER { side=\"GUER\"; tiers[]={\"SoldierGB\"}; playerClassCiv=\"SoldierGFakeC\"; };\n"
        "    class CIV { side=\"CIV\"; };\n"
        "};\n");
    FactionSources s;
    s.Build(global.Factions(), island.Factions());

    ZoneRegistry reg;
    reg.LoadFromParams(island.file.FindEntry("CfgGuerrillaZones"), s.Factions(), "IDF", "GUER");
    REQUIRE(Str(reg.OccupierSide()) == "WEST");
    REQUIRE(Str(reg.OccupierFaction()) == "IDF");
    REQUIRE(Str(reg.ResistanceSide()) == "GUER");
    REQUIRE(Str(reg.FactionTierClass("IDF", 1.0f)) == "LoBoGolaniWB");
    REQUIRE(Str(reg.FactionValue("IDF", "officer")) == "LoBoOfficer");

    // the outfit seam resolves the island's GUER block through the same table
    const ParamEntry* found = FindGuerrillaFactionEntry(s.Factions(), "GUER");
    REQUIRE(found != nullptr);
    REQUIRE(Str(found->ReadValue("playerClassCiv", RString())) == "SoldierGFakeC");
}
