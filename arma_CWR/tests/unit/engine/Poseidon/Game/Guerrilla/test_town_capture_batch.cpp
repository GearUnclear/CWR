#include <catch2/catch_test_macros.hpp>

#include <Poseidon/AI/AI.hpp>
#include <Poseidon/World/Entities/Vehicles/Vehicle.hpp>
#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp>
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <string.h>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

// OnZoneEvents is the engine wrapper and reads the registry singleton, after
// EvaluateTick has applied every capture in the batch.
struct CaptureBatchFixture final : CounterAttackListener
{
    ParamFile file;
    ZoneRegistry& registry = ZoneRegistry::Instance();
    CounterAttack counter;
    CounterAttackListener* previous = CounterAttack::Listener();
    std::vector<int> counts;

    CaptureBatchFixture()
    {
        const char* config =
            "class CfgGuerrillaZones { occupierForces=0; counterAttackFirstTownChance=0; captureRate=100;"
            "class Zones {"
            "class First { name=\"First\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=55; garrison=0; "
            "position[]={1000,1000,0}; };"
            "class Second { name=\"Second\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=55; garrison=0; "
            "position[]={1200,1000,0}; };"
            "class Held { name=\"Held\"; type=\"CITY\"; owner=\"GUER\"; garrison=0; "
            "position[]={1400,1000,0}; };"
            "class Exempt { name=\"Exempt\"; type=\"CITY\"; owner=\"NEUTRAL\"; support=55; garrison=0; "
            "position[]={1600,1000,0}; };"
            "class Post { name=\"Post\"; type=\"OUTPOST\"; owner=\"EAST\"; garrison=0; "
            "position[]={1800,1000,0}; };"
            "}; };";
        QIStream in(config, strlen(config));
        file.Parse(in);
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), nullptr);
        counter.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
        registry.GetZoneMutable(2)->garrisonExempt = true;
        registry.GetZoneMutable(3)->garrisonExempt = true;
        CampaignEnd::Instance().Clear();
        CounterAttack::SetListener(this);
    }

    ~CaptureBatchFixture()
    {
        CounterAttack::SetListener(previous);
        registry.Clear();
    }

    bool OnTownFlipped(int, int k) override
    {
        counts.push_back(k);
        return false; // exercise the first-town answer chance too
    }
    void OnCounterAttackOp(const CounterAttackOpEvent&) override {}
    void OnZoneEvents(const AutoArray<ZoneEventRecord>&) override {}

    void Capture()
    {
        ZoneTickInputs in;
        in.playerValid = true;
        in.playerX = 1000;
        in.playerZ = 1000;
        in.guerCount.Resize(registry.NZones());
        for (int i = 0; i < registry.NZones(); i++)
        {
            in.guerCount[i] = 1;
        }
        AutoArray<ZoneEventRecord> fired;
        registry.EvaluateTick(in, fired);
        REQUIRE(stricmp(registry.GetZone(0)->owner, registry.ResistanceSide()) == 0);
        REQUIRE(stricmp(registry.GetZone(1)->owner, registry.ResistanceSide()) == 0);
        counter.OnZoneEvents(fired);
    }
};

} // namespace

TEST_CASE("CounterAttack - simultaneous town captures count each flip once",
          "[game][guerrilla][counterattack][town-capture-batch]")
{
    CaptureBatchFixture f;
    int before = 0;
    SECTION("fresh campaign keeps the first-town chance") {}
    SECTION("a town already held contributes to the count")
    {
        f.registry.GetZoneMutable(2)->garrisonExempt = false;
        before = 1;
    }
    SECTION("saved capture history remains above current holdings")
    {
        f.counter.SetTownsTaken(7);
        before = 7;
    }

    f.Capture();
    REQUIRE(f.counts.size() == 2);
    CHECK(f.counts[0] == before);
    CHECK(f.counts[1] == before + 1);
    CHECK(f.counter.TownsTaken() == before + 2);
    CHECK(f.counter.FindByTarget("Exempt") == -1);
    CHECK(f.counter.FindByTarget("Post") == -1);
    CHECK((f.counter.FindByTarget("First") >= 0) == (before > 0));
    REQUIRE(f.counter.FindByTarget("Second") >= 0);
    CHECK(f.counter.Get(f.counter.FindByTarget("Second"))->townsBefore == before + 1);
}
