#pragma once

// Guerrilla Mode traffic: the armed hold-up and the unarmed roadblock
// (issue #66).  A civilian driver who meets the real player pointing a
// weapon at his car rolls ONCE per encounter: surrender (stop, step out,
// flee on foot, the hull released to the player - the old commandeer) or
// escape (stay seated and floor it along the trip, recovering through the
// ordinary blocked-route ladder at full speed).  A player merely standing
// in the lane is a roadblock, not a threat: the car brakes, waits without
// a timeout and drives on once the lane has been clear for a moment.
//
// Types only: the encounter memory each traffic entry carries beside its
// route state, the action vocabulary the pure StepEncounter (Traffic.hpp)
// hands the world layer, and the deferred post-load restore.  Traffic.hpp
// includes this header; the logic lives in TrafficEncounter.cpp.

#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;

namespace Poseidon::Guerrilla
{

// the one roll of an encounter.  Saved as int: append-only (a unit test
// pins the values)
enum TrafficEncounterChoice
{
    EncNone,
    EncSurrender,
    EncEscape,
    NEncounterChoices
};

// the single order a StepEncounter result asks the world layer to execute
enum TrafficEncounterAction
{
    EActNone,
    EActBeginSurrender, // Stop once, state -> TSStopping
    EActSurrenderBail,  // delay elapsed: exit, flee, release (entry removed)
    EActBeginEscape,    // one SpeedFull route (ChooseEscapeLeg)
    EActBeginLaneWait,  // Stop once, laneWait on (TSDriving only)
    EActResumeLaneWait, // one route at normal speed, stall baseline refreshed
    EActDangerTakeover, // run the shared danger decision + reaction now
    EActClear,          // encounter over: call off the stop / restore normal speed
};

// where an escaping driver heads (BeginEscape)
enum TrafficEscapeLeg
{
    EscCurrent, // floor it along the leg it is on
    EscUTurn,   // at the destination with nowhere new to go: back home
    EscNewLeg,  // at the destination: on to another zone
    EscNone,    // no road out at all: the choice stands, no order
};

// the one order the first post-load sub-tick re-issues (never a roll)
enum TrafficLoadRestore
{
    TLRNone,
    TLRSurrenderStop, // the pending surrender's Stop
    TLREscapeRoute,   // the escape leg at SpeedFull
    TLRLaneStop,      // the roadblock wait's Stop
    TLRResumeDrive,   // a downgraded stop/cower: the leg at normal speed
};

// One observation for the pure encounter predicates (engine axes; only X/Z
// are used)
struct CommandeerObs
{
    Vector3 carPos = VZero;
    Vector3 carDir = VForward;
    Vector3 playerPos = VZero;
    Vector3 playerDir = VForward;
    bool weaponInHands = false;
};

// per-entry encounter memory (issue #66), independent of the route state.
// The archive key of each serialized field is in quotes
struct TrafficEncounter
{
    // the one roll ("encounter")
    TrafficEncounterChoice choice = EncNone;
    // danger took command this encounter; the choice's orders are
    // suppressed until it clears ("encounterDanger")
    bool dangerOwned = false;
    // unarmed lane obstruction: braked, waiting; TSDriving only ("laneWait")
    bool laneWait = false;
    // s the pending surrender has waited with the player inside the clear
    // radius ("surrenderTime")
    float surrenderTime = 0;
    // transient: s the player has been continuously outside; -1 = inside or
    // not yet seen outside
    float clearHold = -1;
    // transient: s the lane has been continuously clear during a lane wait;
    // -1 = obstructed
    float laneClear = -1;
    bool Active() const { return choice != EncNone || dangerOwned; }
    bool EscapeOwnsMovement() const { return choice == EncEscape && !dangerOwned; }
};

// flat keys inside the TrafficEntry archive class (called from
// TrafficEntry::Serialize); pre-#66 saves carry none of them and load "no
// encounter"
LSError SerializeEncounter(ParamArchive& ar, TrafficEncounter& enc);
// "none" / "surrender" / "escape" ("none" when out of range)
const char* EncounterChoiceName(int choice);

} // namespace Poseidon::Guerrilla
