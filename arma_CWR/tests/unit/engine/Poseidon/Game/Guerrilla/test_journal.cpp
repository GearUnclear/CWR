#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/Journal.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/UI/Controls/UIControlsBase.hpp> // CHTMLContainer (parser-only subclass)
#include <Poseidon/UI/Guerrilla/GuerrillaJournalPages.hpp>
#include <Poseidon/UI/UITestEngine.hpp> // GetHtmlText

#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

// parser-only container: page metrics without renderer services (same idea
// as TestHtmlContainer in UI/test_optionsUI.cpp).
//
// The default page height is effectively unbounded, so in most cases below
// CheckPageSet's budget / no-'/' / no-image checks are structural only (a
// ~30 P document can never overrun 100000): the budget mechanism itself is
// proven at realistic heights in UI/Guerrilla/test_journal_render.cpp, and
// the tight-height case among the page cases runs the real composed
// document against a height that forces the cut.  The one-page acceptance
// for Contents / Dispatches / Operations / People / Reference at the real
// notepad metrics belongs to the guerrilla_journal_capture_800 / _1440
// Trident lanes, not here (the strlen text widths would make it a guess).
class JournalHtml : public CHTMLContainer
{
  public:
    float pageHeight = 100000;

    void SelectSection(const char* name) { _currentSection = FindSection(name); }
    // GetHtmlText joins fields with one space; the renderer lays them out
    // adjacent (a "Label: " field carries its own space), so collapse runs
    // of spaces to compare against the on-screen reading
    std::string Text(const char* name)
    {
        SelectSection(name);
        if (_currentSection < 0)
        {
            return std::string();
        }
        std::string raw = UITestEngine::GetHtmlText(*this);
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

    float GetPageWidth() const override { return 1000; }
    float GetPageHeight() const override { return pageHeight; }
    float GetTextWidth(float, Font*, const char* text) const override { return (float)std::strlen(text); }
};

} // namespace

TEST_CASE("Journal - empty journal queries are safe", "[game][guerrilla][journal]")
{
    Journal journal;
    CHECK(journal.IsEmpty());
    CHECK(journal.EntryCount() == 0);
    CHECK(journal.ObjectiveCount() == 0);
    CHECK(journal.StatusCount() == 0);
    CHECK(journal.FindObjective("nope") == -1);
    CHECK(journal.FindObjective(nullptr) == -1);
    CHECK(journal.FindStatus("nope") == -1);
    // empty text / id / key writes are no-ops
    journal.AddEntry("Day 1 08:00", "");
    journal.SetObjective("", "text", JOActive);
    journal.SetObjective("x", "", JODone); // state-only write on unknown row
    journal.SetStatus("", "value");
    journal.SetStatus("Companions", ""); // remove unknown row
    CHECK(journal.IsEmpty());
}

TEST_CASE("Journal - diary caps at MaxEntries and keeps the newest", "[game][guerrilla][journal]")
{
    Journal journal;
    unsigned rev = journal.Revision();
    for (int i = 0; i < Journal::MaxEntries + 5; i++)
    {
        char text[32];
        snprintf(text, sizeof(text), "entry %d", i);
        journal.AddEntry("", text);
    }
    CHECK(journal.EntryCount() == Journal::MaxEntries);
    CHECK(std::string((const char*)journal.Entry(0).text) == "entry 5");
    CHECK(std::string((const char*)journal.Entry(journal.EntryCount() - 1).text) ==
          std::string("entry ") + std::to_string(Journal::MaxEntries + 4));
    CHECK(journal.Revision() != rev);
}

TEST_CASE("Journal - objectives upsert by id, status lines by key", "[game][guerrilla][journal]")
{
    Journal journal;
    journal.SetObjective("recruit", "Recruit a fighter", JOActive);
    journal.SetObjective("Recruit", "", JODone); // case-insensitive, state-only
    REQUIRE(journal.ObjectiveCount() == 1);
    CHECK(journal.Objective(0).state == JODone);
    CHECK(std::string((const char*)journal.Objective(0).text) == "Recruit a fighter");
    journal.SetObjective("recruit", "Recruit two fighters", JOActive);
    CHECK(std::string((const char*)journal.Objective(0).text) == "Recruit two fighters");
    CHECK(journal.Objective(0).state == JOActive);
    // out-of-range states clamp to ACTIVE
    journal.SetObjective("odd", "Odd", 99);
    CHECK(journal.Objective(1).state == JOActive);

    CHECK(Journal::ObjectiveStateFromName("done") == JODone);
    CHECK(Journal::ObjectiveStateFromName("FAILED") == JOFailed);
    CHECK(Journal::ObjectiveStateFromName("hidden") == JOHidden);
    CHECK(Journal::ObjectiveStateFromName("active") == JOActive);
    CHECK(Journal::ObjectiveStateFromName("bogus") == -1);
    CHECK(std::string(Journal::ObjectiveStateName(JOFailed)) == "FAILED");

    journal.SetStatus("Companions", "Petra (CORPORAL)");
    journal.SetStatus("Unlocked gear", "AK47");
    journal.SetStatus("companions", "Petra (SERGEANT)"); // upsert, case-insensitive
    REQUIRE(journal.StatusCount() == 2);
    CHECK(std::string((const char*)journal.Status(0).text) == "Petra (SERGEANT)");
    journal.SetStatus("Companions", ""); // remove
    REQUIRE(journal.StatusCount() == 1);
    CHECK(std::string((const char*)journal.Status(0).key) == "Unlocked gear");
}

TEST_CASE("Journal - save/load round-trip keeps entries, objectives and status",
          "[game][guerrilla][journal][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "journal-roundtrip.bin";

    {
        Journal journal;
        journal.AddEntry("Day 1 08:00", "The campaign begins.");
        journal.AddEntry("Day 2 14:20", "Outpost liberated");
        journal.SetObjective("recruit", "Recruit a fighter", JODone);
        journal.SetObjective("town", "Raise a town", JOActive);
        journal.SetStatus("Companions", "Petra (CORPORAL)");

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(journal.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
        CHECK(journal.EntryCount() == 2); // saving leaves the live tables alone
    }

    Journal loaded;
    unsigned rev = loaded.Revision();
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK); // second pass is a no-op, never doubles
    }
    REQUIRE(loaded.EntryCount() == 2);
    CHECK(std::string((const char*)loaded.Entry(0).stamp) == "Day 1 08:00");
    CHECK(std::string((const char*)loaded.Entry(1).text) == "Outpost liberated");
    REQUIRE(loaded.ObjectiveCount() == 2);
    CHECK(loaded.FindObjective("recruit") == 0);
    CHECK(loaded.Objective(0).state == JODone);
    CHECK(loaded.Objective(1).state == JOActive);
    REQUIRE(loaded.StatusCount() == 1);
    CHECK(std::string((const char*)loaded.Status(0).text) == "Petra (CORPORAL)");
    CHECK(loaded.Revision() != rev); // load bumps the revision so the open map repaints

    std::filesystem::remove(archivePath);
}

TEST_CASE("Journal - diary entries carry a zone tag and a kind", "[game][guerrilla][journal]")
{
    Journal journal;
    journal.AddEntry("Day 3 14:02", "Went RED. QRF moving on last known position.", "Airfield", JKDanger);
    journal.AddEntry("Day 3 14:05", "plain line");
    REQUIRE(journal.EntryCount() == 2);
    CHECK(std::string((const char*)journal.Entry(0).zone) == "Airfield");
    CHECK(journal.Entry(0).kind == JKDanger);
    CHECK(journal.Entry(1).zone.GetLength() == 0);
    CHECK(journal.Entry(1).kind == JKPlain);
    // out-of-range kinds clamp to plain
    journal.AddEntry("", "odd", "", 99);
    CHECK(journal.Entry(2).kind == JKPlain);
    CHECK(Journal::EntryKindFromName("danger") == JKDanger);
    CHECK(Journal::EntryKindFromName("GOOD") == JKGood);
    CHECK(Journal::EntryKindFromName("warn") == JKWarn);
    CHECK(Journal::EntryKindFromName("plain") == JKPlain);
    CHECK(Journal::EntryKindFromName("bogus") == -1);
}

namespace
{
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
    JournalRosterRow you;
    you.name = "You";
    you.isPlayer = true;
    you.rank = 2;
    you.role = "Leader";
    you.primary = "AK-74";
    in.roster.Add(you);
    JournalRosterRow petra;
    petra.name = "Petra";
    petra.rank = 1;
    petra.role = "Rifleman";
    petra.primary = "PK";
    in.roster.Add(petra);
    JournalRosterRow andre;
    andre.name = "Andre";
    andre.rank = 0;
    andre.role = "Rifleman";
    andre.primary = "AK-74";
    andre.launcher = "RPG-75";
    andre.wounded = 40;
    in.roster.Add(andre);
    JournalRosterRow squad;
    squad.name = "Outpost squad";
    squad.withPlayer = false;
    squad.zone = "Outpost";
    squad.count = 3;
    squad.role = "Rifleman";
    squad.primary = "AK-74";
    in.roster.Add(squad);
    return in;
}

// true when `text` carries `a` before `b`
bool Before(const std::string& text, const char* a, const char* b)
{
    const size_t pa = text.find(a);
    const size_t pb = text.find(b);
    return pa != std::string::npos && pb != std::string::npos && pa < pb;
}

bool IsRedInk(const PackedColor& c)
{
    return c.R8() == 150 && c.G8() == 22 && c.B8() == 18;
}

std::string S(const RString& s)
{
    return std::string((const char*)s);
}

bool EndsWith(const std::string& text, const char* tail)
{
    const size_t n = std::strlen(tail);
    return text.size() >= n && text.compare(text.size() - n, n, tail) == 0;
}

// hrefs of the bottom-pinned (footer) fields of the section named `name`
std::vector<std::string> FooterHrefs(JournalHtml& html, const char* name)
{
    std::vector<std::string> out;
    const int s = html.FindSection(name);
    if (s < 0)
    {
        return out;
    }
    const HTMLSection& sec = html.GetSection(s);
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        if (sec.fields[f].bottom && sec.fields[f].href.GetLength() > 0)
        {
            out.push_back(S(sec.fields[f].href));
        }
    }
    return out;
}

// the text of the bottom-pinned field carrying `href` ("" when absent)
std::string FooterText(JournalHtml& html, const char* name, const char* href)
{
    const int s = html.FindSection(name);
    if (s < 0)
    {
        return std::string();
    }
    const HTMLSection& sec = html.GetSection(s);
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        if (sec.fields[f].bottom && S(sec.fields[f].href) == href)
        {
            return S(sec.fields[f].text);
        }
    }
    return std::string();
}

// true when any field of the section named `name` carries `href`
bool HasHref(JournalHtml& html, const char* name, const char* href)
{
    const int s = html.FindSection(name);
    if (s < 0)
    {
        return false;
    }
    const HTMLSection& sec = html.GetSection(s);
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        if (S(sec.fields[f].href) == href)
        {
            return true;
        }
    }
    return false;
}

// the whole document read page by page (every section under its own name)
std::string AllText(JournalHtml& html)
{
    std::string out;
    for (int s = 0; s < html.NSections(); s++)
    {
        const HTMLSection& sec = html.GetSection(s);
        if (sec.names.Size() > 0)
        {
            out += html.Text(sec.names[0]);
            out += '\n';
        }
    }
    return out;
}

// a chain read as one string: base, base_2, base_3 ...
std::string ChainText(JournalHtml& html, const char* base)
{
    std::string out = html.Text(base);
    for (int part = 2; part < 20; part++)
    {
        char name[64];
        snprintf(name, sizeof(name), "%s_%d", base, part);
        if (html.FindSection(name) < 0)
        {
            break;
        }
        out += ' ';
        out += html.Text(name);
    }
    return out;
}

// the page set every campaign renders: fixed pages and their legacy aliases
struct NamedPage
{
    const char* name;
    const char* alias; // nullptr when the page has no legacy anchor
};
const NamedPage kFixedPages[] = {
    {"Main", "GM_CONTENTS"},          {"GM_DISPATCH", nullptr},  {"Plan", "GM_OPERATIONS"},  {"GM_OBJECTIVES", nullptr},
    {"GM_ACTIONS", nullptr},          {"GM_SUPPLY", nullptr},    {"GM_FACTION", nullptr},    {"GM_PEOPLE", "GM_CELL"},
    {"GM_ROSTER", nullptr},           {"GM_PLACES", "GM_ZONES"}, {"GM_CHRONICLES", nullptr}, {"GM_RECORD", "GM_LOG"},
    {"GM_REFERENCE", "GM_MAN_INDEX"},
};
const int kFixedPageCount = (int)(sizeof(kFixedPages) / sizeof(kFixedPages[0]));

// every block's first field (the field after a break, or field 0) starts a
// row: rows never straddle blocks, the premise of the per-block budget
bool EveryBlockStartsARow(const HTMLSection& sec)
{
    for (int f = 0; f < sec.fields.Size(); f++)
    {
        if (f > 0 && !sec.fields[f - 1].nextline)
        {
            continue;
        }
        bool startsRow = false;
        for (int r = 0; r < sec.rows.Size() && !startsRow; r++)
        {
            startsRow = sec.rows[r].firstField == f;
        }
        if (!startsRow)
        {
            return false;
        }
    }
    return true;
}

// every fixed page and handbook chapter resolves, every alias lands on the
// page-0 section of its page, no page name is a SplitSection "<name>/<n>",
// and no page overruns the container's height
void CheckPageSet(JournalHtml& html)
{
    for (const NamedPage& page : kFixedPages)
    {
        CAPTURE(page.name);
        const int s = html.FindSection(page.name);
        CHECK(s >= 0);
        if (page.alias)
        {
            CHECK(html.FindSection(page.alias) == s);
        }
    }
    REQUIRE(GuerrillaManualTopicCount() == 10);
    for (int i = 0; i < GuerrillaManualTopicCount(); i++)
    {
        CHECK(std::strlen(GuerrillaManualTopicTitle(i)) > 0);
        CHECK(html.FindSection(GuerrillaManualTopicAnchor(i)) >= 0);
    }
    for (int s = 0; s < html.NSections(); s++)
    {
        const HTMLSection& sec = html.GetSection(s);
        float rows = 0;
        for (int r = 0; r < sec.rows.Size(); r++)
        {
            rows += sec.rows[r].height;
        }
        CHECK(rows <= html.GetPageHeight());
        for (int n = 0; n < sec.names.Size(); n++)
        {
            CHECK(S(sec.names[n]).find('/') == std::string::npos);
        }
    }
}
} // namespace

TEST_CASE("Journal pages - every page renders from inputs and reads as a dossier", "[game][guerrilla][journal][ui]")
{
    Journal journal;
    journal.AddEntry("Day 1 08:00", "Reached the Camp alone.", "Camp", JKPlain);
    journal.AddEntry("Day 1 16:22", "Liberated. Garrison gone; holding squad in place.", "Outpost", JKGood);
    journal.AddEntry("Day 3 14:02", "Went RED. QRF moving on last known position.", "Airfield", JKDanger);
    journal.SetObjective("firstRecruit", "Take a first recruit at the Camp (1 HR)", JODone);
    journal.SetObjective("hqEstablish", "Set up a headquarters in a town or at the Camp", JOActive);
    journal.SetStatus("Companions", "Petra (CORPORAL)");
    journal.SetStatus("Standard issue", "AK-74, PK");
    journal.SetStatus("Next pattern", "AKS-74U - 3 of 25 captured");

    const JournalPageInputs in = SampleInputs();
    JournalHtml html;
    BuildGuerrillaJournalPages(&html, journal, in);

    // the page set: thirteen fixed pages, the chapters, one page per zone,
    // plus whatever continuation pages the lists needed (never fewer)
    CheckPageSet(html);
    CHECK(html.FindSection("GM_ZONE_0") >= 0);
    CHECK(html.FindSection("GM_ZONE_8") >= 0);
    CHECK(html.NSections() >= kFixedPageCount + GuerrillaManualTopicCount() + in.zones.Size());
    // no page is named twice (a continuation carries its own name)
    for (const NamedPage& page : kFixedPages)
    {
        int hits = 0;
        for (int s = 0; s < html.NSections(); s++)
        {
            const HTMLSection& sec = html.GetSection(s);
            for (int n = 0; n < sec.names.Size(); n++)
            {
                if (stricmp(sec.names[n], page.name) == 0)
                {
                    hits++;
                }
            }
        }
        CHECK(hits == 1);
    }

    // CONTENTS ("Main", the Notes tab): the title, the campaign line, the six
    // sections; nothing of the old dashboard or the old Notes page
    const std::string contents = html.Text("Main");
    CHECK(contents.find("Resistance Dossier") != std::string::npos);
    CHECK(contents.find("Malden. FIA against the Soviet Army. Day 3, 14:20.") != std::string::npos);
    CHECK(contents.find("Dispatches") != std::string::npos);
    CHECK(contents.find("Operations") != std::string::npos);
    CHECK(contents.find("People") != std::string::npos);
    CHECK(contents.find("Places") != std::string::npos);
    CHECK(contents.find("Chronicles") != std::string::npos);
    CHECK(contents.find("Reference") != std::string::npos);
    CHECK(Before(contents, "Dispatches", "Operations"));
    CHECK(Before(contents, "Operations", "People"));
    CHECK(Before(contents, "People", "Places"));
    CHECK(Before(contents, "Places", "Chronicles"));
    CHECK(Before(contents, "Chronicles", "Reference"));
    CHECK(contents.find("SITUATION") == std::string::npos);
    CHECK(contents.find("|") == std::string::npos);
    CHECK(contents.find("Treasury") == std::string::npos);
    CHECK(contents.find("ALERT RED") == std::string::npos);
    CHECK(HasHref(html, "Main", "#GM_DISPATCH"));
    CHECK(HasHref(html, "Main", "#Plan"));
    CHECK(HasHref(html, "Main", "#GM_PEOPLE"));
    CHECK(HasHref(html, "Main", "#GM_PLACES"));
    CHECK(HasHref(html, "Main", "#GM_CHRONICLES"));
    CHECK(HasHref(html, "Main", "#GM_REFERENCE"));

    // FOOTERS: Contents ends in the pencil word with no link to itself; a
    // zone page links Contents and its parent; the old seven-name strip is
    // gone everywhere
    CHECK(EndsWith(contents, "Contents"));
    CHECK_FALSE(HasHref(html, "Main", "#Main"));
    CHECK(FooterHrefs(html, "Main").empty());
    {
        const std::vector<std::string> want = {"#Main", "#GM_PLACES"};
        CHECK(FooterHrefs(html, "GM_ZONE_3") == want);
        CHECK(FooterText(html, "GM_ZONE_3", "#Main") == "Contents");
        CHECK(FooterText(html, "GM_ZONE_3", "#GM_PLACES") == "Places");
    }
    {
        const std::vector<std::string> want = {"#Main", "#Plan"};
        CHECK(FooterHrefs(html, "GM_SUPPLY") == want);
        CHECK(FooterText(html, "GM_SUPPLY", "#Plan") == "Operations");
    }
    {
        // a handbook chapter: Contents, the Reference index, then prev / next
        // walking on into the neighbouring CHAPTERS once this chapter's own
        // pages run out (it is one page on this fixture).  That is the whole
        // of the chapter navigation: the body carries none
        const std::vector<std::string> want = {"#Main", "#GM_REFERENCE", "#GM_MAN_COMPANIONS", "#GM_MAN_SAVE"};
        CHECK(FooterHrefs(html, "GM_MAN_UNDERCOVER") == want);
        CHECK(FooterText(html, "GM_MAN_UNDERCOVER", "#GM_MAN_COMPANIONS") == "prev");
        CHECK(FooterText(html, "GM_MAN_UNDERCOVER", "#GM_MAN_SAVE") == "next");
        // the ends of the handbook stop: no prev on the first chapter, no
        // next on the last
        const std::vector<std::string> wantFirst = {"#Main", "#GM_REFERENCE", "#GM_MAN_ZONES"};
        CHECK(FooterHrefs(html, "GM_MAN_MODE") == wantFirst);
        const std::vector<std::string> wantLast = {"#Main", "#GM_REFERENCE", "#GM_MAN_UNDERCOVER"};
        CHECK(FooterHrefs(html, "GM_MAN_SAVE") == wantLast);
    }
    // a top-level page's parent IS Contents: the footer links it once, never
    // "Contents - Contents"
    {
        const std::vector<std::string> want = {"#Main"};
        CHECK(FooterHrefs(html, "GM_DISPATCH") == want);
        CHECK(FooterText(html, "GM_DISPATCH", "#Main") == "Contents");
        CHECK(FooterHrefs(html, "Plan") == want);
        CHECK(FooterHrefs(html, "GM_PEOPLE") == want);
        CHECK(FooterHrefs(html, "GM_CHRONICLES") == want);
        CHECK(FooterHrefs(html, "GM_REFERENCE") == want);
        // the nine-zone Places chain: Contents once, then next
        const std::vector<std::string> wantPlaces = {"#Main", "#GM_PLACES_2"};
        CHECK(FooterHrefs(html, "GM_PLACES") == wantPlaces);
        CHECK(html.Text("GM_DISPATCH").find("Contents - Contents") == std::string::npos);
    }
    CHECK(AllText(html).find("Notes - Plan - Zones - Cell - Resistance - Diary - Handbook") == std::string::npos);

    // DISPATCHES: three typed rows and the cover line in the hand
    const std::string dispatch = html.Text("GM_DISPATCH");
    CHECK(dispatch.find("Dispatches Day 3, 14:20.") != std::string::npos);
    CHECK(dispatch.find("Threat: Airfield went RED. A quick reaction force is out toward our last known position, "
                        "2.4 km NE.") != std::string::npos);
    CHECK(dispatch.find("Objective: Hold every base. 1 of 3.") != std::string::npos);
    CHECK(dispatch.find("Latest: 14:02 Airfield. Went RED. QRF moving on last known position.") != std::string::npos);
    CHECK(dispatch.find("My cover is blown: one patrol knows my face.") != std::string::npos);
    CHECK(Before(dispatch, "Threat:", "Objective:"));
    CHECK(Before(dispatch, "Objective:", "Latest:"));
    CHECK(Before(dispatch, "Latest:", "My cover is blown"));
    {
        // the blown cover is the page's one remark in the hand, in red; the
        // threat and latest rows are typed red too
        const HTMLSection& sec = html.GetSection(html.FindSection("GM_DISPATCH"));
        int redHandLines = 0;
        int redTypeRows = 0;
        for (int f = 0; f < sec.fields.Size(); f++)
        {
            const HTMLField& fld = sec.fields[f];
            if (S(fld.text).find("my face") != std::string::npos)
            {
                CHECK(fld.format == HFH6);
                CHECK(fld.hasColor);
                CHECK(IsRedInk(fld.color));
                redHandLines++;
            }
            else if (fld.format == HFP && fld.hasColor && IsRedInk(fld.color))
            {
                redTypeRows++;
            }
        }
        CHECK(redHandLines == 1);
        CHECK(redTypeRows == 2);
    }
    // the old Notes paragraphs are re-voiced, not copied
    CHECK(dispatch.find("We are three") == std::string::npos);
    CHECK(dispatch.find("We hold") == std::string::npos);
    CHECK(dispatch.find("Companions:") == std::string::npos);
    CHECK(dispatch.find("Standard issue") == std::string::npos);
    CHECK(dispatch.find("Petra") == std::string::npos);

    // OPERATIONS ("Plan", the Plan tab): the hub, one page with four links
    const std::string plan = html.Text("Plan");
    CHECK(plan.find("Operations 3 objectives open. Day 3, 14:20.") != std::string::npos);
    CHECK(plan.find("Objectives") != std::string::npos);
    CHECK(plan.find("Suggested actions") != std::string::npos);
    CHECK(plan.find("Supplies") != std::string::npos);
    CHECK(plan.find("Resistance strength") != std::string::npos);
    CHECK(HasHref(html, "Plan", "#GM_OBJECTIVES"));
    CHECK(HasHref(html, "Plan", "#GM_ACTIONS"));
    CHECK(HasHref(html, "Plan", "#GM_SUPPLY"));
    CHECK(HasHref(html, "Plan", "#GM_FACTION"));
    CHECK(html.FindSection("Plan_2") < 0);

    // OBJECTIVES: the open count, the open lines, the done list
    const std::string objectives = ChainText(html, "GM_OBJECTIVES");
    CHECK(objectives.find("3 objectives open. Day 3, 14:20.") != std::string::npos); // two engine rows + hqEstablish
    CHECK(objectives.find("Hold every base. 1 of 3.") != std::string::npos);
    CHECK(objectives.find("Raise every town. 1 of 5.") != std::string::npos);
    CHECK(objectives.find("Set up a headquarters in a town or at the Camp.") != std::string::npos);
    CHECK(objectives.find("Done Take a first recruit at the Camp (1 HR).") != std::string::npos);
    CHECK(Before(objectives, "Open", "Hold every base"));
    CHECK(Before(objectives, "Set up a headquarters", "Done"));

    // SUGGESTED ACTIONS: the next moves in priority order (seven moves: five
    // on the first page, two on the continuation)
    const std::string actions = ChainText(html, "GM_ACTIONS");
    CHECK(html.FindSection("GM_ACTIONS_2") >= 0);
    CHECK(actions.find("7 moves. Day 3, 14:20.") != std::string::npos);
    CHECK(actions.find("Break contact. Airfield is RED, a QRF is out. 2.4 km NE.") != std::string::npos);
    CHECK(actions.find("Go dark. One patrol knows my face: stow the weapon, lose or drop the witnesses.") !=
          std::string::npos);
    CHECK(actions.find("Outpost, heat 55, on our own ground. Reinforce or pull the squad. 1.3 km N.") !=
          std::string::npos);
    CHECK(actions.find("Raise Chapoi. Support 66, line 60. 2 occupiers in town: clear or wait them out. 3.1 km S.") !=
          std::string::npos);
    CHECK(actions.find("Finish securing Airfield. 40% secured. Fighters inside; garrison 9 still on the field. "
                       "2.4 km NE.") != std::string::npos);
    CHECK(actions.find("Target Seaport. Garrison 6, alert YELLOW. 2.1 km NE.") != std::string::npos);
    CHECK(actions.find("Target Airfield") == std::string::npos); // a base being secured is not a fresh target
    CHECK(actions.find("Recruit at the Camp. 11 HR in reserve, 1 HR a fighter.") != std::string::npos);
    CHECK(Before(actions, "Break contact", "Go dark"));
    CHECK(Before(actions, "Go dark", "Outpost, heat 55"));
    CHECK(Before(actions, "Outpost, heat 55", "Raise Chapoi"));
    CHECK(Before(actions, "Raise Chapoi", "Finish securing Airfield"));
    CHECK(Before(actions, "Finish securing Airfield", "Target Seaport"));
    CHECK(Before(actions, "Target Seaport", "Recruit at the Camp"));
    CHECK(actions.find("URGENT") == std::string::npos);    // no tags: order and ink carry the priority
    CHECK(actions.find("Sweep due") == std::string::npos); // no claims the game cannot make
    {
        const std::vector<std::string> want = {"#Main", "#Plan", "#GM_ACTIONS_2"};
        CHECK(FooterHrefs(html, "GM_ACTIONS") == want);
        const std::vector<std::string> want2 = {"#Main", "#Plan", "#GM_ACTIONS"};
        CHECK(FooterHrefs(html, "GM_ACTIONS_2") == want2);
        CHECK(FooterText(html, "GM_ACTIONS", "#GM_ACTIONS_2") == "next");
        CHECK(FooterText(html, "GM_ACTIONS_2", "#GM_ACTIONS") == "prev");
        CHECK(html.Text("GM_ACTIONS_2").find("Suggested actions, continued.") != std::string::npos);
    }

    // SUPPLIES: the cell's economy, typed
    const std::string supply = html.Text("GM_SUPPLY");
    CHECK(supply.find("Treasury: 480 R.") != std::string::npos);
    CHECK(supply.find("Manpower: 11 HR, pool 12.") != std::string::npos);
    CHECK(supply.find("Income every 10 min: +35 R, +1 HR, from Houdan, Outpost.") != std::string::npos);
    CHECK(supply.find("Arms dealers: Houdan, La Trinite (occupied).") != std::string::npos);
    CHECK(supply.find("Vehicle dealers: Chapoi (rising).") != std::string::npos);
    CHECK(supply.find("Standard issue") == std::string::npos); // arms live on the roster

    // RESISTANCE STRENGTH: the ladder, ground, organisation stubs, all typed
    const std::string res = html.Text("GM_FACTION");
    CHECK(res.find("War level 2 of 10.") != std::string::npos);
    CHECK(res.find("Island held: 25%. Level 3 at 40%; the ladder runs 20, 40, 55, 70, 85.") != std::string::npos);
    CHECK(res.find("The occupier steps up at WL 3, WL 5: better troops, heavier vehicles, sharper eyes.") !=
          std::string::npos);
    CHECK(res.find("Towns: 1 risen, 1 rising, 1 neutral, 1 occupied, 1 unscouted.") != std::string::npos);
    CHECK(res.find("Bases: 1 held, 1 contested, 1 occupied.") != std::string::npos);
    CHECK(res.find("Occupier under arms: 21 known. Airfield 9, Chapoi 2, Seaport 6, La Trinite 4.") !=
          std::string::npos);
    CHECK(res.find("Ours under arms: 6. 3 with me, 3 holding, 11 HR in reserve.") != std::string::npos);
    CHECK(res.find("Heat on our ground: 24 mean over 3 zones.") != std::string::npos);
    CHECK(res.find("Cells: 1, ours. Headquarters at Houdan.") != std::string::npos);
    CHECK(res.find("Holdings: 1 cache, 2 garaged.") != std::string::npos);
    CHECK(res.find("No allied cells. No outside contact.") != std::string::npos);

    // PEOPLE: the index of the named; in Change 1 a single entry, the roster
    const std::string people = html.Text("GM_PEOPLE");
    CHECK(people.find("People 6 under arms, 1 wounded.") != std::string::npos);
    CHECK(people.find("The roster") != std::string::npos);
    CHECK(HasHref(html, "GM_PEOPLE", "#GM_ROSTER"));
    CHECK(html.Text("GM_CELL") == people); // the legacy anchor reads the same page

    // ROSTER: per-fighter rows (a wrapping name, never a cell), then the Arms
    // block.  GetHtmlText joins the two rows of a fighter into one string
    const std::string roster = ChainText(html, "GM_ROSTER");
    CHECK(roster.find("The roster 6 under arms, 1 wounded.") != std::string::npos);
    CHECK(roster.find("With me You Sgt Leader, AK-74.") != std::string::npos);
    CHECK(roster.find("Petra Cpl Rifleman, PK.") != std::string::npos);
    CHECK(roster.find("Andre Pvt Rifleman, AK-74 + RPG-75, WIA 40%.") != std::string::npos);
    CHECK(roster.find("Holding Outpost Outpost squad x3 Rifleman, AK-74.") != std::string::npos);
    CHECK(roster.find("Standard issue: AK-74, PK.") != std::string::npos);
    CHECK(roster.find("Next pattern: AKS-74U - 3 of 25 captured.") != std::string::npos);
    CHECK(roster.find("Cache: Houdan, at the headquarters.") != std::string::npos);
    CHECK(roster.find("Garage: UAZ, Ural.") != std::string::npos);
    CHECK(Before(roster, "With me", "Holding Outpost"));
    CHECK(Before(roster, "Outpost squad", "Standard issue"));
    {
        const std::vector<std::string> want = {"#Main", "#GM_PEOPLE"};
        CHECK(FooterHrefs(html, "GM_ROSTER") == want);
        CHECK(FooterText(html, "GM_ROSTER", "#GM_PEOPLE") == "People");
    }

    // PLACES: a typed index grouped by state, nearest first, linking to zone
    // pages; nine zones at five a page
    const std::string places = ChainText(html, "GM_PLACES");
    CHECK(html.FindSection("GM_PLACES_2") >= 0);
    CHECK(html.FindSection("GM_PLACES_3") < 0);
    CHECK(places.find("8 of 9 scouted.") != std::string::npos);
    CHECK(places.find("Ours Camp - 0.4 km SW") != std::string::npos); // "camp" not repeated after "Camp"
    CHECK(places.find("Houdan - town, 0.9 km E") != std::string::npos);
    CHECK(places.find("Outpost - heat 55, 1.3 km N") != std::string::npos);
    CHECK(places.find("Contested Airfield - 40% secured, garrison 9, RED, 2.4 km NE") != std::string::npos);
    CHECK(places.find("Neutral Chapoi - town, ready to rise, support 66, 2 occupiers in town, 3.1 km S") !=
          std::string::npos);
    CHECK(places.find("Vigny - town, support 12, line 60, 5.2 km S") != std::string::npos);
    CHECK(places.find("Occupied Seaport - port, garrison 6, YELLOW, 2.1 km NE") != std::string::npos);
    CHECK(places.find("Unscouted Le Port - town, 6.3 km NW") != std::string::npos);
    CHECK(Before(places, "Ours", "Contested"));
    CHECK(Before(places, "Contested", "Neutral"));
    CHECK(Before(places, "Chapoi", "Vigny")); // nearest first within a group
    CHECK(HasHref(html, "GM_PLACES", "#GM_ZONE_3"));
    CHECK(html.Text("GM_ZONES") == html.Text("GM_PLACES")); // the legacy anchor reads page 1

    // one page per zone: typed facts + the zone's own record in the hand
    const std::string airfield = html.Text("GM_ZONE_3");
    CHECK(airfield.find("Airfield Contested airfield. Day 3, 14:20.") != std::string::npos);
    CHECK(airfield.find("State: SECURING, garrison 9.") != std::string::npos);
    CHECK(airfield.find("Alert: RED, a quick reaction force is out.") != std::string::npos);
    CHECK(airfield.find("Capture: 40% secured.") != std::string::npos);
    CHECK(airfield.find("Heat: 48, aware.") != std::string::npos);
    CHECK(airfield.find("Distance: 2.4 km NE.") != std::string::npos);
    CHECK(airfield.find("Last seen: 14:02.") != std::string::npos);
    CHECK(airfield.find("Record") != std::string::npos);
    CHECK(airfield.find("14:02 Went RED.") != std::string::npos);
    CHECK(airfield.find("Liberated.") == std::string::npos); // another zone's line
    const std::string lePort = html.Text("GM_ZONE_8");
    CHECK(lePort.find("Le Port Unscouted town.") != std::string::npos);
    CHECK(lePort.find("Not scouted yet") != std::string::npos);
    CHECK(lePort.find("Distance: 6.3 km NW.") != std::string::npos);
    CHECK(lePort.find("Nothing written about this place yet.") != std::string::npos);
    const std::string houdan = html.Text("GM_ZONE_1");
    CHECK(houdan.find("Houdan Our town.") != std::string::npos);
    CHECK(houdan.find("Support: 82, risen.") != std::string::npos);
    CHECK(houdan.find("Here: headquarters, 2 garaged, arms dealer.") != std::string::npos);

    // CHRONICLES: the hub over the record; no History link before Change 2
    const std::string chronicles = html.Text("GM_CHRONICLES");
    CHECK(chronicles.find("Chronicles 3 entries.") != std::string::npos);
    CHECK(chronicles.find("The record") != std::string::npos);
    CHECK(HasHref(html, "GM_CHRONICLES", "#GM_RECORD"));
    CHECK_FALSE(HasHref(html, "GM_CHRONICLES", "#GM_HISTORY"));
    CHECK(html.FindSection("GM_HISTORY") < 0);

    // THE RECORD: newest first, grouped by day, times only under a day head
    const std::string record = ChainText(html, "GM_RECORD");
    CHECK(record.find("3 entries.") != std::string::npos);
    CHECK(Before(record, "Day 3", "Day 1"));
    CHECK(record.find("14:02 Airfield. Went RED.") != std::string::npos);
    CHECK(record.find("16:22 Outpost. Liberated.") != std::string::npos);
    CHECK(record.find("08:00 Camp. Reached the Camp alone.") != std::string::npos);
    CHECK(Before(record, "Went RED.", "Reached the Camp"));
    CHECK(html.Text("GM_LOG") == html.Text("GM_RECORD"));
    {
        const std::vector<std::string> want = {"#Main", "#GM_CHRONICLES"};
        CHECK(FooterHrefs(html, "GM_RECORD") == want);
    }

    // REFERENCE: the index links every chapter; a chapter carries its table,
    // the live cover state and the chapter nav
    const std::string reference = html.Text("GM_REFERENCE");
    CHECK(reference.find("Reference Notes from the old hands.") != std::string::npos);
    CHECK(reference.find(GuerrillaManualTopicTitle(0)) != std::string::npos);
    CHECK(reference.find(GuerrillaManualTopicTitle(9)) != std::string::npos);
    for (int i = 0; i < GuerrillaManualTopicCount(); i++)
    {
        CHECK(HasHref(html, "GM_REFERENCE", (std::string("#") + GuerrillaManualTopicAnchor(i)).c_str()));
    }
    CHECK(html.Text("GM_MAN_INDEX") == reference);
    const std::string undercover = html.Text("GM_MAN_UNDERCOVER");
    CHECK(undercover.find("Undercover How the occupier reads you. Handbook 9 of 10.") != std::string::npos);
    CHECK(undercover.find("YOU ARE SEEN AS RANGE") != std::string::npos);
    CHECK(undercover.find("Rifle slung SUSPECTED under 20 m, or from behind") != std::string::npos);
    CHECK(undercover.find("Your standing: now BLOWN, 1 patrol knows your face") != std::string::npos);
    // the chapter's own body has no navigation: the neighbouring chapters and
    // the index are reached from the bottom-pinned footer, so the page never
    // shows two rows of links
    CHECK(undercover.find("< Companions") == std::string::npos);
    CHECK(undercover.find("Keeping the record >") == std::string::npos);
    CHECK(HasHref(html, "GM_MAN_UNDERCOVER", "#GM_REFERENCE"));
    CHECK(HasHref(html, "GM_MAN_UNDERCOVER", "#GM_MAN_COMPANIONS"));
    CHECK(HasHref(html, "GM_MAN_UNDERCOVER", "#GM_MAN_SAVE"));

    // ink discipline: the hand is inked (blue-black or red), the type is the
    // stock colour or pencil; a picture only ever on a portrait dossier
    // (GM_WHO_*, none in Change 1); no bar or pip fields anywhere
    int hand = 0;
    int redHand = 0;
    std::vector<std::string> names;
    names.push_back(S(in.resistanceName));
    names.push_back(S(in.occupierName));
    for (int i = 0; i < in.roster.Size(); i++)
    {
        names.push_back(S(in.roster[i].name));
    }
    for (int i = 0; i < in.zones.Size(); i++)
    {
        names.push_back(S(in.zones[i].name));
    }
    for (int s = 0; s < html.NSections(); s++)
    {
        const HTMLSection& sec = html.GetSection(s);
        const std::string page = sec.names.Size() > 0 ? S(sec.names[0]) : std::string();
        const bool portraitPage = page.find("GM_WHO_") == 0;
        const bool noClipPage = page == "GM_PEOPLE" || page.find("GM_ROSTER") == 0 || page.find("GM_PLACES") == 0;
        for (int f = 0; f < sec.fields.Size(); f++)
        {
            const HTMLField& fld = sec.fields[f];
            const std::string text = S(fld.text);
            // no em dashes anywhere in the player text
            CHECK(text.find("\xE2\x80\x94") == std::string::npos);
            if (fld.format == HFImg)
            {
                CHECK(portraitPage);
            }
            if (fld.format == HFH6)
            {
                CHECK(fld.hasColor);
                hand++;
                if (IsRedInk(fld.color))
                {
                    redHand++;
                }
            }
            // a name is never a fixed cell (a cell neither wraps nor clips)
            for (const std::string& name : names)
            {
                if (text == name)
                {
                    CHECK(fld.tableWidth == 0.0f);
                }
            }
            // and nothing on the people / roster / places pages is cut to "..."
            if (noClipPage)
            {
                CHECK_FALSE(EndsWith(text, "..."));
            }
        }
    }
    CHECK(hand > 20);
    // the hand is written 1.6x the typed body size (the parser-only container's P is 1)
    CHECK(html.GetFormatSize(HFH6) == Catch::Approx(1.6f * html.GetPHeight()));
    // the title and the head never share a format
    CHECK(html.GetFormatSize(HFH3) == Catch::Approx(1.45f * html.GetPHeight()));
    CHECK(html.GetFormatSize(HFH4) == Catch::Approx(1.15f * html.GetPHeight()));
    CHECK(html.GetFormatSize(HFH3) != html.GetFormatSize(HFH4));
    {
        // a Record page's day separator (head) and its title ride different slots
        const HTMLSection& sec = html.GetSection(html.FindSection("GM_RECORD"));
        HTMLFormat titleFormat = HFP;
        HTMLFormat dayFormat = HFP;
        for (int f = 0; f < sec.fields.Size(); f++)
        {
            const std::string text = S(sec.fields[f].text);
            if (text == "The record" && sec.fields[f].href.GetLength() == 0 && !sec.fields[f].bottom)
            {
                titleFormat = sec.fields[f].format;
            }
            else if (text == "Day 3")
            {
                dayFormat = sec.fields[f].format;
            }
        }
        CHECK(titleFormat == HFH3);
        CHECK(dayFormat == HFH4);
        CHECK(titleFormat != dayFormat);
    }
    CHECK(redHand >= 3);               // the cover line, the urgent moves, the RED entries
    CHECK_FALSE(html.HasFieldColor()); // the pending ink never leaks past a call
}

TEST_CASE("Journal pages - a quiet campaign reads quiet", "[game][guerrilla][journal][ui]")
{
    Journal journal;
    JournalPageInputs in = SampleInputs();
    for (int i = 0; i < in.zones.Size(); i++)
    {
        in.zones[i].alert = 0;
    }
    in.undercoverStatus = 0;
    JournalHtml html;
    BuildGuerrillaJournalPages(&html, journal, in);
    CheckPageSet(html);
    const std::string dispatch = html.Text("GM_DISPATCH");
    CHECK(dispatch.find("Threat: The garrisons are quiet.") != std::string::npos);
    CHECK(dispatch.find("To the occupier I am still a civilian.") != std::string::npos);
    CHECK(dispatch.find("went RED") == std::string::npos);
    CHECK(dispatch.find("my face") == std::string::npos);
    CHECK(dispatch.find("Latest: Nothing written yet.") != std::string::npos);
    const std::string actions = ChainText(html, "GM_ACTIONS");
    CHECK(actions.find("Break contact") == std::string::npos);
    CHECK(actions.find("Go dark") == std::string::npos);
    const std::string places = ChainText(html, "GM_PLACES");
    CHECK(places.find("RED") == std::string::npos);
    CHECK(places.find("YELLOW") == std::string::npos);
    // no red ink anywhere on Dispatches: the threat row and the cover line
    // are stock and hand
    {
        const HTMLSection& sec = html.GetSection(html.FindSection("GM_DISPATCH"));
        for (int f = 0; f < sec.fields.Size(); f++)
        {
            CHECK_FALSE((sec.fields[f].hasColor && IsRedInk(sec.fields[f].color)));
        }
    }
}

TEST_CASE("Journal pages - the objectives-open count follows the page", "[game][guerrilla][journal][ui]")
{
    Journal journal;
    JournalPageInputs in = SampleInputs();
    in.militaryHeld = in.militaryTotal; // every base held: that row moves to Done
    JournalHtml html;
    BuildGuerrillaJournalPages(&html, journal, in);
    const std::string objectives = ChainText(html, "GM_OBJECTIVES");
    CHECK(objectives.find("1 objective open.") != std::string::npos);
    CHECK(objectives.find("Hold every base") == std::string::npos);
    CHECK(objectives.find("Every base held.") != std::string::npos);
    CHECK(objectives.find("Raise every town. 1 of 5.") != std::string::npos);
    CHECK(Before(objectives, "Raise every town", "Done"));
    CHECK(Before(objectives, "Done", "Every base held."));
    // the hub carries the same count
    CHECK(html.Text("Plan").find("1 objective open.") != std::string::npos);

    in.townsRisen = in.townsTotal; // and every town: nothing open, both done
    JournalHtml done;
    BuildGuerrillaJournalPages(&done, journal, in);
    const std::string all = ChainText(done, "GM_OBJECTIVES");
    CHECK(all.find("0 objectives open.") != std::string::npos);
    CHECK(all.find("Every town risen.") != std::string::npos);
    CHECK(done.Text("Plan").find("0 objectives open.") != std::string::npos);
}

TEST_CASE("Journal pages - a zone's record shows its latest lines and points at the record for the rest",
          "[game][guerrilla][journal][ui]")
{
    Journal journal;
    for (int i = 1; i <= 13; i++)
    {
        char stamp[32];
        char text[32];
        snprintf(stamp, sizeof(stamp), "Day 1 %02d:00", i);
        snprintf(text, sizeof(text), "Airfield line %d.", i);
        journal.AddEntry(stamp, text, "Airfield", JKPlain);
    }
    journal.AddEntry("Day 1 20:00", "Elsewhere.", "Camp", JKPlain);
    const JournalPageInputs in = SampleInputs();
    JournalHtml html;
    BuildGuerrillaJournalPages(&html, journal, in);
    CheckPageSet(html);
    const std::string airfield = html.Text("GM_ZONE_3");
    CHECK(airfield.find("Airfield line 13.") != std::string::npos); // newest first
    CHECK(airfield.find("Airfield line 4.") != std::string::npos);  // the tenth
    CHECK(airfield.find("Airfield line 3.") == std::string::npos);  // past the cap
    CHECK(airfield.find("Elsewhere.") == std::string::npos);        // another zone's line
    CHECK(airfield.find("Nothing written about this place yet.") == std::string::npos);
    // (GetHtmlText joins the three runs with spaces: "... in the record .")
    CHECK(airfield.find("3 earlier lines in the record") != std::string::npos);
    // the overflow row's "record" is a link to the record page, following the count
    html.SelectSection("GM_ZONE_3");
    const HTMLSection& sec = html.GetSection(html.CurrentSection());
    bool linked = false;
    for (int f = 1; f < sec.fields.Size(); f++)
    {
        if (S(sec.fields[f].href) == "#GM_RECORD" && S(sec.fields[f - 1].text) == "3 earlier lines in the ")
        {
            linked = true;
            CHECK(S(sec.fields[f].text) == "record");
            CHECK_FALSE(sec.fields[f].bottom);
        }
    }
    CHECK(linked);
    // the whole record is on the record pages: fourteen entries at five a
    // page make three, and the chain's prev / next run through them
    const std::string record = ChainText(html, "GM_RECORD");
    CHECK(html.FindSection("GM_RECORD_2") >= 0);
    CHECK(html.FindSection("GM_RECORD_3") >= 0);
    CHECK(html.FindSection("GM_RECORD_4") < 0);
    CHECK(record.find("Airfield line 1.") != std::string::npos);
    CHECK(record.find("14 entries.") != std::string::npos);
    CHECK(html.Text("GM_RECORD").find("Elsewhere.") != std::string::npos); // the newest, on page 1
    CHECK(html.Text("GM_RECORD_3").find("Airfield line 1.") != std::string::npos);
    CHECK(html.Text("GM_RECORD_2").find("The record, continued.") != std::string::npos);
    CHECK(html.Text("GM_RECORD_2").find("Day 1") != std::string::npos); // a day head tops every continuation
    {
        const std::vector<std::string> want = {"#Main", "#GM_CHRONICLES", "#GM_RECORD_2"};
        CHECK(FooterHrefs(html, "GM_RECORD") == want);
        const std::vector<std::string> want2 = {"#Main", "#GM_CHRONICLES", "#GM_RECORD", "#GM_RECORD_3"};
        CHECK(FooterHrefs(html, "GM_RECORD_2") == want2);
        CHECK(FooterText(html, "GM_RECORD_2", "#GM_RECORD") == "prev");
        CHECK(FooterText(html, "GM_RECORD_2", "#GM_RECORD_3") == "next");
        const std::vector<std::string> want3 = {"#Main", "#GM_CHRONICLES", "#GM_RECORD_2"};
        CHECK(FooterHrefs(html, "GM_RECORD_3") == want3);
    }
    // the legacy anchor lands on page 1 of the chain only
    CHECK(html.FindSection("GM_LOG") == html.FindSection("GM_RECORD"));
}

namespace
{

// the overflow campaign: fourteen entries (thirteen Airfield lines and one
// elsewhere) and a script note past the hand cap, tagged with a zone so both
// the record and the zone's own record carry it clamped
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
    journal.AddEntry("Day 3 15:10",
                     "Walked the whole perimeter fence twice before dawn and counted every truck, every tent and "
                     "every sentry post the garrison has put up since the last time we looked in on them",
                     "Airfield", JKPlain);
    journal.SetObjective("long",
                         "Carry the word to every village between the coast and the ridge so that no family "
                         "there is left wondering which side of the road the war is on tonight",
                         JOActive);
}

// seven fighters with long names everywhere a name can go
JournalPageInputs OverflowInputs()
{
    JournalPageInputs in = SampleInputs();
    in.resistanceName = "Popular Front for the Liberation of the Sinai Peninsula";
    in.occupierName = "Frontier Corps of the Third Field Army";
    in.zones[2].name = "Ras Nasrani Outpost";
    in.zones[4].name = "Marjayoun Barracks";
    in.roster[1].name = "Sgt. Petra \"The Wall\" Novakova of Houdan";
    const char* names[] = {"Marek", "Jana", "Tomas"};
    for (const char* name : names)
    {
        JournalRosterRow row;
        row.name = name;
        row.rank = 0;
        row.role = "Rifleman";
        row.primary = "AK-74";
        in.roster.Add(row);
    }
    return in;
}

} // namespace

TEST_CASE("Journal pages - at a tight page height the composed document still keeps every page under the budget",
          "[game][guerrilla][journal][ui]")
{
    // the invariants that hold at ANY height: every section within the
    // container's height, every block starting a row of its own, no
    // SplitSection "<name>/<n>" name, no arrow image; the fixed pages and
    // aliases still resolve.  One-page acceptance is deliberately absent
    // (see the JournalHtml note): the height here is tight to force cuts,
    // not to model the notepad
    auto Check = [](const Journal& journal, const JournalPageInputs& in, const char* label)
    {
        CAPTURE(label);
        JournalHtml html;
        // budget 8.5 P, footer 2 P: about six typed rows to a page
        html.pageHeight = 12 * html.GetPHeight();
        BuildGuerrillaJournalPages(&html, journal, in);
        CheckPageSet(html);
        for (int s = 0; s < html.NSections(); s++)
        {
            const HTMLSection& sec = html.GetSection(s);
            CAPTURE(sec.names.Size() > 0 ? S(sec.names[0]) : std::string());
            CHECK(EveryBlockStartsARow(sec));
            for (int f = 0; f < sec.fields.Size(); f++)
            {
                CHECK(sec.fields[f].format != HFImg);
            }
        }
        // the height did bite: the first handbook chapter (sixteen-odd rows)
        // and the record continue on Render's own "_2" pages
        CHECK(html.FindSection("GM_MAN_MODE_2") >= 0);
        CHECK(html.FindSection("GM_RECORD_2") >= 0);
    };
    {
        Journal journal;
        journal.AddEntry("Day 1 08:00", "Reached the Camp alone.", "Camp", JKPlain);
        journal.AddEntry("Day 1 16:22", "Liberated. Garrison gone; holding squad in place.", "Outpost", JKGood);
        journal.AddEntry("Day 3 14:02", "Went RED. QRF moving on last known position.", "Airfield", JKDanger);
        journal.SetObjective("hqEstablish", "Set up a headquarters in a town or at the Camp", JOActive);
        Check(journal, SampleInputs(), "default");
    }
    {
        Journal journal;
        FillOverflowJournal(journal);
        Check(journal, OverflowInputs(), "overflow");
        // the over-long note was clamped, not dropped, on both pages that carry it
        JournalHtml html;
        BuildGuerrillaJournalPages(&html, journal, OverflowInputs());
        const std::string record = ChainText(html, "GM_RECORD");
        CHECK(record.find("Walked the whole perimeter fence") != std::string::npos);
        CHECK(record.find("looked in on them") == std::string::npos);
        const std::string airfield = html.Text("GM_ZONE_3");
        CHECK(airfield.find("Walked the whole perimeter fence") != std::string::npos);
        CHECK(airfield.find("looked in on them") == std::string::npos);
    }
}

TEST_CASE("Journal pages - an existing Main/Plan section is appended to, not duplicated",
          "[game][guerrilla][journal][ui]")
{
    JournalHtml html;
    html.LoadBuffer("inline.html", R"html(<html><body>
        <h1><a name="Main"></a>Mission notes</h1>
        <p>Authored notes.</p>
        <hr>
        <h1><a name="Plan"></a>Authored plan</h1>
    </body></html>)html");
    REQUIRE(html.NSections() == 2);

    Journal journal;
    JournalPageInputs in;
    in.islandName = "Malden";
    BuildGuerrillaJournalPages(&html, journal, in);

    CheckPageSet(html);
    CHECK(html.NSections() >= kFixedPageCount + GuerrillaManualTopicCount()); // no zones: no zone pages
    CHECK(html.FindSection("GM_ZONE_0") < 0);
    CHECK(html.FindSection("Main") == 0); // the authored section is reused, not duplicated
    CHECK(html.FindSection("Plan") == 1);
    const std::string contents = html.Text("Main");
    CHECK(contents.find("Authored notes.") != std::string::npos);
    CHECK(contents.find("Resistance Dossier") != std::string::npos);
    CHECK(contents.find("Malden.") != std::string::npos); // no clock: the campaign line alone
    CHECK(contents.find("Day ") == std::string::npos);
    CHECK(Before(contents, "Authored notes.", "Resistance Dossier"));
    const std::string plan = html.Text("Plan");
    CHECK(plan.find("Authored plan") != std::string::npos);
    CHECK(plan.find("Operations") != std::string::npos);
    CHECK(plan.find("Objectives") != std::string::npos);
    CHECK(Before(plan, "Authored plan", "Operations"));
    // the empty campaign still says something on every page
    CHECK(html.Text("GM_DISPATCH").find("Objective: Nothing open.") != std::string::npos);
    CHECK(html.Text("GM_DISPATCH").find("Latest: Nothing written yet.") != std::string::npos);
    const std::string actions = ChainText(html, "GM_ACTIONS");
    CHECK(actions.find("Scout the island.") != std::string::npos);
    CHECK(actions.find("Set up a headquarters. Any town, or the Camp.") != std::string::npos);
    CHECK(html.Text("GM_RECORD").find("Nothing written yet.") != std::string::npos);
}

TEST_CASE("HTML control extensions - field colour and hanging indent land in the model",
          "[game][guerrilla][journal][ui]")
{
    JournalHtml html;
    int sec = html.AddSection();
    html.AddName(sec, "T");
    html.SetFieldColor(PackedColor(1, 2, 3, 255));
    html.AddText(sec, "red", HFP, HALeft, false, false, "");
    html.ClearFieldColor();
    html.AddText(sec, "plain", HFP, HALeft, false, false, "");
    html.SetHanging(30);
    html.AddText(sec, "hang", HFP, HALeft, false, false, "");
    html.SetHanging(0);
    const HTMLSection& section = html.GetSection(sec);
    REQUIRE(section.fields.Size() == 3);
    CHECK(section.fields[0].hasColor);
    CHECK(section.fields[0].color.R8() == 1);
    CHECK(section.fields[0].color.G8() == 2);
    CHECK(section.fields[0].color.B8() == 3);
    CHECK_FALSE(section.fields[1].hasColor);
    CHECK(section.fields[2].hanging == 30.0f);
    CHECK(section.fields[1].hanging == 0.0f);
    // the parser-only container reports the stock format slots; a slot can be
    // resized, and rebound to a face (a null face is a no-op)
    CHECK(html.GetFormatSize(HFP) > 0);
    html.SetFormatSize(HFH6, 2.5f);
    CHECK(html.GetFormatSize(HFH6) == Catch::Approx(2.5f));
    html.SetFormatFont(HFH6, nullptr, nullptr, 9.0f);
    CHECK(html.GetFormatSize(HFH6) == Catch::Approx(2.5f));
    html.FormatSection(sec);
    CHECK(section.rows.Size() >= 1);
}
