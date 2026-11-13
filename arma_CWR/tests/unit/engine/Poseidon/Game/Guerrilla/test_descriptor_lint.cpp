#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/BaseDetect.hpp>
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

// ---------------------------------------------------------------------------
// per-template base audit (issue #76, guerrilla lint --bases)
// ---------------------------------------------------------------------------

namespace
{

// a raw parse (no preprocessor: these configs carry no comments)
struct Parsed
{
    ParamFile file;
    explicit Parsed(const char* text)
    {
        QIStream in(text, static_cast<int>(strlen(text)));
        file.Parse(in);
    }
};

// flat 20 m ground with a lake around (4000, 4000)
struct LintTerrain final : HeightField
{
    float Height(float x, float z) const override
    {
        float dx = x - 4000.0f, dz = z - 4000.0f;
        return dx * dx + dz * dz < 300.0f * 300.0f ? -5.0f : 20.0f;
    }
};

LintTerrain gLintTerrain;

// one ILS airfield (apron at [3050, 1300]) and a pier + quay block on the lake
// shore, the same world the scaffold test uses
BaseDetectInputs LintWorld()
{
    BaseDetectInputs in;
    in.worldClass = "Fakeland";
    in.worldDisplay = "Fakeland";
    in.height = &gLintTerrain;
    in.worldSize = 5000.0f;
    IlsEntry ils;
    ils.x = 3000.0f;
    ils.z = 1200.0f;
    ils.dirX = 1.0f;
    ils.taxiIn.push_back(ScaffoldPoint{3050.0f, 1300.0f});
    ils.taxiOff.push_back(ScaffoldPoint{2200.0f, 1200.0f});
    ils.source = "CfgWorlds >> Fakeland";
    in.ils.push_back(ils);
    DetectObject pier;
    pier.x = 4000.0f;
    pier.z = 3690.0f;
    pier.model = "data3d\\molo_beton.p3d";
    in.objects.push_back(pier);
    DetectObject quay = pier;
    quay.x = 4030.0f;
    quay.model = "data3d\\molo_krychle.p3d";
    in.objects.push_back(quay);
    return in;
}

const BaseLintRow* RowOfType(const std::vector<BaseLintRow>& rows, const char* type)
{
    for (const BaseLintRow& r : rows)
    {
        if (r.type == type)
        {
            return &r;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("lint --bases: an authored airfield near the detected one covers it", "[game][guerrilla][lint]")
{
    Parsed cfg("class CfgGuerrillaZones\n"
               "{\n"
               "    class Zones\n"
               "    {\n"
               "        class Camp { name=\"Camp\"; type=\"CAMP\"; position[]={1000, 4000, 20}; };\n"
               "        class Air { name=\"Field\"; type=\"AIRFIELD\"; position[]={3000, 1350, 20}; };\n"
               "    };\n"
               "};\n");
    const ParamEntry* zonesCfg = cfg.file.FindEntry("CfgGuerrillaZones");
    REQUIRE(zonesCfg != nullptr);
    std::vector<DetectAuthoredZone> authored;
    ReadTemplateZones(zonesCfg, authored);
    REQUIRE(authored.size() == 2);
    REQUIRE(std::string((const char*)authored[1].name) == "Field");
    REQUIRE(authored[1].x == 3000.0f);
    REQUIRE(authored[1].z == 1350.0f);

    SeedBasesSetting seed = ResolveSeedBases(zonesCfg, authored);
    REQUIRE(!seed.explicitKey);
    REQUIRE(!seed.seed); // the template authors an AIRFIELD: default off
    REQUIRE(seed.authoredAirfields == 1);

    BaseDetectInputs in = LintWorld();
    in.authored = authored;
    BaseDetectResult result;
    DetectBases(in, BaseDetectOptions(), result);
    std::vector<BaseLintRow> rows = LintTemplateBases(result, authored, seed);
    REQUIRE(rows.size() == result.candidates.size());

    const BaseLintRow* air = RowOfType(rows, "AIRFIELD");
    REQUIRE(air != nullptr);
    INFO(air->line);
    REQUIRE(air->verdict == "COVERED");
    REQUIRE(air->coveredBy == "Field");
    REQUIRE(air->coveredByType == "AIRFIELD");
    REQUIRE(air->coverDist >= 0.0f);
    REQUIRE(air->coverDist < 300.0f);
    REQUIRE(air->line.rfind("COVERED AIRFIELD ", 0) == 0);
    REQUIRE(air->line.find("by \"Field\" (AIRFIELD, ") != std::string::npos);

    // the harbour is not authored and the seed is off: reported, not seeded
    const BaseLintRow* sea = RowOfType(rows, "SEAPORT");
    REQUIRE(sea != nullptr);
    INFO(sea->line);
    REQUIRE(sea->verdict == "MISSING");
    REQUIRE(!sea->seeded);
    REQUIRE(sea->line.find("NOT seeded (seedBases=0, default: the template authors 1 AIRFIELD / 0 SEAPORT)") !=
            std::string::npos);
    REQUIRE(FormatSeedBasesRow("Guerrilla.Fake", seed) ==
            "BASES Guerrilla.Fake seedBases=0 (default: the template authors 1 AIRFIELD / 0 SEAPORT)");
}

TEST_CASE("lint --bases: a template authoring no base gets both seeded", "[game][guerrilla][lint]")
{
    Parsed cfg("class CfgGuerrillaZones\n"
               "{\n"
               "    playRegion[] = {0, 0, 5000, 5000};\n"
               "    class Zones\n"
               "    {\n"
               "        class Camp { name=\"Camp\"; type=\"CAMP\"; position[]={1000, 4000, 20}; };\n"
               "    };\n"
               "};\n");
    const ParamEntry* zonesCfg = cfg.file.FindEntry("CfgGuerrillaZones");
    std::vector<DetectAuthoredZone> authored;
    ReadTemplateZones(zonesCfg, authored);
    SeedBasesSetting seed = ResolveSeedBases(zonesCfg, authored);
    REQUIRE(seed.seed);
    REQUIRE(!seed.explicitKey);

    BaseDetectInputs in = LintWorld();
    in.authored = authored;
    REQUIRE(ReadPlayRegion(zonesCfg, in.regionMinX, in.regionMinZ, in.regionMaxX, in.regionMaxZ));
    in.hasRegion = true;
    BaseDetectResult result;
    DetectBases(in, BaseDetectOptions(), result);
    std::vector<BaseLintRow> rows = LintTemplateBases(result, authored, seed);
    REQUIRE(rows.size() == 2);
    for (const BaseLintRow& r : rows)
    {
        INFO(r.line);
        REQUIRE(r.verdict == "MISSING");
        REQUIRE(r.seeded);
        REQUIRE(r.line.find("the runtime seeds it (seedBases=1, default: the template authors no AIRFIELD or "
                            "SEAPORT)") != std::string::npos);
    }

    // an explicit seedBases = 0 opts out, whatever the template authors
    Parsed off("class CfgGuerrillaZones { seedBases = 0; class Zones {}; };\n");
    SeedBasesSetting offSeed = ResolveSeedBases(off.file.FindEntry("CfgGuerrillaZones"), {});
    REQUIRE(offSeed.explicitKey);
    REQUIRE(!offSeed.seed);
    std::vector<BaseLintRow> offRows = LintTemplateBases(result, authored, offSeed);
    REQUIRE(!offRows[0].seeded);
    REQUIRE(offRows[0].line.find("NOT seeded (seedBases=0, set by the template)") != std::string::npos);

    // an empty or short playRegion[] is no region
    Parsed bad("class CfgGuerrillaZones { playRegion[] = {100, 100, 50, 900}; };\n");
    float a = 0, b = 0, c = 0, d = 0;
    REQUIRE(!ReadPlayRegion(bad.file.FindEntry("CfgGuerrillaZones"), a, b, c, d));
    REQUIRE(!ReadPlayRegion(nullptr, a, b, c, d));

    // no valid key: the default theatre is the authored zones' box + 5 km
    std::vector<DetectAuthoredZone> two(2);
    two[0].x = 1000;
    two[0].z = 2000;
    two[1].x = 3000;
    two[1].z = 1500;
    bool isDefault = false;
    REQUIRE(ResolvePlayRegion(bad.file.FindEntry("CfgGuerrillaZones"), two, a, b, c, d, &isDefault));
    REQUIRE(isDefault);
    REQUIRE(a == -4000.0f);
    REQUIRE(b == -3500.0f);
    REQUIRE(c == 8000.0f);
    REQUIRE(d == 7000.0f);
    // an authored key wins
    REQUIRE(ResolvePlayRegion(zonesCfg, two, a, b, c, d, &isDefault));
    REQUIRE(!isDefault);
    REQUIRE(c == 5000.0f);
    REQUIRE(!ResolvePlayRegion(nullptr, {}, a, b, c, d));
}

TEST_CASE("lint --bases: outposts are never seeded and skipped candidates say why", "[game][guerrilla][lint]")
{
    BaseDetectResult result;
    BaseCandidate post;
    post.type = "OUTPOST";
    post.name = "Ridge Post";
    post.status = BaseStatus::Offline;
    post.confidence = 0.8f;
    BaseCandidate far;
    far.type = "SEAPORT";
    far.name = "Far Harbour";
    far.status = BaseStatus::OutsideRegion;
    far.confidence = 0.9f;
    result.candidates = {post, far};
    SeedBasesSetting seed;
    seed.seed = true;
    std::vector<BaseLintRow> rows = LintTemplateBases(result, {}, seed);
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0].verdict == "MISSING");
    REQUIRE(!rows[0].seeded);
    REQUIRE(rows[0].line.find("never seeded (the outpost ladder is the author's)") != std::string::npos);
    REQUIRE(rows[1].verdict == "SKIPPED");
    REQUIRE(!rows[1].seeded);
    REQUIRE(rows[1].line.find("(outside-region)") != std::string::npos);
}
