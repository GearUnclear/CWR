#pragma once

// Guerrilla Mode per-zone alert state machine - native replacement for the
// Phase-1 alert.sqs polling loop (guerrilla-mode/mission/Guerrilla.Demo/
// scripts/alert.sqs).  Each zone runs a GREEN/YELLOW/RED FSM driven by the
// occupier garrison's knowledge of the player (Target::FadingSideAccuracy,
// the knowsAbout value).  Escalation edges raise zone Heat; RED is the QRF
// signal - dispatch itself stays scripted, reacting to the "alertChanged"
// event.  The machine also drains the UndercoverSystem's per-observer
// compromise notifications each tick: the campaign-first compromise fires
// the "undercoverBroken" event plus alertHeatBreak on the zone nearest the
// witness, later witness groups add undercoverHeatWitness Heat quietly.
// gmBreakUndercover (the fired-EH half) marks every occupier group that
// currently knows the subject as compromised; every request does, the
// per-group marking is idempotent (issue #19 removed the once-per-campaign
// latch).  The old global vehicle-mount break is gone - vehicles resolve
// per observer (see Undercover.hpp).  The gmUndercover global itself is
// script-owned.  The tick also drains the Traffic service's violent
// patrol/convoy ends (TrafficAmbush): each floors the attributed zone's
// acted-on knowledge for trafficAmbushWindow seconds - a patrol wipe holds
// YELLOW (the disengage window only bleeds under a live contact), a convoy
// wipe pins RED - so a wiped road patrol alerts through the ordinary FSM
// (and qrf.sqs) with no script changes.

#include <Poseidon/Game/Guerrilla/Traffic.hpp>    // TrafficAmbush (tick inputs)
#include <Poseidon/Game/Guerrilla/Undercover.hpp> // UCCompromise (tick inputs)

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
    float alertHeatBreak = 25.0f;     // GM_AL_HEAT_BREAK: Heat on the campaign-first compromise
    // Heat when a LATER witness group turns compromised (no event); the one
    // undercover* config key living here - the machine is its only consumer
    float undercoverHeatWitness = 8.0f;
    // wiped patrol/convoy stimulus (TrafficAmbush drain).  The traffic*
    // keys living here - the machine is their only consumer:
    // seconds the injected knowsAbout floor persists (a patrol wipe floors
    // the YELLOW band, a convoy wipe the RED band - the alert*Knows above)
    float trafficAmbushWindow = 120.0f;
    // attribution bound (m): the wreck alerts the nearest occupier zone
    // inside this.  Deliberately NOT TrafficTuning::radius - that is the
    // ambient-density player band, and alert reach must not move with it
    float trafficAmbushRadius = 1500.0f;
    // Heat per drained ambush on the attributed zone, on top of the FSM
    // edge spikes - repeat ambushes inside a held window still cost (the
    // compromise-drain policy); one kind-independent amount, the convoy's
    // RED edge already pays alertHeatRed
    float trafficAmbushHeat = 4.0f;
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
    float playerX = 0;           // easting
    float playerZ = 0;           // northing
    bool undercover = false;     // script global gmUndercover (nil == false)
    bool breakRequested = false; // script called gmBreakUndercover
    RString breakReason;
    // per-observer compromise notifications drained from the UndercoverSystem
    AutoArray<UCCompromise> compromises;
    // violent patrol/convoy ends drained from the Traffic service
    AutoArray<TrafficAmbush> ambushes;
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
    // the fired-EH mark gate exactly as Simulate applies it to one tick's
    // inputs.  Stateless since issue #19: every break request while cover
    // is up marks witnesses (idempotent per group); the old campaign latch
    // armed here and never re-armed under the keep-cover lifecycle
    bool BreakShouldMark(const AlertTickInputs& in) const { return in.undercover && in.breakRequested; }
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
        // wiped-traffic stimulus: the FSM acts on max(gathered knows,
        // stimulusKnows) while the window timer runs, then decays to the
        // gathered value (never force zone state - the FSM recomputes it
        // from knowledge every tick and would revert a forced write)
        float stimulusKnows = 0;
        float stimulusTimer = 0; // seconds of window left
    };

    // dynamic per-zone state as stored in a savegame
    struct AlertSaveState
    {
        RString name;
        int state = ASGreen;
        float timer = 0;
        bool hasLastKnown = false;
        Vector3 lastKnown = VZero;
        float stimulusKnows = 0;
        float stimulusTimer = 0;

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
    bool _breakPending = false; // gmBreakUndercover awaiting the next tick
    RString _breakReason;
    // deserialized rows waiting for the registry's zone table (applied on
    // the second load pass, after ZoneRegistry rebuilt from config)
    AutoArray<AlertSaveState> _pending;
};

} // namespace Guerrilla
} // namespace Poseidon
