// Occupier air (OCCUPIER-FORCES.md 3.1, 3.2, D3, D17, D20; the phase 6
// spike, OccupierAir): the LZ choice on the 300..500 m ring away from the
// reported threat (dry, gentle, object-free, the engine's gradient < 4 cell
// limit), the insertion state machine (approach, landing order, touchdown,
// the off-LZ failure, the unload, lift-off, an uncommanded landing, the
// crew-first removal only where unseen), the sortie machine (orbit, weapons
// free, home, removal), the limited reveal and the altitude-aware exposure
// geometry.  Pure: the world comes from fakes.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Game/Guerrilla/OccupierAir.hpp>

#include <math.h>
#include <string.h>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

// flat dry land everywhere, with optional sea west of seaX, a slope band and
// object cells
struct FakeProbe final : AirLzProbe
{
    float seaX = -1e9f;      // x below this is sea
    float slopeZ = 1e9f;     // z above this is steep
    float objectsX = 1e9f;   // x above this is wooded
    AirLzFacts Probe(Vector3Par pos, float /*clearRadius*/) const override
    {
        AirLzFacts f;
        f.inMap = pos.X() >= 0 && pos.Z() >= 0 && pos.X() < 12800 && pos.Z() < 12800;
        f.landY = pos.X() < seaX ? -2.0f : 20.0f;
        f.gradient = pos.Z() > slopeZ ? 0.4f : 0.05f;
        f.objects = pos.X() > objectsX ? 3 : 0;
        return f;
    }
};

// a camera at camPos looking along +z; a ridge along z = ridgeZ of height
// ridgeY hides points beyond it whose ray passes below the crest
struct FakeViewer final : AirViewer
{
    bool playerValid = true;
    Vector3 player = Vector3(1000, 20, 1000);
    bool hasCamera = true;
    Vector3 cam = Vector3(1000, 22, 1000);
    float minSpawn = 1200.0f;
    float audible = 800.0f;
    float ridgeZ = 1e9f;
    float ridgeY = 0;
    bool PlayerValid() const override { return playerValid; }
    Vector3 PlayerPos() const override { return player; }
    bool HasCamera() const override { return hasCamera; }
    Vector3 CamPos() const override { return cam; }
    bool InFrustum(Vector3Par pos) const override
    {
        // +-60 deg around +z
        const float dx = pos.X() - cam.X();
        const float dz = pos.Z() - cam.Z();
        const float len = sqrtf(dx * dx + dz * dz);
        return len < 1e-3f || dz / len >= 0.5f;
    }
    bool LosBlocked(Vector3Par p) const override
    {
        if (!(cam.Z() < ridgeZ && p.Z() > ridgeZ))
        {
            return false;
        }
        const float t = (ridgeZ - cam.Z()) / (p.Z() - cam.Z());
        const float yAtRidge = cam.Y() + (p.Y() - cam.Y()) * t;
        return yAtRidge < ridgeY;
    }
    float GroundY(float, float) const override { return 20.0f; }
    float MinSpawn() const override { return minSpawn; }
    float Audible() const override { return audible; }
};

AirFacts Flying(Vector3Par pos, float height = 50.0f, float speed = 40.0f)
{
    AirFacts f;
    f.hullAlive = f.hullExists = true;
    f.pilotAlive = true;
    f.crewAlive = 2;
    f.cargoAlive = 8;
    f.cargoSeated = 8;
    f.pos = pos;
    f.heightAgl = height;
    f.speed = speed;
    return f;
}

} // namespace

TEST_CASE("OccupierAir: the LZ ring, away from the threat first", "[game][guerrilla][occupierforces][air]")
{
    AirTuning t;
    FakeProbe probe;
    const Vector3 target(5000, 0, 5000);
    const Vector3 threat(5000, 0, 4000); // south of the target
    Vector3 lz;
    RString why;
    REQUIRE(OccupierAir::PickLzPure(target, &threat, t, probe, lz, why));
    const float d = sqrtf((lz.X() - target.X()) * (lz.X() - target.X()) + (lz.Z() - target.Z()) * (lz.Z() - target.Z()));
    CHECK(d >= t.lzMin - 0.01f);
    CHECK(d <= t.lzMax + 0.01f);
    CHECK(lz.Z() > target.Z() + 300.0f); // straight away from the threat: north
    CHECK(lz.Y() == Approx(20.0f));

    // no threat: the first spoke is +x
    REQUIRE(OccupierAir::PickLzPure(target, nullptr, t, probe, lz, why));
    CHECK(lz.X() > target.X() + 300.0f);
}

TEST_CASE("OccupierAir: the LZ refuses sea, slopes and objects", "[game][guerrilla][occupierforces][air]")
{
    AirTuning t;
    const Vector3 target(5000, 0, 5000);
    const Vector3 threat(5000, 0, 4000);
    Vector3 lz;
    RString why;
    // north is steep: the pick swings off the away spoke, never into the slope
    FakeProbe slope;
    slope.slopeZ = 5200;
    REQUIRE(OccupierAir::PickLzPure(target, &threat, t, slope, lz, why));
    CHECK(lz.Z() <= 5200.0f);
    // east wooded and west sea: only a narrow band stays
    FakeProbe mixed;
    mixed.objectsX = 5050;
    mixed.seaX = 4950;
    REQUIRE(OccupierAir::PickLzPure(target, &threat, t, mixed, lz, why));
    CHECK(lz.X() >= 4950.0f);
    CHECK(lz.X() <= 5050.0f);
    // everything wet: refused, with the reason counted
    FakeProbe sea;
    sea.seaX = 1e9f;
    int tried = 0;
    CHECK_FALSE(OccupierAir::PickLzPure(target, &threat, t, sea, lz, why, &tried));
    CHECK(tried == t.lzTries);
    CHECK(strstr((const char*)why, "water 48") != nullptr);
    // the candidates stay on the ring and fan around the away direction
    for (int i = 0; i < t.lzTries; i++)
    {
        float off = 0, r = 0;
        OccupierAir::LzCandidate(i, t, off, r);
        CHECK(r >= t.lzMin);
        CHECK(r <= t.lzMax);
        CHECK(fabsf(off) <= 3.1416f + 1e-3f);
    }
}

namespace
{
// the engine probe snaps a candidate to the centre of its 50 m terrain cell
struct SnapProbe final : AirLzProbe
{
    AirLzFacts Probe(Vector3Par pos, float /*clearRadius*/) const override
    {
        AirLzFacts f;
        f.landY = 20.0f;
        f.gradient = 0.05f;
        f.moved = true;
        f.x = (floorf(pos.X() / 50.0f) + 0.5f) * 50.0f;
        f.z = (floorf(pos.Z() / 50.0f) + 0.5f) * 50.0f;
        return f;
    }
};
} // namespace

TEST_CASE("OccupierAir: a probe-moved LZ is used and stays on the ring", "[game][guerrilla][occupierforces][air]")
{
    AirTuning t;
    SnapProbe probe;
    const Vector3 target(5010, 0, 5010);
    Vector3 lz;
    RString why;
    REQUIRE(OccupierAir::PickLzPure(target, nullptr, t, probe, lz, why));
    // the cell centre, not the raw ring point
    CHECK(fmodf(lz.X(), 50.0f) == Approx(25.0f));
    CHECK(fmodf(lz.Z(), 50.0f) == Approx(25.0f));
    const float d = sqrtf((lz.X() - target.X()) * (lz.X() - target.X()) + (lz.Z() - target.Z()) * (lz.Z() - target.Z()));
    CHECK(d >= t.lzMin);
    CHECK(d <= t.lzMax);
    // a ring so thin no cell centre lands on it: every candidate refused
    t.lzMin = 400.0f;
    t.lzMax = 400.5f;
    int tried = 0;
    CHECK_FALSE(OccupierAir::PickLzPure(target, nullptr, t, probe, lz, why, &tried));
    CHECK(tried == t.lzTries);
}

TEST_CASE("OccupierAir: an insertion lands, unloads, departs, removed crew first", "[game][guerrilla][occupierforces][air]")
{
    AirTuning t;
    AirInsertionCore c;
    c.lz = Vector3(5000, 20, 5400);
    c.home = Vector3(3000, 0, 5400);
    const float dt = 0.5f;

    // inbound far away: nothing to do
    CHECK(OccupierAir::StepInsertionPure(c, Flying(Vector3(3500, 0, 5400)), dt, t) == AANone);
    CHECK(c.state == AISInbound);
    // inside the landing order distance: LAND once
    int a = OccupierAir::StepInsertionPure(c, Flying(Vector3(4600, 0, 5400)), dt, t);
    CHECK((a & AALand) != 0);
    CHECK(c.state == AISLanding);
    // low but still moving: not a touchdown
    CHECK(OccupierAir::StepInsertionPure(c, Flying(Vector3(4980, 0, 5400), 3.0f, 8.0f), dt, t) == AANone);
    // low and slow for TouchHold: the touchdown, on the pad -> unload
    CHECK(OccupierAir::StepInsertionPure(c, Flying(Vector3(4990, 0, 5410), 1.0f, 0.5f), dt, t) == AANone);
    a = OccupierAir::StepInsertionPure(c, Flying(Vector3(4990, 0, 5410), 1.0f, 0.2f), dt, t);
    CHECK((a & AAUnload) != 0);
    CHECK(c.state == AISUnloading);
    CHECK(c.landedOffset == Approx(sqrtf(200.0f)));
    CHECK(c.landings == 1);
    // cargo still seated: a retry after UnloadRetry
    AirFacts down = Flying(Vector3(4990, 0, 5410), 1.0f, 0.0f);
    int retries = 0;
    for (int i = 0; i < 8; i++)
    {
        retries += (OccupierAir::StepInsertionPure(c, down, dt, t) & AAUnload) ? 1 : 0;
    }
    CHECK(retries == 1);
    // everyone out: Move home; the pad stays until the hull is gone
    down.cargoSeated = 0;
    a = OccupierAir::StepInsertionPure(c, down, dt, t);
    CHECK((a & AAMoveHome) != 0);
    CHECK((a & AARemovePad) == 0);
    CHECK(c.state == AISDeparting);
    // lifting off
    OccupierAir::StepInsertionPure(c, Flying(Vector3(4900, 0, 5410), 30.0f, 10.0f), dt, t);
    CHECK(c.liftedOffAt > 0);
    // seen: never removed
    for (int i = 0; i < 20; i++)
    {
        a = OccupierAir::StepInsertionPure(c, Flying(Vector3(4000, 0, 5410)), dt, t);
        CHECK((a & (AARemoveCrew | AARemoveHull)) == 0);
    }
    // unseen for removeHold: the crew first, the hull one step later
    AirFacts gone = Flying(Vector3(3200, 0, 5410));
    gone.unseen = true;
    int crewAt = -1, hullAt = -1;
    for (int i = 0; i < 10 && hullAt < 0; i++)
    {
        a = OccupierAir::StepInsertionPure(c, gone, dt, t);
        if (a & AARemoveCrew)
        {
            CHECK((a & AARemoveHull) == 0);
            crewAt = i;
        }
        if (a & AARemoveHull)
        {
            hullAt = i;
            CHECK((a & AARemovePad) != 0); // the pad goes with the hull, after the crew
        }
    }
    CHECK(crewAt >= 3); // removeHold 2 s at 0.5 s steps
    CHECK(hullAt == crewAt + 1);
    CHECK(c.state == AISGone);
    CHECK(c.failReason == AFRNone);
    CHECK(OccupierAir::StepInsertionPure(c, gone, dt, t) == AANone);
}

TEST_CASE("OccupierAir: failures and the uncommanded landing", "[game][guerrilla][occupierforces][air]")
{
    AirTuning t;
    const float dt = 0.5f;
    SECTION("a touchdown past the tolerance fails, goes home, keeps its cargo")
    {
        AirInsertionCore c;
        c.lz = Vector3(5000, 20, 5000);
        c.state = AISLanding;
        AirFacts f = Flying(Vector3(5150, 0, 5000), 1.0f, 0.0f);
        OccupierAir::StepInsertionPure(c, f, dt, t);
        const int a = OccupierAir::StepInsertionPure(c, f, dt, t);
        CHECK(c.state == AISFailed);
        CHECK(c.failReason == AFROffLz);
        CHECK((a & AAMoveHome) != 0);
        CHECK((a & AAUnload) == 0);
    }
    SECTION("a wave-off over the pad: the landing order is re-issued until down")
    {
        AirInsertionCore c;
        c.lz = Vector3(5000, 20, 5000);
        int a = OccupierAir::StepInsertionPure(c, Flying(Vector3(5300, 0, 5000)), dt, t);
        REQUIRE((a & AALand) != 0);
        REQUIRE(c.landOrders == 1);
        int orders = 0;
        for (int i = 0; i < 40; i++) // 20 s hovering 100 m over the pad
        {
            a = OccupierAir::StepInsertionPure(c, Flying(Vector3(5000, 0, 5000), 100.0f, 0.0f), dt, t);
            orders += (a & AALand) ? 1 : 0;
        }
        CHECK(orders == 4); // every LandRetry (5 s)
        CHECK(c.landOrders == 5);
        CHECK(c.state == AISLanding);
    }
    SECTION("inbound too long times out")
    {
        AirInsertionCore c;
        c.lz = Vector3(9000, 20, 9000);
        int a = 0;
        for (int i = 0; i < 800 && c.state == AISInbound; i++)
        {
            a = OccupierAir::StepInsertionPure(c, Flying(Vector3(1000, 0, 1000)), dt, t);
        }
        CHECK(c.state == AISFailed);
        CHECK(c.failReason == AFRTimeout);
        CHECK((a & AAMoveHome) != 0);
    }
    SECTION("a dead pilot fails the record")
    {
        AirInsertionCore c;
        AirFacts f = Flying(Vector3(1000, 0, 1000));
        f.pilotAlive = false;
        OccupierAir::StepInsertionPure(c, f, dt, t);
        CHECK(c.state == AISFailed);
        CHECK(c.failReason == AFRCrewLost);
    }
    SECTION("a destroyed hull ends the record at once")
    {
        AirInsertionCore c;
        AirFacts f = Flying(Vector3(1000, 0, 1000));
        f.hullAlive = false;
        const int a = OccupierAir::StepInsertionPure(c, f, dt, t);
        CHECK(c.state == AISGone);
        CHECK(c.failReason == AFRHullLost);
        CHECK((a & AARemovePad) != 0);
        CHECK((a & (AARemoveCrew | AARemoveHull)) == 0);
    }
    SECTION("low and slow again after lift-off counts a landing and re-issues the Move")
    {
        AirInsertionCore c;
        c.state = AISDeparting;
        OccupierAir::StepInsertionPure(c, Flying(Vector3(1000, 0, 1000), 40.0f), dt, t);
        REQUIRE(c.liftedOffAt > 0);
        const int a = OccupierAir::StepInsertionPure(c, Flying(Vector3(1000, 0, 1000), 1.0f, 0.0f), dt, t);
        CHECK(c.landings == 1);
        CHECK((a & AAMoveHome) != 0);
        // staying down does not count it twice, but the Move home is
        // re-issued every LiftRetry while it stays down
        int moves = 0;
        for (int i = 0; i < 200; i++) // 100 s on the ground
        {
            const int b = OccupierAir::StepInsertionPure(c, Flying(Vector3(1000, 0, 1000), 1.0f, 0.0f), dt, t);
            moves += (b & AAMoveHome) ? 1 : 0;
        }
        CHECK(c.landings == 1);
        CHECK(moves == 2); // at 45 s and 90 s
    }
}

TEST_CASE("OccupierAir: a sortie orbits for its time, goes home, removed unseen", "[game][guerrilla][occupierforces][air]")
{
    AirTuning t;
    AirSortieCore c;
    c.target = Vector3(6000, 20, 6000);
    c.home = Vector3(6000, 0, 12000);
    c.sortieTime = 60.0f;
    const float dt = 0.5f;
    CHECK(OccupierAir::StepSortiePure(c, Flying(Vector3(6000, 0, 9000), 100.0f, 160.0f), dt, t) == AANone);
    int a = OccupierAir::StepSortiePure(c, Flying(Vector3(6000, 0, 7000), 100.0f, 160.0f), dt, t);
    CHECK(c.state == ASSLoiter);
    CHECK((a & AAWeaponsFree) != 0);
    CHECK((a & AAOrbit) != 0);
    int orbits = 0;
    while (c.state == ASSLoiter)
    {
        a = OccupierAir::StepSortiePure(c, Flying(Vector3(6500, 0, 6500), 100.0f, 160.0f), dt, t);
        orbits += (a & AAOrbit) ? 1 : 0;
    }
    CHECK(orbits == 2); // 60 s with a 20 s leg: two more legs before home
    CHECK(c.state == ASSHome);
    CHECK((a & AAMoveHome) != 0);
    CHECK_FALSE(c.touchedGround);
    CHECK(c.minHeightSeen == Approx(100.0f));
    AirFacts far = Flying(Vector3(6000, 0, 11000), 100.0f, 160.0f);
    far.unseen = true;
    int crew = -1, hull = -1;
    for (int i = 0; i < 10 && hull < 0; i++)
    {
        a = OccupierAir::StepSortiePure(c, far, dt, t);
        crew = (a & AARemoveCrew) ? i : crew;
        hull = (a & AARemoveHull) ? i : hull;
    }
    CHECK(hull == crew + 1);
    CHECK(c.state == ASSGone);
    // orbit points ring the target
    for (int i = 0; i < 8; i++)
    {
        const Vector3 p = OccupierAir::OrbitPoint(c.target, 900.0f, i);
        const float dx = p.X() - c.target.X();
        const float dz = p.Z() - c.target.Z();
        CHECK(sqrtf(dx * dx + dz * dz) == Approx(900.0f));
    }
}

TEST_CASE("OccupierAir: reveal only fresh, inexact sightings (D9)", "[game][guerrilla][occupierforces][air]")
{
    AirTuning t;
    CHECK(OccupierAir::RevealAllowed(10.0f, 0.5f, t));
    CHECK_FALSE(OccupierAir::RevealAllowed(61.0f, 0.5f, t));
    CHECK_FALSE(OccupierAir::RevealAllowed(10.0f, 1.0f, t)); // an exact fix never
    CHECK_FALSE(OccupierAir::RevealAllowed(-1.0f, 0.5f, t));
}

TEST_CASE("OccupierAir: exposure at altitude", "[game][guerrilla][occupierforces][air]")
{
    FakeViewer v;
    // inside the air audible floor: seen even behind the camera
    CHECK(OccupierAir::ExposureAt(v, Vector3(1000, 120, 500)) == CAExposeSeen);
    // in front, inside the band: seen; behind: cone-only
    CHECK(OccupierAir::ExposureAt(v, Vector3(1000, 120, 2000)) == CAExposeSeen);
    CHECK(OccupierAir::ExposureAt(v, Vector3(1000, 120, 100)) == CAExposeConeOnly);
    // past the band in 3D: hidden, the sea included (off-map is not special)
    CHECK(OccupierAir::ExposureAt(v, Vector3(1000, 120, 2300)) == CAExposeHidden);
    CHECK(OccupierAir::ExposureAt(v, Vector3(-400, 0, 1000)) == CAExposeHidden);
    // a ridge 40 m high at z = 1500 hides a helicopter low behind it, not one high above it
    v.ridgeZ = 1500;
    v.ridgeY = 60;
    CHECK(OccupierAir::ExposureAt(v, Vector3(1000, 30, 2000)) == CAExposeHidden);
    CHECK(OccupierAir::ExposureAt(v, Vector3(1000, 150, 2000)) == CAExposeSeen);
    // the despawn rule: never in the cone, never inside the audible floor
    CHECK_FALSE(OccupierAir::UnseenAt(v, Vector3(1000, 150, 2000)));
    CHECK_FALSE(OccupierAir::UnseenAt(v, Vector3(1000, 30, 300)));       // behind but audible
    CHECK(OccupierAir::UnseenAt(v, Vector3(1000, 120, -500)));           // behind, past the band
    // past the band in 3D, even in the cone: a speck at altitude is not drawn
    // (phase 6A, the orchestrator's decision on the spike's open item)
    CHECK(OccupierAir::UnseenAt(v, Vector3(1000, 120, 2300)));
    v.player = Vector3(1000, 20, 1000);
    v.ridgeZ = 0; // a ridge behind the camera at z = 0
    v.ridgeY = 200;
    v.cam = Vector3(1000, 22, 1000);
    // (the fake ridge only blocks forward rays; behind and inside the band stays seen)
    CHECK_FALSE(OccupierAir::UnseenAt(v, Vector3(1000, 120, 100)));
    // no player: nothing may be removed, everything counts as seen
    v.playerValid = false;
    CHECK(OccupierAir::ExposureAt(v, Vector3(1000, 120, 9000)) == CAExposeSeen);
    CHECK_FALSE(OccupierAir::UnseenAt(v, Vector3(1000, 120, 9000)));
    // the gate interface is the same rule
    v.playerValid = true;
    const CounterAttackGate& gate = v;
    CHECK(gate.Exposure(Vector3(1000, 120, 2300)) == CAExposeHidden);
}
