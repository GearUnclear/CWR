#pragma once

// The director's per-action log level (OCCUPIER-FORCES.md 6.6, D12; owner
// decision 2026-10-14).  The one-line-per-action INFO trail of the occupier
// director (OccupierForces), the counter-attack service (CounterAttack,
// CounterAttackAir) and the air service (OccupierAir) is a developer and
// playtest instrument, not player-facing output.  It logs at INFO only while
// the toggle is on, and at DEBUG otherwise (so --log-level debug still shows
// it).  The toggle is on when the game runs with --dev
// (AppConfig::DevMode()) or the mission authors forcesLog = 1 in
// CfgGuerrillaZones (OccupierForcesTuning::log, gmForcesTune "log").  WARN
// and ERROR lines are not routed through here: they always log.

#include <Poseidon/Foundation/Framework/Log.hpp>

namespace Poseidon::Guerrilla
{
// spdlog::level::info while the director log toggle is on, debug otherwise
// (defined in OccupierForces.cpp)
spdlog::level::level_enum DirectorLogLevel();
} // namespace Poseidon::Guerrilla

// LOG_INFO's shape at the director's level: GM_DIRECTOR_INFO(Core, "fmt", ...)
#define GM_DIRECTOR_INFO(category, ...)                                                                                \
    LogDetail::Get(::Poseidon::Foundation::LogCategory::category)                                                      \
        ->log(spdlog::source_loc{}, ::Poseidon::Guerrilla::DirectorLogLevel(), __VA_ARGS__)
