r"""Makes a private REAPER resource folder for headless renders.

    python setup_res.py [<resource_dir>]

Reads the owner's %APPDATA%\REAPER as a read-only source and writes a copy of
the few files a headless render needs into <resource_dir> (default:
kaiju-notes\features\sound-vst\reaper-res). Nothing is ever written to
%APPDATA%\REAPER.

Copied as they are: the plugin scan caches (so REAPER does not rescan several
hundred plugins), the licence file (without it REAPER opens a "still
evaluating" window that nobody can click on a hidden desktop).
Rewritten: REAPER.ini. From the owner's file only the [reaper] keys that are
plain preferences are kept; every window position, recent project, project
tab, MIDI device and audio device line is dropped, and these are forced:
  audio system = dummy (no device is opened, nothing can sound)
  no version check on start (no network), no splash, new empty project on start
"""
import os
import shutil
import sys

SRC = os.path.join(os.environ['APPDATA'], 'REAPER')
DST = sys.argv[1] if len(sys.argv) > 1 else r'D:\Arma_CWA_worktrees\kaiju-notes\features\sound-vst\reaper-res'

COPY = ['reaper-vstplugins64.ini', 'reaper-vstshells64.ini', 'reaper-license.rk', 'reaper-reginfo2.ini',
        ]

KEEP_KEYS = {'vstpath64', 'vstfullstate', 'workthreads', 'workbehvr', 'autonbworkerthreads', 'workrender',
             'undomaxmem', 'lv2path_win64'}

FORCED = """multinst=1
splash=0
splashfast=1
verchk=0
loadlastproj=2
newprojdo=0
renderclosewhendone=1
lastproject=
projecttabs=0
midiins=0
midiouts=0
"""

AUDIO = """[audioconfig]
mode=4
allow_sr_override=1
asio_driver=-1
asio_driver_name=
dummy_srate=44100
dummy_blocksize=1024
"""


def main():
    os.makedirs(DST, exist_ok=True)
    for name in COPY:
        s = os.path.join(SRC, name)
        if os.path.exists(s):
            shutil.copy2(s, os.path.join(DST, name))
            print('copied', name)
    kept = []
    section = ''
    nag = []
    with open(os.path.join(SRC, 'REAPER.ini'), 'r', encoding='utf-8', errors='replace') as fh:
        for line in fh:
            line = line.rstrip('\r\n')
            if line.startswith('['):
                section = line
                continue
            if section == '[reaper]' and '=' in line and line.split('=', 1)[0] in KEEP_KEYS:
                kept.append(line)
            if section == '[nag]':
                nag.append(line)
    out = '[reaper]\n' + '\n'.join(kept) + '\n' + FORCED
    if nag:
        out += '[nag]\n' + '\n'.join(nag) + '\n'
    out += AUDIO
    with open(os.path.join(DST, 'reaper.ini'), 'w', encoding='utf-8', newline='\r\n') as fh:
        fh.write(out)
    print('wrote', os.path.join(DST, 'reaper.ini'))


if __name__ == '__main__':
    main()
