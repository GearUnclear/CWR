#include <Poseidon/UI/Options/OptionsScrollList.hpp>
#include <Poseidon/Foundation/Strings/Mbcs.hpp>
#include <Poseidon/UI/Options/OptionsShell.hpp>
#include <Poseidon/UI/Options/FlatMenuPage.hpp>
#include <Poseidon/UI/Options/ScrollListPage.hpp>
#include <Poseidon/Core/resincl.hpp>
#include <Poseidon/UI/OptionsUI.hpp>
#include <Poseidon/UI/Controls/UIControlsBase.hpp>
#include <Poseidon/UI/OptionsUICommon.hpp>
#include <Poseidon/UI/UITestEngine.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <system_error>

using namespace Poseidon;
namespace fs = std::filesystem;

namespace
{
struct TempOptionsDir
{
    fs::path root;

    TempOptionsDir()
    {
        const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
        root = fs::temp_directory_path() / ("poseidon_options_" + std::to_string(nonce));
        fs::create_directories(root);
    }

    ~TempOptionsDir()
    {
        std::error_code ec;
        fs::permissions(root, fs::perms::owner_all, fs::perm_options::add, ec);
        fs::remove_all(root, ec);
    }
};
} // namespace

TEST_CASE("optionsUI compiles", "[optionsUI][tier3]")
{
    REQUIRE(sizeof(AbstractOptionsUI) > 0);
}

class TestableOptionsShell : public OptionsShell
{
  public:
    TestableOptionsShell(bool enableSimulation, bool credits) : OptionsShell(nullptr, enableSimulation, credits) {}
};

class TestableOptionsPage : public OptionsPage
{
  public:
    using OptionsPage::ContainsCycleIdc;

    const char* TitleText() const override { return ""; }
    int DefaultFocusIdc() const override { return -1; }
    const char* ResourceClassName() const override { return ""; }
};

class TestSemanticControl : public IControl
{
  public:
    TestSemanticControl() : IControl(nullptr, 42) {}

    int GetType() override { return 0; }
    int GetStyle() override { return 0; }
    bool IsInside(float, float) override { return false; }
    void Move(float, float) override {}
    void OnDraw(float) override {}
};

class TestHtmlContainer : public CHTMLContainer
{
  public:
    void SelectSection(const char* name) { _currentSection = FindSection(name); }

    float GetPageWidth() const override { return 1000; }
    float GetPageHeight() const override { return 1000; }
    float GetTextWidth(float, Font*, const char* text) const override { return std::strlen(text); }
};

TEST_CASE("OptionsShell propagates the simulation flag to the display base", "[optionsUI][UI]")
{
    TestableOptionsShell pausedShell(false, false);
    CHECK(pausedShell.EnableSimulation() == false);
    CHECK(pausedShell.SimulationEnabled() == false);

    TestableOptionsShell runningShell(true, false);
    CHECK(runningShell.EnableSimulation() == true);
    CHECK(runningShell.SimulationEnabled() == true);
}

TEST_CASE("OptionsScrollList maps every slot-local control IDC back to its slot", "[optionsUI][UI]")
{
    for (int digit = 0; digit <= 9; ++digit)
        CHECK(OptionsScrollList::SlotForControlIdc(540 + digit) == 4);

    CHECK(OptionsScrollList::SlotForControlIdc(499) == -1);
    CHECK(OptionsScrollList::SlotForControlIdc(590) == -1);
    CHECK(OptionsScrollList::SlotForControlIdc(700) == -1);
}

TEST_CASE("OptionsScrollList row policy keeps disabled rows focusable but inert", "[optionsUI][UI]")
{
    CHECK(OptionsScrollList::CanRowReceiveFocus(OptionsScrollList::KindHeader) == false);
    CHECK(OptionsScrollList::CanRowReceiveFocus(OptionsScrollList::KindAction) == true);
    CHECK(OptionsScrollList::CanRowReceiveFocus(OptionsScrollList::KindSlider) == true);

    CHECK(OptionsScrollList::CanRowAdjustValue(OptionsScrollList::KindStepper, false) == true);
    CHECK(OptionsScrollList::CanRowAdjustValue(OptionsScrollList::KindBoolean, false) == true);
    CHECK(OptionsScrollList::CanRowAdjustValue(OptionsScrollList::KindSlider, false) == true);
    CHECK(OptionsScrollList::CanRowAdjustValue(OptionsScrollList::KindAction, false) == false);
    CHECK(OptionsScrollList::CanRowAdjustValue(OptionsScrollList::KindBinding, false) == false);
    CHECK(OptionsScrollList::CanRowAdjustValue(OptionsScrollList::KindStepper, true) == false);

    CHECK(OptionsScrollList::CanRowInvokeAction(OptionsScrollList::KindAction, false) == true);
    CHECK(OptionsScrollList::CanRowInvokeAction(OptionsScrollList::KindAction, true) == false);
    CHECK(OptionsScrollList::CanRowOpenBinding(OptionsScrollList::KindBinding, false) == true);
    CHECK(OptionsScrollList::CanRowOpenBinding(OptionsScrollList::KindBinding, true) == false);
}

TEST_CASE("UITestEngine returns semantic text when controls render a clipped marquee", "[optionsUI][UI]")
{
    TestSemanticControl ctrl;

    UITestEngine::SetSemanticControlText(&ctrl, "Particles & Volumetrics");
    CHECK(UITestEngine::GetControlText(&ctrl) == "Particles & Volumetrics");

    UITestEngine::ClearSemanticControlText(&ctrl);
    CHECK(UITestEngine::GetControlText(&ctrl).empty());
}

TEST_CASE("UITestEngine returns text from the current HTML section", "[ui][html]")
{
    TestHtmlContainer html;
    html.LoadBuffer("inline.html", R"html(<html><body>
        <h1><a name="End1"></a>Mission complete</h1>
        <p>Selected result</p>
        <hr>
        <h1><a name="End2"></a>Hidden alternate result</h1>
    </body></html>)html");

    REQUIRE(html.NSections() == 2);
    html.SelectSection("End1");
    CHECK(UITestEngine::GetHtmlText(html) == "Mission complete Selected result");
    html.SelectSection("End2");
    CHECK(UITestEngine::GetHtmlText(html) == "Hidden alternate result");
    html.SelectSection("Missing");
    CHECK(UITestEngine::GetHtmlText(html).empty());
}

TEST_CASE("OptionsScrollList::FormatCell clips idle cells and marquees focused overflow", "[optionsUI][UI]")
{
    char buf[80];

    // A value within the budget is passed through untouched, idle or focused.
    OptionsScrollList::FormatCell("UP", OptionsScrollList::kBindingAltInnerChars, false, 0, buf, sizeof(buf));
    CHECK(std::string(buf) == "UP");
    OptionsScrollList::FormatCell("UP", OptionsScrollList::kBindingAltInnerChars, true, 0, buf, sizeof(buf));
    CHECK(std::string(buf) == "UP");

    // "Left Shift" (10) overflows the 6-char alt budget: clipped to the head
    // when idle, and also at the start of the marquee cycle (offset 0).
    OptionsScrollList::FormatCell("Left Shift", OptionsScrollList::kBindingAltInnerChars, false, 0, buf, sizeof(buf));
    CHECK(std::string(buf) == "Left S");
    OptionsScrollList::FormatCell("Left Shift", OptionsScrollList::kBindingAltInnerChars, true, 0, buf, sizeof(buf));
    CHECK(std::string(buf) == "Left S");

    // After the start pause plus a few scroll steps the focused window has
    // advanced past the head, so it no longer reads "Left S".
    const DWORD advanced = (DWORD)OptionsScrollList::kPauseMs + (DWORD)OptionsScrollList::kScrollPeriodMs * 4;
    OptionsScrollList::FormatCell("Left Shift", OptionsScrollList::kBindingAltInnerChars, true, advanced, buf,
                                  sizeof(buf));
    CHECK(std::string(buf) != "Left S");

    // An idle cell never advances no matter the elapsed time.
    OptionsScrollList::FormatCell("Left Shift", OptionsScrollList::kBindingAltInnerChars, false, advanced, buf,
                                  sizeof(buf));
    CHECK(std::string(buf) == "Left S");

    OptionsScrollList::FormatCell("Načíst vybrané mody", 8, true, advanced, buf, sizeof(buf));
    CHECK(Foundation::CountUtf8Codepoints(buf) == 8);
}

TEST_CASE("OptionsPage cycle membership helper only accepts listed IDCs", "[optionsUI][UI]")
{
    const int cycle[] = {1101, 1104, 1107};
    TestableOptionsPage page;

    CHECK(page.ContainsCycleIdc(1101, cycle, 3));
    CHECK(page.ContainsCycleIdc(1107, cycle, 3));
    CHECK_FALSE(page.ContainsCycleIdc(1105, cycle, 3));
    CHECK_FALSE(page.ContainsCycleIdc(-1, cycle, 3));
    CHECK_FALSE(page.ContainsCycleIdc(1101, nullptr, 3));
}

TEST_CASE("Continue save selection creates a readable copy", "[optionsUI][savegame]")
{
    TempOptionsDir dir;
    const fs::path save = dir.root / "save.fps";
    const fs::path continueSave = dir.root / "continue.fps";
    std::ofstream(save, std::ios::binary) << "saved-world";

    const std::string saveDir = dir.root.string() + "/";
    EnsureContinueSave(saveDir.c_str());

    std::ifstream input(continueSave, std::ios::binary);
    std::string payload;
    input >> payload;
    CHECK(payload == "saved-world");
}

TEST_CASE("User missions use profile save directories", "[optionsUI][savegame]")
{
    const bool userMission = IsUserMission();
    const RString baseDirectory = GetBaseDirectory();
    const RString baseSubdirectory = GetBaseSubdirectory();
    const RString filenameReal = Glob.header.filenameReal;
    const std::string worldName = Glob.header.worldname;

    SetBaseDirectory(true, GetUserMissionsBase());
    SetBaseSubdirectory("missions/");
    Glob.header.filenameReal = "";

    CHECK(GetSaveDirectory() == GetTmpSaveDirectory());

    Glob.header.filenameReal = "editor_test";
    std::snprintf(Glob.header.worldname, sizeof(Glob.header.worldname), "%s", "demo");
    const RString namedSaveDirectory = GetUserDirectory() + RString("UserSaved/missions/editor_test.demo/");
    CHECK(GetSaveDirectory() == namedSaveDirectory);

#ifndef _WIN32
    Glob.header.filenameReal = "legacy_test";
    const RString legacySaveDirectory = GetUserDirectory() + RString("Saved/") + GetBaseDirectory() +
                                        GetBaseSubdirectory() + RString("legacy_test.demo/");
    fs::create_directories(fs::path(static_cast<const char*>(legacySaveDirectory)));
    std::ofstream(fs::path(static_cast<const char*>(legacySaveDirectory)) / "save.fps", std::ios::binary)
        << "saved-world";

    const RString migratedSaveDirectory = GetSaveDirectory();
    std::ifstream migrated(fs::path(static_cast<const char*>(migratedSaveDirectory)) / "save.fps", std::ios::binary);
    std::string payload;
    migrated >> payload;
    CHECK(payload == "saved-world");
    CHECK(fs::exists(fs::path(static_cast<const char*>(legacySaveDirectory)) / "save.fps"));
#endif

    SetBaseDirectory(userMission, baseDirectory);
    SetBaseSubdirectory(baseSubdirectory);
    Glob.header.filenameReal = filenameReal;
    std::snprintf(Glob.header.worldname, sizeof(Glob.header.worldname), "%s", worldName.c_str());
}

// A page popped from inside its own handler must outlive that handler. The
// owner's crash (2026-10-14, OptionsScrollList::PollPointerActionClick writing
// into an unmapped `this`) is the class this pins: Close row, Esc and modal
// Resolve all reach OptionsShell::PopPage from code running on the page (or its
// scroll list), and that code keeps touching members after PopPage returns
// (the pointer poll's m_pointerLeftWasDown store, AudioPage's preview tick,
// FlatMenuPage's m_lastNavIdc). PopPage used to free the page on the spot.
namespace
{
struct PageLifetime
{
    bool destroyed = false;
    bool aliveAfterAction = false;
};

class OneRowProvider : public OptionsScrollList::Provider
{
  public:
    int RowCount() const override { return 1; }
    const char* RowLabel(int) const override { return "Row"; }
    OptionsScrollList::RowDef RowFor(int) const override { return {-1, nullptr, 2}; }
    int RowValue(int) const override { return 0; }
    void SetRowValue(int, int) override {}
};

class ClosingScrollPage : public ScrollListPage
{
  public:
    explicit ClosingScrollPage(PageLifetime& life) : m_life(life) {}
    ~ClosingScrollPage() override { m_life.destroyed = true; }

    const char* TitleText() const override { return ""; }
    int DefaultFocusIdc() const override { return -1; }
    const char* ResourceClassName() const override { return ""; }

    using ScrollListPage::List;

  protected:
    OptionsScrollList::Provider& ProviderRef() override { return m_provider; }

  private:
    PageLifetime& m_life;
    OneRowProvider m_rows;
    OptionsScrollList::WithCloseRow m_provider{m_rows, "Close", ""};
};

// Mirrors IndexPage's Credits/Close nav row: OnNav pops the page itself and
// FlatMenuPage::OnButtonClicked then records m_lastNavIdc on it.
class SelfClosingFlatPage : public FlatMenuPage
{
  public:
    explicit SelfClosingFlatPage(PageLifetime& life) : FlatMenuPage(kCycle, kCycleSize), m_life(life) {}
    ~SelfClosingFlatPage() override { m_life.destroyed = true; }

    const char* TitleText() const override { return ""; }
    int DefaultFocusIdc() const override { return -1; }
    const char* ResourceClassName() const override { return ""; }

  protected:
    bool OnNav(OptionsShell& shell, int) override
    {
        shell.PopPage();
        m_life.aliveAfterAction = !m_life.destroyed;
        return true;
    }

  private:
    PageLifetime& m_life;
    static constexpr int kCycle[] = {1105, 1106};
    static constexpr int kCycleSize = 2;
};
} // namespace

TEST_CASE("OptionsShell keeps a scroll-list page alive after its own Close row pops it", "[optionsUI][UI][options]")
{
    PageLifetime life;
    {
        TestableOptionsShell shell(true, false);
        auto page = std::make_unique<ClosingScrollPage>(life);
        ClosingScrollPage* raw = page.get();
        shell.PushPage(std::move(page));
        REQUIRE(raw->List() != nullptr);

        // The pinned footer (slot 8) is the Close row; 583 is its hover overlay,
        // the control a pointer click on the row lands on.
        const int closeHover = OptionsScrollList::SlotIdcHover(OptionsScrollList::kVisibleSlots - 1);
        CHECK(raw->OnButtonClicked(shell, closeHover));

        // Popped and unmounted (the list is detached) but not freed: the code
        // that dispatched the click is still running on this object.
        CHECK_FALSE(life.destroyed);
        CHECK(raw->List() == nullptr);
    }
    // Retired pages die with the shell at the latest.
    CHECK(life.destroyed);
}

TEST_CASE("OptionsShell keeps a flat menu page alive after its own nav row pops it", "[optionsUI][UI][options]")
{
    PageLifetime life;
    {
        TestableOptionsShell shell(true, false);
        auto page = std::make_unique<SelfClosingFlatPage>(life);
        SelfClosingFlatPage* raw = page.get();
        shell.PushPage(std::move(page));

        CHECK(raw->OnButtonClicked(shell, 1105));
        CHECK(life.aliveAfterAction);
        CHECK_FALSE(life.destroyed);
    }
    CHECK(life.destroyed);
}
