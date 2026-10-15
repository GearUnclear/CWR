#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <string.h>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

struct PresenceOrderFixture
{
    ParamFile file;
    ZoneRegistry registry;
    CounterAttack counter;
    CounterAttackListener* previous = CounterAttack::Listener();
    AutoArray<ZoneEventRecord> fired;
    CounterAttackView far;

    PresenceOrderFixture()
    {
        const char* config = "class CfgGuerrillaZones { occupierForces=1; class Zones {"
                             "class Target { name=\"Target\"; type=\"CITY\"; owner=\"GUER\"; support=60; "
                             "position[]={1000,1000,0}; };"
                             "class Source { name=\"Source\"; type=\"OUTPOST\"; owner=\"EAST\"; garrison=12; "
                             "position[]={5000,5000,0}; };"
                             "}; };";
        QIStream in(config, strlen(config));
        file.Parse(in);
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), nullptr);
        counter.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
        CounterAttack::SetListener(nullptr);
        far.playerValid = true;
        far.playerX = 20000;
        far.playerZ = 20000;
    }

    ~PresenceOrderFixture() { CounterAttack::SetListener(previous); }

    ZoneRecord& Target() { return *registry.GetZoneMutable(0); }

    void Hold(int kind)
    {
        CounterAttackOrder order;
        order.kind = kind;
        order.opId = 1;
        order.target = 0;
        order.source = 1;
        order.bodies = 5;
        order.funded = true;
        order.abstractSpeed = CounterAttack::FootSpeed;
        if (kind == CAKBase)
        {
            Target().type = "OUTPOST";
        }
        if (kind == CAKHqAssault)
        {
            far.hqEstablished = true;
            far.hqZone = 0;
            far.hqPos = Vector3(1100, 0, 1000);
        }
        REQUIRE(counter.LaunchFunded(registry, far, order, nullptr, fired));
        counter.Step(registry, far, 50000.0f, nullptr, fired);
        REQUIRE(counter.Count() == 1);
        REQUIRE(counter.Get(0)->phase == CAHolding);
        REQUIRE(counter.Get(0)->strength == 5);
        REQUIRE_FALSE(counter.Get(0)->suspended);
    }

    CounterAttackView At(float x) const
    {
        CounterAttackView view = far;
        view.playerX = x;
        view.playerZ = 1000;
        return view;
    }

    // The engine's ordering: sample abstract presence with the registry's
    // current player view and tick the zones BEFORE CounterAttack::Step.
    void TickZone(const CounterAttackView& view)
    {
        ZoneTickInputs in;
        in.playerValid = view.playerValid;
        in.playerX = view.playerX;
        in.playerZ = view.playerZ;
        in.abstractOcc.Resize(registry.NZones());
        for (int i = 0; i < registry.NZones(); i++)
        {
            in.abstractOcc[i] = counter.AbstractPresence(registry, i, &view);
        }
        registry.EvaluateTick(in, fired);
    }
};

} // namespace

TEST_CASE("CounterAttack - zone presence uses the current bubble before the force tick",
          "[game][guerrilla][counterattack][town-presence-order]")
{
    PresenceOrderFixture f;
    int kind = CAKRetake;
    SECTION("town support") {}
    SECTION("base recapture")
    {
        kind = CAKBase;
    }
    f.Hold(kind);

    const CounterAttackView near = f.At(1000 + f.registry.Tuning().cacheRadius);
    const CounterAttackView outside = f.At(near.playerX + 0.25f);
    // The force tick still says off-screen.  Entering, including the exact
    // boundary, must stop the invisible force before support/meter changes.
    CHECK(f.counter.AbstractPresence(f.registry, 0, &near) == 0);
    f.TickZone(near);
    CHECK(f.Target().support == Approx(60.0f));
    CHECK(f.Target().occCapture == Approx(0.0f));
    CHECK_FALSE(f.counter.Get(0)->suspended); // a census does not advance the force

    f.counter.Step(f.registry, near, 1.0f, nullptr, f.fired);
    REQUIRE(f.counter.Get(0)->suspended);
    CHECK(f.counter.AbstractPresence(f.registry, 0) == 0); // last force-tick snapshot
    // The player leaves before the next force tick.  It must resume now,
    // even though the stored suspension flag has not yet caught up.
    CHECK(f.counter.AbstractPresence(f.registry, 0, &outside) == 5);
    f.TickZone(outside);
    if (kind == CAKRetake)
    {
        CHECK(f.Target().support == Approx(55.0f));
    }
    else
    {
        CHECK(f.Target().occCapture == Approx(18.0f));
    }
    CHECK(f.counter.Get(0)->suspended);
}

TEST_CASE("CounterAttack - current bubble follows the holding HQ destination",
          "[game][guerrilla][counterattack][town-presence-order]")
{
    PresenceOrderFixture f;
    f.Hold(CAKHqAssault);
    // The HQ is 100 m east of its zone centre.  Each sample is exactly at
    // one bubble edge, with the other destination outside it.
    const CounterAttackView nearHq = f.At(1100 + f.registry.Tuning().cacheRadius);
    CHECK(f.counter.AbstractPresence(f.registry, 0, &nearHq) == 0);
    f.counter.Step(f.registry, nearHq, 1.0f, nullptr, f.fired);
    REQUIRE(f.counter.Get(0)->suspended);
    const CounterAttackView nearZone = f.At(1000 - f.registry.Tuning().cacheRadius);
    CHECK(f.counter.AbstractPresence(f.registry, 0, &nearZone) == 5);
}
