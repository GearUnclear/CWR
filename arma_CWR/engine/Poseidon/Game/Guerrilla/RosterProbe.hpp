#pragma once

// Offline static gate for a mod's CfgVehicles roster (issue #54 D3).
//
// A class the game will refuse, or crash on, is invisible until someone spawns
// it: a model naming a .p3d the package does not ship ACCESS-VIOLATES Man::Init
// during CreateVehicle (see PlayerBodyModelIssue), and an init EventHandler that
// execs a missing .sqs leaves the unit half-initialised with only a script error
// in the log. Both are decidable from the config plus a file-existence probe, so
// they are decided here, before a mission is ever loaded.
//
// Pure: config roots and the file probe are injected, so the whole gate is unit
// testable without a mounted package. The runtime half (actually createVehicle-ing
// each class) is a separate Trident lane.

#include <Poseidon/Foundation/Strings/RString.hpp>

#include <functional>
#include <vector>

namespace Poseidon
{
class ParamEntry;
class ParamClass;

namespace Guerrilla
{

struct RosterProbeResult
{
    RString className;
    RString owner; //!< ParamEntry::GetOwner - the CfgPatches class that shipped it
    bool ok = false;
    RString reason; //!< empty when ok
    RString model;
    std::vector<RString> scriptRefs; //!< script paths the class's EventHandlers name
};

struct RosterProbeOptions
{
    //! Classes whose owner matches one of these (case-insensitive) are probed.
    std::vector<RString> owners;
    //! Class names probed regardless of owner. A mod folder's bin/config.cpp
    //! classes carry NO owner - AddonSystem only stamps one on a pbo addon's
    //! config - so a caller that knows which names a mod introduced passes them
    //! here rather than losing them.
    std::vector<RString> classNames;
    //! Wildcard over the class name (`*`/`?`); empty means all.
    RString filter;
};

// Every script path an EventHandlers block references, in scan order.
//
// Only the double-quoted form is collected - `exec "\Mod\scripts\x.sqs"`,
// `execVM`, `loadFile`, `preprocessFile`, `preprocessFileLineNumbers`. OFP also
// accepts braces as string delimiters, but braces in an EH body far more often
// hold code and bare tokens (`[{Throw},{JAM_AT4Launcher}]`), so treating them as
// paths would report noise as defects.
std::vector<RString> CollectScriptReferences(const ParamEntry& cls);

// The gate for one class, assumed already selected by IsProbeCandidate.
// `fileExists` takes an engine-relative path (a leading backslash is stripped
// before the call).
RosterProbeResult ProbeRosterClass(const ParamEntry& cls, const std::function<bool(RString)>& fileExists);

// Is this entry a class the gate applies to: Man- or Land-derived, and
// createable (scope >= 2)? `vehiclesCfg` supplies the base classes to test
// against; a package that declares none of them selects nothing.
bool IsProbeCandidate(const ParamEntry* vehiclesCfg, const ParamEntry& entry);

// Walk CfgVehicles and gate every candidate the options select.
std::vector<RosterProbeResult> ProbeRoster(const ParamEntry* vehiclesCfg, const RosterProbeOptions& options,
                                           const std::function<bool(RString)>& fileExists);

// Case-insensitive glob over `*` and `?`. Exposed because the caller filters
// owners with the same rule.
bool RosterWildcardMatch(const char* pattern, const char* text);

} // namespace Guerrilla
} // namespace Poseidon
