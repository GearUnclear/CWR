#include <Poseidon/UI/Settings/ContextControlsConfig.hpp>

#include <Poseidon/Input/InputBinding.hpp>
#include <Poseidon/Input/InputCode.hpp>
#include <Poseidon/Input/UserAction.hpp>
#include <SDL3/SDL_scancode.h>
#include <catch2/catch_test_macros.hpp>

#include "test_fixtures.hpp"

#include <array>
#include <filesystem>
#include <random>
#include <string>

using namespace Poseidon;

namespace
{
std::string TmpPath(const char* leaf)
{
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_int_distribution<unsigned> dist;
    auto root = std::filesystem::temp_directory_path() / ("context_controls_test_" + std::to_string(dist(rng)));
    std::filesystem::create_directories(root);
    return (root / leaf).string();
}
} // namespace

TEST_CASE("ContextControlsConfig: missing file returns false", "[Settings][ContextControlsConfig]")
{
    ContextControlsConfig cfg;
    CHECK_FALSE(cfg.Load(TmpPath("missing.cfg")));
}

TEST_CASE("ContextControlsConfig: Save then Load round-trips separate context profiles",
          "[Settings][ContextControlsConfig]")
{
    const std::string path = TmpPath("context_controls.cfg");
    std::filesystem::remove(path);

    ContextControlsConfig src;
    src.profiles[(int)InputContext::Infantry].Bind(
        UAMoveForward, InputBinding(InputCode::GamepadAx(1), {}, ActivationMode::OnHold, -1.0f));
    src.profiles[(int)InputContext::CarDriver].Bind(
        UAMoveForward, InputBinding(InputCode::GamepadAx(2), InputCode::GamepadBtn(6), ActivationMode::OnHold, 0.5f));
    src.profiles[(int)InputContext::Infantry].Bind(UAFire, InputCode::Key(SDL_SCANCODE_SPACE));

    REQUIRE(src.Save(path));

    ContextControlsConfig dst;
    REQUIRE(dst.Load(path));

    const auto& infantryMove = dst.profiles[(int)InputContext::Infantry].GetBindingEntries(UAMoveForward);
    REQUIRE(infantryMove.size() == 1);
    CHECK(infantryMove[0].code == InputCode::GamepadAx(1));
    CHECK_FALSE(infantryMove[0].modifier.valid());
    CHECK(infantryMove[0].scale == -1.0f);

    const auto& carMove = dst.profiles[(int)InputContext::CarDriver].GetBindingEntries(UAMoveForward);
    REQUIRE(carMove.size() == 1);
    CHECK(carMove[0].code == InputCode::GamepadAx(2));
    CHECK(carMove[0].modifier == InputCode::GamepadBtn(6));
    CHECK(carMove[0].scale == 0.5f);

    CHECK(dst.profiles[(int)InputContext::Infantry].HasBinding(UAFire, InputCode::Key(SDL_SCANCODE_SPACE)));
    CHECK_FALSE(dst.profiles[(int)InputContext::CarDriver].HasBinding(UAFire, InputCode::Key(SDL_SCANCODE_SPACE)));

    std::filesystem::remove(path);
}

TEST_CASE("ContextControlsConfig: Save then Load preserves an empty positional slot",
          "[Settings][ContextControlsConfig]")
{
    const std::string path = TmpPath("context_controls_empty_slot.cfg");
    std::filesystem::remove(path);

    // A cleared primary that keeps its alt: empty slot 0, a real binding in slot 1.
    // The empty placeholder must survive the round-trip so the alt does not shift
    // up into the primary on reload.
    ContextControlsConfig src;
    src.profiles[(int)InputContext::Infantry].Bind(UAMoveForward, InputBinding{});
    src.profiles[(int)InputContext::Infantry].Bind(UAMoveForward, InputCode::Key(SDL_SCANCODE_UP));

    REQUIRE(src.Save(path));

    ContextControlsConfig dst;
    REQUIRE(dst.Load(path));

    const auto& move = dst.profiles[(int)InputContext::Infantry].GetBindingEntries(UAMoveForward);
    REQUIRE(move.size() == 2);
    CHECK_FALSE(move[0].code.valid());                      // empty primary slot kept
    CHECK(move[1].code == InputCode::Key(SDL_SCANCODE_UP)); // alt still in slot 1

    std::filesystem::remove(path);
}

// A real, full contextControls.cfg captured from a version-2 user profile, from
// before several actions existed. Loading and copying the profiles is the path
// InputSubsystem::LoadKeys runs: listed bindings are preserved, and actions the
// file lacks are seeded with their defaults.
TEST_CASE("ContextControlsConfig: an older config keeps its bindings and defaults new actions",
          "[Settings][ContextControlsConfig]")
{
    REQUIRE_FIXTURE("cfg/contextControls_prior.cfg");

    ContextControlsConfig cfg;
    REQUIRE(cfg.Load(GET_FIXTURE("cfg/contextControls_prior.cfg")));
    CHECK(cfg.migratedOnLoad);

    // The array copy that used to fault when object files disagreed on UAN.
    std::array<InputProfile, ContextControlsConfig::ContextCount> copy = cfg.profiles;

    // Single keyboard binding.
    const auto& von = copy[(int)InputContext::Infantry].GetBindingEntries(UAVoiceOverNet);
    REQUIRE(von.size() == 1);
    CHECK(von[0].code.toLegacy() == 57);

    // Multi-binding action (keyboard + gamepad) round-trips both codes in order.
    const auto& fire = copy[(int)InputContext::Infantry].GetBindingEntries(UAFire);
    REQUIRE(fire.size() == 2);
    CHECK(fire[0].code.toLegacy() == 224);
    CHECK(fire[1].code.toLegacy() == 131079);

    // A binding in a different context, to prove per-context separation held.
    const auto& chat = copy[(int)InputContext::Chat].GetBindingEntries(UAChat);
    REQUIRE(chat.size() == 1);
    CHECK(chat[0].code.toLegacy() == 56);

    // Push-to-talk was absent from the file; migration seeds its CapsLock default.
    CHECK(
        copy[(int)InputContext::Infantry].HasBinding(UAVoiceOverNetPushToTalk, InputCode::Key(SDL_SCANCODE_CAPSLOCK)));
}

// A complete version-2 config, as the current engine writes it minus the actions
// added since (map zoom, cheat entry). It parses across every context; the actions
// it lists are kept, the ones it predates are seeded to their defaults, and a save
// then reload comes up current with the fill persisted.
TEST_CASE("ContextControlsConfig: a full version-2 config parses and migrates to 3",
          "[Settings][ContextControlsConfig]")
{
    REQUIRE_FIXTURE("cfg/contextControls_v2_full.cfg");
    const int inf = (int)InputContext::Infantry;

    ContextControlsConfig migrated;
    REQUIRE(migrated.Load(GET_FIXTURE("cfg/contextControls_v2_full.cfg")));
    CHECK(migrated.migratedOnLoad);

    // Listed v2 actions parse and keep their values.
    CHECK(migrated.profiles[inf].BindingCount(UAFire) > 0);
    CHECK(migrated.profiles[inf].HasBinding(UAVoiceOverNetPushToTalk, InputCode::Key(SDL_SCANCODE_CAPSLOCK)));
    // The Map-context optics ZoomIn (ctxMapZoomIn) is name-adjacent to the new
    // MapZoomIn action but stays its own binding.
    CHECK(migrated.profiles[(int)InputContext::Map].BindingCount(UAZoomIn) > 0);

    // Actions the file predates are seeded to their defaults, including the cheat
    // trigger's Shift + Numpad-Minus combo.
    CHECK(migrated.profiles[inf].HasBinding(UAMapZoomIn, InputCode::Key(SDL_SCANCODE_KP_PLUS)));
    CHECK(migrated.profiles[inf].HasBinding(UAMapZoomOut, InputCode::Key(SDL_SCANCODE_KP_MINUS)));
    const auto& cheat = migrated.profiles[inf].GetBindingEntries(UACheatEntry);
    REQUIRE(cheat.size() == 1);
    CHECK(cheat[0].code == InputCode::Key(SDL_SCANCODE_KP_MINUS));
    CHECK(cheat[0].modifier == InputCode::Key(SDL_SCANCODE_LSHIFT));

    // Save the modernized config and reload it: now current, no second migration,
    // seeded defaults still present.
    const std::string path = TmpPath("context_controls_v2_full_migrated.cfg");
    std::filesystem::remove(path);
    REQUIRE(migrated.Save(path));

    ContextControlsConfig reloaded;
    REQUIRE(reloaded.Load(path));
    CHECK_FALSE(reloaded.migratedOnLoad);
    CHECK(reloaded.profiles[inf].HasBinding(UAMapZoomIn, InputCode::Key(SDL_SCANCODE_KP_PLUS)));
    CHECK(reloaded.profiles[inf].HasBinding(UAVoiceOverNetPushToTalk, InputCode::Key(SDL_SCANCODE_CAPSLOCK)));

    std::filesystem::remove(path);
}

// A version-1 config, older still: it predates push-to-talk as well as the newest
// actions. Loading migrates it across the two-version gap, keeping its bindings and
// seeding every action it lacks - push-to-talk, map zoom, and cheat entry.
TEST_CASE("ContextControlsConfig: a full version-1 config parses and migrates", "[Settings][ContextControlsConfig]")
{
    REQUIRE_FIXTURE("cfg/contextControls_v1_full.cfg");
    const int inf = (int)InputContext::Infantry;

    ContextControlsConfig migrated;
    REQUIRE(migrated.Load(GET_FIXTURE("cfg/contextControls_v1_full.cfg")));
    CHECK(migrated.migratedOnLoad);

    // A listed binding is kept.
    CHECK(migrated.profiles[inf].BindingCount(UAFire) > 0);

    // Every action the file predates is seeded to its default.
    CHECK(migrated.profiles[inf].HasBinding(UAVoiceOverNetPushToTalk, InputCode::Key(SDL_SCANCODE_CAPSLOCK)));
    CHECK(migrated.profiles[inf].HasBinding(UAMapZoomIn, InputCode::Key(SDL_SCANCODE_KP_PLUS)));
    CHECK(migrated.profiles[inf].HasBinding(UAMapZoomOut, InputCode::Key(SDL_SCANCODE_KP_MINUS)));
    CHECK(migrated.profiles[inf].BindingCount(UACheatEntry) == 1);

    // Save and reload comes up current with the fill persisted.
    const std::string path = TmpPath("context_controls_v1_full_migrated.cfg");
    std::filesystem::remove(path);
    REQUIRE(migrated.Save(path));

    ContextControlsConfig reloaded;
    REQUIRE(reloaded.Load(path));
    CHECK_FALSE(reloaded.migratedOnLoad);
    CHECK(reloaded.profiles[inf].HasBinding(UAVoiceOverNetPushToTalk, InputCode::Key(SDL_SCANCODE_CAPSLOCK)));
    CHECK(reloaded.profiles[inf].HasBinding(UAMapZoomIn, InputCode::Key(SDL_SCANCODE_KP_PLUS)));

    std::filesystem::remove(path);
}

#include <Poseidon/Input/InputDeviceConstants.hpp>
#include <fstream>
#include <sstream>

namespace
{
std::string ReadAll(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void ReplaceLine(std::string& text, const std::string& from, const std::string& to)
{
    const size_t at = text.find(from);
    REQUIRE(at != std::string::npos);
    text.replace(at, from.size(), to);
}
} // namespace

// A version-3 file written by the pre-Arma-3-right-mouse build: Optics = V +
// Numpad 0, LockTarget = RMB, Watch = T, no ZoomTemp.  Loading must rewrite the
// three pristine KB&M defaults to their v4 values (keeping the gamepad entries
// that share the same array), seed ZoomTemp, and leave everything else alone.
TEST_CASE("ContextControlsConfig: a full version-3 config migrates to 4", "[Settings][ContextControlsConfig]")
{
    REQUIRE_FIXTURE("cfg/contextControls_v3_full.cfg");
    const int inf = (int)InputContext::Infantry;
    const InputCode tapRmb = InputCode::FromLegacy(InputBindingTapCode(INPUT_DEVICE_MOUSE + 1));
    const InputCode rmb = InputCode::MouseButton(1);

    ContextControlsConfig migrated;
    REQUIRE(migrated.Load(GET_FIXTURE("cfg/contextControls_v3_full.cfg")));
    CHECK(migrated.migratedOnLoad);

    const auto& optics = migrated.profiles[inf].GetBindingEntries(UAOptics);
    REQUIRE(optics.size() == 3);
    CHECK(optics[0].code == tapRmb);
    CHECK(optics[1].code == InputCode::Key(SDL_SCANCODE_V));
    CHECK(optics[2].code == InputCode::GamepadBtn(6)); // LT, preserved from the v3 array
    CHECK_FALSE(migrated.profiles[inf].HasBinding(UAOptics, InputCode::Key(SDL_SCANCODE_KP_0)));

    const auto& lock = migrated.profiles[inf].GetBindingEntries(UALockTarget);
    REQUIRE(lock.size() == 1);
    CHECK(lock[0].code == InputCode::Key(SDL_SCANCODE_T));

    const auto& reveal = migrated.profiles[inf].GetBindingEntries(UARevealTarget);
    REQUIRE(reveal.size() == 1);
    CHECK(reveal[0].code == rmb);

    const auto& watch = migrated.profiles[inf].GetBindingEntries(UAWatch);
    REQUIRE(watch.size() == 2);
    CHECK(watch[0].code == InputCode::Key(SDL_SCANCODE_O));
    CHECK(watch[1].code == InputCode::GamepadPov(2));

    const auto& zoomTemp = migrated.profiles[inf].GetBindingEntries(UAZoomTemp);
    REQUIRE(zoomTemp.size() == 1);
    CHECK(zoomTemp[0].code == rmb);

    // A context without the gamepad entries migrates to the bare KB&M list.
    const auto& carOptics = migrated.profiles[(int)InputContext::CarDriver].GetBindingEntries(UAOptics);
    REQUIRE(carOptics.size() == 2);
    CHECK(carOptics[0].code == tapRmb);
    CHECK(carOptics[1].code == InputCode::Key(SDL_SCANCODE_V));

    // Untouched v3 rows keep their values.
    CHECK(migrated.profiles[inf].HasBinding(UAFire, InputCode::Key(SDL_SCANCODE_LCTRL)));
    CHECK(migrated.profiles[inf].HasBinding(UAVoiceOverNetPushToTalk, InputCode::Key(SDL_SCANCODE_CAPSLOCK)));

    // Save, reload: now current, no second migration, the tap flag persisted.
    const std::string path = TmpPath("context_controls_v3_full_migrated.cfg");
    std::filesystem::remove(path);
    REQUIRE(migrated.Save(path));
    const std::string saved = ReadAll(path);
    CHECK(saved.find("contextControlsVersion=4;") != std::string::npos);
    CHECK(saved.find("ctxInfantryOptics[]={81921,25,131078};") != std::string::npos);
    CHECK(saved.find("ctxInfantryLockTarget[]={23};") != std::string::npos);
    CHECK(saved.find("ctxInfantryWatch[]={18,262146};") != std::string::npos);
    CHECK(saved.find("ctxInfantryZoomTemp[]={65537};") != std::string::npos);

    ContextControlsConfig reloaded;
    REQUIRE(reloaded.Load(path));
    CHECK_FALSE(reloaded.migratedOnLoad);
    const auto& reOptics = reloaded.profiles[inf].GetBindingEntries(UAOptics);
    REQUIRE(reOptics.size() == 3);
    CHECK(reOptics[0].code == tapRmb);
    CHECK(reloaded.profiles[inf].HasBinding(UALockTarget, InputCode::Key(SDL_SCANCODE_T)));
    CHECK(reloaded.profiles[inf].HasBinding(UAWatch, InputCode::Key(SDL_SCANCODE_O)));
    CHECK(reloaded.profiles[inf].HasBinding(UAZoomTemp, rmb));

    std::filesystem::remove(path);
}

TEST_CASE("ContextControlsConfig: customized v3 rows are left alone by the v4 rewrite",
          "[Settings][ContextControlsConfig]")
{
    REQUIRE_FIXTURE("cfg/contextControls_v3_full.cfg");
    const int inf = (int)InputContext::Infantry;

    // The user had rebound Infantry Optics to C (+ Numpad 0, + LT) and
    // Infantry LockTarget to Q; both are non-default and must survive.
    std::string text = ReadAll(GET_FIXTURE("cfg/contextControls_v3_full.cfg"));
    ReplaceLine(text, "ctxInfantryOptics[]={25,98,131078};", "ctxInfantryOptics[]={6,98,131078};");
    ReplaceLine(text, "ctxInfantryLockTarget[]={65537};", "ctxInfantryLockTarget[]={20};");

    const std::string path = TmpPath("context_controls_v3_customized.cfg");
    {
        std::ofstream out(path, std::ios::binary);
        out << text;
    }

    ContextControlsConfig cfg;
    REQUIRE(cfg.Load(path));
    CHECK(cfg.migratedOnLoad);

    const auto& optics = cfg.profiles[inf].GetBindingEntries(UAOptics);
    REQUIRE(optics.size() == 3);
    CHECK(optics[0].code == InputCode::Key(SDL_SCANCODE_C));
    CHECK(optics[1].code == InputCode::Key(SDL_SCANCODE_KP_0));
    CHECK(optics[2].code == InputCode::GamepadBtn(6));

    const auto& lock = cfg.profiles[inf].GetBindingEntries(UALockTarget);
    REQUIRE(lock.size() == 1);
    CHECK(lock[0].code == InputCode::Key(SDL_SCANCODE_Q));

    // Pristine rows elsewhere still migrate, and ZoomTemp is still seeded.
    CHECK(cfg.profiles[inf].HasBinding(UAWatch, InputCode::Key(SDL_SCANCODE_O)));
    const auto& carOptics = cfg.profiles[(int)InputContext::CarDriver].GetBindingEntries(UAOptics);
    REQUIRE(carOptics.size() == 2);
    CHECK(carOptics[0].code == InputCode::FromLegacy(InputBindingTapCode(INPUT_DEVICE_MOUSE + 1)));
    CHECK(cfg.profiles[inf].HasBinding(UAZoomTemp, InputCode::MouseButton(1)));

    std::filesystem::remove(path);
}

TEST_CASE("ContextControlsConfig: a v3 row with a modifier or an empty slot counts as customized",
          "[Settings][ContextControlsConfig]")
{
    REQUIRE_FIXTURE("cfg/contextControls_v3_full.cfg");
    const int inf = (int)InputContext::Infantry;

    std::string text = ReadAll(GET_FIXTURE("cfg/contextControls_v3_full.cfg"));
    // Watch = Shift+T (modifier), LockTarget = [empty, RMB] (cleared primary).
    ReplaceLine(text, "ctxInfantryWatch_mod[]={-1,-1};", "ctxInfantryWatch_mod[]={225,-1};");
    ReplaceLine(text, "ctxInfantryLockTarget[]={65537};", "ctxInfantryLockTarget[]={0,65537};");
    ReplaceLine(text, "ctxInfantryLockTarget_mod[]={-1};", "ctxInfantryLockTarget_mod[]={-1,-1};");
    ReplaceLine(text, "ctxInfantryLockTarget_scale[]={1.000000};",
                "ctxInfantryLockTarget_scale[]={1.000000,1.000000};");

    const std::string path = TmpPath("context_controls_v3_modified.cfg");
    {
        std::ofstream out(path, std::ios::binary);
        out << text;
    }

    ContextControlsConfig cfg;
    REQUIRE(cfg.Load(path));

    const auto& watch = cfg.profiles[inf].GetBindingEntries(UAWatch);
    REQUIRE(watch.size() == 2);
    CHECK(watch[0].code == InputCode::Key(SDL_SCANCODE_T));
    CHECK(watch[0].modifier == InputCode::Key(SDL_SCANCODE_LSHIFT));

    const auto& lock = cfg.profiles[inf].GetBindingEntries(UALockTarget);
    REQUIRE(lock.size() == 2);
    CHECK_FALSE(lock[0].code.valid());
    CHECK(lock[1].code == InputCode::MouseButton(1));

    std::filesystem::remove(path);
}

TEST_CASE("ContextControlsConfig: a file newer than the build is treated as unknown layout",
          "[Settings][ContextControlsConfig]")
{
    // An experimental build once wrote version 5 with a v3 layout to a real
    // profile.  A version above ours must not freeze that layout in place: seed
    // the missing actions, apply the equality-gated rewrite, and re-save at our
    // version so the next boot is a plain current-version load.
    REQUIRE_FIXTURE("cfg/contextControls_v3_full.cfg");
    const int inf = (int)InputContext::Infantry;

    std::string text = ReadAll(GET_FIXTURE("cfg/contextControls_v3_full.cfg"));
    ReplaceLine(text, "contextControlsVersion=3;", "contextControlsVersion=99;");
    // Keep one customised row to prove the gate still protects it.
    ReplaceLine(text, "ctxInfantryOptics[]={25,98,131078};", "ctxInfantryOptics[]={25,65540,131078};");

    const std::string path = TmpPath("context_controls_newer_than_build.cfg");
    {
        std::ofstream out(path, std::ios::binary);
        out << text;
    }

    ContextControlsConfig cfg;
    REQUIRE(cfg.Load(path));
    CHECK(cfg.migratedOnLoad);
    CHECK(cfg.profiles[inf].HasBinding(UALockTarget, InputCode::Key(SDL_SCANCODE_T)));
    CHECK(cfg.profiles[inf].HasBinding(UAWatch, InputCode::Key(SDL_SCANCODE_O)));
    CHECK(cfg.profiles[inf].HasBinding(UAZoomTemp, InputCode::MouseButton(1)));
    const auto& optics = cfg.profiles[inf].GetBindingEntries(UAOptics);
    REQUIRE(optics.size() == 3);
    CHECK(optics[0].code == InputCode::Key(SDL_SCANCODE_V));
    CHECK(optics[1].code == InputCode::MouseButton(4));

    REQUIRE(cfg.Save(path));
    CHECK(ReadAll(path).find("contextControlsVersion=4;") != std::string::npos);
    ContextControlsConfig reloaded;
    REQUIRE(reloaded.Load(path));
    CHECK_FALSE(reloaded.migratedOnLoad);
    std::filesystem::remove(path);
}
