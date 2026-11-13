#include <Poseidon/Game/Kaiju/KaijuService.hpp>
#include <Poseidon/Game/Kaiju/KaijuMath.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/EntityAI.hpp>
#include <Poseidon/Audio/IAudioSystem.hpp>
#include <Poseidon/Audio/SoundScene.hpp>
#include <Poseidon/UI/OptionsUI.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/World/Entities/Infantry/SoldierOld.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/World.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace Poseidon
{
GameValue ParticleDrop(const GameState* state, GameValuePar oper1); // Smokes.cpp, the `drop` command

namespace Kaiju
{

int GTraitCount = 0;
int GFarCount = 0;
int GSteerCount = 0;
float GGridRadius = 0;

void EnsureKaijuCommandsLinked(); // KaijuCommands.cpp; keeps its INIT_MODULE in the link

namespace
{
std::vector<std::unique_ptr<Unit>> s_units;
std::vector<Tremor> s_tremors;

constexpr int kMaxHits = 64;
constexpr int kMaxTremors = 32;

float Now()
{
    return Glob.time.toFloat();
}

bool NameIs(const char* a, const char* b)
{
    return stricmp(a, b) == 0;
}
} // namespace

KaijuLight::KaijuLight(Object* vehicle, Vector3Par pos)
    : LightPoint(Color(1, 1, 1), Color(0, 0, 0)), AttachedOnVehicle(vehicle, pos, VForward)
{
}

std::vector<std::unique_ptr<Unit>>& Units()
{
    return s_units;
}

Unit* FindUnit(const Object* obj)
{
    if (!obj)
    {
        return nullptr;
    }
    for (auto& u : s_units)
    {
        if (u->obj.GetLink() == obj)
        {
            return u.get();
        }
    }
    return nullptr;
}

const Traits* FindTraits(const Object* obj)
{
    Unit* u = FindUnit(obj);
    return u ? &u->traits : nullptr;
}

Unit& EnsureUnit(Object* obj)
{
    EnsureKaijuCommandsLinked();
    if (Unit* u = FindUnit(obj))
    {
        return *u;
    }
    s_units.push_back(std::make_unique<Unit>());
    Unit& u = *s_units.back();
    u.obj = obj;
    Recount();
    return u;
}

void RemoveUnit(const Object* obj)
{
    s_units.erase(std::remove_if(s_units.begin(), s_units.end(),
                                 [obj](const std::unique_ptr<Unit>& u) { return u->obj.GetLink() == obj; }),
                  s_units.end());
    Recount();
}

void Recount()
{
    int far = 0, steer = 0;
    float grid = 0;
    for (auto& u : s_units)
    {
        if (u->traits.farDraw > 0)
        {
            far++;
        }
        if (u->steer)
        {
            steer++;
        }
        grid = std::max(grid, u->traits.gridRadius);
    }
    GTraitCount = (int)s_units.size();
    GFarCount = far;
    GSteerCount = steer;
    GGridRadius = grid;
}

bool SetTrait(Object* obj, const char* name, float value)
{
    if (!obj || !name)
    {
        return false;
    }
    Traits& t = EnsureUnit(obj).traits;
    if (NameIs(name, "farDraw"))
    {
        t.farDraw = std::max(value, 0.0f);
    }
    else if (NameIs(name, "farContrast"))
    {
        t.farContrast = Clamp01(value);
    }
    else if (NameIs(name, "wadeDepth"))
    {
        t.wadeDepth = std::max(value, 0.0f);
    }
    else if (NameIs(name, "obstacleBlind"))
    {
        t.obstacleBlind = value > 0.5f;
    }
    else if (NameIs(name, "noSpeedDamage"))
    {
        t.noSpeedDamage = value > 0.5f;
    }
    else if (NameIs(name, "damageCeiling"))
    {
        t.damageCeiling = value < 0 ? -1.0f : std::min(value, 1.0f);
    }
    else if (NameIs(name, "hitReport"))
    {
        t.hitReport = value > 0.5f;
    }
    else if (NameIs(name, "stepShake"))
    {
        t.stepShake = std::max(value, 0.0f);
    }
    else if (NameIs(name, "gridRadius"))
    {
        t.gridRadius = std::max(value, 0.0f);
    }
    else if (NameIs(name, "nearMip"))
    {
        t.nearMip = value > 0.5f;
    }
    else if (NameIs(name, "glow"))
    {
        t.glow = value < 0 ? -1.0f : std::min(value, 2.0f);
    }
    else if (NameIs(name, "farGlow"))
    {
        t.farGlow = Clamp01(value);
    }
    else if (NameIs(name, "plantBlend"))
    {
        t.plantBlend = value > 0.5f;
    }
    else if (NameIs(name, "pace"))
    {
        t.pace = value <= 0 ? 1.0f : std::min(std::max(value, 0.2f), 2.0f);
    }
    else if (NameIs(name, "bodySink"))
    {
        t.bodySink = std::min(std::max(value, 0.0f), 100.0f);
    }
    else if (NameIs(name, "lodPin"))
    {
        t.lodPin = value > 0.5f;
    }
    else
    {
        return false;
    }
    Recount();
    return true;
}

bool GetTrait(const Object* obj, const char* name, float& value)
{
    const Traits* t = FindTraits(obj);
    Traits stock;
    if (!t)
    {
        t = &stock;
    }
    if (NameIs(name, "farDraw"))
    {
        value = t->farDraw;
    }
    else if (NameIs(name, "farContrast"))
    {
        value = t->farContrast;
    }
    else if (NameIs(name, "wadeDepth"))
    {
        value = t->wadeDepth;
    }
    else if (NameIs(name, "obstacleBlind"))
    {
        value = t->obstacleBlind ? 1.0f : 0.0f;
    }
    else if (NameIs(name, "noSpeedDamage"))
    {
        value = t->noSpeedDamage ? 1.0f : 0.0f;
    }
    else if (NameIs(name, "damageCeiling"))
    {
        value = t->damageCeiling;
    }
    else if (NameIs(name, "hitReport"))
    {
        value = t->hitReport ? 1.0f : 0.0f;
    }
    else if (NameIs(name, "stepShake"))
    {
        value = t->stepShake;
    }
    else if (NameIs(name, "gridRadius"))
    {
        value = t->gridRadius;
    }
    else if (NameIs(name, "nearMip"))
    {
        value = t->nearMip ? 1.0f : 0.0f;
    }
    else if (NameIs(name, "glow"))
    {
        value = t->glow;
    }
    else if (NameIs(name, "farGlow"))
    {
        value = t->farGlow;
    }
    else if (NameIs(name, "plantBlend"))
    {
        value = t->plantBlend ? 1.0f : 0.0f;
    }
    else if (NameIs(name, "pace"))
    {
        value = t->pace;
    }
    else if (NameIs(name, "bodySink"))
    {
        value = t->bodySink;
    }
    else if (NameIs(name, "lodPin"))
    {
        value = t->lodPin ? 1.0f : 0.0f;
    }
    else
    {
        return false;
    }
    return true;
}

void ApplyTypeConfig(Object* obj, const ParamEntry& typeCfg)
{
    const ParamEntry* cls = typeCfg.FindEntry("Kaiju");
    if (!cls || !cls->IsClass())
    {
        return;
    }
    static const char* const names[] = {"farDraw",       "farContrast",   "wadeDepth", "obstacleBlind",
                                        "noSpeedDamage", "damageCeiling", "hitReport", "stepShake",
                                        "gridRadius",    "nearMip",       "glow",      "farGlow",
                                        "plantBlend",    "pace",          "bodySink",  "lodPin"};
    for (const char* name : names)
    {
        const ParamEntry* e = cls->FindEntry(name);
        if (e)
        {
            float value = *e;
            SetTrait(obj, name, value);
        }
    }
}

// ------------------------------------------------------------ camera shake

void AddTremor(Vector3Par pos, float strength, float fullRadius, float zeroRadius, float duration, float freq)
{
    if (strength <= 0 || duration <= 0)
    {
        return;
    }
    EnsureKaijuCommandsLinked();
    if ((int)s_tremors.size() >= kMaxTremors)
    {
        s_tremors.erase(s_tremors.begin());
    }
    Tremor t;
    t.pos = pos;
    t.strength = strength;
    t.fullRadius = std::max(fullRadius, 0.0f);
    t.zeroRadius = std::max(zeroRadius, t.fullRadius);
    t.start = Now();
    t.duration = duration;
    t.freq = freq > 0 ? freq : 7.0f;
    s_tremors.push_back(t);
}

float ShakeGainAt(Vector3Par pos, float* freq)
{
    float total = 0;
    float best = 0;
    float now = Now();
    for (const Tremor& t : s_tremors)
    {
        float g = TremorGain(t.strength, t.pos.Distance(pos), t.fullRadius, t.zeroRadius, now - t.start, t.duration);
        total += g;
        if (g > best)
        {
            best = g;
            if (freq)
            {
                *freq = t.freq;
            }
        }
    }
    return std::min(total, kShakeMaxGain);
}

int TremorCount()
{
    return (int)s_tremors.size();
}

void ApplyCameraShake(Matrix4& camTransform)
{
    if (s_tremors.empty())
    {
        return;
    }
    float freq = 7.0f;
    float gain = ShakeGainAt(camTransform.Position(), &freq);
    if (gain <= 0.0005f)
    {
        return;
    }
    ShakeOffset o = ShakeAt(Now(), gain, freq);
    Matrix3 orient = camTransform.Orientation();
    Vector3 pos = camTransform.Position();
    pos += orient.DirectionAside() * o.right + orient.DirectionUp() * o.up + orient.Direction() * o.forward;
    Matrix3 rot = Matrix3(MRotationY, o.yaw) * Matrix3(MRotationX, o.pitch) * Matrix3(MRotationZ, o.roll);
    camTransform.SetOrientation(orient * rot);
    camTransform.SetPosition(pos);
}

// ------------------------------------------------------- light and emitter

bool SetLight(Object* obj, const char* slot, const char* point, Vector3Par offset, ColorVal color, float brightness)
{
    if (!obj || !slot || !GScene)
    {
        return false;
    }
    Unit& u = EnsureUnit(obj);
    auto it =
        std::find_if(u.lights.begin(), u.lights.end(), [slot](const LightSlot& s) { return NameIs(s.name, slot); });
    if (brightness <= 0)
    {
        if (it != u.lights.end())
        {
            u.lights.erase(it);
        }
        return true;
    }
    if (it == u.lights.end())
    {
        LightSlot s;
        s.name = slot;
        u.lights.push_back(s);
        it = u.lights.end() - 1;
    }
    LightSlot& s = *it;
    s.pointIndex = -1;
    s.offset = offset;
    LODShape* shape = obj->GetShape();
    if (point && *point && shape)
    {
        int level = shape->FindMemoryLevel();
        Shape* memory = level >= 0 ? shape->Level(level) : nullptr;
        if (memory)
        {
            s.pointIndex = memory->PointIndex(point);
            if (s.pointIndex >= 0)
            {
                s.offset = shape->MemoryPoint(point);
            }
        }
    }
    if (!s.light)
    {
        s.light = new KaijuLight(obj, s.offset);
        GScene->AddLight(s.light);
    }
    s.light->SetDiffuse(color);
    s.light->SetAmbient(color * 0.15f);
    s.light->SetBrightness(brightness);
    s.light->Switch(true);
    s.light->SetAttachedPos(s.offset, VForward);
    s.light->UpdatePosition();
    return true;
}

int LightCount(const Object* obj)
{
    Unit* u = FindUnit(obj);
    return u ? (int)u->lights.size() : 0;
}

bool SetEmitter(Object* obj, const char* slot, const GameValue& drop, float rate)
{
    if (!obj || !slot)
    {
        return false;
    }
    Unit& u = EnsureUnit(obj);
    auto it = std::find_if(u.emitters.begin(), u.emitters.end(),
                           [slot](const EmitterSlot& s) { return NameIs(s.name, slot); });
    if (rate <= 0)
    {
        if (it != u.emitters.end())
        {
            u.emitters.erase(it);
        }
        return true;
    }
    if (it == u.emitters.end())
    {
        EmitterSlot s;
        s.name = slot;
        u.emitters.push_back(s);
        it = u.emitters.end() - 1;
    }
    it->drop = drop;
    it->rate = std::min(rate, 200.0f);
    return true;
}

// ------------------------------------------------------------------ steer

bool SetSteerPos(Object* obj, Vector3Par pos, Gait gait, float arriveRadius, float rate)
{
    if (!obj)
    {
        return false;
    }
    Unit& u = EnsureUnit(obj);
    u.steer = true;
    u.steerToPos = true;
    u.steerPos = pos;
    u.steerRate = rate;
    u.gait = gait;
    u.arriveRadius = std::max(arriveRadius, 1.0f);
    u.arrived = false;
    Recount();
    return true;
}

bool SetSteerHeading(Object* obj, float heading, Gait gait, float rate)
{
    if (!obj)
    {
        return false;
    }
    Unit& u = EnsureUnit(obj);
    u.steer = true;
    u.steerToPos = false;
    u.steerRate = rate;
    u.steerHeading = heading;
    u.gait = gait;
    u.arrived = false;
    Recount();
    return true;
}

void StopSteer(Object* obj)
{
    if (Unit* u = FindUnit(obj))
    {
        u->steer = false;
        Recount();
    }
}

bool GetSteer(const Man* man, SteerOrder& out)
{
    Unit* u = FindUnit(man);
    if (!u || !u->steer)
    {
        return false;
    }
    Vector3Val dir = man->Direction();
    float heading = std::atan2(dir.X(), dir.Z());
    float wanted = u->steerHeading;
    if (u->steerToPos)
    {
        float dx = u->steerPos.X() - man->Position().X();
        float dz = u->steerPos.Z() - man->Position().Z();
        if (dx * dx + dz * dz <= u->arriveRadius * u->arriveRadius)
        {
            u->arrived = true;
        }
        if (u->arrived)
        {
            out = SteerOrder();
            return true;
        }
        wanted = std::atan2(dx, dz);
    }
    out.headChange = HeadingError(heading, wanted);
    out.gait = u->gait;
    switch (u->gait)
    {
        case GaitWalk:
            out.speed = 1.0f;
            break;
        case GaitSlow:
            out.speed = 3.0f;
            break;
        case GaitFast:
            out.speed = 6.0f;
            break;
        default:
            out.speed = 0;
            break;
    }
    return true;
}

// The clip rate of a steered unit. The gait picks the clip; the pilot's speed
// (GetSteer) only says "move", so without this the walk gait played its clip
// at the slowest rate the move allows.
bool GetSteerRate(const Man* man, float& rate)
{
    const Unit* u = FindUnit(man);
    if (!u || !u->steer || u->arrived || u->gait == GaitStop)
    {
        return false;
    }
    rate = u->steerRate;
    return true;
}

// ------------------------------------------------------------------- glow

bool GetGlow(const Object* obj, float& scale, float& heat)
{
    const Unit* u = FindUnit(obj);
    if (!u || u->traits.glow < 0)
    {
        return false;
    }
    GlowSplit(u->traits.glow, scale, heat);
    return true;
}

// ------------------------------------------------------- damage, knockback

static void ClampUnitDamage(Object* obj, float ceiling)
{
    if (ceiling < 0)
    {
        return;
    }
    const float total = obj->GetRawTotalDammage();
    const float clamped = ClampDamage(total, ceiling);
    if (clamped < total)
    {
        obj->SetTotalDammage(clamped);
    }
    EntityAI* ai = dyn_cast<EntityAI>(obj);
    if (!ai)
    {
        return;
    }
    const HitPointList& hits = ai->GetType()->GetHitPoints();
    for (int i = 0; i < hits.Size(); i++)
    {
        const HitPoint& hp = *hits[i];
        if (hp.GetIndex() >= 0 && ai->GetHitCont(hp) > ceiling)
        {
            ai->ChangeHit(hp.GetIndex(), ceiling);
        }
    }
}

void OnLocalDammage(Object* obj, EntityAI* owner, const char* ammo, Vector3Par modelPos, float delta)
{
    Unit* u = FindUnit(obj);
    if (!u)
    {
        return;
    }
    if (u->traits.hitReport)
    {
        if ((int)u->hits.size() >= kMaxHits)
        {
            u->hits.erase(u->hits.begin());
            u->hitsDropped++;
        }
        HitRecord r;
        r.attacker = owner;
        r.ammo = ammo ? ammo : "";
        r.modelPos = modelPos;
        r.worldPos = obj->PositionModelToWorld(modelPos);
        r.delta = delta;
        r.time = Now();
        u->hits.push_back(r);
    }
    ClampUnitDamage(obj, u->traits.damageCeiling);
}

bool PointWorld(const Object* obj, const char* point, Vector3& out)
{
    LODShape* shape = obj ? obj->GetShape() : nullptr;
    if (!shape || !point || !*point)
    {
        return false;
    }
    int level = shape->FindMemoryLevel();
    Shape* memory = level >= 0 ? shape->Level(level) : nullptr;
    if (!memory)
    {
        return false;
    }
    int index = memory->PointIndex(point);
    if (index < 0)
    {
        return false;
    }
    out = obj->PositionModelToWorld(obj->AnimatePoint(level, index));
    return true;
}

bool Knockback(Object* obj, Vector3Par deltaV)
{
    Entity* ent = dyn_cast<Entity>(obj);
    if (!ent)
    {
        return false;
    }
    ent->AddImpulseNetAware(deltaV * ent->GetMass(), VZero);
    return true;
}

// The roar as a world event: the sound leaves the unit and arrives late with
// distance, every group led from inside revealRadius learns exactly where the
// unit is, and every group led from inside fearRadius (the player's own
// excepted) breaks and runs from it for fearSeconds.
bool Roar(Object* obj, const char* sound, float revealRadius, float fearRadius, float fearSeconds, int& revealed,
          int& fled)
{
    revealed = 0;
    fled = 0;
    if (!obj || !GWorld)
    {
        return false;
    }
    if (sound && *sound && GSoundScene)
    {
        SoundPars pars;
        pars.name = "";
        FindSound(sound, pars);
        if (pars.name.GetLength() > 0)
        {
            IWave* wave = GSoundScene->OpenAndPlayOnce(pars.name, obj->COMPosition(), VZero, pars.vol, pars.freq);
            if (wave)
            {
                GSoundScene->SimulateSpeedOfSound(wave);
                GSoundScene->AddSound(wave);
            }
        }
    }
    EntityAI* ai = dyn_cast<EntityAI>(obj);
    AIGroup* own = ai ? ai->GetGroup() : nullptr;
    AICenter* centers[] = {GWorld->GetWestCenter(), GWorld->GetEastCenter(), GWorld->GetGuerrilaCenter(),
                           GWorld->GetCivilianCenter()};
    for (AICenter* center : centers)
    {
        if (!center)
        {
            continue;
        }
        for (int i = 0; i < center->NGroups(); i++)
        {
            AIGroup* grp = center->GetGroup(i);
            if (!grp || grp == own || !grp->Leader())
            {
                continue;
            }
            float dist2 = grp->Leader()->Position().Distance2(obj->Position());
            if (ai && revealRadius > 0 && dist2 < Square(revealRadius))
            {
                grp->AddTarget(ai, 1, 1, 0);
                revealed++;
            }
            if (fearRadius > 0 && fearSeconds > 0 && dist2 < Square(fearRadius) && !grp->IsAnyPlayerGroup())
            {
                // The engine flees toward the group's first waypoint, which
                // for a script-made group is where it was created (or the
                // map corner), so the waypoint is moved to a point further
                // away from the unit first.
                Vector3 away = grp->Leader()->Position() - obj->Position();
                away[1] = 0;
                if (away.SquareSize() < 1.0f)
                {
                    away = obj->Direction();
                    away[1] = 0;
                }
                away.Normalize();
                Vector3 fleeTo = grp->Leader()->Position() + away * (fearRadius * 0.5f + 300.0f);
                if (grp->NWaypoints() <= 0)
                {
                    grp->AddFirstWaypoint(fleeTo);
                }
                else
                {
                    grp->GetWaypoint(0).position = fleeTo;
                }
                grp->ForceFlee(fearSeconds);
                if (grp->GetFlee())
                {
                    fled++;
                }
            }
        }
    }
    return true;
}

void OnFootfall(Man* man, bool left)
{
    Unit* u = FindUnit(man);
    if (!u)
    {
        return;
    }
    u->stepCount++;
    u->stepLeft = left;
    // the foot that landed, from the model's foot memory points when it has them
    if (!PointWorld(man, left ? "stopaL" : "stopaP", u->stepPos))
    {
        u->stepPos = man->Position();
    }
    if (u->traits.stepShake > 0)
    {
        // felt hard within a body length, gone at a quarter of a kilometre
        float radius = std::max(man->GetRadius(), 5.0f);
        AddTremor(u->stepPos, u->traits.stepShake, radius, radius * 10.0f, 0.9f, 5.0f);
    }
}

// -------------------------------------------------------------- lifetime

void Simulate(float deltaT)
{
    if (s_units.empty() && s_tremors.empty())
    {
        return;
    }
    float now = Now();
    s_tremors.erase(std::remove_if(s_tremors.begin(), s_tremors.end(),
                                   [now](const Tremor& t) { return now - t.start >= t.duration || now < t.start; }),
                    s_tremors.end());

    bool pruned = false;
    for (size_t i = s_units.size(); i-- > 0;)
    {
        if (!s_units[i]->obj.GetLink())
        {
            s_units.erase(s_units.begin() + i);
            pruned = true;
        }
    }
    if (pruned)
    {
        Recount();
    }

    Vector3 camPos = GScene && GScene->GetCamera() ? GScene->GetCamera()->Position() : VZero;
    for (auto& up : s_units)
    {
        Unit& u = *up;
        Object* obj = u.obj.GetLink();
        ClampUnitDamage(obj, u.traits.damageCeiling);

        LODShape* shape = obj->GetShape();
        int memLevel = shape ? shape->FindMemoryLevel() : -1;
        for (LightSlot& s : u.lights)
        {
            if (s.light && s.pointIndex >= 0 && memLevel >= 0)
            {
                s.light->SetAttachedPos(obj->AnimatePoint(memLevel, s.pointIndex), VForward);
            }
        }

        if (!u.emitters.empty() && deltaT > 0 && obj->Position().Distance2(camPos) < Square(3000.0f))
        {
            for (EmitterSlot& e : u.emitters)
            {
                e.carry += e.rate * deltaT;
                int n = (int)e.carry;
                e.carry -= n;
                n = std::min(n, 8);
                for (int k = 0; k < n; k++)
                {
                    ParticleDrop(&GGameState, e.drop);
                }
            }
        }
    }
}

void InitMission()
{
    // Units of the mission being started may already exist (a type's
    // `class Kaiju` block registers them in the Man constructor), so only the
    // records whose unit is gone are dropped here.
    s_units.erase(std::remove_if(s_units.begin(), s_units.end(),
                                 [](const std::unique_ptr<Unit>& u) { return !u->obj.GetLink(); }),
                  s_units.end());
    s_tremors.clear();
    Recount();
}

void Shutdown()
{
    s_units.clear();
    s_tremors.clear();
    Recount();
}

} // namespace Kaiju
} // namespace Poseidon
