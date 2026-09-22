#pragma once

#include <cstdint>
#include <vector>

namespace Poseidon::Guerrilla
{
inline constexpr int PortraitCaptureSize = 512;
inline constexpr int PortraitCardSize = 256;
inline constexpr int PortraitPhotoSize = 236;
inline constexpr const char* PortraitRecipeVersion = "dossier-10";

// Input is top-down, display-encoded RGB from the fixed offscreen target,
// before player gamma. Output is opaque RGBA. Invalid input returns no pixels.
// No graphics, campaign state or window dimensions enter the print treatment.
std::vector<uint8_t> StylePortrait(const std::vector<uint8_t>& rgb);
} // namespace Poseidon::Guerrilla
