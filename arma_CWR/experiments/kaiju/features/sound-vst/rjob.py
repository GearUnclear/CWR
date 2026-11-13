r"""Builds one headless REAPER job (a Lua ReaScript that uses lib.lua) from
Python and runs it. One job = one REAPER start = one or more rendered wavs.

    j = Job('voices_test')
    t = j.track('voice')
    fx = j.fx(t, 'VST3i: Augmented VOICES (Arturia)')
    j.dump(t, fx)                       # parameter list to dumps\<fx name>.txt
    j.set(t, fx, 'Morph', 0.7)          # normalized 0..1, by name or index
    j.env(t, fx, 'Morph', [(0, 0.1), (6, 0.9)])
    j.midi(t, 10, notes=[(0.5, 7.0, 36, 110)], events=[(0.5, 0xB0, 1, 0)])
    wav = j.render(10)                  # renders\voices_test.wav (stereo, 24 bit)
    code = j.run(timeout=240)           # 0 = REAPER ran the script and quit; 124 = timed out and was stopped

Files: the job script and saved project go to <notes>\jobs, the script's own
log to <notes>\logs\<name>.log, renders to <notes>\renders.
"""
import os

import reaper_run

HERE = os.path.dirname(os.path.abspath(__file__))
NOTES = reaper_run.NOTES


def L(s):
    """Lua long string for a path or name."""
    return '[[' + s + ']]'


def lua_val(v):
    if isinstance(v, str):
        return L(v)
    if isinstance(v, (list, tuple)):
        return '{' + ', '.join(lua_val(x) for x in v) + '}'
    if isinstance(v, bool):
        return 'true' if v else 'false'
    if isinstance(v, float):
        return repr(round(v, 6))
    return repr(v)


class Job:
    def __init__(self, name):
        self.name = name
        self.lines = []
        self.nt = 0
        self.nfx = {}
        for d in ('jobs', 'logs', 'renders', 'dumps'):
            os.makedirs(os.path.join(NOTES, d), exist_ok=True)
        self.log = os.path.join(NOTES, 'logs', name + '.log')
        self.lines.append(f'KJ_LOG = {L(self.log)}')
        self.lines.append(f'KJ_EMPTY = {L(os.path.join(NOTES, "jobs", "empty.rpp"))}')
        self.lines.append(f'dofile({L(os.path.join(HERE, "lib.lua"))})')

    def raw(self, lua):
        self.lines.append(lua)

    def track(self, name=None, vol=None):
        t = f't{self.nt}'
        self.nt += 1
        self.lines.append(f'local {t} = kj_track({L(name or t)})')
        self.nfx[t] = 0
        if vol is not None:
            self.lines.append(f'kj_vol({t}, {vol})')
        return t

    def fx(self, t, name, params=None, preset=None, chunk=None):
        f = f'{t}_fx{self.nfx[t]}'
        self.nfx[t] += 1
        self.lines.append(f'local {f} = kj_fx({t}, {L(name)})')
        if chunk:
            self.lines.append(f'kj_load_chunk({t}, {f}, {L(chunk)})')
        if preset is not None:
            self.lines.append(f'kj_preset({t}, {f}, {lua_val(preset)})')
        for k, v in (params or {}).items():
            self.set(t, f, k, v)
        return f

    def set(self, t, f, p, v):
        self.lines.append(f'kj_set({t}, {f}, {lua_val(p)}, {v})')

    def env(self, t, f, p, pts, shape=0):
        self.lines.append(f'kj_env({t}, {f}, {lua_val(p)}, {lua_val([list(x) for x in pts])}, {shape})')

    def dump(self, t, f, tag=None):
        path = os.path.join(NOTES, 'dumps', (tag or self.name) + '.txt')
        self.lines.append(f'kj_dump({t}, {f}, {L(path)})')
        return path

    def dump_presets(self, t, f, tag=None, maxn=4000):
        path = os.path.join(NOTES, 'dumps', (tag or self.name) + '.presets.txt')
        self.lines.append(f'kj_dump_presets({t}, {f}, {L(path)}, {maxn})')
        return path

    def save_chunk(self, t, f, path):
        self.lines.append(f'kj_save_chunk({t}, {f}, {L(path)})')

    def midi(self, t, length, notes=(), events=()):
        self.lines.append(f'kj_midi({t}, {length}, {lua_val([list(n) for n in notes])}, {lua_val([list(e) for e in events])})')

    def audio(self, t, path, at=0.0):
        self.lines.append(f'kj_audio({t}, {L(path)}, {at})')

    def render(self, t1, t0=0.0, name=None, sr=44100, ch=2):
        path = os.path.join(NOTES, 'renders', (name or self.name) + '.wav')
        self.lines.append(f'kj_render({L(path)}, {t0}, {t1}, {sr}, {ch})')
        return path

    def mute(self, t, on=True):
        self.lines.append(f'reaper.SetMediaTrackInfo_Value({t}, "B_MUTE", {1 if on else 0})')

    def run(self, timeout=240):
        rpp = os.path.join(NOTES, 'jobs', self.name + '.rpp')
        lua = os.path.join(NOTES, 'jobs', self.name + '.lua')
        with open(lua, 'w', encoding='utf-8') as fh:
            fh.write('\n'.join(self.lines) + f'\nkj_quit({L(rpp)})\n')
        if os.path.exists(self.log):
            os.remove(self.log)
        code = reaper_run.run(lua, timeout=timeout, name=self.name)
        if os.path.exists(self.log):
            with open(self.log, encoding='utf-8', errors='replace') as fh:
                txt = fh.read()
            for line in txt.splitlines():
                print('   ', line)
            if not txt.rstrip().endswith(')') or 'quit' not in txt:
                print('    [job] the script did not reach kj_quit')
        else:
            print('    [job] no script log: REAPER never ran the script')
        return code
