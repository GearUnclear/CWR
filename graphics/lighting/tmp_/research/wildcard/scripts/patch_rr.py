"""One-off source edit for the scratch worktree: round-robin sky map updates."""
from pathlib import Path

GL = Path(r"D:\Arma_CWA_worktrees\gfx-wildcard\arma_CWR\engine\PoseidonGL33")
p = GL / "EngineGL33_ShadowDepth.cpp"
s = p.read_text(encoding="utf-8")
lines = s.split("\n")
start = [i for i, l in enumerate(lines) if l.startswith("bool RenderSkyMaps")][0]
cstart = start
while lines[cstart - 1].startswith("//"):
    cstart -= 1
end = [i for i, l in enumerate(lines) if l.startswith("} // namespace") and i > start][0]
new_fn = (Path(__file__).parent / "rr_fn.cpp.txt").read_text(encoding="utf-8")
lines[cstart:end] = new_fn.rstrip("\n").split("\n")
s = "\n".join(lines)

a = s.index("    s_skyRendered = false;\n    if (rendered && _lightingActive")
b = s.index("    GpuMark(Poseidon::gpu::Mark::ShadowEnd);", a)
call = (Path(__file__).parent / "rr_call.cpp.txt").read_text(encoding="utf-8")
s = s[:a] + call + s[b:]
s = s.replace("bool s_skyRendered = false;", "bool s_skyRendered = false;\nbool s_skyEverRendered = false;", 1)
old = "    float dir[kSkyMaxDirs * 4];"
new = "    float dir[kSkyMaxDirs * 4];\n    float off[kSkyMaxDirs * 4]; // this pass's camera minus the camera the layer was drawn from"
assert old in s
s = s.replace(old, new, 1)
p.write_text(s, encoding="utf-8")

p = GL / "EngineGL33_LightingShaders.cpp"
s = p.read_text(encoding="utf-8")
old = "    vec4 skyDir[24];   // xyz toward the sky, w solid-angle weight"
new = old + "\n    vec4 skyOff[24];   // xyz: this pass's camera minus the camera the layer was drawn from"
assert old in s
s = s.replace(old, new, 1)
old = """        vec4 lp = skyVP[i] * vec4(p, 1.0);
        vec2 uv = lp.xy * 0.5 + 0.5;
        if (uv.x > 0.0 && uv.x < 1.0 && uv.y > 0.0 && uv.y < 1.0) {
            float z = -dot(p, d);"""
new = """        vec3 pl = p + skyOff[i].xyz;
        vec4 lp = skyVP[i] * vec4(pl, 1.0);
        vec2 uv = lp.xy * 0.5 + 0.5;
        if (uv.x > 0.0 && uv.x < 1.0 && uv.y > 0.0 && uv.y < 1.0) {
            float z = -dot(pl, d);"""
assert old in s
s = s.replace(old, new, 1)
p.write_text(s, encoding="utf-8")
print("ok")
