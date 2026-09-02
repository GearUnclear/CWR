#pragma once

#include <CLI/CLI.hpp>

namespace PoseidonTools
{

// `PoseidonTools guerrilla scaffold` (issue #54 C2): turn any world that has a
// CfgWorlds entry with a Names block and a road net into a playable Guerrilla
// Mode template - description.ext + mission.sqm + init.sqs - in one command,
// with no hand-sampled elevations and no hand-listed addOns[].
//
// This file owns the I/O half: the package mount, the .wrp read, the config
// probes and the file writes. The placement rules and the template text are
// pure and live in engine/Poseidon/Game/Guerrilla/IslandScaffold.{hpp,cpp},
// where PoseidonTests exercises them against synthetic terrain.
class GuerrillaScaffoldCommand
{
  public:
    static void Setup(CLI::App& parent);
};

} // namespace PoseidonTools
