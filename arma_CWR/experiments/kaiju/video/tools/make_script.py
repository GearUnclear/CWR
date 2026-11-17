"""Writes kaiju-notes\\video\\SCRIPT.md from work\\edl.json: the narration for a voice to be added later,
the captions as they stand on screen in this cut, and the shot list, all with their times.

    python make_script.py

The narration lines live in cut.py (VO); each names the slot it belongs to and how far into it the
speech starts. To add the voice: record each line to kaiju-notes\\video\\vo\\<id>.wav and run
build_all.py. No re-shoot and no re-cut is needed: mix_audio.py places each file at its time and
ducks the bed and the music 7 dB under it (the brand line is not ducked: it comes after the last hit).
"""
import os

from edit_common import ROOT, load_edl


def tc(t):
    return '%d:%05.2f' % (int(t // 60), t % 60)


def main():
    edl = load_edl()
    out = []
    a = out.append
    a('# Dedushka: film record No. 0417 (script, first cut)')
    a('')
    a('Written by `tools\\make_script.py` from the cut (`tools\\cut.py`); do not edit by hand. Length of the cut: %.1f s.' % edl['seconds'])
    a('')
    a('## What this is')
    a('')
    a('An in-universe Soviet military film record: reel 2 of 3 of "Film record No. 0417", shot by the film unit of the Everon garrison between 11 and 14 June 1985 and annotated afterwards by the archive. The camera stands where a man could stand. Nobody on screen explains anything.')
    a('')
    a('This cut carries the story as typed captions. The narration below is the same story written for a voice: one calm man, an archive commentator reading a report years later, never raising his voice. It is laid out so the voice can be dropped in without re-shooting or re-cutting (see the end).')
    a('')
    a('## Narration, for a voice to be added later')
    a('')
    a('| Line | Starts | Slot | Room (s) | Text | Direction |')
    a('|---|---|---|---|---|---|')
    for v in edl.get('vo', []):
        a('| %s | %s | %s | %.1f | %s | %s |' % (v['id'], tc(v['t0']), v['slot'], v.get('room', 0), v['text'], v.get('note', '')))
    a('')
    a('The brand is said OOS-loo DOOR, two words, plain u, not shouted. It is spoken after the last hit, in its ring-out, and nothing is ducked under it.')
    a('')
    a('## Captions on screen in this cut')
    a('')
    a('| Id | On | Off | Slot | Header | Text |')
    a('|---|---|---|---|---|---|')
    for c in edl.get('captions', []):
        a('| %s | %s | %s | %s | %s | %s |' % (c['id'], tc(c['t0']), tc(c['t1']), c['slot'], c.get('head', ''), ' / '.join(c['lines'])))
    a('')
    a('## Shot list as cut')
    a('')
    a('| # | In | Out | Length | Slot | Source | What it is |')
    a('|---|---|---|---|---|---|---|')
    for i, s in enumerate(edl['slots']):
        src = ('%s\\%s.mp4 from frame %d' % (s['run'], s['clip'], s['clip_in'])) if s['kind'] == 'shot' else ('card: ' + s['card'])
        a('| %d | %s | %s | %.1f | %s | %s | %s |' % (i + 1, tc(s['t0']), tc(s['t1']), s['len'], s['name'], src, s.get('what', '')))
    a('')
    m = edl.get('music') or {}
    a('## Music slot')
    a('')
    if m:
        a('`music\\%s`, taken from %.2f s of the track, laid from %s to %s of the film (level curve in `cut.py`, MUSIC). It is cut dead on a downbeat under the last hit. To change the music: put another file in `kaiju-notes\\video\\music\\`, name it in MUSIC, set where in the track to start, run `build_all.py`.' % (m.get('file'), m.get('in', 0), tc(m['t0']), tc(m['t1'])))
    else:
        a('Empty in this cut. Put a file in `kaiju-notes\\video\\music\\` and name it in MUSIC in `cut.py`.')
    a('')
    a('## Adding the voice')
    a('')
    a('1. Record each line of the table to `kaiju-notes\\video\\vo\\<line id>.wav` (any sample rate, mono or stereo). Each must fit its room; the room is the time until the next line or the next loud event.')
    a('2. Run `python build_all.py` in `tools\\`. The mix places each file at its start time and ducks the effects, beds and music 7 dB under it. Lines marked no duck are not ducked.')
    a('3. If the captions should go once there is a voice, empty CAPTIONS in `cut.py` (or keep only the date headers) and build again.')
    open(os.path.join(ROOT, 'SCRIPT.md'), 'w', encoding='utf-8').write('\n'.join(out) + '\n')
    print('SCRIPT.md: %d narration lines, %d captions, %d slots' % (len(edl.get('vo', [])), len(edl.get('captions', [])), len(edl['slots'])))


if __name__ == '__main__':
    main()
