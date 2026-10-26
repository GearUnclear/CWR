"""Agent 2 (fogwind): make psFogInject overrides with temporally decorrelated per-cell jitter.

Source: tmp/video/mist/3/shaders/percell/psFogInject.glsl (d91a08e0's per-cell IGN version, assembled).
Replaces the four jitter lines. Writes shaders/<variant>/psFogInject.glsl.

A  per-cell WHITE hash rotation of the frame's low-discrepancy sequences (Cranley-Patterson with a
   hash per cell and per slice; no structured pattern, so nothing can translate).
B  hash(cell, slice, frame): white in time.
C  like A, but each cell also picks one of the three frame sequences (depth golden, x and y plastic)
   and its sign for each axis, so neighbouring cells do not share one temporal frequency.
D  like A, with a high-passed ("blue-ish") phase field: the cell's hash minus the mean of its
   8 neighbours, so neighbouring phases differ more than white noise and blobs of like phase are rarer.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
SRC = os.path.normpath(os.path.join(ROOT, '..', '..', 'mist', '3', 'shaders', 'percell', 'psFogInject.glsl'))

OLD_IGN = "float fogIgn(vec2 p) { return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715)))); }"
OLD_G = "float g_cellJ = 0.0;"
OLD_Z = "    float z = fogSliceDepth((s + fract(fogDepth.w + g_cellJ + 0.618034 * s)) / fogGrid.z);"
OLD_SET = "    g_cellJ = fogIgn(gl_FragCoord.xy);"
OLD_CJ = "    vec2 cj = fract(fogFrame.xy + 0.5 + vec2(fogIgn(gl_FragCoord.xy + vec2(17.0, 59.0)), fogIgn(gl_FragCoord.xy + vec2(83.0, 29.0)))) - 0.5;"

HASH = """uint fogHashU(uint x) {
    x ^= x >> 16u; x *= 0x7feb352du; x ^= x >> 15u; x *= 0x846ca68bu; x ^= x >> 16u;
    return x;
}
float fogHash3(uvec3 p) { return float(fogHashU(p.x + fogHashU(p.y + fogHashU(p.z))) >> 8u) * (1.0 / 16777216.0); }
uvec2 g_cell = uvec2(0u);
uint g_frameKey = 0u;
float g_dSeq = 0.0; float g_dSign = 1.0;"""

def variant(name):
    s = open(SRC, encoding='utf-8').read()
    for o in (OLD_IGN, OLD_G, OLD_Z, OLD_SET, OLD_CJ):
        assert s.count(o) == 1, o
    s = s.replace(OLD_IGN, HASH).replace(OLD_G, '')
    if name == 'A':
        z = "    float z = fogSliceDepth((s + fract(fogDepth.w + fogHash3(uvec3(g_cell, uint(s) + 7u)))) / fogGrid.z);"
        st = "    g_cell = uvec2(gl_FragCoord.xy);"
        cj = ("    vec2 cj = fract(fogFrame.xy + 0.5 + vec2(fogHash3(uvec3(g_cell, 1001u)), fogHash3(uvec3(g_cell, 2003u)))) - 0.5;")
    elif name == 'B':
        z = "    float z = fogSliceDepth((s + fogHash3(uvec3(g_cell, (uint(s) + 7u) ^ g_frameKey))) / fogGrid.z);"
        st = ("    g_cell = uvec2(gl_FragCoord.xy);\n"
              "    g_frameKey = fogHashU(floatBitsToUint(fogDepth.w) ^ fogHashU(floatBitsToUint(fogFrame.x) + 0x9e3779b9u * floatBitsToUint(fogFrame.y)));")
        cj = ("    vec2 cj = vec2(fogHash3(uvec3(g_cell, 1001u ^ g_frameKey)), fogHash3(uvec3(g_cell, 2003u ^ g_frameKey))) - 0.5;")
    elif name == 'C':
        # three frame sequences in 0..1: depth golden, x and y plastic; each axis picks one per cell and a sign
        z = "    float z = fogSliceDepth((s + fract(g_dSign * g_dSeq + fogHash3(uvec3(g_cell, uint(s) + 7u)))) / fogGrid.z);"
        st = ("    g_cell = uvec2(gl_FragCoord.xy);\n"
              "    vec3 seqs = vec3(fogDepth.w, fogFrame.x + 0.5, fogFrame.y + 0.5);\n"
              "    uint pick = fogHashU(g_cell.x * 73856093u ^ g_cell.y * 19349663u);\n"
              "    int i0 = int(pick % 3u); int i1 = int((pick / 3u) % 3u); int i2 = int((pick / 9u) % 3u);\n"
              "    float s0 = ((pick >> 8u) & 1u) == 1u ? -1.0 : 1.0; float s1 = ((pick >> 9u) & 1u) == 1u ? -1.0 : 1.0;\n"
              "    float s2 = ((pick >> 10u) & 1u) == 1u ? -1.0 : 1.0;\n"
              "    g_dSeq = seqs[i0]; g_dSign = s0;")
        cj = ("    vec2 cj = fract(vec2(s1 * seqs[i1], s2 * seqs[i2]) + vec2(fogHash3(uvec3(g_cell, 1001u)), fogHash3(uvec3(g_cell, 2003u)))) - 0.5;")
    elif name == 'D':
        z = "    float z = fogSliceDepth((s + fract(fogDepth.w + fogBluish(uint(s) + 7u))) / fogGrid.z);"
        st = "    g_cell = uvec2(gl_FragCoord.xy);"
        cj = ("    vec2 cj = fract(fogFrame.xy + 0.5 + vec2(fogBluish(1001u), fogBluish(2003u))) - 0.5;")
        s = s.replace("float g_dSeq = 0.0; float g_dSign = 1.0;", """float g_dSeq = 0.0; float g_dSign = 1.0;
// A blue-ish phase: the cell's hash against the mean of its 8 neighbours (a high pass of white noise),
// so neighbouring cells' phases differ more than white noise and like-phase blobs are rarer.
float fogBluish(uint salt) {
    ivec2 c = ivec2(g_cell);
    float m = 0.0;
    for (int j = -1; j <= 1; ++j)
        for (int i = -1; i <= 1; ++i)
            if (i != 0 || j != 0) m += fogHash3(uvec3(uvec2(c + ivec2(i, j)), salt));
    return fract(0.5 + 1.5 * (fogHash3(uvec3(g_cell, salt)) - m / 8.0));
}""")
    else:
        raise SystemExit('unknown variant ' + name)
    s = s.replace(OLD_Z, z).replace(OLD_SET, st).replace(OLD_CJ, cj)
    out = os.path.join(ROOT, 'shaders', name)
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, 'psFogInject.glsl'), 'w', encoding='utf-8', newline='\n') as f:
        f.write(s)
    print('wrote', os.path.join(out, 'psFogInject.glsl'))

OLD_SLICE = "vec4 fogSlice(float s, vec3 dirZ) {"
NL = chr(10)
OLD_PV = ("        vec3 pv = vec3(dot(prevRow0.xyz, rel) + prevRow0.w, dot(prevRow1.xyz, rel) + prevRow1.w," + NL +
          "                       dot(prevRow2.xyz, rel) + prevRow2.w);")
OLD_DIR = "    vec3 d = normalize(dirZ);"


def centre(name, base):
    # <base> + the history read at the UNJITTERED cell centre (agent 6's finding: reading it at the
    # jittered point makes the stored offset a leaky sum of the jitter, gain 8.3).
    p = os.path.join(ROOT, 'shaders', base, 'psFogInject.glsl')
    s = open(p, encoding='utf-8').read()
    for o in (OLD_SLICE, OLD_PV, OLD_DIR):
        assert s.count(o) == 1, o
    s = s.replace(OLD_SLICE, "vec3 g_dirC = vec3(0.0);" + NL + OLD_SLICE + NL +
                  "    vec3 relC = g_dirC * fogSliceDepth((s + 0.5) / fogGrid.z);")
    s = s.replace(OLD_PV, OLD_PV.replace("rel)", "relC)"))
    s = s.replace(OLD_DIR, OLD_DIR + NL + "    {" + NL +
                  "        vec2 ndcC = gl_FragCoord.xy / fogGrid.xy * 2.0 - 1.0;" + NL +
                  "        g_dirC = viewToWorld * vec3(ndcC.x * projInfo.z, ndcC.y * projInfo.w, 1.0);" + NL + "    }")
    out = os.path.join(ROOT, 'shaders', name)
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, 'psFogInject.glsl'), 'w', encoding='utf-8', newline=NL) as f:
        f.write(s)
    print('wrote', os.path.join(out, 'psFogInject.glsl'))


OLD_FETCH = "                cur = mix(texture(fogHistory, vec3(puv, pt)), cur, fogFrame.z);"


def tetra(name, base, a):
    # <base> (a centre-read variant) + a FIXED blur in the history read: 4 trilinear taps at the corners of a
    # tetrahedron, +-a cell / slice in each axis (each axis gets the kernel [a/2, 1-a, a/2], variance a, no
    # cross terms). The old read at the jittered point blurred the stored volume at random every frame (that blur
    # is part of the shipped look); this does it the same every frame, so it adds no noise.
    p = os.path.join(ROOT, 'shaders', base, 'psFogInject.glsl')
    s = open(p, encoding='utf-8').read()
    assert s.count(OLD_FETCH) == 1
    k = "%.4f" % a
    new = ("            {" + NL + "                vec3 tq = vec3(puv, pt), te = vec3(" + k + ") / fogGrid.xyz;" + NL +
           "                vec4 hist = 0.25 * (texture(fogHistory, tq + te * vec3(1.0, 1.0, 1.0)) + texture(fogHistory, tq + te * vec3(1.0, -1.0, -1.0)) +" + NL +
           "                                    texture(fogHistory, tq + te * vec3(-1.0, 1.0, -1.0)) + texture(fogHistory, tq + te * vec3(-1.0, -1.0, 1.0)));" + NL +
           "                cur = mix(hist, cur, fogFrame.z);" + NL + "            }")
    s = s.replace(OLD_FETCH, new)
    out = os.path.join(ROOT, 'shaders', name)
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, 'psFogInject.glsl'), 'w', encoding='utf-8', newline=NL) as f:
        f.write(s)
    print('wrote', os.path.join(out, 'psFogInject.glsl'))


for n in (sys.argv[1:] or ['A', 'B', 'C', 'D']):
    if n.startswith('Act'):
        tetra(n, 'Ac', float(n[3:]) / 100.0)  # Act25 = a 0.25
    elif n.endswith('c') and len(n) == 2:
        centre(n, n[0])
    else:
        variant(n)
