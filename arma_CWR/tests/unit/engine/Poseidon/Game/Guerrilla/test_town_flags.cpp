#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/TownFlags.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
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

FlagSpotSample MakeSample(float dist, float height, bool onRoad = false, bool underwater = false)
{
    FlagSpotSample s;
    s.x = dist; // geometry is irrelevant to the picker; only the fields below matter
    s.z = 0;
    s.height = height;
    s.distFromCenter = dist;
    s.onRoad = onRoad;
    s.underwater = underwater;
    return s;
}

} // namespace

TEST_CASE("TownFlags - flag texture resolution", "[game][guerrilla]")
{
    SECTION("faction descriptor flag key wins over everything")
    {
        CHECK(Str(TownFlags::ResolveFlagTexture("WEST", "\\flags\\israel.jpg")) == "\\flags\\israel.jpg");
        CHECK(Str(TownFlags::ResolveFlagTexture("NEUTRAL", "\\flags\\fia.jpg")) == "\\flags\\fia.jpg");
    }

    SECTION("per-side defaults (textures shipped by the Classic 1.99 data)")
    {
        CHECK(Str(TownFlags::ResolveFlagTexture("WEST", nullptr)) == "\\flags\\usa.jpg");
        CHECK(Str(TownFlags::ResolveFlagTexture("EAST", "")) == "\\flags\\ussr.jpg");
        CHECK(Str(TownFlags::ResolveFlagTexture("GUER", nullptr)) == "\\flags\\fia.jpg");
    }

    SECTION("side match is case-insensitive (owner strings come from config)")
    {
        CHECK(Str(TownFlags::ResolveFlagTexture("east", nullptr)) == "\\flags\\ussr.jpg");
        CHECK(Str(TownFlags::ResolveFlagTexture("Guer", nullptr)) == "\\flags\\fia.jpg");
    }

    SECTION("generic white flag when the side has no specific flag")
    {
        CHECK(Str(TownFlags::ResolveFlagTexture("NEUTRAL", nullptr)) == "\\flags\\bis_white.jpg");
        CHECK(Str(TownFlags::ResolveFlagTexture("CIV", "")) == "\\flags\\bis_white.jpg");
        CHECK(Str(TownFlags::ResolveFlagTexture(nullptr, nullptr)) == "\\flags\\bis_white.jpg");
    }

    SECTION("never returns an empty texture")
    {
        CHECK(TownFlags::ResolveFlagTexture("", "").GetLength() > 0);
    }
}

TEST_CASE("TownFlags - flag spot selection", "[game][guerrilla]")
{
    SECTION("empty candidate set yields no spot")
    {
        AutoArray<FlagSpotSample> samples;
        CHECK(TownFlags::PickFlagSpot(samples) == -1);
    }

    SECTION("road and underwater samples are hard-rejected, even when best-scoring")
    {
        AutoArray<FlagSpotSample> samples;
        samples.Add(MakeSample(100, 50, true));        // highest, but in the road
        samples.Add(MakeSample(100, 40, false, true)); // next, but in the sea
        samples.Add(MakeSample(0, 5));
        CHECK(TownFlags::PickFlagSpot(samples) == 2);
    }

    SECTION("all candidates rejected yields no spot (town keeps its map marker only)")
    {
        AutoArray<FlagSpotSample> samples;
        samples.Add(MakeSample(0, 10, true));
        samples.Add(MakeSample(50, 10, false, true));
        CHECK(TownFlags::PickFlagSpot(samples) == -1);
    }

    SECTION("high ground wins: a hill beats a farther flat outskirt spot")
    {
        AutoArray<FlagSpotSample> samples;
        samples.Add(MakeSample(110, 10)); // town rim, flat
        samples.Add(MakeSample(50, 18));  // inner hill, +8 m
        CHECK(TownFlags::PickFlagSpot(samples) == 1);
    }

    SECTION("on flat terrain the outskirt candidate wins (visible from outside town)")
    {
        AutoArray<FlagSpotSample> samples;
        samples.Add(MakeSample(0, 12));   // town center
        samples.Add(MakeSample(60, 12));  // mid ring
        samples.Add(MakeSample(110, 12)); // outskirts
        CHECK(TownFlags::PickFlagSpot(samples) == 2);
    }

    SECTION("negative-elevation inland samples still qualify when dry")
    {
        // below-sea-level dry land exists on custom terrains; only the
        // underwater flag rejects, not the sign of the height
        AutoArray<FlagSpotSample> samples;
        samples.Add(MakeSample(20, -2));
        CHECK(TownFlags::PickFlagSpot(samples) == 0);
    }

    SECTION("deterministic: first of equal-scoring candidates wins")
    {
        AutoArray<FlagSpotSample> samples;
        samples.Add(MakeSample(50, 10));
        samples.Add(MakeSample(50, 10));
        CHECK(TownFlags::PickFlagSpot(samples) == 0);
    }
}

TEST_CASE("TownFlags - placed rows survive save/load by zone name", "[game][guerrilla][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "town-flags-roundtrip.bin";

    // Serialize resolves zones through the engine-wide ZoneRegistry, and the
    // load's second pass matches rows against it by name, exactly as
    // SetMission's description.ext reparse would during a real load.
    const char* config = "class CfgGuerrillaZones\n"
                         "{\n"
                         "    class Zones\n"
                         "    {\n"
                         "        class Houdan  { name=\"Houdan\";  type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={7100.0, 6000.0, 35.0}; };\n"
                         "        class Larche  { name=\"Larche\";  type=\"CITY\"; owner=\"NEUTRAL\"; "
                         "position[]={5900.0, 8600.0, 20.0}; };\n"
                         "    };\n"
                         "};\n";
    {
        QIStream in(config, strlen(config));
        ExtParsMission.Parse(in);
    }
    ZoneRegistry::Instance().InitMission();
    const int houdan = ZoneRegistry::Instance().FindZoneIndex("Houdan");
    const int larche = ZoneRegistry::Instance().FindZoneIndex("Larche");
    REQUIRE(houdan >= 0);
    REQUIRE(larche >= 0);

    {
        TownFlags flags;
        // one placed town (pole link serializes as a null ref without a live
        // world - recreated at the same spot on the first post-load tick),
        // one still unplaced
        flags.MarkPlacedForTest(houdan, Vector3(7013.0f, 30.0f, 6026.0f));
        REQUIRE(flags.IsPlaced(houdan));
        REQUIRE_FALSE(flags.IsPlaced(larche));

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(flags.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    TownFlags loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }

    // the placed spot came back, keyed by zone name; the unplaced town is
    // still unplaced and will sample fresh
    REQUIRE(loaded.IsPlaced(houdan));
    CHECK(loaded.FlagPos(houdan).X() == Approx(7013.0f));
    CHECK(loaded.FlagPos(houdan).Y() == Approx(30.0f));
    CHECK(loaded.FlagPos(houdan).Z() == Approx(6026.0f));
    CHECK_FALSE(loaded.IsPlaced(larche));

    // scrub the process-wide state other tests expect to be empty
    ExtParsMission.Clear();
    ZoneRegistry::Instance().Clear();
    std::filesystem::remove(archivePath);
}
