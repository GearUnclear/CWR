#include <Poseidon/Game/Guerrilla/BaseProps.hpp>
#include <Poseidon/Game/Guerrilla/BaseGenerate.hpp>
#include <Poseidon/Game/Guerrilla/OccupierAir.hpp> // gmBasePropsLz (the LZ probe through the exclusion)
#include <Poseidon/Game/Guerrilla/TownFlags.hpp>   // ResolveFlagTexture
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // Pars
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/World/World.hpp>              // GWorld / NewNonAIVehicle / Preloaded
#include <Poseidon/World/Detection/Detector.hpp> // FlagCarrier
#include <Poseidon/World/Terrain/Landscape.hpp>  // GLOB_LAND, the object grid, the oper cache
#include <Poseidon/World/Terrain/Roads.hpp>      // GRoadNet
#include <Poseidon/Game/OperMap.hpp>             // IOperCache::RemoveField
#include <Poseidon/Network/Network.hpp>          // GetNetworkManager

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <chrono>
#include <math.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

namespace Poseidon::Guerrilla
{

namespace
{
constexpr float kDeg = 3.14159265358979f / 180.0f;

std::string Fmt1(float v)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%.0f", (double)v);
    return buf;
}

void Note(std::vector<std::string>* notes, const std::string& s)
{
    if (notes)
    {
        notes->push_back(s);
    }
}
} // namespace

// Process-lifetime singleton (the TownFlags convention).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
BaseProps& BaseProps::Instance()
{
    static BaseProps instance;
    return instance;
}
#pragma clang diagnostic pop

float BasePropBox::Radius() const
{
    const float ax = fabsf(minX) > fabsf(maxX) ? fabsf(minX) : fabsf(maxX);
    const float az = fabsf(minZ) > fabsf(maxZ) ? fabsf(minZ) : fabsf(maxZ);
    return sqrtf(ax * ax + az * az);
}

LSError BasePropState::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("base", base, 1, RString()))
    PARAM_CHECK(ar.Serialize("row", row, 1, -1))
    PARAM_CHECK(ar.Serialize("cls", cls, 1, RString()))
    PARAM_CHECK(ar.Serialize("x", x, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("z", z, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("dir", dir, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("minX", box.minX, 1, -1.0f))
    PARAM_CHECK(ar.Serialize("maxX", box.maxX, 1, 1.0f))
    PARAM_CHECK(ar.Serialize("minZ", box.minZ, 1, -1.0f))
    PARAM_CHECK(ar.Serialize("maxZ", box.maxZ, 1, 1.0f))
    // resolved on the second load pass, after the world's building
    // serializer has recreated the prop (the TownFlags pole idiom)
    PARAM_CHECK(ar.SerializeRef("prop", prop, 1))
    return LSOK;
}

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------

void BaseProps::Clear()
{
    _rows.Clear();
    _planned.Clear();
    _accum = TickInterval; // the first Simulate call runs a pass
    _pendingRows.Clear();
    _pendingPlanned.Clear();
    _layouts.clear();
    _layoutsLoaded = false;
}

// ---------------------------------------------------------------------------
// layouts
// ---------------------------------------------------------------------------

static float ItemFloat(const IParamArrayValue& v, float def)
{
    if (v.IsFloatValue() || v.IsIntValue())
    {
        return v.GetFloat();
    }
    if (v.IsTextValue())
    {
        return (float)atof((const char*)v.GetValue());
    }
    return def;
}

bool BaseProps::ParseLayout(const ParamEntry& cls, BaseLayout& out, std::vector<std::string>* notes)
{
    out = BaseLayout();
    out.name = RString((const char*)cls.GetName());
    const ParamEntry* type = cls.FindEntry("type");
    if (!type)
    {
        Note(notes, std::string("layout ") + (const char*)out.name + ": no type");
        return false;
    }
    out.type = RString((const char*)type->GetValue());
    if (const ParamEntry* e = cls.FindEntry("yard"))
    {
        out.yard = (float)*e;
    }
    if (const ParamEntry* e = cls.FindEntry("roadMargin"))
    {
        out.roadMargin = (float)*e;
    }
    if (const ParamEntry* e = cls.FindEntry("jitterDeg"))
    {
        out.jitterDeg = (float)*e;
    }
    if (const ParamEntry* e = cls.FindEntry("minProps"))
    {
        out.minProps = (int)*e;
    }
    const ParamEntry* items = cls.FindEntry("items");
    if (!items || !items->IsArray())
    {
        Note(notes, std::string("layout ") + (const char*)out.name + ": no items[]");
        return false;
    }
    for (int i = 0; i < items->GetSize(); i++)
    {
        const IParamArrayValue& item = (*items)[i];
        if (!item.IsArrayValue() || item.GetItemCount() < 4 || !item[0].IsTextValue())
        {
            Note(notes, std::string("layout ") + (const char*)out.name + ": item " + std::to_string(i) +
                            " is not {class, dx, dz, dir}");
            continue;
        }
        BasePropRow row;
        row.cls = RString((const char*)item[0].GetValue());
        row.dx = ItemFloat(item[1], 0);
        row.dz = ItemFloat(item[2], 0);
        row.dir = ItemFloat(item[3], 0);
        row.jitter = item.GetItemCount() >= 5 && ItemFloat(item[4], 0) != 0;
        out.rows.Add(row);
    }
    return out.rows.Size() > 0;
}

void BaseProps::LoadLayouts(const ParamEntry* cfg)
{
    _layouts.clear();
    _layoutsLoaded = true;
    if (!cfg || !cfg->IsClass())
    {
        LOG_INFO(Core, "BaseProps: no CfgGuerrillaBaseLayouts in the loaded config; generated bases get no props "
                       "(install guerrilla-hq.hpp)");
        return;
    }
    std::vector<std::string> notes;
    for (int i = 0; i < cfg->GetEntryCount(); i++)
    {
        const ParamEntry& e = cfg->GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        BaseLayout layout;
        if (ParseLayout(e, layout, &notes))
        {
            _layouts.push_back(layout);
        }
    }
    for (const std::string& n : notes)
    {
        LOG_WARN(Core, "BaseProps: {}", n);
    }
}

const BaseLayout* BaseProps::FindLayout(const char* zoneType) const
{
    for (const BaseLayout& l : _layouts)
    {
        if (zoneType && stricmp(l.type, zoneType) == 0)
        {
            return &l;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// pure helpers
// ---------------------------------------------------------------------------

void BaseProps::LocalToWorld(float cx, float cz, float headingDeg, float dx, float dz, float& x, float& z)
{
    const float h = headingDeg * kDeg;
    const float s = sinf(h);
    const float c = cosf(h);
    // forward (+dz) = (sin h, cos h); right (+dx) = (cos h, -sin h)
    x = cx + dx * c + dz * s;
    z = cz - dx * s + dz * c;
}

float BaseProps::JitterDeg(int layoutSeed, int row, float jitterDeg)
{
    unsigned long long k = (unsigned long long)(unsigned int)layoutSeed * 0x9E3779B97F4A7C15ull +
                           (unsigned long long)(unsigned int)(row + 1) * 0xBF58476D1CE4E5B9ull;
    k ^= k >> 31;
    k *= 0x94D049BB133111EBull;
    k ^= k >> 29;
    const float u = (float)(k & 0xFFFFFFull) / (float)0xFFFFFFull; // 0..1
    return (u * 2.0f - 1.0f) * jitterDeg;
}

RString BaseProps::ResolveClass(const char* alternates, const BasePropWorld& world)
{
    if (!alternates)
    {
        return RString();
    }
    std::string all(alternates);
    size_t start = 0;
    while (start <= all.size())
    {
        size_t bar = all.find('|', start);
        std::string one = all.substr(start, bar == std::string::npos ? std::string::npos : bar - start);
        // trim
        while (!one.empty() && (one.front() == ' ' || one.front() == '\t'))
        {
            one.erase(one.begin());
        }
        while (!one.empty() && (one.back() == ' ' || one.back() == '\t'))
        {
            one.pop_back();
        }
        if (!one.empty() && world.Spawnable(one.c_str()))
        {
            return RString(one.c_str());
        }
        if (bar == std::string::npos)
        {
            break;
        }
        start = bar + 1;
    }
    return RString();
}

bool BaseProps::InsideBox(const BasePropState& s, float x, float z, float margin)
{
    const float d = s.dir * kDeg;
    const float px = x - s.x;
    const float pz = z - s.z;
    // model +x = (cos d, -sin d), model +z = (sin d, cos d)
    const float lx = px * cosf(d) - pz * sinf(d);
    const float lz = px * sinf(d) + pz * cosf(d);
    return lx >= s.box.minX - margin && lx <= s.box.maxX + margin && lz >= s.box.minZ - margin &&
           lz <= s.box.maxZ + margin;
}

bool BaseProps::NearAny(const AutoArray<BasePropState>& pts, float x, float z, float r)
{
    const float r2 = r * r;
    for (int i = 0; i < pts.Size(); i++)
    {
        const float dx = pts[i].x - x;
        const float dz = pts[i].z - z;
        if (dx * dx + dz * dz < r2)
        {
            return true;
        }
    }
    return false;
}

int BaseProps::PlanLayout(const BaseLayout& layout, const GeneratedBaseRow& base, const BasePropWorld& world,
                          AutoArray<BasePropState>& out, std::vector<std::string>* notes)
{
    const int first = out.Size();
    const std::string who = std::string((const char*)base.name) + ": ";
    for (int r = 0; r < layout.rows.Size(); r++)
    {
        const BasePropRow& row = layout.rows[r];
        const RString cls = ResolveClass(row.cls, world);
        if (cls.GetLength() == 0)
        {
            Note(notes,
                 who + "row " + std::to_string(r) + " skipped: no spawnable class in '" + (const char*)row.cls + "'");
            continue;
        }
        BasePropBox box;
        if (!world.Box(cls, box))
        {
            Note(notes, who + "row " + std::to_string(r) + " skipped: '" + (const char*)cls + "' has no shape");
            continue;
        }
        const float rad = box.Radius();
        const float local = sqrtf(row.dx * row.dx + row.dz * row.dz);
        if (local - rad < layout.yard)
        {
            Note(notes, who + "row " + std::to_string(r) + " refused: '" + (const char*)cls + "' reaches into the " +
                            Fmt1(layout.yard) + " m yard");
            continue;
        }
        const float turn = row.dir + (row.jitter ? JitterDeg(base.layoutSeed, r, layout.jitterDeg) : 0.0f);
        const char* why = "";
        bool placed = false;
        // the authored spot, its left-right mirror, then both pushed outward
        // (away from the yard) by ShiftStep and twice that: a tree or a road
        // edge moves a row a few metres, never into the yard
        for (int attempt = 0; attempt < 6 && !placed; attempt++)
        {
            const bool mirror = (attempt & 1) != 0;
            if (mirror && row.dx == 0)
            {
                continue; // no mirror on the axis
            }
            const float push = (float)(attempt / 2) * ShiftStep;
            const float scale = local > 0 ? (local + push) / local : 1.0f;
            const float dx = (mirror ? -row.dx : row.dx) * scale;
            const float dz = row.dz * scale;
            const float dirLocal = mirror ? -turn : turn;
            float x = 0, z = 0;
            LocalToWorld(base.x, base.z, base.heading, dx, dz, x, z);
            if (world.NearRoad(x, z, rad + layout.roadMargin))
            {
                why = "road";
                continue;
            }
            if (world.Wet(x, z))
            {
                why = "water";
                continue;
            }
            if (world.Blocked(x, z, rad))
            {
                why = "object";
                continue;
            }
            bool overlap = false;
            for (int k = first; k < out.Size() && !overlap; k++)
            {
                const float ox = out[k].x - x;
                const float oz = out[k].z - z;
                const float rr = out[k].box.Radius() + rad;
                overlap = ox * ox + oz * oz < rr * rr;
            }
            if (overlap)
            {
                why = "overlap";
                continue;
            }
            BasePropState s;
            s.base = base.name;
            s.row = r;
            s.cls = cls;
            s.x = x;
            s.z = z;
            float dir = base.heading + dirLocal;
            dir = fmodf(dir, 360.0f);
            if (dir < 0)
            {
                dir += 360.0f;
            }
            s.dir = dir;
            s.box = box;
            out.Add(s);
            placed = true;
            if (attempt > 0)
            {
                Note(notes, who + "row " + std::to_string(r) + " '" + (const char*)cls + "' " +
                                (mirror ? "mirrored" : "moved") +
                                (push > 0 ? " and pushed out " + Fmt1(push) + " m" : "") + " (" + why + ")");
            }
        }
        if (!placed)
        {
            Note(notes, who + "row " + std::to_string(r) + " '" + (const char*)cls + "' dropped (" + why + ")");
        }
    }
    return out.Size() - first;
}

// ---------------------------------------------------------------------------
// the service
// ---------------------------------------------------------------------------

int BaseProps::NPropsOf(const char* base) const
{
    int n = 0;
    for (int i = 0; i < _rows.Size(); i++)
    {
        if (base && stricmp(_rows[i].base, base) == 0)
        {
            n++;
        }
    }
    return n;
}

bool BaseProps::IsPlanned(const char* base) const
{
    for (int i = 0; i < _planned.Size(); i++)
    {
        if (base && stricmp(_planned[i], base) == 0)
        {
            return true;
        }
    }
    return false;
}

bool BaseProps::NudgeClear(float& x, float& z) const
{
    bool moved = false;
    const float m = NudgeMargin;
    for (int pass = 0; pass < 6; pass++)
    {
        bool inside = false;
        for (int i = 0; i < _rows.Size(); i++)
        {
            const BasePropState& s = _rows[i];
            if (!InsideBox(s, x, z, m))
            {
                continue;
            }
            inside = true;
            const float d = s.dir * kDeg;
            const float c = cosf(d);
            const float sn = sinf(d);
            const float px = x - s.x;
            const float pz = z - s.z;
            float lx = px * c - pz * sn;
            float lz = px * sn + pz * c;
            // the shortest way out of the grown box, plus a hair
            const float exits[4] = {(s.box.maxX + m) - lx, lx - (s.box.minX - m), (s.box.maxZ + m) - lz,
                                    lz - (s.box.minZ - m)};
            int best = 0;
            for (int k = 1; k < 4; k++)
            {
                if (exits[k] < exits[best])
                {
                    best = k;
                }
            }
            const float step = exits[best] + 0.1f;
            if (best == 0)
            {
                lx += step;
            }
            else if (best == 1)
            {
                lx -= step;
            }
            else if (best == 2)
            {
                lz += step;
            }
            else
            {
                lz -= step;
            }
            // back to world: inverse of the rotation above
            x = s.x + lx * c + lz * sn;
            z = s.z - lx * sn + lz * c;
            moved = true;
        }
        if (!inside)
        {
            break;
        }
    }
    return moved;
}

int BaseProps::Sync(const GeneratedBaseRow* const* bases, int n, const BasePropWorld& world)
{
    int created = 0;
    for (int b = 0; b < n; b++)
    {
        const GeneratedBaseRow* base = bases[b];
        if (!base || IsPlanned(base->name))
        {
            continue;
        }
        _planned.Add(base->name);
        const BaseLayout* layout = FindLayout(base->type);
        if (!layout)
        {
            LOG_INFO(Core, "BaseProps: no layout for {} '{}'; it stands as a bare zone", (const char*)base->type,
                     (const char*)base->name);
            continue;
        }
        if (base->detected)
        {
            // a BaseDetect outpost already stands among real map props
            LOG_INFO(Core, "BaseProps: '{}' is a detected outpost with its own props; no layout",
                     (const char*)base->name);
            continue;
        }
        std::vector<std::string> notes;
        const int placed = PlanLayout(*layout, *base, world, _rows, &notes);
        for (const std::string& note : notes)
        {
            if (note.find("no spawnable class") != std::string::npos)
            {
                LOG_WARN(Core, "BaseProps: {}", note);
            }
            else
            {
                LOG_INFO(Core, "BaseProps: {}", note);
            }
        }
        if (placed < layout->minProps)
        {
            LOG_WARN(Core, "BaseProps: '{}' placed {} of {} layout rows, below minProps {}", (const char*)base->name,
                     placed, layout->rows.Size(), layout->minProps);
        }
        else
        {
            LOG_INFO(Core, "BaseProps: '{}' layout {}: {} of {} rows placed (heading {:.0f})", (const char*)base->name,
                     (const char*)layout->name, placed, layout->rows.Size(), (double)base->heading);
        }
    }
    for (int i = 0; i < _rows.Size(); i++)
    {
        BasePropState& s = _rows[i];
        if (world.Exists(s))
        {
            continue;
        }
        if (s.made < 0)
        {
            continue; // a creation already failed this session
        }
        if (world.Create(s))
        {
            if (s.made > 0)
            {
                LOG_INFO(Core, "BaseProps: re-created '{}' of '{}' at [{:.0f}, {:.0f}]", (const char*)s.cls,
                         (const char*)s.base, (double)s.x, (double)s.z);
            }
            s.made++;
            s.appliedTexture = RString();
            created++;
        }
        else
        {
            LOG_WARN(Core, "BaseProps: could not create '{}' for '{}'; not retried this session", (const char*)s.cls,
                     (const char*)s.base);
            s.made = -1;
        }
    }
    return created;
}

// ---------------------------------------------------------------------------
// the engine world
// ---------------------------------------------------------------------------

namespace
{

bool IsOwnProp(const Object* obj)
{
    const BaseProps& props = BaseProps::Instance();
    for (int i = 0; i < props.NProps(); i++)
    {
        if (props.PropAt(i)->prop.GetLink() == obj)
        {
            return true;
        }
    }
    return false;
}

class EngineBasePropWorld final : public BasePropWorld
{
  public:
    bool Spawnable(const char* cls) const override
    {
        ParsClassProbe probe;
        return probe.Spawnable(cls);
    }

    bool Box(const char* cls, BasePropBox& out) const override
    {
        for (const auto& c : _boxes)
        {
            if (stricmp(c.first.c_str(), cls) == 0)
            {
                out = c.second;
                return true;
            }
        }
        Ref<Entity> veh = NewNonAIVehicle(cls, nullptr);
        if (!veh || !veh->GetShape())
        {
            return false;
        }
        const Vector3 mn = veh->GetShape()->Min();
        const Vector3 mx = veh->GetShape()->Max();
        out.minX = mn.X();
        out.maxX = mx.X();
        out.minZ = mn.Z();
        out.maxZ = mx.Z();
        _boxes.emplace_back(std::string(cls), out);
        return true;
    }

    bool NearRoad(float x, float z, float clearance) const override
    {
        if (!GRoadNet || !GLandscape)
        {
            return false;
        }
        return GRoadNet->IsOnRoad(Vector3(x, GLOB_LAND->SurfaceY(x, z), z), clearance) != nullptr;
    }

    bool Wet(float x, float z) const override
    {
        if (!GLandscape)
        {
            return true;
        }
        return GLOB_LAND->SurfaceY(x, z) < 0.5f || GLOB_LAND->WaterDepth(x, z) > 0.0f;
    }

    bool Blocked(float x, float z, float r) const override
    {
        if (!GLandscape)
        {
            return false;
        }
        const int cx = toIntFloor(x * InvLandGrid);
        const int cz = toIntFloor(z * InvLandGrid);
        for (int ix = cx - 1; ix <= cx + 1; ix++)
        {
            for (int iz = cz - 1; iz <= cz + 1; iz++)
            {
                if (!InRange(ix, iz))
                {
                    continue;
                }
                const ObjectList& list = GLOB_LAND->GetObjects(iz, ix);
                for (int i = 0; i < list.Size(); i++)
                {
                    const Object* obj = list[i];
                    if (!obj || !obj->GetShape() || !obj->Static() || !obj->HasGeometry())
                    {
                        continue;
                    }
                    if (IsOwnProp(obj))
                    {
                        continue;
                    }
                    const Vector3 mn = obj->GetShape()->Min();
                    const Vector3 mx = obj->GetShape()->Max();
                    BasePropBox b;
                    b.minX = mn.X();
                    b.maxX = mx.X();
                    b.minZ = mn.Z();
                    b.maxZ = mx.Z();
                    const float rr = b.Radius() + r;
                    const float dx = obj->Position().X() - x;
                    const float dz = obj->Position().Z() - z;
                    if (dx * dx + dz * dz < rr * rr)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Exists(const BasePropState& s) const override
    {
        const EntityAI* e = s.prop.GetLink();
        return e && !e->ToDelete();
    }

    bool Create(BasePropState& s) const override
    {
        if (!GWorld || !GLandscape)
        {
            return false;
        }
        Ref<Entity> veh = NewNonAIVehicle(s.cls, nullptr);
        if (!veh)
        {
            return false;
        }
        EntityAI* vehAI = dyn_cast<EntityAI>(veh.GetRef());
        if (!vehAI)
        {
            return false;
        }
        const float d = s.dir * kDeg;
        Matrix3 orient;
        Matrix4 transform;
        transform.SetPosition(Vector3(s.x, GLOB_LAND->SurfaceYAboveWater(s.x, s.z), s.z));
        orient.SetUpAndDirection(VUp, Vector3(sinf(d), 0, cosf(d)));
        transform.SetOrientation(orient);
        // statics stand upright; PlaceOnSurface seats the bounding centre on
        // the ground and nets a buried model origin (StaticSeatOffsetY)
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
        // a cached oper field predating the prop would leave it invisible to
        // pathing and FindFreePosition: drop the 3 x 3 cells around it (the
        // Object::SetDestroyed idiom, Dammage.cpp)
        if (GLandscape->OperationalCache())
        {
            const int xLand = toIntFloor(s.x * InvLandGrid);
            const int zLand = toIntFloor(s.z * InvLandGrid);
            for (int x = xLand - 1; x <= xLand + 1; x++)
            {
                for (int z = zLand - 1; z <= zLand + 1; z++)
                {
                    GLandscape->OperationalCache()->RemoveField(x, z);
                }
            }
        }
        s.prop = vehAI;
        return true;
    }

    void Refresh(BasePropState& s, const char* owner) const override
    {
        FlagCarrier* flag = dyn_cast<FlagCarrier>(s.prop.GetLink());
        if (!flag || !owner)
        {
            return;
        }
        const RString want = TownFlags::ResolveFlagTexture(owner, ZoneRegistry::Instance().FactionValue(owner, "flag"));
        if (stricmp(want, s.appliedTexture) != 0)
        {
            flag->SetFlagTexture(want);
            s.appliedTexture = want;
        }
    }

  private:
    mutable std::vector<std::pair<std::string, BasePropBox>> _boxes;
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
const BasePropWorld& EngineWorld()
{
    static EngineBasePropWorld world;
    return world;
}
#pragma clang diagnostic pop

} // namespace

int BaseProps::SyncRegistry(const BasePropWorld& world)
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!_layoutsLoaded)
    {
        LoadLayouts(Pars.FindEntry("CfgGuerrillaBaseLayouts"));
    }
    std::vector<const GeneratedBaseRow*> bases;
    for (int i = 0; i < registry.NGeneratedBases(); i++)
    {
        bases.push_back(registry.GeneratedBaseAt(i));
    }
    const int created = Sync(bases.data(), (int)bases.size(), world);
    for (int i = 0; i < _rows.Size(); i++)
    {
        const int zi = registry.FindZoneIndex(_rows[i].base);
        const ZoneRecord* z = zi >= 0 ? registry.GetZone(zi) : nullptr;
        world.Refresh(_rows[i], z ? (const char*)z->owner : nullptr);
    }
    return created;
}

void BaseProps::InitMission()
{
    Clear();
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive() || registry.NGeneratedBases() == 0)
    {
        return;
    }
    if (!_worldOverride && (!GWorld || !GLandscape))
    {
        return;
    }
    // behind the loading screen, before the first garrison spawns
    const auto start = std::chrono::steady_clock::now();
    const int created = SyncRegistry(_worldOverride ? *_worldOverride : EngineWorld());
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    LOG_INFO(Core, "BaseProps: {} prop(s) created for {} generated base(s) at new game in {:.1f} ms", created,
             registry.NGeneratedBases(), ms);
}

void BaseProps::Simulate(float deltaT)
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive() || registry.NGeneratedBases() == 0)
    {
        return;
    }
    _accum += deltaT;
    if (_accum < TickInterval)
    {
        return;
    }
    _accum = 0;
    if (!_worldOverride && (!GWorld || !GLandscape))
    {
        return;
    }
    SyncRegistry(_worldOverride ? *_worldOverride : EngineWorld());
}

LSError BaseProps::Serialize(ParamArchive& ar)
{
    if (ar.IsSaving())
    {
        _pendingRows = _rows;
        _pendingPlanned = _planned;
    }
    PARAM_CHECK(ar.Serialize("Rows", _pendingRows, 1))
    PARAM_CHECK(ar.SerializeArray("planned", _pendingPlanned, 1))
    if (ar.IsSaving())
    {
        _pendingRows.Clear();
        _pendingPlanned.Clear();
    }
    else if (ar.GetPass() == ParamArchive::PassSecond)
    {
        // the prop refs resolved on this pass; the planned names stop a
        // second layout from being created over the saved one
        _rows = _pendingRows;
        _planned = _pendingPlanned;
        _pendingRows.Clear();
        _pendingPlanned.Clear();
        _accum = TickInterval;
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla

// ---------------------------------------------------------------------------
// script surface
// ---------------------------------------------------------------------------

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

// gmBaseProps -> one row per layout prop of the generated bases:
// [base, class, [easting, northing, height], dir, alive, radius, made, object,
//  bottom] (height is the object's ASL origin, 0 when it is gone; radius the
// footprint's 2D radius; made the creations this session; bottom the model's
// lowest vertex above the terrain under its origin, 0 when gone: a seated
// static reads about 0, a buried one below)
static GameValue GmBaseProps(const GameState* state)
{
    const BaseProps& props = BaseProps::Instance();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    for (int i = 0; i < props.NProps(); i++)
    {
        const BasePropState& s = *props.PropAt(i);
        EntityAI* e = s.prop.GetLink();
        const bool alive = e && !e->ToDelete();
        GameValue rowValue = state->CreateGameValue(GameArray);
        GameArrayType& row = rowValue;
        row.Resize(9);
        row[0] = GameStringType(s.base);
        row[1] = GameStringType(s.cls);
        GameValue posValue = state->CreateGameValue(GameArray);
        GameArrayType& pos = posValue;
        pos.Resize(3);
        pos[0] = alive ? e->Position().X() : s.x;
        pos[1] = alive ? e->Position().Z() : s.z;
        pos[2] = alive ? e->Position().Y() : 0.0f;
        row[2] = posValue;
        row[3] = s.dir;
        row[4] = alive;
        row[5] = s.box.Radius();
        row[6] = (float)s.made;
        row[7] = GameValueExt((Object*)(alive ? e : nullptr));
        float bottom = 0;
        if (alive && e->GetShape() && GLandscape)
        {
            bottom = e->Position().Y() + e->GetShape()->Min().Y() -
                     GLOB_LAND->SurfaceYAboveWater(e->Position().X(), e->Position().Z());
        }
        row[8] = bottom;
        array.Add(rowValue);
    }
    return value;
}

// gmBasePropsInside [easting, northing] -> the number of layout props whose
// footprint box holds the point (no margin)
static GameValue GmBasePropsInside(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 2))
    {
        return 0.0f;
    }
    const float x = array[0];
    const float z = array[1];
    const BaseProps& props = BaseProps::Instance();
    int n = 0;
    for (int i = 0; i < props.NProps(); i++)
    {
        if (BaseProps::InsideBox(*props.PropAt(i), x, z, 0.0f))
        {
            n++;
        }
    }
    return (float)n;
}

// gmBasePropsLz [easting, northing, minR, maxR] -> [lzEasting, lzNorthing]
// from OccupierAir's own LZ pick (the engine probe, with the generated-base
// exclusion), or [] when it finds none
static GameValue GmBasePropsLz(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& out = value;
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 4))
    {
        return value;
    }
    const Vector3 target((float)array[0], 0.0f, (float)array[1]);
    Vector3 lz;
    RString why;
    if (OccupierAir::Instance().PickLz(target, nullptr, (float)array[2], (float)array[3], lz, why))
    {
        out.Resize(2);
        out[0] = lz.X();
        out[1] = lz.Z();
    }
    return value;
}

INIT_MODULE(GuerrillaBaseProps, 3)
{
    GGameState.NewNularOp(GameNular(GameArray, "gmBaseProps", GmBaseProps));
    GGameState.NewFunction(GameFunction(GameScalar, "gmBasePropsInside", GmBasePropsInside, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "gmBasePropsLz", GmBasePropsLz, GameArray));
}
