"""Debug override: extra sky-map debug views on top of the built-in GLSL.

skyDebug 3: raw occlusion of the most vertical direction (lod 0)
skyDebug 4: coverage of that direction at lod 0 (white = empty)
skyDebug 5: that direction's coverage split: red = solid casters, green = foliage casters
skyDebug 6: receiver class: green = foliage material (forced diffuse), red = crown frame present, blue = other
"""
from pathlib import Path

D = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\wildcard")
src = (D / "glsl_dump" / "psRemasterCommon.glsl").read_text()
old = """    if (skyCtl3.x > 0.5 && skyCtl.x > 0.5) {
        // wildcard debug: 1 = sky visibility through the objects, 2 = bounce."""
new = """    if (skyCtl3.x > 5.5 && skyCtl.x > 0.5) {
        return vec3(vCrown.w > 0.0 ? 1.0 : 0.0, foliage ? 1.0 : 0.0, foliage ? 0.0 : 1.0);
    }
    if (skyCtl3.x > 2.5 && skyCtl.x > 0.5) {
        int i = int(skyCtl.x + 0.5) - 1;
        vec3 rel = vWorldRel + litShadowDelta.xyz;
        vec3 d = skyDir[i].xyz;
        vec3 p = rel + n * (1.5 * skyCtl.z + 0.05) + skyOff[i].xyz;
        vec4 lp = skyVP[i] * vec4(p, 1.0);
        vec2 uv = lp.xy * 0.5 + 0.5;
        if (uv.x < 0.0 || uv.y < 0.0 || uv.x > 1.0 || uv.y > 1.0) return vec3(1.0, 0.0, 1.0);
        vec4 m = textureLod(skyMoments, vec3(uv, float(i)), 0.0);
        if (skyCtl3.x > 4.5) return vec3(m.x - m.w, m.w, 0.0);
        if (skyCtl3.x > 3.5) return vec3(1.0 - m.x);
        float z = -dot(p, d);
        return vec3(1.0 - skyOccluded(m, z));
    }
    if (skyCtl3.x > 0.5 && skyCtl.x > 0.5) {
        // wildcard debug: 1 = sky visibility through the objects, 2 = bounce."""
assert old in src
src = src.replace(old, new, 1)
out = D / "ovr_dbg"
out.mkdir(exist_ok=True)
(out / "psRemasterCommon.glsl").write_text(src)
print("ok")
