#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/RosterProbe.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <string.h>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

// A minimal CfgVehicles with the stock base chain, one good body, and one body
// per failure mode.
const char* kRoster = "class CfgVehicles\n"
                      "{\n"
                      "  class All {};\n"
                      "  class AllVehicles : All {};\n"
                      "  class Land : AllVehicles {};\n"
                      "  class Man : Land {};\n"
                      "  class LandVehicle : Land {};\n"
                      "  class SoldierWB : Man { scope = 2; model = \"\\mini\\man.p3d\"; };\n"
                      "  class GoodBody : SoldierWB\n"
                      "  {\n"
                      "    scope = 2;\n"
                      "    model = \"\\mini\\man.p3d\";\n"
                      "    class EventHandlers\n"
                      "    {\n"
                      "      init = \"[_this select 0] exec \"\"\\mini\\scripts\\init.sqs\"\"; x = []\";\n"
                      "    };\n"
                      "  };\n"
                      "  class NoModelBody : SoldierWB { scope = 2; model = \"\"; };\n"
                      "  class BadModelBody : SoldierWB { scope = 2; model = \"\\nowhere\\missing.p3d\"; };\n"
                      "  class BadScriptBody : SoldierWB\n"
                      "  {\n"
                      "    scope = 2;\n"
                      "    model = \"\\mini\\man.p3d\";\n"
                      "    class EventHandlers { killed = \"_this exec \"\"\\nowhere\\boom.sqs\"\"\"; };\n"
                      "  };\n"
                      "  class HiddenBody : SoldierWB { scope = 1; model = \"\\mini\\man.p3d\"; };\n"
                      "  class GoodCar : LandVehicle { scope = 2; model = \"\\mini\\car.p3d\"; };\n"
                      "  class NotAVehicle { scope = 2; model = \"\\mini\\man.p3d\"; };\n"
                      "};\n";

// The package ships exactly these; everything else fails the gate.
bool FakeFileExists(RString path)
{
    static const char* const kShipped[] = {"mini\\man.p3d", "mini\\car.p3d", "mini\\scripts\\init.sqs"};
    for (const char* p : kShipped)
    {
        if (stricmp(path, p) == 0)
        {
            return true;
        }
    }
    return false;
}

const ParamEntry* Vehicles(ParamFile& pf)
{
    QIStream in(kRoster, static_cast<int>(strlen(kRoster)));
    pf.Parse(in);
    return pf.FindEntry("CfgVehicles");
}

const RosterProbeResult* Find(const std::vector<RosterProbeResult>& results, const char* name)
{
    for (const RosterProbeResult& r : results)
    {
        if (stricmp(r.className, name) == 0)
        {
            return &r;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("RosterProbe: script references are read out of EventHandlers", "[guerrilla][rosterprobe]")
{
    ParamFile pf;
    const ParamEntry* vehicles = Vehicles(pf);
    REQUIRE(vehicles != nullptr);

    std::vector<RString> refs = CollectScriptReferences(*vehicles->FindEntry("GoodBody"));
    REQUIRE(refs.size() == 1);
    REQUIRE(std::string(refs[0].Data()) == "\\mini\\scripts\\init.sqs");

    // No EventHandlers block at all is not an error, just no references.
    REQUIRE(CollectScriptReferences(*vehicles->FindEntry("NoModelBody")).empty());
}

TEST_CASE("RosterProbe: only createable Man/Land classes are candidates", "[guerrilla][rosterprobe]")
{
    ParamFile pf;
    const ParamEntry* vehicles = Vehicles(pf);

    REQUIRE(IsProbeCandidate(vehicles, *vehicles->FindEntry("GoodBody")));
    REQUIRE(IsProbeCandidate(vehicles, *vehicles->FindEntry("GoodCar")));
    // scope 1 is script-only, not a shipped body
    REQUIRE_FALSE(IsProbeCandidate(vehicles, *vehicles->FindEntry("HiddenBody")));
    // outside the Man/Land chain
    REQUIRE_FALSE(IsProbeCandidate(vehicles, *vehicles->FindEntry("NotAVehicle")));
    // the abstract bases themselves carry no scope
    REQUIRE_FALSE(IsProbeCandidate(vehicles, *vehicles->FindEntry("Man")));
}

TEST_CASE("RosterProbe: the static gate names each failure mode", "[guerrilla][rosterprobe]")
{
    ParamFile pf;
    const ParamEntry* vehicles = Vehicles(pf);

    RosterProbeResult good = ProbeRosterClass(*vehicles->FindEntry("GoodBody"), FakeFileExists);
    REQUIRE(good.ok);
    REQUIRE(good.reason.GetLength() == 0);

    RosterProbeResult noModel = ProbeRosterClass(*vehicles->FindEntry("NoModelBody"), FakeFileExists);
    REQUIRE_FALSE(noModel.ok);
    REQUIRE(std::string(noModel.reason.Data()).find("no model") != std::string::npos);

    RosterProbeResult badModel = ProbeRosterClass(*vehicles->FindEntry("BadModelBody"), FakeFileExists);
    REQUIRE_FALSE(badModel.ok);
    REQUIRE(std::string(badModel.reason.Data()).find("missing.p3d") != std::string::npos);

    RosterProbeResult badScript = ProbeRosterClass(*vehicles->FindEntry("BadScriptBody"), FakeFileExists);
    REQUIRE_FALSE(badScript.ok);
    REQUIRE(std::string(badScript.reason.Data()).find("boom.sqs") != std::string::npos);
}

TEST_CASE("RosterProbe: the class-name selection picks up ownerless mod classes", "[guerrilla][rosterprobe]")
{
    ParamFile pf;
    const ParamEntry* vehicles = Vehicles(pf);

    // Nothing in this hand-built config carries an owner, which is exactly the
    // shape of a mod folder's bin/config.cpp: an owner-only selection sees none
    // of it.
    RosterProbeOptions ownerOnly;
    ownerOnly.owners.push_back("SomeAddon");
    REQUIRE(ProbeRoster(vehicles, ownerOnly, FakeFileExists).empty());

    RosterProbeOptions byName;
    byName.classNames.push_back("GoodBody");
    byName.classNames.push_back("BadModelBody");
    byName.classNames.push_back("HiddenBody"); // not a candidate: dropped anyway
    std::vector<RosterProbeResult> results = ProbeRoster(vehicles, byName, FakeFileExists);
    REQUIRE(results.size() == 2);
    REQUIRE(Find(results, "GoodBody")->ok);
    REQUIRE_FALSE(Find(results, "BadModelBody")->ok);

    RosterProbeOptions filtered;
    filtered.classNames = byName.classNames;
    filtered.filter = "Good*";
    REQUIRE(ProbeRoster(vehicles, filtered, FakeFileExists).size() == 1);
}

TEST_CASE("RosterProbe: wildcard matching is case-insensitive", "[guerrilla][rosterprobe]")
{
    REQUIRE(RosterWildcardMatch("*", "LoBo_Terror_01R"));
    REQUIRE(RosterWildcardMatch("lobo_*", "LoBo_Terror_01R"));
    REQUIRE(RosterWildcardMatch("LoBo_Terror_??R", "LoBo_Terror_01R"));
    REQUIRE_FALSE(RosterWildcardMatch("LoBo_Terror_*E", "LoBo_Terror_01R"));
}
