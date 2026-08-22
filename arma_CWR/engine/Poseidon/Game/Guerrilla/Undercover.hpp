#pragma once

// Guerrilla Mode deep-undercover layer - per-observer side resolution for a
// captive subject trying to pass as a civilian.  setCaptive + the script
// global gmUndercover stay the baseline ("the subject is attempting to pass");
// this system layers per-observer overrides on top at the three side
// resolution sites inside EntityAI::TrackTargets (World/Detection/Target.cpp):
// a weapon visibly in hands reads hostile, a slung rifle reads hostile only
// from behind or inside conversational range, an unarmed subject stays a
// civilian unless THIS group previously identified him (Target::ucCompromised,
// per-group, serialized).  Vehicles resolve per observer too - a civilian car
// is anonymous, a stolen military vehicle is a disguise that fails at
// checkpoint range or to a group that witnessed the getaway.  Compromise
// notifications queue here and drain through the AlertMachine tick (event +
// Heat); the machine's old global vehicle-mount break is gone.
//
// Scope v1: the real player only, SP - everything self-disables when
// GWorld->GetRealPlayer() is null.  Inert unless the ZoneRegistry is active,
// gmUndercover is true and the subject is captive.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Time/Time.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;
class AICenter;
class AIGroup;

namespace Poseidon
{
class ParamEntry;
class EntityAI;
class Person;
class AIUnit;
class Man;
struct Target;

namespace Guerrilla
{

// Tunables; config keys of the same names live in CfgGuerrillaZones (the
// AlertTuning naming pattern).  undercoverHeatWitness is the one undercover*
// key NOT here - the AlertMachine is its only consumer, so it lives in
// AlertTuning next to the other Heat knobs.
struct UndercoverTuning
{
    // slung weapon / vehicle occupant recognized inside this radius with LOS
    float undercoverNoticeRadius = 20.0f;
    // cosFacing below this = the observer sees the target's back
    float undercoverBackArcCos = -0.2f;
    // side-accuracy multiplier while the weapon is visibly in hands
    float undercoverInHandsBoost = 2.0f;
    // side-accuracy multiplier for a slung weapon noticed from behind
    float undercoverSlungBoost = 1.4f;
    // boosted accuracy needed for a positive ID (the vanilla 1.5 threshold)
    float undercoverIdentifyAccuracy = 1.5f;
    // below this visibility the observation is ignored entirely
    float undercoverMinVisibility = 0.02f;
    // war-level scaling: warBoost = 1 + scale * warDetect (warDetect = gmWarLevel/10)
    float undercoverWarDetectScale = 0.5f;
    // per-group compromise decay in seconds; 0 (the default) = permanent
    float undercoverForgetSeconds = 0.0f;
    // getaway rule: a compromised person record seen this recently marks the
    // vehicle he boarded (the witnesses watched the getaway)
    float undercoverBoardWitnessSeconds = 10.0f;
};

// How the subject's weapon reads to an observer this tick.
enum UCWeaponShow
{
    UCWNone = 0,    // unarmed-looking (holstered pistol counts - concealed by design)
    UCWSlung = 1,   // long gun on the back
    UCWInHands = 2  // weapon visibly in hands
};

// Per-observation verdict.
enum UCVerdict
{
    UCCivil = 0,   // perceived TCivilian - the vanilla captive result
    UCSuspect = 1, // perceived TSideUnknown, checked - investigate, don't shoot
    UCExposed = 2  // perceived real side; the group's record turns compromised
};

// One observer->subject observation (pure rule input; the world adapter
// fills it from TrackTargets context, unit tests inject values directly).
struct UCObservation
{
    UCWeaponShow weapon = UCWNone;
    float dist2 = 0;     // observer-to-subject distance squared (m^2)
    float cosFacing = 1; // subject facing vs (observer - subject); negative = seen from behind
    float visibility = 0;
    float sideAccuracy = 0; // this tick's sensor side accuracy (pre-boost)
    bool alreadyCompromised = false; // this group's record is compromised
    float warDetect = 0;             // gmWarLevel / 10
};

// The person rule.  Returns the verdict and writes the (possibly boosted)
// side accuracy the caller should record.  Pure and world-free.
UCVerdict EvaluateUndercoverRule(const UCObservation& obs, const UndercoverTuning& tuning, float& outBoostedAccuracy);

// The weapon-show classifier behind UCWeaponShow: a ManPos partition - every
// raised-weapon pose reads in-hands, the patrol walk / binocular poses and
// the civilian moves with a long gun read slung (the back proxy still shows
// it), anything else unarmed.  Shared with the Traffic commandeer predicate
// ("weapon in hands aimed at the car").
UCWeaponShow ClassifyWeaponShow(const Man& man);

// Vehicle-type class as the rule consumes it (resolved from the type's
// _typicalSide by the adapter).
enum UCVehicleClass
{
    UCVCivilian = 0,         // _typicalSide == TCivilian: anonymous by policy
    UCVOccupierMilitary = 1, // the occupier's own military type: theft = disguise
    UCVOther = 2             // any other military type; handled like a theft
};

// One observer->subject-vehicle observation (pure rule input).
struct UCVehicleObservation
{
    UCVehicleClass vehicleClass = UCVCivilian;
    bool vehicleRecordCompromised = false; // this group remembers the vehicle
    bool personRecordCompromised = false;  // this group's record of the subject PERSON
    bool personLastSeenRecent = false;     // ... seen within undercoverBoardWitnessSeconds
    float dist2 = 0;
    float visibility = 0;
    float sideAccuracy = 0;
};

// The vehicle rule ("Vehicle policy" priority list).  UCExposed tells the
// caller to mark the vehicle record compromised.  Pure and world-free.
UCVerdict EvaluateUndercoverVehicleRule(const UCVehicleObservation& obs, const UndercoverTuning& tuning,
                                        float& outBoostedAccuracy);

// One queued witness notification: a group's record of the subject (or his
// vehicle) flipped false->true compromised.  Drained by the AlertMachine
// tick, which turns it into the undercoverBroken event / zone Heat.
struct UCCompromise
{
    Vector3 witnessPos = VZero; // engine axes
    RString reason;
    // campaign-first compromise (the AEUndercoverBroken edge); later
    // witnesses raise undercoverHeatWitness Heat only
    bool firstEver = false;

    LSError Serialize(ParamArchive& ar);
};

class UndercoverSystem
{
  public:
    UndercoverSystem() = default;

    // engine-wide instance (TrackTargets hooks and script commands)
    static UndercoverSystem& Instance();

    // lifecycle -------------------------------------------------------------
    void Clear(); // full reset (lives and dies with the ZoneRegistry)
    // read undercover* tunables from CfgGuerrillaZones; null resets to defaults
    void LoadFromParams(const ParamEntry* zonesCfg);
    const UndercoverTuning& Tuning() const { return _tuning; }

    // cache sync (called from AlertMachine::Simulate every frame) ------------
    // refreshes _active / subject / subject vehicle / occupier side /
    // warDetect; self-disables without a real captive undercover player
    void SyncCaches();

    // world-facing adapter (EntityAI::TrackTargets) --------------------------
    // slow half of the fast gate: subject/vehicle pointer compare + the
    // observer center's side against the occupier side
    bool AppliesTo(const AICenter* center, const EntityAI* ai) const;
    // per-observer verdict; writes target->side/sideChecked/sideAccuracy/
    // sideAccuracyTime and, on a false->true compromise edge, queues a
    // witness notification.  Routes to the vehicle rule when ai is the
    // subject's vehicle.
    void ResolvePerceivedSide(EntityAI* observer, AIUnit* observerUnit, EntityAI* ai, Target* target,
                              float sensorSideAccuracy, float dist2, float visibility);
    // record compromised, honoring the undercoverForgetSeconds decay knob
    bool IsCompromised(const Target& target) const;

    // compromise notifications ----------------------------------------------
    // move all queued notifications to out (queue is left empty)
    void ConsumeCompromises(AutoArray<UCCompromise>& out);
    // gmBreakUndercover semantics: mark the subject's (and his vehicle's)
    // record compromised in every occupier group currently holding a known
    // one; each newly marked group queues a notification
    void MarkAllWitnessesCompromised(RString reason);
    // clear every ucCompromised record world-wide plus the campaign
    // first-compromise latch (gmUndercoverForget; future disguise-swap hook)
    void ForgetAll();
    bool EverCompromised() const { return _everCompromised; }

    // script queries ---------------------------------------------------------
    // 0 clean / 1 suspected (an occupier group holds a known TSideUnknown
    // record of the subject) / 2 compromised (a live compromised record)
    int Status() const;
    // count of occupier groups holding a compromised record
    int WitnessCount() const;

    // save/load; called from inside ZoneRegistry::Serialize as the nested
    // "Undercover" subclass (next to "Alert")
    LSError Serialize(ParamArchive& ar);

  private:
    void ResolvePerceivedSideVehicle(EntityAI* observer, AIUnit* observerUnit, EntityAI* ai, Target* target,
                                     float sensorSideAccuracy, float dist2, float visibility);
    // shared verdict writer; queues the notification on the compromise edge
    void ApplyVerdict(UCVerdict verdict, float boostedAccuracy, EntityAI* observer, EntityAI* ai, Target* target,
                      const char* reason);
    void QueueCompromise(Vector3Par witnessPos, RString reason);
    // subject resolution for on-demand queries (cache, else the real player)
    Person* Subject() const;

    UndercoverTuning _tuning;
    bool _active = false;        // mirrored into GUndercoverActive
    LLink<Person> _subject;      // GWorld->GetRealPlayer while active
    LLink<EntityAI> _subjectVehicle; // vehicle the subject is aboard (null on foot)
    int _occupierSide = -1;      // TargetSide of ZoneRegistry::_occupierSide; -1 unresolved
    float _warDetect = 0;        // gmWarLevel / 10
    bool _everCompromised = false;    // campaign ever had a compromise (serialized)
    AutoArray<UCCompromise> _pending; // queued witness notifications (serialized)
};

// Fast-gate global for EntityAI::TrackTargets: true only while the
// undercover layer is live.  Mirrors UndercoverSystem::_active so the 99.9%
// case (ordinary missions, non-subject targets) costs one bool read.
extern bool GUndercoverActive;

// The TrackTargets fast gate: global bool first, then the pointer/side
// compares.  center is the OBSERVER's command center, ai the tracked entity.
inline bool UndercoverAppliesFast(const AICenter* center, const EntityAI* ai)
{
    if (!GUndercoverActive)
    {
        return false;
    }
    return UndercoverSystem::Instance().AppliesTo(center, ai);
}

} // namespace Guerrilla
} // namespace Poseidon
