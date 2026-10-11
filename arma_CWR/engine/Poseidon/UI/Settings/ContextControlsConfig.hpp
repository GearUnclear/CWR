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

    // Set by Load when the file predated the current action set and was seeded
    // with defaults for the actions it lacked. The caller re-saves to persist it.
    bool migratedOnLoad = false;

    void LoadDefaults();
    // Arma 3 Classic: reserve bare Q/E for infantry lean. Other contexts and
    // modified chords are unchanged. Used by defaults, preset and v5 migration.
    static void ApplyInfantryLeanDefaults(InputProfile& profile);
    // Apply the Arma 3 tap/hold, target and watch layout in every context,
    // plus infantry Q/E leaning. Preserve unrelated keys and gamepad bindings.
    void ApplyArma3Preset();
    bool Load(const std::string& path);
    bool Save(const std::string& path) const;
};
} // namespace Poseidon
