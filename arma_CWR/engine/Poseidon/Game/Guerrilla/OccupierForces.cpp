#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>

#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>  // alert states, alertInterval (read only)
#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp>   // IsMilitaryType / IsAuthoredOccupier, the win latch
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp> // LaunchFunded, the listener, Roll01 / MakeSeed / Spare
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp> // live garrison bodies (a base's spare)
#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp> // HqZone: never a target
#include <Poseidon/Game/Guerrilla/RoadRoute.hpp>     // SameLandmass: ground sources share the target's land
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/Core/Global.hpp> // Glob.time (the town-strike stamps)
#include <Poseidon/World/World.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/VehicleAI.hpp> // Target / TargetList (the sightings scan)

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <stdint.h>
#include <stdio.h> // snprintf (wave labels)
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
    _catalogueBuilt = false;
    _catalogue = ForcePackageCatalogue();
    _waveInbound = false;
    _waveClock = 0;
    _waveBodies = 0;
    _waveFinalStand = false;
    _waveCount = 0;
    _controlDwell = 0;
    _waveWindow = 0;
    _surgeZone = RString();
    _surgeBodies = 0;
    _basesHeld = -1;
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
    _townStrikes = 0;
    _strikesApplied = 0;
    _alarmAnswers = 0;
    _loadedFormat = 0;
    _missionNow = 0;
    _alarmStarved.Clear();
    // the S5 refund hook (3.3): the process-wide service always owns it (it
    // claims nothing while inactive, so a save loaded after Clear keeps it); a
    // local service that installed it gives it up
    if (this == &Instance())
    {
        InstallPendingRefundHook();
    }
    else
    {
        RemovePendingRefundHook();
    }
}

namespace
{
// the service the S5 refund hook credits; constant-initialized
OccupierForces* GPendingRefundTarget = nullptr;
} // namespace

OccupierForces::~OccupierForces()
{
    RemovePendingRefundHook();
}

void OccupierForces::InstallPendingRefundHook()
{
    GPendingRefundTarget = this;
    GarrisonCache::SetPendingRefundHook(&OccupierForces::RefundPendingToPool);
}

void OccupierForces::RemovePendingRefundHook()
{
    if (GPendingRefundTarget != this)
    {
        return;
    }
    GPendingRefundTarget = nullptr;
    if (GarrisonCache::GetPendingRefundHook() == &OccupierForces::RefundPendingToPool)
    {
        GarrisonCache::SetPendingRefundHook(nullptr);
    }
}

bool OccupierForces::RefundPendingToPool(ZoneRegistry& registry, int zone, int originZone, int bodies)
{
    (void)originZone;
    OccupierForces* of = GPendingRefundTarget;
    if (!of || !of->Active() || bodies <= 0)
    {
        return false; // the key off (or not sized): the phase-1 origin / discard path
    }
    of->Refund(bodies, "pending deposit, zone changed hands");
    const ZoneRecord* z = registry.GetZone(zone);
    LOG_INFO(Core, "OccupierForces: {} pending body(ies) at {} back to the pool (the zone changed hands), pool {}",
             bodies, z ? (const char*)z->name : "?", of->Pool());
    return true;
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
    t.enabled = c.ReadValue("occupierForces", OccupierForcesTuning::KeyDefault) != 0.0f;
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
    // phase 4: the HQ assault and the waves (16)
    t.hqAssaultMin = RoundToInt(ReadRanged(c, "forcesHqAssaultMin", (float)t.hqAssaultMin, 1.0f, 12.0f));
    t.waveDwell = RoundToInt(ReadRanged(c, "forcesControlDwell", (float)t.waveDwell, 1.0f, 10.0f));
    t.waveFinal = c.ReadValue("forcesWaveFinal", t.waveFinal ? 1.0f : 0.0f) != 0.0f;
    t.waveShare = ReadRanged(c, "forcesWaveShare", t.waveShare, 0.0f, 2.0f);
    t.waveMin = RoundToInt(ReadRanged(c, "forcesWaveMin", (float)t.waveMin, 0.0f, 200.0f));
    t.waveMax = RoundToInt(ReadRanged(c, "forcesWaveMax", (float)t.waveMax, 0.0f, 400.0f));
    t.waveLead = ReadRanged(c, "forcesWaveLead", t.waveLead, 0.0f, 7200.0f);
    t.waveLeadFinal = ReadRanged(c, "forcesWaveLeadFinal", t.waveLeadFinal, 0.0f, 7200.0f);
    t.waveRetakeShare = ReadRanged(c, "forcesWaveRetakeShare", t.waveRetakeShare, 0.0f, 1.0f);
    t.waveOps = RoundToInt(ReadRanged(c, "forcesWaveOps", (float)t.waveOps, 0.0f, 8.0f));
    t.waveCluster = ReadRanged(c, "forcesWaveCluster", t.waveCluster, 100.0f, 20000.0f);
    t.waveSurge = RoundToInt(ReadRanged(c, "forcesWaveSurge", (float)t.waveSurge, 0.0f, 48.0f));
    t.waveHub = c.ReadValue("forcesWaveHub", RString());

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

RString OccupierForces::AuthoredOwnerOf(const ZoneRegistry& registry, const ZoneRecord& z)
{
    if (stricmp(z.ownerConfig, "OCCUPIER") == 0)
    {
        return registry.OccupierSide();
    }
    if (stricmp(z.ownerConfig, "RESISTANCE") == 0)
    {
        return registry.ResistanceSide();
    }
    return z.ownerConfig;
}

void OccupierForces::Size(const ZoneRegistry& registry, const OccupierForcesTuning& t, OccupierForcesSizing& out,
                          const AutoArray<bool>* seeded, const SettlementProbe* probe, bool log, bool authoredOwner)
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
        // M7: a save that predates the block classifies K by the authored
        // owner, so a town the resistance holds at load still counts (T, over
        // the current owners, then shows it lost).  A fresh campaign sizes on
        // its first frame, where the current owner IS the authored one.
        bool administered = false;
        if (authoredOwner)
        {
            const RString owner = AuthoredOwnerOf(registry, *z);
            administered = stricmp(owner, "NEUTRAL") == 0 || stricmp(owner, registry.OccupierSide()) == 0;
        }
        else
        {
            administered = registry.IsAdministeredTown(*z);
        }
        if (!administered)
        {
            reason = authoredOwner ? "not administered as authored" : "not administered";
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

bool OccupierForces::Activate(const ZoneRegistry& registry, const AutoArray<bool>* seeded, const SettlementProbe* probe,
                              bool authoredOwner)
{
    _activationTried = true;
    if (!registry.IsActive())
    {
        return false;
    }
    const auto start = std::chrono::steady_clock::now();
    OccupierForcesSizing s;
    Size(registry, _tuning, s, seeded, probe, true, authoredOwner);
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
    // the final stand's trigger reads this (StepWaves); a surge on a base
    // the occupier lost is gone with it
    _basesHeld = RosterBasesHeld(registry);
    if (_surgeBodies > 0)
    {
        const ZoneRecord* s = registry.GetZone(registry.FindZoneIndex(_surgeZone));
        if (!s || stricmp(s->owner, registry.OccupierSide()) != 0)
        {
            _surgeBodies = 0;
            _surgeZone = RString();
        }
    }
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
    return OccupierWarLevelFrom(ReadWarLevel());
}

float OccupierForces::OccupierWarLevelFrom(float territory) const
{
    return OccupierWarLevelOf(territory, Active() ? _wavesLanded : 0, _tuning.waveWarLevel);
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

void OccupierForces::RequestWave(const ZoneRegistry& registry, const DirectorView& view, int n)
{
    _waveRequests++;
    if (!Active())
    {
        LOG_INFO(Core, "OccupierForces: wave {} requested while the service is inert; parked, no effect", n);
        return;
    }
    // wave n: its thresholds are marked fired (each fires once); beyond the
    // thresholds it is the final stand; 0 = the next threshold
    if (n <= 0)
    {
        n = _wavesFired + 1;
    }
    const bool final = n > _tuning.nWaveThresholds;
    if (final)
    {
        _finalFired = true;
    }
    else if (n > _wavesFired)
    {
        _wavesFired = n;
    }
    _controlDwell = 0;
    char why[48];
    snprintf(why, sizeof(why), "forced: wave %d", n);
    FireWave(registry, view, final, why);
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
    // the director's own slots: wave operations come on top (7.1)
    int BornOps() const override { return CounterAttack::Instance().BornOps(false); }
    bool HasRecord(int zoneIndex) const override
    {
        const ZoneRecord* z = _registry.GetZone(zoneIndex);
        return z && CounterAttack::Instance().FindByTarget(z->name) >= 0;
    }
    bool HasHqRecord() const override { return CounterAttack::Instance().FindHqAssault() >= 0; }

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
    _missionNow = Glob.time - Foundation::Time(0);
    _directorAccum += deltaT;
    // the sightings scan at the alert cadence (6.1)
    _scanAccum += deltaT;
    const float scanEvery = AlertMachine::Instance().Tuning().alertInterval;
    const bool directorDue = _directorAccum >= _tuning.directorInterval;
    if (_scanAccum >= (scanEvery > 0 ? scanEvery : 5.0f))
    {
        _scanAccum = 0;
        ScanWorld(registry);
        // alarm answers at the sighting cadence (8.2): a RED zone is answered
        // within one scan, not one director tick (Direct runs them too)
        if (!directorDue && !_legacySave)
        {
            if (!_catalogueBuilt)
            {
                BuildEngineCatalogue(registry);
            }
            DirectorView view;
            FillEngineView(registry, view);
            EngineDirectorLauncher launcher(registry);
            AnswerAlarms(registry, view, launcher);
        }
    }
    if (!directorDue)
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

void OccupierForces::RequestWave(int n)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    DirectorView view;
    if (Active())
    {
        FillEngineView(registry, view);
    }
    RequestWave(registry, view, n);
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

bool OccupierForces::FinalPush(const ZoneRegistry& registry, int hqZone)
{
    const RString resistance = registry.ResistanceSide();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || i == hqZone || stricmp(z->owner, resistance) != 0)
        {
            continue;
        }
        // a held base, or a held town (the HQ's own zone does not count: it
        // is what the final push is for)
        if (CampaignEnd::IsMilitaryType(z->type) || stricmp(z->type, "CITY") == 0)
        {
            return false;
        }
    }
    return true;
}

void OccupierForces::Candidates(const ZoneRegistry& registry, const DirectorView& view,
                                const DirectorLauncher& launcher, AutoArray<int>& out)
{
    out.Clear();
    const RString resistance = registry.ResistanceSide();
    const float full = _tuning.heatFull > 0 ? _tuning.heatFull : 1.0f;
    const bool strong = _band == OFBandStrong;
    // hottest first (stable on the zone order)
    auto insertByHeat = [&registry, &out](int zone, float heat) {
        int at = out.Size();
        while (at > 0)
        {
            const ZoneRecord* prev = out[at - 1] >= 0 ? registry.GetZone(out[at - 1]) : nullptr;
            if (!prev || prev->heat >= heat)
            {
                break;
            }
            at--;
        }
        out.Add(zone);
        for (int k = out.Size() - 1; k > at; k--)
        {
            out[k] = out[k - 1];
        }
        out[at] = zone;
    };
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
            // D16: a held base is a candidate by the same heat rule, only
            // while STRONG (its flip also gave +40)
            if (!strong || !hot || i == view.hqZone || Cooldown(z->name) > 0 || launcher.HasRecord(i))
            {
                continue;
            }
            insertByHeat(i, z->heat);
            continue;
        }
        if (!IsHeldTown(registry, *z) || i == view.hqZone || Cooldown(z->name) > 0 || launcher.HasRecord(i) || !hot)
        {
            continue;
        }
        insertByHeat(i, z->heat);
    }
    // the HQ (D13, D14, D24): an established HQ with no assault on it and no
    // cooldown; while STRONG or in a wave by the heat of the zone it stands
    // in (none for an HQ outside every zone), and unconditionally once the
    // resistance holds no base and no town besides the HQ's zone
    if (view.hqEstablished && !launcher.HasHqRecord() && Cooldown(HqCooldownName) <= 0)
    {
        const ZoneRecord* hz = registry.GetZone(view.hqZone);
        const bool finalPush = FinalPush(registry, view.hqZone);
        const bool hotHq = hz && hz->heat / full >= CandidateRoll(*hz);
        if (finalPush)
        {
            out.Add(HqCandidate);
            for (int k = out.Size() - 1; k > 0; k--)
            {
                out[k] = out[k - 1];
            }
            out[0] = HqCandidate;
        }
        else if ((strong || InWave()) && hotHq)
        {
            int at = out.Size();
            while (at > 0)
            {
                const ZoneRecord* prev = out[at - 1] >= 0 ? registry.GetZone(out[at - 1]) : nullptr;
                if (!prev || prev->heat >= hz->heat)
                {
                    break;
                }
                at--;
            }
            out.Add(HqCandidate);
            for (int k = out.Size() - 1; k > at; k--)
            {
                out[k] = out[k - 1];
            }
            out[at] = HqCandidate;
        }
    }
}

int OccupierForces::PushoutTarget(const ZoneRegistry& registry, const DirectorView& view,
                                  const DirectorLauncher& launcher)
{
    const RString occupier = registry.OccupierSide();
    const RString resistance = registry.ResistanceSide();
    int best = -1;
    float bestDist = 0;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || i == view.hqZone || Cooldown(z->name) > 0 || launcher.HasRecord(i))
        {
            continue;
        }
        // a quiet held town, or a quiet held base (6.4, N2)
        const bool heldBase = CampaignEnd::IsMilitaryType(z->type) && stricmp(z->owner, resistance) == 0;
        if (!IsHeldTown(registry, *z) && !heldBase)
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
                                       int origin, int fixedSource, int fixedSpare) const
{
    DirectorAction a;
    a.origin = origin;
    const bool hq = target == HqCandidate;
    const ZoneRecord* z = hq ? registry.GetZone(view.hqZone) : registry.GetZone(target);
    Vector3 dest = VZero;
    if (hq)
    {
        if (!view.hqEstablished)
        {
            a.choice.why = "there is no HQ";
            return a;
        }
        a.kind = CAKHqAssault;
        a.target = z ? view.hqZone : -1;
        a.destPos = view.hqPos;
        dest = view.hqPos;
    }
    else
    {
        if (!z)
        {
            a.choice.why = "no such zone";
            return a;
        }
        a.target = target;
        a.kind = CampaignEnd::IsMilitaryType(z->type) ? CAKBase : CAKRetake;
        dest = z->pos;
    }
    a.urgent = origin == CAODirector && z && Urgent(z->name);
    a.fighters = z ? (float)KnownFighters(registry, hq ? view.hqZone : target) : 0.0f;
    PackageRequest req;
    req.fighters = a.fighters;
    req.defenderWeight = view.defenderWeight;
    req.margin = _tuning.lethalityMargin;
    // a retake: max(forcesRetakeMin, garrisonMax), at most one group (a
    // town authored above twelve defenders is retaken by a full group); an
    // HQ assault: forcesHqAssaultMin (6.5)
    req.minBodies = hq ? _tuning.hqAssaultMin
                       : MinInt(MaxInt(_tuning.retakeMin, RoundToInt(z->garrisonMax)), ForcePackages::MaxGroup);
    req.urgent = a.urgent;
    req.urgentCostShare = _tuning.urgentCostShare;
    req.tierFactor = view.tierFactor;
    req.pool = _pool;
    req.poolActive = Active();
    req.offered = OfferedPackages;
    AutoArray<PackageSource> sources;
    BuildSources(registry, view, dest, fixedSource, fixedSpare, -1, sources);
    // phase 6: a helicopter insertion competes like any package (6.5, D3)
    a.airOrigin = HeldAirfield(registry, dest);
    a.choice = ChooseWithAir(req, sources, a.airOrigin);
    a.go = a.choice.ok;
    return a;
}

void OccupierForces::BuildSources(const ZoneRegistry& registry, const DirectorView& view, Vector3Par dest,
                                  int fixedSource, int fixedSpare, int exclude, AutoArray<PackageSource>& sources) const
{
    sources.Clear();
    const RString occupier = registry.OccupierSide();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* s = registry.GetZone(i);
        if (!s || !CampaignEnd::IsMilitaryType(s->type) || stricmp(s->owner, occupier) != 0)
        {
            continue;
        }
        if ((fixedSource >= 0 && i != fixedSource) || i == exclude)
        {
            continue;
        }
        // the final stand's base never sources an operation (7.1)
        if (_surgeBodies > 0 && stricmp(s->name, _surgeZone) == 0)
        {
            continue;
        }
        // ground packages walk or drive: a base on another landmass than the
        // target (Malden's Southeast Compound) is an AIR-ONLY source (phase
        // 6A): its reserve can still pay a helicopter insertion, never a
        // ground package (ForcePackages::Quote skips it for those)
        PackageSource src;
        src.zone = i;
        src.airOnly = !RoadRoute::SameLandmass(s->pos, dest);
        if (fixedSource >= 0)
        {
            src.spare = fixedSpare;
            src.spareStored = fixedSpare;
        }
        else
        {
            src.spare = i < view.spare.Size() ? view.spare[i] : 0;
            src.spareStored = i < view.spareStored.Size() ? view.spareStored[i] : -1;
        }
        if (src.spare <= 0 && src.spareStored <= 0)
        {
            continue;
        }
        const float dx = s->pos.X() - dest.X();
        const float dz = s->pos.Z() - dest.Z();
        src.distance = sqrtf(dx * dx + dz * dz);
        sources.Add(src);
    }
}

PackageChoice OccupierForces::ChooseWithAir(const PackageRequest& reqIn, const AutoArray<PackageSource>& sources,
                                            int airOrigin) const
{
    // the transport helicopter competes like any package (6.5); off-map (no
    // occupier-held airfield, D3) ForcePackages quotes it at the off-map
    // price.  The strike airframes carry nobody (SeatCap 0): never chosen.
    PackageRequest req = reqIn;
    if (_catalogue.Offered(FPTransportHeli))
    {
        req.offered |= ForcePackages::Bit(FPTransportHeli);
    }
    req.airOffMap = airOrigin < 0;
    return ForcePackages::Choose(req, _catalogue, sources);
}

int OccupierForces::HeldAirfield(const ZoneRegistry& registry, Vector3Par pos)
{
    const RString occupier = registry.OccupierSide();
    int best = -1;
    float bestSq = 0;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || stricmp(z->type, "AIRFIELD") != 0 || stricmp(z->owner, occupier) != 0)
        {
            continue;
        }
        const float dx = z->pos.X() - pos.X();
        const float dz = z->pos.Z() - pos.Z();
        const float dSq = dx * dx + dz * dz;
        if (best < 0 || dSq < bestSq)
        {
            best = i;
            bestSq = dSq;
        }
    }
    return best;
}

static const char* ZoneNameOf(const ZoneRegistry& registry, int index)
{
    const ZoneRecord* z = registry.GetZone(index);
    return z ? (const char*)z->name : "off-map";
}

bool OccupierForces::TryLaunch(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, int target,
                               int origin)
{
    const bool hq = target == HqCandidate;
    const ZoneRecord* z = registry.GetZone(target);
    if (!z && !hq)
    {
        return false;
    }
    // M1: when CounterAttack refuses the chosen source, the same target is
    // re-planned without it (a local copy of the view with its spare at 0),
    // a bounded number of times.  Still one operation per tick: the loop
    // ends at the first launch.
    DirectorView local;
    const DirectorView* v = &view;
    for (int attempt = 0; attempt <= MaxSourceRetries; attempt++)
    {
        const DirectorAction a = PlanFor(registry, *v, target, origin);
        if (!a.go)
        {
            if (LogAllowed())
            {
                LOG_INFO(Core, "OccupierForces: no operation on {}: {}{}", hq ? "the HQ" : (const char*)z->name,
                         (const char*)a.choice.why, _logSuppressed > 0 ? " (other lines suppressed)" : "");
                _logSuppressed = 0;
            }
            return false;
        }
        bool refused = false;
        if (LaunchAction(registry, *v, launcher, a, true, false, 0, &refused))
        {
            return true;
        }
        const int source = a.choice.quote.source;
        if (!refused || source < 0)
        {
            return false; // the pool refused: no other source changes that
        }
        if (v != &local)
        {
            local = view;
            v = &local;
        }
        if (source < local.spare.Size())
        {
            local.spare[source] = 0;
        }
        if (source < local.spareStored.Size())
        {
            local.spareStored[source] = 0;
        }
    }
    return false;
}

bool OccupierForces::LaunchAction(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher,
                                  const DirectorAction& a, bool spendToken, bool staged, int waveNo,
                                  bool* launcherRefused)
{
    const bool hq = a.kind == CAKHqAssault;
    const ZoneRecord* z = registry.GetZone(a.target);
    // a strike on the HQ outside every zone has no zone either
    if (!z && !hq && !(a.kind == CAKStrike && a.strikePos.X() != 0))
    {
        return false;
    }
    const char* targetName = z ? (const char*)z->name : (hq ? "the HQ" : "a reported position");
    const PackageQuote& q = a.choice.quote;
    if (launcherRefused)
    {
        *launcherRefused = false;
    }
    // L1: a refused launch must not leave the band demoted by its own
    // debit (the band's hysteresis would keep it there after the refund)
    const int bandBefore = _band;
    const int opId = _opCounter + 1;
    CounterAttackOrder order;
    order.opId = opId;
    order.kind = a.kind;
    order.origin = a.origin;
    order.package = q.package;
    order.target = a.target;
    order.source = q.source;
    order.bodies = q.bodies;
    order.crew = q.crew;
    order.hullClass = q.hullClass;
    order.priceUnits = q.poolPrice;
    order.urgent = a.urgent;
    order.abstractSpeed = q.speed;
    order.funded = true;
    order.destPos = a.destPos;
    // a column's escort (6.5, phase 5): CounterAttack splits its share off
    order.escortClass = q.escortClass;
    order.escortCrew = q.escortCrew;
    order.escortPriceUnits = q.EscortPoolPrice();
    // phase 6: air and the new kinds (CounterAttackOrders.hpp)
    order.airOrigin = a.airOrigin;
    order.strikeZone = a.strikeZone;
    order.strikePos = a.strikePos;
    order.alarmZone = a.alarmZone;
    order.alarmPos = a.alarmPos;
    // the pool pays hull and crew BEFORE the launch, and gets it back when
    // CounterAttack refuses the order
    if (q.poolPrice > 0 && !Spend(q.poolPrice, "operation hull and crew"))
    {
        return false;
    }
    // a wave operation's infantry comes out of the pool too: staged as a
    // deposit straight into the source's reserve, which the launch debits in
    // this same tick (7.1)
    int stagedBodies = 0;
    ZoneRecord* src = registry.GetZoneMutable(q.source);
    if (staged && src)
    {
        stagedBodies = Deposit(q.bodies, "wave staging");
        if (stagedBodies <= 0)
        {
            Refund(q.poolPrice, "wave staging refused");
            _band = bandBefore;
            return false;
        }
        src->garrison = floorf(src->garrison > 0 ? src->garrison : 0) + (float)stagedBodies;
        order.bodies = stagedBodies;
    }
    // the ledger row exists before the launch: events raised while
    // CounterAttack starts the record find it
    OpRow row;
    row.opId = opId;
    row.target = z ? z->name : RString(hq ? HqCooldownName : "");
    row.kind = a.kind;
    row.origin = a.origin;
    row.package = q.package;
    row.outstanding = q.poolPrice;
    _ops.Add(row);
    _opCounter = opId;
    if (!launcher.Launch(order))
    {
        Refund(q.poolPrice, "operation refused");
        if (stagedBodies > 0 && src)
        {
            // the staging goes back where it came from
            src->garrison = floorf(src->garrison) - (float)stagedBodies;
            if (src->garrison < 0)
            {
                src->garrison = 0;
            }
            Refund(stagedBodies, "wave staging refused");
        }
        const int r = FindOp(opId);
        if (r >= 0)
        {
            _ops.Delete(r);
        }
        // the debit and its refund leave the pool where it was, so the band
        // goes back too.  The op id stays spent (L1): CounterAttack's
        // "[op N] refused" line and the next "[op N]" are never the same id.
        if (_band != bandBefore)
        {
            LOG_DEBUG(Core, "OccupierForces: band back to {} after the refusal", BandName(bandBefore));
            _band = bandBefore;
        }
        if (launcherRefused)
        {
            *launcherRefused = true;
        }
        LOG_INFO(Core, "OccupierForces: [op {}] CounterAttack refused the {} operation from {} to {}", opId,
                 ForcePackages::PackageName(q.package), ZoneNameOf(registry, q.source), targetName);
        return false;
    }
    if (spendToken)
    {
        _tokens = _tokens > 0 ? _tokens - 1 : 0;
    }
    // a RED edge from here on pays the heat of a repel
    if (z)
    {
        RemoveName(_redPaid, z->name);
    }
    PruneOps();

    const bool push = a.origin == CAOPushout;
    const char* shortNote =
        a.choice.shortOfLethality ? ", short of the required lethality: the most lethal affordable package" : "";
    char wave[32] = "";
    if (waveNo > 0)
    {
        snprintf(wave, sizeof(wave), a.kind == CAKReinforce ? "Wave %d: " : "Wave %d strike: ", waveNo);
    }
    if (a.kind == CAKRetake && waveNo == 0)
    {
        // the phase-3 wording, unchanged (tri tests read it)
        const char* label = q.package == FPFoot            ? (push ? "foot squad" : "Foot squad")
                            : q.package == FPTruck         ? (push ? "motorized squad" : "Motorized squad")
                            : q.package == FPTransportHeli ? (push ? "helicopter insertion" : "Helicopter insertion")
                                                           : ForcePackages::PackageName(q.package);
        if (q.package == FPFoot)
        {
            LOG_INFO(Core,
                     "OccupierForces: [op {}] {}{} ({}) from {} to {} ({} unit(s), the pool pays {}, lethality {:.2f} "
                     "of {:.2f}, {} fighter(s) reported{}{}, about {:.0f} s)",
                     opId, push ? "Push-out: " : "", label, q.bodies, ZoneNameOf(registry, q.source), targetName,
                     q.price, q.poolPrice, (double)q.lethality, (double)a.choice.lReq, RoundToInt(a.fighters),
                     a.urgent ? ", urgent" : "", shortNote, (double)q.seconds);
        }
        else
        {
            char from[96];
            if (q.package == FPTransportHeli)
            {
                // the airframe's origin, then the base that pays the infantry
                snprintf(from, sizeof(from), "%s (infantry of %s)",
                         a.airOrigin >= 0 ? ZoneNameOf(registry, a.airOrigin) : "off-map",
                         ZoneNameOf(registry, q.source));
            }
            else
            {
                snprintf(from, sizeof(from), "%s", ZoneNameOf(registry, q.source));
            }
            LOG_INFO(Core,
                     "OccupierForces: [op {}] {}{} ({}, {}) from {} to {} ({} unit(s), the pool pays {}, lethality "
                     "{:.2f} of {:.2f}, {} fighter(s) reported{}{}, about {:.0f} s)",
                     opId, push ? "Push-out: " : "", label, q.bodies, (const char*)q.hullClass, from, targetName,
                     q.price, q.poolPrice, (double)q.lethality, (double)a.choice.lReq, RoundToInt(a.fighters),
                     a.urgent ? ", urgent" : "", shortNote, (double)q.seconds);
        }
        return true;
    }
    if (a.kind == CAKStrike)
    {
        // 6.6: "Jet strike over Houdan (town)"
        const char* air = q.package == FPJet ? "Jet strike" : "Attack helicopter strike";
        const char* what = z && stricmp(z->type, "CITY") == 0 ? "town" : (hq ? "the HQ" : "opener");
        LOG_INFO(Core,
                 "OccupierForces: [op {}] {} over {} ({}, {}) from {} (the pool pays {}, {} fighter(s) reported, at "
                 "{}){}",
                 opId, air, targetName, what, (const char*)q.hullClass,
                 a.airOrigin >= 0 ? ZoneNameOf(registry, a.airOrigin) : "off-map", q.poolPrice,
                 RoundToInt(a.fighters), a.strikePos.X() != 0 ? "the reported fighters" : "the town centre",
                 a.urgent ? ", urgent" : "");
        return true;
    }
    if (a.kind == CAKAlarm)
    {
        // 6.6: "Alarm answer: APC squad from Airfield to Outpost"
        char from[96];
        if (q.package == FPTransportHeli)
        {
            snprintf(from, sizeof(from), "%s (infantry of %s)",
                     a.airOrigin >= 0 ? ZoneNameOf(registry, a.airOrigin) : "off-map", ZoneNameOf(registry, q.source));
        }
        else
        {
            snprintf(from, sizeof(from), "%s", ZoneNameOf(registry, q.source));
        }
        LOG_INFO(Core,
                 "OccupierForces: [op {}] Alarm answer: {} ({}{}{}) from {} to {} ({} unit(s), the pool pays {}, "
                 "lethality {:.2f} of {:.2f}, {} fighter(s) reported{}{}, about {:.0f} s)",
                 opId, ForcePackages::PackageName(q.package), q.bodies, q.package != FPFoot ? ", " : "",
                 (const char*)q.hullClass, from, targetName, q.price, q.poolPrice, (double)q.lethality,
                 (double)a.choice.lReq, RoundToInt(a.fighters), a.urgent ? ", urgent" : ", THIN: the cheapest",
                 shortNote, (double)q.seconds);
        return true;
    }
    // base retakes, HQ assaults and wave strikes (6.6 wording)
    const char* what = hq                          ? "HQ assault"
                       : a.kind == CAKBase         ? "Base retake"
                       : a.kind == CAKReinforce    ? "Reinforce column"
                                                   : "Town retake";
    const char* pkg = q.package == FPFoot ? "foot" : (q.package == FPTruck ? "motorized" : "package");
    LOG_INFO(Core,
             "OccupierForces: [op {}] {}{}{} ({} {}{}{}) from {} on {} ({} unit(s), the pool pays {}{}, lethality "
             "{:.2f} of {:.2f}, {} fighter(s) reported{}{}, about {:.0f} s)",
             opId, push ? "Push-out: " : "", wave, what, pkg, q.bodies, q.package != FPFoot ? ", " : "",
             (const char*)q.hullClass, ZoneNameOf(registry, q.source), targetName, q.price, q.poolPrice,
             stagedBodies > 0 ? " and stages the infantry" : "", (double)q.lethality, (double)a.choice.lReq,
             RoundToInt(a.fighters), a.urgent ? ", urgent" : "", shortNote, (double)q.seconds);
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
    StepStanding(registry, elapsed);
    // after the win (or the loss) nothing launches and no wave moves (7.4)
    if (view.won || view.lost)
    {
        return;
    }
    StepWaves(registry, view, launcher, elapsed);
    // alarm answers (8.2, phase 6): their own limits, no token, any posture
    AnswerAlarms(registry, view, launcher);
    // W: before the wake the director considers nothing
    if (_posture != OFAwake || _tokens <= 0 || !CanOperate())
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
        const int target = candidates[i];
        const ZoneRecord* tz = registry.GetZone(target);
        const int kind = target == HqCandidate ? CAKHqAssault
                         : (tz && CampaignEnd::IsMilitaryType(tz->type)) ? CAKBase
                                                                        : CAKRetake;
        if (TryLaunch(registry, view, launcher, target, CAODirector))
        {
            // the strike that goes with it (7.3): a base / HQ opener, or an
            // urgent town with fighters reported
            MaybeStrike(registry, view, launcher, target, kind);
            return; // one operation per tick
        }
        // no ground package could go: an urgent town with fighters may still
        // be struck from the air on its own (it takes the token)
        if (kind == CAKRetake && StrikeDue(registry, view, target, kind))
        {
            const DirectorAction sa = PlanStrike(registry, view, target, true);
            if (sa.go && LaunchAction(registry, view, launcher, sa, true, false, 0))
            {
                return;
            }
        }
    }
    if (candidates.Size() == 0 && _band == OFBandStrong)
    {
        // 6.4: strong, a token and nothing hot: retake a quiet town or base
        const int quiet = PushoutTarget(registry, view, launcher);
        if (quiet >= 0)
        {
            TryLaunch(registry, view, launcher, quiet, CAOPushout);
        }
    }
}

// ---------------------------------------------------------------------------
// waves (OCCUPIER-FORCES.md 7.1, D7; phase 4)
// ---------------------------------------------------------------------------

int OccupierForces::WaveSize(const OccupierForcesTuning& t, int poolStart)
{
    const int raw = RoundToInt(t.waveShare * (float)poolStart);
    const int lo = t.waveMin < t.waveMax ? t.waveMin : t.waveMax;
    return raw < lo ? lo : (raw > t.waveMax ? t.waveMax : raw);
}

float OccupierForces::ClusterScore(const ZoneRegistry& registry, int zi, float radius, int hqZone, Vector3Par hqPos)
{
    const ZoneRecord* z = registry.GetZone(zi);
    if (!z)
    {
        return 0;
    }
    const RString resistance = registry.ResistanceSide();
    const float rSq = radius * radius;
    float score = 0;
    bool hqCounted = false;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* y = registry.GetZone(i);
        if (!y || stricmp(y->owner, resistance) != 0)
        {
            continue;
        }
        const float dx = y->pos.X() - z->pos.X();
        const float dz = y->pos.Z() - z->pos.Z();
        if (dx * dx + dz * dz > rSq)
        {
            continue;
        }
        float w = 0;
        if (i == hqZone)
        {
            w = 4;
            hqCounted = true;
        }
        else if (CampaignEnd::IsMilitaryType(y->type))
        {
            w = 3;
        }
        else if (stricmp(y->type, "CITY") == 0)
        {
            w = 1;
        }
        score += w + y->heat / 40.0f;
    }
    // an HQ outside every held zone still draws the wave
    if (!hqCounted && (hqPos.X() != 0 || hqPos.Z() != 0))
    {
        const float dx = hqPos.X() - z->pos.X();
        const float dz = hqPos.Z() - z->pos.Z();
        if (dx * dx + dz * dz <= rSq)
        {
            score += 4;
        }
    }
    return score;
}

int OccupierForces::BestCluster(const ZoneRegistry& registry, float radius, int hqZone, Vector3Par hqPos)
{
    const RString resistance = registry.ResistanceSide();
    int best = -1;
    float bestScore = 0;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || stricmp(z->owner, resistance) != 0)
        {
            continue;
        }
        const float s = ClusterScore(registry, i, radius, hqZone, hqPos);
        if (best < 0 || s > bestScore)
        {
            best = i;
            bestScore = s;
        }
    }
    return best;
}

int OccupierForces::PickHub(const ZoneRegistry& registry, const OccupierForcesTuning& t, const AutoArray<int>* live)
{
    const RString occupier = registry.OccupierSide();
    auto held = [&registry, &occupier](const ZoneRecord* z) {
        return z && CampaignEnd::IsMilitaryType(z->type) && stricmp(z->owner, occupier) == 0;
    };
    if (t.waveHub.GetLength() > 0)
    {
        const int named = registry.FindZoneIndex(t.waveHub);
        if (held(registry.GetZone(named)))
        {
            return named;
        }
    }
    auto strength = [&registry, live](int i) {
        const ZoneRecord* z = registry.GetZone(i);
        const int l = live && i < live->Size() ? (*live)[i] : 0;
        return (z ? (int)floorf(z->garrison > 0 ? z->garrison : 0) + z->pending : 0) + l;
    };
    const char* order[2] = {"AIRFIELD", "SEAPORT"};
    for (int pass = 0; pass < 3; pass++)
    {
        int best = -1;
        for (int i = 0; i < registry.NZones(); i++)
        {
            const ZoneRecord* z = registry.GetZone(i);
            if (!held(z) || (pass < 2 && stricmp(z->type, order[pass]) != 0))
            {
                continue;
            }
            if (best < 0 || strength(i) > strength(best))
            {
                best = i;
            }
        }
        if (best >= 0)
        {
            return best;
        }
    }
    return -1;
}

int OccupierForces::RosterBasesHeld(const ZoneRegistry& registry) const
{
    const RString occupier = registry.OccupierSide();
    int n = 0;
    for (int i = 0; i < _roster.Size(); i++)
    {
        const ZoneRecord* z = registry.GetZone(registry.FindZoneIndex(_roster[i].name));
        n += z && stricmp(z->owner, occupier) == 0 ? 1 : 0;
    }
    return n;
}

int OccupierForces::SurgeAt(const char* zone) const
{
    return zone && _surgeBodies > 0 && stricmp(_surgeZone, zone) == 0 ? _surgeBodies : 0;
}

void OccupierForces::FireWave(const ZoneRegistry& registry, const DirectorView& view, bool final, const char* why)
{
    if (_waveInbound)
    {
        // a trigger due while a wave is inbound merges into it; a merged
        // final stand shortens the lead
        if (final && !_waveFinalStand)
        {
            _waveFinalStand = true;
            if (_waveClock > _tuning.waveLeadFinal)
            {
                _waveClock = _tuning.waveLeadFinal;
            }
        }
        LOG_INFO(Core, "OccupierForces: Wave {} absorbs a further trigger ({}){}", _waveCount, why ? why : "",
                 final ? ": it carries the final stand" : "");
        return;
    }
    _waveCount++;
    _waveInbound = true;
    _waveFinalStand = final;
    _waveBodies = WaveSize(_tuning, _poolStart);
    _waveClock = final ? _tuning.waveLeadFinal : _tuning.waveLead;
    const int hub = PickHub(registry, _tuning, &view.garrisonLive);
    LOG_INFO(Core, "OccupierForces: Wave {} ({}) inbound to {}, control {:.2f}{} ({}; lands in {:.0f} s)", _waveCount,
             _waveBodies, hub >= 0 ? ZoneNameOf(registry, hub) : "off-map", (double)_control,
             final ? ", the final stand" : "", why ? why : "", (double)_waveClock);
}

void OccupierForces::StepWaves(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher,
                               float elapsed)
{
    if (_waveWindow > 0)
    {
        _waveWindow -= elapsed;
        if (_waveWindow < 0)
        {
            _waveWindow = 0;
        }
    }
    // the triggers, on what Tick measured: control under a threshold not yet
    // fired, or one roster base left (the final stand)
    const int crossed = WavesCrossed(_tuning, _control);
    const bool thresholdDue = crossed > _wavesFired;
    const bool finalDue = _tuning.waveFinal && !_finalFired && _basesHeld == 1;
    if (thresholdDue || finalDue)
    {
        _controlDwell++;
        if (_controlDwell >= (_tuning.waveDwell > 0 ? _tuning.waveDwell : 1))
        {
            // everything due now merges into one wave, each trigger once
            char why[96];
            snprintf(why, sizeof(why), "%s%s%s", thresholdDue ? "control threshold" : "",
                     thresholdDue && finalDue ? " and " : "", finalDue ? "one base left" : "");
            if (thresholdDue)
            {
                _wavesFired = crossed;
            }
            if (finalDue)
            {
                _finalFired = true;
            }
            _controlDwell = 0;
            const bool merged = _waveInbound;
            FireWave(registry, view, finalDue, why);
            if (!merged)
            {
                return; // a wave fired this tick starts its full lead from now
            }
        }
    }
    else
    {
        _controlDwell = 0;
    }
    if (!_waveInbound)
    {
        return;
    }
    _waveClock -= elapsed;
    if (_waveClock > 0)
    {
        return;
    }
    // the landing
    const int bodies = _waveBodies;
    const bool final = _waveFinalStand;
    _waveInbound = false;
    _waveClock = 0;
    _waveBodies = 0;
    _waveFinalStand = false;
    _waveWindow = WaveWindow;
    LandWave(bodies);
    const int rest = AllocateWave(registry, view, launcher, bodies, final);
    LOG_INFO(Core, "OccupierForces: Wave {}: {} stay in the pool (pool {} of {})", _waveCount, rest, _pool,
             _poolStart);
}

int OccupierForces::AllocateWave(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher,
                                 int bodies, bool final)
{
    int budget = bodies;
    const RString occupier = registry.OccupierSide();
    float px = 0;
    float pz = 0;
    // the S5 deposit bubble only (where the player stood at the last zone
    // tick decides pending vs reserve, never what the director sends)
    const bool playerValid = registry.LastTickPlayer(px, pz);
    auto liveAt = [&view](int i) { return i >= 0 && i < view.garrisonLive.Size() ? view.garrisonLive[i] : 0; };
    auto shortfall = [&registry, &liveAt](int i) {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z)
        {
            return 0;
        }
        const float have = floorf(z->garrison > 0 ? z->garrison : 0) + (float)z->pending + (float)liveAt(i);
        const float missing = floorf(z->garrisonMax) - have;
        return missing > 0 ? (int)missing : 0;
    };
    // 1. the hub top-up
    const int hub = PickHub(registry, _tuning, &view.garrisonLive);
    if (hub >= 0 && budget > 0)
    {
        const int want = MinInt(shortfall(hub), budget);
        const int got = want > 0 ? Deposit(want, "wave hub top-up") : 0;
        if (got > 0)
        {
            const bool pending = GarrisonCache::AddReserve(registry, hub, got, -1, playerValid, px, pz);
            budget -= got;
            LOG_INFO(Core, "OccupierForces: Wave {}: hub top-up {} at {}{}", _waveCount, got,
                     ZoneNameOf(registry, hub), pending ? " (pending)" : "");
        }
    }
    // 2. the final stand: the last base's surge, above its strength
    if (final && budget > 0)
    {
        int last = -1;
        for (int i = 0; i < _roster.Size(); i++)
        {
            const int zi = registry.FindZoneIndex(_roster[i].name);
            const ZoneRecord* z = registry.GetZone(zi);
            if (z && stricmp(z->owner, occupier) == 0)
            {
                last = zi;
                break;
            }
        }
        const int want = MinInt(_tuning.waveSurge, budget);
        const int got = last >= 0 && want > 0 ? Deposit(want, "wave final-stand surge") : 0;
        if (got > 0)
        {
            const bool pending = GarrisonCache::AddReserve(registry, last, got, -1, playerValid, px, pz);
            budget -= got;
            const ZoneRecord* z = registry.GetZone(last);
            if (stricmp(_surgeZone, z->name) != 0)
            {
                _surgeBodies = 0;
            }
            _surgeZone = z->name;
            _surgeBodies += got;
            LOG_INFO(Core, "OccupierForces: Wave {}: final-stand surge {} at {}{} (never spare)", _waveCount, got,
                     (const char*)z->name, pending ? " (pending)" : "");
        }
    }
    // 3. strikes on the best cluster, only once the director is AWAKE (W)
    int ops = 0;
    if (_posture != OFAwake)
    {
        LOG_INFO(Core, "OccupierForces: Wave {}: no strikes, the director is DORMANT", _waveCount);
    }
    else if (budget > 0 && _tuning.waveOps > 0)
    {
        int strikeBudget = MinInt(budget, (int)floorf(_tuning.waveRetakeShare * (float)bodies));
        const int centre = BestCluster(registry, _tuning.waveCluster, view.hqZone, view.hqEstablished ? view.hqPos : VZero);
        const ZoneRecord* c = registry.GetZone(centre);
        AutoArray<int> targets;
        if (c)
        {
            const float rSq = _tuning.waveCluster * _tuning.waveCluster;
            auto inCluster = [&c, rSq](Vector3Par p) {
                const float dx = p.X() - c->pos.X();
                const float dz = p.Z() - c->pos.Z();
                return dx * dx + dz * dz <= rSq;
            };
            // the HQ if it is in the cluster, then bases, then towns
            DirectorLauncher& l = launcher;
            if (view.hqEstablished && inCluster(view.hqPos) && !l.HasHqRecord() && Cooldown(HqCooldownName) <= 0)
            {
                targets.Add(HqCandidate);
            }
            for (int pass = 0; pass < 2; pass++)
            {
                for (int i = 0; i < registry.NZones(); i++)
                {
                    const ZoneRecord* z = registry.GetZone(i);
                    if (!z || i == view.hqZone || !inCluster(z->pos) || l.HasRecord(i))
                    {
                        continue;
                    }
                    const bool base = CampaignEnd::IsMilitaryType(z->type) &&
                                      stricmp(z->owner, registry.ResistanceSide()) == 0;
                    const bool town = IsHeldTown(registry, *z);
                    if ((pass == 0 && base) || (pass == 1 && town))
                    {
                        targets.Add(i);
                    }
                }
            }
            LOG_INFO(Core, "OccupierForces: Wave {}: strikes on the cluster around {} ({} target(s), up to {})",
                     _waveCount, (const char*)c->name, targets.Size(), strikeBudget);
        }
        for (int t = 0; t < targets.Size() && ops < _tuning.waveOps && strikeBudget > 0; t++)
        {
            const Vector3 dest = targets[t] == HqCandidate ? view.hqPos : registry.GetZone(targets[t])->pos;
            // the source: the hub, else (until phase 6) the nearest occupier base
            int source = hub;
            if (_surgeBodies > 0 && source >= 0 && stricmp(registry.GetZone(source)->name, _surgeZone) == 0)
            {
                source = -1; // the last base never sources a strike
            }
            if (source < 0)
            {
                float bestD = -1;
                for (int i = 0; i < registry.NZones(); i++)
                {
                    const ZoneRecord* s = registry.GetZone(i);
                    if (!s || !CampaignEnd::IsMilitaryType(s->type) || stricmp(s->owner, occupier) != 0 ||
                        SurgeAt(s->name) > 0 || !RoadRoute::SameLandmass(s->pos, dest))
                    {
                        continue;
                    }
                    const float dx = s->pos.X() - dest.X();
                    const float dz = s->pos.Z() - dest.Z();
                    const float d = dx * dx + dz * dz;
                    if (bestD < 0 || d < bestD)
                    {
                        bestD = d;
                        source = i;
                    }
                }
            }
            if (source < 0)
            {
                LOG_INFO(Core, "OccupierForces: Wave {}: no base can send a strike", _waveCount);
                break;
            }
            const DirectorAction a = PlanFor(registry, view, targets[t], CAOWave, source, strikeBudget);
            if (!a.go || a.choice.quote.bodies > strikeBudget)
            {
                LOG_INFO(Core, "OccupierForces: Wave {}: no strike on {}: {}", _waveCount,
                         targets[t] == HqCandidate ? "the HQ" : ZoneNameOf(registry, targets[t]),
                         a.go ? "over the strike budget" : (const char*)a.choice.why);
                continue;
            }
            const int poolBefore = _pool;
            if (LaunchAction(registry, view, launcher, a, false, true, _waveCount))
            {
                const int used = MaxInt(poolBefore - _pool, 0);
                strikeBudget -= a.choice.quote.bodies;
                budget -= MinInt(used, budget);
                ops++;
            }
        }
    }
    // 4. reinforce columns from the hub to bled bases (the most bled first)
    int columns = 0;
    while (hub >= 0 && budget > 0 && columns < _tuning.waveOps)
    {
        int best = -1;
        int bestShort = 0;
        for (int i = 0; i < registry.NZones(); i++)
        {
            const ZoneRecord* z = registry.GetZone(i);
            if (!z || i == hub || !CampaignEnd::IsMilitaryType(z->type) || stricmp(z->owner, occupier) != 0 ||
                SurgeAt(z->name) > 0 || launcher.HasRecord(i))
            {
                continue;
            }
            const int s = shortfall(i);
            if (s > bestShort)
            {
                best = i;
                bestShort = s;
            }
        }
        if (best < 0)
        {
            break;
        }
        DirectorAction a;
        a.go = true;
        a.kind = CAKReinforce;
        a.origin = CAOWave;
        a.target = best;
        a.choice.ok = true;
        PackageQuote& q = a.choice.quote;
        q.package = FPFoot;
        q.source = hub;
        q.bodies = MinInt(bestShort, MinInt(budget, ForcePackages::MaxGroup));
        q.price = q.bodies;
        q.speed = ForcePackages::FootSpeed;
        q.lethality = (float)q.bodies;
        const int poolBefore = _pool;
        if (!LaunchAction(registry, view, launcher, a, false, true, _waveCount))
        {
            break;
        }
        budget -= MinInt(MaxInt(poolBefore - _pool, 0), budget);
        columns++;
    }
    return budget;
}

// ---------------------------------------------------------------------------
// occupier standing (OCCUPIER-FORCES.md 7.3, D21: the model; phase 4)
// ---------------------------------------------------------------------------

float OccupierForces::MoraleFactor(const ZoneRecord& z)
{
    return stricmp(z.type, "CITY") == 0 && z.standing < LowStanding ? MoraleSkill : 1.0f;
}

void OccupierForces::IngestTip(const ZoneRegistry& registry, int zone, int count, Vector3Par pos)
{
    const ZoneRecord* z = registry.GetZone(zone);
    if (!z || count <= 0)
    {
        return;
    }
    SightingRow row;
    row.zone = z->name;
    row.count = count;
    row.stamp = Now();
    row.positions.Add(pos);
    for (int r = 0; r < _sightings.Size(); r++)
    {
        if (stricmp(_sightings[r].zone, z->name) == 0)
        {
            // a fresher report from the town's own soldiers wins over a tip
            if (_sightings[r].stamp >= row.stamp && _sightings[r].count >= count)
            {
                return;
            }
            _sightings[r] = row;
            return;
        }
    }
    _sightings.Add(row);
}

void OccupierForces::StepStanding(ZoneRegistry& registry, float elapsed)
{
    // recovery toward 50 when nothing happens (strikes are phase 6)
    const float step = elapsed > 0 ? elapsed / StandingRecoverSeconds : 0.0f;
    for (int i = 0; i < registry.NZones(); i++)
    {
        ZoneRecord* z = registry.GetZoneMutable(i);
        if (!z || stricmp(z->type, "CITY") != 0)
        {
            continue;
        }
        if (z->standing < DefaultStanding)
        {
            z->standing = z->standing + step > DefaultStanding ? DefaultStanding : z->standing + step;
        }
        else if (z->standing > DefaultStanding)
        {
            z->standing = z->standing - step < DefaultStanding ? DefaultStanding : z->standing - step;
        }
    }
    // the refill factor reads the mean standing of the K towns (5.2)
    float sum = 0;
    int n = 0;
    for (int i = 0; i < _towns.Size(); i++)
    {
        const ZoneRecord* z = registry.GetZone(registry.FindZoneIndex(_towns[i].name));
        if (z)
        {
            sum += z->standing;
            n++;
        }
    }
    if (n > 0)
    {
        SetMeanStanding(sum / (float)n);
    }
    // informant tips (D9-legal intel).  THE ONE PLACE THE DIRECTOR READS THE
    // TRUE CENSUS: a town that still trusts the occupier (standing >= 50)
    // and has resistance units standing in it (the zone tick's HOLD count)
    // may yield a tip with chance standing / 200, because a local talked.
    // The roll is hashed from the town and the director's clock (no engine
    // RNG), the position is the town centre jittered +-TipJitter m.
    const int tick = (int)floorf(_clock / (_tuning.directorInterval > 1 ? _tuning.directorInterval : 1.0f));
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || stricmp(z->type, "CITY") != 0 || z->standing < DefaultStanding || z->lastGuerHold <= 0)
        {
            continue;
        }
        const unsigned int seed = CounterAttack::MakeSeed(z->name, tick, 0, 0);
        if (CounterAttack::Roll01(seed, TipSalt) >= z->standing / 200.0f)
        {
            continue;
        }
        const float jx = (CounterAttack::Roll01(seed, TipSalt + 1) * 2.0f - 1.0f) * TipJitter;
        const float jz = (CounterAttack::Roll01(seed, TipSalt + 2) * 2.0f - 1.0f) * TipJitter;
        const Vector3 at(z->pos.X() + jx, z->pos.Y(), z->pos.Z() + jz);
        IngestTip(registry, i, z->lastGuerHold, at);
        if (LogAllowed())
        {
            LOG_INFO(Core, "OccupierForces: a local in {} talked: {} fighter(s) reported (standing {:.0f}){}",
                     (const char*)z->name, z->lastGuerHold, (double)z->standing,
                     _logSuppressed > 0 ? " (other lines suppressed)" : "");
            _logSuppressed = 0;
        }
    }
}

bool OccupierForces::OnTownFlipped(int zoneIndex, int /*k*/)
{
    return NoteTownFlipped(ZoneRegistry::Instance(), zoneIndex);
}

void OccupierForces::OnCounterAttackOp(const CounterAttackOpEvent& ev)
{
    // CounterAttack's tick may run before ours this frame: the strike stamp
    // takes the mission time now
    _missionNow = Glob.time - Foundation::Time(0);
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
        if (ev.type == ZEHqLost)
        {
            // the HQ fell: whatever was latched for it belonged to that HQ
            ForgetZone(HqCooldownName);
            continue;
        }
        if (ev.type != ZECaptured && ev.type != ZETownLost && ev.type != ZEBaseLost)
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
    if (ev.type == CAOpStruck)
    {
        // D17, D21: whoever ordered it (a director strike or a test lever),
        // the struck town and its neighbours pay
        ApplyStrikeEffects(registry, ev.zone >= 0 ? ev.zone : ev.target, ev.pos, _missionNow, ev.opId);
        const int r = FindOp(ev.opId);
        if (r >= 0 && _ops[r].kind == CAKStrike)
        {
            _ops[r].born = true;
        }
        return;
    }
    if (!Active())
    {
        return;
    }
    const int row = FindOp(ev.opId);
    const ZoneRecord* z = registry.GetZone(ev.target);
    // an HQ assault rests the HQ, not the zone it stands in
    const char* target = ev.kind == CAKHqAssault ? HqCooldownName : (z ? (const char*)z->name : "");
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
            // rests (3.1 step 5).  Only the director's own operations: a
            // legacy record (op 0) drives no director action, not even
            // while DORMANT (invariant W, M6).
            if (!mine)
            {
                LOG_DEBUG(Core, "OccupierForces: [op {}] cancelled unborn (not a director operation)", ev.opId);
                break;
            }
            // L3: CounterAttack names the units still owed (the carrier's and
            // the escort's prices not yet settled); crew it already booked
            // lost are not in them, so the rest of the row is written off
            const int refund = MinInt(ev.units > 0 ? ev.units : 0, _ops[row].outstanding);
            _ops[row].outstanding = 0;
            _ops[row].finished = true;
            Refund(refund, "cancelled unborn");
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
            // a RED edge during the fight already paid it.  Only the
            // director's own operations (M6, invariant W): a legacy repel
            // raises no heat and rests nothing.
            if (!mine)
            {
                LOG_DEBUG(Core, "OccupierForces: [op {}] repelled (not a director operation)", ev.opId);
                break;
            }
            const bool paid = RedPaid(target);
            // an alarm answer fights at an occupier zone and a strike is no
            // retake: neither raises the resistance zone's heat
            if (!paid && ev.target >= 0 && ev.kind != CAKReinforce && ev.kind != CAKAlarm && ev.kind != CAKStrike)
            {
                registry.HeatRaise(ev.target, RepelHeat);
            }
            RemoveName(_redPaid, target);
            SetCooldown(target, _tuning.townCooldown);
            _ops[row].finished = true;
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
                // CounterAttack found no stored spare for the hull at launch
                // and sent the force on foot (it rewrote its record's
                // package): the ledger row follows, so gmOccupierOps and the
                // row agree.  The note is CounterAttack::LaunchCore's.
                if (!_ops[row].born && ev.note && strncmp(ev.note, "no stored spare", 15) == 0)
                {
                    _ops[row].package = FPFoot;
                }
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
        case CAOpReturned:
            // an alarm answer or a strike has no settle: its return ends it
            if (mine && (_ops[row].kind == CAKAlarm || _ops[row].kind == CAKStrike))
            {
                _ops[row].finished = true;
            }
            break;
        case CAOpAirborne:
            if (mine)
            {
                _ops[row].born = true;
            }
            LOG_DEBUG(Core, "OccupierForces: [op {}] airborne from {}", ev.opId,
                      ev.zone >= 0 ? ZoneNameOf(registry, ev.zone) : "off-map");
            break;
        case CAOpLanded:
            LOG_DEBUG(Core, "OccupierForces: [op {}] landed near {}", ev.opId, target);
            break;
        case CAOpAlarmAnswered:
            if (mine)
            {
                LOG_INFO(Core, "OccupierForces: [op {}] Alarm answer at {}: searching the reported positions", ev.opId,
                         ZoneNameOf(registry, ev.zone >= 0 ? ev.zone : ev.target));
            }
            break;
        default:
            // Launched, Arrived: CounterAttack routes the bodies (5.5)
            // itself; nothing for the ledger
            break;
    }
    // a strike's airframe home (refund) or lost ends the strike
    if (mine && _ops[row].kind == CAKStrike && (ev.type == CAOpHullRefund || ev.type == CAOpHullLost) &&
        _ops[row].outstanding <= 0)
    {
        _ops[row].finished = true;
    }
    PruneOps();
}

// ---------------------------------------------------------------------------
// phase 6: air strikes, strike effects, alarm answers (7.3, 8.2, D4, D17, D20,
// D21)
// ---------------------------------------------------------------------------

Vector3 OccupierForces::ReportedCentroid(const ZoneRegistry& registry, int zone) const
{
    const ZoneRecord* z = registry.GetZone(zone);
    if (!z)
    {
        return VZero;
    }
    for (int i = 0; i < _sightings.Size(); i++)
    {
        const SightingRow& row = _sightings[i];
        if (stricmp(row.zone, z->name) != 0 || row.positions.Size() == 0 ||
            Now() - row.stamp > _tuning.intelDecay)
        {
            continue;
        }
        float x = 0, y = 0, zz = 0;
        for (int k = 0; k < row.positions.Size(); k++)
        {
            x += row.positions[k].X();
            y += row.positions[k].Y();
            zz += row.positions[k].Z();
        }
        const float n = (float)row.positions.Size();
        return Vector3(x / n, y / n, zz / n);
    }
    return VZero;
}

Vector3 OccupierForces::LatestReport(const ZoneRegistry& registry, int zone) const
{
    const ZoneRecord* z = registry.GetZone(zone);
    if (!z)
    {
        return VZero;
    }
    for (int i = 0; i < _sightings.Size(); i++)
    {
        const SightingRow& row = _sightings[i];
        if (stricmp(row.zone, z->name) == 0 && row.positions.Size() > 0 && Now() - row.stamp <= _tuning.intelDecay)
        {
            return row.positions[row.positions.Size() - 1];
        }
    }
    return VZero;
}

bool OccupierForces::StrikeDue(const ZoneRegistry& registry, const DirectorView& view, int target,
                               int launchedKind) const
{
    if (!Active() || _band != OFBandStrong)
    {
        return false;
    }
    if (!_catalogue.Offered(FPJet) && !_catalogue.Offered(FPAttackHeli))
    {
        return false;
    }
    // the opener of a base retake or an HQ assault
    if (launchedKind == CAKBase || launchedKind == CAKHqAssault)
    {
        return true;
    }
    const ZoneRecord* z = registry.GetZone(target);
    if (!z || !IsHeldTown(registry, *z) || !Urgent(z->name))
    {
        return false;
    }
    if (KnownFighters(registry, target) < StrikeMinFighters)
    {
        return false;
    }
    // a town inside its strike window is not struck again
    return !registry.IsStruck(target, view.now);
}

DirectorAction OccupierForces::PlanStrike(const ZoneRegistry& registry, const DirectorView& view, int target,
                                          bool urgent) const
{
    DirectorAction a;
    a.kind = CAKStrike;
    a.origin = CAODirector;
    a.urgent = urgent;
    const bool hq = target == HqCandidate;
    const int zone = hq ? view.hqZone : target;
    const ZoneRecord* z = registry.GetZone(zone);
    if (!z && !(hq && view.hqEstablished))
    {
        a.choice.why = "no such zone";
        return a;
    }
    a.target = z ? zone : -1;
    a.strikeZone = a.target;
    a.fighters = z ? (float)KnownFighters(registry, zone) : 0.0f;
    // D9: what was reported, else the town (or the HQ the assault marches
    // on); never a hidden player position
    a.strikePos = z ? ReportedCentroid(registry, zone) : VZero;
    if (hq && a.strikePos.X() == 0 && a.strikePos.Z() == 0)
    {
        a.strikePos = view.hqPos;
    }
    const Vector3 dest = a.strikePos.X() != 0 || a.strikePos.Z() != 0 ? a.strikePos : (z ? z->pos : view.hqPos);
    a.airOrigin = HeldAirfield(registry, dest);
    const bool offMap = a.airOrigin < 0;
    const ZoneRecord* origin = registry.GetZone(a.airOrigin);
    PackageQuote best;
    best.source = -1;
    bool found = false;
    PackageQuote quotes[2];
    int n = 0;
    for (int p = FPAttackHeli; p <= FPJet; p++)
    {
        if ((StrikePackages & ForcePackages::Bit(p)) == 0 || !_catalogue.Offered(p))
        {
            continue;
        }
        PackageQuote q;
        q.package = p;
        q.source = -1;
        q.bodies = 0;
        q.crew = ForcePackages::CrewOf(p, _catalogue);
        q.hullPrice = ForcePackages::HullPrice(p, offMap);
        q.poolPrice = q.hullPrice + q.crew;
        q.price = q.poolPrice;
        q.lethality = ForcePackages::LethalityAdd(p);
        q.lethal = true;
        q.affordable = !Active() || (q.poolPrice <= _pool);
        q.speed = ForcePackages::SpeedOf(p, _catalogue);
        q.hullClass = _catalogue.hulls[p].className;
        if (origin && q.speed > 0)
        {
            const float dx = origin->pos.X() - dest.X();
            const float dz = origin->pos.Z() - dest.Z();
            q.seconds = sqrtf(dx * dx + dz * dz) / q.speed;
        }
        quotes[n++] = q;
    }
    a.choice.nQuotes = 0;
    int cheapest = -1;
    for (int i = 0; i < n; i++)
    {
        a.choice.quotes[a.choice.nQuotes++] = quotes[i];
        if (quotes[i].affordable && (cheapest < 0 || quotes[i].price < quotes[cheapest].price))
        {
            cheapest = i;
        }
    }
    if (cheapest < 0)
    {
        a.choice.why = n == 0 ? "no strike airframe offered" : "the pool cannot pay a strike";
        return a;
    }
    int pick = cheapest;
    if (urgent)
    {
        // the fastest within the urgent cost share of the cheapest (6.5)
        const float share = _tuning.urgentCostShare > 1 ? _tuning.urgentCostShare : 1.0f;
        for (int i = 0; i < n; i++)
        {
            if (quotes[i].affordable && (float)quotes[i].price <= share * (float)quotes[cheapest].price + 1e-4f &&
                quotes[i].speed > quotes[pick].speed)
            {
                pick = i;
            }
        }
    }
    best = quotes[pick];
    found = true;
    a.choice.ok = found;
    a.choice.quote = best;
    a.go = true;
    return a;
}

void OccupierForces::MaybeStrike(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher,
                                 int target, int launchedKind)
{
    if (!StrikeDue(registry, view, target, launchedKind))
    {
        return;
    }
    const DirectorAction a = PlanStrike(registry, view, target, true);
    if (!a.go)
    {
        LOG_DEBUG(Core, "OccupierForces: no strike with the operation: {}", (const char*)a.choice.why);
        return;
    }
    // no extra token: the strike goes with the operation just launched
    LaunchAction(registry, view, launcher, a, false, false, 0);
}

int OccupierForces::ApplyStrikeEffects(ZoneRegistry& registry, int zone, Vector3Par pos, float now, int opId)
{
    AutoArray<ZoneEventRecord> fired;
    const ZoneRecord* z = registry.GetZone(zone);
    const bool town = z && stricmp(z->type, "CITY") == 0;
    const int ring = registry.ApplyStrike(zone, pos, now, fired);
    // the script handler (civilians.sqs: the panic) runs on the next zone
    // tick, never inside CounterAttack's
    for (int i = 0; i < fired.Size(); i++)
    {
        registry.QueueEvent(fired[i].type, fired[i].zoneIndex);
    }
    _strikesApplied++;
    if (town)
    {
        _townStrikes++;
        LOG_INFO(Core,
                 "OccupierForces: [op {}] Struck {}: support {:.0f}, standing {:.0f} (-{:.0f} each), heat +{:.0f}, "
                 "half income until {:.0f}; {} town(s) within {:.0f} m -{:.0f} each",
                 opId, (const char*)z->name, (double)z->support, (double)z->standing, (double)ZoneRegistry::StrikeCut,
                 (double)ZoneRegistry::StrikeHeat, (double)z->struckUntil, ring,
                 (double)ZoneRegistry::StrikeRingRadius, (double)ZoneRegistry::StrikeRingCut);
    }
    else
    {
        LOG_INFO(Core, "OccupierForces: [op {}] Struck {}: {} town(s) within {:.0f} m -{:.0f} support and standing",
                 opId, z ? (const char*)z->name : "a reported position", ring, (double)ZoneRegistry::StrikeRingRadius,
                 (double)ZoneRegistry::StrikeRingCut);
    }
    return ring;
}

int OccupierForces::AlarmTarget(const ZoneRegistry& registry, const DirectorView& view,
                                const DirectorLauncher& launcher) const
{
    const RString resistance = registry.ResistanceSide();
    int best = -1;
    int bestFighters = -1;
    for (int i = 0; i < registry.NZones() && i < view.alert.Size(); i++)
    {
        if (view.alert[i] < (int)ASRed)
        {
            continue;
        }
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || stricmp(z->owner, resistance) == 0 || Cooldown(z->name) > 0 || launcher.HasRecord(i))
        {
            continue;
        }
        const int f = KnownFighters(registry, i);
        if (f > bestFighters)
        {
            best = i;
            bestFighters = f;
        }
    }
    return best;
}

DirectorAction OccupierForces::PlanAlarm(const ZoneRegistry& registry, const DirectorView& view, int zone) const
{
    DirectorAction a;
    a.kind = CAKAlarm;
    a.origin = CAOAlarm;
    const ZoneRecord* z = registry.GetZone(zone);
    if (!z)
    {
        a.choice.why = "no such zone";
        return a;
    }
    a.target = zone;
    a.alarmZone = zone;
    a.alarmPos = LatestReport(registry, zone);
    // 8.2: always urgent; a THIN occupier answers with the cheapest package
    a.urgent = _band != OFBandThin;
    a.fighters = (float)KnownFighters(registry, zone);
    const Vector3 dest = a.alarmPos.X() != 0 || a.alarmPos.Z() != 0 ? a.alarmPos : z->pos;
    PackageRequest req;
    req.fighters = a.fighters;
    req.defenderWeight = view.defenderWeight;
    req.margin = _tuning.lethalityMargin;
    req.minBodies = ForcePackages::AlarmMinBodies;
    req.urgent = a.urgent;
    req.urgentCostShare = _tuning.urgentCostShare;
    req.tierFactor = view.tierFactor;
    req.pool = _pool;
    req.poolActive = Active();
    req.offered = OfferedPackages;
    AutoArray<PackageSource> sources;
    // never the zone under attack (8.2)
    BuildSources(registry, view, dest, -1, 0, zone, sources);
    a.airOrigin = HeldAirfield(registry, dest);
    if (a.airOrigin == zone)
    {
        a.airOrigin = -1; // the airfield under attack launches nothing
    }
    a.choice = ChooseWithAir(req, sources, a.airOrigin);
    a.go = a.choice.ok;
    return a;
}

bool OccupierForces::AlarmInFlight(const ZoneRegistry& registry, const DirectorLauncher& launcher)
{
    bool any = false;
    for (int i = 0; i < _ops.Size(); i++)
    {
        OpRow& row = _ops[i];
        if (row.kind != CAKAlarm || row.finished)
        {
            continue;
        }
        // the record is gone without a closing event: the row closes here
        if (!launcher.HasRecord(registry.FindZoneIndex(row.target)))
        {
            row.finished = true;
            continue;
        }
        any = true;
    }
    return any;
}

void OccupierForces::AnswerAlarms(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher)
{
    if (!Active() || _legacySave || view.won || view.lost)
    {
        return;
    }
    // forget the starvation notes of zones no longer RED
    for (int i = _alarmStarved.Size() - 1; i >= 0; i--)
    {
        const int zi = registry.FindZoneIndex(_alarmStarved[i].name);
        if (zi < 0 || zi >= view.alert.Size() || view.alert[zi] < (int)ASRed)
        {
            _alarmStarved.Delete(i);
        }
    }
    if (AlarmInFlight(registry, launcher))
    {
        return; // one alarm answer at a time (8.2)
    }
    const int zone = AlarmTarget(registry, view, launcher);
    if (zone < 0)
    {
        return;
    }
    const ZoneRecord* z = registry.GetZone(zone);
    DirectorAction a;
    const char* why = nullptr;
    if (_band == OFBandSpent)
    {
        why = "the pool is spent";
    }
    else
    {
        a = PlanAlarm(registry, view, zone);
        if (!a.go)
        {
            why = a.choice.why.GetLength() > 0 ? (const char*)a.choice.why : "no package";
        }
    }
    if (why)
    {
        if (!HasName(_alarmStarved, z->name))
        {
            AddName(_alarmStarved, z->name);
            LOG_INFO(Core, "OccupierForces: Starved: no answer to {} alarm ({})", (const char*)z->name, why);
        }
        return;
    }
    if (LaunchAction(registry, view, launcher, a, false, false, 0))
    {
        _alarmAnswers++;
    }
    else
    {
        SetCooldown(z->name, AlarmRefusedRest);
    }
}

bool OccupierForces::LegacyQrf() const
{
    if (!_tuning.enabled)
    {
        return true; // the key off: the script QRF as before
    }
    if (Active())
    {
        return _legacySave;
    }
    // sized off (no roster) or a restored block now inert: the script QRF;
    // before the first sizing: neither yet
    return _activationTried || _restored;
}

void OccupierForces::EngineView(DirectorView& view)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    _missionNow = Glob.time - Foundation::Time(0);
    if (!_catalogueBuilt && Active())
    {
        BuildEngineCatalogue(registry);
    }
    FillEngineView(registry, view);
}

bool OccupierForces::ForceStrike(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher,
                                 int zone)
{
    if (!Active())
    {
        return false;
    }
    const DirectorAction a = PlanStrike(registry, view, zone, true);
    if (!a.go)
    {
        LOG_INFO(Core, "OccupierForces: forced strike refused: {}", (const char*)a.choice.why);
        return false;
    }
    return LaunchAction(registry, view, launcher, a, false, false, 0);
}

bool OccupierForces::ForceAlarm(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, int zone)
{
    if (!Active())
    {
        return false;
    }
    const DirectorAction a = PlanAlarm(registry, view, zone);
    if (!a.go)
    {
        LOG_INFO(Core, "OccupierForces: forced alarm answer refused: {}", (const char*)a.choice.why);
        return false;
    }
    if (!LaunchAction(registry, view, launcher, a, false, false, 0))
    {
        return false;
    }
    _alarmAnswers++;
    return true;
}

bool OccupierForces::ForceStrike(int zone)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    DirectorView view;
    EngineView(view);
    EngineDirectorLauncher launcher(registry);
    return ForceStrike(registry, view, launcher, zone);
}

bool OccupierForces::ForceAlarm(int zone)
{
    ZoneRegistry& registry = ZoneRegistry::Instance();
    DirectorView view;
    EngineView(view);
    EngineDirectorLauncher launcher(registry);
    return ForceAlarm(registry, view, launcher, zone);
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
    LOG_INFO(Core, "OccupierForces: package catalogue for {} ({}, {} ladder class(es)); offers foot{}{}{}{}{}{}{}",
             (const char*)occupier, f ? (const char*)f->className : "no descriptor", ladder.Size(),
             _catalogue.Offered(FPTruck) ? ", truck" : "", _catalogue.Offered(FPArmedCar) ? ", armed car" : "",
             _catalogue.Offered(FPApc) ? ", APC" : "", _catalogue.Offered(FPTank) ? ", tank" : "",
             _catalogue.Offered(FPTransportHeli) ? ", transport helicopter" : "",
             _catalogue.Offered(FPAttackHeli) ? ", attack helicopter" : "", _catalogue.Offered(FPJet) ? ", jet" : "");
}

void OccupierForces::FillEngineView(const ZoneRegistry& registry, DirectorView& view) const
{
    const int n = registry.NZones();
    view.alert.Resize(n);
    view.spare.Resize(n);
    view.spareStored.Resize(n);
    view.garrisonLive.Resize(n);
    const AlertMachine& alert = AlertMachine::Instance();
    const GarrisonCache& cache = GarrisonCache::Instance();
    const CounterAttack& ca = CounterAttack::Instance();
    const RString occupier = registry.OccupierSide();
    for (int i = 0; i < n; i++)
    {
        view.alert[i] = alert.GetZoneState(i);
        view.spare[i] = 0;
        view.spareStored[i] = 0;
        view.garrisonLive[i] = cache.LiveCount(i);
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || !CampaignEnd::IsMilitaryType(z->type) || stricmp(z->owner, occupier) != 0)
        {
            continue;
        }
        const int live = view.garrisonLive[i];
        // the final stand's surge is never spare (7.1)
        const float stock = z->garrison - (float)SurgeAt(z->name);
        // M1: CounterAttack grants a funded order from the source's STORED
        // reserve (it marches live bodies only as an invisible fallback and
        // refuses an order it cannot fill), so the quote is the stored spare
        // alone.  D9: whether the base is live (the player's proximity) no
        // longer steers the choice, beyond the physical fact that a
        // materialized garrison holds no stored reserve.
        const int stored = CounterAttack::Spare(ca.Tuning(), stock, live, z->garrisonMax, false);
        view.spare[i] = stored;
        view.spareStored[i] = stored;
    }
    view.now = _missionNow;
    view.hqZone = GuerrillaBase::Instance().HqZone();
    view.won = CampaignEnd::Instance().Won();
    view.lost = CampaignEnd::Instance().Lost();
    view.hqEstablished = GuerrillaBase::Instance().IsEstablished();
    view.hqPos = view.hqEstablished ? GuerrillaBase::Instance().HqPos() : VZero;
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
                // known but never reported (posReported still the origin):
                // nothing to place yet (L9)
                if (t->posReported.X() == 0 && t->posReported.Z() == 0)
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
    // K by the authored owners (M7): the towns the player already holds stay
    // in K, and control measures them lost
    if (!Activate(registry, nullptr, nullptr, true))
    {
        return;
    }
    // the pool from config, scaled by how much the occupier still controls
    _control = ComputeControl(registry);
    int pool = RoundToInt((float)_poolStart * _control);
    _pool = pool < 0 ? 0 : (pool > _poolStart ? _poolStart : pool);
    // thresholds already crossed never fire, nor a final stand already due
    _wavesFired = WavesCrossed(_tuning, _control);
    _finalFired = RosterBasesHeld(registry) <= 1;
    // the distinct-town set from the towns the resistance holds now, less
    // the ones it was authored to hold (never taken, L12)
    _townsEver.Clear();
    const RString resistance = registry.ResistanceSide();
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (z && !z->garrisonExempt && stricmp(z->type, "CITY") == 0 && stricmp(z->owner, resistance) == 0 &&
            stricmp(AuthoredOwnerOf(registry, *z), resistance) != 0)
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
    _legacySave = true; // D20: the old save keeps its own script QRF
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
    // the waves (phase 4; absent in older blocks: none inbound, none fired
    // beyond wavesFired / finalFired above)
    PARAM_CHECK(ar.Serialize("waveInbound", _waveInbound, 1, false))
    PARAM_CHECK(ar.Serialize("waveClock", _waveClock, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("waveBodies", _waveBodies, 1, 0))
    PARAM_CHECK(ar.Serialize("waveFinalStand", _waveFinalStand, 1, false))
    PARAM_CHECK(ar.Serialize("waveCount", _waveCount, 1, 0))
    PARAM_CHECK(ar.Serialize("controlDwell", _controlDwell, 1, 0))
    PARAM_CHECK(ar.Serialize("waveWindow", _waveWindow, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("surgeZone", _surgeZone, 1, RString()))
    PARAM_CHECK(ar.Serialize("surgeBodies", _surgeBodies, 1, 0))
    // phase 6 (absent in older blocks: 0, which marks the block legacy)
    PARAM_CHECK(ar.Serialize("townStrikes", _townStrikes, 1, 0))
    PARAM_CHECK(ar.Serialize("strikesApplied", _strikesApplied, 1, 0))
    PARAM_CHECK(ar.Serialize("alarmAnswers", _alarmAnswers, 1, 0))
    int format = SaveFormat;
    PARAM_CHECK(ar.Serialize("format", format, 1, 0))
    if (ar.IsLoading())
    {
        _loadedFormat = format;
        // D20: a block saved before the native alarm answers existed keeps
        // the campaign's own script QRF (its text rides in the save) and no
        // native answers run beside it
        if (format < SaveFormat && _active && !_legacySave)
        {
            _legacySave = true;
            LOG_INFO(Core,
                     "OccupierForces: block format {} predates the native alarm answers: legacySave, the script QRF "
                     "stays",
                     format);
        }
        _alarmStarved.Clear();
        // transient director state is re-derived: the alert baseline from
        // the next scan (no false edges), the catalogue from the config
        _alertPrev.Clear();
        _scanAccum = 0;
        _basesHeld = -1;
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
