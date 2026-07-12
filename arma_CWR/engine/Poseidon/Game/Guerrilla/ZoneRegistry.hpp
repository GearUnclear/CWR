#pragma once

// Guerrilla Mode zone registry - native replacement for the Phase-1
// zones.sqs polling loop (guerrilla-mode/mission/Guerrilla.Demo/scripts).
// Zone and faction data come from mission config (class CfgGuerrillaZones /
// class CfgGuerrillaFactions in description.ext, with the global config as
// fallback).  Without config the registry is inactive: zero zones, Simulate
// is a no-op and the gm* script commands return empty defaults, so ordinary
// missions are unaffected.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

class ParamArchive;
class AICenter;

namespace Poseidon
{
class ParamEntry;

namespace Guerrilla
{

struct ZoneTuning
{
    float tickInterval = 3.0f; // s between zone ticks
    float zoneArea = 150.0f;   // capture / presence radius (m)
    float revealRadius = 1500.0f;
    float cacheRadius = 800.0f;  // player-distance gate for capture math
    float supportRate = 5.0f;    // CITY support gained per tick while GUER present
    float supportFlip = 60.0f;   // CITY support threshold for NEUTRAL -> GUER
    float heatCapSpike = 40.0f;  // Heat added on capture (clamped to 100)
    float defaultIncome = 25.0f; // income tap opened on military capture
    float holdCount = 3.0f;      // friendly hold-garrison size (spawn stays script-side)
    // military consolidation meter (ZoneRecord::capture, 0..100)
    float captureRate = 6.0f;           // progress/tick per effective attacker (100 = legacy instant flip)
    float captureCrewCap = 3.0f;        // attacker-count multiplier cap (matches holdCount)
    float captureDecayDefended = 10.0f; // decay/tick while only defenders hold the zone
    float captureDecayAbandoned = 2.0f; // decay/tick while nobody holds the zone
    // CITY intimidation: occupier-only presence bleeds support, never below the floor
    float supportDecayOccupied = 0.5f;
    float supportDecayFloor = 20.0f;
    // defenders >= ratio * attackers treats a contested zone as DEFENDED
    // (a token straggler cannot hold a re-secured post hostage); 0 disables.
    // Any live defender still blocks SECURING regardless of this ratio.
    float contestOutnumberRatio = 4.0f;
    // CITY auto-seed from the world's CfgWorlds >> <world> >> Names entries
    bool seedCities = false;
    float seedCitySupport = 20.0f; // starting support of each seeded CITY
};

struct ZoneRecord
{
    // static, from config
    RString name;   // unique id
    RString type;   // "CAMP" | "AIRFIELD" | "SEAPORT" | "OUTPOST" | "CITY"
    RString marker; // map marker owned by the zone (may be absent from markersMap)
    // COORDINATE CONTRACT: config position[] is authored in script/getPos
    // order [easting, northing, elevation]; engine axes are X=easting,
    // Y=elevation, Z=northing.  Script-facing arrays keep getPos order.
    Vector3 pos = VZero;

    // raw config owner, kept verbatim: a literal side string or one of the
    // generic tokens "OCCUPIER" / "RESISTANCE" (re-resolved when the
    // campaign's faction sides change, e.g. on savegame load)
    RString ownerConfig;
    // per-zone captureRate override (0 = use tuning); static, config-rebuilt
    float captureRateOverride = 0;

    // dynamic, serialized (matched by zone name on load)
    RString owner;           // concrete side: resistance / occupier / "NEUTRAL" / ...
    float garrison = 0;      // occupier body count while despawned
    float support = 0;       // 0..100, CITY flips on this
    float income = 0;        // per economy tick when GUER-owned
    float heat = 0;          // 0..100 per-zone Heat
    float liveOccupiers = 0; // live-count mirror written natively by GarrisonCache
                             // (script-overridable via gmZoneSet); informational
                             // only - NOT part of the capture predicate
    float capture = 0;       // 0..100 military consolidation meter
    bool revealed = false;   // fog-of-war state

    // transient tick bookkeeping, never serialized
    bool contestedLastTick = false;    // edge detection for the contested event/marker
    bool supportReadyNotified = false; // supportThreshold fired for the current crossing
};

// One occupier or resistance faction entry (a subclass of
// CfgGuerrillaFactions).  Non-array text entries (officer, holdClass, ...)
// are kept generically and queried by key.
struct FactionRecord
{
    struct NamedValue
    {
        RString key;
        RString value;
    };

    RString side;      // "EAST" / "GUER" / ...
    RString className; // config subclass name (second lookup key after side)
    AutoArray<RString> tiers;
    AutoArray<float> tierThresholds; // ascending war levels; tier i+1 from thresholds[i]
    AutoArray<RString> vehicles;
    float vehicleThreshold = 3.0f;      // legacy 2-step ladder (index 0 -> 1)
    AutoArray<float> vehicleThresholds; // full ladder, mirrors tierThresholds
    AutoArray<NamedValue> values;
};

enum ZoneEventType
{
    ZECaptured,         // any owner flip
    ZESupportThreshold, // CITY crossed supportFlip (the town is READY; flip is separate)
    ZERevealed,         // zone first became revealed
    ZECampaignLoaded,   // savegame finished loading (fires once, next tick)
    ZECaptureStarted,   // military capture meter left 0
    ZEContested,        // both sides in a zone with capture in progress
    ZECaptureLost,      // capture meter decayed back to 0
    NZoneEventTypes
};

struct ZoneEventRecord
{
    ZoneEventType type;
    int zoneIndex;
};

// Per-tick world observations.  The engine path fills this from the live
// world (GatherInputs); unit tests inject values directly.
struct ZoneTickInputs
{
    bool playerValid = false;      // real player exists and is alive
    float playerX = 0;             // easting
    float playerZ = 0;             // northing
    bool playerUndercover = false; // script-owned gmUndercover global
    // per zone: live units within zoneArea, positionally counted per side.
    // The resistance count excludes the real player while playerUndercover
    // (a man the AI cannot engage is not an armed force securing ground).
    AutoArray<int> guerCount;
    AutoArray<int> occCount;
};

class ZoneRegistry : public SerializeClass
{
  public:
    ZoneRegistry() = default;

    // engine-wide instance (used by the World hooks and script commands)
    static ZoneRegistry& Instance();

    // hard ceiling on the zone table (explicit + seeded CITY zones)
    static constexpr int MaxZones = 64;

    // lifecycle -----------------------------------------------------------
    void Clear();       // full reset, including event handlers
    void InitMission(); // Clear + LoadFromConfig; call at mission start
    // rebuild static zone/faction tables from ExtParsMission, then Pars;
    // also resolves the campaign's occupier/resistance sides from the
    // gmSelOccupier / gmSelResistance script globals (new-game UI)
    void LoadFromConfig();
    // testable core: either entry may be null (that table stays empty).
    // selOccupier / selResistance are raw faction selections (a
    // CfgGuerrillaFactions class name or a side string); unmatched or
    // null/empty selections fall back to the CfgGuerrillaZones
    // defaultOccupier / defaultResistance keys (direct mission launches
    // without the new-game UI), then to the built-in "EAST"/"GUER".
    // worldNamesCfg is the world's Names class for the optional CITY
    // auto-seed (null: no seeding).
    void LoadFromParams(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg, const char* selOccupier = nullptr,
                        const char* selResistance = nullptr, const ParamEntry* worldNamesCfg = nullptr);

    // queries -------------------------------------------------------------
    bool IsActive() const { return _zones.Size() > 0; }
    int NZones() const { return _zones.Size(); }
    const ZoneRecord* GetZone(int index) const;
    ZoneRecord* GetZoneMutable(int index);
    int FindZoneIndex(const char* name) const; // -1 when not found
    const ZoneTuning& Tuning() const { return _tuning; }

    // campaign faction sides (resolved at load; serialized with the save)
    RString OccupierSide() const { return _occupierSide; }
    RString ResistanceSide() const { return _resistanceSide; }

    // clamped Heat writes (script surface)
    void HeatRaise(int index, float amount); // clamp at 100
    void HeatDecay(int index, float amount); // clamp at 0

    // faction queries; the faction is looked up by side string or config
    // class name (side wins); empty string when faction/key/level is unknown
    RString FactionTierClass(const char* side, float warLevel) const;
    RString FactionVehicle(const char* side, float warLevel) const;
    RString FactionValue(const char* side, const char* key) const;

    // events ----------------------------------------------------------------
    void SetEventHandler(ZoneEventType type, RString handler);
    RString GetEventHandler(ZoneEventType type) const;
    static int EventTypeFromName(const char* name); // -1 when unknown

    // campaign load notification: queued by Serialize at the end of a load,
    // consumed by the next Simulate tick, which dispatches the
    // "campaignLoaded" handler with _this = [loadedSaveVersion].  Replaces
    // the GM_SAVED sentinel + 5 s poll of the script era.
    void MarkCampaignLoaded(int loadedVersion);
    bool ConsumeCampaignLoaded(int& loadedVersion);

    // simulation ------------------------------------------------------------
    // per-frame engine hook; internally throttled to tickInterval
    void Simulate(float deltaT);
    // pure tick logic: mutates zone state, appends fired events (no world access)
    void EvaluateTick(const ZoneTickInputs& in, AutoArray<ZoneEventRecord>& fired);

    // save/load; dynamic per-zone state keyed by zone name
    LSError Serialize(ParamArchive& ar) override;

  private:
    // dynamic per-zone state as stored in a savegame
    struct ZoneSaveState
    {
        RString name;
        RString owner;
        float garrison = 0;
        float support = 0;
        float income = 0;
        float heat = 0;
        float liveOccupiers = 0;
        float capture = 0;
        bool revealed = false;

        LSError Serialize(ParamArchive& ar);
    };

    void LoadZones(const ParamEntry& cfg);
    void LoadFactions(const ParamEntry& cfg);
    void SeedCityZones(const ParamEntry& namesCfg);
    // side match first, then faction class name (both case-insensitive)
    const FactionRecord* FindFaction(const char* sideOrClass) const;
    // map a raw selection to a concrete side via the faction table;
    // unmatched/empty keeps the current value
    void ResolveSides(const char* selOccupier, const char* selResistance);
    // "OCCUPIER"/"RESISTANCE" -> the resolved sides; anything else verbatim
    RString ResolveOwnerToken(const RString& owner) const;
    void ApplyOwnerTokens(); // re-map every zone's owner from ownerConfig
    void GatherInputs(ZoneTickInputs& in) const;
    void UpdateMarkers();
    void DispatchEvents(const AutoArray<ZoneEventRecord>& fired);
    void ApplyPendingLoad();

    ZoneTuning _tuning;
    AutoArray<ZoneRecord> _zones;
    AutoArray<FactionRecord> _factions;
    // campaign faction sides; defaults match the Phase-1 Demo campaign
    RString _occupierSide = RString("EAST");
    RString _resistanceSide = RString("GUER");
    RString _handlers[NZoneEventTypes];
    float _accum = 0;
    // deserialized rows waiting for the mission config (applied on the
    // second load pass, once description.ext has been reparsed)
    AutoArray<ZoneSaveState> _pending;
    // deserialized faction sides, same two-pass parking (scalar archive
    // reads happen on the first pass only; applied on the second)
    RString _pendingOccupierSide;
    RString _pendingResistanceSide;
    // queued campaignLoaded notification (see MarkCampaignLoaded)
    bool _pendingLoaded = false;
    int _loadedSaveVersion = 0;
};

// Map a side string to its live command center WITHOUT creating one (an
// absent center means the side has no units yet) - the non-creating sibling
// of GarrisonCache's EnsureCenter.  Null when the side name or world is
// unknown.  Shared by ZoneRegistry (resistance presence) and AlertMachine
// (occupier perception).
AICenter* FindSideCenter(const char* sideName);

} // namespace Guerrilla
} // namespace Poseidon
