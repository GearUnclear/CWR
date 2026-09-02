#pragma once

#include <CLI/CLI.hpp>

namespace PoseidonTools
{

// `PoseidonTools guerrilla probe` - static spawn gate over a mod's CfgVehicles
// roster (issue #54 D3).
class GuerrillaProbeCommand
{
  public:
    static void Setup(CLI::App& parent);
};

} // namespace PoseidonTools
