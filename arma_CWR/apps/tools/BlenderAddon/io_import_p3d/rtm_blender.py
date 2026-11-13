"""Blender side of the RTM pipeline: stock man rig, RTM import and export, mesh binding.

Spaces. Blender (x, y, z) is game file space (x, z, y), the same swap the P3D
importer and exporter use. The swap is a mirror, so a matrix is conjugated,
never just copied: ``B = SWAP @ G @ SWAP`` and back. The conjugate of a
rotation is a rotation, so a Blender pose stays a proper (det +1) rotation in
the game. The rig object must stay at the origin, unrotated and unscaled:
armature space is game space.

Deformation. The engine skins with absolute matrices: ``v' = sum w_i M_i v``.
A Blender pose bone deforms with ``pose_bone.matrix @ bone.matrix_local^-1``
(armature space), which is exactly ``M_i`` after the swap. So export samples
the evaluated pose (constraints and drivers included) at every frame and
writes that product; import inverts it. Rest joint positions only decide
where bones pivot while animating; they never change what the game sees.

Entry points (headless): ``build_rig``, ``import_rtm``, ``export_rtm``,
``bind_mesh``. Operators wrap them for the UI.
"""

import math
import os

import bpy
from bpy.props import BoolProperty, EnumProperty, FloatVectorProperty, IntProperty, StringProperty
from bpy_extras.io_utils import ExportHelper, ImportHelper
from mathutils import Matrix, Vector

from . import rtm_io, rtm_rig

SWAP = Matrix(((1, 0, 0, 0), (0, 0, 1, 0), (0, 1, 0, 0), (0, 0, 0, 1)))
RIG_TAG = "ud_rtm_rig"
DEFAULT_FRAMES_PER_PHASE = 4


def default_rest_json():
    """Rest file from UD_RTM_REST_JSON, else empty (operators then ask for it)."""
    return os.environ.get("UD_RTM_REST_JSON", "")


def to_blender_point(p):
    return Vector((p[0], p[2], p[1]))


def rtm_to_blender(m):
    """12-float RTM matrix (game file space) to a Blender 4x4 (armature space)."""
    g = Matrix((
        (m[0], m[3], m[6], m[9]),
        (m[1], m[4], m[7], m[10]),
        (m[2], m[5], m[8], m[11]),
        (0.0, 0.0, 0.0, 1.0),
    ))
    return SWAP @ g @ SWAP


def blender_to_rtm(b):
    """Blender 4x4 (armature space) to a 12-float RTM matrix (game file space)."""
    g = SWAP @ b @ SWAP
    return tuple(float(g[i][j]) for j in range(4) for i in range(3))


def _ensure_object_mode(context):
    if context.object and context.object.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")


def find_rig(context=None, obj=None):
    context = context or bpy.context
    obj = obj or context.active_object
    if obj and obj.type == "ARMATURE":
        return obj
    if obj and obj.parent and obj.parent.type == "ARMATURE":
        return obj.parent
    if obj and obj.type == "MESH":
        for mod in obj.modifiers:
            if mod.type == "ARMATURE" and mod.object:
                return mod.object
    for o in context.scene.objects:
        if o.type == "ARMATURE" and o.data.get(RIG_TAG):
            return o
    return None


# ---------------------------------------------------------------------------
# Rig


def build_rig(rest_json=None, *, name="ud_man_rig", collection=None, context=None):
    """Armature with the 25 stock man bones at the rest joints from ``rest_json``.

    rest_json: path of the file written by ``rtm_rig.save_rest_json`` (or the
    loaded dict). Heads sit on the joints derived from the stock RTMs; bricho
    is the root. All bones are unconnected so any bone can also translate.
    Returns the armature object (at the origin, identity transform).
    """
    context = context or bpy.context
    rest_json = rest_json or default_rest_json()
    if not rest_json:
        raise ValueError("no rest file: pass rest_json or set UD_RTM_REST_JSON")
    rest = rest_json if isinstance(rest_json, dict) else rtm_rig.load_rest_json(rest_json)
    layout = rtm_rig.rest_layout(rest)
    _ensure_object_mode(context)
    arm = bpy.data.armatures.new(name)
    arm.display_type = "OCTAHEDRAL"
    obj = bpy.data.objects.new(name, arm)
    (collection or context.scene.collection).objects.link(obj)
    obj.show_in_front = True
    prev_active = context.view_layer.objects.active
    context.view_layer.objects.active = obj
    bpy.ops.object.mode_set(mode="EDIT")
    for b in rtm_rig.bones_parent_first():
        spec = layout[b]
        eb = arm.edit_bones.new(b)
        eb.head = to_blender_point(spec["head"])
        eb.tail = to_blender_point(spec["tail"])
        eb.align_roll(to_blender_point(spec["roll_ref"]))
        if spec["parent"]:
            eb.parent = arm.edit_bones[spec["parent"]]
        eb.use_connect = False
        eb.use_deform = True
    bpy.ops.object.mode_set(mode="OBJECT")
    for pb in obj.pose.bones:
        pb.rotation_mode = "QUATERNION"
    arm[RIG_TAG] = 1
    arm["ud_rest_source"] = rest_json if isinstance(rest_json, str) else "dict"
    if prev_active is not None:
        context.view_layer.objects.active = prev_active
    return obj


def bind_mesh(arm_obj, mesh_objs):
    """Deform meshes with the rig the way the engine skins them.

    Adds (or reuses) one Armature modifier per mesh, using vertex groups only
    (they are named after the stock bones, like the engine's named
    selections), no envelopes and no volume preservation (the engine blends
    linearly). Returns {mesh name: {"bones": n matched, "unweighted": n verts}}.
    """
    bones = {b.name.lower() for b in arm_obj.data.bones}
    report = {}
    for obj in mesh_objs:
        if obj.type != "MESH":
            continue
        mod = None
        for m in obj.modifiers:
            if m.type == "ARMATURE":
                if mod is None:
                    mod = m
                else:
                    m.show_viewport = m.show_render = False
        if mod is None:
            mod = obj.modifiers.new(name="UD RTM rig", type="ARMATURE")
        mod.object = arm_obj
        mod.use_vertex_groups = True
        mod.use_bone_envelopes = False
        mod.use_deform_preserve_volume = False
        mod.use_multi_modifier = False
        mod.show_viewport = mod.show_render = True
        idx = list(obj.modifiers).index(mod)
        if idx != 0:
            obj.modifiers.move(idx, 0)
        matched = {vg.index for vg in obj.vertex_groups if vg.name.lower() in bones}
        unweighted = sum(1 for v in obj.data.vertices
                         if not any(g.group in matched and g.weight > 0 for g in v.groups))
        report[obj.name] = {"bones": len(matched), "unweighted": unweighted}
    if any(abs(v) > 1e-9 for row in (arm_obj.matrix_world - Matrix.Identity(4)) for v in row):
        print("[RTM] warning: the rig object is not at identity; armature space is game space only at identity")
    return report


# ---------------------------------------------------------------------------
# Import


def _fcurve(action, path, index, group, cache):
    key = (path, index)
    fc = cache.get(key)
    if fc is None:
        fc = action.fcurves.find(path, index=index) or action.fcurves.new(path, index=index, action_group=group)
        cache[key] = fc
    return fc


def import_rtm(arm_obj, source, *, action_name=None, frame_start=1, cycle_frames=None,
               frames_per_phase=DEFAULT_FRAMES_PER_PHASE, looped=True, set_scene_range=True, context=None):
    """Read an RTM into a new Action on ``arm_obj`` and assign it.

    Phase time t lands on frame ``frame_start + t * cycle_frames``
    (cycle_frames defaults to (phases - 1) * frames_per_phase, so evenly
    spaced stock phases fall on whole frames). Keys are linear, like the
    engine's blend. Bones in the file but not in the rig are skipped; rig
    bones missing from the file stay at rest (identity), as in the engine.
    The step and the cycle range are stored on the Action for export.
    Returns the Action; ``action["rtm_import_max_error"]`` is the largest
    distance (m) between the RTM matrix and the keyed pose over a 1 m probe
    (non-zero only for stock matrices with scale or shear).
    """
    context = context or bpy.context
    anim = source if isinstance(source, rtm_io.RtmAnimation) else rtm_io.read_rtm(source)
    errors, warnings = rtm_io.check_rtm(anim)
    for w in warnings[:5]:
        print(f"[RTM] warning: {w}")
    if errors:
        raise rtm_io.RtmError("; ".join(errors))
    n = anim.frame_count
    if cycle_frames is None:
        cycle_frames = max(1, (n - 1) * frames_per_phase)
    if action_name is None:
        action_name = os.path.splitext(os.path.basename(source))[0] if isinstance(source, str) else "rtm"
    rig_bones = {b.name.lower(): b for b in arm_obj.data.bones}
    file_idx = {nm.lower(): i for i, nm in enumerate(anim.bone_names)}
    skipped = [nm for nm in anim.bone_names if nm.lower() not in rig_bones]
    if skipped:
        print(f"[RTM] bones not in the rig, skipped: {skipped}")

    action = bpy.data.actions.new(action_name)
    if arm_obj.animation_data is None:
        arm_obj.animation_data_create()
    arm_obj.animation_data.action = action
    for pb in arm_obj.pose.bones:
        pb.rotation_mode = "QUATERNION"

    order = [b for b in _parent_first(arm_obj)]
    rest = {b.name: b.matrix_local.copy() for b in arm_obj.data.bones}
    rest_inv = {k: v.inverted() for k, v in rest.items()}
    cache = {}
    prev_q = {}
    keys = {}  # bone -> list of (frame, loc, quat, scale)
    max_err = 0.0
    for fr in anim.frames:
        frame = frame_start + fr.time * cycle_frames
        deform = {}
        for b in order:
            i = file_idx.get(b.lower())
            deform[b] = rtm_to_blender(fr.matrices[i]) if i is not None else Matrix.Identity(4)
        # Key each bone against the pose its parent actually gets from its own
        # (decomposed) keys, so a decomposition error does not propagate down.
        posed = {}
        for b in order:
            bone = arm_obj.data.bones[b]
            d = deform[b]
            target = d @ rest[b]  # pose_bone.matrix wanted, armature space
            if bone.parent:
                p = bone.parent.name
                local = (posed[p] @ rest_inv[p] @ rest[b]).inverted_safe() @ target
            else:
                local = rest_inv[b] @ target
            loc, quat, scale = local.decompose()
            pq = prev_q.get(b)
            if pq is not None and pq.dot(quat) < 0:
                quat.negate()
            prev_q[b] = quat.copy()
            keys.setdefault(b, []).append((frame, loc, quat, scale))
            basis = Matrix.LocRotScale(loc, quat, scale)
            if bone.parent:
                posed[b] = posed[p] @ rest_inv[p] @ rest[b] @ basis
            else:
                posed[b] = rest[b] @ basis
            # error of the keyed pose against the file, on a 1 m probe frame
            got = posed[b] @ rest_inv[b]
            for probe in ((0, 0, 0), (1, 0, 0), (0, 1, 0), (0, 0, 1)):
                v = Vector(probe)
                max_err = max(max_err, ((got @ v) - (d @ v)).length)

    for b, ks in keys.items():
        group = b
        path = f'pose.bones["{b}"]'
        need_scale = any(abs(s - 1.0) > 1e-5 for _, _, _, sc in ks for s in sc)
        channels = [("location", 3, lambda k: k[1]), ("rotation_quaternion", 4, lambda k: k[2])]
        if need_scale:
            channels.append(("scale", 3, lambda k: k[3]))
        for prop, count, get in channels:
            for ci in range(count):
                fc = _fcurve(action, f"{path}.{prop}", ci, group, cache)
                fc.keyframe_points.add(len(ks))
                flat = []
                for k in ks:
                    flat += [k[0], get(k)[ci]]
                fc.keyframe_points.foreach_set("co", flat)
                for kp in fc.keyframe_points:
                    kp.interpolation = "LINEAR"
                fc.update()

    action["rtm_step"] = list(anim.step)
    action["rtm_frame_start"] = float(frame_start)
    action["rtm_frame_end"] = float(frame_start + cycle_frames)
    action["rtm_looped"] = bool(looped)
    action["rtm_source"] = source if isinstance(source, str) else ""
    action["rtm_import_max_error"] = float(max_err)
    action.use_fake_user = True
    if set_scene_range:
        scene = context.scene
        scene.frame_start = int(math.floor(frame_start))
        scene.frame_end = int(math.ceil(frame_start + cycle_frames))
    return action


def _parent_first(arm_obj):
    out = []
    names = [b.name for b in arm_obj.data.bones]
    pending = list(names)
    while pending:
        for b in list(pending):
            bone = arm_obj.data.bones[b]
            if bone.parent is None or bone.parent.name in out:
                out.append(b)
                pending.remove(b)
    return out


# ---------------------------------------------------------------------------
# Export


def _key_frames(action, fs, fe):
    frames = {fs, fe}
    for fc in action.fcurves:
        for kp in fc.keyframe_points:
            x = kp.co[0]
            if fs - 1e-6 <= x <= fe + 1e-6:
                frames.add(round(x, 6))
    return sorted(frames)


def sample_pose(arm_obj, frame, context=None, bones=rtm_io.STOCK_MAN_BONES):
    """Evaluated deformation matrices (12-float RTM, game space) of ``bones`` at ``frame``."""
    context = context or bpy.context
    scene = context.scene
    whole = int(math.floor(frame))
    scene.frame_set(whole, subframe=frame - whole)
    depsgraph = context.evaluated_depsgraph_get()
    ev = arm_obj.evaluated_get(depsgraph)
    out = []
    for b in bones:
        pb = ev.pose.bones.get(b)
        if pb is None:
            raise rtm_io.RtmError(f"rig has no bone {b!r}")
        out.append(blender_to_rtm(pb.matrix @ pb.bone.matrix_local.inverted()))
    return out


def export_rtm(filepath, arm_obj, action=None, *, frame_start=None, frame_end=None, step=None,
               looped=None, sample="frames", bones=rtm_io.STOCK_MAN_BONES, context=None, verify=True):
    """Write ``action`` (default: the rig's current Action) as an RTM.

    The cycle runs from frame_start (time 0) to frame_end (time 1), both
    written; defaults come from the Action's rtm_frame_start / rtm_frame_end
    (set by import) or its frame range. sample="frames" writes every whole
    frame, "keys" writes the keyed frames only. step defaults to the Action's
    rtm_step (game file space, metres per cycle; walking forward is negative
    Z); step="feet" measures it from the planted feet instead. With verify
    the bytes are read back strictly, and any mirrored matrix,
    unsorted time or missing bone raises RtmError before the file is written.
    Returns a summary dict.
    """
    context = context or bpy.context
    scene = context.scene
    if arm_obj.animation_data is None:
        arm_obj.animation_data_create()
    old_action = arm_obj.animation_data.action
    action = action or old_action
    if action is None:
        raise rtm_io.RtmError("the rig has no Action")
    fs = frame_start if frame_start is not None else action.get("rtm_frame_start", action.frame_range[0])
    fe = frame_end if frame_end is not None else action.get("rtm_frame_end", action.frame_range[1])
    fs, fe = float(fs), float(fe)
    if fe <= fs:
        raise rtm_io.RtmError(f"frame_end {fe} must be after frame_start {fs}")
    if step is None:
        step = tuple(action.get("rtm_step", (0.0, 0.0, 0.0)))
    if looped is None:
        looped = bool(action.get("rtm_looped", True))
    if sample == "keys":
        frames = _key_frames(action, fs, fe)
    else:
        frames = [float(f) for f in range(int(math.ceil(fs)), int(math.floor(fe)) + 1)]
        if frames[0] != fs:
            frames.insert(0, fs)
        if frames[-1] != fe:
            frames.append(fe)
    old_frame = (scene.frame_current, scene.frame_subframe)
    arm_obj.animation_data.action = action
    try:
        out_frames = []
        for f in frames:
            t = (f - fs) / (fe - fs)
            out_frames.append((t, sample_pose(arm_obj, f, context, bones)))
    finally:
        arm_obj.animation_data.action = old_action
        scene.frame_set(old_frame[0], subframe=old_frame[1])
    if isinstance(step, str) and step == "feet":
        # planted feet slide by -step per cycle (rtm_engine.estimate_step)
        from . import rtm_engine
        rest = rtm_rig.load_rest_json(arm_obj.data.get("ud_rest_source") or default_rest_json())
        probe = rtm_io.new_animation(bones, out_frames)
        est = rtm_engine.estimate_step(probe, rtm_rig.man_foot_contacts(rest))
        if est is None:
            raise rtm_io.RtmError("step='feet': no planted foot found")
        step = (round(est[0], 4), 0.0, round(est[2], 4))
    anim = rtm_io.new_animation(bones, out_frames, step)
    errors, warnings = rtm_io.check_rtm(anim, require_bones=rtm_io.ENGINE_REQUIRED_MAN_BONES, looped=looped)
    if errors and verify:
        raise rtm_io.RtmError("; ".join(errors))
    data = rtm_io.write_rtm(anim, None, verify=verify, looped=looped)
    if verify:
        rtm_io.read_rtm_strict(data)
    with open(filepath, "wb") as fh:
        fh.write(data)
    # A looped move never plays its t = 1 phase (RemoveLoopFrame): the engine
    # blends the last kept phase back into phase 0, so the end pose must match the start.
    gap = 0.0
    for m0, m1 in zip(out_frames[0][1], out_frames[-1][1]):
        for probe in ((0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0)):
            gap = max(gap, math.dist(rtm_io.mat_apply(m0, probe), rtm_io.mat_apply(m1, probe)))
    if looped and gap > 0.005:
        warnings = list(warnings) + [f"looped move: the last frame differs from the first by {gap * 1000:.0f} mm "
                                     "(1 m probe); the engine skips the last frame and blends into the first"]
    ortho = max(rtm_io.mat_orthonormal_error(m) for _, ms in out_frames for m in ms)
    det_min = min(rtm_io.mat_det(m) for _, ms in out_frames for m in ms)
    return {
        "file": filepath,
        "bytes": len(data),
        "phases": len(out_frames),
        "frames": (fs, fe),
        "step": tuple(step),
        "looped": looped,
        "max_orthonormal_error": ortho,
        "min_det": det_min,
        "loop_gap_m": gap,
        "warnings": warnings,
    }


# ---------------------------------------------------------------------------
# Operators and panel


class POSEIDON_OT_rtm_build_rig(bpy.types.Operator):
    """Build the stock man rig (25 bones at the joints derived from the stock RTMs)"""

    bl_idname = "poseidon.rtm_build_rig"
    bl_label = "Build Stock Man Rig"
    bl_options = {"REGISTER", "UNDO"}

    rest_json: StringProperty(name="Rest File", subtype="FILE_PATH", default="",
                              description="stock_rest.json from rtm_rig.save_rest_json (UD_RTM_REST_JSON if empty)")

    def invoke(self, context, event):
        if not self.rest_json:
            self.rest_json = default_rest_json()
        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):
        try:
            obj = build_rig(bpy.path.abspath(self.rest_json) if self.rest_json else None, context=context)
        except Exception as e:  # noqa: BLE001
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        context.view_layer.objects.active = obj
        self.report({"INFO"}, f"Built {obj.name}")
        return {"FINISHED"}


class POSEIDON_OT_rtm_bind_mesh(bpy.types.Operator):
    """Deform the selected meshes with the active rig (vertex groups named after the stock bones)"""

    bl_idname = "poseidon.rtm_bind_mesh"
    bl_label = "Bind Selected Meshes to Rig"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        rig = find_rig(context)
        if rig is None:
            self.report({"ERROR"}, "No rig: build one or make it active")
            return {"CANCELLED"}
        meshes = [o for o in context.selected_objects if o.type == "MESH"]
        if not meshes:
            self.report({"ERROR"}, "Select the meshes to bind")
            return {"CANCELLED"}
        rep = bind_mesh(rig, meshes)
        msg = "; ".join(f"{k}: {v['bones']} bones, {v['unweighted']} unweighted" for k, v in rep.items())
        self.report({"INFO"}, msg)
        return {"FINISHED"}


class IMPORT_ANIM_OT_rtm(bpy.types.Operator, ImportHelper):
    """Import a Poseidon RTM as an Action on the stock man rig"""

    bl_idname = "import_anim.rtm"
    bl_label = "Import RTM"
    bl_options = {"REGISTER", "UNDO"}
    filename_ext = ".rtm"
    filter_glob: StringProperty(default="*.rtm", options={"HIDDEN"})

    frame_start: IntProperty(name="Start Frame", default=1)
    frames_per_phase: IntProperty(name="Frames per Phase", default=DEFAULT_FRAMES_PER_PHASE, min=1,
                                  description="Cycle length = (phases - 1) x this, unless Cycle Frames is set")
    cycle_frames: IntProperty(name="Cycle Frames", default=0, min=0, description="0 = automatic")
    looped: BoolProperty(name="Looped Move", default=True)

    def execute(self, context):
        rig = find_rig(context)
        if rig is None:
            self.report({"ERROR"}, "No rig: build the stock man rig first")
            return {"CANCELLED"}
        try:
            act = import_rtm(rig, self.filepath, frame_start=self.frame_start,
                             cycle_frames=self.cycle_frames or None, frames_per_phase=self.frames_per_phase,
                             looped=self.looped, context=context)
        except Exception as e:  # noqa: BLE001
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Imported {act.name}: frames {act['rtm_frame_start']:.0f}-{act['rtm_frame_end']:.0f}")
        return {"FINISHED"}


class EXPORT_ANIM_OT_rtm(bpy.types.Operator, ExportHelper):
    """Export the rig's Action as a Poseidon RTM (RTM_0101)"""

    bl_idname = "export_anim.rtm"
    bl_label = "Export RTM"
    filename_ext = ".rtm"
    filter_glob: StringProperty(default="*.rtm", options={"HIDDEN"})

    use_action_range: BoolProperty(name="Use Action Cycle Range", default=True,
                                   description="rtm_frame_start/end on the Action, else the fields below")
    frame_start: IntProperty(name="Start Frame (time 0)", default=1)
    frame_end: IntProperty(name="End Frame (time 1)", default=33)
    use_action_step: BoolProperty(name="Use Action Step", default=True)
    step: FloatVectorProperty(name="Step (game x, y, z per cycle)", size=3, default=(0.0, 0.0, 0.0),
                              description="Walking forward is negative Z")
    looped: BoolProperty(name="Looped Move", default=True)
    sample: EnumProperty(name="Sample", items=(("frames", "Every Frame", ""), ("keys", "Keyed Frames", "")),
                         default="frames")

    def execute(self, context):
        rig = find_rig(context)
        if rig is None:
            self.report({"ERROR"}, "No rig")
            return {"CANCELLED"}
        act = rig.animation_data.action if rig.animation_data else None
        kw = {}
        if not (self.use_action_range and act is not None and "rtm_frame_start" in act):
            kw["frame_start"], kw["frame_end"] = self.frame_start, self.frame_end
        if not (self.use_action_step and act is not None and "rtm_step" in act):
            kw["step"] = tuple(self.step)
        try:
            res = export_rtm(self.filepath, rig, looped=self.looped, sample=self.sample, context=context, **kw)
        except Exception as e:  # noqa: BLE001
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Wrote {res['phases']} phases to {os.path.basename(self.filepath)}")
        return {"FINISHED"}


class POSEIDON_PT_rtm(bpy.types.Panel):
    bl_label = "RTM Animation"
    bl_idname = "POSEIDON_PT_rtm"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Poseidon"
    bl_order = 8

    def draw(self, context):
        layout = self.layout
        col = layout.column(align=True)
        col.operator(POSEIDON_OT_rtm_build_rig.bl_idname, icon="ARMATURE_DATA")
        col.operator(POSEIDON_OT_rtm_bind_mesh.bl_idname, icon="MOD_ARMATURE")
        col.operator(IMPORT_ANIM_OT_rtm.bl_idname, text="Import RTM...", icon="IMPORT")
        col.operator(EXPORT_ANIM_OT_rtm.bl_idname, text="Export RTM...", icon="EXPORT")
        rig = find_rig(context)
        act = rig.animation_data.action if rig and rig.animation_data else None
        if act is not None:
            box = layout.box()
            box.label(text=f"Action: {act.name}", icon="ACTION")
            for key in ("rtm_frame_start", "rtm_frame_end", "rtm_step", "rtm_looped"):
                if key in act:
                    box.prop(act, f'["{key}"]', text=key[4:])


rtm_classes = (
    POSEIDON_OT_rtm_build_rig,
    POSEIDON_OT_rtm_bind_mesh,
    IMPORT_ANIM_OT_rtm,
    EXPORT_ANIM_OT_rtm,
    POSEIDON_PT_rtm,
)


def menu_func_import(self, context):
    self.layout.operator(IMPORT_ANIM_OT_rtm.bl_idname, text="Poseidon RTM Animation (.rtm)")


def menu_func_export(self, context):
    self.layout.operator(EXPORT_ANIM_OT_rtm.bl_idname, text="Poseidon RTM Animation (.rtm)")
