#include <Poseidon/Game/Guerrilla/Journal.hpp>

#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/Core/Global.hpp> // Glob.clock
#include <Poseidon/World/World.hpp> // GWorld (GameState for gmDayCount)
#include <Evaluator/express.hpp>    // GameState / GameValue (VarGet)

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <cstdio>
#include <cstring>

namespace Poseidon::Guerrilla
{

// Defined in JournalCommands.cpp.  Referencing it from here forces the
// command TU (whose only other content is an INIT_MODULE registration) into
// the link - same pattern as EnsureStashRegistryCommandsLinked.
void EnsureJournalCommandsLinked();

// Process-lifetime singleton - no global constructor (see express.hpp's
// GGameState for the convention).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
Journal& Journal::Instance()
{
    EnsureJournalCommandsLinked();
    static Journal instance;
    return instance;
}
#pragma clang diagnostic pop

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------

void Journal::Clear()
{
    _entries.Clear();
    _objectives.Clear();
    _status.Clear();
    _revision++;
}

void Journal::InitMission()
{
    Clear();
}

// ---------------------------------------------------------------------------
// diary
// ---------------------------------------------------------------------------

void Journal::AddEntry(RString stamp, RString text)
{
    if (text.GetLength() == 0)
    {
        return;
    }
    JournalEntry entry;
    entry.stamp = stamp;
    entry.text = text;
    _entries.Add(entry);
    while (_entries.Size() > MaxEntries)
    {
        _entries.Delete(0);
    }
    _revision++;
    LOG_INFO(Core, "Journal: [{}] {}", (const char*)stamp, (const char*)text);
}

// ---------------------------------------------------------------------------
// objectives
// ---------------------------------------------------------------------------

int Journal::FindObjective(const char* id) const
{
    if (!id)
    {
        return -1;
    }
    for (int i = 0; i < _objectives.Size(); i++)
    {
        if (stricmp(_objectives[i].id, id) == 0)
        {
            return i;
        }
    }
    return -1;
}

void Journal::SetObjective(RString id, RString text, int state)
{
    if (id.GetLength() == 0)
    {
        return;
    }
    if (state < JOActive || state > JOHidden)
    {
        state = JOActive;
    }
    int i = FindObjective(id);
    if (i < 0)
    {
        if (text.GetLength() == 0)
        {
            return; // a state-only write on an unknown row is a no-op
        }
        JournalObjective row;
        row.id = id;
        row.text = text;
        row.state = state;
        _objectives.Add(row);
        _revision++;
        return;
    }
    JournalObjective& row = _objectives[i];
    if (text.GetLength() > 0)
    {
        row.text = text;
    }
    row.state = state;
    _revision++;
}

int Journal::ObjectiveStateFromName(const char* name)
{
    if (!name)
    {
        return -1;
    }
    if (stricmp(name, "ACTIVE") == 0)
    {
        return JOActive;
    }
    if (stricmp(name, "DONE") == 0)
    {
        return JODone;
    }
    if (stricmp(name, "FAILED") == 0)
    {
        return JOFailed;
    }
    if (stricmp(name, "HIDDEN") == 0)
    {
        return JOHidden;
    }
    return -1;
}

const char* Journal::ObjectiveStateName(int state)
{
    switch (state)
    {
        case JODone:
            return "DONE";
        case JOFailed:
            return "FAILED";
        case JOHidden:
            return "HIDDEN";
        default:
            return "ACTIVE";
    }
}

// ---------------------------------------------------------------------------
// status lines
// ---------------------------------------------------------------------------

int Journal::FindStatus(const char* key) const
{
    if (!key)
    {
        return -1;
    }
    for (int i = 0; i < _status.Size(); i++)
    {
        if (stricmp(_status[i].key, key) == 0)
        {
            return i;
        }
    }
    return -1;
}

void Journal::SetStatus(RString key, RString text)
{
    if (key.GetLength() == 0)
    {
        return;
    }
    int i = FindStatus(key);
    if (text.GetLength() == 0)
    {
        if (i >= 0)
        {
            _status.Delete(i);
            _revision++;
        }
        return;
    }
    if (i < 0)
    {
        JournalStatusLine row;
        row.key = key;
        row.text = text;
        _status.Add(row);
    }
    else
    {
        _status[i].text = text;
    }
    _revision++;
}

// ---------------------------------------------------------------------------
// serialization
// ---------------------------------------------------------------------------

LSError JournalEntry::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("stamp", stamp, 1, RString()))
    PARAM_CHECK(ar.Serialize("text", text, 1, RString()))
    return LSOK;
}

LSError JournalObjective::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("id", id, 1, RString()))
    PARAM_CHECK(ar.Serialize("text", text, 1, RString()))
    PARAM_CHECK(ar.Serialize("state", state, 1, (int)JOActive))
    return LSOK;
}

LSError JournalStatusLine::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("key", key, 1, RString()))
    PARAM_CHECK(ar.Serialize("text", text, 1, RString()))
    return LSOK;
}

LSError Journal::Serialize(ParamArchive& ar)
{
    // plain values only - one pass is enough, but the World serializer
    // calls us on both load passes; only touch state on the first
    if (ar.IsLoading() && ar.GetPass() != ParamArchive::PassFirst)
    {
        return LSOK;
    }
    PARAM_CHECK(ar.Serialize("Entries", _entries, 1))
    PARAM_CHECK(ar.Serialize("Objectives", _objectives, 1))
    PARAM_CHECK(ar.Serialize("Status", _status, 1))
    if (ar.IsLoading())
    {
        // drop rows a hand-edited or truncated save left empty
        for (int i = _entries.Size() - 1; i >= 0; i--)
        {
            if (_entries[i].text.GetLength() == 0)
            {
                _entries.Delete(i);
            }
        }
        for (int i = _objectives.Size() - 1; i >= 0; i--)
        {
            if (_objectives[i].id.GetLength() == 0)
            {
                _objectives.Delete(i);
            }
        }
        for (int i = _status.Size() - 1; i >= 0; i--)
        {
            if (_status[i].key.GetLength() == 0)
            {
                _status.Delete(i);
            }
        }
        _revision++;
    }
    return LSOK;
}

// ---------------------------------------------------------------------------
// time stamp
// ---------------------------------------------------------------------------

RString JournalStampNow()
{
    if (!GWorld)
    {
        return RString();
    }
    int day = 1;
    GameState* gstate = GWorld->GetGameState();
    if (gstate)
    {
        // shakedown.sqs bumps gmDayCount on every midnight wrap (0 at boot)
        GameValue value = gstate->VarGet("gmdaycount");
        if (value.GetType() == GameScalar)
        {
            day = toInt((float)value) + 1;
        }
    }
    float hours = Glob.clock.GetTimeOfDay() * 24.0f;
    int hh = (int)hours; // truncate, never round up past the hour
    int mm = (int)((hours - (float)hh) * 60.0f);
    if (hh < 0 || hh > 23)
    {
        hh = 0;
    }
    if (mm < 0 || mm > 59)
    {
        mm = 0;
    }
    char buffer[48];
    snprintf(buffer, sizeof(buffer), "Day %d %02d:%02d", day, hh, mm);
    return RString(buffer);
}

} // namespace Poseidon::Guerrilla
