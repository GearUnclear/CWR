"""Renders a built creature P3D in motion, in Blender, without the game.

  blender -b --factory-startup -P motion_render.py -- <model.p3d> <anim dir> <texture png dir> <out dir>
          [--lod 0] [--size 512] [--moves walk,stomp,...] [--cams quarter,side] [--turntable 16] [--tag a]

For each move it poses the chosen LOD at a row of phases with the clip (the engine's own blend:
p3d_pose.py), and renders each pose from fixed cameras. Frames: <out>/<move>_<cam>_<nn>.png. The
strips are put together by motion_sheet.py (plain Python). Textures: <png dir>/<paa stem>.png (any
size; 1024 copies are enough here). Fullbright faces are drawn as emitters, the rest lit by one sun
and a dim sky, so this is a look at SHAPE AND MOTION, not at the game's light.

Blender space here: x = his left, y = back, z = up (file (x, y, z) -> Blender (x, z, y)), so he faces -Y.
"""
import math
import os
import sys

import bpy
import numpy as np
from mathutils import Vector

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import p3d_pose as pp  # noqa: E402

argv = sys.argv[sys.argv.index("--") + 1:]
P3D, ANIM, TEXDIR, OUT = argv[:4]
OPT = {"lod": "0", "size": "512", "moves": "", "cams": "quarter,side", "turntable": "0", "tag": "", "bg": "dusk"}
i = 4
while i < len(argv):
    OPT[argv[i].lstrip("-")] = argv[i + 1]
    i += 2
os.makedirs(OUT, exist_ok=True)
SIZE = int(OPT["size"])

# move -> (clip file stem, phases)
MOVES = {
    "idle": ("ded_idle", [0.0, 0.17, 0.33, 0.5, 0.67, 0.83]),
    "walk": ("ded_walk", [k / 8.0 for k in range(8)]),
    "turnl": ("ded_turnl", [0.0, 0.25, 0.5, 0.75]),
    "turnr": ("ded_turnr", [0.0, 0.25, 0.5, 0.75]),
    "roar": ("ded_roar", [0.0, 0.2, 0.38, 0.5, 0.62, 0.8]),
    "beamrise": ("ded_beamrise", [0.0, 0.35, 0.7, 1.0]),
    "beamhold": ("ded_beamhold", [0.0, 0.5]),
    "beamlower": ("ded_beamlower", [0.0, 0.3, 0.6, 1.0]),
    "stomp": ("ded_stomp", [0.0, 0.3, 0.5, 0.56, 0.63, 0.69, 0.8, 0.92]),
    "swat": ("ded_swat", [0.0, 0.2, 0.32, 0.42, 0.52, 0.62, 0.75, 0.9]),
    "flinch": ("ded_flinch", [0.0, 0.12, 0.3, 0.6]),
    "dormant": ("ded_dormant", [0.0, 0.5]),
    "die": ("ded_die", [0.0, 0.5, 0.8, 1.0]),
}
want = [m for m in OPT["moves"].split(",") if m] or list(MOVES)

model = pp.load(P3D)
lod = model.visual[int(OPT["lod"])]
H = float(lod.points[:, 1].max())


def to_bl(p):
    return np.stack([p[:, 0], p[:, 2], p[:, 1]], axis=1)


# ---- scene ---------------------------------------------------------------------------------------
bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene
scene.render.engine = "BLENDER_EEVEE_NEXT"
scene.eevee.taa_render_samples = 8
scene.render.resolution_x = SIZE
scene.render.resolution_y = SIZE
scene.render.image_settings.file_format = "PNG"
scene.render.film_transparent = False
scene.view_settings.view_transform = "Standard"
world = bpy.data.worlds.new("w")
scene.world = world
world.use_nodes = True
bg = world.node_tree.nodes["Background"]
if OPT["bg"] == "night":
    bg.inputs[0].default_value = (0.012, 0.015, 0.03, 1)
    sun_power = 0.25
else:
    bg.inputs[0].default_value = (0.36, 0.40, 0.48, 1)
    sun_power = 2.6
bg.inputs[1].default_value = 1.0
sun = bpy.data.objects.new("sun", bpy.data.lights.new("sun", "SUN"))
sun.data.energy = sun_power
sun.rotation_euler = Vector((0.45, 0.7, -0.62)).to_track_quat("-Z", "Y").to_euler()
scene.collection.objects.link(sun)
# ground
gm = bpy.data.meshes.new("ground")
g = 140.0
gm.from_pydata([(-g, -g, 0), (g, -g, 0), (g, g, 0), (-g, g, 0)], [], [(0, 1, 2, 3)])
ground = bpy.data.objects.new("ground", gm)
gmat = bpy.data.materials.new("ground")
gmat.use_nodes = True
gmat.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = (0.19, 0.21, 0.16, 1)
gmat.node_tree.nodes["Principled BSDF"].inputs["Roughness"].default_value = 1.0
gm.materials.append(gmat)
scene.collection.objects.link(ground)

# ---- mesh ----------------------------------------------------------------------------------------
mesh = bpy.data.meshes.new("creature")
rest = to_bl(lod.points)
# file tris are in the engine's front order for a left-handed space; the swap of y and z mirrors, so
# reverse them to keep the faces outward in Blender
tris = lod.tris[:, ::-1]
mesh.from_pydata([tuple(p) for p in rest], [], [tuple(int(v) for v in t) for t in tris])
uvl = mesh.uv_layers.new(name="uv")
uv = lod.tri_uv[:, ::-1, :].reshape(-1, 2).copy()
uv[:, 1] = 1.0 - uv[:, 1]
uvl.data.foreach_set("uv", uv.ravel())
texs = sorted(set(lod.tri_tex))
slot = {}
for t in texs:
    stem = os.path.splitext(os.path.basename(t.replace("\\", "/")))[0]
    mat = bpy.data.materials.new(stem)
    mat.use_nodes = True
    nt = mat.node_tree
    bsdf = nt.nodes["Principled BSDF"]
    bsdf.inputs["Roughness"].default_value = 1.0
    if "Specular IOR Level" in bsdf.inputs:
        bsdf.inputs["Specular IOR Level"].default_value = 0.0
    png = os.path.join(TEXDIR, stem + ".png")
    if not os.path.isfile(png):
        png = os.path.join(os.path.dirname(os.path.abspath(P3D)), stem + ".png")     # this build's own far sheets
    if os.path.isfile(png):
        img = nt.nodes.new("ShaderNodeTexImage")
        img.image = bpy.data.images.load(png)
        nt.links.new(img.outputs["Color"], bsdf.inputs["Base Color"])
        glow = any((lod.tri_flags[k] & 1) for k in range(len(lod.tri_tex)) if lod.tri_tex[k] == t)
        if glow:
            nt.links.new(img.outputs["Color"], bsdf.inputs["Emission Color"])
            bsdf.inputs["Emission Strength"].default_value = 1.6
    else:
        bsdf.inputs["Base Color"].default_value = (0.5, 0.45, 0.4, 1) if t else (0.6, 0.2, 0.6, 1)
    mat.use_backface_culling = False
    slot[t] = len(mesh.materials)
    mesh.materials.append(mat)
mesh.polygons.foreach_set("material_index", [slot[t] for t in lod.tri_tex])
mesh.polygons.foreach_set("use_smooth", [True] * len(mesh.polygons))
mesh.update()
obj = bpy.data.objects.new("creature", mesh)
scene.collection.objects.link(obj)

# ---- cameras -------------------------------------------------------------------------------------
cam = bpy.data.objects.new("cam", bpy.data.cameras.new("cam"))
scene.collection.objects.link(cam)
scene.camera = cam
cam.data.clip_end = 2000.0


def aim(pos, target, lens=35.0):
    cam.location = Vector(pos)
    d = Vector(target) - Vector(pos)
    cam.rotation_euler = d.to_track_quat("-Z", "Y").to_euler()
    cam.data.lens = lens


def cam_orbit(angle_deg, dist, height, target_h, lens=35.0):
    # angle 0 = dead ahead of him (he faces -Y), positive = round to his left (+X)
    a = math.radians(angle_deg)
    aim((dist * math.sin(a), -dist * math.cos(a), height), (0, 0, target_h), lens)


CAMS = {
    "quarter": lambda: cam_orbit(-35, 1.4 * H, 0.42 * H, 0.5 * H),       # front right quarter: right hand and thigh
    "quarterl": lambda: cam_orbit(35, 1.4 * H, 0.42 * H, 0.5 * H),
    "side": lambda: cam_orbit(-90, 1.4 * H, 0.42 * H, 0.5 * H),          # his right side
    "sidel": lambda: cam_orbit(90, 1.4 * H, 0.42 * H, 0.5 * H),
    "front": lambda: cam_orbit(0, 1.4 * H, 0.42 * H, 0.5 * H),
    "back": lambda: cam_orbit(180, 1.4 * H, 0.42 * H, 0.5 * H),
    "backq": lambda: cam_orbit(-145, 1.4 * H, 0.42 * H, 0.5 * H),        # behind his right: hip, tail
    "feet": lambda: cam_orbit(-60, 0.75 * H, 0.05 * H, 0.14 * H, 28.0),  # low, at the right foot
    "feetl": lambda: cam_orbit(60, 0.75 * H, 0.05 * H, 0.14 * H, 28.0),
    "hip": lambda: cam_orbit(-125, 1.0 * H, 0.42 * H, 0.42 * H, 40.0),   # close on the right hip from behind
    "waist": lambda: cam_orbit(-20, 1.1 * H, 0.5 * H, 0.52 * H, 40.0),   # close on the waist from in front
    "man": lambda: cam_orbit(-30, 1.6 * H, 1.7, 0.5 * H, 24.0),          # a man's eye height
}


def set_pose(pts_file):
    mesh.vertices.foreach_set("co", to_bl(pts_file).ravel())
    mesh.update()


def shot(name):
    scene.render.filepath = os.path.join(OUT, name + ".png")
    bpy.ops.render.render(write_still=True)


cams = [c for c in OPT["cams"].split(",") if c]
log = open(os.path.join(OUT, "motion_render.log"), "a", encoding="utf-8")
for mv in want:
    stem, phases = MOVES[mv]
    path = os.path.join(ANIM, stem + ".rtm")
    if not os.path.isfile(path):
        log.write("missing clip %s\n" % path)
        continue
    pose = pp.Pose(lod, pp.read_clip(path))
    for k, ph in enumerate(phases):
        set_pose(pose.at(ph))
        for c in cams:
            CAMS[c]()
            shot("%s_%s_%02d" % (mv, c, k))
    log.write("%s: %d phases x %d cameras\n" % (mv, len(phases), len(cams)))
    log.flush()

n_turn = int(OPT["turntable"])
if n_turn > 0:
    pose = pp.Pose(lod, pp.read_clip(os.path.join(ANIM, "ded_walk.rtm")))
    for k in range(n_turn):
        set_pose(pose.at((2.0 * k / n_turn) % 1.0))        # two walk cycles in one turn
        cam_orbit(-360.0 * k / n_turn, 1.45 * H, 0.42 * H, 0.5 * H)
        shot("turntable_%02d" % k)
    log.write("turntable: %d frames\n" % n_turn)
log.write("done\n")
log.close()
