#pragma once

namespace Poseidon
{

#define N_MOUSE_BUTTONS 8
#define N_JOYSTICK_BUTTONS 12 // [0-3]=A/B/X/Y [4-5]=LB/RB [6-7]=LT/RT thresh [8-9]=Back/Start [10-11]=LThumb/RThumb
#define N_JOYSTICK_AXES 8     // 3 normal, 3 rotate, 2 sliders
#define N_JOYSTICK_POV 8      // pov - 8 state

#define INPUT_DEVICE_MASK 0x00ff0000
#define INPUT_DEVICE_KEYBOARD 0x00000000
#define INPUT_DEVICE_MOUSE 0x00010000
#define INPUT_DEVICE_STICK 0x00020000
#define INPUT_DEVICE_STICK_AXIS 0x00030000
#define INPUT_DEVICE_STICK_POV 0x00040000

// Binding mode bits live below INPUT_DEVICE_MASK so the existing packed
// device class stays stable on disk and in conflict checks.
#define INPUT_BINDING_DOUBLE_TAP 0x00008000
// Tap binding: edge-only, fires on RELEASE when the press lasted less than
// the tap window (Arma 3 "quick click = toggle sights" on the same button
// whose plain hold is a level action).
#define INPUT_BINDING_TAP 0x00004000
#define INPUT_BINDING_VALUE_MASK 0x00003fff

// Default press-to-release window (ms) below which a press counts as a tap.
// 0 disables taps.  Session-tunable through InputSubsystem::SetTapWindowMs.
constexpr int kDefaultTapWindowMs = 250;

inline bool InputBindingIsDoubleTap(int packedCode)
{
    return (packedCode & INPUT_BINDING_DOUBLE_TAP) != 0;
}

inline bool InputBindingIsTap(int packedCode)
{
    return (packedCode & INPUT_BINDING_TAP) != 0;
}

inline int InputBindingDevice(int packedCode)
{
    return packedCode & INPUT_DEVICE_MASK;
}

inline int InputBindingValue(int packedCode)
{
    return packedCode & INPUT_BINDING_VALUE_MASK;
}

inline int InputBindingBaseCode(int packedCode)
{
    return InputBindingDevice(packedCode) | InputBindingValue(packedCode);
}

inline int InputBindingDoubleTapCode(int packedCode)
{
    return InputBindingBaseCode(packedCode) | INPUT_BINDING_DOUBLE_TAP;
}

inline int InputBindingTapCode(int packedCode)
{
    return InputBindingBaseCode(packedCode) | INPUT_BINDING_TAP;
}
} // namespace Poseidon
