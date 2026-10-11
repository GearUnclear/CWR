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
    // no gendered third-person pronoun: singular they can refer to a
    // companion whose script name and body class disagree
    const bool pronoun = ContainsWord(text, "he") || ContainsWord(text, "she") || ContainsWord(text, "his") ||
                         ContainsWord(text, "her") || ContainsWord(text, "him") || ContainsWord(text, "hers");
    CHECK_FALSE(pronoun);
    CHECK_FALSE(HasAntithesis(text));
}

// A bare faction modifier takes no article or possessive of its own.
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

// A faction slot renders as a group of people ("Soviet Army troops", "FIA
// units"), so a LOCATIVE preposition in front of it turns those people into a
// place: "In FIA units the delay ..." is not English. Prepositions that take
// people (to, for, with, from, under, against, among, beside) stay allowed;
// only the place-words are rejected. This lint reads the templates before
// substitution; CheckFactionInsertion reads the rendered text for articles.
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

// Faction names modify plural people/formation nouns. Checking the complete
// rendered group covers both acronyms and multiword names without forcing an
// artificial introducer such as "the standard now raised as".
void CheckFactionGroup(const std::string& text, const std::string& faction)
{
    const std::string low = Lower(text);
    const std::string name = Lower(faction);
    for (size_t at = low.find(name); at != std::string::npos; at = low.find(name, at + 1))
    {
        const size_t end = at + name.size();
        if ((at > 0 && IsWordChar(low[at - 1])) || (end < low.size() && IsWordChar(low[end])))
        {
            continue;
        }
        // PLO is also the first token of PLO East. Check the longer name on
        // its own pass rather than treating East as a group noun.
        bool longerName = false;
        for (const char* shipped : kShippedNames)
        {
            const std::string candidate = Lower(shipped);
            if (candidate.size() > name.size() && candidate.compare(0, name.size(), name) == 0 &&
                low.compare(at, candidate.size(), candidate) == 0)
            {
                longerName = true;
            }
        }
        if (longerName)
        {
            continue;
        }
        REQUIRE(end < low.size());
        REQUIRE(low[end] == ' ');
        size_t stop = end + 1;
        while (stop < low.size() && IsWordChar(low[stop]))
        {
            stop++;
        }
        const std::string noun = low.substr(end + 1, stop - end - 1);
        INFO(text << " / " << faction << " / " << noun);
        CHECK((noun == "troops" || noun == "units" || noun == "soldiers" || noun == "forces" || noun == "fighters"));
    }
}

// Count complete labels rather than common nouns used elsewhere in the story.
std::string FactionLabel(int index, const char* faction)
{
    return std::string(faction) + " " + HistoryFactionPhrase(index);
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

// Every sentence starts with a capital and the block ends on sentence punctuation.
// is what catches a lower-case construction dropped into a sentence-initial
// slot, which is exactly the mistake {Occupier} instead of {OccupierCap} makes.
void CheckSentenceShape(const std::string& text)
{
    INFO(text);
    REQUIRE_FALSE(text.empty());
    size_t last = text.size();
    while (last > 0 && text[last - 1] == '"')
    {
        last--;
    }
    REQUIRE(last > 0);
    CHECK((text[last - 1] == '.' || text[last - 1] == '?' || text[last - 1] == '!'));
    size_t begin = 0;
    while (begin < text.size())
    {
        while (begin < text.size() && (text[begin] == ' ' || text[begin] == '"'))
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
        const size_t stop = text.find_first_of(".!?", begin);
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

TEST_CASE("Faction history - faction group labels read naturally for every shipped name",
          "[game][guerrilla][legends][history]")
{
    REQUIRE(HistoryFactionPhraseCount() == 5);
    for (int i = 0; i < HistoryFactionPhraseCount(); i++)
    {
        for (int n = 0; n < kShippedNameCount; n++)
        {
            const std::string label = FactionLabel(i, kShippedNames[n]);
            CheckFactionGroup(label, kShippedNames[n]);
            CheckFactionInsertion(label, kShippedNames[n]);
        }
    }
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
                    // Names are modifiers in complete plural group labels.
                    CheckFactionGroup(text, pair.occupier);
                    CheckFactionGroup(text, pair.resistance);
                    CheckFactionInsertion(text, pair.occupier);
                    CheckFactionInsertion(text, pair.resistance);
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

TEST_CASE("Faction history - every event variant fits wide place and faction names",
          "[game][guerrilla][legends][history]")
{
    HistoryInputs in;
    in.resistanceName = "Egyptian Frontier Force";
    in.occupierName = "Israel Defense Forces";
    in.islandName = "";
    in.seed = 3;
    in.features.Add(MakePlace("The High Pass", false));
    in.settlements.Add(MakePlace("Ras Nasrani Point", true));
    in.settlements.Add(MakePlace("El Tor Junction", true));
    for (int variant = 0; variant < HistoryVariantCount(); variant++)
    {
        HistoryForcedIndices forced;
        for (int event = 0; event < kHistoryEvents; event++)
        {
            forced.events[event] = variant;
        }
        const HistoryRecord rec = GenerateHistoryForced(in, forced);
        for (int event = 0; event < kHistoryEvents; event++)
        {
            INFO("variant " << variant << " event " << event << ": " << Str(rec.eventTitle[event]));
            CHECK(Words(rec.eventTitle[event]) <= 6);
            CHECK(Words(rec.eventText[event]) >= kHistoryEventMinWords);
            CHECK(Words(rec.eventText[event]) <= kHistoryEventMaxWords);
            CheckSharedLints(Str(rec.eventTitle[event]), "wide event title");
            CheckSharedLints(Str(rec.eventText[event]), "wide event text");
            CheckRenderedProse(Str(rec.eventText[event]));
        }
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
                    const int uses = CountSubstring(whole, FactionLabel(i, kPairs[p].occupier)) +
                                     CountSubstring(whole, FactionLabel(i, kPairs[p].resistance));
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
                        // Earlier violence belongs in a backstory; live campaign
                        // kill totals remain in the character's recorded deeds.
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
                    // Callers without a page budget must still respect the
                    // public biography limit.
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

TEST_CASE("Faction history - every biography variant fits beside its portrait", "[game][guerrilla][legends][history]")
{
    const HistoryRecord rec = GenerateHistory(MakeInputs(kPairs[0], "Malden", 7));
    const RString names[] = {RString("Petra"),
                             RString("Heartless Jean-Baptiste \"The Fence-Builder\" Vakalalabure the Collaborator")};
    const char* places[] = {"Larche", "Ras Nasrani Point", "the old crossing"};
    for (int event = 0; event < kHistoryEvents; event++)
    {
        for (int hostile = 0; hostile <= 1; hostile++)
        {
            for (int variant = 0; variant < HistoryVariantCount(); variant++)
            {
                for (const RString& name : names)
                {
                    for (const char* place : places)
                    {
                        HistoryRecord placed = rec;
                        placed.eventPlace[event] = place;
                        const RString bio =
                            GenerateBioForced(placed, event, name, RString("FIA"), hostile != 0, 1, variant);
                        INFO("event " << event << " hostile " << hostile << " variant " << variant << ": " << Str(bio));
                        CHECK(Words(bio) >= kHistoryBioMinWords);
                        CHECK(Words(bio) <= kHistoryBioPageWords);
                        CHECK(CountSubstring(Str(bio), Str(name)) == 1);
                        CHECK(CountSubstring(Str(bio), place) == 1);
                        CheckSharedLints(Str(bio), "forced bio");
                        CheckSentenceShape(Str(bio));
                        CheckRenderedProse(Str(bio));
                    }
                }
            }
        }
    }
}

TEST_CASE("Faction history - a biography never repeats a faction label the history spent",
          "[game][guerrilla][legends][history]")
{
    // No biography names a faction yet. One that does takes the label the
    // history left unused (BioPhraseIndex), so a dossier never repeats a label
    // the Chronicles already printed. Every cell and variant is rendered, so the
    // guard runs on the shipped output rather than on one sample.
    for (int p = 0; p < kPairCount; p++)
    {
        const HistoryRecord rec = GenerateHistory(MakeInputs(kPairs[p], "Malden", 3));
        const int spare = rec.BioPhraseIndex();
        REQUIRE(spare >= 0);
        REQUIRE(spare < HistoryFactionPhraseCount());
        for (int i = 0; i < kHistoryPhrases; i++)
        {
            CHECK(rec.phraseIndex[i] != spare);
        }
        const std::string whole = Str(rec.openingPage1) + " " + Str(rec.openingPage2) + " " + Str(rec.eventText[1]) +
                                  " " + Str(rec.eventText[2]);
        CHECK(CountSubstring(whole, FactionLabel(spare, kPairs[p].occupier)) == 0);
        CHECK(CountSubstring(whole, FactionLabel(spare, kPairs[p].resistance)) == 0);
        for (int event = 0; event < kHistoryEvents; event++)
        {
            for (int hostile = 0; hostile <= 1; hostile++)
            {
                const char* faction = hostile ? kPairs[p].occupier : kPairs[p].resistance;
                for (int variant = 0; variant < HistoryVariantCount(); variant++)
                {
                    const std::string bio = Str(
                        GenerateBioForced(rec, event, RString("Petra"), RString(faction), hostile != 0, 1, variant));
                    INFO(faction << " event " << event << " variant " << variant << ": " << bio);
                    REQUIRE(ContainsWord(bio, "Petra"));
                    for (int i = 0; i < kHistoryPhrases; i++)
                    {
                        CHECK(CountSubstring(bio, FactionLabel(rec.phraseIndex[i], faction)) == 0);
                    }
                }
            }
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
        CHECK(ContainsWord(Str(rec.openingPage1), "Lebanon"));
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
            CHECK(Str(rec.openingPage1).find("this country") != std::string::npos);
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
          "The people of Malden still tell how families near Larche lost rights their grandparents had "
          "expected to leave to the children. Soviet Army troops arrived with orders to enforce obedience. A "
          "complaint could bring armed visitors to the same door where neighbours once gathered.");
    CHECK(Str(rec.openingPage2) ==
          "People near Houdan accepted a settlement they could live with. When the authorities broke it, even "
          "patient neighbours talked of fighting. Fighting followed. FIA soldiers remember the dead through "
          "surviving friends, who can still describe their laughter as readily as their deaths.");
    CHECK(Str(rec.eventTitle[0]) == "Terrace Claims near Larche");
    CHECK(Str(rec.eventText[0]) ==
          "Families near Larche built terraces on the slopes and farmed them for generations. Surveyors later "
          "divided the land into plots and registered it under outside owners. The families who had built and "
          "maintained the terraces were left without a recognised claim to the ground that fed them.");
    CHECK(Str(rec.eventTitle[1]) == "Closed Roads near Houdan");
    CHECK(Str(rec.eventText[1]) ==
          "Representatives near Houdan agreed that the high ground would remain common and the roads open. "
          "Within a season, Soviet Army units fenced the ground and gated the roads. Families who had accepted "
          "the settlement now needed permission to reach land and routes the agreement had left open.");
    CHECK(Str(rec.eventTitle[2]) == "Road Battle near Chapoi");
    CHECK(Str(rec.eventText[2]) ==
          "Defenders near Chapoi stopped an advancing column for two days without relief. The column broke "
          "through on the third day, leaving local fighters dead and others missing. Survivors later gave FIA "
          "fighters an account of the positions held and the names of those who failed to return.");

    const RString bio = GenerateBioForced(rec, 0, RString("Petra"), RString("FIA"), false, 0, 0);
    CHECK(Str(bio) == "Petra worked as a farmhand near Larche until the landlord evicted the family from its rented "
                      "fields. The farmhand kept writing poems and still hoped to publish a collection.");
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
          "Near Ghajar, families in Lebanon inherited an old dispute along with their grandparents' "
          "belongings. Outsiders had claimed what kept those families fed. Hizballah units controlled the main "
          "roads. People learned which guards would listen, which would shout, and when to turn around without "
          "arguing.");
    CHECK(Str(rec.openingPage2) ==
          "Representatives near Tyre came home with an agreement and promises. When the authorities broke it, "
          "neighbours demanded to know what happened. IDF fighters lost people in the fighting that followed. "
          "Some families still ask returning fighters for news nobody wants to give.");
    CHECK(Str(rec.eventTitle[1]) == "Pump Restrictions near Tyre");
    CHECK(Str(rec.eventText[1]) ==
          "Witnesses from six villages signed an agreement near Tyre to share water and keep the coast road "
          "open. The terms also barred armed guards from the pumps. Hizballah troops seized the pumps and "
          "began checking road traffic twice a day, restricting the access the witnesses had secured.");
    CHECK(Str(rec.eventTitle[2]) == "The Fires near Saida");
    CHECK(Str(rec.eventText[2]) ==
          "Fighting near Saida set homes alight, and the fires continued through the night. Families who "
          "returned found collapsed houses and neighbours missing. IDF soldiers collected names from survivors "
          "and recorded where each person had last been seen. Relatives used the list to search for people "
          "carried away during the evacuation.");

    const RString bio = GenerateBioForced(rec, 1, RString("Petra"), RString("IDF"), false, 0, 1);
    CHECK(Str(bio) ==
          "Former postal worker Petra from Tyre became a resistance courier, then quit after police seized the "
          "courier's address book, found a friend's home address inside, and arrested the friend there.");
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
          "For generations, families near Nuweiba in Sinai taught children where they belonged. Then outsiders "
          "arrived with papers saying the children had no claim. Under IDF forces, residents watched their "
          "words in public. Families argued behind closed doors about whether silence was keeping anyone safe "
          "anymore.");
    CHECK(Str(rec.openingPage2) ==
          "Near El Tor, both sides agreed to a settlement. After the authorities broke it, residents brought "
          "copies to confront the guards. Later fighting left empty places at family tables. PLO East troops "
          "keep the names, including those neighbours still find difficult to discuss.");
    CHECK(Str(rec.eventTitle[0]) == "Water Rights near Nuweiba");
    CHECK(Str(rec.eventTitle[2]) == "Delaying Action near Ras Nasrani");
    CHECK(Str(rec.eventText[2]) ==
          "Defenders with two machine guns held an advancing column near Ras Nasrani until dusk. The delay "
          "allowed families to leave the coastal route, but several defenders died before the survivors "
          "withdrew. The coast fell the next day. PLO East units preserved the survivors' account of the "
          "action and the losses.");

    const RString bio = GenerateBioForced(rec, 2, RString("Petra"), RString("PLO East"), false, 0, 2);
    CHECK(Str(bio) == "An orchard worker from Ras Nasrani, Petra led relatives away from fighting along familiar farm "
                      "roads. Among displaced families, the sociable picker entertained new friends with stories from "
                      "years on the harvest crews.");
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

TEST_CASE("Faction history - a version 1 record loads verbatim under the current generator",
          "[game][guerrilla][legends][history][save]")
{
    // A save from before the version 2 prose carries a version 1 record. The
    // bump must not reroll it: the record keeps its version and its prose, and a
    // companion recruited after the load hangs a new biography on its places.
    REQUIRE(kHistoryVersion > 1);
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "faction-history-v1.bin";

    HistoryRecord written = GenerateHistory(MakeInputs(kPairs[0], "Malden", 5));
    written.version = 1;
    // version 1 prose, as an old save stores it
    written.openingPage1 = "Every power that has held this ground began by writing it down, and the ranks now called "
                           "Soviet Army began the same way.";
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
    CHECK(loaded.version == 1);
    CHECK(Str(loaded.openingPage1) == Str(written.openingPage1));
    CHECK(Str(loaded.openingPage2) == Str(written.openingPage2));
    for (int k = 0; k < kHistoryEvents; k++)
    {
        CHECK(Str(loaded.eventTitle[k]) == Str(written.eventTitle[k]));
        CHECK(Str(loaded.eventText[k]) == Str(written.eventText[k]));
        CHECK(Str(loaded.eventPlace[k]) == Str(written.eventPlace[k]));
    }
    const RString bio = GenerateBio(loaded, 1, RString("Petra"), RString("FIA"), false, HashKey("comp_3_petra", 5),
                                    kHistoryBioPageWords);
    INFO(Str(bio));
    CHECK(Words(bio) >= kHistoryBioMinWords);
    CHECK(Words(bio) <= kHistoryBioPageWords);
    CHECK(Str(bio).find(Str(loaded.eventPlace[1])) != std::string::npos);

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
