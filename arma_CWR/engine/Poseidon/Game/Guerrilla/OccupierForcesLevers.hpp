#pragma once

// The eight author levers of the occupier forces director (OCCUPIER-FORCES.md
// 16, phase 3 of round 4, owner decision 2 of 2026-10-16).
//
// About 75 tuning keys fed the director; almost none were authored.  They
// fold into eight CfgGuerrillaZones keys, each with four preset values
// (Easy / Normal / Hard / Brutal) that the difficulty key picks:
//
//   forcesDifficulty   0 Easy, 1 Normal (default), 2 Hard, 3 Brutal: picks the
//                      preset value of every lever below
//   forcesCadence      s per operation while STRONG          1200 / 600 / 420 / 300
//   forcesStrength     poolStart scale                       0.75 / 1 / 1.25 / 1.5
//   forcesRecovery     h, an exhausted pool back to full     8 / 4 / 3 / 2
//   forcesReach        s of ETA before it rides or flies     3000 / 1500 / 1200 / 900
//   forcesAggression   heat, lethality, first-town, strikes  0.75 / 1 / 1.25 / 1.5
//   forcesWaves        the wave set, by level 0..3           0 / 1 / 2 / 3
//   forcesMinimums     retake bodies                         3 / 4 / 6 / 8
//   forcesConcurrency  born operations in flight, STRONG     2 / 3 / 4 / 5
//
// An authored lever key overrides its preset value (its own unit, repaired
// into range).  Normal derives exactly today's defaults, field by field.
// Every old key (forcesHeatFull, forcesTokenInterval[] ...) still reads, as an
// EXPERT OVERRIDE of the field its lever derived: one WARN per authored key
// and load naming the lever (or "a constant").  Tuning is not saved: it is
// re-read from the mission config on load.
//
// The derivations (Derive):
//   cadence c      tokenInterval[] = {c, 1.75c, 3c, 0}; townCooldown = 3c
//   strength s     poolStart = max(forcesPoolMin, round(s x (R + 2K)))
//                  (OccupierForces::PoolStartOf; read at activation only)
//   recovery h     refillHours = h
//   reach r        airEta = r; slowEta = 0.6r
//   aggression a   heatFull = rearmHeat = 40/a; urgentHeat = 50/a, its exit
//                  0.7 x that; lethalityMargin = 1.2a (with
//                  counterAttackDefenderWeight 3, L_req = 3.6a per fighter);
//                  strikeMinFighters = round(3/a); counterAttackFirstTownChance
//                  = 0.4a; ZoneRegistry heatCapSpike = 40/a
//   waves w        thresholds / share / lead / retake share / war level per
//                  wave from WaveSets[w]; two thresholds and the final stand
//                  at every level (D7: three big moments)
//   minimums m     retakeMin = alarmMin = m; hqAssaultMin = spareCapFloor = m + 2;
//                  spareCap on
//   concurrency n  opsMaxStrong = n; opsMax = n - 1; forcesLiveHullsMax = 2n;
//                  counterAttackMaxLive = n - 1

namespace Poseidon
{
class ParamEntry;

namespace Guerrilla
{
struct OccupierForcesTuning;
struct CounterAttackTuning;

// forcesDifficulty values
enum ForcesPreset
{
    ForcesEasy,
    ForcesNormal,
    ForcesHard,
    ForcesBrutal,
    NForcesPresets
};

struct OccupierForcesLevers
{
    // the four presets of each lever (Easy, Normal, Hard, Brutal)
    static constexpr float CadencePresets[NForcesPresets] = {1200.0f, 600.0f, 420.0f, 300.0f};
    static constexpr float StrengthPresets[NForcesPresets] = {0.75f, 1.0f, 1.25f, 1.5f};
    static constexpr float RecoveryPresets[NForcesPresets] = {8.0f, 4.0f, 3.0f, 2.0f};
    static constexpr float ReachPresets[NForcesPresets] = {3000.0f, 1500.0f, 1200.0f, 900.0f};
    static constexpr float AggressionPresets[NForcesPresets] = {0.75f, 1.0f, 1.25f, 1.5f};
    static constexpr int WavesPresets[NForcesPresets] = {0, 1, 2, 3};
    static constexpr int MinimumsPresets[NForcesPresets] = {3, 4, 6, 8};
    static constexpr int ConcurrencyPresets[NForcesPresets] = {2, 3, 4, 5};

    // forcesWaves levels: two control thresholds (plus the final stand) at
    // every level, the wave's share of poolStart, the inbound lead (s), the
    // strikes' share of the wave, the occupier war level per landed wave
    struct WaveSet
    {
        float thresholds[2];
        float share;
        float lead;
        float retakeShare;
        float warLevel;
    };
    static constexpr WaveSet WaveSets[NForcesPresets] = {
        {{0.70f, 0.45f}, 0.35f, 900.0f, 0.4f, 0.75f},
        {{0.75f, 0.50f}, 0.5f, 600.0f, 0.5f, 1.0f},
        {{0.80f, 0.60f}, 0.6f, 450.0f, 0.6f, 1.25f},
        {{0.85f, 0.65f}, 0.75f, 300.0f, 0.7f, 1.5f},
    };

    // the base numbers the aggression lever divides or scales (its Normal)
    static constexpr float BaseHeatFull = 40.0f;
    static constexpr float BaseUrgentHeat = 50.0f;
    static constexpr float BaseLethalityMargin = 1.2f;
    static constexpr float BaseFirstTownChance = 0.4f;
    static constexpr int BaseStrikeMinFighters = 3;

    int difficulty = ForcesNormal; // forcesDifficulty, 0..3
    float cadence = 600.0f;        // forcesCadence, 60..7200 s
    float strength = 1.0f;         // forcesStrength, 0.25..4
    float recovery = 4.0f;         // forcesRecovery, 0.5..48 h
    float reach = 1500.0f;         // forcesReach, 0..86400 s (0 = the arrival rule off)
    float aggression = 1.0f;       // forcesAggression, 0.25..4
    int waves = 1;                 // forcesWaves, 0..3
    int minimums = 4;              // forcesMinimums, 1..10
    int concurrency = 3;           // forcesConcurrency, 2..6

    // every lever at the preset (clamped into 0..3)
    static OccupierForcesLevers Preset(int difficulty);
    // forcesDifficulty, then every authored lever key over its preset value
    // (repaired into range; logged when log is set: OccupierForces logs,
    // CounterAttack and ZoneRegistry read the same keys quietly); null = Normal
    static OccupierForcesLevers Read(const ParamEntry* zonesCfg, bool log = true);
    // a gmForcesTune lever: "difficulty" (every lever at that preset) or a
    // lever's name without the prefix ("cadence" .. "concurrency"), clamped
    // into the config range.  False: not a lever name
    bool Set(const char* name, float v);

    // the derived numbers (the WaveSet and ZoneRegistry's capture heat)
    float HeatFull() const;
    float HeatCapSpike() const;
    // the director's fields this set derives (the rest of t is untouched)
    void Derive(OccupierForcesTuning& t) const;
    // CounterAttack's: counterAttackMaxLive, forcesLiveHullsMax,
    // counterAttackFirstTownChance
    void Derive(CounterAttackTuning& t) const;
};

// The expert-override seam: when key is authored in cfg, one WARN ("<who>:
// <key> is an expert override of the <lever> lever", or "... of a constant"
// for lever null) and true.  Every old key has exactly one reader, so a load
// warns once per authored key.
bool ForcesExpertOverride(const ParamEntry& cfg, const char* key, const char* lever, const char* who);

// CounterAttack::LoadFromParams' share: the levers' derived fields, then the
// expert overrides counterAttackFirstTownChance, counterAttackMaxLive,
// forcesLiveHullsMax and forcesSortieTime
void LoadCounterAttackLevers(CounterAttackTuning& t, const ParamEntry& cfg);
// ZoneRegistry's capture heat (heatCapSpike): 40 / forcesAggression, or the
// authored heatCapSpike as an expert override
float ZoneHeatCapSpike(const ParamEntry& cfg, float def);

} // namespace Guerrilla
} // namespace Poseidon
