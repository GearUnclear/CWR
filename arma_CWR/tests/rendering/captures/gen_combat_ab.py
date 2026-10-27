"""Generate the combat light A/B capture test (step B4; plan sections 7.2 and 8).

    python gen_combat_ab.py [--no-timing] [--out <dir>]

Writes combat_ab.test.sqf + .test.toml next to this file. The B4 hero views
(plan section 8: the `ffh` flash and the flare `flr`) are not static scenes:
the shell and the flare move and the fire flickers, so each still is shot in
P mode (plan section 7.2): the simulation is stepped at a fixed 1/30 s, paused,
and the same paused frame is shot three times, with the classic lighting
(label <view>__classic), with the combat switches off (<view>__off, the off
setting of features_ab) and with the shipped defaults (<view>__on), 6 rendered
frames after each switch. The three shots of a still share the simulation
state exactly. The two stills without a burst, combat_fires and
combat_eye_k00, are shot twice more on the same paused frame: with the
combat light on smoke off (<view>__smoke0, combatSmoke=0) and on the air off
(<view>__scatter0, combatScatter=0), to tell which of the two lights the
burning wreck's black plume (it must read darker than the sky above its lit
base, as in Classic).

The kNN label of a still counts the fixed 1/30 s steps triSimFrames ran
since the trigger (the shell fired, the volley), not the age of the burst:
pausing and unpausing for each still take a simulation step each, so every
earlier still of a sequence adds two steps, and a shell bursts about 5 steps
after it is fired. A probe of combat_eye found the burst 0.03 s old at k05,
0.13 s at k06 and 0.27 s at k08; by the same count k11 is 0.43 s and k16
0.67 s after it.

Views, all on Abel at 23:00 on a moonless night, the ffh stage (capFFStageN's
layout: three SoldierWB and three SoldierEB posed, a burning Ural at [36, 13],
a Fire at [21, -12]):
  - combat_fires: the stage before any shell, camera 5 m up (ffh): the fire
    and the burning wreck alone;
  - combat_shell_kNN: a Shell120 fired straight down from 40 m at [27, 1];
  - combat_eye_kNN: the same shell at [40, -6] seen from eye height (the
    research's ffn view, where the flash frame read darker than Classic);
    k00 is the stage before the shell;
  - combat_flare_kNN: a white Flare fired upward from 45 m at [30, -35];
  - combat_muzzle_kNN: the research's muz view, a rifleman and a machine
    gunner (fighters, MOVE off) fire one volley at night (the stage without
    the fire and the wreck);
  - combat_day_kNN: the ffh stage and shell at 12:30 (the gate's day side:
    continuous lights at 12%, flashes at a fifth, no shadow mask);
  - two views without any combat light, shot off then on in one game, where
    the shader changes must leave every pixel as it was: sunrise_ridge (day)
    and the moonless road with a Jeep's headlights (night, lamps only).
Unless --no-timing, the test ends with three interleaved off / on rounds at
combat_fires, unpaused, 90 settle frames each, for the GPU sections (the
capture set's capMode / capReadTimers rhythm), labels combat_timing_rN; and
three at the paused flash frame of combat_eye (k05: the worst case, a burst
lighting the whole view with its shadow traced), 60 frames each, labels
combat_flashtiming_rN.

1920x1080 windowed, render scale 1.0, 4x MSAA, the presented frame. Run it with
run-captures.ps1 -Tests tests/rendering/captures/combat_ab.test.sqf.
"""
import argparse
from pathlib import Path

import capgen
import views

NAME = "combat_ab"
FPS = 30
OFF = "remastered,combatLights=0,combatShadows=0"
SETTINGS = (("classic", "classic"), ("off", OFF), ("on", "remastered"))
# The stills without a burst: the lit smoke and the lit air each off as well.
KNOBS = (("smoke0", "remastered,combatSmoke=0"), ("scatter0", "remastered,combatScatter=0"))

STAGE_FFH = dict(name="ffh", x=7600, z=10850, eye=5, h=0, p=-12, date=[1985, 7, 12, 23, 0], vd=1000)
STAGE_EYE = dict(STAGE_FFH, name="ffn", eye=1.8, p=-2)
HELPERS = r'''
// [forward, right, height, [vx, vy, vz], ammo] call capShellAt: a shell of that
// ammo fired from a point relative to the staging view (createShell).
capShellAt = {
    private ["_p"];
    _p = [_this select 0, _this select 1] call capRel;
    capShellObj = createShell [_this select 4, [_p select 0, _p select 1, _this select 2], _this select 3, player, objNull];
    format ["view=%1;event=shell;ammo=%2;pos=%3;object=%4", capView, _this select 4, _p, capShellObj] call capLog;
}
capPause = { capPauseRes = triPauseGame }
capUnpause = { capPauseRes = triUnpauseGame }
capFixStep = { capFixRes = triSetFixedStep _this }
capBurn = { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
capVolley = { { _x fire (primaryWeapon _x) } forEach capShooters }
capTimerReset = { capGpuOff = triGpuTimes false; capGpuOn = triGpuTimes true; capPerfRes = triPerfReset 0 }
'''
STAGE_MUZ = dict(name="muz", x=7640, z=10860, eye=1.6, h=20, p=-4, date=[1985, 7, 12, 23, 0], vd=800)
MUZ_PROPS = ['capShooters = [];',
             '{ capNew = [[_x select 2], _x select 0, _x select 1, -80, "fighter"] call capSpawn; '
             'if (not (isNull capNew)) then { capShooters = capShooters + [capNew] } } forEach '
             '[[6.5, 1.2, "SoldierWB"], [9.5, -1.8, "SoldierWMG"]];']


def stage_block(label, stage, sim_time, props=None):
    v = dict(stage, props=views.FFH_PROPS if props is None else props)
    out = capgen.begin(v, label, sim_time=sim_time)
    out.append("triSimFrames 150")
    out.append('"remastered" call capMode')
    out.append("[] call capPin")
    out.append("[] call capBurn")
    out.append(f"{capgen.num(1.0 / FPS)} call capFixStep")
    out.append("triSimFrames 30")
    return out


def p_still(label, knobs=False):
    """The current (paused) frame shot with each setting (and each knob off)."""
    out = ["[] call capPause"]
    for suffix, setting in SETTINGS + (KNOBS if knobs else ()):
        shot = f"{label}__{suffix}"
        out += [f'"{setting}" call capModeQuick', "triSimFrames 6", f'triScreenshot "{shot}"', f'"{shot}" call capRecord']
    out.append("[] call capUnpause")
    return out


def p_sequence(label, steps, fire_sqf):
    """Fire, then a P-mode still at each step count after it."""
    out = [fire_sqf]
    done = 0
    for k in steps:
        out.append(f"triSimFrames {k - done}")
        done = k
        out += p_still(f"{label}_k{k:02d}")
    out.append("triSimFrames 60")
    return out


def s_pair(v, label, sim_time):
    """A view without combat light, off then on in one game (features_ab rhythm)."""
    out = capgen.begin(v, label, sim_time=sim_time)
    out.append("triSimFrames 150")
    out += capgen.shot(v, label, OFF, f"{label}__off")
    out += capgen.shot(v, label, "remastered", f"{label}__on")
    out += capgen.end(v)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--no-timing", action="store_true")
    ap.add_argument("--out", default=str(capgen.HERE))
    a = ap.parse_args()
    title = "Combat light A/B (B4) on Abel: gen_combat_ab.py; P-mode stills classic / off / on."
    lines = [capgen.header("Abel", STAGE_FFH, title, extra_includes=("video/capture_motion.sqf",))]
    lines.append(HELPERS)

    # Steady fires, then the shell, from 5 m up.
    lines.append("\n// ---- combat_fires and combat_shell (the ffh stage, 5 m up)")
    lines += stage_block("combat_fires", STAGE_FFH, 2000)
    lines += p_still("combat_fires", knobs=True)
    lines += p_sequence("combat_shell", [5, 6, 8, 11, 16],
                        '[27, 1, 40, [0, 0, -250], "Shell120"] call capShellAt')
    lines += ["[] call capClear", "0 call capFixStep", "call { capDriftRes = triPinCloudDrift [] }"]

    # The same stage at eye height: the flash frame against Classic's.
    lines.append("\n// ---- combat_eye (the ffn view: eye height)")
    lines += stage_block("combat_eye", STAGE_EYE, 2600)
    lines += p_still("combat_eye_k00", knobs=True)
    lines += p_sequence("combat_eye", [5, 6, 8, 11],
                        '[40, -6, 40, [0, 0, -250], "Shell120"] call capShellAt')
    lines += ["[] call capClear", "0 call capFixStep", "call { capDriftRes = triPinCloudDrift [] }"]

    # The flare, from 5 m up.
    lines.append("\n// ---- combat_flare (the flr view)")
    lines += stage_block("combat_flare", STAGE_FFH, 3200)
    lines += p_sequence("combat_flare", [90, 240],
                        '[30, -35, 45, [0, 0, 12], "Flare"] call capShellAt')
    lines += ["[] call capClear", "0 call capFixStep", "call { capDriftRes = triPinCloudDrift [] }"]

    # Muzzle flashes at night, from eye height beside the shooters.
    lines.append("\n// ---- combat_muzzle (the research's muz view)")
    lines += stage_block("combat_muzzle", STAGE_MUZ, 3500, props=MUZ_PROPS)
    lines += p_sequence("combat_muzzle", [4, 5, 6, 8], "[] call capVolley")
    lines += ["[] call capClear", "0 call capFixStep", "call { capDriftRes = triPinCloudDrift [] }"]

    # The same stage and shell at noon.
    lines.append("\n// ---- combat_day (noon)")
    lines += stage_block("combat_day", dict(STAGE_FFH, name="ffday", date=[1985, 7, 5, 12, 30]), 3650)
    lines += p_still("combat_day_k00")
    lines += p_sequence("combat_day", [5, 6], '[27, 1, 40, [0, 0, -250], "Shell120"] call capShellAt')
    lines += ["[] call capClear", "0 call capFixStep", "call { capDriftRes = triPinCloudDrift [] }"]

    # Views without any combat light.
    sunrise = dict(next(v for v in views.TIMING_ABEL if v["name"] == "sunrise_ridge"))
    road = dict(next(v for v in views.TIMING_ABEL if v["name"] == "road_flat_drive_moonless"))
    lines.append("\n// ---- views without combat light (off must equal on)")
    lines += s_pair(sunrise, "nc_sunrise_ridge", 3800)
    lines += s_pair(road, "nc_road_night", 4400)

    if not a.no_timing:
        lines.append("\n// ---- timing: three interleaved off / on rounds at combat_fires, unpaused")
        v = dict(STAGE_FFH, props=views.FFH_PROPS, refresh=views.FFH_REFRESH)
        lines += capgen.begin(v, "combat_timing", sim_time=5000)
        lines.append("triSimFrames 150")
        for r in range(1, 4):
            lines += capgen.shot(v, "combat_timing", OFF, f"combat_timing_r{r}__off")
            lines += capgen.shot(v, "combat_timing", "remastered", f"combat_timing_r{r}__on")
        lines += capgen.end(v)
        # The flash frame, paused: the GPU sections over 60 rendered frames.
        lines.append("\n// ---- timing at the paused flash frame (combat_eye k05)")
        lines += stage_block("combat_flashtiming", STAGE_EYE, 5600)
        lines.append('[40, -6, 40, [0, 0, -250], "Shell120"] call capShellAt')
        lines.append("triSimFrames 5")
        lines.append("[] call capPause")
        for r in range(1, 4):
            for suffix, setting in (("off", OFF), ("on", "remastered")):
                shot = f"combat_flashtiming_r{r}__{suffix}"
                lines += [f'"{setting}" call capModeQuick', "triSimFrames 6", "[] call capTimerReset",
                          "triSimFrames 60", "[] call capReadTimers", f'triScreenshot "{shot}"',
                          f'"{shot}" call capRecord']
        lines.append("[] call capUnpause")
        lines += ["[] call capClear", "0 call capFixStep", "call { capDriftRes = triPinCloudDrift [] }"]

    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = Path(a.out)
    capgen.write(out / f"{NAME}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / f"{NAME}.test.toml", capgen.toml("Abel", title, views.WORLDS, timeout=3600))
    print(f"{NAME}: written")


if __name__ == "__main__":
    main()
