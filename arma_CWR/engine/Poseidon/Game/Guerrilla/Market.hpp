#pragma once

// Guerrilla Mode market (issue #27) - the money sink outside recruiting:
// WEAPON dealers and VEHICLE dealers seeded across the island's CITY zones.
// At the first tick of a fresh campaign each kind is assigned to roughly a
// third of the cities (dealerShare), drawn independently so one town may
// host both; the draw is seeded once and serialized, so a reload keeps the
// same towns.  Each dealer is a CIV-side NPC standing on a deterministic
// off-road spot inside the town (weapon dealers on the cardinal bearings,
// vehicle dealers on the diagonals, so a town hosting both keeps them
// apart); a vehicle dealer also owns a LOT spot nearby where a bought hull
// is dropped.  A killed dealer respawns after dealerRespawnSeconds.
//
// Stock comes from class CfgGuerrillaMarket in the mission's description.ext
// (island data, like CfgGuerrillaZones): class Weapons { class X { weapon=;
// magazine=; mags=; price=; }; } and class Vehicles { class Y { vehicle=;
// price=; }; }.  A row may be a weapon with its magazines, a weapon alone
// (magazine="") or a magazine bundle alone (weapon="").  Classnames are
// resolved against the loaded data package at load (plan-15 posture):
// unresolvable rows are dropped with a log line, never a fatal spawn.
// Display names resolve from the package config's displayName (localized)
// on the engine path only, so the parser stays unit-testable.
//
// The engine owns the data and the NPCs; the purchase itself - the
// gmResources debit, the WeaponHolder / createVehicle, the delivery to the
// headquarters cache or garage - is the script layer's (scripts/market.sqs),
// the second "-" writer of gmResources next to recruit.sqs.  Active only
// when the class exists AND the ZoneRegistry is active.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp> // LLink == OLink (NetworkObject.hpp)
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;
namespace Poseidon { class AIGroup; } // a Poseidon type: a global forward declaration collides with the using-declaration in Core/Types.hpp on Linux clang

namespace Poseidon
{
class ParamEntry;
class EntityAI;

namespace Guerrilla
{
struct ClassProbe;

// keys inside class CfgGuerrillaMarket
struct MarketTuning
{
    float dealerShare = 0.34f;           // dealerShare: fraction of cities per kind
    float dealerRespawnSeconds = 600.0f; // dealerRespawnSeconds
    float hqMoveCost = 500.0f;           // hqMoveCost: R debited by the script when moving the HQ
    RString dealerClass;                 // dealerClass, else the CIV descriptor's civClass1, else "Civilian"
};

enum DealerKind
{
    DKWeapon = 0,
    DKVehicle = 1,
    NDealerKinds
};

struct MarketWeaponRow
{
    RString name;     // config subclass name (stable id)
    RString weapon;   // CfgWeapons class, "" for a magazine-only bundle
    RString magazine; // CfgWeapons/CfgMagazines class, "" for weapon-only
    int mags = 0;     // magazines included
    float price = 0;  // R
    RString displayName;
};

struct MarketVehicleRow
{
    RString name;
    RString vehicle; // CfgVehicles class
    float price = 0;
    RString displayName;
};

// one CITY as the pure planner sees it
struct DealerCity
{
    RString name;
    Vector3 pos = VZero;
};

struct DealerRecord
{
    RString zoneName;
    int kind = DKWeapon;
    Vector3 pos = VZero;    // the dealer's spot (engine axes)
    Vector3 lotPos = VZero; // vehicle delivery spot (== pos for weapon dealers)
    bool placed = false;    // pos/lotPos computed (serialized)
    bool spawned = false;   // the NPC has been created at least once (serialized)
    float respawnIn = -1;   // s until respawn while the NPC is dead; <0 idle
    // OLink semantics: a deleted NPC reads back as null -> respawn
    LLink<EntityAI> npc;
    // transient
    int zoneIndex = -1;
    bool warned = false;

    LSError Serialize(ParamArchive& ar);
};

class Market : public SerializeClass
{
  public:
    Market() = default;

    // engine-wide instance (used by the World hooks and script commands)
    static Market& Instance();

    // lifecycle -----------------------------------------------------------
    void Clear();       // full reset (drops refs only, never objects)
    void InitMission(); // Clear + LoadFromConfig; call at mission start
    void LoadFromConfig();
    // testable core: marketCfg null leaves the market unconfigured;
    // factionsCfg supplies the CIV descriptor (dealer body fallback);
    // probe, when non-null, resolves every classname against the package
    void LoadFromParams(const ParamEntry* marketCfg, const ParamEntry* factionsCfg, const ClassProbe* probe);

    // queries ---------------------------------------------------------------
    bool IsConfigured() const { return _configured; }
    bool IsActive() const; // configured AND ZoneRegistry active
    bool IsAssigned() const { return _assigned; }
    const MarketTuning& Tuning() const { return _tuning; }
    float Value(const char* key) const; // dealerShare|dealerRespawnSeconds|hqMoveCost, 0 unknown
    int NWeapons() const { return _weapons.Size(); }
    const MarketWeaponRow& Weapon(int i) const { return _weapons[i]; }
    int NVehicles() const { return _vehicles.Size(); }
    const MarketVehicleRow& Vehicle(int i) const { return _vehicles[i]; }
    int NAuthoredDealers(int kind) const;
    RString AuthoredDealer(int kind, int i) const;
    int DealerCount() const { return _rows.Size(); }
    const DealerRecord* Dealer(int i) const;           // null out of range
    int NearestDealer(int kind, Vector3Par pos) const; // -1 when none of the kind

    // pure logic (unit-tested; no world access) ------------------------------
    // 0 cities -> 0; otherwise at least one, else round(n * share)
    static int DealerQuota(int nCities, float share);
    // deterministic order: cities sorted by a per-city hash of seed ^ salt ^
    // pos, stable by index; different salts give independent orders
    static void ShuffleOrder(int seed, int salt, const AutoArray<Vector3>& cityPos, AutoArray<int>& order);
    // the assignment: authored lists (zone names, matched case-insensitively
    // against the cities) override the draw for their kind; rows carry the
    // city position as pos (the world layer refines it)
    static void PlanDealers(const AutoArray<DealerCity>& cities, int seed, float share,
                            const AutoArray<RString>& authoredWeapon, const AutoArray<RString>& authoredVehicle,
                            AutoArray<DealerRecord>& out);
    static const char* KindName(int kind);     // "WEAPON" / "VEHICLE"
    static int KindFromName(const char* name); // -1 unknown
    static constexpr int SaltWeapon = 0x5741;
    static constexpr int SaltVehicle = 0x5645;

    // tuning constants (engine facts, not island data)
    static constexpr float TickInterval = 3.0f;  // s between dealer passes
    static constexpr float RoadClearance = 6.0f; // m, GRoadNet::IsOnRoad probe size
    static constexpr float LotMinDist = 15.0f;   // m, lot spot away from the dealer

    // test aid: run the pure assignment over an injected city list (no
    // world); rows come back unplaced/unspawned, _assigned/_seed set
    void AssignForTest(const AutoArray<DealerCity>& cities, int seed);

    // simulation ------------------------------------------------------------
    // per-frame engine hook; internally throttled to TickInterval; assigns
    // lazily on the first tick, spawns/respawns the NPCs
    void Simulate(float deltaT);

    // save/load; dealer rows keyed by zone name (+ NPC refs), the seed and
    // the assigned flag; the NPCs ride the world's vehicle serializer
    LSError Serialize(ParamArchive& ar) override;

  private:
    void ResolveDisplayNames();    // engine path: Pars displayName lookups
    void DropUncreatableClasses(); // engine path: abstract (scope<=0) vehicle rows / dealer body
    void Assign();              // world: CITY zones -> PlanDealers
    bool ComputeDealerSpot(DealerRecord& row) const;
    void SpawnDealer(DealerRecord& row);
    AIGroup* DealerGroup(int kind);
    void ApplyPendingLoad();

    MarketTuning _tuning;
    bool _configured = false;
    // config read for the current mission (cleared by Clear; a save written
    // without our block leaves it false and the first tick reloads)
    bool _configLoaded = false;
    AutoArray<MarketWeaponRow> _weapons;
    AutoArray<MarketVehicleRow> _vehicles;
    AutoArray<RString> _authoredDealers[NDealerKinds];
    AutoArray<DealerRecord> _rows;
    bool _assigned = false;
    int _seed = 0;
    LLinkArray<AIGroup> _groups[NDealerKinds];
    float _accum = 0;
    // deserialized rows waiting for the rebuilt zone table (second load pass)
    AutoArray<DealerRecord> _pending;
};

} // namespace Guerrilla
} // namespace Poseidon
