#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // ExtParsMission (load-pass config rebuild)
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

// keeps the parsed ParamFile alive for the duration of a test
struct CacheFixture
{
    ParamFile file;
    GarrisonCache cache;

    void Load(const char* config)
    {
        QIStream in(config, strlen(config));
        file.Parse(in);
        cache.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
    }
};

GarrisonDecisionInput MakeInput(bool occupierOwned, bool spawned, float reserve, bool playerValid, float playerDist)
{
    GarrisonDecisionInput in;
    in.occupierOwned = occupierOwned;
    in.spawned = spawned;
    in.reserve = reserve;
    in.playerValid = playerValid;
    in.playerDistSq = playerDist * playerDist;
    return in;
}

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

const float kRadius = 800.0f;
const float kHyst = GarrisonCache::DespawnHysteresis;

} // namespace

TEST_CASE("GarrisonCache - config parse of cacheInterval/groupSize", "[game][guerrilla]")
{
    SECTION("absent config keeps defaults")
    {
        GarrisonCache cache;
        cache.LoadFromParams(nullptr);
        REQUIRE(cache.Tuning().cacheInterval == Approx(5.0f));
        REQUIRE(cache.Tuning().groupSize == 12);
    }

    SECTION("keys absent from CfgGuerrillaZones keep defaults")
    {
        CacheFixture f;
        f.Load("class CfgGuerrillaZones { tickInterval = 3; };\n");
        REQUIRE(f.cache.Tuning().cacheInterval == Approx(5.0f));
        REQUIRE(f.cache.Tuning().groupSize == 12);
    }

    SECTION("explicit keys override")
    {
        CacheFixture f;
        f.Load("class CfgGuerrillaZones { cacheInterval = 8; groupSize = 6; };\n");
        REQUIRE(f.cache.Tuning().cacheInterval == Approx(8.0f));
        REQUIRE(f.cache.Tuning().groupSize == 6);
    }

    SECTION("degenerate groupSize is clamped to 1")
    {
        CacheFixture f;
        f.Load("class CfgGuerrillaZones { groupSize = 0; };\n");
        REQUIRE(f.cache.Tuning().groupSize == 1);
    }
}

TEST_CASE("GarrisonCache - spawn/despawn decision", "[game][guerrilla]")
{
    SECTION("spawn: near + despawned + reserve")
    {
        REQUIRE(GarrisonCache::Decide(MakeInput(true, false, 8, true, 500), kRadius, kHyst) == GActSpawn);
        // spawn edge is inclusive at the radius (spawning.sqs <=)
        REQUIRE(GarrisonCache::Decide(MakeInput(true, false, 8, true, 800), kRadius, kHyst) == GActSpawn);
    }

    SECTION("no spawn without reserve")
    {
        REQUIRE(GarrisonCache::Decide(MakeInput(true, false, 0, true, 500), kRadius, kHyst) == GActNone);
        REQUIRE(GarrisonCache::Decide(MakeInput(true, false, 0.5f, true, 500), kRadius, kHyst) == GActNone);
    }

    SECTION("no spawn when far")
    {
        REQUIRE(GarrisonCache::Decide(MakeInput(true, false, 8, true, 801), kRadius, kHyst) == GActNone);
    }

    SECTION("no double spawn while spawned")
    {
        REQUIRE(GarrisonCache::Decide(MakeInput(true, true, 0, true, 500), kRadius, kHyst) == GActNone);
    }

    SECTION("despawn only beyond radius + hysteresis")
    {
        // inside the hysteresis band the garrison holds (anti-flap)
        REQUIRE(GarrisonCache::Decide(MakeInput(true, true, 0, true, 820), kRadius, kHyst) == GActNone);
        REQUIRE(GarrisonCache::Decide(MakeInput(true, true, 0, true, 850), kRadius, kHyst) == GActNone);
        REQUIRE(GarrisonCache::Decide(MakeInput(true, true, 0, true, 851), kRadius, kHyst) == GActDespawn);
    }

    SECTION("dead/absent player freezes the cache")
    {
        REQUIRE(GarrisonCache::Decide(MakeInput(true, false, 8, false, 0), kRadius, kHyst) == GActNone);
        REQUIRE(GarrisonCache::Decide(MakeInput(true, true, 0, false, 9999), kRadius, kHyst) == GActNone);
    }

    SECTION("zone no longer occupier-owned")
    {
        // spawned leftovers are released even with the player nearby...
        REQUIRE(GarrisonCache::Decide(MakeInput(false, true, 0, true, 100), kRadius, kHyst) == GActDespawn);
        // ...but non-occupier zones never spawn
        REQUIRE(GarrisonCache::Decide(MakeInput(false, false, 8, true, 100), kRadius, kHyst) == GActNone);
    }
}

TEST_CASE("GarrisonCache - group planning arithmetic", "[game][guerrilla]")
{
    AutoArray<int> takes;

    SECTION("reserve splits into groupSize chunks with a remainder group")
    {
        REQUIRE(GarrisonCache::PlanGroups(14, 12, takes) == 2);
        REQUIRE(takes[0] == 12);
        REQUIRE(takes[1] == 2);
    }

    SECTION("exact multiples")
    {
        REQUIRE(GarrisonCache::PlanGroups(12, 6, takes) == 2);
        REQUIRE(takes[0] == 6);
        REQUIRE(takes[1] == 6);
    }

    SECTION("small reserve makes one small group")
    {
        REQUIRE(GarrisonCache::PlanGroups(3, 12, takes) == 1);
        REQUIRE(takes[0] == 3);
    }

    SECTION("empty reserve makes no groups")
    {
        REQUIRE(GarrisonCache::PlanGroups(0, 12, takes) == 0);
        REQUIRE(takes.Size() == 0);
    }

    SECTION("degenerate group size clamps to 1")
    {
        REQUIRE(GarrisonCache::PlanGroups(3, 0, takes) == 3);
    }

    SECTION("total across groups equals the reserve (survivor write-back inverse)")
    {
        GarrisonCache::PlanGroups(29, 12, takes);
        int total = 0;
        for (int i = 0; i < takes.Size(); i++)
        {
            total += takes[i];
        }
        REQUIRE(total == 29);
    }
}

TEST_CASE("GarrisonCache - event name mapping and handler bookkeeping", "[game][guerrilla]")
{
    REQUIRE(GarrisonCache::EventTypeFromName("garrisonSpawned") == GESpawned);
    REQUIRE(GarrisonCache::EventTypeFromName("GARRISONDESPAWNED") == GEDespawned); // case-insensitive
    REQUIRE(GarrisonCache::EventTypeFromName("noSuchEvent") == -1);
    REQUIRE(GarrisonCache::EventTypeFromName(nullptr) == -1);

    GarrisonCache cache;
    cache.SetEventHandler(GESpawned, "hint \"up\"");
    cache.SetEventHandler(GEDespawned, "hint \"down\"");
    REQUIRE(Str(cache.GetEventHandler(GESpawned)) == "hint \"up\"");
    REQUIRE(Str(cache.GetEventHandler(GEDespawned)) == "hint \"down\"");

    // Clear drops handlers together with the tuning/state
    cache.Clear();
    REQUIRE(Str(cache.GetEventHandler(GESpawned)).empty());
    REQUIRE(Str(cache.GetEventHandler(GEDespawned)).empty());
}

TEST_CASE("GarrisonCache - queries are safe on an empty cache", "[game][guerrilla]")
{
    GarrisonCache cache;
    REQUIRE_FALSE(cache.IsSpawned(0));
    REQUIRE_FALSE(cache.IsSpawned(-1));
    REQUIRE(cache.LiveCount(0) == 0);
    REQUIRE(cache.NGroups(0) == 0);
    REQUIRE(cache.GetGroup(0, 0) == nullptr);
}

TEST_CASE("GarrisonCache - handlers and spawned-row reconciliation survive save/load", "[game][guerrilla][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "garrison-cache-roundtrip.bin";

    // Serialize resolves zones through the engine-wide ZoneRegistry, and the
    // load's second pass rebuilds it from ExtParsMission, exactly as
    // SetMission's description.ext reparse would during a real load.
    const char* config = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    class Zones\n"
                         "    {\n"
                         "        class Outpost { name=\"Outpost\"; type=\"OUTPOST\"; owner=\"EAST\"; "
                         "garrison=8; position[]={1000.0, 1000.0, 50.0}; };\n"
                         "        class Depot   { name=\"Depot\";   type=\"OUTPOST\"; owner=\"EAST\"; "
                         "garrison=4; position[]={5000.0, 5000.0, 50.0}; };\n"
                         "    };\n"
                         "};\n";
    {
        QIStream in(config, strlen(config));
        ExtParsMission.Parse(in);
    }
    ZoneRegistry::Instance().InitMission();
    const int outpost = ZoneRegistry::Instance().FindZoneIndex("Outpost");
    const int depot = ZoneRegistry::Instance().FindZoneIndex("Depot");
    REQUIRE(outpost >= 0);
    REQUIRE(depot >= 0);

    {
        GarrisonCache cache;
        cache.SetEventHandler(GESpawned, "gmEvtGarSpawned = gmEvtGarSpawned + [_this]");
        cache.SetEventHandler(GEDespawned, "hDespawned");
        // a spawned zone whose group links all died before the save (no live
        // world in a unit test, so the links serialize as null refs)
        cache.MarkSpawnedForTest(outpost, true);
        REQUIRE(cache.IsSpawned(outpost));

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(cache.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    GarrisonCache loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }

    CHECK(Str(loaded.GetEventHandler(GESpawned)) == "gmEvtGarSpawned = gmEvtGarSpawned + [_this]");
    CHECK(Str(loaded.GetEventHandler(GEDespawned)) == "hDespawned");

    // wiped-out reconciliation: a spawned row with no surviving group refs
    // reconciles to an empty zone (spawned = false, reserve zeroed)
    CHECK_FALSE(loaded.IsSpawned(outpost));
    CHECK(loaded.LiveCount(outpost) == 0);
    const ZoneRecord* z = ZoneRegistry::Instance().GetZone(outpost);
    REQUIRE(z != nullptr);
    CHECK(z->garrison == Approx(0.0f));
    CHECK(z->liveOccupiers == Approx(0.0f));
    // the untouched zone keeps its config reserve
    CHECK(ZoneRegistry::Instance().GetZone(depot)->garrison == Approx(4.0f));

    // scrub the process-wide state other tests expect to be empty
    ExtParsMission.Clear();
    ZoneRegistry::Instance().Clear();
    std::filesystem::remove(archivePath);
}
