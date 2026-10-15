#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/ArcadeTemplate.hpp>
#include <Poseidon/Core/SaveVersion.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/UI/Locale/Stringtable/Stringtable.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>

using namespace Poseidon;

TEST_CASE("ai compiles", "[ai]")
{
    REQUIRE(sizeof(AIGroup) > 0);
}

namespace
{
class ArcadeMarkerTextFixture : public SerializeClass
{
  public:
    ArcadeMarkerInfo marker;

    LSError Serialize(ParamArchive& ar) override { return marker.Serialize(ar); }
};

class ArcadeWaypointTextFixture : public SerializeClass
{
  public:
    ArcadeWaypointInfo waypoint;

    LSError Serialize(ParamArchive& ar) override { return waypoint.Serialize(ar); }
};

class ArcadeSensorTextFixture : public SerializeClass
{
  public:
    ArcadeSensorInfo sensor;

    LSError Serialize(ParamArchive& ar) override { return sensor.Serialize(ar); }
};
} // namespace

TEST_CASE("mission arcade user text decodes legacy CP1250 on load", "[ai][arcade][encoding]")
{
    const RString oldLanguage = GLanguage;
    REQUIRE(Poseidon::SetLanguage("English"));

    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path markerPath = dir / "arcade-marker-text-cp1250.bin";
    const std::filesystem::path waypointPath = dir / "arcade-waypoint-text-cp1250.bin";
    const std::filesystem::path sensorPath = dir / "arcade-sensor-text-cp1250.bin";

    {
        ArcadeMarkerTextFixture fixture;
        fixture.marker.position = VZero;
        fixture.marker.name = "dopad";
        fixture.marker.type = "mil_dot";
        fixture.marker.text = "M\xEDsto dopadu";

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(ar.Serialize("ArcadeMarker", fixture, 1) == LSOK);
        REQUIRE(ar.SaveBin(markerPath.string().c_str()));
    }

    {
        ArcadeWaypointTextFixture fixture;
        fixture.waypoint.description = "Dob\xFDt z\xE1"
                                       "kladnu";

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(ar.Serialize("ArcadeWaypoint", fixture, 1) == LSOK);
        REQUIRE(ar.SaveBin(waypointPath.string().c_str()));
    }

    {
        ArcadeSensorTextFixture fixture;
        fixture.sensor.text = "Z\xE1"
                              "kladna v hor\xE1"
                              "ch";

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(ar.Serialize("ArcadeSensor", fixture, 1) == LSOK);
        REQUIRE(ar.SaveBin(sensorPath.string().c_str()));
    }

    {
        ArcadeMarkerTextFixture fixture;
        ATSParams params;
        ParamArchiveLoad ar;
        ar.SetParams(&params);
        REQUIRE(ar.LoadBin(markerPath.string().c_str()));
        ar.FirstPass();
        REQUIRE(ar.Serialize("ArcadeMarker", fixture, 1) == LSOK);
        ar.SecondPass();
        REQUIRE(ar.Serialize("ArcadeMarker", fixture, 1) == LSOK);

        CHECK(std::string(fixture.marker.text.Data()) == "M\xC3\xADsto dopadu");
    }

    {
        ArcadeWaypointTextFixture fixture;
        ATSParams params;
        ParamArchiveLoad ar;
        ar.SetParams(&params);
        REQUIRE(ar.LoadBin(waypointPath.string().c_str()));
        ar.FirstPass();
        REQUIRE(ar.Serialize("ArcadeWaypoint", fixture, 1) == LSOK);
        ar.SecondPass();
        REQUIRE(ar.Serialize("ArcadeWaypoint", fixture, 1) == LSOK);

        CHECK(std::string(fixture.waypoint.description.Data()) == "Dob\xC3\xBDt z\xC3\xA1"
                                                                  "kladnu");
    }

    {
        ArcadeSensorTextFixture fixture;
        ATSParams params;
        ParamArchiveLoad ar;
        ar.SetParams(&params);
        REQUIRE(ar.LoadBin(sensorPath.string().c_str()));
        ar.FirstPass();
        REQUIRE(ar.Serialize("ArcadeSensor", fixture, 1) == LSOK);
        ar.SecondPass();
        REQUIRE(ar.Serialize("ArcadeSensor", fixture, 1) == LSOK);

        CHECK(std::string(fixture.sensor.text.Data()) == "Z\xC3\xA1"
                                                         "kladna v hor\xC3\xA1"
                                                         "ch");
    }

    std::filesystem::remove(markerPath);
    std::filesystem::remove(waypointPath);
    std::filesystem::remove(sensorPath);
    if (oldLanguage.GetLength() > 0)
    {
        REQUIRE(Poseidon::SetLanguage(oldLanguage));
    }
}

// AICenter::AddGroup with no id: a released slot must come back, or a long
// session of groups created and released at runtime walks the id past
// MaxGroups (the letter and colour register; 9 x 7 = 63 on the Remaster)
// with only a handful of groups alive (director scenario, 2026-10-15).
TEST_CASE("new group id takes the lowest free slot of the register", "[ai][groupid]")
{
    const int maxGroups = 63;

    SECTION("an empty center starts at 1")
    {
        REQUIRE(AICenter::PickNewGroupId(nullptr, 0, maxGroups) == 1);
    }
    SECTION("a dense register continues past its last id")
    {
        const int used[] = {1, 2, 3};
        REQUIRE(AICenter::PickNewGroupId(used, 3, maxGroups) == 4);
    }
    SECTION("a gap a released group left is reused before a higher id")
    {
        const int used[] = {1, 40, 3};
        REQUIRE(AICenter::PickNewGroupId(used, 3, maxGroups) == 2);
    }
    SECTION("non-positive ids do not occupy a slot")
    {
        const int used[] = {0, -1, 1};
        REQUIRE(AICenter::PickNewGroupId(used, 3, maxGroups) == 2);
    }
    SECTION("a full register falls back to one past the largest id")
    {
        int used[63];
        for (int i = 0; i < 63; i++)
        {
            used[i] = 63 - i;
        }
        REQUIRE(AICenter::PickNewGroupId(used, 63, maxGroups) == 64);
    }
    SECTION("no register (no config) keeps the upstream one-past-largest rule")
    {
        const int used[] = {1, 5};
        REQUIRE(AICenter::PickNewGroupId(used, 2, 0) == 6);
    }
}

TEST_CASE("overlapping group churn never leaves the register", "[ai][groupid]")
{
    // one long-lived group (the player's) plus two short-lived ones whose
    // lifetimes overlap: each new group is created before the previous one
    // is released.  Upstream's one-past-largest rule climbed by one per cycle
    // and left the register after 61 cycles; the lowest-free rule must stay
    // at ids 1..3 for any number of cycles.
    const int maxGroups = 63;
    int live[3] = {1, 0, 0};
    live[1] = AICenter::PickNewGroupId(live, 2, maxGroups);
    REQUIRE(live[1] == 2);
    int highest = live[1];
    for (int cycle = 0; cycle < 10000; cycle++)
    {
        live[2] = AICenter::PickNewGroupId(live, 2, maxGroups);
        REQUIRE(live[2] >= 1);
        REQUIRE(live[2] <= maxGroups);
        REQUIRE(live[2] != live[0]);
        REQUIRE(live[2] != live[1]);
        if (live[2] > highest)
        {
            highest = live[2];
        }
        live[1] = live[2]; // release the older squad, keep the newer one
        live[2] = 0;
    }
    REQUIRE(highest <= 3);
}
