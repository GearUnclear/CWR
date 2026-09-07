#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_keycode.h>
#include <Poseidon/Input/InputDeviceConstants.hpp>
#include <Poseidon/Input/KeyInput.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace Poseidon;
TEST_CASE("SDL scancodes have expected values", "[input]")
{
    // Verify key scancodes are in valid range
    REQUIRE(SDL_SCANCODE_ESCAPE > 0);
    REQUIRE(SDL_SCANCODE_ESCAPE < SDL_SCANCODE_COUNT);
    REQUIRE(SDL_SCANCODE_W < SDL_SCANCODE_COUNT);
    REQUIRE(SDL_SCANCODE_RETURN < SDL_SCANCODE_COUNT);
    REQUIRE(SDL_SCANCODE_F12 < SDL_SCANCODE_COUNT);
    REQUIRE(SDL_SCANCODE_KP_0 < SDL_SCANCODE_COUNT);

    // Verify distinct common keys don't collide
    REQUIRE(SDL_SCANCODE_W != SDL_SCANCODE_A);
    REQUIRE(SDL_SCANCODE_W != SDL_SCANCODE_S);
    REQUIRE(SDL_SCANCODE_W != SDL_SCANCODE_D);
    REQUIRE(SDL_SCANCODE_ESCAPE != SDL_SCANCODE_RETURN);
    REQUIRE(SDL_SCANCODE_SPACE != SDL_SCANCODE_TAB);
}

TEST_CASE("Key arrays use SDL_SCANCODE_COUNT", "[input]")
{
    Input input = {};
    // Verify arrays are large enough for any scancode
    REQUIRE(sizeof(input.keyboard.keys) / sizeof(input.keyboard.keys[0]) == SDL_SCANCODE_COUNT);
    REQUIRE(sizeof(input.keyboard.keysToDo) / sizeof(input.keyboard.keysToDo[0]) == SDL_SCANCODE_COUNT);
    REQUIRE(sizeof(input.keyboard.keyPressed) / sizeof(input.keyboard.keyPressed[0]) == SDL_SCANCODE_COUNT);

    // Verify common scancodes are valid indices
    input.keyboard.keys[SDL_SCANCODE_W] = 1.0f;
    input.keyboard.keys[SDL_SCANCODE_ESCAPE] = 1.0f;
    input.keyboard.keys[SDL_SCANCODE_F12] = 1.0f;
    REQUIRE(input.keyboard.keys[SDL_SCANCODE_W] == 1.0f);
    REQUIRE(input.keyboard.keys[SDL_SCANCODE_ESCAPE] == 1.0f);
    REQUIRE(input.keyboard.keys[SDL_SCANCODE_F12] == 1.0f);
}

TEST_CASE("SDL keycodes have expected values", "[input]")
{
    REQUIRE(SDLK_SPACE == 0x20);
    REQUIRE(SDLK_RETURN == 0x0Du);
    REQUIRE(SDLK_ESCAPE == 0x1Bu);
    REQUIRE(SDLK_TAB == 0x09u);
    // SDLK_LEFT etc. have the SDLK_SCANCODE_MASK bit set
    REQUIRE(SDLK_LEFT != 0);
    REQUIRE(SDLK_UP != 0);
    REQUIRE(SDLK_LEFT != SDLK_RIGHT);
    REQUIRE(SDLK_UP != SDLK_DOWN);
}

TEST_CASE("Input device masks are distinct", "[input]")
{
    REQUIRE(INPUT_DEVICE_KEYBOARD == 0x00000000);
    REQUIRE(INPUT_DEVICE_MOUSE == 0x00010000);
    REQUIRE(INPUT_DEVICE_STICK == 0x00020000);
    REQUIRE(INPUT_DEVICE_STICK_AXIS == 0x00030000);
    REQUIRE(INPUT_DEVICE_STICK_POV == 0x00040000);
    REQUIRE((INPUT_DEVICE_MOUSE & INPUT_DEVICE_MASK) == INPUT_DEVICE_MOUSE);
    REQUIRE((INPUT_DEVICE_STICK & INPUT_DEVICE_MASK) == INPUT_DEVICE_STICK);
}

#include <Poseidon/Input/InputCode.hpp>

TEST_CASE("Input binding tap flag: helpers, masks and legacy round-trip", "[input]")
{
    // The two mode bits are disjoint from each other, from the value field and
    // from the device field.
    REQUIRE((INPUT_BINDING_TAP & INPUT_BINDING_DOUBLE_TAP) == 0);
    REQUIRE((INPUT_BINDING_TAP & INPUT_BINDING_VALUE_MASK) == 0);
    REQUIRE((INPUT_BINDING_DOUBLE_TAP & INPUT_BINDING_VALUE_MASK) == 0);
    REQUIRE((INPUT_BINDING_TAP & INPUT_DEVICE_MASK) == 0);
    REQUIRE((INPUT_BINDING_DOUBLE_TAP & INPUT_DEVICE_MASK) == 0);

    // Every scancode fits below the mode bits.
    REQUIRE(SDL_SCANCODE_COUNT - 1 <= INPUT_BINDING_VALUE_MASK);

    const int rmb = INPUT_DEVICE_MOUSE + 1;
    const int tapRmb = InputBindingTapCode(rmb);
    REQUIRE(InputBindingIsTap(tapRmb));
    REQUIRE_FALSE(InputBindingIsDoubleTap(tapRmb));
    REQUIRE_FALSE(InputBindingIsTap(rmb));
    REQUIRE_FALSE(InputBindingIsTap(InputBindingDoubleTapCode(rmb)));
    REQUIRE(tapRmb == 0x14001);
    REQUIRE(InputBindingDevice(tapRmb) == INPUT_DEVICE_MOUSE);
    REQUIRE(InputBindingValue(tapRmb) == 1);

    // BaseCode strips both flags, and re-flagging a flagged code swaps the mode.
    REQUIRE(InputBindingBaseCode(tapRmb) == rmb);
    REQUIRE(InputBindingBaseCode(InputBindingDoubleTapCode(rmb)) == rmb);
    REQUIRE(InputBindingBaseCode(tapRmb | INPUT_BINDING_DOUBLE_TAP) == rmb);
    REQUIRE(InputBindingTapCode(InputBindingDoubleTapCode(rmb)) == tapRmb);
    REQUIRE(InputBindingDoubleTapCode(tapRmb) == InputBindingDoubleTapCode(rmb));

    // The flag survives the InputCode legacy round-trip (this is what persists
    // it in contextControls.cfg).
    const InputCode code = InputCode::FromLegacy(tapRmb);
    REQUIRE(code.toLegacy() == tapRmb);
    REQUIRE(InputBindingIsTap(code.toLegacy()));
    REQUIRE(code.device() == InputDevice::Mouse);
    REQUIRE(code != InputCode::FromLegacy(rmb));

    // Keyboard codes flag the same way.
    const int tapV = InputBindingTapCode((int)SDL_SCANCODE_V);
    REQUIRE(InputBindingIsTap(tapV));
    REQUIRE(InputBindingDevice(tapV) == INPUT_DEVICE_KEYBOARD);
    REQUIRE(InputBindingValue(tapV) == (int)SDL_SCANCODE_V);
}
