#include "GuerrillaCommand.hpp"

#include "GuerrillaLintCommand.hpp"
#include "GuerrillaProbeCommand.hpp"
#include "GuerrillaScaffoldCommand.hpp"

#include <CLI/App.hpp>

namespace PoseidonTools
{

void GuerrillaCommand::Setup(CLI::App& app)
{
    auto* guerrilla = app.add_subcommand("guerrilla", "Guerrilla Mode content checks and generators");
    guerrilla->require_subcommand(1);

    GuerrillaLintCommand::Setup(*guerrilla);
    GuerrillaProbeCommand::Setup(*guerrilla);
    GuerrillaScaffoldCommand::Setup(*guerrilla);
}

} // namespace PoseidonTools
