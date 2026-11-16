#include <Evaluator/express.hpp>
#include <Poseidon/AI/AICenter.hpp>
#include <Poseidon/Core/resincl.hpp>
#include <Poseidon/Foundation/Logging/Logging.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Shared/WindowMode.hpp>
#include <Poseidon/UI/Map/UIMap.hpp>
#include <SDL3/SDL_video.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>

using namespace Poseidon;

// Window and lifecycle verbs for the hitch scenarios (PERF-PLAN 3.5: resize,
// window mode, minimise and restore, a device reset, a mission retry). Each
// one drives the path the player's action takes and logs how long the call
// itself took ("[tri] <verb> ... ms=<n>"); the frames after it carry the rest
// of the hitch, which the frame rings (triPerfRing, triGpuRing) record. GLOBAL
// scope like the rest of the tri verb surface (registered, with forward decls,
// in GameStateExtTestAudio.cpp). With none of them called, play is unchanged.

namespace
{
using HitchClock = std::chrono::steady_clock;

double MsSince(HitchClock::time_point t0)
{
    return std::chrono::duration<double, std::milli>(HitchClock::now() - t0).count();
}

// The game's window: the first SDL window (the game opens one), or null
// without a video subsystem (the dummy renderer, unit tests). Not the keyboard
// focus: tri's hidden-desktop runs never have it.
SDL_Window* GameWindow()
{
    int count = 0;
    SDL_Window** windows = SDL_GetWindows(&count);
    SDL_Window* w = windows && count > 0 ? windows[0] : nullptr;
    SDL_free(windows);
    return w;
}

std::string ArgString(GameValuePar arg)
{
    if (arg.GetType() != GameString)
        return {};
    GameStringType s = static_cast<GameStringType>(arg);
    return (const char*)s ? (const char*)s : "";
}

std::string Ok(const char* what, double ms)
{
    char buf[96];
    std::snprintf(buf, sizeof(buf), "OK:%s:%.1f", what, ms);
    return buf;
}
} // namespace

/// Window mode words of triSetWindowMode: "fullscreen", "borderless",
/// "windowed". False for any other word.
bool TriParseWindowMode(const std::string& word, WindowMode& mode)
{
    if (word == "fullscreen")
        mode = WindowMode::Fullscreen;
    else if (word == "borderless")
        mode = WindowMode::Borderless;
    else if (word == "windowed")
        mode = WindowMode::Windowed;
    else
        return false;
    return true;
}

/// triResizeWindow [w, h]: resize the game window as a drag of its border
/// does, through Engine::SwitchRes at the current pixel size (windowed: the
/// window and the frame target follow; fullscreen: the display mode nearest to
/// w x h). w and h are 64..16384. Returns "OK:<W>x<H>:<ms>" with the engine's
/// size after the call (a window manager may grant another size) and the time
/// the call took, or "FAIL:<why>".
GameValue TriResizeWindow(const GameState* /*state*/, GameValuePar arg)
{
    if (!GEngine)
        return GameValue("FAIL:no engine");
    if (arg.GetType() != GameArray)
        return GameValue("FAIL:expected [w, h]");
    const GameArrayType& a = arg;
    if (a.Size() != 2 || a[0].GetType() != GameScalar || a[1].GetType() != GameScalar)
        return GameValue("FAIL:expected [w, h]");
    const int w = toInt(static_cast<float>(static_cast<GameScalarType>(a[0])));
    const int h = toInt(static_cast<float>(static_cast<GameScalarType>(a[1])));
    if (w < 64 || h < 64 || w > 16384 || h > 16384)
        return GameValue("FAIL:size out of range");
    const HitchClock::time_point t0 = HitchClock::now();
    const bool ok = GEngine->SwitchRes(w, h, GEngine->PixelSize());
    const double ms = MsSince(t0);
    char size[48];
    std::snprintf(size, sizeof(size), "%dx%d", GEngine->Width(), GEngine->Height());
    LOG_INFO(Core, "[tri] triResizeWindow {}x{} -> {} ok={} ms={:.1f}", w, h, size, ok ? 1 : 0, ms);
    if (!ok)
        return GameValue("FAIL:SwitchRes refused");
    return GameValue(Ok(size, ms).c_str());
}

/// triSetWindowMode "<fullscreen|borderless|windowed>": switch the window mode
/// as the Display screen does (Engine::SetWindowMode). The switch completes
/// asynchronously (SDL confirms it with events a few frames later); read it
/// back with triGetWindowMode. Returns "OK:<mode>:<ms>" (the call's own time)
/// or "FAIL:<why>".
GameValue TriSetWindowMode(const GameState* /*state*/, GameValuePar arg)
{
    if (!GEngine)
        return GameValue("FAIL:no engine");
    const std::string word = ArgString(arg);
    WindowMode mode = WindowMode::Windowed;
    if (!TriParseWindowMode(word, mode))
        return GameValue("FAIL:expected fullscreen, borderless or windowed");
    const HitchClock::time_point t0 = HitchClock::now();
    const bool ok = GEngine->SetWindowMode(mode);
    const double ms = MsSince(t0);
    LOG_INFO(Core, "[tri] triSetWindowMode {} ok={} ms={:.1f}", word, ok ? 1 : 0, ms);
    if (!ok)
        return GameValue("FAIL:SetWindowMode refused");
    return GameValue(Ok(word.c_str(), ms).c_str());
}

/// triMinimize: minimise the game window (SDL_MinimizeWindow), as the taskbar
/// button does; the engine sees the same window events. Returns "OK:<ms>" or
/// "FAIL:no window".
GameValue TriMinimize(const GameState* /*state*/)
{
    SDL_Window* w = GameWindow();
    if (!w)
        return GameValue("FAIL:no window");
    const HitchClock::time_point t0 = HitchClock::now();
    const bool ok = SDL_MinimizeWindow(w);
    const double ms = MsSince(t0);
    LOG_INFO(Core, "[tri] triMinimize ok={} ms={:.1f}", ok ? 1 : 0, ms);
    if (!ok)
        return GameValue("FAIL:SDL_MinimizeWindow");
    char buf[32];
    std::snprintf(buf, sizeof(buf), "OK:%.1f", ms);
    return GameValue(buf);
}

/// triRestore: restore the minimised game window (SDL_RestoreWindow). Returns
/// "OK:<ms>" or "FAIL:no window".
GameValue TriRestore(const GameState* /*state*/)
{
    SDL_Window* w = GameWindow();
    if (!w)
        return GameValue("FAIL:no window");
    const HitchClock::time_point t0 = HitchClock::now();
    const bool ok = SDL_RestoreWindow(w);
    const double ms = MsSince(t0);
    LOG_INFO(Core, "[tri] triRestore ok={} ms={:.1f}", ok ? 1 : 0, ms);
    if (!ok)
        return GameValue("FAIL:SDL_RestoreWindow");
    char buf[32];
    std::snprintf(buf, sizeof(buf), "OK:%.1f", ms);
    return GameValue(buf);
}

/// triGraphicsReset "<hard|content>": a device reset. "hard" (or "") rebuilds
/// every device resource with the window and context kept (Engine::ResetHard,
/// the development harness's reset); "content" drops what is tied to game
/// content, the textures above all, which reload on demand over the next
/// frames (Engine::ResetForRemount, the mod re-mount's reset). Logs "[tri]
/// triGraphicsReset <mode> ok=<0|1> ms=<n>" and returns "OK:<mode>:<ms>" or
/// "FAIL:<why>".
GameValue TriGraphicsReset(const GameState* /*state*/, GameValuePar arg)
{
    if (!GEngine)
        return GameValue("FAIL:no engine");
    std::string mode = ArgString(arg);
    if (mode.empty())
        mode = "hard";
    if (mode != "hard" && mode != "content")
        return GameValue("FAIL:expected hard or content");
    const HitchClock::time_point t0 = HitchClock::now();
    bool ok = true;
    if (mode == "hard")
        ok = GEngine->ResetHard();
    else
        GEngine->ResetForRemount();
    const double ms = MsSince(t0);
    LOG_INFO(Core, "[tri] triGraphicsReset {} ok={} ms={:.1f}", mode, ok ? 1 : 0, ms);
    if (!ok)
        return GameValue("FAIL:reset failed");
    return GameValue(Ok(mode.c_str(), ms).c_str());
}

/// triRetryMission: retry the running mission as the pause menu's Retry does
/// (DisplayMission::OnChildDestroyed with IDD_INTERRUPT / IDC_INT_RETRY, which
/// runs RetryMission: the autosave when there is one, else a fresh start of
/// the mission). Only from the mission display itself, with no menu or dialog
/// open over it, and only while the mission has a retry left (a retry with
/// none left aborts the mission). Returns "OK:<ms>" (the reload's time) or
/// "FAIL:<why>".
GameValue TriRetryMission(const GameState* /*state*/)
{
    Display* disp = CurrentDisplay();
    if (!disp || disp->IDD() != IDD_MISSION)
        return GameValue("FAIL:not on the mission display (a menu is open, or no mission)");
    if (GStats._mission._lives == 0)
        return GameValue("FAIL:no retry left");
    const HitchClock::time_point t0 = HitchClock::now();
    disp->OnChildDestroyed(IDD_INTERRUPT, IDC_INT_RETRY);
    const double ms = MsSince(t0);
    LOG_INFO(Core, "[tri] triRetryMission ms={:.1f}", ms);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "OK:%.1f", ms);
    return GameValue(buf);
}
