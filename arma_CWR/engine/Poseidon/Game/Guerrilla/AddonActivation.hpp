#pragma once

// Guerrilla launch-path addon activation (issue #54 C1).
//
// A mission's addOns[] is what World::ActivateAddons resets the active list
// to. For a Guerrilla template that list used to be hand-authored to the
// transitive closure of everything the template places AND everything its
// factions spawn - every weapon and magazine pbo, or the boot died on
// "Access denied" (a bare "StartAutoTest could not boot" under --autotest).
// ArcadeUnitInfo::RequiredAddons now follows weapons[]/magazines[], and this
// seam activates that closure at launch, so a template needs only its world
// and the classes it actually places in mission.sqm. The faction half
// (everything the descriptors name) is ZoneRegistry::InitMission's job,
// which runs after unit creation; this half must run BEFORE the centers
// create the placed units, next to the player-body substitution.
//
// Runtime visibility grant only: nothing is written into the template's
// addOns[], so a saved template keeps its authored manifest.

namespace Poseidon
{
struct ArcadeTemplate;

namespace Guerrilla
{
// Activates every addon the template's placed units and empty vehicles need
// beyond what addOns[] already activated. Gated on the mission being a
// Guerrilla template (ExtParsMission carries CfgGuerrillaZones), so every
// other launch path stays byte-identical. Logs the additions at INFO.
void ActivateTemplateAddons(const ArcadeTemplate& t);
} // namespace Guerrilla
} // namespace Poseidon
