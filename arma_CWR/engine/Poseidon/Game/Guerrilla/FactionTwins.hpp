#pragma once

// Faction side twins - one roster, authored twice.
//
// A CfgGuerrillaFactions descriptor may carry `sideTwin = "<OtherClass>"`,
// meaning "the same roster re-authored on another side; substitute freely".
// The campaign's resistance side is welded to the template's player unit
// (mission.sqm), so a resistance pick moves the ROSTER onto the player's side
// rather than moving the side; a twin is the config-clean way to do that, and
// the occupier uses the same mechanism to step off a colliding side.
//
// ZoneRegistry performs the substitution and GuerrillaNewGame decides whether
// a pick is launchable at all.  Both resolve twins through these helpers so
// the block and the substitution cannot drift apart - the previous dead
// IDC_OK guard is exactly what that drift looks like.
//
// Pure config-level: no World, no globals, no engine state.

#include <Poseidon/Foundation/Strings/RString.hpp>

namespace Poseidon
{
class ParamEntry;

namespace Guerrilla
{

// Chain-walk bound.  sideTwin is content, and content is never trusted to be
// acyclic: A -> B -> A, or a chain longer than this, terminates the walk empty
// instead of hanging.
constexpr int kMaxTwinHops = 8;

// Side of a descriptor: its `side` key, defaulting to the class name.  Empty
// when factionsCfg is null or `faction` names no subclass.
RString FactionSideOf(const ParamEntry* factionsCfg, RString faction);

// First member of `faction`'s sideTwin chain (the faction itself included)
// whose side is wantSide, case-insensitively.  Empty when none - including
// when a sideTwin names a class the config does not carry, which ends the walk
// rather than aborting (plan-15: unknown incoming data degrades).
RString TwinOnSide(const ParamEntry* factionsCfg, RString faction, const char* wantSide);

// First member of the chain whose side is NOT avoidSide.  Empty when none.
RString TwinOffSide(const ParamEntry* factionsCfg, RString faction, const char* avoidSide);

// First of the three war sides {GUER, WEST, EAST} that is not avoidSide - the
// landing site for a faction that must leave a side and has no twin to leave
// it by.  CIV is never a candidate: the civilian center is friendly to every
// side (AICenterImpl::BeginArcade) and carries no war semantics, so an
// occupier rebased onto it would never fight.  Empty when nothing is free.
RString FirstFreeWarSide(const char* avoidSide);

} // namespace Guerrilla
} // namespace Poseidon
