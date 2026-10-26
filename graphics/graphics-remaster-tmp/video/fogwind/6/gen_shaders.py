# agent 6 (fogwind): build the spatial-cancellation shader overrides.
#   S2   = 2x2 antithetic jitter + history at the cell/slice centre + tent in psFogIntegrate
#   S2nt = S2 without the tent (stock integrate)
#   S1   = d91a08e0 per-cell inject (unchanged) + tent in psFogIntegrate
#   C0   = frame-wide jitter (pre-d91) + history at the centre (no tent)
# Run: python gen_shaders.py   (writes shaders/<variant>/psFogInject.glsl [+ psFogIntegrate.glsl])
import os, re

HERE = os.path.dirname(os.path.abspath(__file__)).replace('\\', '/')
BASE = 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/3/shaders/percell/psFogInject.glsl'
SRC_CPP = 'D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/engine/PoseidonGL33/EngineGL33_PostFogShaders.cpp'

base = open(BASE, encoding='utf-8').read()

OLD_SLICE_START = base.index('// The jitter of a cell: the frame\'s offset')
OLD_MAIN_HEAD_END = base.index('    vec2 ndc = (gl_FragCoord.xy + cellJ) / fogGrid.xy * 2.0 - 1.0;')
OLD_MAIN_HEAD_END = base.index('\n', OLD_MAIN_HEAD_END) + 1
assert 0 < OLD_SLICE_START < OLD_MAIN_HEAD_END

HIST = r'''
// History at a point (camera-relative now), blended with cur by share.
vec4 fogHistoryAt(vec3 rel, vec4 cur) {
    if (fogFrame.z < 1.0) {
        vec3 pv = vec3(dot(prevRow0.xyz, rel) + prevRow0.w, dot(prevRow1.xyz, rel) + prevRow1.w,
                       dot(prevRow2.xyz, rel) + prevRow2.w);
        if (pv.z > 0.05) {
            vec2 puv = pv.xy * prevProj.xy / pv.z * 0.5 + 0.5;
            float pt = prevProj.w < 1e-3 ? pv.z / prevProj.z
                                         : log(1.0 + pv.z * (exp(prevProj.w) - 1.0) / prevProj.z) / prevProj.w;
            if (puv.x >= 0.0 && puv.x <= 1.0 && puv.y >= 0.0 && puv.y <= 1.0 && pt <= 1.0)
                cur = mix(texture(fogHistory, vec3(puv, pt)), cur, fogFrame.z);
        }
    }
    return cur;
}
'''

# S2 / S2nt: 2x2 antithetic block jitter, history read at the cell and slice centre.
ANTI = HIST + r'''
// agent 6: the jitter of a cell is one of the four 90-degree rotations of the frame's
// offset r (cos t, sin t), by the cell's place in its 2x2 block (diagonal cells opposite),
// and in depth r cos(t + k pi/2 + pi/2 + golden angle * slice). The four cells of a block
// sum to zero every frame and their second moments are constant in time, so the history's
// residual is a 2-periodic pattern that can only stand, never travel, and the integration's
// 3x3 tent (zero response at the 2x2 frequencies) removes it.
const float kAntiR = 0.408248; // sqrt(1/6): a box's second moment per axis
float g_angle = 0.0;           // this cell's angle this frame
vec3 g_dirC = vec3(0.0);       // the column through the cell centre (history lookup)

vec4 fogSlice(float s, vec3 dirZ) {
    float dz = kAntiR * cos(g_angle + 1.5707963 + 2.3999632 * s);
    float z = fogSliceDepth((s + 0.5 + dz) / fogGrid.z);
    vec4 cur = fogCell(dirZ * z);
    return fogHistoryAt(g_dirC * fogSliceDepth((s + 0.5) / fogGrid.z), cur);
}

void main() {
    ivec2 ci = ivec2(gl_FragCoord.xy);
    int px = ci.x & 1, py = ci.y & 1;
    int kq = py == 0 ? px : 3 - px;
    g_angle = 6.2831853 * fogDepth.w + 1.5707963 * float(kq);
    vec2 cellJ = kAntiR * vec2(cos(g_angle), sin(g_angle));
    vec2 ndcC = gl_FragCoord.xy / fogGrid.xy * 2.0 - 1.0;
    g_dirC = viewToWorld * vec3(ndcC.x * projInfo.z, ndcC.y * projInfo.w, 1.0);
    vec2 ndc = (gl_FragCoord.xy + cellJ) / fogGrid.xy * 2.0 - 1.0;
'''

# C0: the pre-d91 frame-wide jitter, history at the centre.
C0 = HIST + r'''
vec3 g_dirC = vec3(0.0);
vec4 fogSlice(float s, vec3 dirZ) {
    float z = fogSliceDepth((s + fogDepth.w) / fogGrid.z);
    vec4 cur = fogCell(dirZ * z);
    return fogHistoryAt(g_dirC * fogSliceDepth((s + 0.5) / fogGrid.z), cur);
}

void main() {
    vec2 cellJ = fogFrame.xy;
    vec2 ndcC = gl_FragCoord.xy / fogGrid.xy * 2.0 - 1.0;
    g_dirC = viewToWorld * vec3(ndcC.x * projInfo.z, ndcC.y * projInfo.w, 1.0);
    vec2 ndc = (gl_FragCoord.xy + cellJ) / fogGrid.xy * 2.0 - 1.0;
'''


def inject(body):
    return base[:OLD_SLICE_START] + body + base[OLD_MAIN_HEAD_END:]


# The integrate shader from the engine source, with the tent.
cpp = open(SRC_CPP, encoding='utf-8').read()
m = re.search(r'const char kPostFogIntegrateGLSL\[\] = R"\((.*?)\)";', cpp, re.S)
integ = m.group(1)
OLD_FETCH = '    vec4 v = texelFetch(fogInject, ivec3(cell, s), 0) * 0.001;'
assert OLD_FETCH in integ
NEW_FETCH = r'''    // agent 6: the injected volume through a 3x3 tent [1 2 1] x [1 2 1] / 16 in xy (four
    // bilinear taps at +-0.5 cell, the slice centre in depth): zero response at the 2x2
    // block's frequencies, so the jitter's standing residual cancels.
    vec3 tc = vec3((vec2(cell) + 0.5) / fogGrid.xy, (float(s) + 0.5) / fogGrid.z);
    vec2 hc = 0.5 / fogGrid.xy;
    vec4 v = (texture(fogInject, tc + vec3(-hc.x, -hc.y, 0.0)) + texture(fogInject, tc + vec3(hc.x, -hc.y, 0.0)) +
              texture(fogInject, tc + vec3(-hc.x, hc.y, 0.0)) + texture(fogInject, tc + vec3(hc.x, hc.y, 0.0))) *
             0.00025;'''
integ_tent = integ.replace(OLD_FETCH, NEW_FETCH)

variants = {
    'S2': (inject(ANTI), integ_tent),
    'S2nt': (inject(ANTI), None),
    'S1': (base, integ_tent),
    'C0': (inject(C0), None),
}
for name, (inj, itg) in variants.items():
    d = os.path.join(HERE, 'shaders', name)
    os.makedirs(d, exist_ok=True)
    open(os.path.join(d, 'psFogInject.glsl'), 'w', encoding='utf-8', newline='\n').write(inj)
    p = os.path.join(d, 'psFogIntegrate.glsl')
    if itg is not None:
        open(p, 'w', encoding='utf-8', newline='\n').write(itg)
    elif os.path.exists(p):
        os.remove(p)
    print(name, len(inj), 'integrate' if itg else '')
