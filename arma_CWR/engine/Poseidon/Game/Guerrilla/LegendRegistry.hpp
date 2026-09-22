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
//                   _seeded seeding tick, then 1 Hz PollCompanions(),
//                   SpawnBosses() and BossTick().
//
// THE THREE ENEMY LEGENDS (Change 3), stated once because every one of these is
// a decision someone will otherwise try to "fix":
//   * They stand OUTSIDE the zone presence radius (the inner ring is 350 m, the
//     default zoneArea 150 m), so a commander never garrisons his zone: the
//     outpost is capturable while he lives, his existence never pins the zone's
//     alert state, and killing him is a SEPARATE objective in either order.
//   * A boss NEVER respawns.  row.spawned is persisted and latched BEFORE the
//     actors are created, so a fault between the two costs a commander rather
//     than duplicating one.
//   * Destroying a tank does NOT complete the objective.  row.body (the named
//     commander) and row.vehicle (the hull) are two independent links and two
//     independent questions; the hull's death prunes the link and nothing else.
//     The commander often, but not always, survives it - Transport::Destroy
//     deals the crew 0.5..1.0 - and if he does he fights on foot.
//   * Boss groups are invisible to GarrisonCache and to qrf.sqs BY
//     CONSTRUCTION (neither is ever told about them).  They are NOT hidden from
//     the Undercover system, which walks every occupier group unfiltered: a
//     Legend's bodyguards can blow the player's cover, which is accepted as
//     fair play rather than worked around.
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
#include <Poseidon/Game/Guerrilla/LegendPlacement.hpp> // boss roles and stands

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
    // how many bodies stand with him: one for a sniper, four for a commander,
    // the hull's remaining crew seats for a tank commander
    int guardCount = 0;
    // the body link RESOLVED at least once.  This, not link nullness, is what
    // separates "his body was deleted after he died" from "he was never
    // spawned": an LLink to a corpse does not null, only a deleted one does.
    bool bodySeen = false;
    // guards[] holds a TANK CREW rather than bodyguards.  This is the only fact
    // that separates the two populations afterwards: roleResolved still reads
    // "Tank Commander" when the hull refused to materialize and the men in
    // guards[] are on foot, and row.vehicle is cleared the moment the hull
    // dies.  Only a crew is movement-pinned; bodyguards must be free to defend.
    bool crewed = false;
    RString markerName;         // "gmLegend_<id>", written once at spawn
    bool markerPainted = false; // the map marker carries the DEFEATED paint

    // TRANSIENT, never serialized: where the body was last seen while alive, so
    // the death deed and the death diary line can name a place.
    Vector3 lastPos = VZero;
    RString lastZone;
    // TRANSIENT: the post-load re-assert has run over this row this session.
    bool reasserted = false;

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

    // --- the enemy Legends (Change 3) -------------------------------------
    // A commander is a campaign-scale actor: better than any garrison body, and
    // his retinue is better than a conscript.  Normal damage rules; no
    // allowDammage anywhere.
    static constexpr float kBossSkill = 0.9f;
    static constexpr float kGuardSkill = 0.75f;
    static constexpr int kSniperGuards = 1;
    static constexpr int kEliteGuards = 4;
    static constexpr float kGuardRadiusMin = 8.0f, kGuardRadiusMax = 15.0f;
    // The defeated marker is deliberately NOT ColorBlack: this game's own map
    // language already spends black on an unrevealed zone (ZoneRegistry::
    // UpdateMarkers), so a black Legend marker reads as an unscouted one.
    static constexpr const char* kMarkerType = "Warning";
    static constexpr const char* kMarkerColorLive = "ColorRed";
    static constexpr const char* kMarkerColorDefeated = "ColorGreen";

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

    // enemy Legend queries (the gm* readers and the suite) --------------------
    int BossCount() const;     // rows of kind LKBoss
    int DefeatedCount() const; // ... of those, the ones already killed
    // the persisted stand; VZero for a row that never found one
    Vector3 RowPos(int row) const;
    // the live actors; null when never spawned, deleted, or out of range.  A
    // DEAD body is still returned - aliveness is a separate question, and
    // asking it of a link is the bug this feature is careful about.
    Object* RowBody(int row) const;
    Object* RowVehicle(int row) const;

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

    // The half of Bind that actually stamps the identity, addressed by ROW
    // rather than by companion index.  A boss row has compIndex == -1, so
    // Bind's FindByCompIndex lookup can never reach it and would leave the
    // commander wearing the createUnit pool name (and row.body unset, which
    // would make the defeat poll and all four refs inert).  Bind is now the
    // companion-side lookup in front of this.
    bool BindRow(int rowIndex, Object* body);

    // repaint: _revision always, the journal's revision only when something the
    // dossier renders moved.  Public so the suite can pin the two edges.
    void Touch(bool dossierVisible);
    int LadderIndexForXp(float xp) const;

    // test seams ------------------------------------------------------------
    typedef LegendCompanionSnapshot CompanionSnapshot;
    void SeedForTest(unsigned seed, const HistoryInputs& in);
    void PollCompanionsForTest(const CompanionSnapshot& snapshot);
    void SetProgressionForTest(bool on) { _progression = on; }
    // ResolveBosses with the world's three answers injected: the faction's
    // capability, the zone ladder's output and the candidate stands.
    void ResolveBossesForTest(const LegendRoleCapability& cap, const AutoArray<LegendZoneCandidate>& zones,
                              const AutoArray<LegendSpotSample>& samples);
    // One boss poll with the two world questions injected: is his body alive,
    // and did his hull just die.  The live BossTick asks the world instead.
    void BossTickForTest(int row, bool alive, bool hullDestroyed);
    // The spawn that latched and then failed - a null group at MaxGroups, a
    // body class that would not materialize.  Drives the same two steps the
    // live SpawnBosses takes on that path, so the "he never existed" state can
    // be asserted without a world to fail a spawn in.
    // keepPlacement reproduces the row an EARLIER build wrote on that path,
    // which kept its stand and so advertised a commander who did not exist:
    // that is the state the load pass's bodySeen gates defend against.
    void SpawnFailureForTest(int row, bool keepPlacement = false);

  private:
    void SeedCampaign();
    void PollCompanions();
    void ApplySnapshot(const CompanionSnapshot& snapshot, bool live);
    int EnsureCompanionRow(int compIndex, const RString& baseName, float xp, bool alive);
    void AwardSlot(LegendRow& row, int which);
    void RecordDeed(LegendRow& row, const RString& text, int kind);
    // zoneOverride wins over the row's transient lastZone.  A boss stands 350 m
    // or more OUTSIDE his zone, so lastZone would resolve to the nearest zone
    // (routinely the Camp) or to the player's - and that string is also the
    // entry's zone COLUMN, which the zone Record page filters on.  His entries
    // are filed under the zone he was placed to watch.
    void WriteEntry(const LegendRow& row, const RString& text, int kind, const RString& zoneOverride = RString());
    void PreRollBossIdentities();
    void ReconcileAfterLoad();
    unsigned long long RowKey(const RString& id) const;

    // --- enemy Legends ------------------------------------------------------
    void ResolveBosses(); // from SeedCampaign, after PreRollBossIdentities
    void ApplyBossResolution(const LegendRoleCapability& cap, const AutoArray<LegendZoneCandidate>& zones,
                             const AutoArray<LegendSpotSample>& samples, float zoneArea);
    void EnsureBossRoles(); // rebuild the transient role cache when it is cold
    int BossOrdinal(const LegendRow& row) const;
    void SpawnBosses();
    bool SpawnOneBoss(LegendRow& row);
    // A spawn that latched and then failed: the placement goes with it, so the
    // row has no stand, no marker and no objective for anything to disagree
    // with, and the dossier reads him as missing intelligence.
    void LatchSpawnFailure(LegendRow& row);
    void BossTick();
    void LatchDefeat(LegendRow& row);
    void CreateBossMarker(LegendRow& row);
    void RepaintBossMarker(LegendRow& row);
    void AssertBossObjective(LegendRow& row);
    void ReassertBossActors();
    void PinBossActors(LegendRow& row);
    void PruneBossHull(LegendRow& row);

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
    // transient: the three resolved boss profiles (classes and seat counts).
    // Only role/roleRequested/roleResolved/guardCount are persisted; the class
    // names are re-derived from the live faction, which is all the spawn pass
    // needs because a loaded campaign never spawns a boss again.
    AutoArray<LegendRoleResolution> _bossRoles;
    // TRANSIENT.  Set by the load pass, consumed by the first BossTick after
    // it, because no marker may be created or grown while the archive walk is
    // still running: Serialize on PassSecond re-walks the marker array with
    // n = markersMap.Size() and no re-read of "items" (ParamArchive.hpp), so a
    // marker appended between the Legends block (WorldImpl.cpp:2140) and the
    // marker array (:2203) makes SerializeArrayItem ask for an Item the archive
    // does not have and puts the whole load into an error state.  Do NOT
    // "simplify" the deferral back into ReconcileAfterLoad.
    bool _loadReassertPending = false;
};

// Journal.cpp:19-33 anchor pattern: referencing this from LegendRegistry.cpp
// forces the command TU into the link.
void EnsureLegendRegistryCommandsLinked();

} // namespace Guerrilla
} // namespace Poseidon
