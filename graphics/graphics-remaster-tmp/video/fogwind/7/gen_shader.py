"""Agent 7 (fogwind swarm): build psFogInject overrides with a stable far field.

python gen_shader.py <name> --from 80 --to 160 --taps 4 --xy spread|centre [--share 1.0] [--near percell|frozen]

Slices whose mid depth lies past --from (ramped to --to) take --taps FIXED taps (depth strata x an R2
pattern in the cell), averaged, without jitter and (share 1) without history. Nearer slices keep the
per-cell jitter + history of d91a08e0 (--near percell). The column's light (g_air, ring, sun share,
lobe) is evaluated at the cell centre for every slice.
Writes shaders/<name>/psFogInject.glsl next to this script.
"""
import argparse
import os

HERE = os.path.dirname(os.path.abspath(__file__)).replace('\\', '/')
BASE = 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/mist/6/shaders/percell/psFogInject.glsl'

OLD_SLICE_START = '// The cell of slice s on this column: this frame\'s jittered point, blended with'
OLD_MAIN_HEAD = '''void main() {
    g_cellJ = fogIgn(gl_FragCoord.xy);
    vec2 cj = fract(fogFrame.xy + 0.5 + vec2(fogIgn(gl_FragCoord.xy + vec2(17.0, 59.0)), fogIgn(gl_FragCoord.xy + vec2(83.0, 29.0)))) - 0.5;
    vec2 ndc = (gl_FragCoord.xy + cj) / fogGrid.xy * 2.0 - 1.0;
    vec3 dirZ = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0); // per metre of view depth
    vec3 d = normalize(dirZ);'''

NEW_MAIN_HEAD = '''void main() {
    g_cellJ = fogIgn(gl_FragCoord.xy);
    vec2 cj = fract(fogFrame.xy + 0.5 + vec2(fogIgn(gl_FragCoord.xy + vec2(17.0, 59.0)), fogIgn(gl_FragCoord.xy + vec2(83.0, 29.0)))) - 0.5;
    NEAR_XY
    vec2 ndcJ = (gl_FragCoord.xy + cj) / fogGrid.xy * 2.0 - 1.0;
    vec3 dirJ = viewToWorld * vec3(ndcJ.x * projInfo.z, ndcJ.y * projInfo.w, 1.0); // the near slices' jittered column
    g_cellC = gl_FragCoord.xy;
    // agent 7: the column's light at the cell centre (no jitter in it)
    vec2 ndc = gl_FragCoord.xy / fogGrid.xy * 2.0 - 1.0;
    vec3 dirZ = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0); // per metre of view depth
    vec3 d = normalize(dirZ);'''

FAR_CODE = '''// agent 7: the stable far field. Past kFarFrom (ramped to kFarTo) a slice's cell is the mean of
// kFarTaps fixed points of the cell (depth strata, an R2 pattern across the cell), the same every
// frame: no jitter, so nothing for a history to average and no residual to travel or pump.
const float kFarFrom = FAR_FROM;
const float kFarTo = FAR_TO;
const int kFarTaps = FAR_TAPS;
const float kFarShare = FAR_SHARE; // the far field's history share (1 = none)
const float kFarXY = FAR_XY;
const bool kFarVisOnce = FAR_VISONCE; // the sun's visibility once per far cell (its middle tap)       // 1 = taps spread over the cell, 0 = at its centre column
vec2 g_cellC = vec2(0.5);

vec4 fogHistoryAt(vec3 rel, vec4 cur, float share) {
    if (share < 1.0) {
        vec3 pv = vec3(dot(prevRow0.xyz, rel) + prevRow0.w, dot(prevRow1.xyz, rel) + prevRow1.w,
                       dot(prevRow2.xyz, rel) + prevRow2.w);
        if (pv.z > 0.05) {
            vec2 puv = pv.xy * prevProj.xy / pv.z * 0.5 + 0.5;
            float pt = prevProj.w < 1e-3 ? pv.z / prevProj.z
                                         : log(1.0 + pv.z * (exp(prevProj.w) - 1.0) / prevProj.z) / prevProj.w;
            if (puv.x >= 0.0 && puv.x <= 1.0 && puv.y >= 0.0 && puv.y <= 1.0 && pt <= 1.0)
                cur = mix(texture(fogHistory, vec3(puv, pt)), cur, share);
        }
    }
    return cur;
}

vec4 fogFar(float s) {
    vec4 acc = vec4(0.0);
    g_visFix = -1.0;
    for (int k = 0; k < kFarTaps; ++k) {
        int i = (k + kFarTaps / 2) % kFarTaps; // the middle tap first (its sun visibility may serve all)
        float fi = float(i);
        vec2 o = kFarXY * (kFarTaps == 2 ? vec2(fi - 0.5) * 0.5 : fract(vec2(0.5) + fi * vec2(0.7548776662, 0.5698402910)) - 0.5);
        vec2 ndc = (g_cellC + o) / fogGrid.xy * 2.0 - 1.0;
        vec3 dz = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0);
        float z = fogSliceDepth((s + (fi + 0.5) / float(kFarTaps)) / fogGrid.z);
        acc += fogCell(dz * z);
        if (kFarVisOnce) g_visFix = g_visLast;
    }
    g_visFix = -1.0;
    acc *= 1.0 / float(kFarTaps);
    if (kFarShare < 1.0 && fogFrame.z < 1.0) {
        vec2 ndc = g_cellC / fogGrid.xy * 2.0 - 1.0;
        vec3 dz = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0);
        acc = fogHistoryAt(dz * fogSliceDepth((s + 0.5) / fogGrid.z), acc, kFarShare);
    }
    return acc;
}

// The cell of slice s on this column: this frame\'s jittered point, blended with
// the last frame\'s volume at the same point (the near slices), or the far field.
float fogIgn(vec2 p) { return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715)))); }
float g_cellJ = 0.0;
vec4 fogSlice(float s, vec3 dirZ) {
    float w = smoothstep(kFarFrom, kFarTo, fogSliceDepth((s + 0.5) / fogGrid.z));
    vec4 res = vec4(0.0);
    if (w < 1.0) {
        float z = fogSliceDepth((s + NEAR_Z) / fogGrid.z);
        vec3 rel = dirZ * z;
        res = fogHistoryAt(rel, fogCell(rel), fogFrame.z);
    }
    if (w > 0.0) res = mix(res, fogFar(s), w);
    return res;
}
'''


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('name')
    ap.add_argument('--from', dest='frm', type=float, default=80.0)
    ap.add_argument('--to', type=float, default=160.0)
    ap.add_argument('--taps', type=int, default=4)
    ap.add_argument('--xy', choices=['spread', 'centre'], default='spread')
    ap.add_argument('--share', type=float, default=1.0)
    ap.add_argument('--near', choices=['percell', 'frozen'], default='percell')
    ap.add_argument('--visonce', action='store_true')
    a = ap.parse_args()
    src = open(BASE, encoding='utf-8').read().replace('\r\n', '\n')
    i0 = src.index(OLD_SLICE_START)
    i1 = src.index('void main() {', i0)
    assert OLD_MAIN_HEAD in src
    far = (FAR_CODE.replace('FAR_FROM', '%.1f' % a.frm).replace('FAR_TO', '%.1f' % a.to)
           .replace('FAR_TAPS', str(a.taps)).replace('FAR_SHARE', '%.3f' % a.share)
           .replace('FAR_XY', '1.0' if a.xy == 'spread' else '0.0')
           .replace('FAR_VISONCE', 'true' if a.visonce else 'false'))
        # near depth: per-cell jitter (d91a08e0) or the slice centre
    nz = 'fract(fogDepth.w + g_cellJ + 0.618034 * s)' if a.near == 'percell' else '0.5'
    far = far.replace('NEAR_Z', nz)
    head = NEW_MAIN_HEAD.replace('NEAR_XY', '' if a.near == 'percell' else 'cj = vec2(0.0);')
    out = src[:i0] + far + '\n' + src[i1:]
    vis_old = '        vis = fogSunVisibility(rel);'
    assert out.count(vis_old) == 1 and out.count('vec4 fogCell(vec3 rel) {') == 1
    out = out.replace(vis_old, '        vis = g_visFix >= 0.0 ? g_visFix : fogSunVisibility(rel);\n        g_visLast = vis;')
    out = out.replace('vec4 fogCell(vec3 rel) {',
                      'float g_visFix = -1.0; // agent 7: >= 0 = this sun visibility for the cell\n'
                      'float g_visLast = 1.0;\nvec4 fogCell(vec3 rel) {')
    assert 'float g_visFix' in out and 'g_visLast = vis;' in out
    out = out.replace(OLD_MAIN_HEAD, head)
    out = out.replace('fog0 = fogSlice(base + 0.0, dirZ);', 'fog0 = fogSlice(base + 0.0, dirJ);')
    for k in range(1, 8):
        out = out.replace('fog%d = fogSlice(base + %d.0, dirZ);' % (k, k), 'fog%d = fogSlice(base + %d.0, dirJ);' % (k, k))
    assert out.count('fogSlice(base') == 8 and out.count(', dirJ);') == 8
    d = os.path.join(HERE, 'shaders', a.name)
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, 'psFogInject.glsl'), 'w', encoding='utf-8', newline='\n') as f:
        f.write(out)
    print('wrote', d, vars(a))


if __name__ == '__main__':
    main()
