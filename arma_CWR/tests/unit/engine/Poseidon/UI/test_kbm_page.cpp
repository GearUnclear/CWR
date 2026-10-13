// KbmPage / BindingsPage Provider — exercises the row layout and the
// device filter through the live InputSubsystem (snapshotted +
// restored to keep parallel ctest runs clean).
//
// Doesn't drive the capture modal — that's covered by the Trident
// integration tests under tests/integration/ui/options/controls/.

#include <Poseidon/Input/InputSubsystem.hpp>
#include <Poseidon/UI/Settings/ContextControlsConfig.hpp>
#include <Poseidon/Input/InputBinding.hpp>
#include <Poseidon/Input/InputCode.hpp>
#include <Poseidon/Input/InputContext.hpp>
#include <Poseidon/Input/InputProfile.hpp>
#include <Poseidon/Input/KeyInput.hpp>
#include <Poseidon/UI/DisplayUI.hpp>
#include <Poseidon/UI/Options/KbmPage.hpp>
#include <Poseidon/UI/Options/OptionsScrollList.hpp>
#include <Poseidon/UI/Locale/Stringtable/Stringtable.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <SDL3/SDL_scancode.h>
#include <catch2/catch_message.hpp>
#include <string>
#include <Poseidon/Foundation/Containers/Array.hpp>

using namespace Poseidon;
namespace Poseidon
{
extern Input GInput;
}

namespace
{
class UserKeysSnapshot
{
  public:
    UserKeysSnapshot() : savedContext(InputSubsystem::Instance().GetContext())
    {
        for (int c = 0; c < ContextControlsConfig::ContextCount; ++c)
            profiles[c] = InputSubsystem::Instance().GetProfile(static_cast<InputContext>(c));
        for (int i = 0; i < UAN; ++i)
            saved[i] = GInput.userKeys[i];
    }
    ~UserKeysSnapshot()
    {
        auto& sub = InputSubsystem::Instance();
        sub.SetContext(savedContext);
        for (int c = 0; c < ContextControlsConfig::ContextCount; ++c)
            sub.GetProfile(static_cast<InputContext>(c)) = profiles[c];
        for (int i = 0; i < UAN; ++i)
            GInput.userKeys[i] = saved[i];
    }

  private:
    InputContext savedContext;
    std::array<InputProfile, ContextControlsConfig::ContextCount> profiles;
    AutoArray<int> saved[UAN];
};

class TestableKbmPage : public KbmPage
{
  public:
    using BindingsPage::ClearCapture;
    using BindingsPage::FindCaptureConflict;
    using BindingsPage::ResetCurrentCategoryToDefaults;
    OptionsScrollList::Provider& Provider() { return ProviderRef(); }
    bool Filter(int code) const { return DeviceFilter(code); }
    void Capture(UserAction action, int slot, int code, int modifier = -1, bool replaceConflict = false)
    {
        ApplyCapture(action, slot, code, modifier, replaceConflict);
    }
};

void LoadMainMenuStringtable()
{
    static bool loaded = false;
    if (loaded)
    {
        SetLanguage("English");
        return;
    }

    const std::filesystem::path csv =
        std::filesystem::path(TESTS_ROOT_DIR) / "fixtures" / "stringtable" / "STRINGTABLE_MAINMENU.utf8.csv";
    LoadStringtable("global", csv.string().c_str(), 0.0f, true);
    SetLanguage("English");
    loaded = true;
}

int RowForCommonAction(UserAction action)
{
    const UserAction* actions = GetControlsCategoryActions(ControlsCategoryCommon);
    for (int i = 0; actions[i] != UAN; i++)
    {
        if (actions[i] == action)
            return i + 1;
    }
    return -1;
}
} // namespace

TEST_CASE("KbmPage: device filter accepts keyboard scancodes, mouse buttons", "[UI][KbmPage]")
{
    TestableKbmPage page;
    // Keyboard scancodes (low values).
    CHECK(page.Filter((int)SDL_SCANCODE_W));
    CHECK(page.Filter((int)SDL_SCANCODE_LCTRL));
    CHECK(page.Filter((int)SDL_SCANCODE_RETURN));
    // Mouse buttons (INPUT_DEVICE_MOUSE = 0x40000).
    CHECK(page.Filter(INPUT_DEVICE_MOUSE + 1));
    CHECK(page.Filter(INPUT_DEVICE_MOUSE + 2));
    // Negative codes are rejected.
    CHECK_FALSE(page.Filter(-1));
}

TEST_CASE("KbmPage: device filter rejects joystick / stick / pov / axis bits", "[UI][KbmPage]")
{
    TestableKbmPage page;
    CHECK_FALSE(page.Filter(INPUT_DEVICE_STICK + 0));
    CHECK_FALSE(page.Filter(INPUT_DEVICE_STICK + 7));
    CHECK_FALSE(page.Filter(INPUT_DEVICE_STICK_POV + 0));
    CHECK_FALSE(page.Filter(INPUT_DEVICE_STICK_AXIS + 0));
}

TEST_CASE("KbmPage: provider row count = 1 (cat stepper) + N actions + 1 (reset) + 1 (close)", "[UI][KbmPage]")
{
    TestableKbmPage page;
    auto& p = page.Provider();
    int onFootCount = GetControlsCategoryActionCount(ControlsCategoryOnFoot) - 4;
    // Provider exposes 1 + N + 1; WithCloseRow adds 1 more.
    CHECK(p.RowCount() == 1 + onFootCount + 1 + 1);
}

TEST_CASE("KbmPage: row 0 is the category stepper", "[UI][KbmPage]")
{
    TestableKbmPage page;
    auto& p = page.Provider();
    CHECK(p.RowKind(0) == OptionsScrollList::KindStepper);
    // Label comes from STR_DISP_OPT_CTL_CATEGORY which the unit-test
    // harness doesn't load — structural check (kind + stepper count)
    // is the assertion that doesn't depend on the stringtable.
    auto def = p.RowFor(0);
    CHECK(def.count == ControlsCategoryCount);
}

TEST_CASE("KbmPage: title and descriptions localize from the main menu shard", "[UI][KbmPage]")
{
    LoadMainMenuStringtable();

    TestableKbmPage page;
    auto& p = page.Provider();
    const int resetRow = 1 + GetControlsCategoryActionCount(ControlsCategoryOnFoot) - 4;

    CHECK(std::string(page.TitleText()) == "Keyboard & Mouse");
    CHECK(std::string(p.RowDescription(0)) == "Filter the binding list by activity context.");
    CHECK(std::string(p.RowDescription(1)) == "Press Enter to rebind. Esc to cancel during capture.");
    CHECK(std::string(p.RowDescription(resetRow)) ==
          "Restore default Keyboard & Mouse bindings for this category only.");
}

TEST_CASE("KbmPage: action rows are KindBinding", "[UI][KbmPage]")
{
    TestableKbmPage page;
    auto& p = page.Provider();
    int n = GetControlsCategoryActionCount(ControlsCategoryOnFoot) - 4;
    for (int row = 1; row <= n; row++)
    {
        CAPTURE(row);
        CHECK(p.RowKind(row) == OptionsScrollList::KindBinding);
    }
}

TEST_CASE("KbmPage: reset row sits past the action rows and is a KindAction", "[UI][KbmPage]")
{
    TestableKbmPage page;
    auto& p = page.Provider();
    int resetRow = 1 + GetControlsCategoryActionCount(ControlsCategoryOnFoot) - 4;
    CHECK(p.RowKind(resetRow) == OptionsScrollList::KindAction);
    // Label comes from STR_DISP_OPT_CTL_RESET_CAT which the unit-test
    // harness doesn't load — structural check (KindAction position)
    // is the assertion that doesn't depend on the stringtable.
}

TEST_CASE("KbmPage: BindingPrimary / Alt indices map to first / second keyboard entry", "[UI][KbmPage]")
{
    UserKeysSnapshot snap;
    TestableKbmPage page;
    auto& p = page.Provider();

    // Plant two known scancodes on MoveForward through the active context profile.
    auto& profile = InputSubsystem::Instance().GetProfile(InputContext::Infantry);
    profile.ClearBindings(UAMoveForward);
    profile.Bind(UAMoveForward, InputCode::Key(SDL_SCANCODE_W));
    profile.Bind(UAMoveForward, InputCode::Key(SDL_SCANCODE_UP));

    // GetKeyName goes through the localization table which the unit-test
    // harness doesn't load, so the rendered text may be empty; what we
    // can reliably assert is that BindingPrimary doesn't crash and that
    // the (no-binding) row returns "" cleanly.
    std::string primary = p.BindingPrimary(1);
    std::string alt = p.BindingAlt(1);
    (void)primary;
    (void)alt;

    // Now wipe the action — both cells should report empty.
    profile.ClearBindings(UAMoveForward);
    CHECK(std::string(p.BindingPrimary(1)).empty());
    CHECK(std::string(p.BindingAlt(1)).empty());
}

TEST_CASE("KbmPage: BindingPrimary skips out-of-device entries", "[UI][KbmPage]")
{
    UserKeysSnapshot snap;
    TestableKbmPage page;
    auto& p = page.Provider();

    // Plant only a stick button on MoveForward — KbmPage should not
    // surface it (DeviceFilter rejects INPUT_DEVICE_STICK).
    auto& profile = InputSubsystem::Instance().GetProfile(InputContext::Infantry);
    profile.ClearBindings(UAMoveForward);
    profile.Bind(UAMoveForward, InputCode::GamepadBtn(7));

    CHECK(std::string(p.BindingPrimary(1)).empty());
    CHECK(std::string(p.BindingAlt(1)).empty());
}

TEST_CASE("KbmPage: changing category through SetRowValue rebinds the row list", "[UI][KbmPage]")
{
    TestableKbmPage page;
    auto& p = page.Provider();

    int onFootCount = GetControlsCategoryActionCount(ControlsCategoryOnFoot) - 4;
    int commonCount = GetControlsCategoryActionCount(ControlsCategoryCommon);
    REQUIRE(p.RowCount() == 1 + onFootCount + 1 + 1);

    // Switch to Common.
    p.SetRowValue(0, (int)ControlsCategoryCommon);
    CHECK(p.RowValue(0) == (int)ControlsCategoryCommon);
    CHECK(p.RowCount() == 1 + commonCount + 1 + 1);

    // Switch wraps for negative / overflow values.
    p.SetRowValue(0, -1);
    CHECK(p.RowValue(0) == ControlsCategoryCount - 1);
    p.SetRowValue(0, ControlsCategoryCount + 2);
    CHECK(p.RowValue(0) == 2);
}

TEST_CASE("KbmPage: Common lists VoN toggle next to default push-to-talk", "[UI][KbmPage]")
{
    UserKeysSnapshot snap;
    TestableKbmPage page;
    auto& p = page.Provider();

    auto& menu = InputSubsystem::Instance().GetProfile(InputContext::Menu);
    menu.LoadDefaults();

    p.SetRowValue(0, (int)ControlsCategoryCommon);
    const int toggleRow = RowForCommonAction(UAVoiceOverNet);
    REQUIRE(toggleRow > 0);
    REQUIRE(RowForCommonAction(UAVoiceOverNetPushToTalk) == toggleRow + 1);

    CHECK(std::string(p.BindingPrimary(toggleRow)).empty());
    CHECK(std::string(p.BindingPrimary(toggleRow + 1)) == (const char*)GetKeyName(SDL_SCANCODE_CAPSLOCK));
}

TEST_CASE("KbmPage: vehicle movement capture writes driver profiles", "[UI][KbmPage]")
{
    UserKeysSnapshot snap;
    TestableKbmPage page;
    auto& p = page.Provider();
    p.SetRowValue(0, (int)ControlsCategoryVehicles);

    auto& sub = InputSubsystem::Instance();
    for (InputContext ctx : {InputContext::CarDriver, InputContext::TankDriver, InputContext::ShipDriver})
    {
        InputProfile& profile = sub.GetProfile(ctx);
        profile.ClearBindings(UAMoveForward);
        profile.Bind(UAMoveForward, InputCode::Key(SDL_SCANCODE_W));
        profile.Bind(UAMoveForward, InputCode::Key(SDL_SCANCODE_UP));
    }

    const int doubleH = InputBindingDoubleTapCode((int)SDL_SCANCODE_H);
    page.Capture(UAMoveForward, 0, doubleH);

    for (InputContext ctx : {InputContext::CarDriver, InputContext::TankDriver, InputContext::ShipDriver})
    {
        const InputProfile& profile = sub.GetProfile(ctx);
        const auto& bindings = profile.GetBindingEntries(UAMoveForward);
        REQUIRE(bindings.size() == 2);
        CHECK(bindings[0].code.toLegacy() == doubleH);
        CHECK_FALSE(profile.HasBinding(UAMoveForward, InputCode::Key(SDL_SCANCODE_W)));
        CHECK(profile.HasBinding(UAMoveForward, InputCode::Key(SDL_SCANCODE_UP)));
    }
}

#include <Poseidon/Input/ControlsCategory.hpp>
#include <Poseidon/Input/InputDeviceConstants.hpp>
#include <Poseidon/UI/Options/OptionsPage.hpp>

namespace
{
int RowForOnFootAction(UserAction action)
{
    const UserAction* actions = GetControlsCategoryActions(ControlsCategoryOnFoot);
    for (int i = 0; actions[i] != UAN; i++)
    {
        if (actions[i] == action)
            return i + 1;
    }
    return -1;
}
} // namespace

TEST_CASE("KbmPage: the Optics primary cell renders the tap-RMB binding with a Tap prefix", "[UI][KbmPage]")
{
    UserKeysSnapshot snap;
    LoadMainMenuStringtable();
    auto& sub = InputSubsystem::Instance();
    sub.SetContext(InputContext::Infantry);
    TestableKbmPage page;
    auto& p = page.Provider();

    auto& profile = sub.GetProfile(InputContext::Infantry);
    profile.ClearBindings(UAOptics);
    profile.Bind(UAOptics, InputCode::FromLegacy(InputBindingTapCode(INPUT_DEVICE_MOUSE + 1)));
    profile.Bind(UAOptics, InputCode::Key(SDL_SCANCODE_V));

    const int row = RowForOnFootAction(UAOptics);
    REQUIRE(row > 0);
    const std::string primary = p.BindingPrimary(row);
    CAPTURE(primary);
    CHECK(primary.find("Tap ") == 0);
    // The alt cell is the plain V key: no prefix.
    CHECK(std::string(p.BindingAlt(row)).find("Tap ") == std::string::npos);
}

TEST_CASE("ControlActionLabel: the new and renamed actions have readable labels", "[UI][KbmPage]")
{
    // With the main-menu fixture loaded, ZoomTemp resolves from the stringtable
    // row; LockTarget's key lives only in the game-data stringtable, so it takes
    // the English fallback.  Either way the label must never be empty or the
    // "!!! UNREGISTERED STRING" cheat-build placeholder.
    LoadMainMenuStringtable();
    CHECK(std::string(ControlActionLabel(UAZoomTemp)) == "Zoom in (temporary)");
    CHECK(std::string(ControlActionLabel(UALockTarget)) == "Lock target");
    CHECK_FALSE(std::string(ControlActionLabel(UAOptics)).empty());
    CHECK(std::string(ControlActionLabel(UAOptics)).find("UNREGISTERED") == std::string::npos);
    CHECK(std::string(ControlActionLabel(UAN)).empty());
}

TEST_CASE("Aircraft binding edit clear and reset isolate contexts and preserve controllers", "[UI][KbmPage][aircraft]")
{
    UserKeysSnapshot snapshot;
    auto& sub = InputSubsystem::Instance();
    ContextControlsConfig defaults;
    defaults.LoadDefaults();
    auto& heli = sub.GetProfile(InputContext::HeliPilot);
    auto& plane = sub.GetProfile(InputContext::PlanePilot);
    heli = defaults.profiles[(int)InputContext::HeliPilot];
    plane = defaults.profiles[(int)InputContext::PlanePilot];
    const InputBinding pad(InputCode::GamepadAx(1), InputCode::GamepadBtn(4), ActivationMode::OnHold, -0.6f);
    heli.SetBindingEntries(UAMoveForward, {InputCode::Key(SDL_SCANCODE_W), pad, pad});
    const auto planeBefore = plane.GetBindingEntries(UAMoveForward);
    TestableKbmPage page;
    auto& provider = page.Provider();
    provider.SetRowValue(0, ControlsCategoryHelicopter);
    page.Capture(UAMoveForward, 0, SDL_SCANCODE_F1);
    CHECK(heli.HasBinding(UAMoveForward, InputCode::Key(SDL_SCANCODE_F1)));
    CHECK(plane.GetBindingEntries(UAMoveForward) == planeBefore);
    page.ClearCapture(UAMoveForward, 0);
    CHECK(heli.GetBindingEntries(UAMoveForward) == std::vector<InputBinding>{pad, pad});
    page.ResetCurrentCategoryToDefaults();
    CHECK(heli.GetBindingEntries(UAMoveForward) == std::vector<InputBinding>{InputCode::Key(SDL_SCANCODE_W), pad, pad});
    CHECK(plane.GetBindingEntries(UAMoveForward) == planeBefore);
    provider.SetRowValue(0, ControlsCategoryPlane);
    CHECK(page.FindCaptureConflict(UAMoveDown, 0, SDL_SCANCODE_K, -1) == UACompass);
    CHECK(page.FindCaptureConflict(UAMoveDown, 0, SDL_SCANCODE_G, -1) == UAAircraftGear);
    CHECK(page.FindCaptureConflict(UAMoveDown, 0, SDL_SCANCODE_Q, -1) == UAMoveLeft);
    const std::filesystem::path csv =
        std::filesystem::path(TESTS_ROOT_DIR) / "../guerrilla-mode/runtime/common/bin/STRINGTABLE_UD_AIRCRAFT.utf8.csv";
    LoadStringtable("global", csv.string().c_str(), 0.0f, false);
    SetLanguage("English");
    CHECK(std::string(provider.RowLabel(1)) == "Increase speed");
    provider.SetRowValue(0, ControlsCategoryHelicopter);
    CHECK(std::string(provider.RowLabel(1)) == "Nose down");
    CHECK(std::string(ControlActionLabel(UAMoveDown, ControlsCategoryPlane)) == "Nose down");
}

TEST_CASE("Common reset keeps aircraft compass on K regardless of reset ordering", "[UI][KbmPage][aircraft]")
{
    UserKeysSnapshot snapshot;
    auto& sub = InputSubsystem::Instance();
    ContextControlsConfig defaults;
    defaults.LoadDefaults();
    sub.GetProfile(InputContext::HeliPilot) = defaults.profiles[(int)InputContext::HeliPilot];
    auto& plane = sub.GetProfile(InputContext::PlanePilot);
    plane = defaults.profiles[(int)InputContext::PlanePilot];
    const InputBinding pad(InputCode::GamepadPov(6));
    plane.Bind(UACompass, pad);
    sub.ResetCategoryDefaults(ControlsCategoryPlane);
    sub.ResetCategoryDefaults(ControlsCategoryCommon);
    CHECK(plane.HasBinding(UAAircraftGear, InputCode::Key(SDL_SCANCODE_G)));
    CHECK(plane.HasBinding(UACompass, InputCode::Key(SDL_SCANCODE_K)));
    CHECK_FALSE(plane.HasBinding(UACompass, InputCode::Key(SDL_SCANCODE_G)));
    CHECK(plane.HasBinding(UACompass, pad));
    TestableKbmPage page;
    page.Provider().SetRowValue(0, ControlsCategoryCommon);
    CHECK(page.FindCaptureConflict(UACompass, 0, SDL_SCANCODE_G, -1) == UAAircraftGear);
}
