"""Where the trailer tools find things.

Every location can be set with an environment variable; the defaults follow the repo
layout, so no tool names one machine's folders.

    UD_VIDEO_WORK      the trailer's work folder (clips, frames, montages, voice, final)
                       default: <repo>/tmp/video/v2 (tmp/ is gitignored)
    UD_VANILLA_ROOT    the checkout of Bohemia's upstream engine that shoots the vanilla side
                       default: a sibling folder of this checkout named vanilla-upstream
    UD_VANILLA_LOBO    @LoBo addons and dta (no Bin) mounted by the vanilla side
                       default: <UD_VANILLA_ROOT>/tmp/lobo-mods
    UD_FONT_DIR        the game's OFL fonts; default: <repo>/arma_CWR/thirdparty/guerrilla-fonts
    UD_WHISPER_PYTHON  a Python with faster-whisper; default: the Python running the tool
    UD_TTS_OUT         the WAVs of a TTS take; default: <UD_VIDEO_WORK>/tts
    UD_ENV_FILE        the .env holding OPENROUTER_API_KEY; default: the main checkout's .env
    FFMPEG, FFPROBE    the ffmpeg tools; default: found on PATH
"""
import os
import shutil
import subprocess
import sys
from pathlib import Path

VIDEO_DIR = Path(__file__).resolve().parent
ENGINE_ROOT = VIDEO_DIR.parents[3]  # arma_CWR
REPO_ROOT = VIDEO_DIR.parents[4]


def _env_path(name, default):
    value = os.environ.get(name)
    return Path(value) if value else Path(default)


def _tool(env_name, exe):
    value = os.environ.get(env_name)
    if value:
        return value
    return shutil.which(exe) or exe


def _main_checkout():
    """The main checkout's root (a worktree's .env is not where the key lives)."""
    try:
        common = subprocess.run(["git", "-C", str(REPO_ROOT), "rev-parse", "--path-format=absolute",
                                 "--git-common-dir"], capture_output=True, text=True, check=True).stdout.strip()
        return Path(common).parent
    except (OSError, subprocess.CalledProcessError):
        return REPO_ROOT


WORK = _env_path("UD_VIDEO_WORK", REPO_ROOT / "tmp" / "video" / "v2")
VANILLA_ROOT = _env_path("UD_VANILLA_ROOT", REPO_ROOT.parent / "vanilla-upstream")
VANILLA_LOBO = _env_path("UD_VANILLA_LOBO", VANILLA_ROOT / "tmp" / "lobo-mods")
FONT_DIR = _env_path("UD_FONT_DIR", ENGINE_ROOT / "thirdparty" / "guerrilla-fonts")
WHISPER_PY = _env_path("UD_WHISPER_PYTHON", sys.executable)
TTS_OUT = _env_path("UD_TTS_OUT", WORK / "tts")
FFMPEG = _tool("FFMPEG", "ffmpeg")
FFPROBE = _tool("FFPROBE", "ffprobe")


def env_file():
    value = os.environ.get("UD_ENV_FILE")
    if value:
        return Path(value)
    local = REPO_ROOT / ".env"
    return local if local.exists() else _main_checkout() / ".env"
