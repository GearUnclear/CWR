#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp>
#include <Poseidon/Game/Guerrilla/StashRegistry.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Evaluator/express.hpp> // GameState / GameValue (gmSelStartTown)

#include <Poseidon/World/World.hpp>             // GWorld / NewVehicle
#include <Poseidon/World/Terrain/Landscape.hpp> // GLOB_LAND surface Y
#include <Poseidon/World/Terrain/Roads.hpp>     // GRoadNet road avoidance
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/World/Entities/Vehicles/House.hpp> // Building (Paths LOD positions)
#include <Poseidon/World/Entities/Vehicles/Transport.hpp>
#include <Poseidon/World/Entities/Vehicles/Ground/Car.hpp>
#include <Poseidon/World/Entities/Vehicles/Ground/Motorcycle.hpp>
#include <Poseidon/World/Entities/Weapons/Weapons.hpp> // MuzzleType (the horn muzzle)
#include <Poseidon/AI/AI.hpp>                          // AIUnit::FindFreePosition
#include <Poseidon/AI/EntityAIType.hpp>                // MagazineSlot
#include <Poseidon/AI/VehicleAI.hpp>                   // VehicleSupply (keep-when-empty)
#include <Poseidon/Audio/IAudioSystem.hpp>             // IWave
#include <Poseidon/Audio/SoundScene.hpp>               // GSoundScene
#include <Poseidon/Network/Network.hpp>                // GetNetworkManager

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <string.h>

using namespace Poseidon;

// the AI's soldier-sized probe vehicle (World/WorldInit.cpp) - FindFreePosition
// needs a non-null vehicle to cost the operative fields (Transport.cpp's
// FindDropPos uses it the same way)
extern SRef<EntityAI> GDummyVehicle;

namespace Poseidon::Guerrilla
{

// Defined in GuerrillaBaseCommands.cpp.  Referencing it from here forces the
// command TU (whose only other content is an INIT_MODULE registration) into
// the link - same pattern as EnsureStashRegistryCommandsLinked.
void EnsureGuerrillaBaseCommandsLinked();

// Process-lifetime singleton - no global constructor (see express.hpp's
// GGameState for the convention).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
GuerrillaBase& GuerrillaBase::Instance()
{
    EnsureGuerrillaBaseCommandsLinked();
    static GuerrillaBase instance;
    return instance;
}
#pragma clang diagnostic pop

static float Dist2D(Vector3Par a, Vector3Par b)
{
    float dx = a.X() - b.X();
    float dz = a.Z() - b.Z();
    return sqrtf(dx * dx + dz * dz);
}

// ---------------------------------------------------------------------------
// lifecycle / config
// ---------------------------------------------------------------------------

void GuerrillaBase::Clear()
{
    _tuning = BaseTuning();
    _configLoaded = false;
    _established = false;
    _indoors = false;
    _zone = RString();
    _hqPos = VZero;
    _garagePos = VZero;
    _cachePos = VZero;
    _building = nullptr;
    _cache = nullptr;
    _rows.Clear();
    _moveCount = 0;
    _autoTried = false;
    _cacheWarned = false;
    _accum = 0;
    _beep = BeepState();
}

void GuerrillaBase::InitMission()
{
    Clear();
    LoadFromConfig();
}

void GuerrillaBase::LoadFromConfig()
{
    const ParamEntry* zones = ExtParsMission.FindEntry("CfgGuerrillaZones");
    if (!zones)
    {
        zones = Pars.FindEntry("CfgGuerrillaZones");
    }
    LoadFromParams(zones);
    _configLoaded = true;
}

void GuerrillaBase::LoadFromParams(const ParamEntry* zonesCfg)
{
    _tuning = BaseTuning();
    if (!zonesCfg)
    {
        return;
    }
    _tuning.hqMinPos = toInt(zonesCfg->ReadValue("hqMinPos", (float)_tuning.hqMinPos));
    _tuning.garageRadius = zonesCfg->ReadValue("garageRadius", _tuning.garageRadius);
    _tuning.garageInvulnerable =
        zonesCfg->ReadValue("garageInvulnerable", _tuning.garageInvulnerable ? 1.0f : 0.0f) != 0.0f;
    // sanity floors: a building with no AI positions is a wall, a zero ring
    // could never hold a vehicle
    if (_tuning.hqMinPos < 1)
    {
        _tuning.hqMinPos = 1;
    }
    if (_tuning.garageRadius < 10.0f)
    {
        _tuning.garageRadius = 10.0f;
    }
}

// ---------------------------------------------------------------------------
// queries
// ---------------------------------------------------------------------------

bool GuerrillaBase::IsActive() const
{
    return ZoneRegistry::Instance().IsActive();
}

EntityAI* GuerrillaBase::Building() const
{
    return _established ? _building.GetLink() : nullptr;
}

EntityAI* GuerrillaBase::Cache() const
{
    return _established ? _cache.GetLink() : nullptr;
}

int GuerrillaBase::ZoneAt(Vector3Par pos) const
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    float area = registry.Tuning().zoneArea;
    int best = -1;
    float bestDist = 0;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z)
        {
            continue;
        }
        float d = Dist2D(pos, z->pos);
        if (d <= area && (best < 0 || d < bestDist))
        {
            best = i;
            bestDist = d;
        }
    }
    return best;
}

bool GuerrillaBase::InGarageRange(Vector3Par pos) const
{
    return _established && InRange2D(pos, _garagePos, _tuning.garageRadius);
}

EntityAI* GuerrillaBase::GarageVehicle(int i) const
{
    if (i < 0 || i >= _rows.Size())
    {
        return nullptr;
    }
    return _rows[i].veh.GetLink();
}

bool GuerrillaBase::GarageHas(const EntityAI* veh) const
{
    if (!veh)
    {
        return false;
    }
    for (int i = 0; i < _rows.Size(); i++)
    {
        if (_rows[i].veh.GetLink() == veh)
        {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// pure logic (unit-tested)
// ---------------------------------------------------------------------------

int GuerrillaBase::PickHqBuilding(const AutoArray<HqCandidate>& candidates)
{
    int best = -1;
    for (int i = 0; i < candidates.Size(); i++)
    {
        const HqCandidate& c = candidates[i];
        if (best < 0)
        {
            best = i;
            continue;
        }
        const HqCandidate& b = candidates[best];
        if (c.nPos > b.nPos || (c.nPos == b.nPos && c.dist < b.dist))
        {
            best = i;
        }
    }
    return best;
}

int GuerrillaBase::PickSpot(const AutoArray<HqSpotSample>& samples)
{
    for (int i = 0; i < samples.Size(); i++)
    {
        const HqSpotSample& s = samples[i];
        if (s.onRoad || s.underwater || !s.free)
        {
            continue;
        }
        return i;
    }
    return -1;
}

bool GuerrillaBase::InRange2D(Vector3Par a, Vector3Par b, float radius)
{
    return Dist2D(a, b) <= radius;
}

// ---------------------------------------------------------------------------
// test aids
// ---------------------------------------------------------------------------

void GuerrillaBase::MarkEstablishedForTest(const char* zone, Vector3Par hqPos, Vector3Par garagePos,
                                           Vector3Par cachePos, bool indoors)
{
    _established = true;
    _indoors = indoors;
    _zone = zone ? RString(zone) : RString();
    _hqPos = hqPos;
    _garagePos = garagePos;
    _cachePos = cachePos;
}

void GuerrillaBase::AddGarageRowForTest()
{
    GarageRow row;
    _rows.Add(row);
}

// ---------------------------------------------------------------------------
// world-touching internals (engine path only)
// ---------------------------------------------------------------------------

// the best enterable building inside the zone area: Paths LOD present, at
// least hqMinPos AI positions, standing; most positions wins, nearest the
// zone centre breaks ties (PickHqBuilding).  The interior point is the
// building's first AI position (House.hpp: GetPos -> path point index,
// IPaths::GetPosition -> animated world point).
bool GuerrillaBase::PickBuilding(int zoneIndex, EntityAI*& outBuilding, Vector3& outInterior) const
{
    outBuilding = nullptr;
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    const ZoneRecord* z = registry.GetZone(zoneIndex);
    if (!z || !GWorld)
    {
        return false;
    }
    float area = registry.Tuning().zoneArea;
    AutoArray<HqCandidate> candidates;
    for (int i = 0; i < GWorld->NBuildings(); i++)
    {
        Entity* e = GWorld->GetBuilding(i);
        Poseidon::Building* b = dyn_cast<Poseidon::Building>(e);
        if (!b)
        {
            continue;
        }
        float d = Dist2D(b->Position(), z->pos);
        if (d > area)
        {
            continue;
        }
        LODShapeWithShadow* shape = b->GetShape();
        if (!shape || shape->FindPaths() < 0)
        {
            continue;
        }
        if (b->NPos() < _tuning.hqMinPos || b->IsDammageDestroyed())
        {
            continue;
        }
        HqCandidate c;
        c.index = i;
        c.nPos = b->NPos();
        c.dist = d;
        candidates.Add(c);
    }
    int best = PickHqBuilding(candidates);
    if (best < 0)
    {
        return false;
    }
    Poseidon::Building* b = dyn_cast<Poseidon::Building>(GWorld->GetBuilding(candidates[best].index));
    if (!b)
    {
        return false;
    }
    // IPaths-qualified: Object carries its own GetPos/GetPosition overloads
    outInterior = b->IPaths::GetPosition(b->IPaths::GetPos(0));
    outBuilding = b;
    return true;
}

// ring samples around an anchor (16 bearings per ring, rings in the given
// order) -> PickSpot (off-road, dry) -> AI free-position nudge (vehicle
// sized, so the garage spot can actually hold a hull); the nudged point is
// re-checked against the road net and water and the raw sample kept when
// the nudge lands somewhere worse.  No randomness: the same anchor yields
// the same spot, so a reload reproduces the placement.
bool GuerrillaBase::ComputeOutdoorSpot(Vector3Par anchor, const float* ringRadii, int nRings, Vector3& out) const
{
    if (!GLandscape)
    {
        return false;
    }
    AutoArray<HqSpotSample> samples;
    const int bearings = 16;
    for (int r = 0; r < nRings; r++)
    {
        for (int b = 0; b < bearings; b++)
        {
            float angle = (2.0f * H_PI) * (float)b / (float)bearings;
            HqSpotSample s;
            s.x = anchor.X() + ringRadii[r] * cosf(angle);
            s.z = anchor.Z() + ringRadii[r] * sinf(angle);
            s.height = GLOB_LAND->SurfaceY(s.x, s.z);
            s.distFromAnchor = ringRadii[r];
            s.underwater = s.height < 0.1f; // sea level is Y=0; keep off the wash
            if (GRoadNet)
            {
                s.onRoad = GRoadNet->IsOnRoad(Vector3(s.x, s.height, s.z), RoadClearance) != nullptr;
            }
            samples.Add(s);
        }
    }
    int best = PickSpot(samples);
    if (best < 0)
    {
        return false;
    }
    Vector3 pos(samples[best].x, samples[best].height, samples[best].z);
    if (GDummyVehicle)
    {
        Vector3 nudged = pos;
        Vector3 normal = VUp;
        if (AIUnit::FindFreePosition(nudged, normal, false, GDummyVehicle))
        {
            float y = GLOB_LAND->SurfaceY(nudged.X(), nudged.Z());
            bool road = GRoadNet && GRoadNet->IsOnRoad(Vector3(nudged.X(), y, nudged.Z()), RoadClearance) != nullptr;
            if (!road && y >= 0.1f)
            {
                pos = Vector3(nudged.X(), y, nudged.Z());
            }
        }
    }
    out = pos;
    return true;
}

// a WeaponHolder flagged keep-when-empty and tracked by the StashRegistry -
// mirrors DropWeapon (Transport.cpp) minus the drop grid.  Indoors the
// holder is set straight onto the interior point: PlaceOnSurface ignores
// building floors (Simul.cpp, Static branch) and would drop it to the
// terrain under the house.
EntityAI* GuerrillaBase::CreateCache(Vector3Par where, bool indoors) const
{
    if (!GWorld)
    {
        return nullptr;
    }
    Ref<EntityAI> veh = NewVehicle("WeaponHolder");
    if (!veh)
    {
        return nullptr;
    }
    VehicleSupply* holder = dyn_cast<VehicleSupply>(veh.GetRef());
    if (!holder)
    {
        return nullptr;
    }

    Vector3 pos = where;
    Vector3 normal = VUp;
    if (!indoors && GLandscape)
    {
        if (AIUnit::FindFreePosition(pos, normal, false, veh))
        {
            // the collision resolution may shove the holder; never trade the
            // road-checked sample for a spot in the road
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
    }

    Matrix3 dir;
    Matrix4 transform;
    transform.SetPosition(pos);
    dir.SetUpAndDirection(normal, VForward);
    transform.SetOrientation(dir);
    if (!indoors)
    {
        veh->PlaceOnSurface(transform);
    }
    veh->SetTransform(transform);
    veh->Init(transform);

    GWorld->AddBuilding(veh);
    if (GWorld->GetMode() == GModeNetware)
    {
        GetNetworkManager().CreateVehicle(veh, VLTBuilding, "", -1);
    }

    holder->SetKeepCargoWhenEmpty(true);
    StashRegistry::Instance().Register(holder);
    return holder;
}

// move an existing holder (contents travel with it) - the setPos idiom
// (ObjSetPos, GameStateExtGrp.cpp) minus the soldier-in-vehicle branches
void GuerrillaBase::MoveCache(EntityAI* cache, Vector3Par where, bool indoors) const
{
    if (!cache)
    {
        return;
    }
    Matrix4 trans = cache->Transform();
    trans.SetPosition(where);
    if (!indoors)
    {
        cache->PlaceOnSurface(trans);
    }
    cache->MoveNetAware(trans);
    cache->OnPositionChanged();
}

// ---------------------------------------------------------------------------
// election
// ---------------------------------------------------------------------------

bool GuerrillaBase::Establish(Vector3Par pos)
{
    if (!GWorld || !GLandscape || !IsActive())
    {
        return false;
    }
    int zoneIndex = ZoneAt(pos);
    if (zoneIndex < 0)
    {
        return false;
    }
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    const ZoneRecord* z = registry.GetZone(zoneIndex);
    if (!z)
    {
        return false;
    }
    float area = registry.Tuning().zoneArea;

    EntityAI* building = nullptr;
    Vector3 interior = VZero;
    Vector3 hqPos = VZero;
    Vector3 garagePos = VZero;
    Vector3 cachePos = VZero;
    bool indoors = PickBuilding(zoneIndex, building, interior);
    if (indoors)
    {
        // the garage ring sits beside the house; no ring spot at all means
        // a house hemmed in by others - take the edge-of-town fallback
        const float rings[] = {20.0f, 35.0f, 50.0f};
        if (ComputeOutdoorSpot(building->Position(), rings, 3, garagePos))
        {
            hqPos = building->Position();
            cachePos = interior;
        }
        else
        {
            indoors = false;
            building = nullptr;
        }
    }
    if (!indoors)
    {
        // edge of town: the outer rings of the zone area, cache and garage
        // together (the cache a few metres aside so a hull never parks on it)
        const float rings[] = {0.6f * area, 0.75f * area, 0.45f * area, 0.9f * area};
        if (!ComputeOutdoorSpot(z->pos, rings, 4, garagePos))
        {
            LOG_WARN(Core, "GuerrillaBase: no dry off-road spot for a headquarters near zone '{}'",
                     (const char*)z->name);
            return false;
        }
        hqPos = garagePos;
        cachePos = garagePos + Vector3(0, 0, CacheAside);
        cachePos[1] = GLOB_LAND->SurfaceYAboveWater(cachePos.X(), cachePos.Z());
    }

    bool moving = _established;
    EntityAI* cache = _cache.GetLink();
    if (cache)
    {
        MoveCache(cache, cachePos, indoors);
    }
    else
    {
        cache = CreateCache(cachePos, indoors);
        if (!cache)
        {
            // degrade non-fatal: the HQ stands, the tick self-heals the holder
            LOG_WARN(Core, "GuerrillaBase: WeaponHolder unavailable - cache deferred to the next tick");
        }
    }

    _established = true;
    _indoors = indoors;
    _zone = z->name;
    _hqPos = hqPos;
    _garagePos = garagePos;
    _cachePos = cachePos;
    _building = building;
    _cache = cache;
    _cacheWarned = false;
    if (moving)
    {
        _moveCount++;
        // vehicles left outside the relocated ring are no longer garaged
        for (int i = _rows.Size() - 1; i >= 0; i--)
        {
            EntityAI* v = _rows[i].veh.GetLink();
            if (!v || !InRange2D(v->Position(), _garagePos, _tuning.garageRadius * ReleaseFactor))
            {
                ReleaseRow(i);
            }
        }
    }
    LOG_INFO(Core, "GuerrillaBase: headquarters {} in zone '{}' ({}) at [{:.0f},{:.0f}], garage at [{:.0f},{:.0f}]",
             moving ? "moved" : "established", (const char*)_zone, indoors ? "building" : "edge of town", _hqPos.X(),
             _hqPos.Z(), _garagePos.X(), _garagePos.Z());
    return true;
}

// ---------------------------------------------------------------------------
// garage
// ---------------------------------------------------------------------------

void GuerrillaBase::AssertRow(GarageRow& row)
{
    EntityAI* v = row.veh.GetLink();
    Transport* t = dyn_cast<Transport>(v);
    if (!t)
    {
        return;
    }
    t->SetLock(LSLocked);
    if (_tuning.garageInvulnerable)
    {
        v->SetAllowDammage(false);
    }
}

void GuerrillaBase::ReleaseRow(int i)
{
    if (i < 0 || i >= _rows.Size())
    {
        return;
    }
    EntityAI* v = _rows[i].veh.GetLink();
    if (Transport* t = dyn_cast<Transport>(v))
    {
        t->SetLock(LSUnlocked);
        v->SetAllowDammage(true);
    }
    _rows.Delete(i);
}

bool GuerrillaBase::GarageLock(EntityAI* veh, bool lock)
{
    if (!veh)
    {
        return false;
    }
    Transport* t = dyn_cast<Transport>(veh);
    if (!t)
    {
        return false;
    }
    if (!lock)
    {
        for (int i = 0; i < _rows.Size(); i++)
        {
            if (_rows[i].veh.GetLink() == veh)
            {
                ReleaseRow(i);
                return true;
            }
        }
        return false;
    }
    if (!_established || veh->IsDammageDestroyed() || !InGarageRange(veh->Position()))
    {
        return false;
    }
    // never lock the player in: the real player must be on foot
    if (GWorld)
    {
        Person* player = GWorld->GetRealPlayer();
        AIUnit* unit = player ? player->Brain() : nullptr;
        if (unit && unit->GetVehicleIn() == t)
        {
            return false;
        }
    }
    if (!GarageHas(veh))
    {
        GarageRow row;
        row.veh = veh;
        _rows.Add(row);
    }
    for (int i = 0; i < _rows.Size(); i++)
    {
        if (_rows[i].veh.GetLink() == veh)
        {
            AssertRow(_rows[i]);
        }
    }
    Beep(veh);
    LOG_INFO(Core, "GuerrillaBase: vehicle locked in the garage ({} garaged)", _rows.Size());
    return true;
}

// ---------------------------------------------------------------------------
// beep-beep (the horn muzzle, two short bursts; no data files of our own)
// ---------------------------------------------------------------------------

// the horn muzzle of a car or motorcycle: a magazine-less muzzle with a
// sound (CfgWeapons CarHorn / TruckHorn: ammo="", magazines[]={}, the
// drySound[] the Car::FireWeapon horn path plays); empty when none
static RString HornSound(EntityAI* veh, float& vol, float& freq)
{
    vol = 1.0f;
    freq = 1.0f;
    if (!veh)
    {
        return RString();
    }
    if (!dyn_cast<Car>(veh) && !dyn_cast<Motorcycle>(veh))
    {
        return RString();
    }
    for (int s = 0; s < veh->NMagazineSlots(); s++)
    {
        const MuzzleType* muzzle = veh->GetMagazineSlot(s)._muzzle;
        if (!muzzle || muzzle->_magazines.Size() > 0 || muzzle->_sound.name.GetLength() == 0)
        {
            continue;
        }
        vol = muzzle->_sound.vol;
        freq = muzzle->_sound.freq;
        return muzzle->_sound.name;
    }
    return RString();
}

static IWave* PlayHornBurst(EntityAI* veh)
{
    if (!veh || !GSoundScene)
    {
        return nullptr;
    }
    float vol, freq;
    RString name = HornSound(veh, vol, freq);
    if (name.GetLength() == 0)
    {
        return nullptr;
    }
    IWave* wave = GSoundScene->OpenAndPlayOnce(name, veh->Position(), VZero, vol, freq);
    if (wave)
    {
        GSoundScene->SimulateSpeedOfSound(wave);
        GSoundScene->AddSound(wave);
    }
    return wave;
}

void GuerrillaBase::Beep(EntityAI* veh)
{
    // a cue already in flight is cut short - the new lock owns the horn
    if (_beep.wave)
    {
        _beep.wave->Stop();
        _beep.wave = nullptr;
    }
    _beep = BeepState();
    IWave* wave = PlayHornBurst(veh);
    if (!wave)
    {
        return; // horn-less hull or no sound scene (headless): silent lock
    }
    _beep.veh = veh;
    _beep.wave = wave;
    _beep.stage = 0;
    _beep.t = 0;
}

void GuerrillaBase::SimulateBeep(float deltaT)
{
    if (_beep.stage < 0)
    {
        return;
    }
    _beep.t += deltaT;
    switch (_beep.stage)
    {
        case 0:
            if (_beep.t >= BeepOn)
            {
                if (_beep.wave)
                {
                    _beep.wave->Stop();
                    _beep.wave = nullptr;
                }
                _beep.stage = 1;
            }
            break;
        case 1:
            if (_beep.t >= BeepGap)
            {
                IWave* wave = PlayHornBurst(_beep.veh.GetLink());
                if (!wave)
                {
                    _beep = BeepState();
                    break;
                }
                _beep.wave = wave;
                _beep.stage = 2;
            }
            break;
        default:
            if (_beep.t >= BeepGap + BeepOn)
            {
                if (_beep.wave)
                {
                    _beep.wave->Stop();
                }
                _beep = BeepState();
            }
            break;
    }
}

// ---------------------------------------------------------------------------
// start-town auto-election (issue #16 M4, engine half)
// ---------------------------------------------------------------------------

// the new-game START TOWN pick, published by OptionsUIApp as gmSelStartTown
// and mirrored into the campaign variable bank; read like the faction
// selections (ZoneRegistry's ReadSideSelection) - nil outside the menu flow
static RString ReadStartTownSelection()
{
    if (!GWorld)
    {
        return RString();
    }
    GameState* gstate = GWorld->GetGameState();
    if (!gstate)
    {
        return RString();
    }
    GameValue value = gstate->VarGet("gmselstarttown");
    if (value.GetType() != GameString)
    {
        return RString();
    }
    return (RString)value;
}

void GuerrillaBase::RelocatePlayer() const
{
    if (!GWorld || !GLandscape)
    {
        return;
    }
    Person* player = GWorld->GetRealPlayer();
    if (!player)
    {
        return;
    }
    AIUnit* unit = player->Brain();
    if (unit && unit->GetVehicleIn())
    {
        return; // an authored vehicle start is left alone
    }
    Vector3 pos = _garagePos + Vector3(PlayerAside, 0, 0);
    Vector3 normal = VUp;
    if (AIUnit::FindFreePosition(pos, normal, true, player))
    {
        pos[1] = GLOB_LAND->RoadSurfaceYAboveWater(pos[0], pos[2]);
    }
    Matrix4 trans = player->Transform();
    trans.SetPosition(pos);
    player->PlaceOnSurface(trans);
    player->MoveNetAware(trans);
    player->OnPositionChanged();
}

void GuerrillaBase::TryAutoElection()
{
    if (_autoTried || _established || !GWorld)
    {
        return;
    }
    _autoTried = true; // one shot per campaign, whatever the outcome
    RString town = ReadStartTownSelection();
    if (town.GetLength() == 0)
    {
        return; // no pick (direct launch / "(camp)"): the authored start stands
    }
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    int zoneIndex = registry.FindZoneIndex(town);
    if (zoneIndex < 0)
    {
        LOG_WARN(Core, "GuerrillaBase: start town '{}' is not a zone of this island - authored start kept",
                 (const char*)town);
        return;
    }
    const ZoneRecord* z = registry.GetZone(zoneIndex);
    if (!z || !Establish(z->pos))
    {
        LOG_WARN(Core, "GuerrillaBase: could not establish the start-town headquarters in '{}'", (const char*)town);
        return;
    }
    RelocatePlayer();
    LOG_INFO(Core, "GuerrillaBase: campaign opens at the start town '{}'", (const char*)town);
}

// ---------------------------------------------------------------------------
// simulation
// ---------------------------------------------------------------------------

void GuerrillaBase::Simulate(float deltaT)
{
    if (!IsActive())
    {
        return;
    }
    if (!_configLoaded)
    {
        // a save written without our block (pre-feature, or never
        // established under the old gate) cleared us without a reload
        LoadFromConfig();
    }
    SimulateBeep(deltaT);
    if (!_autoTried)
    {
        TryAutoElection();
    }
    _accum += deltaT;
    if (_accum < TickInterval)
    {
        return;
    }
    _accum = 0;
    if (!GWorld || !_established)
    {
        return;
    }

    // garage rows: prune the dead/deleted, release the ones that left the
    // ring, re-assert lock + invulnerability on the rest (neither flag is
    // serialized for us - the tick is the source of truth)
    for (int i = _rows.Size() - 1; i >= 0; i--)
    {
        EntityAI* v = _rows[i].veh.GetLink();
        if (!v || v->ToDelete() || v->IsDammageDestroyed())
        {
            LOG_INFO(Core, "GuerrillaBase: garaged vehicle gone - row pruned ({} left)", _rows.Size() - 1);
            _rows.Delete(i);
            continue;
        }
        if (!InRange2D(v->Position(), _garagePos, _tuning.garageRadius * ReleaseFactor))
        {
            LOG_INFO(Core, "GuerrillaBase: garaged vehicle left the ring - released");
            ReleaseRow(i);
            continue;
        }
        AssertRow(_rows[i]);
    }

    // self-heal: a cache that did not survive (or was never created) is
    // recreated at the serialized spot; its contents are whatever the
    // holder carries now (the holder rides the world's building serializer)
    if (!_cache.GetLink())
    {
        EntityAI* cache = CreateCache(_cachePos, _indoors);
        if (cache)
        {
            _cache = cache;
            LOG_INFO(Core, "GuerrillaBase: headquarters cache recreated at [{:.0f},{:.0f}]", _cachePos.X(),
                     _cachePos.Z());
        }
        else if (!_cacheWarned)
        {
            LOG_WARN(Core, "GuerrillaBase: WeaponHolder unavailable - the headquarters has no cache");
            _cacheWarned = true;
        }
    }
}

// ---------------------------------------------------------------------------
// serialization
// ---------------------------------------------------------------------------

LSError GuerrillaBase::GarageRow::Serialize(ParamArchive& ar)
{
    // the vehicle ref resolves on the second load pass (SerializeRef), after
    // the world's vehicle serializer has recreated the hull
    PARAM_CHECK(ar.SerializeRef("veh", veh, 1))
    return LSOK;
}

LSError GuerrillaBase::Serialize(ParamArchive& ar)
{
    if (ar.IsLoading() && ar.GetPass() == ParamArchive::PassFirst)
    {
        // a load never reruns InitMission; the mission config was reparsed
        // during this pass, so the tuning is rebuilt here
        LoadFromConfig();
    }
    PARAM_CHECK(ar.Serialize("established", _established, 1, false))
    PARAM_CHECK(ar.Serialize("indoors", _indoors, 1, false))
    PARAM_CHECK(ar.Serialize("zone", _zone, 1, RString()))
    PARAM_CHECK(ar.Serialize("hqPos", _hqPos, 1, VZero))
    PARAM_CHECK(ar.Serialize("garagePos", _garagePos, 1, VZero))
    PARAM_CHECK(ar.Serialize("cachePos", _cachePos, 1, VZero))
    PARAM_CHECK(ar.Serialize("moveCount", _moveCount, 1, 0))
    PARAM_CHECK(ar.Serialize("autoTried", _autoTried, 1, false))
    // the building and the holder ride the world's building serializer;
    // the refs resolve on the second load pass
    PARAM_CHECK(ar.SerializeRef("building", _building, 1))
    PARAM_CHECK(ar.SerializeRef("cache", _cache, 1))
    PARAM_CHECK(ar.Serialize("Garage", _rows, 1))

    if (ar.IsLoading() && ar.GetPass() == ParamArchive::PassSecond)
    {
        // rows whose hull did not survive are dropped; the survivors get
        // their lock + invulnerability back right away (not left to the
        // first tick, so a loaded hull is never briefly vulnerable)
        for (int i = _rows.Size() - 1; i >= 0; i--)
        {
            if (!_rows[i].veh.GetLink())
            {
                _rows.Delete(i);
                continue;
            }
            AssertRow(_rows[i]);
        }
        _cacheWarned = false;
        _accum = 0;
        _beep = BeepState();
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
