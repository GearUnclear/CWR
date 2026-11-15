"""The cut of the Dedushka film record: writes work\\edl.json, the one contract the other tools read.

    python make_edl.py            # the film
    python make_edl.py --test     # a three-slot pipeline test from the probe clips

A slot is a card or a game shot. A game shot names its run folder under shots\\, the slot and camera
it was filmed as, the frame of the clip it starts on and its length in seconds. From the run's
per-frame log (shots\\<run>\\<slot>.json) and camera list (cams.json) this tool works out, per shot:
the sim time of its first frame, where its camera stood, and the tremor of every footfall the log
shows inside it (strength by the distance from the foot to the camera). Captions and the narration
lines are placed against slots (slot name + seconds into it), so re-timing a slot moves them with it.

Nothing here is heard or seen by the tool: levels and timings are the editor's, in SLOTS below.
"""
import argparse
import json
import math
import os

from edit_common import FPS, SHOTS, WORK

# ----------------------------------------------------------------------------------------------
# sound presets per kind of shot (see mix_audio.py for what each key does); dB
FAR_NIGHT = dict(wind=-34, sea=-22, motor=-34, furnace=-30, furnace_far=True, events=True, delay_cap=1.5, floor=-22)
DUSK_FAR = dict(wind=-27, motor=-33, furnace=-30, furnace_far=True, delay_cap=1.2, floor=-18)
DUSK_MID = dict(wind=-28, motor=-33, furnace=-24, delay_cap=1.0, floor=-12)
DUSK_NEAR = dict(wind=-29, motor=-33, furnace=-18, delay_cap=0.6, floor=-8)
CARD = dict(motor=-30)


def shot(name, run, slot, cam, clip_in, length, look='dusk', move=None, snd=None, **kw):
    d = dict(name=name, kind='shot', run=run, slot=slot, cam=cam, clip='%s-%s' % (slot, cam), clip_in=clip_in, len=length, look=look,
             move=move or {}, snd=dict(snd or {}))
    d.update(kw)
    return d


def card(name, which, length, move=None, snd=None, **kw):
    d = dict(name=name, kind='card', card=which, len=length, move=move or {'z0': 1.02}, snd=dict(snd or CARD))
    d.update(kw)
    return d


def cap(cid, slot, at, dur, lines, head='', **kw):
    d = dict(id=cid, slot=slot, at=at, dur=dur, head=head, lines=lines)
    d.update(kw)
    return d


def film():
    from cut import CAPTIONS, MUSIC, SLOTS, VO
    return SLOTS, CAPTIONS, VO, MUSIC


def test():
    slots = [
        card('leader', 'leader', 2.0),
        shot('t_walk', 'probe1', 'walk', 'north', 0, 5.0, snd=DUSK_MID),
        shot('t_street', 'probe1', 'walk', 'street', 60, 3.0, snd=DUSK_MID, move={'z0': 1.04, 'z1': 1.12, 'sway': 3}),
        card('brand', 'brand', 2.5, snd=dict(extra=[dict(t=0.0, snd='synth:subhit', gain=0, bus='end')])),
    ]
    caps = [cap('c01', 't_walk', 0.5, 3.5, ['It has not moved in two days.', 'The men call it Dedushka.'], head='13 JUNE 1985  19:10  MORTON')]
    return slots, caps, [], {}


def finish(slots, caps, vo, music):
    t = 0
    by = {}
    for s in slots:
        s['frames'] = int(round(s['len'] * FPS))
        s['t0'] = round(t / FPS, 4)
        t += s['frames']
        s['t1'] = round(t / FPS, 4)
        by[s['name']] = s
        if s['kind'] != 'shot':
            continue
        frames = json.load(open(os.path.join(SHOTS, s['run'], s['slot'] + '.json')))
        cams = json.load(open(os.path.join(SHOTS, s['run'], 'cams.json')))
        cam = cams['%s-%s' % (s['slot'], s['cam'])]
        a = s['clip_in']
        if a + s['frames'] > len(frames):
            print('WARNING %s: the log has %d frames, the slot wants %d from %d' % (s['name'], len(frames), s['frames'], a))
        s['sim0'] = frames[min(a, len(frames) - 1)]['t']
        s['eye'] = cam['eye']
        s['fov'] = cam['fov']
        # an extra tremor for each footfall in the shot, only where a slot asks for it (shake_gain): the
        # engine's own camera shake is already in the footage of every camera near him
        if 'shakes' not in s:
            sh = []
            prev = frames[a - 1]['steps'] if a > 0 else frames[a]['steps']
            for i, f in enumerate(frames[a:a + s['frames']]):
                if f['steps'] > prev:
                    d = math.hypot(f['pos'][0] - s['eye'][0], f['pos'][1] - s['eye'][2])
                    # a long lens magnifies the tremor of the tripod as much as it magnifies him
                    amp = min(30.0, s.get('shake_gain', 0.0) * 9.0 * (140.0 / max(d, 40.0)) ** 0.8 * min(3.0, 0.3 / max(0.03, s['fov'])) ** 0.5)
                    if d < 2600 and amp >= 1.0:
                        sh.append([round(i / FPS + min(0.25, d / 3000.0), 3), round(amp, 1), 0.28, 8.5])
                prev = f['steps']
            s['shakes'] = sh
    for c in caps:
        s = by[c['slot']]
        c['t0'] = round(s['t0'] + c['at'], 3)
        c['t1'] = round(c['t0'] + c['dur'], 3)
    for v in vo:
        s = by[v['slot']]
        v['t0'] = round(s['t0'] + v['at'], 3)
    if music:
        music = dict(music)
        music['t0'] = round(by[music['from']]['t0'] + music.get('at', 0), 3)
        music['t1'] = round(by[music['to']]['t0'] + music.get('to_at', 0), 3)
        if 'gain_at' in music:
            music['gains'] = [[round(by[sl]['t0'] + at, 3), g] for sl, at, g in music['gain_at']]
    edl = dict(fps=FPS, frames=t, seconds=round(t / FPS, 3), slots=slots, captions=caps, vo=vo, music=music, master_gain=1.0)
    return edl


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--test', action='store_true')
    args = ap.parse_args()
    slots, caps, vo, music = test() if args.test else film()
    edl = finish(slots, caps, vo, music)
    if not args.test:
        import cut
        edl['master_gain'] = getattr(cut, 'MASTER_GAIN', 1.0)
    json.dump(edl, open(os.path.join(WORK, 'edl.json'), 'w', encoding='utf-8'), indent=1, ensure_ascii=False)
    print('edl: %d slots, %d frames, %.2f s, %d captions, %d voice lines' % (len(slots), edl['frames'], edl['seconds'], len(caps), len(vo)))
    for s in edl['slots']:
        print('  %6.2f  %-12s %5.2f s  %s' % (s['t0'], s['name'], s['len'], (s.get('run', '') + '/' + s.get('clip', s.get('card', ''))) + ('  shakes %d' % len(s.get('shakes', [])) if s['kind'] == 'shot' else '')))


if __name__ == '__main__':
    main()
