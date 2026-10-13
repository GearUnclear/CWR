// Occupier forces pool (OCCUPIER-FORCES.md phase 2, OccupierForces.hpp): the
// key and the inert service, the roster / K / G sizing with the exemption
// and the settlement probe, poolStart, the refill formula (the 4 h figure
// from full exhaustion with D2 drawing first, the 1.6 h best case, the
// standing factor), banded hysteresis (no SPENT / THIN flapping, one SPENT
// exit during recovery), Draw and the floor, control, the war-level split,
// the conservation ledger, the save round trip and the old-save derivation.
// Pure: the registry is built from inline config, the service is driven
// through Activate / Tick with no world.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp>
#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
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
        REQUIRE(registry.IsActive());
        const int hq = registry.FindZoneIndex("HqTown");
        if (exemptHq && hq >= 0)
        {
            registry.ExemptTownGarrison(hq);
        }
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
    CHECK(t.tokenInterval[OFBandStrong] == Approx(1800.0f));
    CHECK(t.tokenInterval[OFBandStretched] == Approx(2700.0f));
    CHECK(t.tokenInterval[OFBandThin] == Approx(3600.0f));
    CHECK(t.tokenInterval[OFBandSpent] == Approx(0.0f));
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

TEST_CASE("OccupierForces - key off: inert, the seams pass through, nothing to save",
          "[game][guerrilla][occupierforces]")
{
    Fixture f;
    f.Load(kForcesConfig);
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
    // the probe never touches an authored (unseeded) town
    seeded[f.registry.FindZoneIndex("Other")] = false;
    OccupierForces::Size(f.registry, f.of.Tuning(), s, &seeded, &probe);
    CHECK(s.K() == 2);
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
    CHECK(OccupierForces::TokenInterval(t, OFBandStrong) == Approx(1800.0f));
    CHECK(OccupierForces::TokenInterval(t, OFBandStretched) == Approx(2700.0f));
    CHECK(OccupierForces::TokenInterval(t, OFBandThin) == Approx(3600.0f));
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
    // refunds may push the pool above poolStart
    f.of.Refund(80, "surplus");
    CHECK(f.of.Pool() == 82);
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
    // the K towns are the ones administered now: Other only (Town is held)
    CHECK(f.of.TownCount() == 1);
    CHECK(f.of.PoolStart() == 24); // max(24, 18 + 2)
    // M = 6 / 18, T = 1 / 1: C = 0.2 + 0.4 = 0.6
    CHECK(f.of.Control() == Approx(0.6f));
    CHECK(f.of.Pool() == 14);      // round(24 * 0.6)
    CHECK(f.of.WavesFired() == 1); // 0.75 crossed, 0.50 not
    CHECK(f.of.Tokens() == 0);
    CHECK(f.of.Band() == OFBandStretched);
    // the distinct towns: the ones the resistance holds (the exempt HQ town
    // and Camp never count)
    CHECK(f.of.TownsEverCount() == 2);
    CHECK(f.of.TownEver("Town"));
    CHECK(f.of.TownEver("Held"));
    CHECK_FALSE(f.of.TownEver("HqTown"));
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
