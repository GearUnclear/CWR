#pragma once

// Guerrilla Mode character registry: stable ids, five-slot earned names,
// appearance, generated biographies, recorded deeds, and (Change 3) the three
// enemy Legends' placement and defeat state.
//
// It OWNS no progression.  companions.sqs keeps XP / rank / skill / spawning;
// the registry OBSERVES the GM_COMP_* arrays once a second and supplies the
// canonical display name that the live unit identity, roster, promotion and
// death hints, dossiers and memorials all show (every one of those surfaces
// reads Person::GetInfo()._name, so one write makes them agree).
//
//   InitMission() = Clear(); LoadFromConfig(); _initRan = true;  and nothing else.
//   Simulate()    = ZoneRegistry::IsActive() gate, then the serialized one-shot
//                   _seeded seeding tick, then 1 Hz PollCompanions().
//
// ROW ORDER is part of the contract: companions by compIndex ascending, then
// the pre-rolled boss rows by id.  A new companion is INSERTED ahead of the
// boss block rather than appended, so gmLegendInfo 0 is the first companion for
// the whole campaign and the journal's People page can walk _rows straight
// through.  Rows are never reordered inside Serialize (ParamArchive re-walks
// Item%d by index across the two passes).
//
// TWO MODES, decided once per campaign and serialized:
//   * PROGRESSION (_progression true) - a campaign seeded by this build.  It
//     has a seed, a history, three pre-rolled enemy Legends, earned names and
//     the four diary lines of D2.5.
//   * DERIVED (_progression false) - a save written before the registry
//     existed.  Rows are rebuilt from GM_COMP_* so the dossiers work, but the
//     registry writes ZERO journal entries and ZERO objectives for that
//     campaign's life: the serialized companions.sqs still running inside it
//     writes its own.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/LLinks.hpp> // LLink / LLinkArray (== OLink, NetworkObject.hpp)
#include <Poseidon/IO/Serialization/SerializeClass.hpp>

#include <Poseidon/Game/Guerrilla/FactionHistory.hpp>
#include <Poseidon/Game/Guerrilla/LegendNames.hpp>

class ParamArchive;
namespace Poseidon
{
class AIGroup;
} // namespace Poseidon

namespace Poseidon
{

class Object;
class Person;

namespace Guerrilla
{

enum LegendKind
{
    LKCompanion = 0,
    LKBoss = 1
};

// Award latch bits.  Persisted, so a load, a RetryMission, an identical re-poll
// and a threshold crossed before the registry existed all converge on "already
// awarded".
enum LegendAward
{
    LAFirst = 1 << 0,
    LASecond = 1 << 1
};

enum LegendDeedKind
{
    LDPromotion = 0,
    LDMilestone = 1,
    LDAward = 2,
    LDDeath = 3,
    LDDefeat = 4
};

// One line of a character's recorded half.  Deeds are the earned record; the
// generated biography is the inherited half and never overlaps them.
struct LegendDeed
{
    RString stamp; // "Day 3 14:20", "" when logged without a clock
    RString text;
    int kind = LDPromotion;

    LSError Serialize(ParamArchive& ar);
};

struct LegendRow
{
    RString id; // comp_<index>_<slug> or boss_<n>; written once, never rewritten
    int kind = LKCompanion;
    int compIndex = -1; // GM_COMP_* index for a companion, -1 for a boss
    RString baseName;   // the script's own name, preserved verbatim

    // the five name slots; first/last are personal names, the other three are
    // nickname words that stay empty until awarded
    RString first, last;
    RString prefix, describer, title;

    int awardMask = 0;   // LegendAward bits; the idempotence latch
    bool legend = false; // second award reached (or a boss, notorious from the start)

    RString face;      // one of kPortraitFaces, or "Default" when validation refused them all
    RString bodyClass; // the live body's config class, for the portrait key
    int outfit = 0;    // OutfitSelect rung, Change 4

    int namePool = -1; // LegendNames pool index
    int tone = ToneFriendly;

    // Resolved prose, generated once at row creation and PERSISTED, so a fix to
    // a biography template only reaches campaigns started after it: an already
    // seeded save keeps the wording it was written with, and the composed page
    // clamps whatever it carries.
    RString bio;
    int bioEvent = 0; // which shared history beat the bio hangs on

    int rankSeen = -1; // ladder index last observed; seeded from the creating snapshot
    float xpSeen = 0;

    bool alive = true;
    int deathDay = 0, deathMinute = 0;
    int daysSurvived = 0;

    AutoArray<LegendDeed> deeds;

    // Boss fields written by Change 3.  Declared NOW so the row format does not
    // change between changes and a Change 2 save loads in Change 3.  role is
    // pre-set to "Commander" on the three pre-rolled rows: that is Change 3's
    // not-yet-resolved sentinel.
    RString role;
    RString roleRequested, roleResolved;
    RString zoneName;
    Vector3 pos = VZero;
    bool spawned = false, defeated = false;
    LLink<Object> body, vehicle;
    LLink<AIGroup> group;
    LLinkArray<Object> guards;

    // TRANSIENT, never serialized: where the body was last seen while alive, so
    // the death deed and the death diary line can name a place.
    Vector3 lastPos = VZero;
    RString lastZone;

    LSError Serialize(ParamArchive& ar);
};

// The GM_COMP_* observation the registry acts on.  The live poll builds one per
// tick; the unit suite hands one in directly.
struct LegendCompanionSnapshot
{
    AutoArray<RString> names;
    AutoArray<float> xp;
    AutoArray<RString> rank; // cross-checked only; the ladder index comes from xp
    AutoArray<bool> alive;
    int day = 1;
    int minuteOfDay = 0;
};

class LegendRegistry : public SerializeClass
{
  public:
    LegendRegistry() = default;

    static LegendRegistry& Instance();

    static constexpr float TickInterval = 1.0f;

    // The ladder is companions.sqs' own, mirrored verbatim.  The XP thresholds
    // are GM_COMP_XPTHRESH (companions.sqs); the words are GM_COMP_RANKLADDER.
    // The engine's Rank enum is NOT usable here: it spells the fourth rung
    // LIEUTNANT while the script spells it LIEUTENANT, and JournalText::RankShort
    // lives in the UI layer, which the Game layer must not include.
    static constexpr int NRankLadder = 7;
    static const float kXpThresholds[NRankLadder];
    static const char* const kRankLadder[NRankLadder];
    static constexpr int kFirstAwardRank = 2;  // SERGEANT
    static constexpr int kSecondAwardRank = 6; // COLONEL
    static constexpr int NMilestones = 4;
    static const int kMilestoneDays[NMilestones];
    static constexpr int kBossCount = 3;
    static constexpr int NPortraitFaces = 4;
    static const char* const kPortraitFaces[NPortraitFaces];
    // past this the OLDEST non-award, non-death deed is dropped, so the earned
    // names and the death line are never the ones evicted
    static constexpr int kMaxDeeds = 24;

    // lifecycle -----------------------------------------------------------
    void Clear();
    void InitMission();
    void LoadFromConfig(); // documented no-op in Change 2; the shape matches its neighbours
    void Simulate(float deltaT);

    LSError Serialize(ParamArchive& ar) override;
    // Taken when a load finds no GuerrillaLegends block at all, i.e. a save
    // written before the registry existed.  Builds companion rows from
    // GM_COMP_* and locks the campaign into DERIVED mode forever.
    void DeriveFromExistingSave();

    // queries ---------------------------------------------------------------
    // "this campaign has a generated seed" - false in DERIVED mode.
    bool IsSeeded() const { return _seed != 0; }
    // "this registry holds campaign state worth saving" - the save-block
    // discriminator.  A DERIVED campaign has no seed but MUST still write its
    // block, or every load would re-derive it and lose the rows and deeds.
    bool HasState() const { return _seeded; }
    bool HasProgression() const { return _progression; }
    unsigned Seed() const { return _seed; }
    unsigned Revision() const { return _revision; }
    int RowCount() const { return _rows.Size(); }
    const LegendRow& Row(int i) const { return _rows[i]; }
    int FindByCompIndex(int compIndex) const; // -1 when unknown
    int FindById(const char* id) const;       // -1 when unknown
    const HistoryRecord& History() const { return _history; }

    // The five slots assembled.  Never localized, never prefixed: the result is
    // written onto the body as AIUnitInfo::_name and persisted.
    RString DisplayName(const LegendRow& row) const;
    // gmLegendName's contract: the row's display name, else the GM_COMP_NAMES
    // entry, else "" plus one warning per process.  It NEVER returns "" while
    // GM_COMP_NAMES has the index, which is what lets companions.sqs call it at
    // boot, before the first poll has created any row.
    RString CompanionDisplayName(int compIndex) const;

    // Stamp the registry's identity onto a live body.  Creates the row when the
    // companion has never been polled (the very first GM_fnCompSpawn runs
    // before the first 1 Hz tick), so the HUD never shows the createUnit pool
    // identity.  Must run AFTER createUnit returns.
    bool Bind(int compIndex, Object* body);

    // repaint: _revision always, the journal's revision only when something the
    // dossier renders moved.  Public so the suite can pin the two edges.
    void Touch(bool dossierVisible);
    int LadderIndexForXp(float xp) const;

    // test seams ------------------------------------------------------------
    typedef LegendCompanionSnapshot CompanionSnapshot;
    void SeedForTest(unsigned seed, const HistoryInputs& in);
    void PollCompanionsForTest(const CompanionSnapshot& snapshot);
    void SetProgressionForTest(bool on) { _progression = on; }

  private:
    void SeedCampaign();
    void PollCompanions();
    void ApplySnapshot(const CompanionSnapshot& snapshot, bool live);
    int EnsureCompanionRow(int compIndex, const RString& baseName, float xp, bool alive);
    void AwardSlot(LegendRow& row, int which);
    void RecordDeed(LegendRow& row, const RString& text, int kind);
    void WriteEntry(const LegendRow& row, const RString& text, int kind);
    void PreRollBossIdentities();
    void ReconcileAfterLoad();
    unsigned long long RowKey(const RString& id) const;

    AutoArray<LegendRow> _rows;
    HistoryRecord _history;
    unsigned _seed = 0;
    bool _progression = false;
    bool _seeded = false;
    bool _initRan = false;
    unsigned _revision = 0;
    float _accum = 0;
    int _resistancePool = -1, _occupierPool = -1;
    RString _resistanceName, _occupierName;
    // transient change detector: the digest of the last observation acted on,
    // so an unchanged roster costs one hash per second and never repaints
    unsigned long long _pollDigest = 0;
    // transient: row ids whose GM_COMP_RANK string already earned its one
    // disagreement warning (A1), so a wrong string does not log once a second
    AutoArray<RString> _rankWarned;
};

// Journal.cpp:19-33 anchor pattern: referencing this from LegendRegistry.cpp
// forces the command TU into the link.
void EnsureLegendRegistryCommandsLinked();

} // namespace Guerrilla
} // namespace Poseidon
