#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp> // CounterAttackGate (S5 release)
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp>
#include <Poseidon/Game/Guerrilla/OccupierForces.hpp> // the pool seams (D2 Draw, refunds)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // ExtParsMission (load-pass config rebuild)
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <filesystem>
#include <functional>
#include <math.h>
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

// S5 fixture: a post near the Camp, a second post, a town and a far post
const char* kDepositConfig =
    "class CfgGuerrillaZones\n"
    "{\n"
    "    class Zones\n"
    "    {\n"
    "        class Camp    { name=\"Camp\";    type=\"CAMP\";    owner=\"GUER\"; "
    "position[]={1000.0, 1000.0, 0.0}; };\n"
    "        class Post    { name=\"Post\";    type=\"OUTPOST\"; owner=\"EAST\"; garrison=6; "
    "position[]={1600.0, 1000.0, 0.0}; };\n"
    "        class Base    { name=\"Base\";    type=\"AIRFIELD\"; owner=\"EAST\"; garrison=10; "
    "position[]={5000.0, 5000.0, 0.0}; };\n"
    "        class Town    { name=\"Town\";    type=\"CITY\";    owner=\"NEUTRAL\"; support=20; "
    "position[]={1000.0, 1400.0, 0.0}; };\n"
    "    };\n"
    "};\n"
    "class CfgGuerrillaFactions { class East { side=\"EAST\"; tiers[]={\"SoldierEB\"}; }; };\n";

struct DepositFixture
{
    ParamFile file;
    ZoneRegistry registry;
    int camp = -1, post = -1, base = -1, town = -1;

    DepositFixture()
    {
        QIStream in(kDepositConfig, strlen(kDepositConfig));
        file.Parse(in);
        registry.LoadFromParams(file.FindEntry("CfgGuerrillaZones"), file.FindEntry("CfgGuerrillaFactions"));
        camp = registry.FindZoneIndex("Camp");
        post = registry.FindZoneIndex("Post");
        base = registry.FindZoneIndex("Base");
        town = registry.FindZoneIndex("Town");
        REQUIRE(town >= 0);
    }
    ZoneRecord& Z(int i) { return *registry.GetZoneMutable(i); }
};

struct FakeGate final : CounterAttackGate
{
    std::function<int(Vector3Par)> exposure;
    mutable int calls = 0;
    explicit FakeGate(std::function<int(Vector3Par)> e) : exposure(std::move(e)) {}
    int Exposure(Vector3Par pos) const override
    {
        calls++;
        return exposure(pos);
    }
};

// a refund hook for the phase-2 seam test
int GHookBodies = 0;
int GHookOrigin = -2;
bool TestRefundHook(ZoneRegistry&, int, int originZone, int bodies)
{
    GHookBodies += bodies;
    GHookOrigin = originZone;
    return true;
}

} // namespace

TEST_CASE("GarrisonCache - S5: routing a reserve write by the deposit bubble", "[game][guerrilla]")
{
    DepositFixture f;
    const Vector3 post = f.Z(f.post).pos;
    // the player at the Camp, 600 m from the Post: inside cacheRadius + 50
    CHECK(GarrisonCache::InDepositBubble(f.Z(f.post), true, 1000.0f, 1000.0f, kRadius));
    CHECK_FALSE(GarrisonCache::InDepositBubble(f.Z(f.base), true, 1000.0f, 1000.0f, kRadius));
    // the edge is the despawn edge (850 m), not the spawn edge (800 m)
    CHECK(GarrisonCache::InDepositBubble(f.Z(f.post), true, post.X() - 849.0f, post.Z(), kRadius));
    CHECK_FALSE(GarrisonCache::InDepositBubble(f.Z(f.post), true, post.X() - 851.0f, post.Z(), kRadius));
    CHECK_FALSE(GarrisonCache::InDepositBubble(f.Z(f.post), false, post.X(), post.Z(), kRadius));

    // inside: pending, the reserve untouched
    CHECK(GarrisonCache::AddReserve(f.registry, f.post, 3, f.base, true, 1000.0f, 1000.0f));
    CHECK(f.Z(f.post).garrison == Approx(6.0f));
    CHECK(f.Z(f.post).pending == 3);
    CHECK(f.Z(f.post).pendingOrigin == f.base);
    // a second deposit from elsewhere marks it mixed
    GarrisonCache::Deposit(f.registry, f.post, 1, f.town);
    CHECK(f.Z(f.post).pending == 4);
    CHECK(f.Z(f.post).pendingOrigin == -1);
    // outside: a direct reserve write, as before S5
    CHECK_FALSE(GarrisonCache::AddReserve(f.registry, f.base, 2, -1, true, 1000.0f, 1000.0f));
    CHECK(f.Z(f.base).garrison == Approx(12.0f));
    CHECK(f.Z(f.base).pending == 0);
    // nothing to write: no-op
    CHECK_FALSE(GarrisonCache::AddReserve(f.registry, f.base, 0, -1, true, 1000.0f, 1000.0f));
    CHECK(f.Z(f.base).garrison == Approx(12.0f));
}

TEST_CASE("GarrisonCache - S5: a pending deposit neither defends nor materializes", "[game][guerrilla]")
{
    DepositFixture f;
    f.Z(f.post).garrison = 0;
    GarrisonCache::Deposit(f.registry, f.post, 5, f.post);
    // the cache decision reads the reserve only: nothing to spawn
    GarrisonDecisionInput in = MakeInput(true, false, f.Z(f.post).garrison, true, 100.0f);
    CHECK(GarrisonCache::Decide(in, kRadius, kHyst) == GActNone);
    // the capture meter climbs as against an empty post
    AutoArray<ZoneEventRecord> fired;
    ZoneTickInputs tick;
    tick.guerCount.Resize(f.registry.NZones());
    tick.occCount.Resize(f.registry.NZones());
    for (int i = 0; i < f.registry.NZones(); i++)
    {
        tick.guerCount[i] = 0;
        tick.occCount[i] = 0;
    }
    tick.playerValid = true;
    tick.playerX = f.Z(f.post).pos.X();
    tick.playerZ = f.Z(f.post).pos.Z();
    tick.guerCount[f.post] = 1;
    f.registry.EvaluateTick(tick, fired);
    CHECK(f.Z(f.post).capture > 0);
    CHECK(f.Z(f.post).pending == 5);
    // a town: the support gate ignores it too
    f.Z(f.town).garrison = 0;
    GarrisonCache::Deposit(f.registry, f.town, 4, -1);
    tick.guerCount[f.post] = 0;
    tick.guerCount[f.town] = 1;
    tick.playerX = f.Z(f.town).pos.X();
    tick.playerZ = f.Z(f.town).pos.Z();
    f.registry.EvaluateTick(tick, fired);
    CHECK(f.Z(f.town).support == Approx(25.0f));
}

TEST_CASE("GarrisonCache - S5: release on leaving the bubble or when every spawn point is hidden",
          "[game][guerrilla]")
{
    DepositFixture f;
    GarrisonCache::Deposit(f.registry, f.post, 3, f.post);
    const Vector3 post = f.Z(f.post).pos;
    const float spread = f.registry.Tuning().zoneArea / 3.0f;

    SECTION("the player in the bubble, no gate (unit test or no world): it waits")
    {
        CHECK(GarrisonCache::FlushPending(f.registry, f.post, true, 1000.0f, 1000.0f, nullptr) == PFNone);
        CHECK(f.Z(f.post).pending == 3);
        CHECK(f.Z(f.post).garrison == Approx(6.0f));
    }
    SECTION("the player beyond the despawn edge: it joins the reserve")
    {
        CHECK(GarrisonCache::FlushPending(f.registry, f.post, true, post.X() - 851.0f, post.Z(), nullptr) ==
              PFReleasedBubble);
        CHECK(f.Z(f.post).pending == 0);
        CHECK(f.Z(f.post).pendingOrigin == -1);
        CHECK(f.Z(f.post).garrison == Approx(9.0f));
    }
    SECTION("every sample hidden: it joins the reserve (and may materialize)")
    {
        FakeGate hidden([](Vector3Par) { return (int)CAExposeHidden; });
        CHECK(GarrisonCache::FlushPending(f.registry, f.post, true, 1000.0f, 1000.0f, &hidden) == PFReleasedHidden);
        CHECK(hidden.calls == 9);
        CHECK(f.Z(f.post).garrison == Approx(9.0f));
        CHECK(f.Z(f.post).pending == 0);
    }
    SECTION("one cone-only sample (a corner of the placement square): it stays pending")
    {
        const Vector3 corner(post.X() + spread, 0, post.Z() + spread);
        FakeGate oneCone(
            [corner](Vector3Par p)
            {
                const bool atCorner = fabsf(p.X() - corner.X()) < 0.5f && fabsf(p.Z() - corner.Z()) < 0.5f;
                return atCorner ? (int)CAExposeConeOnly : (int)CAExposeHidden;
            });
        CHECK_FALSE(GarrisonCache::PendingSpawnHidden(post, spread, oneCone));
        CHECK(GarrisonCache::FlushPending(f.registry, f.post, true, 1000.0f, 1000.0f, &oneCone) == PFNone);
        CHECK(f.Z(f.post).pending == 3);
        CHECK(f.Z(f.post).garrison == Approx(6.0f));
    }
    SECTION("the samples are the centre, the edge midpoints and the corners of the square")
    {
        int inside = 0;
        FakeGate probe(
            [&inside, post, spread](Vector3Par p)
            {
                if (fabsf(p.X() - post.X()) <= spread + 0.01f && fabsf(p.Z() - post.Z()) <= spread + 0.01f)
                {
                    inside++;
                }
                return (int)CAExposeHidden;
            });
        CHECK(GarrisonCache::PendingSpawnHidden(post, spread, probe));
        CHECK(inside == 9);
    }
    SECTION("a dead player freezes it")
    {
        CHECK(GarrisonCache::FlushPending(f.registry, f.post, false, 0, 0, nullptr) == PFNone);
        CHECK(f.Z(f.post).pending == 3);
    }
}

TEST_CASE("GarrisonCache - S5: an owner change refunds the deposit to its origin or discards it",
          "[game][guerrilla]")
{
    SECTION("origin still occupier-held: back to its reserve, capped at its strength")
    {
        DepositFixture f;
        f.Z(f.base).garrison = 7; // room for 3 of 10
        GarrisonCache::Deposit(f.registry, f.post, 5, f.base);
        f.Z(f.post).owner = "GUER"; // the Post fell
        CHECK(GarrisonCache::FlushPending(f.registry, f.post, true, 1000.0f, 1000.0f, nullptr) == PFRefunded);
        CHECK(f.Z(f.post).pending == 0);
        CHECK(f.Z(f.post).garrison == Approx(6.0f)); // the new owner gets nothing
        CHECK(f.Z(f.base).garrison == Approx(10.0f));
    }
    SECTION("origin lost (or the zone itself): discarded")
    {
        DepositFixture f;
        GarrisonCache::Deposit(f.registry, f.post, 5, f.base);
        f.Z(f.base).owner = "GUER";
        f.Z(f.post).owner = "GUER";
        f.Z(f.base).garrison = 0;
        GarrisonCache::RefundPending(f.registry, f.post, true, 1000.0f, 1000.0f);
        CHECK(f.Z(f.post).pending == 0);
        CHECK(f.Z(f.base).garrison == Approx(0.0f));

        DepositFixture g;
        GarrisonCache::Deposit(g.registry, g.post, 4, g.post); // a wiped garrison's remainder
        g.Z(g.post).owner = "GUER";
        CHECK(GarrisonCache::FlushPending(g.registry, g.post, true, 1000.0f, 1000.0f, nullptr) == PFRefunded);
        CHECK(g.Z(g.post).pending == 0);
    }
    SECTION("the capture flip refunds at once (EvaluateTick)")
    {
        DepositFixture f;
        f.Z(f.post).garrison = 0;
        f.Z(f.base).garrison = 5;
        GarrisonCache::Deposit(f.registry, f.post, 2, f.base);
        AutoArray<ZoneEventRecord> fired;
        ZoneTickInputs tick;
        tick.guerCount.Resize(f.registry.NZones());
        tick.occCount.Resize(f.registry.NZones());
        for (int i = 0; i < f.registry.NZones(); i++)
        {
            tick.guerCount[i] = 0;
            tick.occCount[i] = 0;
        }
        tick.playerValid = true;
        tick.playerX = f.Z(f.post).pos.X();
        tick.playerZ = f.Z(f.post).pos.Z();
        tick.guerCount[f.post] = 3;
        for (int i = 0; i < 10 && Str(f.Z(f.post).owner) == "EAST"; i++)
        {
            f.registry.EvaluateTick(tick, fired);
        }
        REQUIRE(Str(f.Z(f.post).owner) == "GUER");
        CHECK(f.Z(f.post).pending == 0);
        CHECK(f.Z(f.base).garrison == Approx(7.0f)); // the base is far from the player: direct
    }
    SECTION("the phase-2 hook takes the refund when set")
    {
        DepositFixture f;
        GarrisonCache::Deposit(f.registry, f.post, 4, f.base);
        f.Z(f.post).owner = "GUER";
        GHookBodies = 0;
        GHookOrigin = -2;
        GarrisonCache::SetPendingRefundHook(&TestRefundHook);
        GarrisonCache::RefundPending(f.registry, f.post, true, 1000.0f, 1000.0f);
        GarrisonCache::SetPendingRefundHook(nullptr);
        CHECK(GHookBodies == 4);
        CHECK(GHookOrigin == f.base);
        CHECK(f.Z(f.base).garrison == Approx(10.0f)); // untouched: the hook took them
        CHECK(GarrisonCache::GetPendingRefundHook() == nullptr);
    }
}

TEST_CASE("GarrisonCache - U4 count of live bodies near a zone is safe on an empty cache", "[game][guerrilla]")
{
    GarrisonCache cache;
    CHECK(cache.LiveCountWithin(0, Vector3(0, 0, 0), 800.0f) == 0);
    CHECK(cache.LiveCountWithin(-1, Vector3(0, 0, 0), 800.0f) == 0);
}

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
        f.Load("class CfgGuerrillaZones { cacheInterval = 8; groupSize = 6; garrisonRecoverInterval = 15; };\n");
        REQUIRE(f.cache.Tuning().cacheInterval == Approx(8.0f));
        REQUIRE(f.cache.Tuning().groupSize == 6);
        REQUIRE(f.cache.Tuning().recoverInterval == Approx(15.0f));
    }

    SECTION("D2 recovery defaults to one body per 600 s")
    {
        CacheFixture f;
        f.Load("class CfgGuerrillaZones { tickInterval = 3; };\n");
        REQUIRE(f.cache.Tuning().recoverInterval == Approx(600.0f));
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

// D2 (issue #71): a distant, calm, untouched occupier post regains one body
// per interval up to its garrisonMax
TEST_CASE("GarrisonCache - D2 reserve recovery step", "[game][guerrilla]")
{
    const float R = 800.0f;
    const float H = GarrisonCache::DespawnHysteresis;
    auto eligible = []()
    {
        GarrisonRecoverInput in;
        in.occupierOwned = true;
        in.spawned = false;
        in.reserve = 2;
        in.reserveMax = 6;
        in.playerValid = true;
        in.playerDistSq = 3000.0f * 3000.0f;
        in.alertGreen = true;
        in.capture = 0;
        in.resistancePresent = 0;
        return in;
    };

    SECTION("one body per full interval of calm, carried across passes")
    {
        GarrisonRecoverInput in = eligible();
        float clock = 0;
        int gained = 0;
        for (int pass = 0; pass < 119; pass++) // 595 s in 5 s passes
        {
            gained += GarrisonCache::StepRecover(in, 5.0f, 600.0f, R, H, clock);
        }
        REQUIRE(gained == 0);
        REQUIRE(clock == Approx(595.0f));
        REQUIRE(GarrisonCache::StepRecover(in, 5.0f, 600.0f, R, H, clock) == 1);
        REQUIRE(clock == Approx(0.0f));
    }

    SECTION("never past reserveMax; at strength the clock rests at 0")
    {
        GarrisonRecoverInput in = eligible();
        in.reserve = 5;
        float clock = 0;
        REQUIRE(GarrisonCache::StepRecover(in, 1800.0f, 600.0f, R, H, clock) == 1); // 3 intervals, room for 1
        REQUIRE(clock == Approx(0.0f));
        in.reserve = 6;
        REQUIRE(GarrisonCache::StepRecover(in, 600.0f, 600.0f, R, H, clock) == 0);
        REQUIRE(clock == Approx(0.0f));
    }

    SECTION("a long elapsed window regains several bodies")
    {
        GarrisonRecoverInput in = eligible();
        float clock = 0;
        REQUIRE(GarrisonCache::StepRecover(in, 1300.0f, 600.0f, R, H, clock) == 2);
        REQUIRE(clock == Approx(100.0f));
    }

    SECTION("every disqualifier restarts the clock")
    {
        auto check = [&](GarrisonRecoverInput in)
        {
            float clock = 500.0f;
            REQUIRE(GarrisonCache::StepRecover(in, 200.0f, 600.0f, R, H, clock) == 0);
            REQUIRE(clock == Approx(0.0f));
        };
        GarrisonRecoverInput in = eligible();
        in.occupierOwned = false; // liberated zones never re-garrison (no recapture in v1)
        check(in);
        in = eligible();
        in.spawned = true; // a live garrison never grows a body
        check(in);
        in = eligible();
        in.playerDistSq = (R + H) * (R + H); // at the despawn edge is not distant
        check(in);
        in = eligible();
        in.alertGreen = false;
        check(in);
        in = eligible();
        in.capture = 6; // a capture in progress
        check(in);
        in = eligible();
        in.resistancePresent = 1; // fighters standing in the post
        check(in);
        in = eligible();
        in.reserveMax = 0; // nothing authored to recover to
        check(in);
    }

    SECTION("a dead player freezes the clock instead of restarting it")
    {
        GarrisonRecoverInput in = eligible();
        in.playerValid = false;
        float clock = 500.0f;
        REQUIRE(GarrisonCache::StepRecover(in, 200.0f, 600.0f, R, H, clock) == 0);
        REQUIRE(clock == Approx(500.0f));
    }

    SECTION("interval 0 turns recovery off")
    {
        GarrisonRecoverInput in = eligible();
        float clock = 0;
        REQUIRE(GarrisonCache::StepRecover(in, 5000.0f, 0.0f, R, H, clock) == 0);
        REQUIRE(clock == Approx(0.0f));
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
    // reconciles to a despawned zone; the reserve stored beside it (here the
    // config 8, kept under U1) becomes a PENDING deposit (S5) that joins the
    // reserve on the first pass that finds it out of the player's sight
    CHECK_FALSE(loaded.IsSpawned(outpost));
    CHECK(loaded.LiveCount(outpost) == 0);
    const ZoneRecord* z = ZoneRegistry::Instance().GetZone(outpost);
    REQUIRE(z != nullptr);
    CHECK(z->garrison == Approx(0.0f));
    CHECK(z->pending == 8);
    CHECK(z->pendingOrigin == outpost);
    CHECK(z->liveOccupiers == Approx(0.0f));
    // the untouched zone keeps its config reserve
    CHECK(ZoneRegistry::Instance().GetZone(depot)->garrison == Approx(4.0f));

    // scrub the process-wide state other tests expect to be empty
    ExtParsMission.Clear();
    ZoneRegistry::Instance().Clear();
    std::filesystem::remove(archivePath);
}

TEST_CASE("GarrisonCache - occupier pool routing: D2 draws on the pool, a flipped garrison refunds it (5.3, 5.5)",
          "[game][guerrilla]")
{
    DepositFixture f;
    ZoneRecord& post = f.Z(f.post);

    // the pool inactive (the key off): everything granted, the legacy D2
    post.garrison = 2;
    post.recoverClock = 0;
    CHECK(GarrisonCache::ApplyRecover(post, 2, 600.0f) == 2);
    CHECK(post.garrison == Approx(4.0f));
    // ... and a flipped zone's survivors land on its record as before
    f.Z(f.base).owner = "GUER";
    f.Z(f.base).garrison = 0;
    GarrisonCache::WriteBackSurvivors(f.registry, f.base, 3, 1);
    CHECK(f.Z(f.base).garrison == Approx(3.0f));
    f.Z(f.base).owner = "EAST";
    f.Z(f.base).garrison = 10;

    // the pool active
    struct PoolOn
    {
        ParamFile file;
        explicit PoolOn(ZoneRegistry& registry)
        {
            OccupierForces& of = OccupierForces::Instance();
            of.Clear();
            const std::string cfg = "class CfgGuerrillaZones { occupierForces = 1; };\n";
            QIStream in(cfg.c_str(), cfg.size());
            file.Parse(in);
            of.LoadFromParams(file.FindEntry("CfgGuerrillaZones"));
            REQUIRE(of.Activate(registry));
        }
        ~PoolOn() { OccupierForces::Instance().Clear(); }
    } pool(f.registry);
    OccupierForces& of = OccupierForces::Instance();
    REQUIRE(of.Active());

    // D2 takes what the pool grants; an empty pool grants nothing and the
    // clock is clamped (due again at once, nothing banked)
    of.SetPool(1);
    post.garrison = 2;
    post.recoverClock = 0;
    CHECK(GarrisonCache::ApplyRecover(post, 2, 600.0f) == 1);
    CHECK(post.garrison == Approx(3.0f));
    CHECK(post.recoverClock == Approx(600.0f));
    CHECK(of.Pool() == 0);
    CHECK(GarrisonCache::ApplyRecover(post, 1, 600.0f) == 0);
    CHECK(post.garrison == Approx(3.0f));

    // a flipped base's live garrison goes to the pool, not onto the new
    // owner's record; the dead are counted as losses
    const int lostBefore = of.Lost();
    f.Z(f.base).owner = "GUER";
    f.Z(f.base).garrison = 0;
    GarrisonCache::WriteBackSurvivors(f.registry, f.base, 3, 2);
    CHECK(f.Z(f.base).garrison == Approx(0.0f));
    CHECK(of.Pool() == 3);
    CHECK(of.Lost() == lostBefore + 2);
    // a held zone writes its survivors back (U1)
    GarrisonCache::WriteBackSurvivors(f.registry, f.post, 2, 0);
    CHECK(post.garrison == Approx(5.0f));
    CHECK(of.Pool() == 3);
}
