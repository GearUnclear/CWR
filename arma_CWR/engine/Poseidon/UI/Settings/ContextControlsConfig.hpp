#pragma once

// Context-scoped action binding persistence. Stores one InputProfile per
// InputContext. gamepad.cfg and mouse.cfg store scalar settings only.

#include <Poseidon/Input/InputContext.hpp>
#include <Poseidon/Input/InputProfile.hpp>

#include <array>
#include <string>

namespace Poseidon
{
class ContextControlsConfig
{
  public:
    static constexpr int ContextCount = static_cast<int>(InputContext::Count);

    std::array<InputProfile, ContextCount> profiles;

    // Set when Load upgrades the file format. Older action sets may be seeded;
    // v5 keeps its saved rows and initializes new aircraft shortcuts unbound.
    bool migratedOnLoad = false;

    void LoadDefaults();
    // Arma 3 Classic: reserve bare Q/E for infantry lean. Other contexts and
    // modified chords are unchanged. Used by defaults, preset and v5 migration.
    static void ApplyInfantryLeanDefaults(InputProfile& profile);
    // Original Arma 3 aircraft keyboard layout; controller entries are retained.
    static void ApplyAircraftKeyboardDefaults(InputContext context, InputProfile& profile);
    static void ResetKeyboardAction(InputContext context, InputProfile& profile, UserAction action);
    // Apply the Arma 3 tap/hold, target and watch layout in every context,
    // plus infantry Q/E leaning and original Arma 3 aircraft keyboard controls.
    // Preserve unrelated keys and gamepad bindings.
    void ApplyArma3Preset();
    bool Load(const std::string& path);
    bool Save(const std::string& path) const;
};
} // namespace Poseidon
