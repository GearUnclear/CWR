#include <Poseidon/Game/Guerrilla/StashRegistry.hpp>

#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/AI/EntityAI.hpp> // EntityAI (Position / ToDelete)

#include <Poseidon/Foundation/Framework/DebugLog.hpp>

namespace Poseidon::Guerrilla
{

// Defined in StashRegistryCommands.cpp.  Referencing it from here forces the
// command TU (whose only other content is an INIT_MODULE registration) into
// the link - same pattern as EnsureZoneRegistryCommandsLinked.
void EnsureStashRegistryCommandsLinked();

// Process-lifetime singleton - no global constructor (see express.hpp's
// GGameState for the convention).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
StashRegistry& StashRegistry::Instance()
{
    EnsureStashRegistryCommandsLinked();
    static StashRegistry instance;
    return instance;
}
#pragma clang diagnostic pop

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------

void StashRegistry::Clear()
{
    _rows.Clear();
    _pending.Clear();
    _accum = 0;
}

void StashRegistry::InitMission()
{
    Clear();
}

// ---------------------------------------------------------------------------
// bookkeeping
// ---------------------------------------------------------------------------

int StashRegistry::FindRow(const EntityAI* obj) const
{
    for (int i = 0; i < _rows.Size(); i++)
    {
        if (_rows[i].obj.GetLink() == obj)
        {
            return i;
        }
    }
    return -1;
}

bool StashRegistry::Register(EntityAI* obj)
{
    if (!obj)
    {
        return false;
    }
    if (FindRow(obj) >= 0)
    {
        return true; // idempotent
    }
    StashRow row;
    row.obj = obj;
    row.pos = obj->Position();
    _rows.Add(row);
    LOG_INFO(Core, "StashRegistry: registered stash at [{:.0f},{:.0f}] ({} total)", row.pos.X(), row.pos.Z(),
             _rows.Size());
    return true;
}

bool StashRegistry::Unregister(EntityAI* obj)
{
    if (!obj)
    {
        return false;
    }
    int i = FindRow(obj);
    if (i < 0)
    {
        return false;
    }
    _rows.Delete(i);
    return true;
}

int StashRegistry::Count() const
{
    return _rows.Size();
}

EntityAI* StashRegistry::GetObject(int i) const
{
    if (i < 0 || i >= _rows.Size())
    {
        return nullptr;
    }
    return _rows[i].obj.GetLink();
}

Vector3 StashRegistry::GetPos(int i) const
{
    if (i < 0 || i >= _rows.Size())
    {
        return VZero;
    }
    return _rows[i].pos;
}

void StashRegistry::AddRowForTest(Vector3Par pos)
{
    StashRow row;
    row.pos = pos;
    _rows.Add(row);
}

// ---------------------------------------------------------------------------
// simulation
// ---------------------------------------------------------------------------

void StashRegistry::Simulate(float deltaT)
{
    // deliberately NOT gated on the ZoneRegistry - stashes work in ordinary
    // missions; an empty registry costs one Size() check per frame
    if (_rows.Size() == 0)
    {
        return;
    }
    _accum += deltaT;
    if (_accum < TickInterval)
    {
        return;
    }
    _accum = 0;

    for (int i = _rows.Size() - 1; i >= 0; i--)
    {
        EntityAI* obj = _rows[i].obj.GetLink();
        // ToDelete closes the window between SetDelete (e.g. a script's
        // deleteVehicle) and the world actually removing the holder
        if (!obj || obj->ToDelete())
        {
            LOG_INFO(Core, "StashRegistry: stash at [{:.0f},{:.0f}] gone - row pruned", _rows[i].pos.X(),
                     _rows[i].pos.Z());
            _rows.Delete(i);
            continue;
        }
        _rows[i].pos = obj->Position();
    }
}

// ---------------------------------------------------------------------------
// serialization
// ---------------------------------------------------------------------------

LSError StashRegistry::StashRow::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("pos", pos, 1, VZero))
    // the holder ref resolves on the second load pass (SerializeRef), after
    // the world's building serializer has recreated the WeaponHolder
    PARAM_CHECK(ar.SerializeRef("obj", obj, 1))
    return LSOK;
}

void StashRegistry::ApplyPendingLoad()
{
    for (int r = 0; r < _pending.Size(); r++)
    {
        const StashRow& row = _pending[r];
        if (!row.obj.GetLink())
        {
            // the holder rides the world's _buildings serializer, so a null
            // ref here means the object is genuinely gone - drop the row
            LOG_INFO(Core, "StashRegistry: stash at [{:.0f},{:.0f}] did not survive the load - row dropped",
                     row.pos.X(), row.pos.Z());
            continue;
        }
        _rows.Add(row);
    }
}

LSError StashRegistry::Serialize(ParamArchive& ar)
{
    if (ar.IsSaving())
    {
        _pending.Clear();
        for (int i = 0; i < _rows.Size(); i++)
        {
            _pending.Add(_rows[i]);
        }
    }

    PARAM_CHECK(ar.Serialize("Stashes", _pending, 1))

    if (ar.IsSaving())
    {
        _pending.Clear();
    }
    else if (ar.GetPass() == ParamArchive::PassSecond)
    {
        // obj refs were resolved by this pass's SerializeRef; adopt the rows
        // that came back with a live holder, drop the rest
        _rows.Clear();
        ApplyPendingLoad();
        _pending.Clear();
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
