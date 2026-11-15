"""Per-creature settings of the finishing exporter (finish_model.py). File space in every number:
x = his left, y = up, z = back, metres (the joint file's space).

A = the Starved Bear (UDDedushka), D = the dog and the lizard (UDDedushkaHound).
"""

# ---- weights ---------------------------------------------------------------------------------------
# ("root", child, parent, joint, toward, t_in, t_out): the tube of bone `child` is pushed into the
#   body of `parent`. Along the line from `joint` toward the joint `toward`, everything of that tube
#   further in than t_in metres belongs to the parent, everything past t_out to the child, a smooth
#   blend between. So the root of a limb stays in the hip or shoulder when the limb swings.
# ("spine", [(bone_low, bone_high, y0, y1), ...]): along the height of the trunk tube that carries
#   those bones, a smooth blend from bone_low under y0 to bone_high over y1.
# ("ball", side): the ball of the foot bends over a metre and a half, not at one ring.
WEIGHTS = {
    "a": [
        ("spine", [("bricho", "zebra", 17.0, 22.2), ("zebra", "hrudnik", 23.4, 29.6)]),
        ("root", "lstehno", "bricho", "lstehno", "lholen", -1.3, 2.6),
        ("root", "pstehno", "bricho", "pstehno", "pholen", -1.3, 2.6),
        ("root", "lbiceps", "lrameno", "lbiceps", "lloket", -1.2, 2.4),
        ("root", "pbiceps", "prameno", "pbiceps", "ploket", -1.2, 2.4),
        ("root", "krk", "hrudnik", "krk", "hlava", -1.5, 1.8),
        ("ball", "l"), ("ball", "p"),
    ],
    "d": [
        ("root", "pstehno", "bricho", "pstehno", "pholen", -1.6, 3.0),
        ("root", "lstehno", "bricho", "lstehno", "lholen", -1.2, 2.6),
        ("root", "lbiceps", "lrameno", "lbiceps", "lloket", -1.0, 2.2),
        ("root", "pbiceps", "prameno", "pbiceps", "ploket", -1.0, 2.2),
        ("root", "ded_tail1", "bricho", "ded_tail1", "ded_tail2", -0.8, 1.6),
    ],
}

# ---- clips: settings written into the joint file's "anim" block (gen_ded_anim.py, TUNING) ----------
ANIM = {
    "a": {"arm_out_l": 3.0, "arm_out_p": 5.0, "arm_back": 5.0, "hand_clearance": 0.25, "heel_off": 19.0, "toe_droop": 9.0,
          "hang_swing": 1.6},
    "d": {"tail_lift": 16.0, "tail_sway": 2.4, "tail_whip": 7.0, "hang_swing": 1.5, "heel_off": 18.0, "toe_droop": 8.0,
          "tall_arm_swing": 6.0},
}

# ---- collision and hit shapes ----------------------------------------------------------------------
# (name, owner bone, [bones whose vertices it wraps], axis from joint, axis to joint or None for up, options)
# One tapered box each: convex, in ONE bone, so it follows the animation. `fire` only: not in the
# collision shape (Geometry), which also drops everything under its floor.
HULLS = {
    "a": [
        ("pelvis", "bricho", ["bricho"], None, None, {}),
        ("waist", "zebra", ["zebra"], "zebra", "hrudnik", {"min_side": 3.0}),
        ("chest", "hrudnik", ["hrudnik", "ded_rib_l", "ded_rib_r", "lrameno", "prameno"], None, None, {"split_front": "furnace"}),
        ("neck", "krk", ["krk"], "krk", "hlava", {}),
        ("head", "hlava", ["hlava"], "hlava", "@mouth", {}),
        ("jaw", "ded_jaw", ["ded_jaw"], "ded_jaw", "@mouth", {}),
        ("arm_l", "lbiceps", ["lbiceps"], "lbiceps", "lloket", {}),
        ("arm_r", "pbiceps", ["pbiceps"], "pbiceps", "ploket", {}),
        ("forearm_l", "lloket", ["lloket"], "lloket", "lruka", {"fire": True}),
        ("forearm_r", "ploket", ["ploket"], "ploket", "pruka", {"fire": True}),
        ("hand_l", "lruka", ["lruka"], "lruka", "@hand_l", {"fire": True}),
        ("hand_r", "pruka", ["pruka"], "pruka", "@hand_r", {"fire": True}),
        ("thigh_l", "lstehno", ["lstehno"], "lstehno", "lholen", {}),
        ("thigh_r", "pstehno", ["pstehno"], "pstehno", "pholen", {}),
        ("shin_l", "lholen", ["lholen"], "lholen", "lchodidlo", {"fire": True}),
        ("shin_r", "pholen", ["pholen"], "pholen", "pchodidlo", {"fire": True}),
        ("foot_l", "lchodidlo", ["lchodidlo", "lprsty"], "@heel_l", "lprsty", {"fire": True, "flat": True}),
        ("foot_r", "pchodidlo", ["pchodidlo", "pprsty"], "@heel_r", "pprsty", {"fire": True, "flat": True}),
    ],
    "d": [
        ("pelvis", "bricho", ["bricho", "zebra"], None, None, {}),
        ("chest", "hrudnik", ["hrudnik", "ded_rib_l", "ded_rib_r"], None, None, {"split_front": "furnace"}),
        ("neck", "krk", ["krk"], "krk", "hlava", {}),
        ("head", "hlava", ["hlava"], "hlava", "@mouth", {}),
        ("jaw", "ded_jaw", ["ded_jaw"], "ded_jaw", "@mouth", {}),
        ("deadhead", "ded_hang1", ["ded_hang1"], "ded_hang1", "@deadhead_tip", {}),
        ("arm_l", "lbiceps", ["lbiceps"], "lbiceps", "lloket", {}),
        ("arm_r", "pbiceps", ["pbiceps"], "pbiceps", "ploket", {}),
        ("forearm_l", "lloket", ["lloket"], "lloket", "lruka", {"fire": True}),
        ("forearm_r", "ploket", ["ploket"], "ploket", "pruka", {"fire": True}),
        ("hand_l", "lruka", ["lruka"], "lruka", "@hand_l", {"fire": True}),
        ("hand_r", "pruka", ["pruka"], "pruka", "@hand_r", {"fire": True}),
        ("thigh_l", "lstehno", ["lstehno"], "lstehno", "lholen", {}),
        ("thigh_r", "pstehno", ["pstehno"], "pstehno", "pholen", {}),
        ("shin_l", "lholen", ["lholen"], "lholen", "lchodidlo", {}),
        ("shin_r", "pholen", ["pholen"], "pholen", "pchodidlo", {}),
        ("hock_r", "pchodidlo", ["pchodidlo"], "pchodidlo", "pprsty", {"fire": True}),
        ("foot_l", "lchodidlo", ["lchodidlo", "lprsty"], "@heel_l", "lprsty", {"fire": True, "flat": True}),
        ("paw_r", "pprsty", ["pprsty"], None, None, {"fire": True, "flat": True}),
        ("tail1", "ded_tail1", ["ded_tail1"], "ded_tail1", "ded_tail2", {}),
        ("tail2", "ded_tail2", ["ded_tail2", "ded_tail3"], "ded_tail2", "ded_tail3", {"fire": True}),
    ],
}

# Named hit regions: selection name in FireGeometry -> the hulls in it. The combat feature tells the
# place of a hit from its position; these names are there for it (or the engine) to use.
REGIONS = {
    "ded_hit_furnace": ["furnace"],
    "ded_hit_head": ["head", "jaw", "neck"],
    "ded_hit_body": ["chest", "pelvis", "waist", "deadhead", "tail1", "tail2"],
    "ded_hit_arms": ["arm_l", "arm_r", "forearm_l", "forearm_r", "hand_l", "hand_r"],
    "ded_hit_legs": ["thigh_l", "thigh_r", "shin_l", "shin_r", "foot_l", "foot_r", "hock_r", "paw_r"],
}

# Share of the height under which the collision shape (Geometry) has nothing: men, cars, tanks and
# wrecks pass under it (integration loose end 1). FireGeometry reaches the ground.
GEO_FLOOR = 0.17
# ... and in motion: swinging parts are cut this much higher, so no clip brings them under the floor.
GEO_SWING_MARGIN = {"a": 2.0, "d": 2.0}
# The radius of the collision shape (half the diagonal of its box) of the builds the fights were tuned
# on (integrationuild\lift_a.log, lift_d.log). The new shape is drawn in to it: see finish_model.py.
GEO_RADIUS = {"a": 21.32, "d": 20.43}

# ---- far LODs --------------------------------------------------------------------------------------
# (resolution as a share of the height, voxel size m, triangles, hide sheet px, glow sheet px, smallest piece kept m)
FAR = {
    "a": [(0.50, 0.30, 2600, 1024, 256, 0.9), (0.80, 0.50, 1000, 512, 128, 1.6), (1.15, 0.80, 440, 256, 64, 2.4)],
    "d": [(0.50, 0.30, 2600, 1024, 256, 0.9), (0.80, 0.40, 1000, 512, 128, 1.6), (1.15, 0.50, 440, 256, 64, 2.4)],
}
# Bones whose pieces are left out of the far skins and only painted on: D's pale ribs cross the furnace
# slot, and as lumps of skin at range they hid it; as dark bars painted across the bright face they
# frame it. A's rib gates stay: they are a third of his outline.
FAR_SKIN_DROP = {"a": (), "d": ("ded_rib_l", "ded_rib_r")}
# Rings of faces round the furnace that are also made fullbright, per far LOD, and how much the fire's
# paint is lifted there. D's furnace is a slot a quarter the size of A's and was a spark at 300 m.
FAR_GLOW_GROW = {"a": (0, 0, 0), "d": (0, 1, 1)}
FAR_GLOW_GAIN = {"a": 1.15, "d": 1.25}
# Of four looks at LOD0 through a far face, how many must see fire for the face to be fullbright. A's
# fire is behind rib gates: one look is enough there (the gates are then painted on it as dark bars),
# or the chest goes dark at night at range. D's slot is open: two.
FAR_GLOW_VOTES = {"a": 1, "d": 2}
# ---- eyes (memory points ded_eye_l, ded_eye_r): a guess, moved onto the nearest face of the head -------
# from_pilot: metres from the artist's eye point in Blender space after the turn (x = his left, y = back, z = up);
# at: file space.
EYES = {
    "a": {"l": {"from_pilot": [1.6, 0.0, 0.0]}, "r": {"from_pilot": [-1.6, 0.0, 0.0]}},
    "d": {"r": {"at": [-13.0, 31.35, -9.2]}, "l": {"at": [-10.2, 30.15, -10.55]}},
}

# ---- openings widened (mesh_fixes.open_wound): (which, widen, heighten, reach, (depth whole, depth gone)) --
# D's furnace was a slot 7 m across: a spark at 300 m and nothing from behind. The slot and the split in
# the back are drawn open; no vertex or UV is added.
WOUNDS = {
    "a": [],
    "d": [("front", 0.28, 0.08, 2.0, (6.0, 10.0)), ("others", 0.5, 0.06, 2.2, (3.0, 6.0))],
}
