"""Dedushka rig: joint file, canonical bones, pose channels, leg IK (numpy, no bpy).

A clip is a list of POSES; a pose is a dict {channel: value}. ``Rig.evaluate``
turns one pose into one absolute matrix per canonical bone (RTM file space:
X = the creature's left, Y up, Z = back, so forward is -Z; origin on the
ground between the feet; the rest pose IS the standing pose, so an identity
matrix leaves a bone where the modeller put it).

Channels (all default to 0; angles in degrees, lengths in metres):

  root.x root.y root.z        pelvis offset (x left, y up, z BACK)
  root.pitch root.yaw root.roll
  <bone>.pitch                rotation about the left-right axis through the
                              bone's joint. POSITIVE tips an upright part BACK
                              and swings a hanging part FORWARD. A forward bow
                              of the spine is therefore NEGATIVE (use BOW).
  <bone>.yaw                  about the vertical; positive turns it LEFT
  <bone>.roll                 about the fore-aft axis; positive leans it LEFT
  lfoot.* / pfoot.*           the foot TARGET (legs are solved by IK):
      .x .y .z                offset of the sole contact point from rest
      .orbit                  turn the target about the creature's vertical
                              axis (positive = left), for turns on the spot
      .pitch .yaw .roll       the foot about its sole contact point
      .heel                   heel rise about the toe joint (toes stay flat)
      .toe                    toe bend about the toe joint

Bones without a joint in the joint file ride on their parent, so a model
without a jaw, ribs, tail or hanging parts simply ignores those channels.
"""
import json
import math

import numpy as np

# The 25 stock man bones, in the order of the stock RTM header (rtm_io.STOCK_MAN_BONES).
STOCK_BONES = (
    "pchodidlo", "lchodidlo", "pprsty", "lprsty", "lholen", "pholen", "pstehno", "lstehno",
    "pzadek", "lzadek", "bricho", "zebra", "hrudnik", "krk", "prameno", "lrameno", "hlava",
    "pbiceps", "lbiceps", "ploket", "lloket", "roura", "zbran", "pruka", "lruka",
)
# Dedushka's own optional bones. Always written to the RTM, so the skeleton
# is the same for every model; a model without them has no such selection.
EXTRA_BONES = (
    "ded_jaw", "ded_rib_l", "ded_rib_r", "ded_tail1", "ded_tail2", "ded_tail3", "ded_hang1", "ded_hang2",
)
BONES = STOCK_BONES + EXTRA_BONES

PARENTS = {
    "bricho": None, "zebra": "bricho", "hrudnik": "zebra", "krk": "hrudnik", "hlava": "krk",
    "lrameno": "hrudnik", "prameno": "hrudnik", "lbiceps": "lrameno", "pbiceps": "prameno",
    "lloket": "lbiceps", "ploket": "pbiceps", "lruka": "lloket", "pruka": "ploket",
    "lzadek": "bricho", "pzadek": "bricho", "lstehno": "lzadek", "pstehno": "pzadek",
    "lholen": "lstehno", "pholen": "pstehno", "lchodidlo": "lholen", "pchodidlo": "pholen",
    "lprsty": "lchodidlo", "pprsty": "pchodidlo", "zbran": "pruka", "roura": "hrudnik",
    "ded_jaw": "hlava", "ded_rib_l": "hrudnik", "ded_rib_r": "hrudnik",
    "ded_tail1": "bricho", "ded_tail2": "ded_tail1", "ded_tail3": "ded_tail2",
    "ded_hang1": "hrudnik", "ded_hang2": "krk",
}
# Joints the generator cannot work without.
REQUIRED_PIVOTS = (
    "zebra", "hrudnik", "krk", "hlava",
    "lrameno", "lbiceps", "lloket", "lruka", "prameno", "pbiceps", "ploket", "pruka",
    "lstehno", "lholen", "lchodidlo", "lprsty", "pstehno", "pholen", "pchodidlo", "pprsty",
)
LEG_BONES = tuple(s + b for s in "lp" for b in ("stehno", "holen", "chodidlo", "prsty"))
SIDES = ("l", "p")              # l = left, p = right (the stock prefix)
SIDE_EN = {"l": "l", "p": "r"}  # joint file point names use l / r


# ---------------------------------------------------------------------------
# Matrices (4x4 homogeneous, column vectors)

def vec(p):
    return np.array([float(p[0]), float(p[1]), float(p[2])])


def rx(deg):
    """About X (left). Positive: up tips BACK (+Z), a hanging limb swings FORWARD."""
    a = math.radians(deg)
    c, s = math.cos(a), math.sin(a)
    return np.array([[1.0, 0.0, 0.0], [0.0, c, -s], [0.0, s, c]])


def ry(deg):
    """About Y (up). Positive: forward (0,0,-1) turns LEFT (+X)."""
    a = math.radians(deg)
    c, s = math.cos(a), math.sin(a)
    return np.array([[c, 0.0, -s], [0.0, 1.0, 0.0], [s, 0.0, c]])


def rz(deg):
    """About Z (back). Positive: up leans LEFT (+X); a hanging right arm lifts out to the right."""
    a = math.radians(deg)
    c, s = math.cos(a), math.sin(a)
    return np.array([[c, s, 0.0], [-s, c, 0.0], [0.0, 0.0, 1.0]])


def euler(pitch, yaw, roll):
    return ry(yaw) @ rx(pitch) @ rz(roll)


def m4(r3=None, t=None):
    m = np.eye(4)
    if r3 is not None:
        m[:3, :3] = r3
    if t is not None:
        m[:3, 3] = t
    return m


def about(pivot, r3):
    """Rotation r3 about the point ``pivot``."""
    p = vec(pivot)
    return m4(r3, p - r3 @ p)


def apply(m, p):
    return m[:3, :3] @ vec(p) + m[:3, 3]


def to12(m):
    """The RTM layout: aside, up, dir (the COLUMNS of the 3x3) then pos."""
    return tuple(float(v) for v in (m[0, 0], m[1, 0], m[2, 0], m[0, 1], m[1, 1], m[2, 1],
                                    m[0, 2], m[1, 2], m[2, 2], m[0, 3], m[1, 3], m[2, 3]))


def unit(v):
    n = float(np.linalg.norm(v))
    return v / n if n > 1e-9 else np.array([0.0, 1.0, 0.0])


def rot_between(u, v):
    """Smallest rotation that turns direction u onto direction v."""
    u, v = unit(u), unit(v)
    axis = np.cross(u, v)
    s = float(np.linalg.norm(axis))
    c = float(np.dot(u, v))
    if s < 1e-9:
        if c > 0:
            return np.eye(3)
        # opposite: turn half a circle about any axis square to u
        other = np.array([1.0, 0.0, 0.0]) if abs(u[0]) < 0.9 else np.array([0.0, 0.0, 1.0])
        axis = unit(np.cross(u, other))
        return 2.0 * np.outer(axis, axis) - np.eye(3)
    axis = axis / s
    k = np.array([[0.0, -axis[2], axis[1]], [axis[2], 0.0, -axis[0]], [-axis[1], axis[0], 0.0]])
    return np.eye(3) + s * k + (1.0 - c) * (k @ k)


def rot_angle_deg(r3):
    return math.degrees(math.acos(max(-1.0, min(1.0, (float(np.trace(r3)) - 1.0) / 2.0))))


# ---------------------------------------------------------------------------
# Easing

def smooth(x):
    """Smoothstep on 0..1, flat at both ends."""
    x = max(0.0, min(1.0, x))
    return x * x * (3.0 - 2.0 * x)


def smoother(x):
    x = max(0.0, min(1.0, x))
    return x * x * x * (x * (6.0 * x - 15.0) + 10.0)


def ramp(t, t0, t1):
    """0 before t0, 1 after t1, a smooth S between."""
    if t1 <= t0:
        return 1.0 if t >= t1 else 0.0
    return smoother((t - t0) / (t1 - t0))


def pulse(t, tau):
    """A soft shock: 0 at t <= 0, peaks at 1 when t = tau, then dies away."""
    if t <= 0.0:
        return 0.0
    x = t / tau
    return x * math.exp(1.0 - x)


# ---------------------------------------------------------------------------
# Poses

def mix(a, b, w):
    """a + w * (b - a), channel by channel."""
    out = dict(a)
    for k in set(a) | set(b):
        out[k] = a.get(k, 0.0) + w * (b.get(k, 0.0) - a.get(k, 0.0))
    return out


def add(a, b, w=1.0):
    """a + w * b."""
    out = dict(a)
    for k, v in b.items():
        out[k] = out.get(k, 0.0) + w * v
    return out


class Rig:
    """Joints of one model and the pose solver for it."""

    def __init__(self, rest):
        if rest.get("format") != "ud-rtm-rest/1":
            raise ValueError("joint file is not ud-rtm-rest/1")
        piv = {k.lower(): vec(v) for k, v in rest["pivots"].items()}
        missing = [b for b in REQUIRED_PIVOTS if b not in piv]
        if missing:
            raise ValueError("joint file lacks joints: " + ", ".join(missing))
        self.rest = rest
        self.parents = dict(PARENTS)
        for b, p in (rest.get("parents") or {}).items():
            if b.lower() in self.parents and (p is None or p.lower() in self.parents):
                self.parents[b.lower()] = p.lower() if p else None
        self.has = {b: (b in piv) for b in BONES}
        self.root = vec(rest["root"]) if rest.get("root") else 0.5 * (piv["lstehno"] + piv["pstehno"])
        piv["bricho"] = self.root
        for s in SIDES:
            piv.setdefault(s + "zadek", self.root)
        # A bone without a joint rides on its parent: give it the parent's joint.
        for b in self._parent_first():
            if b not in piv:
                p = self.parents[b]
                piv[b] = piv[p] if p else self.root
        self.piv = piv
        self.height = float(rest.get("height_m") or (piv["hlava"][1] * 1.1))
        pts = {k.lower(): vec(v) for k, v in (rest.get("points") or {}).items()}
        self.points = {}
        for s in SIDES:
            en = SIDE_EN[s]
            ankle, toe = piv[s + "chodidlo"], piv[s + "prsty"]
            # Sole contact: under the middle of the foot, on the ground.
            self.points["foot_" + s] = pts.get("foot_" + en, np.array([ankle[0], 0.0, 0.5 * (ankle[2] + toe[2])]))
            wrist, elbow = piv[s + "ruka"], piv[s + "loket"]
            self.points["hand_" + s] = pts.get("hand_" + en, wrist + 0.25 * (wrist - elbow))
        self.points["furnace"] = pts.get("furnace", 0.5 * (piv["hrudnik"] + piv["krk"]))
        self.points["mouth"] = pts.get("mouth", piv["hlava"] + np.array([0.0, 0.25, -0.5]) * (self.height * 0.1))
        self.points["head_top"] = pts.get("head_top", np.array([piv["hlava"][0], self.height, piv["hlava"][2]]))
        self.legs = {}
        for s in SIDES:
            hip, knee, ankle = piv[s + "stehno"], piv[s + "holen"], piv[s + "chodidlo"]
            l1, l2 = float(np.linalg.norm(knee - hip)), float(np.linalg.norm(ankle - knee))
            line = unit(ankle - hip)
            off = (knee - hip) - float(np.dot(knee - hip, line)) * line
            # Which way the knee bends: the rest pose says so when the leg is
            # bent there, otherwise forward.
            pole = unit(off) if float(np.linalg.norm(off)) > 0.03 * (l1 + l2) else np.array([0.0, 0.0, -1.0])
            toe_ground = np.array([piv[s + "prsty"][0], 0.0, piv[s + "prsty"][2]])
            contact = self.points["foot_" + s]
            # The back of the sole: given, or as far behind the contact point as the toe joint is ahead of it.
            heel_ground = pts.get("heel_" + SIDE_EN[s], 2.0 * contact - toe_ground)
            # A sole point ahead of the toe joint lies on the toes (a paw: the joint is its wrist),
            # one behind it lies on the foot.
            ahead = lambda p: "toe" if p[2] < toe_ground[2] - 1e-6 else "foot"
            self.legs[s] = {"hip": hip, "knee": knee, "ankle": ankle, "toe": piv[s + "prsty"],
                            "contact": contact, "toe_ground": toe_ground, "heel_ground": heel_ground,
                            "contact_on": ahead(contact), "heel_on": ahead(heel_ground),
                            "l1": l1, "l2": l2, "pole": pole, "rest_len": float(np.linalg.norm(ankle - hip))}
        self.leg_len = min(l["l1"] + l["l2"] for l in self.legs.values())
        self.hand_clearance = None      # metres; set to keep the hands off the ground

    def _parent_first(self):
        out, pending = [], list(BONES)
        while pending:
            for b in list(pending):
                p = self.parents[b]
                if p is None or p in out:
                    out.append(b)
                    pending.remove(b)
        return out

    # -- pose solver --------------------------------------------------------

    def _local(self, pose, bone):
        p, y, r = pose.get(bone + ".pitch", 0.0), pose.get(bone + ".yaw", 0.0), pose.get(bone + ".roll", 0.0)
        if p == 0.0 and y == 0.0 and r == 0.0:
            return None
        return about(self.piv[bone], euler(p, y, r))

    def _trunk(self, pose, root_y_extra=0.0):
        off = np.array([pose.get("root.x", 0.0), pose.get("root.y", 0.0) + root_y_extra, pose.get("root.z", 0.0)])
        r_pelvis = euler(pose.get("root.pitch", 0.0), pose.get("root.yaw", 0.0), pose.get("root.roll", 0.0))
        m_root = m4(t=off) @ about(self.root, r_pelvis)
        out = {}
        for b in self._parent_first():
            if b in LEG_BONES:
                continue
            p = self.parents[b]
            if p is None:
                out[b] = m_root
                continue
            loc = self._local(pose, b)
            out[b] = out[p] if loc is None else out[p] @ loc
        return out, r_pelvis

    def _foot(self, pose, s):
        leg = self.legs[s]
        k = s + "foot"
        orbit = pose.get(k + ".orbit", 0.0)
        target = ry(orbit) @ leg["contact"] + np.array([pose.get(k + ".x", 0.0), pose.get(k + ".y", 0.0),
                                                        pose.get(k + ".z", 0.0)])
        yaw = orbit + pose.get(k + ".yaw", 0.0)
        r_foot = euler(pose.get(k + ".pitch", 0.0), yaw, pose.get(k + ".roll", 0.0))
        base = m4(r_foot, target - r_foot @ leg["contact"])
        heel = pose.get(k + ".heel", 0.0)
        toe = pose.get(k + ".toe", 0.0)
        m_foot = base @ about(leg["toe"], rx(-heel)) if heel else base
        m_toe = base @ about(leg["toe"], rx(toe)) if toe else base
        # No part of the sole goes under the ground, whatever the foot is asked to do.
        owner = {"foot": m_foot, "toe": m_toe}
        low = min(float(apply(owner[leg["contact_on"]], leg["contact"])[1]),
                  float(apply(owner[leg["heel_on"]], leg["heel_ground"])[1]), float(apply(m_toe, leg["toe_ground"])[1]))
        if low < 0.0:
            up = m4(t=np.array([0.0, -low, 0.0]))
            m_foot, m_toe = up @ m_foot, up @ m_toe
        return m_foot, m_toe, yaw

    def _leg(self, s, m_pelvis, r_pelvis, m_foot, foot_yaw, pelvis_yaw):
        leg = self.legs[s]
        hip = apply(m_pelvis, leg["hip"])
        ankle = apply(m_foot, leg["ankle"])
        l1, l2 = leg["l1"], leg["l2"]
        d = ankle - hip
        dist = float(np.linalg.norm(d))
        reach = (l1 + l2) * 0.9995
        lc = max(abs(l1 - l2) + 1e-3, min(dist, reach))
        along = d / dist if dist > 1e-9 else np.array([0.0, -1.0, 0.0])
        # The knee points halfway between where the pelvis and the foot face.
        pole = ry(0.5 * (foot_yaw - pelvis_yaw)) @ (r_pelvis @ leg["pole"])
        perp = pole - float(np.dot(pole, along)) * along
        perp = unit(perp) if float(np.linalg.norm(perp)) > 1e-6 else unit(np.cross(along, np.array([1.0, 0.0, 0.0])))
        cos_a = max(-1.0, min(1.0, (l1 * l1 + lc * lc - l2 * l2) / (2.0 * l1 * lc)))
        sin_a = math.sqrt(max(0.0, 1.0 - cos_a * cos_a))
        knee = hip + l1 * (cos_a * along + sin_a * perp)
        ankle_eff = hip + lc * along
        r_th = rot_between(r_pelvis @ (leg["knee"] - leg["hip"]), knee - hip) @ r_pelvis
        m_th = m4(r_th, hip - r_th @ leg["hip"])
        r_sh = rot_between(r_th @ (leg["ankle"] - leg["knee"]), ankle_eff - knee) @ r_th
        m_sh = m4(r_sh, knee - r_sh @ leg["knee"])
        return m_th, m_sh, dist - reach

    def reach_drop(self, pose, slack=0.0):
        """How far the pelvis must come down for both feet to be reached with
        ``slack`` metres of leg to spare (0 = it need not come down)."""
        lo, hi = 0.0, 0.0
        if self._stretch(pose, 0.0) + slack <= 0.0:
            return 0.0
        hi = 0.25
        while self._stretch(pose, -hi) + slack > 0.0 and hi < self.leg_len:
            hi *= 2.0
        for _ in range(30):
            mid = 0.5 * (lo + hi)
            if self._stretch(pose, -mid) + slack > 0.0:
                lo = mid
            else:
                hi = mid
        return hi

    def _stretch(self, pose, dy):
        trunk, r_pelvis = self._trunk(pose, dy)
        worst = -1e9
        for s in SIDES:
            m_foot, _, yaw = self._foot(pose, s)
            _, _, st = self._leg(s, trunk[s + "zadek"], r_pelvis, m_foot, yaw, pose.get("root.yaw", 0.0))
            worst = max(worst, st)
        return worst

    def evaluate(self, pose):
        """One pose -> ({bone: 4x4}, worst leg stretch in metres; above 0 the foot was not reached)."""
        pose = dict(pose)
        trunk, r_pelvis = self._trunk(pose)
        if self.hand_clearance is not None:
            for s in SIDES:
                for _ in range(40):
                    if apply(trunk[s + "ruka"], self.points["hand_" + s])[1] >= self.hand_clearance:
                        break
                    pose[s + "loket.pitch"] = pose.get(s + "loket.pitch", 0.0) + 2.0
                    trunk, r_pelvis = self._trunk(pose)
        out = dict(trunk)
        worst = -1e9
        for s in SIDES:
            m_foot, m_toe, yaw = self._foot(pose, s)
            m_th, m_sh, st = self._leg(s, trunk[s + "zadek"], r_pelvis, m_foot, yaw, pose.get("root.yaw", 0.0))
            out[s + "stehno"], out[s + "holen"] = m_th, m_sh
            out[s + "chodidlo"], out[s + "prsty"] = m_foot, m_toe
            worst = max(worst, st)
        return out, worst

    # -- helpers for clips and reports --------------------------------------

    def point(self, mats, name):
        """Posed position of a named point (foot_l, hand_p, furnace, mouth, head_top)."""
        owner = {"furnace": "hrudnik", "mouth": "hlava", "head_top": "hlava"}.get(name)
        if owner is None:
            s = name[-1]
            if name.startswith("foot"):
                owner = s + ("prsty" if self.legs[s]["contact_on"] == "toe" else "chodidlo")
            else:
                owner = s + "ruka"
        return apply(mats[owner], self.points[name])

    def spine_lean_deg(self):
        """Forward lean of the rest spine (hip centre to neck base), degrees."""
        d = self.piv["krk"] - self.root
        return math.degrees(math.atan2(-d[2], d[1]))


def load_rig(path):
    with open(path, encoding="utf-8") as fh:
        return Rig(json.load(fh))


def script_xyz(p):
    """File space point -> [right, forward, up] metres, the frame a mission script wants."""
    return [round(-float(p[0]), 2) + 0.0, round(-float(p[2]), 2) + 0.0, round(float(p[1]), 2) + 0.0]
