#pragma once

// Character-select player-body substitution (issue #25 + its class-driven
// follow-up): the player's authored mission.sqm class is rewritten in place
// before World::InitVehicles creates the centers, on either menu channel -
// an explicit BODY-browser pick (gmSelPlayerClass, any side's CfgVehicles
// classname, which takes precedence) or the outfit token (gmSelOutfit =
// "civilian" substitutes the resistance descriptor's playerClassCiv).
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
#include <Poseidon/Foundation/Containers/RStringArray.hpp> // FindArrayRStringCI

#include <functional> // injected shape-file probe (PlayerBodyModelIssue)

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

// The warrior body follows the resistance PICK (issue #54 A3, #26 item 3).
// The template's mission.sqm welds the player to its DEFAULT resistance's
// body; picking another roster (Jordan on Sinai, an addon faction pack on
// Abel) substitutes that roster's playerClassWarrior. Returns EMPTY, keeping
// the authored class, when: nothing was picked; the pick names no block
// (WARN); the pick IS the template's default resistance (its warrior
// documents the authored class, nothing to do, no log); the block authors no
// playerClassWarrior (WARN); or the class fails the probe (WARN). The
// default resistance is resolved as the registry does: the zones config's
// defaultResistance, else the built-in GUER side.
RString ResolveWarriorPlayerClass(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg, const char* selResistance,
                                  const ClassProbe& probe);

// Full player-body precedence (the class-driven follow-up to issue #25):
// an explicit BODY-browser pick (gmSelPlayerClass, an exact CfgVehicles
// classname from ANY side) beats the outfit token; with no pick, a CIVILIAN
// token resolves through ResolveCivilianPlayerClass; otherwise (WARRIOR or
// no token) the resistance pick decides through ResolveWarriorPlayerClass.
// A pick that fails the probe returns EMPTY - the AUTHORED mission.sqm class
// is kept, and deliberately WITHOUT falling through to the outfit token: the
// pick explicitly replaced the outfit resolution for the body, so its
// failure degrades to the authored class, never to a third body (plan-15
// shaped, stricter - the seam never invents a substitute).
RString ResolvePlayerBodyClass(const ParamEntry* zonesCfg, const ParamEntry* factionsCfg, const char* selPlayerClass,
                               const char* selOutfit, const char* selResistance, const ClassProbe& probe);

// Collect the addons that must be active for `className` to build without an
// "Access denied" denial from World::CheckAddon: the class's own owner plus the
// owners of every entry in its weapons[] and magazines[], plus the magazines[]
// of those weapons. Owners come from ParamEntry::GetOwner(), already lowercased
// by the AddonSystem; base-game classes carry an EMPTY owner, which
// ParamOwnerList treats as always visible, so empties are skipped rather than
// collected. Config roots are passed in (rather than read off the global Pars)
// for the same reason ResolvePlayerBodyClass takes a ClassProbe: it keeps the
// walk unit-testable against a synthetic ParamFile. Entries are added with
// AddUnique, so the result is de-duplicated; an unknown className yields an
// empty list.
//
// Closest existing relative is ArcadeUnitInfo::RequiredAddons
// (AI/ArcadeTemplate.cpp), which attributes a template's units to addons - but
// it is a DIFFERENT lookup (it scans CfgPatches >> <patch> >> units[] for the
// classname and returns the patch name, falling back to GetOwner), and it does
// not walk weapons[]/magazines[] at all. The two agree in practice because both
// bottom out at a CfgPatches class name, but they fail differently: a class
// present in a pbo yet absent from its units[] has an owner and no
// RequiredAddons hit. GetOwner is used here because it is the exact field
// World::CheckAddon tests, so this walk collects precisely what that gate would
// otherwise deny. The missing weapons[]/magazines[] hop is also why
// Guerrilla.Sinai/mission.sqm has to hand-list the transitive owners.
void CollectPlayerBodyAddons(const ParamEntry* vehiclesCfg, const ParamEntry* weaponsCfg,
                             const ParamEntry* magazinesCfg, RString className, FindArrayRStringCI& addons);

// Why a resolvable class can still be the wrong answer (issue #46 seam 4).
// ClassProbe asks only whether CfgVehicles carries the name; the new-game
// roster additionally requires a non-empty model[] whose .p3d the package
// ships (GuerrillaListPlayerBodies / GuerrillaOutfitPreviewClass), because a
// shapeless Man is not merely ugly: a model naming a .p3d the package does not
// ship ACCESS-VIOLATES the process in Man::Init during CreateVehicle (measured,
// not inferred - VehicleTypes' empty-model guard misses it because the model
// string is non-empty and ShapeBank answers with an empty LODShape rather than
// a null). The CHARACTER browser can never offer such a class (its roster is
// built behind that gate), but a descriptor's playerClassCiv is a raw string
// that nothing shape-probes, so the substitution seam checks it here. Returns a
// human-readable reason when the package cannot render the class, EMPTY when it
// can (and empty for an unknown class - the ClassProbe owns that verdict). The
// caller treats a non-empty reason as a failed existence test and keeps the
// authored class, which is the never-a-substitute rule documented above, not an
// exception to it. The probe is injected for the same reason ClassProbe is.
RString PlayerBodyModelIssue(const ParamEntry* vehiclesCfg, RString className,
                             const std::function<bool(RString)>& shapeFileExists);

// Engine wrapper: reads gmselplayerclass/gmseloutfit/gmselresistance from
// the GameState (the campaign variable bank is re-applied before
// InitVehicles runs, so the menu publish is visible here), locates
// CfgGuerrillaZones/Factions, and rewrites t.FindPlayer()->vehicle when the
// pure core resolves a class. The two descriptor blocks are NOT a gate (issue
// #46 seam 1): a body pick resolves against CfgVehicles alone, so requiring
// both made a template that authors one block and not the other discard every
// pick without a word in the log. What keeps this a no-op on a non-Guerrilla
// template is the absence of a published selection, not the config shape; a
// template with no player unit now says so instead of returning in silence.
// Mutating
// CurrentTemplate is safe: every launch re-runs ParseMission, and re-running
// the substitution is idempotent.
//
// Accepted emergent behaviour (documented, not to be "fixed"): the
// substituted body keeps its CONFIG side for distant identification - an
// observer resolves an unidentified man through the vanilla side-resolve
// ladder (stolen-uniform band at 1.35x the id range, real identity at
// 1.5x), so a body picked from the occupier's roster reads as occupier at
// range while the INSTANCE side stays the mission side (GUER) - any body
// class fights as resistance. The per-observer undercover layer is
// untouched by this seam.
void ApplyPlayerOutfitSelection(ArcadeTemplate& t);

} // namespace Guerrilla
} // namespace Poseidon
