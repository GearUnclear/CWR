"""Paths and helpers shared by the Dedushka edit tools (ffmpeg + Python, re-runnable).

Everything the tools read or write that is footage, sound or a picture lives under
kaiju-notes\\video (never in the worktree). The tools themselves are source and live here.
"""
import json
import os
import subprocess

FF = r'C:\ffmpeg\bin\ffmpeg.exe'
FP = r'C:\ffmpeg\bin\ffprobe.exe'
ROOT = r'D:\Arma_CWA_worktrees\kaiju-notes\video'
SHOTS = os.path.join(ROOT, 'shots')
WORK = os.path.join(ROOT, 'work')
FINAL = os.path.join(ROOT, 'final')
WIP = os.path.join(ROOT, 'wip')
FONTS = r'D:\Arma_CWA\arma_CWR\thirdparty\guerrilla-fonts'
SND_R2 = r'D:\Arma_CWA_worktrees\kaiju-notes\features\sound-vst\round2\build\wav'
SND_R1 = r'D:\Arma_CWA_worktrees\kaiju-notes\features\sound-vst\build\wav'
FPS = 30
W, H = 1080, 1920
SR = 48000

for d in (WORK, FINAL, WIP):
    os.makedirs(d, exist_ok=True)


def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode != 0:
        raise SystemExit('command failed: %s\n%s' % (' '.join(str(c) for c in cmd)[:600], r.stderr[-3000:]))
    return r


def ff(*args):
    return run([FF, '-y', '-loglevel', 'error'] + list(args))


def nframes(path):
    r = run([FP, '-v', 'error', '-count_frames', '-select_streams', 'v:0', '-show_entries', 'stream=nb_read_frames', '-of', 'csv=p=0', path])
    return int(r.stdout.strip().split(',')[0])


def load_edl():
    return json.load(open(os.path.join(WORK, 'edl.json'), encoding='utf-8'))


def fr(t):
    return int(round(t * FPS))


# The look: a faded Soviet colour stock. Game footage only (cards are made to match by hand).
# Bloom on the hottest parts (the furnace, the beam, fires), then fade: less colour, lifted and
# slightly green blacks, warm rolled-off highlights, a little softness, vignette, grain.
def grade_chain(night=False, grain=14, sat=None):
    sat = sat if sat is not None else (0.8 if night else 0.68)
    lift = '0/0.035 0.1/0.08 0.3/0.265 0.6/0.6 0.85/0.865 1/0.94' if night else '0/0.05 0.1/0.095 0.3/0.275 0.6/0.6 0.85/0.865 1/0.94'
    return ("format=gbrp,split[ga][gb];[gb]gblur=sigma=20,curves=all='0/0 0.55/0 1/1'[gg];[ga][gg]blend=all_mode=screen:all_opacity=0.45,"
            "eq=saturation=%g:contrast=1.04,"
            "colorbalance=rs=0.0:gs=0.035:bs=-0.03:rm=0.02:gm=0.01:bm=-0.03:rh=0.04:gh=0.02:bh=-0.05,"
            "curves=master='%s',vignette=angle=0.6:aspect=0.5625,"
            "noise=c0s=%d:c0f=t+u:c1s=6:c1f=t+u:c2s=6:c2f=t+u,gblur=sigma=0.75" % (sat, lift, int(grain * 1.5)))
