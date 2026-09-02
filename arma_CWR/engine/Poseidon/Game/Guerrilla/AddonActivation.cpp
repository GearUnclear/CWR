#include <Poseidon/Game/Guerrilla/AddonActivation.hpp>

#include <Poseidon/AI/ArcadeTemplate.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission
#include <Poseidon/World/World.hpp>

#include <Poseidon/Foundation/Containers/RStringArray.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>

namespace Poseidon::Guerrilla
{

void ActivateTemplateAddons(const ArcadeTemplate& t)
{
    if (!GWorld || !ExtParsMission.FindEntry("CfgGuerrillaZones"))
    {
        return; // not a Guerrilla template: the authored addOns[] stands alone
    }
    FindArrayRStringCI addons;
    t.RequiredAddonsFrom(Pars.FindEntry("CfgPatches"), Pars.FindEntry("CfgVehicles"), Pars.FindEntry("CfgWeapons"),
                         Pars.FindEntry("CfgMagazines"), addons);
    RString added;
    int nAdded = 0;
    for (int i = 0; i < addons.Size(); i++)
    {
        if (GWorld->IsAddonActive(addons[i]))
        {
            continue;
        }
        GWorld->ActivateAddon(addons[i]);
        added = added + (nAdded > 0 ? RString(", ") : RString()) + addons[i];
        nAdded++;
    }
    if (nAdded > 0)
    {
        LOG_INFO(Core,
                 "Guerrilla launch: activated {} addon(s) the placed units need beyond the mission's addOns[]: {}",
                 nAdded, (const char*)added);
    }
}

} // namespace Poseidon::Guerrilla
