#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>

#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp> // IsMilitaryType / IsAuthoredOccupier (the roster predicate)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
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
    _cooldowns.Clear();
    _sightings.Clear();
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

void OccupierForces::Simulate(float deltaT)
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
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
    if (_directorAccum < _tuning.directorInterval)
    {
        return;
    }
    const float elapsed = _directorAccum;
    _directorAccum = 0;
    Tick(registry, elapsed);
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
    if (ar.IsLoading())
    {
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
