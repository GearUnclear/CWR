#include <Poseidon/Game/Guerrilla/Undercover.hpp>

#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp> // FindSideCenter / occupier side
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <Evaluator/express.hpp> // GameState / GameValue (VarGet)

#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/World/Entities/Infantry/SoldierOld.hpp> // Man / ManPos
#include <Poseidon/World/Entities/Weapons/Weapons.hpp>     // MaskSlotPrimary / MaskSlotSecondary
#include <Poseidon/AI/AI.hpp>
#include <Poseidon/AI/VehicleAI.hpp> // Target / TargetList

#include <Poseidon/Core/Global.hpp> // Glob.time
#include <Poseidon/Foundation/Common/FltOpts.hpp> // Square / saturate*
#include <Poseidon/Foundation/Enums/EnumNames.hpp> // GetEnumValue<TargetSide>
#include <Poseidon/Foundation/platform.hpp>

namespace Poseidon::Guerrilla
{

// Defined in UndercoverCommands.cpp.  Referencing it from here forces the
// command TU (whose only other content is an INIT_MODULE registration) into
// the link - same pattern as EnsureZoneRegistryCommandsLinked.
void EnsureUndercoverCommandsLinked();

// Process-lifetime singleton - no global constructor (same convention as
// ZoneRegistry::Instance).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
UndercoverSystem& UndercoverSystem::Instance()
{
    EnsureUndercoverCommandsLinked();
    static UndercoverSystem instance;
    return instance;
}
#pragma clang diagnostic pop

bool GUndercoverActive = false;

// ---------------------------------------------------------------------------
// pure rules
// ---------------------------------------------------------------------------

UCVerdict EvaluateUndercoverRule(const UCObservation& obs, const UndercoverTuning& tuning, float& outBoostedAccuracy)
{
    outBoostedAccuracy = obs.sideAccuracy;
    if (obs.alreadyCompromised)
    {
        // this group already knows the face; keep the record pinned at the
        // ID threshold so it does not fade back below hostile
        saturateMax(outBoostedAccuracy, tuning.undercoverIdentifyAccuracy);
        return UCExposed;
    }
    if (obs.visibility < tuning.undercoverMinVisibility)
    {
        return UCCivil; // effectively unseen: nothing to evaluate
    }
    float warBoost = 1.0f + tuning.undercoverWarDetectScale * obs.warDetect;
    if (obs.weapon == UCWInHands)
    {
        outBoostedAccuracy = obs.sideAccuracy * tuning.undercoverInHandsBoost * warBoost;
        return outBoostedAccuracy >= tuning.undercoverIdentifyAccuracy ? UCExposed : UCSuspect;
    }
    if (obs.weapon == UCWSlung)
    {
        if (obs.dist2 < Square(tuning.undercoverNoticeRadius))
        {
            // conversational range: the rifle on the back is unmissable
            saturateMax(outBoostedAccuracy, tuning.undercoverIdentifyAccuracy);
            return UCExposed;
        }
        if (obs.cosFacing < tuning.undercoverBackArcCos)
        {
            outBoostedAccuracy = obs.sideAccuracy * tuning.undercoverSlungBoost * warBoost;
            return outBoostedAccuracy >= tuning.undercoverIdentifyAccuracy ? UCExposed : UCSuspect;
        }
        return UCCivil; // front-on: the rifle hides behind the torso
    }
    return UCCivil; // unarmed and not previously identified
}

UCVerdict EvaluateUndercoverVehicleRule(const UCVehicleObservation& obs, const UndercoverTuning& tuning,
                                        float& outBoostedAccuracy)
{
    outBoostedAccuracy = obs.sideAccuracy;
    // 1. the group already remembers this vehicle
    if (obs.vehicleRecordCompromised)
    {
        saturateMax(outBoostedAccuracy, tuning.undercoverIdentifyAccuracy);
        return UCExposed;
    }
    bool seen = obs.visibility >= tuning.undercoverMinVisibility;
    bool close = obs.dist2 < Square(tuning.undercoverNoticeRadius);
    // 2. witnessed boarding / recognized occupant: the group that identified
    //    the PERSON engages the vehicle he fled into (or looks through the
    //    windshield up close); every other group still sees a civilian car
    if (obs.personRecordCompromised && (obs.personLastSeenRecent || (seen && close)))
    {
        saturateMax(outBoostedAccuracy, tuning.undercoverIdentifyAccuracy);
        return UCExposed;
    }
    // 3. civilian vehicle type: anonymous by policy - no suspicion
    //    accumulates from the vehicle itself
    if (obs.vehicleClass == UCVCivilian)
    {
        return UCCivil;
    }
    // 4. military vehicle (theft): the disguise fails at checkpoint range...
    if (seen && close)
    {
        saturateMax(outBoostedAccuracy, tuning.undercoverIdentifyAccuracy);
        return UCExposed;
    }
    // ...the vanilla stolen-vehicle idiom stays active (Target.cpp:942 -
    //    friendly markings, hostile crew reads TSideUnknown at >= 1.35)...
    if (obs.sideAccuracy >= 1.35f)
    {
        return UCSuspect;
    }
    // ...and at range the disguise holds (a captive crew reads TCivilian in
    //    vanilla too - either way, not a contact)
    return UCCivil;
}

// ---------------------------------------------------------------------------
// lifecycle / config
// ---------------------------------------------------------------------------

void UndercoverSystem::Clear()
{
    _tuning = UndercoverTuning();
    _active = false;
    _subject = nullptr;
    _subjectVehicle = nullptr;
    _occupierSide = -1;
    _warDetect = 0;
    _everCompromised = false;
    _pending.Clear();
    GUndercoverActive = false;
}

void UndercoverSystem::LoadFromParams(const ParamEntry* zonesCfg)
{
    // tunables only; live state is reset solely by Clear so a savegame's
    // second-pass config rebuild does not wipe loaded state
    _tuning = UndercoverTuning();
    if (!zonesCfg)
    {
        return;
    }
    _tuning.undercoverNoticeRadius = zonesCfg->ReadValue("undercoverNoticeRadius", _tuning.undercoverNoticeRadius);
    _tuning.undercoverBackArcCos = zonesCfg->ReadValue("undercoverBackArcCos", _tuning.undercoverBackArcCos);
    _tuning.undercoverInHandsBoost = zonesCfg->ReadValue("undercoverInHandsBoost", _tuning.undercoverInHandsBoost);
    _tuning.undercoverSlungBoost = zonesCfg->ReadValue("undercoverSlungBoost", _tuning.undercoverSlungBoost);
    _tuning.undercoverIdentifyAccuracy =
        zonesCfg->ReadValue("undercoverIdentifyAccuracy", _tuning.undercoverIdentifyAccuracy);
    _tuning.undercoverMinVisibility = zonesCfg->ReadValue("undercoverMinVisibility", _tuning.undercoverMinVisibility);
    _tuning.undercoverWarDetectScale =
        zonesCfg->ReadValue("undercoverWarDetectScale", _tuning.undercoverWarDetectScale);
    _tuning.undercoverForgetSeconds = zonesCfg->ReadValue("undercoverForgetSeconds", _tuning.undercoverForgetSeconds);
    _tuning.undercoverBoardWitnessSeconds =
        zonesCfg->ReadValue("undercoverBoardWitnessSeconds", _tuning.undercoverBoardWitnessSeconds);
}

// ---------------------------------------------------------------------------
// cache sync
// ---------------------------------------------------------------------------

void UndercoverSystem::SyncCaches()
{
    _active = false;
    _subject = nullptr;
    _subjectVehicle = nullptr;
    _occupierSide = -1;
    _warDetect = 0;

    World* world = GWorld;
    if (world)
    {
        ZoneRegistry& registry = ZoneRegistry::Instance();
        Person* player = registry.IsActive() ? world->GetRealPlayer() : nullptr;
        AIUnit* unit = player && !player->IsDammageDestroyed() ? player->CommanderUnit() : nullptr;
        if (unit && unit->GetCaptive())
        {
            // undercover global is script-owned; nil == false.  War level
            // defaults to 1 when undefined (matches init.sqs / GarrisonCache)
            bool undercover = false;
            float warLevel = 1.0f;
            GameState* gstate = world->GetGameState();
            if (gstate)
            {
                GameValue ucValue = gstate->VarGet("gmundercover");
                undercover = ucValue.GetType() == GameBool && (GameBoolType)ucValue;
                GameValue wlValue = gstate->VarGet("gmwarlevel");
                if (wlValue.GetType() == GameScalar)
                {
                    warLevel = (float)wlValue;
                }
            }
            if (undercover)
            {
                using Poseidon::Foundation::GetEnumValue;
                _occupierSide = (int)GetEnumValue<TargetSide>((const char*)registry.OccupierSide());
                _subject = player;
                EntityAI* veh = unit->GetVehicle();
                _subjectVehicle = veh && veh != player ? veh : nullptr;
                _warDetect = warLevel * 0.1f;
                _active = true;
            }
        }
    }
    GUndercoverActive = _active;
}

Person* UndercoverSystem::Subject() const
{
    Person* subject = _subject;
    if (!subject && GWorld)
    {
        subject = GWorld->GetRealPlayer();
    }
    return subject;
}

// ---------------------------------------------------------------------------
// world-facing adapter
// ---------------------------------------------------------------------------

bool UndercoverSystem::AppliesTo(const AICenter* center, const EntityAI* ai) const
{
    if (!_active || !center || !ai)
    {
        return false;
    }
    const EntityAI* subject = _subject.GetLink();
    const EntityAI* subjectVehicle = _subjectVehicle.GetLink();
    if (ai != subject && (!subjectVehicle || ai != subjectVehicle))
    {
        return false;
    }
    // only the occupying force evaluates the disguise; everyone else keeps
    // the vanilla captive result
    return (int)center->GetSide() == _occupierSide;
}

bool UndercoverSystem::IsCompromised(const Target& target) const
{
    if (!target.ucCompromised)
    {
        return false;
    }
    if (_tuning.undercoverForgetSeconds <= 0)
    {
        return true; // permanent per group (the default)
    }
    return Glob.time - target.ucCompromisedTime < _tuning.undercoverForgetSeconds;
}

// ManPos partition per the design doc: in-hands covers every raised-weapon
// pose (incl. drawn handguns and the AT tube), on-back the patrol walk and
// binocular poses, unarmed-looking the civilian moves.  A carried long gun
// still shows on the back proxy in the civilian moves, so it counts as slung.
UCWeaponShow ClassifyWeaponShow(const Man& man)
{
    bool hasLongGun = man.FindWeaponType(MaskSlotPrimary | MaskSlotSecondary) >= 0;
    bool hasAnyWeapon = hasLongGun || man.FindWeaponType(MaskSlotHandGun) >= 0;
    switch (man.GetActUpDegree())
    {
        case ManPosWeapon:
        case ManPosLying:
        case ManPosHandGunLying:
        case ManPosCrouch:
        case ManPosHandGunCrouch:
        case ManPosCombat:
        case ManPosHandGunStand:
            // a raised-weapon pose with nothing carried (transient right
            // after weapon removal) shows empty hands, not a weapon
            if (hasAnyWeapon)
            {
                return UCWInHands;
            }
            return UCWNone;
        case ManPosStand:
        case ManPosBinocLying:
        case ManPosBinoc:
        case ManPosBinocStand:
        case ManPosNoWeapon:
        case ManPosLyingNoWeapon:
        default:
            // holstered pistol / binocs alone stay concealed by design
            return hasLongGun ? UCWSlung : UCWNone;
    }
}

void UndercoverSystem::ResolvePerceivedSide(EntityAI* observer, AIUnit* observerUnit, EntityAI* ai, Target* target,
                                            float sensorSideAccuracy, float dist2, float visibility)
{
    if (!observer || !ai || !target)
    {
        return;
    }
    // materialize the decay knob: an expired compromise reverts the record
    if (target->ucCompromised && !IsCompromised(*target))
    {
        target->ucCompromised = false;
        target->ucCompromisedTime = TIME_MIN;
    }
    // unseen tick: leave the record untouched so fade semantics survive (this
    // resolver runs every tick, unlike vanilla's improvement-gated write; a
    // steady stream of zero-information writes would freeze the fade).  A
    // compromised record still resolves - memory needs no line of sight.
    if (visibility < _tuning.undercoverMinVisibility && !target->ucCompromised)
    {
        return;
    }
    // effective accuracy: this group is never judged worse than what it
    // already knows (the faded record) just because this tick's observation
    // is momentarily poor
    float effAccuracy = floatMax(sensorSideAccuracy, target->FadingSideAccuracy());
    if (_subjectVehicle.GetLink() && ai == _subjectVehicle.GetLink())
    {
        ResolvePerceivedSideVehicle(observer, observerUnit, ai, target, effAccuracy, dist2, visibility);
        return;
    }

    UCObservation obs;
    const Man* man = dyn_cast<Man>(ai);
    obs.weapon = man ? ClassifyWeaponShow(*man) : UCWNone;
    obs.dist2 = dist2;
    obs.cosFacing = ai->Direction().CosAngle(observer->Position() - ai->Position());
    obs.visibility = visibility;
    obs.sideAccuracy = effAccuracy;
    obs.alreadyCompromised = target->ucCompromised;
    obs.warDetect = _warDetect;

    float boosted = sensorSideAccuracy;
    UCVerdict verdict = EvaluateUndercoverRule(obs, _tuning, boosted);
    const char* reason = obs.weapon == UCWInHands ? "weapon" : (obs.weapon == UCWSlung ? "slung" : "recognized");
    ApplyVerdict(verdict, boosted, observer, ai, target, reason);
}

void UndercoverSystem::ResolvePerceivedSideVehicle(EntityAI* observer, AIUnit* observerUnit, EntityAI* ai,
                                                   Target* target, float sensorSideAccuracy, float dist2,
                                                   float visibility)
{
    UCVehicleObservation obs;
    TargetSide typical = ai->GetType()->GetTypicalSide();
    if (typical == TCivilian)
    {
        obs.vehicleClass = UCVCivilian;
    }
    else if ((int)typical == _occupierSide)
    {
        obs.vehicleClass = UCVOccupierMilitary;
    }
    else
    {
        obs.vehicleClass = UCVOther;
    }
    obs.vehicleRecordCompromised = target->ucCompromised;
    // the getaway rule reads this GROUP's record of the subject person
    AIGroup* group = observerUnit ? observerUnit->GetGroup() : nullptr;
    Person* subject = _subject;
    if (group && subject)
    {
        const Target* personRec = group->FindTargetAll(subject);
        if (personRec && IsCompromised(*personRec))
        {
            obs.personRecordCompromised = true;
            obs.personLastSeenRecent = Glob.time - personRec->lastSeen < _tuning.undercoverBoardWitnessSeconds;
        }
    }
    obs.dist2 = dist2;
    obs.visibility = visibility;
    obs.sideAccuracy = sensorSideAccuracy;

    float boosted = sensorSideAccuracy;
    UCVerdict verdict = EvaluateUndercoverVehicleRule(obs, _tuning, boosted);
    ApplyVerdict(verdict, boosted, observer, ai, target, "vehicle");
}

void UndercoverSystem::ApplyVerdict(UCVerdict verdict, float boostedAccuracy, EntityAI* observer, EntityAI* ai,
                                    Target* target, const char* reason)
{
    // stay inside the vanilla side-accuracy range (sensor path caps at 4)
    saturate(boostedAccuracy, 0, 4);
    switch (verdict)
    {
        case UCExposed:
            // no-arg real side preserves the renegade TEnemy special case
            target->side = ai->GetTargetSide();
            target->sideChecked = true;
            if (!target->ucCompromised)
            {
                target->ucCompromised = true;
                target->ucCompromisedTime = Glob.time;
                QueueCompromise(observer->Position(), RString(reason));
            }
            else
            {
                // refresh the decay window while the contact holds
                target->ucCompromisedTime = Glob.time;
            }
            break;
        case UCSuspect:
            // the stolen-vehicle idiom's TSideUnknown+checked: +10 interest
            // in HowMuchInteresting - investigate, don't shoot
            target->side = TSideUnknown;
            target->sideChecked = true;
            break;
        case UCCivil:
        default:
            target->side = TCivilian;
            target->sideChecked = boostedAccuracy >= _tuning.undercoverIdentifyAccuracy;
            break;
    }
    target->sideAccuracy = boostedAccuracy;
    target->sideAccuracyTime = Glob.time;
}

// ---------------------------------------------------------------------------
// compromise notifications / queries
// ---------------------------------------------------------------------------

void UndercoverSystem::QueueCompromise(Vector3Par witnessPos, RString reason)
{
    UCCompromise entry;
    entry.witnessPos = witnessPos;
    entry.reason = reason;
    entry.firstEver = !_everCompromised;
    _everCompromised = true;
    _pending.Add(entry);
}

void UndercoverSystem::ConsumeCompromises(AutoArray<UCCompromise>& out)
{
    out.Clear();
    for (int i = 0; i < _pending.Size(); i++)
    {
        out.Add(_pending[i]);
    }
    _pending.Clear();
}

// witness reference position: the group leader, else its first alive unit
// (the AlertMachine::GatherInputs pattern); false when nobody is left
static bool GroupWitnessPosition(const AIGroup* grp, Vector3& pos)
{
    AIUnit* refUnit = grp->Leader();
    if (!refUnit || refUnit->GetLifeState() != AIUnit::LSAlive)
    {
        refUnit = nullptr;
        for (int u = 0; u < MAX_UNITS_PER_GROUP && !refUnit; u++)
        {
            AIUnit* unit = grp->UnitWithID(u + 1);
            if (unit && unit->GetLifeState() == AIUnit::LSAlive)
            {
                refUnit = unit;
            }
        }
    }
    if (!refUnit)
    {
        return false;
    }
    pos = refUnit->Position();
    return true;
}

void UndercoverSystem::MarkAllWitnessesCompromised(RString reason)
{
    if (!GWorld)
    {
        return;
    }
    Person* subject = Subject();
    if (!subject)
    {
        return;
    }
    EntityAI* vehicle = _subjectVehicle;
    AICenter* occupier = FindSideCenter(ZoneRegistry::Instance().OccupierSide());
    if (!occupier)
    {
        return;
    }
    for (int g = 0; g < occupier->NGroups(); g++)
    {
        AIGroup* grp = occupier->GetGroup(g);
        if (!grp)
        {
            continue;
        }
        bool marked = false;
        Target* t = grp->FindTarget(subject);
        if (t && !t->destroyed && !t->ucCompromised)
        {
            t->ucCompromised = true;
            t->ucCompromisedTime = Glob.time;
            marked = true;
        }
        if (vehicle)
        {
            Target* tv = grp->FindTarget(vehicle);
            if (tv && !tv->destroyed && !tv->ucCompromised)
            {
                tv->ucCompromised = true;
                tv->ucCompromisedTime = Glob.time;
                marked = true;
            }
        }
        if (marked)
        {
            Vector3 pos;
            if (!GroupWitnessPosition(grp, pos))
            {
                pos = subject->Position();
            }
            QueueCompromise(pos, reason);
        }
    }
}

void UndercoverSystem::ForgetAll()
{
    _everCompromised = false;
    _pending.Clear();
    if (!GWorld)
    {
        return;
    }
    // scrub every record of every center - old getaway vehicles included
    for (int s = 0; s < TSideUnknown; s++)
    {
        AICenter* center = GWorld->GetCenter((TargetSide)s);
        if (!center)
        {
            continue;
        }
        for (int g = 0; g < center->NGroups(); g++)
        {
            AIGroup* grp = center->GetGroup(g);
            if (!grp)
            {
                continue;
            }
            const TargetList& list = grp->GetTargetList();
            for (int i = 0; i < list.Size(); i++)
            {
                const Target* t = list[i];
                if (t)
                {
                    Target* record = const_cast<Target*>(t);
                    record->ucCompromised = false;
                    record->ucCompromisedTime = TIME_MIN;
                }
            }
        }
    }
}

int UndercoverSystem::Status() const
{
    if (!GWorld)
    {
        return 0;
    }
    Person* subject = Subject();
    if (!subject)
    {
        return 0;
    }
    EntityAI* vehicle = _subjectVehicle;
    AICenter* occupier = FindSideCenter(ZoneRegistry::Instance().OccupierSide());
    if (!occupier)
    {
        return 0;
    }
    int status = 0;
    for (int g = 0; g < occupier->NGroups(); g++)
    {
        AIGroup* grp = occupier->GetGroup(g);
        if (!grp)
        {
            continue;
        }
        const Target* t = grp->FindTargetAll(subject);
        const Target* tv = vehicle ? grp->FindTargetAll(vehicle) : nullptr;
        if ((t && !t->destroyed && IsCompromised(*t)) || (tv && !tv->destroyed && IsCompromised(*tv)))
        {
            return 2;
        }
        if (status < 1)
        {
            bool suspectPerson = t && !t->destroyed && t->IsKnown() && t->sideChecked && t->side == TSideUnknown;
            bool suspectVehicle = tv && !tv->destroyed && tv->IsKnown() && tv->sideChecked && tv->side == TSideUnknown;
            if (suspectPerson || suspectVehicle)
            {
                status = 1;
            }
        }
    }
    return status;
}

int UndercoverSystem::WitnessCount() const
{
    if (!GWorld)
    {
        return 0;
    }
    Person* subject = Subject();
    if (!subject)
    {
        return 0;
    }
    EntityAI* vehicle = _subjectVehicle;
    AICenter* occupier = FindSideCenter(ZoneRegistry::Instance().OccupierSide());
    if (!occupier)
    {
        return 0;
    }
    int count = 0;
    for (int g = 0; g < occupier->NGroups(); g++)
    {
        AIGroup* grp = occupier->GetGroup(g);
        if (!grp)
        {
            continue;
        }
        const Target* t = grp->FindTargetAll(subject);
        const Target* tv = vehicle ? grp->FindTargetAll(vehicle) : nullptr;
        if ((t && !t->destroyed && IsCompromised(*t)) || (tv && !tv->destroyed && IsCompromised(*tv)))
        {
            count++;
        }
    }
    return count;
}

// ---------------------------------------------------------------------------
// serialization
// ---------------------------------------------------------------------------

LSError UCCompromise::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("witnessPos", witnessPos, 1, VZero))
    PARAM_CHECK(ar.Serialize("reason", reason, 1, RString()))
    PARAM_CHECK(ar.Serialize("firstEver", firstEver, 1, false))
    return LSOK;
}

LSError UndercoverSystem::Serialize(ParamArchive& ar)
{
    PARAM_CHECK(ar.Serialize("everCompromised", _everCompromised, 1, false))
    // a compromise queued in the tick-interval window before the save must
    // survive the load (same contract as the AlertMachine's breakPending)
    PARAM_CHECK(ar.Serialize("Pending", _pending, 1))
    return LSOK;
}

} // namespace Poseidon::Guerrilla
