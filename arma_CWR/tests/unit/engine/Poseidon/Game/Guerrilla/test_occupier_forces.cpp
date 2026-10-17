// Occupier forces pool (OCCUPIER-FORCES.md phase 2, OccupierForces.hpp): the
// key and the inert service, the roster / K / G sizing with the exemption
// and the settlement probe, poolStart, the refill formula (the 4 h figure
// from full exhaustion with D2 drawing first, the 1.6 h best case, the
// standing factor), banded hysteresis (no SPENT / THIN flapping, one SPENT
// exit during recovery), Draw and the floor, control, the war-level split,
// the conservation ledger, the save round trip and the old-save derivation.
// Phase 3, the director: invariant W, the heat candidates and their roll,
// urgency with hysteresis, tokens and the born-operation slots, one launch
// per tick, refusals refunded, repel and cancel cooldowns (+15 heat unless a
// RED edge paid it), hull refunds, push-out only when STRONG, anti-1:1
// pacing, intel (D9: the choice ignores the true census; a dead group's last
// report is kept, then decays), the won freeze, and the director state
// across a save.
// Pure: the registry is built from inline config, the service is driven
// through Activate / Tick with no world.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp>
#include <Poseidon/Game/Guerrilla/AlertMachine.hpp> // ASRed (the alarm answers, phase 6)
#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp> // IsGeneratedOccupier (5.7 conservation)
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>
#include <Poseidon/Game/Guerrilla/DirectorLog.hpp> // the director log toggle (forcesLog)
#include <Poseidon/Game/Guerrilla/ForcePackages.hpp>
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp> // the S5 refund hook (3.3)
#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/Platform/AppConfig.hpp> // DevMode (the director log toggle)
#include <Poseidon/Foundation/Framework/Log.hpp>      // LogDetail (the ceiling line capture)

#include <spdlog/sinks/callback_sink.h>
#include <initializer_list>
#include <memory>

#include <algorithm>
#include <filesystem>
#include <string.h>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

// A Camp, two occupier posts (garrison 6 and 12: R = 18), one resistance
// post (not roster), two administered towns (K = 2, G = 8 at the default
// cityGarrison 4), a headquarters town (exempted by the fixture) and a town
// the resistance holds from the start.
const char* kForcesConfig = "class CfgGuerrillaZones\n"
                            "{\n"
                            "    class Zones\n"
                            "    {\n"
                            "        class Camp     { name=\"Camp\";     type=\"CAMP\";     owner=\"RESISTANCE\"; "
                            "position[]={1000.0, 1000.0, 0.0}; };\n"
                            "        class Town     { name=\"Town\";     type=\"CITY\";     owner=\"NEUTRAL\"; "
                            "support=20; position[]={1000.0, 1300.0, 0.0}; };\n"
                            "        class Other    { name=\"Other\";    type=\"CITY\";     owner=\"NEUTRAL\"; "
                            "support=20; position[]={5000.0, 5000.0, 0.0}; };\n"
                            "        class HqTown   { name=\"HqTown\";   type=\"CITY\";     owner=\"NEUTRAL\"; "
                            "support=20; position[]={3000.0, 1000.0, 0.0}; };\n"
                            "        class Held     { name=\"Held\";     type=\"CITY\";     owner=\"RESISTANCE\"; "
                            "support=80; position[]={1000.0, 3000.0, 0.0}; };\n"
                            "        class Post     { name=\"Post\";     type=\"OUTPOST\";  owner=\"OCCUPIER\"; "
                            "garrison=6; position[]={1600.0, 1600.0, 0.0}; };\n"
                            "        class FarBase  { name=\"FarBase\";  type=\"AIRFIELD\"; owner=\"OCCUPIER\"; "
                            "garrison=12; position[]={9000.0, 9000.0, 0.0}; };\n"
                            "        class RebelPost{ name=\"RebelPost\"; type=\"OUTPOST\"; owner=\"RESISTANCE\"; "
                            "position[]={2000.0, 2000.0, 0.0}; };\n"
                            "    };\n"
                            "};\n";

std::string ConfigWith(const char* keys, const char* base = kForcesConfig)
{
    std::string cfg = base;
    const std::string anchor = "class CfgGuerrillaZones\n{\n";
    cfg.insert(anchor.size(), std::string("    ") + keys + "\n");
    return cfg;
}

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

struct Fixture
{
    std::string text;
    ParamFile file;
    ZoneRegistry registry;
    OccupierForces of;

    void Load(const std::string& config, bool exemptHq = true)
    {
        text = config;
        QIStream in(text.c_str(), text.size());
        file.Parse(in);
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), nullptr);
        of.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
        // the ceiling banks its excess into this registry's bases (5.5)
        of.BindRegistry(&registry);
        REQUIRE(registry.IsActive());
        const int hq = registry.FindZoneIndex("HqTown");
        if (exemptHq && hq >= 0)
        {
            registry.ExemptTownGarrison(hq);
        }
        // a zone tick saw the player far away (L10: before any tick a
        // reserve write would go pending)
        registry.NoteTickPlayer(true, 99999.0f, 99999.0f);
    }
    ZoneRecord& Z(const char* name)
    {
        const int i = registry.FindZoneIndex(name);
        REQUIRE(i >= 0);
        return *registry.GetZoneMutable(i);
    }
    void GiveToResistance(const char* name) { Z(name).owner = RString("GUER"); }
};

// a settlement probe that rejects the listed names
struct FakeSettlement final : SettlementProbe
{
    std::vector<std::string> labels;
    bool IsSettlement(const char* name, const Vector3&) const override
    {
        for (const std::string& l : labels)
        {
            if (stricmp(l.c_str(), name) == 0)
            {
                return false;
            }
        }
        return true;
    }
};

// An Abel-sized theatre: four occupier bases of 8 (R = 32) and fourteen
// administered towns of 4 (K = 14, G = 56), so poolStart = 32 + 2 * 14 = 60
// and the refill is (60 + 32 + 56) / 4 = 37 an hour.
std::string AbelLikeConfig()
{
    std::string cfg = "class CfgGuerrillaZones\n{\n    occupierForces = 1;\n    class Zones\n    {\n";
    cfg += "        class Camp { name=\"Camp\"; type=\"CAMP\"; owner=\"RESISTANCE\"; position[]={100.0, 100.0, 0.0}; "
           "};\n";
    const char* baseTypes[4] = {"AIRFIELD", "SEAPORT", "OUTPOST", "OUTPOST"};
    for (int i = 0; i < 4; i++)
    {
        cfg += "        class Base" + std::to_string(i) + " { name=\"Base" + std::to_string(i) + "\"; type=\"" +
               baseTypes[i] + "\"; owner=\"OCCUPIER\"; garrison=8; position[]={" + std::to_string(2000 + 1000 * i) +
               ".0, 8000.0, 0.0}; };\n";
    }
    for (int i = 0; i < 14; i++)
    {
        cfg += "        class T" + std::to_string(i) + " { name=\"T" + std::to_string(i) +
               "\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=20; position[]={" + std::to_string(1000 + 500 * i) +
               ".0, 3000.0, 0.0}; };\n";
    }
    cfg += "    };\n};\n";
    return cfg;
}

// The D2 regrowth stand-in: one body per 600 s per short zone, drawn from
// the pool.  A zone whose draw is refused keeps its clock clamped at the
// interval, so it takes the next unit the refill brings.
struct FakeD2
{
    static constexpr float Interval = 600.0f;
    std::vector<int> reserve;
    std::vector<int> maxv;
    std::vector<float> clock;

    void Add(int r, int m, float c)
    {
        reserve.push_back(r);
        maxv.push_back(m);
        clock.push_back(c);
    }
    int Reserves() const
    {
        int n = 0;
        for (int r : reserve)
        {
            n += r;
        }
        return n;
    }
    bool Full() const
    {
        for (size_t i = 0; i < reserve.size(); i++)
        {
            if (reserve[i] < maxv[i])
            {
                return false;
            }
        }
        return true;
    }
    void Step(OccupierForces& of, float elapsed)
    {
        for (size_t i = 0; i < reserve.size(); i++)
        {
            if (reserve[i] >= maxv[i])
            {
                clock[i] = 0;
                continue;
            }
            clock[i] += elapsed;
            while (clock[i] >= Interval && reserve[i] < maxv[i])
            {
                if (of.Draw(1) == 0)
                {
                    clock[i] = Interval; // nothing granted: clamped, retried next tick
                    break;
                }
                reserve[i]++;
                clock[i] -= Interval;
            }
        }
    }
};

} // namespace

// ---------------------------------------------------------------------------
// key, tuning
// ---------------------------------------------------------------------------

TEST_CASE("OccupierForces - defaults match the knob table and the key is off", "[game][guerrilla][occupierforces]")
{
    OccupierForcesTuning t;
    CHECK_FALSE(t.enabled);
    CHECK(t.poolMin == 24);
    CHECK(t.poolPerTown == 2);
    CHECK(t.refillHours == Approx(4.0f));
    CHECK(t.poolFloor == Approx(0.2f));
    CHECK(t.directorInterval == Approx(60.0f));
    CHECK(t.tokensMax == 2);
    // the playable-rate intervals (owner, 2026-10-15); the phase 3 ones stay
    // reachable by authoring LegacyTokenInterval
    CHECK(t.tokenInterval[OFBandStrong] == Approx(600.0f));
    CHECK(t.tokenInterval[OFBandStretched] == Approx(1050.0f));
    CHECK(t.tokenInterval[OFBandThin] == Approx(1800.0f));
    CHECK(t.tokenInterval[OFBandSpent] == Approx(0.0f));
    CHECK(OccupierForcesTuning::LegacyTokenInterval[OFBandStrong] == Approx(1800.0f));
    CHECK(OccupierForcesTuning::LegacyTokenInterval[OFBandStretched] == Approx(2700.0f));
    CHECK(OccupierForcesTuning::LegacyTokenInterval[OFBandThin] == Approx(3600.0f));
    CHECK(OccupierForcesTuning::LegacyTokenInterval[OFBandSpent] == Approx(0.0f));
    CHECK(t.controlMilWeight == Approx(0.6f));
    CHECK(t.nWaveThresholds == 2);
    CHECK(t.waveThresholds[0] == Approx(0.75f));
    CHECK(t.waveThresholds[1] == Approx(0.50f));
    CHECK(t.waveWarLevel == Approx(1.0f));
}

TEST_CASE("OccupierForces - config keys are read and repaired into range", "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1; forcesPoolMin = 500; forcesPoolPerTown = 3; forcesPoolRefillHours = 0.1; "
                      "forcesPoolFloor = 0.3; forcesDirectorInterval = 5; forcesTokensMax = 1; "
                      "forcesTokenInterval[] = {600, 900, 99999}; forcesControlMilWeight = 0.5; "
                      "forcesWaveThresholds[] = {0.8, 0.6, 0.4, 0.2}; forcesWaveWarLevel = 2;"));
    const OccupierForcesTuning& t = f.of.Tuning();
    CHECK(t.enabled);
    CHECK(t.poolMin == 200); // repaired
    CHECK(t.poolPerTown == 3);
    CHECK(t.refillHours == Approx(0.5f)); // config floor; gmForcesTune reaches lower
    CHECK(t.poolFloor == Approx(0.3f));
    CHECK(t.directorInterval == Approx(10.0f));
    CHECK(t.tokensMax == 1);
    CHECK(t.tokenInterval[0] == Approx(600.0f));
    CHECK(t.tokenInterval[1] == Approx(900.0f));
    CHECK(t.tokenInterval[2] == Approx(86400.0f)); // repaired
    CHECK(t.tokenInterval[3] == Approx(0.0f));     // short array: default kept
    CHECK(t.controlMilWeight == Approx(0.5f));
    CHECK(t.nWaveThresholds == 3); // at most three
    CHECK(t.waveThresholds[2] == Approx(0.4f));
    CHECK(t.waveWarLevel == Approx(2.0f));
}

TEST_CASE("OccupierForces - forcesLog: default off, 0..1, and the director log level it selects",
          "[game][guerrilla][occupierforces]")
{
    CHECK_FALSE(OccupierForcesTuning().log); // default 0: the per-action lines are DEBUG outside --dev
    {
        Fixture f;
        f.Load(ConfigWith("occupierForces = 1;"));
        CHECK_FALSE(f.of.Tuning().log); // not authored
    }
    {
        Fixture f;
        f.Load(ConfigWith("occupierForces = 1; forcesLog = 1;"));
        CHECK(f.of.Tuning().log);
    }
    {
        Fixture f;
        f.Load(ConfigWith("occupierForces = 1; forcesLog = 7;"));
        CHECK(f.of.Tuning().log); // repaired to 1
    }
    {
        Fixture f;
        f.Load(ConfigWith("occupierForces = 1; forcesLog = -3;"));
        CHECK_FALSE(f.of.Tuning().log); // repaired to 0
    }
    {
        Fixture f;
        f.Load(ConfigWith("occupierForces = 0; forcesLog = 1;"));
        CHECK(f.of.Tuning().log); // read with the service off too: CounterAttack logs through it
    }

    // the level comes from the singleton's tuning (or --dev)
    OccupierForcesTuning& live = OccupierForces::Instance().TuningMutable();
    const bool saved = live.log;
    live.log = true;
    CHECK(DirectorLogLevel() == spdlog::level::info);
    live.log = false;
    if (!Poseidon::Foundation::AppConfig::Instance().DevMode())
    {
        CHECK(DirectorLogLevel() == spdlog::level::debug);
    }
    else
    {
        CHECK(DirectorLogLevel() == spdlog::level::info);
    }
    live.log = saved;
}

TEST_CASE("OccupierForces - key off: inert, the seams pass through, nothing to save",
          "[game][guerrilla][occupierforces]")
{
    Fixture on;
    on.Load(kForcesConfig);
    CHECK(on.of.Enabled()); // phase 7: the key defaults on when not authored
    Fixture f;
    f.Load(ConfigWith("occupierForces = 0;"));
    CHECK_FALSE(f.of.Enabled());
    CHECK_FALSE(f.of.Active());
    f.of.Tick(f.registry, 3600.0f);
    CHECK(f.of.Pool() == 0);
    CHECK(f.of.Draw(3) == 3);         // legacy D2: regrowth from nothing
    CHECK(f.of.Deposit(4, "t") == 4); // legacy revert refill
    CHECK(f.of.Spend(5, "op"));       // free, nothing debited
    f.of.Refund(5, "r");
    f.of.SetPool(10);
    CHECK(f.of.Pool() == 0);
    CHECK_FALSE(f.of.HasState());
    // an old save loaded with the key off derives nothing
    f.of.DeriveFromExistingSave(f.registry);
    CHECK_FALSE(f.of.HasState());
}

// ---------------------------------------------------------------------------
// sizing (5.1)
// ---------------------------------------------------------------------------

TEST_CASE("OccupierForces - roster, K with the exemption, G and poolStart", "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1; campaignEnd = 0;")); // the roster ignores campaignEnd
    OccupierForcesSizing s;
    OccupierForces::Size(f.registry, f.of.Tuning(), s);
    REQUIRE(s.roster.Size() == 2);
    CHECK(Str(s.roster[0]) == "Post");
    CHECK(Str(s.roster[1]) == "FarBase");
    CHECK(s.R == 18);
    REQUIRE(s.K() == 2);
    CHECK(Str(s.towns[0]) == "Town");
    CHECK(Str(s.towns[1]) == "Other");
    CHECK(s.G == 8);
    REQUIRE(s.excluded.Size() == 2);
    CHECK(Str(s.excluded[0]) == "HqTown"); // exempt
    CHECK(Str(s.excluded[1]) == "Held");   // not administered
    CHECK(s.poolStart == 24);              // max(24, 18 + 2 * 2 = 22)

    REQUIRE(f.of.Activate(f.registry));
    CHECK(f.of.Active());
    CHECK(f.of.PoolStart() == 24);
    CHECK(f.of.Pool() == 24);
    CHECK(f.of.RosterStrength() == 18);
    CHECK(f.of.TownCount() == 2);
    CHECK(f.of.TownGarrisons() == 8);
    CHECK(f.of.Band() == OFBandStrong);
    CHECK(f.of.Posture() == OFDormant);
    CHECK(f.of.Tokens() == 0);
    CHECK(f.of.Control() == Approx(1.0f));
    CHECK(f.of.HasState());
}

TEST_CASE("OccupierForces - poolStart above the minimum and the per-town weight", "[game][guerrilla][occupierforces]")
{
    OccupierForcesTuning t;
    CHECK(OccupierForces::PoolStartOf(t, 32, 14) == 60); // Abel
    CHECK(OccupierForces::PoolStartOf(t, 14, 37) == 88); // Sinai
    CHECK(OccupierForces::PoolStartOf(t, 14, 3) == 24);  // Lebanon80: the minimum
    t.poolMin = 0;
    CHECK(OccupierForces::PoolStartOf(t, 18, 2) == 22);
    t.poolPerTown = 0;
    CHECK(OccupierForces::PoolStartOf(t, 18, 2) == 18);
}

TEST_CASE("OccupierForces - a seeded town counts only when the settlement probe accepts it",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    AutoArray<bool> seeded;
    seeded.Resize(f.registry.NZones());
    for (int i = 0; i < seeded.Size(); i++)
    {
        seeded[i] = false;
    }
    seeded[f.registry.FindZoneIndex("Other")] = true;
    seeded[f.registry.FindZoneIndex("Town")] = true;
    FakeSettlement probe;
    probe.labels.push_back("Other");
    OccupierForcesSizing s;
    OccupierForces::Size(f.registry, f.of.Tuning(), s, &seeded, &probe);
    REQUIRE(s.K() == 1);
    CHECK(Str(s.towns[0]) == "Town"); // seeded, and a settlement
    CHECK(s.G == 4);
    REQUIRE(s.excluded.Size() == 3);
    CHECK(Str(s.excluded[0]) == "Other");
    CHECK(s.labels.Size() == 1);
    // the probe never touches an authored (unseeded) town
    seeded[f.registry.FindZoneIndex("Other")] = false;
    OccupierForces::Size(f.registry, f.of.Tuning(), s, &seeded, &probe);
    CHECK(s.K() == 2);
    CHECK(s.labels.Size() == 0);
}

TEST_CASE("OccupierForces - the engine path probes the seeded towns once and sizes K from the stored verdicts",
          "[game][guerrilla][occupierforces][save]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "occupier-forces-probe.bin";
    const std::string config = ConfigWith("occupierForces = 1;");
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    f.Load(config);
    // Town and Other stand in for seeded labels (seedCities = 1)
    f.Z("Town").seeded = true;
    f.Z("Other").seeded = true;
    FakeSettlement probe;
    probe.labels.push_back("Other");
    REQUIRE(f.of.ActivateProbed(f.registry, &probe));
    CHECK(f.Z("Town").settlement == 1);
    CHECK(f.Z("Other").settlement == -1);
    CHECK(f.Z("Held").settlement == 0); // authored: never probed
    CHECK(f.of.TownCount() == 1);       // K: Town only
    CHECK(f.of.TownGarrisons() == 4);
    CHECK(f.of.ProbedOutCount() == 1);
    CHECK(f.of.PoolStart() == 24); // max(24, 18 + 2 * 1)
    bool otherExcluded = false;
    for (int i = 0; i < f.of.ExcludedCount(); i++)
    {
        otherExcluded = otherExcluded || Str(f.of.ExcludedName(i)) == "Other";
    }
    CHECK(otherExcluded);

    // the verdict is stored: a later sizing with a probe that now rejects
    // everything keeps the first answer (K stable across sessions)
    FakeSettlement rejectAll;
    rejectAll.labels = {"Town", "Other"};
    OccupierForces again;
    again.LoadFromParams(f.file.FindEntry("CfgGuerrillaZones"));
    REQUIRE(again.ActivateProbed(f.registry, &rejectAll));
    CHECK(again.TownCount() == 1);
    CHECK(again.ProbedOutCount() == 1);

    // the probed-out count rides in the block
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    OccupierForces loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(loaded.TownCount() == 1);
    CHECK(loaded.ProbedOutCount() == 1);
    ExtParsMission.Clear();
    std::filesystem::remove(path);
}

TEST_CASE("OccupierForces - a save without the block probes its seeded towns when it derives the pool",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.Z("Other").seeded = true;
    FakeSettlement probe;
    probe.labels.push_back("Other");
    f.of.DeriveFromExistingSave(f.registry, &probe);
    REQUIRE(f.of.Active());
    CHECK(f.Z("Other").settlement == -1);
    CHECK(f.of.TownCount() == 1);
    CHECK(f.of.ProbedOutCount() == 1);
}

TEST_CASE("OccupierForces - an unprobed seeded town counts in K", "[game][guerrilla][occupierforces]")
{
    // settlement 0 (never probed: the test path, or no landscape) keeps the
    // phase-7 answer
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.Z("Other").seeded = true;
    REQUIRE(f.of.Activate(f.registry));
    CHECK(f.of.TownCount() == 2);
    CHECK(f.of.ProbedOutCount() == 0);
}

TEST_CASE("OccupierForces - an empty roster switches the service off", "[game][guerrilla][occupierforces]")
{
    const char* noBases = "class CfgGuerrillaZones\n"
                          "{\n"
                          "    occupierForces = 1;\n"
                          "    class Zones\n"
                          "    {\n"
                          "        class Camp { name=\"Camp\"; type=\"CAMP\"; owner=\"RESISTANCE\"; "
                          "position[]={1000.0, 1000.0, 0.0}; };\n"
                          "        class Town { name=\"Town\"; type=\"CITY\"; owner=\"NEUTRAL\"; "
                          "position[]={1000.0, 1300.0, 0.0}; };\n"
                          "        class Rebel { name=\"Rebel\"; type=\"OUTPOST\"; owner=\"RESISTANCE\"; "
                          "position[]={2000.0, 2000.0, 0.0}; };\n"
                          "    };\n"
                          "};\n";
    Fixture f;
    f.Load(noBases);
    CHECK(f.of.Enabled());
    CHECK_FALSE(f.of.Activate(f.registry));
    CHECK_FALSE(f.of.Active());
    CHECK_FALSE(f.of.HasState());
    CHECK(f.of.Draw(2) == 2); // inert: the seams pass through
}

// ---------------------------------------------------------------------------
// refill (5.2)
// ---------------------------------------------------------------------------

TEST_CASE("OccupierForces - the refill formula and the standing factor", "[game][guerrilla][occupierforces]")
{
    CHECK(OccupierForces::StandingFactor(50.0f) == Approx(1.0f));
    CHECK(OccupierForces::StandingFactor(0.0f) == Approx(0.75f));
    CHECK(OccupierForces::StandingFactor(100.0f) == Approx(1.25f));
    CHECK(OccupierForces::StandingFactor(-20.0f) == Approx(0.75f));
    // Abel: (60 + 32 + 56) / 4 = 37 an hour at the default standing
    CHECK(OccupierForces::RefillPerHour(60, 32, 56, 4.0f, 50.0f) == Approx(37.0f));
    CHECK(OccupierForces::RefillPerHour(60, 32, 56, 4.0f, 0.0f) == Approx(27.75f));
    CHECK(OccupierForces::RefillPerHour(60, 32, 56, 0.0f, 50.0f) == Approx(0.0f));
}

TEST_CASE("OccupierForces - full exhaustion refills in 4 h, garrisons first, one SPENT exit",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    REQUIRE(f.of.PoolStart() == 60);
    REQUIRE(f.of.RosterStrength() == 32);
    REQUIRE(f.of.TownCount() == 14);
    REQUIRE(f.of.TownGarrisons() == 56);

    // exhausted: pool 0, every garrison empty, every D2 clock already
    // waiting at its interval (the garrisons have been short for a while)
    FakeD2 d2;
    for (int i = 0; i < 4; i++)
    {
        d2.Add(0, 8, FakeD2::Interval);
    }
    for (int i = 0; i < 14; i++)
    {
        d2.Add(0, 4, FakeD2::Interval);
    }
    f.of.SetPool(0);
    REQUIRE(f.of.Band() == OFBandSpent);

    const float step = f.of.Tuning().directorInterval;
    float t = 0;
    float garrisonsFullAt = -1;
    float fullAt = -1;
    int maxPoolWhileShort = 0;
    std::vector<int> bands;
    bands.push_back(f.of.Band());
    for (int i = 0; i < 6 * 60 && fullAt < 0; i++)
    {
        f.of.Tick(f.registry, step);
        d2.Step(f.of, step);
        t += step;
        if (f.of.Band() != bands.back())
        {
            bands.push_back(f.of.Band());
        }
        if (!d2.Full() && f.of.Pool() > maxPoolWhileShort)
        {
            maxPoolWhileShort = f.of.Pool();
        }
        if (garrisonsFullAt < 0 && d2.Full())
        {
            garrisonsFullAt = t;
        }
        if (d2.Full() && f.of.Pool() >= f.of.PoolStart())
        {
            fullAt = t;
        }
    }
    REQUIRE(fullAt > 0);
    // 88 garrison bodies at 37 an hour, then a pool of 60: 4 h in all.  The
    // total is fixed by the refill (148 units at 37 an hour)
    CHECK(fullAt / 3600.0f == Approx(4.0f).margin(0.05f));
    // the garrisons fill first, but not quite at 88 / 37 = 2.38 h: at the
    // tail only a few zones are still short and each takes at most one body
    // per 600 s, so the per-zone D2 cap binds there (measured 2.57 h) and the
    // surplus collects in the pool
    CHECK(garrisonsFullAt / 3600.0f > 88.0f / 37.0f - 0.05f);
    CHECK(garrisonsFullAt / 3600.0f < 2.7f);
    // D2 draws first: the pool stays low while garrisons are short (it
    // reaches 7, about 12 %, only in that tail)
    CHECK(maxPoolWhileShort <= 8);
    // one exit from SPENT, then straight up: no flapping
    REQUIRE(bands.size() == 4);
    CHECK(bands[0] == OFBandSpent);
    CHECK(bands[1] == OFBandThin);
    CHECK(bands[2] == OFBandStretched);
    CHECK(bands[3] == OFBandStrong);
    CHECK(f.of.RefillTotal() == 148);
}

TEST_CASE("OccupierForces - an empty pool with full garrisons refills in about 1.6 h",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetPool(0);
    float t = 0;
    while (f.of.Pool() < f.of.PoolStart() && t < 6 * 3600.0f)
    {
        f.of.Tick(f.registry, 60.0f);
        t += 60.0f;
    }
    CHECK(t / 3600.0f == Approx(60.0f / 37.0f).margin(0.03f));
    // the refill never pushes the pool above poolStart
    f.of.Tick(f.registry, 3600.0f);
    CHECK(f.of.Pool() == f.of.PoolStart());
    CHECK(f.of.RefillAccum() == Approx(0.0f));
}

TEST_CASE("OccupierForces - standing scales the refill", "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetPool(0);
    f.of.SetMeanStanding(0.0f); // a heavily bombed theatre: 27.75 an hour
    f.of.Tick(f.registry, 3600.0f);
    CHECK(f.of.Pool() == 27);
    f.of.SetMeanStanding(50.0f);
}

// ---------------------------------------------------------------------------
// bands (5.6)
// ---------------------------------------------------------------------------

TEST_CASE("OccupierForces - band hysteresis", "[game][guerrilla][occupierforces]")
{
    const int P = 100;
    CHECK(OccupierForces::InitialBand(100, P) == OFBandStrong);
    CHECK(OccupierForces::InitialBand(66, P) == OFBandStretched); // enter is strictly above 66 %
    CHECK(OccupierForces::InitialBand(67, P) == OFBandStrong);
    CHECK(OccupierForces::InitialBand(33, P) == OFBandStretched);
    CHECK(OccupierForces::InitialBand(32, P) == OFBandThin);
    CHECK(OccupierForces::InitialBand(0, P) == OFBandSpent);
    CHECK(OccupierForces::InitialBand(5, 0) == OFBandSpent);

    // STRONG: leave below 60 %
    CHECK(OccupierForces::StepBand(OFBandStrong, 62, P) == OFBandStrong);
    CHECK(OccupierForces::StepBand(OFBandStrong, 60, P) == OFBandStrong);
    CHECK(OccupierForces::StepBand(OFBandStrong, 59, P) == OFBandStretched);
    // STRETCHED: STRONG above 66 %, THIN below 33 %
    CHECK(OccupierForces::StepBand(OFBandStretched, 64, P) == OFBandStretched);
    CHECK(OccupierForces::StepBand(OFBandStretched, 67, P) == OFBandStrong);
    CHECK(OccupierForces::StepBand(OFBandStretched, 33, P) == OFBandStretched);
    CHECK(OccupierForces::StepBand(OFBandStretched, 32, P) == OFBandThin);
    // THIN: leave above 40 %
    CHECK(OccupierForces::StepBand(OFBandThin, 38, P) == OFBandThin);
    CHECK(OccupierForces::StepBand(OFBandThin, 40, P) == OFBandThin);
    CHECK(OccupierForces::StepBand(OFBandThin, 41, P) == OFBandStretched);
    // SPENT: entered at 0 from anywhere, left above 10 %
    CHECK(OccupierForces::StepBand(OFBandStrong, 0, P) == OFBandSpent);
    CHECK(OccupierForces::StepBand(OFBandThin, 0, P) == OFBandSpent);
    CHECK(OccupierForces::StepBand(OFBandSpent, 1, P) == OFBandSpent);
    CHECK(OccupierForces::StepBand(OFBandSpent, 10, P) == OFBandSpent);
    CHECK(OccupierForces::StepBand(OFBandSpent, 11, P) == OFBandThin);
    // a jump crosses several bands at once (a wave)
    CHECK(OccupierForces::StepBand(OFBandSpent, 90, P) == OFBandStrong);
    CHECK(OccupierForces::StepBand(OFBandSpent, 50, P) == OFBandStretched);
    CHECK(OccupierForces::StepBand(OFBandStrong, 20, P) == OFBandThin);

    // THIN / STRETCHED: a pool wobbling around 33 % does not flap
    int band = OFBandStretched;
    int changes = 0;
    const int wobble[] = {34, 32, 35, 33, 38, 31, 39, 34, 40, 36};
    for (int pool : wobble)
    {
        const int next = OccupierForces::StepBand(band, pool, P);
        changes += next != band ? 1 : 0;
        band = next;
    }
    CHECK(changes == 1); // into THIN at 32, never back out below 41
    CHECK(band == OFBandThin);
    // SPENT / THIN: a pool near 0 during recovery leaves SPENT once
    band = OFBandSpent;
    changes = 0;
    const int recovery[] = {0, 1, 0, 2, 1, 0, 3, 5, 8, 10, 11, 9, 12, 15};
    for (int pool : recovery)
    {
        const int next = OccupierForces::StepBand(band, pool, P);
        changes += next != band ? 1 : 0;
        band = next;
    }
    CHECK(changes == 1);
    CHECK(band == OFBandThin);
}

TEST_CASE("OccupierForces - token intervals by band", "[game][guerrilla][occupierforces]")
{
    OccupierForcesTuning t;
    CHECK(OccupierForces::TokenInterval(t, OFBandStrong) == Approx(600.0f));
    CHECK(OccupierForces::TokenInterval(t, OFBandStretched) == Approx(1050.0f));
    CHECK(OccupierForces::TokenInterval(t, OFBandThin) == Approx(1800.0f));
    CHECK(OccupierForces::TokenInterval(t, OFBandSpent) == Approx(0.0f));
    CHECK(OccupierForces::TokenInterval(t, 9) == Approx(0.0f));
    CHECK(Str(OccupierForces::BandName(OFBandStretched)) == "STRETCHED");
    CHECK(Str(OccupierForces::BandName(-1)) == "OFF");
    CHECK(Str(OccupierForces::PostureName(OFDormant)) == "DORMANT");
}

// ---------------------------------------------------------------------------
// Draw, Spend and the floor (5.3, 5.6)
// ---------------------------------------------------------------------------

TEST_CASE("OccupierForces - Draw grants what the pool holds; the floor stops operations",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry)); // poolStart 60
    CHECK(f.of.Draw(0) == 0);
    CHECK(f.of.Draw(5) == 5);
    CHECK(f.of.Pool() == 55);
    CHECK(f.of.CanOperate());
    CHECK(f.of.Spend(10, "op"));
    CHECK(f.of.Pool() == 45);
    CHECK(f.of.Band() == OFBandStrong); // 75 %: STRONG is left only below 60 %
    CHECK(f.of.Spend(20, "op"));
    CHECK(f.of.Pool() == 25);
    CHECK(f.of.Band() == OFBandStretched);
    CHECK_FALSE(f.of.Spend(50, "too dear")); // more than the pool
    CHECK(f.of.Pool() == 25);

    // THIN below 20 % (12 of 60): no operation, D2 still draws
    f.of.SetPool(15);
    CHECK(f.of.Band() == OFBandThin);
    CHECK(f.of.CanOperate()); // 15 >= 12
    f.of.SetPool(11);
    CHECK_FALSE(f.of.CanOperate());
    CHECK_FALSE(f.of.Spend(1, "op"));
    CHECK(f.of.Pool() == 11);
    CHECK(f.of.Draw(3) == 3);
    CHECK(f.of.Deposit(2, "revert") == 2); // not SPENT: the revert is refilled
    CHECK(f.of.Pool() == 6);
    // an empty pool grants nothing; SPENT
    CHECK(f.of.Draw(10) == 6);
    CHECK(f.of.Pool() == 0);
    CHECK(f.of.Band() == OFBandSpent);
    CHECK(f.of.Draw(1) == 0);
    CHECK_FALSE(f.of.CanOperate());
    // SPENT leaves reverts hollow even once something trickles back
    f.of.Refund(3, "survivors");
    CHECK(f.of.Band() == OFBandSpent);
    CHECK(f.of.Deposit(2, "revert") == 0);
    CHECK(f.of.Draw(1) == 1); // ... but D2 keeps running (D15)
    // a refund never pushes the pool above poolStart (5.5, the ceiling):
    // every base sits at its garrisonMax here, so the rest is dropped and
    // booked (a base with room banks it: the banking cases below)
    CHECK(f.of.Refund(80, "surplus") == 58);
    CHECK(f.of.Banked() == 0);
    CHECK(f.of.Pool() == 60);
    CHECK(f.of.Dropped() == 22);
    CHECK(f.of.Band() == OFBandStrong);
    CHECK(OccupierForces::CanOperateIn(OFBandThin, 12, 60, 0.2f));
    CHECK_FALSE(OccupierForces::CanOperateIn(OFBandThin, 11, 60, 0.2f));
    CHECK_FALSE(OccupierForces::CanOperateIn(OFBandSpent, 30, 60, 0.2f));
    CHECK(OccupierForces::CanOperateIn(OFBandStretched, 1, 60, 0.2f));
}

// ---------------------------------------------------------------------------
// control (7.1) and the war-level split
// ---------------------------------------------------------------------------

TEST_CASE("OccupierForces - control from the roster strength and the administered towns",
          "[game][guerrilla][occupierforces]")
{
    CHECK(OccupierForces::ControlOf(18, 18, 2, 2, 0.6f) == Approx(1.0f));
    CHECK(OccupierForces::ControlOf(0, 18, 2, 2, 0.6f) == Approx(0.4f));
    CHECK(OccupierForces::ControlOf(9, 18, 0, 2, 0.6f) == Approx(0.3f));
    CHECK(OccupierForces::ControlOf(9, 18, 0, 0, 0.6f) == Approx(0.5f));       // no towns: C = M
    CHECK(OccupierForces::ControlOf(0, 0, 1, 2, 0.6f) == Approx(0.6f + 0.2f)); // empty roster strength: M = 1

    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    CHECK(f.of.Control() == Approx(1.0f));
    f.GiveToResistance("FarBase"); // M = 6 / 18
    f.of.Tick(f.registry, 60.0f);
    CHECK(f.of.Control() == Approx(0.6f * 6.0f / 18.0f + 0.4f));
    f.GiveToResistance("Town"); // T = 1 / 2
    f.of.Tick(f.registry, 60.0f);
    CHECK(f.of.Control() == Approx(0.6f * 6.0f / 18.0f + 0.2f));
    CHECK(f.of.ComputeControl(f.registry) == Approx(f.of.Control()));
    // a town the resistance held at the start never counts (it is not a K town)
    f.Z("Held").owner = RString("NEUTRAL");
    CHECK(f.of.ComputeControl(f.registry) == Approx(f.of.Control()));
}

TEST_CASE("OccupierForces - waves crossed by control", "[game][guerrilla][occupierforces]")
{
    OccupierForcesTuning t;
    CHECK(OccupierForces::WavesCrossed(t, 1.0f) == 0);
    CHECK(OccupierForces::WavesCrossed(t, 0.76f) == 0);
    CHECK(OccupierForces::WavesCrossed(t, 0.75f) == 1);
    CHECK(OccupierForces::WavesCrossed(t, 0.6f) == 1);
    CHECK(OccupierForces::WavesCrossed(t, 0.5f) == 2);
    CHECK(OccupierForces::WavesCrossed(t, 0.1f) == 2);
}

TEST_CASE("OccupierForces - the occupier war level adds landed waves to the territory level",
          "[game][guerrilla][occupierforces]")
{
    CHECK(OccupierForces::OccupierWarLevelOf(2.0f, 0, 1.0f) == Approx(2.0f));
    CHECK(OccupierForces::OccupierWarLevelOf(2.0f, 1, 1.0f) == Approx(3.0f));
    CHECK(OccupierForces::OccupierWarLevelOf(2.0f, 3, 1.0f) == Approx(5.0f));
    CHECK(OccupierForces::OccupierWarLevelOf(5.0f, 3, 1.0f) == Approx(6.0f)); // capped
    CHECK(OccupierForces::OccupierWarLevelOf(6.5f, 1, 1.0f) == Approx(6.5f)); // never lowered
    CHECK(OccupierForces::OccupierWarLevelOf(2.0f, 2, 0.0f) == Approx(2.0f));
    CHECK(OccupierForces::OccupierWarLevelOf(1.0f, 2, 0.5f) == Approx(2.0f));

    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.LandWave(12);
    CHECK(f.of.WavesLanded() == 1);
    CHECK(f.of.Pool() == 36); // above poolStart
    CHECK(f.of.Band() == OFBandStrong);
}

// ---------------------------------------------------------------------------
// conservation (5.5)
// ---------------------------------------------------------------------------

TEST_CASE("OccupierForces - the ledger is conserved through draws, operations, refunds, losses, refill and waves",
          "[game][guerrilla][occupierforces]")
{
    CHECK(OccupierForces::ConservationCheck(60, 88, 0, 0, 60, 32, 56, 0, 0, 0));
    CHECK(OccupierForces::ConservationImbalance(59, 88, 0, 0, 60, 32, 56, 0, 0, 0) == -1);

    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart(), R = f.of.RosterStrength(), G = f.of.TownGarrisons();
    // the fake ledger: the garrison reserves start full, nothing pending,
    // nothing in the field
    int reserves = R + G;
    int pending = 0;
    int op = 0;
    int losses = 0;
    auto check = [&]()
    {
        CHECK(OccupierForces::ConservationCheck(f.of.Pool(), reserves, pending, op, P, R, G, f.of.RefillTotal(),
                                                f.of.WavesTotal(), losses));
    };
    check();
    // a garrison of 6 is wiped out
    reserves -= 6;
    losses += 6;
    f.of.NoteLoss(6, "garrison wiped");
    check();
    // D2 regrows two of them from the pool
    reserves += f.of.Draw(2);
    check();
    // an operation of 8 leaves; 3 die, 5 come home with no base to return to
    REQUIRE(f.of.Spend(8, "retake"));
    op += 8;
    check();
    op -= 3;
    losses += 3;
    f.of.NoteLoss(3, "repelled");
    check();
    op -= 5;
    f.of.Refund(5, "no base to return to");
    check();
    // a reverted town's garrison as a pending deposit, later materialized
    const int dep = f.of.Deposit(4, "revert");
    pending += dep;
    check();
    pending -= dep;
    reserves += dep;
    check();
    // an hour of refill, then a wave
    f.of.Tick(f.registry, 3600.0f);
    check();
    f.of.LandWave(30);
    check();
    CHECK(f.of.Lost() == 9);
    CHECK(f.of.WavesTotal() == 30);
}

namespace
{
// every Core log payload while it lives (the director lines log at
// DirectorLogLevel; the capture opens the logger to trace and restores it)
struct CoreLines
{
    spdlog::logger* logger = LogDetail::Get(Poseidon::Foundation::LogCategory::Core);
    std::vector<spdlog::sink_ptr> saved;
    spdlog::level::level_enum savedLevel = spdlog::level::off;
    std::vector<std::string> lines;
    CoreLines()
    {
        saved = logger->sinks();
        savedLevel = logger->level();
        logger->sinks().clear();
        logger->sinks().push_back(std::make_shared<spdlog::sinks::callback_sink_mt>(
            [this](const spdlog::details::log_msg& m) { lines.emplace_back(m.payload.data(), m.payload.size()); }));
        logger->set_level(spdlog::level::trace);
    }
    ~CoreLines()
    {
        logger->sinks() = saved;
        logger->set_level(savedLevel);
    }
    // the lines containing every needle
    std::vector<std::string> With(std::initializer_list<const char*> needles) const
    {
        std::vector<std::string> out;
        for (const std::string& l : lines)
        {
            bool all = true;
            for (const char* n : needles)
            {
                all = all && l.find(n) != std::string::npos;
            }
            if (all)
            {
                out.push_back(l);
            }
        }
        return out;
    }
};
} // namespace

TEST_CASE("OccupierForces - the ceiling: a refund over poolStart is dropped once, booked, and the ledger holds",
          "[game][guerrilla][occupierforces]")
{
    // the comparison video (tmp/of-director-video-3): the refill topped the
    // pool back up while an operation was out, and the operation's refund
    // then landed it above poolStart (A: 51 of 50; B: 53 of 50 after an APC
    // came home).  Nothing but a wave lifts the pool above poolStart
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart(), R = f.of.RosterStrength(), G = f.of.TownGarrisons();
    int reserves = R + G;
    int op = 0;
    int losses = 0;
    auto check = [&]()
    {
        // the dropped units leave the ledger beside the losses (5.5)
        CHECK(OccupierForces::ConservationCheck(f.of.Pool(), reserves, 0, op, P, R, G, f.of.RefillTotal(),
                                                f.of.WavesTotal(), losses + f.of.Dropped()));
    };
    check();
    REQUIRE(f.of.WaveHeadroom() == 0);
    REQUIRE(f.of.PoolCeiling() == P);
    // an APC of 11 leaves; the refill tops the pool back up meanwhile
    REQUIRE(f.of.Spend(11, "APC"));
    op += 11;
    check();
    f.of.Tick(f.registry, 3600.0f);
    REQUIRE(f.of.Pool() == P);
    check();
    // the hull and its crew come home: nothing above poolStart, one line
    CoreLines log;
    CHECK(f.of.Refund(10, "hull home intact") == 0);
    op -= 10;
    CHECK(f.of.Pool() == P);
    CHECK(f.of.Dropped() == 10);
    CHECK(log.With({"ledger: 10 unit(s) over poolStart dropped"}).size() == 1);
    check();
    // a partial refund: what fits goes in, the rest is dropped
    REQUIRE(f.of.Spend(4, "truck"));
    op += 4;
    CHECK(f.of.Refund(1, "partial spawn") == 1); // room for it
    op -= 1;
    check();
    CHECK(f.of.Refund(3, "surplus") == 3);
    op -= 3;
    CHECK(f.of.Pool() == P);
    CHECK(f.of.Dropped() == 10); // nothing over: nothing dropped, no line
    CHECK(log.With({"over poolStart dropped"}).size() == 1);
    check();
    // the ledger never reads above the ceiling, whatever comes back
    CHECK(f.of.Refund(25, "survivors with no base") == 0);
    CHECK(f.of.Pool() == P);
    CHECK(f.of.Dropped() == 35);
    // those 25 were reserve bodies with no room: the fake ledger books them
    reserves -= 25;
    check();
    CHECK(f.of.Lost() == 0); // the `lost` field still names the bodies killed only
}

TEST_CASE("OccupierForces - the ceiling keeps a landed wave's headroom; lost wave strength takes it",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart();
    // the headroom alone: no wave surge to take the excess (surgeShare 0),
    // so what the ceiling keeps out is banked or dropped as outside a wave
    f.of.TuningMutable().surgeShare = 0;
    // a wave lifts the pool above poolStart (7.1) and the ceiling with it
    f.of.LandWave(12);
    CHECK(f.of.Pool() == P + 12);
    CHECK(f.of.WaveHeadroom() == 12);
    CHECK(f.of.PoolCeiling() == P + 12);
    // wave strength spent and refunded comes back above poolStart
    REQUIRE(f.of.Spend(10, "wave strike hull"));
    CHECK(f.of.Refund(10, "hull home intact") == 10);
    CHECK(f.of.Pool() == P + 12);
    CHECK(f.of.Dropped() == 0);
    // spent below poolStart, the refill tops the pool up meanwhile: the
    // column home brings the pool back to the wave's level, and the units
    // the refill credited in the meantime are the ones dropped
    REQUIRE(f.of.Spend(20, "column"));
    REQUIRE(f.of.Pool() == P - 8);
    const int refill0 = f.of.RefillTotal();
    f.of.Tick(f.registry, 300.0f);
    const int credited = f.of.RefillTotal() - refill0;
    REQUIRE(credited > 0);
    REQUIRE(credited < 8);
    CHECK(f.of.Refund(20, "column home") == 20 - credited);
    CHECK(f.of.Pool() == P + 12);
    CHECK(f.of.Dropped() == credited);
    CHECK(f.of.WaveHeadroom() == 12);
    // a second wave raises the headroom to its own overshoot
    f.of.LandWave(3);
    CHECK(f.of.Pool() == P + 15);
    CHECK(f.of.WaveHeadroom() == 15);
    // wave strength lost in the field takes its headroom with it: of 15
    // spent, 12 are lost, so a later refund lifts the pool at most by what
    // can still come back
    REQUIRE(f.of.Spend(15, "column"));
    f.of.NoteLoss(12, "column lost");
    const int dropped0 = f.of.Dropped();
    CHECK(f.of.Refund(3, "survivors") == 3);
    CHECK(f.of.WaveHeadroom() == 3);
    CHECK(f.of.Refund(4, "a surplus the refill already replaced") == 0);
    CHECK(f.of.Pool() == P + 3);
    CHECK(f.of.Dropped() == dropped0 + 4);
    // a forced pool carries its own headroom (the gmForcesForce lever)
    f.of.SetPool(P);
    CHECK(f.of.WaveHeadroom() == 0);
    f.of.SetPool(P + 5);
    CHECK(f.of.WaveHeadroom() == 5);
}

TEST_CASE("OccupierForces - the ceiling survives save/load", "[game][guerrilla][occupierforces][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "occupier-forces-ceiling.bin";
    const std::string config = AbelLikeConfig();
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    f.Load(config, false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart();
    f.of.TuningMutable().surgeShare = 0; // the headroom alone (no wave surge)
    f.of.LandWave(6);
    REQUIRE(f.of.Spend(6, "op"));
    f.of.Tick(f.registry, 1.0f); // the pool sits at poolStart: no refill, no headroom used
    REQUIRE(f.of.Refund(9, "op home") == 6);
    REQUIRE(f.of.Dropped() == 3);
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    OccupierForces loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(loaded.Pool() == P + 6);
    CHECK(loaded.WaveHeadroom() == 6);
    CHECK(loaded.PoolCeiling() == P + 6);
    CHECK(loaded.Dropped() == 3);
    ExtParsMission.Clear();
    std::filesystem::remove(path);
}

namespace
{
// the S5 pending deposits over every zone of a fixture
int PendingOf(Fixture& f)
{
    int n = 0;
    for (int i = 0; i < f.registry.NZones(); i++)
    {
        n += f.registry.GetZone(i)->pending;
    }
    return n;
}
} // namespace

TEST_CASE("OccupierForces - the ceiling banks the excess as garrison at a base with room, drops nothing",
          "[game][guerrilla][occupierforces]")
{
    // the owner, 2026-10-16: "bank surplus survivors into garrisons instead
    // of dropping them".  Refund(80) on an empty pool of poolStart 60: 60 go
    // back into the pool, the 20 over poolStart bank at the one base with
    // room, as a pending deposit, and nothing is dropped
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart();
    REQUIRE(P == 60);
    f.Z("Base1").garrisonMax = 30; // room 22 (8 of 30)
    f.of.SetPool(0);
    CoreLines log;
    CHECK(f.of.Refund(80, "survivors home") == 60);
    CHECK(f.of.Pool() == P);
    CHECK(f.of.Dropped() == 0);
    CHECK(f.of.Banked() == 20);
    CHECK(f.Z("Base1").pending == 20);
    CHECK(f.Z("Base1").garrison == Approx(8.0f)); // pending first: S5, never beside the player
    CHECK(f.Z("Base0").pending == 0);
    CHECK(log.With({"ledger: 20 unit(s) over poolStart banked at Base1"}).size() == 1);
    CHECK(log.With({"over poolStart dropped"}).empty());
    // the deposit joins the reserve once the player is away, so the
    // director can spend it again (Spare reads the reserve)
    const int base1 = f.registry.FindZoneIndex("Base1");
    CHECK(GarrisonCache::FlushPending(f.registry, base1, true, 99999.0f, 99999.0f, nullptr) == PFReleasedBubble);
    CHECK(f.Z("Base1").garrison == Approx(28.0f));
    CHECK(f.Z("Base1").pending == 0);
    // the base is 2 short of its garrisonMax now: the next excess banks 2
    // there and drops the rest, every base being full
    CHECK(f.of.Refund(5, "surplus") == 0);
    CHECK(f.of.Banked() == 22);
    CHECK(f.of.Dropped() == 3);
    CHECK(log.With({"ledger: 2 unit(s) over poolStart banked at Base1"}).size() == 1);
    CHECK(log.With({"ledger: 3 unit(s) over poolStart dropped"}).size() == 1);
}

TEST_CASE("OccupierForces - the ceiling banks at the source base first, then the nearest base with room",
          "[game][guerrilla][occupierforces]")
{
    // Base0..Base3 stand 1 km apart on a line (x = 2000..5000)
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart();
    const int b0 = f.registry.FindZoneIndex("Base0");
    const int b3 = f.registry.FindZoneIndex("Base3");
    REQUIRE(b0 >= 0);
    REQUIRE(b3 >= 0);
    f.Z("Base1").garrison = 3; // room 5
    f.Z("Base3").garrison = 2; // room 6
    REQUIRE(f.of.Pool() == P);
    CoreLines log;

    // two bases with room, the nearer one (Base0) full: the excess banks at
    // the nearest with room, Base1, not at Base3
    const Vector3 atBase0 = f.Z("Base0").pos;
    CHECK(f.of.Refund(4, "survivors near Base0", -1, &atBase0) == 0);
    CHECK(f.Z("Base1").pending == 4);
    CHECK(f.Z("Base3").pending == 0);
    // the source base is full, so its position decides: Base1 again (room
    // 1 left), then the next nearest, Base3, for the rest
    CHECK(f.of.Refund(3, "surplus over Base0's room", b0) == 0);
    CHECK(f.Z("Base1").pending == 5);
    CHECK(f.Z("Base3").pending == 2);
    CHECK(f.Z("Base1").pendingOrigin == -1); // two origins on one deposit: mixed (3.3)
    CHECK(f.Z("Base3").pendingOrigin == b0);
    CHECK(log.With({"ledger: 1 unit(s) over poolStart banked at Base1"}).size() == 1);
    CHECK(log.With({"ledger: 2 unit(s) over poolStart banked at Base3"}).size() == 1);
    // a source base with room takes it before any nearer one
    f.Z("Base1").garrison = 0; // room 3 again, and nearer to Base0 than Base3 is
    CHECK(f.of.Refund(2, "home to Base3", b3, &atBase0) == 0);
    CHECK(f.Z("Base3").pending == 4);
    CHECK(f.Z("Base1").pending == 5);
    // nothing known: the base with the most room (Base1, 3, over Base3, 2)
    CHECK(f.of.Refund(2, "partial spawn") == 0);
    CHECK(f.Z("Base1").pending == 7);
    CHECK(f.of.Dropped() == 0);
    CHECK(f.of.Banked() == 11);
    // only occupier-held military zones bank: a resistance-held base with
    // room and the towns take nothing; the rest is dropped with the line
    f.GiveToResistance("Base3");
    f.Z("Base1").garrison = 1; // full again (1 + 7 pending of 8)
    CHECK(f.of.Refund(6, "survivors with no base") == 0);
    CHECK(f.Z("Base3").pending == 4);
    CHECK(f.of.Banked() == 11);
    CHECK(f.of.Dropped() == 6);
    CHECK(log.With({"ledger: 6 unit(s) over poolStart dropped", "every occupier base at garrisonMax"}).size() == 1);
    CHECK(f.of.Pool() == P);
}

TEST_CASE("OccupierForces - the ceiling: every base full drops the excess with the line; a live garrison is room "
          "taken",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart();
    CoreLines log;
    CHECK(f.of.Refund(7, "HQ assault done") == 0);
    CHECK(f.of.Pool() == P);
    CHECK(f.of.Banked() == 0);
    CHECK(f.of.Dropped() == 7);
    CHECK(PendingOf(f) == 0);
    CHECK(log.With({"ledger: 7 unit(s) over poolStart dropped", "every occupier base at garrisonMax"}).size() == 1);
    CHECK(log.With({"over poolStart banked"}).empty());
    // the room counts the live garrison and the pending deposit (5.5)
    const int b2 = f.registry.FindZoneIndex("Base2");
    f.Z("Base2").garrison = 0;
    CHECK(OccupierForces::BankRoom(f.registry, b2, 0) == 8);
    CHECK(OccupierForces::BankRoom(f.registry, b2, 5) == 3);
    f.Z("Base2").pending = 2;
    CHECK(OccupierForces::BankRoom(f.registry, b2, 5) == 1);
    CHECK(OccupierForces::BankRoom(f.registry, b2, 9) == 0);
    CHECK(OccupierForces::BankRoom(f.registry, f.registry.FindZoneIndex("T0"), 0) == 0); // a town
    // a local service with no registry bound has nowhere to bank: it drops
    OccupierForces local;
    CHECK(local.BankRegistry() == nullptr);
    CHECK(OccupierForces::Instance().BankRegistry() == &ZoneRegistry::Instance());
}

TEST_CASE("OccupierForces - the ledger is conserved with the excess banked as garrison",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart(), R = f.of.RosterStrength(), G = f.of.TownGarrisons();
    // the fake reserves start full; pending is read from the registry
    int reserves = R + G;
    int op = 0;
    int losses = 0;
    auto check = [&]()
    {
        // banking moves units from op strength into pending: no new term;
        // only the units no base could take leave beside the losses
        CHECK(OccupierForces::ConservationCheck(f.of.Pool(), reserves, PendingOf(f), op, P, R, G,
                                                f.of.RefillTotal(), f.of.WavesTotal(), losses + f.of.Dropped()));
    };
    check();
    // a truck column of 8 leaves Base2's whole reserve; 4 of them die
    f.Z("Base2").garrison = 0;
    reserves -= 8;
    REQUIRE(f.of.Spend(4, "truck and driver"));
    op += 8 + 4;
    check();
    op -= 4;
    losses += 4;
    f.of.NoteLoss(4, "repelled");
    check();
    // the refill tops the pool back up while the column is out
    f.of.Tick(f.registry, 3600.0f);
    REQUIRE(f.of.Pool() == P);
    check();
    // the hull and the four survivors come home to a full pool: Base2 has
    // room 8, so all eight bank there and nothing is dropped
    CHECK(f.of.Refund(4, "hull home intact", f.registry.FindZoneIndex("Base2")) == 0);
    op -= 4;
    check();
    CHECK(f.of.Refund(4, "surplus") == 0);
    op -= 4;
    check();
    CHECK(f.of.Dropped() == 0);
    CHECK(f.of.Banked() == 8);
    CHECK(f.Z("Base2").pending == 8);
    CHECK(op == 0);
    // with no room left, the last two are dropped and still balance
    CHECK(f.of.Pool() == P);
    reserves -= 2;
    CHECK(f.of.Refund(2, "survivors") == 0); // 2 more out of the fake reserves
    CHECK(f.of.Dropped() == 2);
    check();
}

TEST_CASE("OccupierForces - S5: a pending deposit whose zone changes owner goes back to the pool, the ledger holds",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    f.of.InstallPendingRefundHook();
    REQUIRE(GarrisonCache::GetPendingRefundHook() == &OccupierForces::RefundPendingToPool);
    const int P = f.of.PoolStart(), R = f.of.RosterStrength(), G = f.of.TownGarrisons();
    // the fake reserves start full; pending is read from the registry
    int reserves = R + G;
    const int op = 0;
    const int losses = 0;
    auto pending = [&]()
    {
        int n = 0;
        for (int i = 0; i < f.registry.NZones(); i++)
        {
            n += f.registry.GetZone(i)->pending;
        }
        return n;
    };
    auto check = [&]()
    {
        // the units the ceiling dropped leave the ledger beside the losses
        CHECK(OccupierForces::ConservationCheck(f.of.Pool(), reserves, pending(), op, P, R, G, f.of.RefillTotal(),
                                                f.of.WavesTotal(), losses + f.of.Dropped()));
    };
    check();
    // a revert refill of 4 into T0, paid by the pool, with the player
    // standing in the town: a PENDING deposit (S5)
    const int t0 = f.registry.FindZoneIndex("T0");
    REQUIRE(t0 >= 0);
    const float px = f.Z("T0").pos.X();
    const float pz = f.Z("T0").pos.Z();
    const int dep = f.of.Deposit(4, "revert");
    REQUIRE(dep == 4);
    REQUIRE(GarrisonCache::AddReserve(f.registry, t0, dep, -1, true, px, pz));
    CHECK(pending() == 4);
    check();
    // the town changes owner while the deposit waits: the four go back to
    // the pool, not to an origin post and not discarded
    const int poolBefore = f.of.Pool();
    f.GiveToResistance("T0");
    CHECK(GarrisonCache::FlushPending(f.registry, t0, true, px, pz, nullptr) == PFRefunded);
    CHECK(pending() == 0);
    CHECK(f.Z("T0").pendingOrigin == -1);
    CHECK(f.of.Pool() == poolBefore + 4);
    check();
    // a deposit from a named post (a survivor return) is pooled the same way
    const int base1 = f.registry.FindZoneIndex("Base1");
    REQUIRE(base1 >= 0);
    reserves -= 3; // three leave Base1's reserve ...
    f.Z("Base1").garrison -= 3.0f;
    const float base1Left = f.Z("Base1").garrison;
    const int t1 = f.registry.FindZoneIndex("T1");
    GarrisonCache::Deposit(f.registry, t1, 3, base1); // ... and wait at T1
    check();
    f.GiveToResistance("T1");
    GarrisonCache::RefundPending(f.registry, t1, true, px, pz);
    CHECK(f.Z("Base1").garrison == Approx(base1Left)); // the reserve itself gets nothing back
    // the pool is back at poolStart: the three are over the ceiling (5.5)
    // and bank as a pending deposit at the post they came from, which has
    // room for them; nothing is dropped and the ledger still holds
    REQUIRE(poolBefore + 4 == P);
    CHECK(f.of.Pool() == P);
    CHECK(f.of.Dropped() == 0);
    CHECK(f.of.Banked() == 3);
    CHECK(f.Z("Base1").pending == 3);
    CHECK(f.Z("Base1").pendingOrigin == base1);
    check();
    f.of.RemovePendingRefundHook();
    CHECK(GarrisonCache::GetPendingRefundHook() == nullptr);
}

TEST_CASE("OccupierForces - S5: with the key off the refund hook declines and the phase-1 path runs",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 0;"));
    REQUIRE_FALSE(f.of.Active());
    f.of.InstallPendingRefundHook();
    CHECK_FALSE(OccupierForces::RefundPendingToPool(f.registry, 0, -1, 3));
    // a deposit at the Town from FarBase; the Town falls: back to FarBase
    // (room 4), as before the pool existed
    const int town = f.registry.FindZoneIndex("Town");
    const int farBase = f.registry.FindZoneIndex("FarBase");
    f.Z("FarBase").garrison = 8.0f;
    GarrisonCache::Deposit(f.registry, town, 5, farBase);
    f.GiveToResistance("Town");
    CHECK(GarrisonCache::FlushPending(f.registry, town, true, 20000.0f, 20000.0f, nullptr) == PFRefunded);
    CHECK(f.Z("Town").pending == 0);
    CHECK(f.Z("FarBase").garrison == Approx(12.0f));
    CHECK(f.of.Pool() == 0);
    // a local service gives the hook up when it goes away
    {
        OccupierForces local;
        local.InstallPendingRefundHook();
        CHECK(GarrisonCache::GetPendingRefundHook() == &OccupierForces::RefundPendingToPool);
    }
    CHECK(GarrisonCache::GetPendingRefundHook() == nullptr);
    CHECK_FALSE(OccupierForces::RefundPendingToPool(f.registry, 0, -1, 3));
}

// ---------------------------------------------------------------------------
// persistence (11)
// ---------------------------------------------------------------------------

TEST_CASE("OccupierForces - the pool survives save/load", "[game][guerrilla][occupierforces][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "occupier-forces.bin";
    const std::string config = ConfigWith("occupierForces = 1; forcesPoolRefillHours = 2;");
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }

    Fixture f;
    f.Load(config);
    REQUIRE(f.of.Activate(f.registry));
    f.GiveToResistance("FarBase");
    REQUIRE(f.of.Spend(10, "op"));
    f.of.Tick(f.registry, 90.0f); // a fractional refill accumulator
    f.of.NoteLoss(4, "test");
    f.of.LandWave(3);
    f.of.RequestWave();
    const int pool = f.of.Pool();
    const float accum = f.of.RefillAccum();
    const float control = f.of.Control();
    const int band = f.of.Band();
    REQUIRE(accum > 0);
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }

    OccupierForces loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(loaded.HasState());
    CHECK(loaded.Active());
    CHECK(loaded.Pool() == pool);
    CHECK(loaded.PoolStart() == 24);
    CHECK(loaded.RosterStrength() == 18);
    CHECK(loaded.TownCount() == 2);
    CHECK(loaded.TownGarrisons() == 8);
    REQUIRE(loaded.RosterCount() == 2);
    CHECK(Str(loaded.RosterName(1)) == "FarBase");
    CHECK(Str(loaded.TownName(0)) == "Town");
    REQUIRE(loaded.ExcludedCount() == 2);
    CHECK(Str(loaded.ExcludedName(0)) == "HqTown");
    CHECK(loaded.Band() == band);
    CHECK(loaded.Control() == Approx(control));
    CHECK(loaded.RefillAccum() == Approx(accum));
    CHECK(loaded.RefillTotal() == f.of.RefillTotal());
    CHECK(loaded.Lost() == 4);
    CHECK(loaded.WavesLanded() == 1);
    CHECK(loaded.WavesTotal() == 3);
    CHECK(loaded.WaveRequests() == 1);
    CHECK(loaded.Posture() == OFDormant);
    CHECK(loaded.Tokens() == 0);
    CHECK_FALSE(loaded.LegacySave());
    CHECK(loaded.Tuning().refillHours == Approx(2.0f)); // reread from the mission config
    // the reloaded service continues exactly like the original
    f.of.Tick(f.registry, 600.0f);
    loaded.Tick(f.registry, 600.0f);
    CHECK(loaded.Pool() == f.of.Pool());
    CHECK(loaded.Control() == Approx(f.of.Control()));

    // a block loaded with the key now off is kept, but inert
    const std::string off = ConfigWith("occupierForces = 0;");
    {
        QIStream in(off.c_str(), off.size());
        ExtParsMission.Parse(in);
    }
    OccupierForces inert;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(inert.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(inert.Serialize(ar) == LSOK);
    }
    CHECK(inert.HasState());
    CHECK_FALSE(inert.Active());
    CHECK(inert.Pool() == pool);
    CHECK(inert.Draw(5) == 5);
    CHECK(inert.Pool() == pool);

    ExtParsMission.Clear();
    std::filesystem::remove(path);
}

TEST_CASE("OccupierForces - a save without the block derives the pool from control",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    // the old campaign: the far base and one town taken
    f.GiveToResistance("FarBase");
    f.GiveToResistance("Town");
    f.of.DeriveFromExistingSave(f.registry);
    REQUIRE(f.of.Active());
    CHECK(f.of.HasState());
    CHECK(f.of.LegacySave());
    // M7: K by the AUTHORED owners: Town (held now, authored NEUTRAL) still
    // counts with Other; Held (authored RESISTANCE) and the exempt HQ town
    // do not
    CHECK(f.of.TownCount() == 2);
    CHECK(f.of.ExcludedCount() == 2);
    CHECK(f.of.PoolStart() == 24); // max(24, 18 + 2 * 2)
    // M = 6 / 18, T over the current owners = 1 / 2: C = 0.2 + 0.2 = 0.4
    CHECK(f.of.Control() == Approx(0.4f));
    CHECK(f.of.Pool() == 10);      // round(24 * 0.4)
    CHECK(f.of.WavesFired() == 2); // 0.75 and 0.50 crossed
    CHECK(f.of.Tokens() == 0);
    CHECK(f.of.Band() == OFBandStretched);
    // the distinct towns: the ones the resistance took (L12: Held was
    // authored to it and was never taken; the exempt HQ town and Camp never
    // count)
    CHECK(f.of.TownsEverCount() == 1);
    CHECK(f.of.TownEver("Town"));
    CHECK_FALSE(f.of.TownEver("Held"));
    CHECK_FALSE(f.of.TownEver("HqTown"));
    CHECK(f.of.Posture() == OFDormant); // one town taken, wakeTowns 2
    // a second call (or a later first tick) never re-derives
    f.of.SetPool(3);
    f.of.DeriveFromExistingSave(f.registry);
    CHECK(f.of.Pool() == 3);
}

TEST_CASE("OccupierForces - deep losses mark both wave thresholds fired on derivation",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    f.GiveToResistance("FarBase");
    f.GiveToResistance("Post");
    f.of.DeriveFromExistingSave(f.registry);
    REQUIRE(f.of.Active());
    // M = 0, T = 1: C = 0.4
    CHECK(f.of.Control() == Approx(0.4f));
    CHECK(f.of.WavesFired() == 2);
    CHECK(f.of.Pool() == 10); // round(24 * 0.4) = round(9.6)
    CHECK(f.of.Band() == OFBandStretched);
}

// ---------------------------------------------------------------------------
// the director (phase 3)
// ---------------------------------------------------------------------------

namespace
{

// CounterAttack::LaunchFunded stand-in: records the orders, holds a record on
// each target until Resolve, reports a settable number of born operations
struct FakeLauncher final : DirectorLauncher
{
    std::vector<CounterAttackOrder> orders;
    std::vector<int> records; // targets with a record
    int born = 0;
    bool refuse = false;
    std::vector<int> refuseSources; // CounterAttack cannot grant from these
    std::vector<CounterAttackOrder> refused;

    bool Launch(const CounterAttackOrder& order) override
    {
        if (refuse ||
            std::find(refuseSources.begin(), refuseSources.end(), order.source) != refuseSources.end())
        {
            refused.push_back(order);
            return false;
        }
        orders.push_back(order);
        records.push_back(order.target);
        return true;
    }
    int BornOps() const override { return born; }
    bool HasRecord(int zoneIndex) const override
    {
        return std::find(records.begin(), records.end(), zoneIndex) != records.end();
    }
    void Resolve(int zoneIndex)
    {
        records.erase(std::remove(records.begin(), records.end(), zoneIndex), records.end());
    }
};

// the director's view of a fixture: every occupier base spares its stored
// reserve less a quarter of garrisonMax (CounterAttack::Spare)
DirectorView ViewOf(const ZoneRegistry& registry)
{
    DirectorView v;
    const int n = registry.NZones();
    v.alert.Resize(n);
    v.spare.Resize(n);
    CounterAttackTuning t;
    for (int i = 0; i < n; i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        v.alert[i] = 0;
        v.spare[i] = 0;
        if (z && stricmp(z->owner, registry.OccupierSide()) == 0 && stricmp(z->type, "CITY") != 0)
        {
            v.spare[i] = CounterAttack::Spare(t, z->garrison, 0, z->garrisonMax, false);
        }
    }
    return v;
}

// a town flip as the registry makes it (owner, stamp, +40 heat) reported to
// the director as CounterAttack's listener hook would
bool FlipTown(Fixture& f, const char* name, int minute)
{
    ZoneRecord& z = f.Z(name);
    z.owner = f.registry.ResistanceSide();
    z.flipDay = 1;
    z.flipMinute = minute;
    z.heat = z.heat + 40.0f > 100.0f ? 100.0f : z.heat + 40.0f;
    return f.of.NoteTownFlipped(f.registry, f.registry.FindZoneIndex(name));
}

ForcePackageCatalogue TruckCatalogue()
{
    ForcePackageCatalogue cat;
    cat.hulls[FPTruck].className = "Truck5t";
    cat.hulls[FPTruck].maxSpeedKmh = 80;
    cat.hulls[FPTruck].cargo = 12;
    cat.hulls[FPTruck].crew = 1;
    return cat;
}

} // namespace

TEST_CASE("OccupierForces - director knobs default to the knob table", "[game][guerrilla][occupierforces]")
{
    OccupierForcesTuning t;
    CHECK(t.wakeTowns == 2);
    CHECK(t.opsMax == 2);
    CHECK(t.heatFull == Approx(40.0f));
    CHECK(t.urgentHeat == Approx(50.0f));
    CHECK(t.townCooldown == Approx(1800.0f));
    CHECK(t.pushoutQuiet == Approx(1800.0f));
    CHECK(t.retakeMin == 4);
    CHECK(t.lethalityMargin == Approx(1.2f));
    CHECK(t.urgentCostShare == Approx(1.5f));
    CHECK(t.intelDecay == Approx(1800.0f));
    CHECK(OccupierForces::UrgentExitHeat == Approx(35.0f));
    CHECK(OccupierForces::OfferedPackages == ForcePackages::DefaultPackages);

    Fixture f;
    f.Load(ConfigWith("occupierForces = 1; forcesWakeTowns = 1; forcesOpsMax = 9; forcesHeatFull = 30; "
                      "forcesRetakeMin = 6; forcesLethalityMargin = 0.5; forcesIntelDecay = 600;"));
    const OccupierForcesTuning& r = f.of.Tuning();
    CHECK(r.wakeTowns == 2); // repaired into 2..99
    CHECK(r.opsMax == 6);
    CHECK(r.heatFull == Approx(30.0f));
    CHECK(r.retakeMin == 6);
    CHECK(r.lethalityMargin == Approx(1.0f));
    CHECK(r.intelDecay == Approx(600.0f));
}

TEST_CASE("OccupierForces - W: a base and then town 1 stay DORMANT (legacy roll); town 2 wakes",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    FakeLauncher launcher;

    // a base flips first: not a town, nothing counted
    f.GiveToResistance("Post");
    AutoArray<ZoneEventRecord> fired;
    ZoneEventRecord ev;
    ev.type = ZECaptured;
    ev.zoneIndex = f.registry.FindZoneIndex("Post");
    fired.Add(ev);
    f.of.NoteZoneEvents(f.registry, fired);
    CHECK(f.of.Posture() == OFDormant);
    CHECK(f.of.TownsEverCount() == 0);

    // town 1: the director does not take it (CounterAttack rolls Decide)
    CHECK_FALSE(FlipTown(f, "Town", 10));
    CHECK(f.of.Posture() == OFDormant);
    CHECK(f.of.TownsEverCount() == 1);
    CHECK(f.of.Tokens() == 0);
    // the director considers nothing before the wake, however hot the town
    f.Z("Town").heat = 100;
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    CHECK(launcher.orders.empty());
    // a re-flip of town 1 while DORMANT is still legacy
    f.Z("Town").owner = RString("NEUTRAL");
    CHECK_FALSE(FlipTown(f, "Town", 20));
    CHECK(f.of.TownsEverCount() == 1);

    // town 2 (distinct) wakes the director with a full purse and is its own
    CHECK(FlipTown(f, "Other", 30));
    CHECK(f.of.Posture() == OFAwake);
    CHECK(f.of.Tokens() == 2);
    CHECK(f.of.TownsEverCount() == 2);
    CHECK(f.of.TownEver("Other"));
    // AWAKE is latched: every later flip goes to the director
    f.Z("Town").owner = RString("NEUTRAL");
    CHECK(FlipTown(f, "Town", 40));

    // an inactive service never takes a flip
    Fixture off;
    off.Load(ConfigWith("occupierForces = 0;"));
    CHECK_FALSE(FlipTown(off, "Town", 10));
    CHECK_FALSE(FlipTown(off, "Other", 11));
}

TEST_CASE("OccupierForces - a held town is a candidate while heat / 40 >= its flip roll",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    FakeLauncher launcher;
    FlipTown(f, "Town", 125);
    FlipTown(f, "Other", 126);
    const int town = f.registry.FindZoneIndex("Town");
    const float u = OccupierForces::CandidateRoll(f.Z("Town"));
    CHECK(u == Approx(CounterAttack::Roll01(CounterAttack::MakeSeed("Town", 1, 125, 0), 7)));
    CHECK(u >= 0.0f);
    CHECK(u < 1.0f);
    f.Z("Other").heat = 0;
    AutoArray<int> c;
    f.Z("Town").heat = 40.0f * u + 0.5f;
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    REQUIRE(c.Size() == 1);
    CHECK(c[0] == town);
    if (u > 0.05f)
    {
        f.Z("Town").heat = 40.0f * u - 0.5f;
        f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
        CHECK(c.Size() == 0);
    }
    // a fresh flip (40) is always a candidate
    f.Z("Town").heat = 40;
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    CHECK(c.Size() == 1);
    // never the HQ's town, never on a record, never the exempt start town
    DirectorView hq = ViewOf(f.registry);
    hq.hqZone = town;
    f.of.Candidates(f.registry, hq, launcher, c);
    CHECK(c.Size() == 0);
    launcher.records.push_back(town);
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    CHECK(c.Size() == 0);
    launcher.Resolve(town);
    f.Z("HqTown").owner = f.registry.ResistanceSide();
    f.Z("HqTown").heat = 100;
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    REQUIRE(c.Size() == 1);
    CHECK(c[0] == town);
    // hottest first
    f.Z("Other").heat = 90;
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    REQUIRE(c.Size() == 2);
    CHECK(c[0] == f.registry.FindZoneIndex("Other"));
    CHECK(c[1] == town);
    // a hot resistance base is a candidate while STRONG (D16, phase 4), by
    // heat with the towns; never while the band is lower
    f.Z("RebelPost").heat = 100;
    REQUIRE(f.of.Band() == OFBandStrong);
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    REQUIRE(c.Size() == 3);
    CHECK(c[0] == f.registry.FindZoneIndex("RebelPost"));
    const DirectorAction a = f.of.PlanFor(f.registry, ViewOf(f.registry), c[0], CAODirector);
    CHECK(a.kind == CAKBase);
    f.of.SetPool(12);
    REQUIRE(f.of.Band() == OFBandStretched);
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    CHECK(c.Size() == 2);
}

TEST_CASE(
    "OccupierForces - urgency: a fresh quiet flip is not urgent; heat 50 or an alert edge latches it until heat < 35",
    "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    FakeLauncher launcher;
    launcher.born = 99; // no launches: this test watches the latches only
    FlipTown(f, "Town", 1);
    FlipTown(f, "Other", 2);
    const DirectorView view = ViewOf(f.registry);
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK_FALSE(f.of.Urgent("Town")); // a fresh quiet flip (40) whatever the tick
    f.Z("Town").heat = 50;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK(f.of.Urgent("Town"));
    f.Z("Town").heat = 36;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK(f.of.Urgent("Town")); // hysteresis
    f.Z("Town").heat = 34.9f;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK_FALSE(f.of.Urgent("Town"));

    // an alert edge on the held town latches it at heat 40
    f.Z("Town").heat = 40;
    AutoArray<int> states;
    states.Resize(f.registry.NZones());
    for (int i = 0; i < states.Size(); i++)
    {
        states[i] = 0;
    }
    f.of.NoteAlertStates(f.registry, states); // the baseline: no edge
    CHECK_FALSE(f.of.Urgent("Town"));
    states[f.registry.FindZoneIndex("Town")] = 1; // YELLOW
    f.of.NoteAlertStates(f.registry, states);
    CHECK(f.of.Urgent("Town"));
    CHECK_FALSE(f.of.RedPaid("Town"));
    states[f.registry.FindZoneIndex("Town")] = 2; // RED
    f.of.NoteAlertStates(f.registry, states);
    CHECK(f.of.RedPaid("Town"));
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK(f.of.Urgent("Town"));
    f.Z("Town").heat = 30;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK_FALSE(f.of.Urgent("Town"));
    // an edge at an occupier zone latches nothing
    states[f.registry.FindZoneIndex("Post")] = 2;
    f.of.NoteAlertStates(f.registry, states);
    CHECK_FALSE(f.of.Urgent("Post"));
    // a re-flip forgets the old latch and payment
    f.Z("Town").heat = 60;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK(f.of.Urgent("Town"));
    FlipTown(f, "Town", 3);
    CHECK_FALSE(f.of.Urgent("Town"));
    CHECK_FALSE(f.of.RedPaid("Town"));
}

TEST_CASE("OccupierForces - tokens, one launch per tick, the born-operation slots and the order",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry)); // poolStart 24
    f.of.SetCatalogue(TruckCatalogue());
    FakeLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    f.Z("Town").heat = 60; // hottest
    const DirectorView view = ViewOf(f.registry);
    const int pool0 = f.of.Pool();

    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1); // one per tick
    const CounterAttackOrder o = launcher.orders[0];
    CHECK(o.opId == 1);
    CHECK(o.kind == CAKRetake);
    CHECK(o.origin == CAODirector);
    CHECK(o.target == f.registry.FindZoneIndex("Town"));
    CHECK(o.source == f.registry.FindZoneIndex("Post")); // the nearest base with the minimum spare
    CHECK(o.funded);
    CHECK(o.package == FPFoot); // no sightings: L_req 0, foot 4 is the cheapest
    CHECK(o.bodies == 4);
    CHECK(o.crew == 0);
    CHECK(o.priceUnits == 0);
    CHECK(o.abstractSpeed == Approx(1.5f));
    CHECK(o.urgent); // heat 60 >= 50
    CHECK(f.of.Tokens() == 1);
    CHECK(f.of.OpCounter() == 1);
    CHECK(f.of.Pool() == pool0); // foot infantry is paid by the base, not the pool
    REQUIRE(f.of.FindOp(1) >= 0);

    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 2);
    CHECK(launcher.orders[1].target == f.registry.FindZoneIndex("Other"));
    CHECK(launcher.orders[1].opId == 2);
    CHECK(f.of.Tokens() == 0);

    // no token: nothing until the STRONG interval (600 s since 2026-10-15)
    // refills one
    launcher.Resolve(o.target);
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK(launcher.orders.size() == 2);
    f.of.Direct(f.registry, view, launcher, 600.0f);
    CHECK(launcher.orders.size() == 3);

    // born operations at the band's cap hold every launch (unborn ones do
    // not count): STRONG allows forcesOpsMaxStrong (3) since 2026-10-15
    REQUIRE(f.of.Band() == OFBandStrong);
    REQUIRE(f.of.OpsMax() == 3);
    launcher.Resolve(f.registry.FindZoneIndex("Town"));
    f.of.Direct(f.registry, view, launcher, 1800.0f);
    const size_t before = launcher.orders.size();
    launcher.born = 3;
    launcher.Resolve(f.registry.FindZoneIndex("Town"));
    launcher.Resolve(f.registry.FindZoneIndex("Other"));
    f.of.Direct(f.registry, view, launcher, 1800.0f);
    CHECK(launcher.orders.size() == before);
    CHECK(f.of.Tokens() >= 1);
    launcher.born = 2;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK(launcher.orders.size() == before + 1);
}

TEST_CASE("OccupierForces - a motorized order is paid by the pool first and refunded when refused",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    FakeLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    f.Z("Other").heat = 0;
    f.Z("Town").heat = 60; // urgent
    // three fighters reported in Town: L_req 10.8.  FarBase spares 9 (12 -
    // 3): motorized 9 (11.25) for 13 units; foot 9 (9.0) falls short
    AutoArray<SightingReport> reports;
    for (int i = 0; i < 3; i++)
    {
        SightingReport r;
        r.zone = f.registry.FindZoneIndex("Town");
        r.id = 100 + i;
        r.pos = f.Z("Town").pos;
        reports.Add(r);
    }
    f.of.IngestSightings(f.registry, reports);
    REQUIRE(f.of.KnownFighters(f.registry, f.registry.FindZoneIndex("Town")) == 3);
    DirectorView view = ViewOf(f.registry);
    view.spare[f.registry.FindZoneIndex("Post")] = 0; // the near post cannot spare the minimum
    const int pool0 = f.of.Pool();

    launcher.refuse = true;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK(launcher.orders.empty());
    CHECK(f.of.Pool() == pool0); // spent and refunded
    CHECK(f.of.Tokens() == 2);   // the token is kept
    CHECK(f.of.OpCounter() == 1); // the refused op id stays spent (L1)
    CHECK(f.of.OpRows() == 0);

    launcher.refuse = false;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1);
    const CounterAttackOrder o = launcher.orders[0];
    CHECK(o.package == FPTruck);
    CHECK(o.source == f.registry.FindZoneIndex("FarBase"));
    CHECK(o.bodies == 9);
    CHECK(o.crew == 1);
    CHECK(o.priceUnits == 4);
    CHECK(std::string((const char*)o.hullClass) == "Truck5t");
    CHECK(o.abstractSpeed == Approx(0.6f * 80.0f / 3.6f));
    CHECK(f.of.Pool() == pool0 - 4);
    const int row = f.of.FindOp(o.opId);
    REQUIRE(row >= 0);
    CHECK(f.of.OpOutstanding(row) == 4);

    // the hull comes home intact: its price (3) back to the pool
    CounterAttackOpEvent ev;
    ev.opId = o.opId;
    ev.target = o.target;
    ev.type = CAOpSettled;
    f.of.NoteOpEvent(f.registry, ev);
    ev.type = CAOpHullRefund;
    ev.units = 3;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.of.Pool() == pool0 - 1);
    CHECK(f.of.OpOutstanding(f.of.FindOp(o.opId)) == 1);
    // the crewman comes home as a body (CounterAttack put him in a reserve):
    // the row lets go of him, the pool is credited nothing, the row reads 0
    ev.type = CAOpCrewSettled;
    ev.units = 1;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.of.Pool() == pool0 - 1);
    CHECK(f.of.OpOutstanding(f.of.FindOp(o.opId)) == 0);
    // a second refund never pays more than the op still holds
    ev.type = CAOpHullRefund;
    ev.units = 3;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.of.Pool() == pool0 - 1);
}

TEST_CASE("OccupierForces - the air re-plan's airframe goes on the operation's ledger row (3.1 step 6, 5.5)",
          "[game][guerrilla][occupierforces][air]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    FakeLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    f.Z("Other").heat = 0;
    f.Z("Town").heat = 60; // urgent
    AutoArray<SightingReport> reports;
    for (int i = 0; i < 3; i++)
    {
        SightingReport r;
        r.zone = f.registry.FindZoneIndex("Town");
        r.id = 100 + i;
        r.pos = f.Z("Town").pos;
        reports.Add(r);
    }
    f.of.IngestSightings(f.registry, reports);
    DirectorView view = ViewOf(f.registry);
    view.spare[f.registry.FindZoneIndex("Post")] = 0;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1);
    const CounterAttackOrder o = launcher.orders[0];
    REQUIRE(o.package == FPTruck);
    auto held = [&]() { return f.of.Pool() + f.of.OpOutstanding(f.of.FindOp(o.opId)); };
    const int held0 = held();

    // another listener hears CounterAttack's events: nothing is booked on the
    // row (it would never be settled), the caller settles directly
    bool booked = true;
    REQUIRE(f.of.SpendOnOp(o.opId, 12, FPTransportHeli, "air re-plan", booked));
    CHECK_FALSE(booked);
    CHECK(f.of.OpOutstanding(f.of.FindOp(o.opId)) == 4);
    f.of.Refund(12, "test: settled directly");
    CHECK(held() == held0);

    // the director's own ledger hears them: the row holds the airframe
    struct Ear
    {
        explicit Ear(CounterAttackListener* l) { CounterAttack::SetListener(l); }
        ~Ear() { CounterAttack::SetListener(nullptr); }
    } ear(&f.of);
    REQUIRE(f.of.SpendOnOp(o.opId, 12, FPTransportHeli, "air re-plan", booked));
    CHECK(booked);
    const int row = f.of.FindOp(o.opId);
    CHECK(f.of.OpPackage(row) == FPTransportHeli);
    CHECK(f.of.OpOutstanding(row) == 16);
    CHECK(held() == held0);
    // the ground hull goes back as CounterAttack names it
    CounterAttackOpEvent ev;
    ev.opId = o.opId;
    ev.target = o.target;
    ev.type = CAOpHullRefund;
    ev.units = 4;
    ev.note = "air re-plan: ground hull not used";
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.of.OpOutstanding(f.of.FindOp(o.opId)) == 12);
    CHECK(f.of.OpPackage(f.of.FindOp(o.opId)) == FPTransportHeli);
    CHECK(held() == held0);
    // shot down: the airframe's 10 from the row to the losses
    const int lost0 = f.of.Lost();
    ev.type = CAOpHullLost;
    ev.units = 10;
    ev.note = "airframe lost";
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(held() == held0 - 10);
    CHECK(f.of.Lost() == lost0 + 10);
    // refused: nothing debited, nothing booked
    f.of.SetPool(5);
    CHECK_FALSE(f.of.SpendOnOp(o.opId, 12, FPTransportHeli, "air re-plan", booked));
    CHECK_FALSE(booked);
    CHECK(f.of.Pool() == 5);
}

TEST_CASE("OccupierForces - repels and unborn cancels rest the target; a repel adds heat unless RED paid it",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    // this case pins the plain rest (forcesTownCooldown); the jitter and the
    // repel ladder have their own cases
    f.of.TuningMutable().restJitter = 0;
    FakeLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    const int town = f.registry.FindZoneIndex("Town");
    const int other = f.registry.FindZoneIndex("Other");

    // M6: a legacy record's repel or unborn cancel (op 0) drives nothing:
    // no heat, no rest, no pool change (invariant W)
    f.Z("Town").heat = 20;
    const int poolLegacy = f.of.Pool();
    CounterAttackOpEvent ev;
    ev.opId = 0;
    ev.type = CAOpRepelled;
    ev.target = town;
    ev.units = 3;
    ev.bodies = 2;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.Z("Town").heat == Approx(20.0f));
    CHECK(f.of.Cooldown("Town") == Approx(0.0f));
    ev.type = CAOpCancelledUnseen;
    ev.units = 4;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.of.Cooldown("Town") == Approx(0.0f));
    CHECK(f.of.Pool() == poolLegacy);

    // a director operation on Town (the hottest), repelled with no RED
    // edge: +15 heat and a 30 min rest
    f.Z("Town").heat = 60;
    f.Z("Other").heat = 0;
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1);
    REQUIRE(launcher.orders[0].target == town);
    f.Z("Town").heat = 20;
    ev = CounterAttackOpEvent();
    ev.opId = launcher.orders[0].opId;
    ev.type = CAOpRepelled;
    ev.target = town;
    ev.units = 3;
    ev.bodies = 2;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.Z("Town").heat == Approx(35.0f));
    CHECK(f.of.Cooldown("Town") == Approx(1800.0f));
    f.Z("Town").heat = 90;
    AutoArray<int> c;
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    bool townListed = false;
    for (int i = 0; i < c.Size(); i++)
    {
        townListed = townListed || c[i] == town;
    }
    CHECK_FALSE(townListed);
    // the rest runs out
    FakeLauncher idle;
    idle.born = 99;
    f.of.Direct(f.registry, ViewOf(f.registry), idle, 1700.0f);
    CHECK(f.of.Cooldown("Town") == Approx(100.0f));
    f.of.Direct(f.registry, ViewOf(f.registry), idle, 120.0f);
    CHECK(f.of.Cooldown("Town") == Approx(0.0f));

    // a director operation on Other; a RED edge during it already paid the
    // heat
    f.Z("Town").heat = 0;
    f.Z("Other").heat = 60;
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 2);
    REQUIRE(launcher.orders[1].target == other);
    ev.opId = launcher.orders[1].opId;
    AutoArray<int> states;
    states.Resize(f.registry.NZones());
    for (int i = 0; i < states.Size(); i++)
    {
        states[i] = 0;
    }
    f.of.NoteAlertStates(f.registry, states);
    states[other] = 2;
    f.of.NoteAlertStates(f.registry, states);
    REQUIRE(f.of.RedPaid("Other"));
    f.Z("Other").heat = 20;
    ev.target = other;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.Z("Other").heat == Approx(20.0f));
    CHECK(f.of.Cooldown("Other") == Approx(1800.0f));
    CHECK_FALSE(f.of.RedPaid("Other")); // consumed

    // a cancelled unborn director op: the units CounterAttack still owes
    // (the full hull and crew price here) back, and the rest.  Town's repel
    // above is forgotten here: the ladder would escalate this plan
    f.of.ForceRepels("Town", 0);
    launcher.Resolve(town);
    f.Z("Town").heat = 60;
    f.Z("Other").heat = 0;
    AutoArray<SightingReport> reports;
    for (int i = 0; i < 3; i++)
    {
        SightingReport r;
        r.zone = town;
        r.id = 7 + i;
        reports.Add(r);
    }
    f.of.IngestSightings(f.registry, reports);
    DirectorView view = ViewOf(f.registry);
    view.spare[f.registry.FindZoneIndex("Post")] = 0;
    const int pool0 = f.of.Pool();
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 3);
    REQUIRE(launcher.orders[2].target == town);
    REQUIRE(launcher.orders[2].priceUnits == 4);
    CHECK(f.of.Pool() == pool0 - 4);
    ev = CounterAttackOpEvent();
    ev.opId = launcher.orders[2].opId;
    ev.type = CAOpCancelledUnseen;
    ev.target = town;
    ev.units = 4;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.of.Pool() == pool0);
    CHECK(f.of.Cooldown("Town") == Approx(1800.0f));
}

namespace
{

// the motorized setup above: AWAKE with two tokens, Town hot and urgent with
// three fighters reported, the near Post unable to spare the minimum, so a
// truck from FarBase (9 infantry, 4 pool units) is the plan
DirectorView MotorizedSetup(Fixture& f)
{
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    f.Z("Other").heat = 0;
    f.Z("Town").heat = 60;
    AutoArray<SightingReport> reports;
    for (int i = 0; i < 3; i++)
    {
        SightingReport r;
        r.zone = f.registry.FindZoneIndex("Town");
        r.id = 100 + i;
        r.pos = f.Z("Town").pos;
        reports.Add(r);
    }
    f.of.IngestSightings(f.registry, reports);
    DirectorView view = ViewOf(f.registry);
    view.spare[f.registry.FindZoneIndex("Post")] = 0;
    return view;
}

} // namespace

TEST_CASE("OccupierForces - a source CounterAttack refuses is skipped: the target is re-planned from the next base",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    FakeLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    f.Z("Other").heat = 0;
    f.Z("Town").heat = 60;
    const int town = f.registry.FindZoneIndex("Town");
    const int post = f.registry.FindZoneIndex("Post");
    const int farBase = f.registry.FindZoneIndex("FarBase");
    const DirectorView view = ViewOf(f.registry);
    REQUIRE(view.spare[post] >= 4);
    REQUIRE(view.spare[farBase] >= 4);

    // CounterAttack cannot grant from the near post: FarBase goes instead,
    // in the same tick, and only one operation launches
    launcher.refuseSources.push_back(post);
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.refused.size() == 1);
    CHECK(launcher.refused[0].source == post);
    CHECK(launcher.refused[0].opId == 1);
    REQUIRE(launcher.orders.size() == 1);
    CHECK(launcher.orders[0].source == farBase);
    CHECK(launcher.orders[0].target == town);
    CHECK(launcher.orders[0].opId == 2); // the refused id stays spent (L1)
    CHECK(f.of.OpCounter() == 2);
    CHECK(f.of.OpRows() == 1);
    CHECK(f.of.FindOp(1) < 0);
    CHECK(f.of.FindOp(2) >= 0);
    CHECK(f.of.Tokens() == 1);

    // every source refused: nothing launches, the token is kept, and the
    // re-planning is bounded
    launcher.Resolve(town);
    launcher.refused.clear();
    launcher.refuseSources.push_back(farBase);
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK(launcher.orders.size() == 1);
    CHECK(launcher.refused.size() >= 2);
    CHECK(launcher.refused.size() <= (size_t)(2 * (OccupierForces::MaxSourceRetries + 1)));
    CHECK(f.of.Tokens() == 1);
    CHECK(f.of.OpRows() == 1);
}

TEST_CASE("OccupierForces - a refused launch leaves the band where it was (L1)", "[game][guerrilla][occupierforces]")
{
    Fixture f;
    const DirectorView view = MotorizedSetup(f);
    FakeLauncher launcher;
    // 15 of 24 (62 %): STRONG holds above 60 %; the truck's 4 units would
    // drop it to 46 % (STRETCHED), and the refund alone (62 %) cannot climb
    // back over 66 %
    f.of.SetPool(15);
    REQUIRE(f.of.Band() == OFBandStrong);
    launcher.refuse = true;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.refused.size() >= 1);
    CHECK(launcher.refused[0].priceUnits == 4);
    CHECK(launcher.orders.empty());
    CHECK(f.of.Pool() == 15);
    CHECK(f.of.Band() == OFBandStrong);
    CHECK(f.of.Tokens() == 2);
    CHECK(f.of.OpRows() == 0);
    CHECK(f.of.OpCounter() == (int)launcher.refused.size()); // never rolled back
}

TEST_CASE("OccupierForces - an unborn cancel refunds only what CounterAttack still owes (L3)",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    const DirectorView view = MotorizedSetup(f);
    f.of.TuningMutable().restJitter = 0; // the plain rest (the jitter has its own case)
    FakeLauncher launcher;
    const int pool0 = f.of.Pool();
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1);
    const CounterAttackOrder o = launcher.orders[0];
    REQUIRE(o.priceUnits == 4);
    CHECK(f.of.Pool() == pool0 - 4);
    const int row = f.of.FindOp(o.opId);
    REQUIRE(row >= 0);
    // the crew (1) were already booked lost: CounterAttack owes the hull's 3
    CounterAttackOpEvent ev;
    ev.opId = o.opId;
    ev.type = CAOpCancelledUnseen;
    ev.target = o.target;
    ev.units = 3;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.of.Pool() == pool0 - 1);
    CHECK(f.of.OpOutstanding(f.of.FindOp(o.opId)) == 0);
    CHECK(f.of.Cooldown("Town") == Approx(1800.0f));
    // a claim above the row never pays more than the row held
    Fixture g;
    const DirectorView gview = MotorizedSetup(g);
    FakeLauncher glauncher;
    const int gpool0 = g.of.Pool();
    g.of.Direct(g.registry, gview, glauncher, 60.0f);
    REQUIRE(glauncher.orders.size() == 1);
    ev.opId = glauncher.orders[0].opId;
    ev.units = 10;
    g.of.NoteOpEvent(g.registry, ev);
    CHECK(g.of.Pool() == gpool0);
}

TEST_CASE("OccupierForces - a hull package CounterAttack sends on foot at launch shows foot in the ledger",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    const DirectorView view = MotorizedSetup(f);
    FakeLauncher launcher;
    const int pool0 = f.of.Pool();
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1);
    const CounterAttackOrder o = launcher.orders[0];
    REQUIRE(o.package == FPTruck);
    REQUIRE(f.of.OpPackage(f.of.FindOp(o.opId)) == FPTruck);
    // another hull refund (a hull home) never changes the package
    CounterAttackOpEvent ev;
    ev.opId = o.opId;
    ev.target = o.target;
    ev.type = CAOpHullRefund;
    ev.units = 0;
    ev.note = "abstract: home";
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.of.OpPackage(f.of.FindOp(o.opId)) == FPTruck);
    // CounterAttack's launch-time downgrade (LaunchCore's note)
    ev.units = 4;
    ev.note = "no stored spare: on foot";
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.of.OpPackage(f.of.FindOp(o.opId)) == FPFoot);
    CHECK(f.of.Pool() == pool0);
}

namespace
{

// the anti-1:1 hour: four flips 15 min apart; an operation is over 10 min
// after it left.  Returns the director's orders; legacy counts the flips
// CounterAttack answers with Decide.
std::vector<CounterAttackOrder> FourFlipsHour(const std::string& config, int& legacy, int& awakeMinute)
{
    Fixture f;
    f.Load(config, false);
    REQUIRE(f.of.Activate(f.registry));
    FakeLauncher launcher;
    const char* towns[4] = {"T0", "T1", "T2", "T3"};
    legacy = 0;
    awakeMinute = -1;
    std::vector<int> launchedAt;
    for (int minute = 0; minute <= 60; minute++)
    {
        if (minute % 15 == 0 && minute / 15 < 4)
        {
            if (!FlipTown(f, towns[minute / 15], minute))
            {
                legacy++; // CounterAttack answers this one with Decide
            }
        }
        // the held-zone heat decay: 0.25 per 10 s
        for (int i = 0; i < f.registry.NZones(); i++)
        {
            ZoneRecord* z = f.registry.GetZoneMutable(i);
            if (stricmp(z->owner, f.registry.ResistanceSide()) == 0)
            {
                f.registry.HeatDecay(i, 1.5f);
            }
        }
        // an operation is over 10 minutes after it left
        for (size_t k = 0; k < launchedAt.size(); k++)
        {
            if (launchedAt[k] == minute - 10)
            {
                launcher.Resolve(launcher.orders[k].target);
            }
        }
        if (awakeMinute < 0 && f.of.Posture() == OFAwake)
        {
            awakeMinute = minute;
        }
        const size_t before = launcher.orders.size();
        f.of.Tick(f.registry, 60.0f);
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        for (size_t k = before; k < launcher.orders.size(); k++)
        {
            launchedAt.push_back(minute);
        }
    }
    CHECK(f.of.Posture() == OFAwake);
    return launcher.orders;
}

std::string WithKeys(const std::string& config, const char* keys)
{
    std::string cfg = config;
    const std::string anchor = "class CfgGuerrillaZones\n{\n";
    cfg.insert(anchor.size(), std::string("    ") + keys + "\n");
    return cfg;
}

} // namespace

TEST_CASE("OccupierForces - anti-1:1: four flips 15 min apart give fewer than 4 operations",
          "[game][guerrilla][occupierforces]")
{
    // the phase 3 cadence (the legacy token intervals, the old STRONG cap)
    // keeps the property: about one answer per two flips
    int legacy = 0;
    int awake = -1;
    const std::vector<CounterAttackOrder> old = FourFlipsHour(
        WithKeys(AbelLikeConfig(), "forcesTokenInterval[] = {1800, 2700, 3600, 0}; forcesOpsMaxStrong = 0;"), legacy,
        awake);
    CHECK(legacy == 1);
    CHECK(old.size() >= 1);
    CHECK(old.size() < 4);
}

TEST_CASE("OccupierForces - the playable rate: a STRONG director spends a token every 10 minutes",
          "[game][guerrilla][occupierforces]")
{
    // the same hour at the 2026-10-15 defaults: the pool, not the flip count,
    // decides (the director is not 1:1 with flips: it spends its budget on
    // the hottest town, then pushes out on quiet ones).  Two tokens at the
    // wake, then one per 600 s while STRONG: at most 2 + awake minutes / 10.
    int legacy = 0;
    int awake = -1;
    const std::vector<CounterAttackOrder> now = FourFlipsHour(AbelLikeConfig(), legacy, awake);
    CHECK(legacy == 1);
    REQUIRE(awake >= 0);
    const int bound = 2 + (60 - awake) / 10;
    CHECK((int)now.size() >= 4);
    CHECK((int)now.size() <= bound);
    for (const CounterAttackOrder& o : now)
    {
        CHECK((o.origin == CAODirector || o.origin == CAOPushout));
    }
}

TEST_CASE("OccupierForces - push-out only when STRONG, on the quiet town or base nearest a base",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    FakeLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    // nothing hot any more
    f.Z("Town").heat = 0;
    f.Z("Other").heat = 0;
    f.Z("Held").heat = 0;
    // a fighter was seen in Town (nearest the Post) a minute ago: not quiet
    AutoArray<SightingReport> reports;
    SightingReport r;
    r.zone = f.registry.FindZoneIndex("Town");
    r.id = 1;
    reports.Add(r);
    f.of.IngestSightings(f.registry, reports);

    // STRETCHED: no push-out
    f.of.SetPool(12);
    REQUIRE(f.of.Band() == OFBandStretched);
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    CHECK(launcher.orders.empty());

    // STRONG: the quiet held zone nearest an occupier base.  Since phase 4
    // (6.4, N2) bases are in push-out too: RebelPost (566 m from the Post)
    // beats Held (1.5 km); Town is nearer still but not quiet
    f.of.SetPool(24);
    REQUIRE(f.of.Band() == OFBandStrong);
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1);
    CHECK(launcher.orders[0].origin == CAOPushout);
    CHECK(launcher.orders[0].target == f.registry.FindZoneIndex("RebelPost"));
    CHECK(launcher.orders[0].kind == CAKBase);
    CHECK(launcher.orders[0].package == FPFoot); // the minimal package
    CHECK_FALSE(launcher.orders[0].urgent);
    CHECK(launcher.orders[0].bodies == 4);

    // half an hour later Town is quiet too and nearest: the next push-out
    f.of.Tick(f.registry, 1800.0f);
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 1800.0f);
    REQUIRE(launcher.orders.size() == 2);
    CHECK(launcher.orders[1].target == f.registry.FindZoneIndex("Town"));
    CHECK(launcher.orders[1].origin == CAOPushout);
}

TEST_CASE("OccupierForces - intel: the choice ignores the true census; a dead group's report stays, then decays",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    const int town = f.registry.FindZoneIndex("Town");
    f.Z("Town").heat = 60;

    // two groups report three distinct fighters (one seen by both) in Town
    // and one outside every zone
    AutoArray<SightingReport> reports;
    const unsigned long long ids[4] = {11, 12, 13, 11};
    for (int i = 0; i < 4; i++)
    {
        SightingReport r;
        r.zone = town;
        r.id = ids[i];
        r.pos = Vector3(1000.0f + (float)i, 0.0f, 1300.0f);
        reports.Add(r);
    }
    SightingReport stray;
    stray.zone = -1;
    stray.id = 20;
    stray.pos = Vector3(7000.0f, 0.0f, 7000.0f);
    reports.Add(stray);
    f.of.IngestSightings(f.registry, reports);
    CHECK(f.of.KnownFighters(f.registry, town) == 3);
    REQUIRE(f.of.SightingRows() == 2);
    CHECK(OccupierForces::ZoneOfPosition(f.registry, Vector3(1010.0f, 0.0f, 1310.0f)) == town);
    CHECK(OccupierForces::ZoneOfPosition(f.registry, Vector3(7000.0f, 0.0f, 7000.0f)) == -1);

    DirectorView view = ViewOf(f.registry);
    const DirectorAction a = f.of.PlanFor(f.registry, view, town, CAODirector);
    REQUIRE(a.go);
    CHECK(a.fighters == Approx(3.0f));
    CHECK(a.choice.lReq == Approx(10.8f));

    // the TRUE census of the town changes (ten fighters, the player among
    // them) while the sightings do not: the choice is the same
    f.Z("Town").lastGuer = 10;
    f.Z("Town").lastGuerHold = 10;
    f.Z("Town").lastOcc = 0;
    const DirectorAction b = f.of.PlanFor(f.registry, view, town, CAODirector);
    REQUIRE(b.go);
    CHECK(b.choice.quote.package == a.choice.quote.package);
    CHECK(b.choice.quote.bodies == a.choice.quote.bodies);
    CHECK(b.choice.quote.source == a.choice.quote.source);
    CHECK(b.choice.lReq == Approx(a.choice.lReq));
    CHECK(b.fighters == Approx(a.fighters));
    AutoArray<int> c1;
    FakeLauncher launcher;
    f.of.Candidates(f.registry, view, launcher, c1);
    f.Z("Town").lastGuer = 0;
    f.Z("Town").lastGuerHold = 0;
    AutoArray<int> c2;
    f.of.Candidates(f.registry, view, launcher, c2);
    REQUIRE(c1.Size() == c2.Size());
    for (int i = 0; i < c1.Size(); i++)
    {
        CHECK(c1[i] == c2[i]);
    }

    // the reporting groups die: the next scans report nothing, the last
    // report stays ("it radioed before it died") ...
    AutoArray<SightingReport> none;
    f.of.Tick(f.registry, 600.0f);
    f.of.IngestSightings(f.registry, none);
    CHECK(f.of.KnownFighters(f.registry, town) == 3);
    CHECK(f.of.SightingAge(0) == Approx(600.0f));
    // ... a newer scan replaces it with the latest count
    AutoArray<SightingReport> one;
    one.Add(reports[0]);
    f.of.IngestSightings(f.registry, one);
    CHECK(f.of.KnownFighters(f.registry, town) == 1);
    // ... and it decays after forcesIntelDecay (1800 s)
    f.of.Tick(f.registry, 1700.0f);
    CHECK(f.of.KnownFighters(f.registry, town) == 1);
    f.of.Tick(f.registry, 200.0f);
    CHECK(f.of.KnownFighters(f.registry, town) == 0);
    FakeLauncher idle;
    idle.born = 99;
    f.of.Direct(f.registry, view, idle, 0.0f);
    CHECK(f.of.SightingRows() == 0);
}

TEST_CASE("OccupierForces - nothing launches once the campaign is won", "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    FakeLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    DirectorView won = ViewOf(f.registry);
    won.won = true;
    f.of.Direct(f.registry, won, launcher, 60.0f);
    CHECK(launcher.orders.empty());
    CHECK(f.of.Tokens() == 2);
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    CHECK(launcher.orders.size() == 1);
}

TEST_CASE("OccupierForces - the director survives save/load", "[game][guerrilla][occupierforces][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "occupier-forces-director.bin";
    const std::string config = ConfigWith("occupierForces = 1;");
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    f.Load(config);
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    FakeLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    f.Z("Town").heat = 60;
    f.Z("Other").heat = 0;
    AutoArray<SightingReport> reports;
    for (int i = 0; i < 3; i++)
    {
        SightingReport r;
        r.zone = f.registry.FindZoneIndex("Town");
        r.id = 50 + i;
        r.pos = Vector3(1000.0f, 5.0f, 1300.0f + (float)i);
        reports.Add(r);
    }
    f.of.IngestSightings(f.registry, reports);
    DirectorView view = ViewOf(f.registry);
    view.spare[f.registry.FindZoneIndex("Post")] = 0;
    f.of.Tick(f.registry, 60.0f);
    f.of.Direct(f.registry, view, launcher, 60.0f); // a motorized op on Town
    REQUIRE(launcher.orders.size() == 1);
    // a rest elsewhere (a legacy repel no longer rests a zone, M6: the test
    // lever stands in); a RED edge: a payment
    f.of.ForceCooldown("Other", 1800.0f);
    AutoArray<int> states;
    states.Resize(f.registry.NZones());
    for (int i = 0; i < states.Size(); i++)
    {
        states[i] = 0;
    }
    f.of.NoteAlertStates(f.registry, states);
    states[f.registry.FindZoneIndex("Town")] = 2;
    f.of.NoteAlertStates(f.registry, states);
    f.of.Tick(f.registry, 90.0f);
    launcher.born = 3;                              // the STRONG slots (3) are full: no push-out now
    f.of.Direct(f.registry, view, launcher, 90.0f); // the token clock runs
    const int tokens = f.of.Tokens();
    const float tokenClock = f.of.TokenClock();
    const float cooldown = f.of.Cooldown("Other");
    const float now = f.of.Now();
    REQUIRE(tokenClock > 0);
    REQUIRE(cooldown > 0);
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    OccupierForces loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(loaded.Posture() == OFAwake);
    CHECK(loaded.TownsEverCount() == 2);
    CHECK(loaded.TownEver("Town"));
    CHECK(loaded.Tokens() == tokens);
    CHECK(loaded.TokenClock() == Approx(tokenClock));
    CHECK(loaded.Urgent("Town"));
    CHECK(loaded.RedPaid("Town"));
    CHECK(loaded.Cooldown("Other") == Approx(cooldown));
    CHECK(loaded.OpCounter() == 1);
    CHECK(loaded.Now() == Approx(now));
    REQUIRE(loaded.SightingRows() == 1);
    CHECK(std::string((const char*)loaded.SightingZone(0)) == "Town");
    CHECK(loaded.SightingCount(0) == 3);
    REQUIRE(loaded.SightingPositions(0));
    REQUIRE(loaded.SightingPositions(0)->Size() == 3);
    CHECK((*loaded.SightingPositions(0))[2].Z() == Approx(1302.0f));
    CHECK(loaded.KnownFighters(f.registry, f.registry.FindZoneIndex("Town")) == 3);
    const int row = loaded.FindOp(1);
    REQUIRE(row >= 0);
    CHECK(loaded.OpOutstanding(row) == 4);
    // the reloaded director refunds the hull exactly like the original
    loaded.SetCatalogue(TruckCatalogue());
    const int pool = loaded.Pool();
    CounterAttackOpEvent refund;
    refund.opId = 1;
    refund.type = CAOpHullRefund;
    refund.units = 3;
    loaded.NoteOpEvent(f.registry, refund);
    CHECK(loaded.Pool() == pool + 3);
    ExtParsMission.Clear();
    std::filesystem::remove(path);
}

#include <math.h> // fabsf (phase-4 cases)

// ===========================================================================
// Phase 4 (OCCUPIER-FORCES.md 6.3, 6.4, 7.1, 7.3, 11): base and HQ
// candidates, the final push, waves (triggers with dwell and merge, size,
// cluster, hub, allocation order, the final-stand surge), the war-level
// split, occupier standing (recovery, the refill mean, informant tips,
// morale) and the wave state across a save.
// ===========================================================================

namespace
{

// a launcher that can report an HQ_ASSAULT in flight
struct HqLauncher final : DirectorLauncher
{
    std::vector<CounterAttackOrder> orders;
    std::vector<int> records;
    bool hq = false;
    bool Launch(const CounterAttackOrder& order) override
    {
        orders.push_back(order);
        if (order.kind == CAKHqAssault)
        {
            hq = true;
        }
        else
        {
            records.push_back(order.target);
        }
        return true;
    }
    int BornOps() const override { return 0; }
    bool HasRecord(int zoneIndex) const override
    {
        return std::find(records.begin(), records.end(), zoneIndex) != records.end();
    }
    bool HasHqRecord() const override { return hq; }
};

bool HasCandidate(const AutoArray<int>& c, int v)
{
    for (int i = 0; i < c.Size(); i++)
    {
        if (c[i] == v)
        {
            return true;
        }
    }
    return false;
}

// the HQ standing at the Camp
DirectorView HqView(Fixture& f)
{
    DirectorView v = ViewOf(f.registry);
    v.hqEstablished = true;
    v.hqZone = f.registry.FindZoneIndex("Camp");
    v.hqPos = f.Z("Camp").pos;
    return v;
}

// one director tick as the engine runs it: Tick (refill, control, the
// roster bases held), then Direct
void DirectorTick(Fixture& f, const DirectorView& v, DirectorLauncher& l, float elapsed)
{
    f.of.Tick(f.registry, elapsed);
    f.of.Direct(f.registry, v, l, elapsed);
}

} // namespace

TEST_CASE("OccupierForces - the HQ is a candidate only while STRONG, in a wave, or as the final push",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    HqLauncher launcher;
    DirectorView v = HqView(f);
    f.Z("Held").heat = 0;
    f.Z("RebelPost").heat = 0;
    AutoArray<int> c;

    // STRONG and the HQ's zone hot: a candidate
    f.Z("Camp").heat = 100;
    REQUIRE(f.of.Band() == OFBandStrong);
    f.of.Candidates(f.registry, v, launcher, c);
    CHECK(HasCandidate(c, OccupierForces::HqCandidate));
    // STRETCHED, not in a wave, the resistance still holds a town and a base
    f.of.SetPool(12);
    REQUIRE(f.of.Band() == OFBandStretched);
    f.of.Candidates(f.registry, v, launcher, c);
    CHECK_FALSE(HasCandidate(c, OccupierForces::HqCandidate));
    // in a wave (inbound) it is a candidate whatever the band
    f.of.RequestWave(f.registry, v, 1);
    REQUIRE(f.of.InWave());
    f.of.Candidates(f.registry, v, launcher, c);
    CHECK(HasCandidate(c, OccupierForces::HqCandidate));

    // a cold HQ zone is never a heat candidate (unless its roll is 0)
    Fixture g;
    g.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(g.of.Activate(g.registry));
    g.of.SetCatalogue(TruckCatalogue());
    DirectorView gv = HqView(g);
    g.Z("Held").heat = 0;
    g.Z("RebelPost").heat = 0;
    g.Z("Camp").heat = 0;
    if (OccupierForces::CandidateRoll(g.Z("Camp")) > 0.0f)
    {
        g.of.Candidates(g.registry, gv, launcher, c);
        CHECK_FALSE(HasCandidate(c, OccupierForces::HqCandidate));
    }
    // the final push: no held base, no held town besides the HQ's zone; the
    // HQ goes first, cold and STRETCHED
    g.Z("Held").owner = RString("NEUTRAL");
    g.Z("RebelPost").owner = RString("EAST");
    CHECK(OccupierForces::FinalPush(g.registry, gv.hqZone));
    g.of.SetPool(12);
    g.of.Candidates(g.registry, gv, launcher, c);
    REQUIRE(c.Size() >= 1);
    CHECK(c[0] == OccupierForces::HqCandidate);
    // never with an assault already on it, never without an HQ
    HqLauncher busy;
    busy.hq = true;
    g.of.Candidates(g.registry, gv, busy, c);
    CHECK_FALSE(HasCandidate(c, OccupierForces::HqCandidate));
    DirectorView none = gv;
    none.hqEstablished = false;
    g.of.Candidates(g.registry, none, launcher, c);
    CHECK_FALSE(HasCandidate(c, OccupierForces::HqCandidate));

    // the plan: always an HQ_ASSAULT of at least 6 on HqPos
    const DirectorAction a = g.of.PlanFor(g.registry, gv, OccupierForces::HqCandidate, CAODirector);
    REQUIRE(a.go);
    CHECK(a.kind == CAKHqAssault);
    CHECK(a.target == gv.hqZone);
    CHECK(a.destPos.X() == Approx(gv.hqPos.X()));
    CHECK(a.destPos.Z() == Approx(gv.hqPos.Z()));
    CHECK(a.choice.quote.bodies >= 6);
    CHECK(a.choice.quote.source == g.registry.FindZoneIndex("FarBase")); // the Post cannot spare 6
    // an HQ outside every zone: target -1, still HqPos
    DirectorView outside = gv;
    outside.hqZone = -1;
    const DirectorAction b = g.of.PlanFor(g.registry, outside, OccupierForces::HqCandidate, CAODirector);
    CHECK(b.go);
    CHECK(b.target == -1);
}

TEST_CASE("OccupierForces - the director launches the final push as an HQ assault", "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    HqLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2)); // AWAKE, two tokens
    // the towns fall back, the base and Held are gone: only the HQ is left
    f.Z("Town").owner = RString("NEUTRAL");
    f.Z("Other").owner = RString("NEUTRAL");
    f.Z("Held").owner = RString("NEUTRAL");
    f.Z("RebelPost").owner = RString("EAST");
    const DirectorView v = HqView(f);
    f.of.Direct(f.registry, v, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1);
    const CounterAttackOrder& o = launcher.orders[0];
    CHECK(o.kind == CAKHqAssault);
    CHECK(o.origin == CAODirector);
    CHECK(o.bodies >= 6);
    CHECK(o.destPos.X() == Approx(v.hqPos.X()));
    CHECK(f.of.Tokens() == 1);
    // a repelled assault rests the HQ, not the Camp zone
    CounterAttackOpEvent ev;
    ev.opId = o.opId;
    ev.type = CAOpRepelled;
    ev.kind = CAKHqAssault;
    ev.target = v.hqZone;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.of.Cooldown(OccupierForces::HqCooldownName) > 0);
    CHECK(f.of.Cooldown("Camp") == Approx(0.0f));
}

TEST_CASE("OccupierForces - wave size, cluster score and the hub", "[game][guerrilla][occupierforces]")
{
    OccupierForcesTuning t;
    CHECK(t.waveDwell == 2);
    CHECK(t.waveFinal);
    CHECK(t.waveShare == Approx(0.5f));
    CHECK(t.waveMin == 12);
    CHECK(t.waveMax == 48);
    CHECK(t.waveLead == Approx(600.0f));
    CHECK(t.waveLeadFinal == Approx(120.0f));
    CHECK(t.waveRetakeShare == Approx(0.5f));
    CHECK(t.waveOps == 4);
    CHECK(t.waveCluster == Approx(2000.0f));
    CHECK(t.waveSurge == 12);
    CHECK(t.hqAssaultMin == 6);
    CHECK(OccupierForces::WaveSize(t, 24) == 12);
    CHECK(OccupierForces::WaveSize(t, 60) == 30);
    CHECK(OccupierForces::WaveSize(t, 88) == 44);
    CHECK(OccupierForces::WaveSize(t, 200) == 48);
    CHECK(OccupierForces::WaveSize(t, 10) == 12);

    Fixture f;
    f.Load(AbelLikeConfig());
    REQUIRE(f.of.Activate(f.registry));
    // the hub: the occupier-held AIRFIELD, then the SEAPORT, then the strongest
    CHECK(OccupierForces::PickHub(f.registry, f.of.Tuning(), nullptr) == f.registry.FindZoneIndex("Base0"));
    f.GiveToResistance("Base0");
    CHECK(OccupierForces::PickHub(f.registry, f.of.Tuning(), nullptr) == f.registry.FindZoneIndex("Base1"));
    f.GiveToResistance("Base1");
    f.Z("Base3").garrison = 2;
    CHECK(OccupierForces::PickHub(f.registry, f.of.Tuning(), nullptr) == f.registry.FindZoneIndex("Base2"));
    // a template hub by name wins while the occupier holds it
    f.of.TuningMutable().waveHub = "Base3";
    CHECK(OccupierForces::PickHub(f.registry, f.of.Tuning(), nullptr) == f.registry.FindZoneIndex("Base3"));
    f.GiveToResistance("Base2");
    f.GiveToResistance("Base3");
    CHECK(OccupierForces::PickHub(f.registry, f.of.Tuning(), nullptr) == -1);

    // the cluster: held zones within 2 km, 4 for the HQ's zone, 3 a base, 1
    // a town, plus heat / 40
    Fixture g;
    g.Load(AbelLikeConfig());
    g.GiveToResistance("T0"); // (1000, 3000)
    g.GiveToResistance("T1"); // (1500, 3000), 500 m away
    g.GiveToResistance("Base3");
    g.Z("T0").heat = 40;
    g.Z("T1").heat = 20;
    g.Z("Base3").heat = 0;
    const int t0 = g.registry.FindZoneIndex("T0");
    const int b3 = g.registry.FindZoneIndex("Base3");
    CHECK(OccupierForces::ClusterScore(g.registry, t0, 2000.0f, -1, VZero) == Approx(1 + 1 + 1 + 0.5f));
    CHECK(OccupierForces::ClusterScore(g.registry, b3, 2000.0f, -1, VZero) == Approx(3.0f));
    CHECK(OccupierForces::BestCluster(g.registry, 2000.0f, -1, VZero) == t0);
    // the HQ's zone counts 4; an HQ standing outside every held zone adds 4
    CHECK(OccupierForces::ClusterScore(g.registry, t0, 2000.0f, t0, g.Z("T0").pos) == Approx(4 + 1 + 1 + 0.5f));
    CHECK(OccupierForces::ClusterScore(g.registry, b3, 2000.0f, -1, g.Z("Base3").pos) == Approx(3 + 4));
    g.Z("Base3").heat = 100;
    CHECK(OccupierForces::BestCluster(g.registry, 2000.0f, -1, VZero) == b3);
}

TEST_CASE("OccupierForces - wave triggers: dwell, each fires once, the final stand and its surge",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig());
    REQUIRE(f.of.Activate(f.registry)); // poolStart 60: waves of 30
    FakeLauncher launcher;
    DirectorView v = ViewOf(f.registry);
    // two of four bases fall: C = 0.6 x 0.5 + 0.4 = 0.70 <= 0.75
    f.GiveToResistance("Base2");
    f.GiveToResistance("Base3");
    DirectorTick(f, v, launcher, 60.0f);
    CHECK(f.of.Control() == Approx(0.7f));
    CHECK_FALSE(f.of.WaveInbound()); // the first due tick: dwell 1 of 2
    CHECK(f.of.ControlDwell() == 1);
    DirectorTick(f, v, launcher, 60.0f);
    REQUIRE(f.of.WaveInbound());
    CHECK(f.of.WaveCount() == 1);
    CHECK(f.of.WaveBodies() == 30);
    CHECK(f.of.WavesFired() == 1);
    CHECK_FALSE(f.of.WaveFinalStand());
    CHECK(f.of.WaveClock() == Approx(600.0f));
    CHECK(f.of.InWave());
    const int pool = f.of.Pool();
    // it lands after the lead: the pool grows, the occupier war level rises;
    // DORMANT, so no strike goes out, and the full hub needs no top-up
    DirectorTick(f, v, launcher, 300.0f);
    CHECK(f.of.WaveInbound());
    DirectorTick(f, v, launcher, 300.0f);
    CHECK_FALSE(f.of.WaveInbound());
    CHECK(f.of.WavesLanded() == 1);
    CHECK(f.of.Pool() >= pool + 30);
    CHECK(launcher.orders.empty());
    CHECK(f.of.InWave()); // the window after a landing
    // the threshold never fires again
    for (int i = 0; i < 5; i++)
    {
        DirectorTick(f, v, launcher, 60.0f);
    }
    CHECK(f.of.WaveCount() == 1);

    // one roster base left: the final stand (C 0.55, the 0.50 line not crossed)
    f.GiveToResistance("Base1");
    DirectorTick(f, v, launcher, 60.0f);
    CHECK_FALSE(f.of.WaveInbound());
    DirectorTick(f, v, launcher, 60.0f);
    REQUIRE(f.of.WaveInbound());
    CHECK(f.of.WaveFinalStand());
    CHECK(f.of.FinalFired());
    CHECK(f.of.WaveCount() == 2);
    CHECK(f.of.WaveClock() == Approx(120.0f));
    const float before = f.Z("Base0").garrison;
    DirectorTick(f, v, launcher, 120.0f);
    CHECK(f.of.WavesLanded() == 2);
    // the surge: 12 above the last base's strength, never trimmed, never spare
    CHECK(Str(f.of.SurgeZone()) == "Base0");
    CHECK(f.of.SurgeBodies() == 12);
    CHECK(f.Z("Base0").garrison == Approx(before + 12));
    CHECK(f.Z("Base0").garrison > f.Z("Base0").garrisonMax);
    CHECK(f.of.SurgeAt("Base0") == 12);
    // the last base never sources an operation
    const DirectorAction a = f.of.PlanFor(f.registry, v, f.registry.FindZoneIndex("Base1"), CAODirector);
    CHECK_FALSE(a.go);
    for (int i = 0; i < 5; i++)
    {
        DirectorTick(f, v, launcher, 60.0f);
    }
    CHECK(f.of.WaveCount() == 2);
    // losing the base takes the surge with it
    f.GiveToResistance("Base0");
    f.of.Tick(f.registry, 60.0f);
    CHECK(f.of.SurgeBodies() == 0);
}

TEST_CASE("OccupierForces - triggers due together merge into one wave; nothing moves after the win or loss",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig());
    REQUIRE(f.of.Activate(f.registry));
    FakeLauncher launcher;
    DirectorView v = ViewOf(f.registry);
    // three bases and seven towns at once: both thresholds and the final stand
    f.GiveToResistance("Base1");
    f.GiveToResistance("Base2");
    f.GiveToResistance("Base3");
    for (int i = 0; i < 7; i++)
    {
        f.GiveToResistance(("T" + std::to_string(i)).c_str());
    }
    DirectorTick(f, v, launcher, 60.0f);
    DirectorTick(f, v, launcher, 60.0f);
    REQUIRE(f.of.WaveInbound());
    CHECK(f.of.WaveCount() == 1);
    CHECK(f.of.WavesFired() == 2);
    CHECK(f.of.FinalFired());
    CHECK(f.of.WaveFinalStand());
    CHECK(f.of.WaveClock() == Approx(120.0f));

    // a trigger due while a wave is inbound merges into it
    Fixture g;
    g.Load(AbelLikeConfig());
    REQUIRE(g.of.Activate(g.registry));
    DirectorView gv = ViewOf(g.registry);
    g.of.RequestWave(g.registry, gv, 1);
    REQUIRE(g.of.WaveInbound());
    g.GiveToResistance("Base1");
    g.GiveToResistance("Base2");
    g.GiveToResistance("Base3");
    DirectorTick(g, gv, launcher, 60.0f);
    DirectorTick(g, gv, launcher, 60.0f);
    CHECK(g.of.WaveCount() == 1);
    CHECK(g.of.WaveFinalStand()); // the merged final stand
    CHECK(g.of.WaveClock() <= 120.0f);

    // won (or lost): the inbound wave holds still
    const float clock = g.of.WaveClock();
    DirectorView won = gv;
    won.won = true;
    DirectorTick(g, won, launcher, 60.0f);
    CHECK(g.of.WaveClock() == Approx(clock));
    DirectorView lost = gv;
    lost.lost = true;
    DirectorTick(g, lost, launcher, 60.0f);
    CHECK(g.of.WaveClock() == Approx(clock));
    CHECK(g.of.WavesLanded() == 0);
}

TEST_CASE("OccupierForces - a landed wave: hub top-up, strikes on the cluster, reinforce columns, the rest pooled",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig());
    REQUIRE(f.of.Activate(f.registry)); // poolStart 60
    f.of.SetCatalogue(TruckCatalogue());
    FakeLauncher launcher;
    REQUIRE(FlipTown(f, "T13", 1) == false);
    REQUIRE(FlipTown(f, "T12", 2)); // AWAKE
    const int tokens = f.of.Tokens();
    // a held, hot base (the densest cluster), a bled base, a short hub
    f.GiveToResistance("Base3");
    f.Z("Base3").heat = 100;
    f.Z("Base2").garrison = 3;
    f.Z("Base0").garrison = 5;
    f.of.TuningMutable().waveLead = 0;
    DirectorView v = ViewOf(f.registry);
    f.of.RequestWave(f.registry, v, 1); // wave 1 (30) inbound, lead 0
    REQUIRE(f.of.WaveInbound());
    const int pool = f.of.Pool();
    f.of.Direct(f.registry, v, launcher, 60.0f); // lands now
    CHECK(f.of.WavesLanded() == 1);
    // 1. the hub (Base0, the airfield) is topped up to its garrisonMax
    // 3. one strike on Base3, from the hub, its infantry staged there
    // 4. a reinforce column from the hub to the bled Base2
    REQUIRE(launcher.orders.size() >= 2);
    const CounterAttackOrder& strike = launcher.orders[0];
    CHECK(strike.kind == CAKBase);
    CHECK(strike.origin == CAOWave);
    CHECK(strike.target == f.registry.FindZoneIndex("Base3"));
    CHECK(strike.source == f.registry.FindZoneIndex("Base0"));
    CHECK(strike.bodies == 8); // max(4, garrisonMax 8), within the 15 the strikes may take
    // the arrival rule covers wave strikes since 2026-10-16 (video 5): the
    // 3 km walk from the hub (2000 s, over forcesSlowEta) rides a truck, its
    // hull and driver paid by the pool
    CHECK(strike.package == FPTruck);
    CHECK(strike.priceUnits == 4);
    const CounterAttackOrder& column = launcher.orders[1];
    CHECK(column.kind == CAKReinforce);
    CHECK(column.origin == CAOWave);
    CHECK(column.target == f.registry.FindZoneIndex("Base2"));
    CHECK(column.source == f.registry.FindZoneIndex("Base0"));
    CHECK(column.bodies == 5);
    // the fake launcher debits nothing: the hub holds its top-up plus both
    // staged forces (5 + 3 + 8 + 5)
    CHECK(f.Z("Base0").garrison == Approx(21.0f));
    // wave operations take no director token
    int directorPaid = 0;
    for (size_t i = 2; i < launcher.orders.size(); i++)
    {
        CHECK(launcher.orders[i].origin == CAODirector);
        directorPaid += launcher.orders[i].priceUnits;
    }
    if (launcher.orders.size() == 2)
    {
        CHECK(f.of.Tokens() == tokens);
    }
    // the pool: + 30, - 3 (top-up), - 8, - 5 (staging); 14 stay, less the
    // strike's truck and what a director operation of the same tick paid
    // for its hull (the arrival rule sends a truck to a far town since
    // 2026-10-15)
    CHECK(f.of.Pool() == pool + 14 - strike.priceUnits - directorPaid);
}

TEST_CASE("OccupierForces - the occupier war level: territory plus landed waves; equal with the key off",
          "[game][guerrilla][occupierforces]")
{
    Fixture off;
    off.Load(ConfigWith("occupierForces = 0;"));
    CHECK(off.of.OccupierWarLevelFrom(2.0f) == Approx(2.0f));
    off.of.LandWave(30); // inert: nothing lands
    CHECK(off.of.OccupierWarLevelFrom(3.5f) == Approx(3.5f));

    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    CHECK(f.of.OccupierWarLevelFrom(2.0f) == Approx(2.0f));
    f.of.LandWave(12);
    CHECK(f.of.OccupierWarLevelFrom(2.0f) == Approx(3.0f));
    f.of.LandWave(12);
    CHECK(f.of.OccupierWarLevelFrom(2.0f) == Approx(4.0f));
    CHECK(f.of.OccupierWarLevelFrom(5.5f) == Approx(6.0f)); // capped
    f.of.TuningMutable().waveWarLevel = 0;
    CHECK(f.of.OccupierWarLevelFrom(2.0f) == Approx(2.0f));
}

TEST_CASE("OccupierForces - standing: recovery toward 50, the refill mean, informant tips, morale",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    // recovery: 1 point per 600 s toward 50, from either side; posts untouched
    f.Z("Town").standing = 20;
    f.Z("Other").standing = 80;
    f.Z("Post").standing = 10;
    f.of.StepStanding(f.registry, 600.0f);
    CHECK(f.Z("Town").standing == Approx(21.0f));
    CHECK(f.Z("Other").standing == Approx(79.0f));
    CHECK(f.Z("Post").standing == Approx(10.0f));
    f.Z("Other").standing = 50.5f;
    f.of.StepStanding(f.registry, 600.0f);
    CHECK(f.Z("Other").standing == Approx(50.0f)); // never past 50
    // the refill factor reads the mean of the K towns (Town 22, Other 50)
    CHECK(f.of.MeanStanding() == Approx(36.0f));

    // tips: never from a town below 50, however long fighters stand there
    f.Z("Town").standing = 0;
    f.Z("Town").lastGuerHold = 3;
    for (int i = 0; i < 60; i++)
    {
        f.of.Tick(f.registry, 60.0f);
        f.of.StepStanding(f.registry, 60.0f);
    }
    CHECK(f.of.KnownFighters(f.registry, f.registry.FindZoneIndex("Town")) == 0);
    // a trusting town (standing near 100: chance about 0.5 a tick) talks
    f.Z("Other").standing = 100;
    f.Z("Other").lastGuerHold = 2;
    for (int i = 0; i < 40 && f.of.KnownFighters(f.registry, f.registry.FindZoneIndex("Other")) == 0; i++)
    {
        f.of.Tick(f.registry, 60.0f);
        f.of.StepStanding(f.registry, 60.0f);
    }
    CHECK(f.of.KnownFighters(f.registry, f.registry.FindZoneIndex("Other")) == 2);
    bool near = false;
    for (int r = 0; r < f.of.SightingRows(); r++)
    {
        if (Str(f.of.SightingZone(r)) == "Other")
        {
            const AutoArray<Vector3>* p = f.of.SightingPositions(r);
            REQUIRE(p != nullptr);
            REQUIRE(p->Size() == 1);
            const float dx = (*p)[0].X() - f.Z("Other").pos.X();
            const float dz = (*p)[0].Z() - f.Z("Other").pos.Z();
            near = fabsf(dx) <= OccupierForces::TipJitter + 0.01f && fabsf(dz) <= OccupierForces::TipJitter + 0.01f;
        }
    }
    CHECK(near);

    // morale: a garrison in a town that turned on the occupier
    ZoneRecord town = f.Z("Town");
    town.standing = 20;
    CHECK(OccupierForces::MoraleFactor(town) == Approx(0.8f));
    town.standing = 25;
    CHECK(OccupierForces::MoraleFactor(town) == Approx(1.0f));
    ZoneRecord post = f.Z("Post");
    post.standing = 0;
    CHECK(OccupierForces::MoraleFactor(post) == Approx(1.0f));
}

TEST_CASE("OccupierForces - the wave state survives save/load", "[game][guerrilla][occupierforces][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "occupier-forces-waves.bin";
    const std::string config = AbelLikeConfig();
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    f.Load(config);
    REQUIRE(f.of.Activate(f.registry));
    FakeLauncher launcher;
    DirectorView v = ViewOf(f.registry);
    // a forced final stand lands (the surge), then wave 2 is inbound
    f.of.RequestWave(f.registry, v, 3);
    REQUIRE(f.of.WaveFinalStand());
    f.of.Direct(f.registry, v, launcher, 120.0f);
    REQUIRE(f.of.SurgeBodies() > 0);
    f.of.RequestWave(f.registry, v, 1);
    f.of.Direct(f.registry, v, launcher, 100.0f);
    REQUIRE(f.of.WaveInbound());
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    OccupierForces loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(loaded.WaveInbound());
    CHECK(loaded.WaveClock() == Approx(f.of.WaveClock()));
    CHECK(loaded.WaveBodies() == f.of.WaveBodies());
    CHECK(loaded.WaveCount() == 2);
    CHECK_FALSE(loaded.WaveFinalStand());
    CHECK(loaded.FinalFired());
    CHECK(loaded.WavesFired() == 1);
    CHECK(loaded.WavesLanded() == 1);
    CHECK(Str(loaded.SurgeZone()) == Str(f.of.SurgeZone()));
    CHECK(loaded.SurgeBodies() == f.of.SurgeBodies());
    CHECK(loaded.WaveWindowLeft() == Approx(f.of.WaveWindowLeft()));
    ExtParsMission.Clear();
    std::filesystem::remove(path);
}

// ===========================================================================
// Phase 6B: the director's air buying, town strikes and their effects, alarm
// answers, legacySave and gmLegacyQrf (OCCUPIER-FORCES.md 6.5, 7.3, 8.2, 11;
// D3, D4, D9, D17, D18, D20, D21)
// ===========================================================================

namespace
{

// the truck plus every airframe (FarBase is an occupier AIRFIELD in the fixture)
ForcePackageCatalogue AirCatalogue()
{
    ForcePackageCatalogue cat = TruckCatalogue();
    cat.hulls[FPTransportHeli].className = "Mi17";
    cat.hulls[FPTransportHeli].maxSpeedKmh = 250;
    cat.hulls[FPTransportHeli].cargo = 12;
    cat.hulls[FPTransportHeli].crew = 2;
    cat.hulls[FPAttackHeli].className = "Mi24";
    cat.hulls[FPAttackHeli].maxSpeedKmh = 300;
    cat.hulls[FPAttackHeli].cargo = 8;
    cat.hulls[FPAttackHeli].crew = 2;
    cat.hulls[FPJet].className = "Su25";
    cat.hulls[FPJet].maxSpeedKmh = 1000;
    cat.hulls[FPJet].cargo = 0;
    cat.hulls[FPJet].crew = 1;
    return cat;
}

void Report(Fixture& f, const char* zone, int count, Vector3Par at)
{
    AutoArray<SightingReport> reports;
    for (int i = 0; i < count; i++)
    {
        SightingReport r;
        r.zone = f.registry.FindZoneIndex(zone);
        r.id = 500 + (unsigned long long)i;
        r.pos = at;
        reports.Add(r);
    }
    f.of.IngestSightings(f.registry, reports);
}

// awake, two towns held, Town hot (urgent), Other cold, a rich pool
void StrikeSetup(Fixture& f)
{
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(AirCatalogue());
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    f.Z("Other").heat = 0;
    f.Z("Town").heat = 60;
    f.of.SetPool(60);
    REQUIRE(f.of.Band() == OFBandStrong);
}

int CountKind(const FakeLauncher& l, int kind)
{
    int n = 0;
    for (const CounterAttackOrder& o : l.orders)
    {
        n += o.kind == kind ? 1 : 0;
    }
    return n;
}

const CounterAttackOrder* FirstKind(const FakeLauncher& l, int kind)
{
    for (const CounterAttackOrder& o : l.orders)
    {
        if (o.kind == kind)
        {
            return &o;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("OccupierForces - a town strike goes with an urgent town retake while STRONG with 3 fighters",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    StrikeSetup(f);
    const int town = f.registry.FindZoneIndex("Town");
    const Vector3 seen(1010.0f, 0.0f, 1320.0f);
    Report(f, "Town", 3, seen);
    FakeLauncher launcher;
    DirectorView view = ViewOf(f.registry);
    const int pool0 = f.of.Pool();
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 2);
    CHECK(launcher.orders[0].kind == CAKRetake);
    const CounterAttackOrder& s = launcher.orders[1];
    CHECK(s.kind == CAKStrike);
    CHECK(s.origin == CAODirector);
    CHECK(s.target == town);
    CHECK(s.strikeZone == town);
    // D9: the reported position, never the true one
    CHECK(s.strikePos.X() == Approx(seen.X()));
    CHECK(s.strikePos.Z() == Approx(seen.Z()));
    // FarBase is an occupier AIRFIELD: the airfield prices.  Attack heli 18 +
    // 2 crew = 20, jet 22 + 1 = 23: urgent takes the faster jet within 1.5 x 20
    CHECK(s.package == FPJet);
    CHECK(std::string((const char*)s.hullClass) == "Su25");
    CHECK(s.airOrigin == f.registry.FindZoneIndex("FarBase"));
    CHECK(s.priceUnits == 23);
    CHECK(s.bodies == 0);
    CHECK(s.source == -1);
    CHECK(s.funded);
    CHECK(f.of.Tokens() == 1); // the strike takes no extra token
    CHECK(f.of.Pool() == pool0 - launcher.orders[0].priceUnits - 23);
    const int row = f.of.FindOp(s.opId);
    REQUIRE(row >= 0);
    CHECK(f.of.OpPackage(row) == FPJet);
    CHECK(f.of.OpOutstanding(row) == 23);
}

TEST_CASE("OccupierForces - the bought-when rule refuses strikes outside 7.3", "[game][guerrilla][occupierforces]")
{
    const Vector3 seen(1000.0f, 0.0f, 1300.0f);
    SECTION("two fighters are not enough")
    {
        Fixture f;
        StrikeSetup(f);
        Report(f, "Town", 2, seen);
        FakeLauncher launcher;
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        CHECK(CountKind(launcher, CAKRetake) == 1);
        CHECK(CountKind(launcher, CAKStrike) == 0);
    }
    SECTION("not urgent")
    {
        Fixture f;
        StrikeSetup(f);
        f.Z("Town").heat = 45; // a candidate, but below forcesUrgentHeat and no edge
        Report(f, "Town", 3, seen);
        FakeLauncher launcher;
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        CHECK(CountKind(launcher, CAKStrike) == 0);
        CHECK_FALSE(f.of.StrikeDue(f.registry, ViewOf(f.registry), f.registry.FindZoneIndex("Town"), CAKRetake));
    }
    SECTION("not STRONG")
    {
        Fixture f;
        StrikeSetup(f);
        f.of.SetPool(12); // 50 %: STRETCHED
        REQUIRE(f.of.Band() == OFBandStretched);
        Report(f, "Town", 3, seen);
        FakeLauncher launcher;
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        CHECK(CountKind(launcher, CAKStrike) == 0);
    }
    SECTION("inside its strike window")
    {
        Fixture f;
        StrikeSetup(f);
        Report(f, "Town", 3, seen);
        f.Z("Town").struckUntil = 1000.0f;
        DirectorView view = ViewOf(f.registry);
        FakeLauncher latch; // a refused tick latches the urgency (heat 60)
        latch.refuse = true;
        f.of.Direct(f.registry, view, latch, 1.0f);
        REQUIRE(f.of.Urgent("Town"));
        view.now = 500.0f;
        CHECK_FALSE(f.of.StrikeDue(f.registry, view, f.registry.FindZoneIndex("Town"), CAKRetake));
        view.now = 1000.0f;
        CHECK(f.of.StrikeDue(f.registry, view, f.registry.FindZoneIndex("Town"), CAKRetake));
    }
    SECTION("no strike airframe offered")
    {
        Fixture f;
        StrikeSetup(f);
        f.of.SetCatalogue(TruckCatalogue());
        Report(f, "Town", 3, seen);
        FakeLauncher launcher;
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        CHECK(CountKind(launcher, CAKStrike) == 0);
        CHECK_FALSE(f.of.PlanStrike(f.registry, ViewOf(f.registry), f.registry.FindZoneIndex("Town"), true).go);
    }
}

namespace
{
// StrikeSetup with the towns cold and the resistance-held post flipped loud:
// a base candidate while STRONG
void BaseOpenerSetup(Fixture& f)
{
    StrikeSetup(f);
    f.Z("Town").heat = 0;
    f.Z("Other").heat = 0;
    ZoneRecord& rebel = f.Z("RebelPost");
    rebel.flipDay = 1;
    rebel.flipMinute = 3;
    rebel.heat = 80;
}
} // namespace

TEST_CASE("OccupierForces - a strike opens a base retake while STRONG only on a defender reported",
          "[game][guerrilla][occupierforces]")
{
    // the comparison video (tmp/of-director-video-3, both branches): a base
    // retake on an EMPTY base, 0 fighters reported, still bought a 23-unit
    // jet opener.  The opener is a strike on defenders: with nobody reported
    // there is nothing to strike (7.3)
    SECTION("0 reported: the retake goes, no opener")
    {
        Fixture f;
        BaseOpenerSetup(f);
        const int rebel = f.registry.FindZoneIndex("RebelPost");
        FakeLauncher launcher;
        const int pool0 = f.of.Pool();
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        REQUIRE(CountKind(launcher, CAKBase) == 1);
        CHECK(CountKind(launcher, CAKStrike) == 0);
        CHECK(f.of.Pool() == pool0 - FirstKind(launcher, CAKBase)->priceUnits); // no 23 for a jet
        CHECK(f.of.OpenerFighters(f.registry, ViewOf(f.registry), rebel) == 0);
        CHECK_FALSE(f.of.StrikeDue(f.registry, ViewOf(f.registry), rebel, CAKBase));
    }
    SECTION("3 reported: the opener strikes the reported positions")
    {
        Fixture f;
        BaseOpenerSetup(f);
        const int rebel = f.registry.FindZoneIndex("RebelPost");
        const Vector3 seen(f.Z("RebelPost").pos.X() + 20.0f, 0.0f, f.Z("RebelPost").pos.Z() - 10.0f);
        Report(f, "RebelPost", 3, seen);
        FakeLauncher launcher;
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        REQUIRE(CountKind(launcher, CAKBase) == 1);
        const CounterAttackOrder* s = FirstKind(launcher, CAKStrike);
        REQUIRE(s != nullptr);
        CHECK(s->strikeZone == rebel);
        CHECK(s->strikePos.X() == Approx(seen.X())); // D9: what was reported
        CHECK(s->strikePos.Z() == Approx(seen.Z()));
        CHECK(f.of.OpenerFighters(f.registry, ViewOf(f.registry), rebel) == 3);
        CHECK(f.of.StrikeDue(f.registry, ViewOf(f.registry), rebel, CAKBase));
    }
    SECTION("a decayed report is nobody")
    {
        Fixture f;
        BaseOpenerSetup(f);
        const int rebel = f.registry.FindZoneIndex("RebelPost");
        Report(f, "RebelPost", 3, f.Z("RebelPost").pos);
        f.of.Tick(f.registry, f.of.Tuning().intelDecay + 60.0f); // the report ages out
        CHECK(f.of.OpenerFighters(f.registry, ViewOf(f.registry), rebel) == 0);
        CHECK_FALSE(f.of.StrikeDue(f.registry, ViewOf(f.registry), rebel, CAKBase));
    }
}

TEST_CASE("OccupierForces - an HQ assault opener needs a defender reported at the HQ",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    StrikeSetup(f);
    DirectorView v = ViewOf(f.registry);
    v.hqEstablished = true;
    v.hqZone = f.registry.FindZoneIndex("Camp");
    v.hqPos = f.Z("Camp").pos;
    // the HQ in a zone: that zone's reported fighters
    CHECK(f.of.OpenerFighters(f.registry, v, OccupierForces::HqCandidate) == 0);
    CHECK_FALSE(f.of.StrikeDue(f.registry, v, OccupierForces::HqCandidate, CAKHqAssault));
    Report(f, "Camp", 2, v.hqPos);
    CHECK(f.of.OpenerFighters(f.registry, v, OccupierForces::HqCandidate) == 2);
    CHECK(f.of.StrikeDue(f.registry, v, OccupierForces::HqCandidate, CAKHqAssault));
    // an HQ outside every zone: the reported positions within 150 m of HqPos
    DirectorView out = v;
    out.hqZone = -1;
    out.hqPos = Vector3(5000.0f, 0.0f, 5000.0f);
    CHECK(f.of.OpenerFighters(f.registry, out, OccupierForces::HqCandidate) == 0);
    AutoArray<SightingReport> reports;
    for (int i = 0; i < 3; i++)
    {
        SightingReport r;
        r.zone = -1;
        r.id = 700 + (unsigned long long)i;
        // two near the HQ, one 400 m away
        r.pos = i < 2 ? Vector3(5040.0f + 10.0f * i, 0.0f, 4980.0f) : Vector3(5400.0f, 0.0f, 5000.0f);
        reports.Add(r);
    }
    f.of.IngestSightings(f.registry, reports);
    CHECK(f.of.OpenerFighters(f.registry, out, OccupierForces::HqCandidate) == 2);
    CHECK(f.of.StrikeDue(f.registry, out, OccupierForces::HqCandidate, CAKHqAssault));
}

namespace
{
// records the band the service shows at every launch and refuses strikes:
// an opener bought, then refunded in the same step
struct BandProbeLauncher final : DirectorLauncher
{
    OccupierForces* of = nullptr;
    std::vector<CounterAttackOrder> orders;
    std::vector<int> records;
    std::vector<int> bands; // Band() as each Launch saw it
    int strikesRefused = 0;
    bool Launch(const CounterAttackOrder& order) override
    {
        bands.push_back(of->Band());
        if (order.kind == CAKStrike)
        {
            strikesRefused++;
            return false;
        }
        orders.push_back(order);
        records.push_back(order.target);
        return true;
    }
    int BornOps() const override { return 0; }
    bool HasRecord(int zoneIndex) const override
    {
        return std::find(records.begin(), records.end(), zoneIndex) != records.end();
    }
};
} // namespace

TEST_CASE("OccupierForces - the band stays steady across an opener bought and refunded in one step",
          "[game][guerrilla][occupierforces]")
{
    // the operation and its opener are one ledger movement (BandHold, as
    // the air re-plan of 197a7d3): an opener bought and refunded in the same
    // step must leave no edge, not even one that the launcher could see.
    // Since 2026-10-16 (video 4) the director buys no airframe that would
    // take the pool below STRONG, so at this pool the jet is out and the
    // cheaper attack helicopter (20, which keeps STRONG) is the opener
    Fixture f;
    BaseOpenerSetup(f);
    const int rebel = f.registry.FindZoneIndex("RebelPost");
    Report(f, "RebelPost", 3, f.Z("RebelPost").pos);
    const DirectorView view = ViewOf(f.registry);
    const int start = f.of.PoolStart();
    const int strongFloor = (OccupierForces::StrongLeavePct * start + 99) / 100;
    // the prices at a rich pool: the ground package and the urgent strike
    const DirectorAction ground = f.of.PlanFor(f.registry, view, rebel, CAODirector);
    REQUIRE(ground.go);
    const int g = ground.choice.quote.poolPrice;
    const DirectorAction strike = f.of.PlanStrike(f.registry, view, rebel, true);
    REQUIRE(strike.go);
    const int s = strike.choice.quote.poolPrice;
    // the ground debit keeps STRONG, the jet's debit would demote it
    const int before = g + s + strongFloor - 1;
    f.of.SetPool(before);
    REQUIRE(f.of.Band() == OFBandStrong);
    REQUIRE(OccupierForces::StepBand(OFBandStrong, before - g, start) == OFBandStrong);
    REQUIRE(OccupierForces::StepBand(OFBandStrong, before - g - s, start) == OFBandStretched);
    REQUIRE(f.of.PlanFor(f.registry, view, rebel, CAODirector).choice.quote.poolPrice == g);

    BandProbeLauncher launcher;
    launcher.of = &f.of;
    REQUIRE(OccupierForces::StepBand(OFBandStrong, before - g - 20, start) == OFBandStrong);
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1);
    CHECK(launcher.orders[0].kind == CAKBase);
    CHECK(launcher.strikesRefused == 1); // the opener was bought, then refunded
    REQUIRE(launcher.bands.size() == 2);
    CHECK(launcher.bands[0] == OFBandStrong);
    CHECK(launcher.bands[1] == OFBandStrong); // not demoted while the jet's price was out
    CHECK(f.of.Band() == OFBandStrong);
    CHECK(f.of.Pool() == before - g); // the net movement: the ground package only
    CHECK(f.of.Dropped() == 0);       // the refund is the debit coming back, never over the ceiling
}

TEST_CASE("OccupierForces - an opener is bought only on a band the ground operation leaves STRONG",
          "[game][guerrilla][occupierforces]")
{
    // inside the hold the band is not re-stepped, but the bought-when rule
    // reads the pool as it stands (BandNow): a ground debit that takes the
    // pool below the STRONG floor buys no opener
    Fixture f;
    BaseOpenerSetup(f);
    const int rebel = f.registry.FindZoneIndex("RebelPost");
    Report(f, "RebelPost", 3, f.Z("RebelPost").pos);
    const DirectorView view = ViewOf(f.registry);
    const int start = f.of.PoolStart();
    const int strongFloor = (OccupierForces::StrongLeavePct * start + 99) / 100;
    f.of.SetPool(strongFloor + 2);
    REQUIRE(f.of.StrikeDue(f.registry, view, rebel, CAKBase));
    {
        OccupierForces::BandHold hold(f.of);
        REQUIRE(f.of.Spend(3, "the ground package"));
        CHECK(f.of.Band() == OFBandStrong);       // held: not stepped, not logged
        CHECK(f.of.BandNow() == OFBandStretched); // where the movement goes
        CHECK_FALSE(f.of.StrikeDue(f.registry, view, rebel, CAKBase));
    }
    CHECK(f.of.Band() == OFBandStretched);
    CHECK(f.of.BandNow() == OFBandStretched);
}

TEST_CASE("OccupierForces - strike prices off-map without an occupier airfield", "[game][guerrilla][occupierforces]")
{
    Fixture f;
    StrikeSetup(f);
    f.GiveToResistance("FarBase");
    CHECK(OccupierForces::HeldAirfield(f.registry, f.Z("Town").pos) == -1);
    DirectorView view = ViewOf(f.registry);
    const DirectorAction a = f.of.PlanStrike(f.registry, view, f.registry.FindZoneIndex("Town"), true);
    REQUIRE(a.go);
    CHECK(a.airOrigin == -1);
    // off-map: attack heli 27 + 2 = 29, jet 33 + 1 = 34 (within 1.5 x 29)
    CHECK(a.choice.quote.package == FPJet);
    CHECK(a.choice.quote.poolPrice == 34);
    const DirectorAction slow = f.of.PlanStrike(f.registry, view, f.registry.FindZoneIndex("Town"), false);
    REQUIRE(slow.go);
    CHECK(slow.choice.quote.package == FPAttackHeli); // not urgent: the cheapest
    CHECK(slow.choice.quote.poolPrice == 29);
    // a pool that pays neither: no strike
    f.of.SetPool(20);
    CHECK_FALSE(f.of.PlanStrike(f.registry, view, f.registry.FindZoneIndex("Town"), true).go);
}

TEST_CASE("OccupierForces - a helicopter insertion competes as a package; airOrigin and the off-map price",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    StrikeSetup(f);
    const int town = f.registry.FindZoneIndex("Town");
    DirectorView view = ViewOf(f.registry);
    view.spare[f.registry.FindZoneIndex("Post")] = 0; // the infantry comes from FarBase (about 11 km)
    // urgent, nothing reported: L_req 0, minimum 4.  Foot 4 (4 units), truck
    // 4 + 1 + 3 = 8, helicopter 4 + 2 + 10 = 16.  Since phase 1 C (d90a942)
    // urgency never buys air on its own, even with a cost share of 4
    f.of.TuningMutable().urgentCostShare = 4.0f;
    FakeLauncher latch; // a refused tick latches the urgency (heat 60)
    latch.refuse = true;
    f.of.Direct(f.registry, view, latch, 1.0f);
    REQUIRE(f.of.Urgent("Town"));
    DirectorAction a = f.of.PlanFor(f.registry, view, town, CAODirector);
    REQUIRE(a.go);
    CHECK(a.urgent);
    CHECK(a.choice.quote.package != FPTransportHeli);
    // a fighter reported, 11 km out of walking reach: the insertion competes
    // and goes (L_req 3.6: four inserted bodies bring 5)
    Report(f, "Town", 1, f.Z("Town").pos);
    a = f.of.PlanFor(f.registry, view, town, CAODirector);
    REQUIRE(a.go);
    CHECK(a.choice.quote.package == FPTransportHeli);
    CHECK(a.choice.quote.poolPrice == 12);
    CHECK(a.airOrigin == f.registry.FindZoneIndex("FarBase"));
    // without the airfield the helicopter is quoted at the off-map price (15)
    f.GiveToResistance("FarBase");
    view.spare[f.registry.FindZoneIndex("Post")] = 4;
    a = f.of.PlanFor(f.registry, view, town, CAODirector);
    REQUIRE(a.go);
    CHECK(a.airOrigin == -1);
    bool quoted = false;
    for (int i = 0; i < a.choice.nQuotes; i++)
    {
        if (a.choice.quotes[i].package == FPTransportHeli && !a.choice.quotes[i].HasEscort())
        {
            CHECK(a.choice.quotes[i].hullPrice == 15);
            quoted = true;
        }
    }
    CHECK(quoted);
}

TEST_CASE("OccupierForces - CAOpStruck applies the strike effects and the tallies", "[game][guerrilla][occupierforces]")
{
    Fixture f;
    StrikeSetup(f);
    const int town = f.registry.FindZoneIndex("Town");
    f.Z("Town").support = 70;
    f.Z("Town").standing = 50;
    f.Z("Held").support = 80; // 1.7 km from Town
    f.Z("Held").standing = 50;
    f.Z("HqTown").support = 30; // 2.02 km: outside the ring
    f.of.SetMissionNow(300.0f);
    CounterAttackOpEvent ev;
    ev.opId = 9; // not a director operation (a test lever): applied all the same
    ev.type = CAOpStruck;
    ev.kind = CAKStrike;
    ev.target = town;
    ev.zone = town;
    ev.pos = Vector3(1010.0f, 0.0f, 1310.0f);
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.Z("Town").support == Approx(30.0f));
    CHECK(f.Z("Town").standing == Approx(10.0f));
    CHECK(f.Z("Town").struckUntil == Approx(1500.0f));
    CHECK(f.Z("Held").support == Approx(70.0f));
    CHECK(f.Z("Held").standing == Approx(40.0f));
    CHECK(f.Z("HqTown").support == Approx(30.0f));
    CHECK(f.of.TownStrikes() == 1);
    CHECK(f.of.StrikesApplied() == 1);
    // a struck town is easy to retake: at or below townRevertSupport the next
    // enemy standing in it reverts it (D5, D18), the occupier motive
    CHECK(f.Z("Town").support <= f.registry.Tuning().townRevertSupport);
    // a strike that hits a base cuts the ring only
    ev.zone = f.registry.FindZoneIndex("Post");
    ev.pos = VZero;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.of.TownStrikes() == 1);
    CHECK(f.of.StrikesApplied() == 2);
}

TEST_CASE("OccupierForces - a RED occupier zone is answered with one CAKAlarm order", "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    REQUIRE(f.of.Posture() == OFDormant); // answered before the wake (8.2)
    const int post = f.registry.FindZoneIndex("Post");
    const int far = f.registry.FindZoneIndex("FarBase");
    const Vector3 seen(1620.0f, 0.0f, 1590.0f);
    Report(f, "Post", 2, seen);
    FakeLauncher launcher;
    DirectorView view = ViewOf(f.registry);
    // nothing RED: nothing
    f.of.AnswerAlarms(f.registry, view, launcher);
    CHECK(launcher.orders.empty());
    view.alert[post] = ASRed;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1);
    const CounterAttackOrder o = launcher.orders[0];
    CHECK(o.kind == CAKAlarm);
    CHECK(o.origin == CAOAlarm);
    CHECK(o.target == post);
    CHECK(o.alarmZone == post);
    CHECK(o.alarmPos.X() == Approx(seen.X())); // the reported position (D9)
    CHECK(o.source == far);                    // never the zone under attack
    CHECK(o.bodies >= ForcePackages::AlarmMinBodies);
    CHECK(o.urgent);
    CHECK(f.of.AlarmAnswers() == 1);
    CHECK(f.of.Tokens() == 0); // none taken (none held while DORMANT)
    // one at a time: the record lives
    f.of.AnswerAlarms(f.registry, view, launcher);
    CHECK(launcher.orders.size() == 1);
    // the record ends: the next scan answers again while it is still RED
    launcher.Resolve(post);
    f.of.AnswerAlarms(f.registry, view, launcher);
    REQUIRE(launcher.orders.size() == 2);
    // a repelled answer rests the zone (the cooldown), no heat on an occupier zone
    const float heat = f.Z("Post").heat;
    CounterAttackOpEvent ev;
    ev.opId = launcher.orders[1].opId;
    ev.type = CAOpRepelled;
    ev.kind = CAKAlarm;
    ev.target = post;
    f.of.NoteOpEvent(f.registry, ev);
    CHECK(f.Z("Post").heat == Approx(heat));
    CHECK(f.of.Cooldown("Post") > 0);
    launcher.Resolve(post);
    f.of.AnswerAlarms(f.registry, view, launcher);
    CHECK(launcher.orders.size() == 2);
}

TEST_CASE("OccupierForces - alarm limits: THIN the cheapest, SPENT none, legacySave none, resistance zones none",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    const int post = f.registry.FindZoneIndex("Post");
    FakeLauncher launcher;
    DirectorView view = ViewOf(f.registry);
    // a RED zone the resistance holds is no alarm
    view.alert[f.registry.FindZoneIndex("RebelPost")] = ASRed;
    f.of.AnswerAlarms(f.registry, view, launcher);
    CHECK(launcher.orders.empty());
    view.alert[post] = ASRed;
    // legacySave: the script QRF answers, never the director
    f.of.SetLegacySave(true);
    f.of.AnswerAlarms(f.registry, view, launcher);
    CHECK(launcher.orders.empty());
    f.of.SetLegacySave(false);
    // SPENT: starved
    f.of.SetPool(0);
    REQUIRE(f.of.Band() == OFBandSpent);
    f.of.AnswerAlarms(f.registry, view, launcher);
    CHECK(launcher.orders.empty());
    // THIN: answered, not urgent (the cheapest package)
    f.of.SetPool(9); // 37.5 %: out of SPENT (above 10 %), THIN until above 40 %
    REQUIRE(f.of.Band() == OFBandThin);
    f.of.AnswerAlarms(f.registry, view, launcher);
    REQUIRE(launcher.orders.size() == 1);
    CHECK_FALSE(launcher.orders[0].urgent);
    CHECK(launcher.orders[0].package == FPFoot);
    // the won freeze
    launcher.Resolve(post);
    view.won = true;
    f.of.AnswerAlarms(f.registry, view, launcher);
    CHECK(launcher.orders.size() == 1);
}

TEST_CASE("OccupierForces - gmLegacyQrf: key off, legacy save; false with the native answers",
          "[game][guerrilla][occupierforces]")
{
    {
        Fixture f;
        f.Load(ConfigWith("occupierForces = 0;"));
        CHECK(f.of.LegacyQrf());
    }
    {
        Fixture f;
        f.Load(ConfigWith("occupierForces = 1;"));
        CHECK_FALSE(f.of.LegacyQrf()); // before the first sizing
        REQUIRE(f.of.Activate(f.registry));
        CHECK_FALSE(f.of.LegacyQrf());
        f.of.SetLegacySave(true);
        CHECK(f.of.LegacyQrf());
    }
    {
        // an old save without the block (D20)
        Fixture f;
        f.Load(ConfigWith("occupierForces = 1;"));
        f.of.DeriveFromExistingSave(f.registry);
        REQUIRE(f.of.Active());
        CHECK(f.of.LegacySave());
        CHECK(f.of.LegacyQrf());
    }
}

TEST_CASE("OccupierForces - phase 6 state survives save/load; an older block becomes legacySave",
          "[game][guerrilla][occupierforces][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "occupier-forces-phase6.bin";
    const std::filesystem::path oldPath = dir / "occupier-forces-phase5.bin";
    const std::string config = ConfigWith("occupierForces = 1;");
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    StrikeSetup(f);
    f.of.SetMissionNow(10.0f);
    f.of.ApplyStrikeEffects(f.registry, f.registry.FindZoneIndex("Town"), VZero, 10.0f, 0);
    FakeLauncher launcher;
    DirectorView view = ViewOf(f.registry);
    view.alert[f.registry.FindZoneIndex("Post")] = ASRed;
    f.of.AnswerAlarms(f.registry, view, launcher);
    REQUIRE(f.of.AlarmAnswers() == 1);
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    {
        // the same block as a phase-5 build wrote it: no "format" key
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        ar.GetParamFile()->Delete("format");
        REQUIRE(ar.SaveBin(oldPath.string().c_str()));
    }
    auto load = [](const std::filesystem::path& p, OccupierForces& into) {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(p.string().c_str()));
        ar.FirstPass();
        REQUIRE(into.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(into.Serialize(ar) == LSOK);
    };
    OccupierForces loaded;
    load(path, loaded);
    CHECK(loaded.Active());
    CHECK(loaded.LoadedFormat() == OccupierForces::SaveFormat);
    CHECK_FALSE(loaded.LegacySave());
    CHECK_FALSE(loaded.LegacyQrf());
    CHECK(loaded.TownStrikes() == 1);
    CHECK(loaded.StrikesApplied() == 1);
    CHECK(loaded.AlarmAnswers() == 1);
    const int row = loaded.FindOp(launcher.orders[0].opId);
    REQUIRE(row >= 0);
    CHECK(loaded.OpPackage(row) == launcher.orders[0].package);

    OccupierForces old;
    load(oldPath, old);
    CHECK(old.Active());
    CHECK(old.LoadedFormat() == 0);
    CHECK(old.LegacySave());
    CHECK(old.LegacyQrf());
    // no native answer beside the old save's script QRF
    FakeLauncher l2;
    old.SetCatalogue(TruckCatalogue());
    old.AnswerAlarms(f.registry, view, l2);
    CHECK(l2.orders.empty());

    ExtParsMission.Clear();
    std::filesystem::remove(path);
    std::filesystem::remove(oldPath);
}

// ---------------------------------------------------------------------------
// LOW review items L6, L8, L14 (tmp/of-review-phases-1-3.md)
// ---------------------------------------------------------------------------

namespace
{
CounterAttackOpEvent OpEvent(int opId, int type, int target, int units = 0)
{
    CounterAttackOpEvent ev;
    ev.opId = opId;
    ev.type = type;
    ev.target = target;
    ev.units = units;
    return ev;
}
} // namespace

TEST_CASE("OccupierForces - L6: the ledger never drops a row with units outstanding", "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetPool(1000);
    const int pool0 = f.of.Pool();
    const int town = f.registry.FindZoneIndex("Town");
    int outstanding = 0;
    // 40 operations, each settled with its truck (4 units) still driving home
    for (int i = 1; i <= 40; i++)
    {
        REQUIRE(f.of.Spend(4, "op"));
        REQUIRE(f.of.BookLeverOp(i, "Town", CAKRetake, FPTruck, 4));
        f.of.NoteOpEvent(f.registry, OpEvent(i, CAOpSettled, town));
        outstanding += 4;
    }
    CHECK(f.of.OpRows() == 40); // past MaxOpRows: nothing is settled, nothing goes
    REQUIRE(f.of.FindOp(1) >= 0);
    CHECK(f.of.Pool() + outstanding == pool0);
    // the oldest truck comes home: its price back, and its settled row goes
    f.of.NoteOpEvent(f.registry, OpEvent(1, CAOpHullRefund, town, 4));
    outstanding -= 4;
    CHECK(f.of.FindOp(1) < 0);
    CHECK(f.of.OpRows() == 39);
    CHECK(f.of.Pool() + outstanding == pool0);
    // a live operation (not finished) is never dropped either
    REQUIRE(f.of.Spend(4, "op"));
    REQUIRE(f.of.BookLeverOp(41, "Town", CAKRetake, FPTruck, 4));
    outstanding += 4;
    // past MaxOpRows * 4 rows the oldest finished row is settled to the pool
    // first: the units are never lost
    for (int i = 42; i <= 140; i++)
    {
        REQUIRE(f.of.Spend(4, "op"));
        REQUIRE(f.of.BookLeverOp(i, "Town", CAKRetake, FPTruck, 4));
        f.of.NoteOpEvent(f.registry, OpEvent(i, CAOpSettled, town));
        outstanding += 4;
    }
    CHECK(f.of.OpRows() <= OccupierForces::MaxOpRows * 4);
    CHECK(f.of.FindOp(41) >= 0);
    int held = 0;
    for (int r = 0; r < f.of.OpRows(); r++)
    {
        held += f.of.OpOutstanding(r);
    }
    CHECK(f.of.Pool() + held == pool0);
}

TEST_CASE("OccupierForces - the crew ledger: settled crew leave the row, so pruning past the cap creates nothing",
          "[game][guerrilla][occupierforces]")
{
    // a truck is 3 hull + 1 crew on the row.  The hull comes home (its 3 back
    // to the pool), the crewman comes home as a body (CounterAttack's
    // CrewHome puts him in a reserve; CAOpCrewSettled lets the row go of him
    // and credits nothing).  140 such operations, past MaxOpRows * 4: every
    // row reads 0 when settled, so no prune ever refunds a crewman twice.
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetPool(1000);
    const int pool0 = f.of.Pool();
    const int town = f.registry.FindZoneIndex("Town");
    int crewHome = 0; // bodies the crews brought home (a reserve, not the pool)
    auto held = [&]()
    {
        int h = 0;
        for (int r = 0; r < f.of.OpRows(); r++)
        {
            h += f.of.OpOutstanding(r);
        }
        return h;
    };
    for (int i = 1; i <= 140; i++)
    {
        REQUIRE(f.of.Spend(4, "op"));
        REQUIRE(f.of.BookLeverOp(i, "Town", CAKRetake, FPTruck, 4));
        f.of.NoteOpEvent(f.registry, OpEvent(i, CAOpSettled, town));
        f.of.NoteOpEvent(f.registry, OpEvent(i, CAOpCrewSettled, town, 1));
        crewHome += 1;
        f.of.NoteOpEvent(f.registry, OpEvent(i, CAOpHullRefund, town, 3));
        CHECK(f.of.Pool() + held() + crewHome == pool0);
    }
    CHECK(f.of.OpRows() <= OccupierForces::MaxOpRows);
    CHECK(held() == 0);
    CHECK(f.of.Pool() == pool0 - 140);
    // a crew settle never takes more than the row holds, and never pays
    const int pool1 = f.of.Pool();
    REQUIRE(f.of.Spend(4, "op"));
    REQUIRE(f.of.BookLeverOp(141, "Town", CAKRetake, FPTruck, 4));
    f.of.NoteOpEvent(f.registry, OpEvent(141, CAOpCrewSettled, town, 9));
    CHECK(f.of.OpOutstanding(f.of.FindOp(141)) == 0);
    CHECK(f.of.Pool() == pool1 - 4);
}

TEST_CASE("OccupierForces - a ledger row from a save before the crew ledger drops its settled crew on load",
          "[game][guerrilla][occupierforces][save][load]")
{
    // an older save's row kept its crew after they settled as bodies: a
    // truck whose hull came home reads 1, not 0.  The load reconcile drops
    // it to what CounterAttack still owes the operation (nothing here),
    // crediting nothing; a row made since is left alone
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetPool(100);
    const int town = f.registry.FindZoneIndex("Town");
    REQUIRE(f.of.Spend(4, "op"));
    REQUIRE(f.of.BookLeverOp(900101, "Town", CAKRetake, FPTruck, 4));
    REQUIRE(f.of.Spend(4, "op"));
    REQUIRE(f.of.BookLeverOp(900102, "Town", CAKRetake, FPTruck, 4));
    f.of.NoteOpEvent(f.registry, OpEvent(900101, CAOpSettled, town));
    f.of.NoteOpEvent(f.registry, OpEvent(900101, CAOpHullRefund, town, 3)); // the old code's residue: 1
    REQUIRE(f.of.OpOutstanding(f.of.FindOp(900101)) == 1);
    REQUIRE(CounterAttack::Instance().OwedOnOp(900101) == 0);
    f.of.MarkOpRowLegacy(f.of.FindOp(900101));
    const int pool = f.of.Pool();
    f.of.ReconcileLegacyOpRowsForTest();
    CHECK(f.of.OpOutstanding(f.of.FindOp(900101)) == 0);
    CHECK(f.of.OpOutstanding(f.of.FindOp(900102)) == 4); // a current row: untouched
    CHECK(f.of.Pool() == pool);                           // nothing credited
    f.of.ReconcileLegacyOpRowsForTest();                  // once only
    CHECK(f.of.OpOutstanding(f.of.FindOp(900102)) == 4);
    CHECK(f.of.Pool() == pool);
}

TEST_CASE("OccupierForces - L8: a phase-2 block with an empty distinct-town set is seeded from the owners",
          "[game][guerrilla][occupierforces][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "occupier-forces-phase2.bin";
    const std::string config = ConfigWith("occupierForces = 1;");
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    f.Load(config);
    // the campaign: two towns taken while the block never filled the set
    f.GiveToResistance("Town");
    f.GiveToResistance("Other");
    {
        // a phase-2 block, written field by field: no TownsEver rows, no
        // director, no "format"
        ParamArchiveSave ar(WorldSerializeVersion);
        bool active = true;
        int pool = 10, poolStart = 24, posture = OFDormant;
        REQUIRE(ar.Serialize("active", active, 1, false) == LSOK);
        REQUIRE(ar.Serialize("pool", pool, 1, 0) == LSOK);
        REQUIRE(ar.Serialize("poolStart", poolStart, 1, 0) == LSOK);
        REQUIRE(ar.Serialize("posture", posture, 1, 0) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    OccupierForces loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    REQUIRE(loaded.Active());
    CHECK(loaded.TownsEverCount() == 0);
    CHECK(loaded.TownsEverPending());
    CHECK(loaded.Posture() == OFDormant);
    // the engine runs it on the second pass with the restored registry
    loaded.SeedTownsEverOnLoad(f.registry);
    CHECK_FALSE(loaded.TownsEverPending());
    CHECK(loaded.TownsEverCount() == 2);
    CHECK(loaded.TownEver("Town"));
    CHECK(loaded.TownEver("Other"));
    CHECK_FALSE(loaded.TownEver("Held")); // authored to the resistance: never taken (L12)
    CHECK(loaded.Posture() == OFAwake);    // wakeTowns 2
    // once per load
    f.Z("Town").owner = RString("NEUTRAL");
    loaded.SeedTownsEverOnLoad(f.registry);
    CHECK(loaded.TownsEverCount() == 2);

    // a current block (format SaveFormat) is never re-seeded, even empty
    REQUIRE(f.of.Activate(f.registry));
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    OccupierForces current;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(current.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(current.Serialize(ar) == LSOK);
    }
    CHECK_FALSE(current.TownsEverPending());
    ExtParsMission.Clear();
    std::filesystem::remove(path);
}

TEST_CASE("OccupierForces - L14: a test lever's funded operation books a ledger row and balances 5.5",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart(), R = f.of.RosterStrength(), G = f.of.TownGarrisons();
    const int reserves = R + G;
    int op = 0;
    int losses = 0;
    auto check = [&]()
    {
        CHECK(OccupierForces::ConservationCheck(f.of.Pool(), reserves, 0, op, P, R, G, f.of.RefillTotal(),
                                                f.of.WavesTotal(), losses));
    };
    const int t0 = f.registry.FindZoneIndex("T0");
    const float heat0 = f.Z("T0").heat;
    // gmCounterAttackLaunchFunded's order: a column of truck (3 + 1) and an
    // APC escort (8 + 2)
    REQUIRE(f.of.Spend(14, "test lever"));
    REQUIRE(f.of.BookLeverOp(900001, "T0", CAKRetake, FPTruck, 14));
    op += 14;
    check();
    CHECK_FALSE(f.of.BookLeverOp(900001, "T0", CAKRetake, FPTruck, 4)); // one row per op id
    CHECK_FALSE(f.of.BookLeverOp(900002, "T0", CAKRetake, FPTruck, 0)); // nothing spent, nothing booked
    // repelled: a lever row drives no director action (no heat, no rest)
    f.of.NoteOpEvent(f.registry, OpEvent(900001, CAOpRepelled, t0));
    CHECK(f.Z("T0").heat == Approx(heat0));
    CHECK(f.of.Cooldown("T0") == Approx(0.0f));
    // the escort comes home, the truck is destroyed
    f.of.NoteOpEvent(f.registry, OpEvent(900001, CAOpHullRefund, t0, 10));
    op -= 10;
    check();
    f.of.NoteOpEvent(f.registry, OpEvent(900001, CAOpHullLost, t0, 4));
    op -= 4;
    losses += 4;
    check();
    CHECK(f.of.OpOutstanding(f.of.FindOp(900001)) == 0);
    // a refused lever launch gives the row back
    REQUIRE(f.of.Spend(4, "test lever"));
    REQUIRE(f.of.BookLeverOp(900003, "T0", CAKRetake, FPTruck, 4));
    f.of.CancelLeverOp(900003);
    CHECK(f.of.FindOp(900003) < 0);
    check();
}

TEST_CASE("OccupierForces - a band hold steps the band once, on the net pool, when the last hold closes",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    const int start = f.of.PoolStart();
    REQUIRE(start >= 20);
    const int strongFloor = (OccupierForces::StrongLeavePct * start + 99) / 100;
    // a debit that demotes and a credit that would not restore (hysteresis)
    // net to a STRONG pool: no edge at all
    f.of.SetPool(strongFloor + 2);
    REQUIRE(f.of.Band() == OFBandStrong);
    {
        OccupierForces::BandHold outer(f.of);
        {
            OccupierForces::BandHold inner(f.of); // holds nest
            REQUIRE(f.of.Spend(6, "test debit"));
            CHECK(f.of.Band() == OFBandStrong); // held: not stepped
        }
        CHECK(f.of.Band() == OFBandStrong); // the outer hold is still open
        f.of.Refund(4, "test credit");
        CHECK(f.of.Pool() == strongFloor);
    }
    CHECK(f.of.Band() == OFBandStrong);
    // the same moves without a hold: the false edge sticks
    f.of.SetPool(strongFloor + 2);
    REQUIRE(f.of.Spend(6, "test debit"));
    f.of.Refund(4, "test credit");
    CHECK(f.of.Pool() == strongFloor);
    CHECK(f.of.Band() == OFBandStretched);
    // a real net demotion is still taken, once, when the hold closes
    f.of.SetPool(start);
    REQUIRE(f.of.Band() == OFBandStrong);
    {
        OccupierForces::BandHold hold(f.of);
        REQUIRE(f.of.Spend(start - strongFloor + 1, "test debit"));
        CHECK(f.of.Band() == OFBandStrong);
    }
    CHECK(f.of.Band() == OFBandStretched);
    // a hold with no pool change steps nothing
    {
        OccupierForces::BandHold hold(f.of);
    }
    CHECK(f.of.Band() == OFBandStretched);
}

TEST_CASE("OccupierForces - a configured pool floor admits its exact fractional boundary",
          "[game][guerrilla][occupierforces][poolfloor]")
{
    Fixture f;
    f.Load(ConfigWith("forcesPoolFloor = 0.3; forcesPoolMin = 100;", AbelLikeConfig().c_str()), false);
    REQUIRE(f.of.Activate(f.registry));
    REQUIRE(f.of.PoolStart() == 100);
    // Enter THIN below its 30% entry edge, then recover to the floor.
    f.of.SetPool(29);
    REQUIRE(f.of.Band() == OFBandThin);
    CHECK_FALSE(f.of.CanOperate());
    CHECK_FALSE(f.of.Spend(1, "below floor"));
    f.of.SetPool(30);
    REQUIRE(f.of.Band() == OFBandThin);
    CHECK(f.of.CanOperate());
    CHECK(f.of.Spend(1, "at floor"));
    CHECK(f.of.Pool() == 29);
    CHECK_FALSE(f.of.CanOperate());

    CHECK(OccupierForces::CanOperateIn(OFBandThin, 15, 50, 0.3f));
    CHECK_FALSE(OccupierForces::CanOperateIn(OFBandThin, 14, 50, 0.3f));
    CHECK(OccupierForces::CanOperateIn(OFBandThin, 16, 50, 0.3f));
    CHECK_FALSE(OccupierForces::CanOperateIn(OFBandThin, 15, 50, 0.3001f));
    CHECK_FALSE(OccupierForces::CanOperateIn(OFBandSpent, 15, 50, 0.3f));
}

// ===========================================================================
// 2026-10-15, the playable rate (owner): the band cadence and the STRONG
// operation cap, the spare cap on base retakes and HQ assaults, and the
// arrival rule in the package choice (OCCUPIER-FORCES.md 6.3, 6.5, 7.2, 16)
// ===========================================================================

namespace
{

// one hour of a director that always has a hot candidate and whose
// operations end at once: the operations it launches after spending the
// tokens of its wake.  bandPool sets the band (poolStart 24 in the fixture).
int LaunchesPerHour(const std::string& config, int bandPool, int expectBand)
{
    Fixture f;
    f.Load(config);
    REQUIRE(f.of.Activate(f.registry));
    FakeLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    f.of.SetPool(bandPool);
    REQUIRE(f.of.Band() == expectBand);
    const DirectorView view = ViewOf(f.registry);
    auto tick = [&]() {
        f.Z("Town").heat = 45.0f;
        f.Z("Other").heat = 45.0f;
        launcher.Resolve(f.registry.FindZoneIndex("Town"));
        launcher.Resolve(f.registry.FindZoneIndex("Other"));
        f.of.Direct(f.registry, view, launcher, 60.0f);
    };
    // the wake's tokens go first
    for (int i = 0; i < 4 && f.of.Tokens() > 0; i++)
    {
        tick();
    }
    if (expectBand != OFBandSpent)
    {
        REQUIRE(f.of.Tokens() == 0);
    }
    const size_t before = launcher.orders.size();
    for (int minute = 0; minute < 60; minute++)
    {
        tick();
    }
    return (int)(launcher.orders.size() - before);
}

// Malden's eastern bases and three towns at the distances of
// of-generated-bases-design.md section 1: Houdan 0.5 km from the Outpost,
// a La Trinite at 1.7 km, a Goisse 4.6 km west (6.0 km from the Harbour,
// 6.2 km from the Airfield).  R 26, K 3, poolStart 32.
const char* kMaldenLike = "class CfgGuerrillaZones\n"
                          "{\n"
                          "    occupierForces = 1;\n"
                          "    class Zones\n"
                          "    {\n"
                          "        class Camp     { name=\"Camp\";     type=\"CAMP\";     owner=\"RESISTANCE\"; "
                          "position[]={7515.0, 5790.0, 0.0}; };\n"
                          "        class Outpost  { name=\"Outpost\";  type=\"OUTPOST\";  owner=\"OCCUPIER\"; "
                          "garrison=6; position[]={7600.0, 6300.0, 0.0}; };\n"
                          "        class Harbour  { name=\"Harbour\";  type=\"SEAPORT\";  owner=\"OCCUPIER\"; "
                          "garrison=8; position[]={8410.0, 3690.0, 0.0}; };\n"
                          "        class Airfield { name=\"Airfield\"; type=\"AIRFIELD\"; owner=\"OCCUPIER\"; "
                          "garrison=12; position[]={8030.0, 9905.0, 0.0}; };\n"
                          "        class Houdan   { name=\"Houdan\";   type=\"CITY\";     owner=\"NEUTRAL\"; "
                          "support=20; position[]={7117.0, 6069.0, 0.0}; };\n"
                          "        class Trinite  { name=\"Trinite\";  type=\"CITY\";     owner=\"NEUTRAL\"; "
                          "support=20; position[]={7600.0, 8000.0, 0.0}; };\n"
                          "        class Goisse   { name=\"Goisse\";   type=\"CITY\";     owner=\"NEUTRAL\"; "
                          "support=20; position[]={3000.0, 6300.0, 0.0}; };\n"
                          "    };\n"
                          "};\n";

// the EAST ladder as the director sees it: a Ural (80 km/h, one driver, 12
// seats) and a Mi17 (crew 2, 12 seats)
ForcePackageCatalogue UralMi17()
{
    ForcePackageCatalogue cat;
    cat.hulls[FPTruck].className = "Ural";
    cat.hulls[FPTruck].maxSpeedKmh = 80;
    cat.hulls[FPTruck].cargo = 12;
    cat.hulls[FPTruck].crew = 1;
    cat.hulls[FPTransportHeli].className = "Mi17";
    cat.hulls[FPTransportHeli].maxSpeedKmh = 240;
    cat.hulls[FPTransportHeli].cargo = 12;
    cat.hulls[FPTransportHeli].crew = 2;
    return cat;
}

} // namespace

TEST_CASE("OccupierForces - the cadence per band: 10 / 17.5 / 30 minutes, none SPENT, old intervals reachable",
          "[game][guerrilla][occupierforces]")
{
    // poolStart 24: STRONG at 24, STRETCHED at 12 (50 %), THIN at 7 (29 %,
    // above the 20 % operations floor), SPENT at 0
    const std::string now = ConfigWith("occupierForces = 1;");
    CHECK(LaunchesPerHour(now, 24, OFBandStrong) == 6);    // 3600 / 600
    CHECK(LaunchesPerHour(now, 12, OFBandStretched) == 3); // 3600 / 1050 = 3.4
    CHECK(LaunchesPerHour(now, 7, OFBandThin) == 2);       // 3600 / 1800
    CHECK(LaunchesPerHour(now, 0, OFBandSpent) == 0);      // no tokens, no operations
    // the phase 3 intervals, authored
    const std::string old = ConfigWith("occupierForces = 1; forcesTokenInterval[] = {1800, 2700, 3600, 0};");
    CHECK(LaunchesPerHour(old, 24, OFBandStrong) == 2);
    CHECK(LaunchesPerHour(old, 12, OFBandStretched) == 1);
    CHECK(LaunchesPerHour(old, 7, OFBandThin) == 1);
}

TEST_CASE("OccupierForces - three born operations while STRONG, two otherwise; forcesOpsMaxStrong 0 is the old cap",
          "[game][guerrilla][occupierforces]")
{
    OccupierForcesTuning t;
    CHECK(t.opsMax == 2);
    CHECK(t.opsMaxStrong == 3);
    CHECK(OccupierForces::OpsMaxFor(t, OFBandStrong) == 3);
    CHECK(OccupierForces::OpsMaxFor(t, OFBandStretched) == 2);
    CHECK(OccupierForces::OpsMaxFor(t, OFBandThin) == 2);
    CHECK(OccupierForces::OpsMaxFor(t, OFBandSpent) == 2);
    t.opsMaxStrong = 0; // the old rule: forcesOpsMax in every band
    CHECK(OccupierForces::OpsMaxFor(t, OFBandStrong) == 2);
    t.opsMaxStrong = 3;
    t.opsMax = 5; // a larger forcesOpsMax is never lowered by the STRONG key
    CHECK(OccupierForces::OpsMaxFor(t, OFBandStrong) == 5);

    Fixture r;
    r.Load(ConfigWith("occupierForces = 1; forcesOpsMaxStrong = 9;"));
    CHECK(r.of.Tuning().opsMaxStrong == 6); // repaired into 0..6

    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    FakeLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    f.Z("Town").heat = 60;
    const DirectorView view = ViewOf(f.registry);
    // STRONG: two born operations leave a slot
    launcher.born = 2;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK(launcher.orders.size() == 1);
    // STRETCHED: two fill it
    launcher.Resolve(f.registry.FindZoneIndex("Town"));
    f.of.SetPool(12);
    REQUIRE(f.of.Band() == OFBandStretched);
    REQUIRE(f.of.Tokens() >= 1);
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK(launcher.orders.size() == 1);
    launcher.born = 1;
    f.of.Direct(f.registry, view, launcher, 60.0f);
    CHECK(launcher.orders.size() == 2);
}

TEST_CASE("OccupierForces - the spare cap: a 12-man base retake no base can source goes with the largest spare",
          "[game][guerrilla][occupierforces]")
{
    // the rule alone
    CHECK(OccupierForces::SpareCappedMin(12, 6, 6) == 6);   // the Airfield from the Harbour
    CHECK(OccupierForces::SpareCappedMin(12, 9, 6) == 9);   // the largest spare, not the floor
    CHECK(OccupierForces::SpareCappedMin(12, 4, 6) == 6);   // never below the floor
    CHECK(OccupierForces::SpareCappedMin(12, 14, 6) == 12); // never above the minimum
    CHECK(OccupierForces::SpareCappedMin(4, 2, 6) == 4);    // a smaller minimum stays
    CHECK(OccupierForces::SpareCappedMin(6, 5, 6) == 6);    // an HQ assault keeps its 6

    Fixture f;
    f.Load(kMaldenLike, false);
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(UralMi17());
    // the Airfield falls; the Outpost spares 5, the Harbour 6 (garrisonMax x
    // 0.25 stays home)
    f.GiveToResistance("Airfield");
    f.Z("Airfield").heat = 100;
    const int airfield = f.registry.FindZoneIndex("Airfield");
    const DirectorView view = ViewOf(f.registry);
    REQUIRE(view.spare[f.registry.FindZoneIndex("Outpost")] == 5);
    REQUIRE(view.spare[f.registry.FindZoneIndex("Harbour")] == 6);

    const DirectorAction a = f.of.PlanFor(f.registry, view, airfield, CAODirector);
    CHECK(a.kind == CAKBase);
    CHECK(a.minAsked == 12); // min(max(4, 12), 12)
    CHECK(a.minUsed == 6);
    REQUIRE(a.go);
    CHECK(a.choice.quote.source == f.registry.FindZoneIndex("Harbour"));
    CHECK(a.choice.quote.bodies == 6);

    // forcesSpareCap 0: the one-source rule refuses, naming the shortfall
    f.of.TuningMutable().spareCap = false;
    const DirectorAction b = f.of.PlanFor(f.registry, view, airfield, CAODirector);
    CHECK_FALSE(b.go);
    CHECK(b.minUsed == 12);
    CHECK(Str(b.choice.why) ==
          "no occupier base can spare the minimum (12 needed, the largest single-base spare is 6: short 6)");
    CHECK(b.choice.largestSpare == 6);

    // a floor above every spare: still refused, the shortfall named
    f.of.TuningMutable().spareCap = true;
    f.of.TuningMutable().spareCapFloor = 8;
    const DirectorAction c = f.of.PlanFor(f.registry, view, airfield, CAODirector);
    CHECK_FALSE(c.go);
    CHECK(c.minUsed == 8);
    CHECK(Str(c.choice.why) ==
          "no occupier base can spare the minimum (8 needed, the largest single-base spare is 6: short 2)");

    // the director launches it: a BASE operation of 6 from one base
    f.of.TuningMutable().spareCapFloor = 6;
    FakeLauncher launcher;
    FlipTown(f, "Houdan", 1);
    REQUIRE(FlipTown(f, "Trinite", 2));
    f.Z("Houdan").heat = 0;
    f.Z("Trinite").heat = 0;
    f.of.ForceCooldown("Houdan", 9999.0f);
    f.of.ForceCooldown("Trinite", 9999.0f);
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() >= 1);
    const CounterAttackOrder& o = launcher.orders[0];
    CHECK(o.kind == CAKBase);
    CHECK(o.target == airfield);
    CHECK(o.bodies == 6);
    CHECK(o.source == f.registry.FindZoneIndex("Harbour"));
}

TEST_CASE("OccupierForces - the arrival rule on Malden distances: foot near, a truck at 1.7 and 4.6 km, air on a report",
          "[game][guerrilla][occupierforces][air]")
{
    Fixture f;
    f.Load(kMaldenLike, false);
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(UralMi17());
    CHECK(f.of.Tuning().slowEta == Approx(900.0f));
    CHECK(f.of.Tuning().slowCostShare == Approx(3.0f));
    CHECK(f.of.Tuning().airEta == Approx(1500.0f));
    // the road quote (2026-10-16): no road net in the fixture, so a ground
    // leg is the straight line x forcesRoadFactor (1.5); this case pins the
    // straight line (factor 1) to keep its distances
    CHECK(f.of.Tuning().roadFactor == Approx(1.5f));
    f.of.TuningMutable().roadFactor = 1.0f;
    ForcePackages::SetRoadFactor(1.0f); // the theatre's one factor (phase 1 C): the quote reads it
    FakeLauncher launcher;
    FlipTown(f, "Houdan", 1);
    REQUIRE(FlipTown(f, "Trinite", 2));
    REQUIRE(f.of.Posture() == OFAwake);
    FlipTown(f, "Goisse", 3);
    // quiet flips (heat 45: candidates, not urgent): the rule alone decides
    f.Z("Houdan").heat = 45;
    f.Z("Trinite").heat = 45;
    f.Z("Goisse").heat = 45;
    const DirectorView view = ViewOf(f.registry);
    const int houdan = f.registry.FindZoneIndex("Houdan");
    const int trinite = f.registry.FindZoneIndex("Trinite");
    const int goisse = f.registry.FindZoneIndex("Goisse");

    // Houdan, 0.5 km: foot in about 330 s, under forcesSlowEta
    const DirectorAction h = f.of.PlanFor(f.registry, view, houdan, CAODirector);
    REQUIRE(h.go);
    CHECK_FALSE(h.urgent);
    CHECK(h.choice.quote.package == FPFoot);
    CHECK(h.choice.etaRule == PackageChoice::EtaNone);

    // La Trinite, 1.7 km: foot needs about 1133 s (> 900); the truck (4 +
    // 1 + 3 = 8 units, 2 x foot) arrives in about 128 s; the Mi17 (4 + 2 +
    // 10 = 16, 4 x) is outside the share and the foot ETA is under 1500 s
    const DirectorAction t = f.of.PlanFor(f.registry, view, trinite, CAODirector);
    REQUIRE(t.go);
    CHECK(t.choice.cheapestPackage == FPFoot);
    CHECK(t.choice.cheapestSeconds == Approx(1700.0f / 1.5f).margin(1.0f));
    CHECK(t.choice.quote.package == FPTruck);
    CHECK(t.choice.quote.price == 8);
    CHECK(t.choice.etaRule == PackageChoice::EtaSlow);

    // Goisse, 4.6 km: foot needs about 3067 s (a 51-minute walk, 65 by road).
    // Since phase 1 C (d90a942, video 5: every insertion flew to an empty
    // target) an empty town a truck reaches well under forcesAirEta gets the
    // truck (about 345 s), the cheapest package that meets forcesSlowEta
    const DirectorAction g = f.of.PlanFor(f.registry, view, goisse, CAODirector);
    REQUIRE(g.go);
    CHECK(g.choice.cheapestSeconds == Approx(4600.0f / 1.5f).margin(1.0f));
    CHECK(g.choice.quote.package == FPTruck);
    CHECK(g.choice.etaRule == PackageChoice::EtaSlow);
    CHECK(g.choice.reason == PackageChoice::ReasonGroundCheapest);
    CHECK(g.choice.quote.poolPrice == 4); // the Ural 3 + its driver 1

    // the old choice is reachable: both rules off, everything walks
    f.of.TuningMutable().slowEta = 0;
    f.of.TuningMutable().airEta = 0;
    CHECK(f.of.PlanFor(f.registry, view, trinite, CAODirector).choice.quote.package == FPFoot);
    CHECK(f.of.PlanFor(f.registry, view, goisse, CAODirector).choice.quote.package == FPFoot);
    // the slow rule alone sends Goisse a truck (about 345 s)
    f.of.TuningMutable().slowEta = 900;
    const DirectorAction gt = f.of.PlanFor(f.registry, view, goisse, CAODirector);
    CHECK(gt.choice.quote.package == FPTruck);
    CHECK(gt.choice.etaRule == PackageChoice::EtaSlow);
    // a fighter reported in Goisse, out of walking reach: the insertion goes
    // (phase 1 C's "air: fighters reported"), from the Airfield
    f.of.TuningMutable().airEta = 1500;
    Report(f, "Goisse", 1, f.Z("Goisse").pos);
    const DirectorAction ga = f.of.PlanFor(f.registry, view, goisse, CAODirector);
    REQUIRE(ga.go);
    CHECK(ga.choice.quote.package == FPTransportHeli);
    CHECK(ga.choice.etaRule == PackageChoice::EtaAir);
    CHECK(ga.airOrigin == f.registry.FindZoneIndex("Airfield"));
    CHECK(ga.choice.quote.poolPrice == 12); // the Mi17 10 + its crew 2
    // a push-out takes the arrival rule too since 2026-10-16 (6.4, video 4:
    // a push-out walked 55 minutes from the Airfield): the reported Goisse
    // gets the insertion, while Houdan's foot quote, under forcesSlowEta,
    // stays foot
    const DirectorAction gp = f.of.PlanFor(f.registry, view, goisse, CAOPushout);
    CHECK(gp.choice.quote.package == FPTransportHeli);
    CHECK(gp.choice.etaRule == PackageChoice::EtaAir);
    CHECK_FALSE(gp.urgent);
    const DirectorAction hp = f.of.PlanFor(f.registry, view, houdan, CAOPushout);
    CHECK(hp.choice.quote.package == FPFoot);
    CHECK(hp.choice.etaRule == PackageChoice::EtaNone);

    // the director launches them: La Trinite first (hotter), then Goisse
    f.Z("Houdan").heat = 0;
    f.of.ForceCooldown("Houdan", 9999.0f);
    f.Z("Trinite").heat = 48;
    const int pool0 = f.of.Pool();
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1);
    CHECK(launcher.orders[0].target == trinite);
    CHECK(launcher.orders[0].package == FPTruck);
    CHECK(launcher.orders[0].priceUnits == 4);
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 2);
    CHECK(launcher.orders[1].target == goisse);
    CHECK(launcher.orders[1].package == FPTransportHeli);
    CHECK(launcher.orders[1].airOrigin == f.registry.FindZoneIndex("Airfield"));
    CHECK(launcher.orders[1].priceUnits == 12);
    CHECK(f.of.Pool() == pool0 - 4 - 12);
    ForcePackages::SetRoadFactor(ForcePackages::DefaultRoadFactor); // no state for the next case
}

// ===========================================================================
// Round 3.5 (tmp/of-director-video-4, 2026-10-16): the re-arm after a rest
// (6.3), a director strike keeps STRONG after both debits (7.3), the HQ
// assault's settled line and the escort in the launch line (6.6), push-outs
// under the arrival rule (6.4).
// ===========================================================================

namespace
{

// a flip minute whose candidate roll for name is at least floor: the old
// rule then lists the town only at heat >= 40 x floor
int MinuteWithRoll(const char* name, float floor)
{
    for (int m = 1; m < 1440; m++)
    {
        if (CounterAttack::Roll01(CounterAttack::MakeSeed(name, 1, m, 0), OccupierForces::CandidateSalt) >= floor)
        {
            return m;
        }
    }
    return 1;
}

// Arudy's timeline in video 4: a defended held town (3 fighters, a local
// talks every tipEvery s) is attacked and repelled, rests 1800 s while its
// heat decays at 0.25 per 10 s, and the director comes back for it (or does
// not).  Every other held zone rests for good, so nothing else is a
// candidate or a push-out.  Returns the director's orders on Town and, per
// repel, the second the rest ended.
struct ArudyRun
{
    std::vector<float> launchedAt; // director orders on Town
    std::vector<float> restEnds;
    int rearmLines = 0;
    int pushouts = 0;
    float heatAtEnd = 0;
};

ArudyRun RunArudy(float rearmHeat, int repels, float seconds, float tipEvery)
{
    ArudyRun run;
    CoreLines log;
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    f.of.TuningMutable().rearmHeat = rearmHeat;
    FakeLauncher launcher;
    FlipTown(f, "Other", 1);
    REQUIRE(FlipTown(f, "Town", MinuteWithRoll("Town", 0.75f))); // the second town: AWAKE
    REQUIRE(OccupierForces::CandidateRoll(f.Z("Town")) >= 0.75f);
    f.Z("Town").heat = 50; // a loud flip: the first operation goes at once
    f.Z("Other").heat = 0;
    for (const char* rest : {"Other", "Held", "RebelPost", "Camp"})
    {
        f.of.ForceCooldown(rest, 1.0e6f);
    }
    const int town = f.registry.FindZoneIndex("Town");
    const Vector3 at = f.Z("Town").pos;
    int done = 0;
    int pendingOp = 0;
    float repelAt = -1;
    bool resting = false;
    for (float t = 0; t <= seconds; t += 60.0f)
    {
        // the held-zone heat decay: 0.25 per 10 s
        for (int i = 0; i < f.registry.NZones(); i++)
        {
            if (stricmp(f.registry.GetZone(i)->owner, f.registry.ResistanceSide()) == 0)
            {
                f.registry.HeatDecay(i, 1.5f);
            }
        }
        // "a local in Arudy talked": the true count, every tipEvery s
        if (tipEvery > 0 && fmodf(t, tipEvery) < 1.0f)
        {
            f.of.IngestTip(f.registry, town, 3, at);
        }
        // the operation in flight is repelled a minute after it left
        if (pendingOp > 0 && t >= repelAt)
        {
            CounterAttackOpEvent ev;
            ev.opId = pendingOp;
            ev.type = CAOpRepelled;
            ev.kind = CAKRetake;
            ev.target = town;
            ev.units = 4;
            f.of.NoteOpEvent(f.registry, ev);
            launcher.Resolve(town);
            resting = true;
            pendingOp = 0;
            done++;
        }
        const size_t before = launcher.orders.size();
        f.of.Tick(f.registry, 60.0f);
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        if (resting && f.of.Cooldown("Town") <= 0)
        {
            run.restEnds.push_back(t); // the tick the rest ran out
            resting = false;
        }
        for (size_t k = before; k < launcher.orders.size(); k++)
        {
            const CounterAttackOrder& o = launcher.orders[k];
            run.pushouts += o.origin == CAOPushout ? 1 : 0;
            if (o.target != town)
            {
                continue;
            }
            run.launchedAt.push_back(t);
            if (done < repels)
            {
                pendingOp = o.opId;
                repelAt = t + 60.0f;
            }
        }
    }
    run.heatAtEnd = f.Z("Town").heat;
    run.rearmLines = (int)log.With({"Town rested with", "re-armed"}).size();
    return run;
}

} // namespace

TEST_CASE("OccupierForces - re-arm: a defended town repelled twice is a candidate again within one token interval",
          "[game][guerrilla][occupierforces]")
{
    OccupierForcesTuning t;
    CHECK(t.rearmHeat == Approx(40.0f)); // forcesHeatFull's: every roll u < 1 lists the town
    CHECK(t.rearmHeat == Approx(t.heatFull));
    const float interval = OccupierForces::TokenInterval(t, OFBandStrong);
    REQUIRE(interval == Approx(600.0f));

    // video 4: Arudy repelled op 5 and op 7, tips kept it from being quiet,
    // its heat decayed to 0 inside the rest: from then a STRONG director
    // with two tokens never went back.  With the re-arm, each rest's end
    // brings the operation within one token interval
    const ArudyRun fixed = RunArudy(40.0f, 2, 6000.0f, 240.0f);
    REQUIRE(fixed.restEnds.size() == 2);
    REQUIRE(fixed.launchedAt.size() == 3);
    CHECK(fixed.launchedAt[1] >= fixed.restEnds[0]);
    CHECK(fixed.launchedAt[1] - fixed.restEnds[0] <= interval);
    CHECK(fixed.launchedAt[2] >= fixed.restEnds[1]);
    CHECK(fixed.launchedAt[2] - fixed.restEnds[1] <= interval);
    CHECK(fixed.rearmLines == 2);
    CHECK(fixed.pushouts == 0); // a reported town is never quiet

    // the old rule (forcesRearmHeat 0): the heat left by the repel decays
    // inside the rest, the roll (>= 0.75) keeps the town out, the tips keep
    // it from push-out: one operation, then nothing for the rest of the run
    const ArudyRun old = RunArudy(0.0f, 2, 6000.0f, 240.0f);
    REQUIRE(old.restEnds.size() == 1);
    CHECK(old.launchedAt.size() == 1);
    CHECK(old.rearmLines == 0);
    CHECK(old.pushouts == 0);
    CHECK(old.heatAtEnd < 1.0f);

    // nobody reported when the rest ends (no tips): no re-arm; the town is
    // quiet, and the STRONG director's push-out goes for it (6.4)
    const ArudyRun quiet = RunArudy(40.0f, 1, 6000.0f, 0.0f);
    REQUIRE(quiet.restEnds.size() == 1);
    CHECK(quiet.rearmLines == 0);
    REQUIRE(quiet.launchedAt.size() >= 2);
    CHECK(quiet.pushouts >= 1);
}

TEST_CASE("OccupierForces - re-arm: released by the launch, a lost town or decayed reports; saved",
          "[game][guerrilla][occupierforces][save][load]")
{
    const std::string config = ConfigWith("occupierForces = 1;");
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    f.Load(config);
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    const int town = f.registry.FindZoneIndex("Town");
    f.of.IngestTip(f.registry, town, 3, f.Z("Town").pos);
    f.of.ForceCooldown("Town", 120.0f);
    f.Z("Town").heat = 0;
    FakeLauncher idle;
    idle.born = 99; // every slot full: the re-armed town waits
    f.of.Direct(f.registry, ViewOf(f.registry), idle, 60.0f);
    CHECK_FALSE(f.of.Rearmed("Town")); // still resting
    f.of.Direct(f.registry, ViewOf(f.registry), idle, 60.0f);
    REQUIRE(f.of.Rearmed("Town"));
    CHECK(f.Z("Town").heat == Approx(40.0f));
    // held at the floor on every tick while it waits
    f.registry.HeatDecay(town, 30.0f);
    f.of.Direct(f.registry, ViewOf(f.registry), idle, 60.0f);
    CHECK(f.Z("Town").heat == Approx(40.0f));
    AutoArray<int> c;
    f.of.Candidates(f.registry, ViewOf(f.registry), idle, c);
    CHECK(HasCandidate(c, town));

    // saved and loaded with the block
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "occupier-forces-rearm.bin";
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    OccupierForces loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(loaded.Rearmed("Town"));
    CHECK(loaded.RearmedCount() == 1);
    ExtParsMission.Clear();
    std::filesystem::remove(path);

    // the launch releases it
    FakeLauncher launcher;
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    REQUIRE(launcher.orders.size() >= 1);
    CHECK(launcher.orders[0].target == town);
    CHECK_FALSE(f.of.Rearmed("Town"));

    // reports that decay release it (the town is quiet then: push-out's)
    Fixture g;
    g.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(g.of.Activate(g.registry));
    FlipTown(g, "Town", 1);
    REQUIRE(FlipTown(g, "Other", 2));
    g.of.IngestTip(g.registry, g.registry.FindZoneIndex("Town"), 3, g.Z("Town").pos);
    g.of.ForceCooldown("Town", 60.0f);
    g.of.Direct(g.registry, ViewOf(g.registry), idle, 60.0f);
    REQUIRE(g.of.Rearmed("Town"));
    g.of.Tick(g.registry, g.of.Tuning().intelDecay + 60.0f);
    g.of.Direct(g.registry, ViewOf(g.registry), idle, 60.0f);
    CHECK_FALSE(g.of.Rearmed("Town"));

    // a town the resistance lost releases it (an owner change forgets it)
    Fixture h;
    h.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(h.of.Activate(h.registry));
    FlipTown(h, "Town", 1);
    REQUIRE(FlipTown(h, "Other", 2));
    h.of.IngestTip(h.registry, h.registry.FindZoneIndex("Town"), 3, h.Z("Town").pos);
    h.of.ForceCooldown("Town", 60.0f);
    h.of.Direct(h.registry, ViewOf(h.registry), idle, 60.0f);
    REQUIRE(h.of.Rearmed("Town"));
    h.Z("Town").owner = RString("NEUTRAL");
    h.of.Direct(h.registry, ViewOf(h.registry), idle, 60.0f);
    CHECK_FALSE(h.of.Rearmed("Town"));
    // forcesRearmHeat 0 is the old rule: nothing re-armed
    Fixture k;
    k.Load(ConfigWith("occupierForces = 1; forcesRearmHeat = 0;"));
    REQUIRE(k.of.Activate(k.registry));
    CHECK(k.of.Tuning().rearmHeat == Approx(0.0f));
    FlipTown(k, "Town", 1);
    REQUIRE(FlipTown(k, "Other", 2));
    k.of.IngestTip(k.registry, k.registry.FindZoneIndex("Town"), 3, k.Z("Town").pos);
    k.of.ForceCooldown("Town", 60.0f);
    k.of.Direct(k.registry, ViewOf(k.registry), idle, 60.0f);
    CHECK_FALSE(k.of.Rearmed("Town"));
    // the key is read and repaired into 0..100
    Fixture r;
    r.Load(ConfigWith("occupierForces = 1; forcesRearmHeat = 250;"));
    CHECK(r.of.Tuning().rearmHeat == Approx(100.0f));
}

TEST_CASE("OccupierForces - a director strike keeps STRONG after the ground debit and its own",
          "[game][guerrilla][occupierforces][air]")
{
    // video 4: a 22-unit column on Le Port took a pool of 62 to 40, still
    // STRONG, and the 23-unit jet then took it to 17 (THIN) for 38 s against
    // 4 reported fighters.  The strike rule judges the pool after both
    // debits; the test lever keeps the old affordability
    const Vector3 seen(1010.0f, 0.0f, 1320.0f);
    const int town = 1; // checked below
    auto setup = [&](Fixture& f, int ground, int extra) {
        StrikeSetup(f);
        REQUIRE(f.registry.FindZoneIndex("Town") == town);
        Report(f, "Town", 4, seen);
        const int start = f.of.PoolStart();
        const int strongFloor = (OccupierForces::StrongLeavePct * start + 99) / 100;
        f.of.SetPool(ground + strongFloor + extra);
        REQUIRE(f.of.Band() == OFBandStrong);
        return strongFloor;
    };
    // the ground package's pool price at a rich pool
    int g = 0;
    {
        Fixture f;
        StrikeSetup(f);
        Report(f, "Town", 4, seen);
        const DirectorAction a = f.of.PlanFor(f.registry, ViewOf(f.registry), town, CAODirector);
        REQUIRE(a.go);
        g = a.choice.quote.poolPrice;
    }
    SECTION("neither airframe keeps STRONG: no strike, one INFO line")
    {
        CoreLines log;
        Fixture f;
        setup(f, g, 19); // the attack helicopter (20) and the jet (23) both demote
        FakeLauncher launcher;
        const int before = f.of.Pool();
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        CHECK(CountKind(launcher, CAKRetake) == 1);
        CHECK(CountKind(launcher, CAKStrike) == 0);
        CHECK(f.of.Pool() == before - g);
        CHECK(f.of.Band() == OFBandStrong);
        CHECK(log.With({"no strike with the operation on Town", "a strike would take the pool below STRONG"}).size() ==
              1);
        // the test lever bypasses the band rule: the pool can pay, it goes
        FakeLauncher lever;
        CHECK(f.of.ForceStrike(f.registry, ViewOf(f.registry), lever, town));
        CHECK(CountKind(lever, CAKStrike) == 1);
    }
    SECTION("the attack helicopter keeps STRONG, the jet would not: the helicopter goes")
    {
        Fixture f;
        setup(f, g, 20);
        FakeLauncher launcher;
        const int before = f.of.Pool();
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        const CounterAttackOrder* s = FirstKind(launcher, CAKStrike);
        REQUIRE(s != nullptr);
        CHECK(s->package == FPAttackHeli);
        CHECK(s->priceUnits == 20);
        CHECK(f.of.Pool() == before - g - 20);
        CHECK(f.of.Band() == OFBandStrong);
    }
    SECTION("both keep STRONG: the urgent pick, the faster jet")
    {
        Fixture f;
        setup(f, g, 23);
        FakeLauncher launcher;
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        const CounterAttackOrder* s = FirstKind(launcher, CAKStrike);
        REQUIRE(s != nullptr);
        CHECK(s->package == FPJet);
        CHECK(f.of.Band() == OFBandStrong);
    }
    SECTION("the planner alone: keepStrong refuses, the lever form does not")
    {
        Fixture f;
        setup(f, 0, 19);
        const DirectorAction keep = f.of.PlanStrike(f.registry, ViewOf(f.registry), town, true, true);
        CHECK_FALSE(keep.go);
        CHECK(Str(keep.choice.why) == OccupierForces::StrikeKeepsStrongWhy);
        CHECK(f.of.PlanStrike(f.registry, ViewOf(f.registry), town, true).go);
    }
}

TEST_CASE("OccupierForces - the HQ assault's settled line says where the bodies went, not a retake",
          "[game][guerrilla][occupierforces]")
{
    CoreLines log;
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    f.of.SetCatalogue(TruckCatalogue());
    HqLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    f.Z("Town").owner = RString("NEUTRAL");
    f.Z("Other").owner = RString("NEUTRAL");
    f.Z("Held").owner = RString("NEUTRAL");
    f.Z("RebelPost").owner = RString("EAST");
    const DirectorView v = HqView(f);
    f.of.Direct(f.registry, v, launcher, 60.0f);
    REQUIRE(launcher.orders.size() == 1);
    const CounterAttackOrder& o = launcher.orders[0];
    REQUIRE(o.kind == CAKHqAssault);
    // the pool has room for 4 of the 6; the Post has room for the rest
    f.of.SetPool(f.of.PoolStart() - 4);
    f.Z("Post").garrison = 2;
    f.Z("Post").pending = 0;
    // CounterAttack's order: the refund, then the settle (the HQ fell)
    f.of.Refund(f.registry, 6, "HQ assault done");
    CounterAttackOpEvent ev;
    ev.opId = o.opId;
    ev.type = CAOpSettled;
    ev.kind = CAKHqAssault;
    ev.target = v.hqZone;
    ev.bodies = 6;
    ev.units = 6;
    ev.note = "the HQ fell";
    f.of.NoteOpEvent(f.registry, ev);
    const std::string id = "[op " + std::to_string(o.opId) + "]";
    CHECK(log.With({id.c_str(), "HQ assault done: 4 back to the pool, 2 banked at Post"}).size() == 1);
    CHECK(log.With({id.c_str(), "retaken"}).empty());
    CHECK(log.With({id.c_str(), "stay as its garrison"}).empty());
    // nobody left standing
    CounterAttackOpEvent none = ev;
    none.bodies = 0;
    none.units = 0;
    none.note = "the HQ fell; nobody left standing";
    f.of.NoteOpEvent(f.registry, none);
    CHECK(log.With({id.c_str(), "HQ assault done: nobody of the assault left standing"}).size() == 1);
    // a town retake keeps its line
    CHECK(log.With({"retaken: "}).empty());
}

TEST_CASE("OccupierForces - a column's launch line names its escort", "[game][guerrilla][occupierforces]")
{
    // video 4, op 1: "Motorized squad (4, Ural) from Le Port Harbour to Le
    // Port (25 unit(s), the pool pays 21, ...)": 4 + 17 of the 21 were a
    // T72 the line did not name
    CoreLines log;
    Fixture f;
    f.Load(ConfigWith("occupierForces = 1;"));
    REQUIRE(f.of.Activate(f.registry));
    ForcePackageCatalogue cat;
    cat.hulls[FPTruck].className = "Ural";
    cat.hulls[FPTruck].maxSpeedKmh = 80;
    cat.hulls[FPTruck].cargo = 12;
    cat.hulls[FPTruck].crew = 1;
    cat.hulls[FPTank].className = "T72";
    cat.hulls[FPTank].maxSpeedKmh = 60;
    cat.hulls[FPTank].cargo = 0;
    cat.hulls[FPTank].crew = 3;
    f.of.SetCatalogue(cat);
    FakeLauncher launcher;
    FlipTown(f, "Town", 1);
    REQUIRE(FlipTown(f, "Other", 2));
    f.Z("Other").heat = 0;
    f.Z("Town").heat = 45;
    // four fighters (L_req 14.4) and a Post that spares only 4: the truck
    // alone is 5, the column with the T72 17
    Report(f, "Town", 4, f.Z("Town").pos);
    DirectorView view = ViewOf(f.registry);
    view.spare[f.registry.FindZoneIndex("FarBase")] = 0;
    view.spare[f.registry.FindZoneIndex("Post")] = 4;
    f.of.SetPool(60);
    f.of.Direct(f.registry, view, launcher, 60.0f);
    REQUIRE(launcher.orders.size() >= 1);
    REQUIRE(std::string((const char*)launcher.orders[0].escortClass) == "T72");
    CHECK(log.With({"Motorized squad (4, Ural) with T72 from Post to Town"}).size() == 1);
}

// ===========================================================================
// The repel ladder, massing, value-weighted candidates, the rest jitter and
// the wave strikes' arrival rule (OCCUPIER-FORCES.md 6.3, 6.5, 7.1, 7.3;
// 2026-10-16, video 5: seven identical columns on Le Port, five identical
// APCs on Arudy, a 41-minute walk on the HQ)
// ===========================================================================

namespace
{

// video 5's Malden (tmp/of-director-video-5): the Harbour 0.5 km from Le
// Port, Arudy 2.2 km from the Outpost, the Airfield 6.7 km north of Le Port;
// eight far towns the occupier administers keep control above 0.75 (no
// wave), and forcesPoolMin gives Guerrilla.Abel's poolStart 60
std::string Video5Config()
{
    std::string cfg = "class CfgGuerrillaZones\n{\n    occupierForces = 1;\n    forcesPoolMin = 60;\n"
                      "    class Zones\n    {\n";
    cfg += "        class Camp { name=\"Camp\"; type=\"CAMP\"; owner=\"RESISTANCE\"; position[]={7515.0, 5790.0, 0.0}; "
           "};\n";
    cfg += "        class Outpost { name=\"Outpost\"; type=\"OUTPOST\"; owner=\"OCCUPIER\"; garrison=6; "
           "position[]={7600.0, 6300.0, 0.0}; };\n";
    cfg += "        class Harbour { name=\"Le Port Harbour\"; type=\"SEAPORT\"; owner=\"OCCUPIER\"; garrison=8; "
           "position[]={8410.0, 3690.0, 0.0}; };\n";
    cfg += "        class Airfield { name=\"Airfield\"; type=\"AIRFIELD\"; owner=\"OCCUPIER\"; garrison=12; "
           "position[]={8030.0, 9905.0, 0.0}; };\n";
    cfg += "        class Village { name=\"Village\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=20; "
           "position[]={7117.0, 6069.0, 0.0}; };\n";
    cfg += "        class LePort { name=\"Le Port\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=20; "
           "position[]={8242.0, 3205.0, 0.0}; };\n";
    cfg += "        class Arudy { name=\"Arudy\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=20; "
           "position[]={5600.0, 7200.0, 0.0}; };\n";
    for (int i = 0; i < 8; i++)
    {
        cfg += "        class F" + std::to_string(i) + " { name=\"F" + std::to_string(i) +
               "\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=20; position[]={" + std::to_string(1000 + 400 * i) +
               ".0, 1000.0, 0.0}; };\n";
    }
    cfg += "    };\n};\n";
    return cfg;
}

// the EAST ladder of Guerrilla.Abel: Ural, BMP, T72, Mi17, Su25
ForcePackageCatalogue EastLadder()
{
    ForcePackageCatalogue cat = UralMi17();
    cat.hulls[FPApc].className = "BMP";
    cat.hulls[FPApc].maxSpeedKmh = 65;
    cat.hulls[FPApc].cargo = 8;
    cat.hulls[FPApc].crew = 3;
    cat.hulls[FPTank].className = "T72";
    cat.hulls[FPTank].maxSpeedKmh = 60;
    cat.hulls[FPTank].cargo = 0;
    cat.hulls[FPTank].crew = 3;
    cat.hulls[FPJet].className = "Su25";
    cat.hulls[FPJet].maxSpeedKmh = 1000;
    cat.hulls[FPJet].cargo = 0;
    cat.hulls[FPJet].crew = 1;
    return cat;
}

// awake with two tokens; Le Port, Arudy and the Village held (heat 40 each),
// the Village resting for good; 4 fighters reported in Le Port, 3 in Arudy
void Video5Setup(Fixture& f)
{
    f.Load(Video5Config(), false);
    REQUIRE(f.of.Activate(f.registry));
    REQUIRE(f.of.PoolStart() == 60);
    f.of.SetCatalogue(EastLadder());
    FlipTown(f, "Village", 1);
    REQUIRE(FlipTown(f, "Le Port", 2));
    FlipTown(f, "Arudy", 3);
    f.of.ForceCooldown("Village", 1.0e7f);
    f.of.IngestTip(f.registry, f.registry.FindZoneIndex("Le Port"), 4, f.Z("Le Port").pos);
    f.of.IngestTip(f.registry, f.registry.FindZoneIndex("Arudy"), 3, f.Z("Arudy").pos);
    REQUIRE(f.of.Band() == OFBandStrong);
}

// CounterAttack's report of a repelled operation: lost bodies, none home,
// then its hulls home intact (the video's "the T72 drives home")
void RepelOrder(Fixture& f, FakeLauncher& l, const CounterAttackOrder& o)
{
    CounterAttackOpEvent ev;
    ev.opId = o.opId;
    ev.type = CAOpRepelled;
    ev.kind = o.kind;
    ev.origin = o.origin;
    ev.target = o.target;
    ev.source = o.source;
    ev.units = o.bodies;
    f.of.NoteOpEvent(f.registry, ev);
    if (o.priceUnits > 0)
    {
        ev.type = CAOpHullRefund;
        ev.units = o.priceUnits;
        f.of.NoteOpEvent(f.registry, ev);
    }
    l.Resolve(o.target);
}

// the ground orders on a zone
std::vector<CounterAttackOrder> GroundOn(const FakeLauncher& l, int zone)
{
    std::vector<CounterAttackOrder> out;
    for (const CounterAttackOrder& o : l.orders)
    {
        if (o.target == zone && o.kind != CAKStrike)
        {
            out.push_back(o);
        }
    }
    return out;
}

// runs a rest out with nothing launching (every slot full)
void RestOut(Fixture& f, float seconds)
{
    FakeLauncher idle;
    idle.born = 99;
    f.of.Direct(f.registry, ViewOf(f.registry), idle, seconds);
}

} // namespace

TEST_CASE("OccupierForces - the repel ladder: escalate after one repel, change approach after two, contain after three",
          "[game][guerrilla][occupierforces]")
{
    CoreLines log;
    Fixture f;
    Video5Setup(f);
    const int lePort = f.registry.FindZoneIndex("Le Port");
    const int harbour = f.registry.FindZoneIndex("Le Port Harbour");
    const int airfield = f.registry.FindZoneIndex("Airfield");
    f.of.ForceCooldown("Arudy", 1.0e7f); // Le Port alone here
    // a repel's +15 on the re-armed 40 would make Le Port urgent (55), and the
    // plain strike rule (7.3) would then buy the jet: urgency is kept out so
    // the escalation strike alone decides
    f.of.TuningMutable().urgentHeat = 100;
    FakeLauncher launcher;

    // op 1: the plain plan, L_req 1.2 x 3 x 4 = 14.4, four bodies at least
    const DirectorAction a0 = f.of.PlanFor(f.registry, ViewOf(f.registry), lePort, CAODirector);
    REQUIRE(a0.go);
    CHECK(a0.escalation == 0);
    CHECK(a0.choice.lReq == Approx(14.4f));
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    REQUIRE(GroundOn(launcher, lePort).size() == 1);
    const CounterAttackOrder op1 = GroundOn(launcher, lePort)[0];
    CHECK(op1.source == harbour);
    RepelOrder(f, launcher, op1);
    CHECK(f.of.Repels("Le Port") == 1);
    CHECK(log.With({"Le Port: first repel", "next attack needs lethality x1.5 and at least 8 infantry"}).size() == 1);

    // op 2: L_req x 1.5 = 21.6 and twice the 4 fighters reported as infantry;
    // the Harbour spares 6, so the Airfield (9) staffs it
    RestOut(f, 2400.0f);
    REQUIRE(f.of.Cooldown("Le Port") <= 0);
    const DirectorAction a1 = f.of.PlanFor(f.registry, ViewOf(f.registry), lePort, CAODirector);
    REQUIRE(a1.go);
    CHECK(a1.escalation == 1);
    CHECK(a1.needBodies == 8);
    CHECK(a1.choice.lReq == Approx(21.6f));
    CHECK(a1.choice.quote.bodies >= 8);
    CHECK(a1.choice.quote.source == airfield);
    for (int tick = 0; tick < 6 && GroundOn(launcher, lePort).size() < 2; tick++)
    {
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f); // massing may hold a few ticks
    }
    REQUIRE(GroundOn(launcher, lePort).size() == 2);
    const CounterAttackOrder op2 = GroundOn(launcher, lePort)[1];
    CHECK(op2.bodies >= 8);
    CHECK(op2.source == airfield);
    CHECK(log.With({"the repel ladder on Le Port (1 repel(s))", "at least 8 infantry"}).size() == 1);
    RepelOrder(f, launcher, op2);
    CHECK(f.of.Repels("Le Port") == 2);
    CHECK(log.With({"Le Port: second repel", "next attack from a different source or by air"}).size() == 1);

    // op 3: a different approach (not op 2's package from the Airfield), and
    // an escalation strike while the pool stays STRETCHED (a richer pool:
    // the ground answer and the 23-unit jet would leave 60 THIN)
    RestOut(f, 2400.0f);
    REQUIRE_FALSE(f.of.Urgent("Le Port"));
    const DirectorAction a2 = f.of.PlanFor(f.registry, ViewOf(f.registry), lePort, CAODirector, -1, 0, true);
    REQUIRE(a2.go);
    CHECK(a2.escalation == 2);
    CHECK(a2.approach.GetLength() > 0);
    CHECK_FALSE((a2.choice.quote.source == airfield && a2.choice.quote.package == op2.package));
    f.of.SetPool(90);
    for (int tick = 0; tick < 6 && GroundOn(launcher, lePort).size() < 3; tick++)
    {
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f); // massing may hold a few ticks
    }
    REQUIRE(GroundOn(launcher, lePort).size() == 3);
    const CounterAttackOrder op3 = GroundOn(launcher, lePort)[2];
    CHECK_FALSE((op3.source == op2.source && op3.package == op2.package));
    CHECK(log.With({"the repel ladder on Le Port (2 repel(s))", "a different approach"}).size() == 1);
    const CounterAttackOrder* strike = FirstKind(launcher, CAKStrike);
    REQUIRE(strike != nullptr);
    CHECK(strike->target == lePort);
    std::string strikeLines;
    for (const std::string& l : log.With({"strike"}))
    {
        strikeLines += l + " | ";
    }
    INFO(strikeLines);
    CHECK(log.With({"escalation strike: Le Port repelled 2 time(s)"}).size() == 1);
    CHECK((f.of.Band() == OFBandStrong || f.of.Band() == OFBandStretched));

    // repel 3: contained; the Harbour (nearest) surged by the 8 the next
    // operation would have needed, paid by the pool
    const float harbourBefore = f.Z("Le Port Harbour").garrison + (float)f.Z("Le Port Harbour").pending;
    const int poolBefore = f.of.Pool();
    RepelOrder(f, launcher, op3);
    CHECK(f.of.Repels("Le Port") == 3);
    CHECK(f.of.Contained("Le Port"));
    REQUIRE(log.With({"Le Port: third repel", "contained", "Le Port Harbour surged by 8"}).size() == 1);
    CHECK(f.Z("Le Port Harbour").garrison + (float)f.Z("Le Port Harbour").pending == Approx(harbourBefore + 8.0f));
    CHECK(f.of.Pool() == poolBefore + op3.priceUnits - 8);

    // contained: no candidate, no push-out, no launch while it is reported
    RestOut(f, 2400.0f);
    AutoArray<int> c;
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    CHECK_FALSE(HasCandidate(c, lePort));
    CHECK(f.of.PushoutTarget(f.registry, ViewOf(f.registry), launcher) != lePort);
    const size_t before = GroundOn(launcher, lePort).size();
    for (int tick = 0; tick < 10; tick++)
    {
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    }
    CHECK(GroundOn(launcher, lePort).size() == before);
    CHECK_FALSE(f.of.Rearmed("Le Port"));

    // the reports decay: the containment lifts (the memory stays)
    f.of.Tick(f.registry, f.of.Tuning().intelDecay + 60.0f);
    RestOut(f, 60.0f);
    CHECK_FALSE(f.of.Contained("Le Port"));
    CHECK(f.of.Repels("Le Port") == 3);
    CHECK(log.With({"Le Port: containment lifted: its reports decayed"}).size() == 1);

    // the zone changes hands: the ladder resets
    f.Z("Le Port").owner = f.registry.OccupierSide();
    RestOut(f, 60.0f);
    CHECK(f.of.Repels("Le Port") == 0);
    CHECK(f.of.RepelRows() == 0);
    CHECK(log.With({"Le Port changed hands: the repel ladder resets (3 repel(s))"}).size() == 1);
}

TEST_CASE("OccupierForces - video 5 replayed: Le Port and Arudy escalate, change approach, then are contained",
          "[game][guerrilla][occupierforces]")
{
    // tmp/of-director-video-5/trace.json: the director sent Le Port the same
    // Ural + T72 from the Harbour seven times (repelled 300 s after each
    // launch) and Arudy the same BMP from the Outpost five times (460 s),
    // every rest exactly 1800 s.  The same sequence of repels now climbs the
    // ladder, and both towns end contained
    CoreLines log;
    Fixture f;
    Video5Setup(f);
    const int lePort = f.registry.FindZoneIndex("Le Port");
    const int arudy = f.registry.FindZoneIndex("Arudy");
    FakeLauncher launcher;
    struct Pending
    {
        CounterAttackOrder order;
        float at;
    };
    std::vector<Pending> pending;
    std::vector<float> lePortAt;
    std::vector<float> arudyAt;
    for (float t = 0; t <= 4.0f * 3600.0f; t += 60.0f)
    {
        for (int i = 0; i < f.registry.NZones(); i++)
        {
            if (stricmp(f.registry.GetZone(i)->owner, f.registry.ResistanceSide()) == 0)
            {
                f.registry.HeatDecay(i, 1.5f); // gmHeatDecayHeld
            }
        }
        if (fmodf(t, 240.0f) < 1.0f)
        {
            // locals keep talking (7.3): the three and four fighters stay reported
            f.of.IngestTip(f.registry, lePort, 4, f.Z("Le Port").pos);
            f.of.IngestTip(f.registry, arudy, 3, f.Z("Arudy").pos);
        }
        for (size_t k = 0; k < pending.size();)
        {
            if (t >= pending[k].at)
            {
                if (pending[k].order.kind == CAKStrike)
                {
                    CounterAttackOpEvent ev;
                    ev.opId = pending[k].order.opId;
                    ev.type = CAOpHullRefund;
                    ev.kind = CAKStrike;
                    ev.target = pending[k].order.target;
                    ev.units = pending[k].order.priceUnits;
                    f.of.NoteOpEvent(f.registry, ev); // the ground record stays until its repel
                }
                else
                {
                    RepelOrder(f, launcher, pending[k].order);
                }
                pending.erase(pending.begin() + (long)k);
                continue;
            }
            k++;
        }
        const size_t n = launcher.orders.size();
        f.of.Tick(f.registry, 60.0f);
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        for (size_t k = n; k < launcher.orders.size(); k++)
        {
            const CounterAttackOrder& o = launcher.orders[k];
            if (o.kind == CAKStrike)
            {
                pending.push_back({o, t + 120.0f});
                continue;
            }
            REQUIRE((o.target == lePort || o.target == arudy));
            (o.target == lePort ? lePortAt : arudyAt).push_back(t);
            pending.push_back({o, t + (o.target == lePort ? 300.0f : 460.0f)});
        }
    }
    for (const int zone : {lePort, arudy})
    {
        const char* name = zone == lePort ? "Le Port" : "Arudy";
        const int fighters = zone == lePort ? 4 : 3;
        const std::vector<CounterAttackOrder> ops = GroundOn(launcher, zone);
        INFO(name);
        // three attacks, then nothing: contained for the rest of the run
        REQUIRE(ops.size() == 3);
        CHECK(f.of.Repels(name) == 3);
        CHECK(f.of.Contained(name));
        // escalate: the second brings twice the fighters as infantry, or
        // the director massed for it first
        CHECK((ops[1].bodies >= 2 * fighters || !log.With({"massing for ", name}).empty()));
        // change approach: the third is not the second again
        CHECK_FALSE((ops[2].source == ops[1].source && ops[2].package == ops[1].package));
        const std::string first = std::string(name) + ": first repel";
        const std::string second = std::string(name) + ": second repel";
        const std::string third = std::string(name) + ": third repel";
        CHECK(log.With({first.c_str()}).size() == 1);
        CHECK(log.With({second.c_str()}).size() == 1);
        CHECK(log.With({third.c_str(), "contained"}).size() == 1);
    }
    // the rests carry the jitter: no two answers 1800 s + a token apart to
    // the second (video 5: Le Port 241, 2643, 4565, 6487, 8409, ...)
    REQUIRE(lePortAt.size() == 3);
    REQUIRE(arudyAt.size() == 3);
    CHECK(lePortAt[2] - lePortAt[1] != Approx(lePortAt[1] - lePortAt[0]).margin(30.0f));
    // the strongest town first: Le Port, by its Harbour, won the tie at 40
    CHECK(lePortAt[0] == Approx(0.0f));
}

TEST_CASE("OccupierForces - massing: an escalated need above every base's spare holds the token",
          "[game][guerrilla][occupierforces]")
{
    auto setup = [](Fixture& f)
    {
        Video5Setup(f);
        f.of.ForceCooldown("Arudy", 1.0e7f);
        f.of.ForceRepels("Le Port", 1); // the ladder: 8 infantry for 4 fighters
        f.Z("Airfield").garrison = 8;   // spares 5; the Harbour 6, the Outpost 5
    };
    const int massingMax = OccupierForcesTuning().massingMax;
    REQUIRE(massingMax == 3);
    SECTION("held forcesMassingMax ticks, then the strongest package goes")
    {
        CoreLines log;
        Fixture f;
        setup(f);
        const int lePort = f.registry.FindZoneIndex("Le Port");
        FakeLauncher launcher;
        const int tokens = f.of.Tokens();
        for (int tick = 1; tick <= massingMax; tick++)
        {
            f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
            CHECK(launcher.orders.empty());
            CHECK(Str(f.of.MassingTarget()) == "Le Port");
            CHECK(f.of.MassingTicks() == tick);
            CHECK(f.of.Tokens() == tokens); // kept, not consumed
        }
        CHECK(log.With({"massing for Le Port: need 8 infantry, best source spares 6"}).size() == (size_t)massingMax);
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        REQUIRE(GroundOn(launcher, lePort).size() == 1);
        CHECK(GroundOn(launcher, lePort)[0].bodies == 6);
        CHECK(GroundOn(launcher, lePort)[0].source == f.registry.FindZoneIndex("Le Port Harbour"));
        CHECK(log.With({"massing for Le Port ends", "forcesMassingMax ticks held"}).size() == 1);
        CHECK(Str(f.of.MassingTarget()).empty());
    }
    SECTION("a base that can staff it ends the hold")
    {
        CoreLines log;
        Fixture f;
        setup(f);
        FakeLauncher launcher;
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        REQUIRE(launcher.orders.empty());
        f.Z("Airfield").garrison = 12; // regrown: spares 9
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        REQUIRE(launcher.orders.size() >= 1);
        CHECK(launcher.orders[0].bodies >= 8);
        CHECK(launcher.orders[0].source == f.registry.FindZoneIndex("Airfield"));
        CHECK(log.With({"massing for Le Port ends: Airfield can staff it"}).size() == 1);
    }
    SECTION("a further token ends the hold: the largest package goes")
    {
        CoreLines log;
        Fixture f;
        setup(f);
        f.of.TuningMutable().tokensMax = 1; // the purse trims to one
        FakeLauncher launcher;
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        REQUIRE(launcher.orders.empty());
        REQUIRE(f.of.Tokens() == 1);
        f.of.TuningMutable().tokensMax = 2;
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 600.0f);
        REQUIRE(launcher.orders.size() >= 1);
        CHECK(launcher.orders[0].bodies == 6);
        CHECK(log.With({"massing for Le Port ends", "a further token"}).size() == 1);
    }
    SECTION("never on a first attempt; massingMax 0 never holds")
    {
        Fixture f;
        setup(f);
        f.of.ForceRepels("Le Port", 0);
        FakeLauncher launcher;
        f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
        CHECK(launcher.orders.size() >= 1);
        Fixture g;
        setup(g);
        g.of.TuningMutable().massingMax = 0;
        FakeLauncher l2;
        g.of.Direct(g.registry, ViewOf(g.registry), l2, 60.0f);
        REQUIRE(l2.orders.size() >= 1);
        CHECK(l2.orders[0].bodies == 6);
    }
}

TEST_CASE("OccupierForces - candidates by value within the same urgency and heat: bases, towns by a base, far towns",
          "[game][guerrilla][occupierforces]")
{
    CoreLines log;
    Fixture f;
    Video5Setup(f);
    const int lePort = f.registry.FindZoneIndex("Le Port");
    const int arudy = f.registry.FindZoneIndex("Arudy");
    const int outpost = f.registry.FindZoneIndex("Outpost");
    CHECK(f.of.CandidateValue(f.registry, lePort) == OccupierForces::ValueAdjacentTown); // 0.5 km from the Harbour
    CHECK(f.of.CandidateValue(f.registry, arudy) == OccupierForces::ValueFarTown);       // 2.2 km from the Outpost
    CHECK(f.of.CandidateValue(f.registry, outpost) == OccupierForces::ValueBase);
    CHECK(f.of.CandidateValue(f.registry, OccupierForces::HqCandidate) == OccupierForces::ValueHq);
    FakeLauncher launcher;
    AutoArray<int> c;
    // video 5: both re-armed at 40; the town by the Harbour first
    f.Z("Le Port").heat = 40;
    f.Z("Arudy").heat = 40.6f;
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    REQUIRE(c.Size() == 2);
    CHECK(c[0] == lePort);
    CHECK(c[1] == arudy);
    // a whole point hotter wins over value
    f.Z("Arudy").heat = 41.2f;
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    CHECK(c[0] == arudy);
    // urgency first: an alert edge on Le Port
    f.Z("Arudy").heat = 60;
    AutoArray<int> states;
    states.Resize(f.registry.NZones());
    for (int i = 0; i < states.Size(); i++)
    {
        states[i] = 0;
    }
    f.of.NoteAlertStates(f.registry, states);
    states[lePort] = 1;
    f.of.NoteAlertStates(f.registry, states);
    REQUIRE(f.of.Urgent("Le Port"));
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    CHECK(c[0] == lePort);
    // a held base at the same heat goes before both towns (STRONG)
    states[lePort] = 0;
    f.of.NoteAlertStates(f.registry, states);
    f.Z("Le Port").heat = 30; // below the urgency exit: released next step
    f.Z("Arudy").heat = 40;
    RestOut(f, 0.0f);
    REQUIRE_FALSE(f.of.Urgent("Le Port"));
    REQUIRE_FALSE(f.of.Urgent("Arudy"));
    f.Z("Le Port").heat = 40;
    f.Z("Arudy").heat = 40;
    f.GiveToResistance("Outpost");
    f.Z("Outpost").heat = 40;
    f.of.Candidates(f.registry, ViewOf(f.registry), launcher, c);
    REQUIRE(c.Size() == 3);
    CHECK(c[0] == outpost);
    // the launch names the candidate's weight
    FakeLauncher l2;
    f.of.Direct(f.registry, ViewOf(f.registry), l2, 60.0f);
    REQUIRE(l2.orders.size() >= 1);
    CHECK(log.With({"candidate Outpost, 1 of 3: a base (weight 3), heat 40"}).size() == 1);
}

TEST_CASE("OccupierForces - the rest carries a hashed jitter of +-20 %; 0 is the old clockwork",
          "[game][guerrilla][occupierforces]")
{
    OccupierForcesTuning t;
    CHECK(t.restJitter == Approx(0.2f));
    float lo = 1.0e9f;
    float hi = 0;
    for (int op = 1; op <= 200; op++)
    {
        const float r = OccupierForces::JitteredRest(t, "Le Port", op);
        CHECK(r >= 1440.0f - 0.01f);
        CHECK(r <= 2160.0f + 0.01f);
        CHECK(r == OccupierForces::JitteredRest(t, "Le Port", op)); // deterministic
        lo = r < lo ? r : lo;
        hi = r > hi ? r : hi;
    }
    CHECK(hi - lo > 500.0f); // it spreads
    CHECK(OccupierForces::JitteredRest(t, "Le Port", 1) != OccupierForces::JitteredRest(t, "Arudy", 1));
    t.restJitter = 0;
    CHECK(OccupierForces::JitteredRest(t, "Le Port", 7) == Approx(1800.0f));
    // a repel rests the town for its jittered rest, named in the line
    CoreLines log;
    Fixture f;
    Video5Setup(f);
    f.of.ForceCooldown("Arudy", 1.0e7f);
    FakeLauncher launcher;
    f.of.Direct(f.registry, ViewOf(f.registry), launcher, 60.0f);
    REQUIRE(launcher.orders.size() >= 1);
    const CounterAttackOrder o = launcher.orders[0];
    RepelOrder(f, launcher, o);
    const float rest = OccupierForces::JitteredRest(f.of.Tuning(), "Le Port", o.opId);
    CHECK(f.of.Cooldown("Le Port") == Approx(rest));
    char words[64];
    snprintf(words, sizeof(words), "Le Port rests %.0f s", (double)rest);
    CHECK(log.With({words}).size() == 1);
}

TEST_CASE("OccupierForces - a wave strike takes the arrival rule: the HQ assault from the hub is not a walk",
          "[game][guerrilla][occupierforces][air]")
{
    // video 5, op 7: "Wave 1 strike: HQ assault (foot 6) from Le Port
    // Harbour on Camp (..., road 2450 s)": a 41-minute walk from the harbour
    // that had just landed the wave
    for (const bool rule : {true, false})
    {
        Fixture f;
        Video5Setup(f);
        if (!rule)
        {
            f.of.TuningMutable().slowEta = 0; // the arrival rule off: the old plain choice
            f.of.TuningMutable().airEta = 0;
        }
        f.of.TuningMutable().waveLead = 0;
        f.of.TuningMutable().tokensMax = 0; // the wave alone
        DirectorView v = ViewOf(f.registry);
        v.hqEstablished = true;
        v.hqZone = f.registry.FindZoneIndex("Camp");
        v.hqPos = f.Z("Camp").pos;
        FakeLauncher launcher;
        f.of.RequestWave(f.registry, v, 1);
        f.of.Direct(f.registry, v, launcher, 60.0f);
        REQUIRE(f.of.WavesLanded() == 1);
        const CounterAttackOrder* hq = FirstKind(launcher, CAKHqAssault);
        REQUIRE(hq != nullptr);
        CHECK(hq->origin == CAOWave);
        CHECK(hq->source == f.registry.FindZoneIndex("Airfield")); // the hub
        // the Airfield is 4.1 km from the Camp: about 4150 s on foot
        if (rule)
        {
            CHECK(hq->package != FPFoot);
        }
        else
        {
            CHECK(hq->package == FPFoot);
        }
    }
}

TEST_CASE("OccupierForces - containment lifts on a wave and on a control band; the ladder survives save/load",
          "[game][guerrilla][occupierforces][save][load]")
{
    const std::string config = Video5Config();
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    CoreLines log;
    Fixture f;
    Video5Setup(f);
    f.of.ForceRepels("Le Port", 3);
    f.of.ForceRepels("Arudy", 2);
    REQUIRE(f.of.Contained("Le Port"));
    REQUIRE_FALSE(f.of.Contained("Arudy"));
    // saved with the block
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "occupier-forces-repels.bin";
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    OccupierForces loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(loaded.RepelRows() == 2);
    CHECK(loaded.Repels("Le Port") == 3);
    CHECK(loaded.Contained("Le Port"));
    CHECK(loaded.Repels("Arudy") == 2);
    CHECK_FALSE(loaded.Contained("Arudy"));
    ExtParsMission.Clear();
    std::filesystem::remove(path);

    // a wave fires: the parent nation sends more, contained zones are weighed again
    DirectorView v = ViewOf(f.registry);
    f.of.RequestWave(f.registry, v, 1);
    CHECK_FALSE(f.of.Contained("Le Port"));
    CHECK(f.of.Repels("Le Port") == 3);
    CHECK(log.With({"Le Port: containment lifted: a wave fired"}).size() == 1);

    // control crosses a wave threshold: lifted too
    Fixture g;
    Video5Setup(g);
    g.of.Tick(g.registry, 60.0f); // the control band is known (0.89: above 0.75)
    g.of.ForceRepels("Arudy", 3);
    REQUIRE(g.of.Contained("Arudy"));
    g.of.Tick(g.registry, 60.0f);
    REQUIRE(g.of.Contained("Arudy"));
    g.GiveToResistance("Airfield"); // M 14 / 26: control about 0.61
    g.of.Tick(g.registry, 60.0f);
    CHECK(g.of.Control() < 0.75f);
    CHECK_FALSE(g.of.Contained("Arudy"));
    CHECK(log.With({"Arudy: containment lifted: control", "crossed a wave threshold"}).size() == 1);
}

TEST_CASE("OccupierForces - the escalation strike keeps STRETCHED, never THIN",
          "[game][guerrilla][occupierforces][air]")
{
    Fixture f;
    Video5Setup(f);
    const int lePort = f.registry.FindZoneIndex("Le Port");
    const DirectorView v = ViewOf(f.registry);
    // not before the second repel
    CHECK_FALSE(f.of.EscalationStrikeDue(f.registry, v, lePort));
    f.of.ForceRepels("Le Port", 2);
    CHECK(f.of.EscalationStrikeDue(f.registry, v, lePort));
    // STRETCHED (35 of 60): the 23-unit jet would leave 12, THIN: refused
    // with its own words
    f.of.SetPool(35);
    REQUIRE(f.of.Band() == OFBandStretched);
    CHECK(f.of.EscalationStrikeDue(f.registry, v, lePort)); // STRETCHED may still strike
    const DirectorAction thin = f.of.PlanStrike(f.registry, v, lePort, true, true, true);
    CHECK_FALSE(thin.go);
    CHECK(Str(thin.choice.why) == OccupierForces::StrikeKeepsStretchedWhy);
    f.of.SetPool(45); // 45 - 23 = 22: STRETCHED
    const DirectorAction ok = f.of.PlanStrike(f.registry, v, lePort, true, true, true);
    REQUIRE(ok.go);
    CHECK(ok.choice.quote.package == FPJet);
    // the plain rule (keepStrong only) refuses the same strike
    CHECK_FALSE(f.of.PlanStrike(f.registry, v, lePort, true, true).go);
    // THIN: no escalation strike at all
    f.of.SetPool(15);
    REQUIRE(f.of.Band() == OFBandThin);
    CHECK_FALSE(f.of.EscalationStrikeDue(f.registry, v, lePort));
    // fewer than StrikeMinFighters (3) reported: none
    Fixture g;
    Video5Setup(g);
    const int arudy = g.registry.FindZoneIndex("Arudy");
    g.of.ForceRepels("Arudy", 2);
    CHECK(g.of.EscalationStrikeDue(g.registry, ViewOf(g.registry), arudy)); // 3 reported
    g.of.Tick(g.registry, g.of.Tuning().intelDecay + 60.0f);
    RestOut(g, 60.0f); // the reports decay
    g.of.IngestTip(g.registry, arudy, 2, g.Z("Arudy").pos);
    REQUIRE(g.of.KnownFighters(g.registry, arudy) == 2);
    CHECK_FALSE(g.of.EscalationStrikeDue(g.registry, ViewOf(g.registry), arudy));
}

TEST_CASE("OccupierForces - generated bases: in the roster and R, their garrisons out of poolStart, waves unchanged",
          "[game][guerrilla][occupierforces][save][load]")
{
    // OCCUPIER-FORCES.md 5.7, the owner's conservation rule (2026-10-16):
    // presence, not strength.  The Abel-sized theatre with one of its four
    // bases generated: authored R 24, Rgen 8, K 14
    const std::string config = AbelLikeConfig();
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    f.Load(config);
    f.Z("Base3").generated = true;
    REQUIRE(CampaignEnd::IsGeneratedOccupier(f.Z("Base3"), f.registry));
    REQUIRE_FALSE(CampaignEnd::IsAuthoredOccupier(f.Z("Base3"), f.registry));
    OccupierForcesSizing s;
    OccupierForces::Size(f.registry, f.of.Tuning(), s);
    CHECK(s.R == 32);
    CHECK(s.Rgen == 8);
    CHECK(s.roster.Size() == 4);
    // max(24, 24 + 2 x 14) - 8 = 44
    CHECK(s.poolStart == 44);
    REQUIRE(f.of.Activate(f.registry));
    CHECK(f.of.PoolStart() == 44);
    CHECK(f.of.RosterStrength() == 32);
    CHECK(f.of.GeneratedStrength() == 8);
    CHECK(f.of.RosterCount() == 4);
    // the refill is unchanged: (44 + 32 + 56) / 4 = 33 an hour, as
    // (52 + 24 + 56) / 4 without the generated base
    CHECK(OccupierForces::RefillPerHour(f.of.PoolStart(), f.of.RosterStrength(), f.of.TownGarrisons(),
                                        f.of.Tuning().refillHours, 50.0f) == Approx(33.0f));
    // a wave stays a share of poolStart + Rgen: 0.5 x 52 = 26
    f.of.RequestWave(f.registry, ViewOf(f.registry), 1);
    CHECK(f.of.WaveBodies() == 26);
    // Rgen is saved with the block
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "occupier-forces-rgen.bin";
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    OccupierForces loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(loaded.GeneratedStrength() == 8);
    CHECK(loaded.PoolStart() == 44);
    ExtParsMission.Clear();
    std::filesystem::remove(path);
}

// ---------------------------------------------------------------------------
// the ledger of 2026-10-16 (decisions 5, 6, 9; the feel assessment's item 10):
// the wave surge, committed airframes, the loss window, the hull flag
// ---------------------------------------------------------------------------

namespace
{
// stored reserve plus pending over every zone (the 5.5 reserves + pending)
int HeldOf(Fixture& f)
{
    int n = 0;
    for (int i = 0; i < f.registry.NZones(); i++)
    {
        const ZoneRecord* z = f.registry.GetZone(i);
        n += (int)floorf(z->garrison > 0 ? z->garrison : 0) + z->pending;
    }
    return n;
}
void FlushAll(Fixture& f)
{
    for (int i = 0; i < f.registry.NZones(); i++)
    {
        GarrisonCache::FlushPending(f.registry, i, true, 99999.0f, 99999.0f, nullptr);
    }
}
CounterAttackOpEvent LedgerEvent(int opId, int type, int units, int source = -1)
{
    CounterAttackOpEvent ev;
    ev.opId = opId;
    ev.type = type;
    ev.kind = CAKRetake;
    ev.origin = CAODirector;
    ev.units = units;
    ev.bodies = type == CAOpCrewSettled ? units : 0;
    ev.source = source;
    return ev;
}
} // namespace

TEST_CASE("OccupierForces - the wave surge: the excess over the ceiling thickens the bases near the wave's cluster, "
          "then decays with nothing dropped",
          "[game][guerrilla][occupierforces]")
{
    // video 5 (tmp/of-director-video-5): after a wave every base sat at
    // garrisonMax and the survivors who came home over the ceiling were
    // dropped (9 dropped, 1 banked).  Decision 5: while the wave is on, the
    // excess is a surge above garrisonMax at the bases nearest the wave's
    // cluster, which decays back to garrisonMax over surgeDecayHours: the
    // bodies rejoin the pool up to the ceiling and the rest stand down
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart(), R = f.of.RosterStrength(), G = f.of.TownGarrisons();
    REQUIRE(P == 60);
    REQUIRE(HeldOf(f) == R + G); // every base and town at its garrisonMax
    int op = 0;
    auto check = [&]()
    {
        CHECK(OccupierForces::ConservationCheck(f.of.Pool(), HeldOf(f), 0, op, P, R, G, f.of.RefillTotal(),
                                                f.of.WavesTotal(),
                                                f.of.Lost() + f.of.Dropped() + f.of.StoodDown()));
    };
    CHECK_FALSE(f.of.SurgeOpen());
    f.of.LandWave(10);
    REQUIRE(f.of.Pool() == P + 10);
    CHECK(f.of.SurgeOpen());
    CHECK(f.of.SurgeWindowLeft() == Approx(OccupierForces::WaveWindow));
    // the wave's cluster is around Base3 (AllocateWave anchors it there)
    f.of.SetSurgeAnchor(f.Z("Base3").pos);
    check();
    // a wave strike of 20 goes out; the refill tops the pool up to poolStart
    // meanwhile, so the 20 who come home meet a ceiling with room for 10
    REQUIRE(f.of.Spend(20, "wave strike"));
    op += 20;
    f.of.Tick(f.registry, 1000.0f);
    REQUIRE(f.of.Pool() == P);
    REQUIRE(f.of.SurgeOpen());
    check();
    CoreLines log;
    CHECK(f.of.Refund(20, "wave survivors home") == 10);
    op -= 20;
    CHECK(f.of.Pool() == P + 10);
    CHECK(f.of.PoolCeiling() == P + 10);
    // 0 dropped, nothing banked: 10 surge above garrisonMax, nearest the
    // cluster first, each base up to +50 % (4 over its 8)
    CHECK(f.of.Dropped() == 0);
    CHECK(f.of.Banked() == 0);
    CHECK(f.of.SurgeTotal() == 10);
    CHECK(f.Z("Base3").pending == 4);
    CHECK(f.Z("Base2").pending == 4);
    CHECK(f.Z("Base1").pending == 2);
    CHECK(f.Z("Base0").pending == 0);
    CHECK(log.With({"surge: 4 unit(s) over poolStart at Base3", "above its garrisonMax"}).size() == 1);
    CHECK(log.With({"over poolStart dropped"}).empty());
    check();
    // the surge joins the reserves once the player is away: bases visibly
    // thicker after the wave
    FlushAll(f);
    CHECK(f.Z("Base3").garrison == Approx(12.0f));
    check();
    // it decays back over about an hour; the pool sits at its ceiling, so
    // every decayed body stands down
    for (int k = 0; k < 70; k++)
    {
        f.of.Tick(f.registry, 60.0f);
        check();
        CHECK(f.of.Pool() <= f.of.PoolCeiling());
    }
    CHECK(f.of.SurgeTotal() == 0);
    CHECK(f.of.SurgeRows() == 0);
    CHECK(f.of.StoodDown() == 10);
    CHECK(f.of.Dropped() == 0);
    CHECK(f.Z("Base3").garrison == Approx(8.0f));
    CHECK(f.Z("Base2").garrison == Approx(8.0f));
    CHECK(f.Z("Base1").garrison == Approx(8.0f));
    CHECK(!log.With({"surge:", "unit(s) stood down at Base3"}).empty());
    check();
    // the wave is over (the window closed): an excess banks or drops again
    CHECK_FALSE(f.of.SurgeOpen());
    CHECK(f.of.Refund(3, "surplus") == 0);
    CHECK(f.of.SurgeTotal() == 0);
    CHECK(f.of.Dropped() == 3);
}

TEST_CASE("OccupierForces - the wave surge rejoins the pool up to the ceiling, the rest stands down; a lost base "
          "ends it",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart();
    f.of.TuningMutable().surgeDecayHours = 0.5f;
    f.of.LandWave(8);
    f.of.SetSurgeAnchor(f.Z("Base0").pos);
    // the pool at its ceiling: an 8-unit refund is all excess
    CHECK(f.of.Refund(8, "wave survivors home") == 0);
    CHECK(f.Z("Base0").pending == 4);
    CHECK(f.Z("Base1").pending == 4);
    FlushAll(f);
    // 5 of the pool's wave strength are spent and lost: the decay brings 5
    // back to the ceiling first, the other 3 stand down
    REQUIRE(f.of.Spend(5, "column"));
    f.of.NoteLoss(5, "column lost");
    REQUIRE(f.of.PoolCeiling() == P + 8); // the pool's own overshoot keeps the headroom
    for (int k = 0; k < 40; k++)
    {
        f.of.Tick(f.registry, 60.0f);
        CHECK(f.of.Pool() <= f.of.PoolCeiling());
    }
    CHECK(f.of.SurgeTotal() == 0);
    CHECK(f.of.Pool() == P + 8);
    CHECK(f.of.StoodDown() == 3);
    CHECK(f.of.Dropped() == 0);
    // a surge at a base the occupier loses ends with it (the bodies follow
    // the zone's own flip rules)
    f.of.LandWave(4);
    f.of.SetSurgeAnchor(f.Z("Base3").pos);
    f.of.SetPool(f.of.PoolCeiling());
    CHECK(f.of.Refund(4, "survivors") == 0);
    REQUIRE(f.of.SurgeRows() == 1);
    CHECK(Str(f.of.SurgeRowZone(0)) == "Base3");
    f.GiveToResistance("Base3");
    f.of.Tick(f.registry, 60.0f);
    CHECK(f.of.SurgeRows() == 0);
}

TEST_CASE("OccupierForces - a sortie does not demote the band; a shot-down airframe does",
          "[game][guerrilla][occupierforces]")
{
    // video 5: two Mi17s took the band from STRONG to STRETCHED for 100 s
    // while they were out.  Decision 6: an airframe out is committed, not
    // spent: the band is stepped on the pool plus the airframes the ledger
    // rows still hold, and only a hull lost moves it
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart();
    f.of.SetPool(40); // 66.7 %: STRONG, 4 units above its exit
    REQUIRE(f.of.Band() == OFBandStrong);
    // the old arithmetic: an 18-unit insertion on 40 would read STRETCHED
    CHECK(OccupierForces::StepBand(OFBandStrong, 40 - 18, P) == OFBandStretched);
    CoreLines log;
    {
        // the launch: the debit and the ledger row are one movement
        OccupierForces::BandHold hold(f.of);
        REQUIRE(f.of.Spend(18, "Mi17 and crew"));
        REQUIRE(f.of.BookLeverOp(101, "T0", CAKRetake, FPTransportHeli, 18));
    }
    CHECK(f.of.Pool() == 22);
    CHECK(f.of.CommittedAir() == 18);
    CHECK(f.of.BandStrength() == 40);
    CHECK(f.of.Band() == OFBandStrong);
    f.of.NoteOpEvent(f.registry, LedgerEvent(101, CAOpAirborne, 0));
    f.of.NoteOpEvent(f.registry, LedgerEvent(101, CAOpLanded, 0));
    f.of.Tick(f.registry, 100.0f); // the refill tops the pool up a little
    CHECK(f.of.Band() == OFBandStrong);
    // home: the crew settle as bodies, the hull is refunded
    f.of.NoteOpEvent(f.registry, LedgerEvent(101, CAOpCrewSettled, 2));
    f.of.NoteOpEvent(f.registry, LedgerEvent(101, CAOpHullRefund, 16, f.registry.FindZoneIndex("Base0")));
    CHECK(f.of.CommittedAir() == 0);
    CHECK(f.of.Band() == OFBandStrong);
    CHECK(log.With({"band STRONG -> STRETCHED"}).empty());
    // a second sortie is shot down: the hull is a real loss and demotes
    f.of.SetPool(40);
    {
        OccupierForces::BandHold hold(f.of);
        REQUIRE(f.of.Spend(18, "Mi17 and crew"));
        REQUIRE(f.of.BookLeverOp(102, "T1", CAKRetake, FPTransportHeli, 18));
    }
    CHECK(f.of.Band() == OFBandStrong);
    f.of.NoteOpEvent(f.registry, LedgerEvent(102, CAOpHullLost, 16));
    CHECK(f.of.CommittedAir() == 2); // the crew, still to settle
    CHECK(f.of.LossDebt() == 16);
    CHECK(f.of.BandStrength() == 24);
    CHECK(f.of.Band() == OFBandStretched);
    CHECK(log.With({"band STRONG -> STRETCHED", "pool 22 of 60"}).size() == 1);
    // a ground hull out is not committed: a column still demotes as before
    CHECK(OccupierForces::BandStrengthOf(22, P, 0, 0) == 22);
}

TEST_CASE("OccupierForces - the band reads losses: 37 killed in an hour on a Malden-sized pool drop it to "
          "STRETCHED while the pool says STRONG",
          "[game][guerrilla][occupierforces]")
{
    // the feel assessment, item 10 (the levers audit: "refill beats the
    // bands", STRONG 99 % of video 5).  The Abel-like fixture is Malden's
    // arithmetic: R 32, K 14, G 56, poolStart 60, refill 37 an hour.  The
    // band is stepped on min(pool, poolStart - lost + repaid) over the last
    // lossWindowHours: 37 killed in an hour while D2 regrowth repays 12 of
    // them (two bases at 1 body per 600 s) leaves strength 35 of 60
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart();
    REQUIRE(P == 60);
    REQUIRE(f.of.Tuning().lossWindowHours == Approx(1.0f));
    CHECK(OccupierForces::RefillPerHour(P, f.of.RosterStrength(), f.of.TownGarrisons(), 4.0f, 50.0f) ==
          Approx(37.0f));
    CoreLines log;
    int killed = 0;
    int bandAt = -1; // the kill that first left STRONG
    f.of.NoteLoss(1, "garrison");
    killed++;
    for (int step = 0; step < 36; step++)
    {
        if (step % 6 == 0)
        {
            CHECK(f.of.Draw(2) == 2); // D2 regrowth at two bases
        }
        f.of.Tick(f.registry, 100.0f);
        f.of.NoteLoss(1, "garrison");
        killed++;
        if (bandAt < 0 && f.of.Band() != OFBandStrong)
        {
            bandAt = killed;
        }
    }
    CHECK(killed == 37);
    CHECK(f.of.LossesInWindow() == 37);
    CHECK(f.of.RepaidInWindow() == 12);
    CHECK(f.of.BandStrength() == 35);
    CHECK(f.of.Band() == OFBandStretched);
    // the pool number alone still says STRONG
    CHECK(f.of.Pool() >= 58);
    CHECK(OccupierForces::InitialBand(f.of.Pool(), P) == OFBandStrong);
    CHECK(bandAt == 37); // 25 unrepaid is the first strength under 60 %
    CHECK(log.With({"band STRONG -> STRETCHED", "lost"}).size() == 1);
    // the losses age out of the window: the band recovers
    f.of.Tick(f.registry, 3700.0f);
    CHECK(f.of.LossesInWindow() == 0);
    CHECK(f.of.BandStrength() == f.of.Pool());
    CHECK(f.of.Band() == OFBandStrong);
    // the pure rule
    CHECK(OccupierForces::BandStrengthOf(60, 60, 0, 25) == 35);
    CHECK(OccupierForces::BandStrengthOf(40, 60, 18, 0) == 58);
    CHECK(OccupierForces::BandStrengthOf(70, 60, 0, 25) == 45); // a wave's overshoot is strength on top
    CHECK(OccupierForces::BandStrengthOf(10, 60, 0, 80) == 0);
}

TEST_CASE("OccupierForces - the hull flag: a hull home over the ceiling is a stored spare, not a garrison",
          "[game][guerrilla][occupierforces]")
{
    // video 5's "22 banked": an APC came home to a full pool and became 22
    // bodies of garrison.  Decision 9, forcesBankHulls 0 (the default): only
    // bodies bank; a hull over the ceiling is a stored spare hull at its base
    // (or the nearest base with a garage), back in the pool when it has room
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart();
    CHECK_FALSE(f.of.Tuning().bankHulls);
    const int b1 = f.registry.FindZoneIndex("Base1");
    f.Z("Base2").garrison = 0; // room to bank bodies, which a hull must not take
    REQUIRE(f.of.Spend(22, "APC and crew"));
    REQUIRE(f.of.BookLeverOp(201, "T0", CAKRetake, FPApc, 22));
    f.of.Tick(f.registry, 3600.0f); // the refill tops the pool back up
    REQUIRE(f.of.Pool() == P);
    CoreLines log;
    f.of.NoteOpEvent(f.registry, LedgerEvent(201, CAOpCrewSettled, 2));
    f.of.NoteOpEvent(f.registry, LedgerEvent(201, CAOpHullRefund, 20, b1));
    CHECK(f.of.Pool() == P);
    CHECK(f.of.Banked() == 0);
    CHECK(f.of.Dropped() == 0);
    CHECK(PendingOf(f) == 0);
    REQUIRE(f.of.StoredHullRows() == 1);
    CHECK(Str(f.of.StoredHullZone(0)) == "Base1");
    CHECK(f.of.StoredHullPackage(0) == FPApc);
    CHECK(f.of.StoredHullUnits(0) == 20);
    CHECK(f.of.StoredHullTotal() == 20);
    CHECK(log.With({"ledger: APC over poolStart stored as a spare hull at Base1", "20 unit(s)"}).size() == 1);
    // the pool spends 8: the stored APC fills it back first (before the refill)
    REQUIRE(f.of.Spend(8, "truck"));
    const int refill0 = f.of.RefillTotal();
    f.of.Tick(f.registry, 60.0f);
    CHECK(f.of.Pool() == P);
    CHECK(f.of.RefillTotal() == refill0);
    CHECK(f.of.StoredHullUnits(0) == 12);
    CHECK(log.With({"the spare APC stored at Base1 rejoins the pool", "8 unit(s)"}).size() == 1);
    // a base full of spares (StoredHullsPerBase): the next goes to the
    // nearest base with a garage
    REQUIRE(f.of.CanStoreHull(f.registry, b1));
    CHECK(f.of.RefundHull(f.registry, 5, FPTruck, "TRUCK", b1) == 0);
    CHECK_FALSE(f.of.CanStoreHull(f.registry, b1));
    CHECK(f.of.RefundHull(f.registry, 4, FPTruck, "TRUCK", b1) == 0);
    REQUIRE(f.of.StoredHullRows() == 3);
    const std::string third = Str(f.of.StoredHullZone(2));
    CHECK((third == "Base0" || third == "Base2"));
    // the base is lost: its spares are lost with it, a real loss
    const int lost0 = f.of.Lost();
    f.GiveToResistance("Base1");
    f.of.Tick(f.registry, 60.0f);
    CHECK(f.of.Lost() == lost0 + 12 + 5);
    CHECK(f.of.StoredHullRows() == 1);
    // no base can store one: dropped, with the line
    for (int i = 0; i < 4; i++)
    {
        f.GiveToResistance(("Base" + std::to_string(i)).c_str());
    }
    f.of.Tick(f.registry, 60.0f); // the last spare is lost with its base too
    CHECK(f.of.StoredHullRows() == 0);
    f.of.SetPool(P);
    const int dropped0 = f.of.Dropped();
    CHECK(f.of.RefundHull(f.registry, 6, FPTank, "TANK", -1) == 0);
    CHECK(f.of.Dropped() == dropped0 + 6);
    CHECK(log.With({"6 hull unit(s) over poolStart dropped", "TANK"}).size() == 1);
}

TEST_CASE("OccupierForces - the hull flag keeps the ledger conserved: stored hulls are pool units in hull form",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(AbelLikeConfig(), false);
    REQUIRE(f.of.Activate(f.registry));
    const int P = f.of.PoolStart(), R = f.of.RosterStrength(), G = f.of.TownGarrisons();
    int op = 0;
    auto check = [&]()
    {
        CHECK(OccupierForces::ConservationCheck(f.of.Pool() + f.of.StoredHullTotal(), HeldOf(f), 0, op, P, R, G,
                                                f.of.RefillTotal(), f.of.WavesTotal(),
                                                f.of.Lost() + f.of.Dropped() + f.of.StoodDown()));
    };
    check();
    REQUIRE(f.of.Spend(22, "APC and crew"));
    REQUIRE(f.of.BookLeverOp(211, "T0", CAKRetake, FPApc, 22));
    op += 22;
    f.of.Tick(f.registry, 3600.0f);
    check();
    // the crew are lost in the fight, the hull comes home over the ceiling
    f.of.NoteLoss(2, "crew");
    f.of.NoteOpEvent(f.registry, LedgerEvent(211, CAOpCrewSettled, 2));
    op -= 2;
    check();
    f.of.NoteOpEvent(f.registry, LedgerEvent(211, CAOpHullRefund, 20, f.registry.FindZoneIndex("Base1")));
    op -= 20;
    CHECK(f.of.StoredHullTotal() == 20);
    check();
    REQUIRE(f.of.Spend(15, "column"));
    op += 15;
    f.of.Tick(f.registry, 60.0f);
    CHECK(f.of.StoredHullTotal() == 5);
    check();
}

TEST_CASE("OccupierForces - forcesBankHulls 1 keeps the df9457e rule: a hull banks as bodies",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(ConfigWith("forcesBankHulls = 1;", AbelLikeConfig().c_str()), false);
    REQUIRE(f.of.Activate(f.registry));
    CHECK(f.of.Tuning().bankHulls);
    const int P = f.of.PoolStart();
    f.Z("Base2").garrison = 0;
    REQUIRE(f.of.Spend(22, "APC and crew"));
    REQUIRE(f.of.BookLeverOp(301, "T0", CAKRetake, FPApc, 22));
    f.of.Tick(f.registry, 3600.0f);
    REQUIRE(f.of.Pool() == P);
    f.of.NoteOpEvent(f.registry, LedgerEvent(301, CAOpHullRefund, 20, f.registry.FindZoneIndex("Base2")));
    CHECK(f.of.StoredHullRows() == 0);
    CHECK(f.of.Banked() == 8); // Base2's room; the rest dropped (every base full)
    CHECK(f.Z("Base2").pending == 8);
    CHECK(f.of.Dropped() == 12);
}

TEST_CASE("OccupierForces - the surge, the stored hulls and the loss window survive save/load",
          "[game][guerrilla][occupierforces][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path path = dir / "occupier-forces-ledger-2026-10-16.bin";
    const std::string config = AbelLikeConfig();
    {
        QIStream in(config.c_str(), config.size());
        ExtParsMission.Parse(in);
    }
    Fixture f;
    f.Load(config, false);
    REQUIRE(f.of.Activate(f.registry));
    f.of.LandWave(6);
    f.of.SetSurgeAnchor(f.Z("Base3").pos);
    REQUIRE(f.of.Refund(6, "wave survivors") == 0);
    REQUIRE(f.of.SurgeTotal() == 6);
    REQUIRE(f.of.RefundHull(f.registry, 9, FPTruck, "TRUCK", f.registry.FindZoneIndex("Base0")) == 0);
    f.of.NoteLoss(7, "repelled");
    REQUIRE(f.of.Spend(4, "op"));
    REQUIRE(f.of.BookLeverOp(401, "T0", CAKStrike, FPJet, 4));
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(f.of.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }
    OccupierForces loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(loaded.SurgeTotal() == 6);
    CHECK(loaded.SurgeRows() == f.of.SurgeRows());
    CHECK(loaded.SurgeOpen());
    CHECK(loaded.SurgeAnchor().X() == Approx(f.Z("Base3").pos.X()));
    REQUIRE(loaded.StoredHullRows() == 1);
    CHECK(Str(loaded.StoredHullZone(0)) == "Base0");
    CHECK(loaded.StoredHullUnits(0) == 9);
    CHECK(loaded.LossesInWindow() == 7);
    CHECK(loaded.CommittedAir() == 4);
    CHECK(loaded.BandStrength() == f.of.BandStrength());
    ExtParsMission.Clear();
    std::filesystem::remove(path);
}
