import sys
sys.path.insert(0, r"D:\Arma_CWA_worktrees\kaiju\arma_CWR\apps\tools\BlenderAddon\io_import_p3d")
import mlod_writer as mw
for path in sys.argv[1:]:
    lods = mw.read_mlod(open(path, "rb").read())
    lod = (lods[1] if isinstance(lods, tuple) else lods)[0]
    vol = 0.0
    for f in lod.faces:
        c = [lod.points[x[0]] for x in f.corners]
        for k in range(1, len(c) - 1):
            a, b, d = c[0], c[k], c[k + 1]
            vol += (a[0] * (b[1] * d[2] - b[2] * d[1]) - a[1] * (b[0] * d[2] - b[2] * d[0]) + a[2] * (b[0] * d[1] - b[1] * d[0])) / 6.0
    print("%-70s LOD0 faces %6d  signed volume in file order %+.1f m3" % (path[-70:], len(lod.faces), vol))
