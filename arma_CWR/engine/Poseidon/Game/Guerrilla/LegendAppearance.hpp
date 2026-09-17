#pragma once

#include <Poseidon/Foundation/Strings/RString.hpp>

namespace Poseidon
{
class ParamEntry;
}

namespace Poseidon::Guerrilla
{
// Shared by live identity binding and portrait preparation. No identity rolls,
// registry writes, asset loads or world access. A null configuration preserves
// the historical headless-test behavior.
bool LegendFaceUsable(const ParamEntry* faces, const char* token, bool bodyIsWoman);
RString ResolveLegendFace(const ParamEntry* faces, const RString& recordedFace, bool bodyIsWoman);
} // namespace Poseidon::Guerrilla
