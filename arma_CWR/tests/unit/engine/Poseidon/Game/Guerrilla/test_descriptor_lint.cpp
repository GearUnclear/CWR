#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/DescriptorLint.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <string.h>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

// A probe backed by a fixed roster, so the resolution pass runs without a
// mounted package.
struct FakeProbe final : ClassProbe
{
    std::vector<std::string> known;

    bool Exists(const char* bank, const char* className) const override
    {
        if (!bank || !className)
        {
            return false;
        }
        std::string key = std::string(bank) + "/" + className;
        for (const std::string& k : known)
        {
            if (stricmp(k.c_str(), key.c_str()) == 0)
            {
                return true;
            }
        }
        return false;
    }
};

const LintFinding* Find(const std::vector<LintFinding>& findings, const char* key)
{
    for (const LintFinding& f : findings)
    {
        if (stricmp(f.key, key) == 0)
        {
            return &f;
        }
    }
    return nullptr;
}

// Load the same descriptor twice - authored, then resolved - the way the lint
// command does.
void LoadPair(const char* config, const ClassProbe& probe, ZoneRegistry& raw, ZoneRegistry& resolved)
{
    static ParamFile pf; // outlives both loads; entries are read through it
    pf.Clear();
    QIStream in(config, static_cast<int>(strlen(config)));
    pf.Parse(in);
    const ParamEntry* factions = pf.FindEntry("CfgGuerrillaFactions");
    REQUIRE(factions != nullptr);
    raw.LoadFromParams(nullptr, factions, nullptr, nullptr, nullptr, nullptr, nullptr);
    resolved.LoadFromParams(nullptr, factions, nullptr, nullptr, nullptr, &probe, nullptr);
}

} // namespace

TEST_CASE("DescriptorLint: an intact descriptor reports every key ok", "[guerrilla][lint]")
{
    const char* config = "class CfgGuerrillaFactions\n"
                         "{\n"
                         "  class Good\n"
                         "  {\n"
                         "    side=\"EAST\";\n"
                         "    tiers[]={\"UnitA\",\"UnitB\"};\n"
                         "    tierThresholds[]={3};\n"
                         "    officer=\"UnitA\";\n"
                         "    vehicles[]={\"CarA\"};\n"
                         "  };\n"
                         "};\n";
    FakeProbe probe;
    probe.known = {"CfgVehicles/UnitA", "CfgVehicles/UnitB", "CfgVehicles/CarA"};

    ZoneRegistry raw;
    ZoneRegistry resolved;
    LoadPair(config, probe, raw, resolved);

    std::vector<LintFinding> findings = DiffFactionRecord(*raw.GetFaction(0), *resolved.GetFaction(0));
    REQUIRE(!findings.empty());
    for (const LintFinding& f : findings)
    {
        INFO("key " << (const char*)f.key);
        REQUIRE(f.outcome == LintOutcome::Ok);
    }
    REQUIRE_FALSE(FactionIsSterile(*resolved.GetFaction(0)));
}

TEST_CASE("DescriptorLint: a substituted tier and a dropped vehicle are named", "[guerrilla][lint]")
{
    const char* config = "class CfgGuerrillaFactions\n"
                         "{\n"
                         "  class Patchy\n"
                         "  {\n"
                         "    side=\"EAST\";\n"
                         "    tiers[]={\"UnitA\",\"MissingUnit\"};\n"
                         "    tierThresholds[]={3};\n"
                         "    officer=\"MissingOfficer\";\n"
                         "    vehicles[]={\"CarA\",\"MissingCar\",\"CarB\"};\n"
                         "    vehicleThresholds[]={3,6};\n"
                         "  };\n"
                         "};\n";
    FakeProbe probe;
    probe.known = {"CfgVehicles/UnitA", "CfgVehicles/CarA", "CfgVehicles/CarB"};

    ZoneRegistry raw;
    ZoneRegistry resolved;
    LoadPair(config, probe, raw, resolved);

    std::vector<LintFinding> findings = DiffFactionRecord(*raw.GetFaction(0), *resolved.GetFaction(0));

    const LintFinding* tier0 = Find(findings, "tiers[0]");
    REQUIRE(tier0 != nullptr);
    REQUIRE(tier0->outcome == LintOutcome::Ok);

    const LintFinding* tier1 = Find(findings, "tiers[1]");
    REQUIRE(tier1 != nullptr);
    REQUIRE(tier1->outcome == LintOutcome::Substituted);
    REQUIRE(std::string(tier1->value.Data()) == "MissingUnit");
    REQUIRE(std::string(tier1->substitute.Data()) == "UnitA");

    // The officer falls back to the resolved tiers[0].
    const LintFinding* officer = Find(findings, "officer");
    REQUIRE(officer != nullptr);
    REQUIRE(officer->outcome == LintOutcome::Substituted);
    REQUIRE(std::string(officer->substitute.Data()) == "UnitA");

    // vehicles[] compacts, so the middle rung is a drop and the survivors keep
    // their authored indices in the report.
    REQUIRE(Find(findings, "vehicles[0]")->outcome == LintOutcome::Ok);
    REQUIRE(Find(findings, "vehicles[1]")->outcome == LintOutcome::Dropped);
    REQUIRE(std::string(Find(findings, "vehicles[1]")->value.Data()) == "MissingCar");
    REQUIRE(Find(findings, "vehicles[2]")->outcome == LintOutcome::Ok);
    REQUIRE(std::string(Find(findings, "vehicles[2]")->value.Data()) == "CarB");
}

TEST_CASE("DescriptorLint: a faction nothing resolves for is sterile", "[guerrilla][lint]")
{
    const char* config = "class CfgGuerrillaFactions\n"
                         "{\n"
                         "  class Sterile\n"
                         "  {\n"
                         "    side=\"NOSUCHSIDE\";\n" // no built-in fallback list for an unknown side
                         "    tiers[]={\"NoSuchClass\"};\n"
                         "  };\n"
                         "};\n";
    FakeProbe probe; // knows nothing

    ZoneRegistry raw;
    ZoneRegistry resolved;
    LoadPair(config, probe, raw, resolved);

    REQUIRE(FactionIsSterile(*resolved.GetFaction(0)));

    std::vector<LintFinding> findings = DiffFactionRecord(*raw.GetFaction(0), *resolved.GetFaction(0));
    const LintFinding* tier0 = Find(findings, "tiers[0]");
    REQUIRE(tier0 != nullptr);
    REQUIRE(tier0->outcome == LintOutcome::Dropped);
}

TEST_CASE("DescriptorLint: RecordingClassProbe answers through and lists the misses", "[guerrilla][lint]")
{
    FakeProbe inner;
    inner.known = {"CfgVehicles/UnitA"};
    RecordingClassProbe probe(inner);

    REQUIRE(probe.Exists("CfgVehicles", "UnitA"));
    REQUIRE_FALSE(probe.Exists("CfgVehicles", "UnitB"));
    REQUIRE_FALSE(probe.Exists("CfgVehicles", "UnitB")); // repeat: one entry in Misses
    REQUIRE_FALSE(probe.Exists("CfgWeapons", "GunA"));

    REQUIRE(probe.Queries().size() == 4);
    std::vector<RString> misses = probe.Misses();
    REQUIRE(misses.size() == 2);
    REQUIRE(std::string(misses[0].Data()) == "CfgVehicles/UnitB");
    REQUIRE(std::string(misses[1].Data()) == "CfgWeapons/GunA");
}
