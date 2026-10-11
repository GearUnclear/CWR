#include <Poseidon/Game/Guerrilla/FactionHistory.hpp>

#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <string.h>

namespace Poseidon::Guerrilla
{

namespace
{

// ---------------------------------------------------------------------------
// channels
// ---------------------------------------------------------------------------
// LegendChannel (LegendSeed.hpp) stops at CH_BOSSWORD = 53. The four faction
// construction draws are their own draw kinds and continue above it; nothing
// else may take these numbers. One channel per insertion point is what keeps a
// campaign's phrases stable when only one of them is re-drawn.
constexpr unsigned kChPhraseOpenB = 60;
constexpr unsigned kChPhraseOpenD = 61;
constexpr unsigned kChPhraseEvent1 = 62;
constexpr unsigned kChPhraseEvent2 = 63;

constexpr int kVariants = 6; // per opening slot, per beat, per biography cell

// ---------------------------------------------------------------------------
// faction constructions
// ---------------------------------------------------------------------------
// A bare faction name modifies a plural group noun: "Soviet Army troops",
// "FIA units", "IDF soldiers". This reads naturally for both full names and
// acronyms without an article, possessive, or a historical identity claim.
// The persisted phrase indices still choose five distinct labels.
static const char* const kFactionPhrase[] = {"troops", "units", "soldiers", "forces", "fighters"};
constexpr int kPhraseCount = (int)(sizeof(kFactionPhrase) / sizeof(kFactionPhrase[0]));

// Last-resort place names, used only when the world offers neither a Names
// block nor a zone table.
static const char* const kGenericPlace[] = {"the old crossing", "the high pass", "the river ford", "the coast road"};

// ---------------------------------------------------------------------------
// opening slot A - the ancient claim ({Island}, {Place} = the beat-0 place)
// ---------------------------------------------------------------------------
static const char* const kOpenA[kVariants] = {
    "The people of {Island} still tell how families near {Place} lost rights their grandparents had "
    "expected to leave to the children.",
    "Near {Place}, families in {Island} inherited an old dispute along with their grandparents' "
    "belongings. Outsiders had claimed what kept those families fed.",
    "For generations, families near {Place} in {Island} taught children where they belonged. Then "
    "outsiders arrived with papers saying the children had no claim.",
    "People in {Island} trace the trouble to {Place}, where outside owners dismissed local agreements. "
    "Families kept copies that nobody in authority would read.",
    "Families near {Place} in {Island} kept old agreements long after outsiders stopped respecting them. "
    "Each generation tried to reclaim those rights.",
    "Families near {Place} in {Island} inherited their grandparents' old unanswered petitions. They kept "
    "sending new ones, asking for the same rights."};

// ---------------------------------------------------------------------------
// opening slot B - the coming of the occupier's line ({Occupier})
// ---------------------------------------------------------------------------
static const char* const kOpenB[kVariants] = {
    "{OccupierCap} arrived with orders to enforce obedience. A complaint could bring armed visitors to "
    "the same door where neighbours once gathered.",
    "{OccupierCap} controlled the main roads. People learned which guards would listen, which would "
    "shout, and when to turn around without arguing.",
    "Under {Occupier}, residents watched their words in public. Families argued behind closed doors "
    "about whether silence was keeping anyone safe anymore.",
    "{OccupierCap} posted guards and demanded obedience. Some residents tried written petitions; others "
    "stopped expecting an answer and began meeting behind shutters.",
    "{OccupierCap} made the rules, but local people still had to eat. Neighbours traded small favours "
    "and warned each other about patrols.",
    "When {Occupier} took control, arguments with officials became arguments with armed soldiers. Even "
    "close friends disagreed about how much they could endure."};

// ---------------------------------------------------------------------------
// opening slot C - the breach ({Place} = the beat-1 place)
// ---------------------------------------------------------------------------
static const char* const kOpenC[kVariants] = {
    "An agreement near {Place} briefly gave families reason to hope. The authorities broke its terms, "
    "leaving its signers to explain.",
    "Representatives near {Place} came home with an agreement and promises. When the authorities broke "
    "it, neighbours demanded to know what happened.",
    "Near {Place}, both sides agreed to a settlement. After the authorities broke it, residents brought "
    "copies to confront the guards.",
    "An agreement near {Place} promised peace. The authorities broke it, and residents who trusted the "
    "negotiators felt betrayed.",
    "People near {Place} accepted a settlement they could live with. When the authorities broke it, even "
    "patient neighbours talked of fighting.",
    "Near {Place}, negotiators promised an agreement would protect local rights. After the authorities "
    "broke it, nobody volunteered for another delegation."};

// ---------------------------------------------------------------------------
// opening slot D - the inheritance ({Resistance}, {Place} = the beat-0 place)
// ---------------------------------------------------------------------------
static const char* const kOpenD[kVariants] = {
    "Fighting followed. {ResistanceCap} remember the dead through surviving friends, who can still "
    "describe their laughter as readily as their deaths.",
    "{ResistanceCap} lost people in the fighting that followed. Some families still ask returning "
    "fighters for news nobody wants to give.",
    "Later fighting left empty places at family tables. {ResistanceCap} keep the names, including those "
    "neighbours still find difficult to discuss.",
    "Fighting killed residents. {ResistanceCap} hear from families still wanting to know where their "
    "relatives were taken or buried.",
    "The fighting left families grieving. {ResistanceCap} still hear from survivors who remember who "
    "came back and who never came home.",
    "People died when the dispute became fighting. {ResistanceCap} carry those losses into every "
    "argument about what another battle might cost."};

// ---------------------------------------------------------------------------
// beat 0 - the ancient grievance. NO faction construction: the grievance
// predates both sides, which is the whole point of the beat.
// ---------------------------------------------------------------------------
static const char* const kTitle0[kVariants] = {"Terrace Claims near {Place}", "Well Ownership near {Place}",
                                               "Water Rights near {Place}",   "Pasture Fences near {Place}",
                                               "Crossing Tolls near {Place}", "Salt Licences near {Place}"};

static const char* const kText0[kVariants] = {
    "Families near {Place} built terraces on the slopes and farmed them for generations. Surveyors later "
    "divided the land into plots and registered it under outside owners. The families who had built and "
    "maintained the terraces were left without a recognised claim to the ground that fed them.",
    "Families near {Place} dug and maintained four wells that supplied the valley. Water was shared "
    "under arrangements passed down through nine generations. An outside clerk later registered the "
    "wells as private property, giving an absent owner control over water the families had previously "
    "drawn by right.",
    "Road builders near {Place} marked the rock beside each well to record who could draw water. Local "
    "families used those marks to settle access disputes. When officials produced the first regional "
    "map, the water rights were omitted. Officials then rejected claims based on the older markings.",
    "Herders near {Place} shared pasture under agreements maintained by local families for generations. "
    "An outside landholder fenced the common ground within a week and barred the herds. Families that "
    "had always used the pasture had to find other grazing or pay to enter it.",
    "Families near {Place} maintained a river crossing and used it freely to move goods between the "
    "banks. A new authority claimed ownership, posted a toll and stationed a guard to collect it. The "
    "families then had to pay whenever work took them across a route those families had kept open.",
    "Workers near {Place} cut salt from nearby flats and agreed on each family's share before the season "
    "began. A company obtained an exclusive licence from the capital and cancelled those arrangements. "
    "Families lost the right to gather salt independently and had to seek wages from the new operator."};

// ---------------------------------------------------------------------------
// beat 1 - the broken settlement. Carries the OCCUPIER construction.
// ---------------------------------------------------------------------------
static const char* const kTitle1[kVariants] = {"Closed Roads near {Place}",       "Pump Restrictions near {Place}",
                                               "The Truce near {Place}",          "The Charter near {Place}",
                                               "Market Checkpoints near {Place}", "The Boundary near {Place}"};

static const char* const kText1[kVariants] = {
    "Representatives near {Place} agreed that the high ground would remain common and the roads open. "
    "Within a season, {Occupier} fenced the ground and gated the roads. Families who had accepted the "
    "settlement now needed permission to reach land and routes the agreement had left open.",
    "Witnesses from six villages signed an agreement near {Place} to share water and keep the coast road "
    "open. The terms also barred armed guards from the pumps. {OccupierCap} seized the pumps and began "
    "checking road traffic twice a day, restricting the access the witnesses had secured.",
    "Elders marked a truce boundary with stones near {Place} and agreed which wells would remain "
    "available to local families. {OccupierCap} later occupied those wells and shifted the markers. "
    "Families arriving for water found guards enforcing a boundary that no longer matched the one the "
    "elders had walked.",
    "A charter near {Place} guaranteed local access to shared land and was read aloud each year. "
    "{OccupierCap} stopped the readings, removed the charter and began requiring permits. Residents "
    "seeking to challenge a refusal could no longer consult the document that set out the original "
    "terms.",
    "An agreement kept the market near {Place} open to both sides and barred armed patrols from its "
    "trading area. {OccupierCap} installed checkpoints at both entrances. Traders now faced searches and "
    "delays on a route the agreement had kept open, although the authorities never formally cancelled "
    "it.",
    "Under an agreement near {Place}, anyone crossing the boundary for a funeral could return before "
    "nightfall. {OccupierCap} began keeping mourners at the posts until dark, then refusing passage. "
    "Families stopped attending burials across the boundary, although the agreement was still posted "
    "beside the guards."};

// ---------------------------------------------------------------------------
// beat 2 - the remembered catastrophe or stand. Carries the RESISTANCE
// construction: this is the beat the present cell claims descent from.
// ---------------------------------------------------------------------------
static const char* const kTitle2[kVariants] = {"Road Battle near {Place}",     "The Fires near {Place}",
                                               "Delaying Action near {Place}", "The Encirclement near {Place}",
                                               "Ridge Battle near {Place}",    "Winter Losses near {Place}"};

static const char* const kText2[kVariants] = {
    "Defenders near {Place} stopped an advancing column for two days without relief. The column broke "
    "through on the third day, leaving local fighters dead and others missing. Survivors later gave "
    "{Resistance} an account of the positions held and the names of those who failed to return.",
    "Fighting near {Place} set homes alight, and the fires continued through the night. Families who "
    "returned found collapsed houses and neighbours missing. {ResistanceCap} collected names from "
    "survivors and recorded where each person had last been seen. Relatives used the list to search for "
    "people carried away during the evacuation.",
    "Defenders with two machine guns held an advancing column near {Place} until dusk. The delay allowed "
    "families to leave the coastal route, but several defenders died before the survivors withdrew. The "
    "coast fell the next day. {ResistanceCap} preserved the survivors' account of the action and the "
    "losses.",
    "A garrison near {Place} was surrounded before dawn and held out until the following evening. "
    "Ammunition ran out before relief arrived, and survivors later reported dead and missing comrades. "
    "{ResistanceCap} gathered the names from returning personnel and families whose relatives had served "
    "in the garrison.",
    "Local farmers with hunting rifles held a ridge near {Place} for eleven hours before an advancing "
    "force drove the defenders off. Survivors returned after dark to recover the dead. {ResistanceCap} "
    "later recorded the names and accounts from families who had sent people up to the ridge.",
    "The siege near {Place} left families short of food and shelter as winter began. More residents died "
    "during that winter than in the fighting. Promised relief reached the area in spring. "
    "{ResistanceCap} recorded the deaths from household accounts, including the names of people buried "
    "before help arrived."};

// ---------------------------------------------------------------------------
// biographies: (beat kind of the referenced event) x tone, six variants each.
// Strictly PRE-campaign. Current rank, XP, kill totals and captured zones
// belong in the deed list. Earlier events can include violence or resistance
// service. Gender-neutral pronouns avoid repeating a long display name.
// ---------------------------------------------------------------------------
static const char* const kBio0Friendly[kVariants] = {
    "{Name} worked as a farmhand near {Place} until the landlord evicted the family from its rented "
    "fields. The farmhand kept writing poems and still hoped to publish a collection.",
    "When fighting scattered the family, {Name}, a furniture maker from {Place}, fled with a younger "
    "sibling. The siblings later rented a shed and began making tables and chairs again.",
    "{Name}, a survey assistant from {Place}, accepted a bribe to falsify a boundary, costing a family "
    "their home. The evicted family knew who had signed the false survey.",
    "{Name} owned a grocery near {Place}. Soldiers confiscated its stock, ruining the business. The "
    "grocer joined a rebel band, then left after its leader ordered members to steal food from "
    "neighbouring families.",
    "{Name}, a shepherd from {Place}, guided displaced neighbours along familiar grazing paths. A cousin "
    "stayed behind to tend the flock; the shepherd lost contact with that cousin during the journey.",
    "{Name}, a municipal gardener from {Place}, escaped the fighting with an elderly parent. At the "
    "woodland shelter where both found refuge, the gardener took responsibility for fetching everyone's "
    "water."};

static const char* const kBio0Hostile[kVariants] = {
    "Before becoming an army quartermaster near {Place}, {Name} worked as a warehouse clerk. When the "
    "army needed more transport, the quartermaster confiscated civilian carts and sent them to military "
    "supply depots.",
    "{Name} earned a living playing at weddings before enlisting for regular wages. Posted near {Place}, "
    "the musician hoped to leave the army and earn money performing again.",
    "While posted near {Place}, military engineer {Name} approved an unsafe bridge repair. The bridge "
    "collapsed, killing two comrades. Since then, the engineer has personally checked every repair "
    "before allowing soldiers to cross.",
    "{Name} sold travel permits near {Place}, using a civilian partner to collect bribes. When "
    "investigators arrived, the administrator named the partner in exchange for keeping the post.",
    "{Name} lost a dock job near {Place} and enlisted for regular wages. During years of army service, "
    "the former laborer regularly helped younger soldiers write letters to their families.",
    "{Name} was a local official near {Place}, authorized to stop evictions. When a former rival "
    "appealed, the official refused, knowing the rival had paid the rent and had the right to stay."};

static const char* const kBio1Friendly[kVariants] = {
    "{Name} taught arithmetic at a school near {Place}. When new restrictions closed the school, the "
    "teacher visited pupils at home and continued the lessons from where each class had stopped.",
    "Former postal worker {Name} from {Place} became a resistance courier, then quit after police seized "
    "the courier's address book, found a friend's home address inside, and arrested the friend there.",
    "{Name} repaired bicycles near {Place} for underground couriers delivering medicine to hidden "
    "clinics. Police searched the workshop and seized the spare parts, leaving several couriers without "
    "working bicycles.",
    "{Name}, a bookkeeper from {Place}, kept records for a resistance cell but left after its leader "
    "ordered the bookkeeper's friend beaten as an informer. The leader had no evidence.",
    "{Name}, a tailor from {Place}, made the clothes for a younger sibling's wedding. The authorities "
    "refused the tailor permission to travel, and the wedding went ahead without them.",
    "{Name} skipped classes to print leaflets against rent increases near {Place}, but dreaded speaking "
    "at meetings. After their mother's arrest, they hid with friends and wrote speeches for others to "
    "read."};

static const char* const kBio1Hostile[kVariants] = {
    "After working as a school inspector, {Name} supervised police near {Place}. When residents came to "
    "the police office seeking protection from soldiers, the former inspector ordered the visitors "
    "detained.",
    "{Name} ran a checkpoint near {Place} and owed a smuggler money after gambling losses. In exchange "
    "for more time to repay the debt, the officer let the smuggler's wagons pass without inspection.",
    "{Name} commanded patrols near {Place}. After surviving an ambush that killed a close colleague, the "
    "commander ordered searches of uninvolved households, instructing soldiers to beat residents.",
    "A former railway porter, {Name} joined the army for regular wages to support an elderly parent. "
    "After being posted near {Place}, they requested a transfer closer to home.",
    "{Name} acted with an amateur theatre group near {Place} before taking a military publicity job. The "
    "former actor later helped announce an army ban that forced the group to close.",
    "Posted to {Place}, officer {Name} missed a first child's birth. Letters home described good meals "
    "and new friends, leaving out long shifts checking travel papers and orders barring leave."};

static const char* const kBio2Friendly[kVariants] = {
    "{Name}, a radio repairer from {Place}, joined a resistance transmitter crew after fighting "
    "destroyed the shop. The repairer maintained equipment and hoped to host music programmes someday.",
    "{Name}, a quarry worker from {Place}, deserted a resistance position during an attack. A surviving "
    "fighter later reported that a companion had stayed to defend the position and been killed.",
    "An orchard worker from {Place}, {Name} led relatives away from fighting along familiar farm roads. "
    "Among displaced families, the sociable picker entertained new friends with stories from years on "
    "the harvest crews.",
    "{Name}, a photographer from {Place}, escaped to a resistance camp and became a medic without "
    "medical training. Despite the commander's ban, the photographer secretly treated a wounded enemy "
    "and stayed with the group.",
    "{Name} worked the family ferry near {Place} until fighting destroyed the boat. Living with their "
    "mother afterward, they began learning the recipes the family used for celebrations.",
    "{Name}, an apprentice mason from {Place}, volunteered for the resistance expecting adventure. "
    "During the first attack, the recruit panicked and could not leave cover. Afterward, they asked for "
    "more training."};

static const char* const kBio2Hostile[kVariants] = {
    "{Name} served as an officer near {Place}. When a friend killed a surrendering fighter, the officer "
    "protected the friend by falsely stating in the unit report that the fighter had died in combat.",
    "{Name} wrote poetry for years before the war. While serving as an officer near {Place}, they "
    "prepared notices for the families of soldiers killed under their command.",
    "Officer {Name} lost a younger sibling in fighting. During a posting near {Place}, the bereaved "
    "officer repeatedly gave one young recruit safer kitchen duties while other recruits went on patrol.",
    "{Name}, a former foundry worker from {Place}, served in the army. After a superior threatened death "
    "for refusing, the soldier helped execute prisoners and later admitted taking part.",
    "{Name}, an army payroll clerk near {Place}, took charge of the records after heavy losses in the "
    "unit. The clerk kept dead soldiers on the payroll and pocketed their wages.",
    "Before becoming a military interrogator near {Place}, {Name} worked as a boxing trainer. The "
    "interrogator taught new guards to beat prisoners until the prisoners signed confessions."};

// Flat view over every table, for the lints.
static const char* const* const kAllTables[] = {
    kOpenA,        kOpenB,       kOpenC,        kOpenD,       kTitle0,        kTitle1,
    kTitle2,       kText0,       kText1,        kText2,       kBio0Friendly,  kBio0Hostile,
    kBio1Friendly, kBio1Hostile, kBio2Friendly, kBio2Hostile, kFactionPhrase, kGenericPlace};
static const int kAllTableSizes[] = {
    kVariants, kVariants, kVariants, kVariants, kVariants,    kVariants,
    kVariants, kVariants, kVariants, kVariants, kVariants,    kVariants,
    kVariants, kVariants, kVariants, kVariants, kPhraseCount, (int)(sizeof(kGenericPlace) / sizeof(kGenericPlace[0]))};
constexpr int kAllTableCount = (int)(sizeof(kAllTables) / sizeof(kAllTables[0]));

// ---------------------------------------------------------------------------
// small text helpers
// ---------------------------------------------------------------------------

// Growable char buffer; RString concatenation in a loop would be quadratic.
class Text
{
  public:
    void Add(const char* s)
    {
        if (!s)
        {
            return;
        }
        for (const char* p = s; *p; p++)
        {
            _buf.Add(*p);
        }
    }
    void AddChar(char c) { _buf.Add(c); }
    RString Str() const { return _buf.Size() > 0 ? RString(_buf.Data(), _buf.Size()) : RString(); }

  private:
    AutoArray<char> _buf;
};

// ASCII only on purpose: no locale, no toupper, so the same byte comes out on
// every platform the engine builds for.
RString CapitalizeFirst(const RString& src)
{
    const char* s = src;
    if (!s || !s[0])
    {
        return src;
    }
    Text out;
    char c = s[0];
    if (c >= 'a' && c <= 'z')
    {
        c = (char)(c - ('a' - 'A'));
    }
    out.AddChar(c);
    out.Add(s + 1);
    return out.Str();
}

// "Soviet Army" + " troops"
RString PhraseWith(int phrase, const RString& factionName)
{
    const int i = (phrase >= 0 && phrase < kPhraseCount) ? phrase : 0;
    Text out;
    out.Add(factionName);
    out.AddChar(' ');
    out.Add(kFactionPhrase[i]);
    return out.Str();
}

struct Slots
{
    RString island;
    RString place;
    RString occupier, occupierCap;
    RString resistance, resistanceCap;
    RString name;
    RString faction, factionCap;
};

bool SlotIs(const char* start, int len, const char* key)
{
    return (int)strlen(key) == len && strncmp(start, key, (size_t)len) == 0;
}

// Substitutes {Island} {Place} {Occupier} {OccupierCap} {Resistance}
// {ResistanceCap} {Name} {Faction} {FactionCap}. An unknown slot is emitted
// verbatim, braces included, so the template lint can see the typo.
RString Render(const char* tmpl, const Slots& s)
{
    Text out;
    for (const char* p = tmpl; p && *p;)
    {
        if (*p != '{')
        {
            out.AddChar(*p);
            p++;
            continue;
        }
        const char* close = strchr(p, '}');
        if (!close)
        {
            out.AddChar(*p);
            p++;
            continue;
        }
        const char* key = p + 1;
        const int len = (int)(close - key);
        if (SlotIs(key, len, "Island"))
        {
            out.Add(s.island);
        }
        else if (SlotIs(key, len, "Place"))
        {
            out.Add(s.place);
        }
        else if (SlotIs(key, len, "Occupier"))
        {
            out.Add(s.occupier);
        }
        else if (SlotIs(key, len, "OccupierCap"))
        {
            out.Add(s.occupierCap);
        }
        else if (SlotIs(key, len, "Resistance"))
        {
            out.Add(s.resistance);
        }
        else if (SlotIs(key, len, "ResistanceCap"))
        {
            out.Add(s.resistanceCap);
        }
        else if (SlotIs(key, len, "Name"))
        {
            out.Add(s.name);
        }
        else if (SlotIs(key, len, "Faction"))
        {
            out.Add(s.faction);
        }
        else if (SlotIs(key, len, "FactionCap"))
        {
            out.Add(s.factionCap);
        }
        else
        {
            for (const char* q = p; q <= close; q++)
            {
                out.AddChar(*q);
            }
        }
        p = close + 1;
    }
    return out.Str();
}

// ---------------------------------------------------------------------------
// place selection
// ---------------------------------------------------------------------------

void AppendNames(const AutoArray<PlaceName>& src, AutoArray<RString>& out)
{
    for (int i = 0; i < src.Size(); i++)
    {
        if (src[i].name.GetLength() > 0)
        {
            out.Add(src[i].name);
        }
    }
}

// Draw without replacement: the entry is removed, so beat one and beat two can
// never name the same settlement while two exist.
RString TakeFrom(AutoArray<RString>& pool, unsigned long long key, unsigned channel)
{
    if (pool.Size() <= 0)
    {
        return RString();
    }
    const int i = (int)Roll(key, channel, pool.Size());
    RString value = pool[i];
    pool.Delete(i);
    return value;
}

struct PlacePools
{
    AutoArray<RString> settlements, features, zones, generic;
};

// features first (only used for the beat-0 slot), then the remaining
// settlements, then the zone table, then the compiled generic table.
RString PickPlace(PlacePools& pools, bool preferFeature, unsigned long long key, unsigned channel)
{
    if (preferFeature)
    {
        RString v = TakeFrom(pools.features, key, channel);
        if (v.GetLength() > 0)
        {
            return v;
        }
    }
    RString v = TakeFrom(pools.settlements, key, channel);
    if (v.GetLength() > 0)
    {
        return v;
    }
    v = TakeFrom(pools.zones, key, channel);
    if (v.GetLength() > 0)
    {
        return v;
    }
    return TakeFrom(pools.generic, key, channel);
}

// ---------------------------------------------------------------------------
// budgets
// ---------------------------------------------------------------------------

// Distance from a count to the closed budget band; 0 inside it.
int BudgetDistance(int words, int lo, int hi)
{
    if (words < lo)
    {
        return lo - words;
    }
    if (words > hi)
    {
        return words - hi;
    }
    return 0;
}

} // namespace

// ---------------------------------------------------------------------------
// public helpers
// ---------------------------------------------------------------------------

int HistoryWordCount(const char* text)
{
    int words = 0;
    bool inWord = false;
    for (const char* p = text; p && *p; p++)
    {
        const bool space = (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r');
        if (space)
        {
            inWord = false;
        }
        else if (!inWord)
        {
            inWord = true;
            words++;
        }
    }
    return words;
}

RString HistoryIslandWord(const RString& display)
{
    const char* s = display;
    Text out;
    // strip a parenthesised tail: "Lebanon (80's)" -> "Lebanon"
    for (const char* p = s; p && *p; p++)
    {
        if (*p == '(')
        {
            break;
        }
        out.AddChar(*p);
    }
    RString head = out.Str();
    // strip trailing digits and trim: "Lebanon80" -> "Lebanon"
    const char* h = head;
    int end = (int)strlen(h);
    while (end > 0 && ((h[end - 1] >= '0' && h[end - 1] <= '9') || h[end - 1] == ' ' || h[end - 1] == '\t'))
    {
        end--;
    }
    int begin = 0;
    while (begin < end && (h[begin] == ' ' || h[begin] == '\t'))
    {
        begin++;
    }
    if (end <= begin)
    {
        return RString("this country");
    }
    Text trimmed;
    for (int i = begin; i < end; i++)
    {
        if (h[i] >= '0' && h[i] <= '9')
        {
            // an interior digit means the name is a code, not a place word
            return RString("this country");
        }
        trimmed.AddChar(h[i]);
    }
    return trimmed.Str();
}

int HistoryTemplateStringCount()
{
    int n = 0;
    for (int t = 0; t < kAllTableCount; t++)
    {
        n += kAllTableSizes[t];
    }
    return n;
}

const char* HistoryTemplateString(int i)
{
    if (i < 0)
    {
        return "";
    }
    for (int t = 0; t < kAllTableCount; t++)
    {
        if (i < kAllTableSizes[t])
        {
            return kAllTables[t][i];
        }
        i -= kAllTableSizes[t];
    }
    return "";
}

int HistoryVariantCount()
{
    return kVariants;
}

int HistoryFactionPhraseCount()
{
    return kPhraseCount;
}

const char* HistoryFactionPhrase(int i)
{
    return (i >= 0 && i < kPhraseCount) ? kFactionPhrase[i] : "";
}

int HistoryRecord::BioPhraseIndex() const
{
    for (int candidate = 0; candidate < kPhraseCount; candidate++)
    {
        bool used = false;
        for (int i = 0; i < kHistoryPhrases; i++)
        {
            if (phraseIndex[i] == candidate)
            {
                used = true;
            }
        }
        if (!used)
        {
            return candidate;
        }
    }
    return 0;
}

LSError HistoryRecord::Serialize(ParamArchive& ar)
{
    // Plain values, every one through the 4-arg default-tolerant overload, so a
    // record written by an older or newer build loads with the missing keys at
    // their defaults instead of failing. ParamArchive has no `unsigned`
    // overload, so the seed rides as an int; it is drawn as
    // RandomValue() * 1073741823 | 1 and always fits.
    // "hver", not "version": ParamArchiveSave writes its own `version` entry at
    // the root of an archive, so a record serialized straight into one would
    // read the archive's own version back as the generator's.
    PARAM_CHECK(ar.Serialize("hver", version, 1, 0))
    int seedInt = (int)seed;
    PARAM_CHECK(ar.Serialize("seed", seedInt, 1, 0))
    seed = (unsigned)seedInt;
    // The three event indices are written INDIVIDUALLY so a default-tolerant
    // read works per element rather than per array.
    PARAM_CHECK(ar.Serialize("ev0", eventIndex[0], 1, 0))
    PARAM_CHECK(ar.Serialize("ev1", eventIndex[1], 1, 0))
    PARAM_CHECK(ar.Serialize("ev2", eventIndex[2], 1, 0))
    PARAM_CHECK(ar.Serialize("op0", openingIndex[0], 1, 0))
    PARAM_CHECK(ar.Serialize("op1", openingIndex[1], 1, 0))
    PARAM_CHECK(ar.Serialize("op2", openingIndex[2], 1, 0))
    PARAM_CHECK(ar.Serialize("op3", openingIndex[3], 1, 0))
    PARAM_CHECK(ar.Serialize("ph0", phraseIndex[0], 1, 0))
    PARAM_CHECK(ar.Serialize("ph1", phraseIndex[1], 1, 0))
    PARAM_CHECK(ar.Serialize("ph2", phraseIndex[2], 1, 0))
    PARAM_CHECK(ar.Serialize("ph3", phraseIndex[3], 1, 0))
    PARAM_CHECK(ar.Serialize("open1", openingPage1, 1, RString()))
    PARAM_CHECK(ar.Serialize("open2", openingPage2, 1, RString()))
    PARAM_CHECK(ar.Serialize("t0", eventTitle[0], 1, RString()))
    PARAM_CHECK(ar.Serialize("x0", eventText[0], 1, RString()))
    PARAM_CHECK(ar.Serialize("p0", eventPlace[0], 1, RString()))
    PARAM_CHECK(ar.Serialize("t1", eventTitle[1], 1, RString()))
    PARAM_CHECK(ar.Serialize("x1", eventText[1], 1, RString()))
    PARAM_CHECK(ar.Serialize("p1", eventPlace[1], 1, RString()))
    PARAM_CHECK(ar.Serialize("t2", eventTitle[2], 1, RString()))
    PARAM_CHECK(ar.Serialize("x2", eventText[2], 1, RString()))
    PARAM_CHECK(ar.Serialize("p2", eventPlace[2], 1, RString()))
    return LSOK;
}

// ---------------------------------------------------------------------------
// generation
// ---------------------------------------------------------------------------

HistoryRecord GenerateHistoryForced(const HistoryInputs& in, const HistoryForcedIndices& forced)
{
    HistoryRecord rec;
    rec.version = kHistoryVersion;
    rec.seed = in.seed;

    // One key for the whole history, so adding a character row never moves a
    // single word of it.
    const unsigned long long key = HashKey("history", (unsigned long long)in.seed);

    // ---- places -----------------------------------------------------------
    PlacePools pools;
    AppendNames(in.settlements, pools.settlements);
    AppendNames(in.features, pools.features);
    for (int i = 0; i < in.zoneNames.Size(); i++)
    {
        if (in.zoneNames[i].GetLength() > 0)
        {
            pools.zones.Add(in.zoneNames[i]);
        }
    }
    for (int i = 0; i < (int)(sizeof(kGenericPlace) / sizeof(kGenericPlace[0])); i++)
    {
        pools.generic.Add(RString(kGenericPlace[i]));
    }
    // beats one and two take settlements first, so the beat-0 slot can still
    // reach a feature, which is the richer table
    rec.eventPlace[1] = PickPlace(pools, false, key, CH_PLACE1);
    rec.eventPlace[2] = PickPlace(pools, false, key, CH_PLACE2);
    rec.eventPlace[0] = PickPlace(pools, true, key, CH_PLACE0);

    // ---- faction constructions, as a permutation --------------------------
    // Independent draws would repeat one of only five distinctive phrases in
    // about ninety-six campaigns out of a hundred, two clicks apart in the
    // journal. Walking circularly past every index already spent makes the four
    // history phrases distinct by construction and leaves exactly one for the
    // biographies.
    static const unsigned kPhraseChannels[kHistoryPhrases] = {kChPhraseOpenB, kChPhraseOpenD, kChPhraseEvent1,
                                                              kChPhraseEvent2};
    for (int p = 0; p < kHistoryPhrases; p++)
    {
        int pick = forced.phrases[p] >= 0 ? (forced.phrases[p] % kPhraseCount)
                                          : (int)Roll(key, kPhraseChannels[p], kPhraseCount);
        for (int guard = 0; guard < kPhraseCount; guard++)
        {
            bool taken = false;
            for (int q = 0; q < p; q++)
            {
                if (rec.phraseIndex[q] == pick)
                {
                    taken = true;
                }
            }
            if (!taken)
            {
                break;
            }
            pick = (pick + 1) % kPhraseCount;
        }
        rec.phraseIndex[p] = pick;
    }

    const RString occupierPhrase = PhraseWith(rec.phraseIndex[0], in.occupierName);
    const RString resistancePhrase = PhraseWith(rec.phraseIndex[1], in.resistanceName);
    const RString eventOccupier = PhraseWith(rec.phraseIndex[2], in.occupierName);
    const RString eventResistance = PhraseWith(rec.phraseIndex[3], in.resistanceName);

    // ---- events -----------------------------------------------------------
    static const char* const* const kTitles[kHistoryEvents] = {kTitle0, kTitle1, kTitle2};
    static const char* const* const kTexts[kHistoryEvents] = {kText0, kText1, kText2};
    for (int k = 0; k < kHistoryEvents; k++)
    {
        const int idx = forced.events[k] >= 0 ? (forced.events[k] % kVariants)
                                              : (int)Roll(key, (unsigned)(CH_EVENT0 + k), kVariants);
        rec.eventIndex[k] = idx;
        Slots s;
        s.place = rec.eventPlace[k];
        if (k == 1)
        {
            s.occupier = eventOccupier;
            s.occupierCap = CapitalizeFirst(eventOccupier);
        }
        else if (k == 2)
        {
            s.resistance = eventResistance;
            s.resistanceCap = CapitalizeFirst(eventResistance);
        }
        rec.eventTitle[k] = Render(kTitles[k][idx], s);
        rec.eventText[k] = Render(kTexts[k][idx], s);
    }

    // ---- opening ----------------------------------------------------------
    const RString islandWord = HistoryIslandWord(in.islandName);

    Slots sa;
    sa.island = islandWord;
    sa.place = rec.eventPlace[0];
    Slots sb;
    sb.occupier = occupierPhrase;
    sb.occupierCap = CapitalizeFirst(occupierPhrase);
    Slots sc;
    sc.place = rec.eventPlace[1];
    Slots sd;
    sd.place = rec.eventPlace[0]; // the closing echo of the ancient place
    sd.resistance = resistancePhrase;
    sd.resistanceCap = CapitalizeFirst(resistancePhrase);

    const int a = forced.opening[0] >= 0 ? (forced.opening[0] % kVariants) : (int)Roll(key, CH_OPEN_A, kVariants);
    const int b = forced.opening[1] >= 0 ? (forced.opening[1] % kVariants) : (int)Roll(key, CH_OPEN_B, kVariants);
    const RString textA = Render(kOpenA[a], sa);
    const RString textB = Render(kOpenB[b], sb);
    const int head = HistoryWordCount(textA) + HistoryWordCount(textB);

    // Slots C and D are chosen TOGETHER. Choosing C first against the running
    // total cannot work: with D still unwritten the total can never reach the
    // floor, so every C would fall through to the "closest" branch and the whole
    // eighty-to-one-hundred guarantee would rest on D alone. Walking all
    // thirty-six pairs from the rolled starts costs nothing and is exact.
    const int startC = forced.opening[2] >= 0 ? (forced.opening[2] % kVariants) : (int)Roll(key, CH_OPEN_C, kVariants);
    const int startD = forced.opening[3] >= 0 ? (forced.opening[3] % kVariants) : (int)Roll(key, CH_OPEN_D, kVariants);
    const int spanC = forced.opening[2] >= 0 ? 1 : kVariants;
    const int spanD = forced.opening[3] >= 0 ? 1 : kVariants;

    int bestC = startC, bestD = startD, bestDistance = -1;
    bool found = false;
    for (int i = 0; i < spanC && !found; i++)
    {
        const int c = (startC + i) % kVariants;
        const int wordsC = HistoryWordCount(Render(kOpenC[c], sc));
        for (int j = 0; j < spanD; j++)
        {
            const int d = (startD + j) % kVariants;
            const int total = head + wordsC + HistoryWordCount(Render(kOpenD[d], sd));
            const int distance = BudgetDistance(total, kHistoryOpeningMinWords, kHistoryOpeningMaxWords);
            if (distance == 0)
            {
                bestC = c;
                bestD = d;
                found = true;
                break;
            }
            if (bestDistance < 0 || distance < bestDistance)
            {
                bestDistance = distance;
                bestC = c;
                bestD = d;
            }
        }
    }

    rec.openingIndex[0] = a;
    rec.openingIndex[1] = b;
    rec.openingIndex[2] = bestC;
    rec.openingIndex[3] = bestD;

    Text page1;
    page1.Add(textA);
    page1.Add(" ");
    page1.Add(textB);
    rec.openingPage1 = page1.Str();

    Text page2;
    page2.Add(Render(kOpenC[bestC], sc));
    page2.Add(" ");
    page2.Add(Render(kOpenD[bestD], sd));
    rec.openingPage2 = page2.Str();

    return rec;
}

HistoryRecord GenerateHistory(const HistoryInputs& in)
{
    const HistoryForcedIndices none;
    return GenerateHistoryForced(in, none);
}

RString GenerateBioForced(const HistoryRecord& history, int eventIndex, const RString& displayName,
                          const RString& factionName, bool hostile, unsigned long long key, int forcedVariant,
                          int maxWords)
{
    if (!history.Present())
    {
        return RString();
    }
    const int beat = (eventIndex >= 0 && eventIndex < kHistoryEvents) ? eventIndex : 0;
    static const char* const* const kCells[kHistoryEvents][2] = {
        {kBio0Friendly, kBio0Hostile}, {kBio1Friendly, kBio1Hostile}, {kBio2Friendly, kBio2Hostile}};
    const char* const* cell = kCells[beat][hostile ? 1 : 0];

    // The one construction the history did not spend, so a dossier never
    // repeats a phrase the Chronicles already used.
    const RString phrase = PhraseWith(history.BioPhraseIndex(), factionName);

    Slots s;
    s.name = displayName;
    s.place = history.eventPlace[beat];
    s.faction = phrase;
    s.factionCap = CapitalizeFirst(phrase);

    if (forcedVariant >= 0)
    {
        return Render(cell[forcedVariant % kVariants], s);
    }

    // The upper bound is the CALLER's page budget, so the walk selects a variant
    // that already fits where the text will be drawn rather than one Compose has
    // to amputate with an ellipsis.  A bound below the band's floor would leave
    // no window at all, so it never narrows past the minimum.
    const int upper = maxWords > kHistoryBioMinWords ? maxWords : kHistoryBioMinWords;
    const int start = (int)Roll(key, CH_BIOVAR, kVariants);
    RString best;
    int bestDistance = -1;
    for (int i = 0; i < kVariants; i++)
    {
        const RString text = Render(cell[(start + i) % kVariants], s);
        const int distance = BudgetDistance(HistoryWordCount(text), kHistoryBioMinWords, upper);
        if (distance == 0)
        {
            return text;
        }
        if (bestDistance < 0 || distance < bestDistance)
        {
            bestDistance = distance;
            best = text;
        }
    }
    return best;
}

RString GenerateBio(const HistoryRecord& history, int eventIndex, const RString& displayName,
                    const RString& factionName, bool hostile, unsigned long long key, int maxWords)
{
    return GenerateBioForced(history, eventIndex, displayName, factionName, hostile, key, -1, maxWords);
}

} // namespace Poseidon::Guerrilla
