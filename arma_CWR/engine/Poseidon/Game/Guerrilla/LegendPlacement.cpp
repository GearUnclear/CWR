#include <Poseidon/Game/Guerrilla/LegendPlacement.hpp>
#include <Poseidon/Game/Guerrilla/WorldNames.hpp>   // world place names (the `named` preference)
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp> // FactionRecord / ParsClassProbe

#include <Poseidon/World/World.hpp>                         // GWorld
#include <Poseidon/World/Terrain/Landscape.hpp>             // GLOB_LAND surface Y
#include <Poseidon/World/Terrain/Roads.hpp>                 // GRoadNet
#include <Poseidon/World/Entities/Vehicles/Transport.hpp>   // TransportType (crew seats)
#include <Poseidon/World/Entities/Vehicles/Ground/Tank.hpp> // TankType (is this hull a tank?)
#include <Poseidon/World/Entities/Vehicles/Vehicle.hpp>     // VehicleTypes bank

#include <Poseidon/Foundation/Common/FltOpts.hpp> // Square
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Strings/StrFormat.hpp> // Format
#include <Poseidon/Foundation/platform.hpp>

#include <float.h>
#include <math.h>
#include <string.h>

namespace Poseidon
{
namespace Guerrilla
{

typedef LegendPlacementConstants LPC;

// A template with no CAMP does not get the Camp rules bent around it: every
// stand reads as very far from a camp that is not there, so the hard floor and
// the 800 m preference both pass and campRelaxed stays false.
static const float kNoCampDistance = 1.0e6f;

// How near a world place name has to be for a stand to count as `named`.  Not a
// LegendPlacementConstants member on purpose: it describes the SAMPLE builder's
// stamping, not the ranking contract the pure picker and its tests share.  The
// rings are 200 m apart and the bearings ~183 m apart on the inner ring, so 150 m
// marks the handful of stands actually beside a named feature rather than a
// whole ring.
static const float kNamedSpotRadius = 150.0f;

// Retinue sizes.  A sniper works with one spotter/guard; a commander keeps four.
// A tank commander's retinue is his crew, so it comes off the hull's seats.
static const int kSniperGuardCount = 1;
static const int kEliteGuardCount = 4;

// ---------------------------------------------------------------------------
// placement: the pure picker
// ---------------------------------------------------------------------------

namespace
{

// The rank key of one candidate, in preference order.  Compared strictly
// lexicographically, ending at the sample index, so the order is TOTAL: two
// candidates never compare equal and no result depends on sort stability.
struct RankKey
{
    bool distinctZone = false; // stands off a zone no other commander uses
    bool named = false;        // a world place name can describe the spot
    bool campOk = false;       // clears the PREFERRED camp distance, not just the floor
    float spreadSq = 0;        // squared distance to the nearest stand already picked
    float campDist = 0;        // m from the Camp
    int index = 0;             // last resort, and the reason this order is total
};

bool Better(const RankKey& a, const RankKey& b)
{
    if (a.distinctZone != b.distinctZone)
    {
        return a.distinctZone;
    }
    if (a.named != b.named)
    {
        return a.named;
    }
    if (a.campOk != b.campOk)
    {
        return a.campOk;
    }
    if (a.spreadSq != b.spreadSq)
    {
        return a.spreadSq > b.spreadSq;
    }
    if (a.campDist != b.campDist)
    {
        return a.campDist > b.campDist;
    }
    return a.index < b.index;
}

float Dist2(const LegendSpotSample& a, const LegendSpotSample& b)
{
    const float dx = a.x - b.x;
    const float dz = a.z - b.z;
    return dx * dx + dz * dz;
}

bool IsCampZone(const LegendZoneCandidate& z)
{
    return stricmp(z.type, "CAMP") == 0;
}

bool IsCityZone(const LegendZoneCandidate& z)
{
    return stricmp(z.type, "CITY") == 0;
}

// Does this spot sit inside some zone's presence radius?  Squared, and over
// the WHOLE zone table rather than the sampled subset, because the zones this
// has to keep a commander out of are exactly the ones the ladder discarded.
bool ForeignZoneRejects(const AutoArray<LegendZoneCandidate>& avoid, float x, float z)
{
    const float floorSq = LegendPlacementConstants::ForeignZoneFloor * LegendPlacementConstants::ForeignZoneFloor;
    for (int i = 0; i < avoid.Size(); i++)
    {
        const float dx = avoid[i].x - x;
        const float dz = avoid[i].z - z;
        if (dx * dx + dz * dz < floorSq)
        {
            return true;
        }
    }
    return false;
}

} // namespace

LegendPlacementResult PickLegendSpots(const AutoArray<LegendSpotSample>& samples, unsigned seed)
{
    // The ranking below consults no randomness at all; see the header for why
    // the seed is taken anyway.
    (void)seed;

    LegendPlacementResult res;
    if (samples.Size() == 0)
    {
        res.reason = LPNoZones;
        return res;
    }

    // Hard rejects, decided once: a stand in the sea, on the road net, or inside
    // the Camp floor is never a candidate no matter what has been picked.  The
    // three counts are what names the failure afterwards.
    AutoArray<bool> rejected;
    rejected.Resize(samples.Size());
    int dry = 0;
    int dryOffRoad = 0;
    for (int i = 0; i < samples.Size(); i++)
    {
        const LegendSpotSample& s = samples[i];
        if (!s.underwater)
        {
            dry++;
            if (!s.onRoad)
            {
                dryOffRoad++;
            }
        }
        rejected[i] = s.underwater || s.onRoad || s.distFromCamp < LPC::CampFloor;
    }

    while (res.picked.Size() < LPC::Wanted)
    {
        int best = -1;
        RankKey bestKey;
        for (int i = 0; i < samples.Size(); i++)
        {
            if (rejected[i])
            {
                continue;
            }
            const LegendSpotSample& s = samples[i];

            // separation floor against every stand already taken; the nearest of
            // them is also this candidate's spread score
            bool crowded = false;
            bool distinct = true;
            float spreadSq = FLT_MAX;
            for (int p = 0; p < res.picked.Size(); p++)
            {
                const LegendSpotSample& taken = samples[res.picked[p]];
                const float floorM = (taken.zone == s.zone) ? LPC::SameZoneFloor : LPC::CrossZoneFloor;
                const float d2 = Dist2(s, taken);
                if (d2 < floorM * floorM)
                {
                    crowded = true;
                    break;
                }
                if (taken.zone == s.zone)
                {
                    distinct = false;
                }
                if (d2 < spreadSq)
                {
                    spreadSq = d2;
                }
            }
            if (crowded)
            {
                continue;
            }

            RankKey key;
            key.distinctZone = distinct;
            key.named = s.named;
            key.campOk = s.distFromCamp >= LPC::CampPreferred;
            key.spreadSq = spreadSq;
            key.campDist = s.distFromCamp;
            key.index = i;
            if (best < 0 || Better(key, bestKey))
            {
                best = i;
                bestKey = key;
            }
        }
        if (best < 0)
        {
            break;
        }
        res.picked.Add(best);
        rejected[best] = true; // never pick the same stand twice
    }

    for (int p = 0; p < res.picked.Size(); p++)
    {
        if (samples[res.picked[p]].distFromCamp < LPC::CampPreferred)
        {
            // Routine, not an error: Abel's Camp is 517 m from its Outpost and
            // the Demo's 289 m, so most of the inner ring is inside the
            // preference on every shipped template.
            res.campRelaxed = true;
        }
    }

    if (res.picked.Size() >= LPC::Wanted)
    {
        res.reason = LPOk;
    }
    else if (dry == 0)
    {
        res.reason = LPAllUnderwater;
    }
    else if (dryOffRoad == 0)
    {
        res.reason = LPAllOnRoad;
    }
    else
    {
        res.reason = LPTooCrowded;
    }
    return res;
}

// ---------------------------------------------------------------------------
// placement: the zone ladder and the sample builder
// ---------------------------------------------------------------------------

void SelectLegendZones(const AutoArray<LegendZoneCandidate>& all, AutoArray<LegendZoneCandidate>& out, int& campIndex)
{
    out.Clear();
    campIndex = -1;

    // (1) the occupier's own military zones
    for (int i = 0; i < all.Size(); i++)
    {
        const LegendZoneCandidate& z = all[i];
        if (IsCampZone(z) || IsCityZone(z))
        {
            continue;
        }
        if (z.occupied)
        {
            out.Add(z);
        }
    }
    // (2) not enough of them: every military zone, whoever owns it.  Four of the
    // five shipped templates take this path (Abel and Demo field ONE military
    // zone each), which is why the same-zone separation floor does most of the
    // work in practice.
    if (out.Size() < LPC::Wanted)
    {
        out.Clear();
        for (int i = 0; i < all.Size(); i++)
        {
            const LegendZoneCandidate& z = all[i];
            if (IsCampZone(z) || IsCityZone(z))
            {
                continue;
            }
            out.Add(z);
        }
    }

    // the Camp rides along as the last entry so the sample builder and the
    // caller agree on ONE array and one set of zone indices
    for (int i = 0; i < all.Size(); i++)
    {
        if (IsCampZone(all[i]))
        {
            campIndex = out.Size();
            out.Add(all[i]);
            break;
        }
    }
}

void BuildLegendSamplesWith(const LegendTerrainProvider* terrain, const AutoArray<LegendZoneCandidate>& zones,
                            int campIndex, AutoArray<LegendSpotSample>& out,
                            const AutoArray<LegendZoneCandidate>* avoid)
{
    out.Clear();
    const bool hasCamp = campIndex >= 0 && campIndex < zones.Size();
    const float campX = hasCamp ? zones[campIndex].x : 0;
    const float campZ = hasCamp ? zones[campIndex].z : 0;

    for (int zi = 0; zi < zones.Size(); zi++)
    {
        const LegendZoneCandidate& zone = zones[zi];
        if (zi == campIndex || IsCampZone(zone))
        {
            continue; // the Camp is the player's; nobody stands guard on it
        }
        for (int r = 0; r < LPC::NRings; r++)
        {
            const float radius = LPC::Rings[r];
            for (int b = 0; b < LPC::Bearings; b++)
            {
                const float angle = (2.0f * H_PI) * (float)b / (float)LPC::Bearings;
                LegendSpotSample s;
                s.zone = zi;
                s.x = zone.x + radius * cosf(angle);
                s.z = zone.z + radius * sinf(angle);
                s.distFromZone = radius;
                // Never inside another zone's presence radius.  The auto-seeded
                // CITY zones are the live case: a commander standing 100 m off
                // a town centre reads as occupier presence there and pins that
                // town's support at the floor for the rest of the campaign.
                // The inner ring is 350 m, so this can never refuse a stand for
                // being near the zone it was placed to watch.
                if (avoid && ForeignZoneRejects(*avoid, s.x, s.z))
                {
                    continue;
                }
                if (terrain)
                {
                    s.height = terrain->SurfaceY(s.x, s.z);
                    // sea level is Y=0; the margin keeps a stand off the wash
                    s.underwater = s.height < LPC::MinDryHeight;
                    s.onRoad = terrain->IsOnRoad(s.x, s.height, s.z, LPC::RoadClearance);
                    s.named = terrain->NamedNear(s.x, s.z);
                }
                if (hasCamp)
                {
                    const float dx = s.x - campX;
                    const float dz = s.z - campZ;
                    s.distFromCamp = sqrtf(dx * dx + dz * dz);
                }
                else
                {
                    s.distFromCamp = kNoCampDistance;
                }
                out.Add(s);
            }
        }
    }
}

namespace
{

// The live world's answers.  Built once per placement pass: the place-name walk
// is a config read, not something to repeat 48 times per zone.
class WorldTerrainProvider final : public LegendTerrainProvider
{
  public:
    explicit WorldTerrainProvider(const AutoArray<LegendZoneCandidate>& zones)
    {
        AutoArray<PlaceName> all;
        CollectWorldPlaceNames(all);
        AutoArray<PlaceName> settlements, features;
        SettlementNames(all, settlements);
        FeatureNames(all, features);
        // Only the names that sit at ring distance off one of the zones can
        // describe a stand: a name in the middle of the zone, or over the
        // horizon, tells the player nothing about where the commander is.
        for (int pass = 0; pass < 2; pass++)
        {
            const AutoArray<PlaceName>& src = pass == 0 ? settlements : features;
            for (int i = 0; i < src.Size(); i++)
            {
                for (int z = 0; z < zones.Size(); z++)
                {
                    const float dx = src[i].pos.X() - zones[z].x;
                    const float dz = src[i].pos.Z() - zones[z].z;
                    const float d = sqrtf(dx * dx + dz * dz);
                    if (d >= LPC::NamedNearMin && d <= LPC::NamedNearMax)
                    {
                        _places.Add(src[i].pos);
                        break;
                    }
                }
            }
        }
    }

    float SurfaceY(float x, float z) const override { return GLOB_LAND->SurfaceY(x, z); }

    bool IsOnRoad(float x, float y, float z, float radius) const override
    {
        if (!GRoadNet)
        {
            return false;
        }
        return GRoadNet->IsOnRoad(Vector3(x, y, z), radius) != nullptr;
    }

    bool NamedNear(float x, float z) const override
    {
        for (int i = 0; i < _places.Size(); i++)
        {
            const float dx = _places[i].X() - x;
            const float dz = _places[i].Z() - z;
            if (dx * dx + dz * dz <= kNamedSpotRadius * kNamedSpotRadius)
            {
                return true;
            }
        }
        return false;
    }

  private:
    AutoArray<Vector3> _places;
};

} // namespace

bool BuildLegendSamples(const AutoArray<LegendZoneCandidate>& zones, int campIndex, AutoArray<LegendSpotSample>& out,
                        const AutoArray<LegendZoneCandidate>* avoid)
{
    out.Clear();
    if (!GLandscape)
    {
        return false; // headless: no terrain to stand on
    }
    WorldTerrainProvider terrain(zones);
    BuildLegendSamplesWith(&terrain, zones, campIndex, out, avoid);
    return out.Size() > 0;
}

bool LegendPlacementPrecheck(const AutoArray<LegendZoneCandidate>& zones, RString& diagnostic, float zoneArea)
{
    diagnostic = RString();

    AutoArray<LegendZoneCandidate> selected;
    int campIndex = -1;
    SelectLegendZones(zones, selected, campIndex);
    const int military = selected.Size() - (campIndex >= 0 ? 1 : 0);
    if (military <= 0)
    {
        diagnostic = RString("No enemy commander can be placed: this zone table has no military zone (every zone is "
                             "the CAMP or a CITY). Add at least one OUTPOST, AIRFIELD or SEAPORT zone.");
        return false;
    }

    // The same sampler and the same picker as the live pass, over bare geometry:
    // no terrain, so only the separation and Camp floors can refuse a stand.
    AutoArray<LegendSpotSample> samples;
    BuildLegendSamplesWith(nullptr, selected, campIndex, samples, &zones);
    const LegendPlacementResult result = PickLegendSpots(samples, 0);
    if (result.picked.Size() < LPC::Wanted)
    {
        diagnostic = Format("Only %d of %d enemy commanders fit: the %.0f to %.0f m rings around this template's %d "
                            "military zone(s) cannot hold %d stands %.0f m apart and %.0f m clear of the Camp. Move "
                            "the Camp farther out or add a military zone.",
                            result.picked.Size(), LPC::Wanted, LPC::Rings[0], LPC::Rings[LPC::NRings - 1], military,
                            LPC::Wanted, LPC::SameZoneFloor, LPC::CampFloor);
        return false;
    }

    // Placeable, but worth saying out loud: the commanders-stand-outside-the-
    // presence-radius reading only holds while the zone presence radius is
    // smaller than the inner ring.
    if (zoneArea >= LPC::Rings[0])
    {
        diagnostic = Format("This template's zoneArea (%.0f m) reaches the inner commander ring (%.0f m), so a "
                            "commander can stand inside his zone's presence radius and block its capture.",
                            zoneArea, LPC::Rings[0]);
    }
    return true;
}

// ---------------------------------------------------------------------------
// roles
// ---------------------------------------------------------------------------

const char* LegendRoleName(int role)
{
    switch (role)
    {
        case LRSniper:
            return "Sniper";
        case LRTank:
            return "Tank Commander";
        default:
            return "Commander";
    }
}

void ResolveLegendRoles(const LegendRoleCapability& cap, int count, AutoArray<LegendRoleResolution>& out)
{
    out.Clear();
    if (count <= 0)
    {
        return;
    }
    static const int kPreference[NLegendRoles] = {LRSniper, LRElite, LRTank};

    // one substitution each, decided once: a faction with no officer key fields
    // its best rifleman as the commander, and a faction with no rifleman rung at
    // all guards him with whatever the officer key named
    const RString elite = cap.eliteClass.GetLength() > 0 ? cap.eliteClass : cap.riflemanClass;
    const RString guard = cap.riflemanClass.GetLength() > 0 ? cap.riflemanClass : cap.eliteClass;

    for (int i = 0; i < count; i++)
    {
        LegendRoleResolution r;
        r.requested = kPreference[i % NLegendRoles];
        r.resolved = LRElite;
        r.bossClass = elite;
        r.guardClass = guard;
        r.guardCount = kEliteGuardCount;

        if (r.requested == LRSniper && cap.sniperClass.GetLength() > 0)
        {
            r.resolved = LRSniper;
            r.bossClass = cap.sniperClass;
            r.guardCount = kSniperGuardCount;
        }
        else if (r.requested == LRTank && cap.tankClass.GetLength() > 0)
        {
            r.resolved = LRTank;
            // the named commander is a man, not the hull: an officer body in the
            // highest seat the hull offers
            r.vehicleClass = cap.tankClass;
            r.crewClass = cap.tankCrewClass.GetLength() > 0 ? cap.tankCrewClass : guard;
            r.guardClass = r.crewClass;
            r.hasCommanderSeat = cap.tankHasCommander;
            r.hasGunnerSeat = cap.tankHasGunner;
            // driver always, plus gunner and commander when the hull has them;
            // the boss takes one of those seats, the rest are his crew
            const int seats = 1 + (cap.tankHasGunner ? 1 : 0) + (cap.tankHasCommander ? 1 : 0);
            r.guardCount = seats - 1;
        }
        // else: DEGRADED, and always in one step to Elite - the fields above are
        // already the Elite profile, so an unsupported sniper or tank simply
        // keeps them.  No cross-degrade and no second step: a repeated role is
        // the intended outcome.
        out.Add(r);
    }
}

namespace
{

// Mirrors ZoneRegistry::TierIndex (private there).  Kept in step deliberately:
// this is the only place a boss reads a WAR-LEVEL rung rather than the faction's
// best one.
int TierIndexOf(const FactionRecord& f, float warLevel)
{
    int tier = 0;
    for (int i = 0; i < f.tierThresholds.Size(); i++)
    {
        if (warLevel >= f.tierThresholds[i])
        {
            tier++;
        }
    }
    if (tier >= f.tiers.Size())
    {
        tier = f.tiers.Size() - 1;
    }
    return tier < 0 ? 0 : tier;
}

RString ValueOf(const FactionRecord& f, const char* key)
{
    for (int i = 0; i < f.values.Size(); i++)
    {
        if (stricmp(f.values[i].key, key) == 0)
        {
            return f.values[i].value;
        }
    }
    return RString();
}

// The faction's BEST rung, not the war level's.  A commander is campaign scale:
// resolution runs once, in the seeding tick, where the war level is always 1,
// and every shipped roster puts its sniper on the top rung only.
RString TopRung(const AutoArray<RString>& ladder)
{
    for (int i = ladder.Size() - 1; i >= 0; i--)
    {
        if (ladder[i].GetLength() > 0)
        {
            return ladder[i];
        }
    }
    return RString();
}

bool EndsWithCrew(const RString& cls)
{
    const int n = cls.GetLength();
    return n >= 4 && stricmp(((const char*)cls) + (n - 4), "Crew") == 0;
}

} // namespace

LegendRoleCapability ReadLegendCapability(const FactionRecord& faction, float warLevel)
{
    LegendRoleCapability cap;

    // Infantry comes off the resolved descriptor: the plan-15 pass already
    // blanked every rung the loaded package cannot spawn, so emptiness is the
    // only test needed here.
    cap.sniperClass = TopRung(faction.tiersSniper);
    cap.riflemanClass = TopRung(faction.tiers);
    const RString officer = ValueOf(faction, "officer");
    cap.eliteClass = officer.GetLength() > 0 ? officer : cap.riflemanClass;

    // Crew from the FACTION first: a hull's config `crew=` names whatever body
    // the hull's own addon shipped, which is routinely the wrong faction, and a
    // unit's side comes from the center it is created on, so nothing downstream
    // would ever catch it.
    RString factionCrew;
    for (int i = faction.tiers.Size() - 1; i >= 0; i--)
    {
        if (faction.tiers[i].GetLength() > 0 && EndsWithCrew(faction.tiers[i]))
        {
            factionCrew = faction.tiers[i];
            break;
        }
    }
    if (factionCrew.GetLength() == 0 && faction.tiers.Size() > 0)
    {
        factionCrew = faction.tiers[TierIndexOf(faction, warLevel)];
    }
    if (factionCrew.GetLength() == 0)
    {
        factionCrew = officer;
    }
    cap.tankCrewClass = factionCrew;

    // The hull is the one key that has to be re-probed: the descriptor pass
    // knows whether a vehicles[] rung can spawn, not whether it is a TANK.  The
    // highest surviving rung wins.  Everything here is guarded, because a
    // headless seeding tick with no world must degrade the third commander to
    // Elite rather than fault.
    if (!GWorld)
    {
        return cap;
    }
    for (int i = faction.vehicles.Size() - 1; i >= 0; i--)
    {
        const RString& cls = faction.vehicles[i];
        if (cls.GetLength() == 0)
        {
            continue;
        }
        const EntityType* type = VehicleTypes.New(cls);
        if (!type || type->IsAbstract())
        {
            continue;
        }
        const TankType* tank = dynamic_cast<const TankType*>(type);
        if (!tank)
        {
            continue;
        }
        cap.tankClass = cls;
        cap.tankHasCommander = tank->HasCommander();
        cap.tankHasGunner = tank->HasGunner();
        if (cap.tankCrewClass.GetLength() == 0)
        {
            // last resort only: the hull's own crew class, gated the same way
            // every other spawned class is
            const RString hullCrew = tank->GetCrew();
            ParsClassProbe probe;
            if (hullCrew.GetLength() > 0 && probe.Spawnable(hullCrew))
            {
                cap.tankCrewClass = hullCrew;
            }
        }
        break;
    }
    return cap;
}

} // namespace Guerrilla
} // namespace Poseidon
