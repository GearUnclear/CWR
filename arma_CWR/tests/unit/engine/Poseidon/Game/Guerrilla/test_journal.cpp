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

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

// parser-only container: page metrics without renderer services (same idea
// as TestHtmlContainer in UI/test_optionsUI.cpp)
class JournalHtml : public CHTMLContainer
{
  public:
    void SelectSection(const char* name) { _currentSection = FindSection(name); }
    // GetHtmlText joins fields with one space; the renderer lays them out
    // adjacent (a "Label: " field carries its own space), so collapse runs
    // of spaces to compare against the on-screen reading
    std::string Text(const char* name)
    {
        SelectSection(name);
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
    float GetPageHeight() const override { return 100000; }
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
} // namespace

TEST_CASE("Journal pages - every page renders from inputs and reads as designed", "[game][guerrilla][journal][ui]")
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
    ApplyGuerrillaJournalTheme(&html); // colours only without an engine
    BuildGuerrillaJournalPages(&html, journal, in);

    // every page exists exactly once: 7 tabs + index + chapters + one per zone
    CHECK(html.FindSection("Main") >= 0);
    CHECK(html.FindSection("Plan") >= 0);
    CHECK(html.FindSection("GM_ZONES") >= 0);
    CHECK(html.FindSection("GM_CELL") >= 0);
    CHECK(html.FindSection("GM_FACTION") >= 0);
    CHECK(html.FindSection("GM_LOG") >= 0);
    CHECK(html.FindSection("GM_MAN_INDEX") >= 0);
    REQUIRE(GuerrillaManualTopicCount() == 10);
    for (int i = 0; i < GuerrillaManualTopicCount(); i++)
    {
        CHECK(std::strlen(GuerrillaManualTopicTitle(i)) > 0);
        CHECK(html.FindSection(GuerrillaManualTopicAnchor(i)) >= 0);
    }
    CHECK(html.FindSection("GM_ZONE_0") >= 0);
    CHECK(html.FindSection("GM_ZONE_8") >= 0);
    CHECK(html.NSections() == 7 + GuerrillaManualTopicCount() + in.zones.Size());

    // SITUATION: masthead, alert strip, the four blocks, the nav row
    const std::string notes = html.Text("Main");
    CHECK(notes.find("SITUATION") != std::string::npos);
    CHECK(notes.find("FIA vs. Soviet Army") != std::string::npos);
    CHECK(notes.find("Day 3 14:20") != std::string::npos);
    CHECK(notes.find("ALERT RED - AIRFIELD") != std::string::npos);
    CHECK(notes.find("2.4 km NE - QRF OUT") != std::string::npos);
    CHECK(notes.find("Treasury 480 R") != std::string::npos);
    CHECK(notes.find("Manpower 11 HR pool 12") != std::string::npos);
    CHECK(notes.find("Fighters with you 3 you, 2 fighters - 1 WIA") != std::string::npos);
    CHECK(notes.find("Holding 3 Outpost squad") != std::string::npos);
    CHECK(notes.find("Headquarters Houdan - house - 2 garaged") != std::string::npos);
    CHECK(notes.find("Companions Petra (CORPORAL)") != std::string::npos);
    CHECK(notes.find("Standard issue") == std::string::npos); // arms live on the Cell page
    CHECK(notes.find("Bases held 1 / 3") != std::string::npos);
    CHECK(notes.find("Towns risen 1 / 5") != std::string::npos);
    CHECK(notes.find("War level 2 / 10") != std::string::npos);
    CHECK(notes.find("Ready to rise 66 SUP Chapoi") != std::string::npos);
    CHECK(notes.find("Securing 40 % Airfield") != std::string::npos);
    CHECK(notes.find("QRF 1 out from Airfield - on last known position") != std::string::npos);
    CHECK(notes.find("Garrisons YELLOW Seaport") != std::string::npos);
    CHECK(notes.find("Heat, highest 55 Outpost") != std::string::npos);
    CHECK(notes.find("Cover BLOWN - known to 1 patrol") != std::string::npos);
    CHECK(notes.find("14:02 AIRFIELD Went RED.") != std::string::npos);    // today: time only
    CHECK(notes.find("D1 16:22 OUTPOST Liberated.") != std::string::npos); // older: day + time
    CHECK(notes.find("Situation | Plan | Zones | Cell | Resistance | Diary | Handbook") != std::string::npos);

    // PLAN: open objectives with progress, the done list, tagged moves in priority order
    const std::string plan = html.Text("Plan");
    CHECK(plan.find("Hold every base 1 / 3") != std::string::npos);
    CHECK(plan.find("Raise every town 1 / 5") != std::string::npos);
    CHECK(plan.find("Set up a headquarters") != std::string::npos);
    CHECK(plan.find("DONE") != std::string::npos);
    CHECK(plan.find("Take a first recruit") != std::string::npos);
    const size_t urgent = plan.find("URGENT Break contact. Airfield RED, QRF out.");
    const size_t dark = plan.find("URGENT Go dark. 1 patrol knows your face.");
    const size_t sweep = plan.find("URGENT Outpost heat 55. On our own holding.");
    const size_t raise = plan.find("READY Raise Chapoi. Support 66, line 60. 2 occupiers in town");
    const size_t secure = plan.find("READY Finish securing Airfield. 40% secured.");
    const size_t target = plan.find("ROUTINE Target Seaport. Garrison 6, alert YELLOW.");
    const size_t recruit = plan.find("ROUTINE Recruit at the Camp. 11 HR in reserve");
    REQUIRE(urgent != std::string::npos);
    REQUIRE(recruit != std::string::npos);
    CHECK(urgent < dark);
    CHECK(dark < sweep);
    CHECK(sweep < raise);
    CHECK(raise < secure);
    CHECK(secure < target);
    CHECK(target < recruit);
    CHECK(plan.find("Sweep due") == std::string::npos); // no claims the game cannot make

    // ZONES: a light index grouped by state, nearest first, linking to zone pages
    const std::string zones = html.Text("GM_ZONES");
    CHECK(zones.find("8 of 9 scouted") != std::string::npos);
    CHECK(zones.find("OURS Camp CAMP") != std::string::npos);
    CHECK(zones.find("CONTESTED Airfield AIR 40% secured") != std::string::npos);
    CHECK(zones.find("NEUTRAL Chapoi TOWN support 66 - line 60") != std::string::npos);
    CHECK(zones.find("OCCUPIED Seaport PORT") != std::string::npos);
    CHECK(zones.find("UNSCOUTED Le Port TOWN") != std::string::npos);
    CHECK(zones.find("OURS") < zones.find("CONTESTED"));
    CHECK(zones.find("CONTESTED") < zones.find("NEUTRAL"));
    CHECK(zones.find("Chapoi") < zones.find("Vigny")); // nearest first within a group
    {
        html.SelectSection("GM_ZONES");
        const HTMLSection& sec = html.GetSection(html.CurrentSection());
        bool linked = false;
        for (int f = 0; f < sec.fields.Size(); f++)
        {
            if (std::string((const char*)sec.fields[f].href) == "#GM_ZONE_3")
            {
                linked = true;
            }
        }
        CHECK(linked);
    }

    // one page per zone: facts + the zone's own record
    const std::string airfield = html.Text("GM_ZONE_3");
    CHECK(airfield.find("AIRFIELD") != std::string::npos);
    CHECK(airfield.find("State SECURING - garrison 9") != std::string::npos);
    CHECK(airfield.find("Alert RED - a quick reaction force is out") != std::string::npos);
    CHECK(airfield.find("Capture 40 % securing") != std::string::npos);
    CHECK(airfield.find("Heat 48 aware") != std::string::npos);
    CHECK(airfield.find("Distance 2.4 km NE") != std::string::npos);
    CHECK(airfield.find("Last seen 14:02") != std::string::npos);
    CHECK(airfield.find("RECORD") != std::string::npos);
    CHECK(airfield.find("Went RED.") != std::string::npos);
    CHECK(airfield.find("Liberated.") == std::string::npos); // another zone's line
    const std::string lePort = html.Text("GM_ZONE_8");
    CHECK(lePort.find("Not scouted yet") != std::string::npos);
    CHECK(lePort.find("Nothing written about this place yet.") != std::string::npos);
    const std::string houdan = html.Text("GM_ZONE_1");
    CHECK(houdan.find("Here headquarters, 2 garaged - arms dealer") != std::string::npos);

    // CELL: roster grouped, arms from the status lines, supply
    const std::string cell = html.Text("GM_CELL");
    CHECK(cell.find("6 under arms - 1 WIA") != std::string::npos);
    CHECK(cell.find("WITH YOU You Sgt Leader AK-74") != std::string::npos);
    CHECK(cell.find("Andre Pvt Rifleman AK-74 + RPG-75 WIA 40%") != std::string::npos);
    CHECK(cell.find("HOLDING - OUTPOST Outpost squad x3 Rifleman AK-74") != std::string::npos);
    CHECK(cell.find("Standard issue AK-74, PK") != std::string::npos);
    CHECK(cell.find("Next pattern AKS-74U - 3 of 25 captured") != std::string::npos);
    CHECK(cell.find("Garage 2 UAZ, Ural") != std::string::npos);
    CHECK(cell.find("Income / 10 min +35 R Houdan, Outpost") != std::string::npos);
    CHECK(cell.find("Arms dealers Houdan - La Trinite OCCUPIED") != std::string::npos);
    CHECK(cell.find("Vehicle dealers Chapoi RISING") != std::string::npos);

    // RESISTANCE: the ladder, ground, organisation stubs
    const std::string res = html.Text("GM_FACTION");
    CHECK(res.find("Island held 25 % WL 3 at 40") != std::string::npos);
    CHECK(res.find("20 40 55 70 85") != std::string::npos);
    CHECK(res.find("Occupier steps up at WL 3, WL 5") != std::string::npos);
    CHECK(res.find("Towns 1 risen - 1 rising - 1 neutral - 1 occupied - 1 unscouted") != std::string::npos);
    CHECK(res.find("Bases 1 held - 1 contested - 1 occupied") != std::string::npos);
    CHECK(res.find("Occupier under arms 21 known: Airfield 9, Chapoi 2, Seaport 6, La Trinite 4") != std::string::npos);
    CHECK(res.find("Cells 1 yours - HQ Houdan") != std::string::npos);
    CHECK(res.find("No allied cells. No outside contact.") != std::string::npos);

    // DIARY: newest first, grouped by day
    const std::string log = html.Text("GM_LOG");
    CHECK(log.find("3 entries") != std::string::npos);
    CHECK(log.find("DAY 3") < log.find("DAY 1"));
    CHECK(log.find("Went RED.") < log.find("Reached the Camp"));

    // HANDBOOK: index links every chapter; a chapter carries its table, the
    // live cover state and the chapter nav
    const std::string index = html.Text("GM_MAN_INDEX");
    CHECK(index.find("HANDBOOK") != std::string::npos);
    CHECK(index.find(GuerrillaManualTopicTitle(0)) != std::string::npos);
    CHECK(index.find(GuerrillaManualTopicTitle(9)) != std::string::npos);
    const std::string undercover = html.Text("GM_MAN_UNDERCOVER");
    CHECK(undercover.find("UNDERCOVER") != std::string::npos);
    CHECK(undercover.find("Handbook 9 / 10") != std::string::npos);
    CHECK(undercover.find("YOU ARE SEEN AS RANGE") != std::string::npos);
    CHECK(undercover.find("Rifle slung SUSPECTED under 20 m, or from behind") != std::string::npos);
    CHECK(undercover.find("YOUR STANDING - NOW BLOWN - 1 PATROL KNOWS YOUR FACE") != std::string::npos);
    CHECK(undercover.find("< Companions") != std::string::npos);
    CHECK(undercover.find("Keeping the record >") != std::string::npos);

    // no em dashes anywhere in the player text
    for (int s = 0; s < html.NSections(); s++)
    {
        const HTMLSection& sec = html.GetSection(s);
        for (int f = 0; f < sec.fields.Size(); f++)
        {
            CHECK(std::string((const char*)sec.fields[f].text).find("\xE2\x80\x94") == std::string::npos);
        }
    }
}

TEST_CASE("Journal pages - no alert strip and no colour bleed when the campaign is quiet",
          "[game][guerrilla][journal][ui]")
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
    const std::string notes = html.Text("Main");
    CHECK(notes.find("ALERT RED") == std::string::npos);
    CHECK(notes.find("Garrisons all GREEN") != std::string::npos);
    CHECK(notes.find("Cover CLEAN - you pass as a civilian") != std::string::npos);
    CHECK(notes.find("Nothing recorded yet.") != std::string::npos);
    // the pending field colour never leaks past a call
    CHECK_FALSE(html.HasFieldColor());
    const std::string plan = html.Text("Plan");
    CHECK(plan.find("Break contact") == std::string::npos);
    CHECK(plan.find("Go dark") == std::string::npos);
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

    CHECK(html.NSections() == 7 + GuerrillaManualTopicCount()); // no zones: no zone pages
    const std::string notes = html.Text("Main");
    CHECK(notes.find("Authored notes.") != std::string::npos);
    CHECK(notes.find("SITUATION") != std::string::npos);
    CHECK(notes.find("Nothing recorded yet.") != std::string::npos);
    const std::string plan = html.Text("Plan");
    CHECK(plan.find("Authored plan") != std::string::npos);
    CHECK(plan.find("Scout the island.") != std::string::npos);
}

TEST_CASE("HTML control extensions - bars, rules, field colour and hanging indent land in the model",
          "[game][guerrilla][journal][ui]")
{
    JournalHtml html;
    int sec = html.AddSection();
    html.AddName(sec, "T");
    html.SetFieldColor(PackedColor(1, 2, 3, 255));
    html.AddText(sec, "red", HFP, HALeft, false, false, "");
    html.ClearFieldColor();
    html.AddText(sec, "plain", HFP, HALeft, false, false, "");
    HTMLField* bar = html.AddBar(sec, 0.4f, 320, 12, PackedColor(9, 9, 9, 255), PackedColor(1, 1, 1, 255));
    REQUIRE(bar);
    CHECK(bar->bar);
    CHECK(bar->format == HFImg);
    CHECK(bar->fill == 0.4f);
    CHECK(bar->width == Catch::Approx(0.5f));    // 320 / 640
    CHECK(bar->height == Catch::Approx(0.025f)); // 12 / 480
    HTMLField* clamped = html.AddBar(sec, 7.0f, 10, 10, PackedColor(9, 9, 9, 255), PackedColor(0, 0, 0, 0));
    CHECK(clamped->fill == 1.0f);
    HTMLField* rule = html.AddRule(sec, 4, PackedColor(9, 9, 9, 255));
    REQUIRE(rule);
    CHECK(rule->fill == 1.0f);
    CHECK(rule->width == Catch::Approx(1000.0f)); // full page width (1000 in the parser-only container)
    html.SetHanging(30);
    html.AddText(sec, "hang", HFP, HALeft, false, false, "");
    html.SetHanging(0);
    const HTMLSection& section = html.GetSection(sec);
    REQUIRE(section.fields.Size() == 6);
    CHECK(section.fields[0].hasColor);
    CHECK_FALSE(section.fields[1].hasColor);
    CHECK(section.fields[5].hanging == 30.0f);
    CHECK(section.fields[1].hanging == 0.0f);
    // layout still works with bar fields in the row
    html.FormatSection(sec);
    CHECK(section.rows.Size() >= 1);
}
