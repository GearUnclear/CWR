#pragma once

#include <CLI/CLI.hpp>

namespace PoseidonTools
{

// The `guerrilla` parent command: offline checks and generators for Guerrilla
// Mode content. Each subcommand lives in its own file and registers itself here.
class GuerrillaCommand
{
  public:
    static void Setup(CLI::App& app);
};

} // namespace PoseidonTools
