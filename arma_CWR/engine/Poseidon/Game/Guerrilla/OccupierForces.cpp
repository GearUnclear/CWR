#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>
#include <Poseidon/Game/Guerrilla/DirectorLog.hpp>

#include <Poseidon/Game/Guerrilla/AlertMachine.hpp>  // alert states, alertInterval (read only)
#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp>   // IsMilitaryType / IsAuthoredOccupier, the win latch
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp> // LaunchFunded, the listener, Roll01 / MakeSeed / Spare
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp> // live garrison bodies (a base's spare)
#include <Poseidon/Game/Guerrilla/GuerrillaBase.hpp> // HqZone: never a target
#include <Poseidon/Game/Guerrilla/RoadRoute.hpp>     // SameLandmass, RoadQuoteCache (the road quote)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/Core/Global.hpp> // Glob.time (the town-strike stamps)
#include <Poseidon/World/World.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/VehicleAI.hpp> // Target / TargetList (the sightings scan)

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Platform/AppConfig.hpp> // DevMode: the director log toggle
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
// a zone's name for the INFO lines ("off-map" for -1); defined with the director
static const char* ZoneNameOf(const ZoneRegistry& registry, int index);

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

// The director log toggle (DirectorLog.hpp): --dev or forcesLog = 1
spdlog::level::level_enum DirectorLogLevel()
{
    const bool on = Poseidon::Foundation::AppConfig::Instance().DevMode() || OccupierForces::Instance().Tuning().log;
    return on ? spdlog::level::info : spdlog::level::debug;
}

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------

void OccupierForces::Clear()
{
    _tuning = OccupierForcesTuning();
    ForcePackages::SetRoadFactor(_tuning.roadFactor);
    _active = false;
    _restored = false;
    _activationTried = false;
    _pool = 0;
    _poolStart = 0;
    _R = 0;
    _Rgen = 0;
    _G = 0;
    _roster.Clear();
    _towns.Clear();
    _excluded.Clear();
    _probedOut = 0;
    _townsEver.Clear();
    _urgent.Clear();
    _redPaid.Clear();
    _cooldowns.Clear();
    _rearm.Clear();
    _repels.Clear();
    _massTarget = RString();
    _massTicks = 0;
    _massTokens = 0;
    _controlBand = -1;
    _sightings.Clear();
    _ops.Clear();
    _lastRefund = RefundTally();
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
    _waveHeadroom = 0;
    _dropped = 0;
    _banked = 0;
    _bankRegistry = nullptr;
    _storedHulls.Clear();
    _surge.Clear();
    _stoodDown = 0;
    _surgeWindow = 0;
    _surgeAnchor = VZero;
    _losses.Clear();
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
    _townsEverPending = false;
    _missionNow = 0;
    _alarmStarved.Clear();
    _minRuleLogged = false;
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
    OccupierForces* of = GPendingRefundTarget;
    if (!of || !of->Active() || bodies <= 0)
    {
        return false; // the key off (or not sized): the phase-1 origin / discard path
    }
    const ZoneRecord* z = registry.GetZone(zone);
    // over the ceiling the bodies bank at the base they came from first,
    // else the one nearest the lost zone (5.5)
    of->Refund(registry, bodies, "pending deposit, zone changed hands", originZone != zone ? originZone : -1,
               z ? &z->pos : nullptr);
    GM_DIRECTOR_INFO(Core, "OccupierForces: {} pending body(ies) at {} back to the pool (the zone changed hands), pool {}",
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

// LoadFromParams (the levers, then the expert overrides) lives in
// OccupierForcesLevers.cpp

// ---------------------------------------------------------------------------
// pure rules
// ---------------------------------------------------------------------------

int OccupierForces::PoolStartOf(const OccupierForcesTuning& t, int R, int K)
{
    // forcesStrength scales the sum (1 at Normal: exactly R + 2K)
    const int sum = R + t.poolPerTown * K;
    return MaxInt(t.poolMin, t.levers.strength == 1.0f ? sum : RoundToInt(t.levers.strength * (float)sum));
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
    // Compare shares at the tuning's precision: e.g. 15/50 equals 0.3f,
    // whereas 0.3f * 50 rounds above 15 and rejects that exact boundary.
    const bool belowFloor =
        poolStart > 0 ? (float)pool / (float)poolStart < floor : (float)pool < floor * (float)poolStart;
    if (band == OFBandThin && belowFloor)
    {
        return false;
    }
    return true;
}

float OccupierForces::TokenInterval(const OccupierForcesTuning& t, int band)
{
    return band >= 0 && band < NOccupierBands ? t.tokenInterval[band] : 0.0f;
}

int OccupierForces::OpsMaxFor(const OccupierForcesTuning& t, int band)
{
    const int base = t.opsMax > 0 ? t.opsMax : 0;
    return band == OFBandStrong ? MaxInt(base, t.opsMaxStrong) : base;
}

float OccupierForces::JitteredRest(const OccupierForcesTuning& t, const char* zone, int salt)
{
    const float share = t.restJitter > 0 ? t.restJitter : 0.0f;
    const float u = CounterAttack::Roll01(CounterAttack::MakeSeed(zone ? zone : "", salt, 0, 0), RestJitterSalt);
    return t.townCooldown * (1.0f + share * (2.0f * u - 1.0f));
}

int OccupierForces::SpareCappedMin(int minBodies, int largestSpare, int floor)
{
    // never above the minimum asked, never below the floor (or the minimum
    // when it is smaller): a 12-man base retake no base can source becomes
    // one of the largest spare, a 6-man HQ assault stays 6
    return MinInt(minBodies, MaxInt(largestSpare, floor));
}

int OccupierForces::LargestSpare(const AutoArray<PackageSource>& sources, bool heliOffered)
{
    int best = 0;
    for (int i = 0; i < sources.Size(); i++)
    {
        const PackageSource& s = sources[i];
        if (s.zone < 0)
        {
            continue;
        }
        // an air-only source (another landmass) pays only an insertion,
        // always from its stored reserve
        const int spare = s.airOnly ? (heliOffered ? s.SpareFor(FPTransportHeli) : 0)
                                    : MaxInt(s.spare, s.SpareFor(FPTruck));
        best = MaxInt(best, spare);
    }
    return best;
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
            else if (CampaignEnd::IsGeneratedOccupier(*z, registry))
            {
                // a generated base (5.7): in R, its garrison out of poolStart
                out.roster.Add(z->name);
                out.R += RoundToInt(z->garrisonMax);
                out.Rgen += RoundToInt(z->garrisonMax);
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
        else if (seeded ? (i < seeded->Size() && (*seeded)[i] && probe && !probe->IsSettlement(z->name, z->pos))
                        : (z->seeded && z->settlement < 0))
        {
            reason = "a seeded label, not a settlement";
            out.labels.Add(z->name);
        }
        if (reason)
        {
            out.excluded.Add(z->name);
            if (log)
            {
                GM_DIRECTOR_INFO(Core, "OccupierForces: town {} not counted ({})", (const char*)z->name, reason);
            }
            continue;
        }
        out.towns.Add(z->name);
        out.G += RoundToInt(z->garrisonMax);
        if (log)
        {
            GM_DIRECTOR_INFO(Core, "OccupierForces: town {} counted (garrison {})", (const char*)z->name,
                     RoundToInt(z->garrisonMax));
        }
    }
    // conservation (5.7, the owner, 2026-10-16): presence, not strength
    out.poolStart = MaxInt(PoolStartOf(t, out.R - out.Rgen, out.K()) - out.Rgen, 0);
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
        GM_DIRECTOR_INFO(Core, "OccupierForces: no military zone with an authored occupier owner, the service is off");
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
    _probedOut = s.labels.Size();
    _R = s.R;
    _Rgen = s.Rgen;
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
    _waveHeadroom = 0;
    _dropped = 0;
    _storedHulls.Clear();
    _surge.Clear();
    _stoodDown = 0;
    _surgeWindow = 0;
    _surgeAnchor = VZero;
    _losses.Clear();
    _legacySave = false;
    _active = true;
    _control = ComputeControl(registry);
    GM_DIRECTOR_INFO(Core,
             "OccupierForces: active, poolStart {} (R {}, K {}, G {}), roster {} zone(s), {} town(s) not counted "
             "({} probed out), refill {:.1f}/h, band {}, sized in {:.1f} ms",
             _poolStart, _R, _towns.Size(), _G, _roster.Size(), _excluded.Size(), _probedOut,
             (double)RefillPerHour(_poolStart, _R, _G, _tuning.refillHours, _meanStanding), BandName(_band), ms);
    if (_probedOut > 0)
    {
        RString names;
        for (int i = 0; i < s.labels.Size(); i++)
        {
            names = i == 0 ? s.labels[i] : names + RString(", ") + s.labels[i];
        }
        GM_DIRECTOR_INFO(Core, "OccupierForces: {} seeded town(s) not counted in K, not a settlement: {}", _probedOut,
                 (const char*)names);
    }
    return true;
}

bool OccupierForces::ActivateProbed(ZoneRegistry& registry, const SettlementProbe* probe, bool authoredOwner)
{
    if (registry.IsActive())
    {
        LandscapeSettlementProbe landscape;
        landscape.log = false; // the probe's own line names the town and its building count
        const auto start = std::chrono::steady_clock::now();
        const int probed = registry.ProbeSeededSettlements(probe ? *probe : landscape);
        if (probed > 0)
        {
            const double ms =
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
            GM_DIRECTOR_INFO(Core, "OccupierForces: settlement probe ran on {} seeded town(s) in {:.1f} ms", probed, ms);
        }
    }
    return Activate(registry, nullptr, nullptr, authoredOwner);
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
    // a forced pool is the ledger now: above poolStart it carries its own
    // headroom, at or below it none
    _waveHeadroom = MaxInt(_pool - _poolStart, 0);
    GM_DIRECTOR_INFO(Core, "OccupierForces: pool forced to {} of {}", _pool, _poolStart);
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
        GM_DIRECTOR_INFO(Core, "OccupierForces: wave {} requested while the service is inert; parked, no effect", n);
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
            // the first active frame: the settlement probe on the seeded
            // towns (once per campaign, stored on the rows), then the
            // roster, K and the pool
            ActivateProbed(registry);
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

void OccupierForces::CountDownCooldowns(float elapsed, AutoArray<RString>* ended)
{
    for (int i = _cooldowns.Size() - 1; i >= 0; i--)
    {
        _cooldowns[i].seconds -= elapsed;
        if (_cooldowns[i].seconds <= 0)
        {
            if (ended)
            {
                ended->Add(_cooldowns[i].zone);
            }
            _cooldowns.Delete(i);
        }
    }
}

void OccupierForces::StepRearm(ZoneRegistry& registry, const DirectorView& view, const AutoArray<RString>& ended)
{
    if (_tuning.rearmHeat <= 0)
    {
        _rearm.Clear(); // the key off: the old rule
        return;
    }
    const RString resistance = registry.ResistanceSide();
    // a zone the director may still go for: held, a town or a base, not the
    // HQ's own zone (never a retake target, N4a)
    auto heldTarget = [&](int zi) {
        const ZoneRecord* z = registry.GetZone(zi);
        if (!z || zi == view.hqZone || stricmp(z->owner, resistance) != 0)
        {
            return false;
        }
        return IsHeldTown(registry, *z) || CampaignEnd::IsMilitaryType(z->type);
    };
    // a rest ran out: a held zone whose fighters the director still has
    // reported (its men, or a local who talked: D9) is re-armed
    for (int i = 0; i < ended.Size(); i++)
    {
        const int zi = registry.FindZoneIndex(ended[i]);
        // a contained zone is not attacked (the repel ladder, 6.3)
        if (zi < 0 || !heldTarget(zi) || HasName(_rearm, ended[i]) || Contained(ended[i]))
        {
            continue;
        }
        const int fighters = KnownFighters(registry, zi);
        if (fighters <= 0)
        {
            continue; // nobody reported: quiet, push-out's (6.4)
        }
        AddName(_rearm, ended[i]);
        GM_DIRECTOR_INFO(Core,
                         "OccupierForces: {} rested with {} fighter(s) reported: re-armed, heat held at {:.0f} until an "
                         "operation goes (forcesRearmHeat)",
                         (const char*)ended[i], fighters, (double)_tuning.rearmHeat);
    }
    for (int i = _rearm.Size() - 1; i >= 0; i--)
    {
        const RString name = _rearm[i].name;
        const int zi = registry.FindZoneIndex(name);
        if (zi < 0 || !heldTarget(zi) || KnownFighters(registry, zi) <= 0 || Contained(name))
        {
            LOG_DEBUG(Core, "OccupierForces: {} no longer re-armed (not held, nobody reported, or contained)",
                      (const char*)name);
            _rearm.Delete(i);
            continue;
        }
        const ZoneRecord* z = registry.GetZone(zi);
        if (z->heat < _tuning.rearmHeat)
        {
            registry.HeatRaise(zi, _tuning.rearmHeat - z->heat);
        }
    }
}

void OccupierForces::ForgetZone(const char* zone)
{
    RemoveName(_urgent, zone);
    RemoveName(_redPaid, zone);
    RemoveName(_rearm, zone);
    // the repel ladder resets when the zone changes hands (6.3)
    const int rr = FindRepel(zone);
    if (rr >= 0)
    {
        GM_DIRECTOR_INFO(Core, "OccupierForces: {} changed hands: the repel ladder resets ({} repel(s){})", zone,
                         _repels[rr].count, _repels[rr].contained ? ", contained" : "");
        _repels.Delete(rr);
    }
    if (zone && stricmp(_massTarget, zone) == 0)
    {
        _massTarget = RString();
        _massTicks = 0;
    }
    for (int i = _cooldowns.Size() - 1; i >= 0; i--)
    {
        if (zone && stricmp(_cooldowns[i].zone, zone) == 0)
        {
            _cooldowns.Delete(i);
        }
    }
}

// ---------------------------------------------------------------------------
// the repel ladder (6.3, 2026-10-16, video 5)
// ---------------------------------------------------------------------------

int OccupierForces::FindRepel(const char* zone) const
{
    for (int i = 0; zone && i < _repels.Size(); i++)
    {
        if (stricmp(_repels[i].zone, zone) == 0)
        {
            return i;
        }
    }
    return -1;
}

void OccupierForces::NoteRepel(ZoneRegistry& registry, int zone, int package, int source)
{
    const ZoneRecord* z = registry.GetZone(zone);
    if (!z)
    {
        return;
    }
    int r = FindRepel(z->name);
    if (r < 0)
    {
        RepelRow fresh;
        fresh.zone = z->name;
        _repels.Add(fresh);
        r = _repels.Size() - 1;
    }
    RepelRow& row = _repels[r];
    row.count++;
    row.package = package;
    const ZoneRecord* src = registry.GetZone(source);
    row.source = src ? src->name : RString();
    const int fighters = KnownFighters(registry, zone);
    const char* nth = RepelOrdinal(row.count);
    char last[96];
    snprintf(last, sizeof(last), "%s from %s", ForcePackages::PackageName(package),
             src ? (const char*)src->name : "an unknown base");
    if (_tuning.containRepels > 0 && row.count >= _tuning.containRepels)
    {
        // contain: the next operation's bodies surge the nearest base instead
        const int baseMin = MinInt(MaxInt(_tuning.retakeMin, RoundToInt(z->garrisonMax)), ForcePackages::MaxGroup);
        const int need = MinInt(MaxInt(baseMin, (int)ceilf(_tuning.escalateInfantry * (float)fighters - 1e-3f)),
                                ForcePackages::MaxGroup);
        const RString occupier = registry.OccupierSide();
        int base = -1;
        float bestSq = 0;
        for (int i = 0; i < registry.NZones(); i++)
        {
            const ZoneRecord* b = registry.GetZone(i);
            if (!b || !CampaignEnd::IsMilitaryType(b->type) || stricmp(b->owner, occupier) != 0 ||
                !RoadRoute::SameLandmass(b->pos, z->pos))
            {
                continue;
            }
            const float dx = b->pos.X() - z->pos.X();
            const float dz = b->pos.Z() - z->pos.Z();
            if (base < 0 || dx * dx + dz * dz < bestSq)
            {
                base = i;
                bestSq = dx * dx + dz * dz;
            }
        }
        const int got = base >= 0 ? Deposit(need, "containment surge") : 0;
        const bool pending = got > 0 && registry.AddReserveNow(base, got, -1);
        row.contained = true;
        RemoveName(_rearm, z->name);
        GM_DIRECTOR_INFO(Core,
                         "OccupierForces: {}: {} repel ({}), contained: no more attacks on it; {} surged by {}{} (the "
                         "containment lifts when its reports decay, a wave fires or control changes band)",
                         (const char*)z->name, nth, last, base >= 0 ? ZoneNameOf(registry, base) : "no base", got,
                         pending ? " (pending)" : "");
        return;
    }
    if (_tuning.approachRepels > 0 && row.count >= _tuning.approachRepels)
    {
        GM_DIRECTOR_INFO(Core,
                         "OccupierForces: {}: {} repel ({}), next attack from a different source or by air; a strike "
                         "may go with it if the pool stays STRETCHED",
                         (const char*)z->name, nth, last);
        return;
    }
    if (fighters > 0)
    {
        GM_DIRECTOR_INFO(
            Core,
            "OccupierForces: {}: {} repel ({}, {} fighter(s) reported), next attack needs lethality x{:.1f} "
            "and at least {} infantry",
            (const char*)z->name, nth, last, fighters, (double)_tuning.escalateLethality,
            MinInt((int)ceilf(_tuning.escalateInfantry * (float)fighters - 1e-3f), ForcePackages::MaxGroup));
    }
    else
    {
        GM_DIRECTOR_INFO(Core,
                         "OccupierForces: {}: {} repel ({}), nobody reported: the next attack escalates once "
                         "fighters are reported",
                         (const char*)z->name, nth, last);
    }
}

void OccupierForces::StepContainment(const ZoneRegistry& registry)
{
    const RString resistance = registry.ResistanceSide();
    for (int i = _repels.Size() - 1; i >= 0; i--)
    {
        RepelRow& row = _repels[i];
        const int zi = registry.FindZoneIndex(row.zone);
        const ZoneRecord* z = registry.GetZone(zi);
        if (!z || stricmp(z->owner, resistance) != 0)
        {
            // a scripted owner change sends no event: it changed hands all the same
            GM_DIRECTOR_INFO(Core, "OccupierForces: {} changed hands: the repel ladder resets ({} repel(s))",
                             (const char*)row.zone, row.count);
            _repels.Delete(i);
            continue;
        }
        if (row.contained && KnownFighters(registry, zi) <= 0)
        {
            row.contained = false;
            GM_DIRECTOR_INFO(Core,
                             "OccupierForces: {}: containment lifted: its reports decayed ({} repel(s) remembered)",
                             (const char*)row.zone, row.count);
        }
    }
}

void OccupierForces::LiftContainment(const char* why)
{
    for (int i = 0; i < _repels.Size(); i++)
    {
        if (_repels[i].contained)
        {
            _repels[i].contained = false;
            GM_DIRECTOR_INFO(Core, "OccupierForces: {}: containment lifted: {} ({} repel(s) remembered)",
                             (const char*)_repels[i].zone, why ? why : "", _repels[i].count);
        }
    }
}

int OccupierForces::CandidateValue(const ZoneRegistry& registry, int target) const
{
    if (target == HqCandidate)
    {
        return ValueHq;
    }
    const ZoneRecord* z = registry.GetZone(target);
    if (!z)
    {
        return ValueFarTown;
    }
    if (CampaignEnd::IsMilitaryType(z->type))
    {
        return ValueBase;
    }
    const RString occupier = registry.OccupierSide();
    const float r2 = _tuning.adjacentTown * _tuning.adjacentTown;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* b = registry.GetZone(i);
        if (!b || !CampaignEnd::IsMilitaryType(b->type) || stricmp(b->owner, occupier) != 0)
        {
            continue;
        }
        const float dx = b->pos.X() - z->pos.X();
        const float dz = b->pos.Z() - z->pos.Z();
        if (dx * dx + dz * dz <= r2)
        {
            return ValueAdjacentTown;
        }
    }
    return ValueFarTown;
}

RString OccupierForces::MassingReason(const DirectorAction& a)
{
    // a first attempt never masses: the ladder says when more is needed
    char buf[192] = "";
    if (a.escalation >= 1 && a.needBodies > a.bestSpare)
    {
        snprintf(buf, sizeof(buf), "need %d infantry, best source spares %d", a.needBodies, a.bestSpare);
    }
    else if (a.approachBlocked)
    {
        snprintf(buf, sizeof(buf), "%s", (const char*)a.choice.why);
    }
    else if (a.escalation >= 1 && a.go && a.choice.shortOfLethality)
    {
        snprintf(buf, sizeof(buf), "need lethality %.1f, the best package brings %.1f", (double)a.choice.lReq,
                 (double)a.choice.quote.lethality);
    }
    return buf[0] ? RString(buf) : RString();
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
    // L6: only a settled row goes (finished, nothing outstanding): a row
    // whose hull is still driving home keeps its refund, and a live
    // operation keeps its row.  When nothing is settled the ledger grows
    // past MaxOpRows (opsMax and the waves bound the live ones); past
    // MaxOpRows * 4 the oldest finished row is settled to the pool first
    // (as if its hull came home) so its units are never lost.
    while (_ops.Size() > MaxOpRows)
    {
        int drop = -1;
        for (int i = 0; i < _ops.Size() && drop < 0; i++)
        {
            if (_ops[i].finished && _ops[i].outstanding <= 0)
            {
                drop = i;
            }
        }
        if (drop < 0 && _ops.Size() > MaxOpRows * 4)
        {
            for (int i = 0; i < _ops.Size() && drop < 0; i++)
            {
                if (_ops[i].finished)
                {
                    drop = i;
                }
            }
            if (drop >= 0 && _ops[drop].outstanding > 0)
            {
                // crew CounterAttack still holds for it settle as bodies
                // (CrewHome, NoteLoss) once its hull is home or lost: only the
                // rest is the pool's, or they would be counted twice (5.5)
                int crew = 0;
                CounterAttack::Instance().OwedOnOp(_ops[drop].opId, &crew);
                const int refund = _ops[drop].outstanding - MinInt(crew, _ops[drop].outstanding);
                LOG_DEBUG(Core,
                          "OccupierForces: [op {}] ledger row pruned: {} outstanding unit(s) settled to the pool"
                          " ({} crew still to settle as bodies)",
                          _ops[drop].opId, refund, _ops[drop].outstanding - refund);
                Refund(refund, "ledger row pruned");
                _ops[drop].outstanding = 0;
            }
        }
        if (drop < 0)
        {
            break;
        }
        _ops.Delete(drop);
    }
}

void OccupierForces::ReconcileLegacyOpRows()
{
    int rows = 0;
    int units = 0;
    for (int i = 0; i < _ops.Size(); i++)
    {
        OpRow& r = _ops[i];
        if (r.crewLedger)
        {
            continue;
        }
        r.crewLedger = true;
        const int owed = CounterAttack::Instance().OwedOnOp(r.opId);
        if (r.outstanding > owed)
        {
            // the crew it kept were already settled as bodies (home, the
            // pool or the losses): the residue is written off, never
            // refunded, or PruneOps would create units from it
            units += r.outstanding - (owed > 0 ? owed : 0);
            r.outstanding = owed > 0 ? owed : 0;
            rows++;
        }
    }
    if (rows > 0)
    {
        GM_DIRECTOR_INFO(Core,
                 "OccupierForces: {} ledger row(s) from a save before the crew ledger held {} settled crew unit(s);"
                 " written off (nothing credited)",
                 rows, units);
    }
}

bool OccupierForces::BookLeverOp(int opId, const char* target, int kind, int package, int units)
{
    if (!Active() || units <= 0 || opId <= 0 || FindOp(opId) >= 0)
    {
        return false;
    }
    OpRow row;
    row.opId = opId;
    row.target = target ? target : "";
    row.kind = kind;
    row.origin = CAODirector;
    row.package = package;
    row.outstanding = units;
    row.lever = true;
    _ops.Add(row);
    LOG_DEBUG(Core, "OccupierForces: [op {}] test lever booked {} unit(s)", opId, units);
    PruneOps();
    return true;
}

void OccupierForces::CancelLeverOp(int opId)
{
    const int row = FindOp(opId);
    if (row < 0 || !_ops[row].lever)
    {
        return;
    }
    const int units = _ops[row].outstanding;
    _ops.Delete(row);
    Refund(units, "test lever refused");
}

void OccupierForces::NoteLeverOpEvent(int row, const CounterAttackOpEvent& ev)
{
    OpRow& r = _ops[row];
    const int units = MinInt(ev.units > 0 ? ev.units : 0, r.outstanding);
    switch (ev.type)
    {
        case CAOpBorn:
        case CAOpAirborne:
            r.born = true;
            break;
        case CAOpCancelledUnseen:
            // what CounterAttack still owes comes back; the rest was booked
            r.outstanding = 0;
            r.finished = true;
            Refund(units, "test lever cancelled unborn", ev.source);
            break;
        case CAOpHullRefund:
            r.outstanding -= units;
            if (ZoneRegistry* bank = BankRegistry())
            {
                // the hull flag (decision 9) applies to a lever's hull too
                char what[48];
                snprintf(what, sizeof(what), "%s%s", ev.note && strncmp(ev.note, "escort", 6) == 0 ? "escort of " : "",
                         ForcePackages::PackageName(r.package));
                RefundHull(*bank, units, r.package, what, ev.source);
            }
            else
            {
                Refund(units, "test lever hull home intact", ev.source);
            }
            break;
        case CAOpHullLost:
            r.outstanding -= units;
            NoteLoss(units, "test lever hull lost");
            break;
        case CAOpCrewSettled:
            // the crew were settled as bodies where they went (home, the
            // pool or the losses): the row lets go of them, the pool gets
            // nothing
            r.outstanding -= units;
            break;
        case CAOpRepelled:
        case CAOpSettled:
        case CAOpAborted:
            r.finished = true;
            break;
        case CAOpReturned:
            if (r.kind == CAKAlarm || r.kind == CAKStrike)
            {
                r.finished = true;
            }
            break;
        default:
            break;
    }
    if (r.kind == CAKStrike &&
        (ev.type == CAOpHullRefund || ev.type == CAOpHullLost || ev.type == CAOpCrewSettled) && r.outstanding <= 0)
    {
        r.finished = true;
    }
    PruneOps();
    // the lever's airframe home or lost moves the band's strength too
    UpdateBand();
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
        else if (z->heat < _tuning.urgentExit)
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
    // the order (6.3, 2026-10-16): urgent, whole heat points, value (the tiebreak
    // of video 5's two towns re-armed at 40), heat; stable on the zone order
    struct Entry
    {
        int zone;
        float heat;
        bool urgent;
        int value;
    };
    AutoArray<Entry> entries;
    auto before = [](const Entry& a, const Entry& b)
    {
        if (a.urgent != b.urgent)
        {
            return a.urgent;
        }
        if (floorf(a.heat) != floorf(b.heat))
        {
            return floorf(a.heat) > floorf(b.heat);
        }
        if (a.value != b.value)
        {
            return a.value > b.value;
        }
        return a.heat > b.heat;
    };
    auto insert = [&](int zone, float heat, const char* name)
    {
        Entry e{zone, heat, Urgent(name), CandidateValue(registry, zone)};
        int at = entries.Size();
        while (at > 0 && before(e, entries[at - 1]))
        {
            at--;
        }
        entries.Add(e);
        for (int k = entries.Size() - 1; k > at; k--)
        {
            entries[k] = entries[k - 1];
        }
        entries[at] = e;
    };
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* z = registry.GetZone(i);
        if (!z || stricmp(z->owner, resistance) != 0 || Contained(z->name))
        {
            continue; // a contained zone is not attacked (the repel ladder)
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
            insert(i, z->heat, z->name);
            continue;
        }
        if (!IsHeldTown(registry, *z) || i == view.hqZone || Cooldown(z->name) > 0 || launcher.HasRecord(i) || !hot)
        {
            continue;
        }
        insert(i, z->heat, z->name);
    }
    // the HQ (D13, D14, D24): an established HQ with no assault on it and no
    // cooldown; while STRONG or in a wave by the heat of the zone it stands
    // in (none for an HQ outside every zone), and unconditionally once the
    // resistance holds no base and no town besides the HQ's zone
    bool finalPush = false;
    if (view.hqEstablished && !launcher.HasHqRecord() && Cooldown(HqCooldownName) <= 0)
    {
        const ZoneRecord* hz = registry.GetZone(view.hqZone);
        finalPush = FinalPush(registry, view.hqZone);
        const bool hotHq = hz && hz->heat / full >= CandidateRoll(*hz);
        if (!finalPush && (strong || InWave()) && hotHq)
        {
            insert(HqCandidate, hz->heat, hz->name);
        }
    }
    if (finalPush)
    {
        out.Add(HqCandidate);
    }
    for (int i = 0; i < entries.Size(); i++)
    {
        out.Add(entries[i].zone);
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
        if (!z || i == view.hqZone || Cooldown(z->name) > 0 || launcher.HasRecord(i) || Contained(z->name))
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

DirectorAction OccupierForces::PlanFor(const ZoneRegistry& registry, const DirectorView& view, int target, int origin,
                                       int fixedSource, int fixedSpare, bool best) const
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
    req.roadFactor = ForcePackages::RoadFactor(); // the road quote (6.5): a ground leg with no road route known
    // the arrival rule (6.5, 2026-10-15) for the director's own operations
    // and, since 2026-10-16 (video 4: a push-out walked 55 minutes from the
    // Airfield), push-outs (6.4): a foot quote under forcesSlowEta stays the
    // cheapest and goes.  Wave strikes too since 2026-10-16 (video 5: the
    // wave's HQ assault walked 41 minutes from the harbour that had just
    // landed it): chosen from the hub within its staged budget (7.1)
    if (origin == CAODirector || origin == CAOPushout || origin == CAOWave)
    {
        req.slowEta = _tuning.slowEta;
        req.slowCostShare = _tuning.slowCostShare;
        req.airEta = _tuning.airEta;
    }
    AutoArray<PackageSource> sources;
    BuildSources(registry, view, dest, fixedSource, fixedSpare, -1, sources);
    // the spare cap (7.2, 2026-10-15): a base retake or an HQ assault whose
    // minimum no single base can spare takes the largest single-base spare,
    // never below forcesSpareCapFloor.  Two-source launches are not built:
    // CounterAttack keeps one record per target zone (FindByTarget) and one
    // HQ_ASSAULT at a time, and the revert adopts that one record.  A wave
    // strike (fixedSource) stages its own infantry and needs no cap.
    a.minAsked = req.minBodies;
    const bool capKind = hq || a.kind == CAKBase;
    if (capKind && fixedSource < 0)
    {
        if (!_minRuleLogged)
        {
            _minRuleLogged = true;
            if (_tuning.spareCap)
            {
                GM_DIRECTOR_INFO(Core,
                                 "OccupierForces: base retakes and HQ assaults take the spare cap: one source, at "
                                 "least the largest single-base spare, never below {} (forcesSpareCap 1)",
                                 _tuning.spareCapFloor);
            }
            else
            {
                GM_DIRECTOR_INFO(Core, "OccupierForces: base retakes and HQ assaults need their whole minimum from "
                                       "one source (forcesSpareCap 0)");
            }
        }
        if (_tuning.spareCap)
        {
            const bool heli = _catalogue.Offered(FPTransportHeli);
            req.minBodies = SpareCappedMin(req.minBodies, LargestSpare(sources, heli), _tuning.spareCapFloor);
        }
    }
    a.minUsed = req.minBodies;
    // the repel ladder (6.3, 2026-10-16) on a director or push-out plan
    const bool heli = _catalogue.Offered(FPTransportHeli);
    const int ri = !hq && fixedSource < 0 && (origin == CAODirector || origin == CAOPushout) ? FindRepel(z->name) : -1;
    const RepelRow* rr = ri >= 0 ? &_repels[ri] : nullptr;
    a.bestSpare = LargestSpare(sources, heli);
    if (rr && rr->count >= 1 && a.fighters > 0)
    {
        // escalate: more lethality, and infantry to assault, not hulls alone
        a.escalation = rr->count;
        req.margin *= _tuning.escalateLethality > 1 ? _tuning.escalateLethality : 1.0f;
        const int need = MinInt((int)ceilf(_tuning.escalateInfantry * a.fighters - 1e-3f), ForcePackages::MaxGroup);
        req.minBodies = MaxInt(req.minBodies, need);
    }
    a.needBodies = req.minBodies;
    if (best && req.minBodies > a.bestSpare)
    {
        // the massing fallback: the strongest package one base can bring
        req.minBodies = MaxInt(a.minUsed, a.bestSpare);
    }
    // phase 6: a helicopter insertion competes like any package (6.5, D3)
    a.airOrigin = HeldAirfield(registry, dest);
    if (rr && _tuning.approachRepels > 0 && rr->count >= _tuning.approachRepels)
    {
        // a different approach: another base, else an insertion (ground after air)
        a.escalation = rr->count;
        const int lastSource = registry.FindZoneIndex(rr->source);
        const bool lastAir = rr->package == FPTransportHeli;
        AutoArray<PackageSource> others;
        for (int i = 0; i < sources.Size(); i++)
        {
            if (sources[i].zone != lastSource)
            {
                others.Add(sources[i]);
            }
        }
        const PackageChoice other = ChooseWithAir(req, others, a.airOrigin);
        PackageRequest rb = req;
        rb.airOffMap = a.airOrigin < 0;
        rb.offered = lastAir ? ForcePackages::GroundPackages : (heli ? ForcePackages::Bit(FPTransportHeli) : 0u);
        PackageChoice mode;
        if (rb.offered != 0)
        {
            mode = ForcePackages::Choose(rb, _catalogue, sources);
        }
        const PackageChoice* pick = other.ok && !other.shortOfLethality ? &other
                                    : mode.ok && !mode.shortOfLethality ? &mode
                                    : other.ok                          ? &other
                                    : mode.ok                           ? &mode
                                                                        : nullptr;
        if (!pick && best)
        {
            // the massing fallback: no other approach can go, so any
            a.choice = ChooseWithAir(req, sources, a.airOrigin);
            a.go = a.choice.ok;
            return a;
        }
        if (!pick)
        {
            a.approachBlocked = true;
            a.choice = other;
            a.choice.ok = false;
            a.choice.why = "no different approach yet: no other base can staff it and no insertion can go";
            a.go = false;
            return a;
        }
        a.choice = *pick;
        a.go = true;
        char note[160];
        const char* lastName = rr->source.GetLength() > 0 ? (const char*)rr->source : "the last source";
        if (pick == &other)
        {
            snprintf(note, sizeof(note), "%s from %s instead of %s from %s",
                     ForcePackages::PackageName(pick->quote.package), ZoneNameOf(registry, pick->quote.source),
                     ForcePackages::PackageName(rr->package), lastName);
        }
        else
        {
            snprintf(note, sizeof(note), "%s instead of %s from %s", lastAir ? "on the ground" : "by air",
                     ForcePackages::PackageName(rr->package), lastName);
        }
        a.approach = note;
        return a;
    }
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
        // the road quote (6.5, 2026-10-16): the road route the service will
        // march, cached per pair (RoadQuoteCache, planned at most once a
        // tick); -1 = the straight line x forcesRoadFactor.  Air flies the
        // straight line.
        src.road = src.airOnly ? -1.0f : RoadQuoteCache::Instance().Length(s->pos, dest);
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

int OccupierForces::TryLaunch(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, int target,
                              int origin)
{
    const bool hq = target == HqCandidate;
    const ZoneRecord* z = registry.GetZone(target);
    if (!z && !hq)
    {
        return LOFailed;
    }
    const RString name = hq ? RString(HqCooldownName) : z->name;
    // M1: when CounterAttack refuses the chosen source, the same target is
    // re-planned without it (a local copy of the view with its spare at 0),
    // a bounded number of times.  Still one operation per tick: the loop
    // ends at the first launch.
    DirectorView local;
    const DirectorView* v = &view;
    for (int attempt = 0; attempt <= MaxSourceRetries; attempt++)
    {
        DirectorAction a = PlanFor(registry, *v, target, origin);
        // massing (6.3): under strength, a director candidate holds its token
        const RString mass = origin == CAODirector ? MassingReason(a) : RString();
        if (mass.GetLength() > 0)
        {
            if (stricmp(_massTarget, name) != 0)
            {
                _massTarget = name;
                _massTicks = 0;
                _massTokens = _tokens;
            }
            _massTicks++;
            const bool token = _tokens > _massTokens;
            if (!token && _massTicks <= _tuning.massingMax)
            {
                GM_DIRECTOR_INFO(Core, "OccupierForces: massing for {}: {} (the token is held, tick {} of {})",
                                 hq ? "the HQ" : (const char*)name, (const char*)mass, _massTicks, _tuning.massingMax);
                return LOMassing;
            }
            GM_DIRECTOR_INFO(Core, "OccupierForces: massing for {} ends ({}): {}, the strongest package goes",
                             hq ? "the HQ" : (const char*)name, (const char*)mass,
                             token ? "a further token" : "forcesMassingMax ticks held");
            _massTarget = RString();
            _massTicks = 0;
            a = PlanFor(registry, *v, target, origin, -1, 0, true);
        }
        else if (stricmp(_massTarget, name) == 0)
        {
            if (a.go)
            {
                GM_DIRECTOR_INFO(Core, "OccupierForces: massing for {} ends: {} can staff it",
                                 hq ? "the HQ" : (const char*)name, ZoneNameOf(registry, a.choice.quote.source));
            }
            _massTarget = RString();
            _massTicks = 0;
        }
        if (!a.go)
        {
            if (LogAllowed())
            {
                GM_DIRECTOR_INFO(Core, "OccupierForces: no operation on {}: {}{}", hq ? "the HQ" : (const char*)z->name,
                         (const char*)a.choice.why, _logSuppressed > 0 ? " (other lines suppressed)" : "");
                _logSuppressed = 0;
            }
            return LOFailed;
        }
        bool refused = false;
        if (LaunchAction(registry, *v, launcher, a, true, false, 0, &refused))
        {
            if (a.escalation > 0)
            {
                // the repel ladder's rung on this launch (6.3)
                GM_DIRECTOR_INFO(Core,
                                 "OccupierForces: [op {}] the repel ladder on {} ({} repel(s)): lethality {:.2f} "
                                 "needed, at least {} infantry{}{}",
                                 _opCounter, (const char*)name, a.escalation, (double)a.choice.lReq, a.needBodies,
                                 a.approach.GetLength() > 0 ? "; a different approach: " : "", (const char*)a.approach);
            }
            return LOLaunched;
        }
        const int source = a.choice.quote.source;
        if (!refused || source < 0)
        {
            return LOFailed; // the pool refused: no other source changes that
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
    return LOFailed;
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
    // the debit, a staging deposit and a refusal's refund are one ledger
    // movement: the band is stepped once, on the net pool (2026-10-16)
    BandHold hold(*this);
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
            Refund(registry, q.poolPrice, "wave staging refused", q.source);
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
    row.airOrigin = a.airOrigin;
    _ops.Add(row);
    _opCounter = opId;
    if (!launcher.Launch(order))
    {
        Refund(registry, q.poolPrice, "operation refused", q.source);
        if (stagedBodies > 0 && src)
        {
            // the staging goes back where it came from
            src->garrison = floorf(src->garrison) - (float)stagedBodies;
            if (src->garrison < 0)
            {
                src->garrison = 0;
            }
            Refund(registry, stagedBodies, "wave staging refused", q.source);
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
        GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] CounterAttack refused the {} operation from {} to {}", opId,
                 ForcePackages::PackageName(q.package), ZoneNameOf(registry, q.source), targetName);
        return false;
    }
    if (spendToken)
    {
        _tokens = _tokens > 0 ? _tokens - 1 : 0;
    }
    // a RED edge from here on pays the heat of a repel; a re-armed zone has
    // its operation (6.3)
    if (z)
    {
        RemoveName(_redPaid, z->name);
        RemoveName(_rearm, z->name);
    }
    PruneOps();

    // the 2026-10-15 rules, one line each when they changed the operation
    if (a.minUsed > 0 && a.minUsed < a.minAsked)
    {
        GM_DIRECTOR_INFO(Core,
                         "OccupierForces: [op {}] spare cap: no base can spare the {} bodies {} asks, {} from {} "
                         "(the largest single-base spare, floor {})",
                         opId, a.minAsked, targetName, q.bodies, ZoneNameOf(registry, q.source),
                         _tuning.spareCapFloor);
    }
    if (a.choice.etaRule != PackageChoice::EtaNone)
    {
        GM_DIRECTOR_INFO(Core,
                         "OccupierForces: [op {}] arrival rule ({}): the cheapest, {}, needs {}; {} goes, {}",
                         opId, ForcePackages::ReasonWords(a.choice.reason),
                         ForcePackages::PackageName(a.choice.cheapestPackage),
                         a.choice.cheapestIndex >= 0
                             ? (const char*)ForcePackages::LogEtaNote(a.choice.quotes[a.choice.cheapestIndex])
                             : "?",
                         ForcePackages::PackageName(q.package), (const char*)ForcePackages::EtaNote(q));
    }

    const bool push = a.origin == CAOPushout;
    // a column names its escort (6.6): "Motorized squad (4, Ural) with T72"
    char escort[80] = "";
    if (q.HasEscort() && q.escortClass.GetLength() > 0)
    {
        snprintf(escort, sizeof(escort), " with %s", (const char*)q.escortClass);
    }
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
            GM_DIRECTOR_INFO(Core,
                     "OccupierForces: [op {}] {}{} ({}) from {} to {} ({} unit(s), the pool pays {}, lethality {:.2f} "
                     "of {:.2f}, {} fighter(s) reported{}{}, {})",
                     opId, push ? "Push-out: " : "", label, q.bodies, ZoneNameOf(registry, q.source), targetName,
                     q.price, q.poolPrice, (double)q.lethality, (double)a.choice.lReq, RoundToInt(a.fighters),
                     a.urgent ? ", urgent" : "", shortNote, (const char*)ForcePackages::LogEtaNote(q));
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
            GM_DIRECTOR_INFO(Core,
                     "OccupierForces: [op {}] {}{} ({}, {}){} from {} to {} ({} unit(s), the pool pays {}, lethality "
                     "{:.2f} of {:.2f}, {} fighter(s) reported{}{}, {})",
                     opId, push ? "Push-out: " : "", label, q.bodies, (const char*)q.hullClass, escort, from, targetName,
                     q.price, q.poolPrice, (double)q.lethality, (double)a.choice.lReq, RoundToInt(a.fighters),
                     a.urgent ? ", urgent" : "", shortNote, (const char*)ForcePackages::LogEtaNote(q));
        }
        return true;
    }
    if (a.kind == CAKStrike)
    {
        // 6.6: "Jet strike over Houdan (town)"
        const char* air = q.package == FPJet ? "Jet strike" : "Attack helicopter strike";
        const char* what = z && stricmp(z->type, "CITY") == 0 ? "town" : (hq ? "the HQ" : "opener");
        GM_DIRECTOR_INFO(Core,
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
        GM_DIRECTOR_INFO(Core,
                 "OccupierForces: [op {}] Alarm answer: {} ({}{}{}){} from {} to {} ({} unit(s), the pool pays {}, "
                 "lethality {:.2f} of {:.2f}, {} fighter(s) reported{}{}, {})",
                 opId, ForcePackages::PackageName(q.package), q.bodies, q.package != FPFoot ? ", " : "",
                 (const char*)q.hullClass, escort, from, targetName, q.price, q.poolPrice, (double)q.lethality,
                 (double)a.choice.lReq, RoundToInt(a.fighters), a.urgent ? ", urgent" : ", THIN: the cheapest",
                 shortNote, (const char*)ForcePackages::LogEtaNote(q));
        return true;
    }
    // base retakes, HQ assaults and wave strikes (6.6 wording)
    const char* what = hq                          ? "HQ assault"
                       : a.kind == CAKBase         ? "Base retake"
                       : a.kind == CAKReinforce    ? "Reinforce column"
                                                   : "Town retake";
    const char* pkg = q.package == FPFoot ? "foot" : (q.package == FPTruck ? "motorized" : "package");
    GM_DIRECTOR_INFO(Core,
             "OccupierForces: [op {}] {}{}{} ({} {}{}{}){} from {} on {} ({} unit(s), the pool pays {}{}, lethality "
             "{:.2f} of {:.2f}, {} fighter(s) reported{}{}, {})",
             opId, push ? "Push-out: " : "", wave, what, pkg, q.bodies, q.package != FPFoot ? ", " : "",
             (const char*)q.hullClass, escort, ZoneNameOf(registry, q.source), targetName, q.price, q.poolPrice,
             stagedBodies > 0 ? " and stages the infantry" : "", (double)q.lethality, (double)a.choice.lReq,
             RoundToInt(a.fighters), a.urgent ? ", urgent" : "", shortNote,
             (const char*)ForcePackages::LogEtaNote(q));
    return true;
}

void OccupierForces::Direct(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher, float elapsed)
{
    if (!Active())
    {
        return;
    }
    AutoArray<RString> rested;
    CountDownCooldowns(elapsed, &rested);
    DecaySightings();
    // the repel ladder: changed hands, decayed reports (6.3)
    StepContainment(registry);
    // a defended held zone whose rest ran out stays a candidate (6.3)
    StepRearm(registry, view, rested);
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
    if (launcher.BornOps() >= OpsMax())
    {
        return; // unborn operations do not count, born ones do (forcesOpsMaxStrong while STRONG)
    }
    AutoArray<int> candidates;
    Candidates(registry, view, launcher, candidates);
    auto nameOf = [&registry](int target)
    {
        const ZoneRecord* cz = registry.GetZone(target);
        return target == HqCandidate ? RString(HqCooldownName) : (cz ? cz->name : RString());
    };
    // massing (6.3): the held target first; dropped when no longer a candidate
    if (_massTarget.GetLength() > 0)
    {
        int at = -1;
        for (int i = 0; i < candidates.Size() && at < 0; i++)
        {
            at = stricmp(nameOf(candidates[i]), _massTarget) == 0 ? i : -1;
        }
        if (at < 0)
        {
            GM_DIRECTOR_INFO(Core, "OccupierForces: massing for {} dropped: no longer a candidate",
                             (const char*)_massTarget);
            _massTarget = RString();
            _massTicks = 0;
        }
        for (int k = at; k > 0; k--)
        {
            const int t = candidates[k];
            candidates[k] = candidates[k - 1];
            candidates[k - 1] = t;
        }
    }
    for (int i = 0; i < candidates.Size(); i++)
    {
        const int target = candidates[i];
        const ZoneRecord* tz = registry.GetZone(target);
        const int kind = target == HqCandidate ? CAKHqAssault
                         : (tz && CampaignEnd::IsMilitaryType(tz->type)) ? CAKBase
                                                                        : CAKRetake;
        // the operation and its opener are one ledger movement (2026-10-16):
        // the band is stepped once on the net pool when the hold closes, so
        // an opener bought and refunded in this step never shows an edge.
        // StrikeDue still reads the band the ground operation left (BandNow)
        BandHold hold(*this);
        const int outcome = TryLaunch(registry, view, launcher, target, CAODirector);
        if (outcome == LOLaunched)
        {
            // the candidate line (6.3, 2026-10-16): its value weight
            const int value = CandidateValue(registry, target);
            const ZoneRecord* hz = target == HqCandidate ? registry.GetZone(view.hqZone) : tz;
            GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] candidate {}, {} of {}: {} (weight {}), heat {:.0f}{}",
                             _opCounter, target == HqCandidate ? "the HQ" : (const char*)nameOf(target), i + 1,
                             candidates.Size(),
                             target == HqCandidate        ? "the HQ"
                             : value == ValueBase         ? "a base"
                             : value == ValueAdjacentTown ? "a town by an occupier base"
                                                          : "a far town",
                             value, hz ? (double)hz->heat : 0.0, hz && Urgent(hz->name) ? ", urgent" : "");
            // the strike that goes with it (7.3): a base / HQ opener with a
            // defender reported, an urgent town with fighters reported, or
            // the repel ladder's escalation strike
            MaybeStrike(registry, view, launcher, target, kind);
            return; // one operation per tick
        }
        if (outcome == LOMassing)
        {
            return; // the token is kept: massing, nothing else goes this tick
        }
        // no ground package could go: an urgent town with fighters may still
        // be struck from the air on its own (it takes the token)
        if (kind == CAKRetake && StrikeDue(registry, view, target, kind))
        {
            const DirectorAction sa = PlanStrike(registry, view, target, true, true);
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
        GM_DIRECTOR_INFO(Core, "OccupierForces: Wave {} absorbs a further trigger ({}){}", _waveCount, why ? why : "",
                 final ? ": it carries the final stand" : "");
        return;
    }
    _waveCount++;
    _waveInbound = true;
    _waveFinalStand = final;
    // the parent nation sends more: contained zones are weighed again (6.3)
    LiftContainment("a wave fired");
    // a wave is a share of poolStart + Rgen (5.7): generated bases cost no wave
    _waveBodies = WaveSize(_tuning, _poolStart + _Rgen);
    _waveClock = final ? _tuning.waveLeadFinal : _tuning.waveLead;
    const int hub = PickHub(registry, _tuning, &view.garrisonLive);
    GM_DIRECTOR_INFO(Core, "OccupierForces: Wave {} ({}) inbound to {}, control {:.2f}{} ({}; lands in {:.0f} s)", _waveCount,
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
    GM_DIRECTOR_INFO(Core, "OccupierForces: Wave {}: {} stay in the pool (pool {} of {})", _waveCount, rest, _pool,
             _poolStart);
}

int OccupierForces::AllocateWave(ZoneRegistry& registry, const DirectorView& view, DirectorLauncher& launcher,
                                 int bodies, bool final)
{
    int budget = bodies;
    const RString occupier = registry.OccupierSide();
    // the S5 deposit bubble only (where the player stands decides pending vs
    // reserve, never what the director sends): ZoneRegistry::AddReserveNow
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
    // the wave's surge gathers at the bases nearest its cluster (decision 5)
    {
        const int anchor =
            BestCluster(registry, _tuning.waveCluster, view.hqZone, view.hqEstablished ? view.hqPos : VZero);
        const ZoneRecord* az = registry.GetZone(anchor >= 0 ? anchor : hub);
        _surgeAnchor = az ? az->pos : VZero;
    }
    if (hub >= 0 && budget > 0)
    {
        const int want = MinInt(shortfall(hub), budget);
        const int got = want > 0 ? Deposit(want, "wave hub top-up") : 0;
        if (got > 0)
        {
            const bool pending = registry.AddReserveNow(hub, got, -1);
            budget -= got;
            GM_DIRECTOR_INFO(Core, "OccupierForces: Wave {}: hub top-up {} at {}{}", _waveCount, got,
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
            const bool pending = registry.AddReserveNow(last, got, -1);
            budget -= got;
            const ZoneRecord* z = registry.GetZone(last);
            if (stricmp(_surgeZone, z->name) != 0)
            {
                _surgeBodies = 0;
            }
            _surgeZone = z->name;
            _surgeBodies += got;
            GM_DIRECTOR_INFO(Core, "OccupierForces: Wave {}: final-stand surge {} at {}{} (never spare)", _waveCount, got,
                     (const char*)z->name, pending ? " (pending)" : "");
        }
    }
    // 3. strikes on the best cluster, only once the director is AWAKE (W)
    int ops = 0;
    if (_posture != OFAwake)
    {
        GM_DIRECTOR_INFO(Core, "OccupierForces: Wave {}: no strikes, the director is DORMANT", _waveCount);
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
            GM_DIRECTOR_INFO(Core, "OccupierForces: Wave {}: strikes on the cluster around {} ({} target(s), up to {})",
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
                GM_DIRECTOR_INFO(Core, "OccupierForces: Wave {}: no base can send a strike", _waveCount);
                break;
            }
            const DirectorAction a = PlanFor(registry, view, targets[t], CAOWave, source, strikeBudget);
            if (!a.go || a.choice.quote.bodies > strikeBudget)
            {
                GM_DIRECTOR_INFO(Core, "OccupierForces: Wave {}: no strike on {}: {}", _waveCount,
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
            GM_DIRECTOR_INFO(Core, "OccupierForces: a local in {} talked: {} fighter(s) reported (standing {:.0f}){}",
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
        GM_DIRECTOR_INFO(Core, "OccupierForces: {} taken, {} distinct town(s) so far", (const char*)z->name, _townsEver.Size());
    }
    if (_posture == OFDormant && _townsEver.Size() >= _tuning.wakeTowns)
    {
        // invariant W: the flip that brings the set to forcesWakeTowns wakes
        // the director with a full purse, and goes to it
        _posture = OFAwake;
        _tokens = _tuning.tokensMax;
        _tokenClock = 0;
        GM_DIRECTOR_INFO(Core, "OccupierForces: AWAKE after {} distinct towns ({} the latest), {} token(s)", _townsEver.Size(),
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
    if (row >= 0 && _ops[row].lever)
    {
        NoteLeverOpEvent(row, ev);
        return;
    }
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
                GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] Re-sourced to {}: the nearer base's road was watched", ev.opId,
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
            Refund(registry, refund, "cancelled unborn", ev.source);
            // the rest carries the hashed jitter (6.3, 2026-10-16)
            const float rest = JitteredRest(_tuning, target, ev.opId);
            SetCooldown(target, rest);
            GM_DIRECTOR_INFO(
                Core, "OccupierForces: [op {}] Cancelled unborn: {}; {} unit(s) back to the pool, {} rests {:.0f} s",
                ev.opId, ev.note && *ev.note ? ev.note : "every route watched", refund, target, (double)rest);
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
            // the rest carries the hashed jitter (6.3, 2026-10-16): no clockwork
            const float rest = JitteredRest(_tuning, target, ev.opId);
            SetCooldown(target, rest);
            _ops[row].finished = true;
            GM_DIRECTOR_INFO(Core,
                             "OccupierForces: [op {}] Repelled at {}: {} lost, {} going home; {} rests {:.0f} s{}",
                             ev.opId, target, ev.units, ev.bodies, target, (double)rest,
                             paid ? " (a RED edge already raised the heat)" : ", heat +15");
            // the repel ladder (6.3): town and base retakes, any origin, never the HQ
            if (ev.target >= 0 && (ev.kind == CAKRetake || ev.kind == CAKBase))
            {
                NoteRepel(registry, ev.target, _ops[row].package, ev.source);
            }
            break;
        }
        case CAOpSettled:
            if (mine && ev.kind == CAKHqAssault)
            {
                // the HQ fell: the assault joins no garrison, its bodies were
                // refunded just before this event (CounterAttack's "HQ assault
                // done"); the line says where they went (6.6, video 4)
                _ops[row].finished = true;
                const RefundTally& t = _lastRefund;
                if (ev.bodies <= 0)
                {
                    GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] HQ assault done: nobody of the assault left standing",
                                     ev.opId);
                }
                else if (t.valid && t.units == ev.bodies && stricmp(t.why, "HQ assault done") == 0)
                {
                    char banked[160] = "";
                    if (t.banked > 0)
                    {
                        snprintf(banked, sizeof(banked), ", %d banked at %s", t.banked, (const char*)t.bankedAt);
                    }
                    char dropped[64] = "";
                    if (t.dropped > 0)
                    {
                        snprintf(dropped, sizeof(dropped), ", %d dropped (every base full)", t.dropped);
                    }
                    GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] HQ assault done: {} back to the pool{}{}", ev.opId,
                                     t.pooled, banked, dropped);
                }
                else
                {
                    GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] HQ assault done: {} back to the pool", ev.opId,
                                     ev.bodies);
                }
                _lastRefund.valid = false;
            }
            else if (mine)
            {
                _ops[row].finished = true;
                GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] {} retaken: {} stay as its garrison", ev.opId, target,
                         ev.bodies);
            }
            break;
        case CAOpAborted:
            if (mine)
            {
                _ops[row].finished = true;
                GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] Aborted: {}", ev.opId, ev.note ? ev.note : "");
            }
            break;
        case CAOpHullRefund:
            if (mine)
            {
                // CounterAttack found no stored spare for the hull at launch
                // and sent the force on foot (it rewrote its record's
                // package): the ledger row follows, so gmOccupierOps and the
                // row agree.  The notes are CounterAttack::LaunchCore's and
                // AirGoLive's (an insertion with no LZ or no airframe).
                const int hullPackage = _ops[row].package; // before a downgrade rewrites it
                if (!_ops[row].born && ev.note &&
                    (strncmp(ev.note, "no stored spare", 15) == 0 ||
                     strncmp(ev.note, "insertion impossible", 20) == 0))
                {
                    _ops[row].package = FPFoot;
                }
                const int units = MinInt(ev.units > 0 ? ev.units : 0, _ops[row].outstanding);
                _ops[row].outstanding -= units;
                // the hull flag (decision 9): over the ceiling a hull comes
                // back as a stored spare at its base (an airframe at the
                // airfield it flew from), never as bodies, unless
                // forcesBankHulls is 1
                const bool escort = ev.note && strncmp(ev.note, "escort", 6) == 0;
                char what[48];
                snprintf(what, sizeof(what), "%s%s", escort ? "escort of " : "",
                         ForcePackages::PackageName(hullPackage));
                const bool airframe = hullPackage == FPTransportHeli || hullPackage == FPAttackHeli ||
                                      hullPackage == FPJet;
                const int home = airframe && _ops[row].airOrigin >= 0 ? _ops[row].airOrigin : ev.source;
                const ZoneRecord* tz = registry.GetZone(ev.target);
                RefundHull(registry, units, hullPackage, what, home, home < 0 && tz ? &tz->pos : nullptr);
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
        case CAOpCrewSettled:
            if (mine)
            {
                // the crew are priced as bodies (1 each) on the row, but they
                // settle as bodies: CounterAttack already sent them home
                // (CrewHome: the reserve, the surplus to the pool), refunded
                // them or booked them lost.  The row lets go of them and the
                // pool is credited nothing, so pool + reserves + pending +
                // outstanding holds and a settled row reads 0 (5.5)
                const int units = MinInt(ev.units > 0 ? ev.units : 0, _ops[row].outstanding);
                _ops[row].outstanding -= units;
                LOG_DEBUG(Core, "OccupierForces: [op {}] {} crew settled ({}); {} outstanding", ev.opId, units,
                          ev.note ? ev.note : "", _ops[row].outstanding);
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
                GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] Alarm answer at {}: searching the reported positions", ev.opId,
                         ZoneNameOf(registry, ev.zone >= 0 ? ev.zone : ev.target));
            }
            break;
        default:
            // Launched, Arrived: CounterAttack routes the bodies (5.5)
            // itself; nothing for the ledger
            break;
    }
    // a strike's airframe home (refund) or lost ends the strike
    if (mine && _ops[row].kind == CAKStrike &&
        (ev.type == CAOpHullRefund || ev.type == CAOpHullLost || ev.type == CAOpCrewSettled) &&
        _ops[row].outstanding <= 0)
    {
        _ops[row].finished = true;
    }
    PruneOps();
    // a ledger row's airframe home or lost moves the band's strength
    // (CommittedAir, 5.6) even when the pool did not move
    UpdateBand();
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
    // STRONG on the pool as it stands: inside a BandHold (the operation and
    // its opener) the band the ground operation's debit would step to
    if (!Active() || BandNow() != OFBandStrong)
    {
        return false;
    }
    if (!_catalogue.Offered(FPJet) && !_catalogue.Offered(FPAttackHeli))
    {
        return false;
    }
    // the opener of a base retake or an HQ assault: a strike on defenders,
    // so only with one reported (2026-10-16: an empty base retake bought a
    // 23-unit jet on 0 reported fighters)
    if (launchedKind == CAKBase || launchedKind == CAKHqAssault)
    {
        return OpenerFighters(registry, view, target) >= StrikeOpenerMinFighters;
    }
    const ZoneRecord* z = registry.GetZone(target);
    if (!z || !IsHeldTown(registry, *z) || !Urgent(z->name))
    {
        return false;
    }
    if (KnownFighters(registry, target) < _tuning.strikeMinFighters)
    {
        return false;
    }
    // a town inside its strike window is not struck again
    return !registry.IsStruck(target, view.now);
}

int OccupierForces::OpenerFighters(const ZoneRegistry& registry, const DirectorView& view, int target) const
{
    if (target != HqCandidate)
    {
        return KnownFighters(registry, target);
    }
    if (registry.GetZone(view.hqZone))
    {
        return KnownFighters(registry, view.hqZone);
    }
    if (!view.hqEstablished)
    {
        return 0;
    }
    // an HQ outside every zone: the reported positions near it, from the
    // rows of reports outside every zone that have not decayed
    int n = 0;
    const float r2 = OpenerHqRadius * OpenerHqRadius;
    for (int i = 0; i < _sightings.Size(); i++)
    {
        const SightingRow& row = _sightings[i];
        if (row.zone.GetLength() > 0 || Now() - row.stamp > _tuning.intelDecay)
        {
            continue;
        }
        for (int p = 0; p < row.positions.Size(); p++)
        {
            const float dx = row.positions[p].X() - view.hqPos.X();
            const float dz = row.positions[p].Z() - view.hqPos.Z();
            n += dx * dx + dz * dz <= r2 ? 1 : 0;
        }
    }
    return n;
}

DirectorAction OccupierForces::PlanStrike(const ZoneRegistry& registry, const DirectorView& view, int target,
                                          bool urgent, bool keepStrong, bool stretchedOk) const
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
    bool keptStrong = false; // an airframe the pool could pay but that would leave STRONG
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
        // the director's own strike (7.3, 2026-10-16): STRONG must hold after
        // the ground operation's debit (BandNow) and this price
        // (the escalation strike, stretchedOk: STRETCHED must hold)
        const int after = StepBand(BandNow(), _pool - q.poolPrice, _poolStart);
        if (keepStrong && Active() && q.affordable &&
            !(after == OFBandStrong || (stretchedOk && after == OFBandStretched)))
        {
            q.affordable = false;
            keptStrong = true;
        }
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
        a.choice.why = n == 0       ? "no strike airframe offered"
                       : keptStrong ? (stretchedOk ? StrikeKeepsStretchedWhy : StrikeKeepsStrongWhy)
                                    : "the pool cannot pay a strike";
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
    // the escalation strike (7.3): from the second repel, while STRETCHED holds
    const bool due = StrikeDue(registry, view, target, launchedKind);
    const bool escalation = !due && EscalationStrikeDue(registry, view, target);
    if (!due && !escalation)
    {
        return;
    }
    const DirectorAction a = PlanStrike(registry, view, target, true, true, escalation);
    if (!a.go)
    {
        // the band rule is a director decision worth a line (7.3); the rest
        // (nothing offered, nothing payable) stays at DEBUG
        if (strcmp((const char*)a.choice.why, StrikeKeepsStrongWhy) == 0 ||
            strcmp((const char*)a.choice.why, StrikeKeepsStretchedWhy) == 0)
        {
            GM_DIRECTOR_INFO(Core, "OccupierForces: no strike with the operation on {}: {} (pool {} of {})",
                             target == HqCandidate ? "the HQ" : ZoneNameOf(registry, target),
                             (const char*)a.choice.why, _pool, _poolStart);
        }
        else
        {
            LOG_DEBUG(Core, "OccupierForces: no strike with the operation: {}", (const char*)a.choice.why);
        }
        return;
    }
    // no extra token: the strike goes with the operation just launched
    if (LaunchAction(registry, view, launcher, a, false, false, 0) && escalation)
    {
        GM_DIRECTOR_INFO(
            Core,
            "OccupierForces: [op {}] escalation strike: {} repelled {} time(s), the pool stays {} (pool {} "
            "of {})",
            _opCounter, ZoneNameOf(registry, target), Repels(ZoneNameOf(registry, target)), BandName(BandNow()), _pool,
            _poolStart);
    }
}

bool OccupierForces::EscalationStrikeDue(const ZoneRegistry& registry, const DirectorView& view, int target) const
{
    if (!Active() || target == HqCandidate || _tuning.approachRepels <= 0)
    {
        return false;
    }
    const int band = BandNow();
    if (band != OFBandStrong && band != OFBandStretched)
    {
        return false;
    }
    if (!_catalogue.Offered(FPJet) && !_catalogue.Offered(FPAttackHeli))
    {
        return false;
    }
    const ZoneRecord* z = registry.GetZone(target);
    if (!z || Repels(z->name) < _tuning.approachRepels)
    {
        return false;
    }
    const bool base = registry.IsHeldBase(*z);
    if (!base && !IsHeldTown(registry, *z))
    {
        return false;
    }
    if (KnownFighters(registry, target) < (base ? StrikeOpenerMinFighters : _tuning.strikeMinFighters))
    {
        return false;
    }
    return !registry.IsStruck(target, view.now);
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
        GM_DIRECTOR_INFO(Core,
                 "OccupierForces: [op {}] Struck {}: support {:.0f}, standing {:.0f} (-{:.0f} each), heat +{:.0f}, "
                 "half income until {:.0f}; {} town(s) within {:.0f} m -{:.0f} each",
                 opId, (const char*)z->name, (double)z->support, (double)z->standing, (double)ZoneRegistry::StrikeCut,
                 (double)ZoneRegistry::StrikeHeat, (double)z->struckUntil, ring,
                 (double)ZoneRegistry::StrikeRingRadius, (double)ZoneRegistry::StrikeRingCut);
    }
    else
    {
        GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] Struck {}: {} town(s) within {:.0f} m -{:.0f} support and standing",
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
    req.minBodies = _tuning.alarmMin; // forcesMinimums (ForcePackages::AlarmMinBodies at Normal)
    req.urgent = a.urgent;
    req.urgentCostShare = _tuning.urgentCostShare;
    req.tierFactor = view.tierFactor;
    req.pool = _pool;
    req.poolActive = Active();
    req.offered = OfferedPackages;
    req.roadFactor = ForcePackages::RoadFactor(); // the road quote (6.5): a ground leg with no road route known
    // the arrival rule (6.5) answers alarms too, except THIN (the cheapest)
    if (a.urgent)
    {
        req.slowEta = _tuning.slowEta;
        req.slowCostShare = _tuning.slowCostShare;
        req.airEta = _tuning.airEta;
    }
    a.minAsked = req.minBodies;
    a.minUsed = req.minBodies;
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
            GM_DIRECTOR_INFO(Core, "OccupierForces: Starved: no answer to {} alarm ({})", (const char*)z->name, why);
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
        GM_DIRECTOR_INFO(Core, "OccupierForces: forced strike refused: {}", (const char*)a.choice.why);
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
        GM_DIRECTOR_INFO(Core, "OccupierForces: forced alarm answer refused: {}", (const char*)a.choice.why);
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
    GM_DIRECTOR_INFO(Core, "OccupierForces: package catalogue for {} ({}, {} ladder class(es)); offers foot{}{}{}{}{}{}{}",
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

void OccupierForces::DeriveFromExistingSave(ZoneRegistry& registry, const SettlementProbe* probe)
{
    if (!_tuning.enabled || !registry.IsActive() || _active)
    {
        return;
    }
    // K by the authored owners (M7): the towns the player already holds stay
    // in K, and control measures them lost
    if (!ActivateProbed(registry, probe, true))
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
    FillTownsEverFromOwners(registry);
    _sightings.Clear();
    _tokens = 0; // a migrated save does not launch at once
    // invariant W from the derived set: awake with no tokens when enough
    // distinct towns are already held
    _posture = _townsEver.Size() >= _tuning.wakeTowns ? OFAwake : OFDormant;
    _legacySave = true; // D20: the old save keeps its own script QRF
    _band = InitialBand(_pool, _poolStart);
    GM_DIRECTOR_INFO(Core,
             "OccupierForces: derived from a save without the block: pool {} of {} (control {:.2f}), {} wave "
             "threshold(s) marked fired, {} town(s) taken, band {}",
             _pool, _poolStart, (double)_control, _wavesFired, _townsEver.Size(), BandName(_band));
}

void OccupierForces::FillTownsEverFromOwners(const ZoneRegistry& registry)
{
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
}

void OccupierForces::SeedTownsEverOnLoad(const ZoneRegistry& registry)
{
    if (!_townsEverPending)
    {
        return;
    }
    _townsEverPending = false;
    if (!registry.IsActive())
    {
        return;
    }
    FillTownsEverFromOwners(registry);
    // invariant W from the seeded set; tokens stay as saved (a phase-2 block
    // holds none)
    if (_posture == OFDormant && _townsEver.Size() >= _tuning.wakeTowns)
    {
        _posture = OFAwake;
    }
    GM_DIRECTOR_INFO(Core, "OccupierForces: the block predates the distinct-town set: {} town(s) seeded from the owners, {}",
             _townsEver.Size(), _posture == OFAwake ? "AWAKE" : "DORMANT");
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
    PARAM_CHECK(ar.Serialize("lever", lever, 1, false))
    // absent in a save before the crew ledger: false, reconciled on load
    PARAM_CHECK(ar.Serialize("crewLedger", crewLedger, 1, false))
    // absent before 2026-10-16: no airfield known (the hull is stored at the
    // operation's source)
    PARAM_CHECK(ar.Serialize("airOrigin", airOrigin, 1, -1))
    return LSOK;
}

LSError OccupierForces::RepelRow::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("zone", zone, 1, RString()))
    PARAM_CHECK(ar.Serialize("count", count, 1, 0))
    PARAM_CHECK(ar.Serialize("package", package, 1, (int)FPFoot))
    PARAM_CHECK(ar.Serialize("source", source, 1, RString()))
    PARAM_CHECK(ar.Serialize("contained", contained, 1, false))
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
        // L8: the zone owners are restored by now (the GuerrillaZones block
        // precedes this one)
        if (this == &Instance())
        {
            SeedTownsEverOnLoad(ZoneRegistry::Instance());
        }
        // CounterAttack's block precedes this one: its records, hulls and
        // airframes driving home are restored by now
        ReconcileLegacyOpRows();
        return LSOK;
    }
    PARAM_CHECK(ar.Serialize("active", _active, 1, false))
    PARAM_CHECK(ar.Serialize("pool", _pool, 1, 0))
    PARAM_CHECK(ar.Serialize("poolStart", _poolStart, 1, 0))
    PARAM_CHECK(ar.Serialize("R", _R, 1, 0))
    int k = _towns.Size(); // informational: K is the Towns row count
    PARAM_CHECK(ar.Serialize("K", k, 1, 0))
    PARAM_CHECK(ar.Serialize("G", _G, 1, 0))
    // the generated garrisons in R (5.7; absent in older blocks: none)
    PARAM_CHECK(ar.Serialize("Rgen", _Rgen, 1, 0))
    // empty arrays write no subclass and load back empty
    PARAM_CHECK(ar.Serialize("Roster", _roster, 1))
    PARAM_CHECK(ar.Serialize("Towns", _towns, 1))
    PARAM_CHECK(ar.Serialize("Excluded", _excluded, 1))
    // the settlement probe (5.1): absent in older blocks = none probed out
    PARAM_CHECK(ar.Serialize("probedOut", _probedOut, 1, 0))
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
    // the re-armed zones (6.3, 2026-10-16; absent in older blocks: none)
    PARAM_CHECK(ar.Serialize("Rearm", _rearm, 1))
    // the repel ladder (6.3, 2026-10-16; absent in older blocks: none)
    PARAM_CHECK(ar.Serialize("Repels", _repels, 1))
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
    // the ceiling (5.5, 2026-10-16).  A block without it loads the headroom
    // as whatever the saved pool already holds above poolStart (-1 marks the
    // key absent), so an older save with a landed wave keeps it
    int headroom = _waveHeadroom;
    PARAM_CHECK(ar.Serialize("waveHeadroom", headroom, 1, -1))
    PARAM_CHECK(ar.Serialize("dropped", _dropped, 1, 0))
    // the ledger of 2026-10-16 (absent in older blocks: none): stored spare
    // hulls, the wave surge, the units stood down, the loss window
    PARAM_CHECK(ar.Serialize("StoredHulls", _storedHulls, 1))
    PARAM_CHECK(ar.Serialize("Surge", _surge, 1))
    PARAM_CHECK(ar.Serialize("stoodDown", _stoodDown, 1, 0))
    PARAM_CHECK(ar.Serialize("surgeWindow", _surgeWindow, 1, 0.0f))
    float anchorX = _surgeAnchor.X();
    float anchorZ = _surgeAnchor.Z();
    PARAM_CHECK(ar.Serialize("surgeAnchorX", anchorX, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("surgeAnchorZ", anchorZ, 1, 0.0f))
    if (ar.IsLoading())
    {
        _surgeAnchor = Vector3(anchorX, 0, anchorZ);
    }
    PARAM_CHECK(ar.Serialize("Losses", _losses, 1))
    if (ar.IsLoading())
    {
        _waveHeadroom = headroom >= 0 ? headroom : MaxInt(_pool - _poolStart, 0);
    }
    int format = SaveFormat;
    PARAM_CHECK(ar.Serialize("format", format, 1, 0))
    if (ar.IsLoading())
    {
        _loadedFormat = format;
        // L8: a phase-2 block carries the field but never filled it
        _townsEverPending = format < SaveFormat && _active && _townsEver.Size() == 0;
        // D20: a block saved before the native alarm answers existed keeps
        // the campaign's own script QRF (its text rides in the save) and no
        // native answers run beside it
        if (format < SaveFormat && _active && !_legacySave)
        {
            _legacySave = true;
            GM_DIRECTOR_INFO(Core,
                     "OccupierForces: block format {} predates the native alarm answers: legacySave, the script QRF "
                     "stays",
                     format);
        }
        _alarmStarved.Clear();
        _massTarget = RString(); // massing is transient: re-weighed next tick
        _massTicks = 0;
        _massTokens = 0;
        _controlBand = -1;
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
