"""Debug override 2: per-direction views.

skyDebug 10+i: direction i alone, through the same two-fetch cone path:
  R = 1 - occlusion (after the bleed cut), G = coverage at the search lod,
  B = t (receiver to mean blocker, m) / 4.
skyDebug 9: number of directions above the receiver's plane / count.
"""
from pathlib import Path

D = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\wildcard")
src = (D / "glsl_dump" / "psRemasterCommon.glsl").read_text()
old = """    if (skyCtl3.x > 0.5 && skyCtl.x > 0.5) {"""
new = """    if (skyCtl3.x > 8.5 && skyCtl.x > 0.5) {
        vec3 rel = vWorldRel + litShadowDelta.xyz;
        float texel = skyCtl.z;
        float lodMax = max(skyCtl3.w - 1.0, 0.0);
        vec3 p = rel + nSky * (terrain ? 0.05 : 1.5 * texel + 0.05) + lookOffset;
        if (skyCtl3.x > 39.5) {
            // 40 + L: coverage of direction 1 at fixed lod L (R), its mean depth - z (G, /8 + 0.5)
            int dd = 1;
            float L = skyCtl3.x - 40.0;
            vec3 pl0 = p + skyOff[dd].xyz;
            vec4 lp0 = skyVP[dd] * vec4(pl0, 1.0);
            vec2 uv0 = lp0.xy * 0.5 + 0.5;
            vec4 mm = textureLod(skyMoments, vec3(uv0, float(dd)), L);
            float z0 = -dot(pl0, skyDir[dd].xyz);
            float mu = mm.x > 1e-4 ? mm.y / mm.x : z0;
            return vec3(mm.x, clamp((z0 - mu) / 8.0 + 0.5, 0.0, 1.0), mm.w);
        }
        if (skyCtl3.x < 9.5) {
            int cnt = 0;
            for (int k = 0; k < 24; ++k) { if (k >= int(skyCtl.x + 0.5)) break; if (dot(nSky, skyDir[k].xyz) > 0.0) cnt++; }
            return vec3(float(cnt) / skyCtl.x);
        }
        int i = int(skyCtl3.x + 0.5) - 10;
        vec3 d = skyDir[i].xyz;
        vec3 pl = p + skyOff[i].xyz;
        vec4 lp = skyVP[i] * vec4(pl, 1.0);
        vec2 uv = lp.xy * 0.5 + 0.5;
        if (uv.x < 0.0 || uv.y < 0.0 || uv.x > 1.0 || uv.y > 1.0) return vec3(1.0, 0.0, 1.0);
        float z = -dot(pl, d);
        float searchLod = clamp(log2(6.0 / texel), 0.0, lodMax);
        float nearLod = clamp(log2(1.0 / texel), 0.0, lodMax);
        vec4 m0 = textureLod(skyMoments, vec3(uv, float(i)), nearLod);
        if (m0.x < 0.02) m0 = textureLod(skyMoments, vec3(uv, float(i)), searchLod);
        float c1 = 0.0;
        float vis = 1.0;
        float t = 0.0;
        if (m0.x > 1e-3) {
            t = max(z - m0.y / m0.x, 0.0);
            float radius = max(t * skyCtl2.y, 0.5 * texel);
            float lod = clamp(log2(2.0 * radius / texel), 0.0, lodMax);
            vec4 m1 = textureLod(skyMoments, vec3(uv, float(i)), lod);
            c1 = m1.x;
            float occ = skyOccluded(m1, z);
            occ = clamp((occ - skyCtl2.x) / (1.0 - skyCtl2.x), 0.0, 1.0);
            vis = 1.0 - occ;
        }
        return vec3(vis, m0.x, c1);
    }
    if (skyCtl3.x > 0.5 && skyCtl.x > 0.5) {"""
assert old in src
src = src.replace(old, new, 1)
out = D / "ovr_dbg2"
out.mkdir(exist_ok=True)
(out / "psRemasterCommon.glsl").write_text(src)
print("ok")
