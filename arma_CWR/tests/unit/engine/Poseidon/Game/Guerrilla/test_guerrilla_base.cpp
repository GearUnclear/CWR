#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp>
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

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

HqCandidate Cand(int index, int nPos, float dist)
{
    HqCandidate c;
    c.index = index;
    c.nPos = nPos;
    c.dist = dist;
    return c;
}

HqSpotSample Spot(float dist, bool onRoad = false, bool underwater = false, bool free = true)
{
    HqSpotSample s;
    s.x = dist;
    s.z = 0;
    s.height = 10;
    s.distFromAnchor = dist;
    s.onRoad = onRoad;
    s.underwater = underwater;
    s.free = free;
    return s;
}

// keeps the parsed ParamFile alive for the duration of a test
struct BaseFixture
{
    ParamFile file;
    GuerrillaBase base;

    void Load(const char* config)
    {
        QIStream in(config, strlen(config));
        file.Parse(in);
        base.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
    }
};

} // namespace

TEST_CASE("GuerrillaBase - HQ building picker: most AI positions, nearest breaks ties", "[game][guerrilla][base]")
{
    SECTION("empty candidate set yields no building")
    {
        AutoArray<HqCandidate> candidates;
        CHECK(GuerrillaBase::PickHqBuilding(candidates) == -1);
    }

    SECTION("the building with the most positions wins regardless of distance")
    {
        AutoArray<HqCandidate> candidates;
        candidates.Add(Cand(10, 4, 5.0f));   // close but small
        candidates.Add(Cand(11, 12, 90.0f)); // far but roomy
        candidates.Add(Cand(12, 6, 30.0f));
        CHECK(GuerrillaBase::PickHqBuilding(candidates) == 1);
    }

    SECTION("equal positions: the one nearest the zone centre")
    {
        AutoArray<HqCandidate> candidates;
        candidates.Add(Cand(1, 8, 60.0f));
        candidates.Add(Cand(2, 8, 12.0f));
        candidates.Add(Cand(3, 8, 40.0f));
        CHECK(GuerrillaBase::PickHqBuilding(candidates) == 1);
    }

    SECTION("a single candidate is picked")
    {
        AutoArray<HqCandidate> candidates;
        candidates.Add(Cand(7, 4, 100.0f));
        CHECK(GuerrillaBase::PickHqBuilding(candidates) == 0);
    }
}

TEST_CASE("GuerrillaBase - outdoor spot picker takes the first dry, off-road, free sample", "[game][guerrilla][base]")
{
    SECTION("nothing qualifies")
    {
        AutoArray<HqSpotSample> samples;
        CHECK(GuerrillaBase::PickSpot(samples) == -1);
        samples.Add(Spot(10, true));
        samples.Add(Spot(20, false, true));
        samples.Add(Spot(30, false, false, false));
        CHECK(GuerrillaBase::PickSpot(samples) == -1);
    }

    SECTION("order is the caller's ring order, hard rejects are skipped")
    {
        AutoArray<HqSpotSample> samples;
        samples.Add(Spot(10, true));                // in the road
        samples.Add(Spot(12, false, true));         // in the sea
        samples.Add(Spot(14, false, false, false)); // occupied
        samples.Add(Spot(16));                      // the one
        samples.Add(Spot(18));
        CHECK(GuerrillaBase::PickSpot(samples) == 3);
    }
}

TEST_CASE("GuerrillaBase - 2D range test ignores elevation", "[game][guerrilla][base]")
{
    CHECK(GuerrillaBase::InRange2D(Vector3(100, 0, 100), Vector3(160, 500, 180), 100.0f));
    CHECK_FALSE(GuerrillaBase::InRange2D(Vector3(100, 0, 100), Vector3(200, 0, 100), 99.0f));
    CHECK(GuerrillaBase::InRange2D(Vector3(0, 0, 0), Vector3(0, 0, 0), 0.0f));
}

TEST_CASE("GuerrillaBase - config parse of the hq*/garage* keys", "[game][guerrilla][base]")
{
    SECTION("absent config keeps defaults")
    {
        GuerrillaBase base;
        base.LoadFromParams(nullptr);
        CHECK(base.Tuning().hqMinPos == 4);
        CHECK(base.Tuning().garageRadius == Approx(100.0f));
        CHECK(base.Tuning().garageInvulnerable);
    }

    SECTION("authored keys override, with sanity floors")
    {
        BaseFixture f;
        f.Load("class CfgGuerrillaZones { hqMinPos = 6; garageRadius = 60; garageInvulnerable = 0; };");
        CHECK(f.base.Tuning().hqMinPos == 6);
        CHECK(f.base.Tuning().garageRadius == Approx(60.0f));
        CHECK_FALSE(f.base.Tuning().garageInvulnerable);

        BaseFixture g;
        g.Load("class CfgGuerrillaZones { hqMinPos = 0; garageRadius = 2; };");
        CHECK(g.base.Tuning().hqMinPos == 1);
        CHECK(g.base.Tuning().garageRadius == Approx(10.0f));
        CHECK(g.base.Tuning().garageInvulnerable); // untouched default
    }
}

TEST_CASE("GuerrillaBase - unestablished queries are safe and empty", "[game][guerrilla][base]")
{
    GuerrillaBase base;
    CHECK_FALSE(base.IsEstablished());
    CHECK_FALSE(base.IsIndoors());
    CHECK(base.ZoneName().GetLength() == 0);
    CHECK(base.HqPos() == VZero);
    CHECK(base.GaragePos() == VZero);
    CHECK(base.Building() == nullptr);
    CHECK(base.Cache() == nullptr);
    CHECK(base.MoveCount() == 0);
    CHECK(base.GarageCount() == 0);
    CHECK(base.GarageVehicle(0) == nullptr);
    CHECK(base.GarageVehicle(-1) == nullptr);
    CHECK_FALSE(base.GarageHas(nullptr));
    CHECK_FALSE(base.GarageLock(nullptr, true));
    CHECK_FALSE(base.GarageLock(nullptr, false));
    CHECK_FALSE(base.InGarageRange(Vector3(0, 0, 0)));
    // no zone registry content: nothing can be established anywhere
    CHECK_FALSE(base.CanEstablish(Vector3(100, 0, 100)));
    base.Simulate(10.0f); // inactive registry: no-op, no crash
}

TEST_CASE("GuerrillaBase - garage range follows the tuning radius once established", "[game][guerrilla][base]")
{
    BaseFixture f;
    f.Load("class CfgGuerrillaZones { garageRadius = 50; };");
    f.base.MarkEstablishedForTest("Houdan", Vector3(1000, 20, 2000), Vector3(1030, 20, 2000), Vector3(1000, 20, 2000),
                                  true);
    CHECK(f.base.IsEstablished());
    CHECK(f.base.IsIndoors());
    CHECK(Str(f.base.ZoneName()) == "Houdan");
    CHECK(f.base.InGarageRange(Vector3(1070, 0, 2000)));
    CHECK_FALSE(f.base.InGarageRange(Vector3(1090, 0, 2000)));
}

TEST_CASE("GuerrillaBase - established state survives save/load, dead garage rows are dropped",
          "[game][guerrilla][base][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "guerrilla-base-roundtrip.bin";

    // the load pass rebuilds the tuning from the mission config (a load never
    // reruns InitMission), so park one in ExtParsMission like SetMission would
    const char* config = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    garageRadius = 80;\n"
                         "    class Zones\n"
                         "    {\n"
                         "        class Houdan { name=\"Houdan\"; type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={7100.0, 6000.0, 35.0}; };\n"
                         "    };\n"
                         "};\n";
    {
        QIStream in(config, strlen(config));
        ExtParsMission.Parse(in);
    }
    ZoneRegistry::Instance().InitMission();
    REQUIRE(ZoneRegistry::Instance().FindZoneIndex("Houdan") >= 0);

    {
        GuerrillaBase base;
        base.LoadFromConfig();
        base.MarkEstablishedForTest("Houdan", Vector3(7090.0f, 36.0f, 6010.0f), Vector3(7120.0f, 35.0f, 6020.0f),
                                    Vector3(7091.0f, 37.0f, 6011.0f), false);
        // two rows with null vehicle links (no live world) - exactly what a
        // hull that did not survive the load looks like
        base.AddGarageRowForTest();
        base.AddGarageRowForTest();
        REQUIRE(base.GarageCount() == 2);

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(base.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
        // saving must not disturb the live state
        CHECK(base.IsEstablished());
        CHECK(base.GarageCount() == 2);
    }

    GuerrillaBase loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }

    CHECK(loaded.IsEstablished());
    CHECK_FALSE(loaded.IsIndoors());
    CHECK(Str(loaded.ZoneName()) == "Houdan");
    CHECK(loaded.HqPos().X() == Approx(7090.0f));
    CHECK(loaded.HqPos().Z() == Approx(6010.0f));
    CHECK(loaded.GaragePos().X() == Approx(7120.0f));
    CHECK(loaded.GaragePos().Z() == Approx(6020.0f));
    CHECK(loaded.CachePos().Y() == Approx(37.0f));
    // the tuning came back from the (re)parsed mission config
    CHECK(loaded.Tuning().garageRadius == Approx(80.0f));
    // null refs mean the hulls are genuinely gone (they ride the world's
    // vehicle serializer) - the contract is DROP, not resurrect
    CHECK(loaded.GarageCount() == 0);
    CHECK(loaded.MoveCount() == 0);

    // scrub the process-wide state other tests expect to be empty
    ExtParsMission.Clear();
    ZoneRegistry::Instance().Clear();
    std::filesystem::remove(archivePath);
}

TEST_CASE("GuerrillaBase - an unestablished base round-trips as unestablished", "[game][guerrilla][base][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "guerrilla-base-empty.bin";
    {
        GuerrillaBase base;
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(base.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }
    GuerrillaBase loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK_FALSE(loaded.IsEstablished());
    CHECK(loaded.GarageCount() == 0);
    std::filesystem::remove(archivePath);
}
