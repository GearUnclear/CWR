#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>

#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>  // alert states, alertInterval (read only)
#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp>   // IsMilitaryType / IsAuthoredOccupier, the win latch
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp> // LaunchFunded, the listener, Roll01 / MakeSeed / Spare
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp> // live garrison bodies (a base's spare)
#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp> // HqZone: never a target
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/World/World.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/VehicleAI.hpp> // Target / TargetList (the sightings scan)

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include <chrono>

namespace Poseidon::Guerrilla
{

// Defined in OccupierForcesCommands.cpp (keeps the command TU in the link).
void EnsureOccupierForcesCommandsLinked();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
OccupierForces& OccupierForces::Instance()
{
    EnsureOccupierForcesCommandsLinked();
    static OccupierForces instance;
    return instance;
}
#pragma clang diagnostic pop

static int MinInt(int a, int b)
{
    return a < b ? a : b;
}

static int MaxInt(int a, int b)
{
    return a > b ? a : b;
}

static int RoundToInt(float v)
{
    return (int)floorf(v + 0.5f);
}

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------

void OccupierForces::Clear()
{
    _tuning = OccupierForcesTuning();
    _active = false;
    _restored = false;
    _activationTried = false;
    _pool = 0;
    _poolStart = 0;
    _R = 0;
    _G = 0;
    _roster.Clear();
    _towns.Clear();
    _excluded.Clear();
    _townsEver.Clear();
    _urgent.Clear();
    _redPaid.Clear();
    _cooldowns.Clear();
    _sightings.Clear();
    _ops.Clear();
    _clock = 0;
    _scanAccum = 0;
    _alertPrev.Clear();
    _basesDeferredLogged = false;
    _catalogueBuilt = false;
    _catalogue = ForcePackageCatalogue();
    _posture = OFDormant;
    _tokens = 0;
    _tokenClock = 0;
    _directorAccum = 0;
    _opCounter = 0;
    _refillAccum = 0;
    _band = OFBandStrong;
    _control = 1.0f;
    _wavesFired = 0;
    _finalFired = false;
    _wavesLanded = 0;
    _waveRequests = 0;
    _lost = 0;
    _refillTotal = 0;
    _wavesTotal = 0;
    _legacySave = false;
    _meanStanding = DefaultStanding;
    _logClock = LogQuiet;
    _logSuppressed = 0;
}

void OccupierForces::InitMission()
{
    Clear();
    LoadFromConfig();
    // the director hears CounterAttack's flips and operation events (the
    // process-wide instance only: a unit test's local service never registers)
    if (this == &Instance())
    {
        CounterAttack::SetListener(this);
    }
}

void OccupierForces::LoadFromConfig()
{
    const ParamEntry* zones = ExtParsMission.FindEntry("CfgGuerrillaZones");
    if (!zones)
    {
        zones = Pars.FindEntry("CfgGuerrillaZones");
    }
    LoadFromParams(zones);
}

// read a key and repair it into [lo, hi] (logged), the CounterAttack idiom
static float ReadRanged(const ParamEntry& cfg, const char* key, float def, float lo, float hi)
{
    const float v = cfg.ReadValue(key, def);
    if (v < lo || v > hi)
    {
        const float fixed = v < lo ? lo : hi;
        LOG_WARN(Core, "OccupierForces: {} = {} repaired to {} (range {}..{})", key, (double)v, (double)fixed,
                 (double)lo, (double)hi);
        return fixed;
    }
    return v;
}

static float RepairElement(const char* key, int index, float v, float lo, float hi)
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

void OccupierForces::LoadFromParams(const ParamEntry* zonesCfg)
{
    _tuning = OccupierForcesTuning();
    if (!zonesCfg)
    {
        return;
    }
    const ParamEntry& c = *zonesCfg;
    OccupierForcesTuning& t = _tuning;
    t.enabled = c.ReadValue("occupierForces", 0.0f) != 0.0f;
    t.poolMin = RoundToInt(ReadRanged(c, "forcesPoolMin", (float)t.poolMin, 0.0f, 200.0f));
    t.poolPerTown = RoundToInt(ReadRanged(c, "forcesPoolPerTown", (float)t.poolPerTown, 0.0f, 8.0f));
    t.refillHours = ReadRanged(c, "forcesPoolRefillHours", t.refillHours, 0.5f, 48.0f);
    t.poolFloor = ReadRanged(c, "forcesPoolFloor", t.poolFloor, 0.0f, 1.0f);
    t.directorInterval = ReadRanged(c, "forcesDirectorInterval", t.directorInterval, 10.0f, 600.0f);
    t.tokensMax = RoundToInt(ReadRanged(c, "forcesTokensMax", (float)t.tokensMax, 0.0f, 6.0f));
    t.controlMilWeight = ReadRanged(c, "forcesControlMilWeight", t.controlMilWeight, 0.0f, 1.0f);
    t.waveWarLevel = ReadRanged(c, "forcesWaveWarLevel", t.waveWarLevel, 0.0f, 3.0f);
    t.wakeTowns = RoundToInt(ReadRanged(c, "forcesWakeTowns", (float)t.wakeTowns, 2.0f, 99.0f));
    t.opsMax = RoundToInt(ReadRanged(c, "forcesOpsMax", (float)t.opsMax, 0.0f, 6.0f));
    t.heatFull = ReadRanged(c, "forcesHeatFull", t.heatFull, 1.0f, 100.0f);
    t.urgentHeat = ReadRanged(c, "forcesUrgentHeat", t.urgentHeat, 1.0f, 100.0f);
    t.townCooldown = ReadRanged(c, "forcesTownCooldown", t.townCooldown, 0.0f, 86400.0f);
    t.pushoutQuiet = ReadRanged(c, "forcesPushoutQuiet", t.pushoutQuiet, 0.0f, 86400.0f);
    t.retakeMin = RoundToInt(ReadRanged(c, "forcesRetakeMin", (float)t.retakeMin, 1.0f, 12.0f));
    t.lethalityMargin = ReadRanged(c, "forcesLethalityMargin", t.lethalityMargin, 1.0f, 3.0f);
    t.urgentCostShare = ReadRanged(c, "forcesUrgentCostShare", t.urgentCostShare, 1.0f, 3.0f);
    t.intelDecay = ReadRanged(c, "forcesIntelDecay", t.intelDecay, 1.0f, 86400.0f);

    // forcesTokenInterval[]: one entry per band; a short array keeps the
    // defaults of the bands it leaves out
    const ParamEntry* tokens = c.FindEntry("forcesTokenInterval");
    if (tokens && tokens->IsArray())
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
    // forcesWaveThresholds[]: at most MaxWaveThresholds control shares
    const ParamEntry* waves = c.FindEntry("forcesWaveThresholds");
    if (waves && waves->IsArray())
    {
        const int n = waves->GetSize();
        if (n > OccupierForcesTuning::MaxWaveThresholds)
        {
            LOG_WARN(Core, "OccupierForces: forcesWaveThresholds[] has {} entries, only the first {} are read", n,
                     OccupierForcesTuning::MaxWaveThresholds);
        }
        t.nWaveThresholds = MinInt(n, OccupierForcesTuning::MaxWaveThresholds);
        for (int k = 0; k < OccupierForcesTuning::MaxWaveThresholds; k++)
        {
            t.waveThresholds[k] =
                k < t.nWaveThresholds ? RepairElement("forcesWaveThresholds", k, (float)(*waves)[k], 0.0f, 1.0f) : 0.0f;
        }
    }
}

// ---------------------------------------------------------------------------
// pure rules
// ---------------------------------------------------------------------------

int OccupierForces::PoolStartOf(const OccupierForcesTuning& t, int R, int K)
{
    return MaxInt(t.poolMin, R + t.poolPerTown * K);
}

float OccupierForces::StandingFactor(float meanStanding)
{
    const float s = meanStanding < 0 ? 0 : (meanStanding > 100 ? 100 : meanStanding);
    return 0.75f + 0.5f * s / 100.0f;
}

float OccupierForces::RefillPerHour(int poolStart, int R, int G, float refillHours, float meanStanding)
{
    if (refillHours <= 0)
    {
        return 0;
    }
    return (float)(poolStart + R + G) / refillHours * StandingFactor(meanStanding);
}

// pool * 100 compared against pct * poolStart, in 64 bits
static bool PctAbove(int pool, int poolStart, int pct)
{
    return (long long)pool * 100 > (long long)pct * poolStart;
}

static bool PctBelow(int pool, int poolStart, int pct)
{
    return (long long)pool * 100 < (long long)pct * poolStart;
}

int OccupierForces::InitialBand(int pool, int poolStart)
{
    if (poolStart <= 0 || pool <= 0)
    {
        return OFBandSpent;
    }
    if (PctAbove(pool, poolStart, StrongEnterPct))
    {
        return OFBandStrong;
    }
    if (PctBelow(pool, poolStart, ThinEnterPct))
    {
        return OFBandThin;
    }
    return OFBandStretched;
}

int OccupierForces::StepBand(int prev, int pool, int poolStart)
{
    if (poolStart <= 0 || pool <= 0)
    {
        return OFBandSpent; // SPENT is entered at 0 from any band
    }
    if (prev < 0 || prev >= NOccupierBands)
    {
        return InitialBand(pool, poolStart);
    }
    int band = prev;
    // a jump (a wave, a forced pool) may cross several bands; each step
    // applies only the current band's exit rule, so no step can undo the
    // previous one
    for (int guard = 0; guard < NOccupierBands; guard++)
    {
        int next = band;
        switch (band)
        {
            case OFBandSpent:
                if (PctAbove(pool, poolStart, SpentLeavePct))
                {
                    next = OFBandThin;
                }
                break;
            case OFBandThin:
                if (PctAbove(pool, poolStart, ThinLeavePct))
                {
                    next = OFBandStretched;
                }
                break;
            case OFBandStretched:
                if (PctAbove(pool, poolStart, StrongEnterPct))
                {
                    next = OFBandStrong;
                }
                else if (PctBelow(pool, poolStart, ThinEnterPct))
                {
                    next = OFBandThin;
                }
                break;
            case OFBandStrong:
            default:
                if (PctBelow(pool, poolStart, StrongLeavePct))
                {
                    next = OFBandStretched;
                }
                break;
        }
        if (next == band)
        {
            break;
        }
        band = next;
    }
    return band;
}

bool OccupierForces::CanOperateIn(int band, int pool, int poolStart, float floor)
{
    if (band == OFBandSpent)
    {
        return false;
    }
    if (band == OFBandThin && (float)pool < floor * (float)poolStart)
    {
        return false;
    }
    return true;
}

float OccupierForces::TokenInterval(const OccupierForcesTuning& t, int band)
{
    return band >= 0 && band < NOccupierBands ? t.tokenInterval[band] : 0.0f;
}

float OccupierForces::ControlOf(float rosterHeld, float rosterTotal, int townsAdministered, int townsTotal,
                                float milWeight)
{
    const float w = milWeight < 0 ? 0 : (milWeight > 1 ? 1 : milWeight);
    const float m = rosterTotal > 0 ? rosterHeld / rosterTotal : 1.0f;
    if (townsTotal <= 0)
    {
        return m;
    }
    const float tShare = (float)townsAdministered / (float)townsTotal;
    return w * m + (1.0f - w) * tShare;
}

int OccupierForces::WavesCrossed(const OccupierForcesTuning& t, float control)
{
    int n = 0;
    for (int i = 0; i < t.nWaveThresholds && i < OccupierForcesTuning::MaxWaveThresholds; i++)
    {
        if (control <= t.waveThresholds[i])
        {
            n++;
        }
    }
    return n;
}

float OccupierForces::OccupierWarLevelOf(float territory, int wavesLanded, float step)
{
    if (wavesLanded <= 0 || step <= 0 || territory >= MaxWarLevel)
    {
        return territory;
    }
    const float v = territory + step * (float)wavesLanded;
    return v > MaxWarLevel ? MaxWarLevel : v;
}

int OccupierForces::ConservationImbalance(int pool, int reserves, int pending, int opStrength, int poolStart, int R,
                                          int G, int refill, int waves, int losses)
{
    return (pool + reserves + pending + opStrength) - (poolStart + R + G + refill + waves - losses);
}

void OccupierForces::Size(const ZoneRegistry& registry, const OccupierForcesTuning& t, OccupierForcesSizing& out,
                          const AutoArray<bool>* seeded, const SettlementProbe* probe, bool log)
{
    out = OccupierForcesSizing();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z)
        {
            continue;
        }
        if (CampaignEnd::IsMilitaryType(z->type))
        {
            if (CampaignEnd::IsAuthoredOccupier(*z, registry))
            {
                out.roster.Add(z->name);
                out.R += RoundToInt(z->garrisonMax);
            }
            continue;
        }
        if (stricmp(z->type, "CITY") != 0)
        {
            continue;
        }
        const char* reason = nullptr;
        if (!registry.IsAdministeredTown(*z))
        {
            reason = "not administered";
        }
        else if (z->garrisonExempt)
        {
            reason = "exempt from its garrison";
        }
        else if (seeded && i < seeded->Size() && (*seeded)[i] && probe && !probe->IsSettlement(z->name, z->pos))
        {
            reason = "a seeded label, not a settlement";
        }
        if (reason)
        {
            out.excluded.Add(z->name);
            if (log)
            {
                LOG_INFO(Core, "OccupierForces: town {} not counted ({})", (const char*)z->name, reason);
            }
            continue;
        }
        out.towns.Add(z->name);
        out.G += RoundToInt(z->garrisonMax);
        if (log)
        {
            LOG_INFO(Core, "OccupierForces: town {} counted (garrison {})", (const char*)z->name,
                     RoundToInt(z->garrisonMax));
        }
    }
    out.poolStart = PoolStartOf(t, out.R, out.K());
}

const char* OccupierForces::BandName(int band)
{
    switch (band)
    {
        case OFBandStrong:
            return "STRONG";
        case OFBandStretched:
            return "STRETCHED";
        case OFBandThin:
            return "THIN";
        case OFBandSpent:
            return "SPENT";
        default:
            return "OFF";
    }
}

const char* OccupierForces::PostureName(int posture)
{
    switch (posture)
    {
        case OFDormant:
            return "DORMANT";
        case OFAwake:
            return "AWAKE";
        default:
            return "OFF";
    }
}

// ---------------------------------------------------------------------------
// the service
// ---------------------------------------------------------------------------

bool OccupierForces::Activate(const ZoneRegistry& registry, const AutoArray<bool>* seeded, const SettlementProbe* probe)
{
    _activationTried = true;
    if (!registry.IsActive())
    {
        return false;
    }
    const auto start = std::chrono::steady_clock::now();
    OccupierForcesSizing s;
    Size(registry, _tuning, s, seeded, probe, true);
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    if (s.roster.Size() == 0)
    {
        _active = false;
        LOG_INFO(Core, "OccupierForces: no military zone with an authored occupier owner, the service is off");
        return false;
    }
    _roster.Clear();
    _towns.Clear();
    _excluded.Clear();
    for (int i = 0; i < s.roster.Size(); i++)
    {
        NameRow row;
        row.name = s.roster[i];
        _roster.Add(row);
    }
    for (int i = 0; i < s.towns.Size(); i++)
    {
        NameRow row;
        row.name = s.towns[i];
        _towns.Add(row);
    }
    for (int i = 0; i < s.excluded.Size(); i++)
    {
        NameRow row;
        row.name = s.excluded[i];
        _excluded.Add(row);
    }
    _R = s.R;
    _G = s.G;
    _poolStart = s.poolStart;
    _pool = _poolStart;
    _band = InitialBand(_pool, _poolStart);
    _posture = OFDormant;
    _tokens = 0;
    _tokenClock = 0;
    _directorAccum = 0;
    _refillAccum = 0;
    _refillTotal = 0;
    _wavesTotal = 0;
    _wavesFired = 0;
    _finalFired = false;
    _wavesLanded = 0;
    _lost = 0;
    _legacySave = false;
    _active = true;
    _control = ComputeControl(registry);
    LOG_INFO(Core,
             "OccupierForces: active, poolStart {} (R {}, K {}, G {}), roster {} zone(s), {} town(s) not counted, "
             "refill {:.1f}/h, band {}, sized in {:.1f} ms",
             _poolStart, _R, _towns.Size(), _G, _roster.Size(), _excluded.Size(),
             (double)RefillPerHour(_poolStart, _R, _G, _tuning.refillHours, _meanStanding), BandName(_band), ms);
    return true;
}

float OccupierForces::ComputeControl(const ZoneRegistry& registry) const
{
    const RString occupier = registry.OccupierSide();
    float heldMax = 0;
    float totalMax = 0;
    int heldCount = 0;
    int totalCount = 0;
    for (int i = 0; i < _roster.Size(); i++)
    {
        const ZoneRecord* z = registry.GetZone(registry.FindZoneIndex(_roster[i].name));
        if (!z)
        {
            continue; // a saved zone the template no longer has
        }
        const bool held = stricmp(z->owner, occupier) == 0;
        totalMax += z->garrisonMax;
        totalCount++;
        if (held)
        {
            heldMax += z->garrisonMax;
            heldCount++;
        }
    }
    if (totalMax <= 0)
    {
        // a roster of empty posts: strength by count
        heldMax = (float)heldCount;
        totalMax = (float)totalCount;
    }
    int townsAdmin = 0;
    int townsTotal = 0;
    for (int i = 0; i < _towns.Size(); i++)
    {
        const ZoneRecord* z = registry.GetZone(registry.FindZoneIndex(_towns[i].name));
        if (!z)
        {
            continue;
        }
        townsTotal++;
        if (registry.IsAdministeredTown(*z))
        {
            townsAdmin++;
        }
    }
    return ControlOf(heldMax, totalMax, townsAdmin, townsTotal, _tuning.controlMilWeight);
}

void OccupierForces::UpdateBand()
{
    const int next = StepBand(_band, _pool, _poolStart);
    if (next != _band)
    {
        LOG_INFO(Core, "OccupierForces: band {} -> {} (pool {} of {})", BandName(_band), BandName(next), _pool,
                 _poolStart);
        _band = next;
    }
}

bool OccupierForces::LogAllowed()
{
    if (_logClock >= LogQuiet)
    {
        _logClock = 0;
        return true;
    }
    _logSuppressed++;
    return false;
}

void OccupierForces::Refill(float elapsed)
{
    if (_pool >= _poolStart || elapsed <= 0)
    {
        _refillAccum = 0; // the refill fills up to poolStart, never above
        return;
    }
    const float perHour = RefillPerHour(_poolStart, _R, _G, _tuning.refillHours, _meanStanding);
    if (perHour <= 0)
    {
        return;
    }
    _refillAccum += perHour * elapsed / 3600.0f;
    const int whole = (int)floorf(_refillAccum);
    if (whole <= 0)
    {
        return;
    }
    _refillAccum -= (float)whole;
    const int credit = MinInt(whole, _poolStart - _pool);
    _pool += credit;
    _refillTotal += credit;
    if (_pool >= _poolStart)
    {
        _refillAccum = 0;
    }
    UpdateBand();
}

void OccupierForces::Tick(const ZoneRegistry& registry, float elapsed)
{
    if (!Active())
    {
        return;
    }
    _logClock += elapsed;
    _clock += elapsed;
    Refill(elapsed);
    _control = ComputeControl(registry);
    UpdateBand();
}

int OccupierForces::Draw(int bodies)
{
    if (bodies <= 0)
    {
        return 0;
    }
    if (!Active())
    {
        return bodies; // legacy D2: regrowth from nothing
    }
    const int granted = MinInt(bodies, _pool);
    if (granted > 0)
    {
        _pool -= granted;
        UpdateBand();
    }
    return granted;
}

int OccupierForces::Deposit(int bodies, const char* why)
{
    if (bodies <= 0)
    {
        return 0;
    }
    if (!Active())
    {
        return bodies;
    }
    if (_band == OFBandSpent)
    {
        if (LogAllowed())
        {
            LOG_INFO(Core, "OccupierForces: SPENT, no deposit of {} ({}){}", bodies, why ? why : "",
                     _logSuppressed > 0 ? " (other lines suppressed)" : "");
            _logSuppressed = 0;
        }
        return 0;
    }
    const int granted = MinInt(bodies, _pool);
    if (granted > 0)
    {
        _pool -= granted;
        LOG_DEBUG(Core, "OccupierForces: deposit {} of {} ({}), pool {}", granted, bodies, why ? why : "", _pool);
        UpdateBand();
    }
    return granted;
}

bool OccupierForces::Spend(int units, const char* why)
{
    if (!Active() || units <= 0)
    {
        return true;
    }
    if (units > _pool || !CanOperate())
    {
        if (LogAllowed())
        {
            LOG_INFO(Core, "OccupierForces: refused {} unit(s) for {} (pool {} of {}, band {}){}", units,
                     why ? why : "", _pool, _poolStart, BandName(_band),
                     _logSuppressed > 0 ? " (other lines suppressed)" : "");
            _logSuppressed = 0;
        }
        return false;
    }
    _pool -= units;
    LOG_DEBUG(Core, "OccupierForces: spent {} for {}, pool {}", units, why ? why : "", _pool);
    UpdateBand();
    return true;
}

void OccupierForces::Refund(int units, const char* why)
{
    if (!Active() || units <= 0)
    {
        return;
    }
    _pool += units;
    LOG_DEBUG(Core, "OccupierForces: refund {} ({}), pool {}", units, why ? why : "", _pool);
    UpdateBand();
}

void OccupierForces::NoteLoss(int units, const char* why)
{
    if (!Active() || units <= 0)
    {
        return;
    }
    _lost += units;
    LOG_DEBUG(Core, "OccupierForces: lost {} ({}), {} in all", units, why ? why : "", _lost);
}

void OccupierForces::LandWave(int bodies)
{
    if (!Active() || bodies <= 0)
    {
        return;
    }
    _pool += bodies;
    _wavesTotal += bodies;
    _wavesLanded++;
    LOG_INFO(Core, "OccupierForces: wave of {} landed, pool {} of {}, {} wave(s) landed", bodies, _pool, _poolStart,
             _wavesLanded);
    UpdateBand();
}

bool OccupierForces::TownEver(const char* name) const
{
    for (int i = 0; i < _townsEver.Size(); i++)
    {
        if (stricmp(_townsEver[i].name, name) == 0)
        {
            return true;
        }
    }
    return false;
}

float OccupierForces::OccupierWarLevel() const
{
    return OccupierWarLevelOf(ReadWarLevel(), Active() ? _wavesLanded : 0, _tuning.waveWarLevel);
}

void OccupierForces::SetPool(int units)
{
    if (!Active())
    {
        return;
    }
    _pool = units < 0 ? 0 : units;
    LOG_INFO(Core, "OccupierForces: pool forced to {} of {}", _pool, _poolStart);
    UpdateBand();
}

void OccupierForces::SetMeanStanding(float standing)
{
    _meanStanding = standing < 0 ? 0 : (standing > 100 ? 100 : standing);
}

void OccupierForces::RequestWave()
{
    _waveRequests++;
    LOG_INFO(Core, "OccupierForces: wave requested ({} pending); waves are not implemented yet, no effect",
             _waveRequests);
}

namespace
{

// The director's engine hands: CounterAttack::LaunchFunded with the same
// view and ops CounterAttack::Simulate steps with; the returned events are
// fired through the registry here (LaunchFunded does not).
class EngineDirectorLauncher final : public DirectorLauncher
{
  public:
    explicit EngineDirectorLauncher(ZoneRegistry& registry) : _registry(registry) {}
    bool Launch(const CounterAttackOrder& order) override
    {
        CounterAttack& ca = CounterAttack::Instance();
        CounterAttackView view;
        ca.BuildView(view);
        AutoArray<ZoneEventRecord> fired;
        const bool ok = ca.LaunchFunded(_registry, view, order, ca.EngineOps(), fired);
        _registry.FireEvents(fired);
        return ok;
    }
    int BornOps() const override { return CounterAttack::Instance().BornOps(); }
    bool HasRecord(int zoneIndex) const override
    {
        const ZoneRecord* z = _registry.GetZone(zoneIndex);
        return z && CounterAttack::Instance().FindByTarget(z->name) >= 0;
    }

  private:
    ZoneRegistry& _registry;
};

} // namespace

void OccupierForces::Simulate(float deltaT)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    if (!registry.IsActive() || !_tuning.enabled)
    {
        return;
    }
    if (!_active)
    {
        if (!_activationTried)
        {
            // the first active frame: the roster, K and the pool.  The engine
            // passes no seeded table yet (ZoneRecord has no seeded flag), so
            // every administered non-exempt CITY counts.
            Activate(registry);
        }
        return;
    }
    _directorAccum += deltaT;
    // the sightings scan at the alert cadence (6.1)
    _scanAccum += deltaT;
    const float scanEvery = AlertMachine::Instance().Tuning().alertInterval;
    if (_scanAccum >= (scanEvery > 0 ? scanEvery : 5.0f))
    {
        _scanAccum = 0;
        ScanWorld(registry);
    }
    if (_directorAccum < _tuning.directorInterval)
    {
        return;
    }
    const float elapsed = _directorAccum;
    _directorAccum = 0;
    Tick(registry, elapsed);
    if (!_catalogueBuilt)
    {
        BuildEngineCatalogue(registry);
    }
    DirectorView view;
    FillEngineView(registry, view);
    EngineDirectorLauncher launcher(registry);
    Direct(registry, view, launcher, elapsed);
}

// ---------------------------------------------------------------------------
// the director (OCCUPIER-FORCES.md 6, phase 3)
//
// D9: THE DIRECTOR KNOWS ONLY WHAT ITS MEN SAW.  Everything in this section
// reads the zone owners, flips and heat, the alert states, the sightings
// table and its own operations' events.  It never reads the player's
// position, the true census (ZoneTickInputs::guerCount, ZoneRecord::lastGuer
// / lastGuerHold) or AlertMachine's last-known position (whose fallback is
// the player's true position).  The sightings scan (ScanWorld) reads only
// the occupier groups' own target records: the REPORTED positions of the
// resistance units they know.  The simulation itself (the drain, the live
// fight) still resolves on true presence.
// ---------------------------------------------------------------------------

bool OccupierForces::HasName(const AutoArray<NameRow>& rows, const char* name)
{
    if (!name)
    {
        return false;
    }
    for (int i = 0; i < rows.Size(); i++)
    {
        if (stricmp(rows[i].name, name) == 0)
        {
            return true;
        }
    }
    return false;
}

void OccupierForces::AddName(AutoArray<NameRow>& rows, const char* name)
{
    if (!name || !*name || HasName(rows, name))
    {
        return;
    }
    NameRow row;
    row.name = name;
    rows.Add(row);
}

void OccupierForces::RemoveName(AutoArray<NameRow>& rows, const char* name)
{
    for (int i = rows.Size() - 1; i >= 0; i--)
    {
        if (name && stricmp(rows[i].name, name) == 0)
        {
            rows.Delete(i);
        }
    }
}

float OccupierForces::Cooldown(const char* zone) const
{
    for (int i = 0; i < _cooldowns.Size(); i++)
    {
        if (zone && stricmp(_cooldowns[i].zone, zone) == 0)
        {
            return _cooldowns[i].seconds;
        }
    }
    return 0;
}

void OccupierForces::SetCooldown(const char* zone, float seconds)
{
    if (!zone || !*zone || seconds <= 0)
    {
        return;
    }
    for (int i = 0; i < _cooldowns.Size(); i++)
    {
        if (stricmp(_cooldowns[i].zone, zone) == 0)
        {
            _cooldowns[i].seconds = seconds;
            return;
        }
    }
    ZoneClockRow row;
    row.zone = zone;
    row.seconds = seconds;
    _cooldowns.Add(row);
}

void OccupierForces::CountDownCooldowns(float elapsed)
{
    for (int i = _cooldowns.Size() - 1; i >= 0; i--)
    {
        _cooldowns[i].seconds -= elapsed;
        if (_cooldowns[i].seconds <= 0)
        {
            _cooldowns.Delete(i);
        }
    }
}

void OccupierForces::ForgetZone(const char* zone)
{
    RemoveName(_urgent, zone);
    RemoveName(_redPaid, zone);
    for (int i = _cooldowns.Size() - 1; i >= 0; i--)
    {
        if (zone && stricmp(_cooldowns[i].zone, zone) == 0)
        {
            _cooldowns.Delete(i);
        }
    }
}

int OccupierForces::FindOp(int opId) const
{
    if (opId <= 0)
    {
        return -1;
    }
    for (int i = 0; i < _ops.Size(); i++)
    {
        if (_ops[i].opId == opId)
        {
            return i;
        }
    }
    return -1;
}

void OccupierForces::PruneOps()
{
    while (_ops.Size() > MaxOpRows)
    {
        int drop = 0;
        for (int i = 0; i < _ops.Size(); i++)
        {
            if (_ops[i].finished)
            {
                drop = i;
                break;
            }
        }
        _ops.Delete(drop);
    }
}

bool OccupierForces::IsHeldTown(const ZoneRegistry& registry, const ZoneRecord& z)
{
    return !z.garrisonExempt && stricmp(z.type, "CITY") == 0 && stricmp(z.owner, registry.ResistanceSide()) == 0;
}

float OccupierForces::CandidateRoll(const ZoneRecord& z)
{
    return CounterAttack::Roll01(CounterAttack::MakeSeed(z.name, z.flipDay, z.flipMinute, 0), CandidateSalt);
}

int OccupierForces::ZoneOfPosition(const ZoneRegistry& registry, Vector3Par pos)
{
    const float area = registry.Tuning().zoneArea;
    float bestSq = area * area;
    int best = -1;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z)
        {
            continue;
        }
        const float dx = z->pos.X() - pos.X();
        const float dz = z->pos.Z() - pos.Z();
        const float dSq = dx * dx + dz * dz;
        if (dSq <= bestSq)
        {
            bestSq = dSq;
            best = i;
        }
    }
    return best;
}

int OccupierForces::KnownFighters(const ZoneRegistry& registry, int zoneIndex) const
{
    const ZoneRecord* z = registry.GetZone(zoneIndex);
    if (!z)
    {
        return 0;
    }
    for (int i = 0; i < _sightings.Size(); i++)
    {
        if (stricmp(_sightings[i].zone, z->name) == 0 && Now() - _sightings[i].stamp <= _tuning.intelDecay)
        {
            return _sightings[i].count;
        }
    }
    return 0;
}

void OccupierForces::IngestSightings(const ZoneRegistry& registry, const AutoArray<SightingReport>& reports)
{
    if (!Active())
    {
        return;
    }
    // one row per reported zone, replaced by this scan; zones nobody reports
    // now keep their last row (a destroyed group radioed before it died)
    AutoArray<bool> done;
    done.Resize(reports.Size());
    for (int i = 0; i < reports.Size(); i++)
    {
        done[i] = false;
    }
    for (int i = 0; i < reports.Size(); i++)
    {
        if (done[i])
        {
            continue;
        }
        const int zone = reports[i].zone;
        const ZoneRecord* z = registry.GetZone(zone);
        const RString name = z ? z->name : RString();
        SightingRow row;
        row.zone = name;
        row.stamp = Now();
        for (int j = i; j < reports.Size(); j++)
        {
            const int zj = registry.GetZone(reports[j].zone) ? reports[j].zone : -1;
            const int zi = z ? zone : -1;
            if (done[j] || zj != zi)
            {
                continue;
            }
            done[j] = true;
            bool seen = false;
            for (int k = i; k < j && !seen; k++)
            {
                const int zk = registry.GetZone(reports[k].zone) ? reports[k].zone : -1;
                seen = zk == zi && reports[k].id == reports[j].id;
            }
            if (seen)
            {
                continue; // the same unit reported by a second group
            }
            row.count++;
            if (row.positions.Size() < MaxSightingPositions)
            {
                row.positions.Add(reports[j].pos);
            }
        }
        bool replaced = false;
        for (int r = 0; r < _sightings.Size() && !replaced; r++)
        {
            if (stricmp(_sightings[r].zone, name) == 0)
            {
                _sightings[r] = row;
                replaced = true;
            }
        }
        if (!replaced)
        {
            _sightings.Add(row);
        }
    }
}

void OccupierForces::DecaySightings()
{
    for (int i = _sightings.Size() - 1; i >= 0; i--)
    {
        if (Now() - _sightings[i].stamp > _tuning.intelDecay)
        {
            _sightings.Delete(i);
        }
    }
}

void OccupierForces::NoteAlertStates(const ZoneRegistry& registry, const AutoArray<int>& states)
{
    const int n = registry.NZones();
    if (_alertPrev.Size() != n)
    {
        _alertPrev.Resize(n);
        for (int i = 0; i < n; i++)
        {
            _alertPrev[i] = -1; // the first sample sets the baseline, no edge
        }
    }
    if (!Active())
    {
        return;
    }
    const RString resistance = registry.ResistanceSide();
    for (int i = 0; i < n; i++)
    {
        const int s = i < states.Size() ? states[i] : (int)ASGreen;
        const int prev = _alertPrev[i];
        _alertPrev[i] = s;
        if (prev < 0 || s <= prev || s < ASYellow)
        {
            continue;
        }
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || stricmp(z->owner, resistance) != 0)
        {
            continue;
        }
        // a YELLOW / RED edge at a zone the resistance holds: occupier units
        // there saw his men.  Urgent until its heat falls below the exit; a
        // RED edge already paid the heat a later repel would add
        if (!Urgent(z->name))
        {
            AddName(_urgent, z->name);
            LOG_DEBUG(Core, "OccupierForces: {} urgent (alert edge {} -> {})", (const char*)z->name, prev, s);
        }
        if (s == ASRed && prev < ASRed)
        {
            AddName(_redPaid, z->name);
        }
    }
}

void OccupierForces::UpdateUrgency(const ZoneRegistry& registry)
{
    const RString resistance = registry.ResistanceSide();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z)
        {
            continue;
        }
        if (stricmp(z->owner, resistance) != 0)
        {
            RemoveName(_urgent, z->name);
            continue;
        }
        if (z->heat >= _tuning.urgentHeat)
        {
            AddName(_urgent, z->name);
        }
        else if (z->heat < UrgentExitHeat)
        {
            RemoveName(_urgent, z->name);
        }
    }
}

void OccupierForces::RefillTokens(float elapsed)
{
    if (_posture != OFAwake)
    {
        return;
    }
    const int maxTokens = _tuning.tokensMax > 0 ? _tuning.tokensMax : 0;
    if (_tokens >= maxTokens)
    {
        _tokens = maxTokens; // a lowered tokensMax trims the purse
        _tokenClock = 0;
        return;
    }
    const float interval = TokenInterval(_tuning, _band);
    if (interval <= 0)
    {
        return; // SPENT: no tokens, the clock waits
    }
    _tokenClock += elapsed;
    while (_tokenClock >= interval && _tokens < maxTokens)
    {
        _tokens++;
        _tokenClock -= interval;
    }
    if (_tokens >= maxTokens)
    {
        _tokenClock = 0;
    }
}

void OccupierForces::Candidates(const ZoneRegistry& registry, const DirectorView& view,
                                const DirectorLauncher& launcher, AutoArray<int>& out)
{
    out.Clear();
    const RString resistance = registry.ResistanceSide();
    const float full = _tuning.heatFull > 0 ? _tuning.heatFull : 1.0f;
    int basesHot = 0;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || stricmp(z->owner, resistance) != 0)
        {
            continue;
        }
        const bool hot = z->heat / full >= CandidateRoll(*z);
        if (CampaignEnd::IsMilitaryType(z->type))
        {
            // D16: a base is a candidate only while STRONG; the BASE kind is
            // phase 4, so it is counted here and never ordered
            basesHot += _band == OFBandStrong && hot ? 1 : 0;
            continue;
        }
        if (!IsHeldTown(registry, *z) || i == view.hqZone || Cooldown(z->name) > 0 || launcher.HasRecord(i) || !hot)
        {
            continue;
        }
        // hottest first (stable on the zone order)
        int at = out.Size();
        while (at > 0 && registry.GetZone(out[at - 1])->heat < z->heat)
        {
            at--;
        }
        out.Add(i);
        for (int k = out.Size() - 1; k > at; k--)
        {
            out[k] = out[k - 1];
        }
        out[at] = i;
    }
    if (basesHot > 0 && !_basesDeferredLogged)
    {
        _basesDeferredLogged = true;
        LOG_INFO(Core, "OccupierForces: base candidate deferred to phase 4 ({} held base(s) hot while STRONG)",
                 basesHot);
    }
}

int OccupierForces::PushoutTarget(const ZoneRegistry& registry, const DirectorView& view,
                                  const DirectorLauncher& launcher)
{
    const RString occupier = registry.OccupierSide();
    int best = -1;
    float bestDist = 0;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || !IsHeldTown(registry, *z) || i == view.hqZone || Cooldown(z->name) > 0 || launcher.HasRecord(i))
        {
            continue;
        }
        // quiet: no sighting there for forcesPushoutQuiet
        bool quiet = true;
        for (int s = 0; s < _sightings.Size() && quiet; s++)
        {
            quiet = !(stricmp(_sightings[s].zone, z->name) == 0 && Now() - _sightings[s].stamp < _tuning.pushoutQuiet);
        }
        if (!quiet)
        {
            continue;
        }
        // nearest an occupier base
        float nearest = -1;
        for (int b = 0; b < registry.NZones(); b++)
        {
            const ZoneRecord* base = registry.GetZone(b);
            if (!base || !CampaignEnd::IsMilitaryType(base->type) || stricmp(base->owner, occupier) != 0)
            {
                continue;
            }
            const float dx = base->pos.X() - z->pos.X();
            const float dz = base->pos.Z() - z->pos.Z();
            const float d = sqrtf(dx * dx + dz * dz);
            if (nearest < 0 || d < nearest)
            {
                nearest = d;
            }
        }
        if (nearest < 0)
        {
            return -1; // no occupier base at all
        }
        if (best < 0 || nearest < bestDist)
        {
            best = i;
            bestDist = nearest;
        }
    }
    return best;
}

DirectorAction OccupierForces::PlanFor(const ZoneRegistry& registry, const DirectorView& view, int target,
                                       int origin) const
{
    DirectorAction a;
    a.target = target;
    a.origin = origin;
    a.kind = CAKRetake;
    const ZoneRecord* z = registry.GetZone(target);
    if (!z)
    {
        a.choice.why = "no such zone";
        return a;
    }
    a.urgent = origin == CAODirector && Urgent(z->name);
    a.fighters = (float)KnownFighters(registry, target);
    PackageRequest req;
    req.fighters = a.fighters;
    req.defenderWeight = view.defenderWeight;
    req.margin = _tuning.lethalityMargin;
    // max(forcesRetakeMin, garrisonMax), at most one group (a town authored
    // above twelve defenders is retaken by a full group)
    req.minBodies = MinInt(MaxInt(_tuning.retakeMin, RoundToInt(z->garrisonMax)), ForcePackages::MaxGroup);
    req.urgent = a.urgent;
    req.urgentCostShare = _tuning.urgentCostShare;
    req.tierFactor = view.tierFactor;
    req.pool = _pool;
    req.poolActive = Active();
    req.offered = OfferedPackages;
    const RString occupier = registry.OccupierSide();
    AutoArray<PackageSource> sources;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* s = registry.GetZone(i);
        if (!s || !CampaignEnd::IsMilitaryType(s->type) || stricmp(s->owner, occupier) != 0)
        {
            continue;
        }
        PackageSource src;
        src.zone = i;
        src.spare = i < view.spare.Size() ? view.spare[i] : 0;
        src.spareStored = i < view.spareStored.Size() ? view.spareStored[i] : -1;
        if (src.spare <= 0 && src.spareStored <= 0)
        {
            continue;
        }
        const float dx = s->pos.X() - z->pos.X();
        const float dz = s->pos.Z() - z->pos.Z();
        src.distance = sqrtf(dx * dx + dz * dz);
        sources.Add(src);
    }
    a.choice = ForcePackages::Choose(req, _catalogue, sources);
    a.go = a.choice.ok;
    return a;
}

static const char* ZoneNameOf(const ZoneRegistry& registry, int index)
{
    const ZoneRecord* z = registry.GetZone(index);
    return z ? (const char*)z->name : "off-map";
}

bool OccupierForces::TryLaunch(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, int target,
                               int origin)
{
    const ZoneRecord* z = registry.GetZone(target);
    if (!z)
    {
        return false;
    }
    const DirectorAction a = PlanFor(registry, view, target, origin);
    if (!a.go)
    {
        if (LogAllowed())
        {
            LOG_INFO(Core, "OccupierForces: no operation on {}: {}{}", (const char*)z->name, (const char*)a.choice.why,
                     _logSuppressed > 0 ? " (other lines suppressed)" : "");
            _logSuppressed = 0;
        }
        return false;
    }
    const PackageQuote& q = a.choice.quote;
    const int opId = _opCounter + 1;
    CounterAttackOrder order;
    order.opId = opId;
    order.kind = a.kind;
    order.origin = origin;
    order.package = q.package;
    order.target = target;
    order.source = q.source;
    order.bodies = q.bodies;
    order.crew = q.crew;
    order.hullClass = q.hullClass;
    order.priceUnits = q.poolPrice;
    order.urgent = a.urgent;
    order.abstractSpeed = q.speed;
    order.funded = true;
    // the pool pays hull and crew BEFORE the launch, and gets it back when
    // CounterAttack refuses the order
    if (q.poolPrice > 0 && !Spend(q.poolPrice, "operation hull and crew"))
    {
        return false;
    }
    // the ledger row exists before the launch: events raised while
    // CounterAttack starts the record find it
    OpRow row;
    row.opId = opId;
    row.target = z->name;
    row.kind = a.kind;
    row.origin = origin;
    row.package = q.package;
    row.outstanding = q.poolPrice;
    _ops.Add(row);
    _opCounter = opId;
    if (!launcher.Launch(order))
    {
        Refund(q.poolPrice, "operation refused");
        const int r = FindOp(opId);
        if (r >= 0)
        {
            _ops.Delete(r);
        }
        _opCounter = opId - 1;
        LOG_INFO(Core, "OccupierForces: CounterAttack refused the {} operation from {} to {}",
                 ForcePackages::PackageName(q.package), ZoneNameOf(registry, q.source), (const char*)z->name);
        return false;
    }
    _tokens = _tokens > 0 ? _tokens - 1 : 0;
    // a RED edge from here on pays the heat of a repel
    RemoveName(_redPaid, z->name);
    PruneOps();

    const bool push = origin == CAOPushout;
    const char* label = q.package == FPFoot    ? (push ? "foot squad" : "Foot squad")
                        : q.package == FPTruck ? (push ? "motorized squad" : "Motorized squad")
                                               : ForcePackages::PackageName(q.package);
    const char* shortNote =
        a.choice.shortOfLethality ? ", short of the required lethality: the most lethal affordable package" : "";
    if (q.package == FPFoot)
    {
        LOG_INFO(Core,
                 "OccupierForces: [op {}] {}{} ({}) from {} to {} ({} unit(s), the pool pays {}, lethality {:.2f} "
                 "of {:.2f}, {} fighter(s) reported{}{}, about {:.0f} s)",
                 opId, push ? "Push-out: " : "", label, q.bodies, ZoneNameOf(registry, q.source), (const char*)z->name,
                 q.price, q.poolPrice, (double)q.lethality, (double)a.choice.lReq, RoundToInt(a.fighters),
                 a.urgent ? ", urgent" : "", shortNote, (double)q.seconds);
    }
    else
    {
        LOG_INFO(Core,
                 "OccupierForces: [op {}] {}{} ({}, {}) from {} to {} ({} unit(s), the pool pays {}, lethality "
                 "{:.2f} of {:.2f}, {} fighter(s) reported{}{}, about {:.0f} s)",
                 opId, push ? "Push-out: " : "", label, q.bodies, (const char*)q.hullClass,
                 ZoneNameOf(registry, q.source), (const char*)z->name, q.price, q.poolPrice, (double)q.lethality,
                 (double)a.choice.lReq, RoundToInt(a.fighters), a.urgent ? ", urgent" : "", shortNote,
                 (double)q.seconds);
    }
    return true;
}

void OccupierForces::Direct(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, float elapsed)
{
    if (!Active())
    {
        return;
    }
    CountDownCooldowns(elapsed);
    DecaySightings();
    UpdateUrgency(registry);
    RefillTokens(elapsed);
    // W: before the wake the director considers nothing; after the win it
    // launches nothing (7.4)
    if (_posture != OFAwake || view.won || _tokens <= 0 || !CanOperate())
    {
        return;
    }
    if (launcher.BornOps() >= _tuning.opsMax)
    {
        return; // unborn operations do not count, born ones do
    }
    AutoArray<int> candidates;
    Candidates(registry, view, launcher, candidates);
    for (int i = 0; i < candidates.Size(); i++)
    {
        if (TryLaunch(registry, view, launcher, candidates[i], CAODirector))
        {
            return; // one operation per tick
        }
    }
    if (candidates.Size() == 0 && _band == OFBandStrong)
    {
        // 6.4: strong, a token and nothing hot: retake a quiet town
        const int quiet = PushoutTarget(registry, view, launcher);
        if (quiet >= 0)
        {
            TryLaunch(registry, view, launcher, quiet, CAOPushout);
        }
    }
}

bool OccupierForces::OnTownFlipped(int zoneIndex, int /*k*/)
{
    return NoteTownFlipped(ZoneRegistry::Instance(), zoneIndex);
}

void OccupierForces::OnCounterAttackOp(const CounterAttackOpEvent& ev)
{
    NoteOpEvent(ZoneRegistry::Instance(), ev);
}

void OccupierForces::OnZoneEvents(const AutoArray<ZoneEventRecord>& fired)
{
    NoteZoneEvents(ZoneRegistry::Instance(), fired);
}

bool OccupierForces::NoteTownFlipped(const ZoneRegistry& registry, int zoneIndex)
{
    if (!Active())
    {
        return false;
    }
    const ZoneRecord* z = registry.GetZone(zoneIndex);
    if (!z)
    {
        return false;
    }
    // a fresh flip: its urgency counts edges from now, no old cooldown
    ForgetZone(z->name);
    if (!TownEver(z->name))
    {
        AddName(_townsEver, z->name);
        LOG_INFO(Core, "OccupierForces: {} taken, {} distinct town(s) so far", (const char*)z->name, _townsEver.Size());
    }
    if (_posture == OFDormant && _townsEver.Size() >= _tuning.wakeTowns)
    {
        // invariant W: the flip that brings the set to forcesWakeTowns wakes
        // the director with a full purse, and goes to it
        _posture = OFAwake;
        _tokens = _tuning.tokensMax;
        _tokenClock = 0;
        LOG_INFO(Core, "OccupierForces: AWAKE after {} distinct towns ({} the latest), {} token(s)", _townsEver.Size(),
                 (const char*)z->name, _tokens);
    }
    return _posture == OFAwake;
}

void OccupierForces::NoteZoneEvents(const ZoneRegistry& registry, const AutoArray<ZoneEventRecord>& fired)
{
    if (!Active())
    {
        return;
    }
    for (int i = 0; i < fired.Size(); i++)
    {
        const ZoneEventRecord& ev = fired[i];
        if (ev.type != ZECaptured && ev.type != ZETownLost)
        {
            continue;
        }
        const ZoneRecord* z = registry.GetZone(ev.zoneIndex);
        if (z)
        {
            // an owner change: the urgency latch, the cooldown and the RED
            // payment belonged to the zone's previous holding
            ForgetZone(z->name);
        }
    }
}

void OccupierForces::NoteOpEvent(ZoneRegistry& registry, const CounterAttackOpEvent& ev)
{
    if (!Active())
    {
        return;
    }
    const int row = FindOp(ev.opId);
    const ZoneRecord* z = registry.GetZone(ev.target);
    const char* target = z ? (const char*)z->name : "";
    const bool mine = row >= 0;
    switch (ev.type)
    {
        case CAOpBorn:
            if (mine)
            {
                _ops[row].born = true;
            }
            LOG_DEBUG(Core, "OccupierForces: [op {}] born ({})", ev.opId, ev.note ? ev.note : "");
            break;
        case CAOpResourced:
            if (mine)
            {
                LOG_INFO(Core, "OccupierForces: [op {}] Re-sourced to {}: the nearer base's road was watched", ev.opId,
                         ZoneNameOf(registry, ev.source));
            }
            break;
        case CAOpCancelledUnseen:
        {
            // nothing could be born: the hull price comes back, the target
            // rests (3.1 step 5)
            int refund = 0;
            if (mine)
            {
                refund = _ops[row].outstanding;
                _ops[row].outstanding = 0;
                _ops[row].finished = true;
                Refund(refund, "cancelled unborn");
            }
            SetCooldown(target, _tuning.townCooldown);
            LOG_INFO(Core,
                     "OccupierForces: [op {}] Cancelled unborn: {}; {} unit(s) back to the pool, {} rests {:.0f} s",
                     ev.opId, ev.note && *ev.note ? ev.note : "every route watched", refund, target,
                     (double)_tuning.townCooldown);
            break;
        }
        case CAOpRepelled:
        {
            // the target rests, and the occupier's anger shows as heat unless
            // a RED edge during the fight already paid it
            const bool paid = RedPaid(target);
            if (!paid && ev.target >= 0)
            {
                registry.HeatRaise(ev.target, RepelHeat);
            }
            RemoveName(_redPaid, target);
            SetCooldown(target, _tuning.townCooldown);
            if (mine)
            {
                _ops[row].finished = true;
            }
            LOG_INFO(Core, "OccupierForces: [op {}] Repelled at {}: {} lost, {} going home; {} rests {:.0f} s{}",
                     ev.opId, target, ev.units, ev.bodies, target, (double)_tuning.townCooldown,
                     paid ? " (a RED edge already raised the heat)" : ", heat +15");
            break;
        }
        case CAOpSettled:
            if (mine)
            {
                _ops[row].finished = true;
                LOG_INFO(Core, "OccupierForces: [op {}] {} retaken: {} stay as its garrison", ev.opId, target,
                         ev.bodies);
            }
            break;
        case CAOpAborted:
            if (mine)
            {
                _ops[row].finished = true;
                LOG_INFO(Core, "OccupierForces: [op {}] Aborted: {}", ev.opId, ev.note ? ev.note : "");
            }
            break;
        case CAOpHullRefund:
            if (mine)
            {
                const int units = MinInt(ev.units > 0 ? ev.units : 0, _ops[row].outstanding);
                _ops[row].outstanding -= units;
                Refund(units, "hull home intact");
            }
            break;
        case CAOpHullLost:
            if (mine)
            {
                const int units = MinInt(ev.units > 0 ? ev.units : 0, _ops[row].outstanding);
                _ops[row].outstanding -= units;
                NoteLoss(units, "hull lost");
            }
            break;
        default:
            // Launched, Arrived, Returned: CounterAttack routes the bodies
            // (5.5) itself; nothing for the ledger
            break;
    }
    PruneOps();
}

// engine path ---------------------------------------------------------------

void OccupierForces::BuildEngineCatalogue(const ZoneRegistry& registry)
{
    _catalogueBuilt = true;
    const RString occupier = registry.OccupierSide();
    const FactionRecord* f = registry.FindFactionForSide(occupier);
    AutoArray<RString> ladder;
    if (f)
    {
        for (int i = 0; i < f->vehicles.Size(); i++)
        {
            ladder.Add(f->vehicles[i]);
        }
    }
    RString overrides[NForcePackages];
    for (int p = 0; p < NForcePackages; p++)
    {
        const char* key = ForcePackages::OverrideKey(p);
        if (*key)
        {
            overrides[p] = registry.FactionValue(occupier, key);
        }
    }
    ConfigHullInspector inspector;
    ForcePackages::BuildCatalogue(ladder, overrides, inspector, _catalogue, true);
    LOG_INFO(Core, "OccupierForces: package catalogue for {} ({}, {} ladder class(es)); phase 3 offers foot{}",
             (const char*)occupier, f ? (const char*)f->className : "no descriptor", ladder.Size(),
             _catalogue.Offered(FPTruck) ? " and truck" : " only (no truck in the ladder)");
}

void OccupierForces::FillEngineView(const ZoneRegistry& registry, DirectorView& view) const
{
    const int n = registry.NZones();
    view.alert.Resize(n);
    view.spare.Resize(n);
    view.spareStored.Resize(n);
    const AlertMachine& alert = AlertMachine::Instance();
    const GarrisonCache& cache = GarrisonCache::Instance();
    const CounterAttack& ca = CounterAttack::Instance();
    const RString occupier = registry.OccupierSide();
    for (int i = 0; i < n; i++)
    {
        view.alert[i] = alert.GetZoneState(i);
        view.spare[i] = 0;
        view.spareStored[i] = 0;
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || !CampaignEnd::IsMilitaryType(z->type) || stricmp(z->owner, occupier) != 0)
        {
            continue;
        }
        const int live = cache.LiveCount(i);
        const int stored = CounterAttack::Spare(ca.Tuning(), z->garrison, live, z->garrisonMax, false);
        const int marched = live > 0 ? CounterAttack::Spare(ca.Tuning(), z->garrison, live, z->garrisonMax, true) : 0;
        view.spare[i] = MaxInt(stored, marched);
        view.spareStored[i] = stored;
    }
    view.hqZone = GuerrillaBase::Instance().HqZone();
    view.won = CampaignEnd::Instance().Won();
    view.defenderWeight = ca.Tuning().defenderWeight;
    // the infantry tier the occupier fields now (its own war level)
    view.tierFactor = 1.0f;
    const FactionRecord* f = registry.FindFactionForSide(occupier);
    if (f)
    {
        const RString cls = registry.FactionTierClass(occupier, OccupierWarLevel());
        for (int t = 0; t < f->tiers.Size(); t++)
        {
            if (stricmp(f->tiers[t], cls) == 0)
            {
                view.tierFactor = ForcePackages::TierFactor(t);
                break;
            }
        }
    }
}

void OccupierForces::ScanWorld(const ZoneRegistry& registry)
{
    // the alert states first (edges latch urgency), then the sightings
    const int n = registry.NZones();
    AutoArray<int> states;
    states.Resize(n);
    const AlertMachine& alert = AlertMachine::Instance();
    for (int i = 0; i < n; i++)
    {
        states[i] = alert.GetZoneState(i);
    }
    NoteAlertStates(registry, states);

    World* world = GWorld;
    if (!world)
    {
        return;
    }
    AICenter* occupier = FindSideCenter(registry.OccupierSide());
    AICenter* resistance = FindSideCenter(registry.ResistanceSide());
    AutoArray<SightingReport> reports;
    if (occupier && resistance)
    {
        const TargetSide resSide = resistance->GetSide();
        for (int g = 0; g < occupier->NGroups(); g++)
        {
            AIGroup* grp = occupier->GetGroup(g);
            if (!grp)
            {
                continue;
            }
            bool alive = false;
            for (int u = 0; u < MAX_UNITS_PER_GROUP && !alive; u++)
            {
                AIUnit* unit = grp->UnitWithID(u + 1);
                alive = unit && unit->GetLifeState() == AIUnit::LSAlive;
            }
            if (!alive)
            {
                continue; // a destroyed group reports nothing new; its last report stays
            }
            const TargetList& list = grp->GetTargetList();
            for (int i = 0; i < list.Size(); i++)
            {
                const Target* t = list[i];
                if (!t || t->destroyed || t->vanished || !t->IsKnown() || t->side != resSide)
                {
                    continue;
                }
                SightingReport rep;
                rep.id = (unsigned long long)(uintptr_t)t->idExact.GetLink();
                rep.pos = t->posReported; // what the group reported, never the true position
                rep.zone = ZoneOfPosition(registry, rep.pos);
                reports.Add(rep);
            }
        }
    }
    IngestSightings(registry, reports);
}

// ---------------------------------------------------------------------------
// persistence
// ---------------------------------------------------------------------------

void OccupierForces::DeriveFromExistingSave(const ZoneRegistry& registry)
{
    if (!_tuning.enabled || !registry.IsActive() || _active)
    {
        return;
    }
    if (!Activate(registry))
    {
        return;
    }
    // the pool from config, scaled by how much the occupier still controls
    _control = ComputeControl(registry);
    int pool = RoundToInt((float)_poolStart * _control);
    _pool = pool < 0 ? 0 : (pool > _poolStart ? _poolStart : pool);
    // thresholds already crossed never fire
    _wavesFired = WavesCrossed(_tuning, _control);
    // the distinct-town set from the towns the resistance holds now
    _townsEver.Clear();
    const RString resistance = registry.ResistanceSide();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (z && !z->garrisonExempt && stricmp(z->type, "CITY") == 0 && stricmp(z->owner, resistance) == 0)
        {
            NameRow row;
            row.name = z->name;
            _townsEver.Add(row);
        }
    }
    _sightings.Clear();
    _tokens = 0; // a migrated save does not launch at once
    // invariant W from the derived set: awake with no tokens when enough
    // distinct towns are already held
    _posture = _townsEver.Size() >= _tuning.wakeTowns ? OFAwake : OFDormant;
    _legacySave = true;
    _band = InitialBand(_pool, _poolStart);
    LOG_INFO(Core,
             "OccupierForces: derived from a save without the block: pool {} of {} (control {:.2f}), {} wave "
             "threshold(s) marked fired, {} town(s) taken, band {}",
             _pool, _poolStart, (double)_control, _wavesFired, _townsEver.Size(), BandName(_band));
}

LSError OccupierForces::NameRow::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("name", name, 1, RString()))
    return LSOK;
}

LSError OccupierForces::ZoneClockRow::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("zone", zone, 1, RString()))
    PARAM_CHECK(ar.Serialize("seconds", seconds, 1, 0.0f))
    return LSOK;
}

LSError OccupierForces::SightingRow::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("zone", zone, 1, RString()))
    PARAM_CHECK(ar.Serialize("count", count, 1, 0))
    PARAM_CHECK(ar.Serialize("stamp", stamp, 1, 0.0f))
    PARAM_CHECK(ar.SerializeArray("positions", positions, 1))
    return LSOK;
}

LSError OccupierForces::OpRow::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("opId", opId, 1, 0))
    PARAM_CHECK(ar.Serialize("target", target, 1, RString()))
    PARAM_CHECK(ar.Serialize("kind", kind, 1, (int)CAKRetake))
    PARAM_CHECK(ar.Serialize("origin", origin, 1, (int)CAODirector))
    PARAM_CHECK(ar.Serialize("package", package, 1, (int)FPFoot))
    PARAM_CHECK(ar.Serialize("outstanding", outstanding, 1, 0))
    PARAM_CHECK(ar.Serialize("born", born, 1, false))
    PARAM_CHECK(ar.Serialize("finished", finished, 1, false))
    return LSOK;
}

LSError OccupierForces::Serialize(ParamArchive& ar)
{
    // plain values and name rows only (the CampaignEnd idiom): the World
    // serializer calls us on both load passes; only the first reads
    if (ar.IsLoading() && ar.GetPass() != ParamArchive::PassFirst)
    {
        // the mission config was reparsed during the first pass
        LoadFromConfig();
        return LSOK;
    }
    PARAM_CHECK(ar.Serialize("active", _active, 1, false))
    PARAM_CHECK(ar.Serialize("pool", _pool, 1, 0))
    PARAM_CHECK(ar.Serialize("poolStart", _poolStart, 1, 0))
    PARAM_CHECK(ar.Serialize("R", _R, 1, 0))
    int k = _towns.Size(); // informational: K is the Towns row count
    PARAM_CHECK(ar.Serialize("K", k, 1, 0))
    PARAM_CHECK(ar.Serialize("G", _G, 1, 0))
    // empty arrays write no subclass and load back empty
    PARAM_CHECK(ar.Serialize("Roster", _roster, 1))
    PARAM_CHECK(ar.Serialize("Towns", _towns, 1))
    PARAM_CHECK(ar.Serialize("Excluded", _excluded, 1))
    PARAM_CHECK(ar.Serialize("TownsEver", _townsEver, 1))
    PARAM_CHECK(ar.Serialize("posture", _posture, 1, (int)OFDormant))
    PARAM_CHECK(ar.Serialize("tokens", _tokens, 1, 0))
    PARAM_CHECK(ar.Serialize("tokenClock", _tokenClock, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("Urgent", _urgent, 1))
    PARAM_CHECK(ar.Serialize("directorAccum", _directorAccum, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("opCounter", _opCounter, 1, 0))
    PARAM_CHECK(ar.Serialize("refillAccum", _refillAccum, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("band", _band, 1, (int)OFBandStrong))
    PARAM_CHECK(ar.Serialize("control", _control, 1, 1.0f))
    PARAM_CHECK(ar.Serialize("wavesFired", _wavesFired, 1, 0))
    PARAM_CHECK(ar.Serialize("finalFired", _finalFired, 1, false))
    PARAM_CHECK(ar.Serialize("wavesLanded", _wavesLanded, 1, 0))
    PARAM_CHECK(ar.Serialize("waveRequests", _waveRequests, 1, 0))
    PARAM_CHECK(ar.Serialize("Cooldowns", _cooldowns, 1))
    PARAM_CHECK(ar.Serialize("Sightings", _sightings, 1))
    PARAM_CHECK(ar.Serialize("lost", _lost, 1, 0))
    PARAM_CHECK(ar.Serialize("refillTotal", _refillTotal, 1, 0))
    PARAM_CHECK(ar.Serialize("wavesTotal", _wavesTotal, 1, 0))
    PARAM_CHECK(ar.Serialize("legacySave", _legacySave, 1, false))
    // the director (phase 3; absent in a phase-2 save: the defaults)
    PARAM_CHECK(ar.Serialize("clock", _clock, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("RedPaid", _redPaid, 1))
    PARAM_CHECK(ar.Serialize("Ops", _ops, 1))
    if (ar.IsLoading())
    {
        // transient director state is re-derived: the alert baseline from
        // the next scan (no false edges), the catalogue from the config
        _alertPrev.Clear();
        _scanAccum = 0;
        _basesDeferredLogged = false;
        _catalogueBuilt = false;
        _catalogue = ForcePackageCatalogue();
        if (this == &Instance())
        {
            CounterAttack::SetListener(this);
        }
        if (_band < 0 || _band >= NOccupierBands)
        {
            _band = InitialBand(_pool, _poolStart);
        }
        if (k != _towns.Size())
        {
            LOG_WARN(Core, "OccupierForces: saved K {} but {} town row(s); the rows win", k, _towns.Size());
        }
        _restored = true;
        _activationTried = true; // a restored campaign is never re-sized
        _logClock = LogQuiet;
        _logSuppressed = 0;
    }
    return LSOK;
}

} // namespace Poseidon::Guerrilla
