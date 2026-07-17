#pragma once

// Guerrilla Mode arms stashes - registry of WeaponHolder-style supply objects
// flagged keep-when-empty (gmStashRegister) so emptying them does not trigger
// the forceSupply self-delete.  The holders themselves ride the world's
// building serializer (including the keepWhenEmpty flag inside their
// ResourceSupply); rows here persist the tracking set (object ref + pos).
// Unlike the other Guerrilla subsystems this one is NOT gated on the
// ZoneRegistry being active - stashes work in ordinary missions too.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp> // LLink == OLink (NetworkObject.hpp)
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;

namespace Poseidon
{
class EntityAI;

namespace Guerrilla
{

class StashRegistry : public SerializeClass
{
  public:
    StashRegistry() = default;

    // engine-wide instance (used by the World hooks and script commands)
    static StashRegistry& Instance();

    // lifecycle -----------------------------------------------------------
    void Clear();       // drops rows only, never objects
    void InitMission(); // Clear; no config of its own

    // bookkeeping (the keep-when-empty flag itself is set by the command
    // layer on the VehicleSupply - this class never touches it) ------------
    bool Register(EntityAI* obj);     // idempotent; false on null
    bool Unregister(EntityAI* obj);   // false when not registered
    int Count() const;                // live rows
    EntityAI* GetObject(int i) const; // null out of range / dead link
    Vector3 GetPos(int i) const;      // VZero out of range
    // test aid: a row with a null object link, so save/load pruning is
    // unit-testable without a live world (cf. TownFlags::MarkPlacedForTest)
    void AddRowForTest(Vector3Par pos);

    // tuning
    static constexpr float TickInterval = 5.0f; // s between prune passes

    // simulation ------------------------------------------------------------
    // per-frame engine hook; internally throttled to TickInterval;
    // prunes dead/deleted holders, refreshes row positions
    void Simulate(float deltaT);

    // save/load; the holder objects (and their keepWhenEmpty flag) ride the
    // world's building serializer - rows here are pure tracking
    LSError Serialize(ParamArchive& ar) override;

  private:
    struct StashRow
    {
        // OLink semantics: a deleted holder reads back as null -> row dropped
        LLink<EntityAI> obj;
        Vector3 pos = VZero; // last known position (map/query aid)

        LSError Serialize(ParamArchive& ar);
    };

    int FindRow(const EntityAI* obj) const;
    void ApplyPendingLoad(); // second pass: adopt rows, drop null refs

    AutoArray<StashRow> _rows;
    // deserialized rows waiting for resolved object refs (second load pass)
    AutoArray<StashRow> _pending;
    float _accum = 0;
};

} // namespace Guerrilla
} // namespace Poseidon
