"""The sound of the Dedushka film record, laid in the edit (the game is recorded silent).

    python mix_audio.py

Reads work\\edl.json. For every game shot it takes the sounds the script set itself sent while that
shot was filmed (the KJROAR snd lines of the game log, kept as shots\\<run>\\events.json: class, sim
time, place), turns sim time into film time, and lets each arrive late by its distance to that
shot's camera (capped, so a far shot still has its sound). So a footfall is heard when the foot of
the picture lands, a beat late when he is far. On top: beds (wind, sea, the camera's motor, the
furnace), the sounds the game does not send (houses coming down, tank guns, shell bursts, the
death), and the hit under the brand card, which is the loudest thing in the film and is not faded.

Slots for later, both optional files: a voice (video\\vo\\<line id>.wav, placed at the line's time,
the bed ducked under it) and music (video\\music\\music.wav, from the EDL's music slot).

Writes work\\mix.wav (48 kHz stereo), work\\mix_report.json, final\\mix_preview.mp3.
"""
import json
import math
import os
import re

import numpy as np
import soundfile as sf
from scipy import signal

import synth
from edit_common import FF, FINAL, FPS, ROOT, SHOTS, SR, WORK, ff, load_edl, run

CLS = {  # game sound class -> (near file, far file or None, base gain dB)
    'KJ_Step': ('step', 'step_far', -5.0), 'KJ_StepFar': ('step', 'step_far', -5.0),
    'KJ_Roar': ('roar_bear1', 'roar_bear1_far', -2.0), 'KJ_RoarFar': ('roar_bear1', 'roar_bear1_far', -2.0),
    'KJ_RoarBear1': ('roar_bear1', 'roar_bear1_far', -2.0), 'KJ_RoarBear1Far': ('roar_bear1', 'roar_bear1_far', -2.0),
    'KJ_RoarBear3': ('roar_bear3', 'roar_bear3_far', -4.0), 'KJ_RoarBear3Far': ('roar_bear3', 'roar_bear3_far', -4.0),
    'KJ_RoarDog1': ('roar_dog1', 'roar_dog1_far', -3.0), 'KJ_RoarDog1Far': ('roar_dog1', 'roar_dog1_far', -3.0),
    'KJ_RoarDog2': ('roar_dog2', 'roar_dog2_far', -4.0), 'KJ_RoarDog2Far': ('roar_dog2', 'roar_dog2_far', -4.0),
    'KJ_BeamCharge': ('beam_charge', None, -5.0), 'KJ_Beam': ('beam', None, -2.0), 'KJ_BeamLoop': ('beam_loop', None, -7.0),
    'KJ_Exhale': ('exhale', None, -8.0), 'KJ_Birds': ('birds', None, -15.0),
}
FAR_AT = {'step': 320.0, 'roar': 650.0}


def db(x):
    return 10 ** (x / 20.0)


class Mix:
    def __init__(self, dur):
        self.n = int(math.ceil(dur * SR)) + SR
        self.bus = {k: np.zeros((self.n, 2)) for k in ('fx', 'bed', 'end', 'vo', 'music')}
        self.log = []

    def add(self, bus, x, t, gain_db=0.0, pan=0.0, fade_in=0.004, fade_out=0.03, cut_at=None, start_at=None, note=''):
        """x mono at film time t (s). pan -1..1. cut_at: film time at which it is faded out.
        start_at: film time before which it is silent (the sound began before the cut)."""
        x = np.asarray(x, dtype=float)
        k0 = int(round(t * SR))
        if start_at is not None and start_at > t:
            skip = int(round((start_at - t) * SR))
            if skip >= len(x):
                return
            x = x[skip:]
            k0 += skip
            fade_in = max(fade_in, 0.03)
        if cut_at is not None:
            keep = int(round((cut_at - k0 / SR) * SR))
            if keep <= 0:
                return
            x = x[:keep + int(fade_out * SR)] if keep < len(x) else x
            if keep < len(x):
                fo = len(x) - keep
                x = x.copy()
                x[keep:] *= np.linspace(1, 0, fo)
        x = x.copy()
        fi = min(len(x), int(fade_in * SR))
        if fi > 1:
            x[:fi] *= np.linspace(0, 1, fi)
        if k0 < 0:
            x = x[-k0:]
            k0 = 0
        if k0 >= self.n or len(x) == 0:
            return
        x = x[:self.n - k0]
        g = db(gain_db)
        l = g * math.cos((pan + 1) * math.pi / 4)
        r = g * math.sin((pan + 1) * math.pi / 4)
        self.bus[bus][k0:k0 + len(x), 0] += x * l * 1.4142
        self.bus[bus][k0:k0 + len(x), 1] += x * r * 1.4142
        self.log.append(dict(bus=bus, t=round(k0 / SR, 3), len=round(len(x) / SR, 2), gain=round(gain_db, 1), note=note))


def loop(x, dur, offset=0.0, xf=0.4):
    """x repeated to dur seconds with a crossfade at each join."""
    n = int(dur * SR)
    nx = len(x)
    k = int(xf * SR)
    out = np.zeros(n + nx)
    pos = 0
    first = True
    start = int(offset * SR) % nx
    seg = np.concatenate([x[start:], x[:start]])
    while pos < n:
        s = seg.copy()
        s[:k] *= np.linspace(0, 1, k) if not first else 1
        s[-k:] *= np.linspace(1, 0, k)
        out[pos:pos + nx] += s[:len(out) - pos]
        pos += nx - k
        first = False
    return out[:n]


def envelope(points, n):
    """A gain curve (linear in dB between points [(t, dB), ...]) as an array of n samples."""
    ts = np.array([p[0] for p in points]) * SR
    vs = np.array([p[1] for p in points])
    return db(np.interp(np.arange(n), ts, vs))


def dist_gain(d, near=120.0, floor=-20.0, per_double=5.0):
    if d <= near:
        return 0.0
    return max(floor, -per_double * math.log2(d / near))


def main():
    edl = load_edl()
    total = edl['frames'] / FPS
    mx = Mix(total)
    report = dict(slots=[])
    bed_points = {k: [] for k in ('wind', 'sea', 'motor')}
    for s in edl['slots']:
        t0, t1 = s['t0'], s['t1']
        snd = s.get('snd', {})
        for k in bed_points:
            g = snd.get(k, -120)
            bed_points[k] += [(t0 + 0.02, g), (t1 - 0.02, g)]
        # ---- extra one-shots given by hand in the EDL: [t in the slot, sound, gain dB, far?]
        for ex in snd.get('extra', []):
            name = ex['snd']
            x = getattr(synth, name[6:])(**ex.get('args', {})) if name.startswith('synth:') else synth.load(name)
            if ex.get('far'):
                x = synth.far_version(x, ex.get('cutoff', 420))
            if ex.get('rate'):
                x = signal.resample(x, int(len(x) / ex['rate']))
            cut = t1 + ex['ring'] if 'ring' in ex else None
            mx.add(ex.get('bus', 'fx'), x, t0 + ex['t'], ex.get('gain', -6), ex.get('pan', 0), cut_at=cut, fade_out=ex.get('fade_out', 0.03),
                   fade_in=ex.get('fade_in', 0.004), note='%s extra %s' % (s['name'], name))
        if s['kind'] != 'shot':
            continue
        sim0, eye = s['sim0'], s['eye']
        sim1 = sim0 + s['frames'] / FPS
        cap = snd.get('delay_cap', 1.2)
        pre = snd.get('pre', 0.0)
        row = dict(name=s['name'], events=[])
        # ---- the furnace under the shot
        if snd.get('furnace') is not None:
            f = loop(synth.load('furnace'), t1 - t0 + 0.3, offset=sim0)
            if snd.get('furnace_far'):
                f = synth.far_version(f, 380)
            g0 = snd['furnace']
            g1 = snd.get('furnace_end', g0)
            f = f * envelope([(0, g0), ((t1 - t0), g1)], len(f))
            mx.add('bed', f, t0, 0, fade_in=0.05, cut_at=t1, fade_out=0.25, note='%s furnace' % s['name'])
        if not snd.get('events', True):
            report['slots'].append(row)
            continue
        events = json.load(open(os.path.join(SHOTS, s['run'], 'events.json')))
        frames = json.load(open(os.path.join(SHOTS, s['run'], s['slot'] + '.json')))
        for ev in events:
            if not (sim0 - pre <= ev['t'] < sim1):
                continue
            c = CLS.get(ev['cls'])
            if not c:
                continue
            d = math.hypot(ev['pos'][0] - eye[0], ev['pos'][1] - eye[2])
            if ev['cls'] == 'KJ_Birds':
                d = 45.0                      # they rise near the listener, which is the shot's first camera
            delay = min(d / 343.0, cap)
            kind = 'step' if 'Step' in ev['cls'] else ('roar' if 'Roar' in ev['cls'] else 'other')
            far = c[1] is not None and d > FAR_AT.get(kind, 1e9)
            if snd.get('force_far'):
                far = c[1] is not None
            if snd.get('force_near'):
                far = False
            x = synth.load(c[1] if far else c[0])
            g = c[2] + dist_gain(d, floor=snd.get('floor', -16)) + snd.get('ev_gain', 0) + (3.0 if far else 0)
            if kind == 'step':
                g += snd.get('step_gain', 0)
            T = t0 + (ev['t'] - sim0) + delay
            cut = None
            if ev['cls'] in ('KJ_BeamLoop', 'KJ_BeamCharge') or snd.get('cut_tails'):
                cut = t1 + snd.get('ring', 0.12)
            if kind == 'roar' and 'roar_ring' in snd:
                cut = t1 + snd['roar_ring']
            mx.add('fx', x, T, g, pan=snd.get('pan', 0.0), start_at=t0 if (T < t0 and not snd.get('jcut')) else None, cut_at=cut, fade_out=0.2 if cut else 0.03,
                   note='%s %s d=%.0f delay=%.2f' % (s['name'], ev['cls'], d, delay))
            row['events'].append(dict(cls=ev['cls'], film_t=round(T, 2), d=round(d), far=bool(far), gain=round(g, 1)))
        # ---- what the game does not send: houses coming down, the guns, the bursts on him
        a = int(s.get('clip_in', 0))
        fr = frames[a:a + s['frames']]
        prev = frames[a - 1] if a > 0 else (fr[0] if fr else None)
        seed = 100
        for i, f in enumerate(fr):
            if prev is None:
                prev = f
                continue
            tf = t0 + i / FPS
            kpos = f.get('pos', [0, 0, 0])
            d = math.hypot(kpos[0] - eye[0], kpos[1] - eye[2])
            delay = min(d / 343.0, cap)
            if snd.get('collapse', True) and f.get('felled', 0) > prev.get('felled', 0):
                seed += 1
                x = synth.collapse(seed=seed)
                if d > 450:
                    x = synth.far_version(x, 500)
                mx.add('fx', x, tf + delay, -7.0 + dist_gain(d, floor=-14), note='%s house down d=%.0f' % (s['name'], d))
                row['events'].append(dict(cls='collapse', film_t=round(tf + delay, 2), d=round(d)))
            # the death, phase by phase (the script set sends no sound of its own for it)
            dp = f.get('death', '')
            if snd.get('death') and dp != prev.get('death', ''):
                g = dist_gain(d, floor=-8)
                if dp == 'stagger':
                    mx.add('fx', synth.load('roar_bear4'), tf + delay, -2.0 + g, note='%s death: stagger, the furnace roar' % s['name'])
                elif dp == 'flare':
                    mx.add('fx', synth.load('beam_charge'), tf + delay, -4.0 + g, note='%s death: flare' % s['name'])
                elif dp == 'vent':
                    mx.add('fx', synth.load('exhale'), tf + delay, -4.0 + g, note='%s death: vent' % s['name'])
                elif dp == 'down':
                    mx.add('fx', synth.subhit(seed=41), tf + delay, -2.0 + g, note='%s death: he comes down' % s['name'])
                    mx.add('fx', synth.load('step_3'), tf + delay, -2.0 + g, note='%s death: he comes down' % s['name'])
                    mx.add('fx', synth.collapse(seed=42, dur=4.5), tf + delay + 0.1, -6.0 + g, note='%s death: he comes down' % s['name'])
                row['events'].append(dict(cls='death ' + dp, film_t=round(tf + delay, 2), d=round(d)))
            dn = f.get('shots', 0) - prev.get('shots', 0)
            if snd.get('guns') is not None and dn > 0:
                for j in range(min(dn, 3)):
                    seed += 1
                    x = synth.gun(far=snd.get('gun_far', False), seed=seed)
                    mx.add('fx', x, tf + snd.get('gun_delay', 0.0) + 0.07 * j, snd['guns'], pan=(-0.4 if seed % 2 else 0.4), note='%s gun' % s['name'])
                    # the burst on him: light first, the crack later by his distance
                    b = synth.burst(seed=seed)
                    if d > 350:
                        b = synth.far_version(b, 700)
                    mx.add('fx', b, tf + 0.25 + delay, snd['guns'] - 5.0 + dist_gain(d, floor=-10), note='%s burst' % s['name'])
                row['events'].append(dict(cls='gun x%d' % dn, film_t=round(tf, 2)))
            prev = f
        report['slots'].append(row)
    # ---- beds over the whole film, their levels stepping at the cuts
    n = mx.n
    w = synth.wind(n / SR + 1, 11)[:n]
    w2 = synth.wind(n / SR + 1, 12)[:n]
    e = envelope(sorted(bed_points['wind']), n)
    mx.bus['bed'][:, 0] += w * e
    mx.bus['bed'][:, 1] += (0.6 * w + 0.4 * w2) * e
    se = synth.sea(n / SR + 1, 21)[:n]
    se2 = synth.sea(n / SR + 1, 22)[:n]
    e = envelope(sorted(bed_points['sea']), n)
    mx.bus['bed'][:, 0] += se * e
    mx.bus['bed'][:, 1] += (0.5 * se + 0.5 * se2) * e
    m = synth.motor(n / SR + 1, 31)[:n]
    e = envelope(sorted(bed_points['motor']), n)
    mx.bus['bed'][:, 0] += m * e
    mx.bus['bed'][:, 1] += m * e
    # ---- the voice, when there is one, and the duck under it
    duck = np.ones(n)
    for line in edl.get('vo', []):
        p = os.path.join(ROOT, 'vo', line['id'] + '.wav')
        if os.path.exists(p):
            x, sr = sf.read(p, dtype='float64')
            if x.ndim > 1:
                x = x.mean(axis=1)
            if sr != SR:
                g = np.gcd(SR, sr)
                x = signal.resample_poly(x, SR // g, sr // g)
            mx.add('vo', x, line['t0'], line.get('gain', -3), note='voice ' + line['id'])
            a, b = int((line['t0'] - 0.12) * SR), int((line['t0'] + len(x) / SR + 0.4) * SR)
            if not line.get('no_duck'):
                duck[max(0, a):b] = db(-7.0)
    # ---- music, when there is some
    # (stereo kept; 'in' is where in the track the film's music starts; 'gains' is the level curve in
    # film time, [[t, dB], ...]; the music is cut dead at t1, on a downbeat, under the last hit)
    mus = edl.get('music', {})
    p = os.path.join(ROOT, 'music', mus.get('file', 'music.wav')) if mus else ''
    if mus and os.path.exists(p):
        x, sr = sf.read(p, dtype='float64')
        if x.ndim == 1:
            x = np.stack([x, x], axis=1)
        if sr != SR:
            g = np.gcd(SR, sr)
            x = signal.resample_poly(x, SR // g, sr // g, axis=0)
        a = int(mus.get('in', 0) * SR)
        k0 = int(round(mus['t0'] * SR))
        k1 = min(n, int(round(mus['t1'] * SR)))
        seg = x[a:a + (k1 - k0)].copy()
        gains = mus.get('gains') or [[mus['t0'], mus.get('gain', -12)], [mus['t1'], mus.get('gain', -12)]]
        seg *= envelope([(t - mus['t0'], g) for t, g in gains], len(seg))[:, None]
        fi = int(mus.get('fade_in', 0.03) * SR)
        fo = int(mus.get('fade_out', 0.012) * SR)
        seg[:fi] *= np.linspace(0, 1, fi)[:, None]
        seg[-fo:] *= np.linspace(1, 0, fo)[:, None]
        mx.bus['music'][k0:k0 + len(seg)] += seg
        mx.log.append(dict(bus='music', t=mus['t0'], len=round(len(seg) / SR, 2), gain='curve', note='music %s from %.2f s of the track' % (mus.get('file'), mus.get('in', 0))))
    if not np.all(duck == 1):
        k = int(0.08 * SR)
        duck = np.convolve(duck, np.ones(k) / k, mode='same')
        for b in ('fx', 'bed', 'music'):
            mx.bus[b] *= duck[:, None]
    # ---- sum, trim to the picture, master
    y = sum(mx.bus.values())[:int(round(total * SR))]
    y = signal.sosfilt(signal.butter(2, 24, btype='high', fs=SR, output='sos'), y, axis=0)
    pre_peak = float(np.abs(y).max())
    target_peak = 0.89          # about -1 dBFS, room for what the AAC encode adds
    y = y * (edl.get('master_gain', 1.0))
    # a look-ahead limiter (hold 60 ms, smoothed) so nothing clips and nothing is squashed that
    # does not have to be; the end hit is meant to sit right at the ceiling
    from scipy.ndimage import gaussian_filter1d, maximum_filter1d
    env = np.abs(y).max(axis=1)
    red = np.maximum(0.0, 1.0 - target_peak / np.maximum(env, 1e-9))
    red = maximum_filter1d(red, size=int(0.06 * SR))
    red = gaussian_filter1d(red, sigma=0.008 * SR)
    red = maximum_filter1d(red, size=int(0.004 * SR))
    y = y * (1.0 - red)[:, None]
    y = np.clip(y, -0.999, 0.999)
    limited = float((red > 0.01).mean())
    out = os.path.join(WORK, 'mix.wav')
    sf.write(out, y, SR, subtype='PCM_24')
    # ---- measurements
    r = run([FF, '-hide_banner', '-nostats', '-i', out, '-af', 'ebur128=peak=true', '-f', 'null', '-'])
    txt = r.stderr
    I = re.findall(r'I:\s+(-?[\d.]+) LUFS', txt)
    tp = re.findall(r'Peak:\s+(-?[\d.]+) dBFS', txt)
    lra = re.findall(r'LRA:\s+(-?[\d.]+) LU', txt)
    win = int(0.4 * SR)
    rms = []
    for s in edl['slots']:
        a, b = int(s['t0'] * SR), int(s['t1'] * SR)
        seg = y[a:b]
        rms.append(dict(name=s['name'], rms_db=round(20 * math.log10(max(1e-9, float(np.sqrt((seg ** 2).mean())))), 1),
                        peak_db=round(20 * math.log10(max(1e-9, float(np.abs(seg).max()))), 1)))
    report.update(lufs=I[-1] if I else None, true_peak=tp[-1] if tp else None, lra=lra[-1] if lra else None, pre_master_peak=round(pre_peak, 3), limited_share=round(limited, 4),
                  per_slot=rms, placements=mx.log)
    json.dump(report, open(os.path.join(WORK, 'mix_report.json'), 'w'), indent=1)
    ff('-i', out, '-c:a', 'libmp3lame', '-b:a', '192k', os.path.join(FINAL, 'mix_preview.mp3'))
    print('mix: %.2f s, integrated %s LUFS, true peak %s dBFS, LRA %s LU, peak before the master %.2f' % (total, report['lufs'], report['true_peak'], report['lra'], pre_peak))
    for q in rms:
        print('  %-14s rms %6.1f dB  peak %6.1f dB' % (q['name'], q['rms_db'], q['peak_db']))
    _ = win


if __name__ == '__main__':
    main()
