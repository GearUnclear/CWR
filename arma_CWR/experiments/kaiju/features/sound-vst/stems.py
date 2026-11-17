r"""Renders the raw material ("strips") out of the owner's instruments, headless.

    python stems.py                 # every group that is not rendered yet
    python stems.py --group brass   # one group
    python stems.py --force         # render again

A strip is one factory preset (or one programmed Chromaphone patch) played
note after note, each note in its own slot of the file, dry (the plugin's own
reverb and delay off where it has those macros). make_sounds.py cuts the notes
out of the strips, so every sound in the game can be traced back to
"instrument / preset / note". Output: <notes>\renders\strips\<name>.wav
(stereo, 24 bit, 44.1 kHz) and strips.json (what is in each file).
"""
import argparse
import json
import os
import re

from arturia import chunk
from audition import VST
from rjob import Job, NOTES

STRIPS = os.path.join(NOTES, 'renders', 'strips')
CHROMA = 'VST3i: Chromaphone 3 (Applied Acoustics Systems)'


def slug(s):
    return re.sub(r'[^A-Za-z0-9]+', '_', s).strip('_').lower()


# name, instrument, preset, notes, hold, slot, morph (from, to), wheel (from, to), velocity
A = 'Augmented '
TABLE = {
    'cmi': [
        ('cmi_wolf', 'CMI V', 'WOLF', (48, 43, 41, 36), 7.0, 8.0, None, None, 120),
        ('cmi_elephant', 'CMI V', 'ELEPHANT', (55, 48, 43, 36), 7.0, 8.0, None, None, 120),
        ('cmi_cow', 'CMI V', 'COW', (48, 41, 36, 31), 7.0, 8.0, None, None, 120),
        ('cmi_scream1', 'CMI V', 'SCREAM1', (55, 48, 43, 36), 7.0, 8.0, None, None, 120),
        ('cmi_scream2', 'CMI V', 'SCREAM2', (55, 48, 41, 36), 7.0, 8.0, None, None, 120),
        ('cmi_screech', 'CMI V', 'SCREECH', (60, 48, 43, 36), 7.0, 8.0, None, None, 120),
        ('cmi_seal', 'CMI V', 'SEAL', (48, 41, 36, 29), 7.0, 8.0, None, None, 120),
        ('cmi_dog', 'CMI V', 'DOG', (48, 41, 36, 29, 24), 5.0, 6.0, None, None, 120),
        ('cmi_multidog', 'CMI V', 'MULTIDOG', (48, 36, 29), 5.0, 6.0, None, None, 120),
        ('cmi_animal', 'CMI V', 'Animal', (48, 41, 36), 7.0, 8.0, None, None, 120),
        ('cmi_jaildoor', 'CMI V', 'JAILDOOR', (60, 48, 41, 36), 5.0, 6.0, None, None, 127),
        ('cmi_door', 'CMI V', 'DOOR', (60, 48, 36), 5.0, 6.0, None, None, 127),
        ('cmi_gong1', 'CMI V', 'GONG1', (60, 48, 36), 9.0, 10.0, None, None, 127),
        ('cmi_steam', 'CMI V', 'STEAM', (60, 48, 36, 29), 7.0, 8.0, None, None, 120),
        ('cmi_ambcrash', 'CMI V', 'AMBCRASH', (48, 36, 29), 7.0, 8.0, None, None, 127),
        ('cmi_bird1', 'CMI V', 'BIRD1', (84, 79, 72, 67, 60), 3.0, 4.0, None, None, 110),
        ('cmi_bird2', 'CMI V', 'BIRD2', (84, 79, 72, 67, 60), 3.0, 4.0, None, None, 110),
        ('cmi_bird3', 'CMI V', 'BIRD3', (84, 79, 72, 67, 60), 3.0, 4.0, None, None, 110),
        ('cmi_bird4', 'CMI V', 'BIRD4', (84, 79, 72, 67, 60), 3.0, 4.0, None, None, 110),
        ('cmi_radioactivity', 'CMI V', 'Lost Radioactivity', (36, 48), 11.0, 12.0, None, (0, 1), 110),
        ('cmi_radiotower', 'CMI V', 'The Old Radiotower', (48, 60), 9.0, 10.0, None, (0, 1), 110),
        ('cmi_noise', 'CMI V', 'Complex Noise', (48, 60), 7.0, 8.0, None, None, 110),
    ],
    'voices': [
        ('vox_ah', A + 'VOICES', 'Ah Voice', (53, 54, 58, 46), 10.0, 11.5, (0.0, 0.5), None, 120),
        ('vox_male', A + 'VOICES', 'Male Choir', (41, 46, 47), 10.0, 11.5, (0.0, 0.4), None, 120),
        ('vox_massive', A + 'VOICES', 'Massive Aah', (53, 41), 10.0, 11.5, (0.0, 0.8), None, 120),
        ('vox_hgc', A + 'VOICES', 'Human Guitar Computer', (41, 36), 10.0, 11.5, (0.0, 0.6), None, 120),
        ('vox_fire', A + 'VOICES', 'Fire Formants', (53, 60, 41), 10.0, 11.5, (0.0, 1.0), None, 120),
        ('vox_melting', A + 'VOICES', 'Granular Melting', (53, 41), 10.0, 11.5, (0.0, 1.0), (0, 1), 120),
        ('vox_dementia', A + 'VOICES', 'Dementia', (41, 29), 14.0, 15.5, (0.0, 1.0), None, 110),
        ('vox_breath', A + 'VOICES', 'Take A Breath', (48, 36), 10.0, 11.5, (0.0, 1.0), None, 110),
    ],
    # round 2: the Fairlight library at and near its natural pitch (note 72), for close detail
    'cmi2': [
        ('c2_bomb1', 'CMI V', 'BOMB1', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_carsmash', 'CMI V', 'CARSMASH', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_smash', 'CMI V', 'SMASH', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_shatter', 'CMI V', 'SHATTER', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_breakgls', 'CMI V', 'BREAKGLS', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_glasmash', 'CMI V', 'GLASMASH', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_crackle1', 'CMI V', 'CRACKLE1', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_sandwalk', 'CMI V', 'SANDWALK', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_mud', 'CMI V', 'MUD', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_rip', 'CMI V', 'RIP', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_symkick', 'CMI V', 'SYMKICK', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_kick1', 'CMI V', 'KICK1', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_stick1', 'CMI V', 'STICK1', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_rifle1', 'CMI V', 'RIFLE1', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_shtgun1', 'CMI V', 'SHTGUN1', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_gedclang', 'CMI V', 'GEDCLANG', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_factory', 'CMI V', 'FACTORY', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_hooves', 'CMI V', 'HOOVES', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_snort', 'CMI V', 'SNORT', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_wildboar', 'CMI V', 'WILDBOAR', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_pigs', 'CMI V', 'PIGS', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_mongrel', 'CMI V', 'MONGREL', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_gurgle', 'CMI V', 'GURGLE', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_whinny', 'CMI V', 'WHINNY', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_wngflaps', 'CMI V', 'WNGFLAPS', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_rooks', 'CMI V', 'ROOKS', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_crow', 'CMI V', 'CROW', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_goose', 'CMI V', 'GOOSE', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_dove', 'CMI V', 'DOVE', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_swan', 'CMI V', 'SWAN', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_dog', 'CMI V', 'DOG', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_elephant', 'CMI V', 'ELEPHANT', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_wolf', 'CMI V', 'WOLF', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_seal', 'CMI V', 'SEAL', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_cow', 'CMI V', 'COW', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_scream1', 'CMI V', 'SCREAM1', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_screech', 'CMI V', 'SCREECH', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_steam', 'CMI V', 'STEAM', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_jaildoor', 'CMI V', 'JAILDOOR', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_door', 'CMI V', 'DOOR', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
        ('c2_ambcrash', 'CMI V', 'AMBCRASH', (72, 65, 60, 53), 3.0, 4.0, None, None, 127),
    ],
    'voices2': [
        ('vox_ah_hi', A + 'VOICES', 'Ah Voice', (65, 70, 75, 62), 10.0, 11.5, (0.0, 0.3), None, 127),
        ('vox_massive_hi', A + 'VOICES', 'Massive Aah', (65, 72), 10.0, 11.5, (0.0, 0.3), None, 127),
        ('vox_candid', A + 'VOICES', 'Candid', (65, 72), 10.0, 11.5, (0.0, 0.2), None, 127),
    ],
    'brass': [
        ('brs_growl', A + 'BRASS', 'Growly Horn', (36, 31, 41), 10.0, 11.5, (0.0, 0.7), (0, 0.6), 120),
        ('brs_dreary', A + 'BRASS', 'Dreary Tales', (36, 29, 33), 10.0, 11.5, (0.0, 0.5), None, 120),
        ('brs_aven', A + 'BRASS', 'Avengement', (36, 31), 10.0, 11.5, (0.0, 0.6), None, 127),
        ('brs_beast', A + 'BRASS', 'Tamed Beast', (36, 29, 24), 14.0, 15.5, (0.0, 1.0), None, 120),
        ('brs_doom', A + 'BRASS', 'Certain Doom', (36, 29), 10.0, 11.5, (0.0, 0.5), None, 127),
    ],
    'winds': [
        ('wnd_beast', A + 'WOODWINDS', 'Evolving Beast', (48, 55, 60), 10.0, 11.5, (0.0, 1.0), None, 120),
        ('wnd_feral', A + 'WOODWINDS', 'Feral Rise', (43, 36), 10.0, 11.5, (0.0, 1.0), None, 120),
        ('wnd_howl', A + 'WOODWINDS', 'Noir Howl', (48, 43), 10.0, 11.5, (0.0, 0.6), None, 120),
        ('wnd_low', A + 'WOODWINDS', 'Low Blow', (29, 36), 14.0, 15.5, (0.0, 0.5), (0, 0.8), 110),
        ('wnd_air', A + 'WOODWINDS', 'Air', (60, 48), 10.0, 11.5, (0.0, 0.5), None, 110),
        ('wnd_breathing', A + 'WOODWINDS', 'Synaptic Breathing', (48, 36), 10.0, 11.5, (0.0, 0.5), None, 110),
        ('wnd_doom', A + 'WOODWINDS', 'Impending Doom', (36, 29), 10.0, 11.5, (0.0, 0.5), None, 127),
    ],
    'strings': [
        ('str_cello', A + 'STRINGS', 'Sacral Cello Spaces', (36, 31), 12.0, 13.5, (0.0, 0.5), (0, 1), 120),
        ('str_fear', A + 'STRINGS', 'Fear 2050', (36, 43), 10.0, 11.5, (0.0, 0.6), None, 120),
        ('str_metal', A + 'STRINGS', 'Frozen Metal Strings', (48, 60, 36), 10.0, 11.5, (0.0, 0.5), None, 120),
        ('str_drone', A + 'STRINGS', 'Dark Drone', (24, 31), 26.0, 27.5, (0.0, 0.6), None, 110),
        ('str_tension', A + 'STRINGS', 'Tension Rising', (48, 60), 10.0, 11.5, (0.0, 1.0), None, 120),
    ],
    'synths': [
        ('syn_birdpatch', 'Synthi V', 'Bird Patch (Mod Wheel)', (60, 72), 14.0, 15.0, None, (0, 1), 110),
        ('syn_seagulls', 'Synthi V', 'Seagulls Three', (60, 72), 12.0, 13.0, None, (1, 0), 110),
        ('syn_breathing', 'Synthi V', 'Heavy Breathing', (48, 36), 12.0, 13.0, None, (0, 1), 110),
        ('syn_windmill', 'Synthi V', 'Squeaky Feedback Windmill', (48, 60), 10.0, 11.0, None, (0, 1), 110),
        ('syn_fmsiren', 'Synthi V', 'FM Siren', (60, 72), 8.0, 9.0, None, (0, 1), 110),
        ('arp_fire', 'ARP 2600 V3', 'Fire', (48, 36), 16.0, 17.0, None, (0, 1), 110),
        ('arp_megasignal', 'ARP 2600 V3', 'Megasignal', (36, 48), 8.0, 9.0, None, (0, 1), 120),
        ('arp_mars', 'ARP 2600 V3', 'Signals From Mars', (48, 60), 10.0, 11.0, None, (0, 1), 110),
        ('arp_screamcut', 'ARP 2600 V3', 'Scream & Cut', (60, 72), 8.0, 9.0, None, None, 120),
        ('ms_ion', 'KORG MS-20 V', 'Ion Storm', (48, 60, 72), 8.0, 9.0, None, (0, 1), 110),
        ('ms_peak', 'KORG MS-20 V', 'Peak Screamer', (72, 84, 60), 8.0, 9.0, None, (0, 1), 120),
        ('ms_sanity', 'KORG MS-20 V', 'Losing Sanity', (84, 72, 36), 8.0, 9.0, None, (0, 1), 120),
        ('ms_steam', 'KORG MS-20 V', 'Steam Locomotive', (48,), 10.0, 11.0, None, None, 110),
        ('cs_wind', 'CS-80 V4', 'Big Wind', (48, 60), 12.0, 13.0, None, None, 110),
        ('cs_fblead', 'CS-80 V4', 'Feedback Distortion Lead', (72, 84, 36), 8.0, 9.0, None, (0, 1), 120),
        ('mod_firecrack', 'Modular V3', 'Fire Crack', (36, 48, 60), 8.0, 9.0, None, (0, 1), 110),
        ('mod_airleak', 'Modular V3', 'Air Leak', (60, 72), 8.0, 9.0, None, (0, 1), 110),
    ],
}

# Chromaphone 3 patches, programmed here by parameter index (see
# dumps\Chromaphone_3.txt; resonator A is 154..178, B is 179..199).
# {index: normalized value}
CH_BASE = {
    112: 0.0,  # mallet off
    137: 1.0,  # noise on
    138: 0.0,  # ADSR
    143: 1.0,  # sustain full
    149: 1.0,  # noise volume
    210: 0.0,
    352: 0.0, 357: 0.0,  # reverb mix off
    242: 0.0,  # delay wet off
}
CHROMA_PATCHES = {
    # torn metal: dense noise bowing a big plate coupled to a beam, slow attack
    'chr_scrape': ({**CH_BASE, **{113: 0.6, 114: 0.45, 119: 0.6, 121: 0.55, 126: 0.0, 140: 0.72, 144: 0.7,
                                    154: 0.625, 155: 0.5, 162: 0.8, 166: 0.35, 167: 0.6, 168: 0.6, 169: 0.2,
                                    179: 0.125, 180: 0.5, 183: 0.75, 200: 0.4, 202: 1.0}},
                   (31, 36, 43, 24), 9.0, 11.0, 120),
    # crackle: sparse clicks ringing a short stiff bar (embers, grit, a counter)
    'chr_crackle': ({**CH_BASE, **{113: 0.4, 114: 0.6, 121: 0.03, 126: 1.0, 140: 0.1, 144: 0.3,
                                     154: 0.125, 155: 0.6, 162: 0.3, 166: 0.5, 167: 0.3, 169: 0.3,
                                     179: 0.25, 180: 0.7, 183: 0.25, 200: 0.5, 202: 0.0}},
                    (60, 72, 48, 84), 12.0, 13.0, 110),
    # flue: dense low noise through a closed tube (air roaring in a chimney, a chest)
    'chr_flue': ({**CH_BASE, **{113: 0.2, 114: 0.5, 121: 0.9, 126: 0.0, 140: 0.7, 144: 0.75,
                                  154: 0.75, 155: 0.5, 162: 0.45, 166: 0.3, 167: 0.7,
                                  179: 0.875, 180: 0.42, 183: 0.45, 200: 0.5, 202: 1.0}},
                 (24, 31, 36, 43), 12.0, 13.5, 110),
    # rattle: sparse clicks in a closed tube (something loose in the chest)
    'chr_rattle': ({**CH_BASE, **{113: 0.2, 114: 0.55, 121: 0.06, 126: 1.0, 140: 0.2, 144: 0.5,
                                    154: 0.75, 155: 0.5, 162: 0.4, 166: 0.4, 167: 0.6,
                                    179: 0.375, 180: 0.4, 183: 0.4, 200: 0.5, 202: 1.0}},
                   (31, 36, 43), 12.0, 13.0, 110),
}
# struck patches (mallet on, noise off): the footfall's drumhead and a slab of metal
CH_HIT = {112: 1.0, 137: 0.0, 138: 0.0, 143: 0.0, 142: 0.9, 144: 0.8, 145: 1.0, 352: 0.0, 357: 0.0, 242: 0.0}
CHROMA_PATCHES.update({
    'chr_thud': ({**CH_HIT, **{105: 0.25, 108: 0.5, 111: 0.3, 154: 0.375, 155: 0.5, 162: 0.62, 166: 0.25,
                                 167: 0.8, 168: 0.4, 169: 0.25, 179: 0.75, 180: 0.4, 183: 0.6, 200: 0.45,
                                 202: 1.0}},
                 (24, 19, 28, 31, 36), 5.0, 6.0, 127),
    'chr_slab': ({**CH_HIT, **{105: 0.6, 108: 0.6, 111: 0.5, 154: 0.625, 155: 0.5, 162: 0.75, 166: 0.3,
                                 167: 0.7, 168: 0.6, 169: 0.15, 179: 0.125, 180: 0.5, 183: 0.7, 200: 0.4,
                                 202: 1.0}},
                 (24, 31, 36, 43), 7.0, 8.0, 127),
    'chr_tick': ({**CH_HIT, **{105: 0.9, 108: 0.2, 154: 0.125, 155: 0.7, 162: 0.3, 166: 0.6, 167: 0.2,
                                 179: 0.25, 180: 0.7, 183: 0.3, 200: 0.5, 202: 0.0, 142: 0.6}},
                 (72, 79, 84, 91, 96, 67), 1.5, 2.0, 100),
})

GROUPS = list(TABLE) + ['chroma']


def render_group(group, force=False, timeout=900):
    os.makedirs(STRIPS, exist_ok=True)
    meta_path = os.path.join(STRIPS, 'strips.json')
    meta = {}
    if os.path.exists(meta_path):
        with open(meta_path) as fh:
            meta = json.load(fh)
    j = Job('strips_' + group)
    todo = []
    tracks = []
    if group == 'chroma':
        rows = [(n, None, None, p[1], p[2], p[3], None, None, p[4]) for n, p in CHROMA_PATCHES.items()]
    else:
        rows = TABLE[group]
    for (name, inst, preset, notes, hold, slot, morph, wheel, vel) in rows:
        wav = os.path.join(STRIPS, name + '.wav')
        if os.path.exists(wav) and not force:
            continue
        t = j.track(name)
        if group == 'chroma':
            f = j.fx(t, CHROMA)
            for idx, val in CHROMA_PATCHES[name][0].items():
                j.set(t, f, idx, val)
        else:
            f = j.fx(t, VST[inst], chunk=chunk(inst, preset))
            if inst.startswith('Augmented'):
                j.set(t, f, 'Reverb', 0.0)
                j.set(t, f, 'Delay', 0.0)
        nts, evs, mor = [], [], []
        for i, n in enumerate(notes):
            a = 0.5 + i * slot
            nts.append((a, a + hold, n, vel))
            if morph:
                mor += [(a - 0.3, morph[0]), (a + 0.5, morph[0]), (a + hold * 0.8, morph[1]), (a + hold + 0.5, morph[1])]
            if wheel:
                for k in range(33):
                    v = wheel[0] + (wheel[1] - wheel[0]) * k / 32
                    evs.append((a + 0.3 + (hold - 0.6) * k / 32, 0xB0, 1, int(round(127 * v))))
                evs.append((a - 0.2, 0xB0, 1, int(round(127 * wheel[0]))))
        if mor:
            j.env(t, f, 'Morph', mor)
        length = 0.5 + len(notes) * slot + 1.0
        j.midi(t, length, notes=nts, events=evs)
        tracks.append(t)
        todo.append((name, t, length))
        meta[name] = {'instrument': inst or 'Chromaphone 3 (programmed here)', 'preset': preset or name,
                      'notes': list(notes), 'hold': hold, 'slot': slot, 'first': 0.5,
                      'morph': morph, 'wheel': wheel, 'velocity': vel}
    if not todo:
        print(f'[stems] {group}: nothing to do')
        return 0
    for (name, t, length) in todo:
        for u in tracks:
            j.mute(u, u != t)
        j.render(length, name=os.path.join('strips', name))
    code = j.run(timeout=timeout)
    with open(meta_path, 'w') as fh:
        json.dump(meta, fh, indent=1)
    return code


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('--group', default='')
    ap.add_argument('--force', action='store_true')
    a = ap.parse_args()
    for g in ([a.group] if a.group else GROUPS):
        render_group(g, a.force)
