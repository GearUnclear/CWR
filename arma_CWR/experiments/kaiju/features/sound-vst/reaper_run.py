r"""Runs one ReaScript (Lua) in a private, hidden, silent REAPER and waits for it.

    python reaper_run.py <script.lua> [--timeout 180] [--name tag]

REAPER is started with its own resource folder (-cfgfile <res>\reaper.ini, see
setup_res.py), as a new instance, on a private Windows desktop (the repo's
tmp\tri-hidden-desktop.ps1), so no window ever reaches the owner's desktop and
no audio device is opened. The script must end by saving the project and
quitting REAPER (lib.lua: kj_quit). If REAPER is still there after the
timeout, the wrapper stops the process tree it started (by process id) and
this returns 124.

Importable: run(lua_path, timeout=180, name=None) -> exit code.
"""
import argparse
import os
import subprocess
import sys
import time

REAPER = r'C:\Program Files\REAPER (x64)\reaper.exe'
NOTES = r'D:\Arma_CWA_worktrees\kaiju-notes\features\sound-vst'
RES = os.path.join(NOTES, 'reaper-res')
WRAPPER = r'D:\Arma_CWA\tmp\tri-hidden-desktop.ps1'
LOGS = os.path.join(NOTES, 'logs')
JOBS = os.path.join(NOTES, 'jobs')


def run(lua_path, timeout=180, name=None):
    lua_path = os.path.abspath(lua_path)
    name = name or os.path.splitext(os.path.basename(lua_path))[0]
    os.makedirs(LOGS, exist_ok=True)
    os.makedirs(JOBS, exist_ok=True)
    cmd_file = os.path.join(JOBS, name + '.cmd')
    with open(cmd_file, 'w') as fh:
        fh.write('@echo off\r\n')
        fh.write(f'"{REAPER}" -cfgfile "{os.path.join(RES, "reaper.ini")}" -newinst -nosplash -ignoreerrors "{lua_path}"\r\n')
        fh.write('exit /b %ERRORLEVEL%\r\n')
    log = os.path.join(LOGS, name + '.wrapper.log')
    t0 = time.time()
    p = subprocess.run(['powershell', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', WRAPPER,
                        '-Cmd', cmd_file, '-Log', log, '-WorkDir', JOBS,
                        '-DesktopName', 'kjvst' + str(os.getpid()), '-TimeoutSec', str(int(timeout))],
                       capture_output=True, text=True)
    dt = time.time() - t0
    print(f'[reaper_run] {name}: exit {p.returncode} in {dt:.1f} s  {p.stdout.strip().replace(chr(10), " | ")}')
    if p.stderr.strip():
        print(p.stderr.strip())
    return p.returncode


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('lua')
    ap.add_argument('--timeout', type=int, default=180)
    ap.add_argument('--name', default=None)
    a = ap.parse_args()
    sys.exit(run(a.lua, a.timeout, a.name))
