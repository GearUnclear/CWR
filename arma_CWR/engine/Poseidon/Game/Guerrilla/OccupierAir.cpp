#include <Poseidon/Game/Guerrilla/OccupierAir.hpp>

#include <Poseidon/Game/Guerrilla/Traffic.hpp>      // PerceptionCamera, EffectiveBand (issue #53)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp> // EnsureSideCenter, CreateSideGroup, faction squads

#include <Poseidon/IO/ParamFileExt.hpp> // Pars (CfgVehicles crew)
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/Core/Application.hpp>         // GApp (ENGINE_CONFIG)
#include <Poseidon/Core/Config/EngineConfig.hpp> // ENGINE_CONFIG.objectsZ / horizontZ

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/World/Entities/Vehicles/Transport.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/AICore.hpp>
#include <Poseidon/AI/VehicleAI.hpp> // Rank
#include <Poseidon/Network/Network.hpp>

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toIntFloor
#include <Poseidon/Foundation/Containers/BoolArray.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <string.h>

// Shared command internals (global namespace, the CounterAttack idiom):
// createUnit / deleteVehicle without the script parsing, and moveIn*.
void CreateUnit(AIGroup* group, RString type, Vector3Par position, RString init, float skill, Rank rank);
void DeleteVehicle(Entity* veh);
bool NativeMoveIn(Poseidon::Person* soldier, Poseidon::Transport* veh, GetInPosition position);

namespace Poseidon::Guerrilla
{

// Defined in OccupierAirCommands.cpp (keeps the command TU in the link).
void EnsureOccupierAirCommandsLinked();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
OccupierAir& OccupierAir::Instance()
{
    EnsureOccupierAirCommandsLinked();
    static OccupierAir instance;
    return instance;
}
#pragma clang diagnostic pop

namespace
{

float Dist2DSq(Vector3Par a, Vector3Par b)
{
    const float dx = a.X() - b.X();
    const float dz = a.Z() - b.Z();
    return dx * dx + dz * dz;
}

float Dist2D(Vector3Par a, Vector3Par b)
{
    return sqrtf(Dist2DSq(a, b));
}

constexpr float Pi = 3.14159265f;

} // namespace

// ---------------------------------------------------------------------------
// pure rules
// ---------------------------------------------------------------------------

int AirViewer::Exposure(Vector3Par pos) const
{
    return OccupierAir::ExposureAt(*this, pos);
}

int OccupierAir::ExposureAt(const AirViewer& v, Vector3Par pos)
{
    if (!v.PlayerValid())
    {
        return CAExposeSeen;
    }
    const float audible = v.Audible();
    if (Dist2DSq(pos, v.PlayerPos()) < audible * audible)
    {
        return CAExposeSeen; // a rotor or a jet is heard well past where it is drawn
    }
    const Vector3 eye = v.HasCamera() ? v.CamPos() : v.PlayerPos();
    const float bound = v.MinSpawn();
    if ((pos - eye).SquareSize() >= bound * bound)
    {
        return CAExposeHidden; // past the object cull + margin, in 3D: not drawn
    }
    if (v.LosBlocked(pos))
    {
        return CAExposeHidden; // a ridge between the camera and the airframe at its altitude
    }
    return v.InFrustum(pos) ? CAExposeSeen : CAExposeConeOnly;
}

bool OccupierAir::UnseenAt(const AirViewer& v, Vector3Par pos)
{
    if (!v.PlayerValid())
    {
        return false;
    }
    const float audible = v.Audible();
    if (Dist2DSq(pos, v.PlayerPos()) < audible * audible)
    {
        return false;
    }
    const float bound = v.MinSpawn();
    if (!v.HasCamera())
    {
        return Dist2DSq(pos, v.PlayerPos()) > bound * bound;
    }
    // the aircraft rule (phase 6A, the orchestrator's decision on the
    // spike's open item): past the band's bound in 3D from the camera an
    // airframe is not drawn, so it is unseen even inside the view cone (a
    // speck at altitude).  Closer: Traffic::CanExposeDespawn at altitude,
    // outside the view cone AND terrain-hidden.
    const float d2 = (pos - v.CamPos()).SquareSize();
    if (d2 >= bound * bound)
    {
        return true;
    }
    if (v.InFrustum(pos))
    {
        return false;
    }
    return v.LosBlocked(pos);
}

void OccupierAir::LzCandidate(int i, const AirTuning& t, float& angleOffset, float& radius)
{
    // angles fan out from the away direction: 0, +30, -30, +60, -60, ... deg
    // (12 spokes cover the circle), each spoke tried at the middle radius,
    // then the inner and the outer edge of the ring
    const int spokes = 12;
    const int spoke = i % spokes;
    const int ring = (i / spokes) % 4;
    const int k = (spoke + 1) / 2;
    const float sign = (spoke % 2 == 1) ? 1.0f : -1.0f;
    angleOffset = spoke == 0 ? 0.0f : sign * (float)k * (Pi / 6.0f);
    const float mid = 0.5f * (t.lzMin + t.lzMax);
    const float span = t.lzMax - t.lzMin;
    static const float ringFrac[4] = {0.0f, -0.35f, 0.35f, 0.15f};
    radius = mid + ringFrac[ring] * span;
}

bool OccupierAir::PickLzPure(Vector3Par target, const Vector3* awayFrom, const AirTuning& t, const AirLzProbe& probe,
                             Vector3& lz, RString& why, int* tried)
{
    float baseAngle = 0; // +x
    if (awayFrom)
    {
        const float dx = target.X() - awayFrom->X();
        const float dz = target.Z() - awayFrom->Z();
        if (dx * dx + dz * dz > 1.0f)
        {
            baseAngle = atan2f(dz, dx); // from the threat through the target and beyond
        }
    }
    int fails[4] = {0, 0, 0, 0}; // map, water, slope, objects
    const int tries = t.lzTries > 0 ? t.lzTries : 1;
    for (int i = 0; i < tries; i++)
    {
        float off = 0, r = 0;
        LzCandidate(i, t, off, r);
        const float a = baseAngle + off;
        Vector3 p(target.X() + cosf(a) * r, 0.0f, target.Z() + sinf(a) * r);
        const AirLzFacts f = probe.Probe(p, t.lzClear);
        if (f.moved)
        {
            p = Vector3(f.x, 0.0f, f.z);
            const float d2 = Dist2DSq(p, target);
            if (d2 < t.lzMin * t.lzMin || d2 > t.lzMax * t.lzMax)
            {
                fails[0]++; // moved off the ring: counted with the map misses
                if (tried)
                {
                    *tried = i + 1;
                }
                continue;
            }
        }
        if (tried)
        {
            *tried = i + 1;
        }
        if (!f.inMap)
        {
            fails[0]++;
            continue;
        }
        if (f.landY < t.lzMinDry)
        {
            fails[1]++;
            continue;
        }
        if (f.gradient > t.lzMaxGradient)
        {
            fails[2]++;
            continue;
        }
        if (f.objects > 0)
        {
            fails[3]++;
            continue;
        }
        lz = Vector3(p.X(), f.landY, p.Z());
        why = "ok";
        return true;
    }
    char buf[160];
    snprintf(buf, sizeof(buf), "no LZ in %d tries (off map %d, water %d, slope %d, objects %d)", tries, fails[0],
             fails[1], fails[2], fails[3]);
    why = buf;
    return false;
}

Vector3 OccupierAir::OrbitPoint(Vector3Par target, float radius, int i)
{
    const float a = (float)(i % 8) * (Pi / 4.0f);
    return Vector3(target.X() + cosf(a) * radius, target.Y(), target.Z() + sinf(a) * radius);
}

bool OccupierAir::RevealAllowed(float ageSeconds, float accuracy, const AirTuning& t)
{
    return ageSeconds >= 0 && ageSeconds < t.revealMaxAge && accuracy < t.revealMaxAccuracy;
}

const char* OccupierAir::StateName(int state)
{
    static const char* names[NAirInsertionStates] = {"INBOUND", "LANDING", "UNLOADING", "DEPARTING", "GONE", "FAILED"};
    return state >= 0 && state < NAirInsertionStates ? names[state] : "?";
}

const char* OccupierAir::SortieStateName(int state)
{
    static const char* names[NAirSortieStates] = {"INBOUND", "LOITER", "HOME", "GONE", "FAILED"};
    return state >= 0 && state < NAirSortieStates ? names[state] : "?";
}

const char* OccupierAir::FailName(int reason)
{
    static const char* names[NAirFailReasons] = {"", "off LZ", "timeout", "hull lost", "crew lost"};
    return reason >= 0 && reason < NAirFailReasons ? names[reason] : "?";
}

int OccupierAir::StepInsertionPure(AirInsertionCore& c, const AirFacts& f, float dt, const AirTuning& t)
{
    if (c.hullRemoved)
    {
        return AANone;
    }
    c.age += dt;
    c.stateTime += dt;
    auto enter = [&c](int s) {
        c.state = s;
        c.stateTime = 0;
        c.touchFor = 0;
    };
    // the pad stays until the hull is gone (crew, then hull, then pad)
    auto fail = [&](int reason) {
        c.failReason = reason;
        enter(AISFailed);
        return (int)AAMoveHome;
    };
    // the second half of the removal: the hull, one step after the crew
    if (c.crewRemoved)
    {
        c.hullRemoved = true;
        enter(AISGone);
        return AARemoveHull | AARemovePad;
    }
    if (!f.hullAlive)
    {
        // deleted or wrecked under the record: nothing left to remove but the pad
        if (c.failReason == AFRNone && c.state != AISDeparting)
        {
            c.failReason = AFRHullLost;
        }
        c.crewRemoved = true;
        c.hullRemoved = true;
        enter(AISGone);
        return AARemovePad;
    }
    const bool low = f.heightAgl <= t.landedHeight && f.speed <= t.landedSpeed;
    c.touchFor = low ? c.touchFor + dt : 0.0f;
    const bool touchdown = c.touchFor >= TouchHold;
    int act = AANone;
    if (!f.pilotAlive && c.state <= AISUnloading)
    {
        // nobody flies it home: removed where unseen, as a failure
        c.failReason = AFRCrewLost;
        enter(AISFailed);
    }
    switch (c.state)
    {
        case AISInbound:
            if (Dist2D(f.pos, c.lz) <= t.landOrderDist)
            {
                enter(AISLanding);
                c.landOrderAt = c.age;
                c.landOrders++;
                act |= AALand;
            }
            else if (c.stateTime > t.inboundTimeout)
            {
                act |= fail(AFRTimeout);
            }
            break;
        case AISLanding:
            if (touchdown)
            {
                c.landed = true;
                c.landings++;
                c.landedAt = f.pos;
                c.landedOffset = Dist2D(f.pos, c.lz);
                if (c.landedOffset > t.landTolerance)
                {
                    act |= fail(AFROffLz);
                }
                else
                {
                    enter(AISUnloading);
                    c.unloadOrdered = true;
                    c.unloadAt = c.age;
                    act |= AAUnload;
                }
            }
            else if (c.stateTime > t.landingTimeout)
            {
                act |= fail(AFRTimeout);
            }
            else if (c.age - c.landOrderAt >= LandRetry)
            {
                c.landOrderAt = c.age;
                c.landOrders++;
                act |= AALand;
            }
            break;
        case AISUnloading:
            if (f.cargoSeated == 0)
            {
                enter(AISDeparting);
                act |= AAMoveHome;
            }
            else if (c.stateTime > t.unloadTimeout)
            {
                enter(AISDeparting); // leaves with what would not get out (counted in the rows)
                act |= AAMoveHome;
            }
            else if (f.heightAgl <= t.landedHeight && c.age - c.unloadAt >= UnloadRetry)
            {
                c.unloadAt = c.age;
                act |= AAUnload;
            }
            break;
        case AISDeparting:
        case AISFailed:
        {
            if (c.liftedOffAt < 0 && f.heightAgl > t.departHeight)
            {
                c.liftedOffAt = c.age;
            }
            if (c.liftedOffAt >= 0 && c.state == AISDeparting && c.touchFor > 0 && c.touchFor <= dt + 1e-3f)
            {
                // low and slow again after lift-off: an uncommanded landing starts
                c.landings++;
                act |= AAMoveHome;
            }
            else if (c.liftedOffAt >= 0 && c.touchFor >= LiftRetry && fmodf(c.touchFor, LiftRetry) < dt)
            {
                act |= AAMoveHome; // still down after an uncommanded landing: the Move again
            }
            if (c.liftedOffAt < 0 && c.stateTime >= LiftRetry && fmodf(c.stateTime, LiftRetry) < dt)
            {
                act |= AAMoveHome; // still on the ground: the Move again
            }
            c.unseenFor = f.unseen ? c.unseenFor + dt : 0.0f;
            const bool timedOut = c.stateTime > t.departTimeout && f.beyondBand;
            if (c.unseenFor >= t.removeHold || timedOut)
            {
                c.crewRemoved = true;
                act |= AARemoveCrew;
            }
            break;
        }
        default:
            break;
    }
    return act;
}

int OccupierAir::StepSortiePure(AirSortieCore& c, const AirFacts& f, float dt, const AirTuning& t)
{
    if (c.hullRemoved)
    {
        return AANone;
    }
    c.age += dt;
    c.stateTime += dt;
    auto enter = [&c](int s) {
        c.state = s;
        c.stateTime = 0;
    };
    if (c.crewRemoved)
    {
        c.hullRemoved = true;
        enter(ASSGone);
        return AARemoveHull;
    }
    if (!f.hullAlive)
    {
        if (c.failReason == AFRNone)
        {
            c.failReason = AFRHullLost;
        }
        c.crewRemoved = true;
        c.hullRemoved = true;
        enter(ASSGone);
        return AANone;
    }
    if (c.age > 5.0f)
    {
        c.minHeightSeen = f.heightAgl < c.minHeightSeen ? f.heightAgl : c.minHeightSeen;
        c.touchedGround = c.touchedGround || f.heightAgl < 3.0f;
    }
    int act = AANone;
    if (!f.pilotAlive && c.state < ASSHome)
    {
        c.failReason = AFRCrewLost;
        enter(ASSFailed);
    }
    switch (c.state)
    {
        case ASSInbound:
            if (Dist2D(f.pos, c.target) <= t.orbitRadius * 1.25f)
            {
                enter(ASSLoiter);
                c.orbitClock = 0;
                act |= AAWeaponsFree | AAOrbit;
            }
            else if (c.stateTime > t.sortieInboundTimeout)
            {
                c.failReason = AFRTimeout;
                enter(ASSFailed);
                act |= AAMoveHome;
            }
            break;
        case ASSLoiter:
            c.orbitClock += dt;
            if (c.stateTime >= c.sortieTime)
            {
                enter(ASSHome);
                act |= AAMoveHome;
            }
            else if (c.orbitClock >= t.orbitLeg)
            {
                c.orbitClock = 0;
                c.orbitIndex++;
                act |= AAOrbit;
            }
            break;
        case ASSHome:
        case ASSFailed:
        {
            c.unseenFor = f.unseen ? c.unseenFor + dt : 0.0f;
            const bool timedOut = c.stateTime > t.departTimeout && f.beyondBand;
            if (c.unseenFor >= t.removeHold || timedOut)
            {
                c.crewRemoved = true;
                act |= AARemoveCrew;
            }
            break;
        }
        default:
            break;
    }
    return act;
}

// ---------------------------------------------------------------------------
// engine
// ---------------------------------------------------------------------------

namespace
{

// the issue #53 camera and band, lifted to altitude
class EngineAirViewer final : public AirViewer
{
  public:
    void Refresh(const AirTuning& t)
    {
        _cam = PerceptionCamera::Capture();
        const TrafficTuning& tt = Traffic::Instance().Tuning();
        _band = Traffic::EffectiveBand(tt, ENGINE_CONFIG.objectsZ, false, ENGINE_CONFIG.horizontZ);
        _audible = t.airAudible > tt.minSpawnDist ? t.airAudible : tt.minSpawnDist;
        Person* player = GWorld ? GWorld->GetRealPlayer() : nullptr;
        _playerValid = player && !player->IsDammageDestroyed();
        _player = _playerValid ? player->Position() : VZero;
    }
    bool PlayerValid() const override { return _playerValid; }
    Vector3 PlayerPos() const override { return _player; }
    bool HasCamera() const override { return _cam.hasCamera; }
    Vector3 CamPos() const override { return _cam.hasCamera ? _cam.camPos : _player; }
    bool InFrustum(Vector3Par pos) const override { return _cam.InFrustum(pos, Traffic::FrustumCosHalfAngle); }
    bool LosBlocked(Vector3Par posAsl) const override { return _cam.LosBlocked(posAsl, 0.0f); }
    float GroundY(float x, float z) const override
    {
        return GLandscape ? GLOB_LAND->SurfaceYAboveWater(x, z) : 0.0f;
    }
    float MinSpawn() const override { return _band.minSpawn; }
    float Audible() const override { return _audible; }

  private:
    PerceptionCamera _cam;
    TrafficEffectiveBand _band;
    float _audible = 800.0f;
    bool _playerValid = false;
    Vector3 _player = VZero;
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
EngineAirViewer& Viewer()
{
    static EngineAirViewer viewer;
    return viewer;
}
#pragma clang diagnostic pop

// the terrain and the object grid at a candidate LZ.  The candidate moves to
// the centre of its terrain cell, and that cell must pass the engine's own
// landing-site test (HelicopterAuto::FindStopPosition fallback: no water, not
// full, no objects, gradient < 4); on top, no forest flag, and a sampled
// slope across the clear radius.  A 50 m object-free cell leaves at least
// 25 m to the nearest object around its centre, room for a rotor disc.
class EngineLzProbe final : public AirLzProbe
{
  public:
    AirLzFacts Probe(Vector3Par pos, float clearRadius) const override
    {
        AirLzFacts f;
        if (!GLandscape)
        {
            f.inMap = false;
            return f;
        }
        const int cx = toIntFloor(pos.X() * InvLandGrid);
        const int cz = toIntFloor(pos.Z() * InvLandGrid);
        if (!InRange(cz, cx))
        {
            f.inMap = false;
            return f;
        }
        const float x = ((float)cx + 0.5f) * LandGrid;
        const float z = ((float)cz + 0.5f) * LandGrid;
        f.moved = true;
        f.x = x;
        f.z = z;
        const float y0 = GLOB_LAND->SurfaceY(x, z);
        f.landY = y0;
        if (GLOB_LAND->WaterDepth(x, z) > 0.0f)
        {
            f.landY = -1.0f; // wet
        }
        // sampled slope: the worst rise over run from the centre to 8 points
        float worst = 0;
        for (int k = 0; k < 8; k++)
        {
            const float a = (float)k * (Pi / 4.0f);
            const float y = GLOB_LAND->SurfaceY(x + cosf(a) * clearRadius, z + sinf(a) * clearRadius);
            const float g = fabsf(y - y0) / clearRadius;
            worst = g > worst ? g : worst;
        }
        f.gradient = worst;
        const GeographyInfo g = GLOB_LAND->GetGeography(cx, cz);
        f.objects = (int)g.u.howManyObjects + (int)g.u.howManyHardObjects + (int)g.u.full + (int)g.u.forestInner +
                    (int)g.u.forestOuter;
        if (g.u.waterDepth > 0)
        {
            f.landY = -1.0f;
        }
        if (g.u.gradient >= 4)
        {
            f.gradient = f.gradient > 0.25f ? f.gradient : 0.25f + 1e-3f; // the engine refuses this cell
        }
        return f;
    }
};

RString SideCrewFallback(const RString& side)
{
    if (stricmp(side, "WEST") == 0)
    {
        return "SoldierWPilot";
    }
    if (stricmp(side, "EAST") == 0)
    {
        return "SoldierEPilot";
    }
    return "SoldierGB";
}

RString SideInfantryFallback(const RString& side)
{
    if (stricmp(side, "WEST") == 0)
    {
        return "SoldierWB";
    }
    if (stricmp(side, "EAST") == 0)
    {
        return "SoldierEB";
    }
    return "SoldierGB";
}

const ParamEntry* VehicleClass(const RString& cls)
{
    const ParamEntry* vehicles = Pars.FindEntry("CfgVehicles");
    const ParamEntry* c = vehicles && cls.GetLength() > 0 ? vehicles->FindEntry(cls) : nullptr;
    return c && c->IsClass() ? c : nullptr;
}

// a seated man leaves his seat first, at the hull, with no sound (the
// CounterAttack EngineOps::LeaveVehicle idiom): ::DeleteVehicle only flags a
// seated man, who then lingers in the world's out-vehicle list and reaches
// every save.  Living and dead alike.
void LeaveVehicle(Person* person)
{
    AIUnit* unit = person ? person->Brain() : nullptr;
    Transport* veh = unit ? unit->GetVehicleIn() : nullptr;
    if (!veh || !person->IsLocal())
    {
        return;
    }
    const Matrix4 at = veh->WorldTransform();
    if (veh->Driver() == person)
    {
        veh->GetOutDriver(at, false);
    }
    else if (veh->Gunner() == person)
    {
        veh->GetOutGunner(at, false);
    }
    else if (veh->Commander() == person)
    {
        veh->GetOutCommander(at, false);
    }
    else
    {
        veh->GetOutCargo(person, at, false);
    }
}

void DeletePerson(Person* person)
{
    if (!person)
    {
        return;
    }
    LeaveVehicle(person);
    if (person->IsLocal())
    {
        ::DeleteVehicle(person);
    }
    else
    {
        GetNetworkManager().AskForDeleteVehicle(person);
    }
}

void DeleteEntity(Entity* e)
{
    if (!e)
    {
        return;
    }
    if (e->IsLocal())
    {
        ::DeleteVehicle(e);
    }
    else
    {
        GetNetworkManager().AskForDeleteVehicle(e);
    }
}

void SetSemaphoreAll(AIGroup* grp, AI::Semaphore sem)
{
    grp->SetSemaphore(sem);
    PackedBoolArray all;
    for (int i = 0; i < MAX_UNITS_PER_GROUP; i++)
    {
        if (grp->UnitWithID(i + 1))
        {
            all.Set(i, true);
        }
    }
    grp->SendSemaphore(sem, all);
}

// one soldier of type in grp, seated at position (the CounterAttack /
// Traffic idiom); null when the class refused to materialize
Person* CreateSeated(AIGroup* grp, const RString& type, Vector3Par near, Transport* veh, int position, bool& seated,
                     float skill, Rank rank)
{
    seated = false;
    if (!grp || type.GetLength() == 0)
    {
        return nullptr;
    }
    AIUnit* before[MAX_UNITS_PER_GROUP];
    for (int i = 0; i < MAX_UNITS_PER_GROUP; i++)
    {
        before[i] = grp->UnitWithID(i + 1);
    }
    ::CreateUnit(grp, type, near, RString(), skill, rank);
    AIUnit* unit = nullptr;
    for (int i = 0; i < MAX_UNITS_PER_GROUP; i++)
    {
        AIUnit* u = grp->UnitWithID(i + 1);
        if (u && u != before[i])
        {
            unit = u;
            break;
        }
    }
    Person* person = unit ? unit->GetPerson() : nullptr;
    if (person && veh)
    {
        seated = ::NativeMoveIn(person, veh, (GetInPosition)position);
    }
    return person;
}

// an airframe born airborne the way the mission loader does it
// (AICenterImpl.cpp CreateVehicle, ASpFlying): oriented first (a plane's
// speed follows Direction()), the flying height set before MakeAirborne,
// which returns the height to add; then SetTransform, Init, AddVehicle
Transport* CreateAirborne(const RString& type, Vector3Par where, float altAgl, Vector3Par heading, RString& why)
{
    if (!GWorld || !GLandscape || type.GetLength() == 0)
    {
        why = "no world";
        return nullptr;
    }
    Ref<Entity> veh = NewNonAIVehicle(type, nullptr);
    if (!veh)
    {
        why = "class refused";
        return nullptr;
    }
    Transport* transport = dyn_cast<Transport>(veh.GetRef());
    EntityAI* ai = dyn_cast<EntityAI>(veh.GetRef());
    if (!transport || !ai || !veh->GetNonAIType()->IsKindOf(GWorld->Preloaded(VTypeAir)))
    {
        why = "not an aircraft";
        return nullptr;
    }
    Vector3 dir = heading;
    dir[1] = 0;
    dir = dir.SquareSize() > 1e-4f ? dir.Normalized() : VForward;
    Vector3 pos = where;
    pos[1] = GLOB_LAND->SurfaceYAboveWater(pos[0], pos[2]);
    Matrix3 orient;
    Matrix4 transform;
    orient.SetUpAndDirection(VUp, dir);
    transform.SetOrientation(orient);
    transform.SetPosition(pos);
    veh->SetTransform(transform);
    if (altAgl > 0)
    {
        ai->SetFlyingHeight(altAgl);
    }
    pos[1] += ai->MakeAirborne();
    transform.SetPosition(pos);
    veh->SetTransform(transform);
    veh->Init(transform);
    GWorld->AddVehicle(veh);
    if (GWorld->GetMode() == GModeNetware)
    {
        GetNetworkManager().CreateVehicle(veh, VLTVehicle, "", -1);
    }
    return transport;
}

class EngineAirOps final : public AirOps
{
  public:
    void Facts(Entity* hull, AIGroup* crew, AIGroup* cargo, const AirViewer& viewer, AirFacts& out) override
    {
        out = AirFacts();
        if (!hull)
        {
            return;
        }
        out.hullExists = true;
        out.hullAlive = !hull->IsDammageDestroyed();
        out.pos = hull->Position();
        out.speed = hull->Speed().Size();
        out.heightAgl = GLandscape ? out.pos.Y() - GLOB_LAND->SurfaceYAboveWater(out.pos.X(), out.pos.Z()) : 0.0f;
        Transport* t = dyn_cast<Transport>(hull);
        if (t)
        {
            Person* driver = t->Driver();
            out.pilotAlive = driver && !driver->IsDammageDestroyed() && t->DriverBrain();
        }
        for (int u = 0; crew && u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = crew->UnitWithID(u + 1);
            if (unit && unit->GetLifeState() == AIUnit::LSAlive && unit->GetVehicleIn() == hull)
            {
                out.crewAlive++;
            }
        }
        for (int u = 0; cargo && u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = cargo->UnitWithID(u + 1);
            if (unit && unit->GetLifeState() == AIUnit::LSAlive)
            {
                out.cargoAlive++;
                out.cargoSeated += unit->GetVehicleIn() == hull ? 1 : 0;
            }
        }
        out.unseen = OccupierAir::UnseenAt(viewer, out.pos);
        const float bound = viewer.MinSpawn();
        out.beyondBand = viewer.PlayerValid() && (out.pos - viewer.CamPos()).SquareSize() >= bound * bound;
    }

    // the Traffic::IssueRoute idiom: one Command::Move to the DRIVER through
    // IssueCommand (arcade waypoints never moved a seated crew).  A Move with
    // a path also restarts a landed helicopter's engine (AIUnitImpl EngineOn).
    void MoveTo(AIGroup* crew, Vector3Par target) override
    {
        if (!crew)
        {
            return;
        }
        Command cmd;
        cmd._message = Command::Move;
        float y = target.Y();
        if (GLandscape)
        {
            y = GLOB_LAND->SurfaceYAboveWater(target.X(), target.Z());
        }
        cmd._destination = Vector3(target.X(), y, target.Z());
        cmd._discretion = Command::Undefined;
        cmd._context = Command::CtxMission;
        cmd._id = crew->GetNextCmdId();
        PackedBoolArray drivers;
        bool any = false;
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = crew->UnitWithID(u + 1);
            if (!unit || unit->GetLifeState() != AIUnit::LSAlive)
            {
                continue;
            }
            Transport* in = unit->GetVehicleIn();
            if (in && in->DriverBrain() == unit)
            {
                drivers.Set(u, true);
                any = true;
            }
        }
        if (any)
        {
            crew->IssueCommand(cmd, drivers);
        }
    }

    // the plan's landing (1.3, the engine facts): the pad is a HeliH in the
    // BUILDINGS list within 500 m of the helicopter; LandStarted puts the
    // pilot into Stopping, FindStopPosition takes the nearest free pad
    void Land(Entity* hull, AIGroup* crew, Entity* pad, Vector3Par lz) override
    {
        (void)crew, (void)pad, (void)lz;
        Transport* t = dyn_cast<Transport>(hull);
        if (t)
        {
            t->LandStarted(Transport::LMLand);
        }
    }

    void CancelLand(Entity* hull) override
    {
        Transport* t = dyn_cast<Transport>(hull);
        if (t)
        {
            t->LandFinished(); // _landing = LMNone
            // nothing else ends a Stopping with no landing order: the pilot
            // would hold a 30 m hover over the spot (trial 13, AIR-SPIKE.md).
            // Wait is the engine's own Stopped -> Wait pass (AIUnitImpl
            // CheckGetOut); the Move that follows re-plans from there.
            AIUnit* pilot = t->PilotUnit();
            if (pilot && (pilot->GetState() == AIUnit::Stopping || pilot->GetState() == AIUnit::Stopped))
            {
                pilot->SetState(AIUnit::Wait);
            }
        }
    }

    // the phase 3 truck dismount plus a per-unit unassign, then the squad
    // walks on target
    void Unload(AIGroup* cargo, Entity* hull, Vector3Par target) override
    {
        if (!cargo)
        {
            return;
        }
        PackedBoolArray all;
        bool any = false;
        for (int u = 0; u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = cargo->UnitWithID(u + 1);
            if (!unit || unit->GetLifeState() != AIUnit::LSAlive)
            {
                continue;
            }
            Transport* seat = unit->GetVehicleIn();
            unit->OrderGetIn(false);
            unit->AllowGetIn(false);
            if (seat && (!hull || seat == hull))
            {
                unit->DoGetOut(seat, false);
            }
            // drop THIS unit's seat assignment (AIUnit::UnassignVehicle, never
            // the group-wide AIGroup::UnassignVehicle that clears the crew's
            // seats too): a cargo squad still assigned to the hull asks for a
            // pickup, and the departing pilot lands again beside it (trial 4,
            // AIR-SPIKE.md)
            if (hull && unit->VehicleAssigned() == hull)
            {
                unit->UnassignVehicle();
            }
            all.Set(u, true);
            any = true;
        }
        cargo->SetCombatModeMajor(CMAware);
        SetSemaphoreAll(cargo, AI::SemaphoreRed);
        if (any)
        {
            Command cmd;
            cmd._message = Command::Move;
            const float y = GLandscape ? GLOB_LAND->SurfaceYAboveWater(target.X(), target.Z()) : target.Y();
            cmd._destination = Vector3(target.X(), y, target.Z());
            cmd._discretion = Command::Undefined;
            cmd._context = Command::CtxMission;
            cmd._id = cargo->GetNextCmdId();
            cargo->IssueCommand(cmd, all);
        }
        GetNetworkManager().UpdateObject(cargo);
    }

    void WeaponsFree(AIGroup* crew) override
    {
        if (!crew)
        {
            return;
        }
        crew->SetCombatModeMajor(CMCombat);
        SetSemaphoreAll(crew, AI::SemaphoreRed);
        GetNetworkManager().UpdateObject(crew);
    }

    void RemoveCrew(AIGroup* crew, AIGroup* cargo, Entity* hull) override
    {
        // collect first - DeleteVehicle mutates the unit table
        AutoArray<Person*> bodies;
        for (int u = 0; crew && u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = crew->UnitWithID(u + 1);
            if (unit && unit->GetPerson())
            {
                bodies.Add(unit->GetPerson());
            }
        }
        for (int u = 0; cargo && hull && u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = cargo->UnitWithID(u + 1);
            if (unit && unit->GetPerson() && unit->GetVehicleIn() == hull)
            {
                bodies.Add(unit->GetPerson()); // cargo still aboard goes with the airframe
            }
        }
        for (int b = 0; b < bodies.Size(); b++)
        {
            DeletePerson(bodies[b]);
        }
        if (crew && crew->NUnits() == 0)
        {
            crew->RemoveFromCenter();
        }
        if (cargo && cargo->NUnits() == 0)
        {
            cargo->RemoveFromCenter();
        }
    }

    void RemoveHull(Entity* hull) override { DeleteEntity(hull); }
    void RemovePad(Entity* pad) override { DeleteEntity(pad); }
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
EngineAirOps& Ops()
{
    static EngineAirOps ops;
    return ops;
}
#pragma clang diagnostic pop

// an invisible helipad in the BUILDINGS list (VehCreate's static branch):
// FindStopPosition iterates GWorld's buildings only
Entity* CreatePad(Vector3Par lz)
{
    if (!GWorld || !GLandscape)
    {
        return nullptr;
    }
    Ref<Entity> pad = NewNonAIVehicle("HeliHEmpty", nullptr);
    if (!pad)
    {
        pad = NewNonAIVehicle("HeliH", nullptr);
    }
    if (!pad || !dyn_cast<EntityAI>(pad.GetRef()))
    {
        return nullptr;
    }
    float dx = 0, dz = 0;
    Vector3 pos = lz;
    pos[1] = GLOB_LAND->SurfaceYAboveWater(pos[0], pos[2], &dx, &dz);
    Matrix3 orient;
    Matrix4 transform;
    transform.SetPosition(pos);
    orient.SetUpAndDirection(Vector3(-dx, 1, -dz), VForward);
    transform.SetOrientation(orient);
    pad->PlaceOnSurface(transform);
    pad->SetTransform(transform);
    pad->Init(transform);
    GWorld->AddBuilding(pad);
    if (GWorld->GetMode() == GModeNetware)
    {
        GetNetworkManager().CreateVehicle(pad, VLTBuilding, "", -1);
    }
    return pad;
}

} // namespace

const AirViewer& OccupierAir::EngineViewer()
{
    Viewer().Refresh(Instance().Tuning());
    return Viewer();
}

void OccupierAir::Clear()
{
    _insertions.Clear();
    _sorties.Clear();
    _accum = 0;
}

int OccupierAir::LiveAirframes() const
{
    int n = 0;
    for (int i = 0; i < _insertions.Size(); i++)
    {
        n += _insertions[i].core.hullRemoved ? 0 : 1;
    }
    for (int i = 0; i < _sorties.Size(); i++)
    {
        n += _sorties[i].core.hullRemoved ? 0 : 1;
    }
    return n;
}

int OccupierAir::LiveGroups() const
{
    int n = 0;
    for (int i = 0; i < _insertions.Size(); i++)
    {
        const AIGroup* crew = _insertions[i].crew;
        const AIGroup* cargo = _insertions[i].cargo;
        n += crew ? 1 : 0;
        n += cargo ? 1 : 0;
    }
    for (int i = 0; i < _sorties.Size(); i++)
    {
        const AIGroup* crew = _sorties[i].crew;
        n += crew ? 1 : 0;
    }
    return n;
}

bool OccupierAir::PickLz(Vector3Par target, const Vector3* awayFrom, float minR, float maxR, Vector3& lz, RString& why)
{
    AirTuning t = _tuning;
    if (minR > 0)
    {
        t.lzMin = minR;
    }
    if (maxR > t.lzMin)
    {
        t.lzMax = maxR;
    }
    EngineLzProbe probe;
    int tried = 0;
    const bool ok = PickLzPure(target, awayFrom, t, probe, lz, why, &tried);
    if (ok)
    {
        LOG_INFO(Core, "OccupierAir: LZ [{:.0f}, {:.0f}] {:.0f} m from [{:.0f}, {:.0f}] after {} tries", lz.X(), lz.Z(),
                 Dist2D(lz, target), target.X(), target.Z(), tried);
    }
    else
    {
        LOG_WARN(Core, "OccupierAir: {} around [{:.0f}, {:.0f}]", (const char*)why, target.X(), target.Z());
    }
    return ok;
}

bool OccupierAir::SpawnInsertion(const AirInsertionPlan& plan, int& id, RString& why)
{
    id = -1;
    const ParamEntry* cfg = VehicleClass(plan.heliClass);
    if (!cfg)
    {
        why = "unknown class";
        LOG_WARN(Core, "OccupierAir: CfgVehicles has no class '{}'", (const char*)plan.heliClass);
        return false;
    }
    AICenter* center = EnsureSideCenter(plan.side);
    if (!center)
    {
        why = "unknown side";
        return false;
    }
    Vector3 heading = plan.lz - plan.birthPos;
    Transport* veh = CreateAirborne(plan.heliClass, plan.birthPos, plan.birthAlt, heading, why);
    if (!veh)
    {
        LOG_WARN(Core, "OccupierAir: could not create '{}' airborne: {}", (const char*)plan.heliClass, (const char*)why);
        return false;
    }
    AIGroup* crewGrp = CreateSideGroup(center);
    if (!crewGrp)
    {
        DeleteEntity(veh);
        why = "no group";
        return false;
    }
    const ParamEntry* crewEntry = cfg->FindEntry("crew");
    RString crewType = crewEntry ? RString((RStringB)*crewEntry) : RString();
    if (crewType.GetLength() == 0)
    {
        crewType = SideCrewFallback(plan.side);
    }
    const Vector3 near = veh->Position();
    bool seated = false;
    Person* pilot = CreateSeated(crewGrp, crewType, near, veh, GIPDriver, seated, 0.6f, RankLieutnant);
    if (!pilot || !seated || veh->Driver() != pilot)
    {
        LOG_WARN(Core, "OccupierAir: could not seat a '{}' pilot into '{}'", (const char*)crewType,
                 (const char*)plan.heliClass);
        Ops().RemoveCrew(crewGrp, nullptr, veh);
        DeleteEntity(veh);
        why = "no pilot";
        return false;
    }
    if (veh->Type()->HasGunner())
    {
        Person* gunner = CreateSeated(crewGrp, crewType, near, veh, GIPGunner, seated, 0.5f, RankSergeant);
        if (gunner && !seated)
        {
            DeletePerson(gunner);
        }
    }
    // the cargo: a SECOND group (TR UNLOAD and the dismount address a
    // foreign group's cargo), seated with the InCargo state
    AIGroup* cargoGrp = plan.cargo > 0 ? CreateSideGroup(center) : nullptr;
    int born = 0;
    if (cargoGrp)
    {
        ZoneRegistry& registry = ZoneRegistry::Instance();
        AutoArray<RString> classes;
        if (registry.IsActive() && stricmp(registry.OccupierSide(), plan.side) == 0)
        {
            registry.FactionSquad(plan.side, ReadWarLevel(), plan.cargo, classes);
        }
        const int seats = (int)cfg->ReadValue("transportSoldier", 0.0f);
        const int n = seats > 0 && plan.cargo > seats ? seats : plan.cargo;
        for (int u = 0; u < n; u++)
        {
            const RString cls = u < classes.Size() ? classes[u] : SideInfantryFallback(plan.side);
            Person* man = CreateSeated(cargoGrp, cls, near, veh, GIPCargo, seated, u == 0 ? 0.6f : 0.5f,
                                       u == 0 ? RankSergeant : RankPrivate);
            if (man && !seated)
            {
                DeletePerson(man);
            }
            else if (man && man->Brain())
            {
                man->Brain()->SetState(AIUnit::InCargo);
                born++;
            }
        }
        if (cargoGrp->NUnits() == 0)
        {
            cargoGrp->RemoveFromCenter();
            cargoGrp = nullptr;
        }
        else
        {
            cargoGrp->SetCombatModeMajor(CMSafe);
            GetNetworkManager().UpdateObject(cargoGrp);
        }
    }
    // the crew flies its errand: careless, holding fire (an insertion, not a strike)
    crewGrp->SetCombatModeMajor(CMCareless);
    SetSemaphoreAll(crewGrp, AI::SemaphoreGreen);
    GetNetworkManager().UpdateObject(crewGrp);

    Entity* pad = CreatePad(plan.lz);
    if (!pad)
    {
        LOG_WARN(Core, "OccupierAir: no helipad could be placed at [{:.0f}, {:.0f}]", plan.lz.X(), plan.lz.Z());
    }
    AirInsertion rec;
    rec.id = _nextId++;
    rec.heliClass = plan.heliClass;
    rec.side = plan.side;
    rec.hull = veh;
    rec.crew = crewGrp;
    rec.cargo = cargoGrp;
    rec.pad = pad;
    rec.cargoBorn = born;
    rec.birthPos = plan.birthPos;
    rec.target = plan.target;
    rec.core.lz = plan.lz;
    rec.core.home = plan.birthPos;
    Ops().MoveTo(crewGrp, plan.lz);
    // keep at most MaxRecords: drop the oldest finished
    while (_insertions.Size() >= MaxRecords)
    {
        int drop = -1;
        for (int i = 0; i < _insertions.Size() && drop < 0; i++)
        {
            drop = _insertions[i].core.hullRemoved ? i : -1;
        }
        if (drop < 0)
        {
            break;
        }
        _insertions.Delete(drop);
    }
    _insertions.Add(rec);
    id = rec.id;
    const Vector3 p = veh->Position();
    LOG_INFO(Core,
             "OccupierAir: insertion #{} {} born airborne at [{:.0f}, {:.0f}] {:.0f} m above ground, {} cargo, LZ "
             "[{:.0f}, {:.0f}]",
             id, (const char*)plan.heliClass, p.X(), p.Z(), p.Y() - GLOB_LAND->SurfaceYAboveWater(p.X(), p.Z()), born,
             plan.lz.X(), plan.lz.Z());
    why = "ok";
    return true;
}

bool OccupierAir::SpawnSortie(const RString& planeClass, Vector3Par birthPos, float birthAlt, Vector3Par target,
                              float sortieTime, const RString& side, int& id, RString& why)
{
    id = -1;
    const ParamEntry* cfg = VehicleClass(planeClass);
    if (!cfg)
    {
        why = "unknown class";
        LOG_WARN(Core, "OccupierAir: CfgVehicles has no class '{}'", (const char*)planeClass);
        return false;
    }
    AICenter* center = EnsureSideCenter(side);
    if (!center)
    {
        why = "unknown side";
        return false;
    }
    Transport* veh = CreateAirborne(planeClass, birthPos, birthAlt, target - birthPos, why);
    if (!veh)
    {
        LOG_WARN(Core, "OccupierAir: could not create '{}' airborne: {}", (const char*)planeClass, (const char*)why);
        return false;
    }
    AIGroup* crewGrp = CreateSideGroup(center);
    if (!crewGrp)
    {
        DeleteEntity(veh);
        why = "no group";
        return false;
    }
    const ParamEntry* crewEntry = cfg->FindEntry("crew");
    RString crewType = crewEntry ? RString((RStringB)*crewEntry) : RString();
    if (crewType.GetLength() == 0)
    {
        crewType = SideCrewFallback(side);
    }
    bool seated = false;
    Person* pilot = CreateSeated(crewGrp, crewType, veh->Position(), veh, GIPDriver, seated, 0.7f, RankCaptain);
    if (!pilot || !seated || veh->Driver() != pilot)
    {
        Ops().RemoveCrew(crewGrp, nullptr, veh);
        DeleteEntity(veh);
        why = "no pilot";
        return false;
    }
    if (veh->Type()->HasGunner())
    {
        Person* gunner = CreateSeated(crewGrp, crewType, veh->Position(), veh, GIPGunner, seated, 0.6f, RankSergeant);
        if (gunner && !seated)
        {
            DeletePerson(gunner);
        }
    }
    crewGrp->SetCombatModeMajor(CMAware);
    SetSemaphoreAll(crewGrp, AI::SemaphoreRed);
    GetNetworkManager().UpdateObject(crewGrp);
    AirSortie rec;
    rec.id = _nextId++;
    rec.planeClass = planeClass;
    rec.side = side;
    rec.hull = veh;
    rec.crew = crewGrp;
    rec.birthPos = birthPos;
    rec.core.target = target;
    rec.core.home = birthPos;
    rec.core.sortieTime = sortieTime > 0 ? sortieTime : 300.0f;
    Ops().MoveTo(crewGrp, target);
    while (_sorties.Size() >= MaxRecords)
    {
        int drop = -1;
        for (int i = 0; i < _sorties.Size() && drop < 0; i++)
        {
            drop = _sorties[i].core.hullRemoved ? i : -1;
        }
        if (drop < 0)
        {
            break;
        }
        _sorties.Delete(drop);
    }
    _sorties.Add(rec);
    id = rec.id;
    const Vector3 p = veh->Position();
    LOG_INFO(Core, "OccupierAir: sortie #{} {} born airborne at [{:.0f}, {:.0f}] {:.0f} m above ground, target "
                   "[{:.0f}, {:.0f}] for {:.0f} s",
             id, (const char*)planeClass, p.X(), p.Z(), p.Y() - GLOB_LAND->SurfaceYAboveWater(p.X(), p.Z()),
             target.X(), target.Z(), rec.core.sortieTime);
    why = "ok";
    return true;
}

bool OccupierAir::Reveal(int sortieId, Entity* target, float ageSeconds, float accuracy)
{
    EntityAI* ai = dyn_cast<EntityAI>(target);
    for (int i = 0; i < _sorties.Size(); i++)
    {
        AirSortie& s = _sorties[i];
        if (s.id != sortieId)
        {
            continue;
        }
        AIGroup* crew = s.crew;
        if (!crew || !ai || !RevealAllowed(ageSeconds, accuracy, _tuning))
        {
            LOG_INFO(Core, "OccupierAir: sortie #{} reveal refused (age {:.0f} s, accuracy {:.2f})", sortieId,
                     ageSeconds, accuracy);
            return false;
        }
        // never an exact fix (D9): the position sharpens only with accuracy
        crew->AddTarget(ai, accuracy, accuracy, 0);
        s.reveals++;
        LOG_INFO(Core, "OccupierAir: sortie #{} revealed a target (age {:.0f} s, accuracy {:.2f})", sortieId,
                 ageSeconds, accuracy);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// phase 6A: lookup and persistence
// ---------------------------------------------------------------------------

const AirInsertion* OccupierAir::FindInsertion(int id) const
{
    for (int i = 0; i < _insertions.Size(); i++)
    {
        if (_insertions[i].id == id)
        {
            return &_insertions[i];
        }
    }
    return nullptr;
}

const AirSortie* OccupierAir::FindSortie(int id) const
{
    for (int i = 0; i < _sorties.Size(); i++)
    {
        if (_sorties[i].id == id)
        {
            return &_sorties[i];
        }
    }
    return nullptr;
}

namespace
{
LSError SerializeVec(ParamArchive& ar, const char* name, Vector3& v)
{
    RString kx = RString(name) + "X";
    RString ky = RString(name) + "Y";
    RString kz = RString(name) + "Z";
    float x = v.X();
    float y = v.Y();
    float z = v.Z();
    PARAM_CHECK(ar.Serialize((const char*)kx, x, 1, 0.0f))
    PARAM_CHECK(ar.Serialize((const char*)ky, y, 1, 0.0f))
    PARAM_CHECK(ar.Serialize((const char*)kz, z, 1, 0.0f))
    v = Vector3(x, y, z);
    return LSOK;
}
} // namespace

LSError AirInsertion::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("id", id, 1, 0))
    PARAM_CHECK(ar.Serialize("heliClass", heliClass, 1, RString()))
    PARAM_CHECK(ar.Serialize("side", side, 1, RString()))
    // the airframe, its crew and cargo ride the world's vehicle serializer;
    // the refs resolve on the second pass
    PARAM_CHECK(ar.SerializeRef("hull", hull, 1))
    PARAM_CHECK(ar.SerializeRef("crew", crew, 1))
    PARAM_CHECK(ar.SerializeRef("cargo", cargo, 1))
    PARAM_CHECK(ar.SerializeRef("pad", pad, 1))
    PARAM_CHECK(ar.Serialize("cargoBorn", cargoBorn, 1, 0))
    PARAM_CHECK(SerializeVec(ar, "birth", birthPos))
    PARAM_CHECK(SerializeVec(ar, "target", target))
    PARAM_CHECK(ar.Serialize("crewGoneAt", crewGoneAt, 1, -1.0f))
    PARAM_CHECK(ar.Serialize("hullGoneAt", hullGoneAt, 1, -1.0f))
    PARAM_CHECK(ar.Serialize("removeSeq", removeSeq, 1, 0))
    AirInsertionCore& c = core;
    PARAM_CHECK(ar.Serialize("state", c.state, 1, (int)AISInbound))
    PARAM_CHECK(ar.Serialize("stateTime", c.stateTime, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("age", c.age, 1, 0.0f))
    PARAM_CHECK(SerializeVec(ar, "lz", c.lz))
    PARAM_CHECK(SerializeVec(ar, "home", c.home))
    PARAM_CHECK(SerializeVec(ar, "landedAt", c.landedAt))
    PARAM_CHECK(ar.Serialize("landed", c.landed, 1, false))
    PARAM_CHECK(ar.Serialize("landedOffset", c.landedOffset, 1, -1.0f))
    PARAM_CHECK(ar.Serialize("landings", c.landings, 1, 0))
    PARAM_CHECK(ar.Serialize("liftedOffAt", c.liftedOffAt, 1, -1.0f))
    PARAM_CHECK(ar.Serialize("unloadAt", c.unloadAt, 1, -1.0f))
    PARAM_CHECK(ar.Serialize("landOrderAt", c.landOrderAt, 1, -1.0f))
    PARAM_CHECK(ar.Serialize("landOrders", c.landOrders, 1, 0))
    PARAM_CHECK(ar.Serialize("unloadOrdered", c.unloadOrdered, 1, false))
    PARAM_CHECK(ar.Serialize("crewRemoved", c.crewRemoved, 1, false))
    PARAM_CHECK(ar.Serialize("hullRemoved", c.hullRemoved, 1, false))
    PARAM_CHECK(ar.Serialize("failReason", c.failReason, 1, 0))
    if (ar.IsLoading())
    {
        c.unseenFor = 0;
        c.touchFor = 0;
    }
    return LSOK;
}

LSError AirSortie::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("id", id, 1, 0))
    PARAM_CHECK(ar.Serialize("planeClass", planeClass, 1, RString()))
    PARAM_CHECK(ar.Serialize("side", side, 1, RString()))
    PARAM_CHECK(ar.SerializeRef("hull", hull, 1))
    PARAM_CHECK(ar.SerializeRef("crew", crew, 1))
    PARAM_CHECK(SerializeVec(ar, "birth", birthPos))
    PARAM_CHECK(ar.Serialize("crewGoneAt", crewGoneAt, 1, -1.0f))
    PARAM_CHECK(ar.Serialize("hullGoneAt", hullGoneAt, 1, -1.0f))
    PARAM_CHECK(ar.Serialize("reveals", reveals, 1, 0))
    AirSortieCore& c = core;
    PARAM_CHECK(ar.Serialize("state", c.state, 1, (int)ASSInbound))
    PARAM_CHECK(ar.Serialize("stateTime", c.stateTime, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("age", c.age, 1, 0.0f))
    PARAM_CHECK(SerializeVec(ar, "target", c.target))
    PARAM_CHECK(SerializeVec(ar, "home", c.home))
    PARAM_CHECK(ar.Serialize("sortieTime", c.sortieTime, 1, 300.0f))
    PARAM_CHECK(ar.Serialize("orbitClock", c.orbitClock, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("orbitIndex", c.orbitIndex, 1, 0))
    PARAM_CHECK(ar.Serialize("touchedGround", c.touchedGround, 1, false))
    PARAM_CHECK(ar.Serialize("crewRemoved", c.crewRemoved, 1, false))
    PARAM_CHECK(ar.Serialize("hullRemoved", c.hullRemoved, 1, false))
    PARAM_CHECK(ar.Serialize("failReason", c.failReason, 1, 0))
    if (ar.IsLoading())
    {
        c.unseenFor = 0;
    }
    return LSOK;
}

LSError OccupierAir::SerializeRecords(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("AirInsertions", _insertions, 1))
    PARAM_CHECK(ar.Serialize("AirSorties", _sorties, 1))
    PARAM_CHECK(ar.Serialize("airNextId", _nextId, 1, 1))
    if (ar.IsLoading())
    {
        _accum = 0;
        _traceClock = 0;
    }
    return LSOK;
}

void OccupierAir::AfterLoad()
{
    for (int i = 0; i < _insertions.Size(); i++)
    {
        AirInsertion& r = _insertions[i];
        if (r.core.hullRemoved || r.pad || (r.core.state != AISInbound && r.core.state != AISLanding))
        {
            continue;
        }
        r.pad = CreatePad(r.core.lz);
        LOG_INFO(Core, "OccupierAir: insertion #{} helipad re-created at [{:.0f}, {:.0f}] after a load", r.id,
                 r.core.lz.X(), r.core.lz.Z());
    }
}

int OccupierAir::Abort(int id, bool hullFirst)
{
    EngineAirOps& ops = Ops();
    auto countCrewOutside = [](AIGroup* crew) {
        int n = 0;
        for (int u = 0; crew && u < MAX_UNITS_PER_GROUP; u++)
        {
            AIUnit* unit = crew->UnitWithID(u + 1);
            Person* p = unit ? unit->GetPerson() : nullptr;
            n += p && unit->GetLifeState() == AIUnit::LSAlive && !unit->GetVehicleIn() && !p->ToDelete() ? 1 : 0;
        }
        return n;
    };
    for (int i = 0; i < _insertions.Size(); i++)
    {
        AirInsertion& r = _insertions[i];
        if (r.id != id || r.core.hullRemoved)
        {
            continue;
        }
        int ejected = 0;
        if (hullFirst)
        {
            ops.RemoveHull(r.hull); // ::DeleteVehicle ejects the crew in place (GameStateExtWorld.cpp)
            r.hullGoneAt = r.core.age;
            ejected = countCrewOutside(r.crew);
            ops.RemoveCrew(r.crew, nullptr, nullptr);
            r.crewGoneAt = r.core.age;
            r.removeSeq = 2;
        }
        else
        {
            ops.RemoveCrew(r.crew, r.cargo, r.hull);
            r.crewGoneAt = r.core.age;
            ops.RemoveHull(r.hull);
            r.hullGoneAt = r.core.age;
            r.removeSeq = 1;
        }
        ops.RemovePad(r.pad);
        r.core.crewRemoved = r.core.hullRemoved = true;
        r.core.state = AISGone;
        LOG_INFO(Core, "OccupierAir: insertion #{} aborted ({}), {} crew ejected alive", id,
                 hullFirst ? "hull first" : "crew first", ejected);
        return ejected;
    }
    for (int i = 0; i < _sorties.Size(); i++)
    {
        AirSortie& s = _sorties[i];
        if (s.id != id || s.core.hullRemoved)
        {
            continue;
        }
        int ejected = 0;
        if (hullFirst)
        {
            ops.RemoveHull(s.hull);
            s.hullGoneAt = s.core.age;
            ejected = countCrewOutside(s.crew);
            ops.RemoveCrew(s.crew, nullptr, nullptr);
            s.crewGoneAt = s.core.age;
        }
        else
        {
            ops.RemoveCrew(s.crew, nullptr, s.hull);
            s.crewGoneAt = s.core.age;
            ops.RemoveHull(s.hull);
            s.hullGoneAt = s.core.age;
        }
        s.core.crewRemoved = s.core.hullRemoved = true;
        s.core.state = ASSGone;
        LOG_INFO(Core, "OccupierAir: sortie #{} aborted ({}), {} crew ejected alive", id,
                 hullFirst ? "hull first" : "crew first", ejected);
        return ejected;
    }
    return -1;
}

void OccupierAir::Step(float dt, const AirViewer& viewer, AirOps& ops)
{
    _traceClock += dt;
    const bool trace = _tuning.trace > 0 && _traceClock >= _tuning.trace;
    if (trace)
    {
        _traceClock = 0;
    }
    for (int i = 0; i < _insertions.Size(); i++)
    {
        AirInsertion& r = _insertions[i];
        if (r.core.hullRemoved)
        {
            continue;
        }
        AirFacts f;
        ops.Facts(r.hull, r.crew, r.cargo, viewer, f);
        const int before = r.core.state;
        const int act = StepInsertionPure(r.core, f, dt, _tuning);
        if (act & AALand)
        {
            ops.Land(r.hull, r.crew, r.pad, r.core.lz);
            if (before == AISLanding)
            {
                LOG_INFO(Core, "OccupierAir: insertion #{} landing order re-issued ({}) at {:.0f} s, {:.0f} m up, {:.0f} m from the LZ",
                         r.id, r.core.landOrders, r.core.age, f.heightAgl, Dist2D(f.pos, r.core.lz));
            }
        }
        if (act & AAUnload)
        {
            ops.Unload(r.cargo, r.hull, r.target);
        }
        if (act & AAMoveHome)
        {
            ops.CancelLand(r.hull);
            ops.MoveTo(r.crew, r.core.home);
        }
        if (act & AARemoveCrew)
        {
            ops.RemoveCrew(r.crew, r.cargo, r.hull);
            r.crewGoneAt = r.core.age;
            r.removeSeq = 1;
        }
        if (act & AARemoveHull)
        {
            ops.RemoveHull(r.hull);
            r.hullGoneAt = r.core.age;
        }
        if (act & AARemovePad)
        {
            ops.RemovePad(r.pad);
        }
        if (trace)
        {
            LOG_INFO(Core, "OccupierAir: trace #{} {} t={:.1f} [{:.0f}, {:.0f}] h={:.1f} v={:.1f} lz={:.0f} crew={} cargo={}/{} unseen={}",
                     r.id, StateName(r.core.state), r.core.age, f.pos.X(), f.pos.Z(), f.heightAgl, f.speed,
                     Dist2D(f.pos, r.core.lz), f.crewAlive, f.cargoSeated, f.cargoAlive, f.unseen ? 1 : 0);
        }
        if (r.core.state != before)
        {
            LOG_INFO(Core,
                     "OccupierAir: insertion #{} {} -> {} at {:.0f} s ([{:.0f}, {:.0f}] {:.0f} m up, {:.1f} m/s, "
                     "cargo {}/{} seated, LZ offset {:.0f} m){}{}",
                     r.id, StateName(before), StateName(r.core.state), r.core.age, f.pos.X(), f.pos.Z(),
                     f.heightAgl, f.speed, f.cargoSeated, f.cargoAlive, r.core.landedOffset,
                     r.core.failReason ? ", failed: " : "", FailName(r.core.failReason));
        }
        else if (act & AAMoveHome && r.core.state == AISDeparting && r.core.landings > 1)
        {
            LOG_WARN(Core, "OccupierAir: insertion #{} landed again uncommanded ({} landings); Move home again",
                     r.id, r.core.landings);
        }
    }
    for (int i = 0; i < _sorties.Size(); i++)
    {
        AirSortie& s = _sorties[i];
        if (s.core.hullRemoved)
        {
            continue;
        }
        AirFacts f;
        ops.Facts(s.hull, s.crew, nullptr, viewer, f);
        const int before = s.core.state;
        const int act = StepSortiePure(s.core, f, dt, _tuning);
        if (act & AAWeaponsFree)
        {
            ops.WeaponsFree(s.crew);
        }
        if (act & AAOrbit)
        {
            ops.MoveTo(s.crew, OrbitPoint(s.core.target, _tuning.orbitRadius, s.core.orbitIndex));
        }
        if (act & AAMoveHome)
        {
            ops.MoveTo(s.crew, s.core.home);
        }
        if (act & AARemoveCrew)
        {
            ops.RemoveCrew(s.crew, nullptr, s.hull);
            s.crewGoneAt = s.core.age;
        }
        if (act & AARemoveHull)
        {
            ops.RemoveHull(s.hull);
            s.hullGoneAt = s.core.age;
        }
        if (trace)
        {
            LOG_INFO(Core, "OccupierAir: trace sortie #{} {} t={:.1f} [{:.0f}, {:.0f}] h={:.1f} v={:.1f} tgt={:.0f} unseen={}",
                     s.id, SortieStateName(s.core.state), s.core.age, f.pos.X(), f.pos.Z(), f.heightAgl, f.speed,
                     Dist2D(f.pos, s.core.target), f.unseen ? 1 : 0);
        }
        if (s.core.state != before)
        {
            LOG_INFO(Core, "OccupierAir: sortie #{} {} -> {} at {:.0f} s ([{:.0f}, {:.0f}] {:.0f} m up, {:.0f} m/s){}{}",
                     s.id, SortieStateName(before), SortieStateName(s.core.state), s.core.age, f.pos.X(), f.pos.Z(),
                     f.heightAgl, f.speed, s.core.failReason ? ", failed: " : "", FailName(s.core.failReason));
        }
    }
}

void OccupierAir::Simulate(float deltaT)
{
    if (_insertions.Size() == 0 && _sorties.Size() == 0)
    {
        _accum = 0;
        return;
    }
    _accum += deltaT;
    if (_accum < TickInterval)
    {
        return;
    }
    const float elapsed = _accum;
    _accum = 0;
    if (!GWorld || LiveAirframes() == 0)
    {
        return;
    }
    Viewer().Refresh(_tuning);
    Step(elapsed, Viewer(), Ops());
}

} // namespace Poseidon::Guerrilla
