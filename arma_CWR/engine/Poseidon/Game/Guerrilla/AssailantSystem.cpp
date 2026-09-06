#include <Poseidon/Game/Guerrilla/AssailantSystem.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/Asset/Addon/AddonClosure.hpp>
#include <Poseidon/Foundation/Containers/RStringArray.hpp>
#include <Poseidon/AI/EntityAI.hpp>
#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/World/Entities/Weapons/Weapons.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Foundation/platform.hpp>
#include <algorithm>
#include <cmath>
#include <cstring>

void DeleteVehicle(Entity* veh);
namespace Poseidon { bool CheckAccessCreate(const ParamEntry& entry); }

namespace Poseidon::Guerrilla
{
void EnsureAssailantCommandsLinked();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
AssailantSystem& AssailantSystem::Instance()
{
    EnsureAssailantCommandsLinked();
    static AssailantSystem system;
    return system;
}
#pragma clang diagnostic pop

int AssailantSystem::SpareSide(int occ, int res)
{
    const int sides[] = {TEast, TWest, TGuerrila};
    bool o = false, r = false;
    for (int s : sides) { o |= s == occ; r |= s == res; }
    if (!o || !r || occ == res) return -1;
    for (int s : sides) if (s != occ && s != res) return s;
    return -1;
}
AssailantMode AssailantSystem::Mode(const char* name)
{
    if (!stricmp(name, "RESISTER")) return ASResister;
    if (!stricmp(name, "ROGUE")) return ASRogue;
    return ASNone;
}
const char* AssailantSystem::Name(AssailantMode mode)
{
    return mode == ASResister ? "RESISTER" : mode == ASRogue ? "ROGUE" : "";
}
bool AssailantSystem::HasCapacity(int live, int rogues, AssailantMode mode)
{
    return (mode == ASResister || mode == ASRogue) && live < MaxLive && (mode != ASRogue || rogues < MaxRogues);
}
bool AssailantSystem::Hostile(AssailantMode mode, bool personal, bool combatant, bool rogue)
{
    return mode == ASRogue || (mode == ASResister && (personal || combatant || rogue));
}
float AssailantSystem::Interval(float roll) { return -2400.0f * std::log1p(-std::clamp(roll, 0.000001f, 0.999999f)); }
float AssailantSystem::Advance(float remaining, float dt, bool eligible)
{
    return eligible ? std::max(0.0f, remaining - std::max(0.0f, dt)) : remaining;
}
void AssailantSystem::Clear()
{
    _records.Clear(); _side = -1; _ready = false; _accum = 0; _weapon = RString(); _magazine = RString();
}
void AssailantSystem::InitMission() { Clear(); Configure(); }
RString AssailantSystem::SideName() const
{
    return _side < 0 ? RString() : RString(Poseidon::Foundation::FindEnumName((TargetSide)_side));
}

// Only the CIV descriptor's assailantPistol / assailantMagazine are used.
// Unspecified pistol: lexical first public handgun with a usable magazine.
// Explicit invalid content fails closed, never the general rifle fallback.
bool AssailantSystem::ResolveEquipment()
{
    _weapon = RString(); _magazine = RString();
    auto& registry = ZoneRegistry::Instance();
    RString requested = registry.FactionValue("CIV", "assailantPistol");
    RString mag = registry.FactionValue("CIV", "assailantMagazine");
    const ParamEntry* bank = Pars.FindEntry("CfgWeapons");
    if (!bank) return false;
    for (int i = 0; i < bank->GetEntryCount(); ++i)
    {
        const ParamEntry& entry = bank->GetEntry(i);
        if (!entry.IsClass() || (requested.GetLength() && stricmp(entry.GetName(), requested))) continue;
        const ParamEntry* type = entry.FindEntry("weaponType");
        const ParamEntry* scope = entry.FindEntry("scopeWeapon");
        if (!type || !scope || (int)*scope < 2 || (int)*type != MaskSlotHandGun) continue;
        Ref<WeaponType> weapon = WeaponTypes.New(entry.GetName());
        if (!weapon || weapon->_weaponType != MaskSlotHandGun || weapon->_muzzles.Size() != 1) continue;
        const MuzzleType* muzzle = weapon->_muzzles[0];
        RString compatible;
        for (int m = 0; m < muzzle->_magazines.Size(); ++m)
        {
            const MagazineType* mt = muzzle->_magazines[m];
            if (!mt || mt->_maxAmmo <= 0 || (mag.GetLength() && stricmp(mt->GetName(), mag))) continue;
            bool bullets = mt->_modes.Size() > 0;
            for (int k = 0; k < mt->_modes.Size(); ++k)
                if (!mt->_modes[k]->_ammo || mt->_modes[k]->_ammo->_simulation != AmmoShotBullet || mt->_modes[k]->_ammo->explosive) bullets = false;
            if (!bullets) continue;
            if (!compatible.GetLength() || stricmp(mt->GetName(), compatible) < 0) compatible = mt->GetName();
        }
        if (compatible.GetLength() && (!_weapon.GetLength() || stricmp(entry.GetName(), _weapon) < 0))
        { _weapon = entry.GetName(); _magazine = compatible; }
    }
    if (!_weapon.GetLength() || !_magazine.GetLength()) return false;
    FindArrayRStringCI addons;
    CollectWeaponAddons(bank, Pars.FindEntry("CfgMagazines"), _weapon, addons);
    CollectWeaponAddons(bank, Pars.FindEntry("CfgMagazines"), _magazine, addons);
    for (int i = 0; i < addons.Size(); ++i) if (!GWorld->IsAddonActive(addons[i])) GWorld->ActivateAddon(addons[i]);
    return true;
}
void AssailantSystem::Configure()
{
    _ready = false;
    auto& registry = ZoneRegistry::Instance();
    if (!registry.IsActive() || !GWorld) { _side = -1; return; }
    using Poseidon::Foundation::GetEnumValue;
    int occ = GetEnumValue<TargetSide>((const char*)registry.OccupierSide());
    int res = GetEnumValue<TargetSide>((const char*)registry.ResistanceSide());
    _side = SpareSide(occ, res);
    if (_side < 0) return;
    AICenter* spare = EnsureSideCenter(SideName());
    AICenter* oc = EnsureSideCenter(registry.OccupierSide());
    AICenter* rc = EnsureSideCenter(registry.ResistanceSide());
    if (!spare || !oc || !rc) return;
    oc->SetFriendship((TargetSide)_side, 0);
    rc->SetFriendship((TargetSide)_side, 0);
    spare->SetFriendship((TargetSide)occ, 0);
    spare->SetFriendship((TargetSide)res, 0);
    spare->SetFriendship(TCivilian, 1);
    spare->SetFriendship((TargetSide)_side, 1);
    _ready = ResolveEquipment();
    if (!_ready) LOG_WARN(Core, "Guerrilla assailants: no valid CIV handgun/magazine pair; conversions and threats disabled");
}
int AssailantSystem::Count(AssailantMode mode) const
{
    int n = 0;
    for (int i = 0; i < _records.Size(); ++i)
    {
        const auto& r = _records[i];
        if (r.body && !r.body->IsDammageDestroyed() && (mode == ASNone || mode == r.mode)) ++n;
    }
    return n;
}
AssailantMode AssailantSystem::Classify(const EntityAI* body) const
{
    if (body) for (int i = 0; i < _records.Size(); ++i) if (_records[i].body.GetLink() == body) return (AssailantMode)_records[i].mode;
    return ASNone;
}
bool AssailantSystem::IsGroup(const AIGroup* group) const
{
    if (group) for (int i = 0; i < _records.Size(); ++i) if (_records[i].group.GetLink() == group) return true;
    return false;
}
RString AssailantSystem::Diagnostic(AssailantMode mode) const
{
    if (!_ready) return RString("Threats unavailable: no valid civilian handgun equipment.");
    if (!HasCapacity(Count(), Count(ASRogue), mode)) return RString("Threats unavailable: too many armed assailants nearby.");
    return RString();
}
bool AssailantSystem::Eligible(Person* body, AssailantMode mode) const
{
    return _ready && HasCapacity(Count(), Count(ASRogue), mode) && body && !body->IsDammageDestroyed() &&
        body->IsLocal() && body->Brain() && body->Brain()->GetGroup() && body->Brain()->GetVehicle() == body &&
        body->Brain()->GetGroup()->GetCenter()->GetSide() == TCivilian && Classify(body) == ASNone;
}
bool AssailantSystem::Register(Person* body, AssailantMode mode, EntityAI* extorter, RString zone)
{
    const auto* home = ZoneRegistry::Instance().GetZone(ZoneRegistry::Instance().FindZoneIndex(zone));
    if (!Eligible(body, mode) || !home || stricmp(home->type, "CITY") != 0 ||
        (mode == ASResister && (!extorter || extorter == body || extorter->IsDammageDestroyed()))) return false;
    // All refusals precede mutation. Validated types + forced slots make the
    // equipment commit infallible, including civilian models with no slots.
    Ref<WeaponType> weapon = WeaponTypes.New(_weapon);
    Ref<MagazineType> magazine = MagazineTypes.New(_magazine);
    if (!weapon || !magazine || !CheckAccessCreate(*weapon->_parClass) || !CheckAccessCreate(*magazine->_parClass)) return false;
    Prune();
    Ref<AIGroup> group = CreateSideGroup(EnsureSideCenter(SideName()));
    if (!group) return false;
    Record row; row.body = body; row.extorter = extorter; row.group = group.GetRef(); row.zone = zone; row.mode = mode;
    _records.Add(row); // publish identity before the side transfer
    Ref<AIUnit> unit = body->Brain();
    Ref<AIGroup> old = unit->GetGroup();
    unit->ForceRemoveFromGroup();
    group->AddUnit(unit);
    group->GetCenter()->SelectLeader(group);
    group->AddFirstWaypoint(body->Position());
    group->AllowFleeing(0);
    group->SetCombatModeMajor(CMCombat);
    if (!old->NUnits()) old->RemoveFromCenter();
    body->RemoveAllWeapons();
    body->RemoveAllMagazines();
    for (int i = 0; i < 3; ++i) body->AddMagazine(_magazine, true);
    body->AddWeapon(weapon, true);
    return true;
}
bool AssailantSystem::Remove(EntityAI* body)
{
    for (int i = 0; i < _records.Size(); ++i) if (_records[i].body.GetLink() == body && body)
    {
        Ref<AIGroup> group = _records[i].group.GetLink();
        if (!body->IsDammageDestroyed()) ::DeleteVehicle(body);
        if (group && !group->NUnits()) group->RemoveFromCenter();
        _records.Delete(i);
        return true;
    }
    return false;
}
void AssailantSystem::Prune()
{
    for (int i = _records.Size() - 1; i >= 0; --i)
    {
        auto& r = _records[i];
        if (r.body && !r.body->IsDammageDestroyed()) continue;
        if (r.group && !r.group->NUnits()) r.group->RemoveFromCenter();
        _records.Delete(i); // deaths already ran killed EH synchronously
    }
}
void AssailantSystem::Simulate(float dt)
{
    if (!IsActive()) return;
    _accum += dt;
    if (_accum < 1) return;
    _accum = 0;
    Prune();
    Person* player = GWorld ? GWorld->GetRealPlayer() : nullptr;
    if (!player) return;
    for (int i = _records.Size() - 1; i >= 0; --i)
    {
        const auto& r = _records[i];
        const auto* z = ZoneRegistry::Instance().GetZone(ZoneRegistry::Instance().FindZoneIndex(r.zone));
        if (r.body && r.body->Position().Distance2(player->Position()) > 1200 * 1200 &&
            (!z || z->pos.Distance2(player->Position()) > 1200 * 1200)) Remove(r.body);
    }
}
bool AssailantSystem::IsHostile(const AIUnit* observer, const EntityAI* target, int side, const AICenter* center) const
{
    if (observer && target) for (int i = 0; i < _records.Size(); ++i)
    {
        const auto& r = _records[i];
        if (r.body.GetLink() != observer->GetPerson()) continue;
        if (target == r.body.GetLink() || target->IsDammageDestroyed()) return false;
        // Only living perceived people/crewed vehicles, never empty scenery.
        if (!target->CommanderUnit()) return false;
        const bool combatant = side == TEast || side == TWest || side == TGuerrila;
        return Hostile((AssailantMode)r.mode, r.extorter.GetLink() == target, combatant, Classify(target) == ASRogue);
    }
    return center && center->IsEnemy((TargetSide)side);
}
bool ObserverHostile(const AIUnit* observer, const EntityAI* target, int side, const AICenter* center)
{ return AssailantSystem::Instance().IsHostile(observer, target, side, center); }
bool IndependentGroup(const AIGroup* group) { return AssailantSystem::Instance().IsGroup(group); }
LSError AssailantSystem::Record::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.SerializeRef("body", body, 1))
    PARAM_CHECK(ar.SerializeRef("extorter", extorter, 1))
    PARAM_CHECK(ar.SerializeRef("group", group, 1))
    PARAM_CHECK(ar.Serialize("zone", zone, 1, RString()))
    PARAM_CHECK(ar.Serialize("mode", mode, 1, 0))
    return LSOK;
}
LSError AssailantSystem::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("Records", _records, 1))
    if (ar.IsLoading() && ar.GetPass() == ParamArchive::PassSecond) { Configure(); Prune(); }
    return LSOK;
}
}
