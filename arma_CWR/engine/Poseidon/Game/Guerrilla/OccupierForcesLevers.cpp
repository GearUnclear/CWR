// The eight author levers (OccupierForcesLevers.hpp, OCCUPIER-FORCES.md 16)
// and the director's config reader, OccupierForces::LoadFromParams: the
// levers derive the fields, then every old key reads as an expert override.

#include <Poseidon/Game/Guerrilla/OccupierForcesLevers.hpp>

#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>
#include <Poseidon/Game/Guerrilla/ForcePackages.hpp> // SetRoadFactor (the theatre's one road factor)
#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>

#include <Poseidon/IO/ParamFileExt.hpp>

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <string.h>

namespace Poseidon::Guerrilla
{

namespace
{

float ClampF(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

int ClampI(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

int RoundI(float v)
{
    return (int)floorf(v + 0.5f);
}

// read a key and repair it into [lo, hi] (logged with who's prefix when log)
float ReadRangedAs(const char* who, const ParamEntry& cfg, const char* key, float def, float lo, float hi,
                   bool log = true)
{
    const float v = cfg.ReadValue(key, def);
    if (v < lo || v > hi)
    {
        const float fixed = v < lo ? lo : hi;
        if (log)
        {
            LOG_WARN(Core, "{}: {} = {} repaired to {} (range {}..{})", who, key, (double)v, (double)fixed, (double)lo,
                     (double)hi);
        }
        return fixed;
    }
    return v;
}

float RepairElement(const char* key, int index, float v, float lo, float hi)
{
    if (v < lo || v > hi)
    {
        const float fixed = v < lo ? lo : hi;
        LOG_WARN(Core, "OccupierForces: {}[{}] = {} repaired to {} (range {}..{})", key, index, (double)v,
                 (double)fixed, (double)lo, (double)hi);
        return fixed;
    }
    return v;
}

// the director's old keys: the derived value unless the key is authored
// (then one WARN and the ranged read)
float Expert(const ParamEntry& c, const char* key, const char* lever, float derived, float lo, float hi)
{
    return ForcesExpertOverride(c, key, lever, "OccupierForces")
               ? ReadRangedAs("OccupierForces", c, key, derived, lo, hi)
               : derived;
}

int ExpertInt(const ParamEntry& c, const char* key, const char* lever, int derived, float lo, float hi)
{
    return ForcesExpertOverride(c, key, lever, "OccupierForces")
               ? RoundI(ReadRangedAs("OccupierForces", c, key, (float)derived, lo, hi))
               : derived;
}

bool ExpertBool(const ParamEntry& c, const char* key, const char* lever, bool derived)
{
    return ForcesExpertOverride(c, key, lever, "OccupierForces")
               ? RoundI(ReadRangedAs("OccupierForces", c, key, derived ? 1.0f : 0.0f, 0.0f, 1.0f)) != 0
               : derived;
}

// the lever names in the WARN lines
constexpr const char* LCadence = "forcesCadence";
constexpr const char* LStrength = "forcesStrength";
constexpr const char* LRecovery = "forcesRecovery";
constexpr const char* LReach = "forcesReach";
constexpr const char* LAggression = "forcesAggression";
constexpr const char* LWaves = "forcesWaves";
constexpr const char* LMinimums = "forcesMinimums";
constexpr const char* LConcurrency = "forcesConcurrency";

} // namespace

// ---------------------------------------------------------------------------
// the levers
// ---------------------------------------------------------------------------

OccupierForcesLevers OccupierForcesLevers::Preset(int difficulty)
{
    const int d = ClampI(difficulty, 0, NForcesPresets - 1);
    OccupierForcesLevers l;
    l.difficulty = d;
    l.cadence = CadencePresets[d];
    l.strength = StrengthPresets[d];
    l.recovery = RecoveryPresets[d];
    l.reach = ReachPresets[d];
    l.aggression = AggressionPresets[d];
    l.waves = WavesPresets[d];
    l.minimums = MinimumsPresets[d];
    l.concurrency = ConcurrencyPresets[d];
    return l;
}

OccupierForcesLevers OccupierForcesLevers::Read(const ParamEntry* zonesCfg, bool log)
{
    if (!zonesCfg)
    {
        return Preset(ForcesNormal);
    }
    const ParamEntry& c = *zonesCfg;
    const char* who = "OccupierForces";
    OccupierForcesLevers l =
        Preset(RoundI(ReadRangedAs(who, c, "forcesDifficulty", (float)ForcesNormal, 0.0f, 3.0f, log)));
    l.cadence = ReadRangedAs(who, c, LCadence, l.cadence, 60.0f, 7200.0f, log);
    l.strength = ReadRangedAs(who, c, LStrength, l.strength, 0.25f, 4.0f, log);
    l.recovery = ReadRangedAs(who, c, LRecovery, l.recovery, 0.5f, 48.0f, log);
    l.reach = ReadRangedAs(who, c, LReach, l.reach, 0.0f, 86400.0f, log);
    l.aggression = ReadRangedAs(who, c, LAggression, l.aggression, 0.25f, 4.0f, log);
    l.waves = RoundI(ReadRangedAs(who, c, LWaves, (float)l.waves, 0.0f, 3.0f, log));
    l.minimums = RoundI(ReadRangedAs(who, c, LMinimums, (float)l.minimums, 1.0f, 10.0f, log));
    l.concurrency = RoundI(ReadRangedAs(who, c, LConcurrency, (float)l.concurrency, 2.0f, 6.0f, log));
    return l;
}

bool OccupierForcesLevers::Set(const char* name, float v)
{
    if (!name)
    {
        return false;
    }
    if (stricmp(name, "difficulty") == 0)
    {
        *this = Preset(RoundI(ClampF(v, 0.0f, 3.0f)));
    }
    else if (stricmp(name, "cadence") == 0)
    {
        cadence = ClampF(v, 60.0f, 7200.0f);
    }
    else if (stricmp(name, "strength") == 0)
    {
        strength = ClampF(v, 0.25f, 4.0f);
    }
    else if (stricmp(name, "recovery") == 0)
    {
        recovery = ClampF(v, 0.5f, 48.0f);
    }
    else if (stricmp(name, "reach") == 0)
    {
        reach = ClampF(v, 0.0f, 86400.0f);
    }
    else if (stricmp(name, "aggression") == 0)
    {
        aggression = ClampF(v, 0.25f, 4.0f);
    }
    else if (stricmp(name, "waves") == 0)
    {
        waves = RoundI(ClampF(v, 0.0f, 3.0f));
    }
    else if (stricmp(name, "minimums") == 0)
    {
        minimums = RoundI(ClampF(v, 1.0f, 10.0f));
    }
    else if (stricmp(name, "concurrency") == 0)
    {
        concurrency = RoundI(ClampF(v, 2.0f, 6.0f));
    }
    else
    {
        return false;
    }
    return true;
}

float OccupierForcesLevers::HeatFull() const
{
    return ClampF(BaseHeatFull / aggression, 1.0f, 100.0f);
}

float OccupierForcesLevers::HeatCapSpike() const
{
    return ClampF(BaseHeatFull / aggression, 0.0f, 100.0f);
}

void OccupierForcesLevers::Derive(OccupierForcesTuning& t) const
{
    t.levers = *this;
    // cadence: the STRONG interval IS the cadence; the other bands keep
    // today's ratios (600 / 1050 / 1800), a target rests three intervals
    t.tokenInterval[OFBandStrong] = cadence;
    t.tokenInterval[OFBandStretched] = cadence * 1.75f;
    t.tokenInterval[OFBandThin] = cadence * 3.0f;
    t.tokenInterval[OFBandSpent] = 0.0f;
    t.townCooldown = cadence * 3.0f;
    // recovery: exhausted to full (D15 at Normal)
    t.refillHours = recovery;
    // reach: the arrival rule (6.5); 3/5 rather than 0.6 keeps 900 exact
    t.airEta = reach;
    t.slowEta = reach * 3.0f / 5.0f;
    // aggression: candidate heat, the re-arm at it, urgency, L_req, strikes
    t.heatFull = HeatFull();
    t.rearmHeat = t.heatFull;
    t.urgentHeat = ClampF(BaseUrgentHeat / aggression, 1.0f, 100.0f);
    t.urgentExit = t.urgentHeat * 7.0f / 10.0f;
    t.lethalityMargin = ClampF(BaseLethalityMargin * aggression, 0.0f, 3.0f);
    const int strike = RoundI((float)BaseStrikeMinFighters / aggression);
    t.strikeMinFighters = strike < 1 ? 1 : strike;
    // waves: the level's set (two thresholds and the final stand: D7)
    const WaveSet& w = WaveSets[ClampI(waves, 0, NForcesPresets - 1)];
    t.nWaveThresholds = 2;
    t.waveThresholds[0] = w.thresholds[0];
    t.waveThresholds[1] = w.thresholds[1];
    for (int k = 2; k < OccupierForcesTuning::MaxWaveThresholds; k++)
    {
        t.waveThresholds[k] = 0.0f;
    }
    t.waveShare = w.share;
    t.waveLead = w.lead;
    t.waveRetakeShare = w.retakeShare;
    t.waveWarLevel = w.warLevel;
    // minimums: retake and alarm bodies, the HQ assault and the spare cap's floor
    t.retakeMin = minimums;
    t.alarmMin = minimums;
    t.hqAssaultMin = minimums + 2;
    t.spareCapFloor = minimums + 2;
    t.spareCap = true;
    // concurrency: born operations in flight
    t.opsMaxStrong = concurrency;
    t.opsMax = concurrency - 1;
}

void OccupierForcesLevers::Derive(CounterAttackTuning& t) const
{
    t.maxLive = concurrency - 1;
    t.forcesLiveHullsMax = 2 * concurrency;
    t.firstTownChance = ClampF(BaseFirstTownChance * aggression, 0.0f, 1.0f);
}

bool ForcesExpertOverride(const ParamEntry& cfg, const char* key, const char* lever, const char* who)
{
    if (!cfg.FindEntry(key))
    {
        return false;
    }
    if (lever)
    {
        LOG_WARN(Core,
                 "{}: {} is an expert override of the {} lever: it replaces the value the lever derives "
                 "(OCCUPIER-FORCES.md 16)",
                 who, key, lever);
    }
    else
    {
        LOG_WARN(Core, "{}: {} is an expert override of a constant (OCCUPIER-FORCES.md 16)", who, key);
    }
    return true;
}

void LoadCounterAttackLevers(CounterAttackTuning& t, const ParamEntry& c)
{
    OccupierForcesLevers::Read(&c, false).Derive(t);
    const char* who = "CounterAttack";
    if (ForcesExpertOverride(c, "counterAttackFirstTownChance", LAggression, who))
    {
        t.firstTownChance = ReadRangedAs(who, c, "counterAttackFirstTownChance", t.firstTownChance, 0.0f, 1.0f);
    }
    if (ForcesExpertOverride(c, "counterAttackMaxLive", LConcurrency, who))
    {
        t.maxLive = (int)ReadRangedAs(who, c, "counterAttackMaxLive", (float)t.maxLive, 0.0f, 8.0f);
    }
    if (ForcesExpertOverride(c, "forcesLiveHullsMax", LConcurrency, who))
    {
        t.forcesLiveHullsMax =
            (int)ReadRangedAs(who, c, "forcesLiveHullsMax", (float)t.forcesLiveHullsMax, 0.0f, 20.0f);
    }
    // phase 6A air (16): the strike time over target, a constant since the levers
    if (ForcesExpertOverride(c, "forcesSortieTime", nullptr, who))
    {
        t.sortieTime = ReadRangedAs(who, c, "forcesSortieTime", t.sortieTime, 60.0f, 1800.0f);
    }
}

float ZoneHeatCapSpike(const ParamEntry& cfg, float def)
{
    const OccupierForcesLevers l = OccupierForcesLevers::Read(&cfg, false);
    const float derived = ClampF(def / l.aggression, 0.0f, 100.0f);
    if (ForcesExpertOverride(cfg, "heatCapSpike", LAggression, "ZoneRegistry"))
    {
        return cfg.ReadValue("heatCapSpike", derived);
    }
    return derived;
}

// ---------------------------------------------------------------------------
// the director's config reader
// ---------------------------------------------------------------------------

void OccupierForces::LoadFromParams(const ParamEntry* zonesCfg)
{
    _tuning = OccupierForcesTuning();
    if (!zonesCfg)
    {
        return;
    }
    const ParamEntry& c = *zonesCfg;
    OccupierForcesTuning& t = _tuning;
    t.enabled = c.ReadValue("occupierForces", OccupierForcesTuning::KeyDefault) != 0.0f;
    // the eight levers (and forcesDifficulty) derive their fields first
    OccupierForcesLevers::Read(zonesCfg).Derive(t);

    // every old key below is an expert override of what the levers derived
    // (or of its constant): read only when authored, one WARN each
    t.poolMin = ExpertInt(c, "forcesPoolMin", nullptr, t.poolMin, 0.0f, 200.0f);
    t.poolPerTown = ExpertInt(c, "forcesPoolPerTown", LStrength, t.poolPerTown, 0.0f, 8.0f);
    t.refillHours = Expert(c, "forcesPoolRefillHours", LRecovery, t.refillHours, 0.5f, 48.0f);
    t.poolFloor = Expert(c, "forcesPoolFloor", nullptr, t.poolFloor, 0.0f, 1.0f);
    t.directorInterval = Expert(c, "forcesDirectorInterval", nullptr, t.directorInterval, 10.0f, 600.0f);
    t.tokensMax = ExpertInt(c, "forcesTokensMax", nullptr, t.tokensMax, 0.0f, 6.0f);
    t.controlMilWeight = Expert(c, "forcesControlMilWeight", nullptr, t.controlMilWeight, 0.0f, 1.0f);
    t.waveWarLevel = Expert(c, "forcesWaveWarLevel", LWaves, t.waveWarLevel, 0.0f, 3.0f);
    t.wakeTowns = ExpertInt(c, "forcesWakeTowns", nullptr, t.wakeTowns, 2.0f, 99.0f);
    t.opsMax = ExpertInt(c, "forcesOpsMax", LConcurrency, t.opsMax, 0.0f, 6.0f);
    t.opsMaxStrong = ExpertInt(c, "forcesOpsMaxStrong", LConcurrency, t.opsMaxStrong, 0.0f, 6.0f);
    t.heatFull = Expert(c, "forcesHeatFull", LAggression, t.heatFull, 1.0f, 100.0f);
    t.urgentHeat = Expert(c, "forcesUrgentHeat", LAggression, t.urgentHeat, 1.0f, 100.0f);
    t.townCooldown = Expert(c, "forcesTownCooldown", LCadence, t.townCooldown, 0.0f, 86400.0f);
    t.rearmHeat = Expert(c, "forcesRearmHeat", LAggression, t.rearmHeat, 0.0f, 100.0f);
    t.pushoutQuiet = Expert(c, "forcesPushoutQuiet", nullptr, t.pushoutQuiet, 0.0f, 86400.0f);
    t.retakeMin = ExpertInt(c, "forcesRetakeMin", LMinimums, t.retakeMin, 1.0f, 12.0f);
    t.lethalityMargin = Expert(c, "forcesLethalityMargin", LAggression, t.lethalityMargin, 1.0f, 3.0f);
    t.urgentCostShare = Expert(c, "forcesUrgentCostShare", nullptr, t.urgentCostShare, 1.0f, 3.0f);
    t.intelDecay = Expert(c, "forcesIntelDecay", nullptr, t.intelDecay, 1.0f, 86400.0f);
    // 2026-10-15: the spare cap (7.2) and the arrival rule (6.5)
    t.spareCap = ExpertBool(c, "forcesSpareCap", LMinimums, t.spareCap);
    t.spareCapFloor = ExpertInt(c, "forcesSpareCapFloor", LMinimums, t.spareCapFloor, 1.0f, 12.0f);
    t.slowEta = Expert(c, "forcesSlowEta", LReach, t.slowEta, 0.0f, 86400.0f);
    t.slowCostShare = Expert(c, "forcesSlowCostShare", nullptr, t.slowCostShare, 1.0f, 5.0f);
    t.airEta = Expert(c, "forcesAirEta", LReach, t.airEta, 0.0f, 86400.0f);
    // 2026-10-16: the road quote (6.5)
    t.roadFactor = Expert(c, "forcesRoadFactor", nullptr, t.roadFactor, 1.0f, 5.0f);
    ForcePackages::SetRoadFactor(t.roadFactor); // the theatre's one road factor (quote and march)
    // phase 4: the HQ assault and the waves (16)
    t.hqAssaultMin = ExpertInt(c, "forcesHqAssaultMin", LMinimums, t.hqAssaultMin, 1.0f, 12.0f);
    t.waveDwell = ExpertInt(c, "forcesControlDwell", nullptr, t.waveDwell, 1.0f, 10.0f);
    if (ForcesExpertOverride(c, "forcesWaveFinal", nullptr, "OccupierForces"))
    {
        t.waveFinal = c.ReadValue("forcesWaveFinal", t.waveFinal ? 1.0f : 0.0f) != 0.0f;
    }
    t.waveShare = Expert(c, "forcesWaveShare", LWaves, t.waveShare, 0.0f, 2.0f);
    t.waveMin = ExpertInt(c, "forcesWaveMin", nullptr, t.waveMin, 0.0f, 200.0f);
    t.waveMax = ExpertInt(c, "forcesWaveMax", nullptr, t.waveMax, 0.0f, 400.0f);
    t.waveLead = Expert(c, "forcesWaveLead", LWaves, t.waveLead, 0.0f, 7200.0f);
    t.waveLeadFinal = Expert(c, "forcesWaveLeadFinal", nullptr, t.waveLeadFinal, 0.0f, 7200.0f);
    t.waveRetakeShare = Expert(c, "forcesWaveRetakeShare", LWaves, t.waveRetakeShare, 0.0f, 1.0f);
    t.waveOps = ExpertInt(c, "forcesWaveOps", nullptr, t.waveOps, 0.0f, 8.0f);
    t.waveCluster = Expert(c, "forcesWaveCluster", nullptr, t.waveCluster, 100.0f, 20000.0f);
    t.waveSurge = ExpertInt(c, "forcesWaveSurge", nullptr, t.waveSurge, 0.0f, 48.0f);
    // 2026-10-16 (decision 9): hull refunds over the ceiling bank as bodies
    // (1) or come back as a stored spare hull (0, the default); a test lever
    t.bankHulls = ExpertBool(c, "forcesBankHulls", nullptr, t.bankHulls);
    // author keys outside the levers (no WARN): the wave hub (geography) and
    // the director log toggle (a diagnostic)
    t.waveHub = c.ReadValue("forcesWaveHub", RString());
    t.log = RoundI(ReadRangedAs("OccupierForces", c, "forcesLog", t.log ? 1.0f : 0.0f, 0.0f, 1.0f)) != 0;

    // forcesTokenInterval[] (forcesCadence): one entry per band; a short
    // array keeps the lever-derived values of the bands it leaves out
    const ParamEntry* tokens = c.FindEntry("forcesTokenInterval");
    if (tokens && tokens->IsArray() && ForcesExpertOverride(c, "forcesTokenInterval", LCadence, "OccupierForces"))
    {
        const int n = tokens->GetSize();
        if (n > NOccupierBands)
        {
            LOG_WARN(Core, "OccupierForces: forcesTokenInterval[] has {} entries, only the first {} are read", n,
                     (int)NOccupierBands);
        }
        for (int k = 0; k < n && k < NOccupierBands; k++)
        {
            t.tokenInterval[k] = RepairElement("forcesTokenInterval", k, (float)(*tokens)[k], 0.0f, 86400.0f);
        }
    }
    // forcesWaveThresholds[] (forcesWaves): at most MaxWaveThresholds control shares
    const ParamEntry* waves = c.FindEntry("forcesWaveThresholds");
    if (waves && waves->IsArray() && ForcesExpertOverride(c, "forcesWaveThresholds", LWaves, "OccupierForces"))
    {
        const int n = waves->GetSize();
        if (n > OccupierForcesTuning::MaxWaveThresholds)
        {
            LOG_WARN(Core, "OccupierForces: forcesWaveThresholds[] has {} entries, only the first {} are read", n,
                     OccupierForcesTuning::MaxWaveThresholds);
        }
        t.nWaveThresholds = n < OccupierForcesTuning::MaxWaveThresholds ? n : OccupierForcesTuning::MaxWaveThresholds;
        for (int k = 0; k < OccupierForcesTuning::MaxWaveThresholds; k++)
        {
            t.waveThresholds[k] =
                k < t.nWaveThresholds ? RepairElement("forcesWaveThresholds", k, (float)(*waves)[k], 0.0f, 1.0f) : 0.0f;
        }
    }
}

} // namespace Poseidon::Guerrilla
