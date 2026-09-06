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
} // namespace

TEST_CASE("Journal pages - every page renders from inputs and reads as a diary", "[game][guerrilla][journal][ui]")
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

    // NOTES: the day's page - dated, the campaign line, the threat first,
    // the cell, the ground, the latest entries, the footer
    const std::string notes = html.Text("Main");
    CHECK(notes.find("Day 3, 14:20") != std::string::npos);
    CHECK(notes.find("Malden. FIA against the Soviet Army.") != std::string::npos);
    CHECK(notes.find("Airfield went RED. A quick reaction force is out toward our last known position, 2.4 km NE.") !=
          std::string::npos);
    CHECK(notes.find("Outpost is on edge, heat 55.") != std::string::npos);
    CHECK(notes.find("My cover is blown: one patrol knows my face.") != std::string::npos);
    CHECK(
        notes.find("We are three: me and two fighters, one wounded. Another three hold Outpost. "
                   "Treasury 480 R, manpower 11 HR, pool 12. Headquarters in a house at Houdan, 2 vehicles garaged.") !=
        std::string::npos);
    CHECK(notes.find("Companions: Petra (CORPORAL).") != std::string::npos);
    CHECK(notes.find("Standard issue") == std::string::npos); // arms live on the Cell page
    CHECK(notes.find("We hold 1 of 3 bases and 1 of 5 towns. War level 2 of 10. "
                     "Chapoi is ready to rise, support 66. Airfield is 40% secured.") != std::string::npos);
    CHECK(notes.find("Latest") != std::string::npos);
    CHECK(notes.find("14:02 Airfield. Went RED.") != std::string::npos);    // today: time only
    CHECK(notes.find("D1 16:22 Outpost. Liberated.") != std::string::npos); // older: day + time
    CHECK(Before(notes, "Airfield went RED", "We are three"));
    CHECK(Before(notes, "We are three", "We hold 1 of 3"));
    CHECK(Before(notes, "Latest", "14:02 Airfield"));
    CHECK(notes.find("Notes - Plan - Zones - Cell - Resistance - Diary - Handbook") != std::string::npos);
    // nothing of the old dashboard survives
    CHECK(notes.find("SITUATION") == std::string::npos);
    CHECK(notes.find("ALERT RED") == std::string::npos);
    CHECK(notes.find("|") == std::string::npos);

    // PLAN: the objectives, the done list, the next moves in priority order
    const std::string plan = html.Text("Plan");
    CHECK(plan.find("3 objectives open. Day 3, 14:20.") != std::string::npos); // two engine rows + hqEstablish
    CHECK(plan.find("Hold every base. 1 of 3.") != std::string::npos);
    CHECK(plan.find("Raise every town. 1 of 5.") != std::string::npos);
    CHECK(plan.find("Set up a headquarters in a town or at the Camp.") != std::string::npos);
    CHECK(plan.find("Done Take a first recruit at the Camp (1 HR).") != std::string::npos);
    CHECK(plan.find("Break contact. Airfield is RED, a QRF is out. 2.4 km NE.") != std::string::npos);
    CHECK(plan.find("Go dark. One patrol knows my face: stow the weapon, lose or drop the witnesses.") !=
          std::string::npos);
    CHECK(plan.find("Outpost, heat 55, on our own ground. Reinforce or pull the squad. 1.3 km N.") !=
          std::string::npos);
    CHECK(plan.find("Raise Chapoi. Support 66, line 60. 2 occupiers in town: clear or wait them out. 3.1 km S.") !=
          std::string::npos);
    CHECK(plan.find("Finish securing Airfield. 40% secured. Fighters inside; garrison 9 still on the field. "
                    "2.4 km NE.") != std::string::npos);
    CHECK(plan.find("Target Seaport. Garrison 6, alert YELLOW. 2.1 km NE.") != std::string::npos);
    CHECK(plan.find("Target Airfield") == std::string::npos); // a base being secured is not a fresh target
    CHECK(plan.find("Recruit at the Camp. 11 HR in reserve, 1 HR a fighter.") != std::string::npos);
    CHECK(Before(plan, "Break contact", "Go dark"));
    CHECK(Before(plan, "Go dark", "Outpost, heat 55"));
    CHECK(Before(plan, "Outpost, heat 55", "Raise Chapoi"));
    CHECK(Before(plan, "Raise Chapoi", "Finish securing Airfield"));
    CHECK(Before(plan, "Finish securing Airfield", "Target Seaport"));
    CHECK(Before(plan, "Target Seaport", "Recruit at the Camp"));
    CHECK(plan.find("URGENT") == std::string::npos);    // no tags: order and ink carry the priority
    CHECK(plan.find("Sweep due") == std::string::npos); // no claims the game cannot make

    // ZONES: a typed index grouped by state, nearest first, linking to zone pages
    const std::string zones = html.Text("GM_ZONES");
    CHECK(zones.find("8 of 9 scouted.") != std::string::npos);
    CHECK(zones.find("Ours Camp - 0.4 km SW") != std::string::npos); // "camp" not repeated after "Camp"
    CHECK(zones.find("Houdan - town, 0.9 km E") != std::string::npos);
    CHECK(zones.find("Outpost - heat 55, 1.3 km N") != std::string::npos);
    CHECK(zones.find("Contested Airfield - 40% secured, garrison 9, RED, 2.4 km NE") != std::string::npos);
    CHECK(zones.find("Neutral Chapoi - town, ready to rise, support 66, 2 occupiers in town, 3.1 km S") !=
          std::string::npos);
    CHECK(zones.find("Vigny - town, support 12, line 60, 5.2 km S") != std::string::npos);
    CHECK(zones.find("Occupied Seaport - port, garrison 6, YELLOW, 2.1 km NE") != std::string::npos);
    CHECK(zones.find("Unscouted Le Port - town, 6.3 km NW") != std::string::npos);
    CHECK(Before(zones, "Ours", "Contested"));
    CHECK(Before(zones, "Contested", "Neutral"));
    CHECK(Before(zones, "Chapoi", "Vigny")); // nearest first within a group
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

    // CELL: the roster typed in columns, arms from the status lines, supply
    const std::string cell = html.Text("GM_CELL");
    CHECK(cell.find("6 under arms, 1 wounded.") != std::string::npos);
    CHECK(cell.find("With me You Sgt Leader AK-74") != std::string::npos);
    CHECK(cell.find("Andre Pvt Rifleman AK-74 + RPG-75 WIA 40%") != std::string::npos);
    CHECK(cell.find("Holding Outpost Outpost squad x3 Rifleman AK-74") != std::string::npos);
    CHECK(cell.find("Standard issue: AK-74, PK.") != std::string::npos);
    CHECK(cell.find("Next pattern: AKS-74U - 3 of 25 captured.") != std::string::npos);
    CHECK(cell.find("Cache: Houdan, at the headquarters.") != std::string::npos);
    CHECK(cell.find("Garage: UAZ, Ural.") != std::string::npos);
    CHECK(cell.find("Treasury: 480 R.") != std::string::npos);
    CHECK(cell.find("Manpower: 11 HR, pool 12.") != std::string::npos);
    CHECK(cell.find("Income every 10 min: +35 R, +1 HR, from Houdan, Outpost.") != std::string::npos);
    CHECK(cell.find("Arms dealers: Houdan, La Trinite (occupied).") != std::string::npos);
    CHECK(cell.find("Vehicle dealers: Chapoi (rising).") != std::string::npos);

    // RESISTANCE: the ladder, ground, organisation stubs, all typed
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

    // DIARY: newest first, grouped by day, times only under a day head
    const std::string log = html.Text("GM_LOG");
    CHECK(log.find("3 entries.") != std::string::npos);
    CHECK(Before(log, "Day 3", "Day 1"));
    CHECK(log.find("14:02 Airfield. Went RED.") != std::string::npos);
    CHECK(log.find("16:22 Outpost. Liberated.") != std::string::npos);
    CHECK(log.find("08:00 Camp. Reached the Camp alone.") != std::string::npos);
    CHECK(Before(log, "Went RED.", "Reached the Camp"));

    // HANDBOOK: index links every chapter; a chapter carries its table, the
    // live cover state and the chapter nav
    const std::string index = html.Text("GM_MAN_INDEX");
    CHECK(index.find("Handbook Notes from the old hands.") != std::string::npos);
    CHECK(index.find(GuerrillaManualTopicTitle(0)) != std::string::npos);
    CHECK(index.find(GuerrillaManualTopicTitle(9)) != std::string::npos);
    const std::string undercover = html.Text("GM_MAN_UNDERCOVER");
    CHECK(undercover.find("Undercover How the occupier reads you. Handbook 9 of 10.") != std::string::npos);
    CHECK(undercover.find("YOU ARE SEEN AS RANGE") != std::string::npos);
    CHECK(undercover.find("Rifle slung SUSPECTED under 20 m, or from behind") != std::string::npos);
    CHECK(undercover.find("Your standing: now BLOWN, 1 patrol knows your face") != std::string::npos);
    CHECK(undercover.find("< Companions") != std::string::npos);
    CHECK(undercover.find("Keeping the record >") != std::string::npos);

    // ink discipline: the hand is inked (blue-black or red), the type is the
    // stock colour or pencil; no picture, bar or pip fields anywhere
    int hand = 0;
    int redHand = 0;
    for (int s = 0; s < html.NSections(); s++)
    {
        const HTMLSection& sec = html.GetSection(s);
        for (int f = 0; f < sec.fields.Size(); f++)
        {
            const HTMLField& fld = sec.fields[f];
            // no em dashes anywhere in the player text
            CHECK(std::string((const char*)fld.text).find("\xE2\x80\x94") == std::string::npos);
            CHECK(fld.format != HFImg);
            if (fld.format == HFH6)
            {
                CHECK(fld.hasColor);
                hand++;
                if (IsRedInk(fld.color))
                {
                    redHand++;
                }
            }
        }
    }
    CHECK(hand > 20);
    // the hand is written 1.6x the typed body size (the parser-only container's P is 1)
    CHECK(html.GetFormatSize(HFH6) == Catch::Approx(1.6f * html.GetPHeight()));
    CHECK(redHand >= 3);               // the RED paragraph, the blown cover, the RED entry
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
    const std::string notes = html.Text("Main");
    CHECK(notes.find("The garrisons are quiet.") != std::string::npos);
    CHECK(notes.find("To the occupier I am still a civilian.") != std::string::npos);
    CHECK(notes.find("went RED") == std::string::npos);
    CHECK(notes.find("Nothing written yet.") != std::string::npos);
    const std::string plan = html.Text("Plan");
    CHECK(plan.find("Break contact") == std::string::npos);
    CHECK(plan.find("Go dark") == std::string::npos);
    const std::string zones = html.Text("GM_ZONES");
    CHECK(zones.find("RED") == std::string::npos);
    CHECK(zones.find("YELLOW") == std::string::npos);
}

TEST_CASE("Journal pages - the objectives-open count follows the page", "[game][guerrilla][journal][ui]")
{
    Journal journal;
    JournalPageInputs in = SampleInputs();
    in.militaryHeld = in.militaryTotal; // every base held: that row moves to Done
    JournalHtml html;
    BuildGuerrillaJournalPages(&html, journal, in);
    const std::string plan = html.Text("Plan");
    CHECK(plan.find("1 objective open.") != std::string::npos);
    CHECK(plan.find("Hold every base") == std::string::npos);
    CHECK(plan.find("Every base held.") != std::string::npos);
    CHECK(plan.find("Raise every town. 1 of 5.") != std::string::npos);

    in.townsRisen = in.townsTotal; // and every town: nothing open, both done
    JournalHtml done;
    BuildGuerrillaJournalPages(&done, journal, in);
    const std::string all = done.Text("Plan");
    CHECK(all.find("0 objectives open.") != std::string::npos);
    CHECK(all.find("Every town risen.") != std::string::npos);
}

TEST_CASE("Journal pages - a zone's record shows its latest lines and points at the Diary for the rest",
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
    const std::string airfield = html.Text("GM_ZONE_3");
    CHECK(airfield.find("Airfield line 13.") != std::string::npos); // newest first
    CHECK(airfield.find("Airfield line 4.") != std::string::npos);  // the tenth
    CHECK(airfield.find("Airfield line 3.") == std::string::npos);  // past the cap
    CHECK(airfield.find("Elsewhere.") == std::string::npos);        // another zone's line
    CHECK(airfield.find("Nothing written about this place yet.") == std::string::npos);
    CHECK(airfield.find("3 earlier lines in the Diary") != std::string::npos);
    // the overflow row's "Diary" is a link to the Diary page (the footer has
    // one too; this one follows the count)
    html.SelectSection("GM_ZONE_3");
    const HTMLSection& sec = html.GetSection(html.CurrentSection());
    bool linked = false;
    for (int f = 1; f < sec.fields.Size(); f++)
    {
        if (std::string((const char*)sec.fields[f].href) == "#GM_LOG" &&
            std::string((const char*)sec.fields[f - 1].text) == "3 earlier lines in the ")
        {
            linked = true;
        }
    }
    CHECK(linked);
    // the whole record is still on the Diary page
    const std::string log = html.Text("GM_LOG");
    CHECK(log.find("Airfield line 1.") != std::string::npos);
    CHECK(log.find("14 entries.") != std::string::npos);
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
    CHECK(notes.find("Notes Malden.") != std::string::npos); // no clock: the page is titled Notes
    CHECK(notes.find("I am alone. No headquarters yet.") != std::string::npos);
    CHECK(notes.find("We hold") == std::string::npos); // no zones: no ground sentence
    CHECK(notes.find("Nothing written yet.") != std::string::npos);
    const std::string plan = html.Text("Plan");
    CHECK(plan.find("Authored plan") != std::string::npos);
    CHECK(plan.find("Scout the island.") != std::string::npos);
    CHECK(plan.find("Set up a headquarters. Any town, or the Camp.") != std::string::npos);
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
