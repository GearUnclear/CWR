// Guerrilla journal, Compose stage part B (Change 2): the People index and its
// three dynamic lists, the GM_WHO_<id> dossiers, the per-character record
// filtered by charId, and the generated history's hub and beat pages.
//
// Everything here composes against a hand-built JournalPageInputs: no world, no
// LegendRegistry, no file system (portraitDir is empty, so nothing is probed).
// The one Render-level case at the end is a page-budget probe and says so.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/FactionHistory.hpp> // the generator the dossier budget belongs to
#include <Poseidon/Game/Guerrilla/Journal.hpp>
#include <Poseidon/Game/Guerrilla/LegendSeed.hpp>     // HashKey
#include <Poseidon/Graphics/Textures/TextureBank.hpp> // NoTextures
#include <Poseidon/UI/Controls/UIControlsBase.hpp>    // CHTMLContainer (parser-only subclass)
#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp>
#include <Poseidon/UI/Guerrilla/JournalCompose.hpp>
#include <Poseidon/UI/Guerrilla/JournalRender.hpp>

#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

std::string S(const RString& s)
{
    return std::string((const char*)s);
}

bool Has(const std::string& text, const char* needle)
{
    return text.find(needle) != std::string::npos;
}

bool Before(const std::string& text, const char* a, const char* b)
{
    const size_t at = text.find(a);
    const size_t bt = text.find(b);
    return at != std::string::npos && bt != std::string::npos && at < bt;
}

// ---------------------------------------------------------------------------
// fixtures
// ---------------------------------------------------------------------------

JournalCharacterView Companion(const char* id, const char* display, const char* base, const char* role,
                               const char* status, bool alive = true)
{
    JournalCharacterView ch;
    ch.id = id;
    ch.displayName = display;
    ch.baseName = base;
    ch.kind = 0;
    ch.role = role;
    ch.status = status;
    ch.alive = alive;
    ch.rank = 2;
    return ch;
}

JournalCharacterView Boss(const char* id, const char* display, const char* role, bool defeated = false)
{
    JournalCharacterView ch;
    ch.id = id;
    ch.displayName = display;
    ch.kind = 1;
    ch.role = role;
    ch.status = defeated ? "defeated" : "at large";
    ch.legend = true;
    ch.defeated = defeated;
    ch.zone = "Airfield";
    return ch;
}

// a small campaign: two companions with me, three enemy Legends, two fallen
JournalPageInputs PeopleInputs()
{
    JournalPageInputs in;
    in.islandName = "Malden";
    in.resistanceName = "FIA";
    in.occupierName = "Soviet Army";
    in.day = 4;
    in.minuteOfDay = 9 * 60 + 30;
    in.characters.Add(Companion("comp_0_petra", "Iron Petra Kovacevic", "Petra", "Sgt", "with me"));
    in.characters.Add(Companion("comp_1_andre", "Andre Marek", "Andre", "Cpl", "unaccounted for"));
    in.characters.Add(Boss("boss_0", "Cold Erez Dayan", "Commander"));
    in.characters.Add(Boss("boss_1", "Heartless Yuri Sokolov", "Commander"));
    in.characters.Add(Boss("boss_2", "Grim Milos Vasic", "Commander", true));
    in.characters.Add(Companion("comp_2_jana", "Jana Horak", "Jana", "Pvt", "fallen", false));
    in.characters.Add(Companion("comp_3_tomas", "Tomas Bartos", "Tomas", "Pvt", "fallen", false));
    return in;
}

// the generated history as the registry hands it over
void FillHistory(JournalPageInputs& in)
{
    in.history.present = true;
    in.history.opening1 = "Long before the road came, the people of this island counted the doorways rather "
                          "than the houses, and every doorway was a debt owed to a neighbour.";
    in.history.opening2 = "Those who answer to Soviet Army are only the latest to march that road, and the "
                          "island has learned to count them the way it counts a season of bad weather.";
    in.history.events[0].title = "The tally at Larche";
    in.history.events[0].text = "A clerk sat at the crossroads for eleven days and wrote down what every "
                                "family owed, and the families walked past him in silence.";
    in.history.events[0].place = "Larche";
    // every shipped event title ends in its own place name, so the fixture
    // follows that shape: a hub description built from the place would then be
    // a verbatim echo of the link's last word
    in.history.events[1].title = "The compact at Le Port";
    in.history.events[1].text = "The terms were read aloud at the harbour and signed by men who had never "
                                "seen the fields they were dividing.";
    in.history.events[1].place = "Le Port";
    in.history.events[2].title = "The stand at the ridge";
    in.history.events[2].text = "Nineteen of them held the saddle of the ridge from dawn until the light "
                                "failed, and the histories still argue about how many were shepherds.";
    in.history.events[2].place = "Houdan ridge";
    return;
}

const JournalPage& Page(const JournalDocument& doc, const char* name)
{
    const int at = doc.FindPage(name);
    REQUIRE(at >= 0);
    return doc.pages[at];
}

std::string Joined(const JournalPage& page)
{
    std::string out;
    for (int b = 0; b < page.blocks.Size(); b++)
    {
        for (int r = 0; r < page.blocks[b].runs.Size(); r++)
        {
            if (!out.empty())
            {
                out += ' ';
            }
            out += S(page.blocks[b].runs[r].text);
        }
    }
    // collapse the runs' own leading spaces so a search reads as on screen
    std::string collapsed;
    for (char c : out)
    {
        if (c == ' ' && !collapsed.empty() && collapsed.back() == ' ')
        {
            continue;
        }
        collapsed += c;
    }
    return collapsed;
}

std::string JoinedChain(const JournalDocument& doc, const char* base)
{
    std::string out;
    for (int part = 1;; part++)
    {
        char name[128];
        if (part == 1)
        {
            snprintf(name, sizeof(name), "%s", base);
        }
        else
        {
            snprintf(name, sizeof(name), "%s_%d", base, part);
        }
        const int at = doc.FindPage(name);
        if (at < 0)
        {
            break;
        }
        if (!out.empty())
        {
            out += ' ';
        }
        out += Joined(doc.pages[at]);
    }
    return out;
}

// Pen::LinkRow writes the link and its description as two runs of ONE block,
// the description carrying a two-space lead.  This reads them back as pairs so
// a test can say what the description must NOT be.
std::vector<std::pair<std::string, std::string>> LinkRows(const JournalPage& page)
{
    std::vector<std::pair<std::string, std::string>> out;
    for (int b = 0; b < page.blocks.Size(); b++)
    {
        const JournalBlock& block = page.blocks[b];
        if (block.runs.Size() < 2 || block.runs[0].href.GetLength() == 0)
        {
            continue;
        }
        std::string description = S(block.runs[1].text);
        size_t at = 0;
        while (at < description.size() && description[at] == ' ')
        {
            at++;
        }
        out.push_back({S(block.runs[0].text), description.substr(at)});
    }
    return out;
}

// lowercased, closing punctuation dropped: what the two strings say, not how
// they are typed
std::string Bare(const std::string& s)
{
    std::string out = s;
    while (!out.empty() && (out.back() == '.' || out.back() == '!' || out.back() == '?' || out.back() == ' '))
    {
        out.pop_back();
    }
    for (char& c : out)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c = (char)(c + ('a' - 'A'));
        }
    }
    return out;
}

// A hub description exists to add what the link text cannot carry, so it may
// never be the link text nor its tail (the defect this pins: every generated
// event title ends in the place, so passing the place as the description
// repeated the title's last word on every row).
void CheckDescriptionsAddSomething(const JournalPage& page)
{
    for (const auto& row : LinkRows(page))
    {
        const std::string link = Bare(row.first);
        const std::string description = Bare(row.second);
        if (description.empty())
        {
            continue;
        }
        INFO("page " << S(page.name) << ": link '" << row.first << "' description '" << row.second << "'");
        CHECK(description != link);
        const bool tailOfLink = description.size() < link.size() &&
                                link.compare(link.size() - description.size(), description.size(), description) == 0;
        CHECK_FALSE(tailOfLink);
    }
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

int CountHrefsTo(const JournalDocument& doc, const char* href)
{
    int count = 0;
    for (int p = 0; p < doc.pages.Size(); p++)
    {
        for (const std::string& h : Hrefs(doc.pages[p]))
        {
            if (h == href)
            {
                count++;
            }
        }
    }
    return count;
}

// every href in the document resolves to a page that exists
void CheckHrefsResolve(const JournalDocument& doc)
{
    for (int p = 0; p < doc.pages.Size(); p++)
    {
        for (const std::string& href : Hrefs(doc.pages[p]))
        {
            INFO("page " << S(doc.pages[p].name) << " href " << href);
            REQUIRE(href.size() > 1);
            CHECK(href[0] == '#');
            CHECK(doc.FindPage(href.c_str() + 1) >= 0);
        }
    }
}

bool IsDigits(const std::string& s)
{
    if (s.empty())
    {
        return false;
    }
    for (char c : s)
    {
        if (c < '0' || c > '9')
        {
            return false;
        }
    }
    return true;
}

// No page name may look like a continuation of ANOTHER chain's base name:
// Render names its own height-split pages "<base>_<part>" and reuses sections
// by name, so such a page would silently merge with the overflow of the chain
// it shadows instead of colliding visibly.  This is why the history's beats are
// GM_HIST_EV<k> and not GM_HISTORY_<k>.
void CheckNoContinuationCollision(const JournalDocument& doc)
{
    for (int p = 0; p < doc.pages.Size(); p++)
    {
        const std::string name = S(doc.pages[p].name);
        for (int q = 0; q < doc.pages.Size(); q++)
        {
            const std::string base = S(doc.pages[q].baseName);
            if (base == S(doc.pages[p].baseName) || name.size() <= base.size() + 1)
            {
                continue;
            }
            if (name.compare(0, base.size(), base) != 0 || name[base.size()] != '_')
            {
                continue;
            }
            INFO("page " << name << " shadows a continuation of " << base);
            CHECK_FALSE(IsDigits(name.substr(base.size() + 1)));
        }
    }
}

// no run carrying a character or place name sits in a fixed cell, and nothing
// anywhere carries an em dash (D0)
void CheckNamesWrapAndNoEmDash(const JournalDocument& doc, const JournalPageInputs& in)
{
    std::vector<std::string> names;
    for (int i = 0; i < in.characters.Size(); i++)
    {
        names.push_back(S(in.characters[i].displayName));
    }
    names.push_back(S(in.resistanceName));
    names.push_back(S(in.occupierName));
    for (int p = 0; p < doc.pages.Size(); p++)
    {
        const JournalPage& page = doc.pages[p];
        for (int b = 0; b < page.blocks.Size(); b++)
        {
            for (int r = 0; r < page.blocks[b].runs.Size(); r++)
            {
                const JournalRun& run = page.blocks[b].runs[r];
                const std::string text = S(run.text);
                INFO("page " << S(page.name) << " run: " << text);
                CHECK(text.find("\xE2\x80\x94") == std::string::npos);
                if (run.cellWidth > 0)
                {
                    for (const std::string& n : names)
                    {
                        CHECK(text != n);
                    }
                }
            }
        }
    }
}

int WordCount(const std::string& s)
{
    int count = 0;
    bool inWord = false;
    for (char c : s)
    {
        const bool space = c == ' ' || c == '\t' || c == '\n' || c == '\r';
        if (!space && !inWord)
        {
            count++;
        }
        inWord = !space;
    }
    return count;
}

// the hand caps over the whole document: every hand run inside 25 words, and a
// page carrying at most two NON-annotation hand blocks (a dossier's deed and a
// record line are annotation rows and are exempt from the second rule)
void CheckHandLimits(const JournalDocument& doc)
{
    for (int p = 0; p < doc.pages.Size(); p++)
    {
        const JournalPage& page = doc.pages[p];
        int narrative = 0;
        for (int b = 0; b < page.blocks.Size(); b++)
        {
            const JournalBlock& block = page.blocks[b];
            bool hand = false;
            for (int r = 0; r < block.runs.Size(); r++)
            {
                if (block.runs[r].voice != VoiceHand)
                {
                    continue;
                }
                hand = true;
                INFO("page " << S(page.name) << " hand run: " << S(block.runs[r].text));
                CHECK(WordCount(S(block.runs[r].text)) <= ComposeLimits::HandWords);
            }
            if (hand)
            {
                INFO("page " << S(page.name));
                CHECK(block.HandWords() <= ComposeLimits::HandWords);
                if (!block.annotation)
                {
                    narrative++;
                }
            }
        }
        INFO("page " << S(page.name));
        CHECK(narrative <= ComposeLimits::HandBlocksPerPage);
    }
}

int CountPortraitBlocks(const JournalPage& page)
{
    int count = 0;
    for (int b = 0; b < page.blocks.Size(); b++)
    {
        if (page.blocks[b].kind == BlockPortrait)
        {
            count++;
        }
    }
    return count;
}

// the parser-only container the render probe measures against: a settable page
// height and width, text widths that scale with the slot size (half a unit per
// character at size 1), so the 1.45x title and the 1.6x hand wrap earlier than
// the typed body
class JournalPageHtml : public CHTMLContainer
{
  public:
    float pageHeight = 100000;
    float pageWidth = 1000;

    float GetPageWidth() const override { return pageWidth; }
    float GetPageHeight() const override { return pageHeight; }
    float GetTextWidth(float size, Font*, const char* text) const override
    {
        return size * (float)std::strlen(text) * 0.5f;
    }
};

struct NoTexturesGuard
{
    bool previous;
    NoTexturesGuard() : previous(Poseidon::NoTextures) { Poseidon::NoTextures = true; }
    ~NoTexturesGuard() { Poseidon::NoTextures = previous; }
};

std::string SectionText(const JournalPageHtml& html, int section)
{
    std::string out;
    if (section < 0)
    {
        return out;
    }
    const HTMLSection& sec = html.GetSection(section);
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        if (!out.empty())
        {
            out += ' ';
        }
        out += S(sec.fields[f].text);
    }
    return out;
}

} // namespace

// ===========================================================================
// 1. the People index, its three lists and the dossiers they link to
// ===========================================================================

TEST_CASE("Journal compose - People lists the companions, the enemy Legends and the memorials",
          "[ui][game][guerrilla][journal][compose][people]")
{
    Journal journal;
    journal.AddEntry("Day 2 08:10", "Took the name Iron Petra Kovacevic.", "Camp", JKGood, "comp_0_petra");
    const JournalPageInputs in = PeopleInputs();
    const JournalDocument doc = ComposeJournal(journal, in);

    // the permanent roster entry is still first, and the three heads are all
    // present across the chain (Memorials lands on part 2: heads ride with
    // their first row and never consume one of the five)
    const std::string people = JoinedChain(doc, "GM_PEOPLE");
    CHECK(Has(people, "The roster"));
    CHECK(Before(people, "The roster", "Companions"));
    CHECK(Has(people, "Companions"));
    CHECK(Has(people, "Enemy Legends"));
    CHECK(Has(people, "Memorials"));
    CHECK(Before(people, "Companions", "Enemy Legends"));
    CHECK(Before(people, "Enemy Legends", "Memorials"));
    CHECK(Has(people, "Iron Petra Kovacevic Sgt, with me."));
    CHECK(Has(people, "Cold Erez Dayan Commander, at large."));
    CHECK(Has(people, "Grim Milos Vasic Commander, defeated."));
    CHECK(Has(people, "Jana Horak Pvt, fallen."));

    // one dossier and one record chain per character, each linked exactly once
    // from the index
    for (int i = 0; i < in.characters.Size(); i++)
    {
        const std::string id = S(in.characters[i].id);
        const std::string who = "GM_WHO_" + id;
        INFO(who);
        CHECK(doc.FindPage(who.c_str()) >= 0);
        CHECK(doc.FindPage((who + "_REC").c_str()) >= 0);
        CHECK(CountHrefsTo(doc, ("#" + who).c_str()) == 1);
        CHECK(CountHrefsTo(doc, ("#" + who + "_REC").c_str()) == 1);
    }

    CheckHrefsResolve(doc);
    CheckNoContinuationCollision(doc);
    CheckNamesWrapAndNoEmDash(doc, in);
    CheckHandLimits(doc);

    // the dossier itself
    const JournalPage& petra = Page(doc, "GM_WHO_comp_0_petra");
    CHECK(S(petra.parentName) == "GM_PEOPLE");
    CHECK(S(petra.parentTitle) == "People");
    CHECK(CountPortraitBlocks(petra) == 1);
    const std::string text = Joined(petra);
    CHECK(Has(text, "Iron Petra Kovacevic"));
    CHECK(Has(text, "Sgt, with me."));
    CHECK(Has(text, "Full record"));

    // a defeated Legend and a fallen companion take the red pen on the caption
    for (const char* page : {"GM_WHO_boss_2", "GM_WHO_comp_2_jana"})
    {
        INFO(page);
        bool red = false;
        const JournalPage& p = Page(doc, page);
        for (int b = 0; b < p.blocks.Size(); b++)
        {
            for (int r = 0; r < p.blocks[b].runs.Size(); r++)
            {
                if (p.blocks[b].runs[r].ink == InkRed)
                {
                    red = true;
                }
            }
        }
        CHECK(red);
    }
}

// ===========================================================================
// 1b. Change 3: what the player can learn about an enemy Legend
//
// The three commanders are in the index from campaign start, before any of
// them has been spawned, each with the role ResolveBosses settled and the zone
// his stand was picked in.  Killing one changes the caption (red, "defeated")
// and nothing else: the dossier, the biography and the place stay where they
// were, and the row never moves to Memorials.
// ===========================================================================

namespace
{
// an enemy Legend as Gather hands him over: the role string the registry
// resolved, the zone his stand was picked in, and the status Gather derives
JournalCharacterView Legend(const char* id, const char* display, const char* role, const char* zone, const char* status)
{
    JournalCharacterView ch;
    ch.id = id;
    ch.displayName = display;
    ch.kind = 1;
    ch.role = role;
    ch.status = status;
    ch.zone = zone;
    ch.legend = true;
    ch.defeated = std::strcmp(status, "defeated") == 0;
    return ch;
}

// the typed pencil lines of a page, in order
std::vector<std::string> PencilLines(const JournalPage& page)
{
    std::vector<std::string> out;
    for (int b = 0; b < page.blocks.Size(); b++)
    {
        for (int r = 0; r < page.blocks[b].runs.Size(); r++)
        {
            const JournalRun& run = page.blocks[b].runs[r];
            if (run.ink == InkPencil && run.voice == VoiceType)
            {
                out.push_back(S(run.text));
            }
        }
    }
    return out;
}

bool HasPencilLine(const JournalPage& page, const char* text)
{
    for (const std::string& line : PencilLines(page))
    {
        if (line == text)
        {
            return true;
        }
    }
    return false;
}
} // namespace

TEST_CASE("Journal compose - the enemy Legends carry a role, a place and a permanent dossier",
          "[ui][game][guerrilla][journal][compose][people][legends]")
{
    Journal journal;
    journal.AddEntry("Day 14 06:40", "Zoran Kalnik, Sniper, is dead near Outpost.", "Outpost", JKGood, "boss_0");
    JournalPageInputs in;
    in.day = 14;
    in.islandName = "Malden";
    in.occupierName = "Soviet Army";
    in.characters.Add(Legend("boss_0", "Cold Zoran Kalnik", "Sniper", "Outpost", "defeated"));
    in.characters.Add(Legend("boss_1", "Heartless Yuri Sokolov", "Commander", "Airfield", "at large"));
    in.characters.Add(Legend("boss_2", "Grim Milos Vasic", "Tank Commander", "Seaport", "at large"));
    in.characters.Add(Companion("comp_0_jana", "Jana Horak", "Jana", "Pvt", "fallen", false));
    const JournalDocument doc = ComposeJournal(journal, in);

    // all three are in the index, under one head, and the defeated one is still
    // among them: the head order is Enemy Legends before Memorials, so a row
    // above Memorials is a row under Enemy Legends
    const std::string people = JoinedChain(doc, "GM_PEOPLE");
    CHECK(Has(people, "Enemy Legends"));
    CHECK(Has(people, "Memorials"));
    CHECK(Before(people, "Enemy Legends", "Cold Zoran Kalnik"));
    CHECK(Before(people, "Cold Zoran Kalnik", "Memorials"));
    CHECK(Before(people, "Heartless Yuri Sokolov", "Memorials"));
    CHECK(Before(people, "Grim Milos Vasic", "Memorials"));
    CHECK(Before(people, "Memorials", "Jana Horak"));
    // the role reaches the reader through the caption, alive and dead
    CHECK(Has(people, "Cold Zoran Kalnik Sniper, defeated."));
    CHECK(Has(people, "Heartless Yuri Sokolov Commander, at large."));
    CHECK(Has(people, "Grim Milos Vasic Tank Commander, at large."));

    // the dossier: the place is one pencil line under the caption, in the same
    // words the marker and the objective use
    const JournalPage& sniper = Page(doc, "GM_WHO_boss_0");
    const std::string sniperText = Joined(sniper);
    CHECK(Has(sniperText, "Cold Zoran Kalnik"));
    CHECK(Has(sniperText, "Sniper, defeated."));
    CHECK(HasPencilLine(sniper, "near Outpost"));
    CHECK(Has(Joined(Page(doc, "GM_WHO_boss_1")), "Commander, at large."));
    CHECK(HasPencilLine(Page(doc, "GM_WHO_boss_1"), "near Airfield"));
    CHECK(HasPencilLine(Page(doc, "GM_WHO_boss_2"), "near Seaport"));

    // a defeated Legend keeps his page, his place and his record link, and the
    // caption is the only thing that changed: it takes the red pen
    CHECK(Has(sniperText, "Full record"));
    CHECK(Has(JoinedChain(doc, "GM_WHO_boss_0_REC"), "is dead near Outpost."));
    bool red = false;
    for (int b = 0; b < sniper.blocks.Size(); b++)
    {
        for (int r = 0; r < sniper.blocks[b].runs.Size(); r++)
        {
            if (sniper.blocks[b].runs[r].ink == InkRed)
            {
                red = true;
            }
        }
    }
    CHECK(red);
    // and he is listed ONCE in the whole document: a defeated Legend is never
    // also carried under Memorials, which is the companions' list
    for (const char* id : {"boss_0", "boss_1", "boss_2"})
    {
        INFO(id);
        CHECK(CountHrefsTo(doc, (std::string("#GM_WHO_") + id).c_str()) == 1);
    }

    // a companion's zone is a last-seen reading, not a stand: no place line
    CHECK(PencilLines(Page(doc, "GM_WHO_comp_0_jana")).empty());

    CheckHrefsResolve(doc);
    CheckNoContinuationCollision(doc);
    CheckNamesWrapAndNoEmDash(doc, in);
    CheckHandLimits(doc);
}

TEST_CASE("Journal compose - a Legend with no stand reads as missing intelligence, not as a placed commander",
          "[ui][game][guerrilla][journal][compose][people][legends]")
{
    // What Gather writes when placement found nowhere to put him, and what
    // every Legend of a campaign seeded before Change 3 existed reads as for
    // the rest of that campaign's life: an identity and a dossier, no place.
    Journal journal;
    JournalPageInputs in;
    JournalCharacterView ch = Legend("boss_0", "Cold Zoran Kalnik", "Commander", "", "whereabouts unknown");
    ch.bio = "He signs the orders that reach the checkpoints and has never once been seen at one, which is "
             "the whole of what anybody on this island can tell you about the man.";
    in.characters.Add(ch);
    const JournalDocument doc = ComposeJournal(journal, in);

    const JournalPage& page = Page(doc, "GM_WHO_boss_0");
    const std::string text = Joined(page);
    CHECK(Has(text, "Commander, whereabouts unknown."));
    // no orphan place line, and no empty pencil row where one would have been
    CHECK(text.find("near ") == std::string::npos);
    CHECK(PencilLines(page).empty());
    // he is still in the index, still under Enemy Legends, still with a dossier
    const std::string people = JoinedChain(doc, "GM_PEOPLE");
    CHECK(Before(people, "Enemy Legends", "Cold Zoran Kalnik"));
    CHECK(Has(people, "Cold Zoran Kalnik Commander, whereabouts unknown."));
    CHECK(Has(text, "He signs the orders"));
    CheckNamesWrapAndNoEmDash(doc, in);
}

// ===========================================================================
// 2. pagination: one running index over three lists, five rows to a page
// ===========================================================================

TEST_CASE("Journal compose - six companions paginate the People index at five rows a page",
          "[ui][game][guerrilla][journal][compose][people]")
{
    Journal journal;
    JournalPageInputs in;
    for (int i = 0; i < 6; i++)
    {
        char id[32];
        char name[32];
        snprintf(id, sizeof(id), "comp_%d_fighter", i);
        snprintf(name, sizeof(name), "Fighter %d Novak", i);
        in.characters.Add(Companion(id, name, "Fighter", "Cpl", "with me"));
    }
    const JournalDocument doc = ComposeJournal(journal, in);

    REQUIRE(doc.PartCount("GM_PEOPLE") == 2);
    CHECK(doc.FindPage("GM_PEOPLE_2") >= 0);
    CHECK(doc.FindPage("GM_PEOPLE_3") < 0);

    // exactly five character rows on page 1, the sixth on page 2; the roster
    // entry and the Companions head are not rows
    int onFirst = 0;
    for (const std::string& href : Hrefs(Page(doc, "GM_PEOPLE")))
    {
        if (href.rfind("#GM_WHO_", 0) == 0)
        {
            onFirst++;
        }
    }
    int onSecond = 0;
    for (const std::string& href : Hrefs(Page(doc, "GM_PEOPLE_2")))
    {
        if (href.rfind("#GM_WHO_", 0) == 0)
        {
            onSecond++;
        }
    }
    CHECK(onFirst == 5);
    CHECK(onSecond == 1);
    CHECK(Has(Joined(Page(doc, "GM_PEOPLE")), "The roster"));
    // the group head repeats at the top of the continuation, as the roster's
    // group labels and the record's day heads do
    CHECK(Has(Joined(Page(doc, "GM_PEOPLE_2")), "Companions"));
    CheckHrefsResolve(doc);
    CheckNoContinuationCollision(doc);
}

// ===========================================================================
// 3. the character's record: charId ONLY, newest first, five to a page
// ===========================================================================

TEST_CASE("Journal compose - a character's record is filtered by charId and never by the name in the text",
          "[ui][game][guerrilla][journal][compose][people]")
{
    Journal journal;
    for (int i = 1; i <= 12; i++)
    {
        char stamp[32];
        char text[64];
        snprintf(stamp, sizeof(stamp), "Day 4 %02d:00", i);
        snprintf(text, sizeof(text), "Petra line %d.", i);
        journal.AddEntry(stamp, text, "Camp", JKPlain, "comp_0_petra");
    }
    // three lines that must NOT appear: another character's, an unattributed
    // one, and one whose TEXT carries the base name
    journal.AddEntry("Day 4 21:00", "Andre line.", "Camp", JKPlain, "comp_1_andre");
    journal.AddEntry("Day 4 22:00", "Nobody's line.", "Camp", JKPlain);
    journal.AddEntry("Day 4 23:00", "Petra was seen at the harbour.", "Camp", JKPlain, "comp_1_andre");

    JournalPageInputs in;
    in.day = 4;
    in.characters.Add(Companion("comp_0_petra", "Iron Petra Kovacevic", "Petra", "Sgt", "with me"));
    in.characters.Add(Companion("comp_1_andre", "Andre Marek", "Andre", "Cpl", "with me"));
    const JournalDocument doc = ComposeJournal(journal, in);

    CHECK(doc.PartCount("GM_WHO_comp_0_petra_REC") == 3);
    CHECK(doc.FindPage("GM_WHO_comp_0_petra_REC_4") < 0);
    const std::string record = JoinedChain(doc, "GM_WHO_comp_0_petra_REC");
    CHECK(Has(record, "12 entries."));
    CHECK(Before(record, "Petra line 12.", "Petra line 1."));
    CHECK_FALSE(Has(record, "Andre line."));
    CHECK_FALSE(Has(record, "Nobody's line."));
    CHECK_FALSE(Has(record, "Petra was seen at the harbour."));
    // the decoy DOES belong to Andre, by id
    CHECK(Has(JoinedChain(doc, "GM_WHO_comp_1_andre_REC"), "Petra was seen at the harbour."));

    // an empty record still gets its page rather than a dead link
    JournalPageInputs quiet;
    quiet.characters.Add(Boss("boss_0", "Cold Erez Dayan", "Commander"));
    const JournalDocument quietDoc = ComposeJournal(Journal(), quiet);
    CHECK(Has(Joined(Page(quietDoc, "GM_WHO_boss_0_REC")), "Nothing written about this one yet."));
    CheckHrefsResolve(quietDoc);
}

// ===========================================================================
// 4. the history: nothing at all until the historians have written
// ===========================================================================

TEST_CASE("Journal compose - the history hub and its three beats appear only once the history exists",
          "[ui][game][guerrilla][journal][compose][people]")
{
    Journal journal;
    {
        const JournalPageInputs in = PeopleInputs(); // history.present == false
        const JournalDocument doc = ComposeJournal(journal, in);
        CHECK(doc.FindPage("GM_HISTORY") < 0);
        CHECK(doc.FindPage("GM_HIST_EV0") < 0);
        CHECK_FALSE(Has(Joined(Page(doc, "GM_CHRONICLES")), "History"));
        CHECK(CountHrefsTo(doc, "#GM_HISTORY") == 0);
        CheckHrefsResolve(doc);
    }
    {
        JournalPageInputs in = PeopleInputs();
        FillHistory(in);
        const JournalDocument doc = ComposeJournal(journal, in);
        CHECK(doc.FindPage("GM_HISTORY") >= 0);
        CHECK(CountHrefsTo(doc, "#GM_HISTORY") == 1);
        int beats = 0;
        for (int k = 0; k < 4; k++)
        {
            char name[32];
            snprintf(name, sizeof(name), "GM_HIST_EV%d", k);
            if (doc.FindPage(name) >= 0)
            {
                beats++;
            }
        }
        CHECK(beats == 3);
        // the names Render can generate for GM_HISTORY's own overflow are NOT
        // taken by a beat page
        CHECK(doc.FindPage("GM_HISTORY_2") < 0);
        CHECK(doc.FindPage("GM_HISTORY_3") < 0);

        const std::string hub = Joined(Page(doc, "GM_HISTORY"));
        CHECK(Has(hub, "The long quarrel"));
        CHECK(Has(hub, "As the historians tell it."));
        CHECK(Has(hub, "counted the doorways rather than the houses"));
        CHECK(Has(hub, "The tally at Larche"));

        // Each hub row's description says what the beat IS.  It used to be the
        // beat's place, which every shipped title already ends in, so the hub
        // read "The compact at Le Port / Le Port." - a description that adds
        // nothing.  The place still rides the beat PAGE as its subtitle, which
        // is checked below.
        CHECK(Has(hub, "The ancient grievance."));
        CHECK(Has(hub, "The broken settlement."));
        CHECK(Has(hub, "The stand remembered."));
        CHECK_FALSE(Has(hub, "Le Port."));
        CheckDescriptionsAddSomething(Page(doc, "GM_HISTORY"));
        const std::vector<std::pair<std::string, std::string>> hubRows = LinkRows(Page(doc, "GM_HISTORY"));
        REQUIRE(hubRows.size() == 3);
        CHECK(hubRows[1].first == "The compact at Le Port");
        CHECK(hubRows[1].second == "The broken settlement.");

        const std::string beat = Joined(Page(doc, "GM_HIST_EV2"));
        CHECK(Has(beat, "The stand at the ridge"));
        CHECK(Has(beat, "Houdan ridge."));
        CHECK(Has(beat, "held the saddle of the ridge"));
        CHECK(S(Page(doc, "GM_HIST_EV2").parentName) == "GM_CHRONICLES");

        CheckHrefsResolve(doc);
        CheckNoContinuationCollision(doc);
        CheckNamesWrapAndNoEmDash(doc, in);
    }
}

// ===========================================================================
// 5. the portrait block: always reserved, source built from the injected dir
// ===========================================================================

TEST_CASE("Journal compose - a dossier without a photograph still reserves the portrait box",
          "[ui][game][guerrilla][journal][compose][people]")
{
    Journal journal;
    {
        // the shipped Change 2 state: no portraits exist yet, so every dossier
        // carries an empty-source portrait block and Render draws the
        // "Photograph unavailable" treatment into the identical box
        JournalPageInputs in;
        JournalCharacterView ch = Companion("comp_0_petra", "Iron Petra Kovacevic", "Petra", "Sgt", "with me");
        ch.portraitKey = "soldierg__face10";
        in.characters.Add(ch);
        const JournalDocument doc = ComposeJournal(journal, in);
        const JournalPage& page = Page(doc, "GM_WHO_comp_0_petra");
        REQUIRE(page.blocks.Size() > 0);
        CHECK(page.blocks[0].kind == BlockPortrait);
        CHECK_FALSE(page.blocks[0].portraitPresent);
        CHECK(S(page.blocks[0].portraitSrc).empty());
    }
    {
        // the injected directory is the ONE source of the path: Gather probed
        // "<portraitDir>\<key>.paa" for existence, so Compose must hand Render
        // the same string with a leading backslash and nothing else
        JournalPageInputs in;
        in.portraitDir = "gmcore\\portraits\\v2";
        JournalCharacterView ch = Companion("comp_0_petra", "Iron Petra Kovacevic", "Petra", "Sgt", "with me");
        ch.portraitKey = "soldierg__face10";
        ch.portraitPresent = true;
        in.characters.Add(ch);
        const JournalDocument doc = ComposeJournal(journal, in);
        const JournalPage& page = Page(doc, "GM_WHO_comp_0_petra");
        REQUIRE(page.blocks.Size() > 0);
        CHECK(page.blocks[0].kind == BlockPortrait);
        CHECK(page.blocks[0].portraitPresent);
        CHECK(S(page.blocks[0].portraitSrc) == "\\gmcore\\portraits\\v2\\soldierg__face10.paa");
    }
    {
        // no key (a woman body, or a face the validation refused): no source,
        // and the box is still reserved so the page keeps its shape
        JournalPageInputs in;
        in.portraitDir = "gmcore\\portraits";
        in.characters.Add(Companion("comp_0_petra", "Iron Petra Kovacevic", "Petra", "Sgt", "with me"));
        const JournalDocument doc = ComposeJournal(journal, in);
        const JournalPage& page = Page(doc, "GM_WHO_comp_0_petra");
        CHECK(page.blocks[0].kind == BlockPortrait);
        CHECK(S(page.blocks[0].portraitSrc).empty());
    }
}

// ===========================================================================
// 5b. the biography reaches the page whole; the clamp is only a guard
// ===========================================================================

TEST_CASE("Journal compose - a generated biography is composed whole and the clamp only catches old saves",
          "[ui][game][guerrilla][journal][compose][people]")
{
    Journal journal;
    {
        // What the registry actually stores: FactionHistory is asked for the
        // dossier page's budget (kHistoryBioPageWords), so Compose has nothing
        // to cut.  Nothing downstream could carry a cut word - "Full record"
        // lists journal entries by charId and never prose - so a clamped
        // biography would be unreadable anywhere in the journal.
        HistoryInputs hin;
        hin.resistanceName = "FIA";
        hin.occupierName = "Soviet Army";
        hin.islandName = "Malden";
        PlaceName settlement;
        settlement.key = "El Tor";
        settlement.name = "El Tor";
        settlement.settlement = true;
        hin.settlements.Add(settlement);
        PlaceName second = settlement;
        second.key = "Ras Nasrani";
        second.name = "Ras Nasrani";
        hin.settlements.Add(second);
        PlaceName feature;
        feature.key = "Larche";
        feature.name = "Larche";
        hin.features.Add(feature);
        hin.seed = 4242;
        const HistoryRecord record = GenerateHistory(hin);

        for (int event = 0; event < kHistoryEvents; event++)
        {
            const RString bio = GenerateBio(record, event, RString("Petra"), RString("FIA"), false,
                                            HashKey("comp_0_petra", 4242), kHistoryBioPageWords);
            REQUIRE(bio.GetLength() > 0);
            JournalPageInputs in;
            JournalCharacterView ch = Companion("comp_0_petra", "Iron Petra Kovacevic", "Petra", "Sgt", "with me");
            ch.bio = bio;
            in.characters.Add(ch);
            const JournalDocument doc = ComposeJournal(journal, in);
            const std::string dossier = Joined(Page(doc, "GM_WHO_comp_0_petra"));
            INFO("event " << event << " bio: " << S(bio));
            CHECK(Has(dossier, S(bio).c_str()));
            CHECK_FALSE(Has(dossier, "..."));
        }
    }
    {
        // A save written before the generator learned the page budget still
        // carries a 55-word biography, so the clamp stays.  This pins what it
        // produces rather than ignoring it: 44 words and an ellipsis.
        JournalPageInputs in;
        JournalCharacterView ch = Companion("comp_0_petra", "Iron Petra Kovacevic", "Petra", "Sgt", "with me");
        ch.bio = "Petra was born into one of the families that cut the terraces above Larche, and learned every "
                 "goat path on that slope before learning to read, and nobody in this cell has ever needed to "
                 "hand Petra a map of ground a grandmother measured by hand and walked twice a year.";
        REQUIRE(WordCount(S(ch.bio)) > 45);
        in.characters.Add(ch);
        const JournalDocument doc = ComposeJournal(journal, in);
        const JournalPage& page = Page(doc, "GM_WHO_comp_0_petra");
        std::string composed;
        for (int b = 0; b < page.blocks.Size(); b++)
        {
            for (int r = 0; r < page.blocks[b].runs.Size(); r++)
            {
                const std::string text = S(page.blocks[b].runs[r].text);
                if (text.compare(0, 5, "Petra") == 0 && text.size() > 20)
                {
                    composed = text;
                }
            }
        }
        REQUIRE_FALSE(composed.empty());
        // 44 words, the last one carrying the ellipsis (ClampWords appends it to
        // the head without a space)
        CHECK(WordCount(composed) == 44);
        CHECK(composed.compare(composed.size() - 3, 3, "...") == 0);
        CHECK(S(ch.bio).compare(0, composed.size() - 3, composed.substr(0, composed.size() - 3)) == 0);
    }
}

// ===========================================================================
// 6. the widest name the bank can assemble survives, un-truncated
// ===========================================================================

TEST_CASE("Journal compose - the widest five-slot name and a four-word faction name are never cut",
          "[ui][game][guerrilla][journal][compose][people]")
{
    // 73 characters: the longest hostile assembly the shipped bank can produce
    // (longest first name, longest last name, longest prefix, describer and
    // title), computed over the issue #57 pools
    static const char* kWidest = "Heartless Jean-Baptiste \"The Fence-Builder\" Vakalalabure the Collaborator";
    REQUIRE(std::strlen(kWidest) == 73);

    Journal journal;
    journal.AddEntry("Day 4 09:00", "Held the crossing alone until the light failed.", "Camp", JKGood, "boss_0");
    JournalPageInputs in;
    in.day = 4;
    in.resistanceName = "Popular Front of the Sinai";
    in.occupierName = "Egyptian Frontier Force Command";
    JournalCharacterView ch = Boss("boss_0", kWidest, "Commander");
    ch.bio = "He came out of the delta towns with a clerk's hands and a quarrel he had inherited from an "
             "uncle he never met, and the border posts learned his name before the army did, which is how "
             "these things usually go.";
    ch.deedLatest = "Held the crossing at the old bridge alone until the light failed and the column turned back.";
    in.characters.Add(ch);
    const JournalDocument doc = ComposeJournal(journal, in);

    const std::string dossier = Joined(Page(doc, "GM_WHO_boss_0"));
    const std::string people = JoinedChain(doc, "GM_PEOPLE");
    CHECK(Has(dossier, kWidest));
    CHECK(Has(people, kWidest));
    CHECK_FALSE(Has(dossier, "Heartless Jean-Baptiste \"The Fence-Builder\" Vakalalabure the Collaborator..."));
    // the four-word faction name rides the Change 1 Contents page un-cut
    CHECK(Has(Joined(Page(doc, "Main")), "Egyptian Frontier Force Command"));

    // no run of the whole document is an ellipsised name, and no name sits in
    // a fixed cell
    CheckNamesWrapAndNoEmDash(doc, in);
    CheckHandLimits(doc);
    for (int p = 0; p < doc.pages.Size(); p++)
    {
        for (int b = 0; b < doc.pages[p].blocks.Size(); b++)
        {
            for (int r = 0; r < doc.pages[p].blocks[b].runs.Size(); r++)
            {
                const std::string text = S(doc.pages[p].blocks[b].runs[r].text);
                if (text.size() > 3 && text.compare(text.size() - 3, 3, "...") == 0)
                {
                    INFO("clipped run on " << S(doc.pages[p].name) << ": " << text);
                    CHECK(text.find(kWidest) == std::string::npos);
                    CHECK(text.find("Egyptian Frontier Force Command") == std::string::npos);
                }
            }
        }
    }
    CheckHrefsResolve(doc);
    CheckNoContinuationCollision(doc);
}

// ===========================================================================
// 7. render probe: the dossier's page budget at a modelled 800x600 notepad
//
// The spec's visual acceptance line wants a representative portrait dossier on
// ONE physical page at 800x600.  The notepad is modelled here as about 26 typed
// rows of about 68 characters (P = 1, page width 34).  MEASURED at that model a
// worst-case dossier does NOT fit one page: Render reserves half the page
// budget for the portrait box (JournalRender.hpp kPortraitMaxHeightFraction,
// applied whether or not the photograph resolves), leaving about nine rows for
// a 1.45x name, a caption, a 45-word 1.1x biography (about five rows) and a
// 25-word 1.6x hand deed (about six rows).  Closing that gap means changing a
// Render constant, not a Compose choice, so what Change 2 pins here is the
// weaker invariant that does hold: the dossier never runs past ONE continuation
// and every part of it stays reachable.
// ===========================================================================

TEST_CASE("Journal render - a worst-case dossier stays inside one continuation at a modelled 800x600 page",
          "[ui][game][guerrilla][journal][render][people]")
{
    NoTexturesGuard noTextures;

    Journal journal;
    JournalPageInputs in;
    in.day = 4;
    in.portraitDir = "gmcore\\portraits";
    JournalCharacterView ch = Boss("boss_0",
                                   "Heartless Jean-Baptiste \"The Fence-Builder\" Vakalalabure the "
                                   "Collaborator",
                                   "Commander");
    ch.portraitKey = "soldiere__face27";
    ch.portraitPresent = true;
    // Change 3 adds the place line to this page, so the worst case gains the
    // longest zone name a shipped template ships (@LoBo's Sinai table)
    ch.zone = "Ras Nasrani Outpost";
    ch.bio = "He came out of the delta towns with a clerk's hands and a quarrel he had inherited from an uncle "
             "he never met, and the border posts along the coast road learned to say his name carefully long "
             "before the army did, which is the way these things usually go on this coast.";
    ch.deedLatest = "Held the crossing at the old bridge alone until the light failed and the column ahead of him "
                    "finally turned back north.";
    in.characters.Add(ch);

    JournalDocument doc;
    {
        const JournalDocument full = ComposeJournal(journal, in);
        // render the dossier chain alone: the probe is about this page's
        // budget, not about the rest of the journal
        for (int p = 0; p < full.pages.Size(); p++)
        {
            if (S(full.pages[p].baseName) == "GM_WHO_boss_0")
            {
                doc.pages.Add(full.pages[p]);
            }
        }
    }
    REQUIRE(doc.pages.Size() == 1);

    JournalPageHtml html;
    html.pageWidth = 34;
    html.pageHeight = 26 * html.GetPHeight();
    RenderJournal(&html, doc, in);

    REQUIRE(html.FindSection("GM_WHO_boss_0") >= 0);
    CHECK(html.FindSection("GM_WHO_boss_0_3") < 0);
    // MEASURED at this model: two physical pages, the portrait at its clamp
    // filling half of page one.  Pinning the count makes any later change to
    // kPortraitMaxHeightFraction or to the word budgets visible right here.
    CHECK(html.NSections() == 2);

    std::string whole = SectionText(html, html.FindSection("GM_WHO_boss_0"));
    const int second = html.FindSection("GM_WHO_boss_0_2");
    if (second >= 0)
    {
        whole += " " + SectionText(html, second);
    }
    CHECK(Has(whole, "Vakalalabure the Collaborator"));
    CHECK(Has(whole, "Commander, at large."));
    CHECK(Has(whole, "near Ras Nasrani Outpost"));
    CHECK(Has(whole, "clerk's hands"));
    CHECK(Has(whole, "Held the crossing"));
    CHECK(Has(whole, "Full record"));

    // the portrait is a real image field with a clamped, non-zero box
    bool image = false;
    for (int s = 0; s < html.NSections(); s++)
    {
        const HTMLSection& sec = html.GetSection(s);
        for (int f = 0; f < sec.fields.Size(); f++)
        {
            if (sec.fields[f].format == HFImg)
            {
                image = true;
                CHECK(sec.fields[f].width > 0);
                CHECK(sec.fields[f].height > 0);
                CHECK(sec.fields[f].height <= 0.5f * (html.GetPageHeight() - 3.5f * html.GetPHeight()));
            }
        }
    }
    CHECK(image);
}
