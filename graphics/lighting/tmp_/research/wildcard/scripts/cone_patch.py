"""Replace the single-blocker estimate with a cone march over the mip chain.

    python cone_patch.py override   -> ovr_cone/psRemasterCommon.glsl from glsl_dump
    python cone_patch.py source     -> patches the scratch worktree C++ source
"""
import sys
from pathlib import Path

D = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\wildcard")
SRC = Path(r"D:\Arma_CWA_worktrees\gfx-wildcard\arma_CWR\engine\PoseidonGL33\EngineGL33_LightingShaders.cpp")

START = "            // Blocker search, nearest first: a 1 m footprint finds a contact"
END = """                    sumB += alb * eOcc * (cw * occ);
                }
            }
"""

NEW = """            // Cone march down the mip chain. Every point of the ray p + d * t
            // projects to the same texel of this orthographic map, so tracing
            // the direction's cone is a walk over footprints that grow with t:
            // step k reads the footprint of the cone's radius at t_k and counts
            // the occluders whose mean depth puts them near t_k, front to back.
            float trans = 1.0;   // share of the cone still open
            float leafAcc = 0.0; // share blocked by foliage (before transmission)
            float tHit = -1.0;   // distance of the first real occluder
            float lodHit = 0.0;
            float tk = 0.25;
            for (int k = 0; k < 6; ++k) {
                float rk = max(tk * skyCtl2.y, 0.5 * texel);
                float lodk = clamp(log2(2.0 * rk / texel), 0.0, lodMax);
                vec4 m = textureLod(skyMoments, vec3(uv, float(i)), lodk);
                if (m.x > 1e-3) {
                    float dk = z - m.y / m.x;
                    float win = smoothstep(0.03, 0.08 + 0.25 * tk, dk) * (1.0 - smoothstep(2.0 * tk, 4.0 * tk, dk));
                    float a = clamp(m.x, 0.0, 1.0) * win;
                    float leaf = clamp(m.w / m.x, 0.0, 1.0);
                    if (tHit < 0.0 && a > 0.1) { tHit = dk; lodHit = lodk; }
                    leafAcc += trans * a * leaf;
                    trans *= 1.0 - a * (1.0 - skyCtl4.x * leaf);
                }
                tk *= 3.0;
            }
            float occ = clamp(((1.0 - trans) - skyCtl2.x) / (1.0 - skyCtl2.x), 0.0, 1.0);
            vis = 1.0 - occ;
            if (tHit >= 0.0) {
                float lod = lodHit;
                // The light foliage lets through takes the leaves' colour
                // (skyCtl3.z: from the caster textures): green shade in woods.
                if (skyCtl4.w > 0.0 && skyCtl3.z > 0.5 && leafAcc > 0.0) {
                    vec4 la = textureLod(skyAlbedo, vec3(uv, float(i)), lod);
                    if (la.a > 1e-3) {
                        vec3 lc = toLinear(la.rgb / la.a);
                        vec3 tn = lc / max(dot(lc, kLuma), 1e-4);
                        float tr = min(skyCtl4.x * leafAcc, vis);
                        visTint = tr * (mix(vec3(1.0), clamp(tn, vec3(0.0), vec3(3.0)), skyCtl4.w) - vec3(1.0));
                    }
                }
                if (skyCtl.w > 0.0 && occ > 0.0) {
                    vec3 q = p + d * max(tHit - 0.3, 0.0);
                    float sv = shadowVisAt(q);
                    vec3 nOcc = normalize(vec3(-d.x, -0.3 * d.y, -d.z));
                    float sunCos = clamp((dot(nOcc, litSunDir.xyz) + 0.3) / 1.3, 0.0, 1.0);
                    vec3 eOcc = litSun.rgb * (sunCos * sv * litSunDir.w) + skyIrradiance(nOcc);
                    vec3 alb = vec3(0.16, 0.16, 0.14);
                    if (skyCtl3.z > 0.5) {
                        vec4 a = textureLod(skyAlbedo, vec3(uv, float(i)), lod);
                        if (a.a > 1e-3) alb = toLinear(a.rgb / a.a);
                    }
                    sumB += alb * eOcc * (cw * occ);
                }
            }
"""


def patch(text):
    a = text.index(START)
    b = text.index(END, a) + len(END)
    return text[:a] + NEW + text[b:]


if __name__ == "__main__":
    mode = sys.argv[1]
    if mode == "override":
        s = (D / "glsl_dump" / "psRemasterCommon.glsl").read_text()
        out = D / "ovr_cone"
        out.mkdir(exist_ok=True)
        (out / "psRemasterCommon.glsl").write_text(patch(s))
    else:
        s = SRC.read_text(encoding="utf-8")
        SRC.write_text(patch(s), encoding="utf-8")
    print("ok", mode)
