// Guerrilla Mode traffic encounters (issue #66): the armed hold-up
// (surrender or escape, one roll per encounter) and the unarmed roadblock
// (brake, wait, drive on).  The pure core (the two observations, the roll,
// the encounter step, the load downgrade) is unit-tested; the world layer
// below it runs on the 0.5 s encounter sub-tick and issues each order once,
// on a transition, through the same doMove-style IssueCommand idiom the
// rest of the traffic service uses.  Member definitions of Traffic; the
// declarations live in Traffic.hpp, the types in TrafficEncounter.hpp.

#include <Poseidon/Game/Guerrilla/TrafficEncounter.hpp>
#include <Poseidon/Game/Guerrilla/Traffic.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/Game/Guerrilla/Undercover.hpp> // ClassifyWeaponShow

#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp> // GLOB_LAND surface Y
#include <Poseidon/World/Terrain/Roads.hpp>     // GRoadNet
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/World/Entities/Infantry/SoldierOld.hpp> // Man
#include <Poseidon/World/Entities/Vehicles/Transport.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/Path/ArcadeWaypoint.hpp> // CombatMode / SpeedMode
#include <Poseidon/Network/Network.hpp>        // GetNetworkManager

#include <Random/randomGen.hpp>

#include <Poseidon/Foundation/Common/FltOpts.hpp> // Square
#include <Poseidon/Foundation/Containers/BoolArray.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>

namespace Poseidon::Guerrilla
{

static float Dist2DSq(Vector3Par a, Vector3Par b)
{
    float dx = a.X() - b.X();
    float dz = a.Z() - b.Z();
    return dx * dx + dz * dz;
}

// ---------------------------------------------------------------------------
// archive
// ---------------------------------------------------------------------------

const char* EncounterChoiceName(int choice)
{
    switch (choice)
    {
        case EncSurrender:
            return "surrender";
        case EncEscape:
            return "escape";
        default:
            return "none";
    }
}

LSError SerializeEncounter(ParamArchive& ar, TrafficEncounter& enc)
{
    // flat keys in the entry's class; all absent from pre-#66 saves, whose
    // rows load with no encounter
    int c = (int)enc.choice;
    PARAM_CHECK(ar.Serialize("encounter", c, 1, (int)EncNone))
    enc.choice = (c >= 0 && c < NEncounterChoices) ? (TrafficEncounterChoice)c : EncNone;
    PARAM_CHECK(ar.Serialize("encounterDanger", enc.dangerOwned, 1, false))
    PARAM_CHECK(ar.Serialize("laneWait", enc.laneWait, 1, false))
    PARAM_CHECK(ar.Serialize("surrenderTime", enc.surrenderTime, 1, 0.0f))
    if (!ar.IsSaving())
    {
        // the hold clocks are transient: worst case a clear waits one more
        // sub-tick
        enc.clearHold = -1;
        enc.laneClear = -1;
        if (enc.surrenderTime < 0)
        {
            enc.surrenderTime = 0;
        }
    }
    return LSOK;
}

// ---------------------------------------------------------------------------
// pure core
// ---------------------------------------------------------------------------

bool Traffic::InLaneObstruction(const CommandeerObs& obs, const TrafficTuning& tuning)
{
    float dx = obs.playerPos.X() - obs.carPos.X();
    float dz = obs.playerPos.Z() - obs.carPos.Z();
    float d2 = dx * dx + dz * dz;
    if (d2 > Square(tuning.commandeerRadius))
    {
        return false;
    }
    if (d2 < 1e-4f)
    {
        return true; // standing on the bonnet: in the way whatever he carries
    }
    float d = sqrtf(d2);
    float tx = dx / d; // car -> player, unit 2D
    float tz = dz / d;

    // car heading, unit 2D
    float cx = obs.carDir.X();
    float cz = obs.carDir.Z();
    float cl = sqrtf(cx * cx + cz * cz);
    if (cl <= 1e-4f)
    {
        return false;
    }
    cx /= cl;
    cz /= cl;
    const float cos20 = 0.9397f;
    if (cx * tx + cz * tz <= cos20)
    {
        return false;
    }
    // lateral offset from the car's line of travel
    float lateral = fabsf(cx * dz - cz * dx);
    return lateral <= tuning.commandeerLaneHalfWidth;
}

bool Traffic::ArmedThreat(const CommandeerObs& obs, const TrafficTuning& tuning)
{
    if (!obs.weaponInHands)
    {
        return false;
    }
    float dx = obs.playerPos.X() - obs.carPos.X();
    float dz = obs.playerPos.Z() - obs.carPos.Z();
    float d2 = dx * dx + dz * dz;
    if (d2 > Square(tuning.commandeerRadius))
    {
        return false;
    }
    if (d2 < 1e-4f)
    {
        return true; // armed on the bonnet: the aim is undefined, the threat is not
    }
    float d = sqrtf(d2);
    float tx = dx / d;
    float tz = dz / d;
    float px = obs.playerDir.X();
    float pz = obs.playerDir.Z();
    float pl = sqrtf(px * px + pz * pz);
    if (pl <= 1e-4f)
    {
        return false;
    }
    px /= pl;
    pz /= pl;
    // player -> car is -t
    const float cos15 = 0.9659f;
    return -(px * tx + pz * tz) > cos15;
}

bool Traffic::ChooseSurrender(float roll, float chance)
{
    if (chance <= 0)
    {
        return false;
    }
    if (chance >= 1)
    {
        return true;
    }
    return roll < chance;
}

bool Traffic::ThreatEligible(TrafficState s)
{
    // a lingering car (observed trip end, crew seated) is exactly a stopped
    // civ car, and so is a panicked one cowering at the roadside
    return s == TSDriving || s == TSArrived || s == TSStalled || s == TSLingering || s == TSPanicked;
}

bool Traffic::InDangerEpisode(float dangerCooldown, TrafficState s)
{
    return dangerCooldown > 0 && s != TSPanicked;
}

bool Traffic::RollPossible(bool armedThreat, TrafficState s, const TrafficEncounter& enc, bool dangerEpisode)
{
    return armedThreat && ThreatEligible(s) && enc.choice == EncNone && !enc.dangerOwned && !dangerEpisode;
}

float Traffic::EncounterClearRadius(const TrafficTuning& tuning)
{
    float r = tuning.commandeerRadius + EncounterClearMargin;
    return r > CommandeerWatchRadius ? r : CommandeerWatchRadius;
}

TrafficEscapeLeg Traffic::ChooseEscapeLeg(bool atDest, bool originResolves, bool newDestResolves)
{
    if (!atDest)
    {
        return EscCurrent;
    }
    if (newDestResolves)
    {
        return EscNewLeg;
    }
    return originResolves ? EscUTurn : EscNone;
}

int Traffic::RouteSpeed(int kind, bool escaping)
{
    if (kind == TKPatrol)
    {
        return SpeedNormal;
    }
    if (kind == TKCiv && escaping)
    {
        return SpeedFull;
    }
    return SpeedLimited;
}

bool Traffic::ArrivalParkEligible(int kind, bool escaping)
{
    return kind == TKCiv && !escaping;
}

TrafficState Traffic::DangerDecideState(TrafficState s, bool atDest)
{
    if (s == TSPanicked || s == TSStopping || s == TSExiting)
    {
        return atDest ? TSArrived : TSDriving;
    }
    return s;
}

bool Traffic::DangerInBand(float distance, float severity, const TrafficTuning& tuning)
{
    return tuning.dangerRadius > 0 && severity > 0 && distance <= tuning.dangerRadius * DangerBandScale(severity);
}

bool Traffic::EncounterDangerRingOnly(const TrafficEncounter& enc, TrafficState s)
{
    return enc.Active() || s == TSStopping || s == TSExiting;
}

TrafficEncounterStep Traffic::StepEncounter(const TrafficEncounter& cur, const TrafficEncounterInput& in,
                                            const TrafficTuning& tuning)
{
    TrafficEncounterStep out;
    out.next = cur;
    TrafficEncounter& n = out.next;
    bool pending = in.state == TSStopping;
    bool active = cur.Active() || pending;
    // the same test the world layer gates its random draw with: the roll is
    // consumed below exactly when it was drawn (threatNow and no takeover)
    bool threatNow = RollPossible(in.armedThreat, in.state, cur, in.dangerEpisode);

    // 1. the clear hold: the encounter ends once the player stayed outside
    // the clear radius for EncounterClearHold.  The first outside
    // observation credits nothing (a stale tick must not count)
    if (active)
    {
        n.clearHold = in.playerOutside ? (cur.clearHold < 0 ? 0.0f : cur.clearHold + in.dt) : -1.0f;
        if (n.clearHold >= EncounterClearHold)
        {
            out.next = TrafficEncounter();
            out.action = EActClear;
            return out;
        }
    }
    else
    {
        n.clearHold = -1;
    }
    // 2. danger precedence: fresh in-band gunfire takes the encounter over
    // (instead of the roll, before the bail, over a running escape)
    if (in.freshDanger && !cur.dangerOwned && (active || threatNow))
    {
        n.dangerOwned = true;
        n.laneWait = false;
        n.laneClear = -1;
        out.action = EActDangerTakeover;
        return out;
    }
    // 3. latched to danger: escalation is the main tier's business
    if (cur.dangerOwned)
    {
        return out;
    }
    // 4. the pending surrender: the delay runs only while the player is near
    if (pending)
    {
        if (cur.choice != EncSurrender)
        {
            n.choice = EncSurrender; // defensive; never rolls
        }
        if (!in.playerOutside)
        {
            n.surrenderTime = cur.surrenderTime + in.dt;
        }
        if (n.surrenderTime >= tuning.commandeerStopDelay)
        {
            out.action = EActSurrenderBail;
        }
        return out;
    }
    // 5. a latched choice: the escape orders went out once; a surrender that
    // is no longer pending is inert
    if (cur.choice != EncNone)
    {
        return out;
    }
    // 6. the one roll of the encounter
    if (threatNow)
    {
        bool surrender = ChooseSurrender(in.roll, tuning.commandeerSurrenderChance);
        n.choice = surrender ? EncSurrender : EncEscape;
        n.laneWait = false;
        n.laneClear = -1;
        n.surrenderTime = 0;
        n.clearHold = -1;
        out.rolled = true;
        out.action = surrender ? EActBeginSurrender : EActBeginEscape;
        return out;
    }
    // 7. the roadblock wait in progress: resume after LaneResumeHold of
    // continuously clear lane (the first clear observation credits nothing)
    if (cur.laneWait)
    {
        if (in.state != TSDriving)
        {
            n.laneWait = false; // someone else moved the car
            n.laneClear = -1;
            return out;
        }
        if (in.laneObstruction)
        {
            n.laneClear = -1;
            return out;
        }
        n.laneClear = cur.laneClear < 0 ? 0.0f : cur.laneClear + in.dt;
        if (n.laneClear >= LaneResumeHold)
        {
            n.laneWait = false;
            n.laneClear = -1;
            out.action = EActResumeLaneWait;
        }
        return out;
    }
    // 8. a roadblock ahead of a driving car: brake and wait - unless a danger
    // reaction (a rush, a U-turn) is driving it, which a man in the road must
    // not overwrite
    if (in.laneObstruction && in.state == TSDriving && !in.dangerEpisode)
    {
        n.laneWait = true;
        n.laneClear = -1;
        out.action = EActBeginLaneWait;
    }
    return out;
}

TrafficLoadRestore Traffic::LoadedEncounterState(TrafficState& state, TrafficEncounter& enc, bool driverSeated)
{
    bool parkFamily = state == TSParking || state == TSDwelling || state == TSDeparting || state == TSLingering;
    bool stopFamily = state == TSStopping || state == TSExiting || state == TSPanicked;
    TrafficLoadRestore restore = TLRNone;
    if (!parkFamily && (enc.Active() || enc.laneWait || stopFamily) && !driverSeated)
    {
        // the driver did not come back seated: the main tick's DriverAlive
        // releases the hull and files a living driver to the fleeing table
        if (stopFamily)
        {
            state = TSDriving;
        }
        enc = TrafficEncounter();
    }
    else if (state == TSStopping && enc.choice == EncSurrender && !enc.dangerOwned)
    {
        restore = TLRSurrenderStop; // the pending surrender resumes where it was
    }
    else if (state == TSStopping || state == TSExiting)
    {
        // a pre-#66 save's commandeer stop, or an inconsistent row: drive on.
        // A danger-owned encounter keeps its memory (no second roll)
        state = TSDriving;
        if (!enc.dangerOwned)
        {
            enc = TrafficEncounter();
        }
        enc.laneWait = false;
        restore = TLRResumeDrive;
    }
    else if (state == TSPanicked)
    {
        // the danger ring is transient, so no cower survives a load
        state = TSDriving;
        enc.laneWait = false;
        restore = TLRResumeDrive;
    }
    else if (state == TSDriving && enc.laneWait && enc.choice == EncNone)
    {
        restore = TLRLaneStop;
    }
    else if (state == TSDriving && enc.EscapeOwnsMovement())
    {
        enc.laneWait = false;
        restore = TLREscapeRoute;
    }
    else if (state == TSDriving && enc.choice == EncSurrender && !enc.dangerOwned)
    {
        enc = TrafficEncounter(); // a surrender that is no longer pending is inert
    }
    // (a danger-owned surrender - gunfire took the pending stop over and the
    // car drove off - falls through and keeps its memory: resetting it would
    // hand the still-aiming player a second roll)
    else
    {
        enc.laneWait = false;
    }
    if (state != TSStopping)
    {
        enc.surrenderTime = 0;
    }
    enc.clearHold = -1;
    enc.laneClear = -1;
    return restore;
}

// ---------------------------------------------------------------------------
// world layer
// ---------------------------------------------------------------------------

bool Traffic::EncounterDangerFresh(const TrafficEntry& e, Vector3Par pos) const
{
    // the RING only (never the wreck sources) and no roll: the precedence
    // test the main tier would pass for this car right now
    if (_tuning.dangerRadius <= 0 || _danger.Size() == 0)
    {
        return false;
    }
    float dist = -1;
    int src = LoudestDanger(_danger, pos, dist);
    if (src < 0)
    {
        return false;
    }
    float severity = _danger[src].severity;
    return DangerInBand(dist, severity, _tuning) &&
           !DangerLatchHolds(e.dangerCooldown, e.state == TSPanicked, e.latchSeverity, severity);
}

// the encounter sub-tick: observe -> StepEncounter -> at most one order per
// entry.  Runs whenever civ traffic exists (the housekeeping must run with
// the player far away too); the per-entry geometry gates every trigger
void Traffic::UpdateCommandeer(float dt)
{
    if (!GWorld)
    {
        return;
    }
    Person* player = GWorld->GetRealPlayer();
    bool playerValid = player && !player->IsDammageDestroyed();
    bool armed = false;
    Vector3 playerPos = VZero;
    Vector3 playerDir = VForward;
    if (playerValid)
    {
        const Man* man = dyn_cast<Man>(player);
        armed = man && ClassifyWeaponShow(*man) == UCWInHands;
        playerPos = player->Position();
        playerDir = player->Direction();
    }
    float clearR2 = Square(EncounterClearRadius(_tuning));

    AutoArray<TrafficEventRecord> fired;
    for (int i = _entries.Size() - 1; i >= 0; i--)
    {
        TrafficEntry& e = _entries[i];
        if (e.kind != TKCiv)
        {
            continue;
        }
        Transport* veh = e.vehicle;
        AIGroup* grp = e.group;
        Person* driver = e.driver;
        if (!veh || !grp || !driver || driver->IsDammageDestroyed() || veh->Driver() != driver)
        {
            continue; // the main tick reconciles dead/vanished entries (and the parked family)
        }
        if (e.loadRestore != TLRNone)
        {
            ExecuteLoadRestore(e, veh);
            e.loadRestore = TLRNone;
        }
        CommandeerObs obs;
        obs.carPos = veh->Position();
        obs.carDir = veh->Direction();
        obs.playerPos = playerPos;
        obs.playerDir = playerDir;
        obs.weaponInHands = armed;

        TrafficEncounterInput in;
        in.state = e.state;
        in.dt = dt;
        in.armedThreat = playerValid && ArmedThreat(obs, _tuning);
        in.laneObstruction = playerValid && InLaneObstruction(obs, _tuning);
        in.playerOutside = !playerValid || Dist2DSq(veh->Position(), playerPos) > clearR2;
        in.dangerEpisode = InDangerEpisode(e.dangerCooldown, e.state);
        // StepEncounter's own threatNow: the draw below happens exactly when
        // the step can consume it
        bool rollable = RollPossible(in.armedThreat, e.state, e.encounter, in.dangerEpisode);
        bool dangerGate = e.encounter.Active() || e.state == TSStopping || rollable;
        in.freshDanger = dangerGate && EncounterDangerFresh(e, veh->Position());
        // draw only when a roll is possible: a draw every 0.5 s would perturb
        // the shared random stream
        in.roll = (rollable && !in.freshDanger) ? GRandGen.RandomValue() : 0.0f;

        TrafficEncounter before = e.encounter;
        TrafficEncounterStep step = StepEncounter(before, in, _tuning);
        e.encounter = step.next;
        if (step.rolled)
        {
            _diag.encounterRolls++;
            LOG_INFO(Core, "Traffic: civ driver at gunpoint chose to {}",
                     step.next.choice == EncSurrender ? "surrender" : "escape");
        }
        if (step.action != EActNone)
        {
            ExecuteEncounterAction(i, before, step.action, playerPos, fired);
        }
    }
    DispatchEvents(fired);
}

void Traffic::ExecuteEncounterAction(int index, const TrafficEncounter& before, TrafficEncounterAction action,
                                     Vector3Par playerPos, AutoArray<TrafficEventRecord>& fired)
{
    TrafficEntry& e = _entries[index];
    Transport* veh = e.vehicle;
    if (!veh)
    {
        return;
    }
    Vector3 pos = veh->Position();
    bool atDest = Dist2DSq(pos, e.dest) <= Square(_tuning.arriveRadius);
    switch (action)
    {
        case EActBeginSurrender:
            // Stop to the whole crew (issued like VehStop silent,
            // GameStateExtGrp.cpp); the lingerReason stays: the row dies at
            // the bail or is resumed on clear
            IssueGroupStop(e.group);
            e.state = TSStopping;
            e.stateTime = 0;
            break;
        case EActSurrenderBail:
            SurrenderBail(index, playerPos, fired);
            break;
        case EActBeginEscape:
            BeginEscape(e, veh);
            break;
        case EActBeginLaneWait:
            // a deliberate wait, not a stall: the main tier freezes the
            // stall ladder while laneWait holds
            IssueGroupStop(e.group);
            e.stallTime = 0;
            e.blockedRetries = 0;
            e.lastPos = pos;
            LOG_INFO(Core, "Traffic: civ car waiting for a man in the road");
            break;
        case EActResumeLaneWait:
            IssueRoute(e, e.dest, CMCareless, RouteSpeed(e.kind, false), false);
            e.stallTime = 0;
            e.blockedRetries = 0;
            e.lastPos = pos;
            break;
        case EActDangerTakeover:
        {
            // the shared danger decision against the RING (the takeover is
            // only ever raised for an in-band, unlatched ring source)
            int src = -1;
            TrafficDangerReaction react = DecideEntryDanger(e, veh, _danger, atDest, src);
            if (react != TDRNone && src >= 0)
            {
                TrafficDangerEvent source = _danger[src];
                LOG_INFO(Core, "Traffic: gunfire takes over a civ car's armed encounter");
                ApplyDangerReaction(index, react, source, atDest, fired); // may remove the entry
            }
            // react None (not expected): dangerOwned stays set and the
            // encounter clears normally
            break;
        }
        case EActClear:
            if (e.state == TSStopping || e.state == TSExiting)
            {
                // the player walked away before the driver bailed: the stop
                // is called off.  At the destination the car returns to
                // TSArrived (no duplicate arrived event, the cower-resume
                // precedent)
                e.state = atDest ? TSArrived : TSDriving;
                e.stateTime = 0;
                e.stallTime = 0;
                e.blockedRetries = 0;
                e.lastPos = pos;
                if (!atDest)
                {
                    IssueRoute(e, e.dest, CMCareless, RouteSpeed(e.kind, false), false);
                }
            }
            else if (before.EscapeOwnsMovement() && (e.state == TSDriving || e.state == TSStalled))
            {
                // the escape is over: back to the ordinary pace (a stalled
                // escape too, or its subgroup would keep SpeedFull into the
                // next recovery).  A danger-owned escape is excluded by
                // EscapeOwnsMovement: the danger reaction set its own pace
                IssueRoute(e, e.dest, CMCareless, RouteSpeed(e.kind, false), false);
            }
            LOG_INFO(Core, "Traffic: civ car encounter cleared");
            break;
        default:
            break;
    }
}

// the escape: the driver stays seated and floors it, issued once.  Mid-trip
// he keeps his leg (a car blocked by the gunman reaches the U-turn through
// the ordinary blocked recovery, at SpeedFull); at the destination he needs
// a real continuation - a new leg, else home, else nothing (the choice
// stands, no reroll)
void Traffic::BeginEscape(TrafficEntry& e, Transport* veh)
{
    Vector3 pos = veh->Position();
    bool atDest = Dist2DSq(pos, e.dest) <= Square(_tuning.arriveRadius);
    const ZoneRecord* back = e.originIndex >= 0 ? ZoneRegistry::Instance().GetZone(e.originIndex) : nullptr;
    bool originOk = back && GRoadNet;
    int next = -1;
    if (atDest)
    {
        AutoArray<TrafficZoneCandidate> zones;
        BuildZoneCandidates(zones);
        next = DestForOrigin(e.kind, zones, e.destIndex, GRandGen.RandomValue()); // a route draw, not the choice
    }
    const ZoneRecord* z = next >= 0 ? ZoneRegistry::Instance().GetZone(next) : nullptr;
    bool nextOk = z && GRoadNet;
    switch (ChooseEscapeLeg(atDest, originOk, nextOk))
    {
        case EscUTurn:
        {
            int backIndex = e.originIndex;
            RString backZone = e.originZone;
            e.originIndex = e.destIndex;
            e.originZone = e.destZone;
            e.destIndex = backIndex;
            e.destZone = backZone;
            e.legs++;
            IssueRoute(e, GRoadNet->GetNearestRoadPoint(back->pos), CMCareless, SpeedFull, false);
            LOG_INFO(Core, "Traffic: civ driver at gunpoint escapes back to {}", (const char*)backZone);
            break;
        }
        case EscNewLeg:
            e.originIndex = e.destIndex;
            e.originZone = e.destZone;
            e.destIndex = next;
            e.destZone = z->name;
            e.legs++;
            IssueRoute(e, GRoadNet->GetNearestRoadPoint(z->pos), CMCareless, SpeedFull, false);
            LOG_INFO(Core, "Traffic: civ driver at gunpoint escapes on to {}", (const char*)e.destZone);
            break;
        case EscCurrent:
            IssueRoute(e, e.dest, CMCareless, SpeedFull, false);
            LOG_INFO(Core, "Traffic: civ driver at gunpoint floors it");
            break;
        case EscNone:
        default:
            LOG_INFO(Core, "Traffic: civ driver chose to run but has no road out");
            return;
    }
    e.state = TSDriving;
    e.stateTime = 0;
    e.stallTime = 0;
    e.blockedRetries = 0;
    e.exposeDefer = 0;
    e.lingerReason = RString();
    e.lastPos = pos;
}

// the surrender: the crew stops wanting the car, the driver steps out and
// runs from the player at full speed; the hull is released to the world
void Traffic::SurrenderBail(int index, Vector3Par playerPos, AutoArray<TrafficEventRecord>& fired)
{
    TrafficEntry& e = _entries[index];
    Transport* veh = e.vehicle;
    AIGroup* grp = e.group;
    Person* driver = e.driver;
    if (!veh || !grp || !driver)
    {
        return;
    }
    AIUnit* unit = driver->Brain();
    grp->UnassignVehicle(veh);
    if (unit)
    {
        // he leaves the traffic system for good: revoke any re-board order
        unit->OrderGetIn(false);
        unit->AllowGetIn(false);
        unit->DoGetOut(veh, false);
    }
    Vector3 away = driver->Position() - playerPos;
    away[1] = 0;
    if (away.SquareSize() < 1e-2f)
    {
        away = -veh->Direction();
        away[1] = 0;
    }
    Vector3 flee = driver->Position() + away.Normalized() * _tuning.fleeDist;
    if (GLandscape)
    {
        float dx, dz;
        flee[1] = GLOB_LAND->SurfaceYAboveWater(flee[0], flee[2], &dx, &dz);
    }
    if (unit)
    {
        Command mv;
        mv._message = Command::Move;
        mv._destination = flee;
        mv._discretion = Command::Undefined;
        mv._context = Command::CtxMission;
        mv._id = grp->GetNextCmdId();
        PackedBoolArray one;
        one.Set(unit->ID() - 1, true);
        grp->IssueCommand(mv, one); // doMove idiom
        // the solo Move pulled him into a command subgroup, and a fresh one
        // constructs as SpeedNormal: the sprint must reach that subgroup
        if (AISubgroup* sub = unit->GetSubgroup())
        {
            sub->SetSpeedMode(SpeedFull);
        }
    }
    grp->SetCombatModeMajor(CMCareless);
    if (grp->MainSubgroup())
    {
        grp->MainSubgroup()->SetSpeedMode(SpeedFull);
    }
    GetNetworkManager().UpdateObject(grp);

    FleeingDriver fd;
    fd.person = driver;
    fd.group = grp;
    fd.age = 0;
    _fleeing.Add(fd);

    TrafficEventRecord ev;
    ev.type = TECommandeered;
    ev.kind = e.kind;
    ev.originIndex = e.originIndex;
    ev.destIndex = e.destIndex;
    ev.vehicle = veh;
    ev.driver = driver;
    fired.Add(ev);

    // registry half: the hull is released (no bodies - the driver is alive
    // and tracked by _fleeing); the player forced this car off the road at
    // gunpoint, so its hull gets the longer memory
    ReleasedEntry r;
    r.vehicle = veh;
    r.playerCaused = true;
    _released.Add(r);
    _entries.Delete(index);
    LOG_INFO(Core, "Traffic: civ car commandeered");
}

// the one order a loaded encounter row needs, issued on the first post-load
// sub-tick (never inside the world serializer's load pass)
void Traffic::ExecuteLoadRestore(TrafficEntry& e, Transport* veh)
{
    switch (e.loadRestore)
    {
        case TLRSurrenderStop:
        case TLRLaneStop:
            IssueGroupStop(e.group);
            break;
        case TLREscapeRoute:
            // e.dest is the serialized escape leg; IssueRoute re-applies the
            // speed on the driver's real subgroup
            IssueRoute(e, e.dest, CMCareless, SpeedFull, false);
            break;
        case TLRResumeDrive:
        {
            Vector3 pos = veh->Position();
            e.stallTime = 0;
            e.lastPos = pos;
            if (Dist2DSq(pos, e.dest) > Square(_tuning.arriveRadius))
            {
                IssueRoute(e, e.dest, CMCareless, RouteSpeed(e.kind, false), false);
            }
            else if (e.state == TSDriving)
            {
                // stopped at its destination: back to TSArrived so the main
                // tick does not fire a second arrived event or re-roll the
                // park (the cower-resume precedent)
                e.state = TSArrived;
            }
            break;
        }
        default:
            break;
    }
}

bool Traffic::EncounterInfo(const Transport* veh, TrafficEncounter& out) const
{
    if (!veh)
    {
        return false;
    }
    for (int i = 0; i < _entries.Size(); i++)
    {
        if (_entries[i].vehicle.GetLink() == veh)
        {
            out = _entries[i].encounter;
            return true;
        }
    }
    return false;
}

void Traffic::ProbeThreat(const Transport* veh, bool& armed, bool& threat, bool& lane) const
{
    armed = threat = lane = false;
    if (!veh || !GWorld)
    {
        return;
    }
    bool tracked = false;
    for (int i = 0; i < _entries.Size() && !tracked; i++)
    {
        tracked = _entries[i].vehicle.GetLink() == veh;
    }
    Person* player = GWorld->GetRealPlayer();
    if (!tracked || !player || player->IsDammageDestroyed())
    {
        return;
    }
    const Man* man = dyn_cast<Man>(player);
    armed = man && ClassifyWeaponShow(*man) == UCWInHands;
    CommandeerObs obs;
    obs.carPos = veh->Position();
    obs.carDir = veh->Direction();
    obs.playerPos = player->Position();
    obs.playerDir = player->Direction();
    obs.weaponInHands = armed;
    threat = ArmedThreat(obs, _tuning);
    lane = InLaneObstruction(obs, _tuning);
}

float Traffic::SetCommandeerSurrenderChance(float chance)
{
    saturate(chance, 0.0f, 1.0f);
    _tuning.commandeerSurrenderChance = chance;
    return chance;
}

} // namespace Poseidon::Guerrilla
