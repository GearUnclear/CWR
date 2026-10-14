#pragma once

// Shared vocabulary between the occupier director (OccupierForces) and the
// movement service (CounterAttack): guerrilla-mode/OCCUPIER-FORCES.md
// sections 4 (ownership), 6.5 (packages), 6.6 (gmOccupierOps) and 14 (the
// CounterAttack additions: record kinds, Schedule with a fixed source,
// LaunchFunded, the AWAKE hook, Record fields).
//
// The director decides WHAT to send (kind, package, bodies, price) and hands
// CounterAttack a CounterAttackOrder through LaunchFunded.  CounterAttack
// moves the force (birth chain, march, hulls, resolution) and reports every
// change of state back through CounterAttackListener, so the director can
// refund hulls, start cooldowns, write its INFO lines and keep the ledger.
// Neither side reaches into the other's state.
//
// Names here are fixed by the orchestrator; both services include this file.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

namespace Poseidon
{
namespace Guerrilla
{
struct ZoneEventRecord;

// what an operation is for (section 4, section 14 item 1)
enum CounterAttackKind
{
    CAKRetake,    // a held town (the legacy flip answer and director retakes)
    CAKBase,      // a resistance-held military zone (D16, D19; phase 4)
    CAKHqAssault, // the player's HQ at HqPos (D13, D14, D24; phase 4)
    CAKAlarm,     // an answer to a RED occupier zone (D4; phase 6)
    CAKReinforce, // a top-up column to a bled base (phase 5)
    CAKStrike,    // a jet or attack-helicopter sortie (D17; phase 6)
    NCounterAttackKinds
};

// who ordered it (gmOccupierOps column `origin`)
enum CounterAttackOrigin
{
    CAOLegacy,   // Decide / OnTownFlipped, the 4400fe9 dispatch (director DORMANT or key off)
    CAODirector, // a director token on a heat candidate
    CAOPushout,  // the quiet-town push-out (6.4)
    CAOWave,     // a reinforcement wave strike (7.1; phase 4)
    CAOAlarm,    // an alarm answer (8.2; phase 6)
    NCounterAttackOrigins
};

// the package catalogue (6.5).  Foot is always offered; every other entry
// is offered only when the faction's ladder or a descriptor key names a hull.
enum ForcePackage
{
    FPFoot,
    FPTruck,
    FPArmedCar,
    FPApc,
    FPTank,
    FPTransportHeli,
    FPAttackHeli,
    FPJet,
    NForcePackages
};

// one hull the catalogue offers for a package
struct ForceHull
{
    RString className;   // config class; empty = the package is not offered
    float maxSpeedKmh = 0; // config maxSpeed (km/h); the abstract leg runs at 0.6 x this (0.8 for air)
    int cargo = 0;         // transportSoldier
    int crew = 0;          // crew bodies the hull needs (driver, gunner, commander, pilots)
};

struct ForcePackageCatalogue
{
    ForceHull hulls[NForcePackages];
    bool Offered(int package) const
    {
        return package == FPFoot || (package >= 0 && package < NForcePackages && hulls[package].className.GetLength() > 0);
    }
};

// the director's order to CounterAttack::LaunchFunded
struct CounterAttackOrder
{
    int opId = 0;            // the director's operation id (gmOccupierOps)
    int kind = CAKRetake;    // CounterAttackKind
    int origin = CAODirector; // CounterAttackOrigin
    int package = FPFoot;    // ForcePackage
    int target = -1;         // zone index (CAKHqAssault: the zone the HQ stands in, or -1 with hqPos set)
    int source = -1;         // zone index of the base that supplies the infantry; -1 = off-map (air, phase 6)
    int bodies = 0;          // infantry bodies, crew excluded (drawn from the source's spare)
    int crew = 0;            // hull crew bodies (paid by the pool as part of priceUnits)
    RString hullClass;       // empty for foot
    int priceUnits = 0;      // what the director already debited from the pool (hull + crew); refunded per 5.5 through events
    bool urgent = false;     // the urgency flag at launch (fastest package rule)
    float abstractSpeed = 0; // m/s for the abstract leg at real speed (3.2); 0 = legacy counterAttackSpeed pacing
    bool funded = false;     // true for a director operation with the key on (pool routing applies)
    // appended by CounterAttack (phase 3 part B) ---------------------------
    float lethality = 0;     // A_lethal per infantry body (6.5, item 12); 0 = 1.0.  The legacy Fight's odds use
                             // bodies x this, its losses the bodies.
    // appended by phase 4 part A (OCCUPIER-FORCES.md 9.1) ------------------
    // explicit march destination: when non-zero it replaces the target
    // zone's centre as the place the force marches to and resolves at.  Set
    // for CAKHqAssault (GuerrillaBase::HqPos()); VZero for every other kind.
    Vector3 destPos = VZero;
};

// what CounterAttack reports back (one event per state change)
enum CounterAttackOpEventType
{
    CAOpLaunched,        // the record exists; bodies left the source (or were debited)
    CAOpBorn,            // the live squad (and hull) exists in the world; `note` = spawn reason
    CAOpResourced,       // the birth chain moved the operation to the next base back (source = the new base)
    CAOpCancelledUnseen, // nothing could be born: cancelled while abstract (bodies returned, hull price to refund)
    CAOpArrived,         // at the town edge (abstract or live)
    CAOpRepelled,        // the force lost: bodies = survivors going home, units = bodies lost
    CAOpSettled,         // the target reverted / fell: bodies became its garrison
    CAOpReturned,        // survivors wrote back to a base (bodies) or had no base (units to the pool)
    CAOpHullRefund,      // a hull deleted at home intact: units = its price
    CAOpHullLost,        // a hull destroyed (or deleted on a return to abstract travel, no refund): units = its price
    CAOpAborted,         // any other end (timeout, source lost, won/lost freeze)
    NCounterAttackOpEventTypes
};

struct CounterAttackOpEvent
{
    int opId = 0;
    int type = CAOpLaunched; // CounterAttackOpEventType
    int kind = CAKRetake;
    int origin = CAOLegacy;
    int target = -1;
    int source = -1;
    int bodies = 0;          // infantry bodies the event is about
    int units = 0;           // pool units the event is about (hull prices, losses)
    const char* note = "";   // free text for the INFO line (never stored)
};

// The director's ear.  CounterAttack calls it from its own tick; the listener
// must not re-enter CounterAttack synchronously (queue instead).
class CounterAttackListener
{
  public:
    virtual ~CounterAttackListener() = default;
    // a non-exempt CITY just flipped to the resistance.  Return true when the
    // director takes the flip (AWAKE, key on): CounterAttack then schedules
    // no legacy answer.  k is the legacy counter (towns taken before).
    virtual bool OnTownFlipped(int zoneIndex, int k) = 0;
    // every state change of an operation
    virtual void OnCounterAttackOp(const CounterAttackOpEvent& ev) = 0;
    // the registry's events of one zone tick, forwarded after CounterAttack
    // handled them (flips, town losses, base losses)
    virtual void OnZoneEvents(const AutoArray<ZoneEventRecord>& fired) = 0;
};

} // namespace Guerrilla
} // namespace Poseidon
