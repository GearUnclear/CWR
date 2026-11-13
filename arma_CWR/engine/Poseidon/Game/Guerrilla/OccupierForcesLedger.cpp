// Guerrilla Mode occupier forces: the ledger (OCCUPIER-FORCES.md 5.2 to 5.6
// and 7.1, OccupierForces.hpp).  Split out of OccupierForces.cpp
// (2026-10-16, FileSize): the pool seams (Draw, Deposit, Spend, Refund), the
// ceiling and its banking, the band's step and its input, the refill, the
// wave landing, and the three ledger rules of 2026-10-16 (decisions 5, 6, 9
// and the feel assessment's item 10):
//   * THE WAVE SURGE: while a landed wave is on, the excess over the ceiling
//     is a surge at the bases nearest the wave's cluster, above garrisonMax,
//     that decays back over surgeDecayHours (the bodies rejoin the pool up to
//     the ceiling, the rest stand down);
//   * COMMITTED AIRFRAMES: an airframe out counts toward the band's strength
//     until it is home or lost (CommittedAir, from the ledger rows);
//   * THE LOSS WINDOW: losses booked in the last lossWindowHours and not yet
//     repaid by the refill take the band's strength down;
//   * THE HULL FLAG: with forcesBankHulls 0 a hull refund over the ceiling
//     is a stored spare hull at its base, not a garrison of bodies.

#include <Poseidon/Game/Guerrilla/OccupierForces.hpp>
#include <Poseidon/Game/Guerrilla/DirectorLog.hpp>

#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp>   // IsMilitaryType, IsMinorPostType
#include <Poseidon/Game/Guerrilla/CounterAttack.hpp> // Listener (SpendOnOp)
#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp> // the pending deposit, live garrison bodies
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <stdio.h>
#include <string.h>

namespace Poseidon::Guerrilla
{

static int LMin(int a, int b)
{
    return a < b ? a : b;
}

static int LMax(int a, int b)
{
    return a > b ? a : b;
}

static bool IsAirframe(int package)
{
    return package == FPTransportHeli || package == FPAttackHeli || package == FPJet;
}

// an occupier-held military zone (every room of 5.5 is one of these)
static bool OccupierBase(const ZoneRegistry& registry, const ZoneRecord* z)
{
    return z && CampaignEnd::IsMilitaryType(z->type) && stricmp(z->owner, registry.OccupierSide()) == 0;
}

static float Dist2DSq(Vector3Par a, Vector3Par b)
{
    const float dx = a.X() - b.X();
    const float dz = a.Z() - b.Z();
    return dx * dx + dz * dz;
}

// ---------------------------------------------------------------------------
// the band and its input (5.6)
// ---------------------------------------------------------------------------

int OccupierForces::BandStrengthOf(int pool, int poolStart, int committed, int debt)
{
    const int strength = pool + LMax(committed, 0);
    const int cap = LMax(poolStart, pool) - LMax(debt, 0);
    const int v = LMin(strength, cap);
    return v > 0 ? v : 0;
}

int OccupierForces::CommittedAir() const
{
    int units = 0;
    for (int i = 0; i < _ops.Size(); i++)
    {
        if (IsAirframe(_ops[i].package) && _ops[i].outstanding > 0)
        {
            units += _ops[i].outstanding;
        }
    }
    return units;
}

int OccupierForces::LossesInWindow() const
{
    const float window = _tuning.lossWindowHours * 3600.0f;
    const float now = Now();
    int units = 0;
    for (int i = 0; i < _losses.Size(); i++)
    {
        if (now - _losses[i].stamp <= window)
        {
            units += _losses[i].units;
        }
    }
    return units;
}

int OccupierForces::RepaidInWindow() const
{
    const float window = _tuning.lossWindowHours * 3600.0f;
    const float now = Now();
    int units = 0;
    for (int i = 0; i < _losses.Size(); i++)
    {
        if (now - _losses[i].stamp <= window)
        {
            units += _losses[i].repaid;
        }
    }
    return units;
}

void OccupierForces::RepayLosses(int units)
{
    const float window = _tuning.lossWindowHours * 3600.0f;
    const float now = Now();
    for (int i = 0; i < _losses.Size() && units > 0; i++)
    {
        LossRow& l = _losses[i];
        if (now - l.stamp > window)
        {
            continue;
        }
        const int pay = LMin(units, l.units - l.repaid);
        if (pay > 0)
        {
            l.repaid += pay;
            units -= pay;
        }
    }
}

void OccupierForces::UpdateBand()
{
    if (_bandHold > 0)
    {
        _bandDirty = true; // stepped once when the last hold closes
        return;
    }
    const int strength = BandStrength();
    const int next = StepBand(_band, strength, _poolStart);
    if (next != _band)
    {
        GM_DIRECTOR_INFO(Core,
                         "OccupierForces: band {} -> {} (pool {} of {}; strength {}: {} committed in the air, {} lost "
                         "and {} repaid in the last {:.1f} h)",
                         BandName(_band), BandName(next), _pool, _poolStart, strength, CommittedAir(),
                         LossesInWindow(), RepaidInWindow(), (double)_tuning.lossWindowHours);
        _band = next;
    }
}

void OccupierForces::ReleaseBand()
{
    if (_bandHold <= 0)
    {
        return;
    }
    _bandHold--;
    if (_bandHold == 0 && _bandDirty)
    {
        _bandDirty = false;
        // the whole movement at once: one band step on the net pool
        if (Active())
        {
            UpdateBand();
        }
    }
}

void OccupierForces::StepBandFrame()
{
    // close the previous frame's hold: one step on what every service booked
    // since (5.6), then hold the band for this frame and the services after
    // OccupierForces in it
    if (_frameHold)
    {
        _frameHold = false;
        ReleaseBand();
    }
    HoldBand();
    _frameHold = true;
}

void OccupierForces::SettleBand()
{
    const int explicitHolds = _bandHold - (_frameHold ? 1 : 0);
    if (explicitHolds > 0 || _bandHold <= 0)
    {
        UpdateBand(); // inside a BandHold: deferred; with no hold: at once
        return;
    }
    // only the frame hold is open: step through it now, on the strength the
    // frame has left so far (the frame's later movements step at its end)
    _bandHold = 0;
    _bandDirty = false;
    UpdateBand();
    _bandHold = 1;
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
    const int credit = LMin(whole, _poolStart - _pool);
    _pool += credit;
    _refillTotal += credit;
    // the refill repays the window's losses (5.6): the band reads what was
    // lost and not yet replaced
    RepayLosses(credit);
    if (_pool >= _poolStart)
    {
        _refillAccum = 0;
    }
    UpdateBand();
}

void OccupierForces::Tick(const ZoneRegistry& registry, float elapsed)
{
    TickCore(registry, nullptr, elapsed);
}

void OccupierForces::Tick(ZoneRegistry& registry, float elapsed)
{
    TickCore(registry, &registry, elapsed);
}

void OccupierForces::TickCore(const ZoneRegistry& registry, ZoneRegistry* writable, float elapsed)
{
    if (!Active())
    {
        return;
    }
    _logClock += elapsed;
    _clock += elapsed;
    // the ledger first: stored hulls and the decaying surge come back
    // before the refill tops the pool up
    StepLedger(registry, writable, elapsed);
    Refill(elapsed);
    _control = ComputeControl(registry);
    // control crossing a wave threshold (either way) lifts every containment
    const int controlBand = WavesCrossed(_tuning, _control);
    if (_controlBand >= 0 && controlBand != _controlBand)
    {
        char why[80];
        snprintf(why, sizeof(why), "control %.2f crossed a wave threshold (band %d -> %d)", (double)_control,
                 _controlBand, controlBand);
        LiftContainment(why);
    }
    _controlBand = controlBand;
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

// ---------------------------------------------------------------------------
// the pool seams (5.3)
// ---------------------------------------------------------------------------

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
    const int granted = LMin(bodies, _pool);
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
            GM_DIRECTOR_INFO(Core, "OccupierForces: SPENT, no deposit of {} ({}){}", bodies, why ? why : "",
                     _logSuppressed > 0 ? " (other lines suppressed)" : "");
            _logSuppressed = 0;
        }
        return 0;
    }
    const int granted = LMin(bodies, _pool);
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
            GM_DIRECTOR_INFO(Core, "OccupierForces: refused {} unit(s) for {} (pool {} of {}, band {}){}", units,
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

bool OccupierForces::SpendOnOp(int opId, int units, int package, const char* why, bool& booked)
{
    booked = false;
    // the debit and the airframe's commitment are one movement: the band
    // never sees the pool short of the airframe it has not committed yet
    BandHold hold(*this);
    if (!Spend(units, why))
    {
        return false;
    }
    const int row = FindOp(opId);
    // only a row whose events reach this ledger: a unit test's listener
    // would leave the units outstanding for good
    if (!Active() || units <= 0 || row < 0 || CounterAttack::Listener() != this)
    {
        return true;
    }
    _ops[row].outstanding += units;
    _ops[row].package = package;
    booked = true;
    GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] {}: {} unit(s) on the ledger row ({} outstanding)", opId,
             why ? why : "", units, _ops[row].outstanding);
    // the airframe is committed from here (CommittedAir), not spent
    UpdateBand();
    return true;
}

// ---------------------------------------------------------------------------
// THE RESERVE FLOOR (OCCUPIER-FORCES.md 6.3, owner decision 2026-10-17).
// Video 7 op 4: one APC and T72 column (the pool paid 28) took a pool of 32
// to 4, under forcesPoolFloor x poolStart (9.6 on Abel), and the occupier
// could answer nothing for most of an hour.  A commander keeps a reserve:
// a director-bought ground operation may not take the pool below the floor
// by its own debit.  It waits instead, and is never sent smaller than its
// plan required.
// ---------------------------------------------------------------------------

int OccupierForces::ReserveRoomOf(int pool, int poolStart, float floor)
{
    for (int p = pool; p > 0; p--)
    {
        const int left = pool - p;
        // the shares at the tuning's precision, as CanOperateIn compares them
        const bool below =
            poolStart > 0 ? (float)left / (float)poolStart < floor : (float)left < floor * (float)poolStart;
        if (!below)
        {
            return p;
        }
    }
    return 0;
}

bool OccupierForces::ReserveBindsKind(int kind, int origin, bool baseTarget, float floor)
{
    if (!(floor > 0.0f) || origin == CAOWave || origin == CAOLegacy)
    {
        return false; // the rule off, a wave's operations (paid from the wave), the legacy dispatch
    }
    switch (kind)
    {
        case CAKRetake:
        case CAKBase:
            return origin == CAODirector || origin == CAOPushout;
        case CAKAlarm:
            return !baseTarget; // an answer to a base under attack may dig into the reserve
        default:
            return false; // HQ assaults, strikes (their own band rules), reinforce columns
    }
}

bool OccupierForces::ReserveBinds(const ZoneRegistry& registry, const DirectorAction& a) const
{
    if (!Active())
    {
        return false;
    }
    const ZoneRecord* z = registry.GetZone(a.kind == CAKAlarm && a.alarmZone >= 0 ? a.alarmZone : a.target);
    return ReserveBindsKind(a.kind, a.origin, z && CampaignEnd::IsMilitaryType(z->type), _tuning.poolFloor);
}

int OccupierForces::FindReserveWait(const char* zone) const
{
    for (int i = 0; zone && i < _reserveWaits.Size(); i++)
    {
        if (stricmp(_reserveWaits[i].zone, zone) == 0)
        {
            return i;
        }
    }
    return -1;
}

void OccupierForces::ClearReserveWait(const char* zone)
{
    const int i = FindReserveWait(zone);
    if (i >= 0)
    {
        _reserveWaits.Delete(i);
    }
}

namespace
{
// "APC (6, BMP) with T72", "FOOT (4)"
RString ReservePlanWords(const PackageQuote& q)
{
    char buf[160];
    char escort[80] = "";
    if (q.HasEscort() && q.escortClass.GetLength() > 0)
    {
        snprintf(escort, sizeof(escort), " with %s", (const char*)q.escortClass);
    }
    if (q.hullClass.GetLength() > 0 && q.package != FPFoot)
    {
        snprintf(buf, sizeof(buf), "%s (%d, %s)%s", ForcePackages::PackageName(q.package), q.bodies,
                 (const char*)q.hullClass, escort);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%s (%d)%s", ForcePackages::PackageName(q.package), q.bodies, escort);
    }
    return RString(buf);
}
} // namespace

void OccupierForces::NoteReserveWait(const ZoneRegistry& registry, const DirectorAction& a, bool best)
{
    const ZoneRecord* z = registry.GetZone(a.kind == CAKAlarm && a.alarmZone >= 0 ? a.alarmZone : a.target);
    if (!z)
    {
        return;
    }
    const PackageQuote& q = a.choice.quote;
    const char* qualifier = a.kind == CAKAlarm             ? ""
                            : a.escalation >= 1            ? "escalation "
                            : HasName(_rearm, z->name)     ? "re-arm "
                            : a.origin == CAOPushout       ? "push-out "
                                                           : "";
    const char* kind = a.kind == CAKAlarm ? "alarm answer" : (a.kind == CAKBase ? "base retake" : "retake");
    char what[96];
    snprintf(what, sizeof(what), "%s%s%s", qualifier, kind, best ? " after massing" : "");
    const RString plan = ReservePlanWords(q);
    const int i = FindReserveWait(z->name);
    if (i >= 0 && _reserveWaits[i].price == q.poolPrice && _reserveWaits[i].package == q.package &&
        _reserveWaits[i].best == best && stricmp(_reserveWaits[i].plan, plan) == 0 &&
        stricmp(_reserveWaits[i].what, what) == 0)
    {
        return; // the same plan waits for the same reason: logged already
    }
    char left[64];
    if (q.poolPrice <= _pool)
    {
        snprintf(left, sizeof(left), "the launch would leave %d", _pool - q.poolPrice);
    }
    else
    {
        snprintf(left, sizeof(left), "the pool is %d short of the price", q.poolPrice - _pool);
    }
    char line[512];
    snprintf(line, sizeof(line),
             "OccupierForces: %s: %s (%s, the pool pays %d, lethality %.2f of %.2f%s) waits, the pool keeps its "
             "reserve (pool %d of %d, floor %.1f: %s)",
             (const char*)z->name, what, (const char*)plan, q.poolPrice, (double)q.lethality,
             (double)a.choice.lReq, a.choice.shortOfLethality ? ", the most lethal package" : "", _pool, _poolStart,
             (double)ReserveFloorUnits(), left);
    GM_DIRECTOR_INFO(Core, "{}", line);
    ReserveWaitRow row;
    row.zone = z->name;
    row.plan = plan;
    row.what = what;
    row.line = line;
    row.package = q.package;
    row.price = q.poolPrice;
    row.pool = _pool;
    row.best = best;
    if (i >= 0)
    {
        _reserveWaits[i] = row;
    }
    else
    {
        _reserveWaits.Add(row);
    }
}

bool OccupierForces::AcceptReservePlan(const ZoneRegistry& registry, DirectorAction& a, const DirectorAction& capped,
                                       bool best)
{
    const PackageQuote& c = capped.choice.quote;
    if (capped.go && !capped.choice.shortOfLethality && KeepsReserve(c.poolPrice))
    {
        // the plan's own cheapest package that still meets it fits: it goes
        const PackageQuote& q = a.choice.quote;
        char note[320];
        snprintf(note, sizeof(note),
                 "%s (the pool pays %d) would leave the pool under its reserve (pool %d of %d, floor %.1f); %s, "
                 "which meets the plan, goes (the pool pays %d)",
                 (const char*)ReservePlanWords(q), q.poolPrice, _pool, _poolStart, (double)ReserveFloorUnits(),
                 (const char*)ReservePlanWords(c), c.poolPrice);
        DirectorAction keep = capped;
        keep.reserveNote = note;
        a = keep;
        return true;
    }
    NoteReserveWait(registry, a, best);
    return false;
}

// ---------------------------------------------------------------------------
// the ceiling (5.5): banking, the wave surge, stored spare hulls
// ---------------------------------------------------------------------------

ZoneRegistry* OccupierForces::BankRegistry() const
{
    if (_bankRegistry)
    {
        return _bankRegistry;
    }
    return this == &Instance() ? &ZoneRegistry::Instance() : nullptr;
}

int OccupierForces::BankRoom(const ZoneRegistry& registry, int zone, int live)
{
    const ZoneRecord* z = registry.GetZone(zone);
    if (!OccupierBase(registry, z))
    {
        return 0;
    }
    // whole bodies (U3); the pending deposit counts against the room, as
    // every room in 5.5 does
    const float room = z->garrisonMax - (float)live - floorf(z->garrison > 0 ? z->garrison : 0) - (float)z->pending;
    return room > 0 ? (int)floorf(room) : 0;
}

int OccupierForces::SurgeRoom(const ZoneRegistry& registry, int zone, int live) const
{
    const ZoneRecord* z = registry.GetZone(zone);
    if (!OccupierBase(registry, z))
    {
        return 0;
    }
    const float share = _tuning.surgeShare > 0 ? _tuning.surgeShare : 0.0f;
    const float cap = floorf(z->garrisonMax * (1.0f + share));
    const float room = cap - (float)live - floorf(z->garrison > 0 ? z->garrison : 0) - (float)z->pending;
    return room > 0 ? (int)floorf(room) : 0;
}

int OccupierForces::BankExcess(ZoneRegistry& registry, int units, const char* why, int sourceZone,
                               const Vector3* near)
{
    const int n = registry.NZones();
    if (units <= 0 || n <= 0)
    {
        return 0;
    }
    // the live garrisons are the engine cache's, indexed on the engine-wide
    // registry; a unit test's registry has none
    const bool engineLive = &registry == &ZoneRegistry::Instance();
    AutoArray<int> room;
    room.Resize(n);
    for (int i = 0; i < n; i++)
    {
        room[i] = BankRoom(registry, i, engineLive ? GarrisonCache::Instance().LiveCount(i) : 0);
    }
    // the reference position: the caller's, else the source base's own
    const ZoneRecord* src = registry.GetZone(sourceZone);
    const bool hasPos = near != nullptr || src != nullptr;
    const Vector3 at = near ? *near : (src ? src->pos : VZero);
    int left = units;
    while (left > 0)
    {
        // the source base first, else the nearest base with room, else the
        // one with the most room (ties: the zone order)
        const bool fromSource = sourceZone >= 0 && sourceZone < n && room[sourceZone] > 0;
        int pick = fromSource ? sourceZone : -1;
        float bestD2 = 0;
        for (int i = 0; i < n && !fromSource; i++)
        {
            if (room[i] <= 0)
            {
                continue;
            }
            const float d2 = Dist2DSq(registry.GetZone(i)->pos, at);
            const bool better = pick < 0 || (hasPos ? d2 < bestD2 : room[i] > room[pick]);
            if (better)
            {
                pick = i;
                bestD2 = d2;
            }
        }
        if (pick < 0)
        {
            break; // every occupier base at garrisonMax
        }
        const int take = LMin(left, room[pick]);
        // pending, never straight into the reserve: the bodies join it once
        // the player is away or every spawn point is hidden (S5, 3.3)
        GarrisonCache::Deposit(registry, pick, take, src ? sourceZone : pick);
        room[pick] = 0;
        left -= take;
        _banked += take;
        const ZoneRecord* b = registry.GetZone(pick);
        _lastRefund.bankedAt = _lastRefund.bankedAt.GetLength() > 0
                                   ? RString(_lastRefund.bankedAt + RString(", ") + b->name)
                                   : RString(b->name);
        GM_DIRECTOR_INFO(Core,
                         "OccupierForces: ledger: {} unit(s) over poolStart banked at {} ({}; pool {} of {}; reserve {}, "
                         "pending {}, of {})",
                         take, (const char*)b->name, why ? why : "", _pool, _poolStart, b->garrison, b->pending,
                         b->garrisonMax);
    }
    return units - left;
}

bool OccupierForces::SurgeOpen() const
{
    if (_surgeWindow > 0)
    {
        return true;
    }
    // a wave operation still out: its survivors come home mid-fight
    for (int i = 0; i < _ops.Size(); i++)
    {
        if (_ops[i].origin == CAOWave && !_ops[i].finished)
        {
            return true;
        }
    }
    return false;
}

int OccupierForces::SurgeTotal() const
{
    int n = 0;
    for (int i = 0; i < _surge.Size(); i++)
    {
        n += _surge[i].bodies;
    }
    return n;
}

int OccupierForces::SurgeExcess(ZoneRegistry& registry, int units, const char* why, int sourceZone,
                                const Vector3* near)
{
    const int n = registry.NZones();
    if (units <= 0 || n <= 0)
    {
        return 0;
    }
    const bool engineLive = &registry == &ZoneRegistry::Instance();
    AutoArray<int> room;
    room.Resize(n);
    for (int i = 0; i < n; i++)
    {
        room[i] = SurgeRoom(registry, i, engineLive ? GarrisonCache::Instance().LiveCount(i) : 0);
    }
    // the reference: the wave's cluster (the anchor), else the caller's
    // position, else the source base's
    const ZoneRecord* src = registry.GetZone(sourceZone);
    const bool anchored = _surgeAnchor.X() != 0 || _surgeAnchor.Z() != 0;
    const bool hasPos = anchored || near != nullptr || src != nullptr;
    const Vector3 at = anchored ? _surgeAnchor : (near ? *near : (src ? src->pos : VZero));
    const float hours = _tuning.surgeDecayHours > 0.001f ? _tuning.surgeDecayHours : 0.001f;
    int left = units;
    while (left > 0)
    {
        int pick = -1;
        float bestD2 = 0;
        for (int i = 0; i < n; i++)
        {
            if (room[i] <= 0)
            {
                continue;
            }
            const float d2 = Dist2DSq(registry.GetZone(i)->pos, at);
            const bool better = pick < 0 || (hasPos ? d2 < bestD2 : room[i] > room[pick]);
            if (better)
            {
                pick = i;
                bestD2 = d2;
            }
        }
        if (pick < 0)
        {
            break; // every base at its surge cap
        }
        const int take = LMin(left, room[pick]);
        GarrisonCache::Deposit(registry, pick, take, src ? sourceZone : pick);
        room[pick] = 0;
        left -= take;
        const ZoneRecord* b = registry.GetZone(pick);
        int row = -1;
        for (int i = 0; i < _surge.Size() && row < 0; i++)
        {
            if (stricmp(_surge[i].zone, b->name) == 0)
            {
                row = i;
            }
        }
        if (row < 0)
        {
            SurgeRow r;
            r.zone = b->name;
            row = _surge.Add(r);
        }
        _surge[row].bodies += take;
        // back at garrisonMax about surgeDecayHours after the last credit
        _surge[row].peak = _surge[row].bodies;
        GM_DIRECTOR_INFO(Core,
                         "OccupierForces: surge: {} unit(s) over poolStart at {}, above its garrisonMax ({}; pool {} of "
                         "{}; reserve {}, pending {}, of {}; {} surge there, back to garrisonMax in {:.1f} h)",
                         take, (const char*)b->name, why ? why : "", _pool, _poolStart, b->garrison, b->pending,
                         b->garrisonMax, _surge[row].bodies, (double)hours);
    }
    return units - left;
}

int OccupierForces::StoredHullTotal() const
{
    int n = 0;
    for (int i = 0; i < _storedHulls.Size(); i++)
    {
        n += _storedHulls[i].units;
    }
    return n;
}

bool OccupierForces::CanStoreHull(const ZoneRegistry& registry, int zone) const
{
    const ZoneRecord* z = registry.GetZone(zone);
    if (!OccupierBase(registry, z) || CampaignEnd::IsMinorPostType(z->type))
    {
        return false;
    }
    int stored = 0;
    for (int i = 0; i < _storedHulls.Size(); i++)
    {
        if (stricmp(_storedHulls[i].zone, z->name) == 0)
        {
            stored++;
        }
    }
    return stored < StoredHullsPerBase;
}

int OccupierForces::StoreHull(const ZoneRegistry& registry, int units, int package, const char* what, int home,
                              const Vector3* near)
{
    if (units <= 0)
    {
        return 0;
    }
    // the base it returns to, else the nearest base with a garage (a base
    // that is not a minor post) to where it is, else to its home
    int pick = CanStoreHull(registry, home) ? home : -1;
    if (pick < 0)
    {
        const ZoneRecord* h = registry.GetZone(home);
        const bool hasPos = near != nullptr || h != nullptr;
        const Vector3 at = near ? *near : (h ? h->pos : VZero);
        float bestD2 = 0;
        for (int i = 0; i < registry.NZones(); i++)
        {
            if (!CanStoreHull(registry, i))
            {
                continue;
            }
            const float d2 = Dist2DSq(registry.GetZone(i)->pos, at);
            if (pick < 0 || (hasPos && d2 < bestD2))
            {
                pick = i;
                bestD2 = d2;
            }
        }
    }
    if (pick < 0)
    {
        return 0;
    }
    StoredHullRow row;
    row.zone = registry.GetZone(pick)->name;
    row.package = package;
    row.what = what ? what : ForcePackages::PackageName(package);
    row.units = units;
    _storedHulls.Add(row);
    GM_DIRECTOR_INFO(Core,
                     "OccupierForces: ledger: {} over poolStart stored as a spare hull at {} ({} unit(s); pool {} of {}; "
                     "back in the pool when it has room)",
                     (const char*)row.what, (const char*)row.zone, units, _pool, _poolStart);
    return units;
}

int OccupierForces::Refund(int units, const char* why, int sourceZone, const Vector3* near)
{
    return RefundInto(BankRegistry(), units, why, sourceZone, near);
}

int OccupierForces::Refund(ZoneRegistry& registry, int units, const char* why, int sourceZone, const Vector3* near)
{
    return RefundInto(&registry, units, why, sourceZone, near);
}

int OccupierForces::CreditUnderCeiling(int units)
{
    // the ceiling (5.5): nothing but a wave lifts the pool above poolStart.
    // The wave headroom never exceeds the strength that can still come back
    // as wave strength: the pool's own overshoot, what the ledger rows still
    // hold and this refund (strength lost in the field shrinks it)
    if (_waveHeadroom > 0)
    {
        int held = LMax(_pool - _poolStart, 0) + units;
        for (int i = 0; i < _ops.Size(); i++)
        {
            held += LMax(_ops[i].outstanding, 0);
        }
        _waveHeadroom = LMin(_waveHeadroom, held);
    }
    const int room = LMax(PoolCeiling() - _pool, 0);
    const int credit = LMin(units, room);
    _pool += credit;
    return credit;
}

int OccupierForces::RefundInto(ZoneRegistry* registry, int units, const char* why, int sourceZone,
                               const Vector3* near)
{
    if (!Active() || units <= 0)
    {
        return 0;
    }
    _lastRefund = RefundTally();
    _lastRefund.valid = true;
    _lastRefund.why = why ? why : "";
    _lastRefund.units = units;
    // a refund over the ceiling is cut there.  While a landed wave is on the
    // excess is the wave's surge at the bases nearest its cluster (decision
    // 5, 7.1); otherwise, and for what the surge cannot take, it is banked
    // as garrison at the occupier bases with room (the owner, 2026-10-16),
    // and only what none can take is dropped, booked beside the losses so
    // the equation holds
    const int credit = CreditUnderCeiling(units);
    const int excess = units - credit;
    const int surged = excess > 0 && registry && SurgeOpen() ? SurgeExcess(*registry, excess, why, sourceZone, near) : 0;
    const int banked =
        excess - surged > 0 && registry ? BankExcess(*registry, excess - surged, why, sourceZone, near) : 0;
    const int dropped = excess - surged - banked;
    _lastRefund.pooled = credit;
    _lastRefund.banked = banked + surged;
    _lastRefund.dropped = dropped;
    if (dropped > 0)
    {
        _dropped += dropped;
        GM_DIRECTOR_INFO(Core,
                         "OccupierForces: ledger: {} unit(s) over poolStart dropped ({}; {}; pool {} of {})", dropped,
                         why ? why : "", registry ? "every occupier base at garrisonMax" : "no registry to bank into",
                         _pool, _poolStart);
    }
    LOG_DEBUG(Core, "OccupierForces: refund {} of {} ({}), pool {}", credit, units, why ? why : "", _pool);
    UpdateBand();
    return credit;
}

int OccupierForces::RefundHull(ZoneRegistry& registry, int units, int package, const char* what, int home,
                               const Vector3* near)
{
    if (!Active() || units <= 0)
    {
        return 0;
    }
    if (_tuning.bankHulls)
    {
        // forcesBankHulls 1: the df9457e rule, a hull banks like bodies
        return RefundInto(&registry, units, "hull home intact", home, near);
    }
    const int credit = CreditUnderCeiling(units);
    const int excess = units - credit;
    const char* name = what && *what ? what : ForcePackages::PackageName(package);
    const int stored = excess > 0 ? StoreHull(registry, excess, package, name, home, near) : 0;
    const int dropped = excess - stored;
    if (dropped > 0)
    {
        _dropped += dropped;
        GM_DIRECTOR_INFO(Core,
                         "OccupierForces: ledger: {} hull unit(s) over poolStart dropped ({}: no occupier base can store "
                         "a spare hull; pool {} of {})",
                         dropped, name, _pool, _poolStart);
    }
    LOG_DEBUG(Core, "OccupierForces: hull refund {} of {} ({}), pool {}", credit, units, name, _pool);
    UpdateBand();
    return credit;
}

void OccupierForces::NoteLoss(int units, const char* why)
{
    if (!Active() || units <= 0)
    {
        return;
    }
    _lost += units;
    // the band's loss window (5.6): two losses within LossMergeSeconds share
    // a row; past MaxLossRows the two oldest merge (the later stamp, so a
    // merged loss never leaves the window early)
    const float now = Now();
    if (_losses.Size() > 0 && now - _losses[_losses.Size() - 1].stamp <= LossMergeSeconds)
    {
        _losses[_losses.Size() - 1].units += units;
    }
    else
    {
        LossRow row;
        row.stamp = now;
        row.units = units;
        _losses.Add(row);
        if (_losses.Size() > MaxLossRows)
        {
            _losses[1].units += _losses[0].units;
            _losses[1].repaid += _losses[0].repaid;
            _losses.Delete(0);
        }
    }
    LOG_DEBUG(Core, "OccupierForces: lost {} ({}), {} in all, {} unrepaid in the window", units, why ? why : "",
              _lost, LossDebt());
    UpdateBand();
}

void OccupierForces::NoteTopUpSettled(const ZoneRegistry& registry, OpRow& row, const CounterAttackOpEvent& ev,
                                      const char* target)
{
    // one honest line (6.6, D12; video 6: "[op 11] Le Port Harbour retaken:
    // 0 stay as its garrison" for a REINFORCE on a base never lost, its 2
    // bodies gone to the pool above poolStart with no line saying so)
    const bool alarm = row.kind == CAKAlarm;
    const int joined = ev.bodies > 0 ? ev.bodies : 0;
    char rest[256] = "";
    if (row.returnHome >= 0)
    {
        const ZoneRecord* home = registry.GetZone(row.returnZone);
        int n = snprintf(rest, sizeof(rest), "; %d back at %s", row.returnHome,
                         home ? (const char*)home->name : "its source");
        if (row.returnPool > 0 && n > 0 && n < (int)sizeof(rest))
        {
            n += snprintf(rest + n, sizeof(rest) - n, ", %d to the pool", row.returnPooled);
            if (row.returnBanked > 0 && n < (int)sizeof(rest))
            {
                n += snprintf(rest + n, sizeof(rest) - n, ", %d banked", row.returnBanked);
            }
            if (row.returnDropped > 0 && n < (int)sizeof(rest))
            {
                snprintf(rest + n, sizeof(rest) - n, ", %d dropped (every base full)", row.returnDropped);
            }
        }
    }
    else if (_lastRefund.valid && _lastRefund.units > 0 &&
             (stricmp((const char*)_lastRefund.why, "alarm answer surplus") == 0 ||
              strncmp((const char*)_lastRefund.why, "top-up over the room", 20) == 0))
    {
        // the surplus an alarm answer leaves, or the stored bodies a live
        // top-up displaced: both went back to the pool just before the settle
        snprintf(rest, sizeof(rest), "; %d %s the pool", _lastRefund.units,
                 alarm ? "over its room to" : "stored bodies gave way to it, to");
    }
    // a pool over poolStart is the wave headroom (5.5): say so
    char over[96] = "";
    if (_pool > _poolStart)
    {
        snprintf(over, sizeof(over), ", %d over poolStart in the wave headroom", _pool - _poolStart);
    }
    GM_DIRECTOR_INFO(Core, "OccupierForces: [op {}] {} {}: {} joined its garrison{} (pool {} of {}{})", ev.opId, target,
                     alarm ? "alarm answer settled" : "reinforced", joined, rest, _pool, _poolStart, over);
    row.returnHome = -1;
    row.returnPool = 0;
    _lastRefund.valid = false;
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
    // what the wave put above poolStart may come back there after it is spent
    _waveHeadroom = LMax(_waveHeadroom, _pool - _poolStart);
    // the wave's surge is open from the landing (decision 5): what the
    // ceiling keeps out while it is on thickens the bases near its cluster
    _surgeWindow = WaveWindow;
    GM_DIRECTOR_INFO(Core, "OccupierForces: wave of {} landed, pool {} of {}, {} wave(s) landed", bodies, _pool, _poolStart,
             _wavesLanded);
    // the landing is a transaction by itself, and the allocation after it in
    // this tick spends the landed bodies: stepped now (a SPENT band would
    // refuse the hub top-up and every wave strike)
    SettleBand();
}

// ---------------------------------------------------------------------------
// the ledger's step: the loss window, stored hulls, the decaying surge
// ---------------------------------------------------------------------------

void OccupierForces::StepLedger(const ZoneRegistry& registry, ZoneRegistry* writable, float elapsed)
{
    if (_surgeWindow > 0)
    {
        _surgeWindow -= elapsed;
        if (_surgeWindow < 0)
        {
            _surgeWindow = 0;
        }
    }
    // the loss window ages
    const float window = _tuning.lossWindowHours * 3600.0f;
    const float now = Now();
    while (_losses.Size() > 0 && now - _losses[0].stamp > window)
    {
        _losses.Delete(0);
    }
    // stored spare hulls: lost with their base, back in the pool when it has
    // room under the ceiling
    for (int i = 0; i < _storedHulls.Size();)
    {
        StoredHullRow& h = _storedHulls[i];
        const ZoneRecord* z = registry.GetZone(registry.FindZoneIndex(h.zone));
        if (!OccupierBase(registry, z))
        {
            GM_DIRECTOR_INFO(Core, "OccupierForces: ledger: the spare {} stored at {} is lost with its base ({} unit(s))",
                             (const char*)h.what, (const char*)h.zone, h.units);
            const int lost = h.units;
            _storedHulls.Delete(i);
            NoteLoss(lost, "stored hull lost with its base");
            continue;
        }
        const int room = LMax(PoolCeiling() - _pool, 0);
        const int take = LMin(h.units, room);
        if (take > 0)
        {
            _pool += take;
            h.units -= take;
            GM_DIRECTOR_INFO(Core, "OccupierForces: ledger: the spare {} stored at {} rejoins the pool ({} unit(s); pool {} "
                             "of {})",
                             (const char*)h.what, (const char*)h.zone, take, _pool, _poolStart);
        }
        if (h.units <= 0)
        {
            _storedHulls.Delete(i);
            continue;
        }
        i++;
    }
    // the wave surge decays toward garrisonMax (needs the writable registry:
    // the bodies leave the base's stored reserve)
    if (!writable)
    {
        return;
    }
    const bool engineLive = writable == &ZoneRegistry::Instance();
    for (int i = 0; i < _surge.Size();)
    {
        SurgeRow& s = _surge[i];
        const int zi = writable->FindZoneIndex(s.zone);
        ZoneRecord* z = writable->GetZoneMutable(zi);
        if (!OccupierBase(*writable, z))
        {
            GM_DIRECTOR_INFO(Core, "OccupierForces: surge: {} at {} ends, the base changed hands", s.bodies,
                             (const char*)s.zone);
            _surge.Delete(i);
            continue;
        }
        // the surge is what the base still holds above garrisonMax: bodies
        // spent from it or killed take their share with them
        const int live = engineLive ? GarrisonCache::Instance().LiveCount(zi) : 0;
        const int reserve = (int)floorf(z->garrison > 0 ? z->garrison : 0);
        const int over = live + reserve + z->pending - (int)floorf(z->garrisonMax);
        s.bodies = LMin(s.bodies, LMax(over, 0));
        if (s.bodies <= 0)
        {
            _surge.Delete(i);
            continue;
        }
        // the lever is read live: peak / surgeDecayHours bodies an hour
        const float hours = _tuning.surgeDecayHours > 0.001f ? _tuning.surgeDecayHours : 0.001f;
        s.accum += (float)s.peak / hours * elapsed / 3600.0f;
        if (s.accum > (float)s.bodies)
        {
            s.accum = (float)s.bodies;
        }
        const int due = LMin((int)floorf(s.accum), s.bodies);
        // only stored bodies leave (a live or pending surge waits)
        const int take = LMin(due, reserve);
        if (take > 0)
        {
            z->garrison = (float)(reserve - take);
            s.accum -= (float)take;
            s.bodies -= take;
            const int rejoin = LMin(take, LMax(PoolCeiling() - _pool, 0));
            _pool += rejoin;
            const int stood = take - rejoin;
            _stoodDown += stood;
            if (stood > 0)
            {
                GM_DIRECTOR_INFO(Core,
                                 "OccupierForces: surge: {} unit(s) stood down at {} ({} back in the pool; {} surge "
                                 "left; pool {} of {})",
                                 stood, (const char*)s.zone, rejoin, s.bodies, _pool, _poolStart);
            }
            else
            {
                GM_DIRECTOR_INFO(Core, "OccupierForces: surge: {} unit(s) from {} back in the pool ({} surge left; pool {} "
                                 "of {})",
                                 rejoin, (const char*)s.zone, s.bodies, _pool, _poolStart);
            }
        }
        if (s.bodies <= 0)
        {
            _surge.Delete(i);
            continue;
        }
        i++;
    }
}

// ---------------------------------------------------------------------------
// the ledger's saved rows
// ---------------------------------------------------------------------------

LSError OccupierForces::StoredHullRow::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("zone", zone, 1, RString()))
    PARAM_CHECK(ar.Serialize("package", package, 1, (int)FPFoot))
    PARAM_CHECK(ar.Serialize("what", what, 1, RString()))
    PARAM_CHECK(ar.Serialize("units", units, 1, 0))
    return LSOK;
}

LSError OccupierForces::SurgeRow::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("zone", zone, 1, RString()))
    PARAM_CHECK(ar.Serialize("bodies", bodies, 1, 0))
    PARAM_CHECK(ar.Serialize("peak", peak, 1, 0))
    PARAM_CHECK(ar.Serialize("accum", accum, 1, 0.0f))
    return LSOK;
}

LSError OccupierForces::LossRow::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("stamp", stamp, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("units", units, 1, 0))
    PARAM_CHECK(ar.Serialize("repaid", repaid, 1, 0))
    return LSOK;
}

} // namespace Poseidon::Guerrilla
