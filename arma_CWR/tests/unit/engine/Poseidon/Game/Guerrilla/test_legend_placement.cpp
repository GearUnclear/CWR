// LegendPlacement: where the three enemy commanders stand, and what each of
// them IS - driven with no world at all.
//
// The whole point of the pure/engine split in LegendPlacement.hpp is that the
// interesting judgement is decidable from injected data.  These cases inject
// it.  The three things that are easy to get wrong and expensive to notice in
// game are pinned hard:
//
//   * THE HARD REJECTS ARE PER SAMPLE.  Water, the road net and the Camp floor
//     refuse one stand, never a whole ring: two candidates in the SAME zone
//     straddling the 400 m Camp floor must resolve differently, and the reason
//     code must name what actually refused them.
//   * THE RANK ORDER IS TOTAL.  It ends at the sample index, so no result may
//     depend on sort stability or on which order the caller happened to build
//     the array in.  A permuted input with the same geometry has to yield the
//     same STANDS.
//   * DEGRADATION IS ONE STEP AND ALWAYS TO ELITE.  A faction that cannot field
//     a sniper or a tank gets a second commander, never a third choice and
//     never a crash; the requested role survives so the dossier can say what
//     was asked for.
//
// The analytic precheck is transcribed against all five shipped zone tables
// here rather than in a template linter, because that is the sentence a
// template author sees when his island cannot seat three commanders.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/LegendPlacement.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp> // FactionRecord

#include <math.h>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

typedef LegendPlacementConstants LPC;

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

// ---------------------------------------------------------------------------
// sample fixtures
// ---------------------------------------------------------------------------

// One candidate stand.  Everything defaults to "acceptable": dry, off the road
// and comfortably clear of the Camp, so a case only has to state the one field
// it is about.
LegendSpotSample Spot(int zone, float x, float z, float campDist = 1000.0f)
{
    LegendSpotSample s;
    s.zone = zone;
    s.x = x;
    s.z = z;
    s.height = 12.0f;
    s.distFromZone = LPC::Rings[0];
    s.distFromCamp = campDist;
    return s;
}

// A ring of `count` stands around (cx, cz).  Neighbours on a 12-point 550 m
// ring sit ~285 m apart and opposite points 1100 m, so a ring is exactly the
// shape that makes the separation floor do visible work.
void AddRing(AutoArray<LegendSpotSample>& out, int zone, float cx, float cz, float radius, int count,
             float campDist = 1000.0f)
{
    for (int i = 0; i < count; i++)
    {
        const float angle = (2.0f * 3.14159265f) * (float)i / (float)count;
        out.Add(Spot(zone, cx + radius * cosf(angle), cz + radius * sinf(angle), campDist));
    }
}

float Gap(const LegendSpotSample& a, const LegendSpotSample& b)
{
    const float dx = a.x - b.x;
    const float dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

// ---------------------------------------------------------------------------
// zone fixtures
// ---------------------------------------------------------------------------

LegendZoneCandidate Zone(const char* name, const char* type, float x, float z, bool occupied = true)
{
    LegendZoneCandidate c;
    c.name = name;
    c.type = type;
    c.occupied = occupied;
    c.x = x;
    c.z = z;
    return c;
}

// The five shipped templates, transcribed from their description.ext Zones
// blocks (guerrilla-mode/mission/Guerrilla.*).  CITY zones are irrelevant to
// the ladder, so only the Camp and the military zones are carried.
AutoArray<LegendZoneCandidate> AbelZones()
{
    AutoArray<LegendZoneCandidate> z;
    z.Add(Zone("Camp", "CAMP", 7515, 5790, false));
    z.Add(Zone("Village", "CITY", 7117, 6069, false));
    z.Add(Zone("Outpost", "OUTPOST", 7600, 6300));
    return z;
}

AutoArray<LegendZoneCandidate> DemoZones()
{
    AutoArray<LegendZoneCandidate> z;
    z.Add(Zone("Camp", "CAMP", 6519.04f, 6473.68f, false));
    z.Add(Zone("Village", "CITY", 6300, 6600, false));
    z.Add(Zone("Outpost", "OUTPOST", 6750, 6300));
    return z;
}

AutoArray<LegendZoneCandidate> SinaiZones()
{
    AutoArray<LegendZoneCandidate> z;
    z.Add(Zone("Camp", "CAMP", 8350, 2950, false));
    z.Add(Zone("Wadi Checkpoint", "OUTPOST", 8300, 3500));
    z.Add(Zone("Ras Nasrani Outpost", "OUTPOST", 9100, 4600));
    z.Add(Zone("El Tor", "CITY", 6827, 2808, false));
    return z;
}

AutoArray<LegendZoneCandidate> Lebanon80Zones()
{
    AutoArray<LegendZoneCandidate> z;
    z.Add(Zone("Camp", "CAMP", 11500, 12000, false));
    z.Add(Zone("Litani Checkpoint", "OUTPOST", 12000, 12000));
    z.Add(Zone("Marjayoun Barracks", "OUTPOST", 13043, 12140));
    z.Add(Zone("Tyre", "CITY", 9300, 9800, false));
    return z;
}

AutoArray<LegendZoneCandidate> EdenZones()
{
    AutoArray<LegendZoneCandidate> z;
    z.Add(Zone("Camp", "CAMP", 2373, 7045, false));
    z.Add(Zone("Outpost 1", "OUTPOST", 7535, 8760));
    z.Add(Zone("Outpost 2", "OUTPOST", 11439, 1557));
    z.Add(Zone("Outpost 3", "OUTPOST", 9267, 4432));
    z.Add(Zone("Montignac", "CITY", 4935, 6994, false));
    return z;
}

// ---------------------------------------------------------------------------
// capability fixtures
// ---------------------------------------------------------------------------

LegendRoleCapability FullCapability()
{
    LegendRoleCapability cap;
    cap.sniperClass = "SoldierESniper";
    cap.eliteClass = "OfficerE";
    cap.riflemanClass = "SoldierEB";
    cap.tankClass = "T72";
    cap.tankCrewClass = "SoldierECrew";
    cap.tankHasCommander = true;
    cap.tankHasGunner = true;
    return cap;
}

// The shipped EAST descriptor (guerrilla-mode/config/guerrilla-factions.hpp),
// transcribed.  The sniper on the TOP rung only is the whole reason
// ReadLegendCapability reads the faction's best rung rather than the war
// level's: roles resolve in the seeding tick, where the war level is 1.
FactionRecord ShippedFaction(const char* side, const char* base, const char* grenadier, const char* crew,
                             const char* sniper, const char* officer)
{
    FactionRecord f;
    f.side = side;
    f.className = side;
    f.tiers.Add(RString(base));
    f.tiers.Add(RString(grenadier));
    f.tiers.Add(RString(crew));
    f.tierThresholds.Add(3.0f);
    f.tierThresholds.Add(5.0f);
    f.tiersSniper.Add(RString(""));
    f.tiersSniper.Add(RString(""));
    f.tiersSniper.Add(RString(sniper));
    FactionRecord::NamedValue v;
    v.key = "officer";
    v.value = officer;
    f.values.Add(v);
    return f;
}

} // namespace

// ===========================================================================
//  the pure picker
// ===========================================================================

TEST_CASE("Legend placement - a distinct zone beats a second stand off the same one",
          "[game][guerrilla][legends][placement]")
{
    // Two zones, 5 km apart, each offering a wide ring.  The first pick is
    // arbitrary-but-deterministic; the SECOND has to cross to the other zone,
    // because a distinct zone outranks every other preference there is.
    AutoArray<LegendSpotSample> samples;
    AddRing(samples, 0, 0, 0, 550, 12);
    AddRing(samples, 1, 5000, 0, 550, 12);

    const LegendPlacementResult res = PickLegendSpots(samples, 1234u);
    REQUIRE(res.picked.Size() == LPC::Wanted);
    CHECK(res.reason == LPOk);
    CHECK(!res.campRelaxed);
    CHECK(samples[res.picked[0]].zone != samples[res.picked[1]].zone);
    // and the third, with no distinct zone left, still clears the floor
    for (int a = 0; a < res.picked.Size(); a++)
    {
        for (int b = a + 1; b < res.picked.Size(); b++)
        {
            CHECK(Gap(samples[res.picked[a]], samples[res.picked[b]]) >= LPC::SameZoneFloor);
        }
    }
}

TEST_CASE("Legend placement - three stands off ONE zone all clear the separation floor",
          "[game][guerrilla][legends][placement]")
{
    // Abel and the Demo each ship exactly one military zone, so this is the
    // normal path, not the degenerate one.
    AutoArray<LegendSpotSample> samples;
    AddRing(samples, 0, 7600, 6300, 350, 12);
    AddRing(samples, 0, 7600, 6300, 550, 12);
    AddRing(samples, 0, 7600, 6300, 750, 12);
    AddRing(samples, 0, 7600, 6300, 950, 12);

    const LegendPlacementResult res = PickLegendSpots(samples, 7u);
    REQUIRE(res.picked.Size() == LPC::Wanted);
    CHECK(res.reason == LPOk);
    for (int a = 0; a < res.picked.Size(); a++)
    {
        CHECK(samples[res.picked[a]].zone == 0);
        for (int b = a + 1; b < res.picked.Size(); b++)
        {
            CHECK(Gap(samples[res.picked[a]], samples[res.picked[b]]) >= LPC::SameZoneFloor);
        }
    }
}

TEST_CASE("Legend placement - a crowded zone names the crowding, not the terrain",
          "[game][guerrilla][legends][placement]")
{
    // Four stands 100 m apart: dry, off the road, far from the Camp, and every
    // one of them inside the separation floor of the first.
    AutoArray<LegendSpotSample> samples;
    for (int i = 0; i < 4; i++)
    {
        samples.Add(Spot(0, (float)(i * 100), 0));
    }
    const LegendPlacementResult res = PickLegendSpots(samples, 3u);
    CHECK(res.picked.Size() == 1);
    CHECK(res.reason == LPTooCrowded);
}

TEST_CASE("Legend placement - water and roads are hard rejects that name themselves",
          "[game][guerrilla][legends][placement]")
{
    // Both sets clear every DISTANCE rule; only the terrain flag refuses them,
    // and the reason has to say which one.
    SECTION("every stand is in the sea")
    {
        AutoArray<LegendSpotSample> samples;
        AddRing(samples, 0, 0, 0, 950, 12);
        for (int i = 0; i < samples.Size(); i++)
        {
            samples[i].underwater = true;
            samples[i].height = -3.0f;
        }
        const LegendPlacementResult res = PickLegendSpots(samples, 11u);
        CHECK(res.picked.Size() == 0);
        CHECK(res.reason == LPAllUnderwater);
    }
    SECTION("every dry stand is on the road net")
    {
        AutoArray<LegendSpotSample> samples;
        AddRing(samples, 0, 0, 0, 950, 12);
        for (int i = 0; i < samples.Size(); i++)
        {
            samples[i].onRoad = true;
        }
        const LegendPlacementResult res = PickLegendSpots(samples, 11u);
        CHECK(res.picked.Size() == 0);
        CHECK(res.reason == LPAllOnRoad);
    }
    SECTION("no samples at all")
    {
        AutoArray<LegendSpotSample> samples;
        const LegendPlacementResult res = PickLegendSpots(samples, 11u);
        CHECK(res.picked.Size() == 0);
        CHECK(res.reason == LPNoZones);
    }
}

TEST_CASE("Legend placement - the Camp rule is PER SAMPLE, and relaxing it is not an error",
          "[game][guerrilla][legends][placement]")
{
    SECTION("inside the floor is refused, between floor and preference is taken")
    {
        AutoArray<LegendSpotSample> samples;
        AddRing(samples, 0, 0, 0, 950, 12, 500.0f); // 400 <= 500 < 800
        const LegendPlacementResult res = PickLegendSpots(samples, 5u);
        REQUIRE(res.picked.Size() == LPC::Wanted);
        CHECK(res.reason == LPOk);
        // the ROUTINE outcome on the shipped templates: Abel's Camp is 517 m
        // from its Outpost centre and the Demo's 289 m
        CHECK(res.campRelaxed);
    }
    SECTION("inside the floor is refused outright")
    {
        AutoArray<LegendSpotSample> samples;
        AddRing(samples, 0, 0, 0, 950, 12, 300.0f);
        const LegendPlacementResult res = PickLegendSpots(samples, 5u);
        CHECK(res.picked.Size() == 0);
        // dry and off-road: the terrain was never the problem
        CHECK(res.reason == LPTooCrowded);
    }
    SECTION("clear of the preference leaves campRelaxed down")
    {
        AutoArray<LegendSpotSample> samples;
        AddRing(samples, 0, 0, 0, 950, 12, 1200.0f);
        const LegendPlacementResult res = PickLegendSpots(samples, 5u);
        REQUIRE(res.picked.Size() == LPC::Wanted);
        CHECK(!res.campRelaxed);
    }
    SECTION("two stands in the SAME zone straddling the floor resolve differently")
    {
        // identical geometry, one field apart: this is the rule the sampler
        // would break if it measured the Camp once per ZONE
        AutoArray<LegendSpotSample> samples;
        samples.Add(Spot(0, 0, 0, 380.0f));   // inside the floor
        samples.Add(Spot(0, 900, 0, 420.0f)); // outside it
        const LegendPlacementResult res = PickLegendSpots(samples, 5u);
        REQUIRE(res.picked.Size() == 1);
        CHECK(res.picked[0] == 1);
        CHECK(res.campRelaxed);
    }
}

TEST_CASE("Legend placement - a named spot beats a bare one at equal geometry", "[game][guerrilla][legends][placement]")
{
    // Same zone, same Camp distance, same everything except the place name -
    // and the named one is deliberately the HIGHER index, so index order alone
    // cannot produce this answer.
    AutoArray<LegendSpotSample> samples;
    samples.Add(Spot(0, 0, 0));
    samples.Add(Spot(0, 900, 0));
    samples[1].named = true;

    const LegendPlacementResult res = PickLegendSpots(samples, 9u);
    REQUIRE(res.picked.Size() == 2);
    CHECK(res.picked[0] == 1);
    CHECK(res.picked[1] == 0);
}

TEST_CASE("Legend placement - the same geometry yields the same stands however it is ordered",
          "[game][guerrilla][legends][placement]")
{
    AutoArray<LegendSpotSample> samples;
    AddRing(samples, 0, 7600, 6300, 550, 12);
    AddRing(samples, 1, 9100, 4600, 750, 12);
    // Every stand gets its OWN Camp distance, all of them clear of the
    // preference.  Ranking a set in which two candidates agree on every
    // geometric key can only fall through to the sample index, and the index is
    // exactly what a permutation changes: this fixture asks the harder
    // question, whether the order leans on index for anything the geometry
    // already decides.
    for (int i = 0; i < samples.Size(); i++)
    {
        samples[i].distFromCamp = 900.0f + 7.0f * (float)i;
    }

    const LegendPlacementResult a = PickLegendSpots(samples, 4242u);
    const LegendPlacementResult b = PickLegendSpots(samples, 4242u);
    REQUIRE(a.picked.Size() == LPC::Wanted);
    REQUIRE(b.picked.Size() == a.picked.Size());
    for (int i = 0; i < a.picked.Size(); i++)
    {
        CHECK(a.picked[i] == b.picked[i]); // byte-identical indices
    }

    // A different SEED must not move a stand either: the ranking consults no
    // randomness, which is what keeps a save's persisted stand and a re-derived
    // one in agreement.
    const LegendPlacementResult seeded = PickLegendSpots(samples, 999999u);
    REQUIRE(seeded.picked.Size() == a.picked.Size());
    for (int i = 0; i < a.picked.Size(); i++)
    {
        CHECK(seeded.picked[i] == a.picked[i]);
    }

    // Reversed input, identical geometry: the INDICES must move (they name
    // different array slots) but the POSITIONS must not.  This is the assertion
    // that would catch a comparison leaning on sort stability.
    AutoArray<LegendSpotSample> reversed;
    for (int i = samples.Size() - 1; i >= 0; i--)
    {
        reversed.Add(samples[i]);
    }
    const LegendPlacementResult r = PickLegendSpots(reversed, 4242u);
    REQUIRE(r.picked.Size() == a.picked.Size());
    for (int i = 0; i < a.picked.Size(); i++)
    {
        CHECK(reversed[r.picked[i]].x == samples[a.picked[i]].x);
        CHECK(reversed[r.picked[i]].z == samples[a.picked[i]].z);
        CHECK(reversed[r.picked[i]].zone == samples[a.picked[i]].zone);
    }
}

// ===========================================================================
//  the zone ladder and the analytic precheck
// ===========================================================================

TEST_CASE("Legend placement - the ladder takes the occupier's military zones, then anybody's",
          "[game][guerrilla][legends][placement]")
{
    AutoArray<LegendZoneCandidate> all;
    all.Add(Zone("Camp", "CAMP", 0, 0, false));
    all.Add(Zone("Town", "CITY", 1000, 0, false));
    all.Add(Zone("Held", "OUTPOST", 2000, 0, true));
    all.Add(Zone("Lost", "OUTPOST", 3000, 0, false));

    AutoArray<LegendZoneCandidate> out;
    int campIndex = -1;
    SelectLegendZones(all, out, campIndex);
    // one occupied military zone is fewer than three, so the ladder falls
    // through to every military zone whoever owns it
    REQUIRE(out.Size() == 3);
    CHECK(Str(out[0].name) == "Held");
    CHECK(Str(out[1].name) == "Lost");
    // the Camp rides along LAST so the sampler, the picker and the caller all
    // index one array
    REQUIRE(campIndex == 2);
    CHECK(Str(out[campIndex].name) == "Camp");
    // and no CITY ever seats a commander
    for (int i = 0; i < out.Size(); i++)
    {
        CHECK(Str(out[i].type) != "CITY");
    }
}

TEST_CASE("Legend placement - a stand is never inside another zone's presence radius",
          "[game][guerrilla][legends][placement]")
{
    // The auto-seeded CITY zones are the live case (the Abel template seeds one
    // per named town), and the ladder throws them away before sampling - so
    // they have to come back as EXCLUSION points or a commander ends up 100 m
    // off a town centre, where he reads as occupier presence and pins that
    // town's support at the floor for the whole campaign.
    AutoArray<LegendZoneCandidate> all;
    all.Add(Zone("Camp", "CAMP", 7515, 5790, false));
    all.Add(Zone("Outpost", "OUTPOST", 7600, 6300));
    // a town sitting squarely on the inner ring, due north of the Outpost
    all.Add(Zone("Houdan", "CITY", 7600, 6650, false));

    AutoArray<LegendZoneCandidate> selected;
    int campIndex = -1;
    SelectLegendZones(all, selected, campIndex);

    AutoArray<LegendSpotSample> open, guarded;
    BuildLegendSamplesWith(nullptr, selected, campIndex, open);
    BuildLegendSamplesWith(nullptr, selected, campIndex, guarded, &all);
    CHECK(guarded.Size() < open.Size()); // the town refused at least one stand
    for (int i = 0; i < guarded.Size(); i++)
    {
        for (int z = 0; z < all.Size(); z++)
        {
            const float dx = guarded[i].x - all[z].x;
            const float dz = guarded[i].z - all[z].z;
            CHECK(sqrtf(dx * dx + dz * dz) >= LPC::ForeignZoneFloor);
        }
    }
    // and the template still seats three, which is the point of an exclusion
    // rather than a whole-zone veto
    const LegendPlacementResult res = PickLegendSpots(guarded, 17u);
    CHECK(res.picked.Size() == LPC::Wanted);
}

TEST_CASE("Legend placement - every shipped template can seat three commanders",
          "[game][guerrilla][legends][placement]")
{
    // The analytic precheck IS the setup diagnostic a template author reads,
    // so it is transcribed against all five shipped zone tables here.  Terrain
    // can still refuse a stand the analytic pass accepted; that is the live
    // pass's business and the Trident lane's.
    struct Case
    {
        const char* name;
        AutoArray<LegendZoneCandidate> zones;
    };
    std::vector<Case> cases;
    cases.push_back({"Abel", AbelZones()});
    cases.push_back({"Demo", DemoZones()});
    cases.push_back({"Sinai", SinaiZones()});
    cases.push_back({"Lebanon80", Lebanon80Zones()});
    cases.push_back({"Eden", EdenZones()});

    for (size_t i = 0; i < cases.size(); i++)
    {
        RString diagnostic;
        INFO("template " << cases[i].name);
        CHECK(LegendPlacementPrecheck(cases[i].zones, diagnostic));
        CHECK(Str(diagnostic).empty()); // silence is the success signal
    }
}

TEST_CASE("Legend placement - a template with no military zone says so in a sentence",
          "[game][guerrilla][legends][placement]")
{
    AutoArray<LegendZoneCandidate> zones;
    zones.Add(Zone("Camp", "CAMP", 5000, 5000, false));
    zones.Add(Zone("Town", "CITY", 5600, 5000, false));
    zones.Add(Zone("Port", "CITY", 4200, 5400, false));

    RString diagnostic;
    CHECK(!LegendPlacementPrecheck(zones, diagnostic));
    const std::string text = Str(diagnostic);
    CHECK(!text.empty());
    CHECK(text.find("military zone") != std::string::npos);
    CHECK(text.find("OUTPOST") != std::string::npos); // it says what to add
}

TEST_CASE("Legend placement - a template whose presence radius reaches the inner ring is called out",
          "[game][guerrilla][legends][placement]")
{
    // zoneArea is a template key and the inner ring is a hard 350 m.  A
    // template that raises it that far reverses the whole outside-the-presence-
    // radius reading: the commander would count as zone presence and could
    // block his own zone's capture.  Placeable, but never silent.
    AutoArray<LegendZoneCandidate> zones = AbelZones();
    RString quiet;
    REQUIRE(LegendPlacementPrecheck(zones, quiet, 150.0f));
    CHECK(Str(quiet).empty());

    RString loud;
    CHECK(LegendPlacementPrecheck(zones, loud, 400.0f)); // still placeable
    const std::string text = Str(loud);
    CHECK(!text.empty());
    CHECK(text.find("zoneArea") != std::string::npos);
}

// ===========================================================================
//  role resolution
// ===========================================================================

TEST_CASE("Legend placement - a full roster fields a sniper, a commander and a tank commander",
          "[game][guerrilla][legends][placement][legends]")
{
    AutoArray<LegendRoleResolution> out;
    ResolveLegendRoles(FullCapability(), 3, out);
    REQUIRE(out.Size() == 3);

    CHECK(out[0].requested == LRSniper);
    CHECK(out[0].resolved == LRSniper);
    CHECK(Str(out[0].bossClass) == "SoldierESniper");
    CHECK(out[0].guardCount == 1); // a sniper works with one spotter

    CHECK(out[1].requested == LRElite);
    CHECK(out[1].resolved == LRElite);
    CHECK(Str(out[1].bossClass) == "OfficerE");
    CHECK(Str(out[1].guardClass) == "SoldierEB");
    CHECK(out[1].guardCount == 4);
    CHECK(Str(out[1].vehicleClass).empty());

    CHECK(out[2].requested == LRTank);
    CHECK(out[2].resolved == LRTank);
    CHECK(Str(out[2].vehicleClass) == "T72");
    CHECK(Str(out[2].crewClass) == "SoldierECrew");
    CHECK(out[2].hasCommanderSeat);
    CHECK(out[2].hasGunnerSeat);
    CHECK(out[2].guardCount == 2); // driver + gunner; the commander is the boss

    CHECK(std::string(LegendRoleName(out[0].resolved)) == "Sniper");
    CHECK(std::string(LegendRoleName(out[1].resolved)) == "Commander");
    CHECK(std::string(LegendRoleName(out[2].resolved)) == "Tank Commander");
    CHECK(std::string(LegendRoleName(-1)) == "Commander"); // never renders empty
}

TEST_CASE("Legend placement - an unfieldable role degrades in ONE step, always to Elite",
          "[game][guerrilla][legends][placement][legends]")
{
    SECTION("no sniper class")
    {
        LegendRoleCapability cap = FullCapability();
        cap.sniperClass = RString();
        AutoArray<LegendRoleResolution> out;
        ResolveLegendRoles(cap, 3, out);
        REQUIRE(out.Size() == 3);
        // the ASK survives, so the dossier can say what was wanted
        CHECK(out[0].requested == LRSniper);
        CHECK(out[0].resolved == LRElite);
        CHECK(Str(out[0].bossClass) == Str(cap.eliteClass));
        CHECK(Str(out[0].guardClass) == Str(cap.riflemanClass));
        CHECK(out[0].guardCount == 4);
        CHECK(Str(out[0].vehicleClass).empty());
        // no cross-degrade: the tank commander is untouched
        CHECK(out[2].resolved == LRTank);
    }
    SECTION("no tank hull")
    {
        LegendRoleCapability cap = FullCapability();
        cap.tankClass = RString();
        AutoArray<LegendRoleResolution> out;
        ResolveLegendRoles(cap, 3, out);
        REQUIRE(out.Size() == 3);
        CHECK(out[2].requested == LRTank);
        CHECK(out[2].resolved == LRElite);
        CHECK(Str(out[2].bossClass) == Str(cap.eliteClass));
        CHECK(out[2].guardCount == 4);
        CHECK(Str(out[2].vehicleClass).empty());
        CHECK(out[0].resolved == LRSniper);
    }
    SECTION("neither: three commanders, and a repeat is the intended outcome")
    {
        LegendRoleCapability cap = FullCapability();
        cap.sniperClass = RString();
        cap.tankClass = RString();
        AutoArray<LegendRoleResolution> out;
        ResolveLegendRoles(cap, 3, out);
        REQUIRE(out.Size() == 3);
        for (int i = 0; i < out.Size(); i++)
        {
            CHECK(out[i].resolved == LRElite);
            CHECK(Str(out[i].bossClass) == "OfficerE");
            CHECK(out[i].guardCount == 4);
        }
    }
    SECTION("no officer key: the best rifleman wears the command")
    {
        LegendRoleCapability cap;
        cap.riflemanClass = "SoldierGB";
        AutoArray<LegendRoleResolution> out;
        ResolveLegendRoles(cap, 3, out);
        REQUIRE(out.Size() == 3);
        for (int i = 0; i < out.Size(); i++)
        {
            CHECK(out[i].resolved == LRElite);
            CHECK(Str(out[i].bossClass) == "SoldierGB");
            CHECK(Str(out[i].guardClass) == "SoldierGB");
        }
    }
    SECTION("a hull with no commander seat sizes the crew down")
    {
        LegendRoleCapability cap = FullCapability();
        cap.tankHasCommander = false;
        AutoArray<LegendRoleResolution> out;
        ResolveLegendRoles(cap, 3, out);
        REQUIRE(out.Size() == 3);
        CHECK(out[2].resolved == LRTank);
        CHECK(!out[2].hasCommanderSeat);
        CHECK(out[2].hasGunnerSeat);
        CHECK(out[2].guardCount == 1); // the boss gunners; one driver rides
    }
    SECTION("a hull with neither turret seat leaves the boss driving alone")
    {
        LegendRoleCapability cap = FullCapability();
        cap.tankHasCommander = false;
        cap.tankHasGunner = false;
        AutoArray<LegendRoleResolution> out;
        ResolveLegendRoles(cap, 3, out);
        REQUIRE(out.Size() == 3);
        CHECK(out[2].resolved == LRTank);
        CHECK(out[2].guardCount == 0);
    }
    SECTION("an empty capability resolves three commanders and does not crash")
    {
        LegendRoleCapability cap;
        AutoArray<LegendRoleResolution> out;
        ResolveLegendRoles(cap, 3, out);
        REQUIRE(out.Size() == 3);
        for (int i = 0; i < out.Size(); i++)
        {
            CHECK(out[i].resolved == LRElite);
            CHECK(Str(out[i].bossClass).empty()); // the spawn refuses it and logs
        }
        // and a zero count is not a special case
        AutoArray<LegendRoleResolution> none;
        ResolveLegendRoles(cap, 0, none);
        CHECK(none.Size() == 0);
    }
}

TEST_CASE("Legend placement - a commander reads the faction's BEST rung, not the war level's",
          "[game][guerrilla][legends][placement][legends]")
{
    // Roles resolve ONCE, in the seeding tick, where the war level is always 1
    // and TierIndex is therefore 0.  Every shipped roster puts its sniper on
    // the top rung only, so reading the war-level rung would mean no campaign
    // ever fielded a sniper at all.  There is no world here, so the tank half
    // early-returns and only the infantry reads are exercised - which is the
    // half this rule is about.
    const FactionRecord east =
        ShippedFaction("EAST", "SoldierEB", "SoldierEG", "SoldierECrew", "SoldierESniper", "OfficerE");
    const LegendRoleCapability capE = ReadLegendCapability(east, 1.0f);
    CHECK(Str(capE.sniperClass) == "SoldierESniper");
    CHECK(Str(capE.eliteClass) == "OfficerE");
    CHECK(Str(capE.riflemanClass) == "SoldierECrew"); // the top rung, war level 1 or not
    // the crew comes off the FACTION (the rung whose name ends in Crew), never
    // the hull's own config crew=, which routinely names another faction's body
    CHECK(Str(capE.tankCrewClass) == "SoldierECrew");

    const FactionRecord west =
        ShippedFaction("WEST", "SoldierWB", "SoldierWG", "SoldierWCrew", "SoldierWSniper", "OfficerW");
    const LegendRoleCapability capW = ReadLegendCapability(west, 1.0f);
    CHECK(Str(capW.sniperClass) == "SoldierWSniper");
    CHECK(Str(capW.eliteClass) == "OfficerW");
    CHECK(Str(capW.tankCrewClass) == "SoldierWCrew");

    // a roster with no sniper rung at all degrades, and says nothing else
    FactionRecord guer;
    guer.side = "GUER";
    guer.tiers.Add(RString("SoldierGB"));
    const LegendRoleCapability capG = ReadLegendCapability(guer, 1.0f);
    CHECK(Str(capG.sniperClass).empty());
    CHECK(Str(capG.eliteClass) == "SoldierGB"); // no officer key: the rifleman
    CHECK(Str(capG.tankCrewClass) == "SoldierGB");

    AutoArray<LegendRoleResolution> out;
    ResolveLegendRoles(capE, 3, out);
    REQUIRE(out.Size() == 3);
    CHECK(out[0].resolved == LRSniper); // the point of the whole rule
}
