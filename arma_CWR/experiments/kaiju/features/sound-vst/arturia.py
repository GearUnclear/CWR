r"""Arturia factory presets without the plugin's own browser.

An Arturia factory preset is a text archive under
C:\ProgramData\Arturia\Presets\<instrument>\Factory\... . The state REAPER
stores for the VST3 plugin is the very same archive, twice (processor and
controller), each behind an 8 byte header: length, then 1 for the first and 0
for the second (found by saving a state with kj_save_chunk and reading it).
So a factory preset can be loaded headless by building that state from the
preset file and handing it to the plugin as its saved state
(TrackFX_SetNamedConfigParm "vst_chunk").

    python arturia.py index "Augmented VOICES"        # writes dumps\presets_<instrument>.tsv
    python arturia.py find "Augmented VOICES" scream   # search names, types and descriptions

    from arturia import chunk
    path = chunk('Augmented VOICES', 'Angst')          # base64 state file for Job.fx(..., chunk=path)
"""
import base64
import os
import re
import struct
import sys

ROOT = r'C:\ProgramData\Arturia\Presets'
NOTES = r'D:\Arma_CWA_worktrees\kaiju-notes\features\sound-vst'


def files(instrument):
    out = {}
    base = os.path.join(ROOT, instrument)
    for d, _, fs in os.walk(base):
        for f in fs:
            if '.' in f:
                continue
            out.setdefault(f, os.path.join(d, f))
    return out


def tokens(data, limit=400):
    """Reads the first tokens of the archive: numbers, and strings written as '<length> <bytes>'."""
    pos = 0
    out = []
    n = len(data)
    while pos < n and len(out) < limit:
        while pos < n and data[pos:pos + 1] == b' ':
            pos += 1
        m = re.match(rb'(\d+) ', data[pos:pos + 12])
        if not m:
            end = data.find(b' ', pos)
            out.append(data[pos:end])
            pos = end
            continue
        num = int(m.group(1))
        start = pos + len(m.group(0))
        cand = data[start:start + num]
        # a string when the byte after it is a space and it holds a letter
        if num > 1 and data[start + num:start + num + 1] == b' ' and re.search(rb'[A-Za-z]', cand):
            out.append(cand)
            pos = start + num
        else:
            out.append(m.group(1))
            pos = start - 1
    return out


def meta(path):
    with open(path, 'rb') as fh:
        data = fh.read(6000)
    t = [x.decode('latin-1') for x in tokens(data)]
    info = {'desc': ''}
    for i, x in enumerate(t):
        if x in ('Type', 'Subtype', 'Characteristics') and i + 1 < len(t) and x not in info:
            info[x] = t[i + 1]
        if len(x) > 40 and ' ' in x and not info['desc'] and 'Characteristics,' not in x:
            info['desc'] = x
    return info


def index(instrument):
    rows = []
    for name, path in sorted(files(instrument).items()):
        m = meta(path)
        rows.append((name, m.get('Type', ''), m.get('Subtype', ''), m.get('Characteristics', ''), m['desc']))
    return rows


def chunk(instrument, preset):
    path = files(instrument)[preset]
    with open(path, 'rb') as fh:
        p = fh.read()
    blob = struct.pack('<II', len(p), 1) + p + struct.pack('<II', len(p), 0) + p
    out_dir = os.path.join(NOTES, 'chunks')
    os.makedirs(out_dir, exist_ok=True)
    out = os.path.join(out_dir, re.sub(r'[^A-Za-z0-9]+', '_', instrument + '__' + preset) + '.b64')
    with open(out, 'w') as fh:
        fh.write(base64.b64encode(blob).decode('ascii'))
    return out


if __name__ == '__main__':
    cmd, inst = sys.argv[1], sys.argv[2]
    rows = index(inst)
    if cmd == 'index':
        out = os.path.join(NOTES, 'dumps', 'presets_' + re.sub(r'[^A-Za-z0-9]+', '_', inst) + '.tsv')
        with open(out, 'w', encoding='utf-8') as fh:
            for r in rows:
                fh.write('\t'.join(r) + '\n')
        print(len(rows), 'presets ->', out)
    else:
        pat = re.compile(sys.argv[3], re.I)
        for r in rows:
            if pat.search(' '.join(r)):
                print(' | '.join(r))
