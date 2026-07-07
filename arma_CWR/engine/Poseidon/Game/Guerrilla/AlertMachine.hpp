#pragma once

// Guerrilla Mode per-zone alert state machine - native replacement for the
// Phase-1 alert.sqs polling loop (guerrilla-mode/mission/Guerrilla.Demo/
// scripts/alert.sqs).  Each zone runs a GREEN/YELLOW/RED FSM driven by the
// occupier garrison's knowledge of the player (Target::FadingSideAccuracy,
// the knowsAbout value).  Escalation edges raise zone Heat; RED is the QRF
// signal - dispatch itself stays scripted, reacting to the "alertChanged"
// event.  The machine also owns the undercover BREAK check (vehicle mount
// natively, the fired-EH half via the gmBreakUndercover command); the
// gmUndercover global itself is script-owned.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;

namespace Poseidon
{
class ParamEntry;

namespace Guerrilla
{
class ZoneRegistry;

// Tunables; defaults are the GM_AL_* constants and knowsAbout bands from
// alert.sqs.  Config keys of the same names live in CfgGuerrillaZones.
struct AlertTuning
{
    float alertInterval = 5.0f;       // GM_AL_TICK: seconds between ticks
    float alertYellowKnows = 0.5f;    // knowsAbout floor of the YELLOW band
    float alertRedKnows = 1.5f;       // knowsAbout floor of the RED band
    float alertWindowSeconds = 20.0f; // GM_AL_WINDOW: YELLOW disengage countdown
    float alertHeatYellow = 4.0f;     // GM_AL_HEAT_YELLOW: Heat on entering YELLOW
    float alertHeatRed = 15.0f;       // GM_AL_HEAT_RED: Heat on entering RED
    float alertHeatBreak = 25.0f;     // GM_AL_HEAT_BREAK: Heat on a cover break
};

enum AlertState
{
    ASGreen = 0,
    ASYellow = 1,
    ASRed = 2
};

enum AlertEventType
{
    AEAlertChanged,     // _this = [zoneIndex, zoneName, oldState, newState]
    AEUndercoverBroken, // _this = [reason]
    NAlertEventTypes
};

struct AlertEventRecord
{
    AlertEventType type;
    int zoneIndex = -1; // alertChanged only
    int oldState = ASGreen;
    int newState = ASGreen;
    RString reason; // undercoverBroken only
};

// Per-zone observations for one tick.
struct AlertZoneInputs
{
    // max FadingSideAccuracy of the player across the zone's occupier groups
    float knows = 0;
    // best reported player position (engine axes); false when no group has one
    bool hasLastKnown = false;
    Vector3 lastKnown = VZero;
};

// Per-tick world observations.  The engine path fills this from the live
// world (GatherInputs); unit tests inject values directly.
struct AlertTickInputs
{
    bool playerValid = false;
    float playerX = 0;            // easting
    float playerZ = 0;            // northing
    bool undercover = false;      // script global gmUndercover (nil == false)
    bool playerInVehicle = false; // the (vehicle aP) != aP poll
    bool breakRequested = false;  // script called gmBreakUndercover
    RString breakReason;
    AutoArray<AlertZoneInputs> zones; // index-aligned to the zone registry
};

class AlertMachine
{
  public:
    AlertMachine() = default;

    // engine-wide instance (ZoneRegistry hooks and script commands)
    static AlertMachine& Instance();

    // lifecycle -------------------------------------------------------------
    void Clear(); // full reset, including event handlers
    // read alert* tunables from CfgGuerrillaZones; null resets to defaults
    void LoadFromParams(const ParamEntry* zonesCfg);
    const AlertTuning& Tuning() const { return _tuning; }

    // queries ---------------------------------------------------------------
    // GREEN when the index is out of range or the machine never ticked
    int GetZoneState(int index) const;
    // false (and pos untouched) when no position was ever recorded
    bool GetLastKnown(int index, Vector3& pos) const;
    // remaining YELLOW disengage countdown; 0 out of range (test aid)
    float GetZoneTimer(int index) const;

    // events ----------------------------------------------------------------
    void SetEventHandler(AlertEventType type, RString handler);
    RString GetEventHandler(AlertEventType type) const;
    static int EventTypeFromName(const char* name); // -1 when unknown

    // undercover break ------------------------------------------------------
    // script-side trigger for the fired-EH half (gmBreakUndercover);
    // consumed by the next tick
    void RequestBreak(RString reason);
    bool BreakLatched() const { return _breakLatched; }
    // pending gmBreakUndercover awaiting the next tick (serialized; test aid)
    bool BreakPending() const { return _breakPending; }
    RString BreakReason() const { return _breakReason; }

    // simulation ------------------------------------------------------------
    // per-frame engine hook (called from ZoneRegistry::Simulate);
    // internally throttled to alertInterval
    void Simulate(float deltaT);
    // pure tick logic: mutates alert state, raises Heat via the registry,
    // appends fired events (no world access)
    void EvaluateAlert(const AlertTickInputs& in, float dt, ZoneRegistry& registry, AutoArray<AlertEventRecord>& fired);

    // save/load; per-zone state keyed by zone name (called from inside
    // ZoneRegistry::Serialize as the nested "Alert" subclass)
    LSError Serialize(ParamArchive& ar, ZoneRegistry& registry);

  private:
    struct ZoneAlertState
    {
        int state = ASGreen;
        float timer = 0; // YELLOW disengage countdown (seconds)
        bool hasLastKnown = false;
        Vector3 lastKnown = VZero; // engine axes
    };

    // dynamic per-zone state as stored in a savegame
    struct AlertSaveState
    {
        RString name;
        int state = ASGreen;
        float timer = 0;
        bool hasLastKnown = false;
        Vector3 lastKnown = VZero;

        LSError Serialize(ParamArchive& ar);
    };

    void SyncZoneCount(int n);
    void GatherInputs(AlertTickInputs& in, const ZoneRegistry& registry) const;
    void DispatchEvents(const AutoArray<AlertEventRecord>& fired, const ZoneRegistry& registry);
    void ApplyPendingLoad(const ZoneRegistry& registry);

    AlertTuning _tuning;
    AutoArray<ZoneAlertState> _states; // index-aligned to the zone registry
    RString _handlers[NAlertEventTypes];
    float _accum = 0;
    bool _breakLatched = false; // undercoverBroken fired; resets when cover drops
    bool _breakPending = false; // gmBreakUndercover awaiting the next tick
    RString _breakReason;
    // deserialized rows waiting for the registry's zone table (applied on
    // the second load pass, after ZoneRegistry rebuilt from config)
    AutoArray<AlertSaveState> _pending;
};

} // namespace Guerrilla
} // namespace Poseidon
