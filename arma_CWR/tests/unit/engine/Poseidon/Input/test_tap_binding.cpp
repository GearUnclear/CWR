// Tap bindings end to end: a tap-flagged binding (INPUT_BINDING_TAP) is
// edge-only.  GetAction (level) returns 0 for it, GetActionToDo fires on the
// frame the key or button is RELEASED after a short press.  The Arma 3 right
// mouse layout hangs off this: plain RMB on ZoomTemp (a level, hold = zoom)
// and tap-RMB on Optics (quick click = toggle sights) share one button.
//
// Modelled on test_combo_fire_regression.cpp: the real InputSubsystem
// singleton with GInput snapshotted and restored around each case.

#include <Poseidon/Input/InputSubsystem.hpp>
#include <Poseidon/Input/InputBinding.hpp>
#include <Poseidon/Input/InputDeviceConstants.hpp>
#include <Poseidon/Input/InputProfile.hpp>
#include <Poseidon/Input/KeyInput.hpp>
#include <SDL3/SDL_scancode.h>
#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Foundation/Containers/Array.hpp>

using namespace Poseidon;
namespace Poseidon
{
extern Input GInput;
}

namespace
{
class FullStateSnapshot
{
  public:
    FullStateSnapshot()
        : savedContext(InputSubsystem::Instance().GetContext()),
          savedMenuProfile(InputSubsystem::Instance().GetProfile(InputContext::Menu)),
          savedTapWindow(InputSubsystem::Instance().GetTapWindowMs())
    {
        for (int i = 0; i < UAN; i++)
        {
            savedKeys[i] = GInput.userKeys[i];
            savedMods[i] = GInput.userKeysModifiers[i];
            savedActionDone[i] = GInput.actionDone[i];
        }
        for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
        {
            savedKb[i] = GInput.keyboard.keys[i];
            savedKbToDo[i] = GInput.keyboard.keysToDo[i];
            savedKbDoubleToDo[i] = GInput.keyboard.keysDoubleTapToDo[i];
            savedKbDoubleActive[i] = GInput.keyboard.keysDoubleTapActive[i];
            savedKbTapToDo[i] = GInput.keyboard.keysTapToDo[i];
        }
        for (int i = 0; i < N_MOUSE_BUTTONS; i++)
        {
            savedMouseButtons[i] = GInput.mouse.buttons[i];
            savedMouseToDo[i] = GInput.mouse.buttonsToDo[i];
            savedMouseDoubleToDo[i] = GInput.mouse.buttonsDoubleToDo[i];
            savedMouseDoubleActive[i] = GInput.mouse.buttonsDoubleActive[i];
            savedMouseTapToDo[i] = GInput.mouse.buttonsTapToDo[i];
        }
        savedFocus = GInput.gameFocusLost;
    }
    ~FullStateSnapshot()
    {
        auto& sub = InputSubsystem::Instance();
        sub.SetContext(savedContext);
        sub.GetProfile(InputContext::Menu) = savedMenuProfile;
        sub.SetTapWindowMs(savedTapWindow);
        for (int i = 0; i < UAN; i++)
        {
            GInput.userKeys[i] = savedKeys[i];
            GInput.userKeysModifiers[i] = savedMods[i];
            GInput.actionDone[i] = savedActionDone[i];
        }
        for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
        {
            GInput.keyboard.keys[i] = savedKb[i];
            GInput.keyboard.keysToDo[i] = savedKbToDo[i];
            GInput.keyboard.keysDoubleTapToDo[i] = savedKbDoubleToDo[i];
            GInput.keyboard.keysDoubleTapActive[i] = savedKbDoubleActive[i];
            GInput.keyboard.keysTapToDo[i] = savedKbTapToDo[i];
        }
        for (int i = 0; i < N_MOUSE_BUTTONS; i++)
        {
            GInput.mouse.buttons[i] = savedMouseButtons[i];
            GInput.mouse.buttonsToDo[i] = savedMouseToDo[i];
            GInput.mouse.buttonsDoubleToDo[i] = savedMouseDoubleToDo[i];
            GInput.mouse.buttonsDoubleActive[i] = savedMouseDoubleActive[i];
            GInput.mouse.buttonsTapToDo[i] = savedMouseTapToDo[i];
        }
        GInput.gameFocusLost = savedFocus;
    }

  private:
    AutoArray<int> savedKeys[UAN];
    AutoArray<int> savedMods[UAN];
    InputContext savedContext;
    InputProfile savedMenuProfile;
    int savedTapWindow;
    bool savedActionDone[UAN];
    float savedKb[SDL_SCANCODE_COUNT];
    bool savedKbToDo[SDL_SCANCODE_COUNT];
    bool savedKbDoubleToDo[SDL_SCANCODE_COUNT];
    bool savedKbDoubleActive[SDL_SCANCODE_COUNT];
    bool savedKbTapToDo[SDL_SCANCODE_COUNT];
    float savedMouseButtons[N_MOUSE_BUTTONS];
    bool savedMouseToDo[N_MOUSE_BUTTONS];
    bool savedMouseDoubleToDo[N_MOUSE_BUTTONS];
    bool savedMouseDoubleActive[N_MOUSE_BUTTONS];
    bool savedMouseTapToDo[N_MOUSE_BUTTONS];
    int savedFocus = 0;
};

void ResetState()
{
    for (int i = 0; i < UAN; i++)
    {
        GInput.userKeys[i].Resize(0);
        GInput.userKeysModifiers[i].Resize(0);
        GInput.actionDone[i] = false;
    }
    for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
    {
        GInput.keyboard.keys[i] = 0;
        GInput.keyboard.keysToDo[i] = false;
        GInput.keyboard.keysDoubleTapToDo[i] = false;
        GInput.keyboard.keysDoubleTapActive[i] = false;
        GInput.keyboard.keysTapToDo[i] = false;
    }
    for (int i = 0; i < N_MOUSE_BUTTONS; i++)
    {
        GInput.mouse.buttons[i] = 0.0f;
        GInput.mouse.buttonsToDo[i] = false;
        GInput.mouse.buttonsDoubleToDo[i] = false;
        GInput.mouse.buttonsDoubleActive[i] = false;
        GInput.mouse.buttonsTapToDo[i] = false;
    }
    GInput.gameFocusLost = 0;

    auto& sub = InputSubsystem::Instance();
    sub.SetContext(InputContext::Menu);
    sub.GetProfile(InputContext::Menu).ClearAll();
}

// Bind one packed code (possibly tap / double-tap flagged) to an action in
// both the legacy userKeys table and the active (Menu) context profile, the
// way the settings UI's ApplyCapture does.
void BindCode(UserAction action, int code)
{
    GInput.userKeys[action].Resize(0);
    GInput.userKeys[action].Add(code);
    GInput.userKeysModifiers[action].Resize(0);
    GInput.userKeysModifiers[action].Add(-1);

    auto& profile = InputSubsystem::Instance().GetProfile(InputContext::Menu);
    profile.ClearBindings(action);
    profile.Bind(action, InputBinding(InputCode::FromLegacy(code), InputCode{}));
}

constexpr int kRmb = INPUT_DEVICE_MOUSE + 1;
} // namespace

TEST_CASE("TapBinding: tap-RMB on Optics is edge-only, plain RMB on ZoomTemp is a level", "[Input][TapBinding]")
{
    FullStateSnapshot snap;
    auto& sub = InputSubsystem::Instance();
    ResetState();

    BindCode(UAOptics, InputBindingTapCode(kRmb));
    BindCode(UAZoomTemp, kRmb);

    // Frame 1: button goes down.  ZoomTemp reads the level; Optics sees no
    // level and no edge (the press edge belongs to plain bindings only).
    GInput.mouse.buttons[1] = 1.0f;
    GInput.mouse.buttonsToDo[1] = true;
    CHECK(sub.GetAction(UAZoomTemp, false) > 0.0f);
    CHECK(sub.GetAction(UAOptics, false) == 0.0f);
    CHECK(sub.GetActionToDo(UAOptics, false, false) == false);

    // Frame 2: held.  Still a level for ZoomTemp, still nothing for Optics.
    GInput.mouse.buttonsToDo[1] = false;
    CHECK(sub.GetAction(UAZoomTemp, false) > 0.0f);
    CHECK(sub.GetActionToDo(UAOptics, false, false) == false);

    // Frame 3: released within the tap window -> MouseState raises the tap
    // edge.  Optics fires once, ZoomTemp level is gone.
    GInput.mouse.buttons[1] = 0.0f;
    GInput.mouse.buttonsTapToDo[1] = true;
    CHECK(sub.GetAction(UAZoomTemp, false) == 0.0f);
    CHECK(sub.GetAction(UAOptics, false) == 0.0f);
    CHECK(sub.GetActionToDo(UAOptics, true, false) == true);
    // Consumed-once edge semantics, same as any other action edge.
    CHECK(sub.GetActionToDo(UAOptics, true, false) == false);
}

TEST_CASE("TapBinding: a long hold released never fires the tap-bound action", "[Input][TapBinding]")
{
    FullStateSnapshot snap;
    auto& sub = InputSubsystem::Instance();
    ResetState();

    BindCode(UAOptics, InputBindingTapCode(kRmb));

    GInput.mouse.buttons[1] = 1.0f;
    GInput.mouse.buttonsToDo[1] = true;
    CHECK(sub.GetActionToDo(UAOptics, false, false) == false);

    // Release after a long hold: MouseState leaves buttonsTapToDo clear, and
    // a plain release edge does not exist, so nothing fires.
    GInput.mouse.buttons[1] = 0.0f;
    GInput.mouse.buttonsToDo[1] = false;
    GInput.mouse.buttonsTapToDo[1] = false;
    CHECK(sub.GetActionToDo(UAOptics, false, false) == false);
}

TEST_CASE("TapBinding: plain V on Optics still fires from the press edge", "[Input][TapBinding]")
{
    FullStateSnapshot snap;
    auto& sub = InputSubsystem::Instance();
    ResetState();

    BindCode(UAOptics, (int)SDL_SCANCODE_V);

    GInput.keyboard.keys[SDL_SCANCODE_V] = 1.0f;
    GInput.keyboard.keysToDo[SDL_SCANCODE_V] = true;
    CHECK(sub.GetActionToDo(UAOptics, true, false) == true);

    // A keyboard tap edge on V means nothing to a plain binding.
    GInput.keyboard.keysToDo[SDL_SCANCODE_V] = false;
    GInput.keyboard.keysTapToDo[SDL_SCANCODE_V] = true;
    CHECK(sub.GetActionToDo(UAOptics, true, false) == false);
}

TEST_CASE("TapBinding: a tap-flagged keyboard binding fires from the key-up tap edge", "[Input][TapBinding]")
{
    FullStateSnapshot snap;
    auto& sub = InputSubsystem::Instance();
    ResetState();

    BindCode(UAOptics, InputBindingTapCode((int)SDL_SCANCODE_V));

    // Press edge: nothing (edge-only on release).
    GInput.keyboard.keys[SDL_SCANCODE_V] = 1.0f;
    GInput.keyboard.keysToDo[SDL_SCANCODE_V] = true;
    CHECK(sub.GetAction(UAOptics, false) == 0.0f);
    CHECK(sub.GetActionToDo(UAOptics, false, false) == false);

    // Release tap edge: fires.
    GInput.keyboard.keys[SDL_SCANCODE_V] = 0.0f;
    GInput.keyboard.keysToDo[SDL_SCANCODE_V] = false;
    GInput.keyboard.keysTapToDo[SDL_SCANCODE_V] = true;
    CHECK(sub.GetActionToDo(UAOptics, true, false) == true);
}

TEST_CASE("TapBinding: legacy GetKey / GetKeyToDo honour the tap flag", "[Input][TapBinding]")
{
    FullStateSnapshot snap;
    auto& sub = InputSubsystem::Instance();
    ResetState();

    const int tapV = InputBindingTapCode((int)SDL_SCANCODE_V);
    GInput.keyboard.keys[SDL_SCANCODE_V] = 1.0f;
    GInput.keyboard.keysToDo[SDL_SCANCODE_V] = true;
    CHECK(sub.GetKey(tapV, false) == 0.0f);
    CHECK(sub.GetKey((int)SDL_SCANCODE_V, false) > 0.0f);
    CHECK(sub.GetKeyToDo(tapV, false, false) == false);
    CHECK(sub.GetKeyToDo((int)SDL_SCANCODE_V, false, false) == true);

    GInput.keyboard.keysTapToDo[SDL_SCANCODE_V] = true;
    CHECK(sub.GetKeyToDo(tapV, true, false) == true);
    CHECK(sub.GetKeyToDo(tapV, true, false) == false);
}

TEST_CASE("TapBinding: SetTapWindowMs fans out to the mouse and keyboard structs and clamps", "[Input][TapBinding]")
{
    FullStateSnapshot snap;
    auto& sub = InputSubsystem::Instance();

    sub.SetTapWindowMs(123);
    CHECK(sub.GetTapWindowMs() == 123);
    CHECK(GInput.mouse.tapWindowMs == 123);
    CHECK(GInput.keyboard.tapWindowMs == 123);

    sub.SetTapWindowMs(-5);
    CHECK(sub.GetTapWindowMs() == 0);
    CHECK(GInput.keyboard.tapWindowMs == 0);

    sub.SetTapWindowMs(99999);
    CHECK(sub.GetTapWindowMs() == 2000);
    CHECK(GInput.mouse.tapWindowMs == 2000);
}

TEST_CASE("TapBinding: the default tap window is 250 ms on a fresh state", "[Input][TapBinding]")
{
    MouseState ms;
    KeyboardState kb;
    CHECK(ms.tapWindowMs == kDefaultTapWindowMs);
    CHECK(kb.tapWindowMs == kDefaultTapWindowMs);
    CHECK(kDefaultTapWindowMs == 250);
}
