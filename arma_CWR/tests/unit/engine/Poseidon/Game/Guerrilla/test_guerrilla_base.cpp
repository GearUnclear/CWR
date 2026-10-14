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
    // no HQ, no HQ zone (the counter-attack dispatch skip stays inert)
    CHECK(base.HqZone() == -1);
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

TEST_CASE("GuerrillaBase - HqZone: the zone the HQ stands in (OCCUPIER-FORCES 9.1, N4a)", "[game][guerrilla][base]")
{
    const char* config = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    zoneArea = 150;\n"
                         "    class Zones\n"
                         "    {\n"
                         "        class Camp   { name=\"Camp\";   type=\"CAMP\"; owner=\"GUER\"; "
                         "position[]={1000.0, 1000.0, 0.0}; };\n"
                         "        class Houdan { name=\"Houdan\"; type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={2000.0, 3000.0, 0.0}; };\n"
                         "        class Other  { name=\"Other\";  type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={5000.0, 5000.0, 0.0}; };\n"
                         "    };\n"
                         "};\n";
    ParamFile file;
    QIStream in(config, strlen(config));
    file.Parse(in);
    ZoneRegistry registry;
    registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), nullptr);
    const int houdan = registry.FindZoneIndex("Houdan");
    REQUIRE(houdan >= 0);

    GuerrillaBase base;
    CHECK(base.HqZone(registry) == -1); // unestablished
    // indoors, 40 m from the centre (engine axes: X easting, Z northing)
    base.MarkEstablishedForTest("Houdan", Vector3(2040, 0, 3000), Vector3(2060, 0, 3000), Vector3(2040, 0, 3000),
                                true);
    CHECK(base.HqZone(registry) == houdan);
    CHECK(GuerrillaBase::ZoneAt(registry, Vector3(2040, 0, 3000)) == houdan);
    // an edge-of-town HQ just outside the zone area (0.9 x zoneArea + the
    // tent offset): still the zone it was established in
    base.MarkEstablishedForTest("Houdan", Vector3(2160, 0, 3000), Vector3(2145, 0, 3000), Vector3(2164, 0, 3000),
                                false);
    CHECK(GuerrillaBase::ZoneAt(registry, Vector3(2160, 0, 3000)) == -1);
    CHECK(base.HqZone(registry) == houdan);
    // a HQ standing in another zone's area answers that zone
    base.MarkEstablishedForTest("Houdan", Vector3(5010, 0, 5000), Vector3(5030, 0, 5000), Vector3(5010, 0, 5000),
                                true);
    CHECK(base.HqZone(registry) == registry.FindZoneIndex("Other"));
}

// ---------------------------------------------------------------------------
// the HQ model, the HQ meter and destruction (OCCUPIER-FORCES.md 9.1)
// ---------------------------------------------------------------------------

namespace
{

// a Camp, an unheld start town and an enemy outpost (engine axes: X easting,
// Z northing; config position[] = {x, z, y})
const char* kHqConfig = "class CfgGuerrillaZones\n"
                        "{\n"
                        "    zoneArea = 150;\n"
                        "    class Zones\n"
                        "    {\n"
                        "        class Houdan  { name=\"Houdan\";  type=\"CITY\";    owner=\"NEUTRAL\";    "
                        "support=20; position[]={2000.0, 3000.0, 0.0}; };\n"
                        "        class Camp    { name=\"Camp\";    type=\"CAMP\";    owner=\"RESISTANCE\"; "
                        "position[]={1000.0, 1000.0, 0.0}; };\n"
                        "        class Outpost { name=\"Outpost\"; type=\"OUTPOST\"; owner=\"OCCUPIER\";   "
                        "garrison=6; position[]={5000.0, 5000.0, 0.0}; };\n"
                        "        class Camp2   { name=\"Camp2\";   type=\"camp\";    owner=\"RESISTANCE\"; "
                        "position[]={8000.0, 8000.0, 0.0}; };\n"
                        "    };\n"
                        "};\n";

int g_assault = 0;
int g_hold = 0;
int g_assaultCalls = 0;
Vector3 g_assaultPos = VZero;
float g_assaultRadius = 0;

int FakeAssault(Vector3Par pos, float radius)
{
    g_assaultCalls++;
    g_assaultPos = pos;
    g_assaultRadius = radius;
    return g_assault;
}

int FakeHold(Vector3Par /*pos*/, float /*radius*/)
{
    return g_hold;
}

// installs the fake presence sources on the process-wide registry built from
// kHqConfig; restores the engine sources and scrubs the registry on exit
struct HqFixture
{
    HqFixture()
    {
        QIStream in(kHqConfig, strlen(kHqConfig));
        ExtParsMission.Parse(in);
        ZoneRegistry::Instance().InitMission();
        g_assault = 0;
        g_hold = 0;
        g_assaultCalls = 0;
        GuerrillaBase::SetHqPresenceSources(&FakeAssault, &FakeHold);
    }
    ~HqFixture()
    {
        GuerrillaBase::SetHqPresenceSources(nullptr, nullptr);
        ExtParsMission.Clear();
        ZoneRegistry::Instance().Clear();
    }
    HqFixture(const HqFixture&) = delete;
    HqFixture& operator=(const HqFixture&) = delete;

    ZoneRegistry& Registry() { return ZoneRegistry::Instance(); }
    const ZoneTuning& Tuning() { return ZoneRegistry::Instance().Tuning(); }
    const ZoneRecord& Zone(const char* name)
    {
        const int i = Registry().FindZoneIndex(name);
        REQUIRE(i >= 0);
        return *Registry().GetZone(i);
    }
};

struct ZoneSnapshot
{
    std::string owner;
    float garrison = 0;
    float support = 0;
    float capture = 0;
    bool exempt = false;

    explicit ZoneSnapshot(const ZoneRecord& z)
        : owner((const char*)z.owner), garrison(z.garrison), support(z.support), capture(z.capture),
          exempt(z.garrisonExempt)
    {
    }
    bool operator==(const ZoneSnapshot& o) const
    {
        return owner == o.owner && garrison == o.garrison && support == o.support && capture == o.capture &&
               exempt == o.exempt;
    }
};

} // namespace

TEST_CASE("GuerrillaBase - the default HQ is the first CAMP zone, once, and never after a fall",
          "[game][guerrilla][base]")
{
    HqFixture f;
    REQUIRE(f.Registry().IsActive());
    // the Camp is the first zone of type CAMP (case-insensitive), not the
    // first zone and not a later camp
    CHECK(GuerrillaBase::FirstCampZone(f.Registry()) == f.Registry().FindZoneIndex("Camp"));
    ZoneRegistry empty;
    CHECK(GuerrillaBase::FirstCampZone(empty) == -1);

    GuerrillaBase base;
    // a fresh campaign with no START TOWN wants its HQ at the Camp; a
    // campaignEnd = 0 fixture does not
    CHECK(base.WantsDefaultHq(true));
    CHECK_FALSE(base.WantsDefaultHq(false));
    // the default camp start makes a real HQ (the world path runs Establish
    // at the Camp; the state is what it commits)
    const ZoneRecord& camp = f.Zone("Camp");
    base.MarkEstablishedForTest("Camp", camp.pos + Vector3(90, 0, 0), camp.pos + Vector3(80, 0, 10),
                                camp.pos + Vector3(94, 0, 0), false);
    CHECK(base.HasHq());
    CHECK(base.IsEstablished());
    CHECK_FALSE(base.HqDestroyed());
    CHECK(base.HqZone() == f.Registry().FindZoneIndex("Camp"));
    CHECK(base.HqEpoch() == 0);
    CHECK_FALSE(base.WantsDefaultHq(true));
    // the Camp falls with the HQ, then is retaken: no HQ comes back by itself
    REQUIRE(base.Destroy() == 0);
    CHECK(Str(f.Zone("Camp").owner) == Str(f.Registry().ResistanceSide()));
    CHECK_FALSE(base.WantsDefaultHq(true));
    CHECK_FALSE(base.HasHq());
    CHECK(base.HqEver());
}

TEST_CASE("GuerrillaBase - the HQ meter step: rate x crew (cap), frozen, decay, clamped", "[game][guerrilla][base]")
{
    ZoneTuning t; // captureRate 6, captureCrewCap 3, captureDecayAbandoned 2
    CHECK(GuerrillaBase::NextHqCapture(0, 1, 0, t) == Approx(6.0f));
    CHECK(GuerrillaBase::NextHqCapture(0, 2, 0, t) == Approx(12.0f));
    CHECK(GuerrillaBase::NextHqCapture(0, 3, 0, t) == Approx(18.0f));
    CHECK(GuerrillaBase::NextHqCapture(0, 9, 0, t) == Approx(18.0f)); // crew cap 3
    // both present: frozen
    CHECK(GuerrillaBase::NextHqCapture(40, 6, 1, t) == Approx(40.0f));
    // no assault: decays, hold or not
    CHECK(GuerrillaBase::NextHqCapture(40, 0, 0, t) == Approx(38.0f));
    CHECK(GuerrillaBase::NextHqCapture(40, 0, 4, t) == Approx(38.0f));
    CHECK(GuerrillaBase::NextHqCapture(1, 0, 0, t) == Approx(0.0f));
    CHECK(GuerrillaBase::NextHqCapture(95, 3, 0, t) == Approx(100.0f));
}

TEST_CASE("GuerrillaBase - the HQ meter counts assault units at HqPos only and leaves the zone alone",
          "[game][guerrilla][base]")
{
    SECTION("an HQ in an enemy zone")
    {
        HqFixture f;
        GuerrillaBase base;
        const ZoneRecord& outpost = f.Zone("Outpost");
        const Vector3 hq = outpost.pos + Vector3(40, 0, 0);
        base.MarkEstablishedForTest("Outpost", hq, hq + Vector3(20, 0, 0), hq, true);
        const ZoneSnapshot before(outpost);

        // passers-by (patrols, columns, QRFs) are not HQ_ASSAULT: the
        // assault source reports none and the meter never moves
        for (int i = 0; i < 5; i++)
        {
            base.TickHqMeter(f.Tuning());
        }
        CHECK(base.HqCapture() == Approx(0.0f));
        // the meter samples HqPos within HqRadius, not the zone centre
        CHECK(g_assaultCalls == 5);
        CHECK(g_assaultPos.X() == Approx(hq.X()));
        CHECK(g_assaultPos.Z() == Approx(hq.Z()));
        CHECK(g_assaultRadius == Approx(GuerrillaBase::HqRadius));

        g_assault = 2; // an assault, nobody home
        base.TickHqMeter(f.Tuning());
        CHECK(base.HqCapture() == Approx(12.0f));
        g_hold = 1; // a recruit (or the player under HOLD) stands there: frozen
        base.TickHqMeter(f.Tuning());
        CHECK(base.HqCapture() == Approx(12.0f));
        g_assault = 0; // the assault leaves: decay
        base.TickHqMeter(f.Tuning());
        CHECK(base.HqCapture() == Approx(10.0f));
        g_hold = 0;
        g_assault = 6;
        for (int i = 0; i < 10 && base.HasHq(); i++)
        {
            base.TickHqMeter(f.Tuning());
        }
        CHECK_FALSE(base.HasHq());
        CHECK(base.HqDestroyed());
        CHECK(base.HqEpoch() == 1);
        // the zone around it: unchanged (D24, no exemption, no census change)
        CHECK(ZoneSnapshot(f.Zone("Outpost")) == before);
        // no HQ, no meter
        base.TickHqMeter(f.Tuning());
        CHECK(base.HqCapture() == Approx(0.0f));
    }
    SECTION("an HQ in an unheld start town")
    {
        HqFixture f;
        GuerrillaBase base;
        const ZoneRecord& town = f.Zone("Houdan");
        base.MarkEstablishedForTest("Houdan", town.pos, town.pos + Vector3(30, 0, 0), town.pos, true);
        const ZoneSnapshot before(town);
        g_assault = 1;
        base.TickHqMeter(f.Tuning());
        CHECK(base.HqCapture() == Approx(6.0f));
        CHECK(ZoneSnapshot(f.Zone("Houdan")) == before);
        CHECK(Str(f.Zone("Houdan").owner) == "NEUTRAL");
    }
}

TEST_CASE("GuerrillaBase - Destroy: cache gone, garage wrecked, epoch bumped, recruit anchor gone",
          "[game][guerrilla][base]")
{
    HqFixture f;
    GuerrillaBase base;
    CHECK(base.Destroy() == -1); // nothing to destroy
    CHECK(base.HqEpoch() == 0);
    CHECK_FALSE(base.HqEver());

    const ZoneRecord& town = f.Zone("Houdan");
    base.MarkEstablishedForTest("Houdan", town.pos, town.pos + Vector3(30, 0, 0), town.pos, true);
    base.AddGarageRowForTest();
    base.AddGarageRowForTest();
    REQUIRE(base.GarageCount() == 2);
    const int moves = base.MoveCount();

    // the destroy list: every garage row is taken and the ring removed
    CHECK(base.Destroy() == 2);
    CHECK(base.GarageCount() == 0);
    CHECK_FALSE(base.HasHq());
    CHECK_FALSE(base.IsEstablished()); // part A reads IsEstablished as "an HQ stands"
    CHECK(base.HqDestroyed());
    CHECK(base.HqEpoch() == 1);
    CHECK(base.HqCapture() == Approx(0.0f));
    CHECK(base.Cache() == nullptr);
    CHECK(base.Prop() == nullptr);
    CHECK(base.Building() == nullptr);
    // the recruit anchor (gmHqPos) and the HQ zone are gone with it
    CHECK(base.HqPos() == VZero);
    CHECK(base.HqZone() == -1);
    CHECK(base.ZoneName().GetLength() == 0);
    CHECK_FALSE(base.InGarageRange(town.pos));
    CHECK(base.MoveCount() == moves);
    // a second fall needs a second HQ
    CHECK(base.Destroy() == -1);
    CHECK(base.HqEpoch() == 1);

    // re-establish anywhere (an enemy outpost), free: a first establishment,
    // not a move; the epoch stays where the fall left it; fresh ring
    const ZoneRecord& outpost = f.Zone("Outpost");
    base.MarkEstablishedForTest("Outpost", outpost.pos, outpost.pos + Vector3(30, 0, 0), outpost.pos, false);
    CHECK(base.HasHq());
    CHECK_FALSE(base.HqDestroyed());
    CHECK(base.HqEpoch() == 1);
    CHECK(base.MoveCount() == moves);
    CHECK(base.GarageCount() == 0);
    CHECK(base.HqCapture() == Approx(0.0f));
    CHECK(base.HqZone() == f.Registry().FindZoneIndex("Outpost"));
    // moving that HQ is a move again
    base.MarkEstablishedForTest("Houdan", town.pos, town.pos + Vector3(30, 0, 0), town.pos, true);
    CHECK(base.MoveCount() == moves + 1);
}

TEST_CASE("GuerrillaBase - the gmHqForceCapture lever", "[game][guerrilla][base]")
{
    HqFixture f;
    GuerrillaBase base;
    base.ForceHqCapture(50.0f); // no HQ: nothing to fill
    CHECK(base.HqCapture() == Approx(0.0f));
    const ZoneRecord& camp = f.Zone("Camp");
    base.MarkEstablishedForTest("Camp", camp.pos, camp.pos + Vector3(30, 0, 0), camp.pos, false);
    base.ForceHqCapture(50.0f);
    CHECK(base.HqCapture() == Approx(50.0f));
    CHECK(base.HasHq());
    base.ForceHqCapture(-5.0f);
    CHECK(base.HqCapture() == Approx(0.0f));
    base.ForceHqCapture(250.0f); // clamped to 100: destroyed at once
    CHECK_FALSE(base.HasHq());
    CHECK(base.HqDestroyed());
    CHECK(base.HqEpoch() == 1);
}

TEST_CASE("GuerrillaBase - HQ loss state survives save/load; old saves read as never destroyed",
          "[game][guerrilla][base][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path meterPath = dir / "guerrilla-base-meter.bin";
    const std::filesystem::path fallenPath = dir / "guerrilla-base-fallen.bin";
    HqFixture f;
    auto save = [](GuerrillaBase& base, const std::filesystem::path& path)
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(base.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    };
    auto load = [](GuerrillaBase& into, const std::filesystem::path& path)
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(path.string().c_str()));
        ar.FirstPass();
        REQUIRE(into.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(into.Serialize(ar) == LSOK);
    };

    GuerrillaBase base;
    const ZoneRecord& camp = f.Zone("Camp");
    base.MarkEstablishedForTest("Camp", camp.pos, camp.pos + Vector3(30, 0, 0), camp.pos, false);
    base.SetAutoTriedForTest(true);
    base.ForceHqCapture(42.0f);
    save(base, meterPath);
    base.Destroy();
    save(base, fallenPath);

    {
        GuerrillaBase loaded;
        load(loaded, meterPath);
        CHECK(loaded.HasHq());
        CHECK_FALSE(loaded.HqDestroyed());
        CHECK(loaded.HqEpoch() == 0);
        CHECK(loaded.HqCapture() == Approx(42.0f));
    }
    {
        GuerrillaBase loaded;
        load(loaded, fallenPath);
        CHECK_FALSE(loaded.HasHq());
        CHECK_FALSE(loaded.IsEstablished());
        CHECK(loaded.HqDestroyed());
        CHECK(loaded.HqEpoch() == 1);
        CHECK(loaded.HqCapture() == Approx(0.0f));
        CHECK(loaded.AutoTried());
        // a destroyed HQ does not come back at the Camp on load
        CHECK_FALSE(loaded.WantsDefaultHq(true));
    }
    {
        // an old save: no hqDestroyed / hqEpoch / hqCapture keys, no HQ
        // (the "(camp)" start before the one-HQ model) - it reads as never
        // destroyed, so the Camp HQ is established on its next tick
        GuerrillaBase old;
        old.SetAutoTriedForTest(true);
        save(old, meterPath);
        GuerrillaBase loaded;
        load(loaded, meterPath);
        CHECK_FALSE(loaded.HqDestroyed());
        CHECK(loaded.HqEpoch() == 0);
        CHECK(loaded.HqCapture() == Approx(0.0f));
        CHECK(loaded.AutoTried());
        CHECK(loaded.WantsDefaultHq(true));
    }
    std::filesystem::remove(meterPath);
    std::filesystem::remove(fallenPath);
}
