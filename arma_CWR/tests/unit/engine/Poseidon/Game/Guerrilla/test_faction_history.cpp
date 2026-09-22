#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/FactionHistory.hpp>
#include <Poseidon/Game/Guerrilla/LegendSeed.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

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

struct FactionPair
{
    const char* resistance;
    const char* occupier;
};

// The four-word row is mandatory: a long faction display name is what pushes
// the opening past one hundred words, and without it the overflow ships.
const FactionPair kPairs[] = {{"FIA", "Soviet Army"},
                              {"IDF", "Hizballah"},
                              {"PLO East", "IDF"},
                              {"Egyptian Frontier Force", "Israel Defense Forces"}};
constexpr int kPairCount = (int)(sizeof(kPairs) / sizeof(kPairs[0]));

const char* const kIslands[] = {"Malden", "Lebanon (80's)", "Sinai", "", "Nogova2"};
constexpr int kIslandCount = (int)(sizeof(kIslands) / sizeof(kIslands[0]));

HistoryInputs MakeInputs(const FactionPair& pair, const char* island, unsigned seed)
{
    HistoryInputs in;
    in.resistanceName = pair.resistance;
    in.occupierName = pair.occupier;
    in.islandName = island;
    in.seed = seed;
    in.settlements.Add(MakePlace("Houdan", true));
    in.settlements.Add(MakePlace("Chapoi", true));
    in.settlements.Add(MakePlace("Ras Nasrani", true));
    in.settlements.Add(MakePlace("El Tor", true));
    in.features.Add(MakePlace("Larche", false));
    in.features.Add(MakePlace("Ghajar", false));
    in.zoneNames.Add(RString("Camp"));
    return in;
}

// ---------------------------------------------------------------------------
// lint helpers
// ---------------------------------------------------------------------------

std::string Lower(const std::string& s)
{
    std::string out = s;
    for (char& c : out)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c = (char)(c + ('a' - 'A'));
        }
    }
    return out;
}

bool IsWordChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '\'';
}

// case-insensitive, word-boundary search
bool ContainsWord(const std::string& hay, const std::string& word)
{
    const std::string h = Lower(hay);
    const std::string w = Lower(word);
    for (size_t at = h.find(w); at != std::string::npos; at = h.find(w, at + 1))
    {
        const bool leftOk = at == 0 || !IsWordChar(h[at - 1]);
        const size_t end = at + w.size();
        const bool rightOk = end >= h.size() || !IsWordChar(h[end]);
        if (leftOk && rightOk)
        {
            return true;
        }
    }
    return false;
}

int CountSubstring(const std::string& hay, const std::string& needle)
{
    if (needle.empty())
    {
        return 0;
    }
    const std::string h = Lower(hay);
    const std::string n = Lower(needle);
    int found = 0;
    for (size_t at = h.find(n); at != std::string::npos; at = h.find(n, at + n.size()))
    {
        found++;
    }
    return found;
}

// house style: no "not X but Y" antithesis inside one sentence
bool HasAntithesis(const std::string& text)
{
    size_t begin = 0;
    while (begin <= text.size())
    {
        size_t end = text.find_first_of(".!?;:", begin);
        if (end == std::string::npos)
        {
            end = text.size();
        }
        const std::string sentence = text.substr(begin, end - begin);
        const std::string low = Lower(sentence);
        const size_t notAt = low.find("not ");
        if (notAt != std::string::npos && (notAt == 0 || !IsWordChar(low[notAt - 1])))
        {
            const size_t butAt = low.find(" but ", notAt);
            if (butAt != std::string::npos)
            {
                return true;
            }
        }
        if (end >= text.size())
        {
            break;
        }
        begin = end + 1;
    }
    return false;
}

// Every rule that holds for a table string AND for a generated string. The
// per-character scans collapse into one boolean each: this runs over tens of
// thousands of generated strings, and a CHECK per byte would dominate the suite.
void CheckSharedLints(const std::string& text, const char* what)
{
    INFO(what << ": " << text);
    REQUIRE_FALSE(text.empty());
    bool ascii = true, control = false, digit = false;
    for (char raw : text)
    {
        const unsigned char c = (unsigned char)raw;
        ascii = ascii && c < 0x80; // pure ASCII is what makes an em dash impossible
        control = control || c == '\n' || c == '\r' || c == '\t';
        digit = digit || (c >= '0' && c <= '9'); // numbers are spelled, so nothing reads as a date
    }
    CHECK(ascii);
    CHECK_FALSE(control);
    CHECK_FALSE(digit);
    CHECK(text.find("--") == std::string::npos);
    CHECK(text.front() != '@'); // Localize would blank such a string on a marker
    CHECK(text.front() != '$');
    CHECK(text.front() != ' ');
    CHECK(text.back() != ' ');
    CHECK(text.find("  ") == std::string::npos);
    // no third-person pronoun anywhere: a template can then never mis-gender a
    // companion whose script name and body class disagree
    const bool pronoun = ContainsWord(text, "he") || ContainsWord(text, "she") || ContainsWord(text, "his") ||
                         ContainsWord(text, "her") || ContainsWord(text, "him") || ContainsWord(text, "hers");
    CHECK_FALSE(pronoun);
    CHECK_FALSE(HasAntithesis(text));
}

// A faction display name must never be preceded by an article nor followed by a
// possessive: that is the whole point of the neutral constructions.
void CheckFactionInsertion(const std::string& text, const std::string& faction)
{
    const std::string low = Lower(text);
    const std::string name = Lower(faction);
    for (size_t at = low.find(name); at != std::string::npos; at = low.find(name, at + 1))
    {
        INFO(text << " / " << faction);
        CHECK(low.compare(at + name.size(), 2, "'s") != 0);
        // token immediately before the name
        size_t end = at;
        while (end > 0 && low[end - 1] == ' ')
        {
            end--;
        }
        size_t begin = end;
        while (begin > 0 && IsWordChar(low[begin - 1]))
        {
            begin--;
        }
        const std::string before = low.substr(begin, end - begin);
        CHECK(before != "the");
        CHECK(before != "a");
        CHECK(before != "an");
    }
}

// A faction slot renders as a whole CONSTRUCTION denoting people ("those who
// answer to X", "arms gathered under the name X"), so a LOCATIVE preposition in
// front of it produces "In arms gathered under the name Soviet Army the delay
// ...", which is not English.  The prepositions the shipped templates use
// correctly - to, for, with, from, under, against, among, beside - are
// explicitly allowed; only the place-words are rejected.  The article lint
// above cannot see this: it inspects the tokens around the faction NAME, and
// the word before the name here is part of the construction.
void CheckNoLocativeBeforeFactionSlot(const std::string& text)
{
    static const char* const kLocative[] = {"in", "at", "on", "within", "inside", "near", "across"};
    for (size_t at = text.find('{'); at != std::string::npos; at = text.find('{', at + 1))
    {
        const size_t close = text.find('}', at);
        if (close == std::string::npos)
        {
            return;
        }
        const std::string slot = text.substr(at + 1, close - at - 1);
        const bool faction = slot == "Faction" || slot == "FactionCap" || slot == "Occupier" || slot == "OccupierCap" ||
                             slot == "Resistance" || slot == "ResistanceCap";
        if (!faction)
        {
            continue;
        }
        size_t end = at;
        while (end > 0 && text[end - 1] == ' ')
        {
            end--;
        }
        size_t begin = end;
        while (begin > 0 && IsWordChar(text[begin - 1]))
        {
            begin--;
        }
        const std::string before = Lower(text.substr(begin, end - begin));
        for (const char* word : kLocative)
        {
            INFO("'" << before << "' before {" << slot << "} in: " << text);
            CHECK(before != word);
        }
    }
}

// ---------------------------------------------------------------------------
// rendered-prose lints
// ---------------------------------------------------------------------------

// Every faction name that can reach the prose from a shipped library, by the
// route it actually arrives on. Only the three vanilla descriptors author a
// displayName (guerrilla-mode/config/guerrilla-factions.hpp:86 "US Army", :153
// "Soviet Army", :207 "FIA"). lobo-factions.hpp authors NONE, so all eight LoBo
// factions reach the prose through the CLASS-NAME fallback in
// WorldNames::FactionDisplayName, which rewrites an underscore to a space:
// IDF, EgyptFrontier, EgyptArmy, Syria, Jordan, Hizballah, PLO and PLO_East ->
// "PLO East". The set below therefore covers four shapes the guerrilla_native
// fixture cannot exercise at all (it authors no displayName and falls back to
// the bare class names EAST and GUER): two-word names, bare acronyms, an
// acronym followed by a word, and a run-together compound with an internal
// capital.
const char* const kShippedNames[] = {"US Army", "Soviet Army", "FIA",    "IDF",           "Hizballah", "PLO East",
                                     "PLO",     "Syria",       "Jordan", "EgyptFrontier", "EgyptArmy"};
constexpr int kShippedNameCount = (int)(sizeof(kShippedNames) / sizeof(kShippedNames[0]));

std::string LastWord(const std::string& s)
{
    const size_t space = s.find_last_of(' ');
    return space == std::string::npos ? s : s.substr(space + 1);
}

// The three words English has that introduce a BARE proper name. "called
// Soviet Army", "as Soviet Army" and "the name Soviet Army" all read; "to
// Soviet Army", "themselves Soviet Army", "of Soviet Army" and "under Soviet
// Army" all want a "the" that no generator can know to insert, because the same
// slot also has to render "FIA".
bool IsNameIntroducer(const std::string& word)
{
    const std::string w = Lower(word);
    return w == "as" || w == "called" || w == "name";
}

// The token immediately before `at`, lowercased; "" when the name opens the
// text (which a construction should make impossible).
std::string TokenBefore(const std::string& low, size_t at)
{
    size_t end = at;
    while (end > 0 && low[end - 1] == ' ')
    {
        end--;
    }
    size_t begin = end;
    while (begin > 0 && IsWordChar(low[begin - 1]))
    {
        begin--;
    }
    return low.substr(begin, end - begin);
}

// Every occurrence of a faction display name must sit immediately behind a
// name-introducing word. This is the whole fix expressed as one rule: it fires
// on "those who answer to Soviet Army" (before = "to") and on "the fighters who
// call themselves Soviet Army" (before = "themselves") for every name shape,
// including the acronyms where the reading was merely odd rather than wrong.
void CheckNameIntroducer(const std::string& text, const std::string& faction)
{
    const std::string low = Lower(text);
    const std::string name = Lower(faction);
    for (size_t at = low.find(name); at != std::string::npos; at = low.find(name, at + 1))
    {
        const bool leftOk = at == 0 || !IsWordChar(low[at - 1]);
        const size_t end = at + name.size();
        const bool rightOk = end >= low.size() || !IsWordChar(low[end]);
        if (!leftOk || !rightOk)
        {
            continue;
        }
        const std::string before = TokenBefore(low, at);
        INFO("'" << before << "' immediately before '" << faction << "' in: " << text);
        CHECK(IsNameIntroducer(before));
    }
}

// The readings that shipped before this case existed, plus the ones spec.md
// section 3 and design.md:68 suggest in passing ("forces of {faction}", "those
// who march under X"). None of them is reachable through an article or a
// locative preposition, so the mechanical lints cannot see them; they are named
// here for the six display names that actually ship.
void CheckNoKnownBadReading(const std::string& text, const std::string& faction)
{
    // "the name of X" is on the list too: it wants a "the" of its own for a
    // two-word name, so it is no safer than "forces of X".
    static const char* const kBadIntro[] = {
        "answer to", "call themselves", "forces of", "march under", "the name of", "loyal to", "under", "of", "for",
        "with",      "themselves",      "to"};
    const std::string low = Lower(text);
    for (const char* intro : kBadIntro)
    {
        const std::string probe = std::string(intro) + " " + Lower(faction);
        INFO("'" << probe << "' in: " << text);
        CHECK(low.find(probe) == std::string::npos);
    }
    // and never an article or a possessive against the name itself
    INFO(text);
    CHECK(low.find("the " + Lower(faction)) == std::string::npos);
    CHECK(low.find("a " + Lower(faction)) == std::string::npos);
    CHECK(low.find("an " + Lower(faction)) == std::string::npos);
    CHECK(low.find(Lower(faction) + "'s") == std::string::npos);
}

// Mechanical damage a substitution can do: an unknown slot left with its braces
// on, an empty substitution collapsing two spaces into one place, a doubled
// article where a construction met a template's own "the".
void CheckRenderedProse(const std::string& text)
{
    INFO(text);
    REQUIRE_FALSE(text.empty());
    CHECK(text.find('{') == std::string::npos);
    CHECK(text.find('}') == std::string::npos);
    CHECK(text.find("  ") == std::string::npos);
    CHECK(text.find(" .") == std::string::npos);
    CHECK(text.find(" ,") == std::string::npos);
    CHECK(text.find(" ;") == std::string::npos);
    CHECK(text.find("..") == std::string::npos);
    CHECK(text.find(",,") == std::string::npos);
    const std::string low = " " + Lower(text) + " ";
    static const char* const kDoubled[] = {" the the ", " the a ", " the an ", " a the ",  " an the ",
                                           " a a ",     " to to ", " as as ",  " and and "};
    for (const char* pair : kDoubled)
    {
        INFO("doubled '" << pair << "'");
        CHECK(low.find(pair) == std::string::npos);
    }
}

// Every sentence starts with a capital and the block ends on a full stop. This
// is what catches a lower-case construction dropped into a sentence-initial
// slot, which is exactly the mistake {Occupier} instead of {OccupierCap} makes.
void CheckSentenceShape(const std::string& text)
{
    INFO(text);
    REQUIRE_FALSE(text.empty());
    CHECK(text.back() == '.');
    size_t begin = 0;
    while (begin < text.size())
    {
        while (begin < text.size() && text[begin] == ' ')
        {
            begin++;
        }
        if (begin >= text.size())
        {
            break;
        }
        const char first = text[begin];
        INFO("sentence starts '" << text.substr(begin, 40) << "'");
        CHECK(((first >= 'A' && first <= 'Z') || first == '"'));
        const size_t stop = text.find('.', begin);
        if (stop == std::string::npos)
        {
            break;
        }
        begin = stop + 1;
    }
}

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

int Words(const RString& s)
{
    return HistoryWordCount((const char*)s);
}

} // namespace

// ---------------------------------------------------------------------------
// table lints
// ---------------------------------------------------------------------------

TEST_CASE("Faction history - every template string passes the house lints", "[game][guerrilla][legends][history]")
{
    const int count = HistoryTemplateStringCount();
    // four opening slots, three titles, three texts, six biography cells, all at
    // six variants, plus the five constructions and the four generic places
    CHECK(count == 16 * HistoryVariantCount() + HistoryFactionPhraseCount() + 4);
    for (int i = 0; i < count; i++)
    {
        const std::string text = HistoryTemplateString(i);
        CheckSharedLints(text, "template");
        CheckNoLocativeBeforeFactionSlot(text);
        // only known slots: an unknown one would render with its braces intact
        for (size_t at = text.find('{'); at != std::string::npos; at = text.find('{', at + 1))
        {
            const size_t close = text.find('}', at);
            REQUIRE(close != std::string::npos);
            const std::string slot = text.substr(at + 1, close - at - 1);
            const bool known = slot == "Island" || slot == "Place" || slot == "Occupier" || slot == "OccupierCap" ||
                               slot == "Resistance" || slot == "ResistanceCap" || slot == "Name" || slot == "Faction" ||
                               slot == "FactionCap";
            INFO("slot " << slot << " in " << text);
            CHECK(known);
        }
    }
}

TEST_CASE("Faction history - the five faction constructions carry no article or possessive",
          "[game][guerrilla][legends][history]")
{
    REQUIRE(HistoryFactionPhraseCount() == 5);
    int plural = 0;
    for (int i = 0; i < HistoryFactionPhraseCount(); i++)
    {
        const std::string phrase = HistoryFactionPhrase(i);
        INFO(phrase);
        CHECK_FALSE(phrase.empty());
        CHECK(phrase.back() != ' ');
        CHECK(phrase.find('\'') == std::string::npos);
        // The construction ends on the word the faction name follows, and that
        // word has to be one that introduces a BARE proper name. English offers
        // three: "as", "called" and "the name". A construction ending anywhere
        // else ("...who answer to", "...who call themselves") leaves the reader
        // supplying an article, and the generator cannot supply one: "the
        // Soviet Army" wants it, "FIA" does not.
        CHECK(IsNameIntroducer(LastWord(phrase)));
        plural +=
            ContainsWord(phrase, "ranks") || ContainsWord(phrase, "fighters") || ContainsWord(phrase, "arms") ? 1 : 0;
    }
    // three plural constructions and two singular ones, which is why every
    // template puts a construction in the subject of a SIMPLE PAST verb or in
    // the object of a preposition and never in front of an "is" or a "were"
    CHECK(plural == 3);
}

TEST_CASE("Faction history - the rendered prose reads correctly for every shipped display name",
          "[game][guerrilla][legends][history]")
{
    // The defect this case exists for: the fixture mission authors no
    // displayName, so a faction reaches the generator as the bare class name
    // EAST or GUER and every construction reads acceptably. The SHIPPED
    // libraries author "US Army", "Soviet Army", "FIA" (guerrilla-factions.hpp)
    // and, through the underscore rewrite, "PLO East" - and a two-word name
    // after "...who answer to" wants an article that nothing can insert. So
    // this case renders the whole history and every biography for the real
    // names, in both campaign roles (spec.md section 5), and asserts on the
    // OUTPUT rather than on the tables.
    const RString kBossName("Cold Erez \"The Land-Taker\" Dayan the Occupier");
    for (unsigned seed = 1; seed <= 11; seed += 2)
    {
        for (int o = 0; o < kShippedNameCount; o++)
        {
            for (int r = 0; r < kShippedNameCount; r++)
            {
                if (o == r)
                {
                    continue; // one campaign never fields a faction against itself
                }
                FactionPair pair;
                pair.occupier = kShippedNames[o];
                pair.resistance = kShippedNames[r];
                const HistoryInputs in = MakeInputs(pair, "Malden", seed);
                const HistoryRecord rec = GenerateHistory(in);
                REQUIRE(rec.Present());

                AutoArray<RString> rendered;
                rendered.Add(rec.openingPage1);
                rendered.Add(rec.openingPage2);
                for (int k = 0; k < kHistoryEvents; k++)
                {
                    rendered.Add(rec.eventText[k]);
                }
                for (int k = 0; k < kHistoryEvents; k++)
                {
                    for (int hostile = 0; hostile <= 1; hostile++)
                    {
                        const RString name = hostile ? kBossName : RString("Petra");
                        const RString faction = hostile ? RString(pair.occupier) : RString(pair.resistance);
                        for (int row = 0; row < 2; row++)
                        {
                            const unsigned long long key = HashKey(row == 0 ? "comp_0_petra" : "boss_1", seed + row);
                            rendered.Add(GenerateBio(rec, k, name, faction, hostile != 0, key));
                        }
                    }
                }

                for (int i = 0; i < rendered.Size(); i++)
                {
                    const std::string text = Str(rendered[i]);
                    INFO("seed " << seed << " occupier " << pair.occupier << " resistance " << pair.resistance << " -> "
                                 << text);
                    CheckRenderedProse(text);
                    CheckSentenceShape(text);
                    // every faction name is introduced by a construction that
                    // makes it a name slot, in both roles
                    CheckNameIntroducer(text, pair.occupier);
                    CheckNameIntroducer(text, pair.resistance);
                    CheckFactionInsertion(text, pair.occupier);
                    CheckFactionInsertion(text, pair.resistance);
                    // the readings the two retired constructions produced, and
                    // the ones spec.md section 3 and design.md suggest, spelled
                    // out for the six names: no mechanical rule would catch
                    // "forces of Soviet Army", because the word before the name
                    // is neither an article nor a locative
                    for (int n = 0; n < kShippedNameCount; n++)
                    {
                        CheckNoKnownBadReading(text, kShippedNames[n]);
                    }
                }

                // and the whole thing actually said both names: an empty
                // substitution would pass every lint above in silence
                const std::string whole = Str(rec.openingPage1) + " " + Str(rec.openingPage2) + " " +
                                          Str(rec.eventText[1]) + " " + Str(rec.eventText[2]);
                INFO(whole);
                CHECK(ContainsWord(whole, pair.occupier));
                CHECK(ContainsWord(whole, pair.resistance));
            }
        }
    }
}

TEST_CASE("Faction history - the extra phrase channels do not collide with LegendChannel",
          "[game][guerrilla][legends][history]")
{
    // FactionHistory.cpp continues the channel space above CH_BOSSWORD with 60,
    // 61, 62 and 63, one per faction insertion point.
    const int used[] = {CH_OPEN_A,    CH_OPEN_B,   CH_OPEN_C,   CH_OPEN_D,  CH_EVENT0,     CH_EVENT1,
                        CH_EVENT2,    CH_PLACE0,   CH_PLACE1,   CH_PLACE2,  CH_LAST,       CH_FACE,
                        CH_BIOEVENT,  CH_BIOVAR,   CH_AWARD1,   CH_AWARD2,  CH_AWARD1WORD, CH_AWARD2WORD,
                        CH_BOSSFIRST, CH_BOSSLAST, CH_BOSSSLOT, CH_BOSSWORD};
    for (int value : used)
    {
        CHECK(value < 60);
    }
}

// ---------------------------------------------------------------------------
// budgets
// ---------------------------------------------------------------------------

TEST_CASE("Faction history - the opening budget is reachable from every rolled A and B",
          "[game][guerrilla][legends][history]")
{
    // Structural, not sampled: for every (A, B) pair and the widest faction
    // names and place names in the matrix, some (C, D) pair lands the opening
    // inside eighty to one hundred words. This is what proves PickToBudget can
    // never fall through to its "closest" branch on shipped data.
    for (int p = 0; p < kPairCount; p++)
    {
        for (int isl = 0; isl < kIslandCount; isl++)
        {
            const HistoryInputs in = MakeInputs(kPairs[p], kIslands[isl], 3);
            for (int a = 0; a < HistoryVariantCount(); a++)
            {
                for (int b = 0; b < HistoryVariantCount(); b++)
                {
                    HistoryForcedIndices forced;
                    forced.opening[0] = a;
                    forced.opening[1] = b;
                    const HistoryRecord rec = GenerateHistoryForced(in, forced);
                    const int total = Words(rec.openingPage1) + Words(rec.openingPage2);
                    INFO("A " << a << " B " << b << " " << kPairs[p].resistance << " vs " << kPairs[p].occupier
                              << " on " << kIslands[isl] << " -> " << total);
                    CHECK(total >= kHistoryOpeningMinWords);
                    CHECK(total <= kHistoryOpeningMaxWords);
                }
            }
        }
    }
}

TEST_CASE("Faction history - the opening table spans the budget from both ends", "[game][guerrilla][legends][history]")
{
    // The static half of the guarantee: the longest first block plus the
    // shortest second block still fits under the ceiling, and the shortest
    // first block plus the longest second block still clears the floor.
    for (int p = 0; p < kPairCount; p++)
    {
        const HistoryInputs in = MakeInputs(kPairs[p], "Malden", 3);
        int maxHead = 0, minHead = 1000, maxTail = 0, minTail = 1000;
        for (int i = 0; i < HistoryVariantCount(); i++)
        {
            for (int j = 0; j < HistoryVariantCount(); j++)
            {
                HistoryForcedIndices forced;
                forced.opening[0] = i;
                forced.opening[1] = j;
                forced.opening[2] = i;
                forced.opening[3] = j;
                const HistoryRecord rec = GenerateHistoryForced(in, forced);
                const int head = Words(rec.openingPage1);
                const int tail = Words(rec.openingPage2);
                maxHead = head > maxHead ? head : maxHead;
                minHead = head < minHead ? head : minHead;
                maxTail = tail > maxTail ? tail : maxTail;
                minTail = tail < minTail ? tail : minTail;
            }
        }
        INFO(kPairs[p].resistance << " head " << minHead << ".." << maxHead << " tail " << minTail << ".." << maxTail);
        CHECK(maxHead + minTail <= kHistoryOpeningMaxWords);
        CHECK(minHead + maxTail >= kHistoryOpeningMinWords);
    }
}

TEST_CASE("Faction history - word budgets and output lints over the generation matrix",
          "[game][guerrilla][legends][history]")
{
    for (unsigned seed = 1; seed <= 401; seed += 2) // 201 odd seeds, as SeedCampaign draws them
    {
        for (int p = 0; p < kPairCount; p++)
        {
            for (int isl = 0; isl < kIslandCount; isl++)
            {
                const HistoryInputs in = MakeInputs(kPairs[p], kIslands[isl], seed);
                const HistoryRecord rec = GenerateHistory(in);
                REQUIRE(rec.Present());
                CHECK(rec.version == kHistoryVersion);
                CHECK(rec.seed == seed);

                const int opening = Words(rec.openingPage1) + Words(rec.openingPage2);
                INFO("seed " << seed << " " << kPairs[p].resistance << " vs " << kPairs[p].occupier << " on "
                             << kIslands[isl] << " opening " << opening);
                CHECK(opening >= kHistoryOpeningMinWords);
                CHECK(opening <= kHistoryOpeningMaxWords);
                CheckSharedLints(Str(rec.openingPage1), "openingPage1");
                CheckSharedLints(Str(rec.openingPage2), "openingPage2");

                for (int k = 0; k < kHistoryEvents; k++)
                {
                    CHECK_FALSE(Str(rec.eventTitle[k]).empty());
                    CHECK(Words(rec.eventTitle[k]) <= 6);
                    CHECK(Words(rec.eventText[k]) >= kHistoryEventMinWords);
                    CHECK(Words(rec.eventText[k]) <= kHistoryEventMaxWords);
                    CHECK_FALSE(Str(rec.eventPlace[k]).empty());
                    CheckSharedLints(Str(rec.eventTitle[k]), "eventTitle");
                    CheckSharedLints(Str(rec.eventText[k]), "eventText");
                    // the place the page's subtitle shows is named by the page
                    // itself, in the title or in the prose
                    const std::string page = Str(rec.eventTitle[k]) + " " + Str(rec.eventText[k]);
                    CHECK(page.find(Str(rec.eventPlace[k])) != std::string::npos);
                }

                const std::string whole = Str(rec.openingPage1) + " " + Str(rec.openingPage2) + " " +
                                          Str(rec.eventText[0]) + " " + Str(rec.eventText[1]) + " " +
                                          Str(rec.eventText[2]);
                CheckFactionInsertion(whole, kPairs[p].resistance);
                CheckFactionInsertion(whole, kPairs[p].occupier);

                // the four history insertion points spend four DIFFERENT
                // constructions: independent draws would repeat one of five
                // phrases in about ninety-six campaigns out of a hundred
                int spent = 0;
                for (int i = 0; i < HistoryFactionPhraseCount(); i++)
                {
                    const int uses = CountSubstring(whole, HistoryFactionPhrase(i));
                    INFO("phrase " << i << " used " << uses << " times");
                    CHECK(uses <= 1);
                    spent += uses;
                }
                CHECK(spent == kHistoryPhrases);
                for (int i = 0; i < kHistoryPhrases; i++)
                {
                    for (int j = i + 1; j < kHistoryPhrases; j++)
                    {
                        CHECK(rec.phraseIndex[i] != rec.phraseIndex[j]);
                    }
                }
            }
        }
    }
}

TEST_CASE("Faction history - biographies stay inside their budget and stay pre-campaign",
          "[game][guerrilla][legends][history]")
{
    // the widest name the shipped bank can assemble, so a boss dossier is
    // budgeted at its worst case
    const RString kLongName("Heartless Jean-Baptiste \"The Fence-Builder\" Vakalalabure the Collaborator");
    for (unsigned seed = 1; seed <= 61; seed += 2)
    {
        for (int p = 0; p < kPairCount; p++)
        {
            const HistoryInputs in = MakeInputs(kPairs[p], "Malden", seed);
            const HistoryRecord rec = GenerateHistory(in);
            for (int event = 0; event < kHistoryEvents; event++)
            {
                for (int hostile = 0; hostile <= 1; hostile++)
                {
                    const RString name = hostile ? kLongName : RString("Petra");
                    const RString faction = hostile ? RString(kPairs[p].occupier) : RString(kPairs[p].resistance);
                    for (int row = 0; row < 4; row++)
                    {
                        const unsigned long long key = HashKey(row == 0 ? "comp_0_petra" : "boss_0", seed + row);
                        const RString bio = GenerateBio(rec, event, name, faction, hostile != 0, key);
                        INFO("seed " << seed << " event " << event << " hostile " << hostile << " -> " << Str(bio));
                        CHECK(Words(bio) >= kHistoryBioMinWords);
                        CHECK(Words(bio) <= kHistoryBioMaxWords);
                        CheckSharedLints(Str(bio), "bio");
                        CheckFactionInsertion(Str(bio), Str(faction));
                        // hung on the referenced event, which is what makes the
                        // dossiers and the Chronicles read as one past
                        CHECK(Str(bio).find(Str(rec.eventPlace[event])) != std::string::npos);
                        // strictly pre-campaign: the deed list owns everything
                        // the registry records
                        CHECK_FALSE(ContainsWord(Str(bio), "XP"));
                        CHECK_FALSE(ContainsWord(Str(bio), "rank"));
                        CHECK_FALSE(ContainsWord(Str(bio), "kill"));
                        CHECK_FALSE(ContainsWord(Str(bio), "killed"));
                        CHECK_FALSE(ContainsWord(Str(bio), "captured"));
                        CHECK(Str(bio).find("promot") == std::string::npos);
                    }
                }
            }
        }
    }
}

TEST_CASE("Faction history - a biography asked for the dossier page's budget fits that page",
          "[game][guerrilla][legends][history]")
{
    // The journal clamps the dossier's biography to kHistoryBioPageWords beside
    // the portrait box and NOTHING downstream carries the overflow ("Full
    // record" lists journal entries, never prose), so a clamped word is a word
    // the player can read nowhere.  The registry therefore asks for a variant
    // that already fits, and this is the case that says it always can, at the
    // worst combination the shipped tables can produce: the widest five-slot
    // boss name, three-word place names and a three-word faction display name
    // inside the longest construction.
    const RString kWidestName("Heartless Jean-Baptiste \"The Fence-Builder\" Vakalalabure the Collaborator");
    const FactionPair kWidestPair = {"Egyptian Frontier Force", "Israel Defense Forces"};

    HistoryInputs in;
    in.resistanceName = kWidestPair.resistance;
    in.occupierName = kWidestPair.occupier;
    in.islandName = "Malden";
    in.settlements.Add(MakePlace("Ras Nasrani Point", true));
    in.settlements.Add(MakePlace("El Tor Junction", true));
    in.settlements.Add(MakePlace("Houdan Ridge Farm", true));
    in.features.Add(MakePlace("The High Pass", false));

    bool startSeen[6] = {false, false, false, false, false, false};
    for (unsigned seed = 1; seed <= 41; seed += 2)
    {
        in.seed = seed;
        const HistoryRecord rec = GenerateHistory(in);
        for (int event = 0; event < kHistoryEvents; event++)
        {
            for (int hostile = 0; hostile <= 1; hostile++)
            {
                const RString name = hostile ? kWidestName : RString("Petra");
                const RString faction = hostile ? RString(kWidestPair.occupier) : RString(kWidestPair.resistance);
                for (int row = 0; row < 6; row++)
                {
                    char id[32];
                    snprintf(id, sizeof(id), "comp_%d_petra", row);
                    const unsigned long long key = HashKey(id, seed);
                    startSeen[Roll(key, CH_BIOVAR, 6) % 6] = true;
                    const RString bio = GenerateBio(rec, event, name, faction, hostile != 0, key, kHistoryBioPageWords);
                    INFO("seed " << seed << " event " << event << " hostile " << hostile << " -> " << Str(bio));
                    CHECK(Words(bio) >= kHistoryBioMinWords);
                    CHECK(Words(bio) <= kHistoryBioPageWords);
                    // the wider default band is still what an unbudgeted caller
                    // gets, so the parameter is doing the work and not a
                    // narrowed table
                    CHECK(Words(GenerateBio(rec, event, name, faction, hostile != 0, key)) <= kHistoryBioMaxWords);
                }
            }
        }
    }
    // every variant in the walk was a starting point somewhere above, so no
    // cell escaped the budget by never being reached
    for (int i = 0; i < 6; i++)
    {
        INFO("variant " << i << " was never a starting index");
        CHECK(startSeen[i]);
    }
}

TEST_CASE("Faction history - a biography takes the one construction the history left unused",
          "[game][guerrilla][legends][history]")
{
    const HistoryInputs in = MakeInputs(kPairs[0], "Malden", 3);
    const HistoryRecord rec = GenerateHistory(in);
    const int spare = rec.BioPhraseIndex();
    for (int i = 0; i < kHistoryPhrases; i++)
    {
        CHECK(rec.phraseIndex[i] != spare);
    }
    const RString bio = GenerateBio(rec, 0, RString("Petra"), RString("FIA"), false, HashKey("comp_0_petra", 3));
    const std::string whole =
        Str(rec.openingPage1) + " " + Str(rec.openingPage2) + " " + Str(rec.eventText[1]) + " " + Str(rec.eventText[2]);
    // the bio's construction, when it carries one, is the one the Chronicles
    // never used
    for (int i = 0; i < HistoryFactionPhraseCount(); i++)
    {
        if (CountSubstring(Str(bio), HistoryFactionPhrase(i)) > 0)
        {
            CHECK(i == spare);
            CHECK(CountSubstring(whole, HistoryFactionPhrase(i)) == 0);
        }
    }
}

TEST_CASE("Faction history - a biography without a history is empty", "[game][guerrilla][legends][history]")
{
    const HistoryRecord absent;
    CHECK_FALSE(absent.Present());
    CHECK(Str(GenerateBio(absent, 0, RString("Petra"), RString("FIA"), false, 1)).empty());
}

// ---------------------------------------------------------------------------
// determinism
// ---------------------------------------------------------------------------

TEST_CASE("Faction history - generation is a pure function of its inputs", "[game][guerrilla][legends][history]")
{
    const HistoryInputs in = MakeInputs(kPairs[1], "Lebanon (80's)", 12345);
    const HistoryRecord first = GenerateHistory(in);
    for (int repeat = 0; repeat < 5; repeat++)
    {
        const HistoryRecord again = GenerateHistory(in);
        CHECK(Str(again.openingPage1) == Str(first.openingPage1));
        CHECK(Str(again.openingPage2) == Str(first.openingPage2));
        for (int k = 0; k < kHistoryEvents; k++)
        {
            CHECK(again.eventIndex[k] == first.eventIndex[k]);
            CHECK(Str(again.eventTitle[k]) == Str(first.eventTitle[k]));
            CHECK(Str(again.eventText[k]) == Str(first.eventText[k]));
            CHECK(Str(again.eventPlace[k]) == Str(first.eventPlace[k]));
        }
        for (int k = 0; k < kHistoryOpenSlots; k++)
        {
            CHECK(again.openingIndex[k] == first.openingIndex[k]);
        }
        for (int k = 0; k < kHistoryPhrases; k++)
        {
            CHECK(again.phraseIndex[k] == first.phraseIndex[k]);
        }
    }

    // nothing is rolled when the record is read: a biography drawn off the same
    // key is byte-identical however many times a dossier is reopened
    const RString bio = GenerateBio(first, 1, RString("Petra"), RString("IDF"), false, HashKey("comp_0_petra", 12345));
    for (int repeat = 0; repeat < 5; repeat++)
    {
        CHECK(Str(GenerateBio(first, 1, RString("Petra"), RString("IDF"), false, HashKey("comp_0_petra", 12345))) ==
              Str(bio));
    }
    // and reading the record never mutates it
    CHECK(Str(GenerateHistory(in).openingPage1) == Str(first.openingPage1));
}

TEST_CASE("Faction history - a different seed tells a different history", "[game][guerrilla][legends][history]")
{
    int differ = 0;
    const int pairs = 200;
    for (int i = 0; i < pairs; i++)
    {
        const unsigned seedA = (unsigned)(2 * i + 1);
        const unsigned seedB = (unsigned)(2 * i + 3);
        const HistoryRecord a = GenerateHistory(MakeInputs(kPairs[0], "Malden", seedA));
        const HistoryRecord b = GenerateHistory(MakeInputs(kPairs[0], "Malden", seedB));
        bool moved = Str(a.openingPage1) != Str(b.openingPage1) || Str(a.openingPage2) != Str(b.openingPage2);
        for (int k = 0; k < kHistoryEvents; k++)
        {
            moved = moved || a.eventIndex[k] != b.eventIndex[k];
        }
        differ += moved ? 1 : 0;
    }
    CHECK(differ * 100 >= pairs * 95);
}

TEST_CASE("Faction history - reordering the settlements moves the places, never the variants",
          "[game][guerrilla][legends][history]")
{
    // Single-word settlement names on purpose: the C and D variants are chosen
    // against a rendered WORD COUNT, so a reorder that changed a place's word
    // count could legitimately move them. What must never move is the choice
    // itself for an unchanged rendering.
    HistoryInputs a;
    a.resistanceName = "FIA";
    a.occupierName = "Soviet Army";
    a.islandName = "Malden";
    a.seed = 4242 | 1;
    a.settlements.Add(MakePlace("Houdan", true));
    a.settlements.Add(MakePlace("Chapoi", true));
    a.settlements.Add(MakePlace("Vigny", true));
    a.features.Add(MakePlace("Larche", false));

    HistoryInputs b = a;
    b.settlements.Clear();
    b.settlements.Add(MakePlace("Vigny", true));
    b.settlements.Add(MakePlace("Chapoi", true));
    b.settlements.Add(MakePlace("Houdan", true));

    const HistoryRecord ra = GenerateHistory(a);
    const HistoryRecord rb = GenerateHistory(b);
    for (int k = 0; k < kHistoryEvents; k++)
    {
        CHECK(ra.eventIndex[k] == rb.eventIndex[k]);
    }
    for (int k = 0; k < kHistoryOpenSlots; k++)
    {
        CHECK(ra.openingIndex[k] == rb.openingIndex[k]);
    }
    CHECK(Str(ra.eventPlace[1]) != Str(rb.eventPlace[1]));
    CHECK(Str(ra.eventPlace[0]) == "Larche"); // the only feature, either way
}

// ---------------------------------------------------------------------------
// island word and place fallbacks
// ---------------------------------------------------------------------------

TEST_CASE("Faction history - the island word survives a decorated display name", "[game][guerrilla][legends][history]")
{
    CHECK(Str(HistoryIslandWord(RString("Lebanon (80's)"))) == "Lebanon");
    CHECK(Str(HistoryIslandWord(RString("Lebanon80"))) == "Lebanon");
    CHECK(Str(HistoryIslandWord(RString("Malden"))) == "Malden");
    CHECK(Str(HistoryIslandWord(RString("Nogova2"))) == "Nogova");
    CHECK(Str(HistoryIslandWord(RString(""))) == "this country");
    CHECK(Str(HistoryIslandWord(RString("2007"))) == "this country");
    CHECK(Str(HistoryIslandWord(RString("(80's)"))) == "this country");
    CHECK(Str(HistoryIslandWord(RString("Everon 2"))) == "Everon");
    CHECK(Str(HistoryIslandWord(RString("Kolgu4jev"))) == "this country"); // an interior digit is a code

    // the island word appears only in the opening, only as "on <word>"
    HistoryInputs in = MakeInputs(kPairs[0], "Lebanon (80's)", 3);
    for (int a = 0; a < HistoryVariantCount(); a++)
    {
        HistoryForcedIndices forced;
        forced.opening[0] = a;
        const HistoryRecord rec = GenerateHistoryForced(in, forced);
        INFO(Str(rec.openingPage1));
        CHECK(Str(rec.openingPage1).find("on Lebanon") != std::string::npos);
        CHECK(Str(rec.openingPage1).find("(80's)") == std::string::npos);
    }
}

TEST_CASE("Faction history - places fall back through features, settlements, zones and the generic table",
          "[game][guerrilla][legends][history]")
{
    for (unsigned seed = 1; seed <= 41; seed += 2)
    {
        {
            // no features: the ancient beat takes a settlement
            HistoryInputs in;
            in.resistanceName = "FIA";
            in.occupierName = "Soviet Army";
            in.islandName = "Malden";
            in.seed = seed;
            in.settlements.Add(MakePlace("Houdan", true));
            in.settlements.Add(MakePlace("Chapoi", true));
            in.settlements.Add(MakePlace("Vigny", true));
            const HistoryRecord rec = GenerateHistory(in);
            const std::string p0 = Str(rec.eventPlace[0]);
            CHECK((p0 == "Houdan" || p0 == "Chapoi" || p0 == "Vigny"));
            CHECK(Str(rec.eventPlace[1]) != Str(rec.eventPlace[2]));
            CHECK(p0 != Str(rec.eventPlace[1]));
            CHECK(p0 != Str(rec.eventPlace[2]));
        }
        {
            // no settlements at all: the zone table carries the campaign
            HistoryInputs in;
            in.resistanceName = "FIA";
            in.occupierName = "Soviet Army";
            in.islandName = "Malden";
            in.seed = seed;
            in.zoneNames.Add(RString("Camp"));
            in.zoneNames.Add(RString("Airfield"));
            in.zoneNames.Add(RString("Quarry"));
            const HistoryRecord rec = GenerateHistory(in);
            for (int k = 0; k < kHistoryEvents; k++)
            {
                const std::string p = Str(rec.eventPlace[k]);
                CHECK((p == "Camp" || p == "Airfield" || p == "Quarry"));
            }
            CHECK(Str(rec.eventPlace[1]) != Str(rec.eventPlace[2]));
        }
        {
            // nothing at all: the compiled generic table, and still non-empty
            HistoryInputs in;
            in.resistanceName = "FIA";
            in.occupierName = "Soviet Army";
            in.islandName = "";
            in.seed = seed;
            const HistoryRecord rec = GenerateHistory(in);
            for (int k = 0; k < kHistoryEvents; k++)
            {
                CHECK_FALSE(Str(rec.eventPlace[k]).empty());
            }
            CHECK(Str(rec.eventPlace[1]) != Str(rec.eventPlace[2]));
            CHECK(Str(rec.openingPage1).find("on this country") != std::string::npos);
        }
    }
}

// ---------------------------------------------------------------------------
// the three worked examples, pinned
// ---------------------------------------------------------------------------

namespace
{

// Seed three puts the first settlement on beat one and the second on beat two,
// so a golden case only has to force the variant and construction indices. No
// test may pin a seed against a LIVE game (the draw depends on how many systems
// drew from GRandGen first that frame); pinning one against the pure generator
// is what makes the worked examples a contract instead of decoration.
constexpr unsigned kGoldenSeed = 3;

HistoryInputs GoldenInputs(const char* resistance, const char* occupier, const char* island, const char* feature,
                           const char* firstTown, const char* secondTown)
{
    HistoryInputs in;
    in.resistanceName = resistance;
    in.occupierName = occupier;
    in.islandName = island;
    in.seed = kGoldenSeed;
    in.settlements.Add(MakePlace(firstTown, true));
    in.settlements.Add(MakePlace(secondTown, true));
    in.features.Add(MakePlace(feature, false));
    return in;
}

} // namespace

TEST_CASE("Faction history - worked example A, FIA against the Soviet Army on Malden",
          "[game][guerrilla][legends][history]")
{
    const HistoryInputs in = GoldenInputs("FIA", "Soviet Army", "Malden", "Larche", "Houdan", "Chapoi");
    HistoryForcedIndices forced;
    forced.opening[0] = 0;
    forced.opening[1] = 0;
    forced.opening[2] = 4;
    forced.opening[3] = 0;
    forced.events[0] = 0;
    forced.events[1] = 0;
    forced.events[2] = 0;
    forced.phrases[0] = 0;
    forced.phrases[1] = 2;
    forced.phrases[2] = 1;
    forced.phrases[3] = 4;
    const HistoryRecord rec = GenerateHistoryForced(in, forced);

    CHECK(Str(rec.openingPage1) ==
          "Long before any flag now flown on Malden, the people of these valleys cut terraces into the hills above "
          "Larche and held them against every season. Then came the columns of a distant power, and the road they "
          "cut has carried every ruler since, the ranks now called Soviet Army among them.");
    CHECK(Str(rec.openingPage2) ==
          "The compact made at Houdan lasted one generation. The terms were kept in an archive, and the ground they "
          "named was taken anyway. The fighters who muster as FIA inherited that quarrel unfinished, and the ground "
          "above Larche has not forgotten a single season of it.");
    CHECK(Str(rec.eventTitle[0]) == "The Terraces of Larche");
    CHECK(Str(rec.eventText[0]) ==
          "The old families cut the slopes above Larche into steps and fed four valleys from them for longer than "
          "any register records. When the first surveyors came with chains and paper, the steps became parcels, and "
          "the parcels became someone else's property. Nobody in the valley signed anything.");
    CHECK(Str(rec.eventTitle[1]) == "The Compact at Houdan");
    CHECK(Str(rec.eventText[1]) ==
          "A settlement was read aloud in the square at Houdan and witnessed by both sides: the high ground would "
          "stay common, the roads would stay open. Within one season the ground was fenced and the roads were "
          "gated. Whoever now marches as Soviet Army took the paper away; the square kept the reading.");
    CHECK(Str(rec.eventTitle[2]) == "The Stand at Chapoi");
    CHECK(Str(rec.eventText[2]) ==
          "At Chapoi the column was stopped for two days by fewer men than it had guns. They were not relieved and "
          "they did not expect to be. The road was opened on the third day. The standard now raised as FIA kept the "
          "count of those two days.");

    const RString bio = GenerateBioForced(rec, 0, RString("Petra"), RString("FIA"), false, 0, 0);
    CHECK(Str(bio) ==
          "Petra was born into one of the families that worked the ground above Larche, and learned every path on "
          "that slope before learning to read. Nobody in this cell has ever needed to hand Petra a map of ground a "
          "grandmother measured by hand.");
}

TEST_CASE("Faction history - worked example B, the IDF fielded as the resistance on Lebanon",
          "[game][guerrilla][legends][history]")
{
    // the role-swapped case: the LoBo IDF descriptor is the RESISTANCE here
    const HistoryInputs in = GoldenInputs("IDF", "Hizballah", "Lebanon (80's)", "Ghajar", "Tyre", "Saida");
    HistoryForcedIndices forced;
    for (int i = 0; i < kHistoryOpenSlots; i++)
    {
        forced.opening[i] = 1;
    }
    for (int i = 0; i < kHistoryEvents; i++)
    {
        forced.events[i] = 1;
    }
    forced.phrases[0] = 1;
    forced.phrases[1] = 4;
    forced.phrases[2] = 0;
    forced.phrases[3] = 2;
    const HistoryRecord rec = GenerateHistoryForced(in, forced);

    CHECK(Str(rec.openingPage1) ==
          "The oldest quarrel on Lebanon began over water. The wells at Ghajar were named, walled and fought for "
          "long before any banner now carried was stitched. Every power that has held this ground began by writing "
          "it down, and whoever now marches as Hizballah began the same way.");
    CHECK(Str(rec.openingPage2) ==
          "At Tyre a settlement was signed that promised the water would stay shared. The seals are still in the "
          "archive; the pumps are not. The standard now raised as IDF took up that grievance from people who had "
          "carried it a long time already.");
    CHECK(Str(rec.eventTitle[1]) == "The Seals at Tyre");
    CHECK(Str(rec.eventText[1]) ==
          "The settlement at Tyre was signed in front of witnesses from six villages: the water shared, the coast "
          "road open to all, no armed man at the pumps. The ranks now called Hizballah took the pumps. The road has "
          "been checked twice a day ever since.");
    CHECK(Str(rec.eventTitle[2]) == "The Burning of Saida");
    CHECK(Str(rec.eventText[2]) ==
          "Saida burned for a day and a night, and the people who came back counted the doorways rather than the "
          "houses. No relief column reached the town. The list of names carried out of it passed to the fighters "
          "who muster as IDF, and it has been read aloud once a year ever since.");

    const RString bio = GenerateBioForced(rec, 1, RString("Petra"), RString("IDF"), false, 0, 1);
    CHECK(Str(bio) ==
          "Petra was in the square at Tyre on the day the gates went up, small enough to be lifted for a better "
          "view and old enough to remember what the adults said afterwards. Nobody in that family has signed "
          "anything since.");
}

TEST_CASE("Faction history - worked example C, PLO East against the IDF on Sinai",
          "[game][guerrilla][legends][history]")
{
    // PLO_East reaches the prose as "PLO East": WorldNames::FactionDisplayName
    // rewrites the underscore on the class-name fallback branch
    const HistoryInputs in = GoldenInputs("PLO East", "IDF", "Sinai", "Nuweiba", "El Tor", "Ras Nasrani");
    HistoryForcedIndices forced;
    for (int i = 0; i < kHistoryOpenSlots; i++)
    {
        forced.opening[i] = 2;
    }
    for (int i = 0; i < kHistoryEvents; i++)
    {
        forced.events[i] = 2;
    }
    forced.phrases[0] = 3;
    forced.phrases[1] = 0;
    forced.phrases[2] = 2;
    forced.phrases[3] = 1;
    const HistoryRecord rec = GenerateHistoryForced(in, forced);

    CHECK(Str(rec.openingPage1) ==
          "Nothing on Sinai is older than the road to the wells, and the caravans that cut it wrote their claim "
          "into the rock at Nuweiba. Stone is a poor deed in a court, and every army that has crossed here has "
          "known it, arms gathered under the name IDF among them.");
    CHECK(Str(rec.openingPage2) ==
          "A truce was measured out at El Tor with stones set in the sand. The stones were moved before the copy of "
          "it had dried. The ranks now called PLO East took this quarrel in hand, along with the road, the wells and "
          "everything else nobody signed for.");
    CHECK(Str(rec.eventTitle[0]) == "The Rock at Nuweiba");
    CHECK(Str(rec.eventTitle[2]) == "The Column at Ras Nasrani");
    CHECK(Str(rec.eventText[2]) ==
          "A column that should have taken the coast in a morning was held at Ras Nasrani until dusk by men with "
          "two machine guns and the high ground. None of them was relieved. The coast fell the next day, and "
          "whoever now marches as PLO East counted the delay a victory anyway.");

    const RString bio = GenerateBioForced(rec, 2, RString("Petra"), RString("PLO East"), false, 0, 2);
    CHECK(Str(bio) ==
          "Petra was a child in Ras Nasrani when the column came through, was carried out through an orchard, and "
          "remembers the orchard better than the column. The standard now raised as PLO East did not have to "
          "recruit Petra, who arrived asking where to sign.");
}

// ---------------------------------------------------------------------------
// persistence
// ---------------------------------------------------------------------------

TEST_CASE("Faction history - the record round-trips as resolved prose", "[game][guerrilla][legends][history][save]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "faction-history-roundtrip.bin";

    const HistoryInputs in = MakeInputs(kPairs[2], "Sinai", 987654321);
    const HistoryRecord written = GenerateHistory(in);
    REQUIRE(written.Present());
    {
        HistoryRecord copy = written;
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(copy.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }

    HistoryRecord loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(loaded.Present());
    CHECK(loaded.version == written.version);
    CHECK(loaded.seed == written.seed);
    CHECK(Str(loaded.openingPage1) == Str(written.openingPage1));
    CHECK(Str(loaded.openingPage2) == Str(written.openingPage2));
    for (int k = 0; k < kHistoryEvents; k++)
    {
        CHECK(loaded.eventIndex[k] == written.eventIndex[k]);
        CHECK(Str(loaded.eventTitle[k]) == Str(written.eventTitle[k]));
        CHECK(Str(loaded.eventText[k]) == Str(written.eventText[k]));
        CHECK(Str(loaded.eventPlace[k]) == Str(written.eventPlace[k]));
    }
    for (int k = 0; k < kHistoryOpenSlots; k++)
    {
        CHECK(loaded.openingIndex[k] == written.openingIndex[k]);
    }
    for (int k = 0; k < kHistoryPhrases; k++)
    {
        CHECK(loaded.phraseIndex[k] == written.phraseIndex[k]);
    }
    // a biography generated from the reloaded record is the same prose: nothing
    // is rerolled by a save/load cycle
    CHECK(Str(GenerateBio(loaded, 2, RString("Petra"), RString("PLO East"), false, 77)) ==
          Str(GenerateBio(written, 2, RString("Petra"), RString("PLO East"), false, 77)));

    std::filesystem::remove(archivePath);
}

TEST_CASE("Faction history - an archive with no history keys loads as absent",
          "[game][guerrilla][legends][history][save]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "faction-history-empty.bin";
    {
        // an archive that carries something else entirely
        ParamArchiveSave ar(WorldSerializeVersion);
        int unrelated = 7;
        REQUIRE(ar.Serialize("unrelated", unrelated, 1, 0) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }
    HistoryRecord loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    CHECK(loaded.version == 0);
    CHECK_FALSE(loaded.Present());
    CHECK(Str(loaded.openingPage1).empty());
    std::filesystem::remove(archivePath);
}
