#include <Poseidon/Game/Guerrilla/LegendRegistry.hpp>

#include <Poseidon/Game/Guerrilla/Journal.hpp>
#include <Poseidon/Game/Guerrilla/ScriptVars.hpp>
#include <Poseidon/Game/Guerrilla/WorldNames.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars (CfgFaces validation)
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Poseidon/Game/Commands/GameStateExt.hpp> // GameDataObject (ScriptVars::ObjectAt)
#include <Poseidon/World/World.hpp>                // GWorld (the real player, the clock)
#include <Poseidon/World/Scene/Object.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/AI/AI.hpp>        // AIUnit::LSAlive
#include <Poseidon/AI/VehicleAI.hpp> // AIUnitInfo

#include <Random/randomGen.hpp> // GRandGen: the ONE stateful draw of the campaign

#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <math.h>
#include <stdio.h>
#include <string.h>

using namespace Poseidon;

namespace Poseidon::Guerrilla
{

// Defined in LegendRegistryCommands.cpp.  Referencing it from here forces the
// command TU (whose only other content is file-static commands plus an
// INIT_MODULE registration) into the link - same pattern as
// EnsureJournalCommandsLinked.
void EnsureLegendRegistryCommandsLinked();

// Process-lifetime singleton - no global constructor (see express.hpp's
// GGameState for the convention).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
LegendRegistry& LegendRegistry::Instance()
{
    EnsureLegendRegistryCommandsLinked();
    static LegendRegistry instance;
    return instance;
}
#pragma clang diagnostic pop

// companions.sqs:37 GM_COMP_XPTHRESH, verbatim.
const float LegendRegistry::kXpThresholds[LegendRegistry::NRankLadder] = {0, 100, 250, 500, 850, 1300, 1900};
// companions.sqs:36 GM_COMP_RANKLADDER, verbatim.  NOT the engine's Rank enum,
// which spells the fourth rung LIEUTNANT (AICenter.cpp EnumName) - the diary has
// to read like the script's own promotion, and the UI layer's RankShort is out
// of reach from the Game layer.
const char* const LegendRegistry::kRankLadder[LegendRegistry::NRankLadder] = {
    "PRIVATE", "CORPORAL", "SERGEANT", "LIEUTENANT", "CAPTAIN", "MAJOR", "COLONEL"};
const int LegendRegistry::kMilestoneDays[LegendRegistry::NMilestones] = {10, 30, 60, 100};
// Four Classic 1.99 male face tokens.  Validated at bind time rather than
// trusted (see FaceUsable): the CfgFaces `woman` flag is unmeasured, so a
// mismatch degrades to "Default" and the dossier draws its no-photograph
// treatment instead of a wrong head.
const char* const LegendRegistry::kPortraitFaces[LegendRegistry::NPortraitFaces] = {"Face10", "Face18", "Face27",
                                                                                    "Face33"};

// ---------------------------------------------------------------------------
// ScriptVars::ObjectAt - declared in ScriptVars.hpp, defined here because it is
// the only reader that needs the command layer's GameDataObject and the world's
// Object.  Same idiom as GameStateExtObj.cpp's ObjIsNull.
// ---------------------------------------------------------------------------

namespace ScriptVars
{
Object* ObjectAt(const char* name, int index)
{
    if (!name || index < 0)
    {
        return nullptr;
    }
    GameValue value = GGameState.VarGet(name);
    if (value.GetType() != GameArray)
    {
        return nullptr;
    }
    const GameArrayType& array = value;
    if (index >= array.Size() || array[index].GetType() != GameObject)
    {
        return nullptr;
    }
    return static_cast<GameDataObject*>(array[index].GetData())->GetObject();
}
} // namespace ScriptVars

// ---------------------------------------------------------------------------
// file-static helpers
// ---------------------------------------------------------------------------

// The three nickname slots, in the fixed order every award draw walks.
static const int kNicknameSlots[3] = {SlotPrefix, SlotDescriber, SlotTitle};

// D2.1: slug = base name lowercased, [a-z0-9] only, 12 chars, "x" when empty.
// Because the slug can never contain '_', an id is always exactly three tokens,
// so a Render height-split page GM_WHO_comp_0_petra_2 can never collide with a
// real id's dossier page.
static void NameSlug(const char* base, char* out, int outSize)
{
    int n = 0;
    for (const char* p = base; p && *p && n < outSize - 1 && n < 12; p++)
    {
        const char c = *p;
        if (c >= 'a' && c <= 'z')
        {
            out[n++] = c;
        }
        else if (c >= 'A' && c <= 'Z')
        {
            out[n++] = (char)(c - 'A' + 'a');
        }
        else if (c >= '0' && c <= '9')
        {
            out[n++] = c;
        }
    }
    if (n == 0)
    {
        out[n++] = 'x';
    }
    out[n] = 0;
}

static RString CompanionRowId(int compIndex, const RString& baseName)
{
    char slug[16];
    NameSlug(baseName, slug, sizeof(slug));
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "comp_%d_%s", compIndex, slug);
    return RString(buffer);
}

// The comparable core of a nickname word: the leading "The " an epithet carries
// is not part of what makes two slots read as the same word.
static const char* WordCore(const char* word)
{
    if (word && strnicmp(word, "The ", 4) == 0)
    {
        return word + 4;
    }
    return word ? word : "";
}

static bool SlotWordClashes(const LegendRow& row, const char* word)
{
    const char* core = WordCore(word);
    if (*core == 0)
    {
        return false;
    }
    const RString* filled[3] = {&row.prefix, &row.describer, &row.title};
    for (int i = 0; i < 3; i++)
    {
        if (filled[i]->GetLength() > 0 && stricmp(WordCore(filled[i]->Data()), core) == 0)
        {
            return true;
        }
    }
    return false;
}

// PickSlotWord, then walk the bank circularly past any word whose core already
// sits in another of this row's slots.  Without it the shipped friendly bank can
// produce "Fearless Petra Kovacevic The Fearless" (prefix "Fearless" alongside
// title "The Fearless") from two independent rolls.
static RString PickSlotWordDistinct(NicknameTone tone, int slot, unsigned long long key, unsigned channel,
                                    const LegendRow& row)
{
    const NicknameBank& bank = Bank(tone);
    const char* const* words = nullptr;
    int count = 0;
    switch (slot)
    {
        case SlotPrefix:
            words = bank.prefix;
            count = bank.nPrefix;
            break;
        case SlotDescriber:
            words = bank.describer;
            count = bank.nDescriber;
            break;
        case SlotTitle:
            words = bank.title;
            count = bank.nTitle;
            break;
        default:
            return RString();
    }
    if (!words || count <= 0)
    {
        return RString();
    }
    const int start = (int)Roll(key, channel, count);
    for (int i = 0; i < count; i++)
    {
        const char* candidate = words[(start + i) % count];
        if (!SlotWordClashes(row, candidate))
        {
            return RString(candidate);
        }
    }
    return RString(words[start]);
}

// Walk the pool circularly from the rolled index to the first entry not already
// spent.  Three bosses drawn independently from one ten-name pool share a first
// name in 28% of campaigns; this makes the result a pure function of the seed
// AND distinct.
static RString PickDistinctName(int pool, unsigned long long key, unsigned channel, bool firstName,
                                const AutoArray<RString>& used)
{
    const NamePool& p = NamePoolAt(pool);
    const char* const* list = firstName ? p.first : p.last;
    const int count = firstName ? p.nFirst : p.nLast;
    if (!list || count <= 0)
    {
        return RString();
    }
    const int start = (int)Roll(key, channel, count);
    for (int i = 0; i < count; i++)
    {
        const char* candidate = list[(start + i) % count];
        bool taken = false;
        for (int u = 0; u < used.Size(); u++)
        {
            if (stricmp(used[u], candidate) == 0)
            {
                taken = true;
                break;
            }
        }
        if (!taken)
        {
            return RString(candidate);
        }
    }
    return RString(list[start]);
}

// D2.2: the rolled token is validated against the package rather than trusted.
// With no config at all (every unit test) the rolled token stands.
static bool FaceUsable(const char* token, bool bodyIsWoman)
{
    const ParamEntry* faces = Pars.FindEntry("CfgFaces");
    if (!faces)
    {
        return true;
    }
    const ParamEntry* face = token ? faces->FindEntry(token) : nullptr;
    if (!face)
    {
        return false;
    }
    if (face->FindEntry("disabled"))
    {
        return false;
    }
    const bool woman = face->ReadValue("woman", 0.0f) > 0.5f;
    return woman == bodyIsWoman;
}

// The live body behind GM_COMP_OBJ select i, or null.  Market.cpp's idiom.
static Person* CompanionBody(int index)
{
    return dyn_cast<Person>(ScriptVars::ObjectAt("gm_comp_obj", index));
}

static bool BodyIsAlive(Person* person)
{
    AIUnit* unit = person ? person->Brain() : nullptr;
    return unit && unit->GetLifeState() == AIUnit::LSAlive;
}

static float Dist2D(Vector3Par a, Vector3Par b)
{
    const float dx = a.X() - b.X();
    const float dz = a.Z() - b.Z();
    return sqrtf(dx * dx + dz * dz);
}

// The zone a death is remembered by: the nearest zone within the registry's own
// cache radius, falling back to the player's zone, then to "".
static RString NearestZoneName(Vector3Par pos)
{
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    const float limit = registry.Tuning().cacheRadius;
    RString best;
    float bestDist = limit;
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* zone = registry.GetZone(i);
        if (!zone)
        {
            continue;
        }
        const float d = Dist2D(pos, zone->pos);
        if (d <= bestDist)
        {
            bestDist = d;
            best = zone->name;
        }
    }
    return best;
}

// A6's fallback half: the zone the player is standing in.  It is folded INTO
// row.lastZone rather than used beside it, because WriteEntry files every diary
// line under row.lastZone: a zone resolved anywhere else would put a place in
// the prose that the entry's own zone column does not carry, and the zone's
// Record page filters strictly on that column.
static RString PlayerZoneName()
{
    Person* player = GWorld ? GWorld->GetRealPlayer() : nullptr;
    return player ? NearestZoneName(player->Position()) : RString();
}

// FNV-1a over the observation, so an unchanged roster costs one hash per second
// and never repaints.  NOT a generation path: no persisted value depends on it.
static void DigestBytes(unsigned long long& h, const void* data, int size)
{
    const unsigned char* p = (const unsigned char*)data;
    for (int i = 0; i < size; i++)
    {
        h ^= (unsigned long long)p[i];
        h *= 0x100000001B3ull;
    }
}

static void DigestText(unsigned long long& h, const char* text)
{
    DigestBytes(h, text ? text : "", text ? (int)strlen(text) : 0);
    h ^= 0xFFull;
    h *= 0x100000001B3ull;
}

// ---------------------------------------------------------------------------
// row serialization
// ---------------------------------------------------------------------------

LSError LegendDeed::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("stamp", stamp, 1, RString()))
    PARAM_CHECK(ar.Serialize("text", text, 1, RString()))
    PARAM_CHECK(ar.Serialize("kind", kind, 1, (int)LDPromotion))
    return LSOK;
}

LSError LegendRow::Serialize(ParamArchive& ar)
{
    // Every key uses the 4-arg default-tolerant overload (ParamArchive.hpp: on a
    // load's PassFirst an absent key writes the default), so a Change 2 row loads
    // in Change 3 and a Change 3 row loads here.
    PARAM_CHECK(ar.Serialize("id", id, 1, RString()))
    PARAM_CHECK(ar.Serialize("kind", kind, 1, (int)LKCompanion))
    PARAM_CHECK(ar.Serialize("comp", compIndex, 1, -1))
    PARAM_CHECK(ar.Serialize("base", baseName, 1, RString()))
    PARAM_CHECK(ar.Serialize("first", first, 1, RString()))
    PARAM_CHECK(ar.Serialize("last", last, 1, RString()))
    PARAM_CHECK(ar.Serialize("prefix", prefix, 1, RString()))
    PARAM_CHECK(ar.Serialize("describer", describer, 1, RString()))
    PARAM_CHECK(ar.Serialize("title", title, 1, RString()))
    PARAM_CHECK(ar.Serialize("awards", awardMask, 1, 0))
    PARAM_CHECK(ar.Serialize("legend", legend, 1, false))
    PARAM_CHECK(ar.Serialize("face", face, 1, RString()))
    PARAM_CHECK(ar.Serialize("class", bodyClass, 1, RString()))
    PARAM_CHECK(ar.Serialize("outfit", outfit, 1, 0))
    PARAM_CHECK(ar.Serialize("pool", namePool, 1, -1))
    PARAM_CHECK(ar.Serialize("tone", tone, 1, (int)ToneFriendly))
    PARAM_CHECK(ar.Serialize("bio", bio, 1, RString()))
    PARAM_CHECK(ar.Serialize("bioEvent", bioEvent, 1, 0))
    PARAM_CHECK(ar.Serialize("rank", rankSeen, 1, -1))
    PARAM_CHECK(ar.Serialize("xp", xpSeen, 1, 0.0f))
    PARAM_CHECK(ar.Serialize("alive", alive, 1, true))
    PARAM_CHECK(ar.Serialize("deathDay", deathDay, 1, 0))
    PARAM_CHECK(ar.Serialize("deathMin", deathMinute, 1, 0))
    PARAM_CHECK(ar.Serialize("days", daysSurvived, 1, 0))
    PARAM_CHECK(ar.Serialize("Deeds", deeds, 1))
    // Change 3's fields, written now so the row format never changes under a
    // save.  role == "Commander" is the not-yet-resolved sentinel.
    PARAM_CHECK(ar.Serialize("role", role, 1, RString()))
    PARAM_CHECK(ar.Serialize("roleReq", roleRequested, 1, RString()))
    PARAM_CHECK(ar.Serialize("roleRes", roleResolved, 1, RString()))
    PARAM_CHECK(ar.Serialize("zone", zoneName, 1, RString()))
    PARAM_CHECK(ar.Serialize("pos", pos, 1, VZero))
    PARAM_CHECK(ar.Serialize("spawned", spawned, 1, false))
    PARAM_CHECK(ar.Serialize("defeated", defeated, 1, false))
    // the refs resolve on the second load pass, after the world's serializers
    // have recreated the bodies
    PARAM_CHECK(ar.SerializeRef("body", body, 1))
    PARAM_CHECK(ar.SerializeRef("vehicle", vehicle, 1))
    PARAM_CHECK(ar.SerializeRef("group", group, 1))
    PARAM_CHECK(ar.SerializeRefs("Guards", guards, 1))
    // lastPos / lastZone are TRANSIENT: they are re-derived by the next poll.
    return LSOK;
}

// ---------------------------------------------------------------------------
// lifecycle
// ---------------------------------------------------------------------------

void LegendRegistry::Clear()
{
    _rows.Clear();
    _history = HistoryRecord();
    _seed = 0;
    _progression = false;
    _seeded = false;
    _initRan = false;
    _revision = 0;
    _accum = 0;
    _resistancePool = -1;
    _occupierPool = -1;
    _resistanceName = RString();
    _occupierName = RString();
    _pollDigest = 0;
    _rankWarned.Clear();
}

void LegendRegistry::InitMission()
{
    Clear();
    LoadFromConfig();
    _initRan = true;
}

void LegendRegistry::LoadFromConfig()
{
    // Deliberately empty in Change 2: the registry reads no config of its own.
    // The shape matches its neighbours (Market, GuerrillaBase) so a later key -
    // a per-faction nickname override, say - has one obvious home.
}

void LegendRegistry::Simulate(float deltaT)
{
    // The inactive-registry early return is what keeps ordinary missions and
    // intros untouched: no seed is drawn, no row is created, no diary line is
    // written outside Guerrilla Mode.
    if (!ZoneRegistry::Instance().IsActive())
    {
        return;
    }
    if (!_seeded)
    {
        SeedCampaign();
    }
    _accum += deltaT;
    if (_accum < TickInterval)
    {
        return;
    }
    _accum = 0;
    PollCompanions();
}

// ---------------------------------------------------------------------------
// seeding
// ---------------------------------------------------------------------------

unsigned long long LegendRegistry::RowKey(const RString& id) const
{
    return HashKey(id, _seed);
}

int LegendRegistry::LadderIndexForXp(float xp) const
{
    // GM_fnCompRankIdxForXp (companions.sqs): the highest index whose threshold
    // is met.  The ONLY source of the ladder index - the GM_COMP_RANK string is
    // cross-checked and never trusted.
    int index = 0;
    for (int i = 0; i < NRankLadder; i++)
    {
        if (xp >= kXpThresholds[i])
        {
            index = i;
        }
    }
    return index;
}

void LegendRegistry::SeedCampaign()
{
    // _seeded FIRST: a throw or an early return below must never leave the
    // campaign able to re-seed and reroll its own history.
    _seeded = true;
    // The ONE draw from a stateful generator in the whole feature, persisted
    // immediately (Market::Assign's idiom).  Everything else is a pure function
    // of (_seed, a stable string key, a named channel).
    _seed = (unsigned)(toInt(GRandGen.RandomValue() * 1073741823.0f) | 1);
    _progression = true;

    const ZoneRegistry& registry = ZoneRegistry::Instance();
    const RString resistanceFaction = registry.ResistanceFaction();
    const RString occupierFaction = registry.OccupierFaction();
    const RString resistanceSide = registry.ResistanceSide();
    const RString occupierSide = registry.OccupierSide();
    const RString resistanceKey = resistanceFaction.GetLength() > 0 ? resistanceFaction : resistanceSide;
    const RString occupierKey = occupierFaction.GetLength() > 0 ? occupierFaction : occupierSide;
    _resistancePool = ResolveNamePool(registry.FactionValue(resistanceKey, "namePool"), resistanceSide, resistanceKey);
    _occupierPool = ResolveNamePool(registry.FactionValue(occupierKey, "namePool"), occupierSide, occupierKey);
    _resistanceName = FactionDisplayName(registry, resistanceFaction, resistanceSide);
    _occupierName = FactionDisplayName(registry, occupierFaction, occupierSide);

    HistoryInputs in;
    in.resistanceName = _resistanceName;
    in.occupierName = _occupierName;
    in.islandName = IslandDisplayName();
    AutoArray<PlaceName> all;
    CollectWorldPlaceNames(all);
    SettlementNames(all, in.settlements);
    FeatureNames(all, in.features);
    for (int i = 0; i < registry.NZones(); i++)
    {
        const ZoneRecord* zone = registry.GetZone(i);
        if (zone && zone->name.GetLength() > 0)
        {
            in.zoneNames.Add(zone->name);
        }
    }
    in.seed = _seed;
    _history = GenerateHistory(in);

    PreRollBossIdentities();
    LOG_INFO(Core, "Legends: campaign seeded ({}), history v{}, {} enemy Legend(s)", _seed, _history.version,
             kBossCount);
    Touch(true);
}

void LegendRegistry::SeedForTest(unsigned seed, const HistoryInputs& in)
{
    _seeded = true;
    _seed = seed | 1u; // the live draw is always odd; keep the test seeds in the same family
    _progression = true;
    if (_resistancePool < 0)
    {
        _resistancePool = ResolveNamePool(nullptr, "GUER", "test-resistance");
    }
    if (_occupierPool < 0)
    {
        _occupierPool = ResolveNamePool(nullptr, "EAST", "test-occupier");
    }
    _resistanceName = in.resistanceName;
    _occupierName = in.occupierName;
    HistoryInputs local = in;
    local.seed = _seed;
    _history = GenerateHistory(local);
    PreRollBossIdentities();
    Touch(true);
}

void LegendRegistry::PreRollBossIdentities()
{
    // D2.3: the three enemy Legends are rolled HERE, in the seeding tick, not by
    // Change 3.  Their names then come out of the same seed in the same channel
    // order whether or not Change 3 has landed, so a Change 2 save keeps its
    // Legend names when Change 3 loads it, and People > Enemy Legends has real
    // content in Change 2.
    AutoArray<RString> usedFirst, usedLast;
    for (int n = 0; n < kBossCount; n++)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "boss_%d", n);
        if (FindById(buffer) >= 0)
        {
            continue;
        }
        const int at = _rows.Add();
        LegendRow& row = _rows[at];
        row.id = RString(buffer);
        row.kind = LKBoss;
        row.compIndex = -1;
        row.namePool = _occupierPool;
        row.tone = ToneHostile;
        const unsigned long long key = RowKey(row.id);
        row.first = PickDistinctName(row.namePool, key, CH_BOSSFIRST, true, usedFirst);
        row.last = PickDistinctName(row.namePool, key, CH_BOSSLAST, false, usedLast);
        usedFirst.Add(row.first);
        usedLast.Add(row.last);
        // one awarded slot at creation: a boss is notorious from the start
        const int slot = kNicknameSlots[Roll(key, CH_BOSSSLOT, 3)];
        const RString word = PickSlotWordDistinct(ToneHostile, slot, key, CH_BOSSWORD, row);
        if (slot == SlotPrefix)
        {
            row.prefix = word;
        }
        else if (slot == SlotDescriber)
        {
            row.describer = word;
        }
        else
        {
            row.title = word;
        }
        row.legend = true;
        // both bits latched, so the companion award machinery can never fire on
        // a boss row
        row.awardMask = LAFirst | LASecond;
        row.role = "Commander"; // Change 3's not-yet-resolved sentinel
        row.face = RString(kPortraitFaces[Roll(key, CH_FACE, NPortraitFaces)]);
        row.bioEvent = (int)Roll(key, CH_BIOEVENT, kHistoryEvents);
        if (_history.Present())
        {
            // budgeted to what the dossier page can show beside the portrait
            // box, so nothing the generator writes is clamped away where it is
            // read (kHistoryBioPageWords)
            row.bio =
                GenerateBio(_history, row.bioEvent, DisplayName(row), _occupierName, true, key, kHistoryBioPageWords);
        }
    }
}

// ---------------------------------------------------------------------------
// rows
// ---------------------------------------------------------------------------

int LegendRegistry::FindByCompIndex(int compIndex) const
{
    if (compIndex < 0)
    {
        return -1;
    }
    for (int i = 0; i < _rows.Size(); i++)
    {
        if (_rows[i].kind == LKCompanion && _rows[i].compIndex == compIndex)
        {
            return i;
        }
    }
    return -1;
}

int LegendRegistry::FindById(const char* id) const
{
    if (!id || *id == 0)
    {
        return -1;
    }
    for (int i = 0; i < _rows.Size(); i++)
    {
        if (strcmp(_rows[i].id, id) == 0)
        {
            return i;
        }
    }
    return -1;
}

RString LegendRegistry::DisplayName(const LegendRow& row) const
{
    NameParts parts;
    parts.prefix = row.prefix;
    parts.first = row.first;
    parts.describer = row.describer;
    parts.last = row.last;
    parts.title = row.title;
    RString name = AssembleDisplayName(parts);
    return name.GetLength() > 0 ? name : row.baseName;
}

RString LegendRegistry::CompanionDisplayName(int compIndex) const
{
    const int i = FindByCompIndex(compIndex);
    if (i >= 0)
    {
        RString name = DisplayName(_rows[i]);
        if (name.GetLength() > 0)
        {
            return name;
        }
    }
    // No row exists until the first poll tick (~1 s after InitMission, and
    // WorldInit runs before RunInitScript), so companions.sqs' boot call to
    // GM_fnCompStatus lands here on every new campaign.  That is correct.
    AutoArray<RString> names;
    ScriptVars::StringArray("gm_comp_names", names);
    if (compIndex >= 0 && compIndex < names.Size() && names[compIndex].GetLength() > 0)
    {
        return names[compIndex];
    }
    static bool warned = false;
    if (!warned)
    {
        warned = true;
        LOG_WARN(Core, "Legends: no name for companion index {} - neither a row nor GM_COMP_NAMES has it", compIndex);
    }
    return RString();
}

int LegendRegistry::EnsureCompanionRow(int compIndex, const RString& baseName, float xp, bool alive)
{
    if (compIndex < 0)
    {
        return -1;
    }
    // A10: match by compIndex first, then by the id the scheme would produce.
    // A row's id is written once at creation and NEVER rewritten, even if the
    // base name later changes.
    int found = FindByCompIndex(compIndex);
    if (found >= 0)
    {
        return found;
    }
    const RString id = CompanionRowId(compIndex, baseName);
    found = FindById(id);
    if (found >= 0)
    {
        _rows[found].compIndex = compIndex;
        return found;
    }

    // Row order is companions by compIndex ascending, then the boss block, so a
    // new companion is INSERTED rather than appended.
    int at = 0;
    while (at < _rows.Size() && _rows[at].kind == LKCompanion && _rows[at].compIndex < compIndex)
    {
        at++;
    }
    _rows.Insert(at);
    LegendRow& row = _rows[at];
    row.id = id;
    row.kind = LKCompanion;
    row.compIndex = compIndex;
    row.baseName = baseName;
    row.first = baseName; // spec: preserve the existing base name verbatim
    row.namePool = _resistancePool >= 0 ? _resistancePool : ResolveNamePool(nullptr, "GUER", "resistance");
    row.tone = ToneFriendly;
    const unsigned long long key = RowKey(row.id);
    // A DERIVED campaign has no seed and no history, so it gets no generated
    // surname and no biography: its dossiers are exactly "derived from available
    // companion records", i.e. DisplayName() is the base name.
    if (_progression)
    {
        row.last = PickLast(row.namePool, key, CH_LAST);
    }
    row.face = RString(kPortraitFaces[Roll(key, CH_FACE, NPortraitFaces)]);
    row.alive = alive;
    // The creating observation is a BASELINE, not an event: the shipped core
    // seeds companions at XP 100 / CORPORAL, so a rankSeen left at -1 would make
    // every brand-new campaign open with a spurious "Petra promoted to CORPORAL"
    // diary line and a promotion deed.
    row.xpSeen = xp;
    row.rankSeen = LadderIndexForXp(xp);
    if (_progression && _history.Present())
    {
        row.bioEvent = (int)Roll(key, CH_BIOEVENT, kHistoryEvents);
        // the dossier page's budget, not the generator's wider band: see the
        // boss branch in PreRollBossIdentities
        row.bio = GenerateBio(_history, row.bioEvent, row.baseName, _resistanceName, false, key, kHistoryBioPageWords);
    }
    // A template that seeds a companion above an award threshold gets the earned
    // words at creation, SILENTLY: the bits and the slot words are part of the
    // baseline, the diary line and the deed are not.
    if (_progression)
    {
        if ((row.awardMask & LAFirst) == 0 && row.rankSeen >= kFirstAwardRank)
        {
            AwardSlot(row, 0);
        }
        if ((row.awardMask & LASecond) == 0 && row.rankSeen >= kSecondAwardRank)
        {
            AwardSlot(row, 1);
        }
    }
    return at;
}

void LegendRegistry::AwardSlot(LegendRow& row, int which)
{
    const unsigned long long key = RowKey(row.id);
    // A2: the bit is set BEFORE the word is written, so a re-entrant poll cannot
    // double fire.
    const int firstIdx = (int)Roll(key, CH_AWARD1, 3);
    int slotIdx = firstIdx;
    unsigned channel = CH_AWARD1WORD;
    if (which == 0)
    {
        row.awardMask |= LAFirst;
    }
    else
    {
        row.awardMask |= LASecond;
        row.legend = true;
        // the (n+1)-th DIFFERENT slot, walking the same three in order
        slotIdx = (firstIdx + 1 + (int)Roll(key, CH_AWARD2, 2)) % 3;
        channel = CH_AWARD2WORD;
    }
    const int slot = kNicknameSlots[slotIdx];
    const RString word = PickSlotWordDistinct((NicknameTone)row.tone, slot, key, channel, row);
    if (word.GetLength() == 0)
    {
        return;
    }
    if (slot == SlotPrefix)
    {
        row.prefix = word;
    }
    else if (slot == SlotDescriber)
    {
        row.describer = word;
    }
    else
    {
        row.title = word;
    }
}

void LegendRegistry::RecordDeed(LegendRow& row, const RString& text, int kind)
{
    if (text.GetLength() == 0)
    {
        return;
    }
    const int at = row.deeds.Add();
    LegendDeed& deed = row.deeds[at];
    deed.stamp = JournalStampNow();
    deed.text = text;
    deed.kind = kind;
    // A5: past the cap the OLDEST NON-AWARD, NON-DEATH deed is dropped, so the
    // earned names and the death line are never the ones evicted.
    while (row.deeds.Size() > kMaxDeeds)
    {
        int victim = -1;
        for (int i = 0; i < row.deeds.Size(); i++)
        {
            if (row.deeds[i].kind != LDAward && row.deeds[i].kind != LDDeath)
            {
                victim = i;
                break;
            }
        }
        if (victim < 0)
        {
            victim = 0; // every deed is an award or the death line: drop the oldest
        }
        row.deeds.Delete(victim);
    }
}

void LegendRegistry::WriteEntry(const LegendRow& row, const RString& text, int kind)
{
    // A7 / D2.5: only a campaign this build seeded writes diary lines.  A DERIVED
    // campaign's own serialized companions.sqs is still writing its own.
    if (!_progression || text.GetLength() == 0)
    {
        return;
    }
    Journal::Instance().AddEntry(JournalStampNow(), text, row.lastZone, kind, row.id);
}

void LegendRegistry::Touch(bool dossierVisible)
{
    _revision++;
    if (dossierVisible)
    {
        Journal::Instance().Touch();
    }
}

// ---------------------------------------------------------------------------
// binding
// ---------------------------------------------------------------------------

bool LegendRegistry::Bind(int compIndex, Object* body)
{
    Person* person = dyn_cast<Person>(body);
    if (!person)
    {
        return false;
    }
    // Rows are otherwise created only by the 1 Hz poll, and GM_fnCompSpawn runs
    // on the 5 s companion loop, so without this the very first body would wear
    // the createUnit pool identity for up to a second.
    int r = FindByCompIndex(compIndex);
    if (r < 0)
    {
        // A row built before the campaign is seeded keeps the un-seeded
        // defaults FOREVER - no surname, no biography, the side-fallback name
        // pool - because nothing revisits those fields once the row exists.
        // gmLegendBind can arrive first: SimulateScripts runs before the
        // registry's own Simulate, so a mission init script binding on frame
        // one precedes SeedCampaign.  Seed here, and outside Guerrilla Mode
        // create no row at all, which is the invariant Simulate() states.
        if (!_seeded)
        {
            if (!ZoneRegistry::Instance().IsActive())
            {
                return false;
            }
            SeedCampaign();
        }
        AutoArray<RString> names;
        ScriptVars::StringArray("gm_comp_names", names);
        const RString base = (compIndex >= 0 && compIndex < names.Size()) ? names[compIndex] : RString();
        AutoArray<float> xp;
        ScriptVars::ScalarArray("gm_comp_xp", xp);
        const float seenXp = (compIndex >= 0 && compIndex < xp.Size()) ? xp[compIndex] : 0.0f;
        r = EnsureCompanionRow(compIndex, base, seenXp, true);
        if (r >= 0)
        {
            Touch(true);
        }
    }
    if (r < 0)
    {
        return false;
    }
    LegendRow& row = _rows[r];
    const RString display = DisplayName(row);
    if (display.GetLength() == 0)
    {
        return false;
    }

    // D2.2: validate the rolled token against the package rather than trust it.
    // On failure walk the remaining tokens in order, then fall back to "Default",
    // which Gather reads as "no portrait" so the dossier draws its no-photograph
    // treatment instead of a wrong head.
    const bool woman = person->IsWoman();
    RString face = row.face;
    if (face.GetLength() == 0 || !FaceUsable(face, woman))
    {
        face = RString();
        for (int i = 0; i < NPortraitFaces; i++)
        {
            if (FaceUsable(kPortraitFaces[i], woman))
            {
                face = RString(kPortraitFaces[i]);
                break;
            }
        }
        if (face.GetLength() == 0)
        {
            face = "Default";
        }
    }

    bool changed = false;
    if (strcmp(row.face, face) != 0)
    {
        row.face = face;
        changed = true;
    }
    const EntityType* type = person->GetNonAIType();
    const RString className = type ? RString(type->GetName()) : RString();
    if (strcmp(row.bodyClass, className) != 0)
    {
        row.bodyClass = className;
        changed = true;
    }
    row.body = body;

    AIUnitInfo& info = person->GetInfo();
    info._name = display;
    info._face = face;
    // the pool identity the engine stamped at createUnit is not ours
    info._identityContext = RString();
    // Person::SetFace is an EMPTY base implementation overridden only by Man, so
    // this is a silent no-op on a non-Man Person; _face still rides
    // AIUnit::Serialize and is re-applied on load, which is the durable half.
    person->SetFace(info._face);
    if (changed)
    {
        Touch(true);
    }
    return true;
}

// ---------------------------------------------------------------------------
// polling
// ---------------------------------------------------------------------------

void LegendRegistry::PollCompanions()
{
    CompanionSnapshot snapshot;
    ScriptVars::StringArray("gm_comp_names", snapshot.names);
    ScriptVars::ScalarArray("gm_comp_xp", snapshot.xp);
    ScriptVars::StringArray("gm_comp_rank", snapshot.rank);
    ScriptVars::BoolArray("gm_comp_alive", snapshot.alive);
    JournalClockNow(snapshot.day, snapshot.minuteOfDay);

    // A8: return early when nothing moved.  Without this the open map would
    // rebuild once a second while the journal is up.
    unsigned long long digest = 0xCBF29CE484222325ull;
    for (int i = 0; i < snapshot.names.Size(); i++)
    {
        DigestText(digest, snapshot.names[i]);
        const float xp = i < snapshot.xp.Size() ? snapshot.xp[i] : 0.0f;
        DigestBytes(digest, &xp, (int)sizeof(xp));
        DigestText(digest, i < snapshot.rank.Size() ? snapshot.rank[i].Data() : "");
        const unsigned char alive = (i < snapshot.alive.Size() && snapshot.alive[i]) ? 1 : 0;
        DigestBytes(digest, &alive, 1);
        Person* person = CompanionBody(i);
        const unsigned char state = person ? (BodyIsAlive(person) ? 1 : 2) : 0;
        DigestBytes(digest, &state, 1);
        DigestText(digest, person ? person->GetInfo()._name.Data() : "");
    }
    DigestBytes(digest, &snapshot.day, (int)sizeof(snapshot.day));
    if (digest == _pollDigest)
    {
        return;
    }
    _pollDigest = digest;
    ApplySnapshot(snapshot, true);
}

void LegendRegistry::PollCompanionsForTest(const CompanionSnapshot& snapshot)
{
    ApplySnapshot(snapshot, false);
}

void LegendRegistry::ApplySnapshot(const CompanionSnapshot& snapshot, bool live)
{
    bool touched = false;
    bool wroteJournal = false;

    for (int i = 0; i < snapshot.names.Size(); i++)
    {
        const RString base = snapshot.names[i];
        const float xp = i < snapshot.xp.Size() ? snapshot.xp[i] : 0.0f;
        const bool scriptAlive = i < snapshot.alive.Size() ? snapshot.alive[i] : true;
        const bool created = FindByCompIndex(i) < 0;
        const int r = EnsureCompanionRow(i, base, xp, scriptAlive);
        if (r < 0)
        {
            continue;
        }
        Person* person = live ? CompanionBody(i) : nullptr;

        if (created)
        {
            // The observation that created the row emits NOTHING: no deed, no
            // diary line, no award line.  Only the row itself is news.
            LegendRow& row = _rows[r];
            if (!scriptAlive)
            {
                row.alive = false;
                row.deathDay = snapshot.day;
                row.deathMinute = snapshot.minuteOfDay;
            }
            else if (snapshot.day > row.daysSurvived)
            {
                row.daysSurvived = snapshot.day;
            }
            touched = true;
            if (person && BodyIsAlive(person))
            {
                Bind(i, person);
            }
            continue;
        }

        LegendRow& row = _rows[r];

        // --- the transient place cache (A6) ------------------------------
        // Refreshed BEFORE the ladder, the awards and the death latch, so a
        // diary line written by THIS poll is filed under the zone the deed
        // happened in.  WriteEntry always passes row.lastZone, so this one
        // cached value is the source of both the prose and the entry's zone
        // column, and a promotion no longer lands with an empty column (which
        // would keep it off the zone's Record page).  The identity re-assert
        // stays below: it must run after the awards, or an earned name would
        // reach the live body a whole poll late.
        if (person && BodyIsAlive(person))
        {
            row.lastPos = person->Position();
            row.lastZone = NearestZoneName(row.lastPos);
            if (row.lastZone.GetLength() == 0)
            {
                row.lastZone = PlayerZoneName();
            }
        }

        // the script may rename a companion; the id never follows it
        if (base.GetLength() > 0 && strcmp(row.baseName, base) != 0)
        {
            row.baseName = base;
            row.first = base;
            touched = true;
        }

        // --- rank ladder (A1) -------------------------------------------
        const int ladder = LadderIndexForXp(xp);
        if (i < snapshot.rank.Size() && snapshot.rank[i].GetLength() > 0 &&
            stricmp(snapshot.rank[i], kRankLadder[ladder]) != 0)
        {
            bool warned = false;
            for (int w = 0; w < _rankWarned.Size(); w++)
            {
                if (strcmp(_rankWarned[w], row.id) == 0)
                {
                    warned = true;
                    break;
                }
            }
            if (!warned)
            {
                _rankWarned.Add(row.id);
                LOG_WARN(Core, "Legends: '{}' reports rank '{}' at {} XP; the XP ladder says '{}' and wins",
                         (const char*)row.id, (const char*)snapshot.rank[i], (double)xp, kRankLadder[ladder]);
            }
        }
        row.xpSeen = xp;
        if (ladder > row.rankSeen)
        {
            row.rankSeen = ladder;
            char line[256];
            snprintf(line, sizeof(line), "Promoted to %s.", kRankLadder[ladder]);
            RecordDeed(row, RString(line), LDPromotion);
            snprintf(line, sizeof(line), "%s promoted to %s.", (const char*)DisplayName(row), kRankLadder[ladder]);
            WriteEntry(row, RString(line), JKGood);
            wroteJournal = wroteJournal || _progression;
            touched = true;
        }
        else if (ladder != row.rankSeen)
        {
            row.rankSeen = ladder; // a demotion is bookkeeping, not news
        }

        // --- awards (A2 / A3 / A4) --------------------------------------
        if (_progression)
        {
            const int thresholds[2] = {kFirstAwardRank, kSecondAwardRank};
            const int bits[2] = {LAFirst, LASecond};
            for (int a = 0; a < 2; a++)
            {
                if ((row.awardMask & bits[a]) != 0 || row.rankSeen < thresholds[a])
                {
                    continue;
                }
                AwardSlot(row, a);
                const RString after = DisplayName(row);
                char line[320];
                if (a == 0)
                {
                    snprintf(line, sizeof(line), "Took the name %s.", (const char*)after);
                    RecordDeed(row, RString(line), LDAward);
                    snprintf(line, sizeof(line), "%s is now known as %s.", (const char*)row.baseName,
                             (const char*)after);
                }
                else
                {
                    RecordDeed(row, RString("Became a legend of the resistance."), LDAward);
                    snprintf(line, sizeof(line), "%s has become a legend of the resistance.", (const char*)after);
                }
                WriteEntry(row, RString(line), JKGood);
                wroteJournal = true;
                touched = true;
            }
        }

        // --- body and identity (A9) --------------------------------------
        if (person)
        {
            row.body = person;
            if (BodyIsAlive(person) && strcmp(person->GetInfo()._name, DisplayName(row)) != 0)
            {
                Bind(i, person);
            }
        }

        // --- death (A6) --------------------------------------------------
        const bool bodyDead = person && !BodyIsAlive(person);
        if (row.alive && (bodyDead || !scriptAlive))
        {
            row.alive = false;
            row.deathDay = snapshot.day;
            row.deathMinute = snapshot.minuteOfDay;
            // The body is already down, so the cache refresh above did not run
            // this tick: apply A6's player fallback to the CACHED FIELD, never
            // to a local, so the deed prose, the diary prose and the entry's
            // zone column are the same string by construction.
            if (live && row.lastZone.GetLength() == 0)
            {
                row.lastZone = PlayerZoneName();
            }
            const RString zone = row.lastZone;
            char line[320];
            if (zone.GetLength() > 0)
            {
                snprintf(line, sizeof(line), "Fell on Day %d near %s.", snapshot.day, (const char*)zone);
            }
            else
            {
                snprintf(line, sizeof(line), "Fell on Day %d.", snapshot.day);
            }
            RecordDeed(row, RString(line), LDDeath);
            if (zone.GetLength() > 0)
            {
                snprintf(line, sizeof(line), "%s fell near %s.", (const char*)DisplayName(row), (const char*)zone);
            }
            else
            {
                snprintf(line, sizeof(line), "%s has fallen.", (const char*)DisplayName(row));
            }
            WriteEntry(row, RString(line), JKDanger);
            wroteJournal = wroteJournal || _progression;
            touched = true;
        }

        // --- survival milestones (A5, deeds only) ------------------------
        if (row.alive && snapshot.day > row.daysSurvived)
        {
            const int was = row.daysSurvived;
            row.daysSurvived = snapshot.day;
            for (int m = 0; m < NMilestones; m++)
            {
                if (was < kMilestoneDays[m] && row.daysSurvived >= kMilestoneDays[m])
                {
                    char line[128];
                    snprintf(line, sizeof(line), "Survived %d days with the cell.", kMilestoneDays[m]);
                    RecordDeed(row, RString(line), LDMilestone);
                    touched = true;
                }
            }
        }
    }

    if (touched)
    {
        // Journal::AddEntry already bumped the journal's revision, so a poll that
        // wrote a line must not bump it a second time.
        Touch(!wroteJournal);
    }
}

// ---------------------------------------------------------------------------
// save / load
// ---------------------------------------------------------------------------

LSError LegendRegistry::Serialize(ParamArchive& ar)
{
    // PassFirst: campaign scalars, then the history, then the rows as plain
    // values.  PassSecond: refs + reconcile.  NOTHING may add, remove or reorder
    // a row between the two passes, or before ar.Serialize("Legends", _rows, 1)
    // returns on PassSecond: ParamArchive re-walks Item%d BY INDEX across the
    // passes, so a row that moved silently corrupts the archive.
    int seedInt = (int)_seed;
    PARAM_CHECK(ar.Serialize("seed", seedInt, 1, 0))
    _seed = (unsigned)seedInt;
    PARAM_CHECK(ar.Serialize("progression", _progression, 1, false))
    PARAM_CHECK(ar.Serialize("seeded", _seeded, 1, false))
    PARAM_CHECK(ar.Serialize("resistancePool", _resistancePool, 1, -1))
    PARAM_CHECK(ar.Serialize("occupierPool", _occupierPool, 1, -1))
    PARAM_CHECK(ar.Serialize("resistanceName", _resistanceName, 1, RString()))
    PARAM_CHECK(ar.Serialize("occupierName", _occupierName, 1, RString()))
    PARAM_CHECK(ar.Serialize("History", _history, 1))
    PARAM_CHECK(ar.Serialize("Legends", _rows, 1))
    if (ar.IsLoading() && ar.GetPass() == ParamArchive::PassSecond)
    {
        ReconcileAfterLoad();
    }
    return LSOK;
}

void LegendRegistry::ReconcileAfterLoad()
{
    bool changed = false;
    for (int i = 0; i < _rows.Size(); i++)
    {
        LegendRow& row = _rows[i];
        // drop guard refs whose objects did not survive; Legend ROWS are never
        // dropped on load, a null body only sets bookkeeping
        const int guards = row.guards.Size();
        row.guards.Compact();
        if (row.guards.Size() != guards)
        {
            changed = true;
        }
        row.lastPos = VZero;
        row.lastZone = RString();
        if (row.kind != LKCompanion)
        {
            continue;
        }
        Person* person = dyn_cast<Person>(row.body.GetLink());
        if (!person || !BodyIsAlive(person))
        {
            continue;
        }
        // WorldImpl's load path rerolls the pool identity onto a recreated body,
        // so the registry re-asserts its own name here rather than waiting a tick
        if (strcmp(person->GetInfo()._name, DisplayName(row)) != 0)
        {
            Bind(row.compIndex, person);
            changed = true;
        }
    }
    _accum = 0;
    _pollDigest = 0; // the next poll must act on whatever the save restored
    if (changed)
    {
        _revision++;
        Journal::Instance().Touch(); // at most once per load
    }
}

void LegendRegistry::DeriveFromExistingSave()
{
    // A save written before the registry existed.  Build dossier rows out of
    // GM_COMP_* and lock the campaign into DERIVED mode: _seeded stops
    // SeedCampaign from ever firing, _progression == false stops every journal
    // write, and _seed stays 0 because there is no generated campaign here.
    _seeded = true;
    _seed = 0;
    _progression = false;
    _history = HistoryRecord();
    _rows.Clear();

    AutoArray<RString> names;
    AutoArray<float> xp;
    AutoArray<bool> alive;
    ScriptVars::StringArray("gm_comp_names", names);
    ScriptVars::ScalarArray("gm_comp_xp", xp);
    ScriptVars::BoolArray("gm_comp_alive", alive);
    int day = 1;
    int minute = 0;
    JournalClockNow(day, minute);

    const ZoneRegistry& registry = ZoneRegistry::Instance();
    const RString resistanceFaction = registry.ResistanceFaction();
    const RString resistanceSide = registry.ResistanceSide();
    const RString resistanceKey = resistanceFaction.GetLength() > 0 ? resistanceFaction : resistanceSide;
    _resistancePool = ResolveNamePool(registry.FactionValue(resistanceKey, "namePool"), resistanceSide, resistanceKey);
    _resistanceName = FactionDisplayName(registry, resistanceFaction, resistanceSide);

    for (int i = 0; i < names.Size(); i++)
    {
        const int at = _rows.Add();
        LegendRow& row = _rows[at];
        row.id = CompanionRowId(i, names[i]);
        row.kind = LKCompanion;
        row.compIndex = i;
        row.baseName = names[i];
        row.first = names[i];
        // no last name and no awarded slot, so DisplayName is exactly the base
        // name: "dossiers derived from available companion records"
        row.namePool = _resistancePool;
        row.tone = ToneFriendly;
        // seed 0 is fine for the face: it only has to be stable
        row.face = RString(kPortraitFaces[Roll(HashKey(row.id, 0), CH_FACE, NPortraitFaces)]);
        row.xpSeen = i < xp.Size() ? xp[i] : 0.0f;
        row.rankSeen = LadderIndexForXp(row.xpSeen);
        row.alive = i < alive.Size() ? alive[i] : true;
        row.daysSurvived = row.alive ? day : 0;
    }
    LOG_INFO(Core, "Legends: no campaign block in this save - {} companion dossier(s) derived, progression off",
             _rows.Size());
    Touch(true);
}

} // namespace Poseidon::Guerrilla
