"""Lists the connected pieces of LOD0 of a built P3D: size, box, main bones, sheet. A look at how a model is put together."""
import sys
import numpy as np
import p3d_pose as pp

def components(n, tris):
    parent = np.arange(n)
    def find(a):
        while parent[a] != a:
            parent[a] = parent[parent[a]]
            a = parent[a]
        return a
    for t in tris:
        a, b, c = find(t[0]), find(t[1]), find(t[2])
        parent[b] = a
        parent[c] = a
    return np.array([find(i) for i in range(n)])

if __name__ == "__main__":
    m = pp.load(sys.argv[1])
    lod = m.visual[int(sys.argv[2]) if len(sys.argv) > 2 else 0]
    comp = components(len(lod.points), lod.tris)
    bones = [b for b in lod.sel if not b.startswith("component")]
    w = np.zeros((len(bones), len(lod.points)))
    for k, b in enumerate(bones):
        for i, x in lod.sel[b].items():
            w[k, i] = x
    main = np.argmax(w, axis=0)
    tri_comp = comp[lod.tris[:, 0]]
    rows = []
    for c in np.unique(comp):
        idx = np.nonzero(comp == c)[0]
        tr = np.nonzero(tri_comp == c)[0]
        if len(tr) == 0:
            continue
        p = lod.points[idx]
        bc = {}
        for i in idx:
            bc[bones[main[i]]] = bc.get(bones[main[i]], 0) + 1
        tex = sorted({lod.tri_tex[t].split("\\")[-1].replace(".paa", "") for t in tr})
        rows.append((len(tr), len(idx), p.min(axis=0), p.max(axis=0), sorted(bc.items(), key=lambda kv: -kv[1])[:4], tex))
    rows.sort(key=lambda r: -r[0])
    print("%d pieces, %d triangles" % (len(rows), len(lod.tris)))
    for r in rows[: int(sys.argv[3]) if len(sys.argv) > 3 else 60]:
        print("%5d tris %5d pts  x %6.1f..%6.1f  y %5.1f..%5.1f  z %6.1f..%6.1f  %s  %s" % (
            r[0], r[1], r[2][0], r[3][0], r[2][1], r[3][1], r[2][2], r[3][2], r[5], r[4]))
