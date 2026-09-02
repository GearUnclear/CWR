#pragma once

#include <CLI/CLI.hpp>

namespace PoseidonTools
{

// `PoseidonTools guerrilla lint` - audit a data package's Guerrilla faction
// descriptors against what the package can actually field (issue #54 A5).
class GuerrillaLintCommand
{
  public:
    static void Setup(CLI::App& parent);
};

} // namespace PoseidonTools
