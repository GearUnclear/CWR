#include <Poseidon/UI/Settings/ContextControlsConfig.hpp>

#include <Poseidon/Input/InputBinding.hpp>
#include <Poseidon/Input/InputDeviceConstants.hpp>
#include <Poseidon/Input/InputSubsystem.hpp>
#include <Poseidon/Input/UserActionDesc.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/UI/Settings/SettingsFile.hpp>

#include <Poseidon/Foundation/Framework/Log.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

#include <iterator>

namespace Poseidon
{
namespace
{
// v4 (2026-09): Arma 3 right mouse.  Optics = tap RMB + V (Numpad 0 dropped),
// LockTarget = T (was RMB, "Lock or Zoom"), Watch = O (was T), new ZoomTemp = RMB.
constexpr int kContextControlsVersion = 4;
constexpr int kGamepadButtonA = 0;
constexpr int kGamepadButtonB = 1;
constexpr int kGamepadButtonX = 2;
constexpr int kGamepadButtonY = 3;
constexpr int kGamepadButtonLB = 4;
constexpr int kGamepadButtonRB = 5;
constexpr int kGamepadButtonLT = 6;
constexpr int kGamepadButtonRT = 7;
constexpr int kGamepadButtonBack = 8;
constexpr int kGamepadButtonStart = 9;
constexpr int kGamepadButtonRightStick = 11;

constexpr int kGamepadPovUp = 0;
constexpr int kGamepadPovRight = 2;
constexpr int kGamepadPovDown = 4;
constexpr int kGamepadPovLeft = 6;

constexpr int kGamepadAxisLeftX = 0;
constexpr int kGamepadAxisLeftY = 1;
constexpr int kGamepadAxisRightX = 3;
constexpr int kGamepadAxisRightY = 4;
constexpr int kGamepadAxisLeftTrigger = 5;
constexpr int kGamepadAxisRightTrigger = 2;

const char* ContextPrefix(InputContext ctx)
{
    switch (ctx)
    {
        case InputContext::Menu:
            return "ctxMenu";
        case InputContext::Infantry:
            return "ctxInfantry";
        case InputContext::CarDriver:
            return "ctxCarDriver";
        case InputContext::TankDriver:
            return "ctxTankDriver";
        case InputContext::TankGunner:
            return "ctxTankGunner";
        case InputContext::HeliPilot:
            return "ctxHeliPilot";
        case InputContext::PlanePilot:
            return "ctxPlanePilot";
        case InputContext::ShipDriver:
            return "ctxShipDriver";
        case InputContext::Gunner:
            return "ctxGunner";
        case InputContext::Spectator:
            return "ctxSpectator";
        case InputContext::Map:
            return "ctxMap";
        case InputContext::Chat:
            return "ctxChat";
        case InputContext::Editor:
            return "ctxEditor";
        default:
            return "ctxUnknown";
    }
}

RString BindingName(InputContext ctx, const UserActionDesc& desc)
{
    return RString(ContextPrefix(ctx)) + RString(desc.name);
}

RString ModifierName(InputContext ctx, const UserActionDesc& desc)
{
    return BindingName(ctx, desc) + RString("_mod");
}

RString ScaleName(InputContext ctx, const UserActionDesc& desc)
{
    return BindingName(ctx, desc) + RString("_scale");
}

void BindAxis(InputProfile& profile, UserAction action, int axis, float scale)
{
    profile.Bind(action, InputBinding(InputCode::GamepadAx(axis), InputCode{}, ActivationMode::OnHold, scale));
}

void BindAxis(InputProfile& profile, UserAction action, int axis, InputCode modifier, float scale)
{
    profile.Bind(action, InputBinding(InputCode::GamepadAx(axis), modifier, ActivationMode::OnHold, scale));
}

void BindButton(InputProfile& profile, UserAction action, int button)
{
    profile.Bind(action, InputCode::GamepadBtn(button));
}

void BindButton(InputProfile& profile, UserAction action, int button, InputCode modifier)
{
    profile.Bind(action, InputBinding(InputCode::GamepadBtn(button), modifier));
}

void BindPov(InputProfile& profile, UserAction action, int pov)
{
    profile.Bind(action, InputCode::GamepadPov(pov));
}

void BindPov(InputProfile& profile, UserAction action, int pov, InputCode modifier)
{
    profile.Bind(action, InputBinding(InputCode::GamepadPov(pov), modifier));
}

void ApplyCommonGamepadDefaults(InputProfile& profile)
{
    BindPov(profile, UAPrevAction, kGamepadPovUp);
    BindPov(profile, UANextAction, kGamepadPovDown);
    BindButton(profile, UAAction, kGamepadButtonA);

    BindButton(profile, UAMap, kGamepadButtonBack);
    BindPov(profile, UACompass, kGamepadPovLeft);
    BindPov(profile, UAWatch, kGamepadPovRight);
    BindButton(profile, UAHelp, kGamepadButtonStart);
    BindButton(profile, UAPersonView, kGamepadButtonB);
}

void ApplyCombatGamepadDefaults(InputProfile& profile)
{
    BindButton(profile, UAFire, kGamepadButtonRT);
    BindButton(profile, UAReloadMagazine, kGamepadButtonY);
    BindButton(profile, UAToggleWeapons, kGamepadButtonX);
    BindButton(profile, UAOptics, kGamepadButtonLT);
    BindButton(profile, UALookCenter, kGamepadButtonRightStick);
}

void ApplyInfantryGamepadDefaults(InputProfile& profile)
{
    ApplyCombatGamepadDefaults(profile);

    BindAxis(profile, UAMoveForward, kGamepadAxisLeftY, -1.0f);
    BindAxis(profile, UAMoveBack, kGamepadAxisLeftY, 1.0f);
    BindAxis(profile, UAMoveLeft, kGamepadAxisLeftX, -1.0f);
    BindAxis(profile, UAMoveRight, kGamepadAxisLeftX, 1.0f);

    BindAxis(profile, UAAimUp, kGamepadAxisRightY, -1.0f);
    BindAxis(profile, UAAimDown, kGamepadAxisRightY, 1.0f);
    BindAxis(profile, UAAimLeft, kGamepadAxisRightX, -1.0f);
    BindAxis(profile, UAAimRight, kGamepadAxisRightX, 1.0f);
}

void ApplyFreelookDefaults(InputProfile& profile)
{
    InputCode lb = InputCode::GamepadBtn(kGamepadButtonLB);
    profile.Bind(UALookAround, lb);
    BindAxis(profile, UALookUp, kGamepadAxisRightY, lb, -1.0f);
    BindAxis(profile, UALookDown, kGamepadAxisRightY, lb, 1.0f);
    BindAxis(profile, UALookLeft, kGamepadAxisRightX, lb, -1.0f);
    BindAxis(profile, UALookRight, kGamepadAxisRightX, lb, 1.0f);
}

void ApplyDirectFreelookDefaults(InputProfile& profile)
{
    BindAxis(profile, UALookUp, kGamepadAxisRightY, -1.0f);
    BindAxis(profile, UALookDown, kGamepadAxisRightY, 1.0f);
    BindAxis(profile, UALookLeft, kGamepadAxisRightX, -1.0f);
    BindAxis(profile, UALookRight, kGamepadAxisRightX, 1.0f);
    BindButton(profile, UALookCenter, kGamepadButtonRightStick);
}

void ApplyDriverGamepadDefaults(InputProfile& profile)
{
    BindAxis(profile, UAMoveForward, kGamepadAxisRightTrigger, 1.0f);
    BindAxis(profile, UAMoveBack, kGamepadAxisLeftTrigger, 1.0f);
    BindAxis(profile, UATurnLeft, kGamepadAxisLeftX, -1.0f);
    BindAxis(profile, UATurnRight, kGamepadAxisLeftX, 1.0f);
    BindButton(profile, UATurbo, kGamepadButtonRB);
    BindButton(profile, UAFire, kGamepadButtonLB);
}

void ApplyContextDefaults(InputContext ctx, InputProfile& profile)
{
    ApplyCommonGamepadDefaults(profile);

    switch (ctx)
    {
        case InputContext::Infantry:
            ApplyInfantryGamepadDefaults(profile);
            ApplyFreelookDefaults(profile);
            break;
        case InputContext::CarDriver:
            ApplyDriverGamepadDefaults(profile);
            ApplyDirectFreelookDefaults(profile);
            break;
        case InputContext::Map:
        case InputContext::Menu:
            BindButton(profile, UAMapZoomIn, kGamepadButtonRB);
            BindButton(profile, UAMapZoomOut, kGamepadButtonLB);
            break;
        default:
            break;
    }
}

// ---- v3 -> v4 rewrite of the three actions whose KB&M defaults changed ----
//
// Load() seeds defaults for actions a pre-v4 file lacks (ZoomTemp), but an
// action the file already lists is taken verbatim, so a changed default never
// reaches an existing profile on its own.  For the three re-bound actions we
// rewrite the KB&M part of each context ONLY when it still equals the v3 default
// exactly (same codes in order, no modifiers, scale 1, no empty slots); anything
// else counts as customised and is left alone.  Gamepad codes live in the same
// array (e.g. ctxInfantryOptics[]={25,98,131078}) and are preserved in order.
struct V3RewriteEntry
{
    UserAction action;
    int v3Keys[2]; // legacy packed KB&M codes; -1 = unused slot
};

constexpr V3RewriteEntry kV3Rewrites[] = {
    {UAOptics, {SDL_SCANCODE_V, SDL_SCANCODE_KP_0}},
    {UALockTarget, {INPUT_DEVICE_MOUSE + 1, -1}},
    {UAWatch, {SDL_SCANCODE_T, -1}},
};

bool IsKbmCode(InputCode code)
{
    return code.device() == InputDevice::Keyboard || code.device() == InputDevice::Mouse;
}

void RewriteV3DefaultsToV4(InputProfile& profile, const UserActionDesc* descs)
{
    for (size_t e = 0; e < std::size(kV3Rewrites); ++e)
    {
        const V3RewriteEntry& entry = kV3Rewrites[e];
        const std::vector<InputBinding>& bindings = profile.GetBindingEntries(entry.action);

        // Split into the KB&M subsequence (empty slots count as KB&M: they are
        // positional KB&M cells) and the rest, both in original order.
        std::vector<InputBinding> kbm;
        std::vector<InputBinding> rest;
        for (const InputBinding& b : bindings)
        {
            if (!b.code.valid() || IsKbmCode(b.code))
                kbm.push_back(b);
            else
                rest.push_back(b);
        }

        int v3Count = 0;
        for (int k : entry.v3Keys)
            if (k >= 0)
                ++v3Count;
        if (static_cast<int>(kbm.size()) != v3Count)
            continue;
        bool pristine = true;
        for (int i = 0; i < v3Count && pristine; ++i)
        {
            const InputBinding& b = kbm[i];
            pristine = b.code.valid() && b.code == InputCode::FromLegacy(entry.v3Keys[i]) && !b.modifier.valid() &&
                       b.scale == 1.0f;
        }
        if (!pristine)
            continue;

        profile.ClearBindings(entry.action);
        const KeyList& defaults = descs[entry.action].keys;
        for (int j = 0; j < defaults.Size(); ++j)
        {
            InputCode code = InputCode::FromLegacy(defaults[j]);
            if (!code.valid() || !IsKbmCode(code))
                continue;
            int mod = DefaultModifierForDefaultKey(entry.action, defaults[j]);
            InputCode modCode = mod >= 0 ? InputCode::FromLegacy(mod) : InputCode{};
            profile.Bind(entry.action, InputBinding(code, modCode));
        }
        for (const InputBinding& b : rest)
            profile.Bind(entry.action, b);
    }
}
} // namespace

void ContextControlsConfig::LoadDefaults()
{
    for (int i = 0; i < ContextCount; ++i)
    {
        profiles[i].LoadDefaults();
        ApplyContextDefaults(static_cast<InputContext>(i), profiles[i]);
    }
}

bool ContextControlsConfig::Load(const std::string& path)
{
    ParamFile cfg;
    if (!ReadSettingsFile(path, cfg))
        return false;

    int version = 0;
    if (auto* e = cfg.FindEntry("contextControlsVersion"))
        version = (int)*e;

    // A file written before newer actions existed has no entries for them. Seed
    // each profile with defaults first so those actions come up bound, then let
    // the file override the actions it does list.
    //
    // A file claiming a version ABOVE this build's (written by a newer or an
    // experimental build) cannot be trusted to carry this build's layout either:
    // treat it the same way, and re-save it at our version. The v3->v4 rewrite
    // below is equality-gated, so a customised or already-v4 row is untouched.
    const bool newerThanBuild = version > kContextControlsVersion;
    if (newerThanBuild)
        LOG_WARN(Config, "contextControls.cfg version {} is newer than this build's {}; treating its layout as unknown",
                 version, kContextControlsVersion);
    const bool seedDefaults = version < kContextControlsVersion || newerThanBuild;
    migratedOnLoad = seedDefaults;

    UserActionDesc* descs = InputSubsystem::GetUserActionDesc();
    for (int c = 0; c < ContextCount; ++c)
    {
        InputContext ctx = static_cast<InputContext>(c);
        InputProfile& profile = profiles[c];
        profile.ClearAll();
        if (seedDefaults)
        {
            profile.LoadDefaults();
            ApplyContextDefaults(ctx, profile);
        }
        for (int a = 0; a < UAN; ++a)
        {
            const ParamEntry* entry = cfg.FindEntry(BindingName(ctx, descs[a]));
            if (!entry)
                continue;
            profile.ClearBindings(static_cast<UserAction>(a));

            const ParamEntry* modEntry = cfg.FindEntry(ModifierName(ctx, descs[a]));
            const ParamEntry* scaleEntry = cfg.FindEntry(ScaleName(ctx, descs[a]));
            const int n = entry->GetSize();
            for (int i = 0; i < n; ++i)
            {
                InputCode code = InputCode::FromLegacy((int)(*entry)[i]);
                if (!code.valid())
                {
                    // Preserve an empty positional slot (a cleared primary that
                    // keeps its alt): gameplay skips it, the controls page shows a dash.
                    profile.Bind(static_cast<UserAction>(a), InputBinding{});
                    continue;
                }

                int modRaw = -1;
                if (modEntry && i < modEntry->GetSize())
                    modRaw = (int)(*modEntry)[i];
                InputCode modifier = modRaw >= 0 ? InputCode::FromLegacy(modRaw) : InputCode{};

                float scale = 1.0f;
                if (scaleEntry && i < scaleEntry->GetSize())
                    scale = (float)(*scaleEntry)[i];

                profile.Bind(static_cast<UserAction>(a), InputBinding(code, modifier, ActivationMode::OnHold, scale));
            }
        }
        if (version < 4 || newerThanBuild)
            RewriteV3DefaultsToV4(profile, descs);
    }

    return true;
}

bool ContextControlsConfig::Save(const std::string& path) const
{
    ParamFile cfg;
    cfg.Add("contextControlsVersion", kContextControlsVersion);

    UserActionDesc* descs = InputSubsystem::GetUserActionDesc();
    for (int c = 0; c < ContextCount; ++c)
    {
        InputContext ctx = static_cast<InputContext>(c);
        const InputProfile& profile = profiles[c];
        for (int a = 0; a < UAN; ++a)
        {
            const auto& entries = profile.GetBindingEntries(static_cast<UserAction>(a));
            if (entries.empty())
                continue;

            ParamEntry* codes = cfg.AddArray(BindingName(ctx, descs[a]));
            codes->Clear();
            ParamEntry* mods = cfg.AddArray(ModifierName(ctx, descs[a]));
            mods->Clear();
            ParamEntry* scales = cfg.AddArray(ScaleName(ctx, descs[a]));
            scales->Clear();

            for (const InputBinding& binding : entries)
            {
                codes->AddValue(binding.code.toLegacy());
                mods->AddValue(binding.modifier.valid() ? binding.modifier.toLegacy() : -1);
                scales->AddValue(binding.scale);
            }
        }
    }

    return WriteSettingsFile(path, cfg);
}
} // namespace Poseidon
