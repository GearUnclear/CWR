#pragma once

// One shared accessor over CfgWorlds >> <world> >> Names, the engine's only
// source of world place names (UIMap.cpp draws it; ZoneRegistry's CITY auto-seed
// and GuerrillaNewGame's START TOWN cycler already read it with the identical
// four-line lookup).  Names entries are NOT capped at the zone table's ceiling
// and are strictly richer than it, which is why the history generator reads them
// rather than the zones.
//
// Entry shape: class key; `name` display string (falls back to the class key);
// `position[]` = {easting, northing[, LABEL SIZE]} - element [2] is the OFP map
// LABEL SIZE, NOT an elevation, and is discarded.  Optional Arma `type` key.
//
// CollectPlaceNames owns the parse and calls ZoneRegistry::NamesEntryIsTown for
// the SETTLEMENT BOOLEAN ONLY: that classifier returns false before it writes
// its name / pos out-params for a typed non-town entry or a short position[],
// which is exactly the set FeatureNames() has to return with a usable name.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp> // Vector3, VZero
#include <Poseidon/Foundation/Strings/RString.hpp>

namespace Poseidon
{

class ParamEntry;

namespace Guerrilla
{

class ZoneRegistry;

struct PlaceName
{
    RString key;             // config class key ("Larche")
    RString name;            // display string, DecodeLegacyTextToRString'd; never empty
    RString type;            // raw `type` value, "" when none
    Vector3 pos = VZero;     // engine axes: X = easting, Y = 0, Z = northing
    bool settlement = false; // ZoneRegistry::NamesEntryIsTown accepted it
};

// PURE: no world, no config lookup of its own.  Config order is preserved, one
// row per class entry, `out` cleared first.  A null entry yields an empty array.
void CollectPlaceNames(const ParamEntry* worldNames, AutoArray<PlaceName>& out);

// Pars >> CfgWorlds >> Glob.header.worldname >> Names; null when any step is
// missing (no world up, unknown world class, no Names block).
const ParamEntry* WorldNamesEntry();

// CollectPlaceNames over the active world.
void CollectWorldPlaceNames(AutoArray<PlaceName>& out);

// Partition helpers: settlements are the entries NamesEntryIsTown accepted,
// features every other named entry.  Both drop rows with an empty display name
// and duplicates of a display name already emitted (case-insensitive).
void SettlementNames(const AutoArray<PlaceName>& all, AutoArray<PlaceName>& out);
void FeatureNames(const AutoArray<PlaceName>& all, AutoArray<PlaceName>& out);

// displayName, else class name with '_' rewritten to ' ' ("PLO_East" -> "PLO
// East"; the LoBo descriptors author no displayName), else the side string.
// Lives here so the Game layer (FactionHistory, LegendRegistry) and the UI layer
// (GuerrillaJournalPages' FactionDisplay, which becomes a forwarder) share one
// copy.  An authored displayName is never rewritten.
RString FactionDisplayName(const ZoneRegistry& registry, const RString& className, const RString& side);

} // namespace Guerrilla
} // namespace Poseidon
