#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>

#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Evaluator/express.hpp> // GameState / GameValue (event dispatch, VarGet)

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/VehicleAI.hpp> // Target / TargetList

#include <Poseidon/Foundation/platform.hpp>

#include <float.h>
#include <string.h>

namespace Poseidon::Guerrilla
{

// Process-lifetime singleton - no global constructor (same convention as
// ZoneRegistry::Instance).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
AlertMachine& AlertMachine::Instance()
{
    static AlertMachine instance;
    return instance;
}
#pragma clang diagnostic pop

static float Dist2DSq(float ax, float az, float bx, float bz)
{
    float dx = ax - bx;
    float dz = az - bz;
    return dx * dx + dz * dz;
}

// ---------------------------------------------------------------------------
// lifecycle / config
// ---------------------------------------------------------------------------

void AlertMachine::Clear()
{
    _tuning = AlertTuning();
    _states.Clear();
    for (int i = 0; i < NAlertEventTypes; i++)
    {
        _handlers[i] = RString();
    }
    _accum = 0;
    _breakLatched = false;
    _breakPending = false;
    _breakReason = RString();
    _pending.Clear();
}

void AlertMachine::LoadFromParams(const ParamEntry* zonesCfg)
{
    // tunables only; live per-zone state is reset solely by Clear so a
    // savegame's second-pass config rebuild does not wipe loaded state
    _tuning = AlertTuning();
    if (!zonesCfg)
    {
        return;
    }
    _tuning.alertInterval = zonesCfg->ReadValue("alertInterval", _tuning.alertInterval);
    _tuning.alertYellowKnows = zonesCfg->ReadValue("alertYellowKnows", _tuning.alertYellowKnows);
    _tuning.alertRedKnows = zonesCfg->ReadValue("alertRedKnows", _tuning.alertRedKnows);
    _tuning.alertWindowSeconds = zonesCfg->ReadValue("alertWindowSeconds", _tuning.alertWindowSeconds);
    _tuning.alertHeatYellow = zonesCfg->ReadValue("alertHeatYellow", _tuning.alertHeatYellow);
    _tuning.alertHeatRed = zonesCfg->ReadValue("alertHeatRed", _tuning.alertHeatRed);
    _tuning.alertHeatBreak = zonesCfg->ReadValue("alertHeatBreak", _tuning.alertHeatBreak);
}

// ---------------------------------------------------------------------------
// queries
// ---------------------------------------------------------------------------

int AlertMachine::GetZoneState(int index) const
{
    if (index < 0 || index >= _states.Size())
    {
        return ASGreen;
    }
    return _states[index].state;
}

bool AlertMachine::GetLastKnown(int index, Vector3& pos) const
{
    if (index < 0 || index >= _states.Size() || !_states[index].hasLastKnown)
    {
        return false;
    }
    pos = _states[index].lastKnown;
    return true;
}

float AlertMachine::GetZoneTimer(int index) const
{
    if (index < 0 || index >= _states.Size())
    {
        return 0;
    }
    return _states[index].timer;
}

// ---------------------------------------------------------------------------
// events
// ---------------------------------------------------------------------------

void AlertMachine::SetEventHandler(AlertEventType type, RString handler)
{
    if (type < 0 || type >= NAlertEventTypes)
    {
        return;
    }
    _handlers[type] = handler;
}

RString AlertMachine::GetEventHandler(AlertEventType type) const
{
    if (type < 0 || type >= NAlertEventTypes)
    {
        return RString();
    }
    return _handlers[type];
}

int AlertMachine::EventTypeFromName(const char* name)
{
    if (!name)
    {
        return -1;
    }
    if (stricmp(name, "alertChanged") == 0)
    {
        return AEAlertChanged;
    }
    if (stricmp(name, "undercoverBroken") == 0)
    {
        return AEUndercoverBroken;
    }
    return -1;
}

void AlertMachine::RequestBreak(RString reason)
{
    if (_breakPending)
    {
        return; // first reason wins until the next tick consumes it
    }
    _breakPending = true;
    _breakReason = reason;
}

// ---------------------------------------------------------------------------
// simulation
// ---------------------------------------------------------------------------

void AlertMachine::SyncZoneCount(int n)
{
    if (_states.Size() == n)
    {
        return;
    }
    // zone count changes only on (re)load; positional state is stale then
    _states.Resize(n);
    for (int i = 0; i < n; i++)
    {
        _states[i] = ZoneAlertState();
    }
}

void AlertMachine::Simulate(float deltaT)
{
    if (!GWorld)
    {
        return;
    }
    ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive())
    {
        return;
    }
    _accum += deltaT;
    if (_accum < _tuning.alertInterval)
    {
        return;
    }
    float dt = _accum;
    _accum = 0;

    AlertTickInputs in;
    GatherInputs(in, registry);
    in.breakRequested = _breakPending;
    in.breakReason = _breakReason;
    _breakPending = false;
    _breakReason = RString();

    AutoArray<AlertEventRecord> fired;
    EvaluateAlert(in, dt, registry, fired);
    // handlers run only after the machine's own state mutation completed
    DispatchEvents(fired, registry);
}

void AlertMachine::EvaluateAlert(const AlertTickInputs& in, float dt, ZoneRegistry& registry,
                                 AutoArray<AlertEventRecord>& fired)
{
    if (!registry.IsActive())
    {
        return;
    }
    const int n = registry.NZones();
    SyncZoneCount(n);

    // ---- undercover break (fire request / vehicle entry, alert.sqs:83-104)
    if (!in.undercover)
    {
        // cover dropped (or was re-established later by script): re-arm
        _breakLatched = false;
    }
    else if (!_breakLatched && (in.breakRequested || in.playerInVehicle))
    {
        _breakLatched = true;
        // Heat spike on the zone nearest the player (append-only, clamped)
        if (in.playerValid)
        {
            int nearest = -1;
            float bestSq = FLT_MAX;
            for (int i = 0; i < n; i++)
            {
                const ZoneRecord* z = registry.GetZone(i);
                float dSq = Dist2DSq(in.playerX, in.playerZ, z->pos.X(), z->pos.Z());
                if (dSq < bestSq)
                {
                    bestSq = dSq;
                    nearest = i;
                }
            }
            if (nearest >= 0)
            {
                registry.HeatRaise(nearest, _tuning.alertHeatBreak);
            }
        }
        AlertEventRecord ev;
        ev.type = AEUndercoverBroken;
        ev.reason = in.breakRequested ? in.breakReason : RString("vehicle");
        fired.Add(ev);
    }

    // ---- per-zone FSM (alert.sqs:125-170)
    for (int i = 0; i < n; i++)
    {
        ZoneAlertState& s = _states[i];
        float know = i < in.zones.Size() ? in.zones[i].knows : 0.0f;
        int oldState = s.state;
        int newState;

        if (know >= _tuning.alertRedKnows)
        {
            newState = ASRed;
            s.timer = 0;
        }
        else if (know >= _tuning.alertYellowKnows)
        {
            if (oldState == ASYellow)
            {
                // already YELLOW: bleed the disengage countdown
                s.timer -= dt;
                if (s.timer <= 0)
                {
                    newState = ASRed;
                    s.timer = 0;
                }
                else
                {
                    newState = ASYellow;
                }
            }
            else
            {
                // entering YELLOW from GREEN, or de-escalating RED -> YELLOW:
                // (re)start the window
                newState = ASYellow;
                s.timer = _tuning.alertWindowSeconds;
            }
        }
        else
        {
            // band GREEN: lost contact -> calm from any state (recoverable)
            newState = ASGreen;
            s.timer = 0;
        }

        // last-known player position while the contact qualifies
        if (know >= _tuning.alertYellowKnows && i < in.zones.Size() && in.zones[i].hasLastKnown)
        {
            s.hasLastKnown = true;
            s.lastKnown = in.zones[i].lastKnown;
        }

        s.state = newState;

        // one-shot Heat spike on ESCALATION edges only (newState > oldState)
        if (newState > oldState)
        {
            registry.HeatRaise(i, newState == ASRed ? _tuning.alertHeatRed : _tuning.alertHeatYellow);
        }
        if (newState != oldState)
        {
            AlertEventRecord ev;
            ev.type = AEAlertChanged;
            ev.zoneIndex = i;
            ev.oldState = oldState;
            ev.newState = newState;
            fired.Add(ev);
        }
    }
}

void AlertMachine::GatherInputs(AlertTickInputs& in, const ZoneRegistry& registry) const
{
    const int n = registry.NZones();
    in.zones.Resize(n);
    for (int i = 0; i < n; i++)
    {
        in.zones[i] = AlertZoneInputs();
    }

    World* world = GWorld;
    if (!world)
    {
        return;
    }

    Person* player = world->GetRealPlayer();
    if (player && !player->IsDammageDestroyed())
    {
        in.playerValid = true;
        in.playerX = player->Position().X();
        in.playerZ = player->Position().Z();
    }

    // undercover global is script-owned; nil (mission never set it) == false
    GameState* gstate = world->GetGameState();
    if (gstate)
    {
        GameValue undercover = gstate->VarGet("gmundercover");
        in.undercover = undercover.GetType() == GameBool && (GameBoolType)undercover;
    }

    if (!in.playerValid)
    {
        return;
    }

    // the (vehicle aP) != aP poll (see ObjVehicle, GameStateExtUi.cpp)
    AIUnit* playerUnit = player->CommanderUnit();
    if (playerUnit)
    {
        EntityAI* veh = playerUnit->GetVehicle();
        in.playerInVehicle = veh && veh != player;
    }

    // per-zone knowsAbout: max FadingSideAccuracy of the player across the
    // occupier center's groups, each group assigned to the nearest zone
    // whose center is within zoneArea of its leader (or first alive unit).
    // No center == no occupier units == nothing perceives the player.
    AICenter* occupier = FindSideCenter(registry.OccupierSide());
    if (!occupier)
    {
        return;
    }
    const float areaSq = registry.Tuning().zoneArea * registry.Tuning().zoneArea;
    for (int g = 0; g < occupier->NGroups(); g++)
    {
        AIGroup* grp = occupier->GetGroup(g);
        if (!grp)
        {
            continue;
        }
        AIUnit* refUnit = grp->Leader();
        if (!refUnit || refUnit->GetLifeState() != AIUnit::LSAlive)
        {
            refUnit = nullptr;
            for (int u = 0; u < MAX_UNITS_PER_GROUP && !refUnit; u++)
            {
                AIUnit* unit = grp->UnitWithID(u + 1);
                if (unit && unit->GetLifeState() == AIUnit::LSAlive)
                {
                    refUnit = unit;
                }
            }
        }
        if (!refUnit)
        {
            continue;
        }
        Vector3 pos = refUnit->Position();
        int zone = -1;
        float bestSq = areaSq;
        for (int i = 0; i < n; i++)
        {
            const ZoneRecord* z = registry.GetZone(i);
            float dSq = Dist2DSq(pos.X(), pos.Z(), z->pos.X(), z->pos.Z());
            if (dSq < bestSq)
            {
                bestSq = dSq;
                zone = i;
            }
        }
        if (zone < 0)
        {
            continue;
        }
        Target* tgt = grp->FindTarget(player);
        if (!tgt)
        {
            continue;
        }
        float know = tgt->FadingSideAccuracy();
        AlertZoneInputs& zi = in.zones[zone];
        if (know > zi.knows)
        {
            zi.knows = know;
            // last-known from the group's reported target position (the
            // knownTargets source); alert.sqs approximated with getPos aP
            if (tgt->IsKnown())
            {
                zi.hasLastKnown = true;
                zi.lastKnown = tgt->posReported;
            }
        }
    }

    // fallback for qualifying contacts without a usable target position:
    // the player's current position, like the script did
    for (int i = 0; i < n; i++)
    {
        AlertZoneInputs& zi = in.zones[i];
        if (zi.knows >= _tuning.alertYellowKnows && !zi.hasLastKnown)
        {
            zi.hasLastKnown = true;
            zi.lastKnown = player->Position();
        }
    }
}

void AlertMachine::DispatchEvents(const AutoArray<AlertEventRecord>& fired, const ZoneRegistry& registry)
{
    if (fired.Size() == 0 || !GWorld)
    {
        return;
    }
    GameState* gstate = GWorld->GetGameState();
    if (!gstate)
    {
        return;
    }
    for (int i = 0; i < fired.Size(); i++)
    {
        const AlertEventRecord& ev = fired[i];
        RString handler = GetEventHandler(ev.type);
        if (handler.GetLength() == 0)
        {
            continue;
        }

        GameArrayType pars;
        if (ev.type == AEAlertChanged)
        {
            const ZoneRecord* z = registry.GetZone(ev.zoneIndex);
            if (!z)
            {
                continue;
            }
            pars.Resize(4);
            pars[0] = (float)ev.zoneIndex;
            pars[1] = GameStringType(z->name);
            pars[2] = (float)ev.oldState;
            pars[3] = (float)ev.newState;
        }
        else
        {
            pars.Resize(1);
            pars[0] = GameStringType(ev.reason);
        }

        // dispatch idiom copied from ZoneRegistry::DispatchEvents
        GameVarSpace local;
        gstate->BeginContext(&local);
        gstate->VarSetLocal("_this", GameValue(pars), true);
        gstate->Execute(handler);
        gstate->EndContext();
    }
}

// ---------------------------------------------------------------------------
// serialization
// ---------------------------------------------------------------------------

LSError AlertMachine::AlertSaveState::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("name", name, 1, RString()))
    PARAM_CHECK(ar.Serialize("state", state, 1, (int)ASGreen))
    PARAM_CHECK(ar.Serialize("timer", timer, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("hasLastKnown", hasLastKnown, 1, false))
    PARAM_CHECK(ar.Serialize("lastKnown", lastKnown, 1, VZero))
    return LSOK;
}

void AlertMachine::ApplyPendingLoad(const ZoneRegistry& registry)
{
    SyncZoneCount(registry.NZones());
    for (int i = 0; i < _pending.Size(); i++)
    {
        const AlertSaveState& row = _pending[i];
        // saved rows are matched to the registry's zone table by NAME; rows
        // without a config zone are dropped, zones without a row keep GREEN
        int index = registry.FindZoneIndex(row.name);
        if (index < 0)
        {
            continue;
        }
        ZoneAlertState& s = _states[index];
        s.state = row.state;
        if (s.state < ASGreen || s.state > ASRed)
        {
            s.state = ASGreen;
        }
        s.timer = row.timer;
        s.hasLastKnown = row.hasLastKnown;
        s.lastKnown = row.lastKnown;
    }
}

LSError AlertMachine::Serialize(ParamArchive& ar, ZoneRegistry& registry)
{
    if (ar.IsSaving())
    {
        _pending.Clear();
        int n = _states.Size();
        if (n > registry.NZones())
        {
            n = registry.NZones();
        }
        for (int i = 0; i < n; i++)
        {
            const ZoneAlertState& s = _states[i];
            AlertSaveState row;
            row.name = registry.GetZone(i)->name;
            row.state = s.state;
            row.timer = s.timer;
            row.hasLastKnown = s.hasLastKnown;
            row.lastKnown = s.lastKnown;
            _pending.Add(row);
        }
    }

    PARAM_CHECK(ar.Serialize("onAlertChanged", _handlers[AEAlertChanged], 1, RString()))
    PARAM_CHECK(ar.Serialize("onUndercoverBroken", _handlers[AEUndercoverBroken], 1, RString()))
    PARAM_CHECK(ar.Serialize("breakLatched", _breakLatched, 1, false))
    // a gmBreakUndercover issued in the tick-interval window before the save
    // must survive the load (defaults keep older saves readable)
    PARAM_CHECK(ar.Serialize("breakPending", _breakPending, 1, false))
    PARAM_CHECK(ar.Serialize("breakReason", _breakReason, 1, RString()))
    PARAM_CHECK(ar.Serialize("Zones", _pending, 1))

    if (ar.IsSaving())
    {
        _pending.Clear();
    }
    else if (ar.GetPass() == ParamArchive::PassSecond)
    {
        // ZoneRegistry::Serialize rebuilt its zone table from the reparsed
        // mission config earlier in this pass, so name lookups work now
        ApplyPendingLoad(registry);
        _pending.Clear();
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
