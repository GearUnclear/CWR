"""Where the Lebanon80 promo tools find things.

Every location comes from udpaths.py (arma_CWR/tests/rendering/captures/video), which reads an
environment variable and falls back to a repo-relative default:

    UD_PROMO_DIR     the production folder; default <repo>/tmp/video/lebanon80-promo (tmp/ is gitignored)
                     clips/   the encoded clips, thirds sheets, shotlist.md
                     work/    runs/ (tri output), logs/, gates/, judder/, post/, sheets/, edit/ (EDL,
                              segments, cards, sheets), the final report
    UD_PROMO_AUDIO   the edit's music (a WAV, muxed untouched); default
                     <UD_PROMO_DIR>/ud-lebanon80-promo-instrumental.wav
    UD_FONT_DIR      the game's OFL fonts; default arma_CWR/thirdparty/guerrilla-fonts
    FFMPEG, FFPROBE  the ffmpeg tools; default: found on PATH

The tools in edit/ and audio/ import this file by adding this folder to sys.path.
"""
import os
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ENGINE_ROOT = HERE.parents[1]  # arma_CWR
VIDEO_DIR = ENGINE_ROOT / "tests" / "rendering" / "captures" / "video"
if str(VIDEO_DIR) not in sys.path:
    sys.path.insert(0, str(VIDEO_DIR))

import udpaths  # noqa: E402

PROMO = udpaths.PROMO_DIR
ROOT = str(PROMO)                       # the os.path tools use strings
CLIPS = os.path.join(ROOT, "clips")
WORK = os.path.join(ROOT, "work")
AUDIO = Path(os.environ.get("UD_PROMO_AUDIO") or PROMO / "ud-lebanon80-promo-instrumental.wav")
FONT_DIR = udpaths.FONT_DIR
FF = udpaths.FFMPEG
FP = udpaths.FFPROBE
