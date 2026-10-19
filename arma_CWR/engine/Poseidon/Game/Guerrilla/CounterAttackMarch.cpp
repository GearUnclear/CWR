// CounterAttack: the live squad's march to the town edge
// (guerrilla-mode/OCCUPIER-FORCES.md 3.2 and 6.5 "At the town edge").
// Split out of CounterAttack.cpp, which sits over the FileSize limit.
//
// A live force walks (or rides) its route leg by leg.  On foot it deploys at
// the zone edge plus TownEdgeMargin; a mounted force rides to
// counterAttackBirthMinToTarget (the 250 m mark), the hull stops, the
// infantry gets out and assaults, an armed hull supports at the town edge
// and a truck holds back.
//
// The stall rule (video 6 of 2026-10-17, op 13): a hull that makes no
// progress toward the town does not keep its infantry seated.  The dismount
// trigger used to be a distance alone, so a Ural born at the arrival point
// 306 m from Sainte Marie that never answered its driver's Move sat there,
// nine men seated, until the live timeout withdrew it.  Now a mounted force
// that has not closed StallProgress metres for StallNearTime seconds within
// DismountReach of the mark, or for StallTime seconds anywhere, dismounts
// where it stands: near the mark the infantry assaults from there, farther
// out it marches the rest of the route on foot.

#include <Poseidon/Game/Guerrilla/CounterAttack.hpp>

#include <Poseidon/Game/Guerrilla/ForcePackages.hpp> // EdgeRole / SupportsAtEdge
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Entities/Vehicles/Transport.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/AICore.hpp>

#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Game/Guerrilla/DirectorLog.hpp> // GM_DIRECTOR_INFO
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>

namespace Poseidon
{
namespace Guerrilla
{

namespace
{

float MarchDist2D(Vector3Par a, Vector3Par b)
{
    const float dx = a.X() - b.X();
    const float dz = a.Z() - b.Z();
    return sqrtf(dx * dx + dz * dz);
}

int MarchMin(int a, int b)
{
    return a < b ? a : b;
}

int MarchMax(int a, int b)
{
    return a > b ? a : b;
}

// the town edge (zoneArea from dest) on the line from dest to pos: where an
// armed hull supports (6.5, phase 5); pos itself when already inside
Vector3 EdgePoint(float edgeR, Vector3Par pos, Vector3Par dest)
{
    Vector3 back = pos - dest;
    back[1] = 0;
    const float d = back.Size();
    return d > edgeR && d > 1e-3f ? dest + back * (edgeR / d) : Vector3(pos);
}

} // namespace

float CounterAttack::StallStep(float& mark, float& stalledFor, float toDest, float elapsed)
{
    if (mark < 0 || toDest <= mark - StallProgress)
    {
        // the first measure, or StallProgress closer than the last mark
        mark = toDest;
        stalledFor = 0;
    }
    else
    {
        stalledFor += elapsed > 0 ? elapsed : 0;
    }
    return stalledFor;
}

int CounterAttack::StallVerdict(float toDest, float birthMin, float stalledFor)
{
    if (toDest <= birthMin + DismountReach)
    {
        return stalledFor >= StallNearTime ? StallAtMark : StallNone;
    }
    return stalledFor >= StallTime ? StallOnRoute : StallNone;
}

void CounterAttack::March(const ZoneRegistry& registry, const CounterAttackView& view, int target, Record& r,
                          CounterAttackOps* ops, Vector3Par leader, float elapsed)
{
    (void)target;
    const Vector3 dest = DestOf(registry, r);
    if ((dest.X() == 0 && dest.Z() == 0) || !ops)
    {
        return;
    }
    const AutoArray<Vector3>& route = RouteOf(registry, r, ops);
    const int n = route.Size();
    // a mounted force rides to counterAttackBirthMinToTarget and dismounts
    // there (3.2); men on foot deploy at the zone edge plus TownEdgeMargin
    const bool mounted = LiveHullOf(r) > 0 && !r.dismounted;
    const float edge = mounted ? _tuning.birthMinToTarget : registry.Tuning().zoneArea + TownEdgeMargin;
    const float toDest = MarchDist2D(leader, dest);
    // the stall rule: a hull that stopped closing on the town dismounts its
    // infantry where it stands (the clock runs while the column waits for
    // its escort too: an escort that never closes up stalls the carrier)
    int stall = StallNone;
    if (mounted && n > 0 && toDest > edge)
    {
        const float stalledFor = StallStep(r.stallMark, r.stallFor, toDest, elapsed);
        stall = r.stopping >= 0 && r.stallVerdict != StallNone
                    ? r.stallVerdict
                    : StallVerdict(toDest, _tuning.birthMinToTarget, stalledFor);
        if (stall != StallNone && r.stallVerdict == StallNone)
        {
            GM_DIRECTOR_INFO(
                Core,
                "CounterAttack: [op {}] the {} carrying the force on {} made no progress for {:.0f} s, {:.0f} m"
                " out: the infantry gets out where it stands{}",
                r.opId, (const char*)r.hullClass, r.kind == CAKHqAssault ? "the HQ" : (const char*)r.target, stalledFor,
                toDest, stall == StallAtMark ? " and assaults from there" : " and marches on");
        }
        r.stallVerdict = stall;
    }
    if (n == 0 || toDest <= edge || stall != StallNone)
    {
        const bool atEdge = stall != StallOnRoute;
        if (atEdge)
        {
            // the town edge: deploy and move in; the live timeout starts now
            r.leg = LegTown;
            if (r.clock < 0)
            {
                r.clock = 0;
            }
            r.resend = ResendInterval;
        }
        if (mounted)
        {
            // the hull stops first: a cargo seat ejects its man with the
            // hull's speed, and at road speed that kills him
            if (r.stopping < 0)
            {
                r.stopping = 0;
                ops->DriveOn(r.crewGroup, leader);
                if (LiveEscortOf(r) > 0 && atEdge)
                {
                    // the escort does not overtake into the town: it closes
                    // up to the town edge and supports from there (6.5)
                    ops->Support(r.escortCrewGroup, EdgePoint(registry.Tuning().zoneArea, leader, dest));
                }
                r.leg = LegRoad;
                return;
            }
            r.stopping += elapsed;
            if (ops->HullSpeed(r.hull) > DismountSpeed && r.stopping < StopMax)
            {
                r.leg = LegRoad;
                return;
            }
            // stopped: the infantry gets out and assaults; a truck holds
            // back where it stands, an armed hull drives up to the town
            // edge and supports (ForcePackages::EdgeRole, phase 5)
            ops->Dismount(r.group, r.hull);
            r.dismounted = true;
            r.stopping = -1;
            r.stallVerdict = StallNone;
            if (ForcePackages::SupportsAtEdge(r.package))
            {
                ops->Support(r.crewGroup, EdgePoint(registry.Tuning().zoneArea, leader, dest));
            }
        }
        if (!atEdge)
        {
            // stalled far out: the infantry walks the rest of its route (its
            // leg re-found from where it stands, sent at once); the escort
            // keeps driving the same legs, a truck holds back where it stands
            r.leg = LegUnknown;
            r.resend = 0;
            GM_DIRECTOR_INFO(Core,
                             "CounterAttack: [op {}] the force on {} dismounted {:.0f} m out; it marches on on foot",
                             r.opId, r.kind == CAKHqAssault ? "the HQ" : (const char*)r.target, toDest);
            return;
        }
        if (LiveEscortOf(r) > 0)
        {
            // a column's escort always supports (6.5)
            ops->Support(r.escortCrewGroup, EdgePoint(registry.Tuning().zoneArea, leader, dest));
        }
        ops->Assault(r.group, dest);
        GM_DIRECTOR_INFO(Core, "CounterAttack: the force on {} reached the town edge ({:.0f} m out){}; it moves in",
                         r.kind == CAKHqAssault ? "the HQ" : (const char*)r.target, toDest,
                         mounted ? " and dismounts" : "");
        Emit(registry, r, CAOpArrived, r.strength, 0, mounted ? "live, dismounted" : "live");
        TraceArrived(view, r, "live", mounted ? "dismounts and moves in" : "moves in", leader);
        return;
    }
    // a column keeps together (6.5): the carrier waits where it is while its
    // escort (a slower tank, say) is more than EscortSpread behind, and
    // drives on once it has closed to EscortRejoin
    if (mounted && LiveEscortOf(r) > 0)
    {
        Vector3 escortAt = VZero;
        if (ops->Alive(r.escortCrewGroup, escortAt) > 0)
        {
            const float spread = MarchDist2D(escortAt, leader);
            if (!r.escortWait && spread > EscortSpread)
            {
                r.escortWait = true;
                ops->DriveOn(r.crewGroup, leader);
                ops->DriveOn(r.escortCrewGroup, leader);
            }
            else if (r.escortWait && spread <= EscortRejoin)
            {
                r.escortWait = false;
                r.resend = 0; // the leg is re-sent below
            }
            if (r.escortWait)
            {
                return;
            }
        }
        else
        {
            r.escortWait = false;
        }
    }
    // a hull covers ground fast: longer legs, so it does not stop at every
    // 200 m point
    const int stride = mounted ? 3 * WaypointStride : WaypointStride;
    // the route point nearest the leader, searched around the current leg
    // (the whole road after a reload)
    const bool known = r.leg == LegRoad;
    const int lo = known ? MarchMax(0, r.wp - 2 * stride) : 0;
    const int hi = known ? MarchMin(n - 1, r.wp + 2 * stride) : n - 1;
    int at = lo;
    float atD = -1;
    for (int i = lo; i <= hi; i++)
    {
        const float d = MarchDist2D(leader, route[i]);
        if (atD < 0 || d < atD)
        {
            atD = d;
            at = i;
        }
    }
    bool issue = false;
    if (!known)
    {
        r.leg = LegRoad;
        r.wp = MarchMin(at + stride, n - 1);
        issue = true;
    }
    else if (r.wp >= 0 && r.wp < n && (at >= r.wp - 1 || MarchDist2D(leader, route[r.wp]) < WaypointReach))
    {
        // this leg is done: the next one ends WaypointStride points on
        const int next = MarchMin(at + stride, n - 1);
        if (next != r.wp)
        {
            r.wp = next;
            issue = true;
        }
    }
    r.resend -= elapsed;
    if (r.resend <= 0)
    {
        issue = true; // a stalled leg is re-sent
    }
    if (issue && r.wp >= 0 && r.wp < n)
    {
        r.resend = ResendInterval;
        if (mounted)
        {
            ops->DriveOn(r.crewGroup, route[r.wp]); // the Traffic idiom: the driver, never an arcade waypoint
        }
        else
        {
            ops->MoveOn(r.group, route[r.wp]);
        }
        if (LiveEscortOf(r) > 0)
        {
            ops->DriveOn(r.escortCrewGroup, route[r.wp]); // the escort drives the same legs (6.5)
        }
    }
}

} // namespace Guerrilla
} // namespace Poseidon
