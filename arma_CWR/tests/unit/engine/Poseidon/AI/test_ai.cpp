#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/AIRadio.hpp>
#include <Poseidon/AI/ArcadeTemplate.hpp>
#include <Poseidon/Core/SaveVersion.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/UI/Locale/Stringtable/Stringtable.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

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

// ---------------------------------------------------------------------------
// The cargo seat-in contract (round 3 B, issue #78 family).  Guerrilla
// traffic seats its cargo riders through NativeMoveIn, which seats and
// assigns but never set the AI state; the first get-out then failed
// "check failed: _state == InCargo" in AIUnit::DoGetOut and strict mode shut
// the game down.  AssignAsCargo now completes the seat-in.
// ---------------------------------------------------------------------------

TEST_CASE("a seated and assigned cargo rider enters InCargo", "[ai][incargo]")
{
    // the native traffic rider: local, alive, seated, still in his spawn state
    CHECK(CargoSeatEntersInCargo(true, true, true, AIUnit::Wait));
    CHECK(CargoSeatEntersInCargo(true, true, true, AIUnit::Busy));
    CHECK(CargoSeatEntersInCargo(true, true, true, AIUnit::Completed));
    CHECK(CargoSeatEntersInCargo(true, true, true, AIUnit::Init));
    // already InCargo (moveInCargo, the mission loader, DoGetIn): no change
    CHECK_FALSE(CargoSeatEntersInCargo(true, true, true, AIUnit::InCargo));
    // assigned before boarding (the ordinary get-in order): not seated yet,
    // the state belongs to the walk to the hull
    CHECK_FALSE(CargoSeatEntersInCargo(true, true, false, AIUnit::Wait));
    // a remote brain: its owner sets the state, DoGetOut only checks local ones
    CHECK_FALSE(CargoSeatEntersInCargo(false, true, true, AIUnit::Wait));
    // a corpse in a cargo seat never gets out through DoGetOut
    CHECK_FALSE(CargoSeatEntersInCargo(true, false, true, AIUnit::Wait));
}

TEST_CASE("AI unit state names cover every state", "[ai][incargo]")
{
    CHECK(std::string(AIUnitStateName(AIUnit::Wait)) == "WAIT");
    CHECK(std::string(AIUnitStateName(AIUnit::Init)) == "INIT");
    CHECK(std::string(AIUnitStateName(AIUnit::Busy)) == "BUSY");
    CHECK(std::string(AIUnitStateName(AIUnit::Completed)) == "OK");
    CHECK(std::string(AIUnitStateName(AIUnit::Delay)) == "DELAY");
    CHECK(std::string(AIUnitStateName(AIUnit::InCargo)) == "CARGO");
    CHECK(std::string(AIUnitStateName(AIUnit::Stopping)) == "STOPPING");
    CHECK(std::string(AIUnitStateName(AIUnit::Replan)) == "REPLAN");
    CHECK(std::string(AIUnitStateName(AIUnit::Stopped)) == "STOPPED");
    CHECK(std::string(AIUnitStateName(AIUnit::Sending)) == "SENDING");
}

namespace
{
std::string ReadAISource(const char* relative)
{
    const std::filesystem::path path = std::filesystem::path(TESTS_ROOT_DIR).parent_path() / relative;
    std::ifstream input(path, std::ios::binary);
    REQUIRE(input);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
} // namespace

TEST_CASE("AssignAsCargo completes the seat-in and DoGetOut keeps its InCargo check", "[ai][incargo]")
{
    const std::string source = ReadAISource("engine/Poseidon/AI/AIUnitImpl.cpp");

    // both successful exits of AssignAsCargo run the seat-in completion
    const size_t assign = source.find("bool AIUnit::AssignAsCargo(Transport* veh)");
    REQUIRE(assign != std::string::npos);
    const size_t assignEnd = source.find("RString AIUnit::GetDebugName() const", assign);
    REQUIRE(assignEnd != std::string::npos);
    const std::string body = source.substr(assign, assignEnd - assign);
    const size_t first = body.find("CompleteCargoSeatIn(this, veh);");
    REQUIRE(first != std::string::npos);
    CHECK(first < body.find("return true; // already assigned as cargo"));
    const size_t second = body.find("CompleteCargoSeatIn(this, veh);", first + 1);
    REQUIRE(second != std::string::npos);
    CHECK(body.find("_vehicleAssigned = veh;", first) < second);

    // the invariant check itself is not downgraded: it stays an AI_ERROR
    // (fatal in strict mode), and the named diagnostic comes before it
    const size_t getOut = source.find("void AIUnit::DoGetOut(Transport* veh, bool parachute)");
    REQUIRE(getOut != std::string::npos);
    const size_t warn = source.find("DoGetOut: cargo {} leaves {} in state {}", getOut);
    const size_t check = source.find("AI_ERROR(_state == InCargo);", getOut);
    REQUIRE(warn != std::string::npos);
    REQUIRE(check != std::string::npos);
    CHECK(warn < check);
}

// ---------------------------------------------------------------------------
// The shared center channel (occupier forces, the break-off column): the
// side's center radio carries the commands of every group of the side.  A
// group sends one there when the support vehicle it sends a unit to belongs
// to another group (CheckFuel / CheckArmor / CheckHealth / CheckAmmo set
// channelCenter), and upstream CommandSent then checked GetFrom() == this
// for every command on that channel: the next fuel, repair, heal or ammo
// check of ANY other group of the side failed "check failed:
// msgCmd->GetFrom() == this" (AIGroupImpl.cpp, CommandSent) and strict mode
// ended the game.  Another group's command (or one whose sender group has
// left) is now skipped there; the group's own channel keeps the check.
// ---------------------------------------------------------------------------

TEST_CASE("a command on the shared center channel counts only for the group that sent it", "[ai][radio]")
{
    Ref<AIGroup> carrierCrew = new AIGroup();
    Ref<AIGroup> infantry = new AIGroup();
    Command refuel;
    refuel._message = Command::Refuel;
    PackedBoolArray none;
    Ref<RadioMessageCommand> fromCrew = new RadioMessageCommand(carrierCrew, none, refuel);

    // the crew's own fuel check finds its own command on the center channel
    CHECK(RadioCommandIsFrom(fromCrew, carrierCrew, true));
    // the infantry's check skips it (upstream: the fatal from-group check)
    CHECK_FALSE(RadioCommandIsFrom(fromCrew, infantry, true));
    // a group's own channel only ever carries its own commands
    CHECK(RadioCommandIsFrom(fromCrew, carrierCrew, false));
    // no message is nobody's
    CHECK_FALSE(RadioCommandIsFrom(nullptr, infantry, true));

    // the sender group left the center while its command was still queued
    // (a released counter-attack group): the link goes null and the
    // command is nobody's on the center channel
    Ref<RadioMessageCommand> orphan = new RadioMessageCommand(carrierCrew, none, refuel);
    carrierCrew.Free();
    CHECK_FALSE(RadioCommandIsFrom(orphan, infantry, true));
}

TEST_CASE("every center-channel command scan skips another group's command", "[ai][radio]")
{
    const std::string impl = ReadAISource("engine/Poseidon/AI/AIGroupImpl.cpp");
    const size_t first = impl.find("bool AIGroup::CommandSent(bool channelCenter)");
    const size_t last = impl.find("void AIGroup::ClearGetInCommands(AIUnit* to)");
    REQUIRE(first != std::string::npos);
    REQUIRE(last != std::string::npos);
    const std::string sent = impl.substr(first, last - first);
    // the three CommandSent overloads read the center channel when asked:
    // none keeps the bare from-group check, each scan and each actual
    // message goes through the helper (two per overload)
    CHECK(sent.find("AI_ERROR(msgCmd->GetFrom() == this);") == std::string::npos);
    size_t uses = 0;
    for (size_t at = sent.find("RadioCommandIsFrom(msgCmd, this, channelCenter)"); at != std::string::npos;
         at = sent.find("RadioCommandIsFrom(msgCmd, this, channelCenter)", at + 1))
    {
        uses++;
    }
    CHECK(uses == 6);
    // the helper keeps the check for the group's own channel
    const size_t helper = impl.find("bool RadioCommandIsFrom(RadioMessageCommand* msg, AIGroup* group, bool channelCenter)");
    REQUIRE(helper != std::string::npos);
    CHECK(impl.find("AI_ERROR(msg->GetFrom() == group);", helper) < first);

    // SendCommand's GetIn / GetOut scan reads the same channel
    const std::string group = ReadAISource("engine/Poseidon/AI/AIGroup.cpp");
    const size_t send = group.find("void AIGroup::SendCommand(Command& cmd, PackedBoolArray list, bool channelCenter)");
    REQUIRE(send != std::string::npos);
    const size_t sendEnd = group.find("Script* AIGroup::GetScript() const", send);
    REQUIRE(sendEnd != std::string::npos);
    const std::string body = group.substr(send, sendEnd - send);
    CHECK(body.find("AI_ERROR(msgCmd->GetFrom() == this);") == std::string::npos);
    CHECK(body.find("RadioCommandIsFrom(msgCmd, this, channelCenter)") != std::string::npos);
}
