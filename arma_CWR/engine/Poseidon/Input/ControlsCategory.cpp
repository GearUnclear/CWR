#include <Poseidon/Input/ControlsCategory.hpp>
#include <iterator>

namespace Poseidon
{

namespace
{
// Per-category UserAction lists.  Terminated by UAN.  Order within each
// list is the order rows appear on the screen (so it's also the
// control-table order — keep them in sync).

const UserAction kOnFoot[] = {
    // Movement
    UAMoveForward, UAMoveBack, UATurnLeft, UATurnRight, UAMoveLeft, UAMoveRight, UAMoveUp, UAMoveDown,
    UAMoveFastForward, UAMoveSlowForward, UATurbo, UALeanLeft, UALeanRight,
    UASlow, // "Walk" — the slow-movement toggle (label/default key live in UserActionDesc)
    // Weapons & combat
    UAFire, UAReloadMagazine, UAToggleWeapons, UAHandgun, UALockTarget, UALockTargets, UARevealTarget, UAOptics,
    UAZoomIn, UAZoomOut, UAZoomTemp,
    // Actions
    UAAction,
    // Gamepad aim grouping; hidden on KB&M on-foot rows.
    UAAimUp, UAAimDown, UAAimLeft, UAAimRight,
    // View / freelook
    UALookAround, UALookAroundToggle, UALookCenter, UALookLeft, UALookRight, UALookUp, UALookDown, UALookLeftUp,
    UALookRightUp, UALookLeftDown, UALookRightDown,
    // HUD
    UABinocular, UANightVision, UAN};

const UserAction kVehicles[] = {
    UAMoveForward, UAMoveBack,         UAMoveFastForward, UAMoveSlowForward, UATurbo,      UATurnLeft,    UATurnRight,
    UAFire,        UALockTarget,       UALockTargets,     UAAction,          UAZoomIn,     UAZoomOut,     UAZoomTemp,
    UALookAround,  UALookAroundToggle, UALookCenter,      UALookLeft,        UALookRight,  UALookUp,      UALookDown,
    UALookLeftUp,  UALookRightUp,      UALookLeftDown,    UALookRightDown,   UAHeadlights, UANightVision, UAN};

const UserAction kHelicopter[] = {UAMoveForward,
                                  UAMoveBack,
                                  UATurnLeft,
                                  UATurnRight,
                                  UAMoveLeft,
                                  UAMoveRight,
                                  UAMoveUp,
                                  UAMoveDown,
                                  UAFire,
                                  UALockTarget,
                                  UALockTargets,
                                  UAZoomIn,
                                  UAZoomOut,
                                  UAZoomTemp,
                                  UALookAround,
                                  UALookAroundToggle,
                                  UALookCenter,
                                  UALookLeft,
                                  UALookRight,
                                  UALookUp,
                                  UALookDown,
                                  UALookLeftUp,
                                  UALookRightUp,
                                  UALookLeftDown,
                                  UALookRightDown,
                                  UAHeadlights,
                                  UAAircraftAutoHover,
                                  UACompass,
                                  UAN};

const UserAction kPlane[] = {UAMoveForward,
                             UAMoveBack,
                             UATurnLeft,
                             UATurnRight,
                             UAMoveLeft,
                             UAMoveRight,
                             UAMoveUp,
                             UAMoveDown,
                             UAFire,
                             UALockTarget,
                             UALockTargets,
                             UAZoomIn,
                             UAZoomOut,
                             UAZoomTemp,
                             UALookAround,
                             UALookAroundToggle,
                             UALookCenter,
                             UALookLeft,
                             UALookRight,
                             UALookUp,
                             UALookDown,
                             UALookLeftUp,
                             UALookRightUp,
                             UALookLeftDown,
                             UALookRightDown,
                             UAHeadlights,
                             UAAircraftGear,
                             UACompass,
                             UAN};

const UserAction kGunner[] = {UAFire,          UAReloadMagazine,
                              UAToggleWeapons, UALockTarget,
                              UALockTargets,   UAAction,
                              UAOptics,        UAZoomIn,
                              UAZoomOut,       UAZoomTemp,
                              UAAimUp,         UAAimDown,
                              UAAimLeft,       UAAimRight,
                              UALookAround,    UALookAroundToggle,
                              UALookCenter,    UALookLeft,
                              UALookRight,     UALookUp,
                              UALookDown,      UALookLeftUp,
                              UALookRightUp,   UALookLeftDown,
                              UALookRightDown, UAN};

const UserAction kCommon[] = {UAPrevAction,   UANextAction,
                              UAPersonView,   UATacticalView,
                              UAMap,          UACompass,
                              UAWatch,        UAHelp,
                              UATimeInc,      UATimeDec,
                              UASelectAll,    UAChat,
                              UAVoiceOverNet, UAVoiceOverNetPushToTalk,
                              UAPrevChannel,  UANextChannel,
                              UANetworkStats, UANetworkPlayers,
                              UAMapZoomIn,    UAMapZoomOut,
                              UACheatEntry,   UAN};

const UserAction* const kCategoryTable[ControlsCategoryCount] = {
    kOnFoot, kVehicles, kHelicopter, kPlane, kGunner, kCommon,
};

int Count(const UserAction* list)
{
    int n = 0;
    while (list[n] != UAN)
        ++n;
    return n;
}
} // namespace

ContextList ContextsForCategory(ControlsCategory cat)
{
    static constexpr InputContext onFoot[] = {InputContext::Infantry};
    static constexpr InputContext vehicles[] = {InputContext::CarDriver, InputContext::TankDriver,
                                                InputContext::ShipDriver};
    static constexpr InputContext helicopter[] = {InputContext::HeliPilot};
    static constexpr InputContext plane[] = {InputContext::PlanePilot};
    static constexpr InputContext gunner[] = {InputContext::TankGunner, InputContext::Gunner};
    static constexpr InputContext common[] = {
        InputContext::Menu,       InputContext::Infantry,  InputContext::CarDriver,  InputContext::TankDriver,
        InputContext::TankGunner, InputContext::HeliPilot, InputContext::PlanePilot, InputContext::ShipDriver,
        InputContext::Gunner,     InputContext::Spectator, InputContext::Map,        InputContext::Chat,
        InputContext::Editor,
    };

    switch (cat)
    {
        case ControlsCategoryOnFoot:
            return {onFoot, static_cast<int>(std::size(onFoot))};
        case ControlsCategoryVehicles:
            return {vehicles, static_cast<int>(std::size(vehicles))};
        case ControlsCategoryHelicopter:
            return {helicopter, static_cast<int>(std::size(helicopter))};
        case ControlsCategoryPlane:
            return {plane, static_cast<int>(std::size(plane))};
        case ControlsCategoryGunner:
            return {gunner, static_cast<int>(std::size(gunner))};
        case ControlsCategoryCommon:
            return {common, static_cast<int>(std::size(common))};
        default:
            return {onFoot, static_cast<int>(std::size(onFoot))};
    }
}

const UserAction* GetControlsCategoryActions(ControlsCategory cat)
{
    if (cat < 0 || cat >= ControlsCategoryCount)
        return kOnFoot;
    return kCategoryTable[cat];
}

int GetControlsCategoryActionCount(ControlsCategory cat)
{
    return Count(GetControlsCategoryActions(cat));
}

bool IsActionInControlsCategory(UserAction action, ControlsCategory cat)
{
    const UserAction* list = GetControlsCategoryActions(cat);
    for (int i = 0; list[i] != UAN; ++i)
        if (list[i] == action)
            return true;
    return false;
}

const char* GetControlsCategoryName(ControlsCategory cat)
{
    switch (cat)
    {
        case ControlsCategoryOnFoot:
            return "On foot";
        case ControlsCategoryVehicles:
            return "Vehicles";
        case ControlsCategoryHelicopter:
            return "Helicopter";
        case ControlsCategoryPlane:
            return "Plane";
        case ControlsCategoryGunner:
            return "Gunner";
        case ControlsCategoryCommon:
            return "Common";
        default:
            return "";
    }
}

bool IsActionVisibleOnKeyboard(UserAction action, ControlsCategory cat)
{
    // On-foot aim is the mouse on keyboard, so the four aim rows are hidden there.
    if (cat == ControlsCategoryOnFoot &&
        (action == UAAimUp || action == UAAimDown || action == UAAimLeft || action == UAAimRight))
        return false;
    return true;
}

bool IsActionVisibleOnGamepad(UserAction action, ControlsCategory cat)
{
    // Analog-stick group heads (movement / aim / freelook) are always shown.
    const bool movementHead = cat == ControlsCategoryOnFoot && action == UAMoveForward;
    const bool aimHead = (cat == ControlsCategoryOnFoot || cat == ControlsCategoryGunner) && action == UAAimRight;
    const bool freelookHead = action == UALookAround;
    if (movementHead || aimHead || freelookHead)
        return true;
    // Direction rows folded into a stick head are hidden on gamepad.
    if (cat == ControlsCategoryOnFoot && (action == UAMoveBack || action == UAMoveLeft || action == UAMoveRight ||
                                          action == UAAimUp || action == UAAimDown || action == UAAimLeft))
        return false;
    if (cat == ControlsCategoryGunner && (action == UAAimUp || action == UAAimDown || action == UAAimLeft))
        return false;
    if (action == UALookLeft || action == UALookRight || action == UALookUp || action == UALookDown ||
        action == UALookLeftUp || action == UALookRightUp || action == UALookLeftDown || action == UALookRightDown)
        return false;
    return true;
}
} // namespace Poseidon
