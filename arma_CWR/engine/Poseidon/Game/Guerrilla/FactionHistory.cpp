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
// THE TEST A CONSTRUCTION MUST PASS: it has to END on a word that introduces a
// bare proper name, and English has only three of those - "as", "called" and
// "the name". Anything else makes the reader supply an article, which the
// generator cannot do, because a display name may or may not want one: the
// shipped library authors "US Army", "Soviet Army" and "FIA"
// (guerrilla-mode/config/guerrilla-factions.hpp), and "answer to Soviet Army"
// is wrong while "answer to FIA" is merely odd. So the whole phrase is checked
// by substitution against the widest set of shapes that ships - "US Army",
// "Soviet Army", "FIA", "IDF", "Hizballah", "PLO East" - and it has to read
// correctly for EVERY one of them, not for the acronyms alone.
//
// Article-free, possessive-free, acronym-free on top of that. Templates use a
// construction only as the subject of a simple past verb or as the object of a
// preposition, because two of the five are grammatically singular and three
// are plural.
static const char* const kFactionPhrase[] = {"the ranks now called", "whoever now marches as",
                                             "the fighters who muster as", "arms gathered under the name",
                                             "the standard now raised as"};
constexpr int kPhraseCount = (int)(sizeof(kFactionPhrase) / sizeof(kFactionPhrase[0]));

// Last-resort place names, used only when the world offers neither a Names
// block nor a zone table.
static const char* const kGenericPlace[] = {"the old crossing", "the high pass", "the river ford", "the coast road"};

// ---------------------------------------------------------------------------
// opening slot A - the ancient claim ({Island}, {Place} = the beat-0 place)
// ---------------------------------------------------------------------------
static const char* const kOpenA[kVariants] = {
    "Long before any flag now flown on {Island}, the people of these valleys cut terraces into the hills above "
    "{Place} and held them against every season.",
    "The oldest quarrel on {Island} began over water. The wells at {Place} were named, walled and fought for long "
    "before any banner now carried was stitched.",
    "Nothing on {Island} is older than the road to the wells, and the caravans that cut it wrote their claim into "
    "the rock at {Place}.",
    "Before there were borders on {Island} there were harvests, and the families who worked the ground around "
    "{Place} counted their right in seasons instead of papers.",
    "Every stone wall on {Island} was laid by somebody, and the walls above {Place} were laid by people whose "
    "names the oldest registers no longer carry.",
    "The first claim on {Island} was made with hands. Herds were moved, wells were dug, and the ground around "
    "{Place} was held by the families who worked it."};

// ---------------------------------------------------------------------------
// opening slot B - the coming of the occupier's line ({Occupier})
// ---------------------------------------------------------------------------
static const char* const kOpenB[kVariants] = {
    "Then came the columns of a distant power, and the road they cut has carried every ruler since, {Occupier} "
    "among them.",
    "Every power that has held this ground began by writing it down, and {Occupier} began the same way.",
    "Stone is a poor deed in a court, and every army that has crossed here has known it, {Occupier} among them.",
    "Then the map-makers came, and after them the garrisons, and the ledgers have passed from hand to hand down "
    "to {Occupier}.",
    "One power after another has held this ground since, each arriving with better paper than the last. "
    "{OccupierCap} arrived that way.",
    "The garrisons changed language and colour with each century, and the ledgers were copied forward. "
    "{OccupierCap} inherited the last copy."};

// ---------------------------------------------------------------------------
// opening slot C - the breach ({Place} = the beat-1 place)
// ---------------------------------------------------------------------------
static const char* const kOpenC[kVariants] = {
    "A settlement was sworn at {Place} and broken within a season.",
    "At {Place} a settlement was signed that promised the water would stay shared. The seals are still in the "
    "archive; the pumps are not.",
    "A truce was measured out at {Place} with stones set in the sand. The stones were moved before the copy of it "
    "had dried.",
    "An agreement was read out at {Place} before witnesses. The ground it named was fenced within two winters.",
    "The compact made at {Place} lasted one generation. The terms were kept in an archive, and the ground they "
    "named was taken anyway.",
    "What was promised at {Place} was written down, witnessed and buried in a strong box. The ground itself "
    "changed hands twice before the ink was dry."};

// ---------------------------------------------------------------------------
// opening slot D - the inheritance ({Resistance}, {Place} = the beat-0 place)
// ---------------------------------------------------------------------------
static const char* const kOpenD[kVariants] = {
    "{ResistanceCap} inherited that quarrel unfinished, and the ground above {Place} has not forgotten a single "
    "season of it.",
    "{ResistanceCap} took up that grievance from people who had carried it a long time already.",
    "{ResistanceCap} took this quarrel in hand, along with the road, the wells and everything else nobody signed "
    "for.",
    "The quarrel outlived the people who began it. It passed down to {Resistance}, and {Place} is still the "
    "argument.",
    "Nothing was settled and nothing was forgotten. The unfinished business came down at last to {Resistance}, "
    "which is where it stands.",
    "Old grievances do not expire; they change hands. This one changed hands until it reached {Resistance}, and "
    "there it has stayed."};

// ---------------------------------------------------------------------------
// beat 0 - the ancient grievance. NO faction construction: the grievance
// predates both sides, which is the whole point of the beat.
// ---------------------------------------------------------------------------
static const char* const kTitle0[kVariants] = {"The Terraces of {Place}", "The Wells at {Place}",
                                               "The Rock at {Place}",     "The Commons at {Place}",
                                               "The Crossing at {Place}", "The Salt at {Place}"};

static const char* const kText0[kVariants] = {
    "The old families cut the slopes above {Place} into steps and fed four valleys from them for longer than any "
    "register records. When the first surveyors came with chains and paper, the steps became parcels, and the "
    "parcels became someone else's property. Nobody in the valley signed anything.",
    "Four wells fed this whole valley, and the families that dug them kept the count by hand across nine "
    "generations. The first ledger written by an outside clerk reduced nine generations to a single line, and the "
    "line named an owner nobody in the valley had met.",
    "Before there was a border there was a road, and the men who cut it marked every well along it in the rock at "
    "{Place}. Those marks were law for as long as anyone needed law. The first map drawn in a capital did not "
    "copy a single one of them.",
    "The pasture above {Place} was held in common for as long as anyone had kept a record, and the herds moved "
    "across it by agreement rather than by permit. The first fence went up in a single week. The agreement had "
    "taken nine generations to build.",
    "The crossing at {Place} belonged to nobody and served everybody, and the families on both banks kept it open "
    "through flood and drought alike. Then a toll was posted, and a soldier was posted beside the toll, and the "
    "crossing has been somebody's property ever since.",
    "Salt was cut from the flats beyond {Place} by hand, in shares agreed at the start of every season and "
    "honoured without a written word. A company arrived with a licence signed in a distant capital. The shares "
    "were never renegotiated; they were simply cancelled."};

// ---------------------------------------------------------------------------
// beat 1 - the broken settlement. Carries the OCCUPIER construction.
// ---------------------------------------------------------------------------
static const char* const kTitle1[kVariants] = {"The Compact at {Place}", "The Seals at {Place}",
                                               "The Stones at {Place}",  "The Charter of {Place}",
                                               "The Market at {Place}",  "The Line at {Place}"};

static const char* const kText1[kVariants] = {
    "A settlement was read aloud in the square at {Place} and witnessed by both sides: the high ground would stay "
    "common, the roads would stay open. Within one season the ground was fenced and the roads were gated. "
    "{OccupierCap} took the paper away; the square kept the reading.",
    "The settlement at {Place} was signed in front of witnesses from six villages: the water shared, the coast "
    "road open to all, no armed man at the pumps. {OccupierCap} took the pumps. The road has been checked twice a "
    "day ever since.",
    "The truce at {Place} was set out in stones because neither side trusted paper. Elders from both sides walked "
    "the line and agreed it in one afternoon. {OccupierCap} took the wells inside that line, and the stones have "
    "been moved twice since.",
    "A charter was granted at {Place} and read out once a year so that nobody could claim to have forgotten it. "
    "The reading stopped in a year no register names. {OccupierCap} took the charter into an office, and the "
    "office was never open.",
    "The market at {Place} was neutral ground by an agreement older than any flag that has flown over it, and no "
    "armed man entered it. {OccupierCap} put a checkpoint at each end of the street. The agreement was never "
    "formally broken.",
    "A line was walked at {Place} by men from both sides and marked with cairns, and every family knew which side "
    "of it their water lay on. {OccupierCap} moved the line onto paper, and on paper it fell somewhere else "
    "entirely."};

// ---------------------------------------------------------------------------
// beat 2 - the remembered catastrophe or stand. Carries the RESISTANCE
// construction: this is the beat the present cell claims descent from.
// ---------------------------------------------------------------------------
static const char* const kTitle2[kVariants] = {"The Stand at {Place}",  "The Burning of {Place}",
                                               "The Column at {Place}", "The Night at {Place}",
                                               "The Ridge at {Place}",  "The Winter at {Place}"};

static const char* const kText2[kVariants] = {
    "At {Place} the column was stopped for two days by fewer men than it had guns. They were not relieved and "
    "they did not expect to be. The road was opened on the third day. {ResistanceCap} kept the count of those two "
    "days.",
    "{Place} burned for a day and a night, and the people who came back counted the doorways rather than the "
    "houses. No relief column reached the town. The list of names carried out of it passed to {Resistance}, and "
    "it has been read aloud once a year ever since.",
    "A column that should have taken the coast in a morning was held at {Place} until dusk by men with two "
    "machine guns and the high ground. None of them was relieved. The coast fell the next day, and {Resistance} "
    "counted the delay a victory anyway.",
    "The garrison at {Place} was surrounded before dawn and expected to surrender by noon. It held until the "
    "second evening, and the ammunition ran out before the will did. Every name from those two days was written "
    "down afterwards, and the page passed intact to {Resistance}.",
    "The ridge above {Place} was held for eleven hours by farmers with hunting rifles, against a force that had "
    "not thought the ridge worth naming. The ground was lost by evening. {ResistanceCap} carried the name of it "
    "away and never gave it back.",
    "The winter that followed the siege of {Place} killed more of the town than the siege had, and the relief "
    "that was promised arrived in the spring with a census taker. {ResistanceCap} learned in that season what a "
    "promise from a capital was worth."};

// ---------------------------------------------------------------------------
// biographies: (beat kind of the referenced event) x tone, six variants each.
// Strictly PRE-campaign. No rank, no XP, no kill, no captured zone: the deed
// list owns all of that. No third-person pronoun anywhere - the name repeats.
// ---------------------------------------------------------------------------
static const char* const kBio0Friendly[kVariants] = {
    "{Name} was born into one of the families that worked the ground above {Place}, and learned every path on "
    "that slope before learning to read. Nobody in this cell has ever needed to hand {Name} a map of ground a "
    "grandmother measured by hand.",
    "{Name} grew up on the wrong side of the ledger that took {Place} away, and heard the whole account of it "
    "from a grandfather who had learned it from a grandfather. That account has been in the family longer than "
    "any of the furniture.",
    "{Name} was raised on the story of what was taken at {Place} before any flag now flying was sewn, and can "
    "still recite the boundary the old families walked. That recitation is worth more to this cell than a printed "
    "map.",
    "The grandparents of {Name} were among the people counted, taxed and moved off the ground at {Place}, and the "
    "family kept every worthless paper from that year. {Name} arrived here already knowing that a signature and a "
    "right are different things.",
    "{Name} spent nine winters herding on ground the family had lost at {Place} without once being told it was "
    "lost. The correction, when it came, was brief. {Name} has been an inconvenient neighbour to authority ever "
    "since.",
    "{Name} learned the old boundary at {Place} the way other children learn a song, walked it every year with an "
    "uncle, and never once saw a fence line agree with it. {Name} arrived here with that argument already a "
    "generation old."};

static const char* const kBio0Hostile[kVariants] = {
    "{Name} came up through a service that has administered ground like {Place} for longer than anyone still in "
    "it can remember. Nothing in that education suggested the old boundary was any business of the office.",
    "The family of {Name} has held commissions for four generations, and the earliest of them signed the survey "
    "that reduced {Place} to parcels. The paperwork of that century is settled law in this office, and every "
    "objection to it is weather.",
    "{Name} was posted to this country young, read the whole file on {Place} in a single week, and concluded that "
    "the ground had been quiet for a long time because it had been governed firmly. Nothing since has moved that "
    "conclusion.",
    "{Name} rose in a service that measures success by how little happens, and the ground around {Place} is the "
    "reason for the posting. {FactionCap} sent an administrator and got a commander, which the file records as "
    "an improvement.",
    "Before this posting {Name} spent eleven years enforcing surveys in another country and never lost a parcel "
    "of one. The file on {Place} was described as straightforward. {Name} accepted the posting on that "
    "description.",
    "{Name} has read the old claims on {Place}, all of them, and dismisses them in the same measured voice kept "
    "for the weather. A claim without a seal is a story, and stories have never held ground."};

static const char* const kBio1Friendly[kVariants] = {
    "{Name} grew up two streets from the archive at {Place} and was taken as a child to see the settlement seals "
    "under glass. That was the year the pumps were closed. {Name} has read this country ever since the way other "
    "people read a face.",
    "{Name} was in the square at {Place} on the day the gates went up, small enough to be lifted for a better "
    "view and old enough to remember what the adults said afterwards. Nobody in that family has signed anything "
    "since.",
    "The father of {Name} witnessed the agreement at {Place} and spent the rest of a long life explaining, to "
    "anyone who would sit still, exactly which clause was broken first. {Name} can still quote the clause and "
    "does so rarely.",
    "{Name} clerked in an office near {Place} long enough to see which promises were filed and which were quietly "
    "reclassified, then left without notice. {FactionCap} gained a fighter who knows exactly where the paperwork "
    "is kept.",
    "{Name} was born the season the compact at {Place} failed, into a household that dated everything from it. "
    "Birthdays, harvests and debts were all counted forward from that year. Nothing in that house was ever "
    "described as settled.",
    "{Name} carried water past the checkpoint at {Place} twice a day for six years and learned every guard "
    "rotation without meaning to. The habit outlived the job, and this cell has been the beneficiary of it."};

static const char* const kBio1Hostile[kVariants] = {
    "{Name} countersigned the order that closed the agreement at {Place} and calls it the cleanest piece of "
    "administration of a long career. The complaints arrived in writing and were answered in writing.",
    "{Name} arrived at {Place} with instructions to restore order and a reputation for doing it without noise. "
    "The gates went up within the week. {FactionCap} rewarded {Name} for it, and the file has never been reopened.",
    "{Name} learned the trade in an office that treated a signed settlement as a starting position, and applied "
    "that lesson at {Place} with patience. Every parcel there was reclassified rather than seized, which took "
    "longer and held better.",
    "The reputation of {Name} rests on the checkpoints at {Place}, which have never been overrun and have never "
    "needed to be. The local memory of the old agreement is filed here as an administrative problem.",
    "{Name} keeps the original of the agreement at {Place} in a drawer and produces it for visitors, unfolded "
    "carefully, as proof that everything done since was lawful. The unfolding is practised. The argument has been "
    "made many times.",
    "{Name} was sent to {Place} to hold a line that a treaty had already drawn badly, and has held it for years "
    "without once being thanked. {FactionCap} counted that as loyalty. {Name} counts it as an unpaid debt."};

static const char* const kBio2Friendly[kVariants] = {
    "{Name} was raised on the story of what happened at {Place} and can name all of the men who held that ground. "
    "{Name} came to this cell with no training worth the word and an unusually exact memory for ground.",
    "Two uncles of {Name} were on the ridge at {Place} and only one came back down it. The family tells both "
    "halves of that story every year, in order, and {Name} has never been able to hear the second half sitting "
    "down.",
    "{Name} was a child in {Place} when the column came through, was carried out through an orchard, and "
    "remembers the orchard better than the column. {FactionCap} did not have to recruit {Name}, who arrived asking "
    "where to sign.",
    "{Name} spent a season after {Place} helping to write down the names, one at a time, from whoever could still "
    "remember them. That list is the reason {Name} is here, and it lives folded in a pocket that never gets "
    "emptied.",
    "The mother of {Name} walked out of {Place} with two children and no shoes and never afterwards described the "
    "walk. {Name} learned the whole of it from neighbours, in pieces, across about ten years, and has been "
    "assembling the account ever since.",
    "{Name} was too young for {Place} by a year and has been making up the difference ever since. Older fighters "
    "find this tiresome. Nobody has yet found a way of telling {Name} to stop."};

static const char* const kBio2Hostile[kVariants] = {
    "{Name} commanded a sector next to {Place} in that season and signed the report that called the whole affair "
    "a policing action. The report is still quoted in training, and everyone who was there has stopped correcting "
    "it.",
    "{Name} was among the officers who took {Place} on the following day and remembers it as an orderly operation "
    "conducted in difficult weather. The account has been repeated so often that {Name} may now believe it.",
    "{Name} lost a brother at {Place} and has been posted within sight of it ever since, by request. "
    "{FactionCap} recorded that as dedication. It is closer to an argument only one side is still allowed to make.",
    "{Name} arrived after {Place} had already burned and spent two years explaining to visiting officials why "
    "nothing further was required. Every one of those officials went away satisfied, and the file has not been "
    "opened since.",
    "{Name} was decorated for the column that reached {Place} on the third day, and wears the ribbon without "
    "comment. The delay that earned it is never mentioned. {FactionCap} never recorded the delay at all.",
    "{Name} keeps a photograph of {Place} taken the week afterwards and shows it to junior officers as an "
    "instructional matter. The lesson drawn from it is about timetables. Nobody in the room has ever asked about "
    "anything else."};

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

// "the ranks now called " + "Soviet Army"
RString PhraseWith(int phrase, const RString& factionName)
{
    const int i = (phrase >= 0 && phrase < kPhraseCount) ? phrase : 0;
    Text out;
    out.Add(kFactionPhrase[i]);
    out.AddChar(' ');
    out.Add(factionName);
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
