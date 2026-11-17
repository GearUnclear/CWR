"""Makes the RTM clips CfgMovesDedushka names, from nothing (plain Python, no bpy).

  python gen_dedushka_rtm.py --out <dir> [--rest <dedushka_rest.json>] [--height 40]

Clips written to <dir>:
  ded_stand.rtm   looped idle: the rest pose with a slow breath and arm sway
  ded_walk.rtm    looped slow walk, feet planted (step read back and checked)
  ded_die.rtm     one shot: topples forward
  ded_dead.rtm    one key: the last frame of ded_die

Why own clips and not scaled stock clips: an RTM matrix is absolute, it maps
the model's rest file space to the posed space. A stock clip only fits a mesh
whose joints sit at the stock man's joints (times the scale) in the stock
rest space (facing +X, origin at hip height). Dedushka's rest space is the
standing pose itself (origin on the ground between the feet, facing -Z in
file space), so the identity matrix is a correct standing pose and every clip
is a small forward-kinematics chain about the model's OWN joints, which come
from a rest JSON (format ud-rtm-rest/1, the one the Blender RTM rig reads).

Without --rest (a statue: every vertex on one bone) the clips are rigid:
stand is the identity, walk glides, die topples the whole body.

Nothing here reads a stock file, so the output is not Bohemia-derived.
File space: X = the creature's left, Y up, Z = back (forward is -Z).
"""
import argparse
import json
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ADDON_PKG = os.path.normpath(os.path.join(HERE, "..", "..", "..", "apps", "tools", "BlenderAddon", "io_import_p3d"))
sys.path.insert(0, ADDON_PKG)
import rtm_io  # noqa: E402

BONES = rtm_io.STOCK_MAN_BONES
IDENT = rtm_io.IDENTITY

# Chains from the root outwards. Every bone not listed moves with its parent.
PARENTS = {
    "bricho": None, "zebra": "bricho", "hrudnik": "zebra", "krk": "hrudnik", "hlava": "krk",
    "lrameno": "hrudnik", "prameno": "hrudnik", "lbiceps": "lrameno", "pbiceps": "prameno",
    "lloket": "lbiceps", "ploket": "pbiceps", "lruka": "lloket", "pruka": "ploket",
    "lzadek": "bricho", "pzadek": "bricho", "lstehno": "lzadek", "pstehno": "pzadek",
    "lholen": "lstehno", "pholen": "pstehno", "lchodidlo": "lholen", "pchodidlo": "pholen",
    "lprsty": "lchodidlo", "pprsty": "pchodidlo", "zbran": "pruka", "roura": "hrudnik",
}


def rot_x(angle, pivot=(0.0, 0.0, 0.0)):
    """Rotation about the left-right axis through ``pivot``. A positive angle
    swings a hanging limb FORWARD (towards -Z) and tips an upright body BACK."""
    c, s = math.cos(angle), math.sin(angle)
    # columns: aside, up, dir
    r = (1.0, 0.0, 0.0, 0.0, c, s, 0.0, -s, c)
    px, py, pz = pivot
    rp = (px, c * py - s * pz, s * py + c * pz)
    return r + (px - rp[0], py - rp[1], pz - rp[2])


def translate(x, y, z):
    return (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, float(x), float(y), float(z))


def pose_matrices(local, root):
    """Absolute matrix per bone from per-bone local transforms about rest pivots.

    ``local[bone]`` is a 12-float matrix in model axes (already built about the
    bone's own rest pivot); ``root`` moves the whole body. A point on bone b
    is moved by its own joint first, then by each ancestor's joint, then by
    the root: M_b = root * A_ancestor_n * ... * A_b.
    """
    out = []
    for bone in BONES:
        chain = []
        b = bone
        while b is not None:
            chain.append(local.get(b, IDENT))
            b = PARENTS[b]
        m = root
        for a in reversed(chain):       # outermost ancestor first
            m = rtm_io.mat_mul(m, a)
        out.append(m)
    return out


def leg_angles(phase, swing_amp, knee_amp):
    """(thigh, knee) for a leg whose stance half starts at ``phase`` 0.

    Stance (0..0.5): the leg is straight and the ankle travels back at a
    constant speed, so with the body advancing by the step the foot stays put.
    Swing (0.5..1): the foot returns forward on a cosine, knee bent to clear
    the ground.
    """
    sin_a = math.sin(swing_amp)
    if phase < 0.5:
        u = phase / 0.5                       # 0 front .. 1 back
        return math.asin(sin_a * (1.0 - 2.0 * u)), 0.0
    s = (phase - 0.5) / 0.5
    thigh = math.asin(-sin_a * math.cos(math.pi * s))
    lift = math.sin(math.pi * s)
    return thigh + 0.5 * knee_amp * lift, knee_amp * lift


def walk_frames(piv, n=32, swing_deg=15.0, knee_deg=28.0, arm_deg=9.0, elbow_deg=6.0):
    swing = math.radians(swing_deg)
    hip_y = piv["lstehno"][1]
    ankle_y = piv["lchodidlo"][1]
    leg = hip_y - ankle_y
    half_stride = 2.0 * leg * math.sin(swing)     # body travel per stance half
    frames = []
    for i in range(n + 1):
        ph = (i % n) / n
        local = {}
        stance_drop = 0.0
        for side, off in (("l", 0.0), ("p", 0.5)):
            p = (ph + off) % 1.0
            thigh, knee = leg_angles(p, swing, math.radians(knee_deg))
            local[side + "stehno"] = rot_x(thigh, piv[side + "stehno"])
            local[side + "holen"] = rot_x(-knee, piv[side + "holen"])
            local[side + "chodidlo"] = rot_x(-(thigh - knee), piv[side + "chodidlo"])   # sole stays level
            if p < 0.5:
                stance_drop = max(stance_drop, leg * (1.0 - math.cos(thigh)))
            # Arms swing against the leg of the same side.
            arm = -math.radians(arm_deg) * math.cos(2.0 * math.pi * p)
            local[side + "biceps"] = rot_x(arm, piv[side + "biceps"])
            local[side + "loket"] = rot_x(math.radians(elbow_deg), piv[side + "loket"])
        frames.append((i / n, pose_matrices(local, translate(0.0, -stance_drop, 0.0))))
    return frames, (0.0, 0.0, -2.0 * half_stride)


def stand_frames(piv, n=8):
    frames = []
    for i in range(n + 1):
        w = math.sin(2.0 * math.pi * (i % n) / n)
        local = {
            "hrudnik": rot_x(math.radians(-0.8) * w, piv["hrudnik"]),
            "hlava": rot_x(math.radians(-1.5) * w, piv["hlava"]),
        }
        for side in "lp":
            local[side + "biceps"] = rot_x(math.radians(1.5) * w, piv[side + "biceps"])
        frames.append((i / n, pose_matrices(local, IDENT)))
    return frames


def die_frames(lift, n=12):
    """Topple forward about the ground line under the toes, ending face down."""
    frames = []
    for i in range(n + 1):
        t = i / n
        ease = t * t                              # slow start, hard landing
        m = rtm_io.mat_mul(translate(0.0, lift * ease, 0.0), rot_x(-0.5 * math.pi * ease))
        frames.append((t, [m] * len(BONES)))
    return frames


def write(path, frames, step, looped):
    anim = rtm_io.new_animation(BONES, frames, step)
    rtm_io.write_rtm(anim, path, verify=True, require_bones=BONES, looped=looped)
    back = rtm_io.read_rtm(path, strict=True)
    print("[rtm] %s: %s" % (os.path.basename(path), rtm_io.describe(back)), flush=True)
    return back


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", required=True)
    ap.add_argument("--rest", default="")
    ap.add_argument("--height", type=float, default=40.0)
    ap.add_argument("--depth", type=float, default=6.0, help="how far the body reaches forward of the origin (m)")
    args = ap.parse_args()
    os.makedirs(args.out, exist_ok=True)

    piv = None
    if args.rest and os.path.isfile(args.rest):
        with open(args.rest, encoding="utf-8") as fh:
            rest = json.load(fh)
        if rest.get("format") != "ud-rtm-rest/1":
            raise SystemExit("%s: not a ud-rtm-rest/1 file" % args.rest)
        piv = {k: tuple(float(c) for c in v) for k, v in rest["pivots"].items()}
        missing = [b for b in ("lstehno", "pstehno", "lholen", "pholen", "lchodidlo", "pchodidlo",
                               "lbiceps", "pbiceps", "lloket", "ploket", "hrudnik", "hlava") if b not in piv]
        if missing:
            raise SystemExit("rest file lacks pivots: %s" % ", ".join(missing))

    summary = {"rest": args.rest if piv else None}
    if piv:
        write(os.path.join(args.out, "ded_stand.rtm"), stand_frames(piv), (0, 0, 0), True)
        frames, step = walk_frames(piv)
        write(os.path.join(args.out, "ded_walk.rtm"), frames, step, True)
        # Planted-foot check: over the stance half the left ankle must slide
        # back by half the step, in a straight line at ankle height.
        li = BONES.index("lchodidlo")
        ankle = piv["lchodidlo"]
        zs = [rtm_io.mat_apply(m[li], ankle) for t, m in frames if t <= 0.5]
        slide = zs[-1][2] - zs[0][2]
        lift = max(abs(p[1] - ankle[1]) for p in zs)
        lin = max(abs((p[2] - zs[0][2]) - slide * k / (len(zs) - 1)) for k, p in enumerate(zs))
        print("[rtm] walk: step %.3f m per cycle, stance slide %.3f m (want %.3f), ankle lift %.4f m, "
              "non-linearity %.4f m" % (-step[2], slide, -step[2] / 2.0, lift, lin), flush=True)
        if abs(slide - (-step[2] / 2.0)) > 0.01 or lift > 0.01 or lin > 0.01:
            raise SystemExit("walk clip would skate: stance foot does not match the step")
        summary["walk_step_m"] = -step[2]
    else:
        two = [(0.0, [IDENT] * len(BONES)), (1.0, [IDENT] * len(BONES))]
        write(os.path.join(args.out, "ded_stand.rtm"), two, (0, 0, 0), True)
        glide = round(0.44 * args.height, 3)
        write(os.path.join(args.out, "ded_walk.rtm"), two, (0, 0, -glide), True)
        summary["walk_step_m"] = glide
    die = die_frames(args.depth)
    write(os.path.join(args.out, "ded_die.rtm"), die, (0, 0, 0), False)
    write(os.path.join(args.out, "ded_dead.rtm"), [(0.0, die[-1][1])], (0, 0, 0), None)
    with open(os.path.join(args.out, "ded_rtm.json"), "w", encoding="utf-8") as fh:
        json.dump(summary, fh, indent=1)


if __name__ == "__main__":
    main()
