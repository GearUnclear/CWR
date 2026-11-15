"""Dedushka animation generator: a model's joint file in, every clip out.

  python gen_ded_anim.py --rest <dedushka_rest.json> --out <dir> [--depth 6]

Plain Python with numpy; no Blender, no stock file is read, so nothing here
is Bohemia-derived. Run it again on the final model's joint file and the
whole set is rebuilt for that body (see SKELETON.md for what the joint file
and the model must provide).

Written to <dir>:
  ded_<clip>.rtm            one RTM per move (14 files)
  ded_anim_gen.hpp          #defines the moves class needs (seconds, turn rate)
  ded_anim_timing.json      every clip: seconds, step, events with positions
  ded_anim_timing.sqf       the same for mission scripts (KJ_anim* globals)
  FOOTFALLS.md              the footfall and event table, readable
  ded_anim_report.txt       the offline checks (slide, reach, speeds, key steps)

The direction: he is in pain and exhausted, every motion costs him, and
slowness is scale, so nothing snaps. Feet are placed by IK and held on the
ground exactly: the engine moves the body by step x phase, so a planted foot
recedes in the clip at exactly that rate.
"""
import argparse
import json
import math
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ADDON_PKG = os.path.normpath(os.path.join(HERE, "..", "..", "..", "..", "apps", "tools", "BlenderAddon",
                                          "io_import_p3d"))
sys.path.insert(0, ADDON_PKG)
sys.path.insert(0, HERE)
import rtm_io  # noqa: E402
import ded_rig as R  # noqa: E402
from ded_rig import add, mix, pulse, ramp, smooth, smoother  # noqa: E402

TAU = 2.0 * math.pi
ADDON = "uddedushka"


# ===========================================================================
# TUNING. Everything that sets the feel is in this block. A joint file may
# override any of it under its "anim" key (the same names).
# ===========================================================================
def default_profile(rig):
    lean = rig.spine_lean_deg()
    upright = lean < 8.0            # a model sculpted standing straight (the proxy)
    hand_y = min(float(rig.points["hand_l"][1]), float(rig.points["hand_p"][1]))
    return {
        # Carriage. hunch: extra forward bow of his normal stance, on top of
        # the rest pose. tall: how far he un-bows from the REST pose for the
        # beam. A model sculpted hunched wants hunch 0 and tall about its lean.
        "hunch": 30.0 if upright else 0.0,
        "tall": 0.0 if upright else min(0.8 * lean, 30.0),
        "tall_arch": 4.0,           # then leans back this much more, chest to the sky
        "knee_flex": 0.965,         # leg length in the normal stance, as a share of full reach
        "pelvis_back": 0.065,       # pelvis set back under the bowed chest, as a share of leg length
        # Walk. One cycle is two steps.
        "walk_secs": 7.0,
        "stride": 0.95,             # metres per cycle as a share of leg length
        "duty": 0.64,               # share of the cycle each foot is down (0.5 = no double support)
        "foot_lift": 0.105,         # swing height as a share of leg length
        "foot_drop": 0.055,         # the last straight drop onto the ground, share of leg length
        "sway": 0.20,               # sideways weight shift as a share of the foot spacing
        "body_lag": 0.028,          # the trunk hangs back at each footfall, share of leg length
        "walk_slowest": 0.6,        # relSpeedMin of the walk
        "walk_blend": 1.0,          # seconds to blend into the walk and the turns
        # Turn on the spot. turn_rate is the engine's turnSpeed for the turn moves.
        "turn_rate": 0.20,          # rad/s
        "turn_secs": 6.0,
        "walk_turn_rate": 0.05,     # rad/s while walking (a planted foot slides when the body turns)
        "idle_turn_rate": 0.03,     # rad/s standing, to finish a turn (under 10 degrees left)
        # Durations, seconds.
        "idle_secs": 12.0, "dormant_secs": 16.0, "roar_secs": 9.0,
        "rise_secs": 6.5, "hold_secs": 2.4, "lower_secs": 7.5,
        "stomp_secs": 8.0, "swat_secs": 9.0, "flinch_secs": 2.6, "die_secs": 5.0,
        "stomp_foot": "p",          # p = right, l = left
        "stomp_reach": 0.40,        # how far ahead the foot comes down, share of leg length
        "stomp_height": 0.42,       # how high it is lifted, share of leg length
        "swat_arm": "p",
        "swat_up": 22.0,            # degrees above level the arm sweeps at
        # Keep hands that reach the ground from dragging through it (metres), or None.
        "hand_clearance": 0.012 * rig.height if hand_y < 0.05 * rig.height else None,
        # ---- finishing round (2026-11-12). Every default leaves the clips as they were. ----
        # Arms carried this many degrees out from the body in every pose, so long claws clear the thighs.
        "arm_out_l": 0.0, "arm_out_p": 0.0,
        # More backward swing of the arm while the leg of its own side comes forward (degrees, walk only).
        "arm_back": 0.0,
        # Tail: carried this many degrees back off the leg, sway scaled, a whip of this many degrees
        # running down it after each footfall.
        "tail_lift": 0.0, "tail_sway": 1.0, "tail_whip": 0.0,
        # Hanging parts (ded_hang1, ded_hang2): scale of their swing.
        "hang_swing": 1.0,
        # Feet: degrees the heel peels off before toe-off; degrees the toes hang in the swing.
        "heel_off": 15.0, "toe_droop": 0.0,
        # One-shot clips get extra keys wherever a bone turns more than this between two keys (degrees).
        "key_step_max": 13.5,
        # Arms in the tall pose of the beam: degrees swung forward (negative: back) and out. A creature
        # whose hand hangs before its pelvis (D's lizard arm) wants them forward, or the pelvis, which
        # comes forward as the back straightens, runs into the hand.
        "tall_arm_swing": -7.0, "tall_arm_out": 15.0,
        # The fall. fall_crumple 1: he falls on his chest and what reaches further forward gives way
        # (see "The fall" in main). fall_depth: metres he is lifted as he goes over, the forward depth
        # of his trunk; None = the --depth of the command line (his whole forward reach).
        "fall_crumple": 0, "fall_depth": None,
    }


# ===========================================================================
# Poses
# ===========================================================================
CARRIAGE = {"arm_out_l": 0.0, "arm_out_p": 0.0, "tail_lift": 0.0, "tail_sway": 1.0, "hang_swing": 1.0}


def arms(pose):
    """Resolve the gravity-relative arm channels into bone channels.

    larm.swing / parm.swing: forward swing of the hanging arm from the plumb
    line; larm.out / parm.out: lift out to the side. The trunk's own bow and
    lean are taken out, so a heavy arm hangs straight down whatever the back
    does."""
    pose = dict(pose)
    t_pitch = pose.get("root.pitch", 0.0) + pose.get("zebra.pitch", 0.0) + pose.get("hrudnik.pitch", 0.0)
    t_roll = pose.get("root.roll", 0.0) + pose.get("zebra.roll", 0.0) + pose.get("hrudnik.roll", 0.0)
    for s, sign in (("l", -1.0), ("p", 1.0)):
        sw = pose.pop(s + "arm.swing", 0.0)
        out = pose.pop(s + "arm.out", 0.0) + CARRIAGE["arm_out_" + s]
        sh_roll = pose.get(s + "rameno.roll", 0.0)
        pose[s + "biceps.pitch"] = pose.get(s + "biceps.pitch", 0.0) + sw - t_pitch
        pose[s + "biceps.roll"] = pose.get(s + "biceps.roll", 0.0) + sign * out - t_roll - sh_roll
    if CARRIAGE["tail_sway"] != 1.0:
        for b in ("ded_tail1", "ded_tail2", "ded_tail3"):
            for ch in (".yaw", ".roll"):
                if b + ch in pose:
                    pose[b + ch] = pose[b + ch] * CARRIAGE["tail_sway"]
    if CARRIAGE["tail_lift"]:
        # a hanging part swings BACK for a negative pitch (ded_rig.rx)
        pose["ded_tail1.pitch"] = pose.get("ded_tail1.pitch", 0.0) - CARRIAGE["tail_lift"]
        pose["ded_tail2.pitch"] = pose.get("ded_tail2.pitch", 0.0) + 0.35 * CARRIAGE["tail_lift"]
    if CARRIAGE["hang_swing"] != 1.0:
        for b in ("ded_hang1", "ded_hang2"):
            for ch in (".pitch", ".yaw", ".roll"):
                if b + ch in pose:
                    pose[b + ch] = pose[b + ch] * CARRIAGE["hang_swing"]
    return pose


def neutral_pose(rig, prof):
    """N: his normal carriage. Every one-shot starts from it and comes back to it."""
    b = prof["hunch"]
    n = {
        "zebra.pitch": -0.30 * b, "hrudnik.pitch": -0.35 * b, "krk.pitch": -0.45 * b, "hlava.pitch": 0.15 * b,
        "larm.swing": 3.0, "parm.swing": 2.0, "lloket.pitch": 6.0, "ploket.pitch": 5.0,
        "ded_jaw.pitch": -4.0,
        "root.z": prof["pelvis_back"] * rig.leg_len if b > 0 else 0.0,
    }
    # Tired knees: lower the pelvis until the longer-reaching leg is at knee_flex.
    lo, hi = 0.0, 0.3 * rig.leg_len
    def ratio(dy):
        p = dict(n)
        p["root.y"] = -dy
        trunk, _ = rig._trunk(arms(p))
        worst = 0.0
        for s in R.SIDES:
            leg = rig.legs[s]
            d = float(np.linalg.norm(leg["ankle"] - R.apply(trunk[s + "zadek"], leg["hip"])))
            worst = max(worst, d / (leg["l1"] + leg["l2"]))
        return worst
    if ratio(0.0) > prof["knee_flex"]:
        for _ in range(40):
            mid = 0.5 * (lo + hi)
            if ratio(mid) > prof["knee_flex"]:
                lo = mid
            else:
                hi = mid
        n["root.y"] = -hi
    return n


def tall_pose(rig, prof, n):
    """T: full height for the beam. Spine straight and a little past it, chest open, arms out and back."""
    b, tall, arch = prof["hunch"], prof["tall"], prof["tall_arch"]
    u = b + tall + arch
    t = dict(n)
    t.update({
        "zebra.pitch": n["zebra.pitch"] + 0.30 * u, "hrudnik.pitch": n["hrudnik.pitch"] + 0.35 * u,
        "krk.pitch": n["krk.pitch"] + 0.45 * (b + tall), "hlava.pitch": 6.0,
        "lrameno.yaw": 8.0, "prameno.yaw": -8.0,
        "larm.swing": prof["tall_arm_swing"], "parm.swing": prof["tall_arm_swing"],
        "larm.out": prof["tall_arm_out"], "parm.out": prof["tall_arm_out"],
        "lloket.pitch": 3.0, "ploket.pitch": 3.0, "lruka.pitch": -8.0, "pruka.pitch": -8.0,
        "ded_rib_l.yaw": 38.0, "ded_rib_r.yaw": -38.0, "ded_jaw.pitch": -9.0,
        "ded_tail1.pitch": 4.0, "ded_tail2.pitch": 4.0,
        "root.z": 0.25 * n.get("root.z", 0.0), "root.y": 0.0,
    })
    # Stand as tall as the legs allow, a hair short of locked.
    drop = rig.reach_drop(arms(t))
    t["root.y"] = -(drop + 0.004 * rig.leg_len) if drop > 0.0 else 0.0
    return t


def breath(w, amount=1.0):
    """Channels of one breath, w in -1..1 (positive = in)."""
    a = amount
    return {
        "hrudnik.pitch": 1.3 * a * w, "zebra.pitch": 0.6 * a * w, "krk.pitch": -0.5 * a * w,
        "hlava.pitch": -0.6 * a * w,
        "lrameno.roll": -1.5 * a * w, "prameno.roll": 1.5 * a * w,
        "ded_rib_l.yaw": 2.5 * a * w, "ded_rib_r.yaw": -2.5 * a * w,
        "root.y": 0.0012 * a * w,       # scaled by leg length by the caller
    }


# ===========================================================================
# Clips. Each returns a dict: move, file, secs, looped, step, keys, fn(p), events.
# fn(p) gives the pose at phase p in 0..1. An event is (label, phase, what):
# what names a rig point, or is a list of them.
# ===========================================================================
def clip_idle(rig, prof, n):
    leg = rig.leg_len
    def fn(p):
        br = math.sin(2.0 * TAU * p)                     # two breaths per loop
        sw = math.sin(TAU * p)                           # one slow weight shift
        lagged = math.sin(TAU * p) - 0.35 * math.sin(2.0 * TAU * p)   # the same, arriving late
        droop = 0.5 * (1.0 - math.cos(TAU * p))          # the head sinks and is hauled back up
        pose = add(n, breath(br))
        pose["root.y"] = n.get("root.y", 0.0) + 0.0012 * leg * br - 0.007 * leg * sw * sw
        pose = add(pose, {
            "root.x": 0.026 * leg * sw, "root.roll": -0.7 * sw, "hrudnik.roll": 1.1 * lagged,
            "root.yaw": 0.6 * sw, "hrudnik.yaw": 1.0 * lagged, "hlava.yaw": 1.8 * lagged,
            "krk.pitch": -1.2 * droop, "hlava.pitch": -1.8 * droop,
            "larm.swing": 1.3 * lagged, "parm.swing": -1.0 * lagged,
            "lruka.pitch": 1.5 * lagged, "pruka.pitch": -1.5 * lagged,
            "ded_tail1.yaw": 2.0 * sw, "ded_tail2.yaw": 3.0 * lagged, "ded_tail3.yaw": 4.0 * lagged,
            "ded_hang1.roll": 1.5 * lagged, "ded_hang2.pitch": 1.5 * lagged, "ded_jaw.pitch": -1.5 * droop,
        })
        return pose
    return {"move": "DedIdle", "file": "ded_idle", "secs": prof["idle_secs"], "looped": True, "keys": 48,
            "fn": fn, "events": [("breath_in", 0.125, "furnace"), ("breath_in", 0.625, "furnace")]}


def clip_dormant(rig, prof, n):
    leg = rig.leg_len
    d = add(n, {"zebra.pitch": -4.0, "hrudnik.pitch": -5.0, "krk.pitch": -12.0, "hlava.pitch": -9.0,
                "root.y": -0.02 * leg, "root.z": 0.015 * leg, "ded_jaw.pitch": -7.0,
                "lloket.pitch": -3.0, "ploket.pitch": -3.0, "larm.swing": -1.0, "parm.swing": -1.0,
                "ded_tail1.pitch": -6.0, "ded_tail2.pitch": -8.0})
    def fn(p):
        br = math.sin(2.0 * TAU * p)
        pose = add(d, breath(br, 0.28))
        pose["root.y"] = d["root.y"] + 0.0003 * leg * br
        return pose
    return {"move": "DedDormant", "file": "ded_dormant", "secs": prof["dormant_secs"], "looped": True,
            "keys": 24, "fn": fn, "events": [], "pose": d}


def _gait(p, land, duty):
    """('stance', 0..1) or ('swing', 0..1) for a foot that lands at phase ``land``."""
    q = (p - land) % 1.0
    if q < duty:
        return "stance", q / duty
    return "swing", (q - duty) / (1.0 - duty)


def _swing_profile(s, hover=0.82):
    """Swing 0..1 -> (travel 0..1, lift 0..1, drop 0..1, falling 0..1).

    The foot travels during the first ``hover`` share of the swing and is over
    its landing spot at the end of it, still ``drop`` high; then it falls
    straight down, speeding up, so it lands hard."""
    x = min(s / hover, 1.0)
    travel = smoother(x)
    lift = math.sin(math.pi * x) ** 2
    if s <= hover:
        return travel, lift, smooth(x), 0.0
    u = (s - hover) / (1.0 - hover)
    return 1.0, 0.0, 1.0 - u * u, u


def clip_walk(rig, prof, n):
    leg = rig.leg_len
    secs = prof["walk_secs"]
    stride = prof["stride"] * leg
    duty = prof["duty"]
    reach = 0.5 * stride * duty                    # how far ahead of the body a foot lands
    # Left foot is planted under the body at phase 0 and the right foot is at
    # the top of its swing: the pose nearest to standing, so a start from the
    # stand lifts the right foot and goes.
    land = {"l": (1.0 - 0.5 * duty) % 1.0, "p": (0.5 - 0.5 * duty) % 1.0}
    spacing = abs(float(rig.points["foot_l"][0] - rig.points["foot_p"][0]))
    lift, drop = prof["foot_lift"] * leg, prof["foot_drop"] * leg
    lag = prof["body_lag"] * leg
    def fn(p):
        pose = dict(n)
        pose["root.z"] = 0.45 * n.get("root.z", 0.0)
        for s in R.SIDES:
            kind, u = _gait(p, land[s], duty)
            k = s + "foot"
            if kind == "stance":
                fwd = reach - stride * duty * u
                pose[k + ".heel"] = prof["heel_off"] * smooth((u - 0.72) / 0.28)
            else:
                travel, up, hold, _fall = _swing_profile(u)
                # In the world the foot goes from its old spot to the new one;
                # the body advances under it meanwhile.
                fwd = -reach + stride * travel - stride * (1.0 - duty) * u
                pose[k + ".y"] = lift * up + drop * hold
                pose[k + ".heel"] = prof["heel_off"] * (1.0 - smooth(u / 0.4))
                if prof["toe_droop"]:
                    # the toes hang while the foot is in the air and come level before it lands
                    pose[k + ".toe"] = -prof["toe_droop"] * math.sin(math.pi * min(u / 0.8, 1.0)) ** 2
                pose[k + ".pitch"] = -9.0 * math.sin(math.pi * min(u / 0.55, 1.0)) ** 2 + 3.0 * up * smooth((u - 0.5) / 0.3)
            pose[k + ".z"] = -fwd
        # Weight over the standing foot; the swing side of the pelvis hangs.
        over_left = math.cos(TAU * (p - 0.03))
        pose["root.x"] = prof["sway"] * spacing * 0.5 * over_left
        pose["root.roll"] = -1.6 * over_left
        pose["hrudnik.roll"] = 2.4 * math.cos(TAU * (p - 0.07))
        pose["hlava.roll"] = -1.0 * math.cos(TAU * (p - 0.12))
        # The hip of the leading foot swings forward; the chest turns against it, late.
        lead = math.cos(TAU * (p - land["p"]))            # +1 at the right footfall
        pose["root.yaw"] = 4.0 * lead
        pose["zebra.yaw"] = -2.0 * math.cos(TAU * (p - land["p"] - 0.05))
        pose["hrudnik.yaw"] = -3.5 * math.cos(TAU * (p - land["p"] - 0.09))
        pose["hlava.yaw"] = 1.5 * math.cos(TAU * (p - land["p"] - 0.14))
        # Twice per cycle: the body hangs back as the foot goes out, then is hauled over it.
        two = 2.0 * TAU * (p - land["p"])
        pose["root.z"] += lag * math.cos(two - 0.5)
        pose["zebra.pitch"] = n["zebra.pitch"] - 0.9 * math.cos(two - 1.4)
        pose["hrudnik.pitch"] = n["hrudnik.pitch"] - 1.3 * math.cos(two - 1.7)
        pose["krk.pitch"] = n["krk.pitch"] - 1.2 * math.cos(two - 2.2)
        pose["hlava.pitch"] = n["hlava.pitch"] - 1.4 * math.cos(two - 2.6)
        bob = 0.008 * leg * math.cos(two - math.pi - 0.3)      # highest over the standing leg
        shock = 0.0
        for s in R.SIDES:
            t = ((p - land[s]) % 1.0) * secs                   # seconds since this foot landed
            shock += pulse(t, 0.24)
            pose["hrudnik.pitch"] -= 1.3 * pulse(t - 0.12, 0.27)
            pose["krk.pitch"] -= 1.6 * pulse(t - 0.24, 0.30)
            pose["hlava.pitch"] -= 2.2 * pulse(t - 0.34, 0.32)
            pose["ded_jaw.pitch"] = pose.get("ded_jaw.pitch", 0.0) - 2.5 * pulse(t - 0.40, 0.3)
            pose["lrameno.roll"] = pose.get("lrameno.roll", 0.0) + 0.8 * pulse(t - 0.2, 0.3)
            pose["prameno.roll"] = pose.get("prameno.roll", 0.0) - 0.8 * pulse(t - 0.2, 0.3)
        pose["root.y"] = n.get("root.y", 0.0) + bob - 0.017 * leg * shock
        # Arms: dead weight, swinging late against the legs.
        pose["larm.swing"] = 3.0 + 6.5 * math.cos(TAU * (p - land["p"] - 0.09))
        pose["parm.swing"] = 2.0 + 6.5 * math.cos(TAU * (p - land["l"] - 0.09))
        pose["lloket.pitch"] = 7.0 + 4.0 * math.cos(TAU * (p - land["p"] - 0.17))
        pose["ploket.pitch"] = 6.0 + 4.0 * math.cos(TAU * (p - land["l"] - 0.17))
        pose["lruka.pitch"] = 4.0 * math.cos(TAU * (p - land["p"] - 0.25))
        pose["pruka.pitch"] = 4.0 * math.cos(TAU * (p - land["l"] - 0.25))
        if prof["arm_back"]:
            # the arm hangs further back while the leg of its own side is forward (footfall of that side)
            pose["larm.swing"] -= prof["arm_back"] * max(0.0, math.cos(TAU * (p - land["l"] - 0.04))) ** 2
            pose["parm.swing"] -= prof["arm_back"] * max(0.0, math.cos(TAU * (p - land["p"] - 0.04))) ** 2
        if prof["tail_whip"]:
            for s in R.SIDES:
                t = ((p - land[s]) % 1.0) * secs
                sgn = 1.0 if s == "l" else -1.0
                pose["ded_tail1.pitch"] = pose.get("ded_tail1.pitch", 0.0) - 0.5 * prof["tail_whip"] * pulse(t - 0.15, 0.45)
                pose["ded_tail2.pitch"] = pose.get("ded_tail2.pitch", 0.0) - 0.8 * prof["tail_whip"] * pulse(t - 0.40, 0.50)
                pose["ded_tail3.pitch"] = pose.get("ded_tail3.pitch", 0.0) - 1.0 * prof["tail_whip"] * pulse(t - 0.65, 0.55)
                pose["ded_tail3.roll"] = pose.get("ded_tail3.roll", 0.0) + sgn * 0.6 * prof["tail_whip"] * pulse(t - 0.65, 0.55)
        pose["ded_tail1.yaw"] = -5.0 * math.cos(TAU * (p - land["p"] - 0.10))
        pose["ded_tail2.yaw"] = -7.0 * math.cos(TAU * (p - land["p"] - 0.20))
        pose["ded_tail3.yaw"] = -9.0 * math.cos(TAU * (p - land["p"] - 0.30))
        pose["ded_hang1.pitch"] = 3.0 * math.cos(two - 2.4)
        pose["ded_hang2.pitch"] = 3.0 * math.cos(two - 2.9)
        return pose
    events = [("foot_r_down", land["p"], "foot_p"), ("foot_l_down", land["l"], "foot_l"),
              ("foot_r_up", (land["p"] + duty) % 1.0, "foot_p"), ("foot_l_up", (land["l"] + duty) % 1.0, "foot_l")]
    return {"move": "DedWalk", "file": "ded_walk", "secs": secs, "looped": True, "keys": 50, "fn": fn,
            "step": (0.0, 0.0, -stride), "events": events, "gait": {"land": land, "duty": duty, "stride": stride}}


def clip_turn(rig, prof, n, direction):
    """Stepping round on the spot. direction +1 = left. The engine turns the
    body at turn_rate; each planted foot is turned back by the same angle, so
    it stays where it is on the ground."""
    leg = rig.leg_len
    secs = prof["turn_secs"]
    duty = prof["duty"]
    per_cycle = math.degrees(prof["turn_rate"]) * secs        # body turn in one cycle
    half = 0.5 * per_cycle * duty
    # The inside foot (the side he turns to) is planted at phase 0, the other is in the air.
    inside, outside = ("l", "p") if direction > 0 else ("p", "l")
    land = {inside: (1.0 - 0.5 * duty) % 1.0, outside: (0.5 - 0.5 * duty) % 1.0}
    spacing = abs(float(rig.points["foot_l"][0] - rig.points["foot_p"][0]))
    lift, drop = 0.07 * leg, 0.035 * leg
    def fn(p):
        pose = dict(n)
        for s in R.SIDES:
            kind, u = _gait(p, land[s], duty)
            k = s + "foot"
            if kind == "stance":
                ahead = half - per_cycle * duty * u
            else:
                travel, up, hold, _fall = _swing_profile(u)
                ahead = -half + per_cycle * travel - per_cycle * (1.0 - duty) * u
                pose[k + ".y"] = lift * up + drop * hold
                pose[k + ".pitch"] = -5.0 * math.sin(math.pi * min(u / 0.6, 1.0)) ** 2
            pose[k + ".orbit"] = direction * ahead
        over_inside = math.cos(TAU * (p - 0.03))
        side = 1.0 if inside == "l" else -1.0
        pose["root.x"] = side * prof["sway"] * spacing * 0.42 * over_inside
        pose["root.roll"] = -side * 1.2 * over_inside
        pose["hrudnik.roll"] = side * 1.8 * math.cos(TAU * (p - 0.08))
        # The head goes first, the chest follows, the arms trail.
        pose["hlava.yaw"] = direction * 7.0
        pose["krk.yaw"] = direction * 3.0
        pose["hrudnik.yaw"] = direction * 2.5
        pose["larm.swing"] = 3.0 + 2.0 * math.cos(TAU * (p - 0.2))
        pose["parm.swing"] = 2.0 - 2.0 * math.cos(TAU * (p - 0.2))
        pose["larm.out"] = 1.5
        pose["parm.out"] = 1.5
        shock = 0.0
        for s in R.SIDES:
            t = ((p - land[s]) % 1.0) * secs
            shock += pulse(t, 0.24)
            pose["hrudnik.pitch"] = pose.get("hrudnik.pitch", 0.0) - 0.8 * pulse(t - 0.12, 0.27)
            pose["hlava.pitch"] = pose.get("hlava.pitch", 0.0) - 1.4 * pulse(t - 0.3, 0.32)
        pose["root.y"] = n.get("root.y", 0.0) - 0.010 * leg * shock
        pose["ded_tail1.yaw"] = -direction * 6.0
        pose["ded_tail2.yaw"] = -direction * 9.0
        pose["ded_tail3.yaw"] = -direction * 9.0
        return pose
    name = "L" if direction > 0 else "R"
    en = {"l": "l", "p": "r"}
    events = [("foot_%s_down" % en[s], land[s], "foot_" + s) for s in (outside, inside)]
    return {"move": "DedTurn" + name, "file": "ded_turn" + name.lower(), "secs": secs, "looped": True, "keys": 50,
            "fn": fn, "events": events,
            "turn": {"direction": direction, "rate": prof["turn_rate"], "land": land, "duty": duty}}


def clip_roar(rig, prof, n):
    leg = rig.leg_len
    secs = prof["roar_secs"]
    u = prof["hunch"] + prof["tall"]
    gather = {"zebra.pitch": -5.0, "hrudnik.pitch": -7.0, "krk.pitch": -10.0, "hlava.pitch": -13.0,
              "root.y": -0.03 * leg, "root.z": 0.012 * leg, "lloket.pitch": 10.0, "ploket.pitch": 10.0,
              "larm.swing": -3.0, "parm.swing": -3.0, "ded_jaw.pitch": 3.0,
              "ded_rib_l.yaw": -3.0, "ded_rib_r.yaw": 3.0, "lrameno.yaw": -4.0, "prameno.yaw": 4.0}
    throw = {"zebra.pitch": 0.25 * u + 3.0, "hrudnik.pitch": 0.30 * u + 6.0, "krk.pitch": 0.35 * u + 12.0,
             "hlava.pitch": 14.0, "root.z": -0.06 * leg, "root.y": 0.012 * leg, "root.pitch": -4.0,
             "ded_jaw.pitch": -30.0, "ded_rib_l.yaw": 17.0, "ded_rib_r.yaw": -17.0,
             "lrameno.yaw": 7.0, "prameno.yaw": -7.0, "larm.out": 13.0, "parm.out": 13.0,
             "larm.swing": -9.0, "parm.swing": -9.0, "lloket.pitch": 9.0, "ploket.pitch": 9.0,
             "lruka.pitch": -12.0, "pruka.pitch": -12.0, "ded_tail1.pitch": 8.0, "ded_tail2.pitch": 8.0}
    sag = {"zebra.pitch": -2.0, "hrudnik.pitch": -3.0, "krk.pitch": -5.0, "hlava.pitch": -5.0, "root.y": -0.012 * leg}
    def fn(p):
        t = p * secs
        w_g = ramp(t, 0.2, 2.4) * (1.0 - ramp(t, 2.5, 4.0))
        w_t = ramp(t, 2.6, 4.5) * (1.0 - ramp(t, 7.0, 8.8))
        spent = ramp(t, 4.6, 7.1)                  # running out of breath while it lasts
        w_s = ramp(t, 6.8, 7.9) * (1.0 - ramp(t, 7.9, secs))
        pose = add(add(add(n, gather, w_g), throw, w_t * (1.0 - 0.22 * spent)), sag, w_s)
        shake = w_t * ramp(t, 3.6, 4.6)
        pose = add(pose, {"hlava.roll": 1.6 * math.sin(TAU * 2.3 * t), "hlava.yaw": 4.0 * math.sin(TAU * 0.31 * (t - 3.4)),
                          "hrudnik.roll": 0.5 * math.sin(TAU * 2.9 * t + 1.0), "ded_jaw.pitch": 1.5 * math.sin(TAU * 3.7 * t),
                          "lruka.pitch": 2.0 * math.sin(TAU * 2.6 * t), "pruka.pitch": 2.0 * math.sin(TAU * 2.6 * t + 2.0),
                          "krk.yaw": 2.0 * math.sin(TAU * 0.31 * (t - 3.4))}, shake)
        return pose
    return {"move": "DedRoar", "file": "ded_roar", "secs": secs, "looped": False, "keys": 90, "fn": fn,
            "events": [("head_down", 2.4 / secs, "mouth"), ("roar_start", 3.4 / secs, "mouth"),
                       ("roar_full", 4.5 / secs, "mouth"), ("roar_end", 7.1 / secs, "mouth")]}


_RISE_GROUPS = (                       # (start s, end s, channel prefixes) from the ground up
    (0.0, 3.0, ("root.",)),
    (0.5, 3.6, ("zebra.",)),
    (1.2, 4.6, ("hrudnik.", "ded_tail", "ded_hang")),
    (2.0, 5.3, ("krk.",)),
    (2.6, 5.9, ("hlava.", "ded_jaw")),
    (2.0, 5.6, ("larm.", "parm.", "lloket", "ploket", "lruka", "pruka", "lrameno", "prameno")),
    (4.3, 6.5, ("ded_rib",)),          # the ribs spread last
)


def clip_beam_rise(rig, prof, n, tall):
    secs = prof["rise_secs"]
    scale = secs / 6.5
    def fn(p):
        t = p * secs
        pose = dict(n)
        for k in set(n) | set(tall):
            w = None
            for t0, t1, prefixes in _RISE_GROUPS:
                if k.startswith(prefixes):
                    w = ramp(t, t0 * scale, t1 * scale)
                    break
            if w is None:
                w = ramp(t, 0.0, secs)
            pose[k] = n.get(k, 0.0) + w * (tall.get(k, 0.0) - n.get(k, 0.0))
        effort = ramp(t, 0.6 * scale, 3.0 * scale) * (1.0 - ramp(t, 5.2 * scale, secs))
        pose = add(pose, {"hrudnik.roll": 0.35 * math.sin(TAU * 1.9 * t), "hlava.roll": 0.6 * math.sin(TAU * 2.3 * t + 0.7),
                          "lruka.pitch": 1.5 * math.sin(TAU * 2.1 * t), "pruka.pitch": 1.5 * math.sin(TAU * 2.1 * t + 1.9)},
                   effort)
        return pose
    return {"move": "DedBeamRise", "file": "ded_beamrise", "secs": secs, "looped": False, "keys": 52, "fn": fn,
            "events": [("ribs_start", 4.3 * scale / secs, "furnace"), ("ribs_open", 1.0, "furnace")]}


def clip_beam_hold(rig, prof, tall):
    def fn(p):
        # Rigid, with the strain showing: whole numbers of cycles so the loop closes.
        return add(tall, {"hrudnik.pitch": 0.22 * math.sin(TAU * 3 * p), "hrudnik.roll": 0.15 * math.sin(TAU * 5 * p + 1.0),
                          "hlava.roll": 0.35 * math.sin(TAU * 5 * p), "hlava.pitch": 0.3 * math.sin(TAU * 3 * p + 2.0),
                          "lruka.pitch": 0.9 * math.sin(TAU * 4 * p), "pruka.pitch": 0.9 * math.sin(TAU * 4 * p + 2.4),
                          "lloket.pitch": 0.4 * math.sin(TAU * 4 * p + 0.5), "ploket.pitch": 0.4 * math.sin(TAU * 4 * p + 2.9),
                          "ded_rib_l.yaw": 0.8 * math.sin(TAU * 6 * p), "ded_rib_r.yaw": -0.8 * math.sin(TAU * 6 * p + 0.6),
                          "ded_jaw.pitch": 0.8 * math.sin(TAU * 5 * p + 1.3)})
    return {"move": "DedBeamHold", "file": "ded_beamhold", "secs": prof["hold_secs"], "looped": True, "keys": 36,
            "fn": fn, "events": [("furnace", 0.0, "furnace")]}


def clip_beam_lower(rig, prof, n, tall):
    leg = rig.leg_len
    secs = prof["lower_secs"]
    scale = secs / 7.5
    sag = {"zebra.pitch": -3.0, "hrudnik.pitch": -5.0, "krk.pitch": -9.0, "hlava.pitch": -7.0,
           "root.y": -0.03 * leg, "root.z": 0.01 * leg, "ded_jaw.pitch": -6.0, "lloket.pitch": -3.0, "ploket.pitch": -3.0}
    def fn(p):
        t = p * secs / scale
        pose = dict(n)
        for k in set(n) | set(tall):
            if k.startswith("ded_rib"):
                w = 1.0 - ramp(t, 0.0, 2.2)            # the ribs close first
            elif k.startswith(("hlava.", "krk.")):
                w = 1.0 - ramp(t, 0.9, 3.4)            # then the head goes
            elif k.startswith(("larm.", "parm.", "lrameno", "prameno", "lruka", "pruka", "lloket", "ploket")):
                w = 1.0 - ramp(t, 0.6, 3.8)
            else:
                w = 1.0 - ramp(t, 1.0, 4.2)
            pose[k] = n.get(k, 0.0) + w * (tall.get(k, 0.0) - n.get(k, 0.0))
        w_s = ramp(t, 1.6, 4.4) * (1.0 - ramp(t, 4.8, 7.4))
        return add(pose, sag, w_s)
    return {"move": "DedBeamLower", "file": "ded_beamlower", "secs": secs, "looped": False, "keys": 52, "fn": fn,
            "events": [("ribs_closed", 2.2 * scale / secs, "furnace"), ("lowest", 4.5 * scale / secs, "head_top")]}


def clip_stomp(rig, prof, n):
    leg = rig.leg_len
    secs = prof["stomp_secs"]
    scale = secs / 8.0
    s = prof["stomp_foot"]
    o = "l" if s == "p" else "p"
    side = 1.0 if s == "l" else -1.0                  # +1: the stomping foot is the left one
    spacing = abs(float(rig.points["foot_l"][0] - rig.points["foot_p"][0]))
    reach, high = prof["stomp_reach"] * leg, prof["stomp_height"] * leg
    k = s + "foot"
    t_top, t_hit = 4.5, 5.5
    def fn(p):
        t = p * secs / scale
        pose = dict(n)
        w_shift = ramp(t, 0.0, 2.3) * (1.0 - ramp(t, 6.4, 7.9))       # weight onto the other foot
        w_up = ramp(t, 2.0, 4.3)
        fall = min(max((t - t_top) / (t_hit - t_top), 0.0), 1.0)
        fall = fall * fall                                           # speeding up all the way down
        back = ramp(t, 6.3, 7.8)                                     # the foot is brought home
        lifted = math.sin(math.pi * back) ** 2                       # clear of the ground on the way
        pose["root.x"] = -side * 0.36 * spacing * w_shift
        pose["root.roll"] = side * 1.5 * w_shift
        pose["hrudnik.roll"] = -side * 3.0 * w_shift
        if t <= t_top:
            pose[k + ".y"] = high * w_up
            pose[k + ".z"] = -0.22 * reach * w_up
            pose[k + ".pitch"] = -14.0 * math.sin(math.pi * min(w_up, 1.0)) * (1.0 - w_up) + 6.0 * w_up
        elif t <= t_hit:
            pose[k + ".y"] = high * (1.0 - fall)
            pose[k + ".z"] = -reach * (0.22 + 0.78 * fall)
            pose[k + ".pitch"] = 6.0 * (1.0 - fall)
        else:
            pose[k + ".y"] = 0.05 * leg * lifted
            pose[k + ".z"] = -reach * (1.0 - back)
        # He leans back as the knee comes up, then throws his weight down after the foot.
        lean = w_up * (1.0 - ramp(t, t_top - 0.3, t_hit + 0.3))
        pose = add(pose, {"zebra.pitch": 2.5, "hrudnik.pitch": 3.5, "krk.pitch": 3.0, "root.z": 0.02 * leg,
                          "larm.out": 9.0, "parm.out": 9.0, "larm.swing": -5.0, "parm.swing": -5.0}, lean)
        after = t - t_hit
        hit = pulse(after, 0.22)
        drive = ramp(t, t_top + 0.1, t_hit + 0.5) * (1.0 - ramp(t, 6.3, 7.8))      # the body arrives after the foot
        pose = add(pose, {"zebra.pitch": -2.0, "hrudnik.pitch": -3.5, "krk.pitch": -3.0, "hlava.pitch": -2.0,
                          "root.z": -0.045 * leg, "root.y": -0.015 * leg, "larm.swing": 5.0, "parm.swing": 5.0}, drive)
        pose = add(pose, {"root.y": -0.022 * leg, "hrudnik.pitch": -1.5, "ded_jaw.pitch": -4.0}, hit)
        pose = add(pose, {"krk.pitch": -2.5, "hlava.pitch": -3.5, "lrameno.roll": 1.2, "prameno.roll": -1.2},
                   pulse(after - 0.2, 0.3))
        shudder = pulse(after - 0.1, 0.35)
        pose = add(pose, {"hrudnik.roll": 0.8 * math.sin(TAU * 2.4 * after), "hlava.roll": 1.2 * math.sin(TAU * 2.4 * after + 0.8)},
                   shudder)
        strain = w_up * (1.0 - ramp(t, t_top, t_hit))
        pose = add(pose, {"hrudnik.roll": 0.4 * math.sin(TAU * 2.2 * t), k + ".roll": 0.8 * math.sin(TAU * 2.2 * t)}, strain)
        return pose
    en = {"l": "l", "p": "r"}[s]
    return {"move": "DedStomp", "file": "ded_stomp", "secs": secs, "looped": False, "keys": 112, "fn": fn,
            "key_step_max": prof["key_step_max"],
            "events": [("foot_%s_up" % en, 2.0 * scale / secs, "foot_" + s), ("foot_top", t_top * scale / secs, "foot_" + s),
                       ("impact", t_hit * scale / secs, "foot_" + s), ("foot_%s_home" % en, 7.8 * scale / secs, "foot_" + s)],
            "other_foot": o}


def clip_swat(rig, prof, n):
    leg = rig.leg_len
    secs = prof["swat_secs"]
    scale = secs / 9.0
    s = prof["swat_arm"]
    o = "l" if s == "p" else "p"
    d = 1.0 if s == "p" else -1.0             # +1: right arm, the sweep goes from his right to his left
    up = prof["swat_up"]
    t0, t1 = 2.9, 5.9                         # the sweep itself
    def fn(p):
        t = p * secs / scale
        pose = dict(n)
        raised = ramp(t, 0.3, 2.7) * (1.0 - ramp(t, 6.3, 8.8))
        sweep = smooth((t - t0) / (t1 - t0))
        # Arm lifted out to its own side and above level; then carried across the front.
        pose[s + "arm.out"] = (90.0 + up) * raised
        pose[s + "biceps.yaw"] = d * (-30.0 * raised + 125.0 * sweep * (1.0 - ramp(t, 6.3, 8.8)))
        pose[s + "loket.pitch"] = n.get(s + "loket.pitch", 0.0) + 22.0 * raised * (1.0 - sweep) - 4.0 * sweep * raised
        pose[s + "ruka.pitch"] = -10.0 * raised
        pose[s + "rameno.roll"] = d * 6.0 * raised
        # The trunk winds back, then goes round with the arm, late.
        wind = ramp(t, 0.4, 2.8) * (1.0 - ramp(t, t0, t1))
        follow = ramp(t, t0 + 0.3, t1 + 0.4) * (1.0 - ramp(t, 6.5, 8.9))
        yaw = d * (-13.0 * wind + 17.0 * follow)
        pose = add(pose, {"root.yaw": 0.25 * yaw, "zebra.yaw": 0.3 * yaw, "hrudnik.yaw": 0.45 * yaw, "hlava.yaw": 0.35 * yaw})
        pose = add(pose, {"zebra.pitch": 2.0, "hrudnik.pitch": 3.0, "hrudnik.roll": d * 3.0, "root.x": d * 0.02 * leg,
                          o + "arm.out": 7.0, o + "arm.swing": -4.0}, wind)
        pose = add(pose, {"zebra.pitch": -2.5, "hrudnik.pitch": -3.5, "hrudnik.roll": -d * 4.0, "root.x": -d * 0.035 * leg,
                          "root.y": -0.012 * leg, "krk.pitch": -2.0, o + "arm.swing": 5.0, o + "arm.out": 4.0}, follow)
        spent = pulse(t - t1, 0.5)
        pose = add(pose, {"hlava.pitch": -3.0, "krk.pitch": -2.0, "root.y": -0.008 * leg}, spent)
        return pose
    hand = "hand_" + s
    scan = [("sweep_%02d" % i, (t0 + (t1 - t0) * i / 8.0) * scale / secs, hand) for i in range(9)]
    return {"move": "DedSwat", "file": "ded_swat", "secs": secs, "looped": False, "keys": 80, "fn": fn,
            "events": [("arm_raised", 2.7 * scale / secs, hand)] + scan + [("arm_down", 8.8 * scale / secs, hand)],
            "sweep": (t0 * scale, t1 * scale)}


def clip_flinch(rig, prof, n):
    leg = rig.leg_len
    secs = prof["flinch_secs"]
    def fn(p):
        t = p * secs
        hit = ramp(t, 0.0, 0.34) * (1.0 - ramp(t, 0.5, 1.7))         # taken, then let go of slowly
        ring = ramp(t, 0.15, 0.5) * math.exp(-max(t - 0.5, 0.0) / 0.55) * (1.0 - ramp(t, 1.9, secs))
        pose = add(n, {"zebra.pitch": 1.0, "hrudnik.pitch": 2.2, "hrudnik.yaw": 2.0, "hrudnik.roll": -1.2,
                       "krk.pitch": -2.5, "hlava.pitch": -3.5, "hlava.yaw": -2.0, "root.y": -0.008 * leg,
                       "lrameno.roll": -2.2, "prameno.roll": 2.2, "lloket.pitch": 3.0, "ploket.pitch": 3.0,
                       "ded_jaw.pitch": 3.0, "ded_rib_l.yaw": -2.0, "ded_rib_r.yaw": 2.0}, hit)
        w = math.sin(TAU * 2.2 * (t - 0.15))
        return add(pose, {"hrudnik.roll": 0.9 * w, "hrudnik.pitch": 0.5 * w, "hlava.roll": -1.4 * w, "hlava.pitch": 0.8 * w,
                          "lruka.pitch": 2.5 * w, "pruka.pitch": -2.5 * w, "larm.swing": 0.8 * w, "parm.swing": -0.8 * w,
                          "ded_tail2.yaw": 3.0 * w, "ded_hang1.roll": 3.0 * w, "ded_hang2.roll": -3.0 * w}, ring)
    return {"move": "DedFlinch", "file": "ded_flinch", "secs": secs, "looped": False, "keys": 48, "fn": fn,
            "events": [("deepest", 0.4 / secs, "head_top")]}


# ===========================================================================
# Building keys, checks, files
# ===========================================================================
def build_keys(rig, clip):
    """Sample the clip, keep both feet reachable, return [(time, {bone: 4x4})]."""
    n = clip["keys"]
    looped = clip["looped"]
    count = n if looped else n + 1
    phases = [i / n for i in range(count)]
    poses = [arms(clip["fn"](p)) for p in phases]
    want = [ps.get("root.y", 0.0) for ps in poses]
    margin = 0.004 * rig.leg_len
    limit = []
    for ps in poses:
        free = dict(ps)
        free["root.y"] = 0.0
        # The pelvis may be at most this far up: where the legs just reach, less a hair,
        # measured from the rest height (or from where the pose wants it, if that is higher).
        top = max(ps.get("root.y", 0.0), 0.0)
        free["root.y"] = top
        drop = rig.reach_drop(free)
        limit.append(top - (drop + margin) if drop > 0.0 else top)
    y = [min(w, l) for w, l in zip(want, limit)]
    lowered = max(w - v for w, v in zip(want, y))
    if lowered > 1e-6:
        # Round the corners where the legs ran out, without rising above the limit.
        for _ in range(3):
            if looped:
                sm = [0.25 * y[i - 1] + 0.5 * y[i] + 0.25 * y[(i + 1) % count] for i in range(count)]
            else:
                sm = [y[0]] + [0.25 * y[i - 1] + 0.5 * y[i] + 0.25 * y[i + 1] for i in range(1, count - 1)] + [y[-1]]
            y = [min(a, b, c) for a, b, c in zip(sm, want, limit)]
    keys = []
    worst = -1e9
    for p, ps, yy in zip(phases, poses, y):
        ps["root.y"] = yy
        mats, stretch = rig.evaluate(ps)
        worst = max(worst, stretch)
        keys.append((p, mats))
    added = 0
    step_max = clip.get("key_step_max")
    if step_max and not looped:
        # The engine blends two keys element by element, so a bone that turns far between two keys
        # shrinks on the way. Where a bone turns more than step_max, put a key halfway (key times need
        # not be even: the engine reads each key's own time). The pelvis height of a new key is the
        # pose's own, held under the leg limit and never above the two keys it sits between when
        # those were lowered for reach.
        def turn(a, b):
            return max(R.rot_angle_deg(a[bn][:3, :3].T @ b[bn][:3, :3]) for bn in R.BONES)
        row = [(p, m, yy, min(w, l)) for (p, m), yy, w, l in zip(keys, y, want, limit)]
        for _ in range(4):
            out, grew = [row[0]], False
            for a, b in zip(row[:-1], row[1:]):
                if turn(a[1], b[1]) > step_max:
                    pm = 0.5 * (a[0] + b[0])
                    ps = arms(clip["fn"](pm))
                    free = dict(ps)
                    top = max(ps.get("root.y", 0.0), 0.0)
                    free["root.y"] = top
                    drop = rig.reach_drop(free)
                    lim = top - (drop + margin) if drop > 0.0 else top
                    ym = min(ps.get("root.y", 0.0), lim)
                    if a[2] < a[3] - 1e-9 or b[2] < b[3] - 1e-9:
                        ym = min(ym, 0.5 * (a[2] + b[2]))
                    ps["root.y"] = ym
                    mats, stretch = rig.evaluate(ps)
                    worst = max(worst, stretch)
                    out.append((pm, mats, ym, ym))
                    grew = True
                    added += 1
                out.append(b)
            row = out
            if not grew:
                break
        keys = [(r[0], r[1]) for r in row]
    if looped:
        keys.append((1.0, keys[0][1]))
    return keys, {"pelvis_lowered_m": lowered, "leg_stretch_m": worst, "keys_added": added, "keys_written": len(keys)}


def sample(keys, p, looped):
    """Matrices at phase p the way the engine makes them: a straight blend of the two keys around p."""
    if looped:
        p = p % 1.0
    p = max(0.0, min(1.0, p))
    for i in range(len(keys) - 1):
        t0, t1 = keys[i][0], keys[i + 1][0]
        if t0 <= p <= t1:
            w = (p - t0) / (t1 - t0) if t1 > t0 else 0.0
            return {b: (1.0 - w) * keys[i][1][b] + w * keys[i + 1][1][b] for b in keys[i][1]}
    return keys[-1][1]


def check_clip(rig, clip, keys, n_pose_mats):
    """Offline numbers for the report."""
    secs = clip["secs"]
    out = {}
    # Largest turn of any bone between two keys: the engine blends matrices
    # straight, so a big step shrinks the bone on the way.
    step_deg, spin = 0.0, 0.0
    for i in range(len(keys) - 1):
        dt = (keys[i + 1][0] - keys[i][0]) * secs
        for b in R.BONES:
            a = R.rot_angle_deg(keys[i][1][b][:3, :3].T @ keys[i + 1][1][b][:3, :3])
            step_deg = max(step_deg, a)
            if dt > 0:
                spin = max(spin, a / dt)
    out["max_key_step_deg"] = step_deg
    out["max_bone_turn_deg_s"] = spin
    fastest = {}
    for name in ("head_top", "hand_l", "hand_p", "foot_l", "foot_p"):
        v = 0.0
        for i in range(len(keys) - 1):
            dt = (keys[i + 1][0] - keys[i][0]) * secs
            if dt > 0:
                v = max(v, float(np.linalg.norm(rig.point(keys[i + 1][1], name) - rig.point(keys[i][1], name))) / dt)
        fastest[name] = v
    out["max_speed_m_s"] = fastest
    # Soles never go through the ground.
    low = 0.0
    for _, mats in keys:
        for s in R.SIDES:
            leg = rig.legs[s]
            on = {"foot": s + "chodidlo", "toe": s + "prsty"}
            low = min(low, float(R.apply(mats[on[leg["contact_on"]]], leg["contact"])[1]),
                      float(R.apply(mats[on[leg["heel_on"]]], leg["heel_ground"])[1]),
                      float(R.apply(mats[s + "prsty"], leg["toe_ground"])[1]))
    out["lowest_sole_m"] = low
    # One-shots start from the normal carriage (or the tall pose) and end on it.
    if not clip["looped"]:
        def gap(mats, ref):
            return max(float(np.linalg.norm(R.apply(mats[b], rig.piv[b]) - R.apply(ref[b], rig.piv[b]))) for b in R.BONES)
        out["start_gap_m"] = gap(keys[0][1], clip.get("start_ref", n_pose_mats))
        out["end_gap_m"] = gap(keys[-1][1], clip.get("end_ref", n_pose_mats))
    # Planted feet: where is each one on the GROUND while it is down?
    slide = None
    if "gait" in clip or "turn" in clip:
        g = clip.get("gait") or clip["turn"]
        slide = 0.0
        for s in R.SIDES:
            leg = rig.legs[s]
            toe_ground = leg["toe_ground"]
            seen = []
            for j in range(401):
                u = 0.01 + 0.98 * j / 400.0
                p = (g["land"][s] + u * g["duty"]) % 1.0
                mats = sample(keys, p, True)
                pos = R.apply(mats[s + "prsty"], toe_ground)
                if "gait" in clip:
                    # unwrap: the body has advanced stride * (phase since landing)
                    world = pos + np.array([0.0, 0.0, -g["stride"] * u * g["duty"]])
                else:
                    ang = math.degrees(g["rate"]) * clip["secs"] * u * g["duty"] * g["direction"]
                    world = R.ry(ang) @ pos
                seen.append(world)
            seen = np.array(seen)
            slide = max(slide, float(np.max(np.linalg.norm(seen - seen[0], axis=1))))
        out["planted_slide_m"] = slide
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rest", required=True, help="the model's joint file (ud-rtm-rest/1)")
    ap.add_argument("--out", required=True)
    ap.add_argument("--depth", type=float, default=0.0, help="how far the body reaches forward of the origin (m), for the fall")
    args = ap.parse_args()
    os.makedirs(args.out, exist_ok=True)

    rig = R.load_rig(args.rest)
    prof = default_profile(rig)
    for k, v in (rig.rest.get("anim") or {}).items():
        if k not in prof:
            raise SystemExit("joint file: unknown anim setting %r" % k)
        prof[k] = v
    rig.hand_clearance = prof["hand_clearance"]
    for k in CARRIAGE:
        CARRIAGE[k] = float(prof[k])
    depth = args.depth or 0.15 * rig.height

    n = neutral_pose(rig, prof)
    tall = tall_pose(rig, prof, n)
    n_mats, n_stretch = rig.evaluate(arms(n))
    t_mats, t_stretch = rig.evaluate(arms(tall))
    if n_stretch > 1e-6 or t_stretch > 1e-6:
        raise SystemExit("the normal or the tall pose cannot be reached by the legs (%.3f, %.3f m)" % (n_stretch, t_stretch))

    clips = [
        clip_idle(rig, prof, n), clip_walk(rig, prof, n), clip_turn(rig, prof, n, +1), clip_turn(rig, prof, n, -1),
        clip_roar(rig, prof, n), clip_beam_rise(rig, prof, n, tall), clip_beam_hold(rig, prof, tall),
        clip_beam_lower(rig, prof, n, tall), clip_stomp(rig, prof, n), clip_swat(rig, prof, n),
        clip_flinch(rig, prof, n), clip_dormant(rig, prof, n),
    ]
    by_move = {c["move"]: c for c in clips}
    by_move["DedBeamRise"]["end_ref"] = t_mats
    by_move["DedBeamLower"]["start_ref"] = t_mats

    report = ["Dedushka animation set, offline checks", "joint file: %s" % os.path.abspath(args.rest),
              "height %.1f m, leg %.2f m, rest spine lean %.1f deg" % (rig.height, rig.leg_len, rig.spine_lean_deg()),
              "optional bones with a joint: %s" % (", ".join(b for b in R.EXTRA_BONES if rig.has[b]) or "none"),
              "settings: " + json.dumps(prof, sort_keys=True), ""]
    timing = {"format": "ded-anim-timing/1", "rest": os.path.abspath(args.rest), "height_m": rig.height,
              "leg_m": rig.leg_len, "frame": "[right, forward, up] metres from the unit's position, unit facing forward",
              "turn_rate_rad_s": prof["turn_rate"], "clips": []}
    failures = []

    def write(file_stem, frames, step, looped):
        path = os.path.join(args.out, file_stem + ".rtm")
        anim = rtm_io.new_animation(R.BONES, [(t, [R.to12(m[b]) for b in R.BONES]) for t, m in frames], step)
        rtm_io.write_rtm(anim, path, verify=True, require_bones=R.BONES, looped=looped)
        back = rtm_io.read_rtm(path, strict=True)
        return rtm_io.describe(back)

    for clip in clips:
        keys, fit = build_keys(rig, clip)
        step = clip.get("step", (0.0, 0.0, 0.0))
        desc = write(clip["file"], keys, step, clip["looped"])
        chk = check_clip(rig, clip, keys, n_mats)
        chk.update(fit)
        events = []
        for label, phase, what in clip["events"]:
            mats = sample(keys, phase, clip["looped"])
            events.append({"name": label, "phase": round(phase % 1.0 if clip["looped"] else phase, 4),
                           "t": round((phase % 1.0 if clip["looped"] else phase) * clip["secs"], 3),
                           "key": round((phase % 1.0 if clip["looped"] else phase) * clip["keys"], 2),
                           "pos": R.script_xyz(rig.point(mats, what))})
        events.sort(key=lambda e: e["t"])
        entry = {"move": clip["move"], "file": "\\%s\\anim\\%s.rtm" % (ADDON, clip["file"]), "secs": clip["secs"],
                 "looped": clip["looped"], "keys": clip["keys"], "step_m": round(-step[2], 4), "events": events}
        if "gait" in clip:
            entry["speed_m_s"] = round(-step[2] / clip["secs"], 4)
            entry["slowest"] = prof["walk_slowest"]
        timing["clips"].append(entry)
        report.append("%s  (%s, %.1f s, %s)" % (clip["move"], desc, clip["secs"], "loop" if clip["looped"] else "one shot"))
        if chk.get("keys_added"):
            report.append("    %d keys added where a bone turned more than %.1f deg between two keys (%d keys written, uneven times)"
                          % (chk["keys_added"], clip["key_step_max"], chk["keys_written"]))
        report.append("    largest key step %.1f deg, fastest bone %.0f deg/s, pelvis lowered for reach %.3f m, "
                      "leg stretch %.4f m, lowest sole %.3f m" % (chk["max_key_step_deg"], chk["max_bone_turn_deg_s"],
                                                                   chk["pelvis_lowered_m"], chk["leg_stretch_m"], chk["lowest_sole_m"]))
        report.append("    fastest points, m/s: " + ", ".join("%s %.1f" % kv for kv in chk["max_speed_m_s"].items()))
        if "planted_slide_m" in chk:
            report.append("    planted foot slide over a stance: %.4f m" % chk["planted_slide_m"])
            if chk["planted_slide_m"] > 0.02:
                failures.append("%s: a planted foot slides %.3f m" % (clip["move"], chk["planted_slide_m"]))
        if "start_gap_m" in chk:
            report.append("    start is %.3f m from its reference pose, end %.3f m" % (chk["start_gap_m"], chk["end_gap_m"]))
            if chk["start_gap_m"] > 0.05 or chk["end_gap_m"] > 0.05:
                failures.append("%s: does not start or end on its reference pose" % clip["move"])
        if chk["leg_stretch_m"] > 1e-4:
            failures.append("%s: a foot is out of reach by %.3f m" % (clip["move"], chk["leg_stretch_m"]))
        if chk["lowest_sole_m"] < -0.02:
            failures.append("%s: a sole goes %.3f m under the ground" % (clip["move"], -chk["lowest_sole_m"]))
        if chk["max_key_step_deg"] > 15.0:
            failures.append("%s: %.1f degrees between two keys (add keys)" % (clip["move"], chk["max_key_step_deg"]))

    # The fall: the normal carriage topples forward about the toes and lands face down.
    die = []
    if not prof["fall_crumple"]:
        for i in range(13):
            t = i / 12.0
            ease = t * t
            fall = R.m4(R.rx(-90.0 * ease), np.array([0.0, depth * ease, 0.0]))
            die.append((t, {b: fall @ n_mats[b] for b in R.BONES}))
    else:
        # Finishing round (the engine agent's finding: the dead body of A hung 11 m up, because the
        # rigid fall lifts him by his whole forward reach, snout and claws included). He now falls on
        # his CHEST: the lift is fall_depth (the forward depth of the trunk), and what reaches further
        # forward gives way as he goes down: the head is thrown back and turned, the jaw drops, the
        # arms are flung out and back, hanging parts swing back. Each is bent until its point (mouth,
        # crown, hands, claw tips) is off the ground in the lying pose.
        lie = float(prof["fall_depth"] or depth)
        dead = dict(n)
        dead.update({"ded_jaw.pitch": -22.0, "hlava.yaw": 24.0, "hlava.roll": 10.0, "larm.out": 26.0, "parm.out": 34.0,
                     "lloket.pitch": 14.0, "ploket.pitch": 8.0, "ded_hang1.pitch": -30.0, "ded_hang2.pitch": -20.0,
                     "ded_rib_l.yaw": 10.0, "ded_rib_r.yaw": -14.0, "hrudnik.roll": 3.0, "zebra.pitch": n.get("zebra.pitch", 0.0) + 4.0})
        land = R.m4(R.rx(-90.0), np.array([0.0, lie, 0.0]))

        def lying_height(pose, names):
            mats, _st = rig.evaluate(arms(pose))
            low = 9e9
            for nm in names:
                owner = mats["hlava"] if nm in ("mouth", "head_top") else mats[nm[-1] + "ruka"]
                p = rig.points.get(nm)
                if p is None:
                    continue
                low = min(low, float(R.apply(land @ owner, p)[1]))
            return low
        for _ in range(45):                                   # the head: back, a neck first, until mouth and crown clear
            if lying_height(dead, ("mouth", "head_top")) >= 0.3:
                break
            dead["krk.pitch"] = dead.get("krk.pitch", 0.0) + 1.5
            dead["hlava.pitch"] = dead.get("hlava.pitch", 0.0) + 2.5
        for s_ in R.SIDES:                                    # each arm: back, until hand and claws clear
            for _ in range(45):
                if lying_height(dead, ("hand_" + s_, "claw_" + s_)) >= 0.3:
                    break
                dead[s_ + "arm.swing"] = dead.get(s_ + "arm.swing", 0.0) - 2.0
                dead[s_ + "loket.pitch"] = dead.get(s_ + "loket.pitch", 0.0) - 0.6
        for i in range(13):
            t = i / 12.0
            ease = t * t
            give = smooth(min(1.0, t / 0.8))                  # he gives way a little ahead of the ground
            pose = mix(n, dead, give)
            mats, _st = rig.evaluate(arms(pose))
            fall = R.m4(R.rx(-90.0 * ease), np.array([0.0, lie * ease, 0.0]))
            die.append((t, {b: fall @ mats[b] for b in R.BONES}))
        report.append("the fall: on his chest, lifted %.2f m (the trunk's forward depth; his whole reach is %.2f m); lying, mouth and crown "
                      "are %.2f m up, the lower hand %.2f m, the furnace point %.2f m"
                      % (lie, depth, lying_height(dead, ("mouth", "head_top")),
                         min(lying_height(dead, ("hand_l", "claw_l")), lying_height(dead, ("hand_p", "claw_p"))),
                         float(R.apply(land @ rig.evaluate(arms(dead))[0]["hrudnik"], rig.points["furnace"])[1])))
    report.append("DedDying  (%s, %.1f s, one shot)" % (write("ded_die", die, (0, 0, 0), False), prof["die_secs"]))
    report.append("DedDead  (%s)" % write("ded_dead", [(0.0, die[-1][1])], (0, 0, 0), None))
    timing["clips"].append({"move": "DedDying", "file": "\\%s\\anim\\ded_die.rtm" % ADDON, "secs": prof["die_secs"],
                            "looped": False, "keys": 12, "step_m": 0.0, "events": []})

    # Poses other features aim with.
    timing["poses"] = {
        "normal": {"furnace": R.script_xyz(rig.point(n_mats, "furnace")), "head_top": R.script_xyz(rig.point(n_mats, "head_top")),
                   "mouth": R.script_xyz(rig.point(n_mats, "mouth"))},
        "tall": {"furnace": R.script_xyz(rig.point(t_mats, "furnace")), "head_top": R.script_xyz(rig.point(t_mats, "head_top")),
                 "mouth": R.script_xyz(rig.point(t_mats, "mouth"))},
    }
    report.append("")
    report.append("normal carriage: top of head %.1f m, furnace %s" % (timing["poses"]["normal"]["head_top"][2],
                                                                      timing["poses"]["normal"]["furnace"]))
    report.append("tall (beam):     top of head %.1f m, furnace %s" % (timing["poses"]["tall"]["head_top"][2],
                                                                      timing["poses"]["tall"]["furnace"]))
    walk = next(c for c in timing["clips"] if c["move"] == "DedWalk")
    report.append("walk: %.2f m per cycle in %.1f s = %.2f m/s (%.1f km/h); slowest %.2f m/s" % (
        walk["step_m"], walk["secs"], walk["speed_m_s"], walk["speed_m_s"] * 3.6, walk["speed_m_s"] * prof["walk_slowest"]))
    total = sum((c["keys"] + 1) for c in timing["clips"]) * len(R.BONES)
    report.append("matrices in all clips: %d (the engine's RTM cache holds 32000)" % total)
    report.append("")
    report.append("FAILED: " + "; ".join(failures) if failures else "all offline checks passed")

    with open(os.path.join(args.out, "ded_anim_report.txt"), "w", encoding="utf-8") as fh:
        fh.write("\n".join(report) + "\n")
    with open(os.path.join(args.out, "ded_anim_timing.json"), "w", encoding="utf-8") as fh:
        json.dump(timing, fh, indent=1)
    write_hpp(os.path.join(args.out, "ded_anim_gen.hpp"), timing, prof)
    write_sqf(os.path.join(args.out, "ded_anim_timing.sqf"), timing, prof)
    write_md(os.path.join(args.out, "FOOTFALLS.md"), timing, prof)
    print("\n".join(report))
    if failures:
        raise SystemExit(1)


def _secs(timing, move):
    return next(c for c in timing["clips"] if c["move"] == move)["secs"]


def write_hpp(path, timing, prof):
    """The numbers moves_dedushka.hpp takes from the clips. Negative speed = seconds per cycle."""
    names = {"DedIdle": "IDLE", "DedWalk": "WALK", "DedTurnL": "TURN", "DedRoar": "ROAR", "DedBeamRise": "RISE",
             "DedBeamHold": "HOLD", "DedBeamLower": "LOWER", "DedStomp": "STOMP", "DedSwat": "SWAT",
             "DedFlinch": "FLINCH", "DedDormant": "DORMANT", "DedDying": "DIE"}
    walk = next(c for c in timing["clips"] if c["move"] == "DedWalk")
    lines = ["// Written by gen_ded_anim.py from the model's joint file. Do not edit: rerun the generator.",
             "// Seconds per cycle as the engine wants them (negative speed)."]
    for move, tag in names.items():
        lines.append("#define DED_SPD_%-8s -%s" % (tag, ("%.3f" % _secs(timing, move)).rstrip("0").rstrip(".")))
    lines += ["// Turn rate caps, rad/s: stepping round on the spot (the turn clips are built for",
              "// exactly this rate), walking, and standing.",
              "#define DED_TURN_SPOT    %.3f" % prof["turn_rate"],
              "#define DED_TURN_WALK    %.3f" % prof["walk_turn_rate"],
              "#define DED_TURN_IDLE    %.3f" % prof["idle_turn_rate"],
              "// The walk: %.2f m per cycle, %.2f m/s at full rate." % (walk["step_m"], walk["speed_m_s"]),
              "#define DED_WALK_SLOWEST %.2f" % prof["walk_slowest"],
              "// Blend into the walk and the turns: 1 / seconds.",
              "#define DED_BLEND_LOOP   %.3f" % (1.0 / prof["walk_blend"]),
              "// Footfalls of the walk as phases, for the engine's own step sound edges.",
              "#define DED_WALK_EDGE1   %.3f" % next(e["phase"] for e in walk["events"] if e["name"] == "foot_l_down"),
              "#define DED_WALK_EDGE2   %.3f" % next(e["phase"] for e in walk["events"] if e["name"] == "foot_r_down"),
              ""]
    with open(path, "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines))


def write_sqf(path, timing, prof):
    """For mission scripts: call compile-free, just `call loadFile "kj\\anim\\ded_anim_timing.sqf"`."""
    def arr(v):
        if isinstance(v, (list, tuple)):
            return "[" + ", ".join(arr(x) for x in v) + "]"
        if isinstance(v, str):
            return '"%s"' % v
        if isinstance(v, bool):
            return "true" if v else "false"
        if isinstance(v, float):
            text = ("%.4f" % v).rstrip("0").rstrip(".")
            return "0" if text in ("-0", "") else text
        return str(v)
    lines = ["// Dedushka animation timing, written by gen_ded_anim.py. Do not edit: rerun the generator.",
             "// Positions are [right, forward, up] in metres from the unit's position, unit facing forward.",
             "// KJ_animClips: [move, seconds, looped, metres per cycle, [[event, seconds, [right, forward, up]], ...]]",
             "KJ_animClips = ["]
    rows = []
    for c in timing["clips"]:
        ev = [[e["name"], e["t"], e["pos"]] for e in c["events"]]
        rows.append("  " + arr([c["move"], float(c["secs"]), c["looped"], float(c["step_m"]), ev]))
    lines.append(",\n".join(rows))
    lines.append("];")
    walk = next(c for c in timing["clips"] if c["move"] == "DedWalk")
    def ev(move, name):
        c = next(c for c in timing["clips"] if c["move"] == move)
        return next(e for e in c["events"] if e["name"] == name)
    stomp = ev("DedStomp", "impact")
    lines += [
        "KJ_animWalkStep = %s;" % arr(float(walk["step_m"])),
        "KJ_animWalkSecs = %s;" % arr(float(walk["secs"])),
        "KJ_animWalkSpeed = %s;" % arr(float(walk["speed_m_s"])),
        "KJ_animWalkSlowest = %s;" % arr(float(prof["walk_slowest"])),
        "KJ_animWalkBlend = %s;" % arr(float(prof["walk_blend"])),
        "// Footfalls of the walk: [phase 0..1, \"L\" or \"R\", [right, forward, up] where the sole lands].",
        "KJ_animWalkFalls = %s;" % arr(sorted([[e["phase"], "L" if e["name"] == "foot_l_down" else "R", e["pos"]]
                                             for e in walk["events"] if e["name"].endswith("_down")])),
        "KJ_animTurnSecs = %s;" % arr(float(_secs(timing, "DedTurnL"))),
        "KJ_animTurnRate = %s;" % arr(float(prof["turn_rate"])),
        "KJ_animTurnFallsL = %s;" % arr(sorted([[e["phase"], "L" if "_l_" in e["name"] else "R", e["pos"]]
                                              for e in next(c for c in timing["clips"] if c["move"] == "DedTurnL")["events"]])),
        "KJ_animTurnFallsR = %s;" % arr(sorted([[e["phase"], "L" if "_l_" in e["name"] else "R", e["pos"]]
                                              for e in next(c for c in timing["clips"] if c["move"] == "DedTurnR")["events"]])),
        "// The stomp: seconds from the start of DedStomp to the impact, and where the sole comes down.",
        "KJ_animStompTime = %s;" % arr(float(stomp["t"])),
        "KJ_animStompPos = %s;" % arr(stomp["pos"]),
        "KJ_animStompFoot = %s;" % arr("L" if prof["stomp_foot"] == "l" else "R"),
        "KJ_animStompSecs = %s;" % arr(float(_secs(timing, "DedStomp"))),
        "KJ_animSwatSecs = %s;" % arr(float(_secs(timing, "DedSwat"))),
        "KJ_animRoarSecs = %s;" % arr(float(_secs(timing, "DedRoar"))),
        "KJ_animFlinchSecs = %s;" % arr(float(_secs(timing, "DedFlinch"))),
        "// The swat: [seconds, hand position] samples across the sweep.",
        "KJ_animSwatPath = %s;" % arr([[e["t"], e["pos"]] for e in next(c for c in timing["clips"] if c["move"] == "DedSwat")["events"]
                                      if e["name"].startswith("sweep_")]),
        "// The roar: seconds from the start of DedRoar to the mouth opening and to the end of the cry.",
        "KJ_animRoarStart = %s;" % arr(float(ev("DedRoar", "roar_start")["t"])),
        "KJ_animRoarEnd = %s;" % arr(float(ev("DedRoar", "roar_end")["t"])),
        "KJ_animRoarMouth = %s;" % arr(ev("DedRoar", "roar_full")["pos"]),
        "// The beam: seconds DedBeamRise takes, and the furnace point while he holds.",
        "KJ_animBeamRiseSecs = %s;" % arr(float(_secs(timing, "DedBeamRise"))),
        "KJ_animBeamHoldSecs = %s;" % arr(float(_secs(timing, "DedBeamHold"))),
        "KJ_animBeamLowerSecs = %s;" % arr(float(_secs(timing, "DedBeamLower"))),
        "KJ_animBeamFurnace = %s;" % arr(timing["poses"]["tall"]["furnace"]),
        "KJ_animFurnace = %s;" % arr(timing["poses"]["normal"]["furnace"]),
        "KJ_animHeightTall = %s;" % arr(float(timing["poses"]["tall"]["head_top"][2])),
        "KJ_animHeight = %s;" % arr(float(timing["poses"]["normal"]["head_top"][2])),
        "KJ_animLoaded = true;", ""]
    with open(path, "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines))


def write_md(path, timing, prof):
    lines = ["# Dedushka clips: footfalls and events", "",
             "Written by `gen_ded_anim.py` from `%s`. Rerun the generator, do not edit." % timing["rest"], "",
             "Times are seconds from the start of the clip at full rate. Key is the keyframe number (the clip has",
             "that many keys per cycle; fractions fall between two keys). Positions are metres from the unit's",
             "position with the unit facing forward: right, forward, up.", "",
             "| Move | Seconds | Loop | Metres per cycle | Event | Time s | Phase | Key | Right | Forward | Up |",
             "|---|---|---|---|---|---|---|---|---|---|---|"]
    for c in timing["clips"]:
        head = "| %s | %s | %s | %s " % (c["move"], c["secs"], "yes" if c["looped"] else "no", c["step_m"] or "")
        if not c["events"]:
            lines.append(head + "| | | | | | | |")
        for e in c["events"]:
            lines.append(head + "| %s | %.2f | %.3f | %.1f of %d | %.1f | %.1f | %.1f |" % (
                e["name"], e["t"], e["phase"], e["key"], c["keys"], e["pos"][0], e["pos"][1], e["pos"][2]))
            head = "| | | | "
    walk = next(c for c in timing["clips"] if c["move"] == "DedWalk")
    lines += ["", "Walk: %.2f m per cycle, %.2f m/s at full rate, %.2f m/s at its slowest (the AI slows him near the end of a path)."
              % (walk["step_m"], walk["speed_m_s"], walk["speed_m_s"] * prof["walk_slowest"]),
              "While he walks the phase is distance walked divided by metres per cycle, so a footfall comes every %.2f m."
              % (walk["step_m"] / 2.0), ""]
    with open(path, "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines))


if __name__ == "__main__":
    main()
