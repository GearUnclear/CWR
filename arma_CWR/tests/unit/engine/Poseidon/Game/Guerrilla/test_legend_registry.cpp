// LegendRegistry: the campaign character registry, driven with no world.
//
// The registry OBSERVES the GM_COMP_* arrays and owns no progression, so every
// case here hands it an observation (LegendCompanionSnapshot) through
// PollCompanionsForTest and then asserts what it did with it.  The three things
// that are easy to get wrong and expensive to notice in game are pinned hard:
//
//   * the AWARD LATCH.  awardMask is persisted, so an award must fire exactly
//     once per threshold no matter how many times the same observation arrives,
//     whether both thresholds are crossed in one jump, or whether the campaign
//     was reloaded in between.
//   * the REPAINT GUARDS.  A poll that changed nothing must leave BOTH the
//     registry's revision and the JOURNAL's revision alone, or the open map
//     rebuilds once a second while the player is reading it.
//   * the SAVE DISCRIMINATOR.  The GuerrillaLegends block's ABSENCE is what
//     tells a load it is looking at a pre-registry save, so a registry holding
//     state must write its block even with zero rows, and a load that finds the
//     block must never re-derive.
//
// Journal::Instance() is process-wide (the registry writes its diary lines
// there), so every case that can write one clears it first and reads its
// revision through the same handle.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/Journal.hpp>
#include <Poseidon/Game/Guerrilla/LegendPlacement.hpp> // the injected boss world
#include <Poseidon/Game/Guerrilla/LegendRegistry.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Evaluator/express.hpp> // GGameState (the DERIVED-mode GM_COMP_* reads)

#include <cstring>
#include <filesystem>
#include <math.h>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

// ---------------------------------------------------------------------------
// fixtures
// ---------------------------------------------------------------------------

PlaceName MakePlace(const char* name, bool settlement)
{
    PlaceName p;
    p.key = name;
    p.name = name;
    p.settlement = settlement;
    return p;
}

HistoryInputs MakeInputs()
{
    HistoryInputs in;
    in.resistanceName = "FIA";
    in.occupierName = "Soviet Army";
    in.islandName = "Malden";
    in.settlements.Add(MakePlace("Houdan", true));
    in.settlements.Add(MakePlace("Chapoi", true));
    in.settlements.Add(MakePlace("Le Port", true));
    in.features.Add(MakePlace("Larche", false));
    in.zoneNames.Add(RString("Camp"));
    return in;
}

// One observation of the roster.  The registry never sees the script's arrays
// directly in these cases, which is what makes them world-free.
LegendCompanionSnapshot MakeSnapshot(const std::vector<const char*>& names, const std::vector<float>& xp,
                                     const std::vector<bool>& alive, int day = 1)
{
    LegendCompanionSnapshot s;
    for (size_t i = 0; i < names.size(); i++)
    {
        s.names.Add(RString(names[i]));
    }
    for (size_t i = 0; i < xp.size(); i++)
    {
        s.xp.Add(xp[i]);
    }
    for (size_t i = 0; i < alive.size(); i++)
    {
        s.alive.Add(alive[i]);
    }
    s.day = day;
    s.minuteOfDay = 8 * 60;
    return s;
}

LegendCompanionSnapshot OneCompanion(const char* name, float xp, bool alive = true, int day = 1)
{
    return MakeSnapshot({name}, {xp}, {alive}, day);
}

// A registry seeded the way SeedCampaign would have, minus the one GRandGen
// draw: the suite pins reproducibility (same seed in, same prose out), never a
// literal seed, because the live draw depends on what else drew that frame.
void Seed(LegendRegistry& registry, unsigned seed = 20260908u)
{
    registry.SeedForTest(seed, MakeInputs());
}

// Every global this file writes, handed back the way it was found.
struct ScopedVars
{
    std::vector<std::string> names;

    void Set(const char* name, const GameValue& value)
    {
        names.push_back(name);
        GGameState.VarSet(name, value);
    }
    ~ScopedVars()
    {
        for (const std::string& name : names)
        {
            GGameState.VarDelete(name.c_str());
        }
    }
};

void SetCompanionGlobals(ScopedVars& vars, const std::vector<const char*>& names, const std::vector<float>& xp,
                         const std::vector<bool>& alive)
{
    GameArrayType n, x, a;
    for (size_t i = 0; i < names.size(); i++)
    {
        n.Add(GameValue(RString(names[i])));
    }
    for (size_t i = 0; i < xp.size(); i++)
    {
        x.Add(GameValue(xp[i]));
    }
    for (size_t i = 0; i < alive.size(); i++)
    {
        a.Add(GameValue(alive[i]));
    }
    vars.Set("GM_COMP_NAMES", GameValue(n));
    vars.Set("GM_COMP_XP", GameValue(x));
    vars.Set("GM_COMP_ALIVE", GameValue(a));
}

// ---------------------------------------------------------------------------
// archive helpers: the WorldImpl block, reproduced so a case can drive both of
// its branches (subclass present -> load it; subclass absent -> derive).
// ---------------------------------------------------------------------------

std::filesystem::path ArchivePath(const char* name)
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    return dir / name;
}

// WorldImpl.cpp's save half: the gate is HasState(), never row emptiness.
void SaveRegistry(LegendRegistry& registry, const std::filesystem::path& path)
{
    ParamArchiveSave ar(WorldSerializeVersion);
    if (registry.HasState())
    {
        REQUIRE(ar.Serialize("GuerrillaLegends", registry, 14) == LSOK);
    }
    REQUIRE(ar.SaveBin(path.string().c_str()));
}

// WorldImpl.cpp's load half, both passes, both branches.  Returns true when the
// subclass was there, i.e. DeriveFromExistingSave did NOT run.
bool LoadRegistry(LegendRegistry& registry, const std::filesystem::path& path)
{
    ParamArchiveLoad ar;
    REQUIRE(ar.LoadBin(path.string().c_str()));
    registry.Clear();
    bool present = false;
    ar.FirstPass();
    if (ar.IsSubclass("GuerrillaLegends"))
    {
        present = true;
        REQUIRE(ar.Serialize("GuerrillaLegends", registry, 14) == LSOK);
    }
    ar.SecondPass();
    if (present)
    {
        REQUIRE(ar.Serialize("GuerrillaLegends", registry, 14) == LSOK);
    }
    else
    {
        registry.DeriveFromExistingSave();
    }
    return present;
}

int CountDeeds(const LegendRow& row, int kind)
{
    int n = 0;
    for (int i = 0; i < row.deeds.Size(); i++)
    {
        if (row.deeds[i].kind == kind)
        {
            n++;
        }
    }
    return n;
}

int CountEntriesFor(const Journal& journal, const char* charId)
{
    int n = 0;
    for (int i = 0; i < journal.EntryCount(); i++)
    {
        if (strcmp(journal.Entry(i).charId, charId) == 0)
        {
            n++;
        }
    }
    return n;
}

bool AnyEntryIncludes(const Journal& journal, const char* needle)
{
    for (int i = 0; i < journal.EntryCount(); i++)
    {
        if (strstr(journal.Entry(i).text, needle))
        {
            return true;
        }
    }
    return false;
}

bool IsPortraitFace(const RString& face)
{
    for (int i = 0; i < LegendRegistry::NPortraitFaces; i++)
    {
        if (strcmp(face, LegendRegistry::kPortraitFaces[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

int FilledSlots(const LegendRow& row)
{
    int n = 0;
    n += row.prefix.GetLength() > 0 ? 1 : 0;
    n += row.describer.GetLength() > 0 ? 1 : 0;
    n += row.title.GetLength() > 0 ? 1 : 0;
    return n;
}

} // namespace

// ===========================================================================
//  lifecycle
// ===========================================================================

TEST_CASE("Legend registry - a fresh registry holds nothing and InitMission does not seed",
          "[game][guerrilla][legends][registry]")
{
    LegendRegistry registry;
    CHECK(!registry.IsSeeded());
    CHECK(!registry.HasState());
    CHECK(!registry.HasProgression());
    CHECK(registry.RowCount() == 0);
    CHECK(registry.Seed() == 0);
    CHECK(!registry.History().Present());

    // InitMission clears and reads no config of its own; SEEDING is the first
    // active Simulate tick's job, not InitMission's.
    Seed(registry);
    REQUIRE(registry.RowCount() > 0);
    registry.InitMission();
    CHECK(registry.RowCount() == 0);
    CHECK(!registry.IsSeeded());
    CHECK(!registry.HasState());
    CHECK(!registry.HasProgression());
    CHECK(!registry.History().Present());
    CHECK(registry.Revision() == 0);
}

TEST_CASE("Legend registry - seeding generates the history and pre-rolls three enemy Legends",
          "[game][guerrilla][legends][registry]")
{
    LegendRegistry registry;
    Seed(registry);

    CHECK(registry.IsSeeded());
    CHECK(registry.HasState());
    CHECK(registry.HasProgression());
    CHECK(registry.Seed() != 0);
    CHECK((registry.Seed() & 1u) == 1u); // the live draw is always odd
    REQUIRE(registry.History().Present());
    CHECK(registry.History().openingPage1.GetLength() > 0);
    CHECK(registry.History().openingPage2.GetLength() > 0);

    REQUIRE(registry.RowCount() == LegendRegistry::kBossCount);
    std::vector<std::string> firsts, lasts;
    for (int i = 0; i < registry.RowCount(); i++)
    {
        const LegendRow& row = registry.Row(i);
        CHECK(row.kind == LKBoss);
        CHECK(row.compIndex == -1);
        CHECK(row.first.GetLength() > 0);
        CHECK(row.last.GetLength() > 0);
        CHECK(row.legend);
        // both bits latched at creation, so the companion award machinery can
        // never fire on a boss row
        CHECK(row.awardMask == (LAFirst | LASecond));
        CHECK(row.tone == ToneHostile);
        CHECK(Str(row.role) == "Commander"); // Change 3's sentinel
        CHECK(FilledSlots(row) == 1);        // notorious from the start, but only one word
        CHECK(IsPortraitFace(row.face));
        CHECK(registry.DisplayName(row).GetLength() > 0);
        CHECK(row.bio.GetLength() > 0);
        firsts.push_back(Str(row.first));
        lasts.push_back(Str(row.last));
    }
    // pairwise distinct: three independent draws from one ten-name pool share a
    // first name in better than a quarter of campaigns without the walk
    CHECK(firsts[0] != firsts[1]);
    CHECK(firsts[0] != firsts[2]);
    CHECK(firsts[1] != firsts[2]);
    CHECK(lasts[0] != lasts[1]);
    CHECK(lasts[0] != lasts[2]);
    CHECK(lasts[1] != lasts[2]);

    // reproducibility, not a literal seed: the same seed in, the same prose out
    LegendRegistry again;
    Seed(again);
    REQUIRE(again.RowCount() == registry.RowCount());
    CHECK(Str(again.History().openingPage1) == Str(registry.History().openingPage1));
    for (int i = 0; i < registry.RowCount(); i++)
    {
        CHECK(Str(again.DisplayName(again.Row(i))) == Str(registry.DisplayName(registry.Row(i))));
    }

    // a different seed moves the identities
    LegendRegistry other;
    other.SeedForTest(777u, MakeInputs());
    bool moved = Str(other.History().openingPage1) != Str(registry.History().openingPage1);
    for (int i = 0; i < other.RowCount() && !moved; i++)
    {
        moved = Str(other.DisplayName(other.Row(i))) != Str(registry.DisplayName(registry.Row(i)));
    }
    CHECK(moved);
}

TEST_CASE("Legend registry - an inactive zone registry seeds nothing and writes nothing",
          "[game][guerrilla][legends][registry]")
{
    // The inactive-registry early return is what keeps ordinary missions and
    // intros untouched.  Without it, every mission in the game would draw a
    // campaign seed and open a diary.
    ZoneRegistry::Instance().Clear();
    REQUIRE(!ZoneRegistry::Instance().IsActive());
    Journal::Instance().Clear();

    LegendRegistry registry;
    registry.InitMission();
    for (int i = 0; i < 5; i++)
    {
        registry.Simulate(1.0f);
    }
    CHECK(registry.RowCount() == 0);
    CHECK(registry.Seed() == 0);
    CHECK(!registry.IsSeeded());
    CHECK(!registry.HasState());
    CHECK(Journal::Instance().IsEmpty());
}

// ===========================================================================
//  the first observation of a companion is a BASELINE, not news
// ===========================================================================

TEST_CASE("Legend registry - the poll that creates a row writes no deed and no diary line",
          "[game][guerrilla][legends][registry]")
{
    // The shipped core seeds Petra at XP 100 / CORPORAL, i.e. ladder index 1 on
    // the very first observation.  A row created with rankSeen == -1 would open
    // every brand-new campaign with a spurious "Petra promoted to CORPORAL."
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry);
    const unsigned journalRev = Journal::Instance().Revision();

    registry.PollCompanionsForTest(OneCompanion("Petra", 100.0f));

    const int r = registry.FindByCompIndex(0);
    REQUIRE(r >= 0);
    const LegendRow& row = registry.Row(r);
    CHECK(row.rankSeen == 1); // seeded from the creating observation
    CHECK(row.xpSeen == 100.0f);
    CHECK(row.deeds.Size() == 0);
    CHECK(Journal::Instance().IsEmpty());
    CHECK(Journal::Instance().Revision() != journalRev); // a new row is still a repaint

    // and a companion seeded ABOVE a threshold gets the earned words silently
    LegendRegistry high;
    Seed(high);
    Journal::Instance().Clear();
    high.PollCompanionsForTest(OneCompanion("Yazan", 2000.0f));
    const int y = high.FindByCompIndex(0);
    REQUIRE(y >= 0);
    CHECK(high.Row(y).awardMask == (LAFirst | LASecond));
    CHECK(high.Row(y).deeds.Size() == 0);
    CHECK(Journal::Instance().EntryCount() == 0);
}

// ===========================================================================
//  awards
// ===========================================================================

TEST_CASE("Legend registry - each award fires exactly once, at its own threshold",
          "[game][guerrilla][legends][registry]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry);

    // sub-threshold: the row exists, no slot is awarded
    registry.PollCompanionsForTest(OneCompanion("Petra", 249.0f));
    const int r = registry.FindByCompIndex(0);
    REQUIRE(r >= 0);
    const RString id = registry.Row(r).id;
    const std::string base = Str(registry.Row(r).baseName);
    CHECK(registry.Row(r).awardMask == 0);
    CHECK(FilledSlots(registry.Row(r)) == 0);
    const std::string plain = Str(registry.DisplayName(registry.Row(r)));

    Journal::Instance().Clear();
    const unsigned rev = registry.Revision();
    const unsigned journalRev = Journal::Instance().Revision();

    // SERGEANT: the first award, plus the promotion line the ladder step owes
    registry.PollCompanionsForTest(OneCompanion("Petra", 250.0f));
    CHECK(registry.Row(r).awardMask == LAFirst);
    CHECK(FilledSlots(registry.Row(r)) == 1);
    CHECK(!registry.Row(r).legend);
    CHECK(registry.Revision() == rev + 1); // one repaint for the whole poll
    CHECK(Journal::Instance().Revision() > journalRev);
    // D2.5's two triggers: the ladder step and the award
    REQUIRE(Journal::Instance().EntryCount() == 2);
    CHECK(AnyEntryIncludes(Journal::Instance(), "promoted to SERGEANT"));
    CHECK(AnyEntryIncludes(Journal::Instance(), "is now known as"));
    CHECK(CountEntriesFor(Journal::Instance(), id) == 2); // every line carries the id
    CHECK(Journal::Instance().Entry(0).kind == JKGood);
    const std::string awarded = Str(registry.DisplayName(registry.Row(r)));
    CHECK(awarded != plain);
    CHECK(awarded.find(base) != std::string::npos); // the base name survives the award
    CHECK(CountDeeds(registry.Row(r), LDAward) == 1);
    CHECK(CountDeeds(registry.Row(r), LDPromotion) == 1);

    // the identical observation again: the latch holds, nothing repaints
    const unsigned rev2 = registry.Revision();
    const unsigned journalRev2 = Journal::Instance().Revision();
    registry.PollCompanionsForTest(OneCompanion("Petra", 250.0f));
    registry.PollCompanionsForTest(OneCompanion("Petra", 250.0f));
    CHECK(registry.Row(r).awardMask == LAFirst);
    CHECK(Str(registry.DisplayName(registry.Row(r))) == awarded);
    CHECK(registry.Revision() == rev2);
    CHECK(Journal::Instance().Revision() == journalRev2);
    CHECK(Journal::Instance().EntryCount() == 2);

    // COLONEL: the second award takes a DIFFERENT slot and makes a Legend
    Journal::Instance().Clear();
    registry.PollCompanionsForTest(OneCompanion("Petra", 1900.0f));
    CHECK(registry.Row(r).awardMask == (LAFirst | LASecond));
    CHECK(FilledSlots(registry.Row(r)) == 2); // added, never swapped
    CHECK(registry.Row(r).legend);
    CHECK(AnyEntryIncludes(Journal::Instance(), "has become a legend of the resistance"));
    CHECK(CountDeeds(registry.Row(r), LDAward) == 2);
    const std::string legend = Str(registry.DisplayName(registry.Row(r)));
    CHECK(legend != awarded);
    CHECK(legend.find(base) != std::string::npos);

    // past the top of the ladder nothing more can fire
    Journal::Instance().Clear();
    const unsigned rev3 = registry.Revision();
    registry.PollCompanionsForTest(OneCompanion("Petra", 99999.0f));
    CHECK(registry.Row(r).awardMask == (LAFirst | LASecond));
    CHECK(Str(registry.DisplayName(registry.Row(r))) == legend);
    CHECK(registry.Revision() == rev3);
    CHECK(Journal::Instance().EntryCount() == 0);
}

TEST_CASE("Legend registry - both thresholds crossed in one jump award both slots, in order",
          "[game][guerrilla][legends][registry]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry);

    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f));
    const int r = registry.FindByCompIndex(0);
    REQUIRE(r >= 0);
    const RString id = registry.Row(r).id;
    Journal::Instance().Clear();

    // one XP jump straight past both thresholds
    registry.PollCompanionsForTest(OneCompanion("Petra", 2000.0f));
    CHECK(registry.Row(r).awardMask == 3);
    CHECK(registry.Row(r).legend);
    CHECK(FilledSlots(registry.Row(r)) == 2);
    CHECK(CountDeeds(registry.Row(r), LDAward) == 2);
    // exactly two award lines, plus the one promotion line for the ladder step
    CHECK(AnyEntryIncludes(Journal::Instance(), "is now known as"));
    CHECK(AnyEntryIncludes(Journal::Instance(), "has become a legend of the resistance"));
    CHECK(AnyEntryIncludes(Journal::Instance(), "promoted to COLONEL"));
    CHECK(Journal::Instance().EntryCount() == 3);
    CHECK(CountEntriesFor(Journal::Instance(), id) == 3);

    // the same jump again is a no-op
    const unsigned rev = registry.Revision();
    registry.PollCompanionsForTest(OneCompanion("Petra", 2000.0f));
    CHECK(registry.Row(r).awardMask == 3);
    CHECK(registry.Revision() == rev);
    CHECK(Journal::Instance().EntryCount() == 3);
}

TEST_CASE("Legend registry - a derived campaign records deeds but never writes a diary line",
          "[game][guerrilla][legends][registry]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry);
    registry.SetProgressionForTest(false);

    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f));
    const int r = registry.FindByCompIndex(0);
    REQUIRE(r >= 0);
    const std::string base = Str(registry.Row(r).baseName);
    CHECK(Str(registry.DisplayName(registry.Row(r))) == base); // no generated surname

    registry.PollCompanionsForTest(OneCompanion("Petra", 2000.0f));
    CHECK(registry.Row(r).awardMask == 0); // no award ever, in either direction
    CHECK(FilledSlots(registry.Row(r)) == 0);
    CHECK(!registry.Row(r).legend);
    CHECK(Str(registry.DisplayName(registry.Row(r))) == base);
    // deeds still accumulate: the dossier is the whole point of derived mode
    CHECK(CountDeeds(registry.Row(r), LDPromotion) == 1);
    CHECK(Journal::Instance().EntryCount() == 0);

    // and death is recorded as a deed with no diary line either
    registry.PollCompanionsForTest(OneCompanion("Petra", 2000.0f, false));
    CHECK(!registry.Row(r).alive);
    CHECK(CountDeeds(registry.Row(r), LDDeath) == 1);
    CHECK(Journal::Instance().EntryCount() == 0);
}

// ===========================================================================
//  repaint guards
// ===========================================================================

TEST_CASE("Legend registry - an unchanged observation repaints nothing", "[game][guerrilla][legends][registry]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry);
    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f));

    const unsigned rev = registry.Revision();
    const unsigned journalRev = Journal::Instance().Revision();

    // twice identical
    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f));
    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f));
    CHECK(registry.Revision() == rev);
    CHECK(Journal::Instance().Revision() == journalRev);

    // an XP delta that does not move the ladder is bookkeeping, not news
    registry.PollCompanionsForTest(OneCompanion("Petra", 40.0f));
    registry.PollCompanionsForTest(OneCompanion("Petra", 99.0f));
    CHECK(registry.Revision() == rev);
    CHECK(Journal::Instance().Revision() == journalRev);
    const int r = registry.FindByCompIndex(0);
    REQUIRE(r >= 0);
    CHECK(registry.Row(r).xpSeen == 99.0f); // still observed, just silently

    // a rank change below the first award threshold moves both by exactly one
    registry.PollCompanionsForTest(OneCompanion("Petra", 100.0f));
    CHECK(registry.Revision() == rev + 1);
    CHECK(Journal::Instance().Revision() == journalRev + 1);
    CHECK(Journal::Instance().EntryCount() == 1);
}

TEST_CASE("Legend registry - Touch separates the registry revision from the journal's",
          "[game][guerrilla][legends][registry]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    const unsigned rev = registry.Revision();
    const unsigned journalRev = Journal::Instance().Revision();

    registry.Touch(false);
    CHECK(registry.Revision() == rev + 1);
    CHECK(Journal::Instance().Revision() == journalRev);

    registry.Touch(true);
    CHECK(registry.Revision() == rev + 2);
    CHECK(Journal::Instance().Revision() == journalRev + 1);
}

// ===========================================================================
//  death, milestones and the deed cap
// ===========================================================================

TEST_CASE("Legend registry - death latches once and writes one danger line", "[game][guerrilla][legends][registry]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry);
    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f));
    const int r = registry.FindByCompIndex(0);
    REQUIRE(r >= 0);
    const RString id = registry.Row(r).id;
    REQUIRE(registry.Row(r).alive);
    Journal::Instance().Clear();

    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f, false, 4));
    CHECK(!registry.Row(r).alive);
    CHECK(registry.Row(r).deathDay == 4);
    CHECK(registry.Row(r).deathMinute == 8 * 60);
    CHECK(CountDeeds(registry.Row(r), LDDeath) == 1);
    REQUIRE(Journal::Instance().EntryCount() == 1);
    CHECK(Journal::Instance().Entry(0).kind == JKDanger);
    CHECK(Str(Journal::Instance().Entry(0).charId) == Str(id));
    CHECK(AnyEntryIncludes(Journal::Instance(), "has fallen"));

    // a second identical poll adds nothing: death is a one-way latch
    const unsigned rev = registry.Revision();
    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f, false, 4));
    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f, false, 9));
    CHECK(CountDeeds(registry.Row(r), LDDeath) == 1);
    CHECK(Journal::Instance().EntryCount() == 1);
    CHECK(registry.Revision() == rev);
    CHECK(registry.Row(r).deathDay == 4);
}

TEST_CASE("Legend registry - survival milestones are deeds only", "[game][guerrilla][legends][registry]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry);
    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f, true, 1));
    const int r = registry.FindByCompIndex(0);
    REQUIRE(r >= 0);
    Journal::Instance().Clear();

    for (int m = 0; m < LegendRegistry::NMilestones; m++)
    {
        const int day = LegendRegistry::kMilestoneDays[m];
        const int before = CountDeeds(registry.Row(r), LDMilestone);
        registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f, true, day));
        CHECK(CountDeeds(registry.Row(r), LDMilestone) == before + 1);
        CHECK(registry.Row(r).daysSurvived == day);
        // a day past the milestone adds nothing at all
        const unsigned rev = registry.Revision();
        registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f, true, day + 1));
        CHECK(CountDeeds(registry.Row(r), LDMilestone) == before + 1);
        CHECK(registry.Revision() == rev);
        // and the day counter never walks backwards
        registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f, true, 1));
        CHECK(registry.Row(r).daysSurvived == day + 1);
    }
    CHECK(CountDeeds(registry.Row(r), LDMilestone) == LegendRegistry::NMilestones);
    // not one diary line for any of it: the record must not fill with bookkeeping
    CHECK(Journal::Instance().EntryCount() == 0);
}

TEST_CASE("Legend registry - the deed cap evicts bookkeeping, never an award or the death line",
          "[game][guerrilla][legends][registry]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry);
    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f));
    const int r = registry.FindByCompIndex(0);
    REQUIRE(r >= 0);

    // both awards, then the death line, then a long tail of promotions.  A
    // demotion is silent bookkeeping, so cycling the XP re-fires the ladder
    // step and is the cheapest way to overrun the cap.
    registry.PollCompanionsForTest(OneCompanion("Petra", 2000.0f));
    registry.PollCompanionsForTest(OneCompanion("Petra", 2000.0f, false, 2));
    REQUIRE(CountDeeds(registry.Row(r), LDAward) == 2);
    REQUIRE(CountDeeds(registry.Row(r), LDDeath) == 1);

    for (int i = 0; i < 40; i++)
    {
        registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f, false, 2));
        registry.PollCompanionsForTest(OneCompanion("Petra", 2000.0f, false, 2));
    }
    CHECK(registry.Row(r).deeds.Size() == LegendRegistry::kMaxDeeds);
    CHECK(CountDeeds(registry.Row(r), LDAward) == 2);
    CHECK(CountDeeds(registry.Row(r), LDDeath) == 1);
    CHECK(CountDeeds(registry.Row(r), LDPromotion) == LegendRegistry::kMaxDeeds - 3);
}

// ===========================================================================
//  ids and faces
// ===========================================================================

TEST_CASE("Legend registry - the id scheme is three tokens and is written once", "[game][guerrilla][legends][registry]")
{
    LegendRegistry registry;
    Seed(registry);
    const char* const kLong = "Aaaaaaaaaabbbbbbbbbbccccccccccdddddddddd"; // 40 chars
    registry.PollCompanionsForTest(
        MakeSnapshot({"Petra", "Abu-Salim", "!!!", kLong}, {0.0f, 0.0f, 0.0f, 0.0f}, {true, true, true, true}));

    REQUIRE(registry.FindByCompIndex(0) >= 0);
    CHECK(Str(registry.Row(registry.FindByCompIndex(0)).id) == "comp_0_petra");
    CHECK(Str(registry.Row(registry.FindByCompIndex(1)).id) == "comp_1_abusalim");
    CHECK(Str(registry.Row(registry.FindByCompIndex(2)).id) == "comp_2_x");
    CHECK(Str(registry.Row(registry.FindByCompIndex(3)).id) == "comp_3_aaaaaaaaaabb"); // 12 slug chars

    // an id is [A-Za-z0-9_] only and has exactly three underscore-separated
    // tokens, which is what keeps a Render continuation page (GM_WHO_<id>_2)
    // out of the id namespace
    for (int i = 0; i < registry.RowCount(); i++)
    {
        const std::string id = Str(registry.Row(i).id);
        int underscores = 0;
        for (char c : id)
        {
            const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
            CHECK(ok);
            underscores += c == '_' ? 1 : 0;
        }
        CHECK(underscores == (registry.Row(i).kind == LKCompanion ? 2 : 1));
    }

    // renaming a companion moves the base name, never the id
    registry.PollCompanionsForTest(
        MakeSnapshot({"Petra Renamed", "Abu-Salim", "!!!", kLong}, {0.0f, 0.0f, 0.0f, 0.0f}, {true, true, true, true}));
    const int r = registry.FindByCompIndex(0);
    REQUIRE(r >= 0);
    CHECK(Str(registry.Row(r).id) == "comp_0_petra");
    CHECK(Str(registry.Row(r).baseName) == "Petra Renamed");
    CHECK(registry.FindById("comp_0_petra") == r);

    // rows stay companions-first, so gmLegendInfo 0 is a companion for the
    // life of the campaign
    for (int i = 1; i < registry.RowCount(); i++)
    {
        CHECK(!(registry.Row(i).kind == LKCompanion && registry.Row(i - 1).kind == LKBoss));
    }
}

TEST_CASE("Legend registry - the rolled face is one of the four and never moves",
          "[game][guerrilla][legends][registry]")
{
    LegendRegistry registry;
    Seed(registry);
    registry.PollCompanionsForTest(OneCompanion("Petra", 0.0f));
    const int r = registry.FindByCompIndex(0);
    REQUIRE(r >= 0);
    const std::string face = Str(registry.Row(r).face);
    CHECK(IsPortraitFace(registry.Row(r).face));

    registry.PollCompanionsForTest(OneCompanion("Petra", 300.0f));
    registry.PollCompanionsForTest(OneCompanion("Petra", 2000.0f));
    CHECK(Str(registry.Row(r).face) == face);

    const std::filesystem::path path = ArchivePath("legend-face.bin");
    SaveRegistry(registry, path);
    LegendRegistry loaded;
    REQUIRE(LoadRegistry(loaded, path));
    const int lr = loaded.FindByCompIndex(0);
    REQUIRE(lr >= 0);
    CHECK(Str(loaded.Row(lr).face) == face);
    std::filesystem::remove(path);
}

// ===========================================================================
//  save and load
// ===========================================================================

TEST_CASE("Legend registry - a seeded registry with no rows still writes its block",
          "[game][guerrilla][legends][registry][save][load]")
{
    // The block's ABSENCE is the pre-registry discriminator, so the gate is
    // HasState(), never row emptiness: an empty AutoArray writes no subclass,
    // and a campaign that has not met a companion yet must not be mistaken for
    // an old save and silently re-derived.
    ScopedVars vars; // no GM_COMP_* at all: derive produces zero rows
    LegendRegistry registry;
    registry.DeriveFromExistingSave();
    REQUIRE(registry.RowCount() == 0);
    REQUIRE(registry.HasState());
    CHECK(!registry.IsSeeded());
    CHECK(!registry.HasProgression());

    const std::filesystem::path path = ArchivePath("legend-empty.bin");
    SaveRegistry(registry, path);

    LegendRegistry loaded;
    CHECK(LoadRegistry(loaded, path)); // the subclass is there: no second derive
    CHECK(loaded.HasState());
    CHECK(!loaded.HasProgression());
    CHECK(loaded.RowCount() == 0);
    std::filesystem::remove(path);
}

TEST_CASE("Legend registry - a campaign round trips its seed, history and rows in order",
          "[game][guerrilla][legends][registry][save][load]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry);
    registry.PollCompanionsForTest(MakeSnapshot({"Petra", "Yazan"}, {250.0f, 0.0f}, {true, true}));

    const unsigned seed = registry.Seed();
    const int rows = registry.RowCount();
    REQUIRE(rows == 2 + LegendRegistry::kBossCount);
    std::vector<std::string> ids, names, faces;
    for (int i = 0; i < rows; i++)
    {
        ids.push_back(Str(registry.Row(i).id));
        names.push_back(Str(registry.DisplayName(registry.Row(i))));
        faces.push_back(Str(registry.Row(i).face));
    }
    const std::string opening1 = Str(registry.History().openingPage1);
    const std::string opening2 = Str(registry.History().openingPage2);
    const int petra = registry.FindByCompIndex(0);
    REQUIRE(petra >= 0);
    const int petraDeeds = registry.Row(petra).deeds.Size();
    const int petraAwards = registry.Row(petra).awardMask;
    REQUIRE(petraAwards == LAFirst);

    const std::filesystem::path path = ArchivePath("legend-campaign.bin");
    SaveRegistry(registry, path);

    Journal::Instance().Clear();
    const unsigned journalRev = Journal::Instance().Revision();
    LegendRegistry loaded;
    REQUIRE(LoadRegistry(loaded, path)); // DeriveFromExistingSave did NOT run
    CHECK(loaded.HasProgression());
    CHECK(loaded.IsSeeded());
    CHECK(loaded.Seed() == seed);
    REQUIRE(loaded.History().Present());
    CHECK(Str(loaded.History().openingPage1) == opening1);
    CHECK(Str(loaded.History().openingPage2) == opening2);
    for (int k = 0; k < kHistoryEvents; k++)
    {
        CHECK(loaded.History().eventIndex[k] == registry.History().eventIndex[k]);
        CHECK(Str(loaded.History().eventText[k]) == Str(registry.History().eventText[k]));
        CHECK(Str(loaded.History().eventPlace[k]) == Str(registry.History().eventPlace[k]));
    }
    REQUIRE(loaded.RowCount() == rows);
    for (int i = 0; i < rows; i++)
    {
        CHECK(Str(loaded.Row(i).id) == ids[i]); // IN ORDER: ParamArchive walks by index
        CHECK(Str(loaded.DisplayName(loaded.Row(i))) == names[i]);
        CHECK(Str(loaded.Row(i).face) == faces[i]);
    }
    const int loadedPetra = loaded.FindByCompIndex(0);
    REQUIRE(loadedPetra >= 0);
    CHECK(loaded.Row(loadedPetra).deeds.Size() == petraDeeds);
    CHECK(loaded.Row(loadedPetra).awardMask == petraAwards);
    // no live bodies to re-bind, so the load repaints nothing
    CHECK(Journal::Instance().Revision() == journalRev);

    // a threshold already crossed before the save never re-awards after it
    const std::string wasName = Str(loaded.DisplayName(loaded.Row(loadedPetra)));
    Journal::Instance().Clear();
    loaded.PollCompanionsForTest(MakeSnapshot({"Petra", "Yazan"}, {250.0f, 0.0f}, {true, true}));
    CHECK(loaded.Row(loadedPetra).awardMask == petraAwards);
    CHECK(Str(loaded.DisplayName(loaded.Row(loadedPetra))) == wasName);
    CHECK(Journal::Instance().EntryCount() == 0);

    // and a second round trip keeps everything it kept the first time
    const std::filesystem::path path2 = ArchivePath("legend-campaign2.bin");
    SaveRegistry(loaded, path2);
    LegendRegistry twice;
    REQUIRE(LoadRegistry(twice, path2));
    CHECK(twice.HasProgression());
    CHECK(twice.Seed() == seed);
    REQUIRE(twice.RowCount() == rows);
    CHECK(Str(twice.DisplayName(twice.Row(loadedPetra))) == wasName);
    CHECK(twice.Row(loadedPetra).awardMask == petraAwards);
    std::filesystem::remove(path);
    std::filesystem::remove(path2);
}

TEST_CASE("Legend registry - a pre-registry save derives dossiers and stays derived",
          "[game][guerrilla][legends][registry][save][load]")
{
    Journal::Instance().Clear();
    ScopedVars vars;
    SetCompanionGlobals(vars, {"Petra", "Ivan"}, {250.0f, 0.0f}, {true, false});

    // an archive with NO GuerrillaLegends block at all
    const std::filesystem::path path = ArchivePath("legend-prelegend.bin");
    {
        ParamArchiveSave ar(WorldSerializeVersion);
        int filler = 1;
        REQUIRE(ar.Serialize("somethingElse", filler, 1) == LSOK);
        REQUIRE(ar.SaveBin(path.string().c_str()));
    }

    LegendRegistry derived;
    CHECK(!LoadRegistry(derived, path)); // the derive branch
    CHECK(derived.HasState());           // it holds rows worth saving
    CHECK(!derived.IsSeeded());          // but has no campaign seed
    CHECK(!derived.HasProgression());
    CHECK(derived.Seed() == 0);
    CHECK(!derived.History().Present());
    REQUIRE(derived.RowCount() == 2);
    CHECK(Str(derived.Row(0).id) == "comp_0_petra");
    CHECK(Str(derived.DisplayName(derived.Row(0))) == "Petra"); // the base name, exactly
    CHECK(derived.Row(0).last.GetLength() == 0);
    CHECK(derived.Row(0).awardMask == 0);
    CHECK(!derived.Row(0).legend);
    CHECK(derived.Row(0).rankSeen == 2); // read off the XP ladder, not the string
    CHECK(derived.Row(0).alive);
    CHECK(!derived.Row(1).alive);
    CHECK(IsPortraitFace(derived.Row(0).face));
    CHECK(Journal::Instance().EntryCount() == 0);

    // derived state must SURVIVE a save: the discriminator is HasState(), not
    // the seed, or every load would re-derive and lose the rows and deeds
    derived.PollCompanionsForTest(MakeSnapshot({"Petra", "Ivan"}, {600.0f, 0.0f}, {true, false}));
    const int deeds = derived.Row(0).deeds.Size();
    REQUIRE(deeds > 0);
    const std::string face = Str(derived.Row(0).face);

    const std::filesystem::path path2 = ArchivePath("legend-derived.bin");
    SaveRegistry(derived, path2);
    LegendRegistry again;
    CHECK(LoadRegistry(again, path2)); // the subclass branch: NO second derive
    CHECK(again.HasState());
    CHECK(!again.HasProgression());
    CHECK(again.Seed() == 0);
    REQUIRE(again.RowCount() == 2);
    CHECK(again.Row(0).deeds.Size() == deeds);
    CHECK(Str(again.Row(0).face) == face);
    CHECK(Str(again.DisplayName(again.Row(0))) == "Petra");
    CHECK(Journal::Instance().EntryCount() == 0);

    std::filesystem::remove(path);
    std::filesystem::remove(path2);
}

TEST_CASE("Legend registry - the companion display name never goes blank while the roster has the index",
          "[game][guerrilla][legends][registry]")
{
    // companions.sqs calls GM_fnCompStatus at boot, before the first poll has
    // created any row, so the fallback is the normal case on a new campaign.
    ScopedVars vars;
    SetCompanionGlobals(vars, {"Petra"}, {100.0f}, {true});

    LegendRegistry registry;
    Seed(registry);
    CHECK(Str(registry.CompanionDisplayName(0)) == "Petra"); // no row yet
    CHECK(Str(registry.CompanionDisplayName(7)).empty());    // no row and no roster slot

    registry.PollCompanionsForTest(OneCompanion("Petra", 100.0f));
    const int r = registry.FindByCompIndex(0);
    REQUIRE(r >= 0);
    CHECK(Str(registry.CompanionDisplayName(0)) == Str(registry.DisplayName(registry.Row(r))));
    CHECK(Str(registry.CompanionDisplayName(0)) != "Petra"); // the generated surname is on
}

// ===========================================================================
//  the enemy Legends (Change 3): resolution, the defeat latch, persistence
// ===========================================================================

namespace
{

// The three world answers ResolveBosses reads, handed in instead: what the
// occupier can field, the zone ladder's output and the candidate stands.
LegendRoleCapability BossCapability(bool sniper = true, bool tank = true)
{
    LegendRoleCapability cap;
    cap.eliteClass = "OfficerE";
    cap.riflemanClass = "SoldierEB";
    cap.tankCrewClass = "SoldierECrew";
    if (sniper)
    {
        cap.sniperClass = "SoldierESniper";
    }
    if (tank)
    {
        cap.tankClass = "T72";
        cap.tankHasCommander = true;
        cap.tankHasGunner = true;
    }
    return cap;
}

AutoArray<LegendZoneCandidate> BossZones()
{
    AutoArray<LegendZoneCandidate> zones;
    LegendZoneCandidate outpost;
    outpost.name = "Outpost";
    outpost.type = "OUTPOST";
    outpost.occupied = true;
    outpost.x = 7600;
    outpost.z = 6300;
    zones.Add(outpost);
    LegendZoneCandidate camp;
    camp.name = "Camp";
    camp.type = "CAMP";
    camp.x = 7515;
    camp.z = 5790;
    zones.Add(camp);
    return zones;
}

// A ring of stands around the Outpost, all dry, off the road and clear of the
// Camp floor: the placement judgement itself is pinned in
// test_legend_placement.cpp, so these cases only need three stands to exist.
AutoArray<LegendSpotSample> BossSamples()
{
    AutoArray<LegendSpotSample> samples;
    for (int i = 0; i < 8; i++)
    {
        const float angle = (2.0f * 3.14159265f) * (float)i / 8.0f;
        LegendSpotSample s;
        s.zone = 0;
        s.x = 7600.0f + 800.0f * cosf(angle);
        s.z = 6300.0f + 800.0f * sinf(angle);
        s.height = 18.0f;
        s.distFromZone = 800.0f;
        s.distFromCamp = 900.0f + 5.0f * (float)i;
        samples.Add(s);
    }
    return samples;
}

// Seed, then resolve the three commanders off the injected world.
void SeedAndResolve(LegendRegistry& registry, bool sniper = true, bool tank = true)
{
    Seed(registry);
    registry.ResolveBossesForTest(BossCapability(sniper, tank), BossZones(), BossSamples());
}

int FirstBossRow(const LegendRegistry& registry)
{
    for (int i = 0; i < registry.RowCount(); i++)
    {
        if (registry.Row(i).kind == LKBoss)
        {
            return i;
        }
    }
    return -1;
}

std::string ObjectiveState(const char* id)
{
    const Journal& journal = Journal::Instance();
    const int i = journal.FindObjective(id);
    return i < 0 ? std::string() : std::string(Journal::ObjectiveStateName(journal.Objective(i).state));
}

// The objective the SPAWN publishes, stood up by hand: these cases place a
// commander but cannot spawn one (that needs a world), and the defeat latch's
// SetObjective(id, "", JODone) is deliberately a state-only write, which
// Journal::SetObjective drops on an unknown row.  Publishing it here is what
// makes "the latch flips the objective the spawn wrote" an honest assertion
// rather than a vacuous one.
void PublishSpawnObjective(const char* id, const char* text)
{
    Journal::Instance().SetObjective(RString(id), RString(text), JOActive);
}

} // namespace

TEST_CASE("Legend registry - resolution places three commanders and moves no row", "[game][guerrilla][legends][boss]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry);
    // a companion row FIRST, so the case can prove resolution touches only the
    // boss half of a mixed table
    registry.PollCompanionsForTest(OneCompanion("Petra", 100.0f));
    REQUIRE(registry.BossCount() == LegendRegistry::kBossCount);

    std::vector<std::string> orderBefore;
    for (int i = 0; i < registry.RowCount(); i++)
    {
        orderBefore.push_back(Str(registry.Row(i).id));
    }
    const int comp = registry.FindByCompIndex(0);
    REQUIRE(comp >= 0);
    const std::string compName = Str(registry.DisplayName(registry.Row(comp)));

    registry.ResolveBossesForTest(BossCapability(), BossZones(), BossSamples());

    // ROW ORDER IS THE CONTRACT: ParamArchive walks Item%d by index across the
    // two passes, so a resolution that reordered rows would corrupt a save
    REQUIRE((int)orderBefore.size() == registry.RowCount());
    for (int i = 0; i < registry.RowCount(); i++)
    {
        CHECK(Str(registry.Row(i).id) == orderBefore[i]);
    }

    std::vector<std::string> roles;
    for (int i = 0; i < registry.RowCount(); i++)
    {
        const LegendRow& row = registry.Row(i);
        if (row.kind != LKBoss)
        {
            continue;
        }
        CHECK(Str(row.zoneName) == "Outpost");
        CHECK(row.pos.X() != 0.0f);
        CHECK(row.pos.Z() != 0.0f);
        // roleResolved is the PROOF that resolution ran: the pre-roll sentinel
        // and the Elite role name are the same word
        CHECK(Str(row.roleResolved) == Str(row.role));
        CHECK(row.roleRequested.GetLength() > 0);
        CHECK(row.guardCount > 0);
        CHECK(!row.spawned);
        CHECK(!row.defeated);
        roles.push_back(Str(row.role));
    }
    REQUIRE(roles.size() == 3);
    CHECK(roles[0] == "Sniper");
    CHECK(roles[1] == "Commander");
    CHECK(roles[2] == "Tank Commander");

    // the three stands are distinct and clear of each other
    const int b0 = FirstBossRow(registry);
    REQUIRE(b0 >= 0);
    for (int a = b0; a < b0 + 3; a++)
    {
        for (int b = a + 1; b < b0 + 3; b++)
        {
            const Vector3 pa = registry.RowPos(a);
            const Vector3 pb = registry.RowPos(b);
            const float dx = pa.X() - pb.X();
            const float dz = pa.Z() - pb.Z();
            CHECK(sqrtf(dx * dx + dz * dz) >= LegendPlacementConstants::SameZoneFloor);
        }
    }

    // the companion row was not touched: no stand, no role rewrite, no deed
    CHECK(Str(registry.Row(comp).zoneName).empty());
    CHECK(Str(registry.DisplayName(registry.Row(comp))) == compName);
    CHECK(registry.RowPos(comp).X() == 0.0f);
}

TEST_CASE("Legend registry - a faction that fields neither gets three commanders", "[game][guerrilla][legends][boss]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    SeedAndResolve(registry, false, false);
    for (int i = 0; i < registry.RowCount(); i++)
    {
        const LegendRow& row = registry.Row(i);
        if (row.kind != LKBoss)
        {
            continue;
        }
        CHECK(Str(row.role) == "Commander");
        CHECK(Str(row.roleResolved) == "Commander");
        CHECK(row.guardCount == LegendRegistry::kEliteGuards);
    }
    // the ASK survives the degrade, so the record can still say what he was
    const int b0 = FirstBossRow(registry);
    REQUIRE(b0 >= 0);
    CHECK(Str(registry.Row(b0).roleRequested) == "Sniper");
    CHECK(Str(registry.Row(b0 + 2).roleRequested) == "Tank Commander");
}

TEST_CASE("Legend registry - a commander with no stand is never placed and never retried",
          "[game][guerrilla][legends][boss]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry);
    // every candidate stand in the sea: the identities survive, the stands do
    // not, and the player is told once
    AutoArray<LegendSpotSample> drowned = BossSamples();
    for (int i = 0; i < drowned.Size(); i++)
    {
        drowned[i].underwater = true;
    }
    registry.ResolveBossesForTest(BossCapability(), BossZones(), drowned);

    for (int i = 0; i < registry.RowCount(); i++)
    {
        const LegendRow& row = registry.Row(i);
        if (row.kind != LKBoss)
        {
            continue;
        }
        CHECK(Str(row.zoneName).empty());
        CHECK(row.pos.X() == 0.0f);
        CHECK(!row.spawned);
        // the identity, the biography and the dossier row all survive
        CHECK(registry.DisplayName(row).GetLength() > 0);
        CHECK(row.bio.GetLength() > 0);
        CHECK(Str(row.roleResolved) == Str(row.role));
    }
    // ONE warning line, not three
    CHECK(Journal::Instance().EntryCount() == 1);
    CHECK(AnyEntryIncludes(Journal::Instance(), "Intelligence names only"));
    CHECK(Journal::Instance().Entry(0).kind == JKWarn);
    // and no objective at all: nothing on the map may disagree with the dossier
    CHECK(Journal::Instance().ObjectiveCount() == 0);
}

TEST_CASE("Legend registry - the defeat latch fires exactly once, however often it is asked",
          "[game][guerrilla][legends][boss]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    SeedAndResolve(registry);
    const int row = FirstBossRow(registry);
    REQUIRE(row >= 0);
    const std::string id = Str(registry.Row(row).id);
    CHECK(id == "boss_0");

    PublishSpawnObjective("legend_boss_0", "Eliminate him, Sniper, near Outpost.");
    const int entriesBefore = Journal::Instance().EntryCount();
    const int deedsBefore = registry.Row(row).deeds.Size();

    registry.BossTickForTest(row, false, false);

    CHECK(registry.Row(row).defeated);
    CHECK(!registry.Row(row).alive);
    CHECK(registry.DefeatedCount() == 1);
    // EXACTLY ONE diary line, attributed to this row by id: that is what puts
    // the same sentence in the campaign record and in his own dossier without
    // anyone parsing prose
    CHECK(Journal::Instance().EntryCount() == entriesBefore + 1);
    CHECK(CountEntriesFor(Journal::Instance(), id.c_str()) == 1);
    // ... filed under the zone he was PLACED to watch, never the nearest one
    const int last = Journal::Instance().EntryCount() - 1;
    CHECK(Str(Journal::Instance().Entry(last).zone) == "Outpost");
    CHECK(Journal::Instance().Entry(last).kind == JKGood);
    CHECK(strstr(Journal::Instance().Entry(last).text, "is dead") != nullptr);
    // one death deed
    CHECK(CountDeeds(registry.Row(row), LDDefeat) == 1);
    CHECK(registry.Row(row).deeds.Size() == deedsBefore + 1);
    // and the objective is done
    CHECK(ObjectiveState("legend_boss_0") == "DONE");

    // THE RESURRECTION GUARD.  Five more polls, a reload's worth: not one of
    // them may write a second line, a second deed, a second objective row or a
    // single repaint.
    const int entries = Journal::Instance().EntryCount();
    const int objectives = Journal::Instance().ObjectiveCount();
    const int deeds = registry.Row(row).deeds.Size();
    const unsigned revision = registry.Revision();
    const unsigned journalRevision = Journal::Instance().Revision();
    for (int i = 0; i < 5; i++)
    {
        registry.BossTickForTest(row, false, false);
    }
    CHECK(Journal::Instance().EntryCount() == entries);
    CHECK(Journal::Instance().ObjectiveCount() == objectives);
    CHECK(registry.Row(row).deeds.Size() == deeds);
    CHECK(registry.Revision() == revision);
    CHECK(Journal::Instance().Revision() == journalRevision);
    CHECK(registry.DefeatedCount() == 1);
    CHECK(CountEntriesFor(Journal::Instance(), id.c_str()) == 1);
}

TEST_CASE("Legend registry - destroying the hull alone does not complete the objective",
          "[game][guerrilla][legends][boss]")
{
    // THE TANK RULE.  row.body (the named commander) and row.vehicle (the
    // hull) are two independent links and two independent questions: the
    // hull's death prunes the link, revokes the standing get-in order and
    // changes nothing else.  He often, but not always, survives it; if he does
    // he fights on foot and the objective stands until he himself is dead.
    Journal::Instance().Clear();
    LegendRegistry registry;
    SeedAndResolve(registry);
    const int b0 = FirstBossRow(registry);
    REQUIRE(b0 >= 0);
    const int tankRow = b0 + 2;
    REQUIRE(Str(registry.Row(tankRow).role) == "Tank Commander");

    PublishSpawnObjective("legend_boss_2", "Eliminate him, Tank Commander, near Outpost.");
    registry.BossTickForTest(tankRow, true, false); // one quiet poll first
    const unsigned revision = registry.Revision();
    const unsigned journalRevision = Journal::Instance().Revision();
    const int entries = Journal::Instance().EntryCount();

    registry.BossTickForTest(tankRow, true, true); // the hull dies under him

    CHECK(registry.RowVehicle(tankRow) == nullptr);
    CHECK(!registry.Row(tankRow).defeated);
    CHECK(registry.Row(tankRow).alive);
    CHECK(registry.DefeatedCount() == 0);
    CHECK(Journal::Instance().EntryCount() == entries);
    CHECK(ObjectiveState("legend_boss_2") == "ACTIVE");
    // a poll that changed nothing the dossier renders repaints nothing: the
    // open map must not rebuild once a second while the player reads it
    CHECK(registry.Revision() == revision);
    CHECK(Journal::Instance().Revision() == journalRevision);

    // and the commander himself still closes it
    registry.BossTickForTest(tankRow, false, false);
    CHECK(registry.Row(tankRow).defeated);
    CHECK(ObjectiveState("legend_boss_2") == "DONE");
    CHECK(CountEntriesFor(Journal::Instance(), "boss_2") == 1);
}

TEST_CASE("Legend registry - a boss poll that changed nothing repaints nothing", "[game][guerrilla][legends][boss]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    SeedAndResolve(registry);
    const int row = FirstBossRow(registry);
    REQUIRE(row >= 0);
    const unsigned revision = registry.Revision();
    const unsigned journalRevision = Journal::Instance().Revision();
    const int entries = Journal::Instance().EntryCount();
    for (int i = 0; i < 4; i++)
    {
        registry.BossTickForTest(row, true, false);
    }
    CHECK(registry.Revision() == revision);
    CHECK(Journal::Instance().Revision() == journalRevision);
    CHECK(Journal::Instance().EntryCount() == entries);
    CHECK(!registry.Row(row).defeated);
}

TEST_CASE("Legend registry - a campaign round trips its commanders, living and defeated",
          "[game][guerrilla][legends][boss][save][load]")
{
    Journal::Instance().Clear();
    LegendRegistry registry;
    SeedAndResolve(registry);
    registry.PollCompanionsForTest(OneCompanion("Petra", 100.0f));
    const int b0 = FirstBossRow(registry);
    REQUIRE(b0 >= 0);
    // one of the three is already dead when the save is written
    registry.BossTickForTest(b0, false, false);
    REQUIRE(registry.Row(b0).defeated);

    struct Snapshot
    {
        std::string id, role, requested, resolved, zone;
        float x = 0, y = 0, z = 0;
        int guardCount = 0;
        bool spawned = false, defeated = false, markerPainted = false;
        std::string markerName;
        int deeds = 0;
    };
    std::vector<Snapshot> before;
    std::vector<std::string> orderBefore;
    for (int i = 0; i < registry.RowCount(); i++)
    {
        const LegendRow& row = registry.Row(i);
        orderBefore.push_back(Str(row.id));
        if (row.kind != LKBoss)
        {
            continue;
        }
        Snapshot s;
        s.id = Str(row.id);
        s.role = Str(row.role);
        s.requested = Str(row.roleRequested);
        s.resolved = Str(row.roleResolved);
        s.zone = Str(row.zoneName);
        s.x = row.pos.X();
        s.y = row.pos.Y();
        s.z = row.pos.Z();
        s.guardCount = row.guardCount;
        s.spawned = row.spawned;
        s.defeated = row.defeated;
        s.markerName = Str(row.markerName);
        s.markerPainted = row.markerPainted;
        s.deeds = row.deeds.Size();
        before.push_back(s);
    }
    REQUIRE(before.size() == 3);
    const int entries = Journal::Instance().EntryCount();

    const std::filesystem::path path = ArchivePath("legend-bosses.bin");
    SaveRegistry(registry, path);
    LegendRegistry loaded;
    REQUIRE(LoadRegistry(loaded, path));

    REQUIRE(loaded.RowCount() == (int)orderBefore.size());
    for (int i = 0; i < loaded.RowCount(); i++)
    {
        CHECK(Str(loaded.Row(i).id) == orderBefore[i]); // IN ORDER
    }
    REQUIRE(loaded.BossCount() == 3);
    CHECK(loaded.DefeatedCount() == 1);
    size_t n = 0;
    for (int i = 0; i < loaded.RowCount(); i++)
    {
        const LegendRow& row = loaded.Row(i);
        if (row.kind != LKBoss)
        {
            continue;
        }
        REQUIRE(n < before.size());
        const Snapshot& s = before[n++];
        CHECK(Str(row.id) == s.id);
        CHECK(Str(row.role) == s.role);
        CHECK(Str(row.roleRequested) == s.requested);
        CHECK(Str(row.roleResolved) == s.resolved);
        CHECK(Str(row.zoneName) == s.zone);
        CHECK(row.pos.X() == s.x);
        CHECK(row.pos.Y() == s.y);
        CHECK(row.pos.Z() == s.z);
        CHECK(row.guardCount == s.guardCount);
        CHECK(row.spawned == s.spawned);
        CHECK(row.defeated == s.defeated);
        CHECK(Str(row.markerName) == s.markerName);
        CHECK(row.markerPainted == s.markerPainted);
        CHECK(row.deeds.Size() == s.deeds);
        // TRANSIENT by design: re-derived by the next poll, never persisted
        CHECK(Str(row.lastZone).empty());
    }

    // a defeat latched before the save never re-fires after it
    const int loadedBoss = FirstBossRow(loaded);
    REQUIRE(loadedBoss >= 0);
    CHECK(loaded.Row(loadedBoss).defeated);
    for (int i = 0; i < 3; i++)
    {
        loaded.BossTickForTest(loadedBoss, false, false);
    }
    CHECK(Journal::Instance().EntryCount() == entries);
    CHECK(CountEntriesFor(Journal::Instance(), "boss_0") == 1);
    CHECK(loaded.DefeatedCount() == 1);

    std::filesystem::remove(path);
}

TEST_CASE("Legend registry - a boss row's Change 3 fields carry documented defaults",
          "[game][guerrilla][legends][boss][save][load]")
{
    // The four Change 3 keys use the default-tolerant four-argument Serialize
    // overload, so a Change 2 archive that carries none of them loads with
    // these values rather than faulting.  A pre-Change-3 save has no placed
    // commanders at all (resolution runs only in the seeding tick, which a
    // loaded campaign never re-runs), so its three Legends stay unlocated for
    // the life of that campaign, and unlocated is exactly this state.
    Journal::Instance().Clear();
    LegendRegistry registry;
    Seed(registry); // pre-rolled identities, NO resolution
    for (int i = 0; i < registry.RowCount(); i++)
    {
        const LegendRow& row = registry.Row(i);
        if (row.kind != LKBoss)
        {
            continue;
        }
        CHECK(row.guardCount == 0);
        CHECK(!row.bodySeen);
        CHECK(Str(row.markerName).empty());
        CHECK(!row.markerPainted);
        CHECK(!row.spawned);
        CHECK(!row.defeated);
        CHECK(Str(row.zoneName).empty());
        // the identity is still real: name, face and biography all pre-rolled
        CHECK(registry.DisplayName(row).GetLength() > 0);
    }

    const std::filesystem::path path = ArchivePath("legend-boss-defaults.bin");
    SaveRegistry(registry, path);
    LegendRegistry loaded;
    REQUIRE(LoadRegistry(loaded, path));
    REQUIRE(loaded.BossCount() == LegendRegistry::kBossCount);
    for (int i = 0; i < loaded.RowCount(); i++)
    {
        const LegendRow& row = loaded.Row(i);
        if (row.kind != LKBoss)
        {
            continue;
        }
        CHECK(row.guardCount == 0);
        CHECK(!row.bodySeen);
        CHECK(Str(row.markerName).empty());
        CHECK(!row.markerPainted);
        CHECK(!row.spawned);
    }
    CHECK(Journal::Instance().ObjectiveCount() == 0);
    std::filesystem::remove(path);
}

TEST_CASE("Legend registry - a spawn that latched and then failed reads as missing intelligence",
          "[game][guerrilla][legends][boss][save][load]")
{
    // The latch goes down BEFORE the actors are built, so row.spawned is true
    // on the failure path too (a null group at MaxGroups, a body class that
    // would not materialize).  SpawnOneBoss returns before it reaches
    // CreateBossMarker or AssertBossObjective, so that row has no marker and no
    // objective: its placement is cleared with it, and the dossier reads him as
    // missing intelligence rather than as a commander standing near a zone
    // where there is nobody to kill.
    Journal::Instance().Clear();
    LegendRegistry registry;
    SeedAndResolve(registry);
    const int row = FirstBossRow(registry);
    REQUIRE(row >= 0);
    REQUIRE(Str(registry.Row(row).id) == "boss_0");
    REQUIRE(Str(registry.Row(row).zoneName) == "Outpost"); // he WAS placed
    const RString name = registry.DisplayName(registry.Row(row));

    registry.SpawnFailureForTest(row);

    CHECK(registry.Row(row).spawned); // latched: a failure is never retried
    CHECK(!registry.Row(row).bodySeen);
    // the caption gate: no stand, so "whereabouts unknown" rather than
    // "at large" (the compose half of that is pinned in test_journal_people)
    CHECK(Str(registry.Row(row).zoneName).empty());
    CHECK(registry.Row(row).pos.X() == 0.0f);
    CHECK(registry.Row(row).pos.Z() == 0.0f);
    CHECK(Str(registry.Row(row).markerName).empty());
    CHECK(!registry.Row(row).defeated);
    CHECK(ObjectiveState("legend_boss_0").empty());
    // the identity, the biography and the dossier row all survive intact
    CHECK(Str(registry.DisplayName(registry.Row(row))) == Str(name));
    CHECK(registry.Row(row).bio.GetLength() > 0);
    // and the other two commanders are untouched by his failure
    CHECK(Str(registry.Row(row + 1).zoneName) == "Outpost");
    CHECK(!registry.Row(row + 1).spawned);

    const std::filesystem::path path = ArchivePath("legend-spawn-failed.bin");
    SaveRegistry(registry, path);
    LegendRegistry loaded;
    REQUIRE(LoadRegistry(loaded, path));
    REQUIRE(loaded.BossCount() == LegendRegistry::kBossCount);
    const int loadedRow = FirstBossRow(loaded);
    REQUIRE(loadedRow >= 0);
    CHECK(loaded.Row(loadedRow).spawned);
    CHECK(!loaded.Row(loadedRow).bodySeen);
    CHECK(Str(loaded.Row(loadedRow).zoneName).empty());
    CHECK(Str(loaded.Row(loadedRow).markerName).empty());
    CHECK(loaded.DefeatedCount() == 0);
    // THE LOAD PASS MANUFACTURES NOTHING for a commander who never existed
    CHECK(Journal::Instance().ObjectiveCount() == 0);
    std::filesystem::remove(path);
}

TEST_CASE("Legend registry - a save carrying a failed spawn's stand still re-asserts nothing",
          "[game][guerrilla][legends][boss][save][load]")
{
    // The same failure as written by an earlier build, which kept the stand on
    // the row.  ReconcileAfterLoad and the deferred marker pass both key off
    // bodySeen rather than off the spawn latch precisely so this archive cannot
    // grow an objective and a marker for a body that was never created.
    Journal::Instance().Clear();
    LegendRegistry registry;
    SeedAndResolve(registry);
    const int row = FirstBossRow(registry);
    REQUIRE(row >= 0);
    registry.SpawnFailureForTest(row, true);
    REQUIRE(registry.Row(row).spawned);
    REQUIRE(!registry.Row(row).bodySeen);
    REQUIRE(Str(registry.Row(row).zoneName) == "Outpost"); // the stale stand

    const std::filesystem::path path = ArchivePath("legend-spawn-failed-legacy.bin");
    SaveRegistry(registry, path);
    LegendRegistry loaded;
    REQUIRE(LoadRegistry(loaded, path));
    const int loadedRow = FirstBossRow(loaded);
    REQUIRE(loadedRow >= 0);
    CHECK(loaded.Row(loadedRow).spawned);
    CHECK(!loaded.Row(loadedRow).bodySeen);
    // no objective, and none of the three carries a marker name
    CHECK(ObjectiveState("legend_boss_0").empty());
    CHECK(Journal::Instance().ObjectiveCount() == 0);
    for (int i = 0; i < loaded.RowCount(); i++)
    {
        if (loaded.Row(i).kind == LKBoss)
        {
            CHECK(Str(loaded.Row(i).markerName).empty());
        }
    }
    std::filesystem::remove(path);
}

TEST_CASE("Legend registry - a foot commander's guards are never movement pinned", "[game][guerrilla][legends][boss]")
{
    // The pin is the named boss's alone: spec section 2 keeps him stationary
    // and asks his guards to defend locally, which a DAMove-disabled guard
    // cannot do at all (the flag is read by LeaderPilot and FormationPilot, so
    // it kills even formation-following).  row.crewed is the discriminator,
    // because roleResolved still reads "Tank Commander" when the hull refused
    // to materialize and row.vehicle is already cleared when a dead hull
    // re-pins.  It defaults false, so a Change 2 archive pins nobody's guards.
    Journal::Instance().Clear();
    LegendRegistry registry;
    SeedAndResolve(registry);
    for (int i = 0; i < registry.RowCount(); i++)
    {
        if (registry.Row(i).kind == LKBoss)
        {
            CHECK(!registry.Row(i).crewed);
        }
    }
    const int row = FirstBossRow(registry);
    REQUIRE(row >= 0);
    const std::filesystem::path path = ArchivePath("legend-crewed.bin");
    SaveRegistry(registry, path);
    LegendRegistry loaded;
    REQUIRE(LoadRegistry(loaded, path));
    const int loadedRow = FirstBossRow(loaded);
    REQUIRE(loadedRow >= 0);
    CHECK(!loaded.Row(loadedRow).crewed);
    std::filesystem::remove(path);
}
