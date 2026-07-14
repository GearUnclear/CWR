#include <Poseidon/Game/Guerrilla/TownFlags.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/World/World.hpp>              // GWorld / NewNonAIVehicle / Preloaded
#include <Poseidon/World/Detection/Detector.hpp> // FlagCarrier
#include <Poseidon/World/Terrain/Landscape.hpp>  // GLOB_LAND surface Y
#include <Poseidon/World/Terrain/Roads.hpp>      // GRoadNet road avoidance
#include <Poseidon/AI/AI.hpp>                    // AIUnit::FindFreePosition
#include <Poseidon/Network/Network.hpp>          // GetNetworkManager

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <string.h>

namespace Poseidon::Guerrilla
{

// Process-lifetime singleton - no global constructor (see express.hpp's
// GGameState for the convention).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
TownFlags& TownFlags::Instance()
{
    static TownFlags instance;
    return instance;
}
#pragma clang diagnostic pop

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------

void TownFlags::Clear()
{
    _states.Clear();
    _accum = 0;
    _pending.Clear();
}

void TownFlags::InitMission()
{
    Clear();
}

bool TownFlags::IsActive() const
{
    return ZoneRegistry::Instance().IsActive();
}

bool TownFlags::IsPlaced(int zoneIndex) const
{
    if (zoneIndex < 0 || zoneIndex >= _states.Size())
    {
        return false;
    }
    return _states[zoneIndex].placed;
}

Vector3 TownFlags::FlagPos(int zoneIndex) const
{
    if (zoneIndex < 0 || zoneIndex >= _states.Size() || !_states[zoneIndex].placed)
    {
        return VZero;
    }
    return _states[zoneIndex].pos;
}

void TownFlags::MarkPlacedForTest(int zoneIndex, Vector3Par pos)
{
    SyncStates();
    if (zoneIndex < 0 || zoneIndex >= _states.Size())
    {
        return;
    }
    _states[zoneIndex].placed = true;
    _states[zoneIndex].pos = pos;
}

// ---------------------------------------------------------------------------
// pure logic (unit-tested)
// ---------------------------------------------------------------------------

int TownFlags::PickFlagSpot(const AutoArray<FlagSpotSample>& samples)
{
    int best = -1;
    float bestScore = 0;
    for (int i = 0; i < samples.Size(); i++)
    {
        const FlagSpotSample& s = samples[i];
        if (s.onRoad || s.underwater)
        {
            continue;
        }
        float score = s.height + OutskirtWeight * s.distFromCenter;
        if (best < 0 || score > bestScore)
        {
            best = i;
            bestScore = score;
        }
    }
    return best;
}

RString TownFlags::ResolveFlagTexture(const char* owner, const char* factionFlag)
{
    if (factionFlag && *factionFlag)
    {
        return RString(factionFlag);
    }
    // per-side defaults; every texture verified present in the Classic 1.99
    // data (Flags.pbo).  NEUTRAL / third parties fly the generic white flag.
    if (owner)
    {
        if (stricmp(owner, "WEST") == 0)
        {
            return RString("\\flags\\usa.jpg");
        }
        if (stricmp(owner, "EAST") == 0)
        {
            return RString("\\flags\\ussr.jpg");
        }
        if (stricmp(owner, "GUER") == 0)
        {
            return RString("\\flags\\fia.jpg");
        }
    }
    return RString("\\flags\\bis_white.jpg");
}

// ---------------------------------------------------------------------------
// world-touching internals (engine path only)
// ---------------------------------------------------------------------------

// deterministic candidate set: the zone center plus three outskirt rings.
// No randomness - the same town always yields the same flag spot, so saves
// stay consistent with the serialized pole position.
static void BuildSamples(Vector3Par center, float zoneArea, AutoArray<FlagSpotSample>& out)
{
    auto addSample = [&out, &center](float x, float z)
    {
        FlagSpotSample s;
        s.x = x;
        s.z = z;
        float dx = x - center.X();
        float dz = z - center.Z();
        s.distFromCenter = sqrtf(dx * dx + dz * dz);
        float surfY = GLOB_LAND->SurfaceY(x, z);
        s.height = surfY;
        // sea level is Y=0; the margin keeps the pole off the wave wash
        s.underwater = surfY < 0.1f;
        if (GRoadNet)
        {
            s.onRoad = GRoadNet->IsOnRoad(Vector3(x, surfY, z), TownFlags::RoadClearance) != nullptr;
        }
        out.Add(s);
    };

    addSample(center.X(), center.Z());
    const float ringFractions[] = {0.35f, 0.55f, 0.75f};
    const int bearings = 16;
    for (float fraction : ringFractions)
    {
        float radius = fraction * zoneArea;
        for (int b = 0; b < bearings; b++)
        {
            float angle = (2.0f * H_PI) * (float)b / (float)bearings;
            addSample(center.X() + radius * cosf(angle), center.Z() + radius * sinf(angle));
        }
    }
}

bool TownFlags::ComputeSpot(Vector3Par center, float zoneArea, Vector3& out) const
{
    if (!GLandscape)
    {
        return false;
    }
    AutoArray<FlagSpotSample> samples;
    BuildSamples(center, zoneArea, samples);
    int best = PickFlagSpot(samples);
    if (best < 0)
    {
        return false;
    }
    out = Vector3(samples[best].x, samples[best].height, samples[best].z);
    return true;
}

// mirrors VehCreate (GameStateExtWorld.cpp) minus the script-value parsing:
// FlagCarrier is a Static-kind EntityAI, so it registers as a building and
// rides the world's building serializer into savegames
EntityAI* TownFlags::CreatePole(Vector3Par where) const
{
    Ref<Entity> veh = NewNonAIVehicle("FlagCarrier", nullptr);
    if (!veh)
    {
        return nullptr;
    }
    EntityAI* vehAI = dyn_cast<EntityAI>(veh.GetRef());
    if (!vehAI)
    {
        return nullptr;
    }

    Vector3 pos = where;
    Vector3 normal = VUp;
    if (AIUnit::FindFreePosition(pos, normal, false, vehAI))
    {
        // collision resolution may shove the pole; the original sample is
        // road-checked, so never trade it for a spot in the road
        if (GRoadNet && GRoadNet->IsOnRoad(pos, RoadClearance))
        {
            pos = where;
            normal = VUp;
        }
        else
        {
            float dx, dz;
            pos[1] = GLOB_LAND->SurfaceYAboveWater(pos[0], pos[2], &dx, &dz);
            normal = Vector3(-dx, 1, -dz);
        }
    }

    Matrix3 dir;
    Matrix4 transform;
    transform.SetPosition(pos);
    dir.SetUpAndDirection(normal, VForward);
    transform.SetOrientation(dir);

    veh->PlaceOnSurface(transform);
    veh->SetTransform(transform);
    veh->Init(transform);

    if (veh->GetNonAIType()->IsKindOf(GWorld->Preloaded(VTypeStatic)))
    {
        GWorld->AddBuilding(veh);
        if (GWorld->GetMode() == GModeNetware)
        {
            GetNetworkManager().CreateVehicle(veh, VLTBuilding, "", -1);
        }
    }
    else
    {
        GWorld->AddVehicle(veh);
        if (GWorld->GetMode() == GModeNetware)
        {
            GetNetworkManager().CreateVehicle(veh, VLTVehicle, "", -1);
        }
    }
    return vehAI;
}

void TownFlags::SyncStates()
{
    int n = ZoneRegistry::Instance().NZones();
    if (_states.Size() != n)
    {
        // config changed under us (mission (re)init); stale pole refs were
        // already dropped by Clear/InitMission
        _states.Clear();
        _states.Resize(n);
    }
}

// ---------------------------------------------------------------------------
// simulation
// ---------------------------------------------------------------------------

void TownFlags::Simulate(float deltaT)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive())
    {
        return;
    }
    _accum += deltaT;
    if (_accum < TickInterval)
    {
        return;
    }
    _accum = 0;
    if (!GWorld || !GLandscape)
    {
        return;
    }
    SyncStates();

    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || i >= _states.Size() || stricmp(z->type, "CITY") != 0)
        {
            continue;
        }
        FlagState& s = _states[i];

        if (!s.placed)
        {
            Vector3 spot;
            if (!ComputeSpot(z->pos, registry.Tuning().zoneArea, spot))
            {
                if (!s.warned)
                {
                    LOG_WARN(Core, "TownFlags: no off-road dry flag spot near zone '{}'", (const char*)z->name);
                    s.warned = true;
                }
                continue;
            }
            s.pos = spot;
            s.placed = true;
        }

        if (!s.pole.GetLink())
        {
            EntityAI* pole = CreatePole(s.pos);
            if (!pole)
            {
                // degrade non-fatal (asset-targeting policy): a package
                // without FlagCarrier keeps the map markers, loses the poles
                if (!s.warned)
                {
                    LOG_WARN(Core, "TownFlags: FlagCarrier unavailable - no flagpole for zone '{}'",
                             (const char*)z->name);
                    s.warned = true;
                }
                continue;
            }
            s.pole = pole;
            s.appliedTexture = RString();
            LOG_INFO(Core, "TownFlags: flagpole for zone '{}' at [{:.0f},{:.0f}]", (const char*)z->name, s.pos.X(),
                     s.pos.Z());
        }

        FlagCarrier* flag = dyn_cast<FlagCarrier>(s.pole.GetLink());
        if (!flag)
        {
            continue;
        }
        RString want = ResolveFlagTexture(z->owner, registry.FactionValue(z->owner, "flag"));
        if (stricmp(want, s.appliedTexture) != 0)
        {
            flag->SetFlagTexture(want);
            s.appliedTexture = want;
        }
    }
}

// ---------------------------------------------------------------------------
// serialization
// ---------------------------------------------------------------------------

LSError TownFlags::FlagSaveState::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("name", name, 1, RString()))
    PARAM_CHECK(ar.Serialize("pos", pos, 1, VZero))
    // the pole ref resolves on the second load pass (SerializeRef), after
    // the world's building serializer has recreated the FlagCarrier
    PARAM_CHECK(ar.SerializeRef("pole", pole, 1))
    return LSOK;
}

void TownFlags::ApplyPendingLoad()
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    SyncStates();
    for (int r = 0; r < _pending.Size(); r++)
    {
        const FlagSaveState& row = _pending[r];
        // rows are matched to the rebuilt zone table by NAME; rows without
        // a config zone are dropped
        int index = registry.FindZoneIndex(row.name);
        if (index < 0 || index >= _states.Size())
        {
            continue;
        }
        FlagState& s = _states[index];
        s.placed = true;
        s.pos = row.pos;
        // a dead/absent link recreates next tick at the serialized spot;
        // appliedTexture stays empty so the first tick re-applies either way
        s.pole = row.pole;
        s.appliedTexture = RString();
    }
}

LSError TownFlags::Serialize(ParamArchive& ar)
{
    if (ar.IsSaving())
    {
        _pending.Clear();
        const ZoneRegistry& registry = ZoneRegistry::Instance();
        for (int i = 0; i < _states.Size(); i++)
        {
            if (!_states[i].placed)
            {
                continue;
            }
            const ZoneRecord* z = registry.GetZone(i);
            if (!z)
            {
                continue;
            }
            FlagSaveState row;
            row.name = z->name;
            row.pos = _states[i].pos;
            row.pole = _states[i].pole;
            _pending.Add(row);
        }
    }

    PARAM_CHECK(ar.Serialize("Flags", _pending, 1))

    if (ar.IsSaving())
    {
        _pending.Clear();
    }
    else if (ar.GetPass() == ParamArchive::PassSecond)
    {
        // The ZoneRegistry rebuilt its zone table just before this block
        // (its subclass precedes ours in World::Serialize), so rows can be
        // matched now; pole refs were resolved by this pass's SerializeRef.
        ApplyPendingLoad();
        _pending.Clear();
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
