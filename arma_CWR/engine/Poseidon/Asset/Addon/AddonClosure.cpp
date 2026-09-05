#include <Poseidon/Asset/Addon/AddonClosure.hpp>

#include <Poseidon/IO/ParamFile/ParamFile.hpp>

namespace Poseidon
{

namespace
{

// Optional string array off a config class. FindEntry (not
// FindEntryNoInheritance) so an array INHERITED from a base class is found:
// ParamClass::FindEntry resolves through _base, and most soldier bodies derive
// their weapons[]/magazines[] from a base class rather than restating them.
// Null when the key is absent or is not an array (ParamEntry::GetSize on a
// non-array raises an EMError, so the IsArray gate is not merely defensive).
const ParamEntry* FindClassArray(const ParamEntry* cls, const char* name)
{
    if (!cls)
    {
        return nullptr;
    }
    const ParamEntry* arr = cls->FindEntry(name);
    if (!arr || !arr->IsArray())
    {
        return nullptr;
    }
    return arr;
}

// Record the addon that owns `entry`. An EMPTY owner means base-game content,
// which ParamOwnerList reports visible unconditionally, so it is skipped:
// adding it would put a meaningless empty name into the active list.
void AddOwnerOf(const ParamEntry* entry, FindArrayRStringCI& addons)
{
    if (!entry)
    {
        return;
    }
    const RStringB& owner = entry->GetOwner();
    if (owner.GetLength() > 0)
    {
        addons.AddUnique(RString(owner));
    }
}

} // namespace

void CollectMagazineAddons(const ParamEntry* magazinesCfg, RString magazineClass, FindArrayRStringCI& addons)
{
    if (!magazinesCfg || magazineClass.GetLength() == 0)
    {
        return;
    }
    AddOwnerOf(magazinesCfg->FindEntry(magazineClass), addons);
}

void CollectWeaponAddons(const ParamEntry* weaponsCfg, const ParamEntry* magazinesCfg, RString weaponClass,
                         FindArrayRStringCI& addons)
{
    if (!weaponsCfg || weaponClass.GetLength() == 0)
    {
        return;
    }
    const ParamEntry* weapon = weaponsCfg->FindEntry(weaponClass);
    AddOwnerOf(weapon, addons);
    // the weapon config names the ammo it accepts, and the engine touches
    // those magazine classes when it kits a unit out
    if (const ParamEntry* mags = FindClassArray(weapon, "magazines"))
    {
        for (int i = 0; i < mags->GetSize(); i++)
        {
            CollectMagazineAddons(magazinesCfg, RString((RStringB)(*mags)[i]), addons);
        }
    }
}

void CollectVehicleClassAddons(const ParamEntry* vehiclesCfg, const ParamEntry* weaponsCfg,
                               const ParamEntry* magazinesCfg, RString className, FindArrayRStringCI& addons)
{
    if (!vehiclesCfg || className.GetLength() == 0)
    {
        return;
    }
    const ParamEntry* body = vehiclesCfg->FindEntry(className);
    if (!body)
    {
        return; // unknown class - nothing to activate, and nothing will spawn
    }
    AddOwnerOf(body, addons);
    // The class's own magazines[]: a mod body commonly carries mod ammo whose
    // CfgMagazines class lives in a DIFFERENT pbo than the body itself.
    if (const ParamEntry* mags = FindClassArray(body, "magazines"))
    {
        for (int i = 0; i < mags->GetSize(); i++)
        {
            CollectMagazineAddons(magazinesCfg, RString((RStringB)(*mags)[i]), addons);
        }
    }
    if (const ParamEntry* weapons = FindClassArray(body, "weapons"))
    {
        for (int i = 0; i < weapons->GetSize(); i++)
        {
            CollectWeaponAddons(weaponsCfg, magazinesCfg, RString((RStringB)(*weapons)[i]), addons);
        }
    }
}

} // namespace Poseidon
