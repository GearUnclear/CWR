#pragma once

// Guerrilla Mode zone registry - native replacement for the Phase-1
// zones.sqs polling loop (guerrilla-mode/mission/Guerrilla.Demo/scripts).
// Zone and faction data come from mission config (class CfgGuerrillaZones /
// class CfgGuerrillaFactions in description.ext, with the global config as
// fallback).  Without config the registry is inactive: zero zones, Simulate
// is a no-op and the gm* script commands return empty defaults, so ordinary
// missions are unaffected.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Containers/RStringArray.hpp> // FindArrayRStringCI (CollectFactionAddons)
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/IO/Serialization/SerializeClass.hpp>
#include <Poseidon/Game/Guerrilla/BaseDetect.hpp> // the AIRFIELD / SEAPORT seed (issue #76)

#include <vector>

class ParamArchive;
namespace Poseidon
{
class AICenter;
} // namespace Poseidon
namespace Poseidon
{
class AIGroup;
class Person;
} // namespace Poseidon

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
    float cacheRadius = 800.0f; // the simulation bubble: zones inside always tick
    // Off-screen contract D1 (issue #71, guerrilla-mode/OFFSCREEN-CONTRACT.md):
    // a zone outside cacheRadius still ticks while live resistance units
    // stand in it, or while its capture meter is above 0 (so an unheld
    // half-capture decays).  Remote CITY ticks gain support only: there is
    // no remote intimidation decay.  Config key remoteTick (0/1).
    bool remoteTick = true;
    // [gmsim] diagnostics line every simDiagInterval s (0 = off) and the
    // live-entity budgets it warns on (issue #71 section 6)
    float simDiagInterval = 60.0f;
    int budgetLiveBodies = 130;  // live AI bodies, every side (hard cap)
    int budgetSideGroups = 40;   // groups on any one side center (engine cap 63)
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
    // Town defenders (issue #79 B2, #72 first slice).  Every CITY the
    // resistance does not own (NEUTRAL seeded towns included) is
    // occupier-ADMINISTERED and keeps an occupier garrison of this many
    // bodies on its zone record, materialized by the production
    // GarrisonCache like any post.  Per-zone override: garrison = N on the
    // CITY entry.  0 restores the legacy free towns.  Range-checked at load
    // (integer, 0..MaxCityGarrison).  See the support gate in EvaluateTick.
    static constexpr int MaxCityGarrison = 24;
    int cityGarrison = 4;
    // Town retake (issue #79 scope addition): a resistance-held CITY with
    // living occupier units inside zoneArea and no resistance presence loses
    // supportRate per tick and reverts to occupier administration (garrison
    // refilled) once support falls below townRevertSupport; resistance presence
    // in a held town consolidates it at the same rate.  0 = liberated towns
    // are permanent and inert (the pre-#79 rule).
    bool townRetake = true;
    // Retake hysteresis (issue #79, the counter-attack follow-up): a held
    // town reverts only once occupier boots have drained its support below
    // this line, not below supportFlip.  A town flips at supportFlip (60) and
    // reverts below townRevertSupport (30), so walking out of a town you
    // just took no longer hands it back on the first occupier tick.
    // Range-checked at load to 0..supportFlip; townRevertSupport = supportFlip
    // restores the pre-hysteresis rule.
    float townRevertSupport = 30.0f;
    // Base recapture (OCCUPIER-FORCES.md 7.2, D16, D19): a resistance-held
    // military zone (CampaignEnd::IsMilitaryType: not CAMP, not CITY) carries
    // the occupier's mirror of the player's capture meter, occCapture.  Any
    // occupier presence (positional, plus an abstract BASE / HQ_ASSAULT force
    // holding it) with no resistance HOLD presence raises it by captureRate x
    // min(crew, captureCrewCap) per tick; both present freeze it; resistance
    // alone decays it at captureDecayDefended, nobody at
    // captureDecayAbandoned.  At 100 the zone reverts (RevertBase).  Config
    // key baseRetake (0/1); absent, it follows the occupierForces layer key,
    // so with the layer off liberated posts stay permanent (the old decision
    // R) and nothing changes for missions that pin the layer off.
    bool baseRetake = false;
    // defenders >= ratio * attackers treats a contested zone as DEFENDED
    // (a token straggler cannot hold a re-secured post hostage); 0 disables.
    // Any live defender still blocks SECURING regardless of this ratio.
    float contestOutnumberRatio = 4.0f;
    // CITY auto-seed from the world's CfgWorlds >> <world> >> Names entries
    // (issue #54 C3). Auto (the key is absent) seeds every Names entry the
    // settlement classifier accepts: town-typed or type-less, on dry land,
    // with buildings around it - so a theatre map's sea, country and terrain
    // labels seed nothing. All (seedCities = 1) is the legacy override that
    // seeds every town-typed / type-less entry with no classification; Off
    // (seedCities = 0) seeds none.
    enum class SeedCities
    {
        Auto,
        Off,
        All
    };
    SeedCities seedCities = SeedCities::Auto;
    float seedCitySupport = 20.0f; // starting support of each seeded CITY
    // AIRFIELD / SEAPORT auto-seed from the loaded landscape (issue #76,
    // BaseDetect): every SEED candidate the detector finds becomes an
    // occupier zone.  Config key seedBases (0/1); absent = on exactly when
    // the template's Zones block authors no AIRFIELD and no SEAPORT zone
    // (authored zones keep precedence).  Detection itself runs whenever a
    // landscape is loaded (gmDetectedBases); this key gates the seeding.
    bool seedBases = false;
    // playRegion[] = {minX, minZ, maxX, maxZ} (easting / northing): the part
    // of the map the campaign is played on.  An AUTHORED region bounds
    // everything: a Names town outside it is not seeded as a CITY nor offered
    // as a START TOWN, and a detected base outside it is not seeded.  Absent
    // (or malformed: warned, ignored) = the default box, the bounding box of
    // the authored zones' positions padded by DefaultRegionPad on every side,
    // which bounds the BASE seed only: the city seed and the START TOWN list
    // stay unbounded, as before issue #76.  No positioned authored zone = no
    // region at all (the whole map).
    static constexpr float DefaultRegionPad = 5000.0f;
    struct PlayRegion
    {
        bool valid = false;
        bool authored = false; // from playRegion[]; false = the default box (or none)
        float minX = 0, minZ = 0, maxX = 0, maxZ = 0;
        // true for every point when no region is set
        bool Contains(float x, float z) const
        {
            return !valid || (x >= minX && x <= maxX && z >= minZ && z <= maxZ);
        }
        // the town bound (city seed, START TOWN list): only an authored
        // region applies, the default box bounds bases alone
        bool ContainsTown(float x, float z) const
        {
            return !authored || Contains(x, z);
        }
    };
    PlayRegion playRegion;
    // The side the template's mission.sqm welds the player's unit to.  The
    // resistance is counted out of that side's center (GatherInputs), so a
    // resistance pick may move the ROSTER onto this side but never move the
    // side itself - see ResolveSideCollisions.  EMPTY (the default) disables
    // that whole pass: a template that authors no playerSide behaves exactly
    // as it did before the pass existed.
    RString playerSide;
    // How the campaign history names the island as a place: a locative phrase,
    // preposition first ("in the Sinai", "in Lebanon").  EMPTY (the default)
    // reads "on <island>", right for every stock world, which are all islands.
    // Only a mainland or regional world needs to author it.
    RString islandPhrase;
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
    // D2 garrison recovery ceiling (issue #71): config garrisonMax, default
    // the authored garrison.  Static, config-rebuilt, never serialized.  For
    // a CITY it is the town's defender strength (cityGarrison or the
    // per-zone garrison override) even when the town starts resistance-held,
    // so a retaken town refills to it.
    float garrisonMax = 0;
    // an auto-seeded zone, not an authored one: a CITY from the world's
    // Names (SeedCityZones) or an AIRFIELD / SEAPORT from the base detector
    // (SeedBaseZones, issue #76).  Static, config-rebuilt, never serialized:
    // the occupier pool counts a seeded town in K only when it passes the
    // settlement probe (OCCUPIER-FORCES.md 5.1).  Every reader of this flag
    // that means "a seeded town" must also test type == CITY.
    bool seeded = false;

    // dynamic, serialized (matched by zone name on load)
    RString owner;           // concrete side: resistance / occupier / "NEUTRAL" / ...
    float garrison = 0;      // occupier reserve: whole stored bodies, not materialized (U3)
    float support = 0;       // 0..100, CITY flips on this
    float income = 0;        // per economy tick when GUER-owned
    float heat = 0;          // 0..100 per-zone Heat
    float liveOccupiers = 0; // live-count mirror written natively by GarrisonCache
                             // (script-overridable via gmZoneSet); informational
                             // only - NOT part of the capture predicate
    float capture = 0;       // 0..100 military consolidation meter
    bool revealed = false;   // fog-of-war state
    // intel age for the journal's zone ledger: the campaign day (1-based, 0
    // = never) and minute of day the zone was last inside the player's
    // simulation bubble (cacheRadius).  Remote ticks (D1) do not stamp it:
    // "seen" means the player was near, not that the meters last moved.
    int seenDay = 0;
    int seenMinute = 0;
    // D2 recovery clock (s): time this zone has spent calm, distant and
    // short of garrisonMax since its last recovered body.  Serialized
    // (absent in older saves: 0).
    float recoverClock = 0;
    // Town defenders (issue #79 B2): a CITY exempted from its occupier
    // garrison (the start-town headquarters, GuerrillaBase) - never
    // garrisoned, recovered or refilled.  Serialized (absent: false).
    bool garrisonExempt = false;
    // campaign day (1-based, 0 = never) and minute of day of the last flip
    // TO the resistance (military capture or town rising).  The seam for a
    // counter-attack service.  Serialized (absent in older saves: 0).
    int flipDay = 0;
    int flipMinute = 0;
    // S5 pending deposit (OCCUPIER-FORCES.md 3.3, GarrisonCache::Deposit):
    // whole bodies written toward this zone's reserve while the player was
    // within cacheRadius + GarrisonCache::DespawnHysteresis of it.  They do
    // NOT defend (no capture, defenders or support-gate predicate reads them)
    // and do NOT materialize; GarrisonCache moves them into the reserve when
    // the zone leaves the bubble or every spawn point is hidden, and refunds
    // them (pendingOrigin) when the occupier loses the zone.  Serialized
    // (absent in older saves: 0).
    int pending = 0;
    int pendingOrigin = -1; // zone the pending bodies came from; -1 none or mixed (saved by name)
    // occupier forces phase 4 (OCCUPIER-FORCES.md 7.2, 7.3, 11; serialized,
    // absent in older saves = the defaults):
    // the occupier's base-recapture meter (0..100), the mirror of capture on
    // a resistance-held military zone.  Its own field, so capture.sqs's
    // "Securing" ticker, ZECaptureStarted and the marker labels never read
    // it as the player's progress.
    float occCapture = 0;
    // occupier standing in a CITY (0..100, D21): authored per CITY (key
    // standing, default 50); recovers toward 50 at 1 per 600 s
    // (OccupierForces); drives informant tips (>= 50), garrison morale
    // (< 25: skill x 0.8) and the refill factor (the K towns' mean)
    float standing = 50.0f;
    // the town-strike stamp (phase 6, 7.3): mission time (the script `time`,
    // Glob.time) of the strike + StrikeStampSeconds.  Until then the town
    // pays half income and no manpower (economy.sqs through gmZoneStruck).
    // 0 = never struck.
    float struckUntil = 0;
    // times the resistance lost this zone back to the occupier (RevertTown,
    // RevertBase); capture.sqs writes "Retaken" instead of "Liberated" for a
    // zone with lostCount > 0 (gmZoneSim slot 15)
    int lostCount = 0;
    // the settlement probe's verdict on a SEEDED CITY for the occupier pool's
    // K (OCCUPIER-FORCES.md 5.1): 0 not probed yet, 1 a settlement, -1 a
    // label with no settlement under it (not counted in K or G).  Written
    // once (ProbeSeededSettlements, when the pool sizes: its first active
    // tick, or the old-save derivation) and serialized, so K is the same in
    // every session.  Authored zones stay 0.  Absent in older saves: 0,
    // probed when the pool next sizes.
    int settlement = 0;

    // transient tick bookkeeping, never serialized
    bool contestedLastTick = false;    // edge detection for the contested event/marker
    bool supportReadyNotified = false; // supportThreshold fired for the current crossing
    // last zone tick's presence and bubble state (gmZoneSim, D2 eligibility)
    int lastGuer = 0;     // resistance CAPTURE presence (the player per PlayerCapturesPresence)
    int lastGuerHold = 0; // resistance HOLD presence (the player per PlayerHoldsPresence)
    int lastOcc = 0;
    int lastGarrisonLive = 0; // the zone's own live cache-garrison bodies, wherever they stand
    int lastGarrisonNear = 0; // ... of those, within cacheRadius of the zone centre (U4)
    bool lastInBubble = false;
    bool lastRemote = false; // the last tick ran this zone's meters from outside the bubble
    // bodies the last RevertTown refill added (to the reserve or the pending
    // deposit); CounterAttack::AdoptLive subtracts an adopted squad from it
    // (U2).  Transient: the adoption runs in the same tick as the revert.
    int revertRefill = 0;
};

// One gmSimDiag / [gmsim] sample (issue #71 section 6): what the off-screen
// contract's budgets are measured against.  Filled by GatherSimDiag.
struct SimDiag
{
    int zonesInBubble = 0;
    int zonesRemoteTicked = 0; // remote meter updates on the last zone tick
    int liveOcc = 0;
    int liveRes = 0;
    int liveCiv = 0;
    int liveTotal = 0; // every side center, player included
    int grpOcc = 0;
    int grpRes = 0;
    int grpCiv = 0;
    int grpMaxSide = 0; // largest group count on any one side center
    int garrisonSpawnedZones = 0;
    int garrisonLiveBodies = 0;
    int trafficCiv = 0;
    int trafficParked = 0;
    int trafficPatrol = 0;
    int trafficConvoy = 0;
    float gmAvg = 0, gmP95 = 0, gmMax = 0;          // FrameProfiler "gm" phase (ms)
    float frameAvg = 0, frameP95 = 0, frameMax = 0; // whole frame (ms)
    // CounterAttack's live occupier groups beyond 850 m of the player
    // (cacheRadius + DespawnHysteresis), by cause (CounterAttackFarCensus)
    int farOcc = 0;
    int farArrival = 0;
    int farMarch = 0;
    int farLeftover = 0;
    int farHomeHull = 0;
    int farAir = 0;
};

enum SimBudgetBreach
{
    SBLiveBodies = 1, // liveTotal > budgetLiveBodies
    SBSideGroups = 2, // grpMaxSide > budgetSideGroups
};

// pure budget check (unit-tested): SimBudgetBreach bits, 0 = within budget.
// A budget <= 0 is off.
int SimBudgetBreaches(const SimDiag& d, const ZoneTuning& t);
// world-touching sampler (engine path); zero sample without a world
void GatherSimDiag(SimDiag& out);

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
    // optional per-tier role variants, parallel to tiers[] (plan 15).  A
    // missing array, short array or "" entry means "this tier fields no such
    // role"; every consumer substitutes the tier's rifleman (tiers[i])
    AutoArray<RString> tiersMG;
    AutoArray<RString> tiersAT;
    AutoArray<RString> tiersMedic;
    AutoArray<RString> tiersSniper;
    // civilian-outfit ladder (issue #25): armed fighters in civilian clothes
    // for garrison/guard/militia rungs.  Indexed by the same
    // tierThresholds[] as tiers[], clamped to its own length; empty = the
    // faction offers no civilian-outfit bodies (callers keep warriors)
    AutoArray<RString> civTiers;
    AutoArray<RString> vehicles;
    // civilian road-traffic hulls (Traffic, CIV descriptor): unresolvable
    // entries are dropped at load; empty = no civilian traffic
    AutoArray<RString> civVehicles;
    float vehicleThreshold = 3.0f;      // legacy 2-step ladder (index 0 -> 1)
    AutoArray<float> vehicleThresholds; // full ladder, mirrors tierThresholds
    AutoArray<NamedValue> values;
};

// Class-existence probe for the faction-descriptor resolution pass (plan 15).
// The engine path probes the merged game config (Pars >> bank); unit tests
// inject a fake so LoadFromParams stays testable without a live config.
struct ClassProbe
{
    virtual ~ClassProbe() = default;
    // bank is a top-level config class ("CfgVehicles" / "CfgWeapons")
    virtual bool Exists(const char* bank, const char* className) const = 0;
    // A CfgVehicles class the loaded package can actually SPAWN: it exists
    // AND the shape file its model names is in the loaded data (issue #46
    // seam 4b). Exists alone is the weaker gate: a class whose config is
    // present but whose .p3d the package never ships passes it, spawns, and
    // access-violates in Man::Init (measured on @LoBo's CoC_Diverdes). The
    // resolution pass asks this for every unit and vehicle key so such a
    // class is substituted through the same ladder as an absent one. The
    // default keeps existence-only fakes and probes on their old answer.
    virtual bool Spawnable(const char* className) const { return Exists("CfgVehicles", className); }
};

// The engine's ClassProbe: a classname exists when the merged game config
// (Pars, i.e. the loaded data package + addons) carries it under the bank.
// Shared by the registry load path and the player outfit-substitution seam
// (OutfitSelect), which runs before the registry loads.
struct ParsClassProbe final : ClassProbe
{
    bool Exists(const char* bank, const char* className) const override;
    // Exists plus the shape-file gate the player-body seam and the menu run
    // (PlayerBodyModelIssue over GetShapeName + QIFStreamB::FileExist).
    bool Spawnable(const char* className) const override;
};

// Settlement classifier for the CITY auto-seed (issue #54 C3): is this Names
// entry a place people live, or a label? OFP-era Names blocks carry no type,
// and a theatre map (Lebanon80) names seas, countries, mountain ranges and
// deep-rear cities in the same block as its towns. The engine probe answers
// from the loaded landscape; unit tests inject a fake.
struct SettlementProbe
{
    virtual ~SettlementProbe() = default;
    virtual bool IsSettlement(const char* name, const Vector3& pos) const = 0;
};

// The engine's SettlementProbe: dry land (Landscape::SurfaceY above sea level at the
// entry) with at least kMinBuildings Building objects of house size
// (bounding sphere > kMinBuildingRadius) within kRadius metres. A run with
// no landscape or world loaded (headless config tests) accepts everything,
// which is the pre-C3 behaviour.
// The occupier pool's K (OCCUPIER-FORCES.md 5.1, 16) applies the same rule to
// the towns seedCities = 1 seeded without it.  log: the Auto seed's INFO line
// per rejected entry ("not seeded"); the pool passes false and writes its own.
struct LandscapeSettlementProbe final : SettlementProbe
{
    static constexpr float kRadius = 300.0f; // the seed dedup radius: one town, one circle
    static constexpr int kMinBuildings = 3;
    static constexpr float kMinBuildingRadius = 4.0f; // fences, poles and sheds do not make a town
    bool log = true;
    bool IsSettlement(const char* name, const Vector3& pos) const override;
};

// World seam of the AIRFIELD / SEAPORT seed (issue #76): fills the
// world-side half of the detector's inputs (worldClass, worldDisplay, ils,
// objects, height, worldSize).  The registry adds the rest itself (the Names
// towns, the authored zones, the playRegion), so a fake only has to model the
// world.  The HeightField it points in.height at must outlive the call's
// caller (DetectBases runs right after Gather).  False = no world to detect
// on (headless): no detection, no seeding.
struct BaseWorldProbe
{
    virtual ~BaseWorldProbe() = default;
    virtual bool Gather(BaseDetectInputs& in) const = 0;
};

// The engine's BaseWorldProbe: Glob.header.worldname, CfgWorlds >> world
// (description, the ILS entries), every landscape object whose model
// ClassifyBaseModel weighs or that is a house-sized Building (bounding
// sphere > LandscapeSettlementProbe::kMinBuildingRadius), the landscape's
// SurfaceY (sea level 0) and land size.  False without a landscape.
struct LandscapeBaseProbe final : BaseWorldProbe
{
    struct SurfaceHeight final : HeightField
    {
        float Height(float x, float z) const override;
    };
    SurfaceHeight height;
    bool Gather(BaseDetectInputs& in) const override;
};

enum ZoneEventType
{
    ZECaptured,         // a flip TO the resistance (military capture or town rising)
    ZESupportThreshold, // CITY crossed supportFlip (the town is READY; flip is separate)
    ZERevealed,         // zone first became revealed
    ZECampaignLoaded,   // savegame finished loading (fires once, next tick)
    ZECaptureStarted,   // military capture meter left 0
    ZEContested,        // both sides in a zone with capture in progress
    ZECaptureLost,      // capture meter decayed back to 0
    ZETownLost,         // "townLost": a resistance-held CITY reverted to occupier administration
    ZECounterAttack,    // "counterAttack": an occupier retake force reached a held town (CounterAttack)
    ZECounterRepelled,  // "counterRepelled": that force was beaten off or withdrew
    // "baseLost": a resistance-held military zone (CAMP / AIRFIELD / SEAPORT /
    // OUTPOST) went back to the occupier: its occCapture meter reached 100
    // (OCCUPIER-FORCES.md 7.2, the base recapture)
    ZEBaseLost,
    // "hqLost": the resistance HQ was destroyed (GuerrillaBase::Destroy queues
    // it through QueueEvent; the zone index is the zone the HQ stood in, or
    // -1 when it stood outside every zone)
    ZEHqLost,
    // "townStruck": an occupier air strike hit this town (OCCUPIER-FORCES.md
    // 7.3, D17, D21; phase 6).  ApplyStrike queues it; civilians.sqs starts
    // the town's panic on it
    ZETownStruck,
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
    // guerCount is the resistance CAPTURE presence: it includes the real
    // player only while PlayerCapturesPresence holds (issue #86: not
    // undercover, exposed, or a weapon in hands).  Read by the military
    // capture meter and the administered-town support rise.
    AutoArray<int> guerCount;
    AutoArray<int> occCount;
    // per zone (optional, missing = guerCount): the resistance HOLD presence,
    // the same count with the player included while PlayerHoldsPresence
    // holds (D11, D23: a uniformed player always holds; a disguised one only
    // exposed, armed or with his weapon seen).  Read by the town retake
    // (the drain of a held town and its consolidation) only.
    AutoArray<int> guerHold;
    // per zone (optional, missing = 0): live bodies of the zone's own
    // GarrisonCache garrison WHEREVER they stand.  Town defenders who left
    // the 150 m circle (chasing, posted at an approach) still hold the town;
    // read by the CITY support gate only.
    AutoArray<int> garrisonLive;
    // per zone (optional, missing = 0): live bodies of the zone's own
    // GarrisonCache garrison within cacheRadius (800 m) of the zone centre
    // (GarrisonCache::LiveCountWithin).  U4 (OCCUPIER-FORCES.md 5.5): a
    // post's garrison that stepped out of the 150 m circle to fight still
    // defends it.  Read by the military capture branch only.
    AutoArray<int> garrisonNear;
    // per zone (optional, missing = 0): ABSTRACT occupier presence, the
    // bodies of an off-screen counter-attack force holding a resistance-held
    // town (CounterAttack::AbstractPresence).  Read by the town retake only:
    // it drains the town exactly like live occupiers would.
    AutoArray<int> abstractOcc;
    // per zone (optional, missing = 0): the HOLDING bodies a revert of this
    // town settles as its new garrison (CounterAttack::SettlePresence; 0
    // with occupierForces off, OCCUPIER-FORCES.md 5.4).  Read by the town
    // retake's RevertTown only.
    AutoArray<int> settleOcc;
};

// When does the real player count as resistance presence?  Two predicates
// over one sample (OCCUPIER-FORCES.md 8.1; owner decisions D11, D23 and
// issue #86).  Inputs:
//   undercover     - the script-owned gmUndercover global (true for the
//                    whole campaign; exposure is per observer);
//   exposed        - a living occupier observer holds an exposed
//                    (compromised) verdict on him: some occupier group with
//                    at least one alive member has a ucCompromised record of
//                    him or his vehicle (UndercoverSystem::LivingWitnessCount);
//   weaponInHands  - he is overtly armed: the undercover weapon-show
//                    classifier reads UCWInHands (any raised-weapon pose with
//                    a weapon carried; a drawn pistol counts, a slung rifle
//                    does not);
//   warriorOutfit  - he wears a warrior body, i.e. NOT a civilian outfit
//                    (ZoneRegistry::IsCivilianOutfitClass false and the body
//                    class's typical side is not civilian);
//   weaponSeen     - a living occupier observer currently rates him suspect
//                    or exposed because of a visible weapon, slung or in
//                    hands (UndercoverSystem::WeaponSeenCount: the per-group
//                    stamp ApplyVerdict writes, at most WeaponSeenWindow old).
// HOLD (PlayerHoldsPresence) blocks the drain of a held town: not undercover,
// or a uniform (weapon out or not, D23); in a civilian outfit only when
// exposed, armed, or with his weapon seen (D11).
// CAPTURE (PlayerCapturesPresence) raises an administered town's support and
// a military zone's capture meter: not undercover, or exposed, or a weapon
// in hands (issue #86); the outfit alone never counts, and weapon-seen is a
// HOLD clause only (the design text of 8.1).
struct PlayerPresenceInput
{
    bool undercover = false;
    bool exposed = false;
    bool weaponInHands = false;
    bool warriorOutfit = false;
    bool weaponSeen = false;
};
bool PlayerHoldsPresence(const PlayerPresenceInput& in);
bool PlayerCapturesPresence(const PlayerPresenceInput& in);

class ZoneRegistry : public SerializeClass
{
  public:
    ZoneRegistry() = default;

    // engine-wide instance (used by the World hooks and script commands)
    static ZoneRegistry& Instance();

    // hard ceiling on the zone table (explicit + seeded bases + seeded CITY zones)
    static constexpr int MaxZones = 64;

    // lifecycle -----------------------------------------------------------
    void Clear();       // full reset, including event handlers
    void InitMission(); // Clear + LoadFromConfig + ActivateFactionAddons; call at mission start
    // World::ActivateAddon for every addon CollectFactionAddons finds that is
    // not active yet (issue #54 C1); no-op without a world or an inactive
    // registry. Logged once at INFO.
    void ActivateFactionAddons() const;
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
    // auto-seed (null: no seeding).  probe, when non-null, runs the
    // faction-descriptor class-resolution pass (plan 15): every
    // classname-valued key is checked against the loaded data package and
    // unresolvable entries are substituted (logged via LOG_WARN) instead of
    // producing fatal/sterile spawns downstream.  bases, when non-null and
    // the zones config is present, runs the AIRFIELD / SEAPORT detector
    // (issue #76) after the authored zones load and before the CITY seed, and
    // seeds its SEED candidates when seedBases resolves on (SeedBaseZones).
    void LoadFromParams(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg, const char* selOccupier = nullptr,
                        const char* selResistance = nullptr, const ParamEntry* worldNamesCfg = nullptr,
                        const ClassProbe* probe = nullptr, const SettlementProbe* settlement = nullptr,
                        const BaseWorldProbe* bases = nullptr);
    // Test seam: the BaseWorldProbe LoadFromConfig (and so a save's load
    // pass) hands LoadFromParams instead of the LandscapeBaseProbe.  Null
    // restores the landscape.  Not owned; Clear resets it.
    void SetBaseProbeOverride(const BaseWorldProbe* probe) { _baseProbeOverride = probe; }

    // the AIRFIELD / SEAPORT detector's last run (issue #76) ----------------
    // every AIRFIELD and SEAPORT candidate, all statuses, in detector order;
    // empty when detection did not run.  Transient, config-rebuilt, never
    // serialized (gmDetectedBases).
    const std::vector<BaseCandidate>& DetectedBases() const { return _detectedBases; }
    bool BaseDetectionRan() const { return _baseDetectionRan; }
    // the candidates seeded as zones, in seeding order (gmSeededBases); each
    // is an index into DetectedBases()
    int NSeededBases() const { return (int)_seededBases.size(); }
    const BaseCandidate* SeededBase(int i) const
    {
        return i >= 0 && i < (int)_seededBases.size() ? &_detectedBases[_seededBases[i]] : nullptr;
    }

    // queries -------------------------------------------------------------
    bool IsActive() const { return _zones.Size() > 0; }
    int NZones() const { return _zones.Size(); }
    // may exceed the descriptor count: DivergeAliasedFactions appends a copy
    // of a roster picked for both sides
    int NFactions() const { return _factions.Size(); }
    // table order (config order, plus DivergeAliasedFactions copies); null
    // out of range
    const FactionRecord* GetFaction(int index) const;
    // The addon closure of every class the loaded faction table names
    // (issue #54 C1): tiers[] and the role/civ ladders, vehicles[] and
    // civVehicles[], the unit-class keys (officer, holdClass, recruit*,
    // companionClass, fallbackClass and their *Civ twins) and the weapon /
    // magazine keys (base*, loot*). Owners resolve through
    // Asset/Addon/AddonClosure over the injected config roots; base-game
    // classes (empty owner) contribute nothing. Runs over the RESOLVED
    // table, so substitutions made by the plan-15 pass are what gets
    // activated. The engine wrapper (ActivateFactionAddons, run by
    // InitMission) activates the result so a template's addOns[] need not
    // list what its factions spawn.
    void CollectFactionAddons(const ParamEntry* vehiclesCfg, const ParamEntry* weaponsCfg,
                              const ParamEntry* magazinesCfg, FindArrayRStringCI& addons) const;
    const ZoneRecord* GetZone(int index) const;
    ZoneRecord* GetZoneMutable(int index);
    int FindZoneIndex(const char* name) const; // -1 when not found
    const ZoneTuning& Tuning() const { return _tuning; }

    // campaign faction sides (resolved at load; serialized with the save)
    RString OccupierSide() const { return _occupierSide; }
    RString ResistanceSide() const { return _resistanceSide; }
    // which DESCRIPTOR each side fields.  A side string alone stopped
    // identifying a pick once several rosters could share one side (Sinai
    // ships five EAST descriptors), and it never identified a rebased one -
    // these are the campaign's actual selections.  Empty when no selection
    // matched (the built-in side defaults are in force).
    RString OccupierFaction() const { return _occupierFaction; }
    RString ResistanceFaction() const { return _resistanceFaction; }

    // clamped Heat writes (script surface)
    void HeatRaise(int index, float amount); // clamp at 100
    void HeatDecay(int index, float amount); // clamp at 0

    // Town strikes (OCCUPIER-FORCES.md 7.3, D17, D21; phase 6) ---------------
    // the 7.3 table: the struck town loses StrikeCut resistance support and
    // StrikeCut occupier standing, gains StrikeHeat heat and is stamped
    // struckUntil = now + StrikeStampSeconds; every OTHER CITY whose centre
    // lies within StrikeRingRadius of the strike point loses StrikeRingCut of
    // each.  Both values floor at 0.
    static constexpr float StrikeCut = 40.0f;
    static constexpr float StrikeRingCut = 10.0f;
    static constexpr float StrikeRingRadius = 2000.0f;
    static constexpr float StrikeHeat = 15.0f;
    static constexpr float StrikeStampSeconds = 1200.0f;
    // Apply one strike's effects.  index: the struck zone (-1: a strike on
    // reported fighters outside every zone: the ring only).  The full cut,
    // the heat, the stamp and ZETownStruck (queued into fired) apply only
    // when index is a CITY; a strike on another zone type (a base's opener)
    // takes the ring only.  pos: the strike point (the ring's centre); VZero
    // = the zone's centre.  now: mission time (s).  Returns the number of
    // ring towns cut.  D18 is unchanged: nothing here raises a value, and a
    // struck town recovers only by the existing presence rules.
    int ApplyStrike(int index, Vector3Par pos, float now, AutoArray<ZoneEventRecord>& fired);
    // a CITY inside its strike window (now < struckUntil)
    bool IsStruck(int index, float now) const;

    // Town defenders (issue #79 B2) ------------------------------------------
    // An occupier-ADMINISTERED town: a CITY owned NEUTRAL or by the occupier
    // (never the resistance, never a third side).  Ownership tokens are NOT
    // changed by this model: NEUTRAL keeps meaning "not yet risen" for the
    // journal and economy; the garrison is what makes a town defended.
    bool IsAdministeredTown(const ZoneRecord& z) const;
    // GarrisonCache's materialization predicate: an occupier-held non-CITY
    // zone, or an administered town not exempted from its garrison.  The
    // garrison always fields the campaign's occupier side.
    bool IsGarrisonable(const ZoneRecord& z) const;
    // Revert a resistance-held CITY to occupier administration (the town
    // retake): owner back to its authored token (NEUTRAL when that token is
    // the resistance), garrison refilled up to garrisonMax unless exempt,
    // meters reset, ZETownLost queued.  The refill is whole bodies; with the
    // player inside the deposit bubble (cacheRadius + DespawnHysteresis, as
    // of the last zone tick) it is a PENDING deposit (S5, GarrisonCache::
    // AddReserve), else a direct reserve write that GarrisonCache
    // materializes on its next pass inside cacheRadius.  False (no-op) for a
    // zone that is not a resistance-held CITY, and after CampaignEnd's win
    // latch (OCCUPIER-FORCES.md 7.4: nothing reverts).  The seam a
    // counter-attack service can call directly.
    // Occupier forces (5.3, 5.4): settled bodies (a holding force's
    // strength) become the garrison first, up to garrisonMax, their surplus
    // goes to the pool; only the remaining room is a pool-funded deposit
    // (OccupierForces::Deposit, which passes everything through while the
    // pool is inactive and grants nothing in SPENT: a hollow revert).
    // revertRefill records the pool-funded part.
    bool RevertTown(int index, AutoArray<ZoneEventRecord>& fired, int settled = 0);
    // Revert a resistance-held military zone to the occupier (the base
    // recapture, OCCUPIER-FORCES.md 7.2): its occCapture reached 100.  Owner
    // back to the authored token (the occupier side when that token is the
    // resistance or neutral), the garrison settled exactly as RevertTown
    // does (settled holding bodies first up to garrisonMax, their surplus to
    // the pool, the rest of the room a pool-funded deposit through
    // OccupierForces::Deposit, pending inside the player's deposit bubble;
    // revertRefill records the pool-funded part so an adopted live squad
    // comes off it), income tap closed, capture and occCapture reset,
    // lostCount + 1, ZEBaseLost queued.  False (no-op) for a zone that is
    // not a resistance-held military zone, and after CampaignEnd's win or
    // lose latch.
    bool RevertBase(int index, AutoArray<ZoneEventRecord>& fired, int settled = 0);
    // a resistance-held military zone the base recapture works on
    // (CampaignEnd::IsMilitaryType, owner the resistance)
    bool IsHeldBase(const ZoneRecord& z) const;
    // Exempt a town from its occupier garrison (GuerrillaBase: the new-game
    // START TOWN headquarters).  Zeroes the reserve and any pending deposit;
    // persistent.
    void ExemptTownGarrison(int index);
    // The occupier pool's settlement pass (OCCUPIER-FORCES.md 5.1): every
    // SEEDED CITY whose settlement verdict is still 0 is put to the probe
    // once and the verdict stored on its row (1 or -1, serialized).  Rows
    // already probed, and authored zones, are never touched.  Returns the
    // number of rows probed now.
    int ProbeSeededSettlements(const SettlementProbe& probe);
    // CITY zones currently held by the resistance (the "towns liberated"
    // count, gmZoneStats)
    int LiberatedTownCount() const;
    // Is className one of the resistance descriptor's civilian-outfit bodies
    // (issue #25 Civ-family keys: playerClassCiv, companionClassCiv,
    // recruitFighterCiv, recruitSpecialistCiv, holdClassCiv, civTier[])?
    // Case-insensitive; false for an empty name.
    bool IsCivilianOutfitClass(const char* className) const;
    // Sample the real player's cover state for PlayerHoldsPresence and
    // PlayerCapturesPresence: gmUndercover, the living-witness exposure, the
    // weapon-show classifier, the outfit and the weapon-seen verdicts.
    // World-touching.
    void SamplePlayerPresence(Person* player, PlayerPresenceInput& out) const;

    // The campaign-aware faction lookup, and the one every side-keyed query
    // below goes through.  FindFaction scans by CLASS NAME first (issue #54:
    // a global faction library makes a bare side string ambiguous), so on a template
    // with several descriptors on one side it returns whichever was DECLARED
    // first - picking Sinai's Syria would silently field EgyptArmy's roster.
    // A side that is the campaign's occupier or resistance therefore resolves
    // to the descriptor that was actually selected; everything else (third
    // parties, class-name lookups) falls through to the by-side scan.
    const FactionRecord* FindFactionForSide(const char* side) const;

    // faction queries; the faction is looked up by side string or config
    // class name (side wins); empty string when faction/key/level is unknown
    RString FactionTierClass(const char* side, float warLevel) const;
    // civilian-outfit rung (issue #25): the civTier[] entry for the war
    // level (same tierThresholds[] gates as tiers[], clamped to the civ
    // ladder's own length); empty when the faction authors no civTier[]
    RString FactionCivTierClass(const char* side, float warLevel) const;
    RString FactionVehicle(const char* side, float warLevel) const;
    // the faction's civVehicles[] (package-resolved, may be empty)
    void FactionCivVehicles(const char* side, AutoArray<RString>& out) const;
    RString FactionValue(const char* side, const char* key) const;
    // role-diverse squad composition (plan 15): fills out with count
    // classnames from the faction's war-level tier - a deterministic
    // military template (leader slot first, then MG/AT/medic/sniper slots
    // interleaved with riflemen; roles the tier does not field become
    // riflemen).  Empty result when the faction has no tiers.
    void FactionSquad(const char* side, float warLevel, int count, AutoArray<RString>& out) const;

    // events ----------------------------------------------------------------
    void SetEventHandler(ZoneEventType type, RString handler);
    RString GetEventHandler(ZoneEventType type) const;
    static int EventTypeFromName(const char* name); // -1 when unknown
    // run the script handlers for events another Guerrilla service raised on
    // a zone (CounterAttack's counterAttack / counterRepelled); same payload
    // [zoneIndex, zoneName, owner] as the registry's own events
    void FireEvents(const AutoArray<ZoneEventRecord>& fired) { DispatchEvents(fired); }
    // the public event queue: another service raises a zone event (the HQ's
    // hqLost, GuerrillaBase::Destroy) without running handlers from inside
    // its own update.  The next zone tick prepends the queued events to its
    // own, so CounterAttack / OccupierForces hear them natively and the
    // script handler runs with the usual payload.  zoneIndex may be -1 (an
    // HQ outside every zone): native listeners hear it, the script handler
    // is skipped (no zone to name).  Transient, never serialized.
    void QueueEvent(ZoneEventType type, int zoneIndex)
    {
        ZoneEventRecord ev;
        ev.type = type;
        ev.zoneIndex = zoneIndex;
        _queued.Add(ev);
    }

    // pure town enumeration (unit-tested; no world access) -------------------
    // One Names entry (CfgWorlds >> <world> >> Names) -> (name, pos) when it
    // is a town SeedCityZones would seed: type-less OFP entries and the
    // city-like Arma types pass, name falls back to the class name, the 2D
    // position[] {x, z} maps to engine axes with elevation 0.
    static bool NamesEntryIsTown(const ParamEntry& e, RString& name, Vector3& pos);
    // NamesEntryIsTown plus the settlement classification (issue #54 C3):
    // under Auto a non-null probe must accept the entry; All skips the probe;
    // Off rejects everything.
    static bool NamesEntryIsSettlement(const ParamEntry& e, ZoneTuning::SeedCities mode, const SettlementProbe* probe,
                                       RString& name, Vector3& pos);
    // The seedCities key: absent = Auto, 0 = Off, anything else = All.
    static ZoneTuning::SeedCities ReadSeedCitiesMode(const ParamEntry* zonesCfg);
    // The seedBases key (issue #76): 0 = off, anything else = on; absent =
    // on exactly when the Zones block authors no AIRFIELD and no SEAPORT.
    // False for a null config.
    static bool ReadSeedBases(const ParamEntry* zonesCfg);
    // The playRegion[] key: {minX, minZ, maxX, maxZ}.  Absent = the default
    // box (the authored zones' bounding box + ZoneTuning::DefaultRegionPad;
    // no region without a positioned zone).  Malformed (not 4 numbers, or
    // min >= max) = the default box, with one WARN when warn is set.
    static ZoneTuning::PlayRegion ReadPlayRegion(const ParamEntry* zonesCfg, bool warn);
    // The play region clause of the base detection INFO line: "playRegion
    // authored [minX, minZ, maxX, maxZ]", "default box for bases only [..]"
    // or "no playRegion (whole map)".
    static RString PlayRegionInfo(const ZoneTuning::PlayRegion& r);
    // The town names a campaign on this zones config + world Names class
    // carries as CITY zones, in zone-table order: the authored CITY zones
    // (config order), then - only when seedCities is set - every Names town
    // SeedCityZones would seed (same filter, same name / 300 m dedup against
    // the authored zones and the towns already collected, same MaxZones
    // cap).  The new-game START TOWN cycler lists exactly this, so a pick
    // always names a zone of the launched campaign.  Either entry may be null.
    // A Names town outside an AUTHORED playRegion[] is not listed
    // (SeedCityZones skips it too); the default box never bounds towns.  Menu time has no landscape, so a Names town the runtime base
    // seed absorbs (within 300 m of a seeded AIRFIELD / SEAPORT) is still
    // listed; such a pick falls back to the camp like any unseeded town.
    static void CollectTownNames(const ParamEntry* zonesCfg, const ParamEntry* namesCfg, AutoArray<RString>& out);

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
    // the last EvaluateTick's bubble census (gmSimDiag)
    int LastTickInBubble() const { return _lastTickInBubble; }
    int LastTickRemote() const { return _lastTickRemote; }
    // the player as the last EvaluateTick saw him (the S5 deposit bubble of
    // RevertTown's refill); false before the first tick
    bool LastTickPlayer(float& x, float& z) const
    {
        x = _tickPlayerX;
        z = _tickPlayerZ;
        return _tickPlayerValid;
    }
    // L10: a reserve write outside EvaluateTick (RevertTown, RevertBase, a
    // wave's allocation) through the S5 deposit bubble.  The bubble's player
    // is the last tick's; before the first tick since a load or activation
    // it is the world's living player now (the engine registry), and with
    // neither known the write is a pending deposit, which the next tick
    // releases once the player is away or the points are hidden.  True
    // when the write went pending.
    bool AddReserveNow(int zone, int bodies, int originZone);
    bool TickSeen() const { return _tickSeen; }
    // the player as a zone tick sees him (EvaluateTick calls it; unit tests
    // stand in for the tick with it)
    void NoteTickPlayer(bool valid, float x, float z)
    {
        _tickPlayerValid = valid;
        _tickPlayerX = x;
        _tickPlayerZ = z;
        _tickSeen = true;
    }
    // sample GatherSimDiag and log one [gmsim] INFO line, plus a WARN per
    // breached budget; returns the SimBudgetBreach bits
    int LogSimDiag();

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
        int seenDay = 0;
        int seenMinute = 0;
        float recoverClock = 0;
        bool garrisonExempt = false;
        int flipDay = 0;
        int flipMinute = 0;
        int pending = 0;       // S5 pending deposit
        RString pendingOrigin; // its origin zone by name (empty: none or mixed)
        float occCapture = 0;
        float standing = -1; // -1: absent in the save, the config value stays
        float struckUntil = 0;
        int lostCount = 0;
        int settlement = 0; // 0: absent in the save (or never probed), the row keeps its value

        LSError Serialize(ParamArchive& ar);
    };

    void LoadZones(const ParamEntry& cfg);
    void LoadFactions(const ParamEntry& cfg);
    // plan-15 resolution pass: validate every classname-valued descriptor
    // entry against the loaded data package, substituting per the resolution
    // table (tiers fall back tier-to-tier then side fallback; roles fall back
    // to the tier rifleman; vehicles are dropped and the ladder compacted;
    // weapon/mag keys fall back to baseWeapon / the matching weapon key)
    void ResolveFactionClasses(const ClassProbe& probe);
    // shared tiers[] index selection for FactionTierClass / FactionSquad
    static int TierIndex(const FactionRecord& f, float warLevel);
    void SeedCityZones(const ParamEntry& namesCfg, const SettlementProbe* settlement);
    // issue #76: run the base detector over the probe's world (plus the
    // Names towns, the authored zones and the playRegion), keep every
    // AIRFIELD / SEAPORT candidate, and add the SEED ones as occupier zones
    // when seedBases is on.  Runs after LoadZones, before SeedCityZones.
    void SeedBaseZones(const ParamEntry* namesCfg, const BaseWorldProbe& probe);
    // side match first, then faction class name (both case-insensitive)
    const FactionRecord* FindFaction(const char* sideOrClass) const;
    // class name only - the key FindFaction reaches last and FindFactionForSide
    // needs first
    const FactionRecord* FindFactionByClass(const char* className) const;
    FactionRecord* FindFactionByClassMutable(const char* className);
    // map a raw selection to a concrete side via the faction table;
    // unmatched/empty keeps the current value
    void ResolveSides(const char* selOccupier, const char* selResistance);
    // pin the resistance to _tuning.playerSide and move a colliding occupier
    // off it (sideTwin where the data ships one, rebase where it does not);
    // a no-op when the template authors no playerSide
    void ResolveSideCollisions(const ParamEntry* factionsCfg);
    // force the selected descriptors' side fields to match the resolved
    // campaign sides; idempotent
    void RebindFactionSides();
    // split the one record a mirror-match pick aliases (occupier faction ==
    // resistance faction on two different sides) into two, so RebindFactionSides
    // has a record per side to write; idempotent.  Called by RebindFactionSides.
    void DivergeAliasedFactions();
    // weld the campaign's two sides into mutual enemies (WEST and GUER are
    // ALLIES by default - see the definition).  True = settled, never call
    // again this load; that covers both a successful weld and a PERMANENT
    // refusal to weld (an unresolved same-side campaign, or a side string no
    // TargetSide parses), which no later tick could fix.  False = the world or
    // its centers are not built yet, retry next tick.
    bool ApplyCampaignFriendship();
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
    // the base detector's last run (issue #76): transient, config-rebuilt by
    // every LoadFromParams, never serialized
    std::vector<BaseCandidate> _detectedBases;
    std::vector<int> _seededBases; // indices into _detectedBases, seeding order
    bool _baseDetectionRan = false;
    const BaseWorldProbe* _baseProbeOverride = nullptr; // test seam, not owned
    // campaign faction sides; defaults match the Phase-1 Demo campaign
    RString _occupierSide = RString("EAST");
    RString _resistanceSide = RString("GUER");
    // the descriptors those sides field (see OccupierFaction)
    RString _occupierFaction;
    RString _resistanceFaction;
    RString _handlers[NZoneEventTypes];
    float _accum = 0;
    float _simDiagAccum = 0;   // [gmsim] cadence
    int _lastTickInBubble = 0; // transient census of the last EvaluateTick
    int _lastTickRemote = 0;
    // transient: the player as the last EvaluateTick saw him (LastTickPlayer)
    bool _tickPlayerValid = false;
    // L10: an EvaluateTick ran since the last Clear (a load clears)
    bool _tickSeen = false;
    float _tickPlayerX = 0;
    float _tickPlayerZ = 0;
    // deserialized rows waiting for the mission config (applied on the
    // second load pass, once description.ext has been reparsed)
    AutoArray<ZoneSaveState> _pending;
    // deserialized faction sides, same two-pass parking (scalar archive
    // reads happen on the first pass only; applied on the second)
    RString _pendingOccupierSide;
    RString _pendingResistanceSide;
    RString _pendingOccupierFaction;
    RString _pendingResistanceFaction;
    // QueueEvent's events, drained by the next Simulate tick
    AutoArray<ZoneEventRecord> _queued;
    // queued campaignLoaded notification (see MarkCampaignLoaded)
    bool _pendingLoaded = false;
    int _loadedSaveVersion = 0;
    // transient, NEVER serialized: the friendship matrix is world state, so a
    // loaded campaign re-welds it on its next tick rather than restoring it
    bool _friendshipApplied = false;
};

// Map a side string to its live command center WITHOUT creating one (an
// absent center means the side has no units yet) - the non-creating sibling
// of GarrisonCache's EnsureCenter.  Null when the side name or world is
// unknown.  Shared by ZoneRegistry (resistance presence) and AlertMachine
// (occupier perception).
AICenter* FindSideCenter(const char* sideName);

// The creating sibling: a side with no mission-placed units has no command
// center yet (Abel's mission.sqm seeds only a GUER group), so the occupier's
// center has to be created on demand - mirrors CenterCreate
// (GameStateExtWorldConfig.cpp).  Null on an unknown side name or no world.
AICenter* EnsureSideCenter(const char* sideName);

// A fresh empty AIGroup on the center, sent an Arcade mission and announced
// to the network layer - mirrors GroupCreate (GameStateExtWorldConfig.cpp).
// Null on a null center or when the center is at MaxGroups.  Shared by
// GarrisonCache (garrisons), Traffic (crews), Market (dealers) and
// GuerrillaBase; every Guerrilla spawner builds its groups through this one
// helper so the group bookkeeping stays in one place.
AIGroup* CreateSideGroup(AICenter* center);

// Script-owned campaign war level ("gmwarlevel"); 1 when the world, game
// state or variable is missing (matches the init.sqs default).  Shared by
// GarrisonCache (garrison tiers) and Traffic (convoy scaling, curfew).
float ReadWarLevel();

} // namespace Guerrilla
} // namespace Poseidon
