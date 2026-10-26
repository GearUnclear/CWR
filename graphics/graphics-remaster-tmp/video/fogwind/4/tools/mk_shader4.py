"""Agent 4 (fogwind): psFogInject overrides that integrate each cell's depth span with fixed taps.

python mk_shader4.py <variant> --taps N [--xy lattice|centre] [--share S]

Base: tmp/video/mist/3/shaders/percell/psFogInject.glsl (= the d91a08e0 shader, assembled).
Replaces fogSlice and main's jitter lines. No temporal jitter at all: each cell's value is
the metric-weighted mean of N taps, tap i at its own depth stratum of the slice and its own
fixed xy point in the cell (a 3D rank-1 lattice, the same in every cell and every frame).
The history blend is kept as a smoother (share = fogFrame.z, or --share S when history is valid).
"""
import argparse
import os

BASE = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/mist/3/shaders/percell/psFogInject.glsl"
OUT = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/4/shaders"

LATTICE = {
    1: ([(0.0, 0.0)], [0]),
    2: ([(-0.25, -0.25), (0.25, 0.25)], [0, 1]),
    4: ([(-0.375, -0.125), (0.125, -0.375), (0.375, 0.125), (-0.125, 0.375)], [0, 2, 1, 3]),
    8: ([((i + 0.5) / 8 - 0.5, ((3 * i) % 8 + 0.5) / 8 - 0.5) for i in range(8)], [(5 * i) % 8 for i in range(8)]),
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("variant")
    ap.add_argument("--taps", type=int, default=4)
    ap.add_argument("--xy", default="lattice", choices=["lattice", "centre"])
    ap.add_argument("--share", type=float, default=-1.0)
    ap.add_argument("--pattern", default="box", choices=["box", "tent"])
    ap.add_argument("--hblur", action="store_true", help="history read through agent 2's fixed tetrahedral +-0.25 blur (Act25)")
    a = ap.parse_args()
    n = a.taps
    xy, zs = LATTICE[n]
    zf = [(z + 0.5) / n for z in zs]
    if a.pattern == "tent":
        # stratified quantiles of a tent on [-1, 1] cell (and slice): the deterministic
        # analogue of the shipped history's blur; equal weights, metric Jacobian in depth.
        import math
        def q(u):
            return -1.0 + math.sqrt(2.0 * u) if u < 0.5 else 1.0 - math.sqrt(2.0 * (1.0 - u))
        xy = [(q(x + 0.5), q(y + 0.5)) for x, y in xy]
        zf = [0.5 + q(z) for z in zf]
    if a.xy == "centre":
        xy = [(0.0, 0.0)] * n
    src = open(BASE, encoding="utf-8").read()
    i0 = src.index("// The cell of slice s on this column")
    i1 = src.index("    vec3 dirZ = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0);")
    xy_s = ", ".join(f"vec2({x:.6f}, {y:.6f})" for x, y in xy)
    z_s = ", ".join(f"{z:.6f}" for z in zf)
    share = "fogFrame.z" if a.share < 0 else f"(fogFrame.z < 1.0 ? {a.share:.4f} : 1.0)"
    new = f"""// Agent 4 (fogwind) variant {a.variant}: each cell integrates its whole depth span with
// {n} fixed taps (metric weights), tap i at its own depth stratum and its own fixed point
// of the cell; no temporal jitter. The history is a smoother only.
const int KT = {n};
const vec2 kTapXY[KT] = vec2[KT]({xy_s});
const float kTapZ[KT] = float[KT]({z_s});
vec3 g_tapDir[KT];
vec4 fogSlice(float s, vec3 dirZ) {{
    vec4 acc = vec4(0.0);
    float wsum = 0.0;
    const float h = 0.5 / float(KT);
    for (int i = 0; i < KT; ++i) {{
        float w = fogSliceDepth((s + kTapZ[i] + h) / fogGrid.z) - fogSliceDepth(max(s + kTapZ[i] - h, 0.0) / fogGrid.z);
        float z = fogSliceDepth(max(s + kTapZ[i], 0.0) / fogGrid.z);
        acc += w * fogCell(g_tapDir[i] * z);
        wsum += w;
    }}
    vec4 cur = acc / max(wsum, 1e-6);
    vec3 rel = dirZ * fogSliceDepth((s + 0.5) / fogGrid.z);
    float share = {share};
    if (share < 1.0) {{
        vec3 pv = vec3(dot(prevRow0.xyz, rel) + prevRow0.w, dot(prevRow1.xyz, rel) + prevRow1.w,
                       dot(prevRow2.xyz, rel) + prevRow2.w);
        if (pv.z > 0.05) {{
            vec2 puv = pv.xy * prevProj.xy / pv.z * 0.5 + 0.5;
            float pt = prevProj.w < 1e-3 ? pv.z / prevProj.z
                                         : log(1.0 + pv.z * (exp(prevProj.w) - 1.0) / prevProj.z) / prevProj.w;
            if (puv.x >= 0.0 && puv.x <= 1.0 && puv.y >= 0.0 && puv.y <= 1.0 && pt <= 1.0)
                cur = mix(HISTREAD, cur, share);
        }}
    }}
    return cur;
}}

void main() {{
    for (int i = 0; i < KT; ++i) {{
        vec2 ndcI = (gl_FragCoord.xy + kTapXY[i]) / fogGrid.xy * 2.0 - 1.0;
        g_tapDir[i] = viewToWorld * vec3(ndcI.x * projInfo.z, ndcI.y * projInfo.w, 1.0);
    }}
    vec2 ndc = gl_FragCoord.xy / fogGrid.xy * 2.0 - 1.0;
"""
    if a.hblur:
        hr = ("0.25 * (texture(fogHistory, vec3(puv, pt) + vec3(0.25) / fogGrid.xyz * vec3(1.0, 1.0, 1.0)) + "
              "texture(fogHistory, vec3(puv, pt) + vec3(0.25) / fogGrid.xyz * vec3(1.0, -1.0, -1.0)) + "
              "texture(fogHistory, vec3(puv, pt) + vec3(0.25) / fogGrid.xyz * vec3(-1.0, 1.0, -1.0)) + "
              "texture(fogHistory, vec3(puv, pt) + vec3(0.25) / fogGrid.xyz * vec3(-1.0, -1.0, 1.0)))")
    else:
        hr = "texture(fogHistory, vec3(puv, pt))"
    new = new.replace("HISTREAD", hr)
    out = src[:i0] + new + src[i1:]
    d = os.path.join(OUT, a.variant)
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, "psFogInject.glsl"), "w", encoding="utf-8", newline="\n") as f:
        f.write(out)
    print("wrote", d, "taps", n, "xy", a.xy, "share", share)


if __name__ == "__main__":
    main()
