// Guerrilla journal, Compose stage: content limits, navigation graph, alias
// routing, five-per-page pagination and the PR #61 content pins, all asserted
// over the composed JournalDocument (no CHTMLContainer: Compose is pure and
// so is this suite).  Render's page budget, footer and slot binding are
// test_journal_render.cpp's; the end-to-end build through the parser-only
// container is test_journal.cpp's.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/Journal.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp>
#include <Poseidon/UI/Guerrilla/JournalCompose.hpp>
#include <Poseidon/UI/Guerrilla/JournalManual.hpp>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

std::string S(const RString& s)
{
    return std::string((const char*)s);
}

// local mirrors of JournalText::WordCount / ZoneAnchor: the suite compiles
// against the Compose, Manual, page-inputs and Journal headers only, and the
// two rules are small enough to restate (whitespace-separated tokens;
// "GM_ZONE_<i>")
int WordCount(const RString& s)
{
    int count = 0;
    bool inWord = false;
    for (const char* p = (const char*)s; p && *p; p++)
    {
        const bool space = *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r';
        if (!space && !inWord)
        {
            count++;
        }
        inWord = !space;
    }
    return count;
}

RString ZoneAnchor(int i)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "GM_ZONE_%d", i);
    return RString(buf);
}

// ---------------------------------------------------------------------------
// fixtures (SampleInputs mirrors test_journal.cpp so the two suites pin the
// same campaign; the overflow fixtures push every dynamic list past five)
// ---------------------------------------------------------------------------

JournalZoneRow Zone(const char* name, const char* type, int holder, float support, float capture, float heat,
                    int garrison, int alert, float distance, const char* bearing, bool revealed = true)
{
    JournalZoneRow z;
    z.name = name;
    z.type = type;
    z.holder = holder;
    z.revealed = revealed;
    z.support = support;
    z.capture = capture;
    z.heat = heat;
    z.garrison = garrison;
    z.alert = alert;
    z.distance = distance;
    z.bearing = bearing;
    z.seenDay = revealed ? 3 : 0;
    z.seenMinute = 14 * 60 + 2;
    return z;
}

JournalRosterRow Fighter(const char* name, int rank, const char* role, const char* primary, const char* launcher = "",
                         int wounded = 0)
{
    JournalRosterRow r;
    r.name = name;
    r.rank = rank;
    r.role = role;
    r.primary = primary;
    r.launcher = launcher;
    r.wounded = wounded;
    return r;
}

JournalPageInputs SampleInputs()
{
    JournalPageInputs in;
    in.islandName = "Malden";
    in.resistanceName = "FIA";
    in.occupierName = "Soviet Army";
    in.day = 3;
    in.minuteOfDay = 14 * 60 + 20;
    in.resources = 480;
    in.manpower = 11;
    in.manpowerCap = 12;
    in.incomeR = 35;
    in.incomeHR = 1;
    in.incomeKnown = true;
    in.econTickSeconds = 600;
    in.warLevel = 2;
    in.economyKnown = true;
    in.occupierTierThresholds.Add(3);
    in.occupierTierThresholds.Add(5);
    in.militaryTotal = 3;
    in.militaryHeld = 1;
    in.townsTotal = 5;
    in.townsRisen = 1;
    in.undercoverArmed = true;
    in.undercoverStatus = 2;
    in.undercoverWitnesses = 1;
    in.stashCount = 1;
    in.hqEstablished = true;
    in.hqZone = "Houdan";
    in.hqIndoors = true;
    in.garageCount = 2;
    in.garage.Add(RString("UAZ"));
    in.garage.Add(RString("Ural"));
    in.marketActive = true;
    in.weaponDealerTowns.Add(RString("Houdan"));
    in.weaponDealerTowns.Add(RString("La Trinite"));
    in.vehicleDealerTowns.Add(RString("Chapoi"));
    in.zones.Add(Zone("Camp", "CAMP", 0, 0, 0, 5, 0, 0, 400, "SW"));
    in.zones.Add(Zone("Houdan", "CITY", 0, 82, 0, 12, 0, 0, 900, "E"));
    in.zones.Add(Zone("Outpost", "OUTPOST", 0, 0, 0, 55, 0, 0, 1300, "N"));
    in.zones.Add(Zone("Airfield", "AIRFIELD", 1, 0, 40, 48, 9, 2, 2400, "NE"));
    in.zones.Add(Zone("Chapoi", "CITY", 2, 66, 0, 8, 2, 0, 3100, "S"));
    in.zones.Add(Zone("Vigny", "CITY", 2, 12, 0, 0, 0, 0, 5200, "S"));
    in.zones.Add(Zone("Seaport", "SEAPORT", 1, 0, 0, 10, 6, 1, 2100, "NE"));
    in.zones.Add(Zone("La Trinite", "CITY", 1, 31, 0, 20, 4, 0, 4000, "SE"));
    in.zones.Add(Zone("Le Port", "CITY", 2, 0, 0, 0, 0, 0, 6300, "NW", false));
    JournalRosterRow you = Fighter("You", 2, "Leader", "AK-74");
    you.isPlayer = true;
    in.roster.Add(you);
    in.roster.Add(Fighter("Petra", 1, "Rifleman", "PK"));
    in.roster.Add(Fighter("Andre", 0, "Rifleman", "AK-74", "RPG-75", 40));
    JournalRosterRow squad = Fighter("Outpost squad", 0, "Rifleman", "AK-74");
    squad.withPlayer = false;
    squad.zone = "Outpost";
    squad.count = 3;
    in.roster.Add(squad);
    return in;
}

// the three-entry journal of test_journal.cpp: two objectives, three status lines
void FillSampleJournal(Journal& journal)
{
    journal.AddEntry("Day 1 08:00", "Reached the Camp alone.", "Camp", JKPlain);
    journal.AddEntry("Day 1 16:22", "Liberated. Garrison gone; holding squad in place.", "Outpost", JKGood);
    journal.AddEntry("Day 3 14:02", "Went RED. QRF moving on last known position.", "Airfield", JKDanger);
    journal.SetObjective("firstRecruit", "Take a first recruit at the Camp (1 HR)", JODone);
    journal.SetObjective("hqEstablish", "Set up a headquarters in a town or at the Camp", JOActive);
    journal.SetStatus("Companions", "Petra (CORPORAL)");
    journal.SetStatus("Standard issue", "AK-74, PK");
    journal.SetStatus("Next pattern", "AKS-74U - 3 of 25 captured");
}

// 13 Airfield lines plus one elsewhere: 14 entries, past the zone record's
// cap and past two record pages
void FillOverflowJournal(Journal& journal)
{
    for (int i = 1; i <= 13; i++)
    {
        char stamp[32];
        char text[32];
        snprintf(stamp, sizeof(stamp), "Day 1 %02d:00", i);
        snprintf(text, sizeof(text), "Airfield line %d.", i);
        journal.AddEntry(stamp, text, "Airfield", JKPlain);
    }
    journal.AddEntry("Day 1 20:00", "Elsewhere.", "Camp", JKPlain);
}

// seven fighters: six with the player, one holding squad
JournalPageInputs SevenFighters()
{
    JournalPageInputs in = SampleInputs();
    in.roster.Clear();
    JournalRosterRow you = Fighter("You", 2, "Leader", "AK-74");
    you.isPlayer = true;
    in.roster.Add(you);
    in.roster.Add(Fighter("Petra", 1, "Rifleman", "PK"));
    in.roster.Add(Fighter("Andre", 0, "Rifleman", "AK-74", "RPG-75", 40));
    in.roster.Add(Fighter("Marek", 0, "Rifleman", "AK-74"));
    in.roster.Add(Fighter("Jana", 0, "Medic", "AK-74"));
    in.roster.Add(Fighter("Tomas", 0, "Grenadier", "AK-74 GL"));
    JournalRosterRow squad = Fighter("Outpost squad", 0, "Rifleman", "AK-74");
    squad.withPlayer = false;
    squad.zone = "Outpost";
    squad.count = 3;
    in.roster.Add(squad);
    return in;
}

// long names everywhere a name can go: a five-slot fighter name, long
// faction names, long place names, and a script objective past the hand cap
JournalPageInputs LongNameInputs()
{
    JournalPageInputs in = SampleInputs();
    in.resistanceName = "Popular Front for the Liberation of the Sinai Peninsula";
    in.occupierName = "Frontier Corps of the Third Field Army";
    in.zones[2].name = "Ras Nasrani Outpost";
    in.zones[4].name = "Marjayoun Barracks";
    in.roster[1].name = "Sgt. Petra \"The Wall\" Novakova of Houdan";
    in.roster[3].zone = "Ras Nasrani Outpost";
    return in;
}

// ... plus a script note past the hand cap, tagged with the Airfield (zone
// 3 of SampleInputs) so the record (with zone) and the zone's own record
// (without) both carry it
void FillLongJournal(Journal& journal)
{
    FillSampleJournal(journal);
    journal.SetObjective("long",
                         "Carry the word to every village between the coast and the ridge so that no family "
                         "there is left wondering which side of the road the war is on tonight",
                         JOActive);
    journal.AddEntry("Day 3 15:10",
                     "Walked the whole perimeter fence twice before dawn and counted every truck, every tent and "
                     "every sentry post the garrison has put up since the last time we looked in on them",
                     "Airfield", JKPlain);
}

// the quiet campaign of test_journal.cpp: no alert, clean cover, empty journal
JournalPageInputs QuietInputs()
{
    JournalPageInputs in = SampleInputs();
    for (int i = 0; i < in.zones.Size(); i++)
    {
        in.zones[i].alert = 0;
    }
    in.undercoverStatus = 0;
    return in;
}

// ---------------------------------------------------------------------------
// document readers
// ---------------------------------------------------------------------------

const JournalPage& Page(const JournalDocument& doc, const char* name)
{
    const int i = doc.FindPage(name);
    REQUIRE(i >= 0);
    return doc.pages[i];
}

// every run's text joined with one space, runs of spaces collapsed (mirrors
// UITestEngine::GetHtmlText plus the test_journal.cpp Text() helper, so the
// two suites pin the same strings)
std::string Joined(const JournalPage& page)
{
    std::string raw;
    for (int b = 0; b < page.blocks.Size(); b++)
    {
        const JournalBlock& block = page.blocks[b];
        for (int r = 0; r < block.runs.Size(); r++)
        {
            const std::string text = S(block.runs[r].text);
            if (text.empty())
            {
                continue;
            }
            if (!raw.empty())
            {
                raw += ' ';
            }
            raw += text;
        }
    }
    std::string out;
    for (char c : raw)
    {
        if (c == ' ' && !out.empty() && out.back() == ' ')
        {
            continue;
        }
        out += c;
    }
    return out;
}

// every part of a chain, in order
std::string JoinedChain(const JournalDocument& doc, const char* base)
{
    std::string out;
    for (int i = 0; i < doc.pages.Size(); i++)
    {
        if (S(doc.pages[i].baseName) == base)
        {
            if (!out.empty())
            {
                out += ' ';
            }
            out += Joined(doc.pages[i]);
        }
    }
    return out;
}

// true when `text` carries `a` before `b`
bool Before(const std::string& text, const char* a, const char* b)
{
    const size_t pa = text.find(a);
    const size_t pb = text.find(b);
    return pa != std::string::npos && pb != std::string::npos && pa < pb;
}

bool Has(const std::string& text, const char* needle)
{
    return text.find(needle) != std::string::npos;
}

std::vector<std::string> Hrefs(const JournalPage& page)
{
    std::vector<std::string> out;
    for (int b = 0; b < page.blocks.Size(); b++)
    {
        for (int r = 0; r < page.blocks[b].runs.Size(); r++)
        {
            if (page.blocks[b].runs[r].href.GetLength() > 0)
            {
                out.push_back(S(page.blocks[b].runs[r].href));
            }
        }
    }
    return out;
}

bool HasHref(const JournalPage& page, const char* href)
{
    for (const std::string& h : Hrefs(page))
    {
        if (h == href)
        {
            return true;
        }
    }
    return false;
}

// the first run whose text contains `needle`; null when none
const JournalRun* FindRun(const JournalPage& page, const char* needle)
{
    for (int b = 0; b < page.blocks.Size(); b++)
    {
        for (int r = 0; r < page.blocks[b].runs.Size(); r++)
        {
            if (Has(S(page.blocks[b].runs[r].text), needle))
            {
                return &page.blocks[b].runs[r];
            }
        }
    }
    return nullptr;
}

bool HasHand(const JournalBlock& block)
{
    for (int r = 0; r < block.runs.Size(); r++)
    {
        if (block.runs[r].voice == VoiceHand)
        {
            return true;
        }
    }
    return false;
}

// a Pen::Note block: typed runs only, the pencil "Label: " head first
bool IsNote(const JournalBlock& block)
{
    if (block.kind != BlockText || block.runs.Size() < 2)
    {
        return false;
    }
    for (int r = 0; r < block.runs.Size(); r++)
    {
        if (block.runs[r].voice != VoiceType)
        {
            return false;
        }
    }
    const std::string head = S(block.runs[0].text);
    return block.runs[0].ink == InkPencil && head.size() >= 2 && head.compare(head.size() - 2, 2, ": ") == 0;
}

// a Pen::LinkRow block: a link run then its serif one-liner
bool IsLinkRow(const JournalBlock& block)
{
    return block.kind == BlockText && block.runs.Size() == 2 && block.runs[0].href.GetLength() > 0 &&
           block.runs[1].voice == VoiceSerif;
}

// a roster name row: name, a space, the pencil rank or count
bool IsNameRow(const JournalBlock& block)
{
    return block.kind == BlockText && block.runs.Size() == 3 && S(block.runs[1].text) == " " &&
           block.runs[2].ink == InkPencil && block.runs[0].voice == VoiceType && block.runs[0].href.GetLength() == 0;
}

int Count(const JournalPage& page, bool (*pred)(const JournalBlock&))
{
    int n = 0;
    for (int b = 0; b < page.blocks.Size(); b++)
    {
        if (pred(page.blocks[b]))
        {
            n++;
        }
    }
    return n;
}

bool IsAnnotationHand(const JournalBlock& block)
{
    return block.annotation && HasHand(block);
}

bool IsNarrativeHand(const JournalBlock& block)
{
    return !block.annotation && HasHand(block);
}

bool IsZoneRow(const JournalBlock& block)
{
    for (int r = 0; r < block.runs.Size(); r++)
    {
        if (S(block.runs[r].href).rfind("#GM_ZONE_", 0) == 0)
        {
            return true;
        }
    }
    return false;
}

// index of the first block past the page furniture: gaps, the title and the
// serif subtitle (part 1 opens Title + Subtitle, a continuation part opens
// with the "<Title>, continued." subtitle alone)
int FirstBodyBlock(const JournalPage& page)
{
    for (int b = 0; b < page.blocks.Size(); b++)
    {
        const JournalBlock& block = page.blocks[b];
        if (block.kind == BlockGap)
        {
            continue;
        }
        if (block.runs.Size() == 1 && (block.runs[0].voice == VoiceSerif || block.runs[0].voice == VoiceTitle))
        {
            continue;
        }
        return b;
    }
    return -1;
}

bool StartsWithHead(const JournalPage& page)
{
    const int b = FirstBodyBlock(page);
    return b >= 0 && page.blocks[b].runs.Size() == 1 && page.blocks[b].runs[0].voice == VoiceHead;
}

// a continuation part opens with the pencil "<Title>, continued." subtitle
bool HasContinuedSubtitle(const JournalPage& page)
{
    if (page.blocks.Size() == 0 || page.blocks[0].runs.Size() != 1)
    {
        return false;
    }
    const JournalRun& run = page.blocks[0].runs[0];
    return run.voice == VoiceSerif && run.ink == InkPencil && S(run.text) == S(page.title) + ", continued.";
}

// the limits every composed document must respect (design D1 Compose limits)
void CheckLimits(const JournalDocument& doc)
{
    for (int p = 0; p < doc.pages.Size(); p++)
    {
        const JournalPage& page = doc.pages[p];
        INFO("page " << S(page.name));
        int narrativeHands = 0;
        for (int b = 0; b < page.blocks.Size(); b++)
        {
            const JournalBlock& block = page.blocks[b];
            if (IsNarrativeHand(block))
            {
                narrativeHands++;
            }
            for (int r = 0; r < block.runs.Size(); r++)
            {
                const JournalRun& run = block.runs[r];
                if (run.voice == VoiceHand)
                {
                    INFO("hand run: " << S(run.text));
                    CHECK(WordCount(run.text) <= ComposeLimits::HandWords);
                }
            }
            if (HasHand(block))
            {
                CHECK(block.HandWords() <= ComposeLimits::HandWords);
            }
        }
        CHECK(narrativeHands <= ComposeLimits::HandBlocksPerPage);
        CHECK(S(page.name).find('/') == std::string::npos);
    }
}

// no name in a fixed-width cell, cells only where the design allows them, no
// em dash anywhere (D0)
void CheckNoClip(const JournalDocument& doc, const JournalPageInputs& in)
{
    std::vector<std::string> names;
    for (int i = 0; i < in.roster.Size(); i++)
    {
        names.push_back(S(in.roster[i].name));
    }
    for (int i = 0; i < in.zones.Size(); i++)
    {
        names.push_back(S(in.zones[i].name));
    }
    names.push_back(S(in.resistanceName));
    names.push_back(S(in.occupierName));
    for (int p = 0; p < doc.pages.Size(); p++)
    {
        const JournalPage& page = doc.pages[p];
        const std::string name = S(page.name);
        const bool cellsAllowed = name == "GM_REFERENCE" || name.rfind("GM_MAN_", 0) == 0;
        for (int b = 0; b < page.blocks.Size(); b++)
        {
            for (int r = 0; r < page.blocks[b].runs.Size(); r++)
            {
                const JournalRun& run = page.blocks[b].runs[r];
                const std::string text = S(run.text);
                INFO("page " << name << " run: " << text);
                CHECK(text.find("\xE2\x80\x94") == std::string::npos);
                if (run.cellWidth > 0)
                {
                    CHECK(cellsAllowed);
                    for (const std::string& n : names)
                    {
                        CHECK(text != n);
                    }
                }
            }
        }
    }
}

} // namespace

// ===========================================================================
// 1. page set and aliases
// ===========================================================================

TEST_CASE("Journal compose - every page exists and the legacy anchors alias the new pages",
          "[game][guerrilla][journal][compose]")
{
    Journal journal;
    FillSampleJournal(journal);
    const JournalPageInputs in = SampleInputs();
    const JournalDocument doc = ComposeJournal(journal, in);

    static const char* kFixed[] = {"Main",          "GM_DISPATCH", "Plan",        "GM_OBJECTIVES", "GM_ACTIONS",
                                   "GM_SUPPLY",     "GM_FACTION",  "GM_PEOPLE",   "GM_ROSTER",     "GM_PLACES",
                                   "GM_CHRONICLES", "GM_RECORD",   "GM_REFERENCE"};
    for (const char* name : kFixed)
    {
        INFO(name);
        CHECK(doc.FindPage(name) >= 0);
    }
    for (int i = 0; i < in.zones.Size(); i++)
    {
        CHECK(doc.FindPage(ZoneAnchor(i)) >= 0);
    }
    REQUIRE(GuerrillaManualTopicCount() == 10);
    for (int i = 0; i < GuerrillaManualTopicCount(); i++)
    {
        INFO(GuerrillaManualTopicAnchor(i));
        CHECK(doc.FindPage(GuerrillaManualTopicAnchor(i)) >= 0);
    }

    // the six aliases, exactly, on page 1 of their chain and nowhere else
    struct Alias
    {
        const char* page;
        const char* alias;
    };
    static const Alias kAliases[] = {{"Main", "GM_CONTENTS"},  {"Plan", "GM_OPERATIONS"},
                                     {"GM_PEOPLE", "GM_CELL"}, {"GM_PLACES", "GM_ZONES"},
                                     {"GM_RECORD", "GM_LOG"},  {"GM_REFERENCE", "GM_MAN_INDEX"}};
    for (const Alias& a : kAliases)
    {
        const JournalPage& page = Page(doc, a.page);
        INFO(a.page);
        REQUIRE(page.aliases.Size() == 1);
        CHECK(S(page.aliases[0]) == a.alias);
        CHECK(page.part == 1);
        // an alias is never also a page name
        CHECK(doc.FindPage(a.alias) < 0);
    }
    for (int p = 0; p < doc.pages.Size(); p++)
    {
        const JournalPage& page = doc.pages[p];
        INFO("page " << S(page.name));
        bool aliased = false;
        for (const Alias& a : kAliases)
        {
            if (S(page.name) == a.page)
            {
                aliased = true;
            }
        }
        if (!aliased)
        {
            CHECK(page.aliases.Size() == 0);
        }
        if (page.part > 1)
        {
            CHECK(page.aliases.Size() == 0);
        }
        CHECK(S(page.name).find('/') == std::string::npos);
        CHECK(page.name.GetLength() > 0);
        CHECK(page.title.GetLength() > 0);
    }
}

// ===========================================================================
// 2. navigation graph
// ===========================================================================

TEST_CASE("Journal compose - parents, hubs and every href resolve inside the document",
          "[game][guerrilla][journal][compose]")
{
    Journal journal;
    FillSampleJournal(journal);
    JournalPageInputs in = SampleInputs();
    const JournalDocument doc = ComposeJournal(journal, in);

    // Contents is the root; every other page names a parent that exists
    for (int p = 0; p < doc.pages.Size(); p++)
    {
        const JournalPage& page = doc.pages[p];
        INFO("page " << S(page.name));
        if (S(page.name) == "Main")
        {
            CHECK(page.parentName.GetLength() == 0);
        }
        else
        {
            CHECK(page.parentName.GetLength() > 0);
            CHECK(page.parentTitle.GetLength() > 0);
            CHECK(doc.FindPage(page.parentName) >= 0);
        }
        // no dead anchor anywhere (this is what catches #GM_HISTORY leaking
        // into Change 1)
        for (const std::string& href : Hrefs(page))
        {
            INFO("href " << href);
            REQUIRE(href.size() > 1);
            CHECK(href[0] == '#');
            CHECK(doc.FindPage(href.c_str() + 1) >= 0);
        }
    }

    // Contents: six link rows in the fixed order, nothing else linked
    {
        const JournalPage& main = Page(doc, "Main");
        const std::vector<std::string> hrefs = Hrefs(main);
        const std::vector<std::string> want = {"#GM_DISPATCH", "#Plan",          "#GM_PEOPLE",
                                               "#GM_PLACES",   "#GM_CHRONICLES", "#GM_REFERENCE"};
        CHECK(hrefs == want);
        CHECK(Count(main, IsLinkRow) == 6);
        CHECK(Count(main, HasHand) == 0);
        CHECK(S(main.title) == "Contents");
        CHECK(doc.PartCount("Main") == 1);
    }
    // Operations hub: exactly four links, one part (UpdatePlan copies page 0)
    {
        const JournalPage& plan = Page(doc, "Plan");
        const std::vector<std::string> hrefs = Hrefs(plan);
        const std::vector<std::string> want = {"#GM_OBJECTIVES", "#GM_ACTIONS", "#GM_SUPPLY", "#GM_FACTION"};
        CHECK(hrefs == want);
        CHECK(doc.PartCount("Plan") == 1);
        CHECK(S(plan.parentName) == "Main");
        // its four pages point back at it
        CHECK(S(Page(doc, "GM_OBJECTIVES").parentName) == "Plan");
        CHECK(S(Page(doc, "GM_ACTIONS").parentName) == "Plan");
        CHECK(S(Page(doc, "GM_SUPPLY").parentName) == "Plan");
        CHECK(S(Page(doc, "GM_FACTION").parentName) == "Plan");
        CHECK(S(Page(doc, "GM_OBJECTIVES").parentTitle) == "Operations");
    }
    // People -> the roster (the only entry in Change 1)
    {
        const JournalPage& people = Page(doc, "GM_PEOPLE");
        const std::vector<std::string> hrefs = Hrefs(people);
        REQUIRE(hrefs.size() == 1);
        CHECK(hrefs[0] == "#GM_ROSTER");
        CHECK(S(Page(doc, "GM_ROSTER").parentName) == "GM_PEOPLE");
    }
    // Places -> every zone page, and back
    {
        for (int i = 0; i < in.zones.Size(); i++)
        {
            const JournalPage& zone = Page(doc, ZoneAnchor(i));
            CHECK(S(zone.parentName) == "GM_PLACES");
            CHECK(S(zone.title) == S(in.zones[i].name));
            const std::string href = "#" + S(ZoneAnchor(i));
            CHECK(Has(JoinedChain(doc, "GM_PLACES"), S(in.zones[i].name).c_str()));
            bool linked = false;
            for (int p = 0; p < doc.pages.Size(); p++)
            {
                if (S(doc.pages[p].baseName) == "GM_PLACES" && HasHref(doc.pages[p], href.c_str()))
                {
                    linked = true;
                }
            }
            CHECK(linked);
        }
    }
    // Chronicles -> the record; History only once the historians have written
    {
        const JournalPage& chron = Page(doc, "GM_CHRONICLES");
        CHECK(HasHref(chron, "#GM_RECORD"));
        CHECK_FALSE(HasHref(chron, "#GM_HISTORY"));
        CHECK(Count(chron, IsLinkRow) == 1);
        CHECK(S(Page(doc, "GM_RECORD").parentName) == "GM_CHRONICLES");

        in.history.present = true;
        const JournalDocument withHistory = ComposeJournal(journal, in);
        const JournalPage& chron2 = Page(withHistory, "GM_CHRONICLES");
        CHECK(HasHref(chron2, "#GM_RECORD"));
        CHECK(HasHref(chron2, "#GM_HISTORY"));
        CHECK(Count(chron2, IsLinkRow) == 2);
    }
    // Reference -> the ten chapters, each chapter back to Reference
    {
        const JournalPage& ref = Page(doc, "GM_REFERENCE");
        CHECK(Hrefs(ref).size() == 10);
        for (int i = 0; i < GuerrillaManualTopicCount(); i++)
        {
            const std::string href = "#" + std::string(GuerrillaManualTopicAnchor(i));
            CHECK(HasHref(ref, href.c_str()));
            const JournalPage& chapter = Page(doc, GuerrillaManualTopicAnchor(i));
            CHECK(S(chapter.parentName) == "GM_REFERENCE");
            CHECK(HasHref(chapter, "#GM_REFERENCE"));
        }
    }
}

// ===========================================================================
// 3. limits and pagination
// ===========================================================================

TEST_CASE("Journal compose - hand caps, the Dispatch shape and the five-per-page chains",
          "[game][guerrilla][journal][compose]")
{
    SECTION("every hand run is under 25 words and no page carries more than two narrative hand blocks")
    {
        Journal journal;
        FillSampleJournal(journal);
        CheckLimits(ComposeJournal(journal, SampleInputs()));

        Journal overflow;
        FillOverflowJournal(overflow);
        CheckLimits(ComposeJournal(overflow, SevenFighters()));

        Journal longJournal;
        FillLongJournal(longJournal);
        const JournalDocument longDoc = ComposeJournal(longJournal, LongNameInputs());
        CheckLimits(longDoc);
        // the over-long script objective was clamped, not dropped
        const std::string objectives = JoinedChain(longDoc, "GM_OBJECTIVES");
        CHECK(Has(objectives, "Carry the word to every village"));
        CHECK(Has(objectives, "..."));
        CHECK_FALSE(Has(objectives, "tonight"));
        // and so was the over-long entry, on the record (stamp, zone and
        // body in one block under the cap) and on the zone's own record
        {
            const std::string record = JoinedChain(longDoc, "GM_RECORD");
            CHECK(Has(record, "15:10 Airfield. Walked the whole perimeter fence"));
            CHECK_FALSE(Has(record, "looked in on them"));
            const JournalPage& airfield = Page(longDoc, "GM_ZONE_3");
            const std::string zone = Joined(airfield);
            CHECK(Has(zone, "15:10 Walked the whole perimeter fence"));
            CHECK_FALSE(Has(zone, "looked in on them"));
            const JournalRun* body = FindRun(airfield, "Walked the whole perimeter");
            REQUIRE(body);
            CHECK(body->voice == VoiceHand);
            CHECK(WordCount(body->text) <= ComposeLimits::HandWords - 1); // the stamp's word is budgeted
            const std::string text = S(body->text);
            CHECK(text.compare(text.size() - 3, 3, "...") == 0);
        }

        Journal empty;
        CheckLimits(ComposeJournal(empty, QuietInputs()));
    }

    SECTION("Dispatches is three typed notes and at most one line in the hand")
    {
        Journal journal;
        FillSampleJournal(journal);
        const JournalDocument doc = ComposeJournal(journal, SampleInputs());
        const JournalPage& dispatch = Page(doc, "GM_DISPATCH");
        CHECK(Count(dispatch, IsNote) == 3);
        CHECK(Count(dispatch, HasHand) <= 1);
        CHECK(Count(dispatch, IsNarrativeHand) <= 1);
        const std::string text = Joined(dispatch);
        CHECK(Has(text, "Threat: "));
        CHECK(Has(text, "Objective: "));
        CHECK(Has(text, "Latest: "));
        CHECK(Before(text, "Threat: ", "Objective: "));
        CHECK(Before(text, "Objective: ", "Latest: "));
        CHECK(doc.PartCount("GM_DISPATCH") == 1);
    }

    SECTION("Contents descriptions are eight words or fewer")
    {
        Journal journal;
        const JournalDocument doc = ComposeJournal(journal, SampleInputs());
        const JournalPage& main = Page(doc, "Main");
        int rows = 0;
        for (int b = 0; b < main.blocks.Size(); b++)
        {
            if (IsLinkRow(main.blocks[b]))
            {
                rows++;
                CHECK(WordCount(main.blocks[b].runs[1].text) <= ComposeLimits::ContentsDescriptionWords);
            }
        }
        CHECK(rows == 6);
    }

    SECTION("Suggested actions: seven moves paginate 5 + 2")
    {
        Journal journal;
        const JournalDocument doc = ComposeJournal(journal, SampleInputs());
        REQUIRE(doc.PartCount("GM_ACTIONS") == 2);
        const JournalPage& p1 = Page(doc, "GM_ACTIONS");
        const JournalPage& p2 = Page(doc, "GM_ACTIONS_2");
        CHECK(Count(p1, IsAnnotationHand) == 5);
        CHECK(Count(p2, IsAnnotationHand) == 2);
        CHECK(Count(p1, IsNarrativeHand) == 0);
        CHECK(HasContinuedSubtitle(p2));
        CHECK(S(p2.parentName) == "Plan");
        CHECK(S(p2.title) == S(p1.title));
        CHECK(p2.part == 2);
        CHECK(Has(Joined(p1), "7 moves."));
    }

    SECTION("Objectives: seven open paginate 5 + 2; Open, Failed, Done in that order")
    {
        Journal seven;
        for (int i = 0; i < 5; i++)
        {
            char id[8];
            char text[32];
            snprintf(id, sizeof(id), "o%d", i);
            snprintf(text, sizeof(text), "Standing order %d", i + 1);
            seven.SetObjective(id, text, JOActive);
        }
        const JournalDocument doc = ComposeJournal(seven, SampleInputs());
        REQUIRE(doc.PartCount("GM_OBJECTIVES") == 2);
        const JournalPage& p1 = Page(doc, "GM_OBJECTIVES");
        const JournalPage& p2 = Page(doc, "GM_OBJECTIVES_2");
        CHECK(Count(p1, IsAnnotationHand) == 5);
        CHECK(Count(p2, IsAnnotationHand) == 2);
        CHECK(HasContinuedSubtitle(p2));
        CHECK(Has(Joined(p1), "7 objectives open. Day 3, 14:20."));
        CHECK(Has(Joined(p1), "Open"));

        Journal mixed;
        mixed.SetObjective("a", "Hold the ridge road", JOActive);
        mixed.SetObjective("f", "Meet the courier at the mill", JOFailed);
        mixed.SetObjective("d", "Reach the Camp", JODone);
        const JournalDocument doc2 = ComposeJournal(mixed, SampleInputs());
        const JournalPage& page = Page(doc2, "GM_OBJECTIVES");
        const std::string text = Joined(page);
        CHECK(Before(text, "Open", "Failed"));
        CHECK(Before(text, "Failed", "Done"));
        CHECK(Before(text, "Hold the ridge road.", "Failed: Meet the courier at the mill."));
        CHECK(Before(text, "Failed: Meet the courier at the mill.", "Reach the Camp."));
        const JournalRun* failed = FindRun(page, "Failed: Meet");
        REQUIRE(failed);
        CHECK(failed->ink == InkRed);
        const JournalRun* done = FindRun(page, "Reach the Camp.");
        REQUIRE(done);
        CHECK(done->ink == InkFaded);
        CHECK(doc2.PartCount("GM_OBJECTIVES") == 1);
    }

    SECTION("Roster: seven fighters paginate 5 + 2 with the Arms block on the last part")
    {
        Journal journal;
        FillSampleJournal(journal);
        const JournalDocument doc = ComposeJournal(journal, SevenFighters());
        REQUIRE(doc.PartCount("GM_ROSTER") == 2);
        const JournalPage& p1 = Page(doc, "GM_ROSTER");
        const JournalPage& p2 = Page(doc, "GM_ROSTER_2");
        CHECK(Count(p1, IsNameRow) == 5);
        CHECK(Count(p2, IsNameRow) == 2);
        CHECK(HasContinuedSubtitle(p2));
        CHECK(S(p2.parentName) == "GM_PEOPLE");
        const std::string t1 = Joined(p1);
        const std::string t2 = Joined(p2);
        CHECK_FALSE(Has(t1, "Standard issue"));
        CHECK(Has(t2, "Arms"));
        CHECK(Has(t2, "Standard issue: AK-74, PK."));
        CHECK(Has(t2, "Next pattern: AKS-74U - 3 of 25 captured."));
        // the group label is repeated at the top of the continuation page
        CHECK(Has(t2, "With me Tomas"));
        CHECK(Has(t2, "Holding Outpost Outpost squad x3"));
        CHECK(Has(t1, "9 under arms, 1 wounded."));
    }

    SECTION("Places: nine zones paginate 5 + 4, group heads not counted")
    {
        Journal journal;
        const JournalPageInputs in = SampleInputs();
        const JournalDocument doc = ComposeJournal(journal, in);
        REQUIRE(doc.PartCount("GM_PLACES") == 2);
        const JournalPage& p1 = Page(doc, "GM_PLACES");
        const JournalPage& p2 = Page(doc, "GM_PLACES_2");
        CHECK(Count(p1, IsZoneRow) == 5);
        CHECK(Count(p2, IsZoneRow) == 4);
        CHECK(HasContinuedSubtitle(p2));
        CHECK(p2.aliases.Size() == 0);
        const std::string t1 = Joined(p1);
        CHECK(Before(t1, "Ours", "Contested"));
        CHECK(Before(t1, "Contested", "Neutral"));
        const std::string all = JoinedChain(doc, "GM_PLACES");
        CHECK(Before(all, "Neutral", "Occupied"));
        CHECK(Before(all, "Occupied", "Unscouted"));
        CHECK(Before(all, "Chapoi", "Vigny")); // nearest first within a group
        int zoneRows = 0;
        for (int p = 0; p < doc.pages.Size(); p++)
        {
            if (S(doc.pages[p].baseName) == "GM_PLACES")
            {
                zoneRows += Count(doc.pages[p], IsZoneRow);
            }
        }
        CHECK(zoneRows == in.zones.Size());
    }

    SECTION("Record: fourteen entries paginate 5 + 5 + 4 with a day head atop every part")
    {
        Journal journal;
        FillOverflowJournal(journal);
        const JournalDocument doc = ComposeJournal(journal, SampleInputs());
        REQUIRE(doc.PartCount("GM_RECORD") == 3);
        const JournalPage& p1 = Page(doc, "GM_RECORD");
        const JournalPage& p2 = Page(doc, "GM_RECORD_2");
        const JournalPage& p3 = Page(doc, "GM_RECORD_3");
        CHECK(Count(p1, IsAnnotationHand) == 5);
        CHECK(Count(p2, IsAnnotationHand) == 5);
        CHECK(Count(p3, IsAnnotationHand) == 4);
        CHECK(StartsWithHead(p1));
        CHECK(StartsWithHead(p2));
        CHECK(StartsWithHead(p3));
        CHECK(HasContinuedSubtitle(p2));
        CHECK(HasContinuedSubtitle(p3));
        CHECK(Has(Joined(p2), "Day 1"));
        CHECK(Has(Joined(p3), "Day 1"));
        CHECK(S(p3.parentName) == "GM_CHRONICLES");
        CHECK(p3.part == 3);
        CHECK(doc.FindPage("GM_RECORD_4") < 0);
    }

    SECTION("Reference is a static ten-row menu on one part")
    {
        Journal journal;
        const JournalDocument doc = ComposeJournal(journal, SampleInputs());
        const JournalPage& ref = Page(doc, "GM_REFERENCE");
        CHECK(Hrefs(ref).size() == 10);
        CHECK(doc.PartCount("GM_REFERENCE") == 1);
        CHECK(doc.FindPage("GM_REFERENCE_2") < 0);
        CHECK(Has(Joined(ref), "Reference Notes from the old hands."));
    }
}

// ===========================================================================
// 4. the zone record cap and its overflow link
// ===========================================================================

TEST_CASE("Journal compose - a zone's record keeps ten lines and points the rest at the record",
          "[game][guerrilla][journal][compose]")
{
    Journal journal;
    FillOverflowJournal(journal);
    const JournalDocument doc = ComposeJournal(journal, SampleInputs());
    const JournalPage& airfield = Page(doc, "GM_ZONE_3");
    const std::string text = Joined(airfield);
    CHECK(Has(text, "Airfield line 13."));      // newest first
    CHECK(Has(text, "Airfield line 4."));       // the tenth
    CHECK_FALSE(Has(text, "Airfield line 3.")); // past the cap
    CHECK_FALSE(Has(text, "Elsewhere."));       // another zone's line
    CHECK_FALSE(Has(text, "Nothing written about this place yet."));
    CHECK(Before(text, "Airfield line 13.", "Airfield line 4."));
    CHECK(Count(airfield, IsAnnotationHand) == ComposeLimits::ZoneRecordCap);
    CHECK(Has(text, "3 earlier lines in the record ."));

    // the overflow block, run for run
    const JournalBlock* overflow = nullptr;
    for (int b = 0; b < airfield.blocks.Size(); b++)
    {
        const JournalBlock& block = airfield.blocks[b];
        if (block.runs.Size() == 3 && S(block.runs[1].href) == "#GM_RECORD")
        {
            overflow = &block;
        }
    }
    REQUIRE(overflow);
    CHECK(S(overflow->runs[0].text) == "3 earlier lines in the ");
    CHECK(overflow->runs[0].ink == InkPencil);
    CHECK(S(overflow->runs[1].text) == "record");
    CHECK(S(overflow->runs[2].text) == ".");
    CHECK(overflow->runs[0].href.GetLength() == 0);
    CHECK(overflow->runs[2].href.GetLength() == 0);
    CHECK_FALSE(HasHref(airfield, "#GM_LOG"));

    // the whole record is still on the record pages
    const std::string record = JoinedChain(doc, "GM_RECORD");
    CHECK(Has(record, "Airfield line 1."));
    CHECK(Has(record, "Elsewhere."));
    CHECK(Has(Joined(Page(doc, "GM_RECORD")), "14 entries."));
    CHECK(Has(Joined(Page(doc, "GM_CHRONICLES")), "14 entries."));
}

// ===========================================================================
// 5. PR #61 pins carried forward
// ===========================================================================

TEST_CASE("Journal compose - the PR #61 content pins hold at the document level", "[game][guerrilla][journal][compose]")
{
    Journal journal;
    FillSampleJournal(journal);
    const JournalPageInputs in = SampleInputs();
    const JournalDocument doc = ComposeJournal(journal, in);

    SECTION("Contents and Dispatches")
    {
        const std::string main = Joined(Page(doc, "Main"));
        CHECK(Has(main, "Resistance Dossier"));
        CHECK(Has(main, "Malden. FIA against the Soviet Army. Day 3, 14:20."));
        CHECK(Has(main, "Dispatches"));
        CHECK(Has(main, "Reference"));
        CHECK_FALSE(Has(main, "SITUATION"));
        CHECK_FALSE(Has(main, "Treasury"));
        CHECK_FALSE(Has(main, "|"));

        const JournalPage& dispatch = Page(doc, "GM_DISPATCH");
        const std::string text = Joined(dispatch);
        CHECK(Has(text, "Dispatches Day 3, 14:20."));
        CHECK(Has(text, "Threat: Airfield went RED. A quick reaction force is out toward our last known position, "
                        "2.4 km NE."));
        CHECK(Has(text, "Objective: Hold every base. 1 of 3."));
        CHECK(Has(text, "Latest: 14:02 Airfield. Went RED. QRF moving on last known position."));
        CHECK(Has(text, "My cover is blown: one patrol knows my face."));
        // the old Notes paragraphs are not copied here
        CHECK_FALSE(Has(text, "We are three"));
        CHECK_FALSE(Has(text, "We hold"));
        CHECK_FALSE(Has(text, "Treasury"));
        CHECK_FALSE(Has(text, "Companions"));
        // red pen: the RED threat, the danger entry, the blown cover
        const JournalRun* threat = FindRun(dispatch, "went RED");
        REQUIRE(threat);
        CHECK(threat->ink == InkRed);
        CHECK(threat->voice == VoiceType);
        const JournalRun* cover = FindRun(dispatch, "my face");
        REQUIRE(cover);
        CHECK(cover->ink == InkRed);
        CHECK(cover->voice == VoiceHand);
        const JournalRun* latest = FindRun(dispatch, "QRF moving");
        REQUIRE(latest);
        CHECK(latest->ink == InkRed);
    }

    SECTION("the objectives-open count follows the page")
    {
        CHECK(Has(Joined(Page(doc, "GM_OBJECTIVES")), "3 objectives open. Day 3, 14:20."));
        CHECK(Has(Joined(Page(doc, "Plan")), "3 objectives open. Day 3, 14:20."));
        const std::string objectives = Joined(Page(doc, "GM_OBJECTIVES"));
        CHECK(Has(objectives, "Hold every base. 1 of 3."));
        CHECK(Has(objectives, "Raise every town. 1 of 5."));
        CHECK(Has(objectives, "Set up a headquarters in a town or at the Camp."));
        CHECK(Has(objectives, "Done Take a first recruit at the Camp (1 HR)."));

        JournalPageInputs held = SampleInputs();
        held.militaryHeld = held.militaryTotal; // every base held: that row moves to Done
        Journal empty;
        const JournalDocument d1 = ComposeJournal(empty, held);
        const std::string o1 = Joined(Page(d1, "GM_OBJECTIVES"));
        CHECK(Has(o1, "1 objective open."));
        CHECK_FALSE(Has(o1, "Hold every base"));
        CHECK(Has(o1, "Every base held."));
        CHECK(Before(o1, "Done", "Every base held."));
        CHECK(Has(o1, "Raise every town. 1 of 5."));
        CHECK(Has(Joined(Page(d1, "Plan")), "1 objective open."));
        CHECK(Has(Joined(Page(d1, "GM_DISPATCH")), "Objective: Raise every town. 1 of 5."));

        held.townsRisen = held.townsTotal; // and every town: nothing open, both done
        const JournalDocument d2 = ComposeJournal(empty, held);
        const std::string o2 = Joined(Page(d2, "GM_OBJECTIVES"));
        CHECK(Has(o2, "0 objectives open."));
        CHECK(Has(o2, "Every town risen."));
        CHECK(Has(o2, "Every base held."));
        CHECK_FALSE(Has(o2, "Open"));
        CHECK(Has(Joined(Page(d2, "GM_DISPATCH")), "Objective: Nothing open."));
    }

    SECTION("Suggested actions in priority order, a securing base is not a target, urgent moves in red")
    {
        const std::string actions = JoinedChain(doc, "GM_ACTIONS");
        CHECK(Has(actions, "Break contact. Airfield is RED, a QRF is out. 2.4 km NE."));
        CHECK(Has(actions, "Go dark. One patrol knows my face: stow the weapon, lose or drop the witnesses."));
        CHECK(Has(actions, "Outpost, heat 55, on our own ground. Reinforce or pull the squad. 1.3 km N."));
        CHECK(
            Has(actions, "Raise Chapoi. Support 66, line 60. 2 occupiers in town: clear or wait them out. 3.1 km S."));
        CHECK(Has(actions, "Finish securing Airfield. 40% secured. Fighters inside; garrison 9 still on the field. "
                           "2.4 km NE."));
        CHECK(Has(actions, "Target Seaport. Garrison 6, alert YELLOW. 2.1 km NE."));
        CHECK_FALSE(Has(actions, "Target Airfield")); // a base being secured is not a fresh target
        CHECK(Has(actions, "Recruit at the Camp. 11 HR in reserve, 1 HR a fighter."));
        CHECK(Before(actions, "Break contact", "Go dark"));
        CHECK(Before(actions, "Go dark", "Outpost, heat 55"));
        CHECK(Before(actions, "Outpost, heat 55", "Raise Chapoi"));
        CHECK(Before(actions, "Raise Chapoi", "Finish securing Airfield"));
        CHECK(Before(actions, "Finish securing Airfield", "Target Seaport"));
        CHECK(Before(actions, "Target Seaport", "Recruit at the Camp"));
        CHECK_FALSE(Has(actions, "URGENT"));
        CHECK_FALSE(Has(actions, "Sweep due"));
        const JournalPage& p1 = Page(doc, "GM_ACTIONS");
        const JournalRun* breakContact = FindRun(p1, "Break contact");
        REQUIRE(breakContact);
        CHECK(breakContact->ink == InkRed);
        CHECK(breakContact->voice == VoiceHand);
        const JournalRun* goDark = FindRun(p1, "Go dark");
        REQUIRE(goDark);
        CHECK(goDark->ink == InkRed);
        const JournalRun* raise = FindRun(p1, "Raise Chapoi");
        REQUIRE(raise);
        CHECK(raise->ink == InkHand);
    }

    SECTION("a quiet campaign reads quiet")
    {
        Journal empty;
        const JournalDocument quiet = ComposeJournal(empty, QuietInputs());
        const JournalPage& dispatch = Page(quiet, "GM_DISPATCH");
        const std::string text = Joined(dispatch);
        CHECK(Has(text, "Threat: The garrisons are quiet."));
        CHECK(Has(text, "To the occupier I am still a civilian."));
        CHECK(Has(text, "Latest: Nothing written yet."));
        CHECK_FALSE(Has(text, "went RED"));
        CHECK_FALSE(Has(text, "my face"));
        for (int b = 0; b < dispatch.blocks.Size(); b++)
        {
            for (int r = 0; r < dispatch.blocks[b].runs.Size(); r++)
            {
                CHECK(dispatch.blocks[b].runs[r].ink != InkRed);
            }
        }
        const std::string actions = JoinedChain(quiet, "GM_ACTIONS");
        CHECK_FALSE(Has(actions, "Break contact"));
        CHECK_FALSE(Has(actions, "Go dark"));
        const std::string places = JoinedChain(quiet, "GM_PLACES");
        CHECK_FALSE(Has(places, "RED"));
        CHECK_FALSE(Has(places, "YELLOW"));
    }

    SECTION("the cover line and the handbook's standing head agree on the face")
    {
        CHECK(Has(Joined(Page(doc, "GM_DISPATCH")), "my face"));
        const std::string undercover = Joined(Page(doc, "GM_MAN_UNDERCOVER"));
        CHECK(Has(undercover, "Undercover How the occupier reads you. Handbook 9 of 10."));
        CHECK(Has(undercover, "YOU ARE SEEN AS RANGE"));
        CHECK(Has(undercover, "Rifle slung SUSPECTED under 20 m, or from behind"));
        CHECK(Has(undercover, "Your standing: now BLOWN, 1 patrol knows your face"));
        CHECK(Has(undercover, "< Companions"));
        CHECK(Has(undercover, "Keeping the record >"));
        CHECK(Has(undercover, "Index"));
        const JournalPage& chapter = Page(doc, "GM_MAN_UNDERCOVER");
        CHECK(HasHref(chapter, "#GM_MAN_COMPANIONS"));
        CHECK(HasHref(chapter, "#GM_MAN_SAVE"));
        CHECK(HasHref(chapter, "#GM_REFERENCE"));
        CHECK_FALSE(HasHref(chapter, "#GM_MAN_INDEX"));
    }

    SECTION("the handbook's first chapter describes the dossier's own page map")
    {
        // the six section labels as the Contents menu itself links them
        std::vector<std::string> labels;
        const JournalPage& contents = Page(doc, "Main");
        for (int b = 0; b < contents.blocks.Size(); b++)
        {
            for (int r = 0; r < contents.blocks[b].runs.Size(); r++)
            {
                if (contents.blocks[b].runs[r].href.GetLength() > 0)
                {
                    labels.push_back(S(contents.blocks[b].runs[r].text));
                }
            }
        }
        REQUIRE(labels.size() == 6);
        const std::string mode = Joined(Page(doc, "GM_MAN_MODE"));
        for (const std::string& label : labels)
        {
            INFO("label " << label);
            CHECK(Has(mode, label.c_str()));
        }
        // and none of the retired page titles, here or in any chapter
        CHECK_FALSE(Has(mode, "ZONES"));
        CHECK_FALSE(Has(mode, "CELL"));
        CHECK_FALSE(Has(mode, "DIARY"));
        for (int t = 0; t < GuerrillaManualTopicCount(); t++)
        {
            const std::string chapter = Joined(Page(doc, GuerrillaManualTopicAnchor(t)));
            INFO("chapter " << GuerrillaManualTopicAnchor(t));
            CHECK_FALSE(Has(chapter, "the Cell page"));
            CHECK_FALSE(Has(chapter, "the Diary"));
            CHECK_FALSE(Has(chapter, "the diary"));
            CHECK_FALSE(Has(chapter, "Notes is"));
        }
    }

    SECTION("the roster in typed rows, the arms lines on the roster only")
    {
        const JournalPage& roster = Page(doc, "GM_ROSTER");
        const std::string text = Joined(roster);
        CHECK(Has(text, "The roster 6 under arms, 1 wounded."));
        CHECK(Has(text, "With me You Sgt Leader, AK-74."));
        CHECK(Has(text, "Petra Cpl Rifleman, PK."));
        CHECK(Has(text, "Andre Pvt Rifleman, AK-74 + RPG-75, WIA 40%."));
        CHECK(Has(text, "Holding Outpost Outpost squad x3 Rifleman, AK-74."));
        CHECK(Before(text, "You Sgt", "Andre Pvt"));
        CHECK(Before(text, "Andre Pvt", "Holding Outpost"));
        CHECK(Has(text, "Standard issue: AK-74, PK."));
        CHECK(Has(text, "Next pattern: AKS-74U - 3 of 25 captured."));
        CHECK(Has(text, "Cache: Houdan, at the headquarters."));
        CHECK(Has(text, "Garage: UAZ, Ural."));
        CHECK_FALSE(Has(text, "Treasury"));
        CHECK_FALSE(Has(text, "Income"));
        CHECK_FALSE(Has(text, "dealers"));
        CHECK(Count(roster, IsNameRow) == 4);
        // the player's name is bold, the wounded part is red, the name wraps
        const JournalRun* you = FindRun(roster, "You");
        REQUIRE(you);
        CHECK(you->bold);
        CHECK(you->cellWidth == 0);
        CHECK(you->href.GetLength() == 0);
        const JournalRun* andre = FindRun(roster, "Andre");
        REQUIRE(andre);
        CHECK_FALSE(andre->bold);
        const JournalRun* wia = FindRun(roster, "WIA 40%");
        REQUIRE(wia);
        CHECK(wia->ink == InkRed);
        CHECK(wia->voice == VoiceType);
        CHECK_FALSE(Has(Joined(Page(doc, "GM_SUPPLY")), "Standard issue"));
        CHECK_FALSE(Has(Joined(Page(doc, "GM_SUPPLY")), "Next pattern"));
        CHECK_FALSE(Has(Joined(Page(doc, "GM_DISPATCH")), "Standard issue"));
        CHECK(Has(Joined(Page(doc, "GM_PEOPLE")), "People 6 under arms, 1 wounded."));
        CHECK(Has(Joined(Page(doc, "GM_PEOPLE")), "The roster"));
    }

    SECTION("Supplies")
    {
        const std::string supply = Joined(Page(doc, "GM_SUPPLY"));
        CHECK(Has(supply, "Supplies Treasury 480 R, manpower 11 HR. Day 3, 14:20."));
        CHECK(Has(supply, "Treasury: 480 R."));
        CHECK(Has(supply, "Manpower: 11 HR, pool 12."));
        CHECK(Has(supply, "Income every 10 min: +35 R, +1 HR, from Houdan, Outpost."));
        CHECK(Has(supply, "Cache: Houdan, at the headquarters."));
        CHECK(Has(supply, "Garage: UAZ, Ural."));
        CHECK(Has(supply, "Arms dealers: Houdan, La Trinite (occupied)."));
        CHECK(Has(supply, "Vehicle dealers: Chapoi (rising)."));
        CHECK_FALSE(Has(supply, "Andre"));
        CHECK_FALSE(Has(supply, "With me"));
    }

    SECTION("Resistance strength")
    {
        const std::string res = Joined(Page(doc, "GM_FACTION"));
        CHECK(Has(res, "War level 2 of 10."));
        CHECK(Has(res, "Island held: 25%. Level 3 at 40%; the ladder runs 20, 40, 55, 70, 85."));
        CHECK(Has(res, "The occupier steps up at WL 3, WL 5: better troops, heavier vehicles, sharper eyes."));
        CHECK(Has(res, "Towns: 1 risen, 1 rising, 1 neutral, 1 occupied, 1 unscouted."));
        CHECK(Has(res, "Bases: 1 held, 1 contested, 1 occupied."));
        CHECK(Has(res, "Occupier under arms: 21 known. Airfield 9, Chapoi 2, Seaport 6, La Trinite 4."));
        CHECK(Has(res, "Ours under arms: 6. 3 with me, 3 holding, 11 HR in reserve."));
        CHECK(Has(res, "Heat on our ground: 24 mean over 3 zones."));
        CHECK(Has(res, "Cells: 1, ours. Headquarters at Houdan."));
        CHECK(Has(res, "Holdings: 1 cache, 2 garaged."));
        CHECK(Has(res, "No allied cells. No outside contact."));
    }

    SECTION("Places and the zone pages")
    {
        const std::string places = JoinedChain(doc, "GM_PLACES");
        CHECK(Has(places, "Places 8 of 9 scouted."));
        CHECK(Has(places, "Ours Camp - 0.4 km SW")); // "camp" not repeated after "Camp"
        CHECK(Has(places, "Houdan - town, 0.9 km E"));
        CHECK(Has(places, "Outpost - heat 55, 1.3 km N"));
        CHECK(Has(places, "Contested Airfield - 40% secured, garrison 9, RED, 2.4 km NE"));
        CHECK(Has(places, "Neutral Chapoi - town, ready to rise, support 66, 2 occupiers in town, 3.1 km S"));
        CHECK(Has(places, "Vigny - town, support 12, line 60, 5.2 km S"));
        CHECK(Has(places, "Occupied Seaport - port, garrison 6, YELLOW, 2.1 km NE"));
        CHECK(Has(places, "Unscouted Le Port - town, 6.3 km NW"));
        const JournalRun* airfieldTail = FindRun(Page(doc, "GM_PLACES"), "40% secured, garrison 9, RED");
        REQUIRE(airfieldTail);
        CHECK(airfieldTail->ink == InkRed);

        const std::string airfield = Joined(Page(doc, "GM_ZONE_3"));
        CHECK(Has(airfield, "Airfield Contested airfield. Day 3, 14:20."));
        CHECK(Has(airfield, "State: SECURING, garrison 9."));
        CHECK(Has(airfield, "Alert: RED, a quick reaction force is out."));
        CHECK(Has(airfield, "Capture: 40% secured."));
        CHECK(Has(airfield, "Heat: 48, aware."));
        CHECK(Has(airfield, "Distance: 2.4 km NE."));
        CHECK(Has(airfield, "Last seen: 14:02."));
        CHECK(Has(airfield, "Record"));
        CHECK(Has(airfield, "14:02 Went RED."));
        CHECK_FALSE(Has(airfield, "Liberated.")); // another zone's line
        CHECK_FALSE(Has(airfield, "Diary"));
        const JournalRun* state = FindRun(Page(doc, "GM_ZONE_3"), "SECURING, garrison 9");
        REQUIRE(state);
        CHECK(state->ink == InkRed);
        const JournalRun* alert = FindRun(Page(doc, "GM_ZONE_3"), "RED, a quick reaction force");
        REQUIRE(alert);
        CHECK(alert->ink == InkRed);
        const JournalRun* wentRed = FindRun(Page(doc, "GM_ZONE_3"), "Went RED.");
        REQUIRE(wentRed);
        CHECK(wentRed->ink == InkRed);
        CHECK(wentRed->voice == VoiceHand);

        const std::string lePort = Joined(Page(doc, "GM_ZONE_8"));
        CHECK(Has(lePort, "Le Port Unscouted town."));
        CHECK(Has(lePort, "Not scouted yet"));
        CHECK(Has(lePort, "Distance: 6.3 km NW."));
        CHECK(Has(lePort, "Nothing written about this place yet."));

        const std::string houdan = Joined(Page(doc, "GM_ZONE_1"));
        CHECK(Has(houdan, "Houdan Our town."));
        CHECK(Has(houdan, "Support: 82, risen."));
        CHECK(Has(houdan, "Here: headquarters, 2 garaged, arms dealer."));
    }

    SECTION("the record, newest first, by day")
    {
        const std::string record = JoinedChain(doc, "GM_RECORD");
        CHECK(Has(record, "The record 3 entries."));
        CHECK(Before(record, "Day 3", "Day 1"));
        CHECK(Has(record, "14:02 Airfield. Went RED."));
        CHECK(Has(record, "16:22 Outpost. Liberated."));
        CHECK(Has(record, "08:00 Camp. Reached the Camp alone."));
        CHECK(Before(record, "Went RED.", "Reached the Camp"));
        CHECK(doc.PartCount("GM_RECORD") == 1);
        CHECK(Has(Joined(Page(doc, "GM_CHRONICLES")), "Chronicles 3 entries."));
    }
}

// ===========================================================================
// 6. no-clip structure and the em dash lint
// ===========================================================================

TEST_CASE("Journal compose - names never sit in fixed cells and nothing carries an em dash",
          "[game][guerrilla][journal][compose]")
{
    {
        Journal journal;
        FillSampleJournal(journal);
        const JournalPageInputs in = SampleInputs();
        CheckNoClip(ComposeJournal(journal, in), in);
    }
    {
        Journal journal;
        FillLongJournal(journal);
        const JournalPageInputs in = LongNameInputs();
        const JournalDocument doc = ComposeJournal(journal, in);
        CheckNoClip(doc, in);
        // the five-slot name is a wrapping run on the roster and never cut
        const JournalPage& roster = Page(doc, "GM_ROSTER");
        const JournalRun* petra = FindRun(roster, "Sgt. Petra \"The Wall\" Novakova of Houdan");
        REQUIRE(petra);
        CHECK(petra->cellWidth == 0);
        CHECK(petra->href.GetLength() == 0);
        CHECK(S(petra->text) == "Sgt. Petra \"The Wall\" Novakova of Houdan");
        // long place names are links in the index and titles on their pages
        const JournalRun* barracks = FindRun(Page(doc, "GM_PLACES"), "Marjayoun Barracks");
        REQUIRE(barracks);
        CHECK(barracks->cellWidth == 0);
        CHECK(S(barracks->href) == "#GM_ZONE_4");
        CHECK(S(Page(doc, "GM_ZONE_2").title) == "Ras Nasrani Outpost");
        CHECK(Has(Joined(Page(doc, "Main")),
                  "Malden. Popular Front for the Liberation of the Sinai Peninsula against the Frontier Corps of "
                  "the Third Field Army."));
        // no "..." on the People / Roster / Places pages: Compose never trims names
        for (int p = 0; p < doc.pages.Size(); p++)
        {
            const JournalPage& page = doc.pages[p];
            const std::string name = S(page.name);
            if (name != "GM_PEOPLE" && name.rfind("GM_ROSTER", 0) != 0 && name.rfind("GM_PLACES", 0) != 0)
            {
                continue;
            }
            for (int b = 0; b < page.blocks.Size(); b++)
            {
                for (int r = 0; r < page.blocks[b].runs.Size(); r++)
                {
                    const std::string text = S(page.blocks[b].runs[r].text);
                    INFO("page " << name << " run: " << text);
                    CHECK(text.find("...") == std::string::npos);
                }
            }
        }
    }
    {
        // the cells that ARE allowed: the Reference index numbers and spacers
        // and the handbook table cells
        Journal journal;
        const JournalDocument doc = ComposeJournal(journal, SampleInputs());
        int cells = 0;
        const JournalPage& ref = Page(doc, "GM_REFERENCE");
        for (int b = 0; b < ref.blocks.Size(); b++)
        {
            for (int r = 0; r < ref.blocks[b].runs.Size(); r++)
            {
                if (ref.blocks[b].runs[r].cellWidth > 0)
                {
                    cells++;
                    CHECK(ref.blocks[b].runs[r].href.GetLength() == 0);
                }
            }
        }
        CHECK(cells == 20); // a number and a spacer per row
        const JournalPage& undercover = Page(doc, "GM_MAN_UNDERCOVER");
        int tableCells = 0;
        for (int b = 0; b < undercover.blocks.Size(); b++)
        {
            for (int r = 0; r < undercover.blocks[b].runs.Size(); r++)
            {
                if (undercover.blocks[b].runs[r].cellWidth > 0)
                {
                    tableCells++;
                }
            }
        }
        CHECK(tableCells > 0);
    }
}

// ===========================================================================
// 7. empty inputs
// ===========================================================================

TEST_CASE("Journal compose - empty inputs still compose every fixed page", "[game][guerrilla][journal][compose]")
{
    Journal journal;
    JournalPageInputs in;
    in.islandName = "Malden";
    const JournalDocument doc = ComposeJournal(journal, in);

    static const char* kFixed[] = {"Main",          "GM_DISPATCH", "Plan",        "GM_OBJECTIVES", "GM_ACTIONS",
                                   "GM_SUPPLY",     "GM_FACTION",  "GM_PEOPLE",   "GM_ROSTER",     "GM_PLACES",
                                   "GM_CHRONICLES", "GM_RECORD",   "GM_REFERENCE"};
    for (const char* name : kFixed)
    {
        INFO(name);
        CHECK(doc.FindPage(name) >= 0);
        CHECK(doc.PartCount(name) == 1);
    }
    for (int i = 0; i < GuerrillaManualTopicCount(); i++)
    {
        CHECK(doc.FindPage(GuerrillaManualTopicAnchor(i)) >= 0);
    }
    CHECK(doc.FindPage("GM_ZONE_0") < 0); // no zones: no zone pages

    // no clock: the campaign line alone
    const JournalPage& main = Page(doc, "Main");
    REQUIRE(main.blocks.Size() >= 2);
    CHECK(S(main.blocks[0].runs[0].text) == "Resistance Dossier");
    CHECK(main.blocks[0].runs[0].voice == VoiceTitle);
    // CampaignLine ends the island in ". " so a faction sentence can follow;
    // with no faction that trailing space is invisible on the page (fields
    // lay adjacently), so compare the trimmed run
    std::string campaign = S(main.blocks[1].runs[0].text);
    while (!campaign.empty() && campaign.back() == ' ')
    {
        campaign.pop_back();
    }
    CHECK(campaign == "Malden.");
    CHECK(main.blocks[1].runs[0].voice == VoiceSerif);
    CHECK(Has(Joined(main), "Resistance Dossier Malden. Dispatches"));
    CHECK(Hrefs(main).size() == 6);

    const std::string dispatch = Joined(Page(doc, "GM_DISPATCH"));
    CHECK(Has(dispatch, "Dispatches Malden."));
    CHECK(Has(dispatch, "Threat: The garrisons are quiet."));
    CHECK(Has(dispatch, "Objective: Nothing open."));
    CHECK(Has(dispatch, "Latest: Nothing written yet."));
    CHECK_FALSE(Has(dispatch, "civilian")); // cover not armed: no line in the hand
    CHECK(Count(Page(doc, "GM_DISPATCH"), HasHand) == 0);

    const std::string actions = Joined(Page(doc, "GM_ACTIONS"));
    CHECK(Has(actions, "2 moves."));
    CHECK(Has(actions, "Scout the island. Zones show once they are within reach of ground we hold."));
    CHECK(Has(actions, "Set up a headquarters. Any town, or the Camp. It gives us a cache and a garage."));
    CHECK(Before(actions, "Scout the island", "Set up a headquarters"));

    // the objectives page ports BuildPlan exactly: with no zones the two
    // engine orders still stand ("Hold every base. 0 of 0."); only Dispatches
    // guards them on a non-empty zone set
    CHECK(Has(Joined(Page(doc, "GM_OBJECTIVES")), "2 objectives open."));
    CHECK(Has(Joined(Page(doc, "GM_RECORD")), "The record 0 entries. Nothing written yet."));
    CHECK(Has(Joined(Page(doc, "GM_ROSTER")), "No fighters recorded."));
    CHECK(Has(Joined(Page(doc, "GM_ROSTER")), "Standard issue: the faction's basic rifle."));
    CHECK(Has(Joined(Page(doc, "GM_SUPPLY")), "Nothing recorded yet."));
    CHECK(Has(Joined(Page(doc, "GM_PLACES")), "Places 0 of 0 scouted."));
    CHECK(Has(Joined(Page(doc, "GM_PEOPLE")), "People 0 under arms."));
    CHECK(Has(Joined(Page(doc, "GM_FACTION")), "War level 1 of 10."));

    CheckLimits(doc);
    CheckNoClip(doc, in);
}
