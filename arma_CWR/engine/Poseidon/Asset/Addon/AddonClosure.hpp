#pragma once

// Transitive addon closure of a config class (issue #54 C1).
//
// The engine only builds a vehicle type when its class AND every weapon and
// magazine class it references belong to an ACTIVE addon (World::CheckAddon
// over ParamOwnerList). The owners come from ParamEntry::GetOwner, the
// CfgPatches name the AddonSystem stamps on every class a pbo's config
// contributes; base-game classes carry an EMPTY owner, which the owner list
// treats as always visible, so empties are never collected.
//
// ArcadeUnitInfo::RequiredAddons used to stop at the class's own owner, so a
// mission whose units carry mod weapons had to hand-list the weapon and
// magazine pbos in addOns[] or die on "Access denied" at boot. These walks
// follow weapons[] and magazines[] (and each weapon's own magazines[]) so a
// template needs only what it PLACES; the rest is derived.
//
// Config roots are injected (not read off the global Pars) so the walks stay
// unit-testable against a synthetic ParamFile with hand-stamped owners.
// Results are added with AddUnique; an unknown class contributes nothing.

#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Containers/RStringArray.hpp> // FindArrayRStringCI

namespace Poseidon
{
class ParamEntry;

// The owner of one CfgMagazines class.
void CollectMagazineAddons(const ParamEntry* magazinesCfg, RString magazineClass, FindArrayRStringCI& addons);
// The owner of one CfgWeapons class plus the owners of its magazines[].
void CollectWeaponAddons(const ParamEntry* weaponsCfg, const ParamEntry* magazinesCfg, RString weaponClass,
                         FindArrayRStringCI& addons);
// The owner of one CfgVehicles class plus the closure of its weapons[] and
// magazines[] (FindEntry, so arrays inherited from a base class count).
void CollectVehicleClassAddons(const ParamEntry* vehiclesCfg, const ParamEntry* weaponsCfg,
                               const ParamEntry* magazinesCfg, RString className, FindArrayRStringCI& addons);

} // namespace Poseidon
