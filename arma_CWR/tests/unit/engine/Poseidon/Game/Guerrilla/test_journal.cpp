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

TEST_CASE("Journal pages - Notes/Plan/diary/manual sections render from inputs", "[game][guerrilla][journal][ui]")
{
    Journal journal;
    journal.AddEntry("Day 1 08:00", "The campaign begins.");
    journal.AddEntry("Day 2 14:20", "Outpost liberated");
    journal.SetObjective("recruit", "Recruit a fighter at the Camp", JODone);
    journal.SetStatus("Companions", "Petra (CORPORAL)");

    JournalPageInputs in;
    in.islandName = "Malden";
    in.resistanceName = "GUER";
    in.occupierName = "EAST";
    in.stamp = "Day 2 15:00";
    in.resources = 140;
    in.manpower = 3;
    in.warLevel = 2;
    in.economyKnown = true;
    in.militaryTotal = 2;
    in.militaryHeld = 1;
    in.townsTotal = 5;
    in.townsRisen = 0;
    in.townsReady.Add(RString("Houdan"));
    in.securing.Add(RString("Airfield 40%"));
    in.targets.Add(RString("Airfield: AIRFIELD, garrison 8, 900 m away"));
    in.alertMax = 2;
    in.alertZone = "Airfield";
    in.heatMax = 55;
    in.heatZone = "Outpost";
    in.undercoverArmed = true;
    in.undercoverStatus = 2;
    in.undercoverWitnesses = 1;
    in.cellSize = 4;
    in.stashCount = 1;
    in.useImages = false; // no texture loader in a unit test

    JournalHtml html;
    BuildGuerrillaJournalPages(&html, journal, in);

    // every page exists exactly once
    CHECK(html.FindSection("Main") >= 0);
    CHECK(html.FindSection("Plan") >= 0);
    CHECK(html.FindSection("GM_LOG") >= 0);
    REQUIRE(GuerrillaManualTopicCount() > 0);
    for (int i = 0; i < GuerrillaManualTopicCount(); i++)
    {
        CHECK(std::strlen(GuerrillaManualTopicTitle(i)) > 0);
    }
    CHECK(html.FindSection("GM_MAN_MODE") >= 0);
    CHECK(html.FindSection("GM_MAN_UNDERCOVER") >= 0);
    CHECK(html.FindSection("GM_MAN_SAVE") >= 0);
    CHECK(html.NSections() == 3 + GuerrillaManualTopicCount());

    const std::string notes = html.Text("Main");
    CHECK(notes.find("Malden campaign") != std::string::npos);
    CHECK(notes.find("Treasury: 140 R") != std::string::npos);
    CHECK(notes.find("Manpower: 3 HR") != std::string::npos);
    CHECK(notes.find("War Level: 2") != std::string::npos);
    CHECK(notes.find("Military zones held: 1 of 2") != std::string::npos);
    CHECK(notes.find("Towns risen: 0 of 5") != std::string::npos);
    CHECK(notes.find("Ready to rise: Houdan") != std::string::npos);
    CHECK(notes.find("Securing: Airfield 40%") != std::string::npos);
    CHECK(notes.find("Alert: RED at Airfield") != std::string::npos);
    CHECK(notes.find("Heat: 55 at Outpost") != std::string::npos);
    CHECK(notes.find("Cover: blown, 1 witness group(s)") != std::string::npos);
    CHECK(notes.find("Fighters with you: 4") != std::string::npos);
    CHECK(notes.find("Arms stashes: 1") != std::string::npos);
    CHECK(notes.find("Companions: Petra (CORPORAL)") != std::string::npos);
    CHECK(notes.find("Outpost liberated") != std::string::npos);          // latest diary lines inline
    CHECK(notes.find(GuerrillaManualTopicTitle(0)) != std::string::npos); // manual index

    const std::string plan = html.Text("Plan");
    CHECK(plan.find("Liberate Malden") != std::string::npos);
    CHECK(plan.find("[ ] Hold every military zone (1 of 2)") != std::string::npos);
    CHECK(plan.find("[ ] Raise every town (0 of 5)") != std::string::npos);
    CHECK(plan.find("[x] Recruit a fighter at the Camp") != std::string::npos);
    CHECK(plan.find("Break contact: Airfield is RED") != std::string::npos);
    CHECK(plan.find("Houdan is ready to rise") != std::string::npos);
    CHECK(plan.find("Finish securing Airfield 40%") != std::string::npos);
    CHECK(plan.find("Target: Airfield: AIRFIELD, garrison 8, 900 m away") != std::string::npos);
    CHECK(plan.find("Recruit at the Camp: 3 HR available") != std::string::npos);
    CHECK(plan.find("Lie low near Outpost") != std::string::npos);

    const std::string log = html.Text("GM_LOG");
    // newest first
    CHECK(log.find("Outpost liberated") < log.find("The campaign begins."));
    CHECK(log.find("Back to notes") != std::string::npos);

    // manual pages carry their title and a way back
    const std::string manual = html.Text("GM_MAN_CAPTURE");
    CHECK(manual.find("Capturing a military zone") != std::string::npos);
    CHECK(manual.find("Back to notes") != std::string::npos);

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

    CHECK(html.NSections() == 2 + 1 + GuerrillaManualTopicCount());
    const std::string notes = html.Text("Main");
    CHECK(notes.find("Authored notes.") != std::string::npos);
    CHECK(notes.find("Field journal") != std::string::npos);
    CHECK(notes.find("Nothing recorded yet.") != std::string::npos);
    const std::string plan = html.Text("Plan");
    CHECK(plan.find("Authored plan") != std::string::npos);
    CHECK(plan.find("Liberate Malden") != std::string::npos);
    CHECK(plan.find("Scout the island") != std::string::npos);
}
