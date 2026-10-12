#!/usr/bin/env python3
"""Check a repair 2026-10-11.4 probe's game log against SPEC 10's log criteria.

Usage (any working directory):
  python check_logs.py r1|r2|r3|r6|r5 <run output dir or game_stdout.log>

<run output dir> is the run-local.ps1 -OutputDir; the log is
<dir>/<test stem>/game_stdout.log (lobo_il28_tailgun, lobo_missing_refs,
lobo_wreck_burn, lobo_pilot_wounds, lobo_official_smoke). Matching is on
case-insensitive substrings after the ANSI colour codes are removed. Prints
one PASS/FAIL/INFO line per criterion and exits 1 when any criterion fails,
2 when the log is missing. Read-only; streams the log, so a multi-GB debug log
is fine. Assertions and screenshots are not re-checked here: a passed runner
plus PASS lines here plus a visual review is the full result.
"""
import re
import sys
from pathlib import Path

ANSI = re.compile(r'\x1b\[[0-9;]*m')
STEMS = {'r1': 'lobo_il28_tailgun', 'r2': 'lobo_missing_refs', 'r3': 'lobo_wreck_burn',
         'r6': 'lobo_pilot_wounds', 'r5': 'lobo_official_smoke'}
BS = '\\'

# SPEC 10 R2: the old names must not appear at all ...
R2_GONE = ['black_matte.pac', f'eaf{BS}tex{BS}back.pac', f'eaf{BS}tex{BS}legsbk.pac', f'rktf5{BS}swtex.paa', 'soundss']
# ... and the repointed targets must not fail to load.
R2_TARGETS = ['black matte.pac', f'eaf{BS}tex{BS}back.paa', f'eaf{BS}tex{BS}legsbk.paa',
              f'lobo_airammo{BS}tex{BS}sw.pac', f'lobomig{BS}sounds{BS}jet_crash.wss']
LOAD_FAIL = ('cannot load texture', 'load failed', 'not found')
R2_SPAWNS = ['spawned=LoBo_Ter_Toy_R ', 'spawned=LoBo_Ter_ToyMG_R ', 'spawned=LoBo_Egypt_Pilot ',
             'wounded=LoBo_Egypt_Pilot ', 'wounded=LoBo_Jordan_Pilot ', 'wounded=LoBo_IAF_Pilot_grey ',
             'spawned=LoBo_E_mig21bis_MR1 ', 'spawned=Lobo_F16_IAF_AA ', 'spawned=Lobo_F16_EAF_AA ']
PILOTS = ['LoBo_Egypt_Pilot', 'LoBo_IAF_Pilot', 'LoBo_IAF_Pilot2', 'LoBo_IAF_Pilot_grey', 'LoBo_IAF_Pilot_grey2',
          'LoBo_IAF_Pilot_grey3', 'LoBo_Jordan_Pilot', 'LoBo_Syria_Pilot']
LOBO_TEX = (f'lobo_pilot{BS}', f'loboter{BS}', f'lobomig{BS}', f'lobo_airammo{BS}', f'lobof16{BS}')


class Report:
    def __init__(self):
        self.failed = False

    def check(self, ok, text):
        print(('PASS ' if ok else 'FAIL ') + text)
        self.failed = self.failed or not ok

    @staticmethod
    def info(text):
        print('INFO ' + text)


def lines(path):
    with open(path, 'rb') as fh:
        for raw in fh:
            yield ANSI.sub('', raw.decode('latin1', 'replace').rstrip('\r\n'))


def common(log, rep, markers):
    """One pass: boot, script errors, access denied, plus the first line of each marker."""
    found = {m: None for m in markers}
    boot = errors = denied = 0
    first_error = None
    for line in lines(log):
        low = line.lower()
        if 'could not boot' in low:
            boot += 1
        if 'script error' in low or 'zero divisor' in low:
            errors += 1
            first_error = first_error or line.strip()
        if 'access denied' in low:
            denied += 1
        for m in markers:
            if found[m] is None and m in line:
                found[m] = line.strip()
    rep.check(boot == 0, f'mission booted (no "could not boot" line; if there is one, see '
                         f'agent_guidelines/flashing-main-menu.md and grep "Access denied") [{boot}]')
    rep.check(errors == 0, f'no "Script error" or "Zero divisor" line [{errors}]'
              + (f' first: {first_error}' if first_error else ''))
    rep.check(denied == 0, f'no "Access denied" line (an owner addon missing from addOns[]) [{denied}]')
    for m in markers:
        rep.check(found[m] is not None, f'marker "{m.strip()}" logged')
    return found


def texture_names(log, prefixes):
    names = {}
    for line in lines(log):
        low = line.lower()
        i = low.find('cannot load texture ')
        if i >= 0:
            name = low[i + len('cannot load texture '):].strip()
            if name.startswith(prefixes):
                names[name] = names.get(name, 0) + 1
    return names


def r1(log, rep):
    common(log, rep, ['LOBO_R4_TG case=C1', 'LOBO_R4_TG case=G', 'LOBO_R4_TG case=C2', 'LOBO_R4_TG case=C3',
                      'LOBO_R4_TG case=A', 'LOBO_R4_TG_DONE'])
    tail = sum(1 for line in lines(log) if 'tailgun.sqs' in line.lower() and 'error' in line.lower())
    rep.check(tail == 0, f'no error line naming tailgun.sqs [{tail}]')


def r2_criteria(log, rep):
    gone = {p: 0 for p in R2_GONE}
    target = {p: 0 for p in R2_TARGETS}
    backfuse = 0
    for line in lines(log):
        low = line.lower()
        for p in R2_GONE:
            if p in low:
                gone[p] += 1
        if any(k in low for k in LOAD_FAIL):
            for p in R2_TARGETS:
                if p in low:
                    target[p] += 1
        if 'backfuse.pac' in low:
            backfuse += 1
    for p, n in gone.items():
        rep.check(n == 0, f'old reference "{p}" absent [{n} lines]')
    for p, n in target.items():
        rep.check(n == 0, f'repointed target "{p}" never fails to load [{n} lines]')
    rep.info(f'backfuse.pac lines: {backfuse} (expected and recorded: one per F-16 variant, SPEC 6.5)')
    names = texture_names(log, LOBO_TEX)
    wound = {n: c for n, c in names.items() if n.startswith(f'lobo_pilot{BS}') and n.endswith('_d.paa')}
    rep.info(f'missing wound (_d.paa) textures under lobo_pilot: {wound or "none"}')
    rest = {n: c for n, c in names.items() if n not in wound}
    rep.info(f'other missing LoBo textures in the spawned models (not .4 targets): {rest or "none"}')


def r2(log, rep):
    common(log, rep, ['LOBO_R4_REFS_START'] + ['LOBO_R4_REFS ' + s for s in R2_SPAWNS] + ['LOBO_R4_REFS_DONE'])
    r2_criteria(log, rep)


def r3(log, rep):
    markers = ['LOBO_R4_BURN case=S ', 'LOBO_R4_BURN case=D1 before', 'LOBO_R4_BURN case=D1 after',
               'LOBO_R4_BURN_T0 ', 'LOBO_R4_BURN case=A ', 'LOBO_R4_BURN case=B start', 'LOBO_R4_BURN case=B end',
               'LOBO_R4_BURN case=C start', 'LOBO_R4_BURN case=C firstStep', 'LOBO_R4_BURN case=C end',
               'LOBO_R4_BURN case=D2 offered', 'LOBO_R4_BURN case=D2 actionWorked', 'LOBO_R4_BURN_D2_END ',
               'LOBO_R4_BURN_CLEANUP ', 'LOBO_R4_BURN_DONE']
    found = common(log, rep, markers)
    phase = 'before'
    scream = fire = fire_after_d2 = eng31 = 0
    fire_fail = 0
    for line in lines(log):
        low = line.lower()
        if 'LOBO_R4_BURN_T0 ' in line:
            phase = 'burn'
        elif 'LOBO_R4_BURN case=D2 actionWorked' in line and phase == 'burn':
            phase = 'after_put_out'
        elif 'LOBO_R4_BURN_D2_END ' in line:
            phase = 'cleanup_wait'
        elif 'LOBO_R4_BURN_CLEANUP ' in line:
            phase = 'after'
        play = 'openandplayonce' in low
        if play and 'eng_lookout3' in low and phase in ('burn', 'after_put_out'):
            scream += 1
        if 'eng31' in low and phase in ('burn', 'after_put_out'):
            eng31 += 1
        if play and f'objects{BS}fire' in low and phase in ('burn', 'after_put_out', 'cleanup_wait'):
            fire += 1
            if phase in ('after_put_out', 'cleanup_wait'):
                fire_after_d2 += 1
        if f'objects{BS}fire' in low and any(k in low for k in LOAD_FAIL):
            fire_fail += 1
    end = found['LOBO_R4_BURN_D2_END ']
    steps = int(re.search(r'stepsTotal=(\d+)', end).group(1)) if end and 'stepsTotal=' in end else None
    if steps is None:
        rep.check(False, 'D2_END marker carries stepsTotal')
    else:
        rep.check(scream == steps, f'one eng_lookout3 scream per burn step between t0 and D2 end '
                                   f'[{scream} lines, {steps} steps]'
                  + ('  STOP (SPEC 11): more than one scream per step' if scream > steps else ''))
    rep.info(f'lines naming eng31 in the same window: {eng31}')
    rep.check(fire >= 100, f'at least 100 say "fire" plays (objects\\fire) between t0 and cleanup [{fire}]')
    rep.check(fire_after_d2 > 0, f'say "fire" continues after the glow was put out [{fire_after_d2}]')
    rep.check(fire_fail == 0, f'no load failure for objects\\fire [{fire_fail}]')
    for key in ('LOBO_R4_BURN case=D1 before', 'LOBO_R4_BURN case=B end', 'LOBO_R4_BURN case=C end',
                'LOBO_R4_BURN case=D2 offered', 'LOBO_R4_BURN case=D2 actionWorked', 'LOBO_R4_BURN_D2_END ',
                'LOBO_R4_BURN_CLEANUP '):
        if found.get(key):
            rep.info(found[key][found[key].find('LOBO_R4'):])


def r6(log, rep):
    found = common(log, rep, [f'LOBO_R4_WOUNDS wounded class={c} ' for c in PILOTS] + ['LOBO_R4_WOUNDS_DONE'])
    for c in PILOTS:
        line = found[f'LOBO_R4_WOUNDS wounded class={c} '] or ''
        rep.check('alive=true' in line, f'{c} alive after wounding')
    done = found['LOBO_R4_WOUNDS_DONE'] or ''
    rep.check('failed=[]' in done, f'no class below the wound threshold ({done[done.find("failed="):] or "no marker"})')
    names = texture_names(log, (f'lobo_pilot{BS}',))
    rep.info(f'missing textures under lobo_pilot (wound _d.paa names would show here): {names or "none"}')


def r5(log, rep):
    common(log, rep, ['LOBO_R4_SMOKE_START', 'LOBO_R4_TG case=G', 'LOBO_R4_REFS_DONE', 'LOBO_R4_SMOKE_SMOKER',
                      'LOBO_R4_SMOKE_DONE'])
    smoker = next((line for line in lines(log) if 'LOBO_R4_SMOKE_SMOKER' in line), '')
    rep.check('smokers=1' in smoker, f'exactly one M1Smoker ({smoker[smoker.find("smokers="):] or "no marker"})')
    r2_criteria(log, rep)


def main(argv):
    if len(argv) != 3 or argv[1] not in STEMS:
        print(__doc__)
        return 2
    target = Path(argv[2])
    log = target if target.is_file() else target / STEMS[argv[1]] / 'game_stdout.log'
    if not log.is_file():
        print(f'no log at {log}')
        return 2
    print(f'log: {log} ({log.stat().st_size:,} bytes)')
    rep = Report()
    globals()[argv[1]](log, rep)
    print('RESULT: ' + ('FAIL' if rep.failed else 'PASS (log criteria only; review the screenshots)'))
    return 1 if rep.failed else 0


if __name__ == '__main__':
    raise SystemExit(main(sys.argv))
