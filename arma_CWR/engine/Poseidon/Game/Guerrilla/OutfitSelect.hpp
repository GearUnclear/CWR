#pragma once

// Character-select outfit substitution (issue #25, milestone M1): when the
// new-game UI published gmSelOutfit = "civilian", the player's authored
// mission.sqm class is rewritten in place to the resistance faction
// descriptor's playerClassCiv before World::InitVehicles creates the centers.
//
// The seam runs BEFORE ZoneRegistry::InitMission (which InitVehicles calls
// only after unit creation), so it resolves the resistance faction block from
// raw config itself - ExtParsMission first, then Pars, the exact lookup
// ZoneRegistry::LoadFromConfig performs - instead of querying the registry.
//
// Fallback semantics are plan-15 shaped but stricter: on any failure (no
// selection, no descriptor, no playerClassCiv key, class not in the loaded
// data package) the authored mission.sqm class is KEPT - never a substitute
// body. Publishing "warrior" and publishing nothing are behaviorally
// identical by design: the seam acts only on "civilian".

#include <Poseidon/Foundation/Strings/RString.hpp>

namespace Poseidon
{
class ParamEntry;
struct ArcadeTemplate;

namespace Guerrilla
{
struct ClassProbe;

// Side-first-then-class-name subclass match over a CfgGuerrillaFactions
// entry - the exact scan order ZoneRegistry::FindFaction runs, so callers
// resolve the same descriptor block the registry resolves for the same
// string. Null when selection is null/empty or names no subclass.
const ParamEntry* FindGuerrillaFactionEntry(const ParamEntry* factionsCfg, const char* selection);

// Pure core (unit-testable with an injected ParamFile + fake probe).
// Returns the civilian player class to substitute, or EMPTY when the
// authored class must be kept. The resistance faction block is resolved with
// the registry's precedence: selResistance (side first, then class name -
// ZoneRegistry::FindFaction order) > the zones config's defaultResistance >
// the built-in "GUER" side.
RString ResolveCivilianPlayerClass(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg, const char* selOutfit,
                                   const char* selResistance, const ClassProbe& probe);

// Engine wrapper: reads gmseloutfit/gmselresistance from the GameState (the
// campaign variable bank is re-applied before InitVehicles runs, so the
// menu publish is visible here), locates CfgGuerrillaZones/Factions, and
// rewrites t.FindPlayer()->vehicle when the pure core resolves a class.
// No-op for non-Guerrilla templates (no zones+factions config) and for
// templates without a player unit. Mutating CurrentTemplate is safe: every
// launch re-runs ParseMission, and re-running the substitution is idempotent.
void ApplyPlayerOutfitSelection(ArcadeTemplate& t);

} // namespace Guerrilla
} // namespace Poseidon
