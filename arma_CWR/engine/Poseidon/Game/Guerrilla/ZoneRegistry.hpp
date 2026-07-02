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

    // dynamic, serialized (matched by zone name on load)
    RString owner;           // "GUER" | "EAST" | "NEUTRAL"
    float garrison = 0;      // occupier body count while despawned
    float support = 0;       // 0..100, CITY flips on this
    float income = 0;        // per economy tick when GUER-owned
    float heat = 0;          // 0..100 per-zone Heat
    float liveOccupiers = 0; // transient mirror written by the spawning script
    bool revealed = false;   // fog-of-war state
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

    RString side; // "EAST" / "GUER" / ...
    AutoArray<RString> tiers;
    AutoArray<float> tierThresholds; // ascending war levels; tier i+1 from thresholds[i]
    AutoArray<RString> vehicles;
    float vehicleThreshold = 3.0f;
    AutoArray<NamedValue> values;
};

enum ZoneEventType
{
    ZECaptured,         // any owner flip
    ZESupportThreshold, // CITY crossed supportFlip
    ZERevealed,         // zone first became revealed
    ZECampaignLoaded,   // savegame finished loading (fires once, next tick)
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
    bool playerValid = false;    // real player exists and is alive
    float playerX = 0;           // easting
    float playerZ = 0;           // northing
    AutoArray<bool> guerPresent; // per zone: live GUER-side unit within zoneArea
};

class ZoneRegistry : public SerializeClass
{
  public:
    ZoneRegistry() = default;

    // engine-wide instance (used by the World hooks and script commands)
    static ZoneRegistry& Instance();

    // lifecycle -----------------------------------------------------------
    void Clear();       // full reset, including event handlers
    void InitMission(); // Clear + LoadFromConfig; call at mission start
    // rebuild static zone/faction tables from ExtParsMission, then Pars
    void LoadFromConfig();
    // testable core: either entry may be null (that table stays empty)
    void LoadFromParams(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg);

    // queries -------------------------------------------------------------
    bool IsActive() const { return _zones.Size() > 0; }
    int NZones() const { return _zones.Size(); }
    const ZoneRecord* GetZone(int index) const;
    ZoneRecord* GetZoneMutable(int index);
    int FindZoneIndex(const char* name) const; // -1 when not found
    const ZoneTuning& Tuning() const { return _tuning; }

    // clamped Heat writes (script surface)
    void HeatRaise(int index, float amount); // clamp at 100
    void HeatDecay(int index, float amount); // clamp at 0

    // faction queries; empty string when side/key/level is unknown
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
        bool revealed = false;

        LSError Serialize(ParamArchive& ar);
    };

    void LoadZones(const ParamEntry& cfg);
    void LoadFactions(const ParamEntry& cfg);
    const FactionRecord* FindFaction(const char* side) const;
    void GatherInputs(ZoneTickInputs& in) const;
    void UpdateMarkers();
    void DispatchEvents(const AutoArray<ZoneEventRecord>& fired);
    void ApplyPendingLoad();

    ZoneTuning _tuning;
    AutoArray<ZoneRecord> _zones;
    AutoArray<FactionRecord> _factions;
    RString _handlers[NZoneEventTypes];
    float _accum = 0;
    // deserialized rows waiting for the mission config (applied on the
    // second load pass, once description.ext has been reparsed)
    AutoArray<ZoneSaveState> _pending;
    // queued campaignLoaded notification (see MarkCampaignLoaded)
    bool _pendingLoaded = false;
    int _loadedSaveVersion = 0;
};

} // namespace Guerrilla
} // namespace Poseidon
